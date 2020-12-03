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
        //{IMI_VIRT,  __phys_to_pfn(IMI_PHYS),    IMI_SIZE,       MT_DEVICE},
};


static const char *mstar_dt_compat[] __initconst = {
    "sstar,infinity5",
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
*        Mstar chip flush function
*************************************/
static DEFINE_SPINLOCK(mstar_l2prefetch_lock);

static void mstar_uart_disable_line(int line)
{
    if(line == 0)  //for debug, do not change
    {
        // UART0_Mode -> X
        //CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4 | BIT5 | BIT6);
    }
    else if(line == 1)
    {
        // UART1_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8 | BIT9);
    }
    else if(line == 2)
    {
        // FUART_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0 | BIT1 | BIT2);
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
#define STORAGE_EMMC                BIT3
#define STORAGE_P_NAND              BIT4
#define STORAGE_SPI_NOR             BIT5
*/
    u8 type = (INREG16(BASE_REG_DIDKEY_PA + 0x70*4) & 0x3C);

    if(BIT4 == type)
        return (int)MS_STORAGE_NAND;
    else if(BIT5 == type)
        return (int)MS_STORAGE_NOR;
    else if(BIT3 == type)
        return (int)MS_STORAGE_EMMC;
    else if(BIT2 == type)
        return (int)MS_STORAGE_SPINAND_ECC;
    else
        return (int)MS_STORAGE_UNKNOWN;
}

static int mstar_get_package_type(void)
{
    static MS_I5_PACKAGE_TYPE pkg_type = MS_I5_PACKAGE_UNKNOWN;

    if (pkg_type != MS_I5_PACKAGE_UNKNOWN)
        return pkg_type;

    if(!strcmp(&mstar_soc_dev_attr.machine[10], "SSC007A-S01A"))
    {
        pkg_type = MS_I5_PACKAGE_BGA_13X13;
    }
    else if(!strcmp(&mstar_soc_dev_attr.machine[10], "SSC007B-S01A") || !strcmp(&mstar_soc_dev_attr.machine[10], "SSC007B-S01B"))
    {
        pkg_type = MS_I5_PACKAGE_QFN;
    }
    else if(!strcmp(&mstar_soc_dev_attr.machine[10], "FPGA"))
    {
        pkg_type = MS_I5_PACKAGE_FPGA_128MB;
    }
    else
    {
        printk(KERN_ERR "!!!!! Machine name [%s] \n", mstar_soc_dev_attr.machine);
    }

    return pkg_type;
}
static char mstar_platform_name[]="I5";

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
        else
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-2");
        }
        if(!of_property_read_u32(np, "power-enable-pad", &pin_data))
        {
            printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
            power_en_gpio = (unsigned char)pin_data;
        }
        else
        {
            printk(KERN_ERR "Can't get power-enable-pad from DTS, set default GPIO(%d)\n", pin_data);
            power_en_gpio = PAD_PM_GPIO2;
        }
        ret = gpio_request(power_en_gpio, "USB0-power-enable");
        if (ret < 0) {
            printk(KERN_INFO "Failed to request USB0-power-enable GPIO(%d)\n", power_en_gpio);
            power_en_gpio =-1;
            return ret;
        }
    }

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
}

struct mcm_client{
    char* name;
    unsigned int reg;
    short slow_down_ratio;
};

static struct mcm_client mcm_clients[] = {
    {"MCU51",       REG_MCM_MCU51,      0},
    {"URDMA",       REG_MCM_URDMA,      0},
    {"BDMA",        REG_MCM_BDMA,       0},
    {"DIP_R",       REG_MCM_DIP_R,      0},
    {"DIP_W",       REG_MCM_DIP_W,      0},
    {"LDC",         REG_MCM_LDC,        0},
    {"SC2_FRM",     REG_MCM_SC2_FRM,    0},
    {"SC3_FRM",     REG_MCM_SC3_FRM,    0},
    {"RSC",         REG_MCM_RSC,        0},
    {"SC1_DBG",     REG_MCM_SC1_DBG,    0},
    {"CMDQ0",       REG_MCM_CMDQ0,      0},
    {"MOVDMA0",     REG_MCM_MOVDMA0,    0},
    {"EMAC",        REG_MCM_EMAC,       0},
    {"3DNR0_R",     REG_MCM_3DNR0_R,    0},
    {"3DNR0_W",     REG_MCM_3DNR0_W,    0},
    {"GOP4",        REG_MCM_GOP4,       0},
    {"ISP_DMAG0",   REG_MCM_ISP_DMAG0,  0},
    {"ISP_DMAG1",   REG_MCM_ISP_DMAG1,  0},
    {"ISP_DMAG2",   REG_MCM_ISP_DMAG2,  0},
    {"GOP2",        REG_MCM_GOP2,       0},
    {"GOP3",        REG_MCM_GOP3,       0},
    {"ISP_DMAG",    REG_MCM_ISP_DMAG,   0},
    {"ISP_STA",     REG_MCM_ISP_STA,    0},
    {"ISP_STA1",    REG_MCM_ISP_STA1,   0},
    {"CMDQ1",       REG_MCM_CMDQ1,      0},
    {"MOVDMA1",     REG_MCM_MOVDMA1,    0},
    {"IVE",         REG_MCM_IVE,        0},
    {"SC_ROT_R",    REG_MCM_SC_ROT_R,   0},
    {"SC_AIP_W",    REG_MCM_SC_AIP_W,   0},
    {"SC0_FRM",     REG_MCM_SC0_FRM,    0},
    {"SC0_SNP",     REG_MCM_SC0_SNP,    0},
    {"SC1_FRM",     REG_MCM_SC1_FRM,    0},
    {"GOP0",        REG_MCM_GOP0,       0},
    {"3DNR1_R",     REG_MCM_3DNR1_R,    0},
    {"3DNR1_W",     REG_MCM_3DNR1_W,    0},
    {"CMDQ2",       REG_MCM_CMDQ2,      0},
    {"AESDMA",      REG_MCM_AESDMA,     0},
    {"USB20",       REG_MCM_USB20,      0},
    {"USB20_H",     REG_MCM_USB20_H,    0},
    {"JPE",         REG_MCM_JPE,        0},
    {"GOP1",        REG_MCM_GOP1,       0},
    {"BACH",        REG_MCM_BACH,       0},
    {"CMDQ3",       REG_MCM_CMDQ3,      0},
    {"SDIO",        REG_MCM_SDIO,       0},
    {"FCIE",        REG_MCM_FCIE,       0},
    {"VEN",         REG_MCM_VEN,        0},

    {"*ALL_CLIENTS",0,                  0}  //use carefully
};

struct device mcm_dev;

static struct bus_type mcm_subsys = {
    .name = "mcm",
    .dev_name = "mcm",
};

static int mcm_rw(int index, int ratio, int write)
{
    int i, addr, cnt;

    cnt = sizeof(mcm_clients)/sizeof(mcm_clients[0]);
    if((index == MCM_ID_ALL) && write)
    {
        for(i=0; i<cnt-1; i++)
        {
            mcm_rw(i, ratio, write);
        }
        return 0;
    }
    else if(index < cnt)
    {
        addr = mcm_clients[index].reg;
    }
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
        str += scnprintf(str, end - str, "[%d] %s: %d\n", i, mcm_clients[i].name, mcm_clients[i].slow_down_ratio);
    }
    str += scnprintf(str, end - str, "[%d] %s\n", i, mcm_clients[i].name);

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

        if (Chip_Get_Storage_Type()!= MS_STORAGE_NOR) //if not nor-flash
            break;
    } while((INREG16(0x1f001014) & 0x1) != 0x0);//check WIP=0
#endif

    while(1)
    {
        OUTREG8(0x1f221000, 0x30+i);
        mdelay(5);
        OUTREG8(0x1f001cb8, 0x79);
    }
}

DT_MACHINE_START(MS_DT, "SStar Soc (Flattened Device Tree)")
    .dt_compat    = mstar_dt_compat,
    .map_io = mstar_map_io,
    .init_machine = mstar_init_machine,
    .init_early = mstar_init_early,
//    .init_time =  ms_init_timer,
//    .init_irq = mstar_init_irqchip,
    .init_late = mstar_init_late,
    .restart = global_reset,
MACHINE_END
