#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdlib.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <ubi_uboot.h>
#include <cmd_osd.h>
#include <common.h>

#include <spi.h>
#include "../drivers/mstar/gpio/infinity2m/padmux.h"
#include "../drivers/mstar/sar/sar.h"


#define CS_GPIO_PIN   (89)
#define RST_GPIO_PIN   (18)
#define CLK_GPIO_PIN   (9)
#define SDO_GPIO_PIN   (10)

#define SPI_HARDWARE_ENABLE    0

#define SPI_MAX_SPEED_HZ       60*1000*1000   //10m
#define SPI_MODE    0 //as kernel spi->mode  =  0
#define SPI_BUS     0
#define SPI_CS      0
#define MAX_SPI_BYTES     128 /* Maximum number of bytes we can handle */

static unsigned char dout[MAX_SPI_BYTES];
static unsigned char din[MAX_SPI_BYTES];


//keypad define start

#define MAXKEYPADCH       4 //Maximun supported keypad channels
#define KEYPAD_STABLE_NUM                       10
#define KEYPAD_STABLE_NUM_MIN                   9

#define ADC_KEY_LEVEL_TOLERANCE        0xFF

#define KEYPAD_FORCEUGRADE_KEY          0x46 //powerkey


typedef enum
{
    EN_BOOT_MODE_UNKNOWN,
    EN_BOOT_MODE_NORMAL,
    EN_BOOT_MODE_RECOVERY,
    EN_BOOT_MODE_FORCEUGRADE,
    EN_BOOT_MODE_USB_UPGRADE,
    EN_BOOT_MODE_OAD_UPGRADE,
}EN_BOOT_MODE;

typedef struct
{
    ///upper bound
    unsigned char u8UpBnd;
    ///low bound
    unsigned char u8LoBnd;
} SAR_KpdBndCfg_t;


/// define SAR Kpd Configuration
typedef struct
{
    bool bEnable;
    unsigned char u8SARChID;
    SAR_KpdBndCfg_t tSARChBnd;
    unsigned char u8KeyLevelNum;  // 0-8 levels
    int u32KeyThreshold[8];  // each threshold match to one keycode
    int u32KeyCode[8];
} SAR_KpdRegCfg_t;


static SAR_KpdRegCfg_t m_KpdConfig[MAXKEYPADCH] = {
    //bEnable, u8SARChID, u8UpBnd, u8LoBnd, u8KeyLevelNum, u8KeyThreshold[8], u8KeyCode[8]
    {0x01, 0x00, {0xFF, 0xD0}, 0x04, {0x84, 0x183, 0x282, 0x380, 0x00, 0x00, 0x00, 0x00}, {0x46, 0x07, 0x06, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF} },
    {0x00, 0x01, {0xFF, 0xD0}, 0x04, {0x27, 0x46, 0x63, 0x7A, 0x00, 0x00, 0x00, 0x00}, {0x46, 0x04, 0x1A, 0x13, 0xFF, 0xFF, 0xFF, 0xFF} },
    {0x00, 0x02, {0xFF, 0xD0}, 0x04, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {0x00, 0x03, {0xFF, 0xD0}, 0x04, {0x10, 0x2F, 0x4D, 0x71, 0x92, 0xAB, 0xC3, 0xE7}, {0x46, 0x1F, 0x19, 0x16, 0x15, 0x0F, 0x04, 0x07} },

};



static void WaitTime(long ms)
{
    int i;
    for (i = 0; i < ms; i++)
    {
        udelay(1000);
    }
}

int Spi_Gpio_SetCmd(unsigned int value, unsigned char u8Bits)
{
    unsigned int i;

    for(i = 0; i < u8Bits; i++)
    {
        mdrv_gpio_set_low(CLK_GPIO_PIN);
        udelay(1);
        if(value & (0x01<<(u8Bits - 1)))
            mdrv_gpio_set_high(SDO_GPIO_PIN);
        else
            mdrv_gpio_set_low(SDO_GPIO_PIN);
        value <<= 1;
        udelay(1);
        mdrv_gpio_set_high(CLK_GPIO_PIN);
        udelay(1);
    }

    mdrv_gpio_set_high(SDO_GPIO_PIN);
    udelay(1);
    mdrv_gpio_set_high(CLK_GPIO_PIN);
    udelay(1);

    return 0;
}

static int do_spi_xfer(int bitlen)
{
    struct spi_slave *slave;
    int ret = 0;

    slave = spi_setup_slave(SPI_BUS, SPI_CS, SPI_MAX_SPEED_HZ, SPI_MODE);
    if (!slave)
    {
        printf("Invalid device %d:%d\n", SPI_BUS, SPI_CS);
        return -EINVAL;
    }

    ret = spi_claim_bus(slave);
    if (ret)
        goto done;
    ret = spi_xfer(slave, bitlen, dout, NULL,
        SPI_XFER_BEGIN | SPI_XFER_END);

    if (ret)
    {
        printf("Error %d during SPI transaction\n", ret);
    }

    //printf(" SPI transa: 0x%2x  bitlen=%d\n", dout[0],bitlen);
    done:
    spi_release_bus(slave);
    return ret;
}


static void init_cmd(unsigned short data)
{
#if SPI_HARDWARE_ENABLE
    //mdrv_gpio_set_low(RS_GPIO_PIN);
    mdrv_gpio_set_low(CS_GPIO_PIN);
    dout[0] = 0x0;
    dout[1] = data;
    do_spi_xfer(9);
    mdrv_gpio_set_high(CS_GPIO_PIN);

#else
    mdrv_gpio_set_low(CS_GPIO_PIN);
    Spi_Gpio_SetCmd(0x00, 1);
    Spi_Gpio_SetCmd(data, 8);
    mdrv_gpio_set_high(CS_GPIO_PIN);
#endif
}

static void init_param(unsigned short data)
{
#if SPI_HARDWARE_ENABLE
    //mdrv_gpio_set_high(RS_GPIO_PIN);
    mdrv_gpio_set_low(CS_GPIO_PIN);
    dout[0]=0x1;
    dout[1]=data;
    do_spi_xfer(9);
    mdrv_gpio_set_high(CS_GPIO_PIN);
#else
    mdrv_gpio_set_low(CS_GPIO_PIN);
    Spi_Gpio_SetCmd(0x01, 1);
    Spi_Gpio_SetCmd(data, 8);
    mdrv_gpio_set_high(CS_GPIO_PIN);
#endif
}

void spi_panel_init(void)
{
    //RST CS CLK SDO gpio init
    MDrv_GPIO_Pad_Set(CS_GPIO_PIN);
    MDrv_GPIO_Pad_Set(RST_GPIO_PIN);
    MDrv_GPIO_Pad_Set(CLK_GPIO_PIN);
    MDrv_GPIO_Pad_Set(SDO_GPIO_PIN);

    mdrv_gpio_set_high(CS_GPIO_PIN);

    mdrv_gpio_set_high(RST_GPIO_PIN);
    WaitTime(200);
    mdrv_gpio_set_low(RST_GPIO_PIN);
    WaitTime(200);
    mdrv_gpio_set_high(RST_GPIO_PIN);
    WaitTime(200);

    //*** ST7789V + CTC2.8 ***//
    //===============================================
    init_cmd(0x11);
    WaitTime(120); //Delay 120ms
    //--------------------------------Display and color format setting----------------------------//
    //WaitTime(2000);

    init_cmd(0x36);
    init_param(0x00);
    init_cmd(0x3a);
    init_param(0x06);//5

    init_cmd(0xB0);
    init_param(0x11);
    init_param(0x00);

    init_cmd(0xB1);
    init_param(0xC0);//0x40
    init_param(0x04);
    init_param(0x0a);
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    init_cmd(0xb2);
    init_param(0x0c);
    init_param(0x0c);
    init_param(0x00);
    init_param(0x33);
    init_param(0x33);
    init_cmd(0xb7);
    init_param(0x35);//22
    //---------------------------------ST7789S Power setting--------------------------------------//
    init_cmd(0xbb);//VOCM
    init_param(0x33);//35
    init_cmd(0xc0);
    init_param(0x2c);
    init_cmd(0xc2);
    init_param(0x01);
    init_cmd(0xc3);//GVDD
    init_param(0x10);//27
    init_cmd(0xc4);
    init_param(0x20);
    init_cmd(0xc6);
    init_param(0x0f);//7f
    init_cmd(0xd0);
    init_param(0xa4);
    init_param(0xa1);
    init_cmd(0xd6);//
    init_param(0xa1);//
    //--------------------------------ST7789S gamma setting---------------------------------------//
    init_cmd(0xe0);
    init_param(0x0d);//d0
    init_param(0x0f);//00
    init_param(0x11);//09
    init_param(0x07);//0f
    init_param(0x05);//10
    init_param(0x02);//1B
    init_param(0x28);//43
    init_param(0x33);//44
    init_param(0x3f);//51
    init_param(0x26);//0A
    init_param(0x14);//13
    init_param(0x15);//11
    init_param(0x24);//20
    init_param(0x28);//24

    init_cmd(0xe1);
    init_param(0x0d);//d0
    init_param(0x0e);//0F
    init_param(0x11);//18
    init_param(0x07);//10
    init_param(0x05);//11
    init_param(0x02);//1a
    init_param(0x28);//42
    init_param(0x22);//44
    init_param(0x3f);//52
    init_param(0x2a);//0B
    init_param(0x18);//17
    init_param(0x19);//19
    init_param(0x26);//21
    init_param(0x28);//24

    init_cmd(0x29);//55

}

void init_panel(void)
{
    spi_panel_init();

}

static bool msKeypad_CH_GetKey(unsigned char Channel, int *pkey)
{
    unsigned char i, j;
    int Key_Value;
    //unsigned char u8ChIdx = msKeypad_GetChanIndex(Channel);

    if(Channel > 4)
        return 1;

    unsigned char u8KeyLevelNum = m_KpdConfig[Channel].u8KeyLevelNum;
    unsigned char KEY_LV[u8KeyLevelNum];
    memset(KEY_LV, 0x0, u8KeyLevelNum);

    *pkey = 0xFFFF;

    for ( i = 0; i < KEYPAD_STABLE_NUM; i++ )
    {
        //printf("u8SARChID: %d\n",m_KpdConfig[Channel].u8SARChID);

        Key_Value = sar_get_value(m_KpdConfig[Channel].u8SARChID);

        for (j=0;j<u8KeyLevelNum;j++)
        {
            if (Key_Value < m_KpdConfig[Channel].u32KeyThreshold[j])
            {

                if((m_KpdConfig[Channel].u32KeyThreshold[j] - Key_Value) <= ADC_KEY_LEVEL_TOLERANCE)
                {
                    KEY_LV[j]++;
                    break;
                }
            }
        }

    }

    for(i=0; i<u8KeyLevelNum; i++)
    {
        if(KEY_LV[i] > KEYPAD_STABLE_NUM_MIN)
        {
            *pkey = m_KpdConfig[Channel].u32KeyCode[i];
            return 0;
        }
    }
    return 1;

}

void sar_keypad_test(void)
{
    int val;
    int channel=0;
    int pkey;

    sar_hw_init();
    while(1)
    {
        if(!msKeypad_CH_GetKey(channel,&pkey))
            printf("sar value: %d\n",pkey);

        //WaitTime(100);
    }
}
#define KEYPAD_HOLD_VALUE               2*1000  // 2 seconds

static EN_BOOT_MODE check_usb_upgrade(int s32SarChn)
{
    int s32KeyPad_KeyVal=0xFFF;

    EN_BOOT_MODE mode = EN_BOOT_MODE_UNKNOWN;
    ulong start_time = 0;

    ulong interval = 0;
    U32 tick = 0;

    start_time = get_timer(0);

    while (interval < KEYPAD_HOLD_VALUE)
    {
        msKeypad_CH_GetKey(s32SarChn,&s32KeyPad_KeyVal);
        if (s32KeyPad_KeyVal != KEYPAD_FORCEUGRADE_KEY)
        {
            break;
        }
        interval = get_timer(start_time);

        if (tick != interval/50)
        {
            tick = interval/50;
            printf("#");
        }
    }
    if (interval >= KEYPAD_HOLD_VALUE)
    {
        mode = EN_BOOT_MODE_USB_UPGRADE;
        printf("\nTime up(%d s), do USB Upgrade\n", KEYPAD_HOLD_VALUE/1000);
    }
    else
    {
        printf("\nNo enough time(%d s < %d s), continue to booting...\n", interval/1000, KEYPAD_HOLD_VALUE/1000);
    }

    return mode;
}


EN_BOOT_MODE get_boot_mode_from_keypad(void)
{
     int s32KeyPad_KeyVal=0xFFF;
     unsigned char u8KeyPad_Channel = 0;
     EN_BOOT_MODE mode = EN_BOOT_MODE_UNKNOWN;

     msKeypad_CH_GetKey(u8KeyPad_Channel,&s32KeyPad_KeyVal);
     printf("fore uup u8KeyPad_KeyVal [0x%x]\n",s32KeyPad_KeyVal);

     switch(s32KeyPad_KeyVal) // NOTE: read IR Key to decide what boot mode to go
    {
        case KEYPAD_FORCEUGRADE_KEY:
            mode = check_usb_upgrade(u8KeyPad_Channel);
            break;
        default:
            mode = EN_BOOT_MODE_UNKNOWN;
            break;
    }

    return mode;
}


int do_bootcheck (void)
{
    bool BootFlag;
    EN_BOOT_MODE BootMode;

    BootFlag = 0;
    BootMode = EN_BOOT_MODE_UNKNOWN;
    printf("bootcheck start\n");
    //check keypad usb upgrade mode
    if(BootFlag == 0)
    {
        BootMode =get_boot_mode_from_keypad();
        if(BootMode!=EN_BOOT_MODE_UNKNOWN)
        {
            //BootMode via KEYPAD
            BootFlag=1;
        }
    }
    printf("BootMode %d\n",BootMode);
    switch(BootMode) // NOTE: read IR Key to decide what boot mode to go
    {
        case EN_BOOT_MODE_USB_UPGRADE:
#if defined(CONFIG_MS_USB)
            run_command("usbstar", 0);
#endif
            break;
        default:
            break;
    }

    return 0;
}

int do_customer_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    //spi panel init sample
    //init_panel();
#ifdef CONFIG_CMD_KEYPAD_FORCE_UPGRADE
    //sar init
    sar_hw_init();
    //sar test
    do_bootcheck();
#endif


    return 0;
}

U_BOOT_CMD(
        customer_init, CONFIG_SYS_MAXARGS, 1,    do_customer_init,
        "customer init \n",
        NULL
        );


