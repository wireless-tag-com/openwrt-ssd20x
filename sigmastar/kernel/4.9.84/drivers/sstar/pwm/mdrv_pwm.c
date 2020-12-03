/*
* mdrv_pwm.c- Sigmastar
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
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#include "ms_msys.h"
#include "mhal_pwm.h"

#if defined(CONFIG_MS_PADMUX)
#include "mdrv_padmux.h"
#include "mdrv_puse.h"
#include "gpio.h"
#endif

static ssize_t group_mode_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_period_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_begin_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_end_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_round_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_enable_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_hold_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_stop_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_polarity_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_info_out(struct device *dev, struct device_attribute *attr, char *buf);
//+++[Only4I6e]
static ssize_t group_hold_mode1_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_duty_qe0_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count);
static ssize_t group_round_out(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t group_output_out(struct device *dev, struct device_attribute *attr, char *buf);
extern irqreturn_t PWM_IRQ(int irq, void *dummy);
extern void MDEV_PWM_AllGrpEnable(struct mstar_pwm_chip *ms_chip);
//---[Only4I6e]


DEVICE_ATTR(group_mode, 0200, NULL, group_mode_in);
DEVICE_ATTR(group_period, 0200, NULL, group_period_in);
DEVICE_ATTR(group_begin, 0200, NULL, group_begin_in);
DEVICE_ATTR(group_end, 0200, NULL, group_end_in);
DEVICE_ATTR(group_round, 0200, NULL, group_round_in);
DEVICE_ATTR(group_enable, 0200, NULL, group_enable_in);
DEVICE_ATTR(group_hold, 0200, NULL, group_hold_in);
DEVICE_ATTR(group_stop, 0200, NULL, group_stop_in);
DEVICE_ATTR(group_polarity, 0200, NULL, group_polarity_in);
DEVICE_ATTR(group_info, 0444, group_info_out, NULL);
//+++[Only4I6e]
DEVICE_ATTR(group_hold_mode1, 0200, NULL, group_hold_mode1_in);
DEVICE_ATTR(group_duty_qe0, 0200, NULL, group_duty_qe0_in);
DEVICE_ATTR(group_round_get, 0200, NULL, group_round_out);
DEVICE_ATTR(group_output, 0444, group_output_out, NULL);
//---[Only4I6e]

static inline struct mstar_pwm_chip *to_mstar_pwm_chip(struct pwm_chip *c)
{
    return container_of(c, struct mstar_pwm_chip, chip);
}

static int mstar_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm, int duty_ns, int period_ns)
{
    struct mstar_pwm_chip *ms_pwm = to_mstar_pwm_chip(chip);

    MS_PWM_DBG("[PWM] %s duty_ns=%d, period_ns=%d\n", __func__, duty_ns, period_ns);

    MDEV_PWM_AllGrpEnable(ms_pwm);
    DrvPWMSetPeriod(ms_pwm, pwm->hwpwm, period_ns);
    DrvPWMSetDuty(ms_pwm, pwm->hwpwm, duty_ns);
    //DrvPWMPadSet(pwm->hwpwm, (U8)ms_pwm->pad_ctrl[pwm->hwpwm]);

    return 0;
}

#if defined(CONFIG_MS_PADMUX)
static int _pwm_is_pad_set(U8 u8PwmId)
{
    // important: need to modify if more MDRV_PUSE_PWM? defined
    if ((u8PwmId == 0 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM0)) ||
        (u8PwmId == 1 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM1)) ||
        (u8PwmId == 2 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM2)) ||
        (u8PwmId == 3 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM3)) ||
        (u8PwmId == 4 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM4)) ||
        (u8PwmId == 5 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM5)) ||
        (u8PwmId == 6 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM6)) ||
        (u8PwmId == 7 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM7)) ||
        (u8PwmId == 8 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM8)) ||
        (u8PwmId == 9 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_PWM9)))
    {
        return TRUE;
    }
    else
        return FALSE;
}
#endif

static int mstar_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct mstar_pwm_chip *ms_pwm = to_mstar_pwm_chip(chip);
    MS_PWM_DBG("[PWM] %s\n", __func__);
    DrvPWMEnable(ms_pwm, pwm->hwpwm, 1);

#if defined(CONFIG_MS_PADMUX)
    if ( 0 == mdrv_padmux_active() ||
         FALSE == _pwm_is_pad_set(pwm->hwpwm) )
#endif
    {
        DrvPWMPadSet(pwm->hwpwm, (U8)ms_pwm->pad_ctrl[pwm->hwpwm]);
    }
    return 0;
}

static void mstar_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct mstar_pwm_chip *ms_pwm = to_mstar_pwm_chip(chip);
    MS_PWM_DBG("[PWM] %s\n", __func__);
    DrvPWMEnable(ms_pwm, pwm->hwpwm, 0);
}

static int mstar_pwm_set_polarity(struct pwm_chip *chip, struct pwm_device *pwm, enum pwm_polarity polarity)
{
    struct mstar_pwm_chip *ms_pwm = to_mstar_pwm_chip(chip);
    MS_PWM_DBG("[PWM] %s %d\n", __func__, (U8)polarity);
    DrvPWMSetPolarity(ms_pwm, pwm->hwpwm, (U8)polarity);
    return 0;
}

static void mstar_pwm_get_state(struct pwm_chip *chip, struct pwm_device *pwm, struct pwm_state *state)
{
    struct mstar_pwm_chip *ms_pwm = to_mstar_pwm_chip(chip);
    U32 u32Period = 0x00000000, u32Duty = 0x00000000;
    U8 enable = 0x00, polarity = 0x00;

    DrvPWMGetPeriod(ms_pwm, pwm->hwpwm, &u32Period);
    DrvPWMGetDuty(ms_pwm, pwm->hwpwm, &u32Duty);
    DrvPWMEnableGet(ms_pwm, pwm->hwpwm, &enable);
    DrvPWMGetPolarity(ms_pwm, pwm->hwpwm, &polarity);
    state->period = u32Period;
    state->duty_cycle = u32Duty;
    state->polarity = polarity;
    state->enabled = enable;
}

static const struct pwm_ops mstar_pwm_ops = {
    .config = mstar_pwm_config,
    .enable = mstar_pwm_enable,
    .disable = mstar_pwm_disable,
    .set_polarity = mstar_pwm_set_polarity,
    .get_state = mstar_pwm_get_state,
    .owner = THIS_MODULE,
};

static int ms_pwm_probe(struct platform_device *pdev)
{
    struct mstar_pwm_chip *ms_pwm;
    struct resource *res;
    U32 U32Pm_Addr=0;
    int ret=0;
    int i;

    ms_pwm = devm_kzalloc(&pdev->dev, sizeof(*ms_pwm), GFP_KERNEL);
    if (ms_pwm == NULL)
    {
        dev_err(&pdev->dev, "failed to allocate memory\n");
        return -ENOMEM;
    }
    if (DrvPWMGroupCap())
    {
        struct device *mstar_class_pwm_device = NULL;
        if (!(mstar_class_pwm_device = device_create(msys_get_sysfs_class(), NULL, MKDEV(0, 0), NULL, "motor")))
        {
            // printk("[%s][%d] create device file fail\n", __FUNCTION__, __LINE__);
            return -ENOMEM;
        }
        // printk("[%s][%d] 0x%08x\n", __FUNCTION__, __LINE__, (int)ms_pwm);
        ms_pwm->group_data = (void*)mstar_class_pwm_device;
        dev_set_drvdata(mstar_class_pwm_device, (void*)ms_pwm);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_mode);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_period);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_begin);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_end);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_round);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_enable);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_hold);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_stop);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_polarity);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_info);
        //+++[Only4I6e]
        device_create_file(mstar_class_pwm_device, &dev_attr_group_hold_mode1);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_duty_qe0);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_round_get);
        device_create_file(mstar_class_pwm_device, &dev_attr_group_output);
        //---[Only4I6e]
    }
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&pdev->dev, "Can't get I/O resource regs for pwm\n");
        return 0;
    }

    //ms_pwm->base = devm_ioremap_resource(&pdev->dev, res);
    ms_pwm->base = (void *)res->start;

    if (IS_ERR(ms_pwm->base))
        return PTR_ERR(ms_pwm->base);

    ms_pwm->clk = devm_clk_get(&pdev->dev, NULL);
    if (IS_ERR(ms_pwm->clk))
        return PTR_ERR(ms_pwm->clk);

    ret = clk_prepare_enable(ms_pwm->clk);
    if (ret)
        return ret;

    platform_set_drvdata(pdev, ms_pwm);

    ms_pwm->chip.dev = &pdev->dev;
    ms_pwm->chip.ops = &mstar_pwm_ops;
    ms_pwm->chip.base = -1;
    if(of_property_read_u32(pdev->dev.of_node, "npwm", &ms_pwm->chip.npwm))
    	ms_pwm->chip.npwm = 4;
    ms_pwm->pad_ctrl = devm_kzalloc(&pdev->dev, ms_pwm->chip.npwm * sizeof(*ms_pwm->pad_ctrl), GFP_KERNEL);
    if (ms_pwm->pad_ctrl == NULL)
    {
        dev_err(&pdev->dev, "failed to allocate memory\n");
        return -ENOMEM;
    }
    if(of_property_read_u32(pdev->dev.of_node, "pm_group_base", &U32Pm_Addr))
    {
        ms_pwm->group_data = NULL;
    }
    else
    {
        if(U32Pm_Addr != FALSE)
            ms_pwm->group_data = (void *)U32Pm_Addr;
        printk("[i6e][pwm] use ms_pwm->group_data\n");
    }

    if((ret=of_property_read_u32_array(pdev->dev.of_node, "pad-ctrl", ms_pwm->pad_ctrl, ms_pwm->chip.npwm)))
        dev_err(&pdev->dev, "read pad-ctrl failed\n");

    // improve boot-up speed, remove print log
#if 1
    for(i=0; i<ms_pwm->chip.npwm; i++)
    {
        DrvPWMInit(ms_pwm, i);
        // MS_PWM_DBG("ms_pwm->pad_ctrl[%d]=%d\n", i, ms_pwm->pad_ctrl[i]);
    }
#endif

    ret = pwmchip_add(&ms_pwm->chip);
    if (ret < 0)
    {
        clk_disable_unprepare(ms_pwm->clk);
        devm_clk_put(&pdev->dev, ms_pwm->clk);
        dev_err(&pdev->dev, "pwmchip_add failed\n");
        return ret;
    }

    for (i = 0; i < PWM_GROUP_NUM; i++)
    {
        DrvPWMGroupEnable(ms_pwm, i, 0);
    }

    //+++[Only4I6e and I6b0]
    /* Register interrupt handler */
    ms_pwm->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (ms_pwm->irq) {
        ret = request_irq(ms_pwm->irq, PWM_IRQ, 0, "pwm-isr", ms_pwm);
        if (ret) {
            dev_err(&pdev->dev, "pwm request irq fail(x%x)\n",ret);
            return ret;
        }
        printk(KERN_NOTICE "[NOTICE]pwm-isr(%d) success. If not i6e or i6b0, pls confirm it on .dtsi\n", ms_pwm->irq);
    }
    //---[Only4I6e]

    // improve boot-up speed, remove print log
    //dev_info(&pdev->dev, "probe successful\n");

    return 0;
}

static int ms_pwm_remove(struct platform_device *pdev)
{
    struct mstar_pwm_chip *ms_pwm = dev_get_drvdata(&pdev->dev);
    int err;

    clk_disable_unprepare(ms_pwm->clk);
    if (!IS_ERR(ms_pwm->clk))
        devm_clk_put(&pdev->dev, ms_pwm->clk);
    err = pwmchip_remove(&ms_pwm->chip);
    if (err < 0)
        return err;
/*
    device_remove_file(hemac->mstar_class_emac_device, &dev_attr_sw_led_flick_speed);
    if (ms_pwm->group_data)
	    device_destroy(msys_get_sysfs_class(), MKDEV(0, 0));
*/
    free_irq(ms_pwm->irq, ms_pwm);
    dev_info(&pdev->dev, "remove successful\n");
    return 0;
}

static ssize_t group_mode_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int pwmId = 0;
    int enable = 0;

    nArg = sscanf(buf, "%d %d", &pwmId, &enable);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (pwm_id, enable)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    enable = (enable) ? 1 : 0;
    DrvPWMGroupJoin(ms_pwm, pwmId, enable);
    DrvPWMDiv(ms_pwm, pwmId, enable);
    DrvPWMSetDben(ms_pwm, pwmId, 1);
    DrvPWMEnable(ms_pwm, pwmId, 0);
#if defined(CONFIG_MS_PADMUX)
    if ( 0 == mdrv_padmux_active() ||
         FALSE == _pwm_is_pad_set(pwmId) )
#endif
    {
        DrvPWMPadSet(pwmId, (U8)ms_pwm->pad_ctrl[pwmId]);
    }
out:
    return count;
}

static ssize_t group_period_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int pwmId = 0;
    int period = 0;
    // printk("[%s][%d] 0x%08x\n", __FUNCTION__, __LINE__, (int)ms_pwm);

    nArg = sscanf(buf, "%d %d", &pwmId, &period);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (pwm_id, period)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] (pwm_id, period) = (%d, %d)\n", __FUNCTION__, __LINE__, pwmId, period);
    DrvPWMSetPeriodEx(ms_pwm, pwmId, period);
out:
    return count;
}

static ssize_t group_polarity_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int pwmId = 0;
    int polarity = 0;

    nArg = sscanf(buf, "%d %d", &pwmId, &polarity);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (pwm_id, polarity)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    polarity = (polarity) ? 1 : 0;
    // printk("[%s][%d] (pwm_id, polarity) = (%d, %d)\n", __FUNCTION__, __LINE__, pwmId, polarity);
    DrvPWMSetPolarityEx(ms_pwm, pwmId, polarity);
out:
    return count;
}

static ssize_t group_end_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int pwmId = 0;
    int end[5] = {0};
    int i;

    nArg = sscanf(buf, "%d %d %d %d %d %d", &pwmId, &end[0], &end[1], &end[2], &end[3], &end[4]);
    // if ((2 > nArg) || (5 < nArg))
    if ((2 > nArg) || (2 < nArg)) // limit to on duty/shift
    {
        printk("[%s][%d] invalid argument (pwm_id, end[0.. 3])\n", __FUNCTION__, __LINE__);
        goto out;
    }
    for (i = 2; i <= nArg; i++)
        DrvPWMSetEnd(ms_pwm, pwmId, i - 2, end[i - 2]);
out:
    return count;
}

static ssize_t group_begin_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int pwmId = 0;
    int begin[5] = {0};
    int i;

    nArg = sscanf(buf, "%d %d %d %d %d %d", &pwmId, &begin[0], &begin[1], &begin[2], &begin[3], &begin[4]);
    // if ((2 > nArg) || (5 < nArg))
    if ((2 > nArg) || (2 < nArg)) // limit to on duty/shift
    {
        printk("[%s][%d] invalid argument (pwm_id, begin[0.. 3])\n", __FUNCTION__, __LINE__);
        goto out;
    }
    for (i = 2; i <= nArg; i++)
        DrvPWMSetBegin(ms_pwm, pwmId, i - 2, begin[i - 2]);
out:
    return count;
}

static ssize_t group_round_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int groupId = 0;
    int round = 0;

    nArg = sscanf(buf, "%d %d", &groupId, &round);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (pwm_id, round)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] (pwm_id, round) = (%d, %d)\n", __FUNCTION__, __LINE__, groupId, round);
    DrvPWMGroupSetRound(ms_pwm, groupId, round);
out:
    return count;
}

static ssize_t group_enable_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0x00000000;
    int groupId = 0x00000000;
    int enable = 0x00000000;
    U8 enable_status = 0x00;

    nArg = sscanf(buf, "%d %d", &groupId, &enable);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (group_id, enable)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    if (!DrvPWMGroupIsEnable(ms_pwm, groupId, &enable_status))
    {
        printk("[%s][%d] unable to get enable status of group %d\n", __FUNCTION__, __LINE__, groupId);
        goto out;
    }
    enable = (enable) ? 1 : 0;
    if (enable == enable_status)
    {
        printk("[%s][%d] cannot enable/disable group %d again. enable status = %d\n", __FUNCTION__, __LINE__, groupId, enable_status);
        goto out;
    }

    if (enable)
    {
        DrvPWMGroupStop(ms_pwm, groupId, 0);
        DrvPWMGroupHold(ms_pwm, groupId, 0);
        DrvPWMGroupEnable(ms_pwm, groupId, 1);
    }
    else
    {
        DrvPWMGroupEnable(ms_pwm, groupId, 0);
        DrvPWMGroupStop(ms_pwm, groupId, 0);
        DrvPWMGroupHold(ms_pwm, groupId, 0);
    }
out:
    return count;
}

static ssize_t group_hold_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int groupId = 0;
    int enable = 0;

    nArg = sscanf(buf, "%d %d", &groupId, &enable);
    if (2 != nArg) {
        printk("[%s][%d] invalid argument (group_id, enable)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] (groupId, enable) = (%d, %d)\n", __FUNCTION__, __LINE__, groupId, enable);
    DrvPWMGroupHold(ms_pwm, groupId, enable);
out:
    return count;
}

static ssize_t group_stop_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0;
    int groupId = 0;
    int enable = 0;

    nArg = sscanf(buf, "%d %d", &groupId, &enable);
    // if (2 != nArg)
    if (1 != nArg)
    {
        printk("[%s][%d] invalid argument (groud_id)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] (pwm_id, enable) = (%d, %d)\n", __FUNCTION__, __LINE__, groupId, enable);
    // DrvPWMGroupStop(ms_pwm, groupId, enable);
    DrvPWMGroupStop(ms_pwm, groupId, 1);
out:
    return count;
}

static ssize_t group_info_out(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    return DrvPWMGroupInfo(ms_pwm, buf, buf + PAGE_SIZE);
}

//+++[Only4I6e]
static ssize_t group_hold_mode1_in(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0x00000000;
    int enable = 0x00000000;

    nArg = sscanf(buf, "%d", &enable);
    if (1 != nArg) {
        printk("[%s][%d] invalid argument (enable)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] enable = %d\n", __FUNCTION__, __LINE__, enable);
    DrvPWMGroupHoldM1(ms_pwm, enable);
out:
    return count;
}

static ssize_t group_duty_qe0_in(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0x00000000;
    int groupId = 0x00000000;
    int enable = 0x00000000;

    nArg = sscanf(buf, "%d %d", &groupId, &enable);
    if (2 != nArg) {
        printk("[%s][%d] invalid argument (group_id, enable)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    // printk("[%s][%d] (groupId, enable) = (%d, %d)\n", __FUNCTION__, __LINE__, groupId, enable);
    DrvPWMDutyQE0(ms_pwm, groupId, enable);
out:
    return count;
}

static ssize_t group_round_out(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    int nArg = 0x00000000;
    int groupId = 0x00000000;
    short round_set = 0x0000;

    //i6b0
    nArg = sscanf(buf, "%d %hd", &groupId, &round_set);
    if (2 != nArg)
    {
        printk("[%s][%d] invalid argument (group_id, round)\n", __FUNCTION__, __LINE__);
        goto out;
    }
    
    DrvPWMGroupGetRoundNum(ms_pwm, groupId, &round_set);

out:
    return count;
    
    //i6e
    /*if (PWM_GROUP_NUM == 0) {
        printk("[%s][%d] WARNING:PWM_GROUP_NUM = 0!\n", __FUNCTION__, __LINE__);
    }
    else {
        for (groupId = 0x00; groupId < PWM_GROUP_NUM; groupId++) {
            DrvPWMGroupGetRoundNum(ms_pwm, groupId, &round_set);
            printk("[%s][%d] grpId:%d round:%d-%s\n", __FUNCTION__, __LINE__, groupId, round_set, (round_set)?"END":"BUSY");
        }
    }
    return 1;*/
}

static ssize_t group_output_out(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct mstar_pwm_chip* ms_pwm = (struct mstar_pwm_chip*)dev_get_drvdata(dev);
    char u8Output = 0x00;

    //printk("[%s][%d] output = x%x\n", __FUNCTION__, __LINE__, u8Output);
    DrvPWMGetOutput(ms_pwm, &u8Output);
    return 1;
}
//---[Only4I6e]

static const struct of_device_id ms_pwm_of_match_table[] = {
    { .compatible = "sstar,infinity-pwm" },
    {}
};

MODULE_DEVICE_TABLE(of, ms_pwm_of_match_table);

static struct platform_driver ms_pwm_driver = {
    .remove = ms_pwm_remove,
    .probe = ms_pwm_probe,
    .driver = {
        .name = "sstar-pwm",
        .owner = THIS_MODULE,
        .of_match_table = ms_pwm_of_match_table,
    },
};

module_platform_driver(ms_pwm_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MStar PWM Driver");
MODULE_LICENSE("GPL v2");
