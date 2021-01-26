/*
* ubootglue.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: jiang.ann <jiang.ann@sigmastar.com.tw>
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

#include "hal/platform.h"
#include "inc/common.h"
#include "inc/drvhostlib.h"
#include "inc/ubootglue.h"
#include "ehci-chip.h"

#ifdef USB_GLUE_DEBUG
#define USB_GLUE_PRINTF MS_MSG
#else
#define USB_GLUE_PRINTF(fmt, ...)
#endif

/* root hub port status */
static struct usb_port_status rh_status = { 0 };
extern int UsbPortSelect;

#define MAX_ROOT_PORTS 8 // depends on chip
struct usb_hcd array_usb_root_port[MAX_ROOT_PORTS] =
{
	{0, MSTAR_UTMI_BASE,MSTAR_UHC_BASE, MSTAR_USBC_BASE, MSTAR_BC_BASE, MSTAR_INIT_FLAG, 0, MSTAR_COMP0},
#ifdef ENABLE_SECOND_EHC
	{1, MSTAR_UTMI1_BASE,MSTAR_UHC1_BASE, MSTAR_USBC1_BASE, MSTAR_BC1_BASE, MSTAR_INIT1_FLAG, 0, MSTAR_COMP1},
#endif
#ifdef ENABLE_THIRD_EHC
	{2, MSTAR_UTMI2_BASE,MSTAR_UHC2_BASE, MSTAR_USBC2_BASE, MSTAR_BC2_BASE, MSTAR_INIT2_FLAG, 0, MSTAR_COMP2},
#endif
#ifdef ENABLE_FORTH_EHC
	{3, MSTAR_UTMI3_BASE,MSTAR_UHC3_BASE, MSTAR_USBC3_BASE, MSTAR_BC3_BASE, MSTAR_INIT3_FLAG, 0, MSTAR_COMP3},
#endif
};

#define NUM_OF_EHCI         (sizeof(array_usb_root_port)/sizeof(struct usb_hcd))

int FirstBulkIn,FirstBulkOut; // for compile

static int gWaitConnectTime = USB_WAIT_TIME;

void InitUSB30GPIO(void)
{
    MS_U16 u16Temp;

    *(volatile MS_U16*)(0x1F000000 + (REG_USB30_GPIO_CONTROL << 1)) = REG_USB30_GPIO_BANK;

    u16Temp = *(volatile MS_U16*)(0x1F000000 + ((REG_USB30_GPIO_CONTROL+2) << 1));
    u16Temp |= (REG_USB30_GPIO_BIT << 8);
    u16Temp |= REG_USB30_GPIO_OFFSET;
    u16Temp |= (REG_USB30_GPIO_LOW_ACTIVE << 11);
    *(volatile MS_U16*)(0x1F000000 + ((REG_USB30_GPIO_CONTROL+2) << 1)) = u16Temp;

#if defined(ENABLE_USB30_GPIO_PORT2)
    *(volatile MS_U16*)(0x1F000000 + (REG_USB30_PORT2_GPIO_CONTROL << 1)) = REG_USB30_GPIO_BANK;

    u16Temp = *(volatile MS_U16*)(0x1F000000 + ((REG_USB30_PORT2_GPIO_CONTROL+2) << 1));
    u16Temp |= (REG_USB30_PORT2_GPIO_BIT << 8);
    u16Temp |= REG_USB30_PORT2_GPIO_OFFSET;
    u16Temp |= (REG_USB30_PORT2_GPIO_LOW_ACTIVE << 11);
    *(volatile MS_U16*)(0x1F000000 + ((REG_USB30_PORT2_GPIO_CONTROL+2) << 1)) = u16Temp;
#endif
}

// USB_Set_WaitTime unit:ms
void USB_Set_WaitTime(unsigned int u32_ms)
{
	gWaitConnectTime = u32_ms;
}
// USB_Get_WaitTime unit:ms
unsigned int USB_Get_WaitTime(void)
{
	return gWaitConnectTime;
}


#if defined(__ARM__)
static u32 miu0_bus_base_addr;
static u32 miu1_bus_base_addr;
#if defined(MIU2_BUS_BASE_ADDR)
static u32 miu2_bus_base_addr;
#endif

unsigned int VA2PA(unsigned int u32_DMAAddr)
{
	u32_DMAAddr = MsOS_USB_VA2PA(u32_DMAAddr);

	if(u32_DMAAddr < miu1_bus_base_addr)
	{
		u32_DMAAddr -= miu0_bus_base_addr;
		u32_DMAAddr += MIU0_PHY_BASE_ADDR;
	}
#if defined(MIU2_BUS_BASE_ADDR)
	else if(u32_DMAAddr < miu2_bus_base_addr)
	{
		u32_DMAAddr -= miu1_bus_base_addr;
		u32_DMAAddr |= MIU1_PHY_BASE_ADDR;
	}
	else
	{
		u32_DMAAddr -= miu2_bus_base_addr;
		u32_DMAAddr |= MIU2_PHY_BASE_ADDR;
	}
#else
	else
	{
		u32_DMAAddr -= miu1_bus_base_addr;
		u32_DMAAddr |= MIU1_PHY_BASE_ADDR;
	}
#endif
	return u32_DMAAddr;
}

unsigned int PA2VA(unsigned int u32_DMAAddr)
{
	if(u32_DMAAddr < MIU1_PHY_BASE_ADDR)
	{
		u32_DMAAddr -= MIU0_PHY_BASE_ADDR;
		u32_DMAAddr += miu0_bus_base_addr;
	}
#if defined(MIU2_BUS_BASE_ADDR)
	else if(u32_DMAAddr < MIU2_PHY_BASE_ADDR)
	{
		u32_DMAAddr -= MIU1_PHY_BASE_ADDR;
		u32_DMAAddr |= miu1_bus_base_addr;
	}
	else
	{
		u32_DMAAddr -= MIU2_PHY_BASE_ADDR;
		u32_DMAAddr |= miu2_bus_base_addr;
	}
#else
	else
	{
		u32_DMAAddr -= MIU1_PHY_BASE_ADDR;
		u32_DMAAddr |= miu1_bus_base_addr;
	}
#endif
	u32_DMAAddr=MsOS_USB_PA2KSEG1(u32_DMAAddr); // mapping to uncache address

	return  u32_DMAAddr;
}
#endif

static void usb_bc_enable(u32 utmi_base, u32 bc_base, u8 enable)
{
	if (!bc_base)
		return;

	if (enable)
	{
		//printf("BC enable \n");
		writeb(readb((void *)(utmi_base+(0x1*2-1))) | 0x40, (void *)(utmi_base+(0x1*2-1)));  //IREF_PDN=1��b1. (utmi+0x01[6] )
		writeb(readb((void *)(bc_base+(0x3*2-1))) | 0x40, (void *)(bc_base+(0x3*2-1)));  // [6]= reg_host_bc_en
		writeb(readb((void *)(bc_base+(0xc*2))) | 0x40, (void *)(bc_base+(0xc*2)));  // [6]= reg_into_host_bc_sw_tri
		writeb(0x00, (void *)(bc_base));  // [15:0] = bc_ctl_ov_en
		writeb(0x00, (void *)(bc_base+(0x1*2-1)));  // [15:0] = bc_ctl_ov_en
		writeb(readb((void *)(bc_base+(0xa*2))) | 0x80, (void *)(bc_base+(0xa*2)));  // [7]=reg_bc_switch_en
	}
	else
	{
		// disable BC
		//printf("BC disable \n");
		writeb(readb((void *)(bc_base+(0xc*2))) & (~0x40), (void *)(bc_base+(0xc*2)));  // [6]= reg_into_host_bc_sw_tri
		writeb(readb((void *)(bc_base+(0x3*2-1))) & (~0x40), (void *)(bc_base+(0x3*2-1)));  // [6]= reg_host_bc_en
		writeb(readb((void *)(utmi_base+(0x1*2-1))) & (~0x40), (void *)(utmi_base+(0x1*2-1)));  //IREF_PDN=1��b1. (utmi+0x01[6] )
	}
}

void xhci_ppc(struct xhc_comp *xhci, int	bOn)
{
	u16 addr_w, bit_num;
	u32 addr, gpio_addr;
	u8  value, low_active;


#if 1  //XHCI one port
	addr_w = readw((void*)(xhci->u3top_base+0xFC*2));
	addr = (u32)addr_w << 8;
	addr_w = readw((void*)(xhci->u3top_base+0xFE*2));
	addr |= addr_w & 0xFF;
	bit_num = (addr_w >> 8) & 0x7;
	low_active = (u8)((addr_w >> 11) & 0x1);
#else
	switch (xhci->port_index) {
	case 0:
		addr_w = readw((void*)(xhci->u3top_base+0xFC*2));
		addr = (u32)addr_w << 8;
		addr_w = readw((void*)(xhci->u3top_base+0xFE*2));
		addr |= addr_w & 0xFF;
		bit_num = (addr_w >> 8) & 0x7;
		low_active = (u8)((addr_w >> 11) & 0x1);
		break;
	case 1:
		addr_w = readw((void*)(xhci->u3top_base+0xE6*2));
		addr = (u32)addr_w << 8;
		addr_w = readw((void*)(xhci->u3top_base+0xE8*2));
		addr |= addr_w & 0xFF;
		bit_num = (addr_w >> 8) & 0x7;
		low_active = (u8)((addr_w >> 11) & 0x1);
		break;
	default:	/* "can't happen" */
		return;
	}
#endif

	if (addr)
	{
		printf("xhci_ppc: turn %s USB3.0 port %d power \n", (bOn) ? "on" : "off", xhci->port_index);
		printf("Addr: 0x%x bit_num: %d low_active:%d\n", addr, bit_num, low_active);

		value = (u8)(1 << bit_num);
		if (addr & 0x1)
			gpio_addr = _MSTAR_PM_BASE+addr*2-1;
		else
			gpio_addr = _MSTAR_PM_BASE+addr*2;

		if (low_active^bOn)
		{
			writeb(readb((void*)gpio_addr) | value, (void*)gpio_addr);
		}
		else
		{
			writeb(readb((void*)gpio_addr) & (u8)(~value), (void*)gpio_addr);
		}
	}
	else {
		printf("\n\n!!!! ERROR : xhci: no GPIO information for vbus port power control  !!!! \n\n");
		return;
	}


}


#if defined(ENABLE_USB_NEW_MIU_SEL)
void MIU_select_setting_ehc(unsigned int USBC_base)
{
	printf("[USB] config miu select [%x] [%x] [%x] ][%x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
	writeb(USB_MIU_SEL0, (void*)(USBC_base+0x14*2));        //Setting MIU0 segment
	writeb(USB_MIU_SEL1, (void*)(USBC_base+0x16*2));        //Setting MIU1 segment
	writeb(USB_MIU_SEL2, (void*)(USBC_base+0x17*2-1));      //Setting MIU2 segment
	writeb(USB_MIU_SEL3, (void*)(USBC_base+0x18*2));        //Setting MIU3 segment
	writeb(readb((void*)(USBC_base+0x19*2-1)) | 0x01, (void*)(USBC_base+0x19*2-1)); //Enable miu partition mechanism
#if !defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
	printf("[USB] enable miu lower bound address subtraction\n");
	writeb(readb((void*)(USBC_base+0x0F*2-1)) | 0x01, (void*)(USBC_base+0x0F*2-1));
#endif
}
#endif

void TV_usb_init(struct usb_hcd *hcd)
{
	USB_GLUE_PRINTF("[USB] TV_usb_init (UTMI Init) ++\n");
	USB_GLUE_PRINTF("[USB] UTMI Base %8x\n", hcd->utmi_base);
	USB_GLUE_PRINTF("[USB] UHC Base %8x\n", hcd->ehc_base);
	USB_GLUE_PRINTF("[USB] USBC Base %8x\n", hcd->usbc_base);
	USB_GLUE_PRINTF("[USB] BC Base %8x\n", hcd->bc_base);

    writew(0x0001, (void*)hcd->utmi_base);
#if defined(ENABLE_USB_NEW_MIU_SEL)
	MIU_select_setting_ehc(hcd->usbc_base);
#endif

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) & (u8)(~BIT3), (void*) (hcd->utmi_base+0x0*2)); //DP_PUEN = 0
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) & (u8)(~BIT4), (void*) (hcd->utmi_base+0x0*2)); //DM_PUEN = 0

    writeb(readb((void*)(hcd->utmi_base+0x0*2)) & (u8)(~BIT5), (void*) (hcd->utmi_base+0x0*2)); //R_PUMODE = 0

    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT6, (void*) (hcd->utmi_base+0x0*2)); //R_DP_PDEN = 1
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT7, (void*) (hcd->utmi_base+0x0*2)); //R_DM_PDEN = 1

    writeb(readb((void*)(hcd->utmi_base+0x10*2)) | MS_BIT6, (void*) (hcd->utmi_base+0x10*2)); //hs_txser_en_cb = 1
    writeb(readb((void*)(hcd->utmi_base+0x10*2)) & (u8)(~MS_BIT7), (void*) (hcd->utmi_base+0x10*2)); //hs_se0_cb = 0

    /* turn on overwrite mode */
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT1, (void*) (hcd->utmi_base+0x0*2)); //tern_ov = 1
    /* new HW term overwrite: on */
    writeb(readb((void*)(hcd->utmi_base+0x52*2)) | (MS_BIT5|MS_BIT4|
        MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), (void*) (hcd->utmi_base+0x52*2));
#endif

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
	 * Before UHC reset, R_DP_PDEN = 1, R_DM_PDEN = 1, tern_ov = 1 */
	writeb(readb((void*)(hcd->utmi_base+0x0*2)) | (MS_BIT7|MS_BIT6|MS_BIT1), (void*) (hcd->utmi_base+0x0*2));
	/* new HW term overwrite: on */
	writeb(readb(hcd->utmi_base+0x52*2) | (MS_BIT5|MS_BIT4|
		MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), hcd->utmi_base+0x52*2);

#ifdef ENABLE_DOUBLE_DATARATE_SETTING
    writeb(readb((void*)(hcd->utmi_base+0x0D*2-1)) | BIT0, (void*) (hcd->utmi_base+0x0D*2-1)); // set reg_double_data_rate, To get better jitter performance
#endif

#ifdef ENABLE_UPLL_SETTING
    // sync code from eCos
    {
        u16 reg_t;

        reg_t = readw((void*)(hcd->utmi_base+0x22*2));
        if ((reg_t & 0x10e0) != 0x10e0)
            writew(0x10e0, (void*)(hcd->utmi_base+0x22*2));
        reg_t = readw((void*)(hcd->utmi_base+0x24*2));
        if (reg_t != 0x1)
            writew(0x1, (void*)(hcd->utmi_base+0x24*2));
    }
    //writeb(0, (void*) (hcd->utmi_base+0x21*2-1));
    //writeb(0x10, (void*) (hcd->utmi_base+0x23*2-1));
    //writeb(0x01, (void*) (hcd->utmi_base+0x24*2));
#endif

	writeb(0x0A, hcd->usbc_base);	/* Disable MAC initial suspend, Reset UHC */
	writeb(0x28, hcd->usbc_base);	/* Release UHC reset, enable UHC XIU function */

	/* Init UTMI squelch level setting before CA */
	if(UTMI_DISCON_LEVEL_2A & (MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0))
	{
		writeb((UTMI_DISCON_LEVEL_2A & (MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0)),
			(void*)(hcd->utmi_base+0x2a*2));
		USB_GLUE_PRINTF("[USB] init squelch level 0x%x\n", readb((void*)(hcd->utmi_base+0x2a*2)));
	}

	writeb(readb(hcd->utmi_base+0x3C*2) | 0x01, hcd->utmi_base+0x3C*2);	/* set CA_START as 1 */

	mdelay(1); // 10->1

	writeb(readb(hcd->utmi_base+0x3C*2) & ~0x01, hcd->utmi_base+0x3C*2);	/* release CA_START */
#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
	writeb(readb((void*)hcd->utmi_base+0x10*2) | 0x40, (void*) (hcd->utmi_base+0x10*2)); // bit<6> for monkey test and HS current
#endif
	while (((unsigned int)(readb(hcd->utmi_base+0x3C*2)) & 0x02) == 0);	/* polling bit <1> (CA_END) */

    if ((0xFFF0 == (readw((void*)(hcd->utmi_base+0x3C*2)) & 0xFFF0 )) ||
		(0x0000 == (readw((void*)(hcd->utmi_base+0x3C*2)) & 0xFFF0 ))  )
		USB_GLUE_PRINTF("WARNING: CA Fail !! \n");

	if (hcd->init_flag & EHCFLAG_DPDM_SWAP)
		writeb(readb(hcd->utmi_base+0x0b*2-1) |0x20, hcd->utmi_base+0x0b*2-1);	/* dp dm swap */

	writeb(readb(hcd->usbc_base+0x02*2) & ~0x03, hcd->usbc_base+0x02*2);	/* UHC select enable */
	writeb(readb(hcd->usbc_base+0x02*2) | 0x01, hcd->usbc_base+0x02*2);	/* UHC select enable */

	writeb(readb(hcd->ehc_base+0x40*2) & ~0x10, hcd->ehc_base+0x40*2);	/* 0: VBUS On. */
	udelay(1);	/* delay 1us */
	writeb(readb(hcd->ehc_base+0x40*2) | 0x08, hcd->ehc_base+0x40*2);	/* Active HIGH */

	/* Turn on overwirte mode for D+/D- floating issue when UHC reset
	 * After UHC reset, disable overwrite bits */
	writeb(readb((void*)(hcd->utmi_base+0x0*2)) & (u8)(~(MS_BIT7|MS_BIT6|MS_BIT1)), (void*) (hcd->utmi_base+0x0*2));
	/* new HW term overwrite: off */
	writeb(readb(hcd->utmi_base+0x52*2) & ~(MS_BIT5|MS_BIT4|
		MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), hcd->utmi_base+0x52*2);

    /* improve the efficiency of USB access MIU when system is busy */
	writeb(readb((void*)(hcd->ehc_base+0x81*2-1)) | (MS_BIT0 | MS_BIT1 | MS_BIT2 | MS_BIT3 | MS_BIT7), (void*)(hcd->ehc_base+0x81*2-1));

	writeb((readb(hcd->utmi_base+0x06*2) & 0x9F) | 0x40, hcd->utmi_base+0x06*2);	/* reg_tx_force_hs_current_enable */

	writeb(readb(hcd->utmi_base+0x03*2-1) | 0x28, hcd->utmi_base+0x03*2-1);	/* Disconnect window select */
	writeb(readb(hcd->utmi_base+0x03*2-1) & 0xef, hcd->utmi_base+0x03*2-1);	/* Disconnect window select */
	writeb(readb(hcd->utmi_base+0x07*2-1) & 0xfd, hcd->utmi_base+0x07*2-1);	/* Disable improved CDR */

#if defined(ENABLE_UTMI_240_AS_120_PHASE_ECO)
	#if defined(UTMI_240_AS_120_PHASE_ECO_INV)
	writeb(readb((void*)hcd->utmi_base+0x08*2) & ~0x08, (void*) (hcd->utmi_base+0x08*2)); // bit<3> special for Eiffel analog LIB issue
	#elif defined(UTMI_240_AS_120_PHASE_ECO_INV_IF_REV_0)
	if (MDrv_SYS_GetChipRev() == 0x0) {
		writeb(readb((void*)hcd->utmi_base+0x08*2) & ~0x08, (void*) (hcd->utmi_base+0x08*2)); // bit<3> special for Eiffel analog LIB issue
	}
	#else
	writeb(readb(hcd->utmi_base+0x08*2) | 0x08, hcd->utmi_base+0x08*2);	/* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
	#endif
#endif

	writeb(readb(hcd->utmi_base+0x09*2-1) | 0x81, hcd->utmi_base+0x09*2-1);	/* UTMI RX anti-dead-loc, ISI effect improvement */
#if defined(UTMI_TX_TIMING_SEL_LATCH_PATH_INV_IF_REV_0)
	if (MDrv_SYS_GetChipRev() == 0x0) {
		writeb(readb(hcd->utmi_base+0x0b*2-1) & ~0x80, hcd->utmi_base+0x0b*2-1);	/* TX timing select latch path */
	}
#else
	writeb(readb(hcd->utmi_base+0x0b*2-1) | 0x80, hcd->utmi_base+0x0b*2-1);	/* TX timing select latch path */
#endif
	writeb(readb(hcd->utmi_base+0x15*2-1) | 0x20, hcd->utmi_base+0x15*2-1);	/* Chirp signal source select */
#if defined(ENABLE_UTMI_55_INTERFACE)
	writeb(readb(hcd->utmi_base+0x15*2-1) | 0x40, hcd->utmi_base+0x15*2-1);	/* change to 55 interface */
#endif

    /* new HW chrip design, defualt overwrite to reg_2A */
	writeb(readb((void*)(hcd->utmi_base+0x40*2)) & (u8)(~MS_BIT4), (void*)(hcd->utmi_base+0x40*2));

	/* Init UTMI disconnect level setting */
	writeb(UTMI_DISCON_LEVEL_2A, (void*)(hcd->utmi_base+0x2a*2));

#if defined(ENABLE_NEW_HW_CHRIP_PATCH)
    /* Init chrip detect level setting */
    writeb(UTMI_CHIRP_DCT_LEVEL_42, (void*)(hcd->utmi_base+0x42*2));
    /* enable HW control chrip/disconnect level */
    writeb(readb((void*)(hcd->utmi_base+0x40*2)) & (u8)(~BIT3), (void*)(hcd->utmi_base+0x40*2));
#endif

    /* Init UTMI eye diagram parameter setting */
    writeb(readb((void*)(hcd->utmi_base+0x2c*2)) | UTMI_EYE_SETTING_2C, (void*)(hcd->utmi_base+0x2c*2));
    writeb(readb((void*)(hcd->utmi_base+0x2d*2-1)) | UTMI_EYE_SETTING_2D, (void*)(hcd->utmi_base+0x2d*2-1));
    writeb(readb((void*)(hcd->utmi_base+0x2e*2)) | UTMI_EYE_SETTING_2E, (void*)(hcd->utmi_base+0x2e*2));
    writeb(readb((void*)(hcd->utmi_base+0x2f*2-1)) | UTMI_EYE_SETTING_2F, (void*)(hcd->utmi_base+0x2f*2-1));

#if defined(ENABLE_LS_CROSS_POINT_ECO)
    /* Enable deglitch SE0 (low-speed cross point) */
    writeb(readb((void*)(hcd->utmi_base+LS_CROSS_POINT_ECO_OFFSET)) | LS_CROSS_POINT_ECO_BITSET, (void*)(hcd->utmi_base+LS_CROSS_POINT_ECO_OFFSET));
#endif

#if defined(ENABLE_PWR_NOISE_ECO)
    /* Enable use eof2 to reset state machine (power noise) */
    writeb(readb((void*)(hcd->usbc_base+0x02*2)) | MS_BIT6, (void*)(hcd->usbc_base+0x02*2));
#endif

#if defined(ENABLE_TX_RX_RESET_CLK_GATING_ECO)
    /* Enable hw auto deassert sw reset(tx/rx reset) */
    writeb(readb((void*)(hcd->utmi_base+TX_RX_RESET_CLK_GATING_ECO_OFFSET)) | TX_RX_RESET_CLK_GATING_ECO_BITSET, (void*)(hcd->utmi_base+TX_RX_RESET_CLK_GATING_ECO_OFFSET));
#endif

#if defined(ENABLE_LOSS_SHORT_PACKET_INTR_ECO)
    /* Enable patch for the assertion of interrupt(Lose short packet interrupt) */
#if defined(LOSS_SHORT_PACKET_INTR_ECO_OPOR)
    writeb(readb((void*)(hcd->usbc_base+LOSS_SHORT_PACKET_INTR_ECO_OFFSET)) | LOSS_SHORT_PACKET_INTR_ECO_BITSET, (void*)(hcd->usbc_base+LOSS_SHORT_PACKET_INTR_ECO_OFFSET));
#else
    writeb(readb((void*)(hcd->usbc_base+0x04*2)) & (u8)(~MS_BIT7), (void*)(hcd->usbc_base+0x04*2));
#endif
#endif

#if defined(ENABLE_BABBLE_ECO)
    /* Enable add patch to Period_EOF1(babble problem) */
    writeb(readb((void*)(hcd->usbc_base+0x04*2)) | MS_BIT6, (void*)(hcd->usbc_base+0x04*2));
#endif

#if defined(ENABLE_MDATA_ECO)
    /* Enable short packet MDATA in Split transaction clears ACT bit (LS dev under a HS hub) */
    writeb(readb((void*)(hcd->usbc_base+MDATA_ECO_OFFSET)) | MDATA_ECO_BITSET, (void*) (hcd->usbc_base+MDATA_ECO_OFFSET));
#endif

#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
    /* Change override to hs_txser_en.  Dm always keep high issue */
    writeb(readb((void*)(hcd->utmi_base+0x10*2)) | MS_BIT6, (void*) (hcd->utmi_base+0x10*2));
#endif

#if defined(ENABLE_HS_CONNECTION_FAIL_INTO_VFALL_ECO)
    /* HS connection fail problem (Gate into VFALL state) */
    writeb(readb((void*)(hcd->usbc_base+0x11*2-1)) | MS_BIT1, (void*)(hcd->usbc_base+0x11*2-1));
#endif

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT6, (void*) (hcd->utmi_base+0x0*2)); //R_DP_PDEN = 1
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT7, (void*) (hcd->utmi_base+0x0*2)); //R_DM_PDEN = 1

    /* turn on overwrite mode */
    writeb(readb((void*)(hcd->utmi_base+0x0*2)) | MS_BIT1, (void*) (hcd->utmi_base+0x0*2)); //tern_ov = 1
    /* new HW term overwrite: on */
    writeb(readb((void*)(hcd->utmi_base+0x52*2)) | (MS_BIT5|MS_BIT4|
        MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), (void*) (hcd->utmi_base+0x52*2));
#endif

#if defined(ENABLE_PV2MI_BRIDGE_ECO)
	writeb(readb((hcd->usbc_base+0x0a*2)) | 0x40, hcd->usbc_base+0x0a*2);
#endif

#if _USB_ANALOG_RX_SQUELCH_PATCH
    /* squelch level adjust by calibration value */
    {
    unsigned int ca_da_ov, ca_db_ov, ca_tmp;

    ca_tmp = readw((void*)(hcd->utmi_base+0x3c*2));
    ca_da_ov = (((ca_tmp >> 4) & 0x3f) - 5) + 0x40;
    ca_db_ov = (((ca_tmp >> 10) & 0x3f) - 5) + 0x40;
    USB_GLUE_PRINTF("[%x]-5 -> (ca_da_ov, ca_db_ov)=(%x,%x)\n", ca_tmp, ca_da_ov, ca_db_ov);
    writeb(ca_da_ov ,(void*)(hcd->utmi_base+0x3B*2-1));
    writeb(ca_db_ov ,(void*)(hcd->utmi_base+0x24*2));
    }
#endif

#if _USB_MINI_PV2MI_BURST_SIZE
    writeb(readb((void*)(hcd->usbc_base+0x0b*2-1)) & ~(MS_BIT1|MS_BIT2|MS_BIT3|MS_BIT4), (void*)(hcd->usbc_base+0x0b*2-1));
#endif

#if defined(ENABLE_UHC_PREAMBLE_ECO)
    /* [7]: reg_etron_en, to enable utmi Preamble function */
    writeb(readb((void*)(hcd->utmi_base+0x3f*2-1)) | MS_BIT7, (void*) (hcd->utmi_base+0x3f*2-1));

    /* [3:]: reg_preamble_en, to enable Faraday Preamble */
    writeb(readb((void*)(hcd->usbc_base+0x0f*2-1)) | MS_BIT3, (void*)(hcd->usbc_base+0x0f*2-1));

    /* [0]: reg_preamble_babble_fix, to patch Babble occurs in Preamble */
    writeb(readb((void*)(hcd->usbc_base+0x10*2)) | MS_BIT0, (void*)(hcd->usbc_base+0x10*2));

    /* [1]: reg_preamble_fs_within_pre_en, to patch FS crash problem */
    writeb(readb((void*)(hcd->usbc_base+0x10*2)) | MS_BIT1, (void*)(hcd->usbc_base+0x10*2));

    /* [2]: reg_fl_sel_override, to override utmi to have FS drive strength */
    writeb(readb((void*)(hcd->utmi_base+0x03*2-1)) | MS_BIT2, (void*) (hcd->utmi_base+0x03*2-1));
#endif

#if defined(ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO)
    /* Don't close RUN bit when device disconnect */
    writeb(readb((void*)(hcd->ehc_base+0x34*2)) | MS_BIT7, (void*)(hcd->ehc_base+0x34*2));
#endif

#if _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH
    /* Enabe PVCI i_miwcplt wait for mi2uh_last_done_z */
    writeb(readb((void*)(hcd->ehc_base+0x83*2-1)) | MS_BIT4, (void*)(hcd->ehc_base+0x83*2-1));
#endif

#if defined(ENABLE_UHC_EXTRA_HS_SOF_ECO)
    /* Extra HS SOF after bus reset */
    writeb(readb((void*)(hcd->ehc_base+0x8C*2)) | MS_BIT0, (void*)(hcd->ehc_base+0x8C*2));
#endif

    /* Enable HS ISO IN Camera Cornor case ECO function */
#if defined(HS_ISO_IN_ECO_OFFSET)
    writeb(readb((void*)(hcd->usbc_base+HS_ISO_IN_ECO_OFFSET)) | HS_ISO_IN_ECO_BITSET, (void*) (hcd->usbc_base+HS_ISO_IN_ECO_OFFSET));
#else
    writeb(readb((void*)(hcd->usbc_base+0x13*2-1)) | MS_BIT0, (void*)(hcd->usbc_base+0x13*2-1));
#endif

#if defined(ENABLE_DISCONNECT_SPEED_REPORT_RESET_ECO)
    /* UHC speed type report should be reset by device disconnection */
    writeb(readb((void*)(hcd->usbc_base+0x20*2)) | MS_BIT0, (void*)(hcd->usbc_base+0x20*2));
#endif

#if defined(ENABLE_BABBLE_PCD_ONE_PULSE_TRIGGER_ECO)
    /* Port Change Detect (PCD) is triggered by babble.
     * Pulse trigger will not hang this condition.
     */
    writeb(readb((void*)(hcd->usbc_base+0x20*2)) | MS_BIT1, (void*)(hcd->usbc_base+0x20*2));
#endif

#if defined(ENABLE_HC_RESET_FAIL_ECO)
    /* generation of hhc_reset_u */
    writeb(readb((void*)(hcd->usbc_base+0x20*2)) | MS_BIT2, (void*)(hcd->usbc_base+0x20*2));
#endif

#if defined(ENABLE_INT_AFTER_WRITE_DMA_ECO)
    /* DMA interrupt after the write back of qTD */
    writeb(readb((void*)(hcd->usbc_base+0x20*2)) | MS_BIT3, (void*)(hcd->usbc_base+0x20*2));
#endif

#if defined(ENABLE_DISCONNECT_HC_KEEP_RUNNING_ECO)
    /* EHCI keeps running when device is disconnected */
    writeb(readb((void*)(hcd->usbc_base+0x19*2-1)) | MS_BIT3, (void*)(hcd->usbc_base+0x19*2-1));
#endif

#if !defined(_EHC_SINGLE_SOF_TO_CHK_DISCONN)
    writeb(0x05, (void*)(hcd->usbc_base+0x03*2-1)); //Use 2 SOFs to check disconnection
#endif

#if defined(ENABLE_SRAM_CLK_GATING_ECO)
    /* do SRAM clock gating automatically to save power */
    writeb(readb((void*)(hcd->usbc_base+0x20*2)) & (u8)(~MS_BIT4), (void*)(hcd->usbc_base+0x20*2));
#endif

#if defined (ENABLE_INTR_SITD_CS_IN_ZERO_ECO)
	/* Enable interrupt in sitd cs in zero packet */
	writeb(readb((void*)(hcd->usbc_base+0x11*2-1)) | BIT7, (void*)(hcd->usbc_base+0x11*2-1));
#endif

	usb_bc_enable(hcd->utmi_base, hcd->bc_base, FALSE);

	USB_GLUE_PRINTF ("[USB] TV_usb_init--\n");
}

int Usb_host_Init(struct usb_hcd *hcd, unsigned int delay)
{
	unsigned int usb_retry_cnt = USB_WAIT_LOOP_COUNT;

	USB_GLUE_PRINTF ("[USB] Usb_host_Init++\n");

	hcd->ever_inited = 1;

	/* Data length byte alignment issue */
	flib_Host20_Fix_DataLength_ByteAlign(hcd);

usb_device_detect_retry:
	if (flib_Host20_Init(hcd, 0, delay)==0)
	{
		if (usb_retry_cnt--)
		{
			MS_MSG("[USB] waiting for connecting...%d\n", usb_retry_cnt+1);
			goto usb_device_detect_retry;
		}
		MS_MSG("[USB] USB init failed\n");
		return -1;
	}

	USB_GLUE_PRINTF ("[USB] Usb_host_Init--\n");
	return hcd->bSpeed;
}

int _usb_lowlevel_init(int entry)
{
	int speed = -1;
	int ret = 0;
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

	if (UsbPortSelect >= NUM_OF_EHCI)
	{
	    return -1;
	}

#if defined(__ARM__)
	miu0_bus_base_addr = MsOS_USB_VA2PA(MIU0_BUS_BASE_ADDR);
	miu1_bus_base_addr = MsOS_USB_VA2PA(MIU1_BUS_BASE_ADDR);
	#if defined(MIU2_BUS_BASE_ADDR)
	miu2_bus_base_addr = MsOS_USB_VA2PA(MIU2_BUS_BASE_ADDR);
	#endif
#endif

	USB_GLUE_PRINTF ("[USB] usb_lowlevel_init++\n");

	MS_MSG ("[USB] USB EHCI LIB VER: %s\n", USBHOST_LIB_VER);
	MS_MSG ("[USB] Port %d is Enabled\n", UsbPortSelect);

	TV_usb_init(hcd);

	if (hcd->init_flag & EHCFLAG_XHC_COMP)
	{
		//enable xHCI clock
		xhci_enable_clock();
		//disable port
		xhci_ssport_set_state(&hcd->xhci, false);
		//turn on power
		xhci_ppc(&hcd->xhci, true);
	}

	if (entry == 1)
		goto port_status;

	speed = (entry == 2) ? Usb_host_Init(hcd, 0) :
		Usb_host_Init(hcd, USB_Get_WaitTime()) ;
port_status:
	if (speed == 1)
	{
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION | USB_PORT_STAT_LOW_SPEED;
	}
	else
	{
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus &= ~USB_PORT_STAT_LOW_SPEED;

		if(speed == -1)
			ret = -1;
	}

	rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;
	USB_GLUE_PRINTF ("[USB] usb_lowlevel_init--\n");
	return ret;
}

inline int usb_lowlevel_init(void)
{
	return _usb_lowlevel_init(0);
}

inline int usb_lowlevel_preinit(void)
{
	return _usb_lowlevel_init(1);
}

inline int usb_lowlevel_postinit(void)
{
	return _usb_lowlevel_init(2);
}

int usb_lowlevel_stop(int p)
{
	struct usb_hcd *hcd = &array_usb_root_port[p];

	if(hcd->ever_inited == 0)
		return 0;

	flib_Host20_StopRun_Setting(hcd, HOST20_Disable);

	/* reset HC to release 45ohm HS term */
	flib_Host20_Reset_HC(hcd);

	if (hcd->init_flag & EHCFLAG_XHC_COMP)
	{
		//turn off power
		xhci_ppc(&hcd->xhci, false);
		//enable port
		xhci_ssport_set_state(&hcd->xhci, true);
		mdelay(1000);
	}

	if (hcd->ever_inited == 2)
	{
		MS_MSG("[USB] Long time stable device. Waiting .5 second...\n");
		mdelay(500);
	}

	return 0;
}

void Usb_host_SetEverInited(int val)
{
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return;
    }
	hcd->ever_inited = val;
}

void SetControl1MaxPacketEx(MS_UINT8 max, int port)
{
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return;
    }
	flib_Host20_SetControl1MaxPacket(hcd, max);
}

void USB_Bulk_InitEx(struct usb_device *dev, int port)
{
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return;
    }
	hcd->FirstBulkIn = 1;
	hcd->FirstBulkOut = 1;
#ifdef USB_LEGEND_DATA_TOGGLE_METHOD
	FirstBulkIn = 1;
	FirstBulkOut = 1;
#endif
	/* report device speed */
	dev->speed = hcd->bSpeed;
	flib_Host20_Bulk_Init(dev, hcd);
}

void USB_Interrupt_InitEx(struct usb_device *dev, int port)
{
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return;
    }
	hcd->FirstIntIn = 0;
	hcd->IntrIn_Complete = 0;
	/* report device speed */
	dev->speed = hcd->bSpeed;
	flib_Host20_Interrupt_Init(dev, hcd);
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int len)
{
	int dir_out = usb_pipeout(pipe);
	//int done = 0;
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return -1;
    }
	/* deal with bulk endpoint data toggle */
#ifdef USB_LEGEND_DATA_TOGGLE_METHOD
	hcd->FirstBulkIn = (FirstBulkIn != hcd->FirstBulkIn) ? FirstBulkIn : hcd->FirstBulkIn;
	hcd->FirstBulkOut = (FirstBulkOut != hcd->FirstBulkOut) ? FirstBulkOut : hcd->FirstBulkOut;
#endif
	dev->status = 0;

	if (flib_Host20_Send_Receive_Bulk_Data(hcd, buffer,len,dir_out)>0)
	{
		dev->status = hcd->urb_status;
		return -1;
	}

	//dev->act_len = done;
	dev->act_len = len;
	/* deal with bulk endpoint data toggle */
#ifdef USB_LEGEND_DATA_TOGGLE_METHOD
	FirstBulkIn = (FirstBulkIn == hcd->FirstBulkIn) ? FirstBulkIn : hcd->FirstBulkIn;
	FirstBulkOut = (FirstBulkOut == hcd->FirstBulkOut) ? FirstBulkOut : hcd->FirstBulkOut;
#endif
	return 0;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		int len,struct devrequest *setup)
{
	int done = 0;
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return -1;
    }
	dev->status = 0;

	if ( flib_Host20_Issue_Control (hcd, dev->devnum,(unsigned char*) setup,len,(unsigned char*) buffer) > 0)
	{
		dev->status = hcd->urb_status;
		done=-1;
	}

	/* status phase */
	dev->act_len = len;

	/* deal with bulk endpoint data toggle */
#ifndef USB_LEGEND_DATA_TOGGLE_METHOD
	/* clear feature: endpoint stall */
	if ((setup->request == USB_REQ_CLEAR_FEATURE) && (setup->requesttype == USB_RECIP_ENDPOINT))
	{
		if (pipe & USB_DIR_IN)
			hcd->FirstBulkIn = 1;
		else
			hcd->FirstBulkOut = 1;
	}
#endif
	return done;
}

u8 usb_bulk_transfer_in_ex(struct usb_device *dev, unsigned int pipe,
            MS_U32 buf, MS_U32 length, MS_U32 *actual_len, int timeout)
{
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];

    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return -1;
    }
	USB_GLUE_PRINTF("usb_bulk_transfer_in_ex: pipe:%x\n", pipe);
	if (flib_Host20_Get_BulkIn_Data(hcd, dev, pipe, buf, length, actual_len, timeout)>0)
	{
		return -1;
	}

	if(dev->status==0)
		return 0;
	else
		return -1;
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int len, int interval)
{
	MS_UINT32 buf, act_len=0;
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];
    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return -1;
    }

#if 0
	if (flib_Host20_Issue_Interrupt(hcd, buf /*(MS_UINT32)pUsbIntrData*/, 8)>0)
	   return -1;

	return 0;
#else
	dev->status=0;
	dev->act_len = 0;
	buf=(MS_UINT32)buffer;
	do {
		  if (flib_Host20_Issue_Interrupt_NonBlock(hcd,  buf, len, &act_len)>0)
		  {
			printf("Send INTR fail\n");
			dev->status = hcd->urb_status;
			return -1;
		  }
		  dev->status = hcd->urb_status;
		  if (hcd->IntrIn_Complete)
		  {
			//USB_GLUE_PRINTF("Get INTR success:%d, %d\n", len, act_len);
			dev->act_len = act_len;
			return 0;
		  }
		  wait_ms(1);
		  interval--;
	}while(interval);
#endif
	USB_GLUE_PRINTF("submit_int_msg time out\n");
	return 0;
}

/* Backward compatible API for old USB LAN drivers */
int usb_bulk_transfer_in(struct usb_device *dev, void *data, int len, int *transdata)
{
	const unsigned long fake_pipe_in = 0x80;
	struct usb_hcd *hcd = &array_usb_root_port[UsbPortSelect];
    if (UsbPortSelect >= NUM_OF_EHCI)
    {
        return -1;
    }

    *transdata = 0;
	if (submit_bulk_msg(dev, fake_pipe_in, data, len) == -1) {
		return -1;
	}
        *transdata = len - hcd->total_bytes;
	return 0;
}

/* Backward compatible API for old USB LAN drivers */
int usb_bulk_transfer_out(struct usb_device *dev, void *data, int len)
{
	const unsigned long fake_pipe_out = 0x0;
	if (submit_bulk_msg(dev, fake_pipe_out, data, len) == -1) {
		return -1;
	}
	return 0;
}
