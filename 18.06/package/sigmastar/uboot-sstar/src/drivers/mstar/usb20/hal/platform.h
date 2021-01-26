/*
* platform.h- Sigmastar
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


#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#define USBHOST_LIB_VER	"2018.01.12"

#include "MsTypes_usb.h"

/* Select a OS Platform */
//#define SERET_ENV
#define MSTAR_UBOOT_ENV

/* General Setting for Mstar Platform USB Portting */
//#ifdef SERET_ENV
//#define CONFIG_USB_STORAGE 1
//#define USB_BAD_DEVICE_RETRY_PATCH 1
/* normal 1 + additional N */
//#define USB_BAD_DEV_MAX_RETRY 2
//#endif
#define USB_WAIT_LOOP_COUNT 0 // for super speed device

/* USB Test Mode */
// #define USB_TEST
#define USB_LEGEND_DATA_TOGGLE_METHOD

/****** Chip variable setting ******/
#if 1	/* Every Mstar chip should appley it */
	#define _USB_FLUSH_BUFFER	1
#else
	#define _USB_FLUSH_BUFFER	0
#endif

//------ Hardware ECO enable switch ----------------------------------
//---- 1. cross point
#if 0 // every chip must enable it manually
#else
    #define ENABLE_LS_CROSS_POINT_ECO
    #define LS_CROSS_POINT_ECO_OFFSET   (0x04*2)
    #define LS_CROSS_POINT_ECO_BITSET   MS_BIT6
#endif

//---- 2. power noise
#if 0 // every chip must enable it manually
#else
    #define ENABLE_PWR_NOISE_ECO
#endif

//---- 3. tx/rx reset clock gating cause XIU timeout
#if 0 // every chip must enable it manually
#else
    #define ENABLE_TX_RX_RESET_CLK_GATING_ECO
    #define TX_RX_RESET_CLK_GATING_ECO_OFFSET   (0x04*2)
    #define TX_RX_RESET_CLK_GATING_ECO_BITSET   MS_BIT5
#endif

//---- 4. short packet lose interrupt without IOC
#if 0
    #define ENABLE_LOSS_SHORT_PACKET_INTR_ECO
    #define LOSS_SHORT_PACKET_INTR_ECO_OPOR
    #define LOSS_SHORT_PACKET_INTR_ECO_OFFSET   (0x04*2)
    #define LOSS_SHORT_PACKET_INTR_ECO_BITSET   BIT7
#endif

//---- 5. babble avoidance
#if 0
    #define ENABLE_BABBLE_ECO
#endif

//---- 6. lose packet in MDATA condition
#if 0
    #define MDATA_ECO_OFFSET    (0x0F*2-1)
    #define MDATA_ECO_BITSET    BIT4
#endif

//---- 7. change override to hs_txser_en condition (DM always keep high issue)
#if 0
    #define ENABLE_HS_DM_KEEP_HIGH_ECO
#endif

//---- 8. fix pv2mi bridge mis-behavior
#if 1	/* Every Mstar chip should appley it */
	#define ENABLE_PV2MI_BRIDGE_ECO
#endif

//---- 9. change to 55 interface
#if 1	/* Every Mstar chip should appley it */
	#define ENABLE_UTMI_55_INTERFACE
#endif

//---- 10. 240's phase as 120's clock
#if 1	/* Every Mstar chip should appley it */
	/* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
	#define ENABLE_UTMI_240_AS_120_PHASE_ECO
	//#define UTMI_240_AS_120_PHASE_ECO_INV
#endif

//---- 11. double date rate (480MHz)
//#define ENABLE_DOUBLE_DATARATE_SETTING

//---- 12. UPLL setting, normally it should be done in sboot
//#define ENABLE_UPLL_SETTING

//---- 13. chip top performance tuning
//#define ENABLE_CHIPTOP_PERFORMANCE_SETTING

//---- 14. HS connection fail problem (Gate into VFALL state)
#define ENABLE_HS_CONNECTION_FAIL_INTO_VFALL_ECO

//---- 15. Enable UHC Preamble ECO function
#define ENABLE_UHC_PREAMBLE_ECO

//---- 16. Don't close RUN bit when device disconnect
#define ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO

//---- 18. Extra HS SOF after bus reset
#define ENABLE_UHC_EXTRA_HS_SOF_ECO

//---- 19. Not yet support MIU lower bound address subtraction ECO (for chips which use ENABLE_USB_NEW_MIU_SLE)
//#define DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO

//---- 20. UHC speed type report should be reset by device disconnection
#define ENABLE_DISCONNECT_SPEED_REPORT_RESET_ECO

//---- 21. Port Change Detect (PCD) is triggered by babble. Pulse trigger will not hang this condition.
/* 1'b0: level trigger
* 1'b1: one-pulse trigger
*/
#define ENABLE_BABBLE_PCD_ONE_PULSE_TRIGGER_ECO

//---- 22. generation of hhc_reset_u
/* 1'b0: hhc_reset is_u double sync of hhc_reset
* 1'b1: hhc_reset_u is one-pulse of hhc_reset
*/
#define ENABLE_HC_RESET_FAIL_ECO

//---- 23. EHCI keeps running when device is disconnected
//#define ENABLE_DISCONNECT_HC_KEEP_RUNNING_ECO

//---- 24. Chirp patch use software overwrite value
/* reg_sw_chirp_override_bit set to 0 */
#define DISABLE_NEW_HW_CHRIP_ECO

//--------------------------------------------------------------------


//------ Software patch enable switch --------------------------------
//---- 1. flush MIU pipe
#if 0 // every chip must apply it
    #define _USB_T3_WBTIMEOUT_PATCH 0
#else
    #define _USB_T3_WBTIMEOUT_PATCH 1
#endif

//---- 2. data structure (qtd ,...) must be 128-byte aligment
#if 0 // every chip must apply it
    #define _USB_128_ALIGMENT 0
#else
    #define _USB_128_ALIGMENT 1
#endif

//---- 3. tx/rx reset clock gating cause XIU timeout
#if 0
    #define _USB_XIU_TIMEOUT_PATCH 1
#else
    #define _USB_XIU_TIMEOUT_PATCH 0
#endif

//---- 4. short packet lose interrupt without IOC
#if 0
    #define _USB_SHORT_PACKET_LOSE_INT_PATCH 1
#else
    #define _USB_SHORT_PACKET_LOSE_INT_PATCH 0
#endif

//---- 5. QH blocking in MDATA condition, split zero-size data
#if 0 // every chip must apply it
    #define _USB_SPLIT_MDATA_BLOCKING_PATCH 0
#else
    #define _USB_SPLIT_MDATA_BLOCKING_PATCH 1
#endif

//---- 6. DM always keep high issue
#if 1 // if without ECO solution, use SW patch.
    #if 0
        #define _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH 1
    #else
        #define _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH 0
    #endif
#else
    #define _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH 0
#endif

//---- 7. clear port eanble when device disconnect while bus reset
#if 0 // every chip must apply it, so far
    #define _USB_CLEAR_PORT_ENABLE_AFTER_FAIL_RESET_PATCH 0
#else
    #define _USB_CLEAR_PORT_ENABLE_AFTER_FAIL_RESET_PATCH 1
#endif

//---- 8. mstar host only supports "Throttle Mode" in split translation
#if 0 // every chip must apply it, so far
    #define _USB_TURN_ON_TT_THROTTLE_MODE_PATCH 0
#else
    #define _USB_TURN_ON_TT_THROTTLE_MODE_PATCH 1
#endif

//---- 9. lower squelch level to cover weak cable link
#if 0
    #define _USB_ANALOG_RX_SQUELCH_PATCH 1
#else
    #define _USB_ANALOG_RX_SQUELCH_PATCH 0
#endif

//---- 10. high speed reset chirp patch
#define _USB_HS_CHIRP_PATCH 1

//---- 11. friendly customer patch
#define _USB_FRIENDLY_CUSTOMER_PATCH 1

//---- 12. enabe PVCI i_miwcplt wait for mi2uh_last_done_z
#if 1	/* Every Mstar New chip should appley it */
#define _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH 1
#endif

//------ UTMI disconnect level parameters ---------------------------------
// 0x00: 550mv, 0x20: 575, 0x40: 600, 0x60: 625
#define UTMI_DISCON_LEVEL_2A	(0x62)

//------ UTMI eye diagram parameters ---------------------------------
#if 0
	// for 40nm
	#define UTMI_EYE_SETTING_2C	(0x98)
	#define UTMI_EYE_SETTING_2D	(0x02)
	#define UTMI_EYE_SETTING_2E	(0x10)
	#define UTMI_EYE_SETTING_2F	(0x01)
	#define UTMI_ALL_EYE_SETTING	(0x01100298)
#elif 0
	// for 40nm after Agate, use 55nm setting7
	#define UTMI_EYE_SETTING_2C	(0x90)
	#define UTMI_EYE_SETTING_2D	(0x03)
	#define UTMI_EYE_SETTING_2E	(0x30)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81300390)
#elif 0
	// for 40nm after Agate, use 55nm setting6
	#define UTMI_EYE_SETTING_2C	(0x10)
	#define UTMI_EYE_SETTING_2D	(0x03)
	#define UTMI_EYE_SETTING_2E	(0x30)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81300310)
#elif 0
	// for 40nm after Agate, use 55nm setting5
	#define UTMI_EYE_SETTING_2C	(0x90)
	#define UTMI_EYE_SETTING_2D	(0x02)
	#define UTMI_EYE_SETTING_2E	(0x30)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81300290)
#elif 0
	// for 40nm after Agate, use 55nm setting4
	#define UTMI_EYE_SETTING_2C	(0x90)
	#define UTMI_EYE_SETTING_2D	(0x03)
	#define UTMI_EYE_SETTING_2E	(0x00)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81000390)
#elif 0
	// for 40nm after Agate, use 55nm setting3
	#define UTMI_EYE_SETTING_2C	(0x10)
	#define UTMI_EYE_SETTING_2D	(0x03)
	#define UTMI_EYE_SETTING_2E	(0x00)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81000310)
#elif 0
	// for 40nm after Agate, use 55nm setting2
	#define UTMI_EYE_SETTING_2C	(0x90)
	#define UTMI_EYE_SETTING_2D	(0x02)
	#define UTMI_EYE_SETTING_2E	(0x00)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81000290)
#else
	// for 40nm after Agate, use 55nm setting1, the default
	#define UTMI_EYE_SETTING_2C	(0x10)
	#define UTMI_EYE_SETTING_2D	(0x02)
	#define UTMI_EYE_SETTING_2E	(0x00)
	#define UTMI_EYE_SETTING_2F	(0x81)
	#define UTMI_ALL_EYE_SETTING	(0x81000210)
#endif

#define CONFIG_MIU0_BUSADDR 0x20000000
#define CONFIG_MIU1_BUSADDR 0xC0000000

/****** Porting environment setting ******/
#if defined(SERET_ENV)
	/* Delay Function */
	#include <timer.h>
	#define udelay(us)		udelay(us)

	/* buncing buffer enable for non cache flush API in cache enable system */
	// #define USB_BOUNCING_BUF	1
	#define readb(addr)		*((unsigned char volatile *)(addr))
	#define writeb(val, addr)	(*((unsigned char volatile *)(addr)) = (unsigned char)val)

	#define mdelay(ms)		{	MS_U32 i;		\
						for(i=0;i<ms;i++)	\
						udelay(1000);		\
					}

	#define USB_DELAY(ms)		mdelay(ms)
	#define wait_ms(ms)		mdelay(ms)
#endif

#if defined(MSTAR_UBOOT_ENV)
	extern MS_U32 MsOS_USB_VA2PA(MS_U32 addr);
	extern MS_U32 MsOS_USB_PA2KSEG0(MS_U32 addr);
	extern MS_U32 MsOS_USB_PA2KSEG1(MS_U32 addr);
	extern void Chip_Read_Memory(void);
	extern void Chip_Flush_Memory(void);
	extern void   flush_cache   (unsigned long, unsigned long);

	/* Delay Function */
	void udelay (unsigned long);

	#define readb(addr)			*((unsigned char volatile *)(addr))
	#define writeb(val, addr)	(*((unsigned char volatile *)(addr)) = (unsigned char)val)
	#define readw(addr) 		*((volatile MS_UINT16 *)(addr))
	#define writew(val, addr)	(*((volatile MS_UINT16 *)(addr)) = (MS_UINT16)val)
	#define readl(addr) 		*((volatile MS_UINT32 *)(addr))
	#define writel(val, addr)	(*((volatile MS_UINT32 *)(addr)) = (MS_UINT32)val)
/*
	#define mdelay(ms)		{	MS_U32 i;		\
						for(i=0;i<ms;i++)	\
						udelay(1000);		\
					}
*/
	#define USB_DELAY(ms)		mdelay(ms)

	#if defined(__ARM__)
		#define KSEG02KSEG1(addr)	MsOS_USB_PA2KSEG1(MsOS_USB_VA2PA((MS_U32)addr))
		#define KSEG12KSEG0(addr)	MsOS_USB_PA2KSEG0(MsOS_USB_VA2PA((MS_U32)addr))
	#else
		#define KSEG02KSEG1(addr)	((void *)((MS_U32)(addr)|0x20000000))  //cached -> unchched
		#define KSEG12KSEG0(addr)	((void *)((MS_U32)(addr)&~0x20000000)) //unchched -> cached
	#endif

	/* virtual address to physical address translation */
	#if defined(__ARM__)

		#define ENABLE_USB_NEW_MIU_SEL	1	/* Unit: 4GB / 16 = 256MB */
		#define USB_MIU_SEL0	0x70	/* MIU0: 2GB */
		#define USB_MIU_SEL1	0xF8	/* MIU1: 2GB */
		#define USB_MIU_SEL2	0xEF
		#define USB_MIU_SEL3	0xEF
		#define MIU0_BUS_BASE_ADDR	CONFIG_MIU0_BUSADDR
		#define MIU0_PHY_BASE_ADDR	0x00000000UL
		#define MIU1_BUS_BASE_ADDR	CONFIG_MIU1_BUSADDR
		#define MIU1_PHY_BASE_ADDR	0x80000000UL

		/* transmit between BUS and USB PHY Addr */
		extern unsigned int VA2PA(unsigned int u32_DMAAddr);
		extern unsigned int PA2VA(unsigned int u32_DMAAddr);
	#elif defined(__MIPS__)
		#define VA2PA(a)  (a)
		#define PA2VA(a)  (a)
	#else // ??? which CPU
		#define VA2PA(a)  ((a) & 0x1FFFFFFF)
		#define PA2VA(a)  ((a) | 0xA0000000) //// mapping to uncache address
	#endif
#endif


#endif /* __PLATFORM_H__*/
