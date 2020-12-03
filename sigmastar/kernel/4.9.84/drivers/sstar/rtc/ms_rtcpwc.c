/*
* ms_rtcpwc.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include "ms_platform.h"
#include "ms_types.h"
#include "ms_msys.h"
#include "reg_rtcpwc.h"

#define DTS_DEFAULT_DATE            "default_date"

#define RTC_DEBUG  0
// #define RTC_CHECK_STATUS_DELAY_TIME_MS  2
#define RTC_CHECK_STATUS_DELAY_TIME_US  100

#define ISO_S0						0x00
#define ISO_S1						0x01
#define ISO_S2						0x03
#define ISO_S3						0x07
#define ISO_S4						0x05
#define ISO_S5						0x01

#define RTC_PASSWORD 0xBABE
 
#define ISO_ACK_RETRY_TIME			20

#if RTC_DEBUG
#define RTC_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define RTC_DBG(fmt, arg...)
#endif
#define RTC_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)


struct ms_rtc_info {
    struct platform_device *pdev;
    struct rtc_device *rtc_dev;
    void __iomem *rtc_base;
    u32 default_base;
    spinlock_t mutex;
};

int auto_wakeup_delay_seconds = 0;
//static AUTL_DATETIME        m_ShadowTime = {0};
static char _bInit = 0;
static char _bIsoctl_fail = 0;

static ssize_t isoctl_check_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "ISO EN sequence: %d\n", _bIsoctl_fail);
    return (str - buf);
}
DEVICE_ATTR(isoctl_check, 0444, isoctl_check_show, NULL);

static ssize_t auto_wakeup_timer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        while (*str && !isspace(*str)) str++;
        len = str - buf;
        if(len)
        {
            auto_wakeup_delay_seconds = simple_strtoul(buf, NULL, 10);
            //printk("\nauto_wakeup_delay_seconds=%d\n", auto_wakeup_delay_seconds);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}
static ssize_t auto_wakeup_timer_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", auto_wakeup_delay_seconds);
    return (str - buf);
}
DEVICE_ATTR(auto_wakeup_timer, 0644, auto_wakeup_timer_show, auto_wakeup_timer_store);

#if 0
static int ms_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;

    seconds = readw(info->rtc_base + REG_RTC_MATCH_VAL_L) | (readw(info->rtc_base + REG_RTC_MATCH_VAL_H) << 16);

    rtc_time_to_tm(seconds, &alarm->time);

    if( !(readw(info->rtc_base + REG_RTC_CTRL) & INT_MASK_BIT) )
        alarm->enabled = 1;

    RTC_DBG("ms_rtc_read_alarm[%d,%d,%d,%d,%d,%d], alarm_en=%d\n",
        alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec, alarm->enabled);


    return 0;
}

static int ms_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;
    u16 reg;

    RTC_DBG("ms_rtc_set_alarm[%d,%d,%d,%d,%d,%d], alarm_en=%d\n",
        alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec, alarm->enabled);

    rtc_tm_to_time(&alarm->time, &seconds);

    writew((seconds & 0xFFFF), info->rtc_base + REG_RTC_MATCH_VAL_L);
    writew((seconds>>16) & 0xFFFF, info->rtc_base + REG_RTC_MATCH_VAL_H);

    reg = readw(info->rtc_base + REG_RTC_CTRL);
    if(alarm->enabled)
    {
        writew(reg & ~(INT_MASK_BIT), info->rtc_base + REG_RTC_CTRL);
    }
    else
    {
        writew(reg | INT_MASK_BIT, info->rtc_base + REG_RTC_CTRL);
    }

    return 0;
}
#endif
//------------------------------------------------------------------------------
//  Function    : ms_RTC_IsValid
//  Description :
//------------------------------------------------------------------------------
/** @brief The function check if current RTC is valid

The function verify the RTC status
@return It reports the status of the operation.
*/
//BOOL ms_rtc_IsValid(struct device *dev)
//{
//    struct ms_rtc_info *info = dev_get_drvdata(dev);
//    U16 reg;

//    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_VAILD);
//    printk("ms_rtc_IsValid %x\n", reg);
//    reg = reg & RTCPWC_RTC2DIG_VAILD_BIT;
//    printk("ms_rtc_IsValid %x\n", reg);
//    return (reg) ? TRUE : FALSE;
//}
//------------------------------------------------------------------------------
//  Function    : ms_rtc_ISOCTL
//  Description :
//------------------------------------------------------------------------------
/** @brief The internal function to send ISO_EN control signal.

The function to enable ISO cell 
@return It reports the status of the operation.
*/
bool ms_rtc_ISOCTL_EX(struct device *dev)
{
    U8 ubCheck = ISO_ACK_RETRY_TIME;
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    U16 reg = 0 ;
    // Input ISO ctrl sequence ,  3'b000(S0) -> 3'b001(S1)  -> 3'b011(S2)  -> 3'b111(S3)  -> 3'b101(S4)  -> 3'b001(S5)  -> 3'b000(S0)
    // Following notes is from MV2
    // The switch of state needs delay, 1ms at least according to designer,
    // but in our test, set to 3ms will still causes incorrect data read.
    // And the sequence should be finished within 1 sec.

    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S0, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg ) && (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S1, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT)&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;


    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S2, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S3, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT) && (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S4, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S5, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S0, info->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);

    reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(info->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = 22;
    do
    {
        reg = readw(info->rtc_base + RTCPWC_DIG2PWC_RTC_TESTBUS);
        if (reg & RTCPWC_ISO_EN)
        {
            break;
        }
        udelay(100);
        ubCheck--;
    }
    while (ubCheck);

    if(ubCheck == 0)
        return FALSE;

    // [from designer Belon.Chen] wait 2 ms is must since read/write base/counter/SW0/SW1 is valid after iso state complete
    mdelay(2);
    return TRUE;
}

static int _ms_rtc_has_1k_clk(struct device *dev)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    U8 ubCheck = 22; /// delay 22 * 100 = 2200 us
    U16 reg = 0 ;

    do
    {
        reg = readw(info->rtc_base + RTCPWC_DIG2PWC_RTC_TESTBUS);
        if (reg & RTCPWC_CLK_1K)
        {
            return 1;
        }
        udelay(100);
        ubCheck--;
    }
    while (ubCheck);
    return 0;
}

void ms_rtc_ISOCTL(struct device *dev)
{
    static int warn_once = 0;
    U8 ubCheck = ISO_ACK_RETRY_TIME;

    if (0 == _ms_rtc_has_1k_clk(dev))
    {
        if (!warn_once)
        {
            warn_once = 1;
            printk("[%s][%d] RTCPWC fail to enter correct state and possibly caused by no power supplied\n", __FUNCTION__, __LINE__);
        }
        return;
    }
    while (!ms_rtc_ISOCTL_EX(dev) && (--ubCheck) )
    {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
    }
    if(ubCheck == 0)
        _bIsoctl_fail = 1;
}

//------------------------------------------------------------------------------
//  Function    : ms_RTC_GetSW0
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used for getting RTC SW0.

This function is used for getting RTC SW0.
@param[out] The value of RTC SW0(magic number).
@return It reports the status of the operation.
*/
// SW0 has only 16 bits
u32 ms_rtc_GetSW0(struct device *dev)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 BaseH = 0, BaseL = 0;
    u32 ulBaseTime = 0;
    u16 reg = 0;
    // I.   read SW0
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew( reg | RTCPWC_DIG2RTC_SW0_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    ms_rtc_ISOCTL(dev);
    // read base time
    BaseH = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_H);
    BaseL = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_L);
    RTC_DBG("SW0 BaseH %x \n", BaseH);
    RTC_DBG("SW0 BaseL %x \n", BaseL);
    ulBaseTime = BaseH << 16;
    ulBaseTime |= BaseL;
    //reset read bit of base time
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_SW0_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    return ulBaseTime;
}

// SW0 has only 16 bits
void ms_rtc_SetSW0(struct device *dev, u32 val)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 reg = 0;
    //Set sw bit
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg | RTCPWC_DIG2RTC_SW0_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    // Set sw password
    writew(((val>> 0) & 0xFFFF), info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L);
    RTC_DBG("Set RTC SetSW0=%x\r\n", readw(info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L));
    //Trigger ISO
    ms_rtc_ISOCTL(dev);
    //reset control bits
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_SW0_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
}

#ifdef CONFIG_RTCPWC_INNER_EHHE
// SW1 has only 16 bits
u32 ms_rtc_GetSW1(struct device *dev)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 BaseH = 0, BaseL = 0;
    u32 ulBaseTime = 0;
    u16 reg = 0;
    // I.   read SW1
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew( reg | RTCPWC_DIG2RTC_SW1_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    ms_rtc_ISOCTL(dev);
    // read base time
    BaseH = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_H);
    BaseL = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_L);
    ulBaseTime = BaseH << 16;
    ulBaseTime |= BaseL;
    //reset read bit of base time
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_SW1_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    return ulBaseTime;
}

// SW1 has only 16 bits
void ms_rtc_SetSW1(struct device *dev, u32 val)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 reg = 0;
    //Set sw bit
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg | RTCPWC_DIG2RTC_SW1_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    // Set sw password
    writew(((val>> 0) & 0xFFFF), info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L);
    RTC_DBG("Set RTC SetSW1=%x\r\n", readw(info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L));
    //Trigger ISO
    ms_rtc_ISOCTL(dev);
    //reset control bits
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_SW1_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
}
#endif

//------------------------------------------------------------------------------
//  Function    : ms_RTC_SetBaseTime
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used for getting RTC BaseTime.

This function is used for getting RTC BaseTime.
@param[out] The value of RTC BaseTime.
@return It reports the status of the operation.
*/
void ms_rtc_SetBaseTime(struct device *dev, unsigned long   seconds)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 reg;

    // Toggle reset
    reg = readw(info->rtc_base + RTCPWC_DIG2PWC_OPT);
    writew(reg | RTCPWC_SW_RST, info->rtc_base + RTCPWC_DIG2PWC_OPT);
    mdelay(1);
    writew(reg, info->rtc_base + RTCPWC_DIG2PWC_OPT);

    //Set Base time bit
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg | RTCPWC_DIG2RTC_BASE_WR_BIT, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);

    // Set RTC Base Time
    writew(seconds, info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L);
    writew((seconds) >> 16, info->rtc_base + RTCPWC_DIG2RTC_WRDATA_H);
    RTC_DBG("Set RTC Base Time=%x\r\n", readw(info->rtc_base + RTCPWC_DIG2RTC_WRDATA_L));
    RTC_DBG("Set RTC Base Time=%x\r\n", readw(info->rtc_base + RTCPWC_DIG2RTC_WRDATA_H));

    //Trigger ISO
    ms_rtc_ISOCTL(dev);

    //Set counter RST bit
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg | RTCPWC_DIG2RTC_CNT_RST_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);

    //Trigger ISO
    ms_rtc_ISOCTL(dev);
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_CNT_RST_WR, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);

    //reset control bits
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_SET);
    writew(reg & ~RTCPWC_DIG2RTC_SET_BIT, info->rtc_base + RTCPWC_DIG2RTC_SET);
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_BASE_WR_BIT, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
}

#ifdef CONFIG_RTCPWC_INNER_EHHE
u32 _ms_rtc_GetBaseTime(struct device *dev)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 BaseH = 0, BaseL = 0;
    u32 ulBaseTime = 0;
    u16 reg;

    //reset read bit
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
    writew(reg & ~RTCPWC_DIG2RTC_CNT_RD_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);

    // Set read bit of base time
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg | RTCPWC_DIG2RTC_BASE_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    ms_rtc_ISOCTL(dev);
    // read base time
    BaseH = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_H);
    BaseL = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_L);
    RTC_DBG("BaseH %x \n", BaseH);
    RTC_DBG("BaseL %x \n", BaseL);
    ulBaseTime = BaseH << 16;
    ulBaseTime |= BaseL;
    //reset read bit of base time
    reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
    writew(reg & ~RTCPWC_DIG2RTC_BASE_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);

    return ulBaseTime;
}
#endif

//------------------------------------------------------------------------------
//  Function    : ms_RTC_GetBaseTime
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used for getting RTC BaseTime.

This function is used for getting RTC BaseTime.
@param[out] The value of RTC BaseTime.
@return It reports the status of the operation.
*/
u32 ms_rtc_GetBaseTime(struct device *dev)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u32 ulBaseTime = 0;
    u32 password = 0;
    password = ms_rtc_GetSW0(dev);

#ifdef CONFIG_RTCPWC_INNER_EHHE
    if ((password == RTC_PASSWORD) && 
        ((ms_rtc_GetSW1(dev) & 0xFFFF) == ((ulBaseTime = _ms_rtc_GetBaseTime(dev)) & 0xFFFF)))
#else
    if(password == RTC_PASSWORD)
#endif
    {
#ifndef CONFIG_RTCPWC_INNER_EHHE
        u16 reg;
        u16 BaseH = 0, BaseL = 0;

        //reset read bit
        reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
        writew(reg & ~RTCPWC_DIG2RTC_CNT_RD_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);

        // Set read bit of base time
        reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
        writew(reg | RTCPWC_DIG2RTC_BASE_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
        ms_rtc_ISOCTL(dev);
        // read base time
        BaseH = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_H);
        BaseL = readw(info->rtc_base + RTCPWC_RTC2DIG_RDDATA_L);
        RTC_DBG("BaseH %x \n", BaseH);
        RTC_DBG("BaseL %x \n", BaseL);
        ulBaseTime = BaseH << 16;
        ulBaseTime |= BaseL;
        //reset read bit of base time
        reg = readw(info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
        writew(reg & ~RTCPWC_DIG2RTC_BASE_RD, info->rtc_base + RTCPWC_DIG2RTC_BASE_WR);
#endif
    }
    else
    {
        RTC_ERR("Please set rtc timer (hwclock -w) \n");
        ms_rtc_SetBaseTime(dev, info->default_base);
        ms_rtc_SetSW0(dev, RTC_PASSWORD);
        ulBaseTime = info->default_base;
#ifdef CONFIG_RTCPWC_INNER_EHHE
        ms_rtc_SetSW1(dev, info->default_base);
#endif // #ifdef CONFIG_RTCPWC_INNER_EHHE
    }
    return ulBaseTime;
}

//------------------------------------------------------------------------------
//  Function    : ms_rtc_read_time
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used for getting RTC information.

This function is used for getting RTC time information.
@warning This function uses OS sleep, please don't call this function in ISR.
@param[in] pointer of structure AUTL_DATETIME.
@return It reports the status of the operation.
*/
static int ms_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    u16 reg = 0;
    u32 run_sec = 0;
    u32 chk_times = 5;
    u64 ullSeconds = 0;
    u16 counterH = 0, counterL = 0;
    int m_ulBaseTimeInSeconds = 0;
    unsigned long flags;

    if (0 == _bInit)
        return 0;

    spin_lock_irqsave(&info->mutex, flags);

    m_ulBaseTimeInSeconds = ms_rtc_GetBaseTime(dev);

    RTC_DBG("m_ulBaseTimeInSeconds= 0x%x\r\n", m_ulBaseTimeInSeconds);

    if(RTC_PASSWORD == ms_rtc_GetSW0(dev))
    {
        // Read RTC Counter
        do
        {
            //Set read bit of RTC counter
            reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
            writew(reg | RTCPWC_DIG2RTC_CNT_RD_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);

            //Trigger ISO
            ms_rtc_ISOCTL(dev);
            chk_times = 5;

            //Latch RTC counter and Check valid bit of RTC counter
            do
            {
                reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD_TRIG);
                writew(reg | RTCPWC_DIG2RTC_CNT_RD_TRIG_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD_TRIG);
                //Note : The first to retrieve RTC counter will failed without below delay
                mdelay(5);
            }while((readw(info->rtc_base + RTCPWC_RTC2DIG_CNT_UPDATING) & RTCPWC_RTC2DIG_CNT_UPDATING_BIT) && (chk_times--));

            if(chk_times == 0)
            {
                RTC_ERR("Check valid bit of RTC counter failed!\n");
                //Reset read bit of RTC counter
                reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
                writew(reg & ~RTCPWC_DIG2RTC_CNT_RD_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
                spin_unlock_irqrestore(&info->mutex, flags);
                return 0;
            }

            //read RTC counter
            {
                counterH = readw(info->rtc_base + RTCPWC_REG_RTC2DIG_RDDATA_CNT_H);
                counterL = readw(info->rtc_base + RTCPWC_REG_RTC2DIG_RDDATA_CNT_L);
                run_sec =  counterH << 16;
                run_sec |= counterL;
                RTC_DBG("CounterL = 0x%x\r\n", counterL);
                RTC_DBG("CounterH = 0x%x\r\n", counterH);
            }
            //Reset read bit of RTC counter
            reg = readw(info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
            writew(reg & ~RTCPWC_DIG2RTC_CNT_RD_BIT, info->rtc_base + RTCPWC_DIG2RTC_CNT_RD);
        } while(0);
    }
    RTC_DBG("run_sec = 0x%x\r\n", run_sec);
    RTC_DBG("m_ulBaseTimeInSeconds = 0x%x\r\n", m_ulBaseTimeInSeconds);
    ullSeconds = m_ulBaseTimeInSeconds + run_sec;

    //_RTC_PRINT("Base = 0x%x, counter = 0x%x, ullSeconds = 0x%x\n",m_ulBaseTimeInSeconds,run_sec,ullSeconds);

    if (ullSeconds > 0xFFFFFFFF) {
        ullSeconds = 0xFFFFFFFF;
    }

    rtc_time_to_tm(ullSeconds, tm);

    RTC_DBG("ms_rtc_read_time[%d,%d,%d,%d,%d,%d]\n",
        tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

    spin_unlock_irqrestore(&info->mutex, flags);
    return rtc_valid_tm(tm);
}

static int ms_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long  seconds;
    unsigned long flags;

    if (0 == _bInit)
        return 0;

    spin_lock_irqsave(&info->mutex, flags);
    RTC_DBG("ms_rtc_set_time[%d,%d,%d,%d,%d,%d]\n",
        tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

    rtc_tm_to_time(tm, &seconds);
    RTC_DBG("RTC Set Time: Base=%ld\r\n", seconds);
    ms_rtc_SetBaseTime(dev, seconds);
    ms_rtc_SetSW0(dev, RTC_PASSWORD);
#ifdef CONFIG_RTCPWC_INNER_EHHE
    ms_rtc_SetSW1(dev, seconds);
#endif // #ifdef CONFIG_RTCPWC_INNER_EHHE
    spin_unlock_irqrestore(&info->mutex, flags);
    return 0;
}

static const struct rtc_class_ops ms_rtcpwc_ops = {
    .read_time = ms_rtc_read_time,
    .set_time = ms_rtc_set_time,
    /*.read_alarm = ms_rtc_read_alarm,
    .set_alarm = ms_rtc_set_alarm,*/
};
#if 0
static irqreturn_t ms_rtc_interrupt(s32 irq, void *dev_id)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev_id);
    u16 reg;

    reg = readw(info->rtc_base + REG_RTC_CTRL);
    reg |= INT_CLEAR_BIT;
    writew(reg, info->rtc_base + REG_RTC_CTRL);
    RTC_DBG("RTC INTERRUPT\n");

    return IRQ_HANDLED;
}


#ifdef CONFIG_PM
static s32 ms_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
    if(auto_wakeup_delay_seconds)
    {
        struct rtc_time tm;
        struct rtc_wkalrm alarm;
        unsigned long seconds;
        ms_rtc_read_time(&pdev->dev, &tm);
        rtc_tm_to_time(&tm, &seconds);
        RTC_DBG("[%s]: Ready to use RTC alarm, time=%ld\n", __func__, seconds);
        seconds += auto_wakeup_delay_seconds;
        rtc_time_to_tm(seconds, &alarm.time);
        alarm.enabled=1;
        ms_rtc_set_alarm(&pdev->dev, &alarm);
    }
    return 0;
}
#endif
static s32 ms_rtc_resume(struct platform_device *pdev)
{
    return 0;
}

#endif
static int ms_rtcpwc_remove(struct platform_device *pdev)
{
    return 0;
}


static int ms_rtcpwc_probe(struct platform_device *pdev)
{
    struct ms_rtc_info *info;
    struct resource *res;
    struct device* rtc_dev;
    dev_t dev;
    int ret = 0;
//    u16 reg;
//    u32 rate;
//    int rc;

    info = devm_kzalloc(&pdev->dev, sizeof(struct ms_rtc_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;
    RTC_DBG("RTC initial\n");
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
        RTC_ERR("[%s]: failed to get IORESOURCE_MEM\n", __func__);
        return -ENODEV;
    }
    info->rtc_base = devm_ioremap_resource(&pdev->dev, res);

    if (IS_ERR(info->rtc_base))
        return PTR_ERR(info->rtc_base);

    info->pdev = pdev;
/*
    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

    if (!res)
    {
		RTC_ERR("[%s]: failed to get IORESOURCE_IRQ\n", __func__);
		return -ENODEV;
    }

    rc = request_irq(res->start, ms_rtc_interrupt, IRQF_SHARED, "ms_rtc", &pdev->dev);

    if (rc)
    {
        RTC_ERR("[%s]: request_irq()is failed. return code=%d\n", __func__, rc);
    }
*/
    platform_set_drvdata(pdev, info);

    info->rtc_dev = devm_rtc_device_register(&pdev->dev,
                dev_name(&pdev->dev), &ms_rtcpwc_ops,
                THIS_MODULE);

    if (IS_ERR(info->rtc_dev)) {
        ret = PTR_ERR(info->rtc_dev);
        RTC_ERR("[%s]: unable to register device (err=%d).\n", __func__, ret);
        return ret;
    }

    //Note: is it needed?
    //device_set_wakeup_capable(&pdev->dev, 1);
    //device_wakeup_enable(&pdev->dev);

    //init rtc
    RTC_DBG("[%s]: hardware initialize\n", __func__);

    if (0 != (ret = alloc_chrdev_region(&dev, 0, 1, "ms_rtcwc")))
        return ret;

    rtc_dev = device_create(msys_get_sysfs_class(), NULL, dev, NULL, "ms_rtcwc");

    device_create_file(rtc_dev, &dev_attr_auto_wakeup_timer);
    device_create_file(rtc_dev, &dev_attr_isoctl_check);
    {
        int num = 0;
        struct rtc_time tm = { 0 };

        info->default_base = 0; // 1970/1/1 00:00:00
        if (0 < (num = of_property_count_elems_of_size(pdev->dev.of_node, DTS_DEFAULT_DATE, sizeof(int))))
        {
            if (!of_property_read_u32_array(pdev->dev.of_node, DTS_DEFAULT_DATE, (u32*)&tm, num))
            {
                rtc_tm_to_time(&tm, (unsigned long*)&info->default_base);
            }
        }
    }
    _bInit = 1;
    spin_lock_init(&info->mutex);
    return ret;
}

static const struct of_device_id ms_rtcpwc_of_match_table[] = {
    { .compatible = "sstar,infinity-rtcpwc" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_rtcpwc_of_match_table);

static struct platform_driver ms_rtcpwc_driver = {
    .remove = ms_rtcpwc_remove,
    .probe = ms_rtcpwc_probe,
#if 0
    .suspend = ms_rtcpwc_suspend,
    .resume = ms_rtcpwc_resume,
#endif
    .driver = {
        .name = "ms_rtcpwc",
        .owner = THIS_MODULE,
        .of_match_table = ms_rtcpwc_of_match_table,
    },
};

module_platform_driver(ms_rtcpwc_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MStar RTC Driver");
MODULE_LICENSE("GPL v2");
