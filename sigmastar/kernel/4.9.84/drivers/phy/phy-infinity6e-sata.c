/*
 * Copyright (c) 2014 Linaro Ltd.
 * Copyright (c) 2014 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include "../drivers/sstar/include/ms_platform.h"

#define BANK_TO_ADDR32(b) (b<<9)
#define MS_IO_OFFSET 0xDE000000

#define GET_BASE_ADDR_BY_BANK(x, y)         ((x) + ((y) << 1))
#define GET_REG16_ADDR(x, y)                ((x) + ((y) << 2))
#define GET_REG8_ADDR(x, y)                 ((x) + ((y) << 1) - ((y) & 1))

#define RIU_BASE                            0x1F200000
#define UTMI_BASE_ADDR                      GET_BASE_ADDR_BY_BANK(RIU_BASE, 0x42100) 
/* macro to get at MMIO space when running virtually */
#define IO_ADDRESS(x)           ( (u32)(x) + MS_IO_OFFSET )

/* read register by word */
#define ms_readw(a) (*(volatile unsigned short *)IO_ADDRESS(a))

/* write register by word */
#define ms_writew(v,a) (*(volatile unsigned short *)IO_ADDRESS(a) = (v))

#define INREG16(x)              ms_readw(x)
#define OUTREG16(x, y)          ms_writew((u16)(y), x)

struct infinity6e_priv {
	void __iomem	*base;
};

enum phy_speed_mode {
	SPEED_MODE_GEN1 = 0,
	SPEED_MODE_GEN2 = 1,
	SPEED_MODE_GEN3 = 2,
};

static int phy_infinity6e_sata_init(struct phy *phy)
{
    unsigned int u4IO_PHY_BASE;
    unsigned int u4phy_bank[3];
    unsigned int dphy_base, aphy_base0, aphy_base1;
	
    of_property_read_u32(phy->dev.of_node, "io_phy_addr", &u4IO_PHY_BASE);
    of_property_read_u32_array(phy->dev.of_node, "banks", (unsigned int*)u4phy_bank, 3);

    dphy_base = BANK_TO_ADDR32(u4phy_bank[0])+u4IO_PHY_BASE ;
    aphy_base0 = BANK_TO_ADDR32(u4phy_bank[1])+u4IO_PHY_BASE ;
    aphy_base1 = BANK_TO_ADDR32(u4phy_bank[2])+u4IO_PHY_BASE ;

    printk("Infinity6e PHY init, d-phy:%x, a-phy0:%x, a-phy1:%x\n", dphy_base, aphy_base0, aphy_base1);
    // For debug test
    OUTREG32((BANK_TO_ADDR32(0x1a21)+u4IO_PHY_BASE) + 0x1c*4, 0x40000);
    SETREG16(aphy_base0 + 0x0d*4, 0x100);
	////SETREG16(aphy_base0 + 0x0c*4, 0x2);
	SETREG16(dphy_base + 0x44*4, 0x80);
	SETREG16(dphy_base + 0x25*4, 0x10);
	SETREG16((BANK_TO_ADDR32(0x1436)+u4IO_PHY_BASE) + 0x1*4, 0x8);
	SETREG16((BANK_TO_ADDR32(0x1433)+u4IO_PHY_BASE) + 0x20*4, 0x7);
	SETREG16((BANK_TO_ADDR32(0x101e)+u4IO_PHY_BASE) + 0x75*4, 0x4000);
	SETREG16((BANK_TO_ADDR32(0x101e)+u4IO_PHY_BASE) + 0x77*4, 0x1b);
	SETREG16((BANK_TO_ADDR32(0x101e)+u4IO_PHY_BASE) + 0x12*4, 0x10);
    // Done for debug test

	// ck debug
	//SETREG16((BANK_TO_ADDR32(0x1436)+u4IO_PHY_BASE) + 0x1*4, 0x8);
	//SETREG16((BANK_TO_ADDR32(0x1433)+u4IO_PHY_BASE) + 0x20*4, 0x7);

	//dylan
	//SETREG16(dphy_base + 0x15*4, 0x18); // force sigdet_cal_en, sigdet_en
	//SETREG16(dphy_base + 0x52*4, 0x20); // set sigdet_cal_en, sigdet_en
	//SETREG16(dphy_base + 0x16*4, 0x80); 
	//SETREG16(dphy_base + 0x55*4, 0x80); //saosc_en
	//SETREG16(dphy_base + 0x19*4, 0x80);
	//CLRREG16(dphy_base + 0x69*4, 0x40); //lfps_pwd

    // USB30 pd_cdr_fsm setting for r-term noise condition
#if 0
    CLRREG16(aphy_base0 + 0x50*4, 0x11);
	CLRREG16(aphy_base0 + 0x54*4, 0x4);
	SETREG16(aphy_base0 + 0x54*4, 0x2);
	SETREG16(aphy_base0 + 0x56*4, 0x1e0);
	CLRREG16(aphy_base0 + 0x71*4, 0xfff);
	SETREG16(aphy_base0 + 0x71*4, 0x9c5);
	CLRREG16(aphy_base0 + 0x75*4, 0xfff);
	SETREG16(aphy_base0 + 0x75*4, 0x9c5);
	CLRREG16(aphy_base0 + 0x72*4, 0xffff);
	SETREG16(aphy_base0 + 0x72*4, 0x21c1);
	CLRREG16(aphy_base0 + 0x76*4, 0xffff);
	SETREG16(aphy_base0 + 0x76*4, 0x21c1);
	CLRREG16(aphy_base0 + 0x73*4, 0x8);
	SETREG16(aphy_base0 + 0x73*4, 0x30);
	CLRREG16(aphy_base0 + 0x77*4, 0x8);
	SETREG16(aphy_base0 + 0x77*4, 0x30);
	SETREG16(aphy_base0 + 0x74*4, 0xa0);
	SETREG16(aphy_base0 + 0x78*4, 0xa0);
	SETREG16(aphy_base0 + 0x57*4, 0xf0);
	CLRREG16(aphy_base0 + 0x54*4, 0x10);

	SETREG16(dphy_base + 0xc*4, 0x100);
	CLRREG16(dphy_base + 0x33*4, 0x80);

	/* Unmask phy irq */
	CLRREG16(dphy_base + 0x1a*4, 0xffff);
	CLRREG16(dphy_base + 0x1b*4, 0xffff);
#endif
    CLRREG16(0x1F284204, 0x80);
    SETREG16(0x1F284224, 0x8000);
    // Unmask USB30_gp2top interrupt
    CLRREG16(0x1F286684, 0x02);
    //CLRREG16(0x1F286694, 0x01);
    /* Sigmastar Infinity6e USB3.0 PHY initialization */


	//SETREG16(aphy_base0 + 0x28*4, 0x40); // set suspend_clk to 24MHz
	//SETREG16(aphy_base0 + 0x50*4, 0x10); // temp add for catc issue
    //SETREG16(aphy_base0 + 0x30*4, 0xe); // RXPLL charge pump control
	//CLRREG16(dphy_base + 0x29*4, 0x400);
	//SETREG16(dphy_base + 0x29*4, 0x400);
	SETREG16(dphy_base + 0x0a*4, 0x2000); // temp mark off

	//SETREG16(aphy_base1 + 0x20*4, 0x4); // For loopback mode symbol swap

	//SETREG16(dphy_base + 0x34*4, 0x300); // For HW-auto k enhance

    CLRREG16(dphy_base + 0x34*4, 0x4000); // RG_SSUSB_LFPS_PWD[14] = 0 // temp add here
    // Turn on TX PLL
    CLRREG16(aphy_base0 + 0x20*4, 0x01); // reg_sata_pd_txpll[0] = 0
    // Turn on RX PLL
    CLRREG16(aphy_base0 + 0x30*4, 0x01); // reg_sata_pd_rxpll[0] = 0
    mdelay(1);
    // De-assert USB PHY reset
    SETREG16(aphy_base1, 0x10); // reg_ssusb_phy_swrst[4] = 1
    // Toggle synthesizer to turn on PLL reference clock input
    SETREG16(aphy_base0 + 0x44*4, 0x01); // reg_sata_phy_synth_sld[0] = 1
    // Diable RXPLL frequency lock detection hardware mode
    
    CLRREG16(aphy_base0 + 0x70*4, 0x04); // reg_sata_phy_rxpll_det_hw_mode_always[2] = 0
    // Enable RXPLL frequency lock detection
    SETREG16(aphy_base0 + 0x70*4, 0x02); // reg_sata_phy_rxpll_det_sw_enable_always[1] = 1
    // Enable TXPLL frequency lock detection
    SETREG16(aphy_base0 + 0x60*4, 0x02); // reg_sata_phy_txpll_det_sw_enable_always[1] = 1
    // CDR state change to freq_unlock_det state when count done
    CLRREG16(aphy_base0 + 0x50*4, 0x2000); // reg_sata_phy_rxpll_cdr_pd_count_done_enable[13] = 0
    // LFPS detector power on
    //CLRREG16(dphy_base + 0x34*4, 0x4000); // RG_SSUSB_LFPS_PWD[14] = 0
    // Tx polarity inverse
    //SETREG16(dphy_base + 0x12*4, 0x2000); // RG_TX_POLARITY_INV[12] = 1 
    CLRREG16(aphy_base1 + 0x49*4, 0x200);
    SETREG16(aphy_base1 + 0x49*4, 0xc4e); // reg_rx_lfps_t_burst_gap = 3150

	//SETREG16(dphy_base + 0x4d*4, 0x2000); // RG_FORCE_TPHY_SPEED


	//SETREG16(aphy_base0 + 0x70*4, 0x80); // reg_sata_phy_rxpll_det_sw_enable_always[7] = 1 =>  sw mode speed sel (by [4:5])
	//SETREG16(aphy_base1 + 0x4a*4, 0x01);
	//CLRREG16(dphy_base + 0x33*4, 0x80);
	//SETREG16(dphy_base + 0x0c*4, 0x100);
	//SETREG16(dphy_base + 0x62*4, 0x3);
	//SETREG16(dphy_base + 0x0e*4, 0x4);
	//SETREG16(dphy_base + 0x0a*4, 0x2000); // temp mark off
	//SETREG16(dphy_base + 0x3c*4, 0xf0);
	//SETREG16(dphy_base + 0x4d*4, 0x100);
    //SETREG16(dphy_base + 0x65*4, 0x100);
    //SETREG16(dphy_base + 0x34*4, 0x300); // RG_SSUSB_LFPS_PWD[14] = 0
	//SETREG16(aphy_base0 + 0x50*4, 0x30);

	//CLRREG16(dphy_base + 0xc*4, 0x100);
	//CLRREG16(aphy_base0 + 0x50*4, 0x31);

    //writew(0x0C2F, (void*) (UTMI_base+0x8*2));
	ms_writew(0x0C2F, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x4));
#ifdef USB_ENABLE_UPLL
    //UTMI_REG_WRITE16(0, 0x4000);
    ms_writew(0x6BC3, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); // Turn on UPLL, reg_pdn: bit<9> reg_pdn: bit<15>, bit <2> ref_pdn
    mdelay(1);
    ms_writeb(0x69, GET_REG16_ADDR(UTMI_BASE_ADDR, 0));   // Turn on UPLL, reg_pdn: bit<9>
    mdelay(2);
    ms_writew(0x0001, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); //Turn all (including hs_current) use override mode
    // Turn on UPLL, reg_pdn: bit<9>
    mdelay(3);
#else
    // Turn on UTMI if it was powered down
    if (0x0001 != ms_readw(GET_REG16_ADDR(UTMI_BASE_ADDR, 0)))
    {
        ms_writew(0x0001, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); //Turn all (including hs_current) use override mode
        mdelay(3);
    }
#endif

    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x1E)) | 0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x1E)); // set CA_START as 1
    mdelay(10);
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x1E)) & ~0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x1E)); // release CA_START
    while (0 == (ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x1E)) & 0x02));		 // polling bit <1> (CA_END)
	
    //msb250x_udc_init_usb_ctrl();
	
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x03)) & 0x9F) | 0x40, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x03));	  //reg_tx_force_hs_current_enable
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x01) + 1) | 0x28), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x01) + 1);	  //Disconnect window select
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x01) + 1) & 0xef), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x01) + 1);	  //Disconnect window select
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x03) + 1) & 0xfd), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x03) + 1);	  //Disable improved CDR
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x04) + 1) | 0x81), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x04) + 1);	  // UTMI RX anti-dead-loc, ISI effect improvement
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x0A) + 1) | 0x20), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x0A) + 1);	  // Chirp signal source select
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x05) + 1) | 0x80), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x05) + 1);	  // set reg_ck_inv_reserved[6] to solve timing problem
	
#if 0//defined(CONFIG_MSTAR_CEDRIC)
    UTMI_REG_WRITE8(0x2c*2,   0x10);
    UTMI_REG_WRITE8(0x2d*2-1, 0x02);
    UTMI_REG_WRITE8(0x2f*2-1, 0x81);
#else
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16)) | 0x98), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1) | 0x02), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1);
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17));
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1) | 0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1);
#endif

    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02)) | 0x80), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02)); //avoid glitch


    return 0;
}

static int phy_infinity6e_sata_power_on(struct phy *phy)
{
    unsigned int u4IO_PHY_BASE;
    unsigned int u4phy_bank[3];
    unsigned int dphy_base, aphy_base0, aphy_base1;
	
    of_property_read_u32(phy->dev.of_node, "io_phy_addr", &u4IO_PHY_BASE);
    of_property_read_u32_array(phy->dev.of_node, "banks", (unsigned int*)u4phy_bank, 3);

    dphy_base = BANK_TO_ADDR32(u4phy_bank[0])+u4IO_PHY_BASE ;
    aphy_base0 = BANK_TO_ADDR32(u4phy_bank[1])+u4IO_PHY_BASE ;
    aphy_base1 = BANK_TO_ADDR32(u4phy_bank[2])+u4IO_PHY_BASE ;

    // Tx polarity inverse
    SETREG16(dphy_base + 0x12*4, 0x1000); // RG_TX_POLARITY_INV[12] = 1 

	return 0;

}


static const struct phy_ops phy_infinity6e_sata_ops = {
	.init		= phy_infinity6e_sata_init,
    .power_on   = phy_infinity6e_sata_power_on,
	.owner		= THIS_MODULE,
};

static int phy_infinity6e_sata_probe(struct platform_device *pdev)
{
	struct phy_provider *phy_provider;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct phy *phy;
	struct infinity6e_priv *priv;


	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);


	printk("Infinity6e PHY probe, base:%x\n", (unsigned int)priv->base);


	phy = devm_phy_create(dev, NULL, &phy_infinity6e_sata_ops);
	if (IS_ERR(phy)) {
		dev_err(dev, "failed to create PHY\n");
		return PTR_ERR(phy);
	}

	phy_set_drvdata(phy, priv);
	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id phy_infinity6e_sata_of_match[] = {
	{.compatible = "sstar,infinity6e-sata-phy",},
	{ },
};
MODULE_DEVICE_TABLE(of, phy_infinity6e_sata_of_match);

static struct platform_driver phy_infinity6e_sata_driver = {
	.probe	= phy_infinity6e_sata_probe,
	.driver = {
		.name	= "phy",
		.of_match_table	= phy_infinity6e_sata_of_match,
	}
};
module_platform_driver(phy_infinity6e_sata_driver);

MODULE_AUTHOR("Jiang Ann <jiang.ann@sigmastar.com>");
MODULE_DESCRIPTION("INFINITY6E SATA PHY driver");
MODULE_ALIAS("platform:phy-infinity6e-sata");
MODULE_LICENSE("GPL v2");
