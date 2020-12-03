/*
* ehci-mstar.c- Sigmastar
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
#include <linux/platform_device.h>
#include "../../../usb/host/ehci.h"
#include "ehci-mstar.h"
#include "xhci-mstar.h"
#include "bc-mstar.h"
#include "mstar-lib.h"
#include "usb_pad.h"
//#include "../../sstar/include/ms_platform.h"

static struct hc_driver __read_mostly ehci_mstar_hc_driver;

#if defined(DYNAMIC_MIU_SIZE_MAPPING)
uintptr_t TO_MIU_SIZE(MIU_DDR_SIZE miu_size)
{
	uintptr_t retval;
	switch(miu_size)
	{
		case E_MIU_DDR_2MB:
			retval = 0x00200000;
			break;
		case E_MIU_DDR_4MB:
			retval = 0x00400000;
			break;
		case E_MIU_DDR_8MB:
			retval = 0x00800000;
			break;
		case E_MIU_DDR_16MB:
			retval = 0x01000000;
			break;
		case E_MIU_DDR_32MB:
			retval = 0x02000000;
			break;
		case E_MIU_DDR_64MB:
			retval = 0x04000000;
			break;
		case E_MIU_DDR_128MB:
			retval = 0x08000000;
			break;
		case E_MIU_DDR_256MB:
			retval = 0x10000000;
			break;
		case E_MIU_DDR_512MB:
			retval = 0x20000000;
			break;
		case E_MIU_DDR_1024MB:
			retval = 0x40000000;
			break;
		case E_MIU_DDR_2048MB:
			retval = 0x80000000;
			break;
		default:
			retval = 0;
	}
	return retval;
}

uintptr_t MIU0_PHY_BASE_ADDR;
uintptr_t MIU0_SIZE;
uintptr_t MIU1_PHY_BASE_ADDR;
uintptr_t MIU1_SIZE;
uintptr_t MIU2_PHY_BASE_ADDR;
uintptr_t MIU2_SIZE;
uintptr_t MIU3_PHY_BASE_ADDR;
uintptr_t MIU3_SIZE;
EXPORT_SYMBOL(MIU0_PHY_BASE_ADDR);
EXPORT_SYMBOL(MIU0_SIZE);
EXPORT_SYMBOL(MIU1_PHY_BASE_ADDR);
EXPORT_SYMBOL(MIU1_SIZE);
EXPORT_SYMBOL(MIU2_PHY_BASE_ADDR);
EXPORT_SYMBOL(MIU2_SIZE);
EXPORT_SYMBOL(MIU3_PHY_BASE_ADDR);
EXPORT_SYMBOL(MIU3_SIZE);

u8 USB_MIU_SEL0;
u8 USB_MIU_SEL1;
u8 USB_MIU_SEL2;
u8 USB_MIU_SEL3;
EXPORT_SYMBOL(USB_MIU_SEL0);
EXPORT_SYMBOL(USB_MIU_SEL1);
EXPORT_SYMBOL(USB_MIU_SEL2);
EXPORT_SYMBOL(USB_MIU_SEL3);

u8 TO_MIU_HW_SEL(uintptr_t phy_start, uintptr_t miu_size)
{
	u8 start_blk;
	u8 span_blk;

	if(miu_size == 0)
		return USB_MIU_SEL_NULL;

	if(miu_size%USB_MIU_SEL_BLK)
		printk("[USB] Warning miu size 0x%x not mutiple of blk 0x%x\n",
		    (unsigned int)miu_size, (unsigned int)USB_MIU_SEL_BLK);

	start_blk = phy_start / USB_MIU_SEL_BLK;
	span_blk = (miu_size-1) / USB_MIU_SEL_BLK;

	return ((start_blk+span_blk) << 4) |  start_blk;
}

int MIU_dynamic_size_init(void)
{
	MIU_DDR_SIZE size_miu0 = 0x0;
	MIU_DDR_SIZE size_miu1 = 0x0;
	MIU_DDR_SIZE size_miu2 = 0x0;
	MIU_DDR_SIZE size_miu3 = 0x0;
	uintptr_t configed_addr = (uintptr_t)USB_MIU_PHY_START;

	USB_MIU_SEL0 = 0x0;
	USB_MIU_SEL1 = 0x0;
	USB_MIU_SEL2 = 0x0;
	USB_MIU_SEL3 = 0x0;

	/* get MIUs size */
	if(!MDrv_MIU_Dram_ReadSize(0, &size_miu0))
		printk("[USB] request MIU0 Size fail\n");
	else
		MIU0_SIZE = TO_MIU_SIZE(size_miu0);

	if(!MDrv_MIU_Dram_ReadSize(1, &size_miu1))
		printk("[USB] request MIU1 Size fail\n");
	else
		MIU1_SIZE = TO_MIU_SIZE(size_miu1);

	if(!MDrv_MIU_Dram_ReadSize(2, &size_miu2))
		printk("[USB] request MIU2 Size fail\n");
	else
		MIU2_SIZE = TO_MIU_SIZE(size_miu2);

	if(!MDrv_MIU_Dram_ReadSize(3, &size_miu3))
		printk("[USB] request MIU3 Size fail\n");
	else
		MIU3_SIZE = TO_MIU_SIZE(size_miu3);

	printk("[USB] get RAW MIU size:\n  [0x%x] [0x%x] [0x%x] [0x%x]\n",
	    (unsigned int)MIU0_SIZE, (unsigned int)MIU1_SIZE,
	    (unsigned int)MIU2_SIZE, (unsigned int)MIU3_SIZE);

	/* set USB phy addr*/
	/* config MIU0 */
	MIU0_PHY_BASE_ADDR = configed_addr;

	if(configed_addr + MIU0_SIZE > USB_HW_MAX_SIZE)
	{
		MIU0_SIZE = USB_HW_MAX_SIZE - configed_addr;
	}

	configed_addr += ((MIU0_SIZE+USB_MIU_SEL_BLK-1)/USB_MIU_SEL_BLK)*USB_MIU_SEL_BLK;

	/* config MIU1 */
	MIU1_PHY_BASE_ADDR = configed_addr;

	if(configed_addr + MIU1_SIZE > USB_HW_MAX_SIZE)
	{
		MIU1_SIZE = USB_HW_MAX_SIZE - configed_addr;
	}

	configed_addr += ((MIU1_SIZE+USB_MIU_SEL_BLK-1)/USB_MIU_SEL_BLK)*USB_MIU_SEL_BLK;

	/* config MIU2 */
	MIU2_PHY_BASE_ADDR = configed_addr;

	if(configed_addr + MIU2_SIZE > USB_HW_MAX_SIZE)
	{
		MIU2_SIZE = USB_HW_MAX_SIZE - configed_addr;
	}

	configed_addr += ((MIU2_SIZE+USB_MIU_SEL_BLK-1)/USB_MIU_SEL_BLK)*USB_MIU_SEL_BLK;

	/* config MIU3 */
	MIU3_PHY_BASE_ADDR = configed_addr;

	if(configed_addr + MIU3_SIZE > USB_HW_MAX_SIZE)
	{
		MIU3_SIZE = USB_HW_MAX_SIZE - configed_addr;
	}

	printk("[USB] Config USB Phy:\n");
	printk("  MIU0: addr:%p szie 0x%x\n",
	  (void*)MIU0_PHY_BASE_ADDR, (unsigned int)MIU0_SIZE);
	printk("  MIU1: addr:%p szie 0x%x\n",
	  (void*)MIU1_PHY_BASE_ADDR, (unsigned int)MIU1_SIZE);
	printk("  MIU2: addr:%p szie 0x%x\n",
	  (void*)MIU2_PHY_BASE_ADDR, (unsigned int)MIU2_SIZE);
	printk("  MIU3: addr:%p szie 0x%x\n",
	  (void*)MIU3_PHY_BASE_ADDR, (unsigned int)MIU3_SIZE);

	USB_MIU_SEL0 = TO_MIU_HW_SEL(MIU0_PHY_BASE_ADDR, MIU0_SIZE);
	USB_MIU_SEL1 = TO_MIU_HW_SEL(MIU1_PHY_BASE_ADDR, MIU1_SIZE);
	USB_MIU_SEL2 = TO_MIU_HW_SEL(MIU2_PHY_BASE_ADDR, MIU2_SIZE);
	USB_MIU_SEL3 = TO_MIU_HW_SEL(MIU3_PHY_BASE_ADDR, MIU3_SIZE);

	return 0;
}
#endif

#if defined(ENABLE_USB_NEW_MIU_SLE)
void MIU_select_setting_ehc(uintptr_t USBC_base)
{
	printk("[USB] config miu select [%02x] [%02x] [%02x] [%02x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
	writeb(USB_MIU_SEL0, (void*)(USBC_base+0x14*2));	//Setting MIU0 segment
	writeb(USB_MIU_SEL1, (void*)(USBC_base+0x16*2));	//Setting MIU1 segment
	writeb(USB_MIU_SEL2, (void*)(USBC_base+0x17*2-1));	//Setting MIU2 segment
	writeb(USB_MIU_SEL3, (void*)(USBC_base+0x18*2));	//Setting MIU3 segment
	writeb(readb((void*)(USBC_base+0x19*2-1)) | BIT0, (void*)(USBC_base+0x19*2-1));	//Enable miu partition mechanism
#if !defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
	printk("[USB] enable miu lower bound address subtraction\n");
	writeb(readb((void*)(USBC_base+0x0F*2-1)) | BIT0, (void*)(USBC_base+0x0F*2-1));
#endif
}
#endif

//extern int Enable_USB_VBUS(int param);

void Titania3_series_start_ehc(uintptr_t UTMI_base, uintptr_t USBC_base, uintptr_t UHC_base, unsigned int flag)
{
	printk("Titania3_series_start_ehc start\n");

    writew(0x0001, (void*) (UTMI_base+0x0*2));
#if defined(ENABLE_USB_NEW_MIU_SLE)
	MIU_select_setting_ehc(USBC_base);
#endif
    writew(0x0C2F, (void*) (UTMI_base+0x8*2));
	if (flag & EHCFLAG_TESTPKG)
	{
		writew(0x2084, (void*)(UTMI_base+0x2*2));
		writew(0x8051, (void*)(UTMI_base+0x20*2));
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
	/* new HW term overwrite: on */
	writeb(readb((void*)(UTMI_base+0x52*2)) | (BIT5|BIT4|
		BIT3|BIT2|BIT1|BIT0), (void*) (UTMI_base+0x52*2));
#endif

	/* Turn on overwirte mode for D+/D- floating issue when UHC reset
	 * Before UHC reset, R_DP_PDEN = 1, R_DM_PDEN = 1, tern_ov = 1 */
	writeb(readb((void*)(UTMI_base+0x0*2)) | (BIT7|BIT6|BIT1), (void*) (UTMI_base+0x0*2));
	/* new HW term overwrite: on */
	writeb(readb((void*)(UTMI_base+0x52*2)) | (BIT5|BIT4|
		BIT3|BIT2|BIT1|BIT0), (void*) (UTMI_base+0x52*2));

#ifdef ENABLE_DOUBLE_DATARATE_SETTING
	writeb(readb((void*)(UTMI_base+0x0D*2-1)) | BIT0, (void*) (UTMI_base+0x0D*2-1)); // set reg_double_data_rate, To get better jitter performance
#endif
#ifdef ENABLE_UPLL_SETTING
	// sync code from eCos
	{
		u16 reg_t;

		reg_t = readw((void*)(UTMI_base+0x22*2));
		if ((reg_t & 0x10e0) != 0x10e0)
			writew(0x10e0, (void*)(UTMI_base+0x22*2));
		reg_t = readw((void*)(UTMI_base+0x24*2));
		if (reg_t != 0x1)
			writew(0x1, (void*)(UTMI_base+0x24*2));
	}
	//writeb(0, (void*) (UTMI_base+0x21*2-1));
	//writeb(0x10, (void*) (UTMI_base+0x23*2-1));
	//writeb(0x01, (void*) (UTMI_base+0x24*2));
#endif

	writeb(0x0a, (void*)(USBC_base)); // Disable MAC initial suspend, Reset UHC
	writeb(0x28, (void*)(USBC_base)); // Release UHC reset, enable UHC and OTG XIU function

	if (flag & EHCFLAG_DOUBLE_DATARATE)
	{
		if ((flag & EHCFLAG_DDR_MASK) == EHCFLAG_DDR_x15)
		{
			// Set usb bus = 480MHz x 1.5
			writeb(readb((void*)(UTMI_base+0x20*2)) | 0x76, (void*)(UTMI_base+0x20*2));
		}
		else if ((flag & EHCFLAG_DDR_MASK) == EHCFLAG_DDR_x18)
		{
			// Set usb bus = 480MHz x 1.8
			writeb(readb((void*)(UTMI_base+0x20*2)) | 0x8e, (void*)(UTMI_base+0x20*2));
		}
#if 0
		else if ((flag & EHCFLAG_DDR_MASK) == EHCFLAG_DDR_x20)
		{
			// Set usb bus = 480MHz x2
			writeb(readb((void*)(UTMI_base+0xd*2-1)) | 0x01, (void*)(UTMI_base+0xd*2-1));
		}
#endif
		/* Set slew rate control for overspeed (or 960MHz) */
		writeb(readb((void*)(UTMI_base+0x2c*2)) | BIT0, (void*) (UTMI_base+0x2c*2));
	}

	/* Init UTMI squelch level setting befor CA */
	if(UTMI_DISCON_LEVEL_2A & (BIT3|BIT2|BIT1|BIT0))
	{
		writeb((UTMI_DISCON_LEVEL_2A & (BIT3|BIT2|BIT1|BIT0)), (void*)(UTMI_base+0x2a*2));
		printk("[USB] init squelch level 0x%x\n", readb((void*)(UTMI_base+0x2a*2)));
	}

	writeb(readb((void*)(UTMI_base+0x3c*2)) | BIT0, (void*)(UTMI_base+0x3c*2)); // set CA_START as 1
	mdelay(1);

	writeb(readb((void*)(UTMI_base+0x3c*2)) & (u8)(~BIT0), (void*)(UTMI_base+0x3c*2)); // release CA_START

	while ((readb((void*)(UTMI_base+0x3c*2)) & BIT1) == 0);	// polling bit <1> (CA_END)

	if ((0xFFF0 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 )) ||
		(0x0000 == (readw((void*)(UTMI_base+0x3C*2)) & 0xFFF0 ))  )
		printk("WARNING: CA Fail !! \n");

	if (flag & EHCFLAG_DPDM_SWAP)
		writeb(readb((void*)(UTMI_base+0x0b*2-1)) | BIT5, (void*)(UTMI_base+0x0b*2-1)); // dp dm swap
#if defined(CONFIG_USB_MS_OTG) || defined(CONFIG_USB_MS_OTG_MODULE)
	if(flag & EHCFLAG_ENABLE_OTG)
	{
		// let OTG driver to handle the UTMI switch control
	}
	else
#endif
		writeb((u8)((readb((void*)(USBC_base+0x02*2)) & ~BIT1) | BIT0), (void*)(USBC_base+0x02*2)); // UHC select enable

	writeb(readb((void*)(UHC_base+0x40*2)) & (u8)(~BIT4), (void*)(UHC_base+0x40*2)); // 0: VBUS On.
	udelay(1); // delay 1us

	writeb(readb((void*)(UHC_base+0x40*2)) | BIT3, (void*)(UHC_base+0x40*2)); // Active HIGH

	/* Turn on overwirte mode for D+/D- floating issue when UHC reset
	 * After UHC reset, disable overwrite bits */
	writeb(readb((void*)(UTMI_base+0x0*2)) & (u8)(~(BIT7|BIT6|BIT1)), (void*) (UTMI_base+0x0*2));
	/* new HW term overwrite: off */
	writeb(readb((void*)(UTMI_base+0x52*2)) & (u8)(~(BIT5|BIT4|
		BIT3|BIT2|BIT1|BIT0)), (void*) (UTMI_base+0x52*2));

	/* improve the efficiency of USB access MIU when system is busy */
	writeb(readb((void*)(UHC_base+0x81*2-1)) | (BIT0 | BIT1 | BIT2 | BIT3 | BIT7), (void*)(UHC_base+0x81*2-1));

	writeb((u8)((readb((void*)(UTMI_base+0x06*2)) & ~BIT5) | BIT6), (void*)(UTMI_base+0x06*2)); // reg_tx_force_hs_current_enable

	writeb((u8)((readb((void*)(UTMI_base+0x03*2-1)) & ~BIT4) | (BIT3 | BIT5)), (void*)(UTMI_base+0x03*2-1)); // Disconnect window select

	writeb(readb((void*)(UTMI_base+0x07*2-1)) & (u8)(~BIT1), (void*)(UTMI_base+0x07*2-1)); // Disable improved CDR

#if defined(ENABLE_UTMI_240_AS_120_PHASE_ECO)
	#if defined(UTMI_240_AS_120_PHASE_ECO_INV)
	writeb(readb((void*)(UTMI_base+0x08*2)) & (u8)(~BIT3), (void*)(UTMI_base+0x08*2)); //Set sprcial value for Eiffel USB analog LIB issue
	#else
	/* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
	writeb(readb((void*)(UTMI_base+0x08*2)) | BIT3, (void*)(UTMI_base+0x08*2));
	#endif
#endif

	writeb(readb((void*)(UTMI_base+0x09*2-1)) | (BIT0 | BIT7), (void*)(UTMI_base+0x09*2-1)); // UTMI RX anti-dead-loc, ISI effect improvement

	if ((flag & EHCFLAG_DOUBLE_DATARATE)==0)
	    writeb(readb((void*)(UTMI_base+0x0b*2-1)) | BIT7, (void*)(UTMI_base+0x0b*2-1)); // TX timing select latch path

	writeb(readb((void*)(UTMI_base+0x15*2-1)) | BIT5, (void*)(UTMI_base+0x15*2-1)); // Chirp signal source select

#if defined(ENABLE_UTMI_55_INTERFACE)
	writeb(readb((void*)(UTMI_base+0x15*2-1)) | BIT6, (void*)(UTMI_base+0x15*2-1)); // change to 55 interface
#endif

	/* new HW chrip design, defualt overwrite to reg_2A */
	writeb(readb((void*)(UTMI_base+0x40*2)) & (u8)(~BIT4), (void*)(UTMI_base+0x40*2));

	/* Init UTMI disconnect level setting */
	writeb(UTMI_DISCON_LEVEL_2A, (void*)(UTMI_base+0x2a*2));

#if defined(ENABLE_NEW_HW_CHRIP_PATCH)
	/* Init chrip detect level setting */
	writeb(UTMI_CHIRP_DCT_LEVEL_42, (void*)(UTMI_base+0x42*2));
	/* enable HW control chrip/disconnect level */
	writeb(readb((void*)(UTMI_base+0x40*2)) & (u8)(~BIT3), (void*)(UTMI_base+0x40*2));
#endif

	/* Init UTMI eye diagram parameter setting */
	writeb(readb((void*)(UTMI_base+0x2c*2)) | UTMI_EYE_SETTING_2C, (void*)(UTMI_base+0x2c*2));
	writeb(readb((void*)(UTMI_base+0x2d*2-1)) | UTMI_EYE_SETTING_2D, (void*)(UTMI_base+0x2d*2-1));
	writeb(readb((void*)(UTMI_base+0x2e*2)) | UTMI_EYE_SETTING_2E, (void*)(UTMI_base+0x2e*2));
	writeb(readb((void*)(UTMI_base+0x2f*2-1)) | UTMI_EYE_SETTING_2F, (void*)(UTMI_base+0x2f*2-1));

#if defined(ENABLE_LS_CROSS_POINT_ECO)
	/* Enable deglitch SE0 (low-speed cross point) */
	writeb(readb((void*)(UTMI_base+LS_CROSS_POINT_ECO_OFFSET)) | LS_CROSS_POINT_ECO_BITSET, (void*)(UTMI_base+LS_CROSS_POINT_ECO_OFFSET));
#endif

#if defined(ENABLE_PWR_NOISE_ECO)
	/* Enable use eof2 to reset state machine (power noise) */
	writeb(readb((void*)(USBC_base+0x02*2)) | BIT6, (void*)(USBC_base+0x02*2));
#endif

#if defined(ENABLE_TX_RX_RESET_CLK_GATING_ECO)
	/* Enable hw auto deassert sw reset(tx/rx reset) */
	writeb(readb((void*)(UTMI_base+TX_RX_RESET_CLK_GATING_ECO_OFFSET)) | TX_RX_RESET_CLK_GATING_ECO_BITSET, (void*)(UTMI_base+TX_RX_RESET_CLK_GATING_ECO_OFFSET));
#endif

#if defined(ENABLE_LOSS_SHORT_PACKET_INTR_ECO)
	/* Enable patch for the assertion of interrupt(Lose short packet interrupt) */
	#if defined(LOSS_SHORT_PACKET_INTR_ECO_OPOR)
	writeb(readb((void*)(USBC_base+LOSS_SHORT_PACKET_INTR_ECO_OFFSET)) | LOSS_SHORT_PACKET_INTR_ECO_BITSET, (void*)(USBC_base+LOSS_SHORT_PACKET_INTR_ECO_OFFSET));
	#else
	writeb(readb((void*)(USBC_base+0x04*2)) & (u8)(~BIT7), (void*)(USBC_base+0x04*2));
	#endif
#endif

#if defined(ENABLE_BABBLE_ECO)
	/* Enable add patch to Period_EOF1(babble problem) */
	writeb(readb((void*)(USBC_base+0x04*2)) | BIT6, (void*)(USBC_base+0x04*2));
#endif

#if defined(ENABLE_MDATA_ECO)
	/* Enable short packet MDATA in Split transaction clears ACT bit (LS dev under a HS hub) */
	writeb(readb((void*)(USBC_base+MDATA_ECO_OFFSET)) | MDATA_ECO_BITSET, (void*) (USBC_base+MDATA_ECO_OFFSET));
#endif

#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
	/* Change override to hs_txser_en.  Dm always keep high issue */
	writeb(readb((void*)(UTMI_base+0x10*2)) | BIT6, (void*) (UTMI_base+0x10*2));
#endif

#if defined(ENABLE_HS_CONNECTION_FAIL_INTO_VFALL_ECO)
	/* HS connection fail problem (Gate into VFALL state) */
	writeb(readb((void*)(USBC_base+0x11*2-1)) | BIT1, (void*)(USBC_base+0x11*2-1));
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
	/* new HW term overwrite: on */
	writeb(readb((void*)(UTMI_base+0x52*2)) | (BIT5|BIT4|
		BIT3|BIT2|BIT1|BIT0), (void*) (UTMI_base+0x52*2));
#endif

#if defined (ENABLE_PV2MI_BRIDGE_ECO)
	writeb(readb((void*)(USBC_base+0x0a*2)) | BIT6, (void*)(USBC_base+0x0a*2));
#endif

#if _USB_ANALOG_RX_SQUELCH_PATCH
	/* squelch level adjust by calibration value */
	{
	unsigned int ca_da_ov, ca_db_ov, ca_tmp;

	ca_tmp = readw((void*)(UTMI_base+0x3c*2));
	ca_da_ov = (((ca_tmp >> 4) & 0x3f) - 5) + 0x40;
	ca_db_ov = (((ca_tmp >> 10) & 0x3f) - 5) + 0x40;
	printk("[%x]-5 -> (ca_da_ov, ca_db_ov)=(%x,%x)\n", ca_tmp, ca_da_ov, ca_db_ov);
	writeb(ca_da_ov ,(void*)(UTMI_base+0x3B*2-1));
	writeb(ca_db_ov ,(void*)(UTMI_base+0x24*2));
	}
#endif

#if _USB_MINI_PV2MI_BURST_SIZE
	writeb(readb((void*)(USBC_base+0x0b*2-1)) & ~(BIT1|BIT2|BIT3|BIT4), (void*)(USBC_base+0x0b*2-1));
#endif

#if defined(ENABLE_UHC_PREAMBLE_ECO)
	/* [7]: reg_etron_en, to enable utmi Preamble function */
	writeb(readb((void*)(UTMI_base+0x3f*2-1)) | BIT7, (void*) (UTMI_base+0x3f*2-1));

	/* [3:]: reg_preamble_en, to enable Faraday Preamble */
	writeb(readb((void*)(USBC_base+0x0f*2-1)) | BIT3, (void*)(USBC_base+0x0f*2-1));

	/* [0]: reg_preamble_babble_fix, to patch Babble occurs in Preamble */
	writeb(readb((void*)(USBC_base+0x10*2)) | BIT0, (void*)(USBC_base+0x10*2));

	/* [1]: reg_preamble_fs_within_pre_en, to patch FS crash problem */
	writeb(readb((void*)(USBC_base+0x10*2)) | BIT1, (void*)(USBC_base+0x10*2));

	/* [2]: reg_fl_sel_override, to override utmi to have FS drive strength */
	writeb(readb((void*)(UTMI_base+0x03*2-1)) | BIT2, (void*) (UTMI_base+0x03*2-1));
#endif

#if defined(ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO)
	/* Don't close RUN bit when device disconnect */
	writeb(readb((void*)(UHC_base+0x34*2)) | BIT7, (void*)(UHC_base+0x34*2));
#endif

#if _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH
	/* Enabe PVCI i_miwcplt wait for mi2uh_last_done_z */
	writeb(readb((void*)(UHC_base+0x83*2-1)) | BIT4, (void*)(UHC_base+0x83*2-1));
#endif

#if defined(ENABLE_UHC_EXTRA_HS_SOF_ECO)
	/* Extra HS SOF after bus reset */
	writeb(readb((void*)(UHC_base+0x8C*2)) | BIT0, (void*)(UHC_base+0x8C*2));
#endif

	/* Enable HS ISO IN Camera Cornor case ECO function */
#if defined(HS_ISO_IN_ECO_OFFSET)
		writeb(readb((void*)(USBC_base+HS_ISO_IN_ECO_OFFSET)) | HS_ISO_IN_ECO_BITSET, (void*) (USBC_base+HS_ISO_IN_ECO_OFFSET));
#else
	writeb(readb((void*)(USBC_base+0x13*2-1)) | BIT0, (void*)(USBC_base+0x13*2-1));
#endif

#if defined(ENABLE_DISCONNECT_SPEED_REPORT_RESET_ECO)
	/* UHC speed type report should be reset by device disconnection */
	writeb(readb((void*)(USBC_base+0x20*2)) | BIT0, (void*)(USBC_base+0x20*2));
#endif

#if defined(ENABLE_BABBLE_PCD_ONE_PULSE_TRIGGER_ECO)
	/* Port Change Detect (PCD) is triggered by babble.
	 * Pulse trigger will not hang this condition.
	 */
	writeb(readb((void*)(USBC_base+0x20*2)) | BIT1, (void*)(USBC_base+0x20*2));
#endif

#if defined(ENABLE_HC_RESET_FAIL_ECO)
	/* generation of hhc_reset_u */
	writeb(readb((void*)(USBC_base+0x20*2)) | BIT2, (void*)(USBC_base+0x20*2));
#endif

#if defined(ENABLE_INT_AFTER_WRITE_DMA_ECO)
	/* DMA interrupt after the write back of qTD */
	writeb(readb((void*)(USBC_base+0x20*2)) | BIT3, (void*)(USBC_base+0x20*2));
#endif

#if defined(ENABLE_DISCONNECT_HC_KEEP_RUNNING_ECO)
	/* EHCI keeps running when device is disconnected */
	writeb(readb((void*)(USBC_base+0x19*2-1)) | BIT3, (void*)(USBC_base+0x19*2-1));
#endif

#if !defined(_EHC_SINGLE_SOF_TO_CHK_DISCONN)
	writeb(0x05, (void*)(USBC_base+0x03*2-1)); //Use 2 SOFs to check disconnection
#endif

#if defined(ENABLE_SRAM_CLK_GATING_ECO)
	/* do SRAM clock gating automatically to save power */
	writeb(readb((void*)(USBC_base+0x20*2)) & (u8)(~BIT4), (void*)(USBC_base+0x20*2));
#endif

#if defined (ENABLE_INTR_SITD_CS_IN_ZERO_ECO)
	/* Enable interrupt in sitd cs in zero packet */
	writeb(readb((void*)(USBC_base+0x11*2-1)) | BIT7, (void*)(USBC_base+0x11*2-1));
#endif

	if (flag & EHCFLAG_TESTPKG)
	{
		writew(0x0600, (void*) (UTMI_base+0x14*2));
		writew(0x0078, (void*) (UTMI_base+0x10*2));
		writew(0x0bfe, (void*) (UTMI_base+0x32*2));
	}
}

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

#if defined(CONFIG_OF)
extern unsigned int irq_of_parse_and_map(struct device_node *node, int index);
#endif

#if MP_USB_MSTAR && _USB_VBUS_RESET_PATCH
 int vbus_gpio_of_parse_and_map(struct device_node *node)
{
    return Get_USB_VBUS_Pin(node);
}
 EXPORT_SYMBOL_GPL(vbus_gpio_of_parse_and_map);
#endif

/**
 * usb_ehci_au1xxx_probe - initialize Au1xxx-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */

int ehci_hcd_mstar_drv_probe(struct platform_device *dev)
{
	int retval=0;
	int irq = -1;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	unsigned int flag = 0;
	u64 dma_mask;
	int param = 0;
#if defined(CONFIG_OF)
	u32 val;
	int ret;
	struct device_node *node = dev->dev.of_node;
#endif
#ifdef ENABLE_SECOND_XHC
	#ifdef _MSTAR_EHC0_COMP_PORT
	struct comp_port ehc0_comp = {_MSTAR_EHC0_COMP_PORT, _MSTAR_EHC0_COMP_U3TOP_BASE, _MSTAR_EHC0_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC1_COMP_PORT
	struct comp_port ehc1_comp = {_MSTAR_EHC1_COMP_PORT, _MSTAR_EHC1_COMP_U3TOP_BASE, _MSTAR_EHC1_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC2_COMP_PORT
	struct comp_port ehc2_comp = {_MSTAR_EHC2_COMP_PORT, _MSTAR_EHC2_COMP_U3TOP_BASE, _MSTAR_EHC2_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC3_COMP_PORT
	struct comp_port ehc3_comp = {_MSTAR_EHC3_COMP_PORT, _MSTAR_EHC3_COMP_U3TOP_BASE, _MSTAR_EHC3_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC4_COMP_PORT
	struct comp_port ehc4_comp = {_MSTAR_EHC4_COMP_PORT, _MSTAR_EHC4_COMP_U3TOP_BASE, _MSTAR_EHC4_COMP_PORT_INDEX};
	#endif
#else /* one root case */
	#ifdef _MSTAR_EHC0_COMP_PORT
	struct comp_port ehc0_comp = {_MSTAR_EHC0_COMP_PORT, _MSTAR_U3TOP_BASE, _MSTAR_EHC0_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC1_COMP_PORT
	struct comp_port ehc1_comp = {_MSTAR_EHC1_COMP_PORT, _MSTAR_U3TOP_BASE, _MSTAR_EHC1_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC2_COMP_PORT
	struct comp_port ehc2_comp = {_MSTAR_EHC2_COMP_PORT, _MSTAR_U3TOP_BASE, _MSTAR_EHC2_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC3_COMP_PORT
	struct comp_port ehc3_comp = {_MSTAR_EHC3_COMP_PORT, _MSTAR_U3TOP_BASE, _MSTAR_EHC3_COMP_PORT_INDEX};
	#endif
	#ifdef _MSTAR_EHC4_COMP_PORT
	struct comp_port ehc4_comp = {_MSTAR_EHC4_COMP_PORT, _MSTAR_U3TOP_BASE, _MSTAR_EHC4_COMP_PORT_INDEX};
	#endif
#endif

	if (usb_disabled())
		return -ENODEV;

#ifdef ENABLE_CHIPTOP_PERFORMANCE_SETTING
	int chipVER = readw((void *)(MSTAR_CHIP_TOP_BASE+0xCE*2));
	/* chip top performance tuning [11:9] = 0xe00 */
	if (chipVER == 0x101) // U02
		writew(readw((void*)(MSTAR_CHIP_TOP_BASE+0x46*2)) | 0xe00,
						(void*) (MSTAR_CHIP_TOP_BASE+0x46*2));
#endif
	#define NAME_LEN (16)
	if( 0==strncmp(dev->name, "soc:Sstar-ehci-1", NAME_LEN) )
	{
		printk("Sstar-ehci-1 H.W init\n");
	#if _USB_UTMI_DPDM_SWAP_P0
		flag |= EHCFLAG_DPDM_SWAP;
	#endif
	#ifdef _USB_ENABLE_OTG_P0
		flag |= EHCFLAG_ENABLE_OTG;
	#endif
		#if defined(CONFIG_OF)
		ret = of_property_read_u32(node, "dpdm_swap", &val);
		if(ret == 0)
			flag |= val;//EHCFLAG_DPDM_SWAP
		#endif
		param = 1&0xFF; // param = (port_num << 16) | (vbus_en & 0xFF)
		Enable_USB_VBUS(param);
		Titania3_series_start_ehc(_MSTAR_UTMI0_BASE, _MSTAR_USBC0_BASE, _MSTAR_UHC0_BASE, flag);
	}
#if !defined(DISABLE_SECOND_EHC)
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-2", NAME_LEN) )
	{
		printk("Sstar-ehci-2 H.W init\n");
	#if _USB_UTMI_DPDM_SWAP_P1
		flag |= EHCFLAG_DPDM_SWAP;
	#endif
	#ifdef _USB_ENABLE_OTG_P1
		flag |= EHCFLAG_ENABLE_OTG;
	#endif
	#if defined(CONFIG_OF)
		ret = of_property_read_u32(node, "dpdm_swap", &val);
		if(ret == 0)
			flag |= val;//EHCFLAG_DPDM_SWAP
	#endif
		param = ((1<<16) | (1&0xFF)); // param = (port_num << 16) | (vbus_en & 0xFF)
		Enable_USB_VBUS(param);
		Titania3_series_start_ehc(_MSTAR_UTMI1_BASE, _MSTAR_USBC1_BASE, _MSTAR_UHC1_BASE, flag);
	}
#endif
#ifdef ENABLE_THIRD_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-3", NAME_LEN) )
	{
		printk("Sstar-ehci-3 H.W init\n");
#if _USB_UTMI_DPDM_SWAP_P2
		flag |= EHCFLAG_DPDM_SWAP;
#endif
	#ifdef _USB_ENABLE_OTG_P2
		flag |= EHCFLAG_ENABLE_OTG;
	#endif
#if defined(CONFIG_OF)
		ret = of_property_read_u32(node, "dpdm_swap", &val);
		if(ret == 0)
		    flag |= val;//EHCFLAG_DPDM_SWAP
#endif
    param = ((2<<16) | (1&0xFF)); // param = (port_num << 16) | (vbus_en & 0xFF)
    Enable_USB_VBUS(param);
		Titania3_series_start_ehc(_MSTAR_UTMI2_BASE, _MSTAR_USBC2_BASE, _MSTAR_UHC2_BASE, flag );
	}
#endif
#ifdef ENABLE_FOURTH_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-4", NAME_LEN) )
	{
		printk("Sstar-ehci-4 H.W init\n");
	#ifdef _USB_ENABLE_OTG_P3
		flag |= EHCFLAG_ENABLE_OTG;
	#endif
		Titania3_series_start_ehc(_MSTAR_UTMI3_BASE, _MSTAR_USBC3_BASE, _MSTAR_UHC3_BASE, flag );
	}
#endif
#ifdef ENABLE_FIFTH_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-5", NAME_LEN) )
	{
		printk("Sstar-ehci-5 H.W init\n");
	#ifdef _USB_ENABLE_OTG_P4
		flag |= EHCFLAG_ENABLE_OTG;
	#endif
		Titania3_series_start_ehc(_MSTAR_UTMI4_BASE, _MSTAR_USBC4_BASE, _MSTAR_UHC4_BASE, flag );
	}
#endif

#if defined(CONFIG_OF)
	if (!dev->dev.platform_data)
	{
		printk(KERN_WARNING "[USB] no platform_data, device tree coming\n");
	}

	if (!dev->dev.dma_mask)
		dev->dev.dma_mask = &dev->dev.coherent_dma_mask;

	if(IS_ENABLED(CONFIG_ARM64) && IS_ENABLED(CONFIG_ZONE_DMA))
	{
#if defined(EHC_DMA_BIT_MASK)
		dma_mask = EHC_DMA_BIT_MASK;
#else
		/* default: 32bit to mask lowest 4G address */
		dma_mask = DMA_BIT_MASK(32);
#endif
	} else
		dma_mask = DMA_BIT_MASK(64);

	if(dma_set_mask(&dev->dev, dma_mask) ||
		dma_set_coherent_mask(&dev->dev, dma_mask))
	{
		printk(KERN_ERR "[USB][EHC] cannot accept dma mask 0x%llx\n", dma_mask);
		return -EOPNOTSUPP;
	}

	printk(KERN_NOTICE "[USB][EHC] dma coherent_mask 0x%llx mask 0x%llx\n",
		dev->dev.coherent_dma_mask, *dev->dev.dma_mask);

	/* try to get irq from device tree */
	irq = irq_of_parse_and_map(dev->dev.of_node, 0);
#else
	if (dev->resource[2].flags != IORESOURCE_IRQ) {
		printk(KERN_WARNING "resource[2] is not IORESOURCE_IRQ");
	}
	else
	{
		irq =  dev->resource[2].start;
	}
#endif

#if !defined(ENABLE_IRQ_REMAP)
	if(irq <= 0)
	{
		printk(KERN_ERR "[USB] can not get irq for %s\n", dev->name);
		return -ENODEV;
	}
#endif

	hcd = usb_create_hcd(&ehci_mstar_hc_driver, &dev->dev, "mstar");
	if (!hcd)
		return -ENOMEM;

	/* ehci_hcd_init(hcd_to_ehci(hcd)); */
	if( 0==strncmp(dev->name, "soc:Sstar-ehci-1", NAME_LEN) )
	{
		hcd->port_index = 1;
		hcd->utmi_base = _MSTAR_UTMI0_BASE;
		hcd->ehc_base = _MSTAR_UHC0_BASE;
		hcd->usbc_base = _MSTAR_USBC0_BASE;
		hcd->bc_base = _MSTAR_BC0_BASE;
		#ifdef _MSTAR_EHC0_COMP_PORT
		hcd->companion = ehc0_comp;
		#endif
		#ifdef ENABLE_IRQ_REMAP
		irq = MSTAR_EHC1_IRQ;
		#endif
	}
#if !defined(DISABLE_SECOND_EHC)
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-2", NAME_LEN) )
	{
		hcd->port_index = 2;
		hcd->utmi_base = _MSTAR_UTMI1_BASE;
		hcd->ehc_base = _MSTAR_UHC1_BASE;
		hcd->usbc_base = _MSTAR_USBC1_BASE;
		hcd->bc_base = _MSTAR_BC1_BASE;
		#ifdef _MSTAR_EHC1_COMP_PORT
		hcd->companion = ehc1_comp;
		#endif
		#ifdef ENABLE_IRQ_REMAP
		irq = MSTAR_EHC2_IRQ;
		#endif
	}
#endif
#ifdef ENABLE_THIRD_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-3", NAME_LEN) )
	{
		hcd->port_index = 3;
		hcd->utmi_base = _MSTAR_UTMI2_BASE;
		hcd->ehc_base = _MSTAR_UHC2_BASE;
		hcd->usbc_base = _MSTAR_USBC2_BASE;
		hcd->bc_base = _MSTAR_BC2_BASE;
		#ifdef _MSTAR_EHC2_COMP_PORT
		hcd->companion = ehc2_comp;
		#endif
		#ifdef ENABLE_IRQ_REMAP
		irq = MSTAR_EHC3_IRQ;
		#endif
	}
#endif
#ifdef ENABLE_FOURTH_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-4", NAME_LEN) )
	{
		hcd->port_index = 4;
		hcd->utmi_base = _MSTAR_UTMI3_BASE;
		hcd->ehc_base = _MSTAR_UHC3_BASE;
		hcd->usbc_base = _MSTAR_USBC3_BASE;
		hcd->bc_base = _MSTAR_BC3_BASE;
		#ifdef _MSTAR_EHC3_COMP_PORT
		hcd->companion = ehc3_comp;
		#endif
		#ifdef ENABLE_IRQ_REMAP
		irq = MSTAR_EHC4_IRQ;
		#endif
	}
#endif
#ifdef ENABLE_FIFTH_EHC
	else if( 0==strncmp(dev->name, "soc:Sstar-ehci-5", NAME_LEN) )
	{
		hcd->port_index = 5;
		hcd->utmi_base = _MSTAR_UTMI4_BASE;
		hcd->ehc_base = _MSTAR_UHC4_BASE;
		hcd->usbc_base = _MSTAR_USBC4_BASE;
		hcd->bc_base = _MSTAR_BC4_BASE;
		#ifdef _MSTAR_EHC4_COMP_PORT
		hcd->companion = ehc4_comp;
		#endif
		#ifdef ENABLE_IRQ_REMAP
		irq = MSTAR_EHC5_IRQ;
		#endif
	}
#endif

	hcd->rsrc_start = hcd->ehc_base;
	hcd->rsrc_len = (0xfe<<1);
	hcd->has_tt = 1;
#if _USB_XIU_TIMEOUT_PATCH
	hcd->usb_reset_lock = __SPIN_LOCK_UNLOCKED(hcd->usb_reset_lock);
#endif

	hcd->regs = (void *)hcd->rsrc_start; // tony
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err1;
	}

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = (struct ehci_regs *)((uintptr_t)hcd->regs + HC_LENGTH(ehci, ehci_readl(ehci, &ehci->caps->hc_capbase)));

	//printk("\nDean: [%s] ehci->regs: 0x%x\n", __FILE__, (unsigned int)ehci->regs);
	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	//Disable default setting
	usb_bc_enable(hcd, false);

#if _UTMI_PWR_SAV_MODE_ENABLE
	usb_power_saving_enable(hcd, true);
#endif

#ifdef USB_MAC_SRAM_POWER_DOWN_ENABLE
	usb20mac_sram_power_saving(hcd, true);
#endif

	printk(KERN_INFO "[USB] %s irq --> %d\n", dev->name, irq);

    /* IRQF_DISABLED was removed from kernel 4.1
       commit d8bf368d0631d4bc2612d8bf2e4e8e74e620d0cc. */
	retval = usb_add_hcd(hcd, irq, 0);

	hcd->root_port_devnum=0;
	hcd->enum_port_flag=0;
	hcd->enum_dbreset_flag=0;
	hcd->lock_usbreset=__SPIN_LOCK_UNLOCKED(hcd->lock_usbreset);

	//usb_add_hcd(hcd, dev->resource[2].start, IRQF_DISABLED | IRQF_SHARED);
	if (retval == 0)
		return retval;
err1:
	usb_put_hcd(hcd);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_ehci_hcd_au1xxx_remove - shutdown processing for Au1xxx-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_ehci_hcd_au1xxx_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
static int ehci_hcd_mstar_drv_remove(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	return 0;
}

#ifdef CONFIG_PM
static int ehci_hcd_mstar_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	printk("ehci_hcd_mstar_platform_suspend...port %d\n", hcd->port_index);
	return ehci_suspend(hcd, false);
}

static int ehci_hcd_mstar_drv_resume(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	unsigned int flag = 0;

#if (_USB_UTMI_DPDM_SWAP_P0) || (_USB_UTMI_DPDM_SWAP_P1)
	flag |= (hcd->port_index == 1 || hcd->port_index == 2) ? EHCFLAG_DPDM_SWAP : 0;
#endif

	printk("ehci_hcd_mstar_platform_resume...port %d\n", hcd->port_index);

	Titania3_series_start_ehc(hcd->utmi_base, hcd->usbc_base, hcd->ehc_base, flag);

	//Disable default setting
	usb_bc_enable(hcd, false);

	ehci_resume(hcd, false);
	return 0;
}
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_OF)
static struct of_device_id mstar_ehci_1_of_device_ids[] = {
	{.compatible = "Sstar-ehci-1"},
	{},
};

#if !defined(DISABLE_SECOND_EHC)
static struct of_device_id mstar_ehci_2_of_device_ids[] = {
	{.compatible = "Sstar-ehci-2"},
	{},
};
#endif

#ifdef ENABLE_THIRD_EHC
static struct of_device_id mstar_ehci_3_of_device_ids[] = {
	{.compatible = "Sstar-ehci-3"},
	{},
};
#endif

#ifdef ENABLE_FOURTH_EHC
static struct of_device_id mstar_ehci_4_of_device_ids[] = {
	{.compatible = "Sstar-ehci-4"},
	{},
};
#endif

#ifdef ENABLE_FIFTH_EHC
static struct of_device_id mstar_ehci_5_of_device_ids[] = {
	{.compatible = "Sstar-ehci-5"},
	{},
};
#endif
#endif

static struct platform_driver ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
#ifdef CONFIG_PM
	.suspend	= ehci_hcd_mstar_drv_suspend,
	.resume		= ehci_hcd_mstar_drv_resume,
#endif
	.driver = {
		.name	= "Sstar-ehci-1",
#if defined(CONFIG_OF)
		.of_match_table = mstar_ehci_1_of_device_ids,
#endif
//		.bus	= &platform_bus_type,
	}
};
#if !defined(DISABLE_SECOND_EHC)
static struct platform_driver second_ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
#ifdef CONFIG_PM
	.suspend	= ehci_hcd_mstar_drv_suspend,
	.resume		= ehci_hcd_mstar_drv_resume,
#endif
	.driver = {
		.name 	= "Sstar-ehci-2",
#if defined(CONFIG_OF)
		.of_match_table = mstar_ehci_2_of_device_ids,
#endif
//		.bus	= &platform_bus_type,
	}
};
#endif
#ifdef ENABLE_THIRD_EHC
static struct platform_driver third_ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
#ifdef CONFIG_PM
	.suspend	= ehci_hcd_mstar_drv_suspend,
	.resume		= ehci_hcd_mstar_drv_resume,
#endif
	.driver = {
		.name 	= "Sstar-ehci-3",
#if defined(CONFIG_OF)
		.of_match_table = mstar_ehci_3_of_device_ids,
#endif
//		.bus	= &platform_bus_type,
	}
};
#endif
#ifdef ENABLE_FOURTH_EHC
static struct platform_driver fourth_ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
#ifdef CONFIG_PM
	.suspend	= ehci_hcd_mstar_drv_suspend,
	.resume		= ehci_hcd_mstar_drv_resume,
#endif
	.driver = {
		.name 	= "Sstar-ehci-4",
#if defined(CONFIG_OF)
		.of_match_table = mstar_ehci_4_of_device_ids,
#endif
//		.bus	= &platform_bus_type,
	}
};
#endif
#ifdef ENABLE_FIFTH_EHC
static struct platform_driver fifth_ehci_hcd_mstar_driver = {
	.probe 		= ehci_hcd_mstar_drv_probe,
	.remove 	= ehci_hcd_mstar_drv_remove,
#ifdef CONFIG_PM
	.suspend	= ehci_hcd_mstar_drv_suspend,
	.resume		= ehci_hcd_mstar_drv_resume,
#endif
	.driver = {
		.name 	= "Sstar-ehci-5",
#if defined(CONFIG_OF)
		.of_match_table = mstar_ehci_5_of_device_ids,
#endif
//		.bus	= &platform_bus_type,
	}
};
#endif
