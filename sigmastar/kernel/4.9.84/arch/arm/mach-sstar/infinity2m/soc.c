/*
* soc.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timecounter.h>
#include <clocksource/arm_arch_timer.h>

#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "_ms_private.h"

/* IO tables */
static struct map_desc mstar_io_desc[] __initdata =
{
    /* Define Registers' physcial and virtual addresses */
        {IO_VIRT,   __phys_to_pfn(IO_PHYS),     IO_SIZE,        MT_DEVICE},
        {SPI_VIRT,  __phys_to_pfn(SPI_PHYS),    SPI_SIZE,       MT_DEVICE},
        {GIC_VIRT,  __phys_to_pfn(GIC_PHYS),    GIC_SIZE,       MT_DEVICE},
#ifdef CONFIG_SS_AMP
        {IMI_VIRT,  __phys_to_pfn(IMI_PHYS),    IMI_SIZE,       MT_DEVICE},
        {IPC_MEM_VIRT, __phys_to_pfn(IPC_MEM_PHYS), IPC_MEM_SIZE, MT_DEVICE},
#endif
};


static const char *mstar_dt_compat[] __initconst = {
    "sstar,infinity2m",
    NULL,
};

static void __init mstar_map_io(void)
{
    iotable_init(mstar_io_desc, ARRAY_SIZE(mstar_io_desc));
}

extern struct ms_chip* ms_chip_get(void);
extern void __init ms_chip_init_default(void);
//extern void __init mstar_init_irqchip(void);


//extern struct timecounter *arch_timer_get_timecounter(void);


static int mcm_rw(int index, int ratio, int write);


/*************************************
*        Sstar chip flush function
*************************************/
static DEFINE_SPINLOCK(mstar_l2prefetch_lock);

static void mstar_uart_disable_line(int line)
{
    if(line == 0)  //for debug, do not change
    {
        // UART0_Mode -> X
        //CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4 | BIT5);
        //CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_09, BIT11);
    }
    else if(line == 1)
    {
        // UART1_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8 | BIT9);
    }
    else if(line == 2)
    {
        // FUART_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0 | BIT1);
    }
}

static void mstar_uart_enable_line(int line)
{
    if(line == 0)  //for debug, do not change
    {
        // UART0_Mode -> PAD_UART0_TX/RX
        //SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4);
    }
    else if(line == 1)
    {
        // UART1_Mode -> PAD_UART1_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8);
    }
    else if(line==2)
    {
        // FUART_Mode -> PAD_FUART_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0);
    }
}

static int mstar_get_device_id(void)
{
    return (int)(INREG16(BASE_REG_PMTOP_PA) & 0x00FF);;
}

static int mstar_get_revision(void)
{
    u16 tmp = 0;
    tmp = INREG16((unsigned int)(BASE_REG_PMTOP_PA + REG_ID_67));
    tmp=((tmp >> 8) & 0x00FF);

    return (tmp+1);
}


static void mstar_chip_flush_miu_pipe(void)
{
    unsigned long   dwLockFlag = 0;
    unsigned short dwReadData = 0;

    spin_lock_irqsave(&mstar_l2prefetch_lock, dwLockFlag);
    //toggle the flush miu pipe fire bit
    *(volatile unsigned short *)(0xFD204414) = 0x10;
    *(volatile unsigned short *)(0xFD204414) = 0x11;

    do
    {
        dwReadData = *(volatile unsigned short *)(0xFD204440);
        dwReadData &= BIT12;  //Check Status of Flush Pipe Finish

    } while(dwReadData == 0);

    spin_unlock_irqrestore(&mstar_l2prefetch_lock, dwLockFlag);

}

static u64 mstar_phys_to_MIU(u64 x)
{

    return ((x) - MIU0_BASE);
}

static u64 mstar_MIU_to_phys(u64 x)
{

    return ((x) + MIU0_BASE);
}


struct soc_device_attribute mstar_soc_dev_attr;

extern const struct of_device_id of_default_bus_match_table[];

static int mstar_get_storage_type(void)
{
/*//check DIDKEY bank, offset 0x70
#define STORAGE_SPI_NAND            BIT2
#define STORAGE_SPI_NAND_SKIPSD     BIT3
#define STORAGE_SPI_NOR_SKIPSD      BIT4
#define STORAGE_SPI_NOR             BIT5
*/
    u8 type = (INREG16(BASE_REG_DIDKEY_PA + 0x70*4) & 0x3C);

    if(BIT3 == type || BIT2 == type)
        return (int)MS_STORAGE_SPINAND_ECC;
    else if(BIT5 == type || BIT4 == type)
        return (int)MS_STORAGE_NOR;
    else
        return (int)MS_STORAGE_UNKNOWN;
}

static int mstar_get_package_type(void)
{
    //printk("BOARD name [%s] \n", &mstar_soc_dev_attr.machine[11]);

    if(!strcmp(&mstar_soc_dev_attr.machine[11], "SSC010A-S01A-S"))
        return MS_I2M_PACKAGE_QFN_DDR3_128MB;
    else if(!strcmp(&mstar_soc_dev_attr.machine[11], "FPGA"))
        return MS_I2M_PACKAGE_FPGA_128MB;
    else
    {
        printk(KERN_ERR "!!!!! Machine name [%s] \n", mstar_soc_dev_attr.machine);
        return MS_I2M_PACKAGE_UNKNOWN;
    }
}
static char mstar_platform_name[]="I2M";

char* mstar_get_platform_name(void)
{
    return mstar_platform_name;
}

static unsigned long long mstar_chip_get_riu_phys(void)
{
    return IO_PHYS;
}

static int mstar_chip_get_riu_size(void)
{
    return IO_SIZE;
}


static int mstar_ir_enable(int param)
{
    printk(KERN_ERR "NOT YET IMPLEMENTED!![%s]",__FUNCTION__);
    return 0;
}


static int mstar_usb_vbus_control(int param)
{

    int ret;
    //int package = mstar_get_package_type();
    int power_en_gpio=-1;

    struct device_node *np;
    int pin_data;
    int port_num = param >> 16;
    int vbus_enable = param & 0xFF;
    if((vbus_enable<0 || vbus_enable>1) && (port_num<0 || port_num>1))
    {
        printk(KERN_ERR "[%s] param invalid:%d %d\n", __FUNCTION__, port_num, vbus_enable);
        return -EINVAL;
    }

    if (power_en_gpio<0)
    {
        if(0 == port_num)
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-1");
        }
        else if (1 == port_num)
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-2");
        }
        else
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-3");
        }

        if(!of_property_read_u32(np, "power-enable-pad", &pin_data))
        {
            printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
            power_en_gpio = (unsigned char)pin_data;

            ret = gpio_request(power_en_gpio, "USB0-power-enable");
            if (ret < 0) {
                printk(KERN_INFO "Failed to request USB0-power-enable GPIO(%d)\n", power_en_gpio);
                power_en_gpio =-1;
                return ret;
            }
        }
    }

    if (power_en_gpio >= 0)
    {
        if(0 == vbus_enable) //disable vbus
        {
            gpio_direction_output(power_en_gpio, 0);
            printk(KERN_INFO "[%s] Disable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
        }
        else if(1 == vbus_enable)
        {
            gpio_direction_output(power_en_gpio, 1);
            printk(KERN_INFO "[%s] Enable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
        }
    }
    return 0;
}
static cycle_t us_ticks_cycle_offset=0;
static u64 us_ticks_factor=1;


static u64 mstar_chip_get_us_ticks(void)
{
	u64 cycles=arch_timer_read_counter();
	u64 usticks=div64_u64(cycles,us_ticks_factor);
	return usticks;
}

void mstar_reset_us_ticks_cycle_offset(void)
{
	us_ticks_cycle_offset=arch_timer_read_counter();
}

static int mstar_chip_function_set(int function_id, int param)
{
    int res=-1;

    printk("CHIP_FUNCTION SET. ID=%d, param=%d\n",function_id,param);
    switch (function_id)
    {
            case CHIP_FUNC_UART_ENABLE_LINE:
                mstar_uart_enable_line(param);
                break;
            case CHIP_FUNC_UART_DISABLE_LINE:
                mstar_uart_disable_line(param);
                break;
            case CHIP_FUNC_IR_ENABLE:
                mstar_ir_enable(param);
                break;
            case CHIP_FUNC_USB_VBUS_CONTROL:
                mstar_usb_vbus_control(param);
                break;
            case CHIP_FUNC_MCM_DISABLE_ID:
                mcm_rw(param, 0, 1);
                break;
            case CHIP_FUNC_MCM_ENABLE_ID:
                mcm_rw(param, 15, 1);
                break;
        default:
            printk(KERN_ERR "Unsupport CHIP_FUNCTION!! ID=%d\n",function_id);

    }

    return res;
}


static void __init mstar_init_early(void)
{


    struct ms_chip *chip=NULL;
    ms_chip_init_default();

    chip=ms_chip_get();

    //enable axi exclusive access
    *(volatile unsigned short *)(0xFD204414) = 0x10;

    chip->chip_flush_miu_pipe=mstar_chip_flush_miu_pipe;
    chip->phys_to_miu=mstar_phys_to_MIU;
    chip->miu_to_phys=mstar_MIU_to_phys;
    chip->chip_get_device_id=mstar_get_device_id;
    chip->chip_get_revision=mstar_get_revision;
    chip->chip_get_platform_name=mstar_get_platform_name;
    chip->chip_get_riu_phys=mstar_chip_get_riu_phys;
    chip->chip_get_riu_size=mstar_chip_get_riu_size;

    chip->chip_function_set=mstar_chip_function_set;
    chip->chip_get_storage_type=mstar_get_storage_type;
    chip->chip_get_package_type=mstar_get_package_type;
    chip->chip_get_us_ticks=mstar_chip_get_us_ticks;

}

static void mstar_analog_ip_powerdown(void)
{
#ifdef CONFIG_ANALOG_PD_XINDIGPLL
    // reg_miu_128bus_pll_pd BANK 1031, 16bit OFFSET 0x01[8]
    SETREG16(BASE_REG_MIUPLL_PA + REG_ID_01, BIT8);
#endif
#ifdef CONFIG_ANALOG_PD_ARMPLL
    // reg_mipspll_pd BANK 1032, 16bit OFFSET 0x11[8]
    SETREG16(BASE_REG_ARMPLL_PA + REG_ID_11, BIT8);
#endif
#ifdef CONFIG_ANALOG_PD_AUDIO
    // BANK 1034, 16bit OFFSET 0x00[1:0]: reg_en_byp_inmux
    //                         0xffffffffffffffff[2]: reg_en_chop_adc0
    //                         0x00[7:4]: reg_en_ck_dac
    //                         0x00[8]: reg_en_dac_disch
    //                         0x00[9]: reg_en_itest_dac
    //                         0x00[10]: reg_en_itest_dac
    //                         0x00[11]: reg_en_msp
    OUTREG16(BASE_REG_AUSDM_PA + REG_ID_00, 0x0000);
    // BANK 1034, 16bit OFFSET 0x01[1:0]: reg_en_mute_inmux
    //                         0x01[2]: reg_en_mute_mic_stg1_l
    //                         0x01[3]: reg_en_mute_mic_stg1_r
    //                         0x01[4]: reg_en_qs_ldo_adc
    //                         0x01[5]: reg_en_qs_ldo_dac
    //                         0x01[6]: reg_en_shrt_l_adc0
    //                         0x01[7]: reg_en_shrt_r_adc0
    //                         0x01[9:8]: reg_en_tst_ibias_adc
    //                         0x01[10]: reg_en_vref_disch
    //                         0x01[12:10]: reg_en_vref_sftdch
    OUTREG16(BASE_REG_AUSDM_PA + REG_ID_01, 0x0000);
    // BANK 1034, 16bit OFFSET 0x03[0]: reg_pd_adc0
    //                         0x03[1]: reg_pd_bias_dac
    //                         0x03[3:2]: reg_pd_inmux
    //                         0x03[4]: reg_pd_l0_dac
    //                         0x03[5]: reg_pd_ldo_adc
    //                         0x03[6]: reg_pd_ldo_dac
    //                         0x03[7]: reg_pd_mic_stg1_l
    //                         0x03[8]: reg_pd_mic_stg1_r
    //                         0x03[9]: reg_pd_r0_dac
    //                         0x03[10]: reg_pd_ref_dac
    //                         0x03[11]: reg_pd_vi
    //                         0x03[12]: reg_pd_vref
    OUTREG16(BASE_REG_AUSDM_PA + REG_ID_03, 0x1FFF);
#endif
#ifdef CONFIG_ANALOG_PD_EMAC
    // BANK 33, 16bit OFFSET 0x79[7]
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_79, BIT7);
    // BANK 33, 16bit OFFSET 0x79[11:8]
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_79, BIT8 | BIT9 | BIT10 | BIT11);
    // BANK 32, 16bit OFFSET 0x1F[14]: reg_analog_testen
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_1F, BIT14);
    // BANK 32, 16bit OFFSET 0x5D[14]: reg_gcr_test_clk_en/PD_REF
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_5D, BIT14);
    // BANK 32, 16bit OFFSET 0x66[3]: reg_gc_adcpl_ccpd1
    //SETREG16(BASE_REG_LANTOP1_PA + REG_ID_66, BIT3);
    // BANK 32, 16bit OFFSET 0x66[4]: reg_pd_adcpl_reg
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_66, BIT4);
    // BANK 32, 16bit OFFSET 0x5B[12]: reg_adc_pd
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_5B, BIT12);
    // BANK 32, 16bit OFFSET 0x7E[1]: PD_REG25
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_7E, BIT1);
    // BANK 32, 16bit OFFSET 0x7E[8]: PD_LDO11
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_7E, BIT11);
    // BANK 32, 16bit OFFSET 0x69[14]: reg_atop_rx_inoff, 0x69[15]: reg_atop_tx_outoff
    SETREG16(BASE_REG_LANTOP1_PA + REG_ID_69, BIT14 | BIT15);
    // BANK 33, 16bit OFFSET 0x44[4]: RX_OFF
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_44, BIT4);
    // BANK 33, 16bit OFFSET 0x50[12]: reg_pd_vbuf
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_50, BIT12);
    // BANK 33, 16bit OFFSET 0x50[13]: reg_pd_sadc
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_50, BIT13);
    // BANK 33, 16bit OFFSET 0x1D[0]: reg_pd_tx_ld
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_1D, BIT0);
    // BANK 33, 16bit OFFSET 0x1D[1]: reg_pd_tx_idac
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_1D, BIT1);
    // BANK 33, 16bit OFFSET 0x78[10]: reg_pd_tx_vbgr
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_78, BIT10);
    // BANK 33, 16bit OFFSET 0x78[11]: reg_pd_tx_trimming_dac
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_78, BIT11);
    // BANK 33, 16bit OFFSET 0x78[12]: reg_pd_tx_ld_dio
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_78, BIT12);
    // BANK 33, 16bit OFFSET 0x78[13]: reg_pd_tx_ldo
    SETREG16(BASE_REG_LANTOP2_PA + REG_ID_78, BIT13);
#endif
#ifdef CONFIG_ANALOG_PD_HDMI_ATOP
    // BANK 1126, 16bit OFFSET 0x16[3:0]: reg_din_en_pstdrv_tap0_ch
    //                         0x16[7:4]: reg_din_en_pstdrv_tap1_ch
    //                         0x16[11:8]: reg_din_en_pstdrv_tap2_ch
    OUTREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_16, 0x0);
    // BANK 1126, 16bit OFFSET 0x1B[1]: reg_gcr_en_hdmitxpll_xtal
    CLRREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_1B, BIT0);
    // BANK 1126, 16bit OFFSET 0x30[3:0]: reg_pd_pstdrv_tap0_ch
    //                         0x30[7:4]: reg_pd_pstdrv_tap1_ch
    //                         0x30[11:8]: reg_pd_pstdrv_tap2_ch
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_30, BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 |
                                                BIT8 | BIT9 | BIT10 | BIT11);
    // BANK 1126, 16bit OFFSET 0x31[3:0]: reg_pd_predrv_tap0_ch
    //                         0x31[7:4]: reg_pd_predrv_tap1_ch
    //                         0x31[11:8]: reg_pd_predrv_tap2_ch
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_31, BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 |
                                                BIT8 | BIT9 | BIT10 | BIT11);
    // BANK 1126, 16bit OFFSET 0x32[3:0]: reg_pd_rterm_ch
    //                         0x32[7:4]: reg_pd_ldo_predrv_ch
    //                         0x32[11:8]: reg_pd_ldo_mux_ch
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_32, BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 |
                                                BIT8 | BIT9 | BIT10 | BIT11);
    // BANK 1126, 16bit OFFSET 0x33[0]: reg_pd_ldo_clktree
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_33, BIT0);
    // BANK 1126, 16bit OFFSET 0x34[0]: reg_pd_hdmitxpll
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_34, BIT0);
    // BANK 1126, 16bit OFFSET 0x35[3:0]: reg_pd_drv_biasgen_ch, 0x35[4]: reg_pd_biasgen
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_35, BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
#endif
#ifdef CONFIG_ANALOG_PD_IDAC_ATOP
    // BANK 1127, 16bit OFFSET 0x15[0]: reg_en_idac_b
    //                         0x15[1]: reg_en_idac_g
    //                         0x15[2]: reg_en_idac_r
    OUTREG16(BASE_REG_DAC_ATOP_PA + REG_ID_15, 0x0);
    // BANK 1127, 16bit OFFSET 0x16[0]: reg_en_idac_ref
    OUTREG16(BASE_REG_DAC_ATOP_PA + REG_ID_16, 0x0);
    // BANK 1127, 16bit OFFSET 0x14[0]: reg_en_hd_dac_b_det
    //                         0x14[1]: reg_en_hd_dac_g_det
    //                         0x14[2]: reg_en_hd_dac_r_det
    OUTREG16(BASE_REG_DAC_ATOP_PA + REG_ID_14, 0x0);
    // BANK 1127, 16bit OFFSET 0x1F[0]: reg_pd_idac_ldo
    OUTREG16(BASE_REG_DAC_ATOP_PA + REG_ID_1F, 0x1);
    // BANK 1127, 16bit OFFSET 0x10[0]: reg_gpio_en_pad_out_b
    //                         0x10[1]: reg_gpio_en_pad_out_g
    //                         0x10[2]: reg_gpio_en_pad_out_r
    OUTREG16(BASE_REG_DAC_ATOP_PA + REG_ID_10, 0x0);
#endif
#ifdef CONFIG_ANALOG_PD_IDAC_LPLL
    // BANK 1127, 16bit OFFSET 0x36[0]: reg_emmcpll_pd
    SETREG16(BASE_REG_HDMI_TX_ATOP_PA + REG_ID_36, BIT0);
#endif
#ifdef CONFIG_ANALOG_PD_DISP_LPLL
    // BANK 1133, 16bit OFFSET 0x40[15]: reg_lpll_ext_pd
    SETREG16(BASE_REG_DISP_LPLL_PA + REG_ID_40, BIT15);
#endif
#ifdef CONFIG_ANALOG_PD_MIPI_DPHY_TX_TOP
     // BANK 1528, 16bit OFFSET 0x00[6]: reg_pd_ldo
    SETREG16(BASE_REG_DPHY_DSI_PA + REG_ID_00, BIT6);
    // BANK 1528, 16bit OFFSET 0x04[4]: reg_sw_lptx_en0
    //                         0x04[6]: reg_sw_lprx_en0
    //                         0x04[9:8]: reg_sw_outconf_ch0_bit
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_04, BIT4 | BIT6 | BIT8 | BIT9);
    // BANK 1528, 16bit OFFSET 0x08[4]: reg_sw_lptx_en1
    //                         0x08[6]: reg_sw_lprx_en1
    //                         0x08[9:8]: reg_sw_outconf_ch1_bit
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_08, BIT4 | BIT6 | BIT8 | BIT9);
    // BANK 1528, 16bit OFFSET 0x0c[4]: reg_sw_lptx_en2
    //                         0x0c[6]: reg_sw_lprx_en2
    //                         0x0c[9:8]: reg_sw_outconf_ch2_bit
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_0C, BIT4 | BIT6 | BIT8 | BIT9);
    // BANK 1528, 16bit OFFSET 0x20[4]: reg_sw_lptx_en3
    //                         0x20[6]: reg_sw_lprx_en3
    //                         0x20[9:8]: reg_sw_outconf_ch3_bit
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_20, BIT4 | BIT6 | BIT8 | BIT9);
    // BANK 1528, 16bit OFFSET 0x23[4]: reg_sw_lptx_en4
    //                         0x23[6]: reg_sw_lprx_en4
    //                         0x23[9:8]: reg_sw_outconf_ch4_bit
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_23, BIT4 | BIT6 | BIT8 | BIT9);
    // BANK 1528, 16bit OFFSET 0x05[10]: reg_sw_hsrx_en_ch0
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_05, BIT10);
    // BANK 1528, 16bit OFFSET 0x09[10]: reg_sw_hsrx_en_ch1
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_09, BIT10);
    // BANK 1528, 16bit OFFSET 0x0D[10]: reg_sw_hsrx_en_ch2
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_0D, BIT10);
    // BANK 1528, 16bit OFFSET 0x21[10]: reg_sw_hsrx_en_ch3
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_21, BIT10);
    // BANK 1528, 16bit OFFSET 0x24[10]: reg_sw_hsrx_en_ch4
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_24, BIT10);
    // BANK 1528, 16bit OFFSET 0x03[0]: reg_sw_dphy_cken
    CLRREG16(BASE_REG_DPHY_DSI_PA + REG_ID_03, BIT0);
#endif
#ifdef CONFIG_ANALOG_PD_MPLL
    // BANK 1030, 16bit OFFSET 0x01[8]: reg_pd_mpll
    //                         0x01[9]: reg_pd_mpll_clk_adc_vco_div2
    //                         0x01[10]: reg_pd_mpll_clk_adc_vco_div2_2
    //                         0x01[11]: reg_pd_mpll_clk_adc_vco_div2_3
    //                         0x01[12]: reg_pd_digclk
    SETREG16(BASE_REG_MPLL_PA + REG_ID_01, BIT8 | BIT9 | BIT10 | BIT11 | BIT12);
    // BANK 1030, 16bit OFFSET 0x02[0]: reg_en_mpll_rst
    CLRREG16(BASE_REG_MPLL_PA + REG_ID_02, BIT0);
    // BANK 1030, 16bit OFFSET 0x04[0]: reg_en_mpll_test
    //                         0x04[1]: reg_en_mpll_ov_sw
    //                         0x04[10]: reg_en_mpll_xtal
    //                         0x04[15]: reg_en_mpll_prdt
    CLRREG16(BASE_REG_MPLL_PA + REG_ID_04, BIT0 | BIT1 | BIT10 | BIT15);
#endif
#ifdef CONFIG_ANALOG_PD_SATA_ATOP
    // BANK 1525, 16bit OFFSET 0x00
    OUTREG16(BASE_REG_SATA_MAC_PA + REG_ID_00, 0x0000);
    // BANK 1527, 16bit OFFSET 0x20[0]: reg_pd_sata_txpll
    SETREG16(BASE_REG_SATA_ATOP_PA + REG_ID_20, BIT0);
    // BANK 1527, 16bit OFFSET 0x30[0]: reg_pd_sata_rxpll
    SETREG16(BASE_REG_SATA_ATOP_PA + REG_ID_30, BIT0);
    // BANK 1527, 16bit OFFSET 0x31[10]: reg_pd_sata_rxpll_cdr
    SETREG16(BASE_REG_SATA_ATOP_PA + REG_ID_31, BIT10);
    // BANK 1526, 16bit OFFSET 0x08[15:0]
    OUTREG16(BASE_REG_SATA_PHY_PA + REG_ID_08, 0xFFFF);
    // BANK 1526, 16bit OFFSET 0x00[15:0]
    OUTREG16(BASE_REG_SATA_PHY_PA + REG_ID_00, 0x105B);
    // BANK 1526, 16bit OFFSET 0x01[5]: reg_ssusb_pll_ssc_en
    //                         0x01[6]: reg_ssusb_rx_impcalib_en
    CLRREG16(BASE_REG_SATA_PHY_PA + REG_ID_00, BIT5 | BIT6);
#endif
#ifdef CONFIG_ANALOG_PD_UPLL_0
    // BANK 1420, 16bit OFFSET 0x00[1]: reg_upll_pd
    //                         0x00[4]: reg_upll_enddisc
    //                         0x00[5]: reg_upll_enfrun
    SETREG16(BASE_REG_UPLL0_PA + REG_ID_00, BIT1 | BIT4 | BIT5);
    // BANK 1420, 16bit OFFSET 0x00[7]: reg_upll_enxtal
    CLRREG16(BASE_REG_UPLL0_PA + REG_ID_00, BIT7);
    // BANK 1420, 16bit OFFSET 0x01[7]: reg_upll_en_prdt
    CLRREG16(BASE_REG_UPLL0_PA + REG_ID_01, BIT7);
    // BANK 1420, 16bit OFFSET 0x02[15:0]: reg_upll_test
    OUTREG16(BASE_REG_UPLL0_PA + REG_ID_02, 0x0000);
    // BANK 1420, 16bit OFFSET 0x07[0]: reg_clk0_upll_384_en
    //                         0x07[1]: reg_upll_en_prdt2
    //                         0x07[2]: reg_en_clk_upll_192m
    CLRREG16(BASE_REG_UPLL0_PA + REG_ID_07, BIT0 | BIT1 | BIT2);
    // BANK 1420, 16bit OFFSET 0x07[3]: reg_pd_clk0_audio
    SETREG16(BASE_REG_UPLL0_PA + REG_ID_07, BIT3);
#endif
#ifdef CONFIG_ANALOG_PD_UPLL_1
    // BANK 141F, 16bit OFFSET 0x00[1]: reg_upll_pd
    //                         0x00[4]: reg_upll_enddisc
    //                         0x00[5]: reg_upll_enfrun
    SETREG16(BASE_REG_UPLL1_PA + REG_ID_00, BIT1 | BIT4 | BIT5);
    // BANK 141F, 16bit OFFSET 0x00[7]: reg_upll_enxtal
    CLRREG16(BASE_REG_UPLL1_PA + REG_ID_00, BIT7);
    // BANK 141F, 16bit OFFSET 0x01[7]: reg_upll_en_prdt
    CLRREG16(BASE_REG_UPLL1_PA + REG_ID_01, BIT7);
    // BANK 141F, 16bit OFFSET 0x02[15:0]: reg_upll_test
    OUTREG16(BASE_REG_UPLL1_PA + REG_ID_02, 0x0000);
    // BANK 141F, 16bit OFFSET 0x07[0]: reg_clk0_upll_384_en
    //                         0x07[1]: reg_upll_en_prdt2
    //                         0x07[2]: reg_en_clk_upll_192m
    CLRREG16(BASE_REG_UPLL1_PA + REG_ID_07, BIT0 | BIT1 | BIT2);
    // BANK 141F, 16bit OFFSET 0x07[3]: reg_pd_clk0_audio
    SETREG16(BASE_REG_UPLL1_PA + REG_ID_07, BIT3);
#endif
#ifdef CONFIG_ANALOG_PD_USB20_P1
    // BANK 1421, 16bit OFFSET 0x00[2]: reg_ref_pdn
    //                         0x00[6]: reg_r_dp_pden
    //                         0x00[7]: reg_r_dm_pden
    //                         0x00[8]: reg_hs_dm_pdn
    //                         0x00[9]: reg_pll_pdn
    //                         0x00[10]: reg_hs_ted_pdn
    //                         0x00[11]: reg_hs_preamp_pdn
    //                         0x00[12]: reg_fl_xcvr_pdn
    //                         0x00[13]: reg_vbusdet_pdn
    //                         0x00[14]: reg_iref_pdn
    //                         0x00[15]: reg_reg_pdn
    SETREG16(BASE_REG_UTMI0_PA + REG_ID_00, BIT2 | BIT6 | BIT7 | BIT8 | BIT9 | BIT10 |
                                            BIT11 | BIT12 | BIT13 | BIT14 | BIT15);
    // BANK 1421, 16bit OFFSET 0x04[7]: reg_clk_extra_0_ena/PD_BG_CURRENT
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_04, 0x0080);
    // BANK 1421, 16bit OFFSET 0x05[6]: reg_clktest_inv/HS_TXSER_EN
    CLRREG16(BASE_REG_UTMI0_PA + REG_ID_05, BIT6);
#endif
#ifdef CONFIG_ANALOG_PD_USB20_P2
    // BANK 1425, 16bit OFFSET 0x00[2]: reg_ref_pdn
    //                         0x00[6]: reg_r_dp_pden
    //                         0x00[7]: reg_r_dm_pden
    //                         0x00[8]: reg_hs_dm_pdn
    //                         0x00[9]: reg_pll_pdn
    //                         0x00[10]: reg_hs_ted_pdn
    //                         0x00[11]: reg_hs_preamp_pdn
    //                         0x00[12]: reg_fl_xcvr_pdn
    //                         0x00[13]: reg_vbusdet_pdn
    //                         0x00[14]: reg_iref_pdn
    //                         0x00[15]: reg_reg_pdn
    SETREG16(BASE_REG_UTMI1_PA + REG_ID_00, BIT2 | BIT6 | BIT7 | BIT8 | BIT9 | BIT10 |
                                            BIT11 | BIT12 | BIT13 | BIT14 | BIT15);
    // BANK 1425, 16bit OFFSET 0x04[7]: reg_clk_extra_0_ena/PD_BG_CURRENT
    OUTREG16(BASE_REG_UTMI1_PA + REG_ID_04, 0x0080);
    // BANK 1425, 16bit OFFSET 0x05[6]: reg_clktest_inv/HS_TXSER_EN
    CLRREG16(BASE_REG_UTMI1_PA + REG_ID_05, BIT6);
#endif
#ifdef CONFIG_ANALOG_PD_USB20_P3
    // BANK 1429, 16bit OFFSET 0x00[2]: reg_ref_pdn
    //                         0x00[6]: reg_r_dp_pden
    //                         0x00[7]: reg_r_dm_pden
    //                         0x00[8]: reg_hs_dm_pdn
    //                         0x00[9]: reg_pll_pdn
    //                         0x00[10]: reg_hs_ted_pdn
    //                         0x00[11]: reg_hs_preamp_pdn
    //                         0x00[12]: reg_fl_xcvr_pdn
    //                         0x00[13]: reg_vbusdet_pdn
    //                         0x00[14]: reg_iref_pdn
    //                         0x00[15]: reg_reg_pdn
    SETREG16(BASE_REG_UTMI2_PA + REG_ID_00, BIT2 | BIT6 | BIT7 | BIT8 | BIT9 | BIT10 |
                                            BIT11 | BIT12 | BIT13 | BIT14 | BIT15);
    // BANK 1429, 16bit OFFSET 0x04[7]: reg_clk_extra_0_ena/PD_BG_CURRENT
    OUTREG16(BASE_REG_UTMI2_PA + REG_ID_04, 0x0080);
    // BANK 1429, 16bit OFFSET 0x05[6]: reg_clktest_inv/HS_TXSER_EN
    CLRREG16(BASE_REG_UTMI2_PA + REG_ID_05, BIT6);
#endif
}

extern char* LX_VERSION;
static void __init mstar_init_machine(void)
{
    struct soc_device *soc_dev;
    struct device *parent = NULL;

    pr_info("\n\nVersion : %s\n\n",LX_VERSION);

    mstar_reset_us_ticks_cycle_offset();
    us_ticks_factor=div64_u64(arch_timer_get_rate(),1000000);

    mstar_soc_dev_attr.family = kasprintf(GFP_KERNEL, mstar_platform_name);
    mstar_soc_dev_attr.revision = kasprintf(GFP_KERNEL, "%d", mstar_get_revision());
    mstar_soc_dev_attr.soc_id = kasprintf(GFP_KERNEL, "%u", mstar_get_device_id());
    mstar_soc_dev_attr.api_version = kasprintf(GFP_KERNEL, ms_chip_get()->chip_get_API_version());
    mstar_soc_dev_attr.machine = kasprintf(GFP_KERNEL, of_flat_dt_get_machine_name());

    soc_dev = soc_device_register(&mstar_soc_dev_attr);
    if (IS_ERR(soc_dev)) {
        kfree((void *)mstar_soc_dev_attr.family);
        kfree((void *)mstar_soc_dev_attr.revision);
        kfree((void *)mstar_soc_dev_attr.soc_id);
        kfree((void *)mstar_soc_dev_attr.machine);
        goto out;
    }

    parent = soc_device_to_device(soc_dev);

    /*
     * Finished with the static registrations now; fill in the missing
     * devices
     */
out:
    of_platform_populate(NULL, of_default_bus_match_table, NULL, parent);

    //write log_buf address to mailbox
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x08), (int)log_buf_addr_get() & 0xFFFF);
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x09), ((int)log_buf_addr_get() >> 16 )& 0xFFFF);

    // power down analog IP here
    mstar_analog_ip_powerdown();
}

struct mcm_client{
    char* name;
    short index;
    short slow_down_ratio;
};


static struct mcm_client mcm_clients[] = {
    {"MCU51",         MCM_ID_MCU51, 0},
    {"URDMA",         MCM_ID_URDMA, 0},
    {"BDMA",          MCM_ID_BDMA, 0},
    {"VHE",           MCM_ID_VHE, 0},
    {"MFE",           MCM_ID_MFE, 0},
    {"JPE",           MCM_ID_JPE, 0},
    {"BACH",          MCM_ID_BACH, 0},
    {"AESDMA",        MCM_ID_AESDMA, 0},
    {"UHC",           MCM_ID_UHC, 0},
    {"EMAC",          MCM_ID_EMAC, 0},
    {"CMDQ",          MCM_ID_CMDQ, 0},
    {"ISP_DNR",       MCM_ID_ISP_DNR, 0},
    {"ISP_DMA",       MCM_ID_ISP_DMA, 0},
    {"GOP",           MCM_ID_GOP, 0},
    {"SC_DNR",        MCM_ID_SC_DNR, 0},
    {"SC_DNR_SAD",    MCM_ID_SC_DNR_SAD, 0},
    {"SC_CROP",       MCM_ID_SC_CROP, 0},
    {"SC1_FRM",       MCM_ID_SC1_FRM, 0},
    {"SC1_SNP",       MCM_ID_SC1_SNP, 0},
    {"SC1_DBG",       MCM_ID_SC1_DBG, 0},
    {"SC2_FRM",       MCM_ID_SC2_FRM, 0},
    {"SC3_FRM",       MCM_ID_SC3_FRM, 0},
    {"FCIE",          MCM_ID_FCIE, 0},
    {"SDIO",          MCM_ID_SDIO, 0},
    {"SC1_SNPI",      MCM_ID_SC1_SNPI, 0},
    {"SC2_SNPI",      MCM_ID_SC2_SNPI, 0},
    {"*ALL_CLIENTS*", MCM_ID_ALL, 0}  //use carefully
};

struct device mcm_dev;

static struct bus_type mcm_subsys = {
    .name = "mcm",
    .dev_name = "mcm",
};

static int mcm_rw(int index, int ratio, int write)
{
    int i, addr;

    if(index == MCM_ID_ALL && write)
    {
        for(i=0; i<(sizeof(mcm_clients)/sizeof(mcm_clients[0]))-1;i++)
            mcm_rw(i, ratio, write);
        return 0;
    }
    else if(index == MCM_ID_MCU51)
        addr = BASE_REG_MCM_DIG_GP_PA + 0x0;
    else if (index == MCM_ID_URDMA)
        addr = BASE_REG_MCM_DIG_GP_PA + 0x1;
    else if (index == MCM_ID_BDMA)
        addr = BASE_REG_MCM_DIG_GP_PA + 0x4;
    else if (index == MCM_ID_VHE)
        addr = BASE_REG_MCM_VHE_GP_PA + 0x0;
    else if (index == MCM_ID_MFE)
        addr = BASE_REG_MCM_SC_GP_PA + 0x0;
    else if (index == MCM_ID_JPE)
        addr = BASE_REG_MCM_SC_GP_PA + 0x1;
    else if (index == MCM_ID_BACH)
        addr = BASE_REG_MCM_SC_GP_PA + 0x4;
    else if (index == MCM_ID_AESDMA)
        addr = BASE_REG_MCM_SC_GP_PA + 0x5;
    else if (index == MCM_ID_UHC)
        addr = BASE_REG_MCM_SC_GP_PA + 0x8;
    else if (index == MCM_ID_EMAC)
        addr = BASE_REG_MCM_SC_GP_PA + 0x9;
    else if (index == MCM_ID_CMDQ)
        addr = BASE_REG_MCM_SC_GP_PA + 0xC;
    else if (index == MCM_ID_ISP_DNR)
        addr = BASE_REG_MCM_SC_GP_PA + 0xD;
    else if (index == MCM_ID_ISP_DMA)
        addr = BASE_REG_MCM_SC_GP_PA + 0x10;
    else if (index == MCM_ID_GOP)
        addr = BASE_REG_MCM_SC_GP_PA + 0x11;
    else if (index == MCM_ID_SC_DNR)
        addr = BASE_REG_MCM_SC_GP_PA + 0x14;
    else if (index == MCM_ID_SC_DNR_SAD)
        addr = BASE_REG_MCM_SC_GP_PA + 0x15;
    else if (index == MCM_ID_SC_CROP)
        addr = BASE_REG_MCM_SC_GP_PA + 0x18;
    else if (index == MCM_ID_SC1_FRM)
        addr = BASE_REG_MCM_SC_GP_PA + 0x19;
    else if (index == MCM_ID_SC1_SNP)
        addr = BASE_REG_MCM_SC_GP_PA + 0x1C;
    else if (index == MCM_ID_SC1_DBG)
        addr = BASE_REG_MCM_SC_GP_PA + 0x1D;
    else if (index == MCM_ID_SC2_FRM)
        addr = BASE_REG_MCM_SC_GP_PA + 0x20;
    else if (index == MCM_ID_SC3_FRM)
        addr = BASE_REG_MCM_SC_GP_PA + 0x21;
    else if (index == MCM_ID_FCIE)
        addr = BASE_REG_MCM_SC_GP_PA + 0x24;
    else if (index == MCM_ID_SDIO)
        addr = BASE_REG_MCM_SC_GP_PA + 0x25;
    else if (index == MCM_ID_SC1_SNPI)
        addr = BASE_REG_MCM_SC_GP_PA + 0x28;
    else if (index == MCM_ID_SC2_SNPI)
        addr = BASE_REG_MCM_SC_GP_PA + 0x29;
    else
    {
        printk(KERN_ERR "mcm_clients[%d] not exists\n", index);
        return -1;
    }

    if(write)
        OUTREG8(addr, (ratio << 4));
    else
        mcm_clients[index].slow_down_ratio = (INREG8(addr) >> 4);

    return 0;
}


static ssize_t mcm_slow_ratio_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int index, ratio;
    sscanf(buf, "%d %d", &index, &ratio);

    if(0 > ratio || ratio > 15)
    {
        printk(KERN_ERR "MCM slow down ratio should be 0~15\n");
        return -1;
    }

    return mcm_rw(index, ratio, 1)?-1:n;
}

static ssize_t mcm_slow_ratio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0;

    for(i=0; i<(sizeof(mcm_clients)/sizeof(mcm_clients[0]))-1;i++)
    {
        mcm_rw(i, 0, 0);
        str += scnprintf(str, end - str, "[%d] %s: %d\n", mcm_clients[i].index, mcm_clients[i].name, mcm_clients[i].slow_down_ratio);
    }
    str += scnprintf(str, end - str, "[%d] %s\n", mcm_clients[i].index, mcm_clients[i].name);

    return (str - buf);
}

DEVICE_ATTR(mcm_slow_ratio, 0644, mcm_slow_ratio_show, mcm_slow_ratio_store);

static void __init mstar_create_MCM_node(void)
{
    int ret;

    mcm_dev.kobj.name="mcm";
    mcm_dev.bus=&mcm_subsys;

    ret = subsys_system_register(&mcm_subsys, NULL);
    if (ret)
    {
        printk(KERN_ERR "Failed to register MCM sub system!! %d\n",ret);
        return;
    }

    ret = device_register(&mcm_dev);
    if(ret)
    {
        printk(KERN_ERR "Failed to register MCM device!! %d\n",ret);
        return;
    }

    device_create_file(&mcm_dev, &dev_attr_mcm_slow_ratio);
}




extern void mstar_create_MIU_node(void);
extern int mstar_pm_init(void);
extern void init_proc_zen(void);
static inline void __init mstar_init_late(void)
{
#ifdef CONFIG_PM_SLEEP
    mstar_pm_init();
#endif
    mstar_create_MIU_node();
    mstar_create_MCM_node();
}

static void global_reset(enum reboot_mode mode, const char * cmd)
{
    U16 i=0;
#if 0
    //fsp
    //Check flash status
    SETREG16(0x1f002dbc, BIT0);//h6F
    OUTREG16(0x1f002db0, 0x0000);//h6C
//  do
//  {
        OUTREG16(0x1f002d80, 0x0005); //h60
        OUTREG16(0x1f002da8, 0x0001); //h6A
        OUTREG16(0x1f002dac, 0x0001); //h6B
        OUTREG16(0x1f002db0, 0x2007); //h6C
        OUTREG16(0x1f002db4, 0x0001); //h6D
        while(!(INREG16(0x1f002db8)&BIT0)) //h6E
        ;
        SETREG16(0x1f002dbc, BIT0);   //h6F//
    } while((INREG16(0x1f002d94)&BIT0) == BIT0); //h65
#else
    //riu_isp
    OUTREG16(0x1f001000, 0xAAAA);
    //password
    do {
        i++;
        OUTREG16(0x1f001004, 0x0005);  //cmd
        OUTREG16(0x1f001030, 0x0001);  //trigger
        while((INREG16(0x1f001054) & 0x1) != 0x1)  //check read data ready
        ;

        if (Chip_Get_Storage_Type()!= MS_STORAGE_NOR) //if no nor-flash
            break;
    }while((INREG16(0x1f001014) & 0x1) != 0x0);//check WIP=0
#endif

    while(1)
    {
        OUTREG8(0x1f221000, 0x30+i);
        mdelay(5);
        OUTREG8(0x1f001cb8, 0x79);
    }
}

#ifdef CONFIG_SMP
extern struct smp_operations infinity2m_smp_ops;
#endif

DT_MACHINE_START(MS_DT, "SStar Soc (Flattened Device Tree)")
    .dt_compat    = mstar_dt_compat,
    .map_io = mstar_map_io,
    .init_machine = mstar_init_machine,
    .init_early = mstar_init_early,
//    .init_time =  ms_init_timer,
//    .init_irq = mstar_init_irqchip,
    .init_late = mstar_init_late,
    .restart = global_reset,
#ifdef CONFIG_SMP
    .smp        = smp_ops(infinity2m_smp_ops),
#endif
MACHINE_END
