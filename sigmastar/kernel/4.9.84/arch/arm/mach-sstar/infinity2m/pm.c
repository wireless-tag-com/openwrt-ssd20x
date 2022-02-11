/*
* pm.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/suspend.h>
#include <asm/fncpy.h>
#include <asm/cacheflush.h>
#include "ms_platform.h"
#include <linux/delay.h>
#include <asm/secure_cntvoff.h>
#include "gpio.h"
#include "mdrv_gpio.h"
#include "gpi-irqs.h"
#include "registers.h"

#define FIN     printk(KERN_ERR"[%s]+++\n",__FUNCTION__)
#define FOUT    printk(KERN_ERR"[%s]---\n",__FUNCTION__)
#define HERE    printk(KERN_ERR"%s: %d\n",__FILE__,__LINE__)
#define SUSPEND_WAKEUP 0
#define SUSPEND_SLEEP  1
#define STR_PASSWORD   0x5A5A55AA
#define IN_SRAM 1

#if IN_SRAM
#define SUSPENDINFO_ADDRESS 0xA000B000
#else
#define SUSPENDINFO_ADDRESS 0x20000000
#endif
//----0xA0000000~0xA000A000---IPL.rom
//----0xA000C000~0xA000D000---IPL.ram
//----0xA0001000~0xA0002000---suspend code




typedef struct {
    char magic[8];
    unsigned int resume_entry;
    unsigned int count;
    unsigned int status;
    unsigned int password;
} suspend_keep_info;

extern void sram_suspend_imi(void);
static void (*mstar_suspend_imi_fn)(void);
static void __iomem *suspend_imi_vbase;
static suspend_keep_info *pStr_info;
int suspend_status = SUSPEND_WAKEUP;

#if !IN_SRAM
static void TurnOnISOFlow(void)
{
retry:
    // Turn-on ISO flow
    OUTREG8(0x1F00680C, 0x00);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x01);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x03);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00){
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x07);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08){
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x05);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;;
    }
    OUTREG8(0x1F00680C, 0x01);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x00);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;
    }
}
#endif

extern int MDrv_GPIO_PadVal_Set(u8 u8IndexGPIO, u32 u32PadMode);
#ifdef CONFIG_MS_SW_I2C
extern u16 MDrv_SW_IIC_Start(u8 u8ChIIC);
extern u16 MDrv_SW_IIC_SendByte(u8 u8ChIIC, u8 u8data, u8 u8Delay4Ack );
extern void MDrv_SW_IIC_Stop(u8 u8ChIIC);
#endif

extern void recode_timestamp(int mark, const char* name);


U64 arch_counter_get_cntpct1(void)
{
    U64 cval;
    asm volatile("isb " : : : "memory");
    asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (cval));
    return cval;
}

unsigned int read_timestamp1(void)
{
    u64 cycles=arch_counter_get_cntpct1();
    u64 usticks=div64_u64(cycles, 6/*us_ticks_factor*/);
    return (unsigned int) usticks;
}
#ifdef CONFIG_MS_SW_I2C
void goodix_irq_handler(void)
{
     u8 gpi_irq = 0;
     u32 int_status = 7;
     u64 int_delay = 0;
     u8 loop = 0;

     do{
                for (loop =0;loop<=20;loop++)
                {
                    MDrv_GPIO_PadVal_Set(2,0); //gpio mode
                    MDrv_GPIO_PadVal_Set(3,0);

                    if(!MDrv_SW_IIC_Start(0))
                        *(unsigned short volatile *) 0xFD200808 = 0x1;
                    if(!MDrv_SW_IIC_SendByte(0, 0XBA , 0))
                        *(unsigned short volatile *) 0xFD200808 = 0x2;
                    if(!MDrv_SW_IIC_SendByte(0, 0x81, 0))
                        *(unsigned short volatile *) 0xFD200808 = 0x3;
                    if(!MDrv_SW_IIC_SendByte(0, 0x4e, 0))
                        *(unsigned short volatile *) 0xFD200808 = 0x4;

                    MDrv_SW_IIC_SendByte(0, 0, 0);
                    MDrv_SW_IIC_Stop(0);
                    
                    MDrv_GPIO_PadVal_Set(2, 0x10); //i2c mode
                    MDrv_GPIO_PadVal_Set(3, 0x10);
                    *(unsigned short volatile *) 0xFD200800 = 0x5;

                    /*clear all gpi*/
                    for( gpi_irq = 0 ; gpi_irq < GPI_FIQ_END ; gpi_irq++)
                    {
                        SETREG16( (BASE_REG_GPI_INT_PA + REG_ID_0A + (gpi_irq/16)*4 ) , (1 << (gpi_irq%16)) );
                    }
                }

                /*sleep*/
                for(int_delay=0;int_delay<=0xf000000;int_delay++){
                    *(unsigned short volatile *) 0xFD200814 = int_delay;
                }

                int_status = (*(unsigned short volatile *) 0xFD20137c); //gpi-source
                *(unsigned short volatile *) 0xFD20080C = int_status;
    }while(int_status==0x0100);//gpi-source
}
#endif
extern unsigned int read_timestamp(void);
static int mstar_suspend_ready(unsigned long ret)
{
#if 1
    long long gpi_irq =0;
//    u8 status = 0;
#endif

    *(unsigned short volatile *) 0xFD200800 = 0x2222;
    mstar_suspend_imi_fn = fncpy(suspend_imi_vbase, (void*)&sram_suspend_imi, 0x1000);
    suspend_status = SUSPEND_SLEEP;
    //resume info
    if (pStr_info) {
        pStr_info->count++;
        pStr_info->status = SUSPEND_SLEEP;
        pStr_info->password = STR_PASSWORD;
    }
#ifdef CONFIG_MS_SW_I2C
    if(*(unsigned short volatile *) 0xFD20137c==0x100)  //int-irq gpi's parent
        goodix_irq_handler();
#endif
  *(unsigned short volatile *) 0xFD200800 = 0x2233;
    //flush cache to ensure memory is updated before self-refresh
    __cpuc_flush_kern_all();
        //flush L3 cache
    Chip_Flush_MIU_Pipe();
    //flush tlb to ensure following translation is all in tlb
    local_flush_tlb_all();

#if 0
    for(i=PAD_GPIO2; i<=PAD_GPIO12; i++){
        MDrv_GPIO_PadVal_Set(i, 0x0);
        MDrv_GPIO_Set_Low(i);
    }

    for(i=PAD_GPIO14; i<=PAD_SATA_GPIO; i++){
        MDrv_GPIO_PadVal_Set(i, 0x0);
        MDrv_GPIO_Set_Low(i);
    }
#endif
    /*clear gpi*/
    for( gpi_irq = 0 ; gpi_irq < GPI_FIQ_END ; gpi_irq++)
    {
        SETREG16( (BASE_REG_GPI_INT_PA + REG_ID_0A + (gpi_irq/16)*4 ) , (1 << (gpi_irq%16)) );
    }
    *(unsigned short volatile *) 0xFD200800 = 0x2224;
    mstar_suspend_imi_fn();
    return 0;
}

static int mstar_suspend_enter(suspend_state_t state)
{
#if !IN_SRAM
     unsigned int resume_pbase = virt_to_phys(cpu_resume);
#endif
    //FIN;
    *(unsigned short volatile *) 0xFD200800 = 0x99aA;
    switch (state) 
    {
        case PM_SUSPEND_MEM:
            printk(KERN_INFO "state = PM_SUSPEND_MEM\n");

#if !IN_SRAM
            /* Keep resume address to RTC SW0/SW1 registers */
            OUTREG16(0x1F006810, (resume_pbase & 0xFFFF));
            OUTREG16(0x1F006800, 0x0020);
            // Turn-on ISO flow
            TurnOnISOFlow();
            OUTREG16(0x1F006810, ((resume_pbase >> 16) & 0xFFFF));
            OUTREG16(0x1F006800, 0x0040);
            // Turn-on ISO flow
            TurnOnISOFlow();
            OUTREG16(0x1F006800, 0x0000);
#endif
            cpu_suspend(0, mstar_suspend_ready);
            *(unsigned short volatile *) 0xFD200800 = 0x99aB;
            #ifdef CONFIG_SMP
                secure_cntvoff_init();
            *(unsigned short volatile *) 0xFD200800 = 0x99aC;
            #endif
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static void mstar_suspend_wake(void)
{
    if (pStr_info) {
        pStr_info->status = SUSPEND_WAKEUP;
        pStr_info->password = 0;
    }
}

struct platform_suspend_ops mstar_suspend_ops = {
    .enter    = mstar_suspend_enter,
    .wake     = mstar_suspend_wake,
    .valid    = suspend_valid_only_mem,
};


int __init mstar_pm_init(void)
{
    unsigned int resume_pbase = virt_to_phys(cpu_resume);
    static void __iomem * suspend_info_vbase = (void *)SUSPENDINFO_ADDRESS;
    suspend_imi_vbase = __arm_ioremap_exec(0xA0001000, 0x1000, false);  //put suspend code at IMI offset 64K;

#if IN_SRAM
    suspend_info_vbase =  __arm_ioremap_exec(SUSPENDINFO_ADDRESS, 0x1000, false);
    pStr_info = (suspend_keep_info *)(suspend_info_vbase);
#else
    pStr_info = (suspend_keep_info *)__va(suspend_info_vbase);
#endif

    memset(pStr_info, 0, sizeof(suspend_keep_info));
    strcpy(pStr_info->magic, "SIG_STR");
    pStr_info->resume_entry = resume_pbase;

    suspend_set_ops(&mstar_suspend_ops);

    printk(KERN_INFO "[%s] resume_pbase=0x%08X, suspend_imi_vbase=0x%08X\n", __func__, (unsigned int)resume_pbase, (unsigned int)suspend_imi_vbase);

    return 0;
}
