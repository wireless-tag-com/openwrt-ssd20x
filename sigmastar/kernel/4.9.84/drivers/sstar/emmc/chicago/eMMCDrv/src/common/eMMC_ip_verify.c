/*
* eMMC_ip_verify.c- Sigmastar
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
#if defined (UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER


//=============================================================
#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMCTest_BlkWRC_ProbeDDR(U32 u32_eMMC_Addr)
{
	U32 u32_err;
	U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf_DDR, *pu32_R=(U32*)gau8_RBuf_DDR;
	U32 u32_BlkCnt=eMMC_TEST_BLK_CNT, u32_BufByteCnt;
	
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


// can use for RF team test
U32 eMMCTest_KeepR_TestDDR(U32 u32_LoopCnt)
{
	U32 u32_err;
	U32 u32_i, u32_j, u32_k;
	U32 u32_BlkCnt, u32_eMMC_Addr;

	u32_eMMC_Addr = eMMC_TEST_BLK_0;
		
	u32_BlkCnt = eMMC_TEST_BLK_CNT;
	u32_BlkCnt = u32_BlkCnt > DDR_TEST_BLK_CNT ? DDR_TEST_BLK_CNT : u32_BlkCnt;
    
	if(8 != u32_BlkCnt)
	{
		eMMC_debug(0,1,"Blk count needs to be 8 \n");
		while(1);
	}
	
	for(u32_j=0; u32_j<u32_BlkCnt; u32_j++){
		// init data pattern
		switch(u32_j)
		{
			case 0: // increase
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE; u32_i++)
	                gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i] = u32_i+1;
			    break;
			case 1: // 0xF00F
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xF0;  
				    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0x0F;
				}
				break;
			case 2: // 0xFF00
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xFF;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0;
				}
				break;				
			case 3: // 0x5AA5
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x5A;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xA5;
				}
				break;				
			case 4: // 0x55AA
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x55;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xAA;
				}
				break;	
			case 5: // 0x5A5A
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x5A;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0x5A;
				}
				break;				
			case 6: // 0x0000
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0;
				}
				break;				
			case 7: // 0xFFFF
				for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
				{
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xFF;  
					gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xFF;
				}
				break;
			
		}

		u32_err = eMMC_CMD25_MIU(u32_eMMC_Addr, gau8_WBuf_DDR, u32_BlkCnt);
        if(eMMC_ST_SUCCESS != u32_err)
	        break;
	}


	for(u32_k=0; u32_k<u32_LoopCnt; u32_k++){

	    u32_err = eMMC_CMD18_MIU(u32_eMMC_Addr, gau8_RBuf_DDR, u32_BlkCnt/2);
	    if(eMMC_ST_SUCCESS != u32_err)
            break;
				
	    u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, (u32_BlkCnt/2)<<eMMC_SECTOR_512BYTE_BITS);
    	if(eMMC_ST_SUCCESS != u32_err)
	    {
		    eMMC_debug(0,1,"Err 1, compare fail, %Xh \n", u32_err);
            break;
	    }

		for(u32_j=0; u32_j<u32_BlkCnt/2; u32_j++)
		{
			u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr+u32_BlkCnt/2+u32_j, 
				gau8_RBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS));
	        if(eMMC_ST_SUCCESS != u32_err)
                break;

			u32_err = eMMC_ComapreData(
				gau8_WBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS), 
				gau8_RBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS), 
				eMMC_SECTOR_512BYTE);
    	    if(eMMC_ST_SUCCESS != u32_err)
	        {
		        eMMC_debug(0,1,"Err 2, compare fail, %Xh \n", u32_err);
                break;
	        }
		}
	}
	
	return u32_err;
}


#endif


//=============================================================
#if defined(IF_IP_VERIFY) && IF_IP_VERIFY

#define TEST_BUFFER_SIZE    0x100000 // 1MB
eMMC_ALIGN0 static U8 gau8_WBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMC_BootMode(void);
#define eMMC_PATTERN_00000000    0x00000000 // smooth
#define eMMC_PATTERN_FFFFFFFF    0xFFFFFFFF // smooth
#define eMMC_PATTERN_000000FF    0x000000FF // board
#define eMMC_PATTERN_0000FFFF    0x0000FFFF // board
#define eMMC_PATTERN_00FF00FF    0x00FF00FF // board
#define eMMC_PATTERN_AA55AA55    0xAA55AA55 // board

U32  eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_MultiBlkWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_Ex(U32 u32_DataPattern);
void eMMCTest_DownCount(U32 u32_Sec);


U32 eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
	U32 u32_err;
	U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
    
	// init data pattern
	for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
	{
        pu32_W[u32_i] = u32_DataPattern;
		pu32_R[u32_i] = ~pu32_W[u32_i];
	}

	u32_err = eMMC_CMD24_CIFD(u32_eMMC_Addr, gau8_WBuf);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \n", u32_err);
		return u32_err;
	}
	
	u32_err = eMMC_CMD17_CIFD(u32_eMMC_Addr, gau8_RBuf);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \n", u32_err);
		return u32_err;
	}

	u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, FCIE_CIFD_BYTE_CNT);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \n", u32_err);
		return u32_err;
	}	

	return u32_err;
}

extern U32 gu32_DMAAddr;
U32 eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
	U32 u32_err;
	U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
    
	// init data pattern
	for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
	{
        pu32_W[u32_i] = u32_DataPattern;
		pu32_R[u32_i] = ~pu32_W[u32_i];
	}

	u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \n", u32_err);
		return u32_err;
	}
	
	u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, gau8_RBuf);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \n", u32_err);
		return u32_err;
	}

	u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, FCIE_CIFD_BYTE_CNT);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \n", u32_err);
		return u32_err;
	}	

	return u32_err;
}


U32 eMMCTest_MultiBlkWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern)
{
	U32 u32_err;
	U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
    U16 u16_BlkCnt_tmp;

    // init data pattern
	for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
	{
        pu32_W[u32_i] = u32_DataPattern;
		pu32_R[u32_i] = ~pu32_W[u32_i];
	}
	
	u16_BlkCnt_tmp = TEST_BUFFER_SIZE >> eMMC_SECTOR_512BYTE_BITS;
	u16_BlkCnt_tmp = u16_BlkCnt_tmp > u16_BlkCnt ? u16_BlkCnt : u16_BlkCnt_tmp;

	for(u32_i=0; u32_i<u16_BlkCnt_tmp; )
	{
		u32_err = eMMC_CMD25_MIU(
			u32_eMMC_Addr+u32_i, gau8_WBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), u16_BlkCnt_tmp-u32_i);
	    if(eMMC_ST_SUCCESS != u32_err)
	    {
		    eMMC_debug(0,1,"Err, MultiBlkWRC W, %u, %Xh \n", u32_i, u32_err);
		    goto LABEL_OF_ERROR;
	    }

		u32_err = eMMC_CMD18_MIU(
			u32_eMMC_Addr+u32_i, gau8_RBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), u16_BlkCnt_tmp-u32_i);
	    if(eMMC_ST_SUCCESS != u32_err)
	    {
		    eMMC_debug(0,1,"Err, MultiBlkWRC R, %u, %Xh \n", u32_i, u32_err);
		    goto LABEL_OF_ERROR;
	    }
		
	    u32_err = eMMC_ComapreData(
			gau8_WBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), gau8_RBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), (u16_BlkCnt_tmp-u32_i)<<eMMC_SECTOR_512BYTE_BITS);
	    if(eMMC_ST_SUCCESS != u32_err)
    	{
    		eMMC_debug(0,1,"Err, MultiBlkWRC C, %u, %Xh \n", u32_i, u32_err); 
    		goto LABEL_OF_ERROR;
	    }

		u32_i += u16_BlkCnt_tmp-u32_i;
	}
	
	return u32_err;

	LABEL_OF_ERROR:
	return u32_err;
}


// =========================================================
#define IF_eMMC_BOOT_MODE_STG2   0

eMMC_ALIGN0 U8 gau8_BootImageOri[] eMMC_ALIGN1 ={
    #include "cmd_mstar_emmc_bootloader.txt"
};

#if 0 == IF_eMMC_BOOT_MODE_STG2
eMMC_ALIGN0 static U8 gau8_BootImage0[0x18200] eMMC_ALIGN1;
#else
eMMC_ALIGN0 static U8 gau8_BootImage0[0x400] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_BootImage1[0x800] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_BootImage2[0x18200-0xC00] eMMC_ALIGN1;
#endif

U32 eMMC_BootMode(void)
{
	U32 u32_err;
	#if IF_eMMC_BOOT_MODE_STG2
	U8 *pu8_buf;
	U32 u32_SecIdx, u32_tmp, u32_i, u32_j;
	#endif
	U32 au32_Addr[3], au32_SectorCnt[3];
	
    eMMC_debug(0,1,"Boot Mode test \n");

	#if 0
	eMMC_debug(0,1,"ChkSum +: %Xh, ByteCnt: %Xh \n", 
		eMMC_ChkSum(gau8_BootImageOri, sizeof(gau8_BootImageOri)),
		sizeof(gau8_BootImageOri));
	#else
    eMMC_debug(0,1,"ChkSum +: %Xh, ByteCnt: %Xh \n", 
		eMMC_ChkSum(gau8_BootImageOri, 0xC1<<eMMC_SECTOR_512BYTE_BITS),
		0xC1<<eMMC_SECTOR_512BYTE_BITS); // for Gladius
	#endif

	au32_Addr[0] = (U32)gau8_BootImage0;
	au32_SectorCnt[0] = sizeof(gau8_BootImageOri)>>eMMC_SECTOR_512BYTE_BITS;
	#if IF_eMMC_BOOT_MODE_STG2
	au32_Addr[1] = (U32)gau8_BootImage1;
	au32_SectorCnt[1] = sizeof(gau8_BootImage1)>>eMMC_SECTOR_512BYTE_BITS;
	au32_Addr[2] = (U32)gau8_BootImage2;
	au32_SectorCnt[2] = sizeof(gau8_BootImage2)>>eMMC_SECTOR_512BYTE_BITS;
	#endif

	#if 0 == IF_eMMC_BOOT_MODE_STG2
    u32_err = eMMC_LoadImages(au32_Addr, au32_SectorCnt, 1);
	#else
    u32_err = eMMC_LoadImages(au32_Addr, au32_SectorCnt, 3);
	#endif
    if(eMMC_ST_SUCCESS != u32_err)
    {
		eMMC_debug(0,1,"Err, eMMC_LoadImages: %Xh\n", u32_err);
	    return u32_err;
	}

	#if 0 == IF_eMMC_BOOT_MODE_STG2
    //eMMC_debug(0,1,"ChkSum -: %Xh \n", eMMC_ChkSum((U8*)au32_Addr, sizeof(gau8_BootImageOri)));
    eMMC_debug(0,1,"ChkSum -: %Xh, Addr: %Xh \n", eMMC_ChkSum((U8*)au32_Addr[0], 0xC1<<eMMC_SECTOR_512BYTE_BITS), au32_Addr[0]);
	#else
	u32_SecIdx = 0;
	for(u32_j=0; u32_j<3; u32_j++)
	for(u32_i=0; u32_i<au32_SectorCnt[u32_j]; u32_i++)
	{
		eMMC_debug(0,1,"Sector: %Xh\n", u32_SecIdx);
		eMMC_debug(0,1,"\n");
		eMMC_dump_mem(au32_Addr[u32_j] + (u32_i<<eMMC_SECTOR_512BYTE_BITS), eMMC_SECTOR_512BYTE);
		u32_SecIdx++;

		pu8_buf = (U8*)(au32_Addr[u32_j] + (u32_i<<eMMC_SECTOR_512BYTE_BITS));
		for(u32_tmp=0; u32_tmp<eMMC_SECTOR_512BYTE; u32_tmp++)
			u32_ChkSum += pu8_buf[u32_tmp];
	}
	#endif

	eMMC_debug(0,1,"Boot Mode test end\n");	
	return eMMC_ST_SUCCESS;
	
}

//========================================================
// exposed function
//========================================================
#define eMMC_SPEED_TEST_COUNTDOWN          3

#define eMMC_SPEED_TEST_SINGLE_BLKCNT_W    500
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W  30*(1024*2) // 30MB

#define eMMC_SPEED_TEST_R_MULTI            10
#define eMMC_SPEED_TEST_SINGLE_BLKCNT_R    eMMC_SPEED_TEST_SINGLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R  eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI

void eMMCTest_DownCount(U32 u32_Sec) // verify the HW Timer
{
	U32 u32_i, u32_t0, u32_t1;
	for(u32_i=0; u32_i<u32_Sec; u32_i++)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"%u \n", u32_Sec-u32_i);
		#if 0
		eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
		#else
		eMMC_hw_timer_start();
        u32_t0 = eMMC_hw_timer_tick();
		u32_t1 = eMMC_hw_timer_tick();
		if(u32_t0 > u32_t1)
			while(0==(u32_t0-eMMC_hw_timer_tick())/eMMC_HW_TIMER_MHZ)
				;
		else
			while(0==(eMMC_hw_timer_tick()-u32_t0)/eMMC_HW_TIMER_MHZ)
				;
		#endif		
	}
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,".\n");
}

U32 eMMC_IPVerify_Performance(void)
{
	volatile U32 u32_err, u32_i, u32_SectorAddr=0, u32_SectorCnt=0, u32_t0, u32_t1, u32_t;
	U8  u8_IfDDRTest=0;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"SectorBuf:%Xh, RBuf:%Xh, WBuf:%Xh\n", 
		(U32)gau8_eMMC_SectorBuf, (U32)gau8_RBuf, (U32)gau8_WBuf);

	// ===============================================
	u32_err = eMMC_Init();
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \n", u32_err);
		return u32_err;
	}
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_Init ok] \n");

	#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
	u32_err = eMMC_FCIE_EnableDDRMode();
	if(eMMC_ST_SUCCESS != u32_err){
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode fail: %Xh\n", u32_err);
		return u32_err;
	}

	if(0 == g_eMMCDrv.DDRTable.u8_SetCnt){
		eMMC_FCIE_BuildDDRTimingTable();
	    if(eMMC_ST_SUCCESS != u32_err){
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: BuildDDRTimingTable fail: %Xh\n", u32_err);
		    return u32_err;
	    }
	}else 
		eMMC_DumpDDRTTable();
    #endif

	eMMCTest_DownCount(eMMC_SPEED_TEST_COUNTDOWN);
	for(u32_i=0; u32_i<TEST_BUFFER_SIZE; u32_i++)
		gau8_WBuf[u32_i] = (U8)u32_i;
	
	// ===============================================
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"======================================\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"SDR\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"======================================\n");
	u32_err = eMMC_FCIE_EnableSDRMode();
	if(eMMC_ST_SUCCESS != u32_err){
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
		eMMC_DumpDriverStatus();
	    return u32_err;
	}
	eMMC_clock_setting(gau8_FCIEClkSel[0]);

	// -----------------------------------------
	#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
	LABEL_TEST_START:
	#endif
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Single Write, total: %uKB, %u blocks\n", 
		(eMMC_SPEED_TEST_SINGLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>10,
		eMMC_SPEED_TEST_SINGLE_BLKCNT_W);

	eMMC_hw_timer_start();
	u32_t0 = eMMC_hw_timer_tick();
	for(u32_i=0; u32_i<eMMC_SPEED_TEST_SINGLE_BLKCNT_W; u32_i++)
	{
		u32_SectorAddr += (g_eMMCDrv.u32_SEC_COUNT>>2) + 333*u32_i;
		u32_SectorAddr %= g_eMMCDrv.u32_SEC_COUNT;
		if(u32_SectorAddr < g_eMMCDrv.u32_SEC_COUNT>>1)
			u32_SectorAddr += g_eMMCDrv.u32_SEC_COUNT>>1;
		u32_SectorAddr -= eMMC_SPEED_TEST_SINGLE_BLKCNT_W;
		u32_SectorAddr = eMMC_TEST_BLK_0+(u32_SectorAddr%eMMC_TEST_BLK_CNT);

		//eMMC_debug(0,0,"%08Xh \n", u32_SectorAddr+u32_i);
		#if eMMC_FEATURE_RELIABLE_WRITE
		u32_err = eMMC_CMD24(u32_SectorAddr, gau8_WBuf);
		#else
		u32_err = eMMC_CMD24_MIU(u32_SectorAddr, gau8_WBuf);
		#endif
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
			return u32_err;
		}
	}
	u32_t1 = eMMC_hw_timer_tick();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : u32_t0-u32_t1;
	if(u32_t > (U32)u32_t*10)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: timer overflow \n");	
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0," %u.%u KB/sec \n", 
		((eMMC_SPEED_TEST_SINGLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>10)*100/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)),
		(((eMMC_SPEED_TEST_SINGLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>10)*1000/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)))%10);

	// -----------------------------------------
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Single Read, total: %uKB, %u blocks\n", 
		((eMMC_SPEED_TEST_SINGLE_BLKCNT_R)<<eMMC_SECTOR_512BYTE_BITS)>>10,
		eMMC_SPEED_TEST_SINGLE_BLKCNT_R);
	eMMC_hw_timer_start();
	u32_t0 = eMMC_hw_timer_tick();
	for(u32_i=0; u32_i<eMMC_SPEED_TEST_SINGLE_BLKCNT_R; u32_i++)
	{
		u32_SectorAddr += (g_eMMCDrv.u32_SEC_COUNT>>2) + 333*u32_i;
		u32_SectorAddr %= g_eMMCDrv.u32_SEC_COUNT;
		if(u32_SectorAddr < g_eMMCDrv.u32_SEC_COUNT>>1)
			u32_SectorAddr += g_eMMCDrv.u32_SEC_COUNT>>1;
		u32_SectorAddr -= eMMC_SPEED_TEST_SINGLE_BLKCNT_R;
		u32_SectorAddr = eMMC_TEST_BLK_0+(u32_SectorAddr%eMMC_TEST_BLK_CNT);
		
		u32_err = eMMC_CMD17_MIU(u32_SectorAddr, gau8_RBuf);
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
			return u32_err;
		}
	}
	u32_t1 = eMMC_hw_timer_tick();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : u32_t0-u32_t1;
	if(u32_t > (U32)u32_t*10)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: timer overflow \n");	
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0," %u.%u KB/sec \n", 
		((eMMC_SPEED_TEST_SINGLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>10)*100/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)),
		(((eMMC_SPEED_TEST_SINGLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>10)*1000/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)))%10);
	
	// -----------------------------------------
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Multi Write, total: %uMB, %u blocks\n", 
		(eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>20,
		eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W);
    u32_SectorAddr = eMMC_TEST_BLK_0;
	u32_SectorCnt = 
		(TEST_BUFFER_SIZE>>eMMC_SECTOR_512BYTE_BITS) > eMMC_TEST_BLK_CNT ? 
		eMMC_TEST_BLK_CNT : (TEST_BUFFER_SIZE>>eMMC_SECTOR_512BYTE_BITS);
	eMMC_hw_timer_start();
	u32_t0 = eMMC_hw_timer_tick();
	for(u32_i=0; u32_i<eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W; )
	{
		u32_err = eMMC_CMD25_MIU(u32_SectorAddr, gau8_WBuf, u32_SectorCnt);
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
			return u32_err;
		}
		u32_i += u32_SectorCnt;
	}
	u32_t1 = eMMC_hw_timer_tick();	
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : u32_t0-u32_t1;
	if(u32_t > (U32)u32_t*10)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: timer overflow \n");	
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0," %u.%u MB/sec \n", 
		((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>20)*100/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)),
		(((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>20)*1000/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)))%10);
	
	// -----------------------------------------
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Multi Read, total: %uMB, %u blocks\n", 
		(eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>20,
		eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R);
	u32_SectorAddr = eMMC_TEST_BLK_0;
	u32_SectorCnt = 
		(TEST_BUFFER_SIZE>>eMMC_SECTOR_512BYTE_BITS) > eMMC_TEST_BLK_CNT ? 
		eMMC_TEST_BLK_CNT : (TEST_BUFFER_SIZE>>eMMC_SECTOR_512BYTE_BITS);
	eMMC_hw_timer_start();
	u32_t0 = eMMC_hw_timer_tick();
	for(u32_i=0; u32_i<eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R;)
	{	
		u32_err = eMMC_CMD18_MIU(u32_SectorAddr, gau8_RBuf, u32_SectorCnt);
		if(eMMC_ST_SUCCESS != u32_err)
		{
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
			return u32_err;
		}
		u32_i += u32_SectorCnt;
	}
	u32_t1 = eMMC_hw_timer_tick();	
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : u32_t0-u32_t1;
	if(u32_t > (U32)u32_t*10)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: timer overflow \n");	
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0," %u.%u MB/sec \n", 
		((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>20)*100/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)),
		(((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>20)*1000/
		(u32_t*10/(eMMC_HW_TIMER_MHZ/10)))%10);

	if(u8_IfDDRTest)
		return eMMC_ST_SUCCESS;
	
	// ===============================================
	#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"======================================\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"DDR\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"======================================\n");
	u32_err = eMMC_FCIE_EnableDDRMode();
	if(eMMC_ST_SUCCESS != u32_err){
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode fail: %Xh\n", u32_err);
		eMMC_DumpDriverStatus();
	    return u32_err;
	}	
	
	u8_IfDDRTest = 1;
	goto LABEL_TEST_START;
	#endif

	return u32_err;

}


U32 eMMC_IPVerify_Main(void)
{
	U32 u32_err;

	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC_IPVerify_Main\n");
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"%Xh %Xh %Xh\n", 
		(U32)gau8_eMMC_SectorBuf, (U32)gau8_RBuf, (U32)gau8_WBuf);

	// ===============================================
	#if 0
    eMMC_BootMode();
	//while(1);
	return 0;
	#endif

	// ===============================================
	u32_err = eMMC_Init();
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \n", u32_err);
		return u32_err;
	}
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_Init ok] \n");		

	// ===============================================
	#if 0
	eMMC_debug(0,1,"BootImage ByteCnt: %Xh, ChkSum:%Xh\n", 
	    sizeof(gau8_BootImageOri), eMMC_ChkSum(gau8_BootImageOri, sizeof(gau8_BootImageOri)));

	u32_err = eMMC_WriteBootPart(gau8_BootImageOri, sizeof(gau8_BootImageOri), 0, 1);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_WriteBootPart fail: %Xh \n", u32_err);
		//goto LABEL_IP_VERIFY_ERROR;
	}
	else
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "[eMMC_WriteBootPart ok] \n");

	while(1);
	#endif

	// ===============================================
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "normal pattern test ok\n\n");
	
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
	eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);
	eMMC_debug(eMMC_DEBUG_LEVEL, 0, "specific pattern test ok\n\n");

	return eMMC_ST_SUCCESS;
}


U32 eMMC_IPVerify_Main_Ex(U32 u32_DataPattern)
{
	static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;	
	
	// make StartSector SectorCnt random
 	u32_StartSector = eMMC_TEST_BLK_0;	
	u32_SectorCnt++;
	while(u32_SectorCnt > eMMC_TEST_BLK_CNT)
		u32_SectorCnt = 1;

	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_DataPattern);
	//eMMC_debug(0,1,"Total Sec: %Xh, StartSec: %Xh, SecCnt: %Xh \n",
	//		g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);	

	#if 1
	if(0==(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)){
    u32_err = eMMCTest_SingleBlkWRC_RIU(u32_StartSector, u32_DataPattern);
	if(eMMC_ST_SUCCESS != u32_err)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_RIU fail: %Xh \n", u32_err);
		//goto LABEL_IP_VERIFY_ERROR;
    }
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_RIU ok] \n");}
	#endif
	#if 1
	u32_err = eMMCTest_SingleBlkWRC_MIU(u32_StartSector, u32_DataPattern);
	if(eMMC_ST_SUCCESS != u32_err)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_MIU fail: %Xh \n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_MIU ok] \n");
	#endif

	#if 1
	u32_err = eMMCTest_MultiBlkWRC_MIU(u32_StartSector, u32_SectorCnt, u32_DataPattern);
	if(eMMC_ST_SUCCESS != u32_err)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_MultiBlkWRC_MIU fail: %Xh \n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
    }
	eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_MultiBlkWRC_MIU ok] \n");
	#endif	

	// ===============================================
	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
	return eMMC_ST_SUCCESS;
    	
	LABEL_IP_VERIFY_ERROR:
	eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
	eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \n",
			g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);		
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \n\n", u32_err);
	eMMC_DumpDriverStatus();
	while(1);
	return u32_err;
}



U32 eMMC_IPVerify_SDRDDR_AllClkTemp(void)
{
	U32 u32_err;
	U8  u8_SDRClkIdx, u8_DDRClkIdx;

	#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
	u32_err = eMMC_FCIE_EnableDDRMode();
	if(eMMC_ST_SUCCESS != u32_err){
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode fail: %Xh\n", u32_err);
		return u32_err;
	}

	if(0 == g_eMMCDrv.DDRTable.u8_SetCnt){
		eMMC_FCIE_BuildDDRTimingTable();
	    if(eMMC_ST_SUCCESS != u32_err){
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: BuildDDRTimingTable fail: %Xh\n", u32_err);
		    return u32_err;
	    }
	}else 
		eMMC_DumpDDRTTable();
	#endif
	
	u8_SDRClkIdx = 0;
	u8_DDRClkIdx = 0;
	
	while(1)
	{
		#if 1
		//eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"======================================\n");
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"SDR\n");
		u32_err = eMMC_FCIE_EnableSDRMode();
	    if(eMMC_ST_SUCCESS != u32_err){
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
			eMMC_DumpDriverStatus();
		    return u32_err;
		}
		//if(0==u8_SDRClkIdx)  u8_SDRClkIdx++; // skip 48MHz
		eMMC_clock_setting(gau8_FCIEClkSel[u8_SDRClkIdx]);
		u8_SDRClkIdx++;
		//u8_SDRClkIdx = u8_SDRClkIdx%(eMMC_FCIE_VALID_CLK_CNT-1);// skip 300KHz
		u8_SDRClkIdx = u8_SDRClkIdx%eMMC_FCIE_VALID_CLK_CNT;

		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
	    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
	    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
	    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
        #endif

		#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
		//eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"======================================\n");
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"DDR\n");
		
		u32_err = eMMC_FCIE_EnableDDRMode();
	    if(eMMC_ST_SUCCESS != u32_err){
		    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode fail: %Xh\n", u32_err);
			eMMC_DumpDriverStatus();
		    return u32_err;
		}
		#if 1
		eMMC_FCIE_ApplyDDRTSet(u8_DDRClkIdx);
        u8_DDRClkIdx++;
	    u8_DDRClkIdx = u8_DDRClkIdx%g_eMMCDrv.DDRTable.u8_SetCnt;
		#endif
		#if 0
		do{ // skip 42.6MHz
			u8_DDRClkIdx++;
		    u8_DDRClkIdx = u8_DDRClkIdx%g_eMMCDrv.DDRTable.u8_SetCnt;
			eMMC_FCIE_ApplyDDRTSet(u8_DDRClkIdx);
		}while(BIT_FCIE_CLK_42_6M == g_eMMCDrv.DDRTable.Set[u8_DDRClkIdx].u8_Clk);
		#endif
		#if 0
		eMMC_FCIE_ApplyDDRTSet(1);
		#endif

		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
		eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
		#endif
	}

	return eMMC_ST_SUCCESS;
}

#endif
#endif
