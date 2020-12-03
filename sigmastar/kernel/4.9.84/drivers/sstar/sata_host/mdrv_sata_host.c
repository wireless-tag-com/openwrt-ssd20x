/*
* mdrv_sata_host.c- Sigmastar
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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>

#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#include <linux/libata.h>
#include <asm/io.h>
#include <mdrv_sata_host.h>
#include <mhal_sata_host.h>
#include <irqs.h>
//#include "MsIRQ.h"
//#include "chip_int.h"
//#include "chip_setup.h"
#include <../../../drivers/ata/ahci.h>

#ifdef CONFIG_ARCH_INFINITY2
#include "mhal_sata_host.c"
#endif

#define SW_OOB_MODE 0

#define sata_reg_write16(val, addr) { (*((volatile unsigned short*)(addr))) = (unsigned short)(val); }

#ifdef CONFIG_OF
    #include <linux/of.h>
    #include <linux/of_device.h>
#endif

//#ifdef CONFIG_ARCH_INFINITY2
#if 0
extern void ahci_save_initial_config(struct device *dev, struct ahci_host_priv *hpriv, unsigned int force_port_map, unsigned int mask_port_map);
#else
extern void ahci_save_initial_config(struct device *dev, struct ahci_host_priv *hpriv);
#endif

extern int ahci_scr_read(struct ata_link *link, unsigned int sc_reg, u32 *val);
extern int ahci_scr_write(struct ata_link *link, unsigned int sc_reg, u32 val);
extern unsigned ahci_scr_offset(struct ata_port *ap, unsigned int sc_reg);

#ifdef CONFIG_ARCH_INFINITY2
    #undef writel
    #undef readl

    extern u32 ahci_reg_read(void __iomem * p_reg_addr);
    extern void ahci_reg_write(u32 data, void __iomem * p_reg_addr);
    //extern u32 ahci_reg_read(phys_addr_t reg_addr);
    //extern void ahci_reg_write(u32 data, phys_addr_t p_reg_addr);;

    #define writel ahci_reg_write
    #define readl ahci_reg_read
#endif

#define SATA_DEBUG
#ifdef SATA_DEBUG
    #define sata_debug(fmt, args...) printk("[SATA0][%s] " fmt, __FUNCTION__, ##args)
#else
    #define sata_debug(fmt, args...) do {} while(0)
#endif
#define sata_info(fmt, args...) printk("[SATA0][INFO] " fmt, ##args)
#define sata_error(fmt, args...) printk("[SATA0][ERR] " fmt, ##args)

//#define SPEED_TEST1

#ifdef SPEED_TEST1
    struct timespec proc_start;
    u32 g_u32Diff1;
    u32 u32Diff[10];
    static u32 u32Cnt = 0;
#endif

struct prdte
{
    u32 dba;
    u32 dbau;
    u32 rev1;

    u32 dbc: 22;
    u32 rev2: 9;
    u32 i: 1;
};

struct sata_sstar_host_priv *ss_hpriv;

#if 0
static void print_fis(hal_cmd_h2dfis *pfis)
{
    hal_cmd_h2dfis * p = pfis;

    printk("fis @ 0x%8.8x, fis size = 0x%x\n", (unsigned int)pfis, sizeof(hal_cmd_h2dfis));
    printk("fis phys @ 0x%8.8x\n", virt_to_phys(pfis));
    printk("fis type = 0x%x\n", p->u8fis_type);
    printk("PMP:4\t = 0x%x\n", p->u8MPM);
    printk("rev:3\t = 0x%x\n", p->reserved_0);
    printk("iscmd:1\t = 0x%x\n", p->isclear);
    printk("ata cmd\t = 0x%x\n", p->ata_cmd);
    printk("feat.\t = 0x%x\n", p->fearure);
    printk("lba_l\t = 0x%x\n", p->lba_l);
    printk("lba_m\t = 0x%x\n", p->lba_m);
    printk("lba_h\t = 0x%x\n", p->lba_h);
    printk("dev\t = 0x%x\n", p->device);
    printk("lba_l_e\t = 0x%x\n", p->lba_l_exp);
    printk("lba_m_e\t = 0x%x\n", p->lba_m_exp);
    printk("lba_h_e\t = 0x%x\n", p->lba_h_exp);
    printk("feat. ex = 0x%x\n", p->fearure_exp);
    printk("sector\t = 0x%x (u16)\n", p->u16sector_cnt);
    printk("rev\t = 0x%x\n", p->reserved_1);
    printk("ctrl\t = 0x%x\n", p->control);
    printk("rev \t = 0x%x (u32)\n", p->reserved_2);
}

static void print_prdt(struct prdte * pt, u32 num)
{
    struct prdte * p = pt;
    int i;
    printk("prdt @ 0x%8.8x, prdte size = 0x%x, total = 0x%x\n", (unsigned int)pt, sizeof(struct prdte), sizeof(struct prdte) * num);

    printk("prdt @ phys 0x%8.8x\n", virt_to_phys(pt));
    for (i = 0; i < num; i++, p++)
    {
        printk("dba\t = 0x%x (u32)\n", p->dba);
        printk("dbau\t = 0x%x (u32)\n", p->dbau);
        printk("rev1\t = 0x%x (u32)\n", p->rev1);
        printk("dbc\t = 0x%x (:22)\n", p->dbc);
        printk("rev2\t = 0x%x (:9)\n", p->rev2);
        printk("i\t = 0x%x (:1)\n", p->i);
    }
}
static void p_prdt(struct prdte * pt)
{
    struct prdte * p = pt;
    int i;
    printk("prdt @ 0x%8.8x, prdte size = 0x%x\n", (unsigned int)pt, sizeof(struct prdte));

    printk("dba\t = 0x%x (u32)\n", p->dba);
    printk("dbau\t = 0x%x (u32)\n", p->dbau);
    printk("rev1\t = 0x%x (u32)\n", p->rev1);
    printk("dbc\t = 0x%x (:22)\n", p->dbc);
    printk("rev2\t = 0x%x (:9)\n", p->rev2);
    printk("i\t = 0x%x (:1)\n", p->i);
}
#endif
#if 0
static void print_cmd_header(hal_cmd_header *p)
{
    printk(KERN_DEBUG "command header @ 0x%8.8x, cmd header size = 0x%x\n", (unsigned int)p, sizeof(hal_cmd_header));

    //    printk(KERN_DEBUG "cmd fis len\t = 0x%x (:5)\n", p->cmd_fis_len);
    //    printk(KERN_DEBUG "isATAPI\t = 0x%x (:1)\n", p->isATA_PI);
    //    printk(KERN_DEBUG "isWrite\t = 0x%x (:1)\n", p->iswrite);
    //    printk(KERN_DEBUG "isprefetch\t = 0x%x (:1)\n", p->isprefetch);
    //    printk(KERN_DEBUG "isswrst\t = 0x%x (:1)\n", p->issoftreset);
    //    printk(KERN_DEBUG "isbist\t = 0x%x (:1)\n", p->isbist);
    //    printk(KERN_DEBUG "isclrok\t = 0x%x (:1)\n", p->isclearok);
    //    printk(KERN_DEBUG "rev1\t = 0x%x (:1)\n", p->reserverd);
    //    printk(KERN_DEBUG "PMPid\t = 0x%x (:4)\n", p->PMPid);
    //    printk(KERN_DEBUG "PRDlen\t = 0x%x (16)\n", p->PRDTlength);
    //    printk(KERN_DEBUG "PRDbyte\t = 0x%x (32)\n", p->PRDBytes);
    //    printk(KERN_DEBUG "ctbal\t = 0x%x (32)\n", p->ctba_lbase);
    //    printk(KERN_DEBUG "ctbah\t = 0x%x (32)\n", p->ctba_hbase);
    //    printk("resv1\t = 0x%x (32)\n", p->resv[0]);
    //    printk("resv2\t = 0x%x (32)\n", p->resv[1]);
    //    printk("resv3\t = 0x%x (32)\n", p->resv[2]);
    //    printk("resv4\t = 0x%x (32)\n", p->resv[3]);
}
#endif

static void build_cmd_fis(void *cmd_tbl, hal_cmd_h2dfis *pfis, phys_addr_t misc_base)
{
#if (SATA_CMD_TYPE != TYPE_RIU)
#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;

    writew(0x00, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_tbl, pfis, sizeof(hal_cmd_h2dfis));
    //print_fis((hal_cmd_h2dfis *)cmd_tbl);

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif
#else // TYPE_RIU mode
    u32 address = (u32)cmd_tbl;
    u32 * dptr = (u32 *)pfis;
    u32 offset;

    address = (u32)cmd_tbl;

    // loop through data
    for (offset = 0; offset < sizeof(hal_cmd_h2dfis); offset += 4)
    {
        // which address to write?
        writel(address, (void*)misc_base + SATA_MISC_CFIFO_ADDRL);
        //printf("write data 0x%8.8x to addr 0x%x\n", address, misc_base + SATA_MISC_CFIFO_ADDRL);

        // what data to write?
        writel(*dptr, (void*)misc_base + SATA_MISC_CFIFO_WDATAL);
        //printf("write data 0x%8.8x to addr 0x%x\n", *dptr, misc_base + SATA_MISC_CFIFO_WDATAL);

        // read(0) or write(1)? normally write
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_RORW);
        //sata_reg_write16(0x01, misc_base + SATA_MISC_CFIFO_RORW);
        //printf("write data 0x0001 to addr 0x%x\n", misc_base + SATA_MISC_CFIFO_RORW);

        // trigger
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_ACCESS);
        //sata_reg_write16(0x01, misc_base + SATA_MISC_CFIFO_ACCESS);
        //printf("write data 0x0001 to addr 0x%x\n", misc_base + SATA_MISC_CFIFO_ACCESS);

        address += 4;
        dptr++;
    }

#endif
}

static void build_cmd_prdt(void *base_address, u32 *pprdt, phys_addr_t misc_base, u32 prdt_num)
{
#if (SATA_CMD_TYPE != TYPE_RIU)
    void *cmd_address = base_address + SATA_KA9_CMD_DESC_OFFSET_TO_PRDT;

#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;


    writew(0x00, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_address, pprdt, sizeof(u32)*prdt_num * 4);
    //print_prdt(cmd_address, prdt_num);

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif
#else // TYPE_RIU
    u32 address = (u32)base_address + SATA_KA9_CMD_DESC_OFFSET_TO_PRDT;
    u32 * dptr = pprdt;
    u32 offset;

    //p_prdt((struct prdte *)pprdt);

    for (offset = 0; offset < sizeof(u32) * prdt_num * 4; offset += 4)
    {
        // which address to write?
        writel(address, (void*)misc_base + SATA_MISC_CFIFO_ADDRL);
        //printf("write data 0x%8.8x to addr 0x%x\n", address, misc_base + SATA_MISC_CFIFO_ADDRL);

        // what data to write?
        writel(*dptr, (void*)misc_base + SATA_MISC_CFIFO_WDATAL);
        //printf("write data 0x%8.8x to addr 0x%x\n", *dptr, misc_base + SATA_MISC_CFIFO_WDATAL);

        // read(0) or write(1)? normally write
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_RORW);
        //printf("write data 0x0001 to addr 0x%8.8x\n", misc_base + SATA_MISC_CFIFO_RORW);

        // trigger
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_ACCESS);
        //printf("write data 0x0001 to addr 0x%x\n", misc_base + SATA_MISC_CFIFO_ACCESS);

        address += 4;
        dptr++;
    }
#endif
}

extern void Chip_Flush_Memory(void);
static void build_cmd_header(void *cmd_slot, u32 u32offset_address, u32 *pcmdheader, phys_addr_t misc_base)
{
#if (SATA_CMD_TYPE != TYPE_RIU)
    void *cmd_address = cmd_slot + u32offset_address;

#if (SATA_CMD_TYPE == TYPE_XIU)
    unsigned long u32MiscAddr = misc_base;

    writew(0x00, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif

    memcpy(cmd_address, pcmdheader, SATA_KA9_CMD_HDR_SIZE);
    //print_cmd_header(cmd_address);
    Chip_Flush_Memory();

#if (SATA_CMD_TYPE == TYPE_XIU)
    writew(0x01, u32MiscAddr + SATA_MISC_ACCESS_MODE);
#endif
#else
    u32 address = (u32)cmd_slot + u32offset_address;
    u32 * dptr = pcmdheader;
    u32 offset;

    for (offset = 0; offset < SATA_KA9_CMD_HDR_SIZE; offset += 4)
    {
        // which address to write?
        writel(address, (void*)misc_base + SATA_MISC_CFIFO_ADDRL);
        //printf("write data 0x%8.8x to addr 0x%x\n", address, misc_base + SATA_MISC_CFIFO_ADDRL);

        // what data to write?
        writel(*dptr, (void*)misc_base + SATA_MISC_CFIFO_WDATAL);
        //printf("write data 0x%8.8x to addr 0x%x\n", *dptr, misc_base + SATA_MISC_CFIFO_WDATAL);

        // read(0) or write(1)? normally write
        //sata_reg_write16(0x01, misc_base + SATA_MISC_CFIFO_RORW);
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_RORW);
        //printf("write data 0x0001 to addr 0x%x\n", misc_base + SATA_MISC_CFIFO_RORW);

        // trigger
        //sata_reg_write16(0x01, misc_base + SATA_MISC_CFIFO_ACCESS);
        writew(0x01, (volatile void *)misc_base + SATA_MISC_CFIFO_ACCESS);
        //printf("write data 0x0001 to addr 0x%x\n", misc_base + SATA_MISC_CFIFO_ACCESS);

        address += 4;
        dptr++;
    }
#endif
}


static irqreturn_t sstar_ahci_single_level_irq_intr(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	struct ahci_host_priv *hpriv;
	unsigned int rc = 0;
	void __iomem *mmio;
	u32 irq_stat, irq_masked;

	VPRINTK("ENTER\n");

	hpriv = host->private_data;
	mmio = hpriv->mmio;

	/* sigh.  0xffffffff is a valid return from h/w */
	irq_stat = readl(mmio + HOST_IRQ_STAT);
	if (!irq_stat)
		return IRQ_NONE;

	irq_masked = irq_stat & hpriv->port_map;

	spin_lock(&host->lock);

	rc = ahci_handle_port_intr(host, irq_masked);

	/* HOST_IRQ_STAT behaves as level triggered latch meaning that
	 * it should be cleared after all the port events are cleared;
	 * otherwise, it will raise a spurious interrupt after each
	 * valid one.  Please read section 10.6.2 of ahci 1.1 for more
	 * information.
	 *
	 * Also, use the unmasked value to clear interrupt as spurious
	 * pending event on a dummy port might cause screaming IRQ.
	 */
	writel(irq_stat, mmio + HOST_IRQ_STAT);

	spin_unlock(&host->lock);

	VPRINTK("EXIT\n");

	return IRQ_RETVAL(rc);
}

static inline unsigned int sata_sstar_tag(unsigned int tag)
{
    /* all non NCQ/queued commands should have tag#0 */
    if (ata_tag_internal(tag))
    {
        return 0;
    }

    if (unlikely(tag >= SATA_SSTAR_QUEUE_DEPTH))
    {
        DPRINTK("tag %d invalid : out of range\n", tag);
        //printk("[%s][%d]\n",__FUNCTION__,__LINE__);
        return 0;
    }
    return tag;
}

static void sata_sstar_setup_cmd_hdr_entry(struct sata_sstar_port_priv *pp,
        unsigned int tag, u32 data_xfer_len, u8 num_prde,
        u8 fis_len, phys_addr_t misc_base)
{
    dma_addr_t cmd_descriptor_address;
    hal_cmd_header cmd_header = {0};
    void *cmd_slot = pp->cmd_slot;

    cmd_descriptor_address = pp->cmd_tbl_dma + tag * SATA_KA9_CMD_DESC_SIZE;

    cmd_header.cmd_fis_len = fis_len;
    cmd_header.PRDTlength = num_prde;
    cmd_header.isclearok = 0;
    cmd_header.PRDBytes = data_xfer_len;
    cmd_header.ctba_hbase = 0;
    cmd_header.ctba_lbase = cmd_descriptor_address;

    build_cmd_header(cmd_slot, tag * SATA_KA9_CMD_HDR_SIZE, (u32 *)&cmd_header, misc_base);
}

static unsigned int sata_sstar_fill_sg(struct ata_queued_cmd *qc,
                                       u32 *ttl, void *cmd_tbl, phys_addr_t misc_base)
{
    struct scatterlist *sg;
    u32 ttl_dwords = 0;
    u32 prdt[SATA_KA9_USED_PRD * 4] = {0};
    unsigned int si;

    for_each_sg(qc->sg, sg, qc->n_elem, si)
    {
        dma_addr_t sg_addr = sg_dma_address(sg);
        u32 sg_len = sg_dma_len(sg);

        if (si == (SATA_KA9_USED_PRD - 1) && ((sg_next(sg)) != NULL))
        {
            sata_error("setting indirect prde , out of prdt\n");
        }
        ttl_dwords += sg_len;

        prdt[si * 4 + 0] =  (u32)(MHal_SATA_bus_address(cpu_to_le32(sg_addr)));
        prdt[si * 4 + 1] =  (u32)(MHal_SATA_bus_address(cpu_to_le32(sg_addr)) >> 16);
        prdt[si * 4 + 2] = 0xFFFFFFFF;
        prdt[si * 4 + 3] = (cpu_to_le32(sg_len) - 1);
    }
    build_cmd_prdt(cmd_tbl, &prdt[0], misc_base, si);
    *ttl = ttl_dwords;
    return si;
}

static u32 sstar_sata_wait_reg(phys_addr_t reg_addr, u32 mask, u32 val, unsigned long interval, unsigned long timeout)
{
    u32 temp;
    unsigned long timeout_vale = 0;

    temp = readl((void*)reg_addr);

    while((temp & mask) == val)
    {
        msleep(interval);
        timeout_vale += interval;
        if (timeout_vale > timeout)
            break;
        temp = readl((void*)reg_addr);
    }
    return temp;
}

static int sstar_ahci_stop_engine(phys_addr_t port_base)
{
    u32 temp;

    temp = readl((void*)SS_PORT_CMD + port_base);

    /* check if the HBA is idle */
    if ((temp & (SS_PORT_CMD_START | PORT_CMD_LIST_ON)) == 0)
        return 0;

    /* setting HBA to idle */
    temp &= ~PORT_CMD_START;
    writel(temp, (void*)SS_PORT_CMD + port_base);

    temp = sstar_sata_wait_reg(SS_PORT_CMD + port_base, PORT_CMD_LIST_ON, PORT_CMD_LIST_ON, 1, 500);

    if (temp & PORT_CMD_LIST_ON)
        return -EIO;

    return 0;
}

static void sstar_ahci_start_engine(phys_addr_t port_base)
{
    u32 temp;

    /* Start Port DMA */
    temp = readl((void*)SS_PORT_CMD + port_base);
    temp |= PORT_CMD_START;
    writel(temp, (void*)SS_PORT_CMD + port_base);
    readl((void*)SS_PORT_CMD + port_base); /* Flush */
}

static void sstar_ahci_start_fis_rx(struct ata_port *ap)
{
    struct sata_sstar_port_priv *pp = ap->private_data;
    phys_addr_t port_base =ss_hpriv->port_base;//SATA_GHC_1_ADDRESS_START;
    u32 tmp;

    // set FIS registers
    tmp = pp->cmd_slot_dma;
    writel(tmp, (void*)SS_PORT_LST_ADDR + port_base);

    tmp = pp->rx_fis_dma;
    writel(tmp, (void*)SS_PORT_FIS_ADDR + port_base);

    // enable FIS reception
    tmp = readl((void*)SS_PORT_CMD + port_base);
    tmp |= PORT_CMD_FIS_RX;
    writel(tmp, (void*)SS_PORT_CMD + port_base);

    // flush
    tmp = readl((void*)SS_PORT_CMD + port_base);
}

static int sstar_ahci_stop_fis_rx(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    u32 tmp;

    // Disable FIS reception
    tmp = readl((void*)SS_PORT_CMD + port_base);
    tmp &= ~PORT_CMD_FIS_RX;
    writel(tmp, (void*)SS_PORT_CMD + port_base);

    // Wait FIS reception Stop for 1000ms
    tmp = sstar_sata_wait_reg(SS_PORT_CMD + port_base, PORT_CMD_FIS_ON, PORT_CMD_FIS_ON, 10, 1000);

    if (tmp & PORT_CMD_FIS_ON)
        return -EBUSY;

    return 0;
}

void GetStartTime(void)
{
#ifdef SPEED_TEST1
    getnstimeofday(&proc_start);
#endif
}

void GetEndTime(void)
{
#ifdef SPEED_TEST1
    u32 delta;
    struct timespec ct;

    getnstimeofday(&ct);
    delta = ct.tv_nsec + ((proc_start.tv_sec != ct.tv_sec) ? NSEC_PER_SEC : 0)
            - proc_start.tv_nsec;

    //if((delta / 1000) <= 100000)
    {
        //if((delta / 1000) > g_u32Diff1)
        g_u32Diff1 = (delta / 1000);
    }
#endif
}

void PrintTime(void)
{
#ifdef SPEED_TEST1
    //int i;

    //if((h2dfis.u8fis_type == 0x27) && ((h2dfis.ata_cmd == 0x60) || (h2dfis.ata_cmd == 0x61)))
    {
        u32Diff[u32Cnt] = g_u32Diff1;
        u32Cnt++;
        if(u32Cnt >= 10)
            u32Cnt = 0;

        if((u32Cnt % 10) == 9)
        {
            //for(i = 0 ;  i < 10 ; i++)
            printk("%d, %d, %d, %d, %d, %d\n", (int)(u32Diff[2]), (int)(u32Diff[3]), (int)(u32Diff[4]), (int)(u32Diff[5]), (int)(u32Diff[6]), (int)(u32Diff[7]));  // 246, 209,209
        }
    }
#endif
}

static void sata_sstar_qc_prep(struct ata_queued_cmd *qc)
{
    struct ata_port *ap = qc->ap;
    struct sata_sstar_port_priv *pp = ap->private_data;
    u32 misc_base = ss_hpriv->misc_base;
    unsigned int tag = sata_sstar_tag(qc->tag);
    hal_cmd_h2dfis h2dfis;
    u32 num_prde = 0;
    u32 ttl_dwords = 0;
    void *cmd_tbl;

#ifdef SPEED_TEST1
    //GetEndTime();
#endif

    //printk("%s\n", __func__);
#ifdef SPEED_TEST1
    //GetStartTime();
#endif

    cmd_tbl = pp->cmd_tbl + (tag * SATA_KA9_CMD_DESC_SIZE);

    ata_tf_to_fis(&qc->tf, qc->dev->link->pmp, 1, (u8 *)&h2dfis);

    build_cmd_fis(cmd_tbl, &h2dfis, misc_base);

    if (qc->flags & ATA_QCFLAG_DMAMAP)
    {
        num_prde = sata_sstar_fill_sg(qc, &ttl_dwords, cmd_tbl, misc_base);
    }

    sata_sstar_setup_cmd_hdr_entry(pp, tag, ttl_dwords,
                                   num_prde, 5, misc_base);

}

static unsigned int sata_sstar_qc_issue(struct ata_queued_cmd *qc)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    unsigned int tag = sata_sstar_tag(qc->tag);
    struct ata_port *ap = qc->ap->host->ports[0];
    struct ata_link *link = qc->dev->link;
    struct ahci_port_priv *pp = ap->private_data;

    //printk("%s\n", __func__);

    if (qc->tf.protocol == ATA_PROT_NCQ)
    {
        //printk("NCQ device- %d\n", qc->tag);
        writel(1 << qc->tag, (void*)SS_PORT_SCR_ACT + port_base);
//        ap->sactive = link->sactive;  // save ncq active tag
        //ap->sactive = link->sactive;  // save ncq active tag
        pp->active_link->sactive = link->sactive;
        //sata_info("sactive = 0x%x\n", ap->sactive);
    }
    writel(1 << tag, (void*)SS_PORT_CMD_ISSUE + port_base);

#ifdef SPEED_TEST1
    //GetStartTime();
#endif

    return 0;
}

static bool sata_sstar_qc_fill_rtf(struct ata_queued_cmd *qc)
{
    struct sata_sstar_port_priv *pp = qc->ap->private_data;
    //struct sata_sstar_host_priv *host_priv = qc->ap->host->private_data;
    //unsigned int tag = sata_sstar_tag(qc->tag);
    //hal_cmd_h2dfis cd;
    //phys_addr_t misc_base = host_priv->misc_base;
    //void *rx_fis;
    u8 *rx_fis;

    rx_fis = pp->rx_fis;
#if 0
    read_cmd_fis(rx_fis, (tag * SATA_KA9_CMD_DESC_SIZE), &cd, misc_base);
    ata_tf_from_fis((const u8 *)&cd, &qc->result_tf);
#endif
    if (qc->tf.protocol == ATA_PROT_PIO && qc->dma_dir == DMA_FROM_DEVICE &&
            !(qc->flags & ATA_QCFLAG_FAILED))
    {
        ata_tf_from_fis(rx_fis + RX_FIS_PIO_SETUP, &qc->result_tf);
        qc->result_tf.command = (rx_fis + RX_FIS_PIO_SETUP)[15];
        DPRINTK("--- result_tf.command = 0x%x\n", qc->result_tf.command);
    }
    else
    {
        ata_tf_from_fis(rx_fis + RX_FIS_D2H_REG, &qc->result_tf);
    }
    return true;
}


static unsigned sata_sstar_scr_offset(struct ata_port *ap, unsigned int sc_reg)
{
	static const int offset[] = {
		[SCR_STATUS]		= PORT_SCR_STAT,
		[SCR_CONTROL]		= PORT_SCR_CTL,
		[SCR_ERROR]		= PORT_SCR_ERR,
		[SCR_ACTIVE]		= PORT_SCR_ACT,
		[SCR_NOTIFICATION]	= PORT_SCR_NTF,
	};
	struct ahci_host_priv *hpriv = ap->host->private_data;

	if (sc_reg < ARRAY_SIZE(offset) &&
	    (sc_reg != SCR_NOTIFICATION || (hpriv->cap & HOST_CAP_SNTF)))
		return offset[sc_reg];
	return 0;
}


static int sata_sstar_scr_read(struct ata_link *link, unsigned int sc_reg, u32 *val)
{
    int offset = sata_sstar_scr_offset(link->ap, sc_reg);
    phys_addr_t port_base = ss_hpriv->port_base;
    if (offset)
    {
        *val = readl((void*)offset + port_base);
        return 0;
    }
    return -1;
}

static int sata_sstar_scr_write(struct ata_link *link, unsigned int sc_reg_in, u32 val)
{
    int offset = sata_sstar_scr_offset(link->ap, sc_reg_in);
    phys_addr_t port_base = ss_hpriv->port_base;
    //struct sata_sstar_host_priv *host_priv = link->ap->host->private_data;
    if (offset)
    {
        writel(val, (void*)offset + port_base);
        return 0;
    }
    return -1;
}

static void sata_sstar_freeze(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    writel(0, (void*)SS_PORT_IRQ_MASK + port_base);
}

static void sata_sstar_thaw(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    phys_addr_t hba_base = ss_hpriv->hba_base;
    u32 u32Temp = 0;

    // clear IRQ
    u32Temp = readl((void*)SS_PORT_IRQ_STAT + port_base);
    writel(u32Temp, (void*)SS_PORT_IRQ_STAT + port_base);

    // Clear Port 0 IRQ on HBA
    u32Temp = readl((void*)SS_HOST_IRQ_STAT + hba_base);
    writel(u32Temp, (void*)SS_HOST_IRQ_STAT + hba_base);

    // Enable Host Interrupt
    u32Temp = readl((void*)SS_HOST_CTL + hba_base);
    u32Temp |= HOST_IRQ_EN;
    writel(u32Temp, (void*)SS_HOST_CTL + hba_base);

    // Enable Port Interrupt
    writel(DEF_PORT_IRQ, (void*)SS_PORT_IRQ_MASK + port_base);
    readl((void*)SS_PORT_IRQ_MASK + port_base);
}

static unsigned int sata_sstar_dev_classify(struct ata_port *ap)
{
    struct ata_taskfile tf;
    u32 temp = 0;
    phys_addr_t port_base = ss_hpriv->port_base;
    temp = readl((void*)PORT_SIG + port_base/*SATA_GHC_0_P0_ADDRESS_START*/);

    tf.lbah = (temp >> 24) & 0xff;
    tf.lbam = (temp >> 16) & 0xff;
    tf.lbal = (temp >> 8) & 0xff;
    tf.nsect = temp & 0xff;
    return ata_dev_classify(&tf);
}

int sstar_ahci_check_ready(struct ata_link *link)
{
    u8 status;
    phys_addr_t port_base = ss_hpriv->port_base;

    status = readl((void*)SS_PORT_TFDATA + port_base) & 0xFF;

    return ata_check_ready(status);
}

#define CONNECTION_RETRY 5
static int sata_sstar_hardreset(struct ata_link *link, unsigned int *class,
                                unsigned long deadline)
{
    struct ata_port *ap = link->ap;
    const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
    phys_addr_t port_base = ss_hpriv->port_base;
    phys_addr_t misc_base = ss_hpriv->misc_base;
    bool online;
    int rc;
    //u32 sstatus;
    //u32 retry = 0;
    //u16 u16Temp;

    //printk("%s:port_base:%X\n", __func__, port_base);
    //printk("%s:host_priv->port_base:%X\n", __func__, host_priv->port_base);

    //hard_reset:
    sstar_ahci_stop_engine(port_base);

    rc = sata_link_hardreset(link, timing, deadline, &online,
                             sstar_ahci_check_ready);

    sstar_ahci_start_engine(port_base);

#if 0//CONFIG_sstar_K6Lite // sw patch for gen3
    if (online)
    {
        rc = sata_sstar_scr_read(link, SCR_STATUS, &sstatus);
        sata_info("SStatus = 0x%x\n", sstatus);
        if (rc == 0 && ((sstatus & 0xf) == 0x3))   // 0x3 => device presence detected and Phy communication established
        {
            if ((((sstatus >> 4) & 0xf) == 0x01) && retry++ < CONNECTION_RETRY)   // gen1
            {
                sata_info("Gen1 connection detected, SATA PHY retrain %d\n", retry);
                u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x10390e << 1));
                u16Temp &= ~0x200;
                writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x10390e << 1));

                u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x10390e << 1));
                u16Temp |= 0x200;
                writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x10390e << 1));
                goto hard_reset;
            }
        }
    }
    if (retry >= CONNECTION_RETRY)
        sata_info("Gen1 HDD detected\n");
#endif

    if (online)
        *class = sata_sstar_dev_classify(ap);
    else
        MHal_SATA_Clock_Config(misc_base, port_base, FALSE);

    printk(KERN_INFO "[%s][port_base:0x%08x] done : %s\n", __func__, port_base, (online ? "online" : "offline"));
    return rc;
}

static int sata_sstar_softreset(struct ata_link *link, unsigned int *class,
                                unsigned long deadline)
{
    // Unused Function
    return 0;
}

static void sata_sstar_error_handler(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    struct ata_device *dev = ap->link.device;

    DPRINTK("%s begin\n", __func__);
    if (!(ap->pflags & ATA_PFLAG_FROZEN))
    {
        sstar_ahci_stop_engine(port_base);
        sstar_ahci_start_engine(port_base);
    }
    sata_pmp_error_handler(ap);

    //if (!ata_dev_enable(ap->link.device))
    if (!(dev->class == ATA_DEV_ATA ||
            dev->class == ATA_DEV_ATAPI ||
            dev->class == ATA_DEV_PMP ||
            dev->class == ATA_DEV_SEMB))
        sstar_ahci_stop_engine(port_base);

    DPRINTK("%s end\n", __func__);
}

static int sstar_ahci_kick_engine(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    struct ahci_host_priv *hpriv = ap->host->private_data;
    u8 status = readl((void*)port_base + SS_PORT_TFDATA);
    u32 tmp;
    int busy, rc;

    rc = sstar_ahci_stop_engine(port_base);
    if (rc)
        goto out_restart;

    busy = status & (ATA_BUSY | ATA_DRQ);
    if (!busy && !sata_pmp_attached(ap))
    {
        rc = 0;
        goto out_restart;
    }
    if (!(hpriv->cap & HOST_CAP_CLO))
    {
        rc = -EOPNOTSUPP;
        goto out_restart;
    }

    tmp = readl((void*)port_base + SS_PORT_CMD);
    tmp |= PORT_CMD_CLO;
    writel(tmp, (void*)port_base + SS_PORT_CMD);

    rc = 0;
    tmp = ata_wait_register(ap, (void __iomem *) port_base + SS_PORT_CMD,
                            (u32)PORT_CMD_CLO, PORT_CMD_CLO, 1, 500);
    if (tmp & PORT_CMD_CLO)
        rc = -EIO;
out_restart:
    sstar_ahci_start_engine(port_base);
    return rc;
}

static void sata_sstar_post_internal_cmd(struct ata_queued_cmd *qc)
{
    struct ata_port *ap = qc->ap;

    if (qc->flags & ATA_QCFLAG_FAILED)
        sstar_ahci_kick_engine(ap);
    //qc->err_mask |= AC_ERR_OTHER;
#if 0
    if (qc->err_mask)
    {
        /* make DMA engine forget about the failed command */
    }
#endif
}

static int sata_sstar_port_start(struct ata_port *ap)
{
    struct sata_sstar_port_priv *pp;
//    struct sata_sstar_host_priv *host_priv = ap->host->ports[0]->private_data;
    u32 temp;
    phys_addr_t port_base = ss_hpriv->port_base;
    u32 GHC_PHY = 0x0;

#if (SATA_CMD_TYPE == TYPE_DRAM)
    struct device *dev = ap->host->dev;
    void *mem;
    dma_addr_t mem_dma;
    size_t dma_sz;
#endif

    // Allocate SATA Port Private Data
    pp = kzalloc(sizeof(*pp), GFP_KERNEL);
    if (!pp)
    {
        printk("[%s][Error] SATA Allocate Port Private Data Fail\n", __func__);
        return -ENOMEM;
    }

#if (SATA_CMD_TYPE == TYPE_XIU)
    printk(KERN_INFO "SATA cmd type: XIU\n");
    pp->cmd_slot = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_P0CLB & 0xfff));
    pp->rx_fis = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_P0FB & 0xfff));
    pp->cmd_tbl = (void *)(SATA_SDMAP_RIU_BASE + (AHCI_CTBA0 & 0xfff));

    pp->cmd_slot_dma = AHCI_P0CLB;
    pp->rx_fis_dma = AHCI_P0FB;
    pp->cmd_tbl_dma = AHCI_CTBA0;

#elif (SATA_CMD_TYPE == TYPE_DRAM)
    sata_info("SATA cmd type: DRAM\n");
    dma_sz = 0x10000;
    mem = dmam_alloc_coherent(dev, dma_sz, &mem_dma, GFP_KERNEL);
    if (!mem)
        return -ENOMEM;
    memset(mem, 0, dma_sz);

    pp->cmd_slot = mem;
    // Translate physical address to bus address since SATA engine uses bus address.
    mem_dma = (dma_addr_t)MHal_SATA_bus_address(mem_dma);
    pp->cmd_slot_dma = mem_dma;

    mem += SATA_CMD_HEADER_SIZE;
    mem_dma += SATA_CMD_HEADER_SIZE;

    pp->rx_fis = mem;
    pp->rx_fis_dma = mem_dma;

    mem += SATA_FIS_SIZE;
    mem_dma += SATA_FIS_SIZE;

    pp->cmd_tbl = mem;
    pp->cmd_tbl_dma = mem_dma;

#elif (SATA_CMD_TYPE == TYPE_RIU)
    sata_info("SATA cmd type: RIU\n");
    pp->cmd_slot = (void *)(AHCI_P0CLB);
    pp->rx_fis = (void *)(AHCI_P0FB);
    pp->cmd_tbl = (void *)(AHCI_CTBA0);

    pp->cmd_slot_dma = AHCI_P0CLB;
    pp->rx_fis_dma = AHCI_P0FB;
    pp->cmd_tbl_dma = AHCI_CTBA0;
#else
#error "SATA_CMD_TYPE Unknown"
#endif

    sata_info("cmd_slot = 0x%x ; cmd_slot_dma = 0x%x\n", (u32)pp->cmd_slot, (u32)pp->cmd_slot_dma);
    sata_info("rx_fis = 0x%x ; rx_fis_dma = 0x%x\n", (u32)pp->rx_fis, (u32)pp->rx_fis_dma);
    sata_info("cmd_tbl = 0x%x ; cmd_tbl_dma = 0x%x\n", (u32)pp->cmd_tbl, (u32)pp->cmd_tbl_dma);

    //sata_info("port_base= 0x%x ;\n", port_base);

    ap->private_data = pp;
//    ap->ss_private_data = host_priv;
    temp = readl((void*)SS_PORT_CMD + port_base) & ~PORT_CMD_ICC_MASK;

    // spin up device
    temp |= PORT_CMD_SPIN_UP;
    writel(temp, (void*)SS_PORT_CMD + port_base);

    // wake up link
    writel((temp | PORT_CMD_ICC_ACTIVE), (void*)SS_PORT_CMD + port_base);

    // start FIS RX
    sstar_ahci_start_fis_rx(ap);

    // Clear IS , Interrupt Status
    writel(0xFFFFFFFF, (void*)SS_PORT_IRQ_STAT + port_base);
    writel(0xFFFFFFFF, (void*)SS_PORT_SCR_ERR + port_base);

    // set to speed limit with gen 1, gen 2 or auto
    temp = readl((void*)SS_PORT_SCR_CTL + port_base);
    temp = temp & (~E_PORT_SPEED_MASK); // clear speed
    temp = temp | MHal_SATA_get_max_speed();
    writel(temp,(void*) SS_PORT_SCR_CTL + port_base);

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    }
#ifdef CONFIG_ARCH_INFINITY2
    else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00
    }
#endif
    // test: enable PHY
    writew(0x9a8f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));  // [0]: Reset
    writew(0x9a8e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x2200, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));  // [9]: Enable host

    printk("sstar_ahci_start_engine\n");
    // MHal_SATA_HW_Inital(misc_base, port_base, hba_base);

    // Start DMA Engine
    sstar_ahci_start_engine(port_base);

    return 0;
}

static void sata_sstar_port_stop(struct ata_port *ap)
{
    phys_addr_t port_base = ss_hpriv->port_base;
    int ret;

    sata_info("[%s]\n", __func__);
    // Stop DMA Engine
    ret = sstar_ahci_stop_engine(port_base);
    if (ret)
    {
        sata_error("[%s][Error] Fail to Stop SATA Port\n", __func__);
    }

    // Disable FIS reception
    ret = sstar_ahci_stop_fis_rx(ap);
    if (ret)
    {
        sata_error("[%s][Error] Fail to Stop FIS RX\n", __func__);
    }
}

#if 0
irqreturn_t sata_sstar_interrupt(int irq, void *dev_instance)
{
    struct ata_host *host = dev_instance;
    struct ata_port *ap = host->ports[0];
    struct sata_sstar_host_priv *host_priv = host->private_data;
    struct ata_link *link = NULL;
    struct ata_eh_info *active_ehi;
    struct ata_eh_info *host_ehi = &ap->link.eh_info;
    struct ata_queued_cmd *active_qc;
    u32 serror = 0;


    phys_addr_t port_base = host_priv->port_base;
    phys_addr_t hba_base = host_priv->hba_base;
    u32 host_status = 0;
    u32 port_status = 0;
    u32 qc_active;
    //u32 port_ie;
    u32 port_err;
    u32 clr_port_err = 0;

#if 0
    link = &ap->link;
    ehi = &link->eh_info;
    ata_ehi_clear_desc(ehi);
#endif

    host_status = sstar_sata_reg_read(HOST_IRQ_STAT + hba_base);
    if (host_status)
        printk("host_status = 0x%x\n", host_status);
    else
        return IRQ_NONE;

    spin_lock(&host->lock);
    port_status = sstar_sata_reg_read(PORT_IRQ_STAT + port_base);

    sstar_sata_reg_write(port_status, PORT_IRQ_STAT + port_base);
    if (port_status)
        printk("port_status = 0x%x\n", port_status);

    if (port_status & (1 << 6)) // PORT_IRQ_CONNECT
    {
        printk(KERN_WARNING "HDD inserted (Device presence change, clear diag.X)\n");
        sstar_sata_reg_write((1 << 26), PORT_SCR_ERR + port_base);
    }
    if (port_status & (1 << 22)) // PORT_IRQ_PHYRDY
    {
        printk(KERN_WARNING "HDD removed (PhyRdy change, clear diag.N)\n");
        sstar_sata_reg_write((1 << 16), PORT_SCR_ERR + port_base);
        //ata_ehi_hotplugged(ehi);
        //ata_ehi_push_desc(ehi, "%s", "PHY RDY changed");
    }

    if (unlikely(port_status & PORT_IRQ_ERROR))
    {
        link = &ap->link;

        active_qc = ata_qc_from_tag(ap, link->active_tag);
        active_ehi = &link->eh_info;

        ata_ehi_clear_desc(host_ehi);
        ata_ehi_push_desc(host_ehi, "irq_status 0x%08x", port_status);

        serror = sstar_sata_reg_read(PORT_SCR_ERR + port_base);
        sstar_sata_reg_write(serror, PORT_SCR_ERR + port_base);
        host_ehi->serror |= serror;

        if (port_status & PORT_IRQ_TF_ERR)
        {
            printk("IRQ TF ERROR\n");
            if (active_qc)
            {
                printk("active_qc on\n");
                active_qc->err_mask |= AC_ERR_DEV;
            }
            else
            {
                printk("active link on\n");
                active_ehi->err_mask |= AC_ERR_DEV;
            }
        }
        if (port_status & PORT_IRQ_FREEZE)
        {
            printk("ata_port_freeze now\n");
            ata_port_freeze(ap);
        }
        else
        {
            printk("ata_port_abort now\n");
            ata_port_abort(ap);
        }
        goto irq_out;
    }

    port_err = sstar_sata_reg_read(PORT_SCR_ERR + port_base);
    if (port_err)
        printk("port_err = 0x%x\n", port_err);
    if (port_err & (1 << 1))
    {
        printk(KERN_WARNING "SATA recovered from communication error\n");
        clr_port_err |= (1 << 1);
    }
    if (port_err & (1 << 9))
    {
        printk(KERN_WARNING "SATA Persistent comm error/data integrity error\n");
        clr_port_err |= (1 << 9);
    }
    if (clr_port_err)
        sstar_sata_reg_write(clr_port_err, PORT_SCR_ERR + port_base);

#if 0
    if (port_status)
    {
        printk("port status = 0x%x\n", port_status);
    }
#endif

    // clear interrupt
    //sstar_sata_reg_write(port_status, PORT_IRQ_STAT + port_base);
irq_out:
    qc_active = sstar_sata_reg_read(PORT_SCR_ACT + port_base);
    qc_active |= sstar_sata_reg_read(PORT_CMD_ISSUE + port_base);

    ata_qc_complete_multiple(ap, qc_active);
    sstar_sata_reg_write(host_status, HOST_IRQ_STAT + hba_base);
    spin_unlock(&host->lock);

    return IRQ_RETVAL(1);
}
#endif



#if (SW_OOB_MODE == 1)
static void sstar_sata_sw_oob_mode1(void)
{
    u16 u16Temp, u16IrqRetry, u16Irq2Retry=2;

    // Enable all SATA interrupt
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (0x15273A << 1));

SW_OOB_MODE1_STAGE0:
    // Clear all SATA interrupt
    writew(0xFFFF, (volatile void *)SSTAR_RIU_BASE + (0x15273C << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (0x15273C << 1));

    // Force reg_sata_rxpll_pd_cdr = 1
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));
    u16Temp |= 0x400; // [10]: reg_sata_rxpll_pd_cdr
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));

    udelay(22);

    for (u16IrqRetry = 0; u16IrqRetry < 5; u16IrqRetry++)
    {
        // Check if IRQ1 is yes
        if (readw((volatile void *)SSTAR_RIU_BASE + (0x15273E << 1)) & INT_SATA_PHY_RXPLL_FREQ_LOCK_FLAG)
            break;

        udelay(2);
    }

    // Force reg_sata_rxpll_pd_cdr = 0
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));
    u16Temp &= ~(0x400); // [10]: reg_sata_rxpll_pd_cdr
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));

    udelay(3);

    u16Irq2Retry--;
    // Check if IRQ2 no
    if ((readw((volatile void *)SSTAR_RIU_BASE + (0x15273E << 1)) & INT_SATA_PHY_RXPLL_FREQ_UNLOCK_FLAG) || u16Irq2Retry == 0)
        goto SW_OOB_MODE1_STAGE0;

    udelay(2);

    for (u16IrqRetry = 0; u16IrqRetry < 2; u16IrqRetry++)
    {
        // Check if IRQ3 is yes
        if (readw((volatile void *)SSTAR_RIU_BASE + (0x15273E << 1)) & INT_SATA_PHY_RX_DATA_VLD_PRE_0)
            break;

        udelay(1);
    }

    // Data Ready

    // Disable all SATA interrupt
    writew(0xFFFF, (volatile void *)SSTAR_RIU_BASE + (0x15273A << 1));
}
#elif (SW_OOB_MODE == 2)
static void sstar_sata_sw_oob_mode2(void)
{
    u16 u16Temp, u16Irq3Retry=2;

    //Enable all SATA interrupt
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (0x15273A << 1));

SW_OOB_MODE2_STAGE0:
    // Clear all SATA interrupt
    writew(0xFFFF, (volatile void *)SSTAR_RIU_BASE + (0x15273C << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (0x15273C << 1));

    // Force reg_sata_rxpll_pd_cdr = 1
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));
    u16Temp |= 0x400; // [10]: reg_sata_rxpll_pd_cdr
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));

    udelay(22);

    // Force reg_sata_rxpll_pd_cdr = 0
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));
    u16Temp &= ~(0x400); // [10]: reg_sata_rxpll_pd_cdr
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (0x152762 << 1));

    udelay(3);

    u16Irq3Retry--;
    // Check if IRQ3 is yes
    if ((readw((volatile void *)SSTAR_RIU_BASE + (0x15273E << 1)) & INT_SATA_PHY_RX_DATA_VLD_PRE_0) || u16Irq3Retry == 0)
        break;

    // Data Ready

    // Disable all SATA interrupt
    writew(0xFFFF, (volatile void *)SSTAR_RIU_BASE + (0x15273A << 1));
}
#endif

static int sstar_sata_hardware_init(phys_addr_t hba_base, phys_addr_t port_base, phys_addr_t misc_base)
{
    u32 i;
    u32 u32Temp = 0;

    MHal_SATA_HW_Inital(misc_base, port_base, hba_base);

#ifdef CONFIG_ARCH_INFINITY2
    writew(HOST_RESET, (volatile void *)hba_base + (HOST_CTL));
#else
    writel(HOST_RESET, (volatile void *)hba_base + (HOST_CTL));
#endif

    #if (SW_OOB_MODE == 1)
    sstar_sata_sw_oob_mode1();
    #elif (SW_OOB_MODE == 2)
    sstar_sata_sw_oob_mode2();
    #else
    u32Temp = sstar_sata_wait_reg(SS_HOST_CTL + hba_base, HOST_RESET, HOST_RESET, 1, 500);

    if (u32Temp & HOST_RESET)
        return -1;
    #endif
    // Turn on AHCI_EN
    u32Temp = readl((void*)SS_HOST_CTL + (phys_addr_t) hba_base);
    if (u32Temp & HOST_AHCI_EN)
    {
        MHal_SATA_Setup_Port_Implement((phys_addr_t)misc_base, (phys_addr_t)port_base, (phys_addr_t)hba_base);
        return 0;
    }

    // Try AHCI_EN Trurn on for a few time
    for (i = 0; i < 5; i++)
    {
        u32Temp |= HOST_AHCI_EN;
        writel(u32Temp, (void*)SS_HOST_CTL + (phys_addr_t)hba_base);
        u32Temp = readl((void*)SS_HOST_CTL + (phys_addr_t)hba_base);
        if (u32Temp & HOST_AHCI_EN)
            break;
        msleep(10);
    }

    MHal_SATA_Setup_Port_Implement((phys_addr_t)misc_base, (phys_addr_t) port_base, (phys_addr_t) hba_base);

    //printk("sstar_sata_hardware_init done !!!!!!!!!\n");
    return 0;
}

static struct scsi_host_template sstar_sata_sht =
{
#if defined(USE_NCQ)
    ATA_NCQ_SHT("sstar_sata"),
#else
    ATA_BASE_SHT("sstar_sata"),
#endif
    .can_queue = SATA_SSTAR_QUEUE_DEPTH,
    .sg_tablesize = SATA_KA9_USED_PRD,
    .dma_boundary = ATA_DMA_BOUNDARY,
};

static struct ata_port_operations sstar_sata_ops =
{
    .inherits       = &sata_pmp_port_ops,

    .qc_defer = ata_std_qc_defer,
    .qc_prep = sata_sstar_qc_prep,
    .qc_issue = sata_sstar_qc_issue,
    .qc_fill_rtf = sata_sstar_qc_fill_rtf,

    .scr_read = sata_sstar_scr_read,
    .scr_write = sata_sstar_scr_write,

    .freeze = sata_sstar_freeze,
    .thaw = sata_sstar_thaw,

    .softreset = sata_sstar_softreset,
    .hardreset = sata_sstar_hardreset,

    .pmp_softreset = sata_sstar_softreset,
    .error_handler = sata_sstar_error_handler,
    .post_internal_cmd = sata_sstar_post_internal_cmd,

    .port_start = sata_sstar_port_start,
    .port_stop = sata_sstar_port_stop,

};

static const struct ata_port_info sstar_sata_port_info[] =
{
    {
        .flags = SATA_SSTAR_HOST_FLAGS,
        .pio_mask = ATA_PIO6,
        .udma_mask = ATA_UDMA6,
        .port_ops = &sstar_sata_ops,
    },
};

int __ss_sata_get_irq_number_host(void)
{
    struct device_node *dev_node;
    struct platform_device *pdev;
    int irq = 0;

    dev_node = of_find_compatible_node(NULL, NULL, SSTAR_SATA_DTS_NAME);

    if (!dev_node)
        return -ENODEV;

    pdev = of_find_device_by_node(dev_node);
    if (!pdev)
    {
        of_node_put(dev_node);
        return -ENODEV;
    }
    irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    printk("[SATA] Virtual IRQ: %d\n", irq);
    return irq;
}

static int sstar_sata_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct ata_port_info pi = sstar_sata_port_info[0];
    const struct ata_port_info *ppi[] = { &pi, NULL };
    struct ahci_host_priv *hpriv;
    struct ata_host *host;
    int irq = 0;
    int ret = 0;

#if 0 // defined(CONFIG_OF)
    struct device_node *np = dev->of_node;
    int hba_base;
    int port_base;
    int misc_base;
#else
    struct resource *port_mem;
    struct resource *misc_mem;
    struct resource *hba_mem;
#endif

    //sata_info("sstar SATA Host Controller Probing...\n");
    sata_info("SATA %d probing...\n", pdev->id);

#if 0 // defined(CONFIG_OF)
    ret = of_property_read_u32(np, "sstar,hba-base", &hba_base);
    if (ret)
    {
        printk("[%s][Error] SATA Get HBA Resource Fail\n", __func__);
        return -EINVAL;
    }
    printk("dtb hba_base = %x\n", hba_base);

    ret = of_property_read_u32(np, "sstar,port-base", &port_base);
    if (ret)
    {
        printk("[%s][Error] SATA Get Port Resource Fail\n", __func__);
        return -EINVAL;
    }
    printk("dtb port_base= %x\n", port_base);

    ret = of_property_read_u32(np, "sstar,misc-base", &misc_base);
    if (ret)
    {
        printk("[%s][Error] SATA Get MISC Resource Fail\n", __func__);
        return -EINVAL;
    }
    printk("dtb misc_base= %x\n", misc_base);

    ret = of_property_read_u32(np, "interrupts", &irq);
    if ((ret) && (irq <= 0))
    {
        printk("[%s][Error] SATA Get IRQ Fail\n", __func__);
        return -EINVAL;
    }
#else
    hba_mem  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    port_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    misc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);

    sata_debug("HBA = 0x%08x, PORT = 0x%08x, MISC = 0x%08x\n", hba_mem->start, port_mem->start, misc_mem->start);
    if (!hba_mem)
    {
        sata_error("[%s]SATA Get HBA Resource Fail\n", __func__);
        return -EINVAL;
    }
    if (!port_mem)
    {
        sata_error("[%s]SATA Get Port Resource Fail\n", __func__);
        return -EINVAL;
    }
    if (!misc_mem)
    {
        sata_error("[%s]SATA Get MISC Resource Fail\n", __func__);
        return -EINVAL;
    }

#if 0
    irq = platform_get_irq(pdev, 0);
#else
    irq =  __ss_sata_get_irq_number_host();
#endif
    if (irq <= 0)
    {
        sata_error("[%s]SATA Get IRQ Fail\n", __func__);
        return -EINVAL;
    }
#endif

    // Allocate Host Private Data
    hpriv = devm_kzalloc(dev, sizeof(struct ahci_host_priv), GFP_KERNEL);
    if (!hpriv)
    {
        sata_error("[%s] SATA Allocate Host Private Data Fail\n", __func__);
        return -ENOMEM;
    }
    ss_hpriv = devm_kzalloc(dev, sizeof(struct sata_sstar_host_priv), GFP_KERNEL);
    if (!ss_hpriv)
    {
        sata_error("[%s]SATA Allocate SS Host Private Data Fail\n", __func__);
        return -ENOMEM;
    }

    //    printk("SSTAR_RIU_BASE = 0x%llx..\n", SSTAR_RIU_BASE);  // sstar_pm_base
    // FIXME: need a way to set DMA mask in DTB, otherwise we will get dma allocation fail.
#if 0 //defined(CONFIG_OF)
    printk("CONFIG_OF\n");
    printk("SSTAR_RIU_BASE = 0x%llx..\n", SSTAR_RIU_BASE);   // sstar_pm_base
    hpriv->hba_base  = (SSTAR_RIU_BASE + (hba_base << 1));   // (mstar_pm_base + (hba_base << 1));
    hpriv->port_base = (SSTAR_RIU_BASE + (port_base << 1));  // (mstar_pm_base + (port_base << 1));
    hpriv->misc_base = (SSTAR_RIU_BASE + (misc_base << 1));  // (mstar_pm_base + (misc_base << 1));
#else
    ss_hpriv->hba_base  = hba_mem->start;
    ss_hpriv->port_base = port_mem->start;
    ss_hpriv->misc_base = misc_mem->start;
#endif

    sata_debug("IRQ = %d\n", irq);

#if defined(USE_NCQ)
    if(ppi[0]->flags & ATA_FLAG_NCQ)
        sata_info("== NCQ enable ==\n");
#endif
#ifdef CONFIG_SATA_PMP
    if(ppi[0]->flags & ATA_FLAG_PMP)
        sata_info("== PMP support ==\n");
#endif

    // Initial SATA Hardware
    if (sstar_sata_hardware_init(hba_mem->start, port_mem->start, misc_mem->start))
    {
        sata_error("[%s]SATA Hardware Initial Failed\n", __func__);
        return -EINVAL;
    }

    host = ata_host_alloc_pinfo(dev, ppi, SATA_PORT_NUM);
    if (!host)
    {
        ret = -ENOMEM;
        sata_error("[%s]SATA Allocate ATA Host Fail\n", __func__);
        goto out_devm_kzalloc_hpriv;
    }

    hpriv->flags |= (unsigned long)pi.private_data;
    hpriv->mmio = (void __iomem *) hba_mem->start;
    hpriv->irq = irq;

//#ifdef CONFIG_ARCH_INFINITY2
#if 0
    ahci_save_initial_config(dev, hpriv, 0, 0);
#else
    ahci_save_initial_config(dev, hpriv);
#endif

    /* DH test */
    if (hpriv->cap & HOST_CAP_NCQ)
        pi.flags |= ATA_FLAG_NCQ;
    //    if (!ncq_en)
    //        pi.flags &= ~ATA_FLAG_NCQ;
    if (hpriv->cap & HOST_CAP_PMP)
        pi.flags |= ATA_FLAG_PMP;

    host->private_data = hpriv;
    host->ports[0]->private_data = ss_hpriv;

    //sata_info("host->ports[0] addr = [0x%x]\n", (u32)host->ports[0]);

    //return ata_host_activate(host, irq, sata_sstar_interrupt, IRQF_SHARED, &sstar_sata_sht);
    //return ata_host_activate(host, irq, ahci_interrupt, IRQF_SHARED, &sstar_sata_sht);
#ifdef CONFIG_ARCH_INFINITY2
    return ata_host_activate(host, irq, sstar_ahci_single_level_irq_intr, IRQF_SHARED, &sstar_sata_sht);
#else
    return ahci_host_activate(host, &sstar_sata_sht);
#endif

out_devm_kzalloc_hpriv:
    devm_kfree(dev, hpriv);
    devm_kfree(dev, ss_hpriv);

    return ret;
}

static int sstar_sata_remove(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct ata_host *host = dev_get_drvdata(dev);
    struct ahci_host_priv *host_priv = host->private_data;
//    struct sata_sstar_host_priv *ss_hpriv = host->ports[0]->ss_private_data;

    ata_host_detach(host);

    devm_kfree(dev, host_priv);
    devm_kfree(dev, ss_hpriv);

    sata_info("[%s]\n", __func__);

    return 0;
}

#ifdef CONFIG_PM
static int sstar_sata_suspend(struct platform_device *pdev, pm_message_t state)
{
    //struct ata_host *host = dev_get_drvdata(&pdev->dev);

    sata_info("[%s]\n", __func__);
    //return ata_host_suspend(host, state);
    return 0;
}

static int sstar_sata_resume(struct platform_device *pdev)
{
    struct ata_host *host = dev_get_drvdata(&pdev->dev);
    //struct sata_sstar_host_priv *hpriv = host->private_data;
    struct resource *port_mem;
    struct resource *misc_mem;
    struct resource *hba_mem;
    sata_info("[%s]\n", __func__);

    hba_mem  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    port_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    misc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);

    if (sstar_sata_hardware_init(hba_mem->start, port_mem->start, misc_mem->start)/*sstar_sata_hardware_init()*/)
    {
        sata_error("[%s][Error] SATA Hardware Initial Failed, SATA resume failed!\n", __func__);
        return -EINVAL;
    }
    //sata_sstar_port_start(struct ata_port *ap)
    sata_sstar_port_start(host->ports[0]);

    //ata_host_resume(host);
    sata_info("[%s] done\n", __func__);
    return 0;
}
#endif

static void sstar_sata_drv_platfrom_release(struct device *device)
{
    phys_addr_t port_base = SATA_GHC_0_P0_ADDRESS_START;
    phys_addr_t misc_base = SATA_MISC_0_ADDRESS_START;

    sata_info("[%s]\n", __func__);
    MHal_SATA_Clock_Config(misc_base, port_base, FALSE);
}

#if defined(CONFIG_ARM64)
#if 0 // defined(CONFIG_OF)
static const struct of_device_id sstar_satahost_dt_match[] =
{
    { .compatible = "sstar-sata", },
    {}
};
MODULE_DEVICE_TABLE(of, sstar_satahost_dt_match);
#endif
#endif

static struct platform_driver sstar_sata_driver =
{
    .probe          = sstar_sata_probe,
    .remove         = sstar_sata_remove,
#ifdef CONFIG_PM
    .suspend        = sstar_sata_suspend,
    .resume         = sstar_sata_resume,
#endif
    .driver         = {
        .name       = "sstar-sata",
#if defined(CONFIG_ARM64)
#if 0 // defined(CONFIG_OF)
        .of_match_table = sstar_satahost_dt_match,
#endif
#endif
        .owner       = THIS_MODULE,
    }
};

#if defined(CONFIG_ARM64)
    static u64 sata_dmamask = DMA_BIT_MASK(64);
#else
    static u64 sata_dmamask = DMA_BIT_MASK(32);
#endif

static struct resource satahost_resources[] =
{
    [0] = {
        .start = SATA_GHC_0_ADDRESS_START,
        .end   = SATA_GHC_0_ADDRESS_END,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = SATA_GHC_0_P0_ADDRESS_START,
        .end   = SATA_GHC_0_P0_ADDRESS_END,
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = SATA_MISC_0_ADDRESS_START,
        .end   = SATA_MISC_0_ADDRESS_END,
        .flags = IORESOURCE_MEM,
    },
#if  defined(CONFIG_ARCH_INFINITY2)
    [3] = {
        .start = INT_IRQ_15_SATA_INTRQ,  //15 + 32, //E_IRQ_SATA_INT,
        .end   = INT_IRQ_15_SATA_INTRQ,  //15 + 32, //E_IRQ_SATA_INT,
        .flags = IORESOURCE_IRQ,
    },
#elif defined(CONFIG_ARCH_INFINITY2M)
    [3] = {
        .start = INT_IRQ_SATA,
        .end   = INT_IRQ_SATA,
        .flags = IORESOURCE_IRQ,
    },
#endif
};

struct platform_device sstar_satahost_device =
{
    .name = "sstar-sata",
    .id = 0,
    .dev = {
        .release = sstar_sata_drv_platfrom_release,
        .dma_mask = &sata_dmamask,
#if defined(CONFIG_ARM64)
        .coherent_dma_mask = DMA_BIT_MASK(64),
#else
        .coherent_dma_mask = DMA_BIT_MASK(32),
#endif
    },
    .num_resources = ARRAY_SIZE(satahost_resources),
    .resource = satahost_resources,
};

static int __init sstar_sata_drv_init(void)
{
    int ret = 0;

    //sata_info("===== [SATA 0] register platform driver START =====\n");
    ret = platform_driver_register(&sstar_sata_driver);
    if (ret < 0)
    {
        sata_error("Unable to register SATA 0 platform driver, %d\n", ret);
        return ret;
    }

    ret = platform_device_register(&sstar_satahost_device);
    if (ret < 0)
    {
        sata_error("Unable to register SATA 0 platform device, %d\n", ret);
        platform_driver_unregister(&sstar_sata_driver);
        return -ret;
    }
    sata_info("===== [SATA 0] register platform device END =====\n");

    return ret;
}

static void __exit sstar_sata_drv_exit(void)
{
    sata_info("[%s]\n", __func__);

    platform_device_unregister(&sstar_satahost_device);
    platform_driver_unregister(&sstar_sata_driver);
}

MODULE_AUTHOR("sstar Semiconductor");
MODULE_DESCRIPTION("sstar 3.0Gbps SATA controller low level driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00");

module_init(sstar_sata_drv_init);
module_exit(sstar_sata_drv_exit);
