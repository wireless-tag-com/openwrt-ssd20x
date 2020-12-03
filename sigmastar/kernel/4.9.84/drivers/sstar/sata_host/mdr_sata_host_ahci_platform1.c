/*
* mdr_sata_host_ahci_platform1.c- Sigmastar
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
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include <linux/acpi.h>
#include <linux/pci_ids.h>
#include "ahci.h"

//#include "mhal_sata_common.c"
#include "mhal_sata_host_ahci.h"
//#include "mhal_sata_host_ahci.c"


#if defined( CONFIG_ARCH_INFINITY2M)
#include "mhal_sata_host.h"
#include "mdrv_sata_host_ahci.h"
#endif


#define DRV_NAME1 "ahci_sstar1"
#define SW_OOB_MODE 0

static int n_ports = 1;//config sata ports //TBD


//#ifdef CONFIG_ARCH_INFINITY2
#if 1
    #undef writel
    #undef readl

    extern u32 ahci_reg_read(void __iomem * p_reg_addr);
    extern void ahci_reg_write(u32 data, void __iomem * p_reg_addr);
    //extern u32 ahci_reg_read(phys_addr_t reg_addr);
    //extern void ahci_reg_write(u32 data, phys_addr_t p_reg_addr);;

    #define writel ahci_reg_write
    #define readl ahci_reg_read
#endif

//extern void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base);


struct sstar_ahci_priv {
	struct device *dev;
	void __iomem *res_ahci;
	void __iomem *res_ahci_port0;
	void __iomem *res_ahci_misc;
	u32 port_mask;
};


#define SSTAR_AHCI_SHT(drv_name)				\
	ATA_NCQ_SHT(drv_name),					\
	.shost_attrs		= ahci_shost_attrs,			\
	.sdev_attrs		= ahci_sdev_attrs


#define   SATA_SSTAR_HOST_FLAGS   (ATA_FLAG_SATA | ATA_FLAG_PIO_DMA | ATA_FLAG_ACPI_SATA | ATA_FLAG_AN | ATA_FLAG_NCQ )

extern void Chip_Flush_Memory(void);

//static int ahci_port_max_speed = E_PORT_SPEED_GEN3;



phys_addr_t sstar_ahci_sata_bus_address1(phys_addr_t phy_address)
{
    if (phy_address >= MIU_INTERVAL_SATA)
    {
        return phy_address + 0x20000000;
    }
    else
    {
        return phy_address - 0x20000000;
    }
}


static unsigned int ahci_fill_sg1(struct ata_queued_cmd *qc, void *cmd_tbl)
{
	struct scatterlist *sg;
	struct ahci_sg *ahci_sg = cmd_tbl + AHCI_CMD_TBL_HDR_SZ;
	unsigned int si;

	VPRINTK("ENTER\n");

	/*
	 * Next, the S/G list.
	 */
	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		dma_addr_t addr = sg_dma_address(sg);
		u32 sg_len = sg_dma_len(sg);
#ifdef CONFIG_SS_SATA_AHCI_PLATFORM_HOST
        ahci_sg[si].addr = (u32)(sstar_ahci_sata_bus_address1(cpu_to_le32(addr & 0xffffffff)));
        ahci_sg[si].addr_hi = (u32)(sstar_ahci_sata_bus_address1(cpu_to_le32((addr >> 16) >> 16)));
#else
		ahci_sg[si].addr = cpu_to_le32(addr & 0xffffffff);
		ahci_sg[si].addr_hi = cpu_to_le32((addr >> 16) >> 16);
#endif
		ahci_sg[si].flags_size = cpu_to_le32(sg_len - 1);
	}

	return si;
}

#if 0
static int ahci_pmp_qc_defer(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ahci_port_priv *pp = ap->private_data;

	if (!sata_pmp_attached(ap) || pp->fbs_enabled)
		return ata_std_qc_defer(qc);
	else
		return sata_pmp_qc_defer_cmd_switch(qc);
}
#endif

static void sstar_ahci_qc_prep1(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ahci_port_priv *pp = ap->private_data;
	int is_atapi = ata_is_atapi(qc->tf.protocol);
	void *cmd_tbl;
	u32 opts;
	const u32 cmd_fis_len = 5; /* five dwords */
	unsigned int n_elem;

	/*
	 * Fill in command table information.  First, the header,
	 * a SATA Register - Host to Device command FIS.
	 */
	cmd_tbl = pp->cmd_tbl + qc->tag * AHCI_CMD_TBL_SZ;

	ata_tf_to_fis(&qc->tf, qc->dev->link->pmp, 1, cmd_tbl);
	if (is_atapi) {
		memset(cmd_tbl + AHCI_CMD_TBL_CDB, 0, 32);
		memcpy(cmd_tbl + AHCI_CMD_TBL_CDB, qc->cdb, qc->dev->cdb_len);
	}

	n_elem = 0;
	if (qc->flags & ATA_QCFLAG_DMAMAP)
		n_elem = ahci_fill_sg1(qc, cmd_tbl);

	/*
	 * Fill in command slot information.
	 */
	opts = cmd_fis_len | n_elem << 16 | (qc->dev->link->pmp << 12);
	if (qc->tf.flags & ATA_TFLAG_WRITE)
		opts |= AHCI_CMD_WRITE;
	if (is_atapi)
		opts |= AHCI_CMD_ATAPI | AHCI_CMD_PREFETCH;

	ahci_fill_cmd_slot(pp, qc->tag, opts);
}

static void sstar_ahci_host_stop1(struct ata_host *host)
{
#ifdef CONFIG_SS_SATA_AHCI_PLATFORM_HOST
    struct device *dev = host->dev;
    struct ahci_platform_data *pdata = dev_get_platdata(dev);
#endif
	struct ahci_host_priv *hpriv = host->private_data;

#ifdef CONFIG_SS_SATA_AHCI_PLATFORM_HOST
    if (pdata && pdata->exit)
        pdata->exit(dev);
#endif

	ahci_platform_disable_resources(hpriv);
}

static int sstar_ahci_port_start1(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct device *dev = ap->host->dev;
	struct ahci_port_priv *pp;
	void *mem;
	dma_addr_t mem_dma;
	size_t dma_sz, rx_fis_sz;

	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	if (ap->host->n_ports > 1) {
		pp->irq_desc = devm_kzalloc(dev, 8, GFP_KERNEL);
		if (!pp->irq_desc) {
			devm_kfree(dev, pp);
			return -ENOMEM;
		}
		snprintf(pp->irq_desc, 8,
			 "%s%d", dev_driver_string(dev), ap->port_no);
	}

	/* check FBS capability */
	if ((hpriv->cap & HOST_CAP_FBS) && sata_pmp_supported(ap)) {
		void __iomem *port_mmio = ahci_port_base(ap);
		u32 cmd = readl(port_mmio + PORT_CMD);
		if (cmd & PORT_CMD_FBSCP)
			pp->fbs_supported = true;
		else if (hpriv->flags & AHCI_HFLAG_YES_FBS) {
			dev_info(dev, "port %d can do FBS, forcing FBSCP\n",
				 ap->port_no);
			pp->fbs_supported = true;
		} else
			dev_warn(dev, "port %d is not capable of FBS\n",
				 ap->port_no);
	}

	if (pp->fbs_supported) {
		dma_sz = AHCI_PORT_PRIV_FBS_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ * 16;
	} else {
		dma_sz = AHCI_PORT_PRIV_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ;
	}

	mem = dmam_alloc_coherent(dev, dma_sz, &mem_dma, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	memset(mem, 0, dma_sz);

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot = mem;
#ifdef CONFIG_SS_SATA_AHCI_PLATFORM_HOST
	// Translate physical address to bus address since SATA engine uses bus address.
	mem_dma = (dma_addr_t)sstar_ahci_sata_bus_address1(mem_dma);
#endif
	pp->cmd_slot_dma = mem_dma;

	mem += AHCI_CMD_SLOT_SZ;
	mem_dma += AHCI_CMD_SLOT_SZ;

	/*
	 * Second item: Received-FIS area
	 */
	pp->rx_fis = mem;
	pp->rx_fis_dma = mem_dma;

	mem += rx_fis_sz;
	mem_dma += rx_fis_sz;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = mem;
	pp->cmd_tbl_dma = mem_dma;

	/*
	 * Save off initial list of interrupts to be enabled.
	 * This could be changed later
	 */
	pp->intr_mask = DEF_PORT_IRQ;

	/*
	 * Switch to per-port locking in case each port has its own MSI vector.
	 */
	if (hpriv->flags & AHCI_HFLAG_MULTI_MSI) {
		spin_lock_init(&pp->lock);
		ap->lock = &pp->lock;
	}

	ap->private_data = pp;

	/* engage engines, captain */
	return ahci_port_resume(ap);
}


struct ata_port_operations sstar_ahci_platform_ops1 = {
	.inherits	= &ahci_ops,
	.qc_prep		= sstar_ahci_qc_prep1,
//	.thaw			= sstar_ahci_thaw,
	//.softreset	= sstar_ahci_softreset,
	//.pmp_softreset	= sstar_ahci_softreset,
	.host_stop     = sstar_ahci_host_stop1,
#ifdef CONFIG_PM
//	.port_suspend		= ahci_port_suspend,  this no need
//	.port_resume		= sstar_ahci_port_resume,
#endif
	.port_start    = sstar_ahci_port_start1,
};


static const struct ata_port_info ahci_port_info1 =
{
	.flags		= SATA_SSTAR_HOST_FLAGS,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &sstar_ahci_platform_ops1,
};


static struct scsi_host_template ahci_platform_sht1 = {
    SSTAR_AHCI_SHT(DRV_NAME1),
    .can_queue = 31,//SATA_SSTAR_QUEUE_DEPTH,
    .sg_tablesize = 24, //SATA_SSTAR_USED_PRD,
    .dma_boundary = 0xffffUL, //ATA_DMA_BOUNDARY,
};



static u32 sstar_sata_wait_reg1(phys_addr_t reg_addr, u32 mask, u32 val, unsigned long interval, unsigned long timeout)
{
    u32 temp;
    unsigned long timeout_vale = 0;

    temp = readl((void *)reg_addr);

    while((temp & mask) == val)
    {
        msleep(interval);
        timeout_vale += interval;
        if (timeout_vale > timeout)
            break;
        temp = readl((void *)reg_addr);
    }
    return temp;
}

//int ss_sata_init(void __iomem *mmio)
//int ss_sata_init(struct device *dev, void __iomem *mmio, int id)
static int ss_sata_init1(void __iomem *mmio)
{
    u32 i;
    u32 u32Temp = 0;

#if  defined(CONFIG_ARCH_INFINITY2)
    phys_addr_t hba_base = (phys_addr_t)mmio; //102B00<<1
    phys_addr_t port_base = (phys_addr_t)(mmio + 0x200); //102C00<<1
    phys_addr_t misc_base = (phys_addr_t)(mmio + 0x400); //102D00<<1
    int port_num;
    int phy_mode =2;

    port_num = n_ports;

    pr_info("[%s] , hba_base =0x%X \n", __func__ , hba_base );
    pr_info("[%s] , port_base =0x%X \n", __func__ , port_base );
    pr_info("[%s] , misc_base =0x%X \n", __func__ , misc_base );

    //printk("sstar sata HW settings start!!!\n");
    ss_sata_misc_init(mmio, phy_mode, port_num);
    ss_sata_phy_init(mmio, phy_mode, port_num);

    // AHCI init
    writew(HOST_RESET, (volatile void *)mmio + (HOST_CTL));
#endif

    #if 0
    writel(0x00000000, (volatile void *)hba_base + (HOST_CAP));
    writel(0x00000001, (volatile void *)hba_base + (HOST_PORTS_IMPL));
    writel(0x00000001, (volatile void *)port_base + (PORT_SCR_CTL));
    writel(0x00000000, (volatile void *)port_base + (PORT_SCR_CTL));
    writel(0x02100000, (volatile void *)port_base + (PORT_LST_ADDR));
    writel(0x02000000, (volatile void *)port_base + (PORT_FIS_ADDR));
    writel(0x00000016, (volatile void *)port_base + (PORT_CMD));
    #endif
    #if (SW_OOB_MODE == 1)
    sstar_sata_sw_oob_mode1();
    #elif (SW_OOB_MODE == 2)
    sstar_sata_sw_oob_mode2();
    #else
    u32Temp = sstar_sata_wait_reg1(HOST_CTL + (phys_addr_t)mmio, HOST_RESET, HOST_RESET, 1, 500);
    if (u32Temp & HOST_RESET)
    {
        printk("SATA host reset fail!\n");
        return -1;
    }
    #endif

    // Turn on AHCI_EN
    u32Temp = readl((void *)HOST_CTL + (phys_addr_t)hba_base);
    if (u32Temp & HOST_AHCI_EN)
    {
        MHal_SATA_Setup_Port_Implement((phys_addr_t)misc_base, (phys_addr_t)port_base, (phys_addr_t)hba_base);
        goto SS_HOST_AHCI_EN_DONE;
    }

    // Try AHCI_EN Trurn on for a few time
    for (i = 0; i < 5; i++)
    {
        u32Temp |= HOST_AHCI_EN;
        writel(u32Temp, (void *)HOST_CTL + (phys_addr_t)hba_base);
        u32Temp = readl((void *)HOST_CTL + (phys_addr_t)hba_base);
        if (u32Temp & HOST_AHCI_EN)
            break;
        msleep(10);
    }

    MHal_SATA_Setup_Port_Implement((phys_addr_t)misc_base, (phys_addr_t)port_base, (phys_addr_t)hba_base);

SS_HOST_AHCI_EN_DONE:
    printk("sstar sata HW settings done!!!\n");
    return 0;
}
//EXPORT_SYMBOL(ss_sata_init);



#if  defined(CONFIG_ARCH_INFINITY2M) && defined(CONFIG_PM_SLEEP)
static int sstar_ahci_suspend1(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);
	struct ahci_host_priv *hpriv = host->private_data;
	struct brcm_ahci_priv *priv = hpriv->plat_data;
	int ret;

	ret = ahci_platform_suspend(dev);

	//brcm_sata_phys_disable(priv);
	return ret;
}

static int sstar_ahci_resume1(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);
	struct ahci_host_priv *hpriv = host->private_data;
	struct brcm_ahci_priv *priv = hpriv->plat_data;

	//brcm_sata_init(priv);
	//brcm_sata_phys_enable(priv);
	//brcm_sata_alpm_init(hpriv);
	return ahci_platform_resume(dev);
}
#endif


#if  defined(CONFIG_ARCH_INFINITY2) && defined(CONFIG_PM_SLEEP)
static int sstar_ahci_suspend1(struct device *dev)
{
    struct ahci_platform_data *pdata = dev_get_platdata(dev);
    struct ata_host *host = dev_get_drvdata(dev);
    struct ahci_host_priv *hpriv = host->private_data;
    void __iomem *mmio = hpriv->mmio;
    u32 ctl;
    int rc;

    if (hpriv->flags & AHCI_HFLAG_NO_SUSPEND)
    {
        dev_err(dev, "firmware update required for suspend/resume\n");
        return -EIO;
    }

    /*
     * AHCI spec rev1.1 section 8.3.3:
     * Software must disable interrupts prior to requesting a
     * transition of the HBA to D3 state.
     */
    ctl = readl(mmio + HOST_CTL);
    ctl &= ~HOST_IRQ_EN;
    writel(ctl, mmio + HOST_CTL);
    readl(mmio + HOST_CTL); /* flush */

    rc = ata_host_suspend(host, PMSG_SUSPEND);
    if (rc)
        return rc;

    if (pdata && pdata->suspend)
        return pdata->suspend(dev);

    if (!IS_ERR(hpriv->clk))
        clk_disable_unprepare(hpriv->clk);

    return 0;
}

static int sstar_ahci_resume1(struct device *dev)
{
    struct ahci_platform_data *pdata = dev_get_platdata(dev);
    struct ata_host *host = dev_get_drvdata(dev);
    struct ahci_host_priv *hpriv = host->private_data;
    int rc;

    if (!IS_ERR(hpriv->clk))
    {
        rc = clk_prepare_enable(hpriv->clk);
        if (rc)
        {
            dev_err(dev, "clock prepare enable failed");
            return rc;
        }
    }

    if (pdata && pdata->resume)
    {
        rc = pdata->resume(dev);
        if (rc)
            goto disable_unprepare_clk;
    }

    if (dev->power.power_state.event == PM_EVENT_SUSPEND)
    {
        rc = ahci_reset_controller(host);
        if (rc)
            goto disable_unprepare_clk;

        ahci_init_controller(host);
    }

    ata_host_resume(host);

    return 0;

disable_unprepare_clk:
    if (!IS_ERR(hpriv->clk))
        clk_disable_unprepare(hpriv->clk);

    return rc;
}
#endif




static SIMPLE_DEV_PM_OPS(ahci_sstar_pm_ops1, sstar_ahci_suspend1, sstar_ahci_resume1);

static const struct of_device_id ahci_of_match1[] = {
//	{ .compatible = "generic-ahci", },
	{ .compatible = "sstar,sata1",    },
	/* Keep the following compatibles for device tree compatibility */
	{},
};

MODULE_DEVICE_TABLE(of, ahci_of_match1);


static int ahci_sstar_probe1(struct platform_device *pdev)
{
    const struct of_device_id *of_id;
    struct device *dev = &pdev->dev;
    struct sstar_ahci_priv *priv;
    struct ahci_host_priv *hpriv;
    struct resource *res;
    int ret;
    //	int rc;

    priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    of_id = of_match_node(ahci_of_match1, pdev->dev.of_node);
    if (!of_id)
        return -ENODEV;

    priv->dev = dev;

//	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ahci");
//	priv->res_ahci = devm_ioremap_resource(dev, res);
//	if (IS_ERR(priv->res_ahci))
//		return PTR_ERR(priv->res_ahci);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ahci_port0");
    priv->res_ahci_port0 = devm_ioremap_resource(dev, res);
    if (IS_ERR(priv->res_ahci_port0))
        return PTR_ERR(priv->res_ahci_port0);

    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ahci_misc");
    priv->res_ahci_misc = devm_ioremap_resource(dev, res);
    if (IS_ERR(priv->res_ahci_misc))
        return PTR_ERR(priv->res_ahci_misc);

    //    pr_info("[%s] , priv->res_ahci =0x%p \n", __func__ , priv->res_ahci );
    pr_info("[%s] , priv->res_ahci_port0 =0x%p \n", __func__ , priv->res_ahci_port0 );
    pr_info("[%s] , priv->res_ahci_misc =0x%p \n", __func__ , priv->res_ahci_misc );


#if  defined(CONFIG_ARCH_INFINITY2)
    ss_sata_init1(priv->res_ahci_port0 - 0x200 );
#else
    ss_sata_init(priv->res_ahci_port0 - 0x100 );
#endif

    hpriv = ahci_platform_get_resources(pdev);
    if (IS_ERR(hpriv))
        return PTR_ERR(hpriv);

    hpriv->plat_data = priv;

    ret = ahci_platform_enable_resources(hpriv);
    if (ret)
        return ret;

    ret = ahci_platform_init_host(pdev, hpriv, &ahci_port_info1, &ahci_platform_sht1);
    if (ret)
    {
        goto disable_resources;
    }


    pr_info("[%s] , SSTAR AHCI SATA registered \n", __func__ );

    return 0;

disable_resources:
	ahci_platform_disable_resources(hpriv);
	return ret;

}



static const struct acpi_device_id ahci_acpi_match1[] = {
	{ ACPI_DEVICE_CLASS(PCI_CLASS_STORAGE_SATA_AHCI, 0xffffff) },
	{},
};
MODULE_DEVICE_TABLE(acpi, ahci_acpi_match1);






static int sstar_ahci_remove1(struct platform_device *pdev)
{
//	struct ata_host *host = dev_get_drvdata(&pdev->dev);
//	struct ahci_host_priv *hpriv = host->private_data;
//	struct brcm_ahci_priv *priv = hpriv->plat_data;
	int ret;

	ret = ata_platform_remove_one(pdev);
	if (ret)
		return ret;

	//brcm_sata_phys_disable(priv);

	return 0;
}

static struct platform_driver ahci_driver1 = {
	.probe = ahci_sstar_probe1,
	.remove =  sstar_ahci_remove1,
	.driver = {
		.name = DRV_NAME1,
		.of_match_table = ahci_of_match1,
		.acpi_match_table = ahci_acpi_match1,
		.pm = &ahci_sstar_pm_ops1,
	},
};
module_platform_driver(ahci_driver1);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sstar Semiconductor");
MODULE_DESCRIPTION("AHCI SATA platform driver");


