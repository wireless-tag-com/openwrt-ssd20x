/*
 * xhci-mstar.c - xHCI host controller driver platform Bus Glue.
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include "xhci.h"
#include "xhci-mstar.h"
#include "bc-mstar.h"
#include "mstar-lib.h"

extern int create_xhci_sysfs_files(struct xhci_hcd *xhci);
extern void remove_xhci_sysfs_files(struct xhci_hcd *xhci);

static void xhci_plat_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	/*
	 * As of now platform drivers don't provide MSI support so we ensure
	 * here that the generic code does not try to make a pci_dev from our
	 * dev struct in order to setup MSI
	 */
	xhci->quirks |= XHCI_PLAT;
	xhci->quirks |= XHCI_RESET_ON_RESUME;
	xhci->quirks |= XHCI_SPURIOUS_SUCCESS;
	// xhci->quirks |= XHCI_TRUST_TX_LENGTH;
	xhci->hci_version = 0x96;  //modified for real version.
}

/* called during probe() after chip reset completes */
static int xhci_plat_setup(struct usb_hcd *hcd)
{
	return xhci_gen_setup(hcd, xhci_plat_quirks);
}

static struct hc_driver mstar_plat_xhci_driver = {
	.description =		"sstar-xhci-hcd",
	.product_desc =		"Sstar xHCI Host Controller",
	.hcd_priv_size =	sizeof(struct xhci_hcd *),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3 | HCD_SHARED,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_plat_setup,
	.start =		xhci_run,
	.stop =			xhci_stop,
	.shutdown =		xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		xhci_urb_enqueue,
	.urb_dequeue =		xhci_urb_dequeue,
	.alloc_dev =		xhci_alloc_dev,
	.free_dev =		xhci_free_dev,
	.alloc_streams =	xhci_alloc_streams,
	.free_streams =		xhci_free_streams,
	.add_endpoint =		xhci_add_endpoint,
	.drop_endpoint =	xhci_drop_endpoint,
	.endpoint_reset =	xhci_endpoint_reset,
	.check_bandwidth =	xhci_check_bandwidth,
	.reset_bandwidth =	xhci_reset_bandwidth,
	.address_device =	xhci_address_device,
	.update_hub_device =	xhci_update_hub_device,
	.reset_device =		xhci_discover_or_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
	.bus_suspend =		xhci_bus_suspend,
	.bus_resume =		xhci_bus_resume,
};

void DEQ_init(uintptr_t U3PHY_D_base, uintptr_t U3PHY_A_base)
{
	writeb(0x00,   (void*)(U3PHY_A_base+0xAE*2));
	writew(0x080C, (void*)(U3PHY_D_base+0x82*2));
	writeb(0x10,   (void*)(U3PHY_D_base+0xA4*2)); //0x10  0x30
	writew(0x4100, (void*)(U3PHY_D_base+0xA0*2));

	writew(0x06,   (void*)(U3PHY_A_base+0x06*2));
}

void  XHCI_enable_testbus(uintptr_t CHIPTOP_base, uintptr_t U3TOP_base, uintptr_t XHCI_base)
{
	writeb(0x00,   (void*)(CHIPTOP_base+0xEC*2));
	writeb(0x20,   (void*)(CHIPTOP_base+0x24*2));
	writeb(0x42,   (void*)(CHIPTOP_base+0xEE*2));   // select usb30_test_out
	// writeb(0x00,   (void*)(CHIPTOP_base+0xEA*2));
	writeb(0x40,   (void*)(CHIPTOP_base+0xEB*2-1));
	//writew(0xFFFF, (void*)(CHIPTOP_base+0x12*2));
	//writew(0xFFFF, (void*)(CHIPTOP_base+0x14*2));
	//writeb(0xFF,   (void*)(CHIPTOP_base+0x16*2));
	//writeb(0x01,   (void*)(CHIPTOP_base+0x21*2-1));
	//writeb(0x02,   (void*)(CHIPTOP_base+0x3A*2));

	//writeb(0x0d,   (void*)(U3PHY_D_base+0xD5*2-1));
	//writeb(0x23,   (void*)(U3PHY_D_base+0x2E*2));
	//writeb(0x40,   (void*)(U3PHY_D_base+0x2F*2-1));

	//writeb(0x09,   (void*)(U3TOP_base+0x0C*2)); 	// [5] = reg_debug_mask to 1'b0
	                                                // [4:0] = 0x06= rrdy & wrdy
	writeb(0x00,   (void*)(U3TOP_base+0x0C*2));

	writeb(0x11,    (void*) (XHCI_base+0x608c));
	writeb(0x30,    (void*) (XHCI_base+0x608f));
	writeb(0x39,    (void*) (XHCI_base+0x6805));
	writeb(0x3a,    (void*) (XHCI_base+0x6806));
	writeb(0x21,    (void*) (XHCI_base+0x6807));
}

#if defined(XHCI_ENABLE_PPC)

static void XHCI_enable_PPC(uintptr_t U3TOP_base)
{
	u16 addr_w, bit_num;
	uintptr_t addr;
	u8  value, low_active;
	u8  i, portnum=1;

	for (i=0; i<portnum; i++) {

		switch (i) {
		case 0:
			addr_w = readw((void*)(U3TOP_base+0xFC*2));
			addr = (uintptr_t)addr_w << 8;
			addr_w = readw((void*)(U3TOP_base+0xFE*2));
			addr |= addr_w & 0xFF;
			bit_num = (addr_w >> 8) & 0x7;
			low_active = (u8)((addr_w >> 8) & 0x8);
			break;
		case 1:
			addr_w = readw((void*)(U3TOP_base+0xE6*2));
			addr = (uintptr_t)addr_w << 8;
			addr_w = readw((void*)(U3TOP_base+0xE8*2));
			addr |= addr_w & 0xFF;
			bit_num = (addr_w >> 8) & 0x7;
			low_active = (u8)((addr_w >> 8) & 0x8);
			break;
		default:	/* "can't happen" */
			return;
		}

		if (addr)
		{
			printk("XHCI_enable_PPC: Turn on USB3.0 port %d power \n", i);
			printk("Addr: 0x%lx bit_num: %d low_active:%d\n", addr, bit_num, low_active);

			value = (u8)(1 << bit_num);

			if (low_active)
			{
				if (addr & 0x1)
					writeb(readb((void*)(_MSTAR_PM_BASE+addr*2-1)) & (u8)(~value), (void*)(_MSTAR_PM_BASE+addr*2-1));
				else
					writeb(readb((void*)(_MSTAR_PM_BASE+addr*2)) & (u8)(~value), (void*)(_MSTAR_PM_BASE+addr*2));
			}
			else
			{
				if (addr & 0x1)
					writeb(readb((void*)(_MSTAR_PM_BASE+addr*2-1)) | value, (void*)(_MSTAR_PM_BASE+addr*2-1));
				else
					writeb(readb((void*)(_MSTAR_PM_BASE+addr*2)) | value, (void*)(_MSTAR_PM_BASE+addr*2));
			}
		}
	}
}
#endif

void Mstar_U2utmi_init(uintptr_t UTMI_base, uintptr_t U3TOP_base, unsigned int flag)
{
	if ((UTMI_base==0) || (U3TOP_base==0))
		return;

	if (0 == readw((void*)(UTMI_base+0x20*2)))
	{
		printk("utmi clk enable\n");
		writew(0x8051, (void*) (UTMI_base+0x20*2));
		writew(0x2088, (void*) (UTMI_base+0x22*2));
		writew(0x0004, (void*) (UTMI_base+0x2*2));
		writew(0x6BC3, (void*) (UTMI_base));
		mdelay(1);
		writew(0x69C3, (void*) (UTMI_base));
		mdelay(1);
		writew(0x0001, (void*) (UTMI_base));
		mdelay(1);
	}

	writeb(0x07, (void*) (UTMI_base+0x8*2));   //default value 0x7; don't change it.

	if (flag & EHCFLAG_TESTPKG)
	{
		writew(0x2084, (void*) (UTMI_base+0x2*2));
		writew(0x8051, (void*) (UTMI_base+0x20*2));
	}

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
	/*
	 * patch for DM always keep high issue
	 * init overwrite register
	 */
	writeb(readb((void*)(UTMI_base+0x0*2)) & (u8)(~BIT3), (void*) (UTMI_base+0x0*2)); //DP_PUEN = 0
	writeb(readb((void*)(UTMI_base+0x0*2)) & (u8)(~BIT4), (void*) (UTMI_base+0x0*2)); //DM_PUEN = 0

	writeb(readb((void*)(UTMI_base+0x0*2)) & (u8)(~BIT5), (void*) (UTMI_base+0x0*2)); //R_PUMODE = 0

	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT6, (void*) (UTMI_base+0x0*2)); //R_DP_PDEN = 1
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT7, (void*) (UTMI_base+0x0*2)); //R_DM_PDEN = 1

	writeb(readb((void*)(UTMI_base+0x10*2)) | BIT6, (void*) (UTMI_base+0x10*2)); //hs_txser_en_cb = 1
	writeb(readb((void*)(UTMI_base+0x10*2)) & (u8)(~BIT7), (void*) (UTMI_base+0x10*2)); //hs_se0_cb = 0

	/* turn on overwrite mode */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT1, (void*) (UTMI_base+0x0*2)); //tern_ov = 1
#endif

	writeb(readb((void*)(UTMI_base+0x3C*2)) | 0x01, (void*) (UTMI_base+0x3C*2)); // set CA_START as 1
	mdelay(1);

	writeb(readb((void*)(UTMI_base+0x3C*2)) & (u8)(~0x01), (void*) (UTMI_base+0x3C*2)); // release CA_START

	while ((readb((void*)(UTMI_base+0x3C*2)) & 0x02) == 0); // polling bit <1> (CA_END)

	if ((0xFFF0 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 )) ||
		(0x0000 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 ))  )
	{
		printk("WARNING: CA Fail !! \n");
	}

	if (flag & EHCFLAG_DPDM_SWAP)
		writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x20, (void*) (UTMI_base+0x0b*2-1)); // dp dm swap

	writeb((u8)((readb((void*)(UTMI_base+0x06*2)) & 0x9F) | 0x40), (void*) (UTMI_base+0x06*2)); //reg_tx_force_hs_current_enable

	writeb(readb((void*)(UTMI_base+0x03*2-1)) | 0x28, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select
	writeb(readb((void*)(UTMI_base+0x03*2-1)) & 0xef, (void*) (UTMI_base+0x03*2-1)); //Disconnect window select

	writeb(readb((void*)(UTMI_base+0x07*2-1)) & 0xfd, (void*) (UTMI_base+0x07*2-1)); //Disable improved CDR
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x81, (void*) (UTMI_base+0x09*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement
	writeb(readb((void*)(UTMI_base+0x0b*2-1)) |0x80, (void*) (UTMI_base+0x0b*2-1)); // TX timing select latch path
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x20, (void*) (UTMI_base+0x15*2-1)); // Chirp signal source select

	//Enable XHCI preamble function with keep-alive
	writeb(readb((void*)(UTMI_base+0x3F*2-1)) |0x80, (void*) (UTMI_base+0x3F*2-1)); // Enable XHCI preamble function

#if defined(XHCI_ENABLE_240MHZ)  //exclude Agate
	/* before Madison, bit4 [for 240 setting], bit5 [no use]
	 * after Madison, bit4 [no use], bit5 [for 240 setting]
	 * setting both bits for all chips.
	 */
	writeb(readb((void*)(UTMI_base+0x08*2)) | 0x38, (void*) (UTMI_base+0x08*2)); // for 240's phase as 120's clock
	#if defined(XHCI_USE_120_PHASE)
	//Fix it for Eiffel Only.
	writeb(readb((void*)(UTMI_base+0x08*2)) & 0xF7, (void*) (UTMI_base+0x08*2)); // Clear setting of "240's phase as 120's clock"
	#endif
#endif

	// change to 55 timing; for all chips.
	writeb(readb((void*)(UTMI_base+0x15*2-1)) |0x40, (void*) (UTMI_base+0x15*2-1)); // change to 55 timing

	// for CLK 120 override enable; for xHCI on all chips
	writeb(readb((void*)(UTMI_base+0x09*2-1)) |0x04, (void*) (UTMI_base+0x09*2-1)); // for CLK 120 override enable

	/* Init UTMI disconnect level setting */
	writeb(UTMI_DISCON_LEVEL_2A, (void*)(UTMI_base+0x2a*2));

	writeb(UTMI_EYE_SETTING_2C, (void*) (UTMI_base+0x2c*2));
	writeb(UTMI_EYE_SETTING_2D, (void*) (UTMI_base+0x2d*2-1));
	writeb(UTMI_EYE_SETTING_2E, (void*) (UTMI_base+0x2e*2));
	writeb(UTMI_EYE_SETTING_2F, (void*) (UTMI_base+0x2f*2-1));

#if defined(ENABLE_LS_CROSS_POINT_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x40, (void*) (UTMI_base+0x04*2));  //enable deglitch SE0¡¨(low-speed cross point)
#endif

#if defined(ENABLE_TX_RX_RESET_CLK_GATING_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x20, (void*) (UTMI_base+0x04*2)); //enable hw auto deassert sw reset(tx/rx reset)
#endif

#if defined(ENABLE_KEEPALIVE_ECO)
	writeb(readb((void*)(UTMI_base+0x04*2)) | 0x80, (void*) (UTMI_base+0x04*2));	//enable LS keep alive & preamble
#endif

#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
	/* Change override to hs_txser_en.	Dm always keep high issue */
	writeb(readb((void*)(UTMI_base+0x10*2)) | BIT6, (void*) (UTMI_base+0x10*2));
#endif

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
	/*
	 * patch for DM always keep high issue
	 * init overwrite register
	 */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT6, (void*) (UTMI_base+0x0*2)); //R_DP_PDEN = 1
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT7, (void*) (UTMI_base+0x0*2)); //R_DM_PDEN = 1

	/* turn on overwrite mode */
	writeb(readb((void*)(UTMI_base+0x0*2)) | BIT1, (void*) (UTMI_base+0x0*2)); //tern_ov = 1
#endif

#if _USB_ANALOG_RX_SQUELCH_PATCH
	/* squelch level adjust by calibration value */
	{
		unsigned int ca_da_ov, ca_db_ov, ca_tmp;
		ca_tmp = readw((void*)(UTMI_base+0x3c*2));
		ca_da_ov = (((ca_tmp >> 4) & 0x3f) - 5) + 0x40;
		ca_db_ov = (((ca_tmp >> 10) & 0x3f) - 5) + 0x40;
		printk("[%x]-5 ->(ca_da_ov, ca_db_ov) = (%x,%x)\n", ca_tmp, ca_da_ov, ca_db_ov);
		writeb(ca_da_ov ,(void*)(UTMI_base+0x3B*2-1));
		writeb(ca_db_ov ,(void*)(UTMI_base+0x24*2));
	}
#endif

#if (XHCI_CHIRP_PATCH)
	writeb(UTMI_DISCON_LEVEL_2A, (void*) (UTMI_base+0x3E*2)); //override value
	writeb((u8)((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x0A), (void*)(U3TOP_base+0x24*2)); // set T1=50, T2=20
	writeb(readb((void*)(UTMI_base+0x3F*2-1)) | 0x1, (void*) (UTMI_base+0x3F*2-1)); //enable the patch
#endif

	if (flag & EHCFLAG_TESTPKG)
	{
		writew(0x0600, (void*) (UTMI_base+0x14*2));
		writew(0x0078, (void*) (UTMI_base+0x10*2));
		writew(0x0BFE, (void*) (UTMI_base+0x32*2));
	}
}


#if defined(XHCI_PHY_MS40)
static void Mstar_U3phy_MS40_init(uintptr_t U3PHY_D_base, uintptr_t U3PHY_A_base)
{

	//U3phy initial sequence
	writew(0x0,    (void*) (U3PHY_A_base)); 		 // power on rx atop
	writew(0x0,    (void*) (U3PHY_A_base+0x2*2));	 // power on tx atop
	writew(0x0910, (void*) (U3PHY_D_base+0x4*2));
	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));
	writew(0x0160, (void*) (U3PHY_D_base+0x18*2));
	writew(0x0,    (void*) (U3PHY_D_base+0x20*2));	 // power on u3_phy clockgen
	writew(0x0,    (void*) (U3PHY_D_base+0x22*2));	 // power on u3_phy clockgen

	writew(0x013F, (void*) (U3PHY_D_base+0x4A*2));
	writew(0x1010, (void*) (U3PHY_D_base+0x4C*2));

	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));	 // override PD control
	writeb(0x1C,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_usb3aeq_acc;  long EQ converge
	writeb(0x40,   (void*) (U3PHY_A_base+0xC9*2-1)); // reg_gcr_usb3aeq_threshold_abs
	writeb(0x10,   (void*) (U3PHY_A_base+0xE5*2-1)); // [4]: AEQ select PD no-delay and 2elay path, 0: delay, 1: no-
	writeb(0x11,   (void*) (U3PHY_A_base+0xC6*2));	 // analog symbol lock and EQ converage step
	writeb(0x02,   (void*) (U3PHY_D_base+0xA0*2));	 // [1] aeq mode

	writeb(0x07,   (void*) (U3PHY_A_base+0xB0*2));	 // reg_gcr_usb3rx_eq_str_ov_value

#if (ENABLE_XHCI_SSC)
	writew(0x04D8,	(void*) (U3PHY_D_base+0xC6*2));  //reg_tx_synth_span
	writew(0x0003,	(void*) (U3PHY_D_base+0xC4*2));  //reg_tx_synth_step
	writew(0x9375,	(void*) (U3PHY_D_base+0xC0*2));  //reg_tx_synth_set
	writeb(0x18,	(void*) (U3PHY_D_base+0xC2*2));  //reg_tx_synth_set
#endif

	////Set Tolerance  //only for Agate_U01
	/// writew(0x0103, (void*) (U3PHY_D_base+0x44*2));

	// Comma
	// writeb(0x84,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_aeq_acc, 8bit

	// RX phase control
	writew(0x0100, (void*)(U3PHY_A_base+0x90*2));
	writew(0x0302, (void*)(U3PHY_A_base+0x92*2));
	writew(0x0504, (void*)(U3PHY_A_base+0x94*2));
	writew(0x0706, (void*)(U3PHY_A_base+0x96*2));
	writew(0x1708, (void*)(U3PHY_A_base+0x98*2));
	writew(0x1516, (void*)(U3PHY_A_base+0x9A*2));
	writew(0x1314, (void*)(U3PHY_A_base+0x9C*2));
	writew(0x1112, (void*)(U3PHY_A_base+0x9E*2));
	writew(0x3000, (void*)(U3PHY_D_base+0xA8*2));
	writew(0x7380, (void*)(U3PHY_A_base+0x40*2));

#if (XHCI_ENABLE_DEQ)
	DEQ_init(U3PHY_D_base, U3PHY_A_base);
#endif

#if (XHCI_TX_SWING_PATCH)
	writeb(0x3F, (void*)(U3PHY_A_base+0x60*2));
	writeb(0x39, (void*)(U3PHY_A_base+0x62*2));
#endif
}
#endif

#if defined(XHCI_PHY_MS28)
static void Mstar_U3phy_MS28_init(uintptr_t U3PHY_D_base, uintptr_t U3PHY_A_base, unsigned int flag)
{
#ifdef XHCI_SINGLE_PORT_ENABLE_MAC
	writeb(readb((void*)(U3PHY_D_base+0x84*2))| 0x40, (void*)(U3PHY_D_base+0x84*2)); // open XHCI MAC clock
#endif

	if (flag & XHCFLAG_DEGRADATION) {
	#ifdef XHCI_DEGRAD_REP
		writeb(readb((void*)(U3PHY_D_base+0xB6*2))| BIT0, (void*)(U3PHY_D_base+0xB6*2));
		writew(0x03E8, (void*) (U3PHY_D_base+0xA*2));
	#endif
	}

	//-- 28 hpm mstar only---
	writew(0x0104, (void*) (U3PHY_A_base+0x6*2));  // for Enable 1G clock pass to UTMI //[2] reg_pd_usb3_purt [7:6] reg_gcr_hpd_vsel

	//U3phy initial sequence
	writew(0x0,    (void*) (U3PHY_A_base)); 		 // power on rx atop
	writew(0x0,    (void*) (U3PHY_A_base+0x2*2));	 // power on tx atop
	//writew(0x0910, (void*) (U3PHY_D_base+0x4*2));        // the same as default
	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));  // overwrite power on rx/tx atop
	writew(0x0160, (void*) (U3PHY_D_base+0x18*2));
	writew(0x0,    (void*) (U3PHY_D_base+0x20*2));	 // power on u3_phy clockgen
	writew(0x0,    (void*) (U3PHY_D_base+0x22*2));	 // power on u3_phy clockgen

	//writew(0x013F, (void*) (U3PHY_D_base+0x4A*2));      // the same as default
	//writew(0x1010, (void*) (U3PHY_D_base+0x4C*2));      // the same as default

	writew(0x0,    (void*) (U3PHY_A_base+0x3A*2));	 // override PD control
#ifdef XHCI_ENABLE_PD_OVERRIDE
        writew(0x308,    (void*) (U3PHY_A_base+0x3A*2)); // [9,8,3] PD_TXCLK_USB3TXPLL, PD_USB3_IBIAS, PD_USB3TXPLL override enable
        writeb(readb((void*)(U3PHY_A_base+0x3*2-1)) & 0xbb,     (void*)(U3PHY_A_base+0x3*2-1)); // override value 0
#endif

#if 0  // not for MS28
	writeb(0x1C,   (void*) (U3PHY_A_base+0xCD*2-1)); // reg_test_usb3aeq_acc;  long EQ converge
	writeb(0x40,   (void*) (U3PHY_A_base+0xC9*2-1)); // reg_gcr_usb3aeq_threshold_abs
	writeb(0x10,   (void*) (U3PHY_A_base+0xE5*2-1)); // [4]: AEQ select PD no-delay and 2elay path, 0: delay, 1: no-
	writeb(0x11,   (void*) (U3PHY_A_base+0xC6*2));	 // analog symbol lock and EQ converage step
	writeb(0x02,   (void*) (U3PHY_D_base+0xA0*2));	 // [1] aeq mode

	writeb(0x07,   (void*) (U3PHY_A_base+0xB0*2));	 // reg_gcr_usb3rx_eq_str_ov_value
#endif

#if (ENABLE_XHCI_SSC)
#ifdef XHCI_SSC_TX_SYNTH_SET_C0
	writew(XHCI_SSC_TX_SYNTH_SET_C0, (void*) (U3PHY_D_base+0xC0*2));  //reg_tx_synth_set
	writeb(XHCI_SSC_TX_SYNTH_SET_C2, (void*) (U3PHY_D_base+0xC2*2));  //reg_tx_synth_set
	writew(XHCI_SSC_TX_SYNTH_STEP_C4, (void*) (U3PHY_D_base+0xC4*2));  //reg_tx_synth_step
	writew(XHCI_SSC_TX_SYNTH_SPAN_C6, (void*) (U3PHY_D_base+0xC6*2));  //reg_tx_synth_span
#else
	writew(0x04D0,	(void*) (U3PHY_D_base+0xC6*2));  //reg_tx_synth_span
	writew(0x0003,	(void*) (U3PHY_D_base+0xC4*2));  //reg_tx_synth_step
	writew(0x9375,	(void*) (U3PHY_D_base+0xC0*2));  //reg_tx_synth_set
	writeb(0x18,	(void*) (U3PHY_D_base+0xC2*2));  //reg_tx_synth_set
#endif
#endif

	// -- TX current ---
	//writeb(0x01, 	(void*) (U3PHY_A_base+0x35*2-1));
	writew(0x3939 /* 0x3932 */,  (void*) (U3PHY_A_base+0x60*2));
	writew(0x3924 /* 0x3939 */,  (void*) (U3PHY_A_base+0x62*2));
	writew(0x3932,  (void*) (U3PHY_A_base+0x64*2));
	writew(0x3939,  (void*) (U3PHY_A_base+0x66*2));
	writew(0x0400,  (void*) (U3PHY_A_base+0x12*2));
	// ---------------

	//-------- New for MS28 ---------
	writeb(0x0,    (void*) (U3PHY_A_base+0xA1*2-1));  //bit[15] EQ override
	writeb(0xF4,   (void*) (U3PHY_D_base+0x12*2));	  //TX lock threshold

	writeb(readb((void*)(U3PHY_A_base+0xF*2-1))&(u8)(~0x4),	(void*)(U3PHY_A_base+0xF*2-1));  // 0xF[10]  Fix AEQ RX-reset issue

#ifdef XHCI_PHY_ENABLE_RX_LOCK
	writeb(readb((void*)(U3PHY_A_base+0x21*2-1)) | 0x80,	(void*)(U3PHY_A_base+0x21*2-1));  // enable rx_lock behavior.
#endif

#ifdef XHCI_PWS_P2
	writeb(readb((void*)(U3PHY_D_base+0x1E*2))| 0x1,	(void*)(U3PHY_D_base+0x1E*2));	//P2 power saving
#endif

}
#endif

#ifdef U3PHY_RX_DETECT_POWER_SAVING
/* 20170522, U3PHY new power saveing mode */
void Mstar_U3phy_power_saving(uintptr_t U3PHY_D_base, u8 regval)
{
        u8 c_val;

        c_val = readb((void*)(U3PHY_D_base+0x3F*2-1)) & 0xf;
        if ((c_val & 0xf) == regval) // the same setting
                return;

        c_val &= ~0xf;
        c_val |= regval;
        writeb(c_val, (void*)(U3PHY_D_base+0x3F*2-1));
}
#endif

#if defined(ENABLE_USB_NEW_MIU_SLE)
void MIU_select_setting_xhc(uintptr_t U3TOP_base)
{
	printk("[USB] config miu select [%02x] [%02x] [%02x] [%02x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
	writeb(USB_MIU_SEL0, (void*)(U3TOP_base+0x3A*2));	//Setting MIU0 segment
	writeb(USB_MIU_SEL1, (void*)(U3TOP_base+0x36*2));	//Setting MIU1 segment
	writeb(USB_MIU_SEL2, (void*)(U3TOP_base+0x37*2-1));	//Setting MIU2 segment
	writeb(USB_MIU_SEL3, (void*)(U3TOP_base+0x38*2));	//Setting MIU3 segment
	writeb(readb((void*)(U3TOP_base+0x39*2-1)) | BIT6, (void*)(U3TOP_base+0x39*2-1));	//Enable miu partition mechanism
#if !defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
	printk("[USB] enable miu lower bound address subtraction\n");
	writeb(readb((void*)(U3TOP_base+0x39*2-1)) | BIT7, (void*)(U3TOP_base+0x39*2-1));
#endif

}
#endif

static void Mstar_xhc_init(const struct u3phy_addr_base *u3phy_addr,
	unsigned int flag)
{
	uintptr_t UTMI_base = u3phy_addr->utmi_base;
	uintptr_t XHCI_base = u3phy_addr->xhci_base;
	uintptr_t U3TOP_base = u3phy_addr->u3top_base;

	printk("Mstar_xhc_init version:%s\n", XHCI_MSTAR_VERSION);

	// Init USB2 UTMI
	Mstar_U2utmi_init(UTMI_base, U3TOP_base, flag);

	// Init USB3 PHY
#ifdef U3PHY_RX_DETECT_POWER_SAVING
        Mstar_U3phy_power_saving(u3phy_addr->u3dtop_base, 0xf);
#endif

#if defined(XHCI_PHY_MS28)
	Mstar_U3phy_MS28_init(u3phy_addr->u3dtop_base, u3phy_addr->u3atop_base, flag);
	#if defined(XHCI_2PORTS) && !defined(CONFIG_MSTAR_XUSB_PCIE_PLATFORM)
		Mstar_U3phy_MS28_init(u3phy_addr->u3dtop1_base, u3phy_addr->u3atop1_base, flag);
	#endif
#elif defined(XHCI_PHY_MS40)
	Mstar_U3phy_MS40_init(u3phy_addr->u3dtop_base, u3phy_addr->u3atop_base);
#else
	#error Unknown xHCI PHY
#endif

#if defined(ENABLE_USB_NEW_MIU_SLE)
	MIU_select_setting_xhc(U3TOP_base);
#endif

	//First token idle
	writeb(readb((void*)(XHCI_base+0x4308)) | 0x0C, (void*)(XHCI_base+0x4308));  //First token idle (bit2~3 = "11")
	//Inter packet delay
	writeb(readb((void*)(XHCI_base+0x430F)) | 0xC0, (void*)(XHCI_base+0x430F));  //Inter packet delay (bit6~7 = "11")
	//LFPS patch
	writeb(readb((void*)(XHCI_base+0x681A)) | 0x10, (void*)(XHCI_base+0x681A));  //LFPS patch  (bit4 = 1)

	//Ignore checking of max packet size=0 in endpoint context (configure endpoint).
	writeb(readb((void*)(XHCI_base+0x4209)) | 0x08, (void*)(XHCI_base+0x4209)); //for SS_BI_0
	writeb(readb((void*)(XHCI_base+0x4249)) | 0x08, (void*)(XHCI_base+0x4249)); //for SS_BI_1

	//force one queue option for IN direction
	writeb(readb((void*)(XHCI_base+0x4210)) | 0x01, (void*)(XHCI_base+0x4210));

	//mask tx and rx stop ep cmd to avoid stop cmd halt issue
	writeb(readb((void*)(XHCI_base+0x4291)) | (BIT5|BIT6), (void*)(XHCI_base+0x4291));

#if defined(XHCI_DISABLE_COMPLIANCE)
	writeb(readb((void*)(XHCI_base+0x6817)) | 0x80, (void*)(XHCI_base+0x6817));
#endif

	//Bus Reset setting => default 50ms; T1=30; T2=20
#if defined(XHCI_BUSRESET_REG_OFFSET_CHG)
	writeb((readb((void*)(U3TOP_base+XHCI_NEW_BUSRESET_REG_OFFSET)) & 0xF0) | 0x8, (void*)(U3TOP_base+XHCI_NEW_BUSRESET_REG_OFFSET));
#else
	writeb((u8)((readb((void*)(U3TOP_base+0x24*2)) & 0xF0) | 0x8), (void*)(U3TOP_base+0x24*2));    // [5] = reg_debug_mask to 1'b0
#endif

#if defined(XHCI_ENABLE_LASTDOWNZ)
	writeb(readb((void*)(U3TOP_base+0x12*2)) | 0x8, (void*)(U3TOP_base+0x12*2));  //check both last_down_z & data count enable
#endif

#if defined(XHCI_MIU1_SEL_BIT30)
	writeb(readb((void*)(U3TOP_base+0x11*2-1)) | 0x2, (void*)(U3TOP_base+0x11*2-1));  //set reg_miu1_sel to check address bit 30
#endif

#if defined(XHCI_MIU_PROTECT_ENABLE)
	writeb(readb((void*)(U3TOP_base+0x21*2-1)) | 0x8, (void*)(U3TOP_base+0x21*2-1)); // enable miu protect ECO
#endif

#if defined (XHCI_ENABLE_LOOPBACK_ECO)
	writeb(readb((void*)(U3TOP_base+LOOPBACK_ECO_OFFSET))|LOOPBACK_ECO_BIT , (void*)(U3TOP_base+LOOPBACK_ECO_OFFSET));
#endif

#if defined(XHCI_ENABLE_HOTRESET_ECO)
	//Re-enable again to prevent from overwitting by sboot PPC function. Only for Edison.
	writeb(readb((void*)(U3TOP_base+0xFF*2-1))|0x80, (void*)(U3TOP_base+0xFF*2-1));
#endif

#if defined(XHCI_ALLOW_LOOPBACK_MODE) //only for Nike, allow HW enter loopback mode
	writeb(readb((void*)(U3TOP_base+0xf8*2)) | 0x1, (void*)(U3TOP_base+0xf8*2));
#endif

#if (XHCI_ENABLE_TESTBUS)
	XHCI_enable_testbus((_MSTAR_USB_BASEADR+(0x1E00*2)), U3TOP_base, XHCI_base);
#endif

#if defined(XHCI_ENABLE_PPC) && !defined(XHCI_DISABLE_SS_MODE)
	XHCI_enable_PPC(U3TOP_base);
#endif

	// SW note U3Phy init done.
	writeb(readb((void*)(U3TOP_base+0xEA*2)) | BIT0, (void*)(U3TOP_base+0xEA*2));
	// Don't wait trbp response when link not in U0
	writeb(readb((void*)(XHCI_base+0x4205)) | 0x80, (void*)(XHCI_base+0x4205));
}

unsigned int mstar_xhci_get_flag(struct platform_device *pdev)
{
	unsigned int flag=0;

#if defined(XHCI_ENABLE_DPDM_SWAP)
	flag |= EHCFLAG_DPDM_SWAP;
#endif

	// get degration flag

	return flag;
}


#if defined(CONFIG_OF)
extern unsigned int irq_of_parse_and_map(struct device_node *node, int index);
#endif

static int xhci_mstar_plat_probe(struct platform_device *pdev)
{
	const struct hc_driver	*driver;
	struct xhci_hcd		*xhci;
	struct usb_hcd		*hcd;
	int			ret;
	int			irq;
	unsigned int flag=0;
#if defined(CONFIG_OF)
	u64 dma_mask;
#endif
	struct u3phy_addr_base u3phy_init_addr;

	if (usb_disabled())
		return -ENODEV;

	printk("xHCI_%x%04x \n", readb((void*)(_MSTAR_PM_BASE+0x1ECC*2)), readw((void*)(_MSTAR_PM_BASE+0x1ECE*2)));
	printk("Sstar-xhci H.W init\n");

	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", strlen("soc:Sstar-xhci-1")) )
	{
		#ifdef DISABLE_FIRST_XHC
		printk("[XHCI] can't init first xHCI due to DISABLE_FIRST_XHC.....\n");
		return -ENODEV;
		#endif
	}
	else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", strlen("soc:Sstar-xhci-2")) )
	{
		#ifdef DISABLE_SECOND_XHC
		printk("[XHCI] can't init second xHCI due to DISABLE_SECOND_XHC.....\n");
		return -ENODEV;
		#endif
	}

#ifdef ENABLE_SECOND_XHC /* 2 roots case */
	#define NAME_LEN (16)
	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) )
	{
		u3phy_init_addr.utmi_base = _MSTAR_U3UTMI0_BASE;
		u3phy_init_addr.xhci_base = _MSTAR_XHCI0_BASE;
		u3phy_init_addr.u3top_base = _MSTAR_U3TOP0_BASE;
		u3phy_init_addr.bc_base = _MSTAR_U3BC0_BASE;
		u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP0_BASE;
		u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP0_BASE;
	}
	else if(0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN)) {
		u3phy_init_addr.utmi_base = _MSTAR_U3UTMI1_BASE;
		u3phy_init_addr.xhci_base = _MSTAR_XHCI1_BASE;
		u3phy_init_addr.u3top_base = _MSTAR_U3TOP1_BASE;
		u3phy_init_addr.bc_base = _MSTAR_U3BC1_BASE;
		u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP1_BASE;
		u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP1_BASE;
	}
#else /* one root case */
	u3phy_init_addr.utmi_base = _MSTAR_U3UTMI_BASE;
	u3phy_init_addr.xhci_base = _MSTAR_XHCI_BASE;
	u3phy_init_addr.u3top_base = _MSTAR_U3TOP_BASE;
	u3phy_init_addr.bc_base = _MSTAR_U3BC_BASE;
#if defined(XHCI_PHY_MS28)
	#if defined(XHCI_2PORTS)
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_P0_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_P0_ATOP_BASE;
	u3phy_init_addr.u3dtop1_base = _MSTAR_U3PHY_P1_DTOP_BASE;
	u3phy_init_addr.u3atop1_base = _MSTAR_U3PHY_P1_ATOP_BASE;
	#else
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP_BASE;
	#endif
#elif defined(XHCI_PHY_MS40)
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP_BASE;
#else
	#error Unknown xHCI PHY
#endif
#endif

	flag |= mstar_xhci_get_flag(pdev);

	Mstar_xhc_init(&u3phy_init_addr, flag);

	driver = &mstar_plat_xhci_driver;

#if defined(CONFIG_OF)
	if (!pdev->dev.platform_data)
	{
		printk(KERN_WARNING "[USB][XHC] no platform_data, device tree coming\n");
	}

	if(IS_ENABLED(CONFIG_ARM64) && IS_ENABLED(CONFIG_ZONE_DMA))
	{
#if defined(XHC_DMA_BIT_MASK)
		dma_mask = XHC_DMA_BIT_MASK;
#else
		/* default: 32bit to mask lowest 4G address */
		dma_mask = DMA_BIT_MASK(32);
#endif
	} else
		dma_mask = DMA_BIT_MASK(64);

	if (dma_set_mask_and_coherent(&pdev->dev, dma_mask) ) {
		printk(KERN_ERR "[USB][XHC] cannot accept dma mask 0x%llx\n", dma_mask);
		return -EOPNOTSUPP;
	}

        printk(KERN_NOTICE "[USB][XHC] dma coherent_mask 0x%llx mask 0x%llx\n",
		pdev->dev.coherent_dma_mask, *pdev->dev.dma_mask);

	/* try to get irq from device tree */
	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
#else
	irq = platform_get_irq(pdev, 0);
#endif

	#if !defined(ENABLE_IRQ_REMAP)
	if (irq < 0)
		return -ENODEV;
	#endif

	#if defined(ENABLE_IRQ_REMAP)
	#ifdef ENABLE_SECOND_XHC
	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) )
		irq = MSTAR_XHCI_IRQ;
	else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN) )
		irq = MSTAR_XHCI2_IRQ;
	#else /* one root case */
	irq = MSTAR_XHCI_IRQ;
	#endif
	#endif

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

#ifdef ENABLE_SECOND_XHC
	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) )
		hcd->rsrc_start = _MSTAR_XHCI0_BASE;
	else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN) )
		hcd->rsrc_start = _MSTAR_XHCI1_BASE;
#else /* one root case */
	hcd->rsrc_start = _MSTAR_XHCI_BASE;
#endif
	hcd->rsrc_len = 0x4000<<1;

	hcd->regs = (void *)(uintptr_t)(hcd->rsrc_start);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		ret = -EFAULT;
		goto put_hcd;
	}

#ifdef ENABLE_SECOND_XHC
	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) ) {
		hcd->xhci_base = _MSTAR_XHCI0_BASE;
		hcd->u3top_base = _MSTAR_U3TOP0_BASE;
		hcd->utmi_base = _MSTAR_U3UTMI0_BASE;
		hcd->bc_base = _MSTAR_U3BC0_BASE;
		hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP0_BASE;
		hcd->port_index = 1;
	}
	else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN) ) {
		hcd->xhci_base = _MSTAR_XHCI1_BASE;
		hcd->u3top_base = _MSTAR_U3TOP1_BASE;
		hcd->utmi_base = _MSTAR_U3UTMI1_BASE;
		hcd->bc_base = _MSTAR_U3BC1_BASE;
		hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP1_BASE;
		hcd->port_index = 2;
	}
#else /* one root case */
	hcd->xhci_base = _MSTAR_XHCI_BASE;
	hcd->u3top_base = _MSTAR_U3TOP_BASE;
	hcd->utmi_base = _MSTAR_U3UTMI_BASE;
	hcd->bc_base = _MSTAR_U3BC_BASE;
#ifdef XHCI_PHY_MS28
	#ifdef XHCI_2PORTS
	hcd->u3dphy_base[0] = _MSTAR_U3PHY_P0_DTOP_BASE;
	hcd->u3dphy_base[1] = _MSTAR_U3PHY_P1_DTOP_BASE;
	#else
	hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP_BASE;
	#endif
#endif
#endif

	//Disable default setting.
	usb_bc_enable(hcd, false);

#if _UTMI_PWR_SAV_MODE_ENABLE
	usb_power_saving_enable(hcd, true);
#endif

#ifdef USB3_MAC_SRAM_POWER_DOWN_ENABLE
        usb30mac_sram_power_saving(hcd, true);
#endif

	xhci = hcd_to_xhci(hcd);
	xhci->main_hcd = hcd;
	xhci->shared_hcd = usb_create_shared_hcd(driver, &pdev->dev,
			dev_name(&pdev->dev), hcd);
	if (!xhci->shared_hcd) {
		ret = -ENOMEM;
		goto put_hcd;
	}

#ifdef ENABLE_SECOND_XHC
	if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) ) {
		xhci->shared_hcd->xhci_base = _MSTAR_XHCI0_BASE;
		xhci->shared_hcd->u3top_base = _MSTAR_U3TOP0_BASE;
		xhci->shared_hcd->utmi_base = _MSTAR_U3UTMI0_BASE;
		xhci->shared_hcd->bc_base = _MSTAR_U3BC0_BASE;
		xhci->shared_hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP0_BASE;
		xhci->shared_hcd->port_index = 1;
	}
	else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN) ) {
		xhci->shared_hcd->xhci_base = _MSTAR_XHCI1_BASE;
		xhci->shared_hcd->u3top_base = _MSTAR_U3TOP1_BASE;
		xhci->shared_hcd->utmi_base = _MSTAR_U3UTMI1_BASE;
		xhci->shared_hcd->bc_base = _MSTAR_U3BC1_BASE;
		xhci->shared_hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP1_BASE;
		xhci->shared_hcd->port_index = 2;
	}
#else /* one root case */
	xhci->shared_hcd->xhci_base = _MSTAR_XHCI_BASE;
	xhci->shared_hcd->u3top_base = _MSTAR_U3TOP_BASE;
	xhci->shared_hcd->utmi_base = _MSTAR_U3UTMI_BASE;
	xhci->shared_hcd->bc_base = _MSTAR_U3BC_BASE;
#ifdef XHCI_PHY_MS28
	#ifdef XHCI_2PORTS
		xhci->shared_hcd->u3dphy_base[0] = _MSTAR_U3PHY_P0_DTOP_BASE;
		xhci->shared_hcd->u3dphy_base[1] = _MSTAR_U3PHY_P1_DTOP_BASE;
	#else
		xhci->shared_hcd->u3dphy_base[0] = _MSTAR_U3PHY_DTOP_BASE;
	#endif
#endif
#endif

	/* IRQF_DISABLED was removed from kernel 4.1
	   commit d8bf368d0631d4bc2612d8bf2e4e8e74e620d0cc. */
	ret = usb_add_hcd(hcd, (unsigned int)irq, 0);
	if (ret)
		goto put_usb3_hcd;

	ret = usb_add_hcd(xhci->shared_hcd, (unsigned int)irq, 0);
	if (ret)
		goto dealloc_usb2_hcd;

#if defined(XHCI_ENABLE_PPC) && defined(XHCI_DISABLE_SS_MODE)
	//disable SS mode
	{
		int i;
		for (i=1; i<=xhci->num_usb3_ports; i++)
			xhci_hub_control(xhci->shared_hcd, ClearPortFeature, USB_PORT_FEAT_ENABLE, i, NULL, 0);
	}
	XHCI_enable_PPC(hcd->u3top_base);
#endif

//=================================
	create_xhci_sysfs_files(xhci);
//=================================

	return 0;


dealloc_usb2_hcd:
	usb_remove_hcd(hcd);

put_usb3_hcd:
	usb_put_hcd(xhci->shared_hcd);

put_hcd:
	usb_put_hcd(hcd);

	return ret;
}

static int xhci_mstar_plat_remove(struct platform_device *dev)
{
	struct usb_hcd	*hcd = platform_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);

//=================================
	remove_xhci_sysfs_files(xhci);
//=================================

	usb_remove_hcd(xhci->shared_hcd);
	usb_remove_hcd(hcd);

	usb_put_hcd(xhci->shared_hcd);
	usb_put_hcd(hcd);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int xhci_hcd_mstar_drv_suspend(struct device *dev)
{
	struct usb_hcd	*hcd = dev_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);


	printk("xhci_hcd_mstar_drv_suspend \n");

	return xhci_suspend(xhci, device_may_wakeup(dev));
}

static int xhci_hcd_mstar_drv_resume(struct device *dev)
{
	struct usb_hcd	*hcd = dev_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	unsigned int flag=0;
	int	   retval = 0;
	struct u3phy_addr_base u3phy_init_addr;
	struct platform_device *pdev = to_platform_device(dev);

	printk("xhci_hcd_mstar_drv_resume \n");

#ifdef ENABLE_SECOND_XHC /* 2 roots case */
	{
        #define NAME_LEN (16)

        if( 0==strncmp(pdev->name, "soc:Sstar-xhci-1", NAME_LEN) ) {
                u3phy_init_addr.utmi_base = _MSTAR_U3UTMI0_BASE;
                u3phy_init_addr.xhci_base = _MSTAR_XHCI0_BASE;
                u3phy_init_addr.u3top_base = _MSTAR_U3TOP0_BASE;
                u3phy_init_addr.bc_base = _MSTAR_U3BC0_BASE;
                u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP0_BASE;
                u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP0_BASE;
        }
        else if( 0==strncmp(pdev->name, "soc:Sstar-xhci-2", NAME_LEN) ) {
                u3phy_init_addr.utmi_base = _MSTAR_U3UTMI1_BASE;
                u3phy_init_addr.xhci_base = _MSTAR_XHCI1_BASE;
                u3phy_init_addr.u3top_base = _MSTAR_U3TOP1_BASE;
                u3phy_init_addr.bc_base = _MSTAR_U3BC1_BASE;
                u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP1_BASE;
                u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP1_BASE;
        }
	}
#else /* one root case */
	u3phy_init_addr.utmi_base = _MSTAR_U3UTMI_BASE;
	u3phy_init_addr.xhci_base = _MSTAR_XHCI_BASE;
	u3phy_init_addr.u3top_base = _MSTAR_U3TOP_BASE;
	u3phy_init_addr.bc_base = _MSTAR_U3BC_BASE;
#if defined(XHCI_PHY_MS28)
	#if defined(XHCI_2PORTS)
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_P0_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_P0_ATOP_BASE;
	u3phy_init_addr.u3dtop1_base = _MSTAR_U3PHY_P1_DTOP_BASE;
	u3phy_init_addr.u3atop1_base = _MSTAR_U3PHY_P1_ATOP_BASE;
	#else
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP_BASE;
	#endif
#elif defined(XHCI_PHY_MS40)
	u3phy_init_addr.u3dtop_base = _MSTAR_U3PHY_DTOP_BASE;
	u3phy_init_addr.u3atop_base = _MSTAR_U3PHY_ATOP_BASE;
#else
	#error Unknown xHCI PHY
#endif
#endif

	flag |= mstar_xhci_get_flag(pdev);

	Mstar_xhc_init(&u3phy_init_addr, flag);

	//Disable default setting.
	usb_bc_enable(hcd, false);

	retval = xhci_resume(xhci, false);
	if (retval) {
		printk(" xhci_resume FAIL : -0x%x !!", -retval);
		return retval;
	}

#if defined(XHCI_ENABLE_PPC) && defined(XHCI_DISABLE_SS_MODE)
	//disable SS mode
	{
		int i;
		for (i=1; i<=xhci->num_usb3_ports; i++)
			xhci_hub_control(xhci->shared_hcd, ClearPortFeature, USB_PORT_FEAT_ENABLE, i, NULL, 0);
	}
	XHCI_enable_PPC(hcd->u3top_base);
#endif

	return 0;
}
static const struct dev_pm_ops mstar_xhci_plat_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xhci_hcd_mstar_drv_suspend, xhci_hcd_mstar_drv_resume)
};
#define DEV_PM_OPS	(&mstar_xhci_plat_pm_ops)
#else
#define DEV_PM_OPS	NULL
#endif /* CONFIG_PM_SLEEP */

#if defined(CONFIG_OF)
static struct of_device_id mstar_xhci_of_device_ids[] = {
	{.compatible = "Sstar-xhci-1"},
	{},
};
#ifdef ENABLE_SECOND_XHC
static struct of_device_id mstar_xhci_1_of_device_ids[] = {
	{.compatible = "Sstar-xhci-2"},
	{},
};
#endif
#endif

static struct platform_driver xhci_mstar_driver = {

	.probe =	xhci_mstar_plat_probe,
	.remove =	xhci_mstar_plat_remove,

	.driver = {
		.name   = "Sstar-xhci-1",
		.pm = DEV_PM_OPS,
#if defined(CONFIG_OF)
		.of_match_table = mstar_xhci_of_device_ids,
#endif
	},
};
#ifdef ENABLE_SECOND_XHC
static struct platform_driver second_xhci_mstar_driver = {

        .probe =        xhci_mstar_plat_probe,
        .remove =       xhci_mstar_plat_remove,

        .driver = {
                .name   = "Sstar-xhci-2",
		.pm = DEV_PM_OPS,
#if defined(CONFIG_OF)
                .of_match_table = mstar_xhci_1_of_device_ids,
#endif
        },
};
#endif

MODULE_ALIAS("platform:mstar-xhci-hcd");

static const struct xhci_driver_overrides mstar_xhci_plat_overrides __initconst = {
	.extra_priv_size = sizeof(struct xhci_hcd),
};

int xhci_register_plat(void)
{
	int retval = 0;

	//xhci_init_driver(&mstar_plat_xhci_driver, &mstar_xhci_plat_overrides);
	mstar_plat_xhci_driver.hcd_priv_size += mstar_xhci_plat_overrides.extra_priv_size;

	retval = platform_driver_register(&xhci_mstar_driver);
	if (retval < 0)
		return retval;

#ifdef ENABLE_SECOND_XHC
	retval = platform_driver_register(&second_xhci_mstar_driver);
#endif

	return retval;
}

void xhci_unregister_plat(void)
{
	platform_driver_unregister(&xhci_mstar_driver);
#ifdef ENABLE_SECOND_XHC
	platform_driver_unregister(&second_xhci_mstar_driver);
#endif
}
