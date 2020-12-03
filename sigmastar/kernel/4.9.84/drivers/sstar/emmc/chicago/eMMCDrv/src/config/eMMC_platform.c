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
#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMCTest_BlkWRC_ProbeDDR(U32 u32_eMMC_Addr)
{
	U32 u32_err;
	U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf_DDR, *pu32_R=(U32*)gau8_RBuf_DDR;
	U32 u32_BlkCnt=eMMC_TEST_BLK_CNT, u32_BufByteCnt;
	
	//eMMC_printf("eMMCTest_BlkWRC_ProbeDDR\r\n");
	
    u32_BlkCnt = u32_BlkCnt > DDR_TEST_BLK_CNT ? DDR_TEST_BLK_CNT : u32_BlkCnt;
    u32_BufByteCnt = u32_BlkCnt << eMMC_SECTOR_512BYTE_BITS;
	
	for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
	    pu32_R[u32_i] = 0;

	for(u32_j=0; u32_j<9; u32_j++)
	{
		// init data pattern
		switch(u32_j)
		{
			case 0: // increase
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
	                pu32_W[u32_i] = u32_i+1;
			    break;			
			case 1: // decrease
			    #if 0
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
					pu32_W[u32_i] = 0-(u32_i+1); // more FF, more chance to lose start bit
				break;
				#else
				continue;
				#endif				
			case 2: // 0xF00F
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0xF00FF00F;
				break;
			case 3: // 0xFF00
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0xFF00FF00;
				break;				
			case 4: // 0x5AA5
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0x5AA55AA5;
				break;				
			case 5: // 0x55AA
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0x55AA55AA;
				break;	
			case 6: // 0x5A5A
			    #if 0
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0x5A5A5A5A;
				break;				
				#else
                continue;
				#endif
			case 7: // 0x0000
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0x00000000;
				break;				
			case 8: // 0xFFFF
				for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
				    pu32_W[u32_i]=0xFFFFFFFF;
				break;
			
		}

		#if 0
		u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
	        break;

		u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, gau8_RBuf_DDR);
		if(eMMC_ST_SUCCESS != u32_err)
            break;

		u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, eMMC_SECTOR_512BYTE);
    	if(eMMC_ST_SUCCESS != u32_err)
	    {
		    eMMC_debug(0,1,"Err, compare fail.single, %Xh \n", u32_err);
            break;
	    }
		#endif
	
      u32_err = eMMC_CMD25_MIU(u32_eMMC_Addr, gau8_WBuf_DDR, u32_BlkCnt);  
	    if(eMMC_ST_SUCCESS != u32_err)
	        break;
		
	    u32_err = eMMC_CMD18_MIU(u32_eMMC_Addr, gau8_RBuf_DDR, u32_BlkCnt);
	    if(eMMC_ST_SUCCESS != u32_err)
            break;
						
	    u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, u32_BufByteCnt);
    	if(eMMC_ST_SUCCESS != u32_err)
	    {
		    eMMC_debug(0,1,"Err, compare fail.multi %Xh \n", u32_err);
            break;
	    }	
	}

	if(eMMC_ST_SUCCESS != u32_err)
	{
		//eMMC_FCIE_ErrHandler_ReInit();
		eMMC_debug(0,0,"data pattern %u: %02X%02X%02X%02Xh \n\n", 
		    u32_j, gau8_WBuf_DDR[3], gau8_WBuf_DDR[2], gau8_WBuf_DDR[1], gau8_WBuf_DDR[0]);
	}
	
	return u32_err;
}

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
#if (defined(eMMC_DRV_G2P_ROM)&&eMMC_DRV_G2P_ROM)
//=============================================================
#elif (defined(eMMC_DRV_EAGLE_LINUX)&&eMMC_DRV_EAGLE_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0 // udelay not work
    volatile u32 i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x38; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
    U32 u32_i = u32us;
	
	while(u32_i > 1000)
	{
		mdelay(1);
		u32_i -= 1000;
	}
    udelay(u32_i);
	#endif
	return u32us;
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
	// TIMER_FREERUN_32K  32 KHz
	// TIMER_FREERUN_XTAL 38.4 MHz,
	// counting down
	return 0;//HalTimerRead(TIMER_FREERUN_32K);
}
//--------------------------------

void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
	eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
	eMMC_debug(0, 1, "CMU (0x%X):0x%X\n", reg_ckg_CMU, REG_FCIE_U16(reg_ckg_CMU));

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}
	eMMC_debug(0, 1, "chiptop_0x5A (0x%X):0x%X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
	eMMC_debug(0, 1, "chiptop_0x1F (0x%X):0x%X\n", reg_chiptop_0x1F, REG_FCIE_U16(reg_chiptop_0x1F));
	eMMC_debug(0, 1, "chiptop_0x70 (0x%X):0x%X\n", reg_chiptop_0x70, REG_FCIE_U16(reg_chiptop_0x70));
	eMMC_debug(0, 1, "chiptop_0x64 (0x%X):0x%X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
	eMMC_debug(0, 1, "chiptop_0x4F (0x%X):0x%X\n", reg_chiptop_0x4F, REG_FCIE_U16(reg_chiptop_0x4F));
	eMMC_debug(0, 1, "chiptop_0x03 (0x%X):0x%X\n", reg_chiptop_0x03, REG_FCIE_U16(reg_chiptop_0x03));
	eMMC_debug(0, 1, "chiptop_0x51 (0x%X):0x%X\n", reg_chiptop_0x51, REG_FCIE_U16(reg_chiptop_0x51));
	eMMC_debug(0, 1, "chiptop_0x6F (0x%X):0x%X\n\n", reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));

	eMMC_debug(0, 1, "chiptop_0x43 (0x%X):0x%X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
	eMMC_debug(0, 1, "chiptop_0x50 (0x%X):0x%X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
	eMMC_debug(0, 1, "chiptop_0x0B (0x%X):0x%X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
	eMMC_debug(0, 1, "chiptop_0x5D (0x%X):0x%X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

	eMMC_debug(0, 1, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
		BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

	switch(u32_FCIE_IF_Type){
		case FCIE_eMMC_DDR:
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: DDR\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN|BIT_PAD_BYPASS_MACRO);

			// set DDR mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN|BIT_SD_DDR_EN);
			// set chiptop
			g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
			break;

		case FCIE_eMMC_SDR:
            //eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: SDR\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN|BIT_PAD_BYPASS_MACRO);

			// set SDR mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_MACRO_EN);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

		case FCIE_eMMC_BYPASS:
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC pads: BYPASS\n");

			REG_FCIE_SETBIT(reg_chiptop_0x43,
				BIT_PAD_EMMC_CLK_SRC|BIT_eMMC_RSTPIN_VAL|BIT_PAD_BYPASS_MACRO);
			REG_FCIE_CLRBIT(reg_chiptop_0x43,
				BIT_EMPTY|BIT_eMMC_RSTPIN_EN);

			// set bypadd mode
			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,
			    BIT_MACRO_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
			break;

		default:
			//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	// set chiptop
	#if 0
	REG_FCIE_CLRBIT(reg_chiptop_0x5A, BIT0|BIT1|BIT2);
	//REG_FCIE_CLRBIT(reg_chiptop_0x1F, BIT0|BIT1); // don't touch for card-detect
	//REG_FCIE_CLRBIT(reg_chiptop_0x70, BIT2); // don't touch for card-detect
	REG_FCIE_CLRBIT(reg_chiptop_0x64, BIT0);
	REG_FCIE_CLRBIT(reg_chiptop_0x4F, BIT13|BIT14);
	REG_FCIE_CLRBIT(reg_chiptop_0x03, BIT6|BIT7);
	//REG_FCIE_CLRBIT(reg_chiptop_0x51, BIT10|BIT11);  //Cause EAGLE CI card lag issue.
	REG_FCIE_CLRBIT(reg_chiptop_0x6F, BIT14|BIT15);
    #endif

	REG_FCIE_CLRBIT(reg_chiptop_0x50, BIT_NAND_CS1_EN|BIT_ALL_PAD_IN);
	REG_FCIE_CLRBIT(reg_chiptop_0x0B, BIT_NAND_CS2_EN|BIT_NAND_CS3_EN|
		BIT_SD0_CFG|BIT_SD0_CFG2|BIT_SD1_CFG|BIT_SD1_CFG2|BIT_NAND_CFG);

	REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_NEW_DESIGN_ENn);

	return eMMC_ST_SUCCESS;
}


static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{0, BIT_FCIE_CLK4X_20M, BIT_FCIE_CLK4X_27M, 0,
 BIT_FCIE_CLK4X_36M, BIT_FCIE_CLK4X_40M, 0, 0,
 0, 0, 0, 0,
 0, 0, 0, BIT_FCIE_CLK4X_48M};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:  g_eMMCDrv.u32_ClkKHz = 300;    break;
	case BIT_FCIE_CLK_20M:   g_eMMCDrv.u32_ClkKHz = 20000;  break;
    case BIT_FCIE_CLK_27M: g_eMMCDrv.u32_ClkKHz = 27000;  break;
	//case BIT_FCIE_CLK_32M:   g_eMMCDrv.u32_ClkKHz = 32000; break;
    case BIT_FCIE_CLK_36M: g_eMMCDrv.u32_ClkKHz = 36000;  break;
	case BIT_FCIE_CLK_40M:   g_eMMCDrv.u32_ClkKHz = 40000; break;
    //case BIT_FCIE_CLK_43_2M: g_eMMCDrv.u32_ClkKHz = 43200; break;
    case BIT_FCIE_CLK_48M:   g_eMMCDrv.u32_ClkKHz = 48000; break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));
	if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

		REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T", "1.5T", "2.5T","  1T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         700 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[4]={3,1,0,2}; // 1T, 1.5T, 2T, 2.5T
	U8  au8_DQS_10T[4]={10,15,20,25}; // 1T, 1.5T, 2T, 2.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8 au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");

	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;

	if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL_LOW,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{
				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + (pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp);
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{
		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();
	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

//---------------------------------------
#ifdef CONFIG_OUTER_CACHE
#include <asm/outercache.h>
#endif
#define HalUtilPHY2MIUAddr(addr)        addr
#define HalUtilMIU2PHYAddr(addr)        addr

#define SYSHAL_DCACHE_LINE_SIZE 32

#define HAL_DCACHE_START_ADDRESS(_addr_) \
    (((u32)(_addr_)) & ~(SYSHAL_DCACHE_LINE_SIZE-1))

#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
    (((u32)((_addr_) + (_asize_) + (SYSHAL_DCACHE_LINE_SIZE-1) )) & \
     ~(SYSHAL_DCACHE_LINE_SIZE-1))

#if 0
static void _hal_dcache_flush(void *base , u32 asize)
{
    register u32 _addr_ = HAL_DCACHE_START_ADDRESS((u32)base);
    register u32 _eaddr_ = HAL_DCACHE_END_ADDRESS((u32)(base), asize);

    for( ; _addr_ < _eaddr_; _addr_ += SYSHAL_DCACHE_LINE_SIZE )
        __asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 1" : : "r" (_addr_));

    /* Drain write buffer */
    _addr_ = 0x00UL;
    __asm__ __volatile__ ("MCR p15, 0, %0, c7, c10, 4" : : "r" (_addr_));
}

#define dma_dcache_flush(addr, size) _hal_dcache_flush(addr, size)
#define dma_dcache_invalidate(addr, size)
#ifdef CONFIG_OUTER_CACHE
#define dma_dcache_flush_invalidate(addr, size) \
    do{ \
        _hal_dcache_flush((void *)addr, size); \
        outer_flush_range(__pa(addr),__pa(addr) + size); \
        outer_inv_range(__pa(addr),__pa(addr) + size); \
    }while(0)
#else
#define dma_dcache_flush_invalidate(addr, size) _hal_dcache_flush((void *)addr, size)
#endif
#endif

#include "chip_int.h"
#include "chip_setup.h"
#if defined(CONFIG_ARM) // ARM: AMBER3, AGATE
#include <mstar/mstar_chip.h>
#endif

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	#if 0
	U32 dmaaddr;
	dma_dcache_flush_invalidate(u32_DMAAddr, u32_ByteCnt);
	dmaaddr = (virt_to_phys((void *)u32_DMAAddr));

	if( dmaaddr >= MSTAR_MIU1_BUS_BASE)
    {
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
        REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
    else
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
	return dmaaddr;
	#else
    extern void ___dma_single_cpu_to_dev(const void *kaddr, size_t size, enum dma_data_direction dir);

	Chip_Flush_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);

	if(virt_to_phys((void *)u32_DMAAddr) >= MSTAR_MIU1_BUS_BASE)
		REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    else
    	REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);

	return virt_to_phys((void *)u32_DMAAddr);
	#endif	
}

void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
{

}

void eMMC_flush_miu_pipe(void)
{

}


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

	#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
	#endif

	return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}


#endif


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	U32 u32_Cnt=0;

    // eagle eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eagle SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);


//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif

    // Temp patch for someone driver overwrite the register
    // The patch for SD card SD_CDZ issue (Eagle).
	REG_FCIE_SETBIT(reg_chiptop_0x1F, BIT0);

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);	
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);

	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    //eMMC_clock_setting(FCIE_SLOW_CLK);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#else
	// as usual
	eMMC_clock_setting(FCIE_SLOWEST_CLK);
	#endif

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

#elif (defined(eMMC_DRV_EDISON_LINUX)&&eMMC_DRV_EDISON_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0 // udelay not work
    volatile u32 i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x38; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
    U32 u32_i = u32us;
	
	while(u32_i > 1000)
	{
		mdelay(1);
		u32_i -= 1000;
	}
    udelay(u32_i);
	#endif
	return u32us;
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
	// Reset PIU Timer1
    REG_FCIE_W(TIMER1_MAX_LOW, 0xFFFF);
    REG_FCIE_W(TIMER1_MAX_HIGH, 0xFFFF);
    REG_FCIE_W(TIMER1_ENABLE, 0);

    // Start PIU Timer1
    REG_FCIE_W(TIMER1_ENABLE, 0x1);
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
	U32 u32HWTimer = 0;
    U32 u32TimerLow = 0;
    U32 u32TimerHigh = 0;

    // Get timer value
    u32TimerLow = REG_FCIE(TIMER1_CAP_LOW);
    u32TimerHigh = REG_FCIE(TIMER1_CAP_HIGH);

    u32HWTimer = (u32TimerHigh<<16) | u32TimerLow;

    REG_FCIE_W(TIMER1_ENABLE, 0);

    return u32HWTimer;
}
//--------------------------------

void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_pcm_d_pe (0x%08X):0x%04X\n", reg_pcm_d_pe, REG_FCIE_U16(reg_pcm_d_pe));
    eMMC_debug(0, 0, "reg_pcm_a_pe (0x%08X):0x%04X\n", reg_pcm_a_pe, REG_FCIE_U16(reg_pcm_a_pe));
    eMMC_debug(0, 0, "reg_pcm2_cd_n_pe (0x%08X):0x%04X\n", reg_pcm2_cd_n_pe, REG_FCIE_U16(reg_pcm2_cd_n_pe));

    eMMC_debug(0, 0, "reg_sd_use_bypass (0x%08X):0x%04X\n", reg_sd_use_bypass, REG_FCIE_U16(reg_sd_use_bypass));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);
    REG_FCIE_SETBIT(reg_pcm_d_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm_a_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm2_cd_n_pe, BIT1);            // reg_pcm_pe
    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
    REG_FCIE_SETBIT(reg_sd_use_bypass, BIT0);           // reg_sd_use_bypass

    REG_FCIE_SETBIT(reg_emmc_drv, 0x0007);
    REG_FCIE_SETBIT(reg_nand_drv, 0xFF00);
    REG_FCIE_SETBIT(reg_nand_ps, 0xFFFF);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT0);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        case FCIE_eMMC_TMUX:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: TMUX\n");

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10|BIT12);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_TMUX;
            break;

        default:
            //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop

    REG_FCIE_CLRBIT(reg_pcmcia_pad, BIT0|BIT4);
    REG_FCIE_CLRBIT(reg_nand_pad, BIT5|BIT6|BIT7);
    REG_FCIE_CLRBIT(reg_sd_pad, BIT8|BIT9|BIT10|BIT11);

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	return eMMC_ST_SUCCESS;
}

static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    BIT_FCIE_CLK4X_20M,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    BIT_FCIE_CLK4X_48M
};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;

	if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

//---------------------------------------
#ifdef CONFIG_OUTER_CACHE
#include <asm/outercache.h>
#endif
#define HalUtilPHY2MIUAddr(addr)        addr
#define HalUtilMIU2PHYAddr(addr)        addr

#define SYSHAL_DCACHE_LINE_SIZE 32

#define HAL_DCACHE_START_ADDRESS(_addr_) \
    (((u32)(_addr_)) & ~(SYSHAL_DCACHE_LINE_SIZE-1))

#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
    (((u32)((_addr_) + (_asize_) + (SYSHAL_DCACHE_LINE_SIZE-1) )) & \
     ~(SYSHAL_DCACHE_LINE_SIZE-1))

static void _hal_dcache_flush(void *base , u32 asize)
{
    register u32 _addr_ = HAL_DCACHE_START_ADDRESS((u32)base);
    register u32 _eaddr_ = HAL_DCACHE_END_ADDRESS((u32)(base), asize);

    for( ; _addr_ < _eaddr_; _addr_ += SYSHAL_DCACHE_LINE_SIZE )
        __asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 1" : : "r" (_addr_));

    /* Drain write buffer */
    _addr_ = 0x00UL;
    __asm__ __volatile__ ("MCR p15, 0, %0, c7, c10, 4" : : "r" (_addr_));
}

#define dma_dcache_flush(addr, size) _hal_dcache_flush(addr, size)
#define dma_dcache_invalidate(addr, size)
#ifdef CONFIG_OUTER_CACHE
#define dma_dcache_flush_invalidate(addr, size) \
    do{ \
        _hal_dcache_flush((void *)addr, size); \
        outer_flush_range(__pa(addr),__pa(addr) + size); \
        outer_inv_range(__pa(addr),__pa(addr) + size); \
    }while(0)
#else
#define dma_dcache_flush_invalidate(addr, size) _hal_dcache_flush((void *)addr, size)
#endif

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	U32 dmaaddr;
		
	dma_dcache_flush_invalidate(u32_DMAAddr, u32_ByteCnt);
	dmaaddr = (virt_to_phys((void *)u32_DMAAddr));

	if( dmaaddr >= MSTAR_MIU1_BUS_BASE)
    {
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
        REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
    else
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
	return dmaaddr;

}

void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
{

}

void eMMC_flush_miu_pipe(void)
{

}


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eagle eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eagle SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);



//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);	
}


void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

#elif (defined(eMMC_DRV_EIFFEL_LINUX)&&eMMC_DRV_EIFFEL_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0 // udelay not work
    volatile u32 i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x38; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
    U32 u32_i = u32us;
	
	while(u32_i > 1000)
	{
		mdelay(1);
		u32_i -= 1000;
	}
    udelay(u32_i);
	#endif
	return u32us;
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
	// Reset PIU Timer1
    REG_FCIE_W(TIMER1_MAX_LOW, 0xFFFF);
    REG_FCIE_W(TIMER1_MAX_HIGH, 0xFFFF);
    REG_FCIE_W(TIMER1_ENABLE, 0);

    // Start PIU Timer1
    REG_FCIE_W(TIMER1_ENABLE, 0x1);
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
	U32 u32HWTimer = 0;
    U32 u32TimerLow = 0;
    U32 u32TimerHigh = 0;

    // Get timer value
    u32TimerLow = REG_FCIE(TIMER1_CAP_LOW);
    u32TimerHigh = REG_FCIE(TIMER1_CAP_HIGH);

    u32HWTimer = (u32TimerHigh<<16) | u32TimerLow;

    REG_FCIE_W(TIMER1_ENABLE, 0);

    return u32HWTimer;
}
//--------------------------------

void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_pcm_d_pe (0x%08X):0x%04X\n", reg_pcm_d_pe, REG_FCIE_U16(reg_pcm_d_pe));
    eMMC_debug(0, 0, "reg_pcm_a_pe (0x%08X):0x%04X\n", reg_pcm_a_pe, REG_FCIE_U16(reg_pcm_a_pe));
    eMMC_debug(0, 0, "reg_pcm2_cd_n_pe (0x%08X):0x%04X\n", reg_pcm2_cd_n_pe, REG_FCIE_U16(reg_pcm2_cd_n_pe));

    eMMC_debug(0, 0, "reg_sd_use_bypass (0x%08X):0x%04X\n", reg_sd_use_bypass, REG_FCIE_U16(reg_sd_use_bypass));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);
    REG_FCIE_SETBIT(reg_pcm_d_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm_a_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm2_cd_n_pe, BIT1);            // reg_pcm_pe
    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
    REG_FCIE_SETBIT(reg_sd_use_bypass, BIT0);           // reg_sd_use_bypass

    REG_FCIE_SETBIT(reg_emmc_drv, 0x0007);
    REG_FCIE_SETBIT(reg_nand_drv, 0xFF00);
    REG_FCIE_SETBIT(reg_nand_ps, 0xFFFF);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT0);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        case FCIE_eMMC_TMUX:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: TMUX\n");

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10|BIT12);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_TMUX;
            break;

        default:
            //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop

    REG_FCIE_CLRBIT(reg_pcmcia_pad, BIT0|BIT4);
    REG_FCIE_CLRBIT(reg_nand_pad, BIT5|BIT6|BIT7);
    REG_FCIE_CLRBIT(reg_sd_pad, BIT8|BIT9|BIT10|BIT11);

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	return eMMC_ST_SUCCESS;
}

static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    BIT_FCIE_CLK4X_20M,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    BIT_FCIE_CLK4X_48M
};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;

	if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

//---------------------------------------
#ifdef CONFIG_OUTER_CACHE
#include <asm/outercache.h>
#endif
#define HalUtilPHY2MIUAddr(addr)        addr
#define HalUtilMIU2PHYAddr(addr)        addr

#define SYSHAL_DCACHE_LINE_SIZE 32

#define HAL_DCACHE_START_ADDRESS(_addr_) \
    (((u32)(_addr_)) & ~(SYSHAL_DCACHE_LINE_SIZE-1))

#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
    (((u32)((_addr_) + (_asize_) + (SYSHAL_DCACHE_LINE_SIZE-1) )) & \
     ~(SYSHAL_DCACHE_LINE_SIZE-1))

static void _hal_dcache_flush(void *base , u32 asize)
{
    register u32 _addr_ = HAL_DCACHE_START_ADDRESS((u32)base);
    register u32 _eaddr_ = HAL_DCACHE_END_ADDRESS((u32)(base), asize);

    for( ; _addr_ < _eaddr_; _addr_ += SYSHAL_DCACHE_LINE_SIZE )
        __asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 1" : : "r" (_addr_));

    /* Drain write buffer */
    _addr_ = 0x00UL;
    __asm__ __volatile__ ("MCR p15, 0, %0, c7, c10, 4" : : "r" (_addr_));
}

#define dma_dcache_flush(addr, size) _hal_dcache_flush(addr, size)
#define dma_dcache_invalidate(addr, size)
#ifdef CONFIG_OUTER_CACHE
#define dma_dcache_flush_invalidate(addr, size) \
    do{ \
        _hal_dcache_flush((void *)addr, size); \
        outer_flush_range(__pa(addr),__pa(addr) + size); \
        outer_inv_range(__pa(addr),__pa(addr) + size); \
    }while(0)
#else
#define dma_dcache_flush_invalidate(addr, size) _hal_dcache_flush((void *)addr, size)
#endif

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	U32 dmaaddr;
		
	dma_dcache_flush_invalidate(u32_DMAAddr, u32_ByteCnt);
	dmaaddr = (virt_to_phys((void *)u32_DMAAddr));

	if( dmaaddr >= MSTAR_MIU1_BUS_BASE)
    {
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
        REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
    else
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
	return dmaaddr;

}

void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
{

}

void eMMC_flush_miu_pipe(void)
{

}


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}
#endif


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eiffel SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);



//---------------------------------------

extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);	
}


void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//=============================================================

#elif (defined(eMMC_DRV_EINSTEIN_LINUX)&&eMMC_DRV_EINSTEIN_LINUX) // [FIXME] clone for your flatform
// [FIXME] -->
U32 eMMC_hw_timer_delay(U32 u32us)
{
	#if 0 // udelay not work
    volatile u32 i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x38; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else
    U32 u32_i = u32us;
	
	while(u32_i > 1000)
	{
		mdelay(1);
		u32_i -= 1000;
	}
    udelay(u32_i);
	#endif
	return u32us;
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
	// Reset PIU Timer1
    REG_FCIE_W(TIMER1_MAX_LOW, 0xFFFF);
    REG_FCIE_W(TIMER1_MAX_HIGH, 0xFFFF);
    REG_FCIE_W(TIMER1_ENABLE, 0);

    // Start PIU Timer1
    REG_FCIE_W(TIMER1_ENABLE, 0x1);
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
	U32 u32HWTimer = 0;
    U32 u32TimerLow = 0;
    U32 u32TimerHigh = 0;

    // Get timer value
    u32TimerLow = REG_FCIE(TIMER1_CAP_LOW);
    u32TimerHigh = REG_FCIE(TIMER1_CAP_HIGH);

    u32HWTimer = (u32TimerHigh<<16) | u32TimerLow;

    REG_FCIE_W(TIMER1_ENABLE, 0);

    return u32HWTimer;
}
//--------------------------------

void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
    eMMC_debug(0, 0, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 0, "FCIE 1X (0x%X):0x%04X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 0, "FCIE 4X (0x%X):0x%04X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 0, "MIU (0x%X):0x%04X\n", reg_ckg_MIU, REG_FCIE_U16(reg_ckg_MIU));
    eMMC_debug(0, 0, "MCU (0x%X):0x%04X\n", reg_ckg_MCU, REG_FCIE_U16(reg_ckg_MCU));

    //----------------------------------------------
    eMMC_debug(0, 0, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
	{
	case FCIE_eMMC_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_eMMC_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_eMMC_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown, %Xh\n", g_eMMCDrv.u8_PadType);  eMMC_die("\n");
	}

    eMMC_debug(0, 0, "reg_all_pad_in (0x%08X):0x%04X\n", reg_all_pad_in, REG_FCIE_U16(reg_all_pad_in));

    eMMC_debug(0, 0, "reg_emmc_pad (0x%08X):0x%04X\n", reg_emmc_pad, REG_FCIE_U16(reg_emmc_pad));
    eMMC_debug(0, 0, "reg_pcm_d_pe (0x%08X):0x%04X\n", reg_pcm_d_pe, REG_FCIE_U16(reg_pcm_d_pe));
    eMMC_debug(0, 0, "reg_pcm_a_pe (0x%08X):0x%04X\n", reg_pcm_a_pe, REG_FCIE_U16(reg_pcm_a_pe));
    eMMC_debug(0, 0, "reg_pcm2_cd_n_pe (0x%08X):0x%04X\n", reg_pcm2_cd_n_pe, REG_FCIE_U16(reg_pcm2_cd_n_pe));

    eMMC_debug(0, 0, "reg_sd_use_bypass (0x%08X):0x%04X\n", reg_sd_use_bypass, REG_FCIE_U16(reg_sd_use_bypass));
    eMMC_debug(0, 0, "reg_fcie2macro_sd_bypass (0x%08X):0x%04X\n", reg_fcie2macro_sd_bypass, REG_FCIE_U16(reg_fcie2macro_sd_bypass));

    eMMC_debug(0, 0, "reg_pcmcia_pad (0x%08X):0x%04X\n", reg_pcmcia_pad, REG_FCIE_U16(reg_pcmcia_pad));
    eMMC_debug(0, 0, "reg_nand_pad (0x%08X):0x%04X\n", reg_nand_pad, REG_FCIE_U16(reg_nand_pad));
    eMMC_debug(0, 0, "reg_sd_pad (0x%08X):0x%04X\n", reg_sd_pad, REG_FCIE_U16(reg_sd_pad));

    eMMC_debug(0, 0, "\n");
}

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT8|BIT9|BIT10|BIT11|BIT12);
    REG_FCIE_SETBIT(reg_pcm_d_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm_a_pe, 0x00FF);              // reg_pcm_pe
    REG_FCIE_SETBIT(reg_pcm2_cd_n_pe, BIT1);            // reg_pcm_pe
    REG_FCIE_CLRBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
    REG_FCIE_SETBIT(reg_sd_use_bypass, BIT0);           // reg_sd_use_bypass

    REG_FCIE_SETBIT(reg_emmc_drv, 0x0007);
    REG_FCIE_SETBIT(reg_nand_drv, 0xFF00);
    REG_FCIE_SETBIT(reg_nand_ps, 0xFFFF);

    switch(u32_FCIE_IF_Type){
        case FCIE_eMMC_DDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: DDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT9);

            REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT0);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: SDR\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;

        case FCIE_eMMC_BYPASS:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: BYPASS\n");

            // Let onboot & PCMCIA detemine the pad (reg_chiptop_0x6E) value

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT8|BIT10|BIT11);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        case FCIE_eMMC_TMUX:
            eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 0,"eMMC pads: TMUX\n");

            REG_FCIE_SETBIT(reg_fcie2macro_sd_bypass, BIT8);    // reg_fcie2marco_sd_bypass
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT10|BIT12);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_TMUX;
            break;

        default:
            //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    // set chiptop

    REG_FCIE_CLRBIT(reg_pcmcia_pad, BIT0|BIT4);
    REG_FCIE_CLRBIT(reg_nand_pad, BIT5|BIT6|BIT7);
    REG_FCIE_CLRBIT(reg_sd_pad, BIT8|BIT9|BIT10|BIT11);

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT15);

	return eMMC_ST_SUCCESS;
}

static U8 sgau8_FCIEClk_1X_To_4X_[0x10]= // index is 1X reg value
{
    0,
    BIT_FCIE_CLK4X_20M,
    BIT_FCIE_CLK4X_27M,
    0,
    BIT_FCIE_CLK4X_36M,
    BIT_FCIE_CLK4X_40M,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    BIT_FCIE_CLK4X_48M
};

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
	case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
	case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_27M:      g_eMMCDrv.u32_ClkKHz = 27000;   break;
	//case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
	case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    //case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
	default:
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
		return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_CLRBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate|BIT_FCIE_CLK_MASK);
	REG_FCIE_SETBIT(reg_ckg_fcie_1X, BIT_FCIE_CLK_SEL|(u16_ClkParam<<BIT_FCIE_CLK_SHIFT));

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
	{
	    REG_FCIE_CLRBIT(reg_ckg_fcie_4X,
			BIT_FCIE_CLK4X_Gate|BIT_FCIE_CLK4X_MASK);
	    REG_FCIE_SETBIT(reg_ckg_fcie_4X,
			(sgau8_FCIEClk_1X_To_4X_[u16_ClkParam]<<BIT_FCIE_CLK4X_SHIFT));

        REG_FCIE_CLRBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		//REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_DDR_TIMING_PATCH);
        REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z_EN);
		REG_FCIE_SETBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_CLRBIT(reg_chip_dummy1,BIT_SW_RST_Z);
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_SD_EN);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG,BIT_SD_DDR_EN);
	}

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "clk:%uKHz, Param:%Xh, fcie_1X(%Xh):%Xh, fcie_4X(%Xh):%Xh\n",
		g_eMMCDrv.u32_ClkKHz, u16_ClkParam,
		reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X),
		reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));

	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
	REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}


U8 gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_40M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_27M,
	BIT_FCIE_CLK_20M,
};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T","0.5T","  1T","1.5T", "  2T", "2.5T", "  3T", "3.5T"};

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
	U32 u32_SectorAddr;

	u32_SectorAddr = eMMC_TEST_BLK_0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel);
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr);
}

#define FCIE_DELAY_CELL_ts         300 // 0.7ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
		{
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
		return eMMC_ST_SUCCESS;
	else
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;

}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx = 0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;

	if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)){
		//eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    printf("eMMC Err: set DDR IF fail: %X\n", u32_err);
		    return u32_err;
		}
	}
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau8_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau8_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk = gau8_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 7 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 7)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u8_Clk)
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk == gau8_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u8_Clk != gau8_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		u32_ret = eMMC_ST_ERR_BUILD_DDRT;
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail! please call CAE or RD for helps, %Xh \n",
			u32_ret);
		eMMC_ResetReadyFlag();
		eMMC_Init();
		eMMC_FCIE_EnableSDRMode();

	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING


void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

//---------------------------------------
#ifdef CONFIG_OUTER_CACHE
#include <asm/outercache.h>
#endif
#define HalUtilPHY2MIUAddr(addr)        addr
#define HalUtilMIU2PHYAddr(addr)        addr

#define SYSHAL_DCACHE_LINE_SIZE 32

#define HAL_DCACHE_START_ADDRESS(_addr_) \
    (((u32)(_addr_)) & ~(SYSHAL_DCACHE_LINE_SIZE-1))

#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
    (((u32)((_addr_) + (_asize_) + (SYSHAL_DCACHE_LINE_SIZE-1) )) & \
     ~(SYSHAL_DCACHE_LINE_SIZE-1))

static void _hal_dcache_flush(void *base , u32 asize)
{
    register u32 _addr_ = HAL_DCACHE_START_ADDRESS((u32)base);
    register u32 _eaddr_ = HAL_DCACHE_END_ADDRESS((u32)(base), asize);

    for( ; _addr_ < _eaddr_; _addr_ += SYSHAL_DCACHE_LINE_SIZE )
        __asm__ __volatile__ ("MCR p15, 0, %0, c7, c14, 1" : : "r" (_addr_));

    /* Drain write buffer */
    _addr_ = 0x00UL;
    __asm__ __volatile__ ("MCR p15, 0, %0, c7, c10, 4" : : "r" (_addr_));
}

#define dma_dcache_flush(addr, size) _hal_dcache_flush(addr, size)
#define dma_dcache_invalidate(addr, size)
#ifdef CONFIG_OUTER_CACHE
#define dma_dcache_flush_invalidate(addr, size) \
    do{ \
        _hal_dcache_flush((void *)addr, size); \
        outer_flush_range(__pa(addr),__pa(addr) + size); \
        outer_inv_range(__pa(addr),__pa(addr) + size); \
    }while(0)
#else
#define dma_dcache_flush_invalidate(addr, size) _hal_dcache_flush((void *)addr, size)
#endif

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt)
{
	U32 dmaaddr;
		
	dma_dcache_flush_invalidate(u32_DMAAddr, u32_ByteCnt);
	dmaaddr = (virt_to_phys((void *)u32_DMAAddr));

	if( dmaaddr >= MSTAR_MIU1_BUS_BASE)
    {
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
        REG_FCIE_SETBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
    else
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);
    }
	return dmaaddr;

}

void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
{

}

void eMMC_flush_miu_pipe(void)
{

}


//---------------------------------------
#if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE

static DECLARE_WAIT_QUEUE_HEAD(fcie_wait);
static volatile U32 fcie_int = 0;

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	if((REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE) != BIT_EMMC_ACTIVE)
	{
		return IRQ_NONE;
	}

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
#endif

	return IRQ_NONE;

}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;

}


#endif


int mstar_mci_exit_checkdone_Ex(void)
{
    U16 u16Reg = 0;

    u16Reg = REG_FCIE(FCIE_SD_STATUS);

    if(u16Reg & BIT_SD_D0)
        return 1;
    else
        return 0;
}

bool mstar_mci_exit_checkdone_ForCI(void)
{
	#if 0
	U32 u32_Cnt=0;

    // eiffel eMMC_mode_1 share pins
	if(BIT_eMMC_CFG_MODE1 != (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
		return true;

	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			eMMC_debug(0,1,"eMMC Info: PCM check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}
	#endif
	return true;
}

EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForCI);


bool mstar_mci_exit_checkdone_ForSD(void)
{
	U32 u32_Cnt=0;

    // eiffel SD_mode_x does not share pins
	//if(BIT_eMMC_CFG_MODE2 == (REG_FCIE(reg_chiptop_0x5D)&BIT_eMMC_CFG_MASK))
	//	return TRUE;
    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	while(mstar_mci_exit_checkdone_Ex() == 0) /* card busy */
	{
		if ((++u32_Cnt) >= HW_TIMER_DELAY_1s*10)
		{
			REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
			eMMC_debug(0,1,"eMMC Info: SD check -> D0 busy\n");
			return false;
		}

		eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

		if(0 == (u32_Cnt%HW_TIMER_DELAY_100us))
		{
			msleep(1);
			u32_Cnt += HW_TIMER_DELAY_1ms;
		}
	}

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	return true;
}
EXPORT_SYMBOL(mstar_mci_exit_checkdone_ForSD);



//---------------------------------------
extern struct semaphore	PfModeSem;
extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	down(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
	REG_FCIE_SETBIT(FCIE_REG16h, BIT_EMMC_ACTIVE);	
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	up(&PfModeSem);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

//---------------------------------------

U32 eMMC_PlatformResetPre(void)
{
	/**((volatile unsigned short *)(0x25007DCC))|=0x02;	// emi mask
	*((volatile unsigned short *)(0x25007C18))|=0x02;	// imi0 mask
	*((volatile unsigned short *)(0x25007C58))|=0x02;	// imi1 mask
	*/
	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
	/**((volatile unsigned short *)(0x25007DCC))&=(~0x02);	// emi unmask
	*((volatile unsigned short *)(0x25007C18))&=(~0x02);	// imi0 unmask
	*((volatile unsigned short *)(0x25007C58))&=(~0x02);	// imi1 unmask
	*/
	return eMMC_ST_SUCCESS;
}


U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_DEFAULT_PAD);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}


char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_CheckIfMemCorrupt(void)
{
	if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
		return eMMC_ST_ERR_MEM_CORRUPT;

	return eMMC_ST_SUCCESS;
}
// <-- [FIXME]

//=============================================================
#elif (defined(eMMC_DRV_CEDRIC_LINUX)&&eMMC_DRV_CEDRIC_LINUX)

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

#include <linux/delay.h>
U32 eMMC_hw_timer_delay(U32 u32usTick)  
{
  #if 1	//xxx tomodify
  udelay(u32usTick); 
  #else
	volatile U32 i = 0;

	for (i = 0; i < (u32usTick>>4); i++)
	{
		#if 0
		volatile int j = 0, tmp;
		for (j = 0; j < 0; j++)
		{
			tmp = j;
		}
		#endif
	}
  #endif

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
	// TIMER_FREERUN_32K  32 KHz
	// TIMER_FREERUN_XTAL 38.4 MHz,
	// counting down
	return 0;//HalTimerRead(TIMER_FREERUN_32K);
}
//--------------------------------


void eMMC_DumpPadClk(void)
{
#if 0
	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
	eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
	eMMC_debug(0, 1, "CMU (0x%X):0x%X\n", reg_ckg_CMU, REG_FCIE_U16(reg_ckg_CMU));

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u32_Flag & FCIE_FLAG_PADTYPE_MASK)
	{
	case FCIE_FLAG_PADTYPE_DDR:
		eMMC_debug(0,0,"DDR\n");  break;
	case FCIE_FLAG_PADTYPE_SDR:
		eMMC_debug(0,0,"SDR\n");  break;
	case FCIE_FLAG_PADTYPE_BYPASS:
		eMMC_debug(0,0,"BYPASS\n");  break;
	default:
		eMMC_debug(0,0,"eMMC Err: Pad unknown\n");  eMMC_die("\n");
	}
	eMMC_debug(0, 1, "chiptop_0x5A (0x%X):0x%X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
	eMMC_debug(0, 1, "chiptop_0x1F (0x%X):0x%X\n", reg_chiptop_0x1F, REG_FCIE_U16(reg_chiptop_0x1F));
	eMMC_debug(0, 1, "chiptop_0x70 (0x%X):0x%X\n", reg_chiptop_0x70, REG_FCIE_U16(reg_chiptop_0x70));
	eMMC_debug(0, 1, "chiptop_0x64 (0x%X):0x%X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
	eMMC_debug(0, 1, "chiptop_0x4F (0x%X):0x%X\n", reg_chiptop_0x4F, REG_FCIE_U16(reg_chiptop_0x4F));
	eMMC_debug(0, 1, "chiptop_0x03 (0x%X):0x%X\n", reg_chiptop_0x03, REG_FCIE_U16(reg_chiptop_0x03));
	eMMC_debug(0, 1, "chiptop_0x51 (0x%X):0x%X\n", reg_chiptop_0x51, REG_FCIE_U16(reg_chiptop_0x51));
	eMMC_debug(0, 1, "chiptop_0x6F (0x%X):0x%X\n\n", reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));

	eMMC_debug(0, 1, "chiptop_0x43 (0x%X):0x%X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
	eMMC_debug(0, 1, "chiptop_0x50 (0x%X):0x%X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
	eMMC_debug(0, 1, "chiptop_0x0B (0x%X):0x%X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
	eMMC_debug(0, 1, "chiptop_0x5D (0x%X):0x%X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

	eMMC_debug(0, 1, "\n");
#endif
}

U8 gu8_NANDeMMC_need_preset_flag = 1;
#define BIT_BOOT_CONFIG_MODE_MASK				BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
		//only for Cedric	eMMC
	if (gu8_NANDeMMC_need_preset_flag == 1)
	{
		eMMC_printf("========== gu8_NANDeMMC_need_preset_flag = 1 ==========");
		//REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_NEW_DESIGN_ENn);
		REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT3);
		eMMC_hw_timer_delay(HW_TIMER_DELAY_10us);
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT3);
	
		gu8_NANDeMMC_need_preset_flag = 0;
	}
	
	
	
//	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
//			    BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

	switch(u32_FCIE_IF_Type)
	{
		case FCIE_eMMC_DDR:
				eMMC_printf("eMMC_pads_switch FCIE_eMMC_DDR\r\n");
				#define R_DDR_MODE	BIT_MACRO_EN | BIT_SD_DDR_EN
			  REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_CONFIG_MODE_MASK);
  			REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_DDR_MODE);
			  REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
				REG_FCIE_CLRBIT(REG_BPS_MODE, 0x3FF);
			  REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
  			REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);  
  
  			REG_FCIE_SETBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);
			
				// set chiptop
				g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
				break;

		case FCIE_eMMC_SDR:
				eMMC_printf("eMMC_pads_switch FCIE_eMMC_SDR\r\n");
				#define R_SDR_MODE		BIT_MACRO_EN
			  REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_CONFIG_MODE_MASK);
			  REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_SDR_MODE);
			  REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
				REG_FCIE_CLRBIT(REG_BPS_MODE, 0x3FF);
			  REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
			  REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);
			 	REG_FCIE_SETBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);

				g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
				break;
		case FCIE_eMMC_BYPASS:
				eMMC_printf("eMMC_pads_switch FCIE_eMMC_BYPASS\r\n");
				#define R_BYPASS_MODE		BIT_MACRO_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS
  			REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_CONFIG_MODE_MASK);
			  REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_BYPASS_MODE);
			  REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
				REG_FCIE_SETBIT(REG_BPS_MODE, 0x3FF);
			  REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
			  REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);
  		  REG_FCIE_CLRBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);

				g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
				break;

		default:
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
				return eMMC_ST_ERR_INVALID_PARAM;
	}

	REG_FCIE_SETBIT(FCIE_REG_2Dh, BIT_NEW_DESIGN_ENn);

	return eMMC_ST_SUCCESS;
}

 
U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	switch(u16_ClkParam)	{
    case BIT_FCIE_CLK_24M:		g_eMMCDrv.u32_ClkKHz = 24000;		break;
    case BIT_FCIE_CLK_12M:		g_eMMCDrv.u32_ClkKHz = 12000;	break;
    case BIT_FCIE_CLK_48M:		g_eMMCDrv.u32_ClkKHz = 48000;	break;
    case BIT_FCIE_CLK_86M:		g_eMMCDrv.u32_ClkKHz = 86000;	break;
    case BIT_FCIE_CLK_108M:		g_eMMCDrv.u32_ClkKHz = 108000;	break;
    case BIT_FCIE_CLK_123M:		g_eMMCDrv.u32_ClkKHz = 123000;	break;
    case BIT_FCIE_CLK_144M:		g_eMMCDrv.u32_ClkKHz = 144000;	break;
    case BIT_FCIE_CLK_172M:		g_eMMCDrv.u32_ClkKHz = 172000;	break;
    case BIT_FCIE_CLK_192M:		g_eMMCDrv.u32_ClkKHz = 192000;	break;
    case BIT_FCIE_CLK_216M:		g_eMMCDrv.u32_ClkKHz = 216000;		break;
    case BIT_FCIE_CLK_240M:		g_eMMCDrv.u32_ClkKHz = 240000;	break;
    case BIT_FCIE_CLK_345M:		g_eMMCDrv.u32_ClkKHz = 345000;	break;
    case BIT_FCIE_CLK_432M:		g_eMMCDrv.u32_ClkKHz = 432000;	break;
    case BIT_FCIE_CLK_1200K:	g_eMMCDrv.u32_ClkKHz = 1200;	break;
    case BIT_FCIE_CLK_300K:		g_eMMCDrv.u32_ClkKHz = 300;	break;
    case BIT_FCIE_CLK_32M:		g_eMMCDrv.u32_ClkKHz = 32000;	break;
    case BIT_FCIE_CLK_36M:		g_eMMCDrv.u32_ClkKHz = 36000;	break;
    case BIT_FCIE_CLK_40M:		g_eMMCDrv.u32_ClkKHz = 40000;	break;
    case BIT_FCIE_CLK_43_2M:	g_eMMCDrv.u32_ClkKHz = 43200;	break;
    case BIT_FCIE_CLK_54M:		g_eMMCDrv.u32_ClkKHz = 54000;	break;
    case BIT_FCIE_CLK_62M:		g_eMMCDrv.u32_ClkKHz = 62000;	break;
    case BIT_FCIE_CLK_72M:		g_eMMCDrv.u32_ClkKHz = 72000;	break;
    case BIT_FCIE_CLK_80M:		g_eMMCDrv.u32_ClkKHz = 80000;	break;

		default:
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
			return eMMC_ST_ERR_INVALID_PARAM;
	}
	
	REG_FCIE_W(REG_CLK_EMMC, (u16_ClkParam));   
	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
	
  eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U32 eMMC_clock_gating(void) 
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(REG_CLK_EMMC, BIT_FCIE_CLK_Gate);
	//REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U16 gau16_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={   

	BIT_FCIE_CLK_192M, BIT_FCIE_CLK_172M,	/*BIT_FCIE_CLK_144M,*/ BIT_FCIE_CLK_123M, /*BIT_FCIE_CLK_108M,*/
	BIT_FCIE_CLK_86M, /*BIT_FCIE_CLK_80M, BIT_FCIE_CLK_72M,*/	BIT_FCIE_CLK_62M, BIT_FCIE_CLK_54M, 
	/*BIT_FCIE_CLK_48M, BIT_FCIE_CLK_43_2M,*/ BIT_FCIE_CLK_40M, /*BIT_FCIE_CLK_36M,*/	BIT_FCIE_CLK_32M,
	BIT_FCIE_CLK_24M, BIT_FCIE_CLK_12M/*, BIT_FCIE_CLK_1200K, BIT_FCIE_CLK_300K*/
};
#define FCIE_SLOW_CLK_4X								BIT_FCIE_CLK_12M

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T", "1.5T", "2.5T","  1T"}; 

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel) 
{
	U32 u32_SectorAddr;
	
	//eMMC_printf("eMMC_FCIE_DetectDDRTiming_Ex\r\n");

	u32_SectorAddr = eMMC_TEST_BLK_0;

	//eMMC_printf("eMMC_TEST_BLK_0: %X\r\n", u32_SectorAddr);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel); 
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr); 
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2]; 

U32 eMMC_FCIE_DetectDDRTiming(void)  
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  au8_DQS_10T[4]={0,15,20,25}; // 0T, 1.5T, 2T, 2.5T
	
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];
	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
//	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))   
		{
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
			eMMC_printf("u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT)\r\n");
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)   
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);    
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_printf("DetectDDRTiming pass\r\n");
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
	{
		eMMC_printf("eMMC_FCIE_DetectDDRTiming OK\r\n");
		return eMMC_ST_SUCCESS;
	}
	else
	{
		eMMC_printf("eMMC_FCIE_DetectDDRTiming OK\r\n");
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;
	}
}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8 au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	//eMMC_printf("eMMC Info: building DDR table, please wait... \n");

	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;



	//iantest ttt
	if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
	{
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();   
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    //RETAILMSG(1, (TEXT("eMMC Error: set DDR IF fail: %Xh\r\n"),u32_err));
		    eMMC_printf("eMMC Error: set DDR IF fail: %Xh\r\n",u32_err);
		    return u32_err;
		}
	}
	
	eMMC_pads_switch(FCIE_eMMC_DDR);	//iantest ttt
   //iantest ttt
   
   
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau16_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();           

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau16_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			eMMC_printf("OK\r\n");

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk = gau16_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK_4X == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk)
			{
				eMMC_printf("FCIE_SLOW_CLK_4X == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk\r\n");
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			}
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	
	eMMC_printf("gau16_FCIEClkSel[0] : %X\r\n", gau16_FCIEClkSel[0]);
	eMMC_printf("g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk : %X\r\n", g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk);
	eMMC_printf("g_eMMCDrv.DDRTable.u8_SetCnt : %X\r\n", g_eMMCDrv.DDRTable.u8_SetCnt);   
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk == gau16_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk != gau16_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		eMMC_FCIE_EnableSDRMode();
	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

#define MSTAR_MIU0_BUS_BASE                      0x40000000
#define MSTAR_MIU1_BUS_BASE                      0xA0000000


U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
#if 1
		extern void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
		extern void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
    //mode 0 for write, 1 for read
    if( mode == WRITE_TO_eMMC )	//Write
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

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}

	#if eMMC_IRQ_DEBUG
	if(0==fcie_int)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: Int St:%Xh, En:%Xh, Evt:%Xh \n",
		    REG_FCIE(FCIE_MIE_EVENT), REG_FCIE(FCIE_MIE_INT_EN), u16_Events);
	#endif

	return IRQ_NONE;
}


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}

#endif



//---------------------------------------
//extern struct semaphore	PfModeSem;
//#include <linux/semaphore.h>
//extern struct semaphore	PfModeSem;
extern struct mutex FCIE3_mutex;
//#define CRIT_SECT_BEGIN(x)	mutex_lock(x)
//#define CRIT_SECT_END(x)	mutex_unlock(x)

extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	mutex_lock(&FCIE3_mutex);
	//eMMC_debug(0,1,"%s 2\n", pu8_str);

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif


	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock	
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	//eMMC_debug(0,1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
	mutex_unlock(&FCIE3_mutex);
	
	//eMMC_debug(0,1,"%s 2\n", pu8_str);
}

U32 eMMC_PlatformResetPre(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

	return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}

char *gpas8_eMMCPartName[]={
	"e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
	"fdd", "tdd", "blogo", "apanic","misc", "cus",
	"e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

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

//=============================================================
#elif (defined(eMMC_DRV_CHICAGO_LINUX)&&eMMC_DRV_CHICAGO_LINUX)

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

#include <linux/delay.h>
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
//--------------------------------


void eMMC_DumpPadClk(void)
{
	//----------------------------------------------
	eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
	eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", REG_CLK_EMMC, REG_FCIE_U16(REG_CLK_EMMC));
	eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", REG_CLK_4X_DIV_EN, REG_FCIE_U16(REG_CLK_4X_DIV_EN));

	//----------------------------------------------
	eMMC_debug(0, 1, "\n[pad setting]: ");
	switch(g_eMMCDrv.u8_PadType)
	{
    	case FCIE_eMMC_DDR:
    		eMMC_debug(0,0,"DDR\n");  
    		break;
    	case FCIE_eMMC_SDR:
    		eMMC_debug(0,0,"SDR\n");  
    		break;
    	case FCIE_eMMC_BYPASS:
    		eMMC_debug(0,0,"BYPASS\n");  
    		break;
    	default:
    		eMMC_debug(0,0,"eMMC Err: Pad unknown\n");  
    		eMMC_die("\n");
	}
	eMMC_debug(0, 1, "padtop_0x00 (0x%X):0x%X\n", REG_PADTOP_00, REG_FCIE_U16(REG_PADTOP_00));
	eMMC_debug(0, 1, "padtop_0x01 (0x%X):0x%X\n", REG_PADTOP_01, REG_FCIE_U16(REG_PADTOP_01));
	eMMC_debug(0, 1, "padtop_0x40 (0x%X):0x%X\n", REG_PADTOP_40, REG_FCIE_U16(REG_PADTOP_40));
	eMMC_debug(0, 1, "padtop_0x43 (0x%X):0x%X\n", REG_PADTOP_43, REG_FCIE_U16(REG_PADTOP_43));
	eMMC_debug(0, 1, "chiptop_0x31 (0x%X):0x%X\n", REG_RESET_PIN, REG_FCIE_U16(REG_RESET_PIN));

	eMMC_debug(0, 1, "fcie_0x2F (0x%X):0x%X\n", FCIE_BOOT_CONFIG, REG_FCIE_U16(FCIE_BOOT_CONFIG));
	eMMC_debug(0, 1, "fcie_0x2D (0x%X):0x%X\n", FCIE_REG_2Dh, REG_FCIE_U16(FCIE_REG_2Dh));


	eMMC_debug(0, 1, "\n");

}

U8 gu8_NANDeMMC_need_preset_flag = 1;

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
	U16 u16_reg;	
	switch(u32_FCIE_IF_Type)
	{
		case FCIE_eMMC_DDR:
	        eMMC_debug(0,0,"eMMC_pads_switch FCIE_eMMC_DDR\r\n");
  			REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
  			u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
  			u16_reg |= (BIT_MACRO_EN | BIT_SD_DDR_EN);
  			REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);
  	
  			REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT14);
  	
  			//CLRREG16(FCIE_BOOT_CONFIG, BIT3);	//to MIU
  			REG_FCIE_R(REG_PADTOP_00, u16_reg);
			u16_reg |= BIT15;
			u16_reg &= ~(BIT0 + BIT14);
			REG_FCIE_W(REG_PADTOP_00, u16_reg);
  
 			REG_FCIE_R(REG_PADTOP_01, u16_reg);
			u16_reg &= ~(BIT3);
			REG_FCIE_W(REG_PADTOP_01, u16_reg);
  
  			REG_FCIE_R(REG_PADTOP_40, u16_reg);
			u16_reg &= ~(BIT3 + BIT8 + BIT9);
			REG_FCIE_W(REG_PADTOP_40, u16_reg);

  			REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);
	
	 		REG_FCIE_SETBIT(REG_CLK_4X_DIV_EN, BIT0);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
			break;

		case FCIE_eMMC_SDR:
			eMMC_debug(0,0,"eMMC_pads_switch FCIE_eMMC_SDR\r\n");
  			REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
  			u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
  			u16_reg |= (BIT_MACRO_EN);
		  	REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);
  	
  			REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT14);
  	
  			//CLRREG16(FCIE_BOOT_CONFIG, BIT3);	//to MIU
		  	REG_FCIE_R(REG_PADTOP_00, u16_reg);
			u16_reg |= BIT15;
			u16_reg &= ~(BIT0 + BIT14);
			REG_FCIE_W(REG_PADTOP_00, u16_reg);
  
 			REG_FCIE_R(REG_PADTOP_01, u16_reg);
			u16_reg &= ~(BIT3);
			REG_FCIE_W(REG_PADTOP_01, u16_reg);
  
  			REG_FCIE_R(REG_PADTOP_40, u16_reg);
			u16_reg &= ~(BIT3 + BIT8 + BIT9);
			REG_FCIE_W(REG_PADTOP_40, u16_reg);
  
  			REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);
			g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
			break;
			
			
		case FCIE_eMMC_BYPASS:
			eMMC_debug(0,0,"eMMC_pads_switch FCIE_eMMC_BYPASS\r\n");	
  			REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
  			u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
  			u16_reg |= (BIT_MACRO_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS);
		  	REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);
			
  			REG_FCIE_CLRBIT(FCIE_REG_2Dh, BIT14);
  	
  			//CLRREG16(FCIE_BOOT_CONFIG, BIT3);	//to MIU
			
  			REG_FCIE_R(REG_PADTOP_00, u16_reg);
  			u16_reg |= BIT15;
			u16_reg &= ~(BIT0 + BIT14);
			REG_FCIE_W(REG_PADTOP_00, u16_reg);
  
 			REG_FCIE_R(REG_PADTOP_01, u16_reg);
			u16_reg &= ~(BIT3);
			REG_FCIE_W(REG_PADTOP_01, u16_reg);
  
  			REG_FCIE_R(REG_PADTOP_40, u16_reg);
			u16_reg |= (BIT3 + BIT8 + BIT9);
			REG_FCIE_W(REG_PADTOP_40, u16_reg);
  
  			REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);
  	
			g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
			break;

		default:
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	return eMMC_ST_SUCCESS;

}

 
U32 eMMC_clock_setting(U16 u16_ClkParam)
{
	eMMC_PlatformResetPre();

	REG_FCIE_SETBIT(REG_CLK_4X_DIV_EN, BIT0);

	switch(u16_ClkParam) {
    case NFIE_REG_4XCLK_1_3M:		g_eMMCDrv.u32_ClkKHz = 1300;	break;
    case NFIE_REG_4XCLK_26M:		g_eMMCDrv.u32_ClkKHz = 26000;	break;
    case NFIE_REG_4XCLK_48M:		g_eMMCDrv.u32_ClkKHz = 48000;	break;
    case NFIE_REG_4XCLK_80M:		g_eMMCDrv.u32_ClkKHz = 80000;	break;
    case NFIE_REG_4XCLK_96M:		g_eMMCDrv.u32_ClkKHz = 96000;	break;
    case NFIE_REG_4XCLK_120M:		g_eMMCDrv.u32_ClkKHz = 120000;	break;
    case NFIE_REG_4XCLK_147M:		g_eMMCDrv.u32_ClkKHz = 	147000;	break;
    case NFIE_REG_4XCLK_160M:		g_eMMCDrv.u32_ClkKHz = 	160000;	break;
    case NFIE_REG_4XCLK_176_8M:		g_eMMCDrv.u32_ClkKHz = 176800;	break;
    case NFIE_REG_4XCLK_192M:		g_eMMCDrv.u32_ClkKHz = 192000;	break;
    case NFIE_REG_4XCLK_221M:		g_eMMCDrv.u32_ClkKHz = 221000;	break;
    case NFIE_REG_4XCLK_240M:		g_eMMCDrv.u32_ClkKHz = 240000;	break;
    case NFIE_REG_4XCLK_294_6M:		g_eMMCDrv.u32_ClkKHz = 294600;	break;
    case NFIE_REG_4XCLK_353_6M:		g_eMMCDrv.u32_ClkKHz = 353600;	break;
    case NFIE_REG_4XCLK_60M:		g_eMMCDrv.u32_ClkKHz =  60000;	break;
		default:
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
			return eMMC_ST_ERR_INVALID_PARAM;
	}

	//printf("REG_CLK_EMMC : %X, %uKHZ\r\n", u16_ClkParam, g_eMMCDrv.u32_ClkKHz/4);

	REG_FCIE_W(REG_CLK_EMMC, (u16_ClkParam));   
	g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;

	
  eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U32 eMMC_clock_gating(void) 
{
	eMMC_PlatformResetPre();
	g_eMMCDrv.u32_ClkKHz = 0;
	REG_FCIE_W(REG_CLK_EMMC, BIT_FCIE_CLK_Gate);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	eMMC_PlatformResetPost();
	return eMMC_ST_SUCCESS;
}

U16 gau16_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={   
	NFIE_REG_4XCLK_192M, NFIE_REG_4XCLK_160M, NFIE_REG_4XCLK_120M,
	NFIE_REG_4XCLK_96M ,NFIE_REG_4XCLK_80M

};

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  0T", "0.5T", "  1T", "1.5T", "  2T", "2.5T", "  3T", "3.5T"}; 

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel) 
{
	U32 u32_SectorAddr;
	
	//eMMC_printf("eMMC_FCIE_DetectDDRTiming_Ex\r\n");

	u32_SectorAddr = eMMC_TEST_BLK_0;

	//eMMC_printf("eMMC_TEST_BLK_0: %X\r\n", u32_SectorAddr);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
		sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

	eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel); 
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr); 
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2]; 

U32 eMMC_FCIE_DetectDDRTiming(void)  
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
	U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3,5T
	U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3,5T
	
	U8  u8_delay_Sel_max;
	U32 u32_ts;
	U32 u32_err;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];
	g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

	//--------------------------------------------------
	sg_DDRTWindow[0].u8_Cnt = 0;
	sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
	sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
	sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
	{
		sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
		sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
	}

	//--------------------------------------------------
	// calculate delay_Sel_max
	u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
//	u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
	//for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

		if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
		    u8_delay_Sel_max =
		        (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
		        *u32_ts/(FCIE_DELAY_CELL_ts*10);
		else
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

		if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))   
		{
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
			//eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
			//continue;
			eMMC_printf("u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT)\r\n");
			u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
		}
		else
			eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

		sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
		sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

		//for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
		for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)   
		{
			u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);    
			if(eMMC_ST_SUCCESS == u32_err)
			{
				eMMC_printf("DetectDDRTiming pass\r\n");
				eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
				pWindow->u8_Cnt++;
				if(0 == pWindow->aParam[0].u8_DQS) // save the window head
				{
					pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
					pWindow->aParam[0].u8_Cell = u8_delay_sel;
				}
				pWindow->au8_DQSValidCellCnt[u8_dqs]++;

				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			}
			else
			{   // save the window tail
				if(0xFF != u8_dqs_prev)
				{
					pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
					pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;

				// discard & re-use the window having less PASS cnt
				pWindow =
				    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
				pWindow->u8_Cnt = 0;
				pWindow->aParam[0].u8_DQS = 0;
				pWindow->aParam[1].u8_DQS = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
					pWindow->au8_DQSValidCellCnt[u8_i]=0;
			}
		}
	}

	// for the case of last try is ok
	if(0xFF != u8_dqs_prev)
	{
		pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
		pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
	}

	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[0].u8_Cnt,
		sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
		sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		sg_DDRTWindow[1].u8_Cnt,
		sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
		sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
	for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
			u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

	if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
	{
		eMMC_printf("eMMC_FCIE_DetectDDRTiming OK\r\n");
		return eMMC_ST_SUCCESS;
	}
	else
	{
		eMMC_printf("eMMC_FCIE_DetectDDRTiming OK\r\n");
		return eMMC_ST_ERR_NO_OK_DDR_PARAM;
	}
}


U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
	U8 au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
	U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
	U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
	eMMC_FCIE_DDRT_WINDOW_t *pWindow;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
	//eMMC_printf("eMMC Info: building DDR table, please wait... \n");

	memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
	u8_SetIdx = 0;



	//iantest ttt
	if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
	{
		u32_err = eMMC_FCIE_EnableDDRMode_Ex();   
		if(eMMC_ST_SUCCESS != u32_err)
		{
		    //RETAILMSG(1, (TEXT("eMMC Error: set DDR IF fail: %Xh\r\n"),u32_err));
		    eMMC_printf("eMMC Error: set DDR IF fail: %Xh\r\n",u32_err);
		    return u32_err;
		}
	}
	
	eMMC_pads_switch(FCIE_eMMC_DDR);	//iantest ttt
   //iantest ttt
   
   
	for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
	{

		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
		eMMC_clock_setting(gau16_FCIEClkSel[u8_ClkIdx]);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

		// ---------------------------
		// search and set the Windows
		u32_err = eMMC_FCIE_DetectDDRTiming();           

		// ---------------------------
		// set the Table
		if(eMMC_ST_SUCCESS == u32_err)
		{
			#if 0
			// before 12MHz, should at least 2 clk is ok for DDR
			if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
				BIT_FCIE_CLK_12M == gau16_FCIEClkSel[u8_ClkIdx])
				u32_ret = eMMC_ST_SUCCESS;
			#endif

			eMMC_printf("OK\r\n");

			g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk = gau16_FCIEClkSel[u8_ClkIdx];

			// ---------------------------
		 	// select Window
			pWindow = NULL;

			// pick up the Window of Cell=0 case
			if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[0];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow = &sg_DDRTWindow[1];
			else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
			   sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
			   pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

			// ---------------------------
			if(NULL != pWindow)
			{

				// pick up the DQS having max valid cell
				u8_tmp = 0;
				for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
				{
					if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
						u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
						u8_DqsIdx = u8_i;
					}
				}
				if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
				{
					if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 &&
						pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
					{
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
						g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
					}
					else
					{
						u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
							pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


						if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
						{
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx-1];

							u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
							g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
						}
						else
						{   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
								au8_DQSRegVal[u8_DqsIdx];
						    g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
								(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
								pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
						}
					}
				}
			}

			// ---------------------------
            // or, pick up the Window of large PASS Cnt
			else //if(NULL == pWindow)
			{
			    pWindow =
				    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
				    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
				g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
					(pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
			}

			#if 0
			// ---------------------------
			// use 12M for Set.Min
			if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
				u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
			#else
			if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk)
			{
				eMMC_printf("FCIE_SLOW_CLK_4X == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk\r\n");
				g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
			}
			#endif

			u8_SetIdx++;
		}
	}

	// dump DDRT Set
	u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
	    eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
	u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
		g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

	eMMC_DumpDDRTTable();

	// ======================================================
	// CAUTION: expect 48MHz can have valid DDRT parameter
	
	eMMC_printf("gau16_FCIEClkSel[0] : %X\r\n", gau16_FCIEClkSel[0]);
	eMMC_printf("g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk : %X\r\n", g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk);
	eMMC_printf("g_eMMCDrv.DDRTable.u8_SetCnt : %X\r\n", g_eMMCDrv.DDRTable.u8_SetCnt);   
	if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk == gau16_FCIEClkSel[0]
		&& 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
		u32_ret = eMMC_ST_SUCCESS;
	else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk != gau16_FCIEClkSel[0])
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
	else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
	// ======================================================

	// ---------------------------
	// save DDRT Table
	if(eMMC_ST_SUCCESS == u32_ret)
	{

		g_eMMCDrv.DDRTable.u32_ChkSum =
		    eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
		memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

		eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

		u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
	    u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
	    if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
	    {
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
			    u32_err, u32_ret);
		    return eMMC_ST_ERR_SAVE_DDRT_FAIL;
	    }
	}
	else
	{
		gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
		gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
		gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
		gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

		eMMC_FCIE_EnableSDRMode();
	}

	return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
	// do nothing
}

void eMMC_reset_WatchDog(void)
{
	// do nothing
}

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
#if 1
		extern void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
		extern void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
    //mode 0 for write, 1 for read
    if( mode == WRITE_TO_eMMC )	//Write
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

#define eMMC_IRQ_DEBUG    1

irqreturn_t eMMC_FCIE_IRQ(int irq, void *dummy)
{
	volatile u16 u16_Events;

	// one time enable one bit
	u16_Events = REG_FCIE(FCIE_MIE_EVENT) & REG_FCIE(FCIE_MIE_INT_EN);

	if(u16_Events & BIT_MIU_LAST_DONE)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
    else if(u16_Events & BIT_CARD_DMA_END)
    {
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);

        fcie_int = 1;
        wake_up(&fcie_wait);
		return IRQ_HANDLED;
    }
	else if(u16_Events & BIT_SD_CMD_END)
	{
		REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_CMD_END);

		fcie_int = 1;
		wake_up(&fcie_wait);
		return IRQ_HANDLED;
	}
    #if defined(ENABLE_FCIE_HW_BUSY_CHECK)&&ENABLE_FCIE_HW_BUSY_CHECK
	else if(u16_Events & BIT_SD_BUSY_END)
	{
        REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, BIT_SD_BUSY_END);
		REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_SD_BUSY_DET_ON);
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


U32 eMMC_WaitCompleteIntr(U32 u32_RegAddr, U16 u16_WaitEvent, U32 u32_MicroSec)
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
			eMMC_FCIE_ErrHandler_Stop();
			return eMMC_ST_ERR_INT_TO;
		}
		else
		{	REG_FCIE_CLRBIT(FCIE_MIE_INT_EN, u16_WaitEvent);
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: but polling ok: %Xh \n", REG_FCIE(u32_RegAddr));
		}

    }

	//----------------------------------------
	if(u16_WaitEvent & BIT_MIU_LAST_DONE)
	{
		for(u32_i=0; u32_i<TIME_WAIT_1_BLK_END; u32_i++)
		{
			if(REG_FCIE(u32_RegAddr) & BIT_CARD_DMA_END)
				break; // should be very fase
			eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
		}

		if(TIME_WAIT_1_BLK_END == u32_i)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no CARD_DMA_END\n");
			eMMC_FCIE_ErrHandler_Stop();
		}
	}

	//----------------------------------------		
	fcie_int = 0;
	return eMMC_ST_SUCCESS;
}

#endif



//---------------------------------------
//extern struct semaphore	PfModeSem;
//#include <linux/semaphore.h>
//extern struct semaphore	PfModeSem;
extern struct mutex FCIE3_mutex;
//#define CRIT_SECT_BEGIN(x)	mutex_lock(x)
//#define CRIT_SECT_END(x)	mutex_unlock(x)

extern bool ms_sdmmc_wait_d0_for_emmc(void);

void eMMC_LockFCIE(U8 *pu8_str)
{
	mutex_lock(&FCIE3_mutex);
	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
	if(false == ms_sdmmc_wait_d0_for_emmc())
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SD keep D0 low \n");
		eMMC_FCIE_ErrHandler_Stop();
	}

	eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	eMMC_pads_switch(g_eMMCDrv.u8_PadType); 
	#endif	
	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // output clock
}

void eMMC_UnlockFCIE(U8 *pu8_str)
{
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN); // not output clock
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

U32 eMMC_PlatformInit(void)
{
	eMMC_pads_switch(FCIE_eMMC_SDR);
	eMMC_clock_setting(FCIE_SLOWEST_CLK);

	return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_StartSector += BL_BLK_OFFSET;
		break;

	case eMMC_PART_OTP:
		u32_StartSector += OTP_BLK_OFFSET;
		break;

	case eMMC_PART_SECINFO:
		u32_StartSector += SecInfo_BLK_OFFSET;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

	if(eMMC_BOOT_PART_W == u8_OP)
		return eMMC_WriteBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);
	else
		return eMMC_ReadBootPart(pDataBuf,
		    u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
		    u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
	U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

	switch(u16_PartType)
	{
	case eMMC_PART_BL:
		u32_eMMCBlkAddr_start = 0;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
		break;

	case eMMC_PART_OTP:
		u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
		break;

	case eMMC_PART_SECINFO:
		u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
		u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
		break;

	default:
		return eMMC_ST_SUCCESS;
	}

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
		u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

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


#else


  #error "Error! no platform functions."
#endif
#endif
