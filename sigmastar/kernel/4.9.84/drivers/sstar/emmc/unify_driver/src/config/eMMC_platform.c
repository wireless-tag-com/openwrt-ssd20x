/*
* eMMC_platform.c- Sigmastar
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

#include "eMMC.h"

#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

//
static U32 _gu32MaxClk = 0;

void eMMC_Prepare_Power_Saving_Mode_Queue(void)
{
    #if (defined(eMMC_DRV_CHICAGO_LINUX) && eMMC_DRV_CHICAGO_LINUX)
    REG_FCIE_SETBIT(REG_BATTERY, reg_nobat_int_en);

    /* (1) Clear HW Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x40), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x41),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

    /* (2) Clear All Interrupt */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x42), 0xffff);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x43),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

    /* (3) Clear SDE MODE Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x44), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x45),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x10);

    /* (4) Clear SDE CTL Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x46), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x47),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x11);

    /* (5) Reset Start */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x48), 0x4800);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x49),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x30);

    /* (6) Reset End */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4A), 0x5800);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x30);

    /* (7) Set "SD_MOD" */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4C), 0x0051);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x10);

    /* (8) Enable "csreg_sd_en" */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4E), 0x0002);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x4F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

    /* (9) Command Content, IDLE */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x50), 0x0040);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x51),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK1 | 0x00);

    /* (10) Command Content, IDLE */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x52), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x53),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK1 | 0x01);

    /* (11) Command Content, IDLE */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x54), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x55),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK1 | 0x02);

    /* (12) Command Size */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x56), 0x0005);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x57),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0E);

    /* (13) Response Size */
    OUTREG16(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x58), 0x0000);
    OUTREG16(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x59),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0D);

    /* (14) Enable Interrupt, SD_CMD_END */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5A), 0x0002);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01);

    /* (15) Command Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5C), 0x0004);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x11);

    /* (16) Wait Interrupt */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5E), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x5F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT);

    /* (17) Clear Interrupt */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x60), 0x0002);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x61),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

    /* (18) Clear HW Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x62), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x63),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

    /* (19) STOP */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x64), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x65),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP);

    REG_FCIE_W(FCIE_PWR_SAVE_MODE, BIT_POWER_SAVE_MODE_EN | BIT_SD_POWER_SAVE_RST);
    #endif
}


//=============================================================
#if eMMC_DRV_LINUX
//=============================================================
U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
    BIT_FCIE_CLK_48M,
    BIT_FCIE_CLK_40M,
    BIT_FCIE_CLK_36M,
    BIT_FCIE_CLK_32M,
    BIT_FCIE_CLK_20M,
};
U32 gu32_eMMCDrvExtFlag = 0;

#if defined(MSTAR_EMMC_CONFIG_OF)
struct clk_data{
    int num_parents;
    struct clk **clk_fcie;
//  struct clk *clk_ecc;
};
extern struct clk_data *clkdata;
#endif

void mdelay_MacroToFun(u32 time)
{
    mdelay(time);
}

U32 eMMC_hw_timer_delay(U32 u32usTick)
{
    volatile U32 u32_i=u32usTick;

    while(u32_i>1000)
    {
        udelay(1000);
        u32_i-=1000;
    }

    udelay(u32usTick);
    return u32usTick + 1;
}

U32 eMMC_hw_timer_sleep(U32 u32ms)
{
    U32 u32_i = u32ms;

    while(u32_i > 1000)
    {
        msleep(1000);
        u32_i -= 1000;
    }

    msleep(u32_i);
    return u32ms;
}

//--------------------------------
// use to performance test
U32 eMMC_hw_timer_start(void)
{
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
    return 0;
}

void eMMC_DumpPadClk(void)
{
    //---------------------------------------------------------------------
    eMMC_debug(0, 0, "[pad setting]:\r\n");
    switch(g_eMMCDrv.u8_PadType)
    {
        case FCIE_eMMC_BYPASS:          eMMC_debug(0, 0, "Bypass\r\n");  break;
        case FCIE_eMMC_SDR:             eMMC_debug(0, 0, "SDR\r\n");  break;
        default:
            eMMC_debug(0, 0, "eMMC Err: Pad unknown, %d\r\n", g_eMMCDrv.u8_PadType); eMMC_die("\r\n");
            break;
    }
}

U8 gu8_NANDeMMC_need_preset_flag = 1;

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
    g_eMMCDrv.u8_PadType = u32_FCIE_IF_Type;

#if PADMUX_CTRL
    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);
    REG_FCIE_CLRBIT(reg_sd_config, BIT_SD_MODE_MASK);
#endif

#if !(SDIO_SETTING_ONLY)
    #if defined(BIT_EMMC_MODE_8X)
    REG_FCIE_CLRBIT(reg_sdio_config, BIT_SDIO_MODE_MASK);
    //Move pad switch to mstar_mci_probe() in mstar_mci_v5.c
    #else
    REG_FCIE_CLRBIT(reg_nand_config, BIT_NAND_MODE_MASK);
    REG_FCIE_SETBIT(reg_emmc_config, BIT_EMMC_MODE_1);
    #endif
#endif

    // fcie
    REG_FCIE_CLRBIT(FCIE_DDR_MODE, BIT_MACRO_MODE_MASK);

    //eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 1, "SDR\r\n");

    REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_PAD_IN_SEL_SD|BIT_FALL_LATCH|BIT10);


    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;

    return eMMC_ST_SUCCESS;

}

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
#if 0 //defined(MSTAR_EMMC_CONFIG_OF)
    U32 u32_clkrate = 0;
    switch(u16_ClkParam)
    {
        case BIT_FCIE_CLK_300K:
            u32_clkrate = 300*1000;
            break;
        case BIT_CLK_XTAL_12M:
            u32_clkrate = 12*1000*1000;
            break;
        case BIT_FCIE_CLK_20M:
            u32_clkrate = 20*1000*1000;
            break;
        case BIT_FCIE_CLK_32M:
            u32_clkrate = 32*1000*1000;
            break;
        case BIT_FCIE_CLK_36M:
            u32_clkrate = 36*1000*1000;
            break;
        case BIT_FCIE_CLK_40M:
            u32_clkrate = 40*1000*1000;
            break;
        case BIT_FCIE_CLK_43_2M:
            u32_clkrate = 43*1000*1000;
            break;
        case BIT_FCIE_CLK_48M:
            u32_clkrate = 48*1000*1000;
            break;
        default:
            eMMC_die();
            break;
    }
    //printk("set clock %d\n", u32_clkrate);
    g_eMMCDrv.u32_ClkKHz = u32_clkrate/1000;
    g_eMMCDrv.u16_ClkRegVal = u16_ClkParam;
    clk_set_rate(clkdata->clk_fcie[0], u32_clkrate);
#else
    eMMC_PlatformResetPre();

#if MAX_CLK_CTRL
    //printk("u16_ClkParam = %d\r\n",u16_ClkParam);
    //printk("_gu32MaxClk = %d\r\n",_gu32MaxClk);
    if (u16_ClkParam < _gu32MaxClk)
    {
        u16_ClkParam = _gu32MaxClk;
    }
#endif

    switch(u16_ClkParam)    {
    case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
    case BIT_CLK_XTAL_12M:      g_eMMCDrv.u32_ClkKHz = 12000;   break;
    case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
    case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
    default:
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
        return eMMC_ST_ERR_INVALID_PARAM;
    }

    //eMMC_debug(0, 1, "clock %dk\n", g_eMMCDrv.u32_ClkKHz);

    REG_FCIE_CLRBIT(reg_ckg_fcie, BIT0|BIT1);
    REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_CLKGEN_FCIE_MASK|BIT_FCIE_CLK_SRC_SEL);
    REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<2);
    REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL); //reg_ckg_sdio(BK:x1038_x43) [B5] -> 0:clk_boot 1:clk_sd

#if SDIO_SETTING_ONLY
    REG_FCIE_SETBIT(reg_sc_gp_ctrl, BIT3);
#else
    REG_FCIE_SETBIT(reg_sc_gp_ctrl, BIT7); //reg_ckg_sd(BK:x1133_x25) [B3]SDIO30 [B7]SD30  -> 0:clk_boot 1:clk_sd
#endif
    g_eMMCDrv.u16_ClkRegVal = u16_ClkParam;

#endif
    eMMC_PlatformResetPost();

    return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
    eMMC_PlatformResetPre();
    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_CLK_EN);
    eMMC_PlatformResetPost();
    return eMMC_ST_SUCCESS;
}

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
    // do nothing
}

void eMMC_reset_WatchDog(void)
{
    // do nothing
}
extern struct platform_device sg_mstar_emmc_device_st;

dma_addr_t eMMC_DMA_MAP_address(uintptr_t ulongBuffer, U32 u32_ByteCnt, int mode)
{
    dma_addr_t dma_addr;

    if(mode == 0)   //write
    {
        dma_addr = dma_map_single(&sg_mstar_emmc_device_st.dev, (void*)ulongBuffer, u32_ByteCnt, DMA_TO_DEVICE);
    }
    else
    {
        dma_addr = dma_map_single(&sg_mstar_emmc_device_st.dev, (void*)ulongBuffer, u32_ByteCnt, DMA_FROM_DEVICE);
    }

    if( dma_mapping_error(&sg_mstar_emmc_device_st.dev, dma_addr) )
    {
        dma_unmap_single(&sg_mstar_emmc_device_st.dev, dma_addr, u32_ByteCnt, (mode) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
        eMMC_die("eMMC_DMA_MAP_address: Kernel can't mapping dma correctly\n");
    }

    return dma_addr;
}

void eMMC_DMA_UNMAP_address(dma_addr_t dma_DMAAddr, U32 u32_ByteCnt, int mode)
{
    if(mode == 0)   //write
    {
        dma_unmap_single(&sg_mstar_emmc_device_st.dev, dma_DMAAddr, u32_ByteCnt, DMA_TO_DEVICE);
    }
    else
    {
        dma_unmap_single(&sg_mstar_emmc_device_st.dev, dma_DMAAddr, u32_ByteCnt, DMA_FROM_DEVICE);
    }
}

#if 0
U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
#if 1
        extern void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
        extern void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
    //mode 0 for write, 1 for read
    if( mode == WRITE_TO_eMMC ) //Write
    {
        //Write (DRAM->NAND)-> flush
        Chip_Clean_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }
    else //Read
    {
        //Read (NAND->DRAM) -> inv
        Chip_Flush_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }
    /*
    if(virt_to_phys((void *)u32_DMAAddr) >= MSTAR_MIU1_BUS_BASE)
    {
        REG_SET_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
    }
    else
        REG_CLR_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
        */

    return virt_to_phys((void *)u32_DMAAddr);
#else
    flush_cache(u32_DMAAddr, u32_ByteCnt);
    return (u32_DMAAddr);
#endif
}
#else
extern U32 MIU0_BUS_ADDR;
U32 eMMC_translate_DMA_address_Ex(dma_addr_t dma_DMAAddr, U32 u32_ByteCnt)
{
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);
    dma_DMAAddr -= MSTAR_MIU0_BUS_BASE;

    return ((U32)dma_DMAAddr);
}

#endif
/*
void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
{
    flush_cache(u32_addr, s32_size);
}

void eMMC_flush_miu_pipe(void)
{

}
*/


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    0

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
    volatile u16 u16_Events;

    // one time enable one bit

    REG_FCIE_R(FCIE_PWR_SAVE_CTL, u16_Events);

    if(u16_Events & BIT_POWER_SAVE_MODE_INT)
    {
        REG_FCIE_CLRBIT(FCIE_PWR_SAVE_CTL, BIT_POWER_SAVE_MODE_INT_EN);
        fcie_int = 1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0, "SAR5 eMMC WARN.\n");
        while(1);
        wake_up(&fcie_wait);
        return IRQ_HANDLED;
    }

    if((REG_FCIE(FCIE_MIE_FUNC_CTL) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
    {
        return IRQ_NONE;
    }

    u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

    if(u16_Events & (BIT_DMA_END|BIT_ERR_STS))
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));

        fcie_int = 1;
        wake_up(&fcie_wait);
        return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CMD_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CMD_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
        return IRQ_HANDLED;
    }
    #if defined(ENABLE_FCIE_HW_BUSY_CHECK)&&ENABLE_FCIE_HW_BUSY_CHECK
    else if(u16_Events & BIT_BUSY_END_INT)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_BUSY_END_INT);
        REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_BUSY_DET_ON);

        fcie_int = 1;
        wake_up(&fcie_wait);

        return IRQ_HANDLED;
    }
    #endif


    #if eMMC_IRQ_DEBUG
    if(0==fcie_int)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
            REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
    #endif

    return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(uintptr_t u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
{
    U32 u32_i=0;

    #if eMMC_IRQ_DEBUG
    U32 u32_isr_tmp[2];
    unsigned long long u64_jiffies_tmp, u64_jiffies_now;
    struct timeval time_st;
    time_t sec_tmp;
    suseconds_t us_tmp;

    u32_isr_tmp[0] = fcie_int;
    do_gettimeofday(&time_st);
    sec_tmp = time_st.tv_sec;
    us_tmp = time_st.tv_usec;
    u64_jiffies_tmp = jiffies_64;
    #endif

    //----------------------------------------
    if(wait_event_timeout(fcie_wait, (fcie_int == 1), usecs_to_jiffies(u32_MicroSec)) == 0)
    {
        #if eMMC_IRQ_DEBUG
        u32_isr_tmp[1] = fcie_int;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh, ISR:%u->%u->%u \n",
            u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN),
            u32_isr_tmp[0], u32_isr_tmp[1], fcie_int);

        do_gettimeofday(&time_st);
        u64_jiffies_now = jiffies_64;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            " PassTime: %lu s, %lu us, %llu jiffies.  WaitTime: %u us, %lu jiffies, HZ:%u.\n",
            time_st.tv_sec-sec_tmp, time_st.tv_usec-us_tmp, u64_jiffies_now-u64_jiffies_tmp,
            u32_MicroSec, usecs_to_jiffies(u32_MicroSec), HZ);
        #else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Warn: int timeout, WaitEvt:%Xh, NowEvt:%Xh, IntEn:%Xh \n",
            u16_WaitEvent, REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN));
        #endif

        // switch to polling
        for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
        {
            if((REG_FCIE(u32_RegAddr) & u16_WaitEvent) == u16_WaitEvent )
                break;

            eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        }

        if(u32_i == u32_MicroSec)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: events lose, WaitEvent: %Xh \n", u16_WaitEvent);
            eMMC_DumpDriverStatus();  eMMC_DumpPadClk();
            eMMC_FCIE_DumpRegisters();eMMC_FCIE_DumpDebugBus();
            REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
            return eMMC_ST_ERR_INT_TO;
        }
        else
        {   REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
        }

    }
    //----------------------------------------
    fcie_int = 0;
    return eMMC_ST_SUCCESS;
}

#endif



//---------------------------------------
//extern struct semaphore   PfModeSem;
//#include <linux/semaphore.h>
//extern struct semaphore   PfModeSem;
extern struct mutex FCIE3_mutex;
//#define CRIT_SECT_BEGIN(x)    mutex_lock(x)
//#define CRIT_SECT_END(x)  mutex_unlock(x)

extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
    mutex_lock(&FCIE3_mutex);
    #if defined (MSTAR_EMMC_CONFIG_OF)
    {
        int i;
        for(i = 0 ;i < clkdata->num_parents; i ++)
            clk_prepare_enable(clkdata->clk_fcie[i]);
    }
    #endif

    #if IF_FCIE_SHARE_IP // && defined(CONFIG_MS_SDMMC)
//  if(false == ms_sdmmc_wait_d0_for_emmc())
//  {
//      eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
//      eMMC_FCIE_ErrHandler_Stop();
//  }

    REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_DEBUG_MODE_MASK);
    REG_FCIE_SETBIT(FCIE_TEST_MODE, 2<<BIT_DEBUG_MODE_SHIFT); // 2: card_data1_dbus = {xxxxx, Wrstate, RDstate}

    REG_FCIE_CLRBIT(FCIE_EMMC_DEBUG_BUS1, BIT11|BIT10|BIT9|BIT8);
    REG_FCIE_SETBIT(FCIE_EMMC_DEBUG_BUS1, 5<<8); // 5: card


    if(REG_FCIE(FCIE_EMMC_DEBUG_BUS0)&0x0FFF) //Check FICE5 StateMachine
    {
        eMMC_FCIE_Init();
    }


    eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
    eMMC_pads_switch(g_eMMCDrv.u8_PadType);
    #endif
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock

    REG_FCIE_SETBIT(FCIE_MIE_FUNC_CTL, BIT_EMMC_ACTIVE);

}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
    REG_FCIE_CLRBIT(FCIE_MIE_FUNC_CTL, BIT_EMMC_ACTIVE);
    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
    #if defined (MSTAR_EMMC_CONFIG_OF)
    {
        int i;
        for(i = 0 ;i < clkdata->num_parents; i ++)
            clk_disable_unprepare(clkdata->clk_fcie[i]);
    }
    #endif

    mutex_unlock(&FCIE3_mutex);
}

U32 eMMC_PlatformResetPre(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
    eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif

    return eMMC_ST_SUCCESS;
}
struct page *eMMC_SectorPage = 0;
struct page *eMMC_PartInfoPage = 0;
U8 *gau8_eMMC_SectorBuf = 0; // 512 bytes
U8 *gau8_eMMC_PartInfoBuf =0; // 512 bytes

U32 eMMC_PlatformInit(void)
{
    eMMC_pads_switch(EMMC_DEFO_SPEED_MODE);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);

    if(gau8_eMMC_SectorBuf == NULL)
    {
        eMMC_SectorPage = alloc_pages(__GFP_COMP, 2);
        if(eMMC_SectorPage ==NULL)
        {
            eMMC_debug(0, 1, "Err allocate page 1 fails\n");
            eMMC_die();
        }
        gau8_eMMC_SectorBuf =(U8*) kmap(eMMC_SectorPage);
    }

    if(gau8_eMMC_PartInfoBuf == NULL)
    {
        eMMC_PartInfoPage = alloc_pages(__GFP_COMP, 0);
        if(eMMC_PartInfoPage ==NULL)
        {
            eMMC_debug(0, 1, "Err allocate page 2 fails\n");
            eMMC_die();
        }
        gau8_eMMC_PartInfoBuf = (U8*)kmap(eMMC_PartInfoPage);
    }

    return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
    return eMMC_ST_SUCCESS;
}

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

U32 eMMC_CheckIfMemCorrupt(void)
{
    if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
        return eMMC_ST_ERR_MEM_CORRUPT;

    return eMMC_ST_SUCCESS;
}

int mstar_mci_Housekeep(void *pData)
{
    #if !(defined(eMMC_HOUSEKEEP_THREAD) && eMMC_HOUSEKEEP_THREAD)
    return 0;
    #endif



    while(1)
    {
        if(kthread_should_stop())
            break;
    }

    return 0;
}

U32 eMMC_PlatformDeinit(void)
{
    return eMMC_ST_SUCCESS;
}


//
void eMMC_SetMaxClk(U32 clk)
{
    _gu32MaxClk = clk;
}

#else


  #error "Error! no platform functions."
#endif
#endif
