/*
* hal_sdmmc_v5.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName hal_sdmmc_v5.c  (Driver Version)
 *     @author jeremy.wang (2015/07/06)
 * Desc:
 *     HAL SD Driver will support basic SD function but not flow process.
 *     (1) It included Register and Buffer opertion code.
 *     (2) h file will has APIs for Driver, but just SD abstract, not register abstract.
 *
 ***************************************************************************************************************/

#include "../inc/hal_sdmmc_v5.h"
#include "../inc/hal_card_timer.h"

#if(EN_BIND_CARD_INT)
#include "../inc/hal_card_intr_v5.h"  //inlcue but may not use
#endif


//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************

// Enable Setting
//-----------------------------------------------------------------------------------------------------------
#define EN_TRFUNC                   (FALSE)
#define EN_DUMPREG                  (TRUE)
#define EN_BYPASSMODE               (FALSE)    //BYPASS MODE or ADVANCE MODE(SDR/DDR)
#define EN_HALT                     (FALSE)

// Retry Times
//-----------------------------------------------------------------------------------------------------------
#define RT_CLEAN_SDSTS              3
#define RT_CLEAN_MIEEVENT           3

// Wait Time
//-----------------------------------------------------------------------------------------------------------
#define WT_DAT0HI_END               1000    //(ms)
#define WT_EVENT_CIFD                500    //(ms)
#define WT_RESET                     100    //(ms)

//***********************************************************************************************************
//***********************************************************************************************************

// Reg Static Init Setting
//-----------------------------------------------------------------------------------------------------------
#define V_MIE_PATH_INIT     0
#define V_MMA_PRI_INIT      (R_MIU_R_PRIORITY|R_MIU_W_PRIORITY|R_MIU_BUS_BURST8)
#define V_MIE_INT_EN_INIT   (R_DATA_END_IEN|R_CMD_END_IEN|R_SDIO_INT_IEN|R_BUSY_END_IEN)
#define V_RSP_SIZE_INIT     0
#define V_CMD_SIZE_INIT     (5<<8)
#define V_SD_CTL_INIT       0
#define V_SD_MODE_INIT      (R_CLK_EN)  // Add
#define V_SDIO_MODE_INIT    (1 << 3) //Low level trigger from Joe's email. Was 0 before.
#define V_DDR_MODE_INIT     0


// Mask Range
//-----------------------------------------------------------------------------------------------------------
#define M_SD_ERRSTS         (R_DAT_RD_CERR|R_DAT_WR_CERR|R_DAT_WR_TOUT|R_CMD_NORSP|R_CMDRSP_CERR|R_DAT_RD_TOUT)
#define M_SD_MIEEVENT       (R_DATA_END|R_CMD_END|R_ERR_STS|R_BUSY_END_INT|R_R2N_RDY_INT)
#define M_RST_STS           (R_RST_MIU_STS|R_RST_MIE_STS|R_RST_MCU_STS)


// Mask Reg Value
//-----------------------------------------------------------------------------------------------------------
#define M_REG_STSERR(IP)               (CARD_REG(A_SD_STS_REG(IP)) & M_SD_ERRSTS)
#define M_REG_SDMIEEvent(IP)           (CARD_REG(A_MIE_EVENT_REG(IP)) & M_SD_MIEEVENT)
#define M_REG_GETDAT0(IP)              (CARD_REG(A_SD_STS_REG(IP)) & R_DAT0)


//-----------------------------------------------------------------------------------------------------------
//IP_FCIE or IP_SDIO Register Basic Address
//-----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)                GET_CARD_BANK(IP, 0)
#define A_SD_CFIFO_POS(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x20)  //Always at FCIE5
#define A_SD_CIFD_POS(IP)               GET_CARD_BANK(IP, 1)

#define A_SD_CIFD_R_POS(IP)             GET_CARD_REG_ADDR(A_SD_CIFD_POS(IP), 0x00)
#define A_SD_CIFD_W_POS(IP)             GET_CARD_REG_ADDR(A_SD_CIFD_POS(IP), 0x20)

#define A_MIE_EVENT_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x00)
#define A_MIE_INT_ENABLE_REG(IP)        GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x01)
#define A_MMA_PRI_REG_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x02)
#define A_DMA_ADDR_15_0_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x03)
#define A_DMA_ADDR_31_16_REG(IP)        GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x04)
#define A_DMA_LEN_15_0_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x05)
#define A_DMA_LEN_31_16_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x06)
#define A_MIE_FUNC_CTL_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x07)
#define A_JOB_BLK_CNT_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x08)
#define A_BLK_SIZE_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x09)
#define A_CMD_RSP_SIZE_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0A)
#define A_SD_MODE_REG(IP)               GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0B)
#define A_SD_CTL_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0C)
#define A_SD_STS_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0D)
#define A_BOOT_MOD_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0E)
#define A_DDR_MOD_REG(IP)               GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0F)
#define A_DDR_TOGGLE_CNT_REG(IP)        GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x10)
#define A_SDIO_MODE_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x11)
#define A_TEST_MODE_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x15)

#define A_WR_SBIT_TIMER_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x17)
#define A_RD_SBIT_TIMER_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x18)

#define A_SDIO_DET_ON(IP)               GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2F)

#define A_CIFD_EVENT_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x30)
#define A_CIFD_INT_EN_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x31)

#define A_BOOT_REG(IP)                  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x37)

#define A_DBG_BUS0_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x38)
#define A_DBG_BUS1_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x39)
#define A_FCIE_RST_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x3F)

#define A_CFIFO_OFFSET(IP, OFFSET)      GET_CARD_REG_ADDR(A_SD_CFIFO_POS(IP), OFFSET)
#define A_CIFD_R_OFFSET(IP, OFFSET)     GET_CARD_REG_ADDR(A_SD_CIFD_R_POS(IP), OFFSET)
#define A_CIFD_W_OFFSET(IP, OFFSET)     GET_CARD_REG_ADDR(A_SD_CIFD_W_POS(IP), OFFSET)


// Reg Dynamic Variable
//-----------------------------------------------------------------------------------------------------------
static RspStruct gst_RspStruct[3];
static volatile BOOL_T  gb_StopWProc[3] = {0};
static volatile U16_T   gu16_WT_NRC[3] = {0};             //Waiting Time for Nrc (us)

static volatile U16_T   gu16_SD_MODE_DatLine[3] = {0};

static volatile U16_T   gu16_DDR_MODE_REG[3] = {0};
static volatile U16_T   gu16_DDR_MODE_REG_ForR2N[3] = {0};

static volatile BOOL_T  gb_SDIODevice[3] = {0};
static volatile U16_T   gu16_SDIO_MODE_IntMode[3] = {0};


static volatile U8_T*   gpu8Buf[3];


static volatile IpType geIpTypeIp[3] = {IP_0_TYPE, IP_1_TYPE, IP_2_TYPE};

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_TRFUNC)
    #define TR_H_SDMMC(p)       p
#else
    #define TR_H_SDMMC(p)
#endif


// Register Opertation Define ==> For FCIE5 Special FCIE/SDIO Function Ctrl Setting
//-----------------------------------------------------------------------------------------------------------
static U16_T _REG_GetMIEFunCtlSetting(IpOrder eIP)
{
    if (geIpTypeIp[eIP] == IP_TYPE_FCIE)
    {
        return R_SD_EN; // FCIE don't have SDIO function.
    }

    return R_SDIO_MODE;
}


// Register Operation Define ==> For Clean Reg and Special Case
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_ClearSDSTS(IpOrder eIP, U8_T u8Retry)
{
    do
    {
        CARD_REG_SETBIT(A_SD_STS_REG(eIP), M_SD_ERRSTS);

        if ( gb_StopWProc[eIP] )
            break;

        if ( !M_REG_STSERR(eIP) )
            return EV_OK;
        else if(!u8Retry)
            break;

    } while(u8Retry--);

    return EV_FAIL; //mark for coverity scan
}


// Register Operation Define ==> For Clear MIE Event
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_ClearMIEEvent(IpOrder eIP, U8_T u8Retry)
{
    /****** Clean global MIEEvent for Interrupt ******/
#if(EN_BIND_CARD_INT)
    Hal_CARD_INT_ClearMIEEvent(eIP);
#endif

    /****** Clean MIEEvent Reg *******/
    do
    {
        CARD_REG(A_MIE_EVENT_REG(eIP)) = M_SD_MIEEVENT;

        if ( gb_StopWProc[eIP] )
            break;

        if ( !M_REG_SDMIEEvent(eIP) )
            return EV_OK;
        else if( !u8Retry )
            break;

    }while( u8Retry-- );
    return EV_FAIL; //mark for coverity scan

}


// Register Operation Define ==> For Wait DAT0 High
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_WaitDat0HI(IpOrder eIP, U32_T u32WaitMs)
{
    U32_T u32DiffTime = 0;

#if(EN_BIND_CARD_INT)
	if (Hal_CARD_INT_MIEIntRunning(eIP))	// Interrupt Mode
	{
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_BUSY_DET_ON );

        if ( !Hal_CARD_INT_WaitMIEEvent(eIP, R_BUSY_END_INT, u32WaitMs) )
            return EV_FAIL;
        else
            return EV_OK;

	}
    else // Polling Mode
#endif
    {
        do
        {
            if ( gb_StopWProc[eIP] )
                return EV_FAIL;

            if ( M_REG_GETDAT0(eIP) )
                return EV_OK;

            Hal_Timer_uDelay(1);
            u32DiffTime++;
        }while( u32DiffTime <= (u32WaitMs*1000) );
    }
    return EV_FAIL;
}


// Register Operation Define ==> For Wait MIE Event or CIFD Event
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_WaitEvent(IpOrder eIP, IPEventEmType eEvent, U16_T u16ReqEvent, U32_T u32WaitMs)
{
    U32_T u32DiffTime = 0;

#if(EN_BIND_CARD_INT)
    if (Hal_CARD_INT_MIEIntRunning(eIP))	// Interrupt Mode
    {
        if(eEvent == EV_MIE)
        {
            if ( !Hal_CARD_INT_WaitMIEEvent(eIP, u16ReqEvent, u32WaitMs) )
                return EV_FAIL;
            else
                return EV_OK;
        }

    }
    else // Polling Mode
#endif
    {
        do
        {
            if ( gb_StopWProc[eIP] )
                return EV_FAIL;

            if(eEvent == EV_MIE)
            {
                if ( (CARD_REG(A_MIE_EVENT_REG(eIP))&u16ReqEvent) == u16ReqEvent )
                {
                    return EV_OK;
                }
            }
            else if(eEvent == EV_CIFD )
            {
                if ( (CARD_REG(A_CIFD_EVENT_REG(eIP))&u16ReqEvent) == u16ReqEvent )
                {
                    return EV_OK;
                }
            }

            Hal_Timer_mDelay(1);
            u32DiffTime++;
        }while(u32DiffTime <= u32WaitMs);

    }

    return EV_FAIL;

}


// Register Operation Define ==> For Software Reset
//-----------------------------------------------------------------------------------------------------------
static void _REG_ResetIP(IpOrder eIP)
{
    U32_T u32DiffTime = 0;

    CARD_REG_CLRBIT(A_SD_CTL_REG(eIP), R_JOB_START);  //Clear For Safe ?

    CARD_REG_CLRBIT(A_FCIE_RST_REG(eIP), R_FCIE_SOFT_RST);

    do
    {
        if( (CARD_REG(A_FCIE_RST_REG(eIP)) & M_RST_STS) == M_RST_STS )
            break;
        Hal_Timer_uDelay(1);
        u32DiffTime++;
    }while ( u32DiffTime <= (1000*WT_RESET) );

    if ( u32DiffTime > (1000*WT_RESET) )
        sdmmc_print("[HSD] IP Reset Switch Low Fail !\r\n");


    u32DiffTime = 0;
    CARD_REG_SETBIT(A_FCIE_RST_REG(eIP), R_FCIE_SOFT_RST);

    do
    {
        if( (CARD_REG(A_FCIE_RST_REG(eIP)) & M_RST_STS) == 0 )
            break;
        Hal_Timer_uDelay(1);
        u32DiffTime++;

    }while ( u32DiffTime <= (1000* WT_RESET) );

    if ( u32DiffTime > (1000*WT_RESET) )
        sdmmc_print("[HSD] IP Reset Switch High Fail !\r\n");
}


// IP Buffer Operation => Get Byte Value form Register
//-----------------------------------------------------------------------------------------------------------
static U8_T _BUF_GetByteFromRegAddr(volatile void *pBuf, U16_T u16Pos)
{
    if(u16Pos & 0x1)
        return CARD_REG_H8(GET_CARD_REG_ADDR(pBuf, u16Pos>>1));
    else
        return CARD_REG_L8(GET_CARD_REG_ADDR(pBuf, u16Pos>>1));

}


// IP Buffer Operation => CIFD FIFO Buffer Operation Define
//-----------------------------------------------------------------------------------------------------------
void _BUF_CIFD_DATA_IO(IpOrder eIP, CmdEmType eCmdType, volatile U16_T *pu16Buf, U8_T u8WordCnt)
{
    U8_T u8Pos = 0;

    for ( u8Pos = 0; u8Pos < u8WordCnt; u8Pos++ )
    {
        if ( eCmdType==EV_CMDREAD )
            pu16Buf[u8Pos] = CARD_REG(A_CIFD_R_OFFSET(eIP, u8Pos));
        else
            CARD_REG(A_CIFD_W_OFFSET(eIP, u8Pos)) = pu16Buf[u8Pos];
    }

}


// IP Buffer Operation => CIFD FIFO Buffer Operation for waiting FCIE5 special Event
//-----------------------------------------------------------------------------------------------------------
static RetEmType _BUF_CIFD_WaitEvent(IpOrder eIP,  CmdEmType eCmdType, volatile U8_T *pu8R2NBuf)
{
    U8_T u8RegionNo = 0, u8RegionMax = 0, u8RemainByte = 0;
    U32_T u32_TranLen  =  CARD_REG(A_DMA_LEN_15_0_REG(eIP)) + ( CARD_REG(A_DMA_LEN_31_16_REG(eIP)) << 16 );

    u8RemainByte = u32_TranLen & (64-1);  // u32_TranLen % 64
    u8RegionMax = (u32_TranLen>>6) + (u8RemainByte ? 1: 0);

    for(u8RegionNo=0; u8RegionNo<u8RegionMax; u8RegionNo++)
    {

        if ( eCmdType==EV_CMDREAD )
        {
            if ( _REG_WaitEvent(eIP, EV_CIFD, R_WBUF_FULL, WT_EVENT_CIFD) )
               return EV_FAIL;

           if ( (u8RegionNo == (u8RegionMax-1)) && (u8RemainByte>0) )
                _BUF_CIFD_DATA_IO(eIP, eCmdType, (volatile U16_T *)(pu8R2NBuf + (u8RegionNo << 6)), u8RemainByte/2);
           else
               _BUF_CIFD_DATA_IO(eIP, eCmdType, (volatile U16_T *)(pu8R2NBuf + (u8RegionNo << 6)), 32);

           CARD_REG(A_CIFD_EVENT_REG(eIP)) = R_WBUF_FULL;
           CARD_REG(A_CIFD_EVENT_REG(eIP)) = R_WBUF_EMPTY_TRIG;
        }
        else // Write
        {
            if ( (u8RegionNo == (u8RegionMax-1)) && (u8RemainByte>0) )
                 _BUF_CIFD_DATA_IO(eIP, eCmdType, (volatile U16_T *)(pu8R2NBuf + (u8RegionNo << 6)), u8RemainByte/2);
            else
                _BUF_CIFD_DATA_IO(eIP, eCmdType, (volatile U16_T *)(pu8R2NBuf + (u8RegionNo << 6)), 32);

             CARD_REG(A_CIFD_EVENT_REG(eIP)) = R_RBUF_FULL_TRIG;

            if ( _REG_WaitEvent(eIP, EV_CIFD, R_RBUF_EMPTY, WT_EVENT_CIFD) )
               return EV_FAIL;

             CARD_REG(A_CIFD_EVENT_REG(eIP)) = R_RBUF_EMPTY;

        }

    }

    return EV_OK;

}


// SDMMC Internel Logic Function
//-----------------------------------------------------------------------------------------------------------
static void _SDMMC_REG_Dump(IpOrder eIP)
{

#if (EN_DUMPREG)

    U8_T u8Pos, u8DGMode;
    sdmmc_print("\n------\r\n");
    sdmmc_print("0x%08X", ((U32_T)A_SD_REG_POS(eIP) & 0x00FFFF00) >> 9);
    sdmmc_print("CMD_%u", gst_RspStruct[eIP].u8Cmd);
    sdmmc_print("(Arg: 0x%08X)", gst_RspStruct[eIP].u32Arg);
    sdmmc_print("[Line: %u]\r\n", gst_RspStruct[eIP].u32ErrLine);
    sdmmc_print("\n------\r\n");

    for(u8Pos = 0; u8Pos < gst_RspStruct[eIP].u8RspSize; u8Pos++)
    {
        if( (u8Pos == 0) || (u8Pos == 8) )
            sdmmc_print("[");

        sdmmc_print("0x%02X,", _BUF_GetByteFromRegAddr((volatile void *)A_SD_CFIFO_POS(eIP), u8Pos));

        if( (u8Pos == 7) || (u8Pos == (gst_RspStruct[eIP].u8RspSize-1)) )
            sdmmc_print("]\n");
    }

    sdmmc_print("---DumpReg---\r\n");

    sdmmc_print("[0x07][MIE_FUNC_CTL_REG]=  0x%04X\r\n", CARD_REG(A_MIE_FUNC_CTL_REG(eIP)));
    sdmmc_print("[0x0B][SD_MODE_REG]=       0x%04X\r\n", CARD_REG(A_SD_MODE_REG(eIP)));
    sdmmc_print("[0x0C][SD_CTL_REG]=        0x%04X\r\n", CARD_REG(A_SD_CTL_REG(eIP)));
    sdmmc_print("[0x0F][DDR_MOD_REG]=       0x%04X\r\n", CARD_REG(A_DDR_MOD_REG(eIP)));
    sdmmc_print("[0x0D][SD_STS_REG]=        0x%04X\r\n", CARD_REG(A_SD_STS_REG(eIP)));

#if(EN_BIND_CARD_INT)
    if (Hal_CARD_INT_MIEIntRunning(eIP))   // Interrupt Mode
    {
        sdmmc_print("[gu16_MIEEvent_ForInt]= 0x%04X\r\n", Hal_CARD_INT_GetMIEEvent(eIP));
    }
#endif

    sdmmc_print("[0x00][MIE_EVENT_REG]=       0x%04X\r\n", CARD_REG(A_MIE_EVENT_REG(eIP)));
    sdmmc_print("[0x03][MMA_ADDR_15_0_REG]=   0x%04X\r\n", CARD_REG(A_DMA_ADDR_15_0_REG(eIP)));
    sdmmc_print("[0x04][MMA_ADDR_31_16_REG]=  0x%04X\r\n", CARD_REG(A_DMA_ADDR_31_16_REG(eIP)));
    sdmmc_print("[0x05][MMA_LEN_15_0_REG]=    0x%04X\r\n", CARD_REG(A_DMA_LEN_15_0_REG(eIP)));
    sdmmc_print("[0x06][MMA_LEN_31_16_REG]=   0x%04X\r\n", CARD_REG(A_DMA_LEN_31_16_REG(eIP)));
    sdmmc_print("[0x08][JOB_BLK_CNT]=         0x%04X\r\n", CARD_REG(A_JOB_BLK_CNT_REG(eIP)));
    sdmmc_print("[0x09][BLK_SIZE]=            0x%04X\r\n", CARD_REG(A_BLK_SIZE_REG(eIP)));

    CARD_REG_CLRBIT(A_DBG_BUS1_REG(eIP), R_DEBUG_MOD0 | R_DEBUG_MOD1 | R_DEBUG_MOD2 | R_DEBUG_MOD3 );
    CARD_REG_SETBIT(A_DBG_BUS1_REG(eIP), R_DEBUG_MOD0 | R_DEBUG_MOD2); //Mode 5

    sdmmc_print("[0x38][DEBUG_BUS0]= ");
    for(u8DGMode = 1; u8DGMode <=4; u8DGMode++)
    {
        CARD_REG_CLRBIT(A_TEST_MODE_REG(eIP), R_SD_DEBUG_MOD0 | R_SD_DEBUG_MOD1 | R_SD_DEBUG_MOD2);
        CARD_REG_SETBIT(A_TEST_MODE_REG(eIP), (u8DGMode<<1));
        sdmmc_print("0x%04X, ", CARD_REG(A_DBG_BUS0_REG(eIP)));
    }
    sdmmc_print("\r\n");
    sdmmc_print("[0x39][DEBUG_BUS1]= ");

    for(u8DGMode = 1; u8DGMode <=4; u8DGMode++)
    {
        CARD_REG_CLRBIT(A_TEST_MODE_REG(eIP), R_SD_DEBUG_MOD0 | R_SD_DEBUG_MOD1 | R_SD_DEBUG_MOD2);
        CARD_REG_SETBIT(A_TEST_MODE_REG(eIP), (u8DGMode<<1));
        sdmmc_print("0x%04X, ", CARD_REG(A_DBG_BUS1_REG(eIP)));
    }

    sdmmc_print("\r\n");
    sdmmc_print("------\r\n");

#endif  //End #if(EN_DUMPREG)


}


static RspErrEmType _SDMMC_EndProcess(IpOrder eIP, CmdEmType eCmdType, RspErrEmType eRspErr, BOOL_T bCloseClk, int Line)
{
    U16_T u16RspErr = (U16_T)eRspErr;
    U16_T u16IPErr = EV_STS_RIU_ERR | EV_STS_MIE_TOUT | EV_STS_DAT0_BUSY;

    /****** (1) Record Information *******/
    gst_RspStruct[eIP].u32ErrLine = (U32_T)Line;
    gst_RspStruct[eIP].u8RspSize = (U8_T)CARD_REG(A_CMD_RSP_SIZE_REG(eIP));
    gst_RspStruct[eIP].eErrCode = eRspErr;

    /****** (2) Dump and the Reg Info + Reset IP *******/
    if ( u16RspErr && gb_StopWProc[eIP] )
    {
        eRspErr = EV_SWPROC_ERR;
        _REG_ResetIP(eIP);
    }
    else if( u16RspErr & u16IPErr ) //SD IP Error
    {
        _SDMMC_REG_Dump(eIP);
        _REG_ResetIP(eIP);
    }
    else if( u16RspErr & M_SD_ERRSTS ) //SD_STS Reg Error
    {
        //Do Nothing
    }

    /****** (3) Close clock and DMA Stop function ******/
    if(bCloseClk && !gb_SDIODevice[eIP])
        CARD_REG_CLRBIT(A_SD_MODE_REG(eIP), R_CLK_EN | R_DMA_RD_CLK_STOP);

    return eRspErr;
}


//***********************************************************************************************************
// SDMMC HAL Function
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetDataWidth
*     @author jeremy.wang (2015/7/9)
* Desc: According as Data Bus Width to Set IP DataWidth
*
* @param eIP : FCIE1/FCIE2/...
* @param eBusWidth : 1BIT/4BITs/8BITs
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetDataWidth(IpOrder eIP, SDMMCBusWidthEmType eBusWidth)
{
    gu16_SD_MODE_DatLine[eIP] = (U16_T)eBusWidth;
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetBusTiming
*     @author jeremy.wang (2015/7/29)
* Desc:
*
* @param eIP : FCIE1/FCIE2/...
* @param eBusTiming : LOW/DEF/HS/SDR12/DDR...
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetBusTiming(IpOrder eIP, BusTimingEmType eBusTiming)
{
    switch ( eBusTiming )
    {

#if (EN_BYPASSMODE)

        case EV_BUS_LOW:
        case EV_BUS_DEF:
            gu16_DDR_MODE_REG[eIP] = 0;
            gu16_DDR_MODE_REG_ForR2N[eIP] = gu16_DDR_MODE_REG[eIP] | R_PAD_IN_BYPASS;
            break;
        case EV_BUS_HS:
            gu16_DDR_MODE_REG[eIP] = 0;
            gu16_DDR_MODE_REG_ForR2N[eIP] = gu16_DDR_MODE_REG[eIP] | 0;
            break;
        default:
            break;
#else
        //ADVANCE MODE(SDR/DDR) ==> Other can't run bypass mode

        case EV_BUS_LOW:
        case EV_BUS_DEF:
        case EV_BUS_HS:
            gu16_DDR_MODE_REG[eIP] = (R_PAD_CLK_SEL|R_PAD_IN_SEL|R_FALL_LATCH);
            gu16_DDR_MODE_REG_ForR2N[eIP] = gu16_DDR_MODE_REG[eIP] | R_PAD_IN_RDY_SEL | R_PRE_FULL_SEL0 | R_PRE_FULL_SEL1;
            break;
        case EV_BUS_SDR12:
            break;
        case EV_BUS_SDR25:
            break;
        case EV_BUS_SDR50:
            break;
        case EV_BUS_SDR104:
            break;
        case EV_BUS_DDR50:
            break;
        case EV_BUS_HS200:
            break;
        default:
            break;
#endif


    }


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetNrcDelay
*     @author jeremy.wang (2015/7/9)
* Desc: According as Current Clock to Set Nrc Delay
*
* @param eIP : FCIE1/FCIE2/...
* @param u32RealClk : Real Clock
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetNrcDelay(IpOrder eIP, U32_T u32RealClk)
{

    if( u32RealClk >= 8000000 )         //>=8MHz
        gu16_WT_NRC[eIP] = 1;
    else if( u32RealClk >= 4000000 )    //>=4MHz
        gu16_WT_NRC[eIP] = 2;
    else if( u32RealClk >= 2000000 )    //>=2MHz
        gu16_WT_NRC[eIP] = 4;
    else if( u32RealClk >= 1000000 )    //>=1MHz
        gu16_WT_NRC[eIP] = 8;
    else if( u32RealClk >= 400000 )     //>=400KHz
        gu16_WT_NRC[eIP] = 20;
    else if( u32RealClk >= 300000 )     //>=300KHz
        gu16_WT_NRC[eIP] = 27;
    else if( u32RealClk >= 100000 )     //>=100KHz
        gu16_WT_NRC[eIP] = 81;
    else if(u32RealClk==0)
        gu16_WT_NRC[eIP] = 100;          //Default

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetCmdToken
*     @author jeremy.wang (2015/7/8)
* Desc: Set Cmd Token
*
* @param eIP : FCIE1/FCIE2/...
* @param u8Cmd : SD Command
* @param u32Arg : SD Argument
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetCmdToken(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg)
{
    gst_RspStruct[eIP].u8Cmd        = u8Cmd;
    gst_RspStruct[eIP].u32Arg       = u32Arg;

    CARD_REG(A_CFIFO_OFFSET(eIP, 0)) = (((U8_T)(u32Arg>>24))<<8) | (0x40 + u8Cmd);
    CARD_REG(A_CFIFO_OFFSET(eIP, 1)) = (((U8_T)(u32Arg>>8))<<8) | ((U8_T)(u32Arg>>16));
    CARD_REG(A_CFIFO_OFFSET(eIP, 2)) = (U8_T)u32Arg;

    TR_H_SDMMC(sdmmc_print("[S_"));
    TR_H_SDMMC(sdmmc_print("%u", eIP));
    TR_H_SDMMC(sdmmc_print("] CMD_"));
    TR_H_SDMMC(sdmmc_print("%u", u8Cmd));

    TR_H_SDMMC(sdmmc_print(" ("));
    TR_H_SDMMC(sdmmc_print("0x%08X", u32Arg));
    TR_H_SDMMC(sdmmc_print(")"));


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_GetRspToken
*     @author jeremy.wang (2015/7/9)
* Desc: Get Command Response Info
*
* @param eIP : FCIE1/FCIE2/...
*
* @return RspStruct*  : Response Struct
----------------------------------------------------------------------------------------------------------*/
RspStruct* Hal_SDMMC_GetRspToken(IpOrder eIP)
{
    U8_T u8Pos;

    TR_H_SDMMC(sdmmc_print(" =>RSP: ("));
    TR_H_SDMMC(sdmmc_print("0x%04X", (U16_T)gst_RspStruct[eIP].eErrCode));
    TR_H_SDMMC(sdmmc_print(")\r\n"));

    for(u8Pos=0; u8Pos<0x10; u8Pos++ )
        gst_RspStruct[eIP].u8ArrRspToken[u8Pos] = 0;

    TR_H_SDMMC(sdmmc_print("["));

    for(u8Pos=0; u8Pos< gst_RspStruct[eIP].u8RspSize; u8Pos++)
    {
        gst_RspStruct[eIP].u8ArrRspToken[u8Pos] = _BUF_GetByteFromRegAddr((volatile void *)A_SD_CFIFO_POS(eIP), u8Pos);
        TR_H_SDMMC(sdmmc_print("0x%02X", gst_RspStruct[eIP].u8ArrRspToken[u8Pos]));
        TR_H_SDMMC(sdmmc_print(", "));

    }

    TR_H_SDMMC(sdmmc_print("]\r\n\r\n"));

    return &gst_RspStruct[eIP];

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_TransCmdSetting
*     @author jeremy.wang (2015/7/15)
* Desc: For Data Transfer Setting
*
* @param eIP : FCIE1/FCIE2/...
* @param eTransType : CIFD/DMA/ADMA/NONE
* @param u16BCnt : Block Cnt
* @param u16BlkSize : Block Size
* @param u32BufAddr : Memory Address or DMA Table Address (32bits)
* @param pu8Buf : If run CIFD, it neet the buf pointer to do io between CIFD and Buf
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_TransCmdSetting(IpOrder eIP, TransEmType eTransType, U16_T u16BlkCnt, U16_T u16BlkSize, volatile U32_T u32BufAddr, volatile U8_T *pu8Buf)
{
    U32_T u32_TranLen = u16BlkCnt * u16BlkSize;

    CARD_REG(A_BLK_SIZE_REG(eIP)) = u16BlkSize;

    if (eTransType==EV_ADMA)
    {
        CARD_REG(A_JOB_BLK_CNT_REG(eIP)) = 1;        //ADMA BLK_CNT = 1
        CARD_REG(A_DMA_LEN_15_0_REG(eIP))  = 0x10;   //ADMA Fixed Value = 0x10
        CARD_REG(A_DMA_LEN_31_16_REG(eIP)) = 0;
    }
    else //R2N or DMA
    {
        CARD_REG(A_JOB_BLK_CNT_REG(eIP)) = u16BlkCnt;
        CARD_REG(A_DMA_LEN_15_0_REG(eIP))  = (U16_T)(u32_TranLen & 0xFFFF);
        CARD_REG(A_DMA_LEN_31_16_REG(eIP)) = (U16_T)(u32_TranLen >> 16);
    }

    if( (eTransType== EV_DMA) || (eTransType==EV_ADMA) )
    {
        CARD_REG(A_DMA_ADDR_15_0_REG(eIP))  = (U16_T)(u32BufAddr & 0xFFFF);
        CARD_REG(A_DMA_ADDR_31_16_REG(eIP)) = (U16_T)(u32BufAddr >> 16);
    }
    else //CIFD (R2N)
    {
        gpu8Buf[eIP] = pu8Buf;
    }

}

#if EN_HALT
//dump current key registers status and disable UART for debugging.
void halt(int eIP)
{
    _SDMMC_REG_Dump(eIP);
    msleep(1000);
    *((U16_T*)0xFD001C24) &= ~BIT11_T;
    while(1);
}
#else
#define HALT(eIP)
#endif

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SendCmdAndWaitProcess
*     @author jeremy.wang (2015/7/14)
* Desc: Send SD Command and Waiting for Process
*
* @param eIP :  FCIE1/FCIE2/...
* @param eTransType : CIFD/DMA/ADMA/NONE
* @param eCmdType : CMDRSP/READ/WRITIE
* @param eRspType : R1/R2/R3/...
* @param bCloseClk : Close Clock or not
*
* @return RspErrEmType  : Response Error Code
----------------------------------------------------------------------------------------------------------*/
RspErrEmType Hal_SDMMC_SendCmdAndWaitProcess(IpOrder eIP, TransEmType eTransType, CmdEmType eCmdType, SDMMCRspEmType eRspType, BOOL_T bCloseClk)
{
    U32_T u32WaitMS = WT_EVENT_RSP;
    U16_T u16WaitMIEEvent = R_CMD_END;
    CARD_REG(A_CMD_RSP_SIZE_REG(eIP)) = V_CMD_SIZE_INIT | ((U8_T)eRspType);
    CARD_REG(A_MIE_FUNC_CTL_REG(eIP)) = V_MIE_PATH_INIT | _REG_GetMIEFunCtlSetting(eIP);
    CARD_REG(A_SD_MODE_REG(eIP)) = V_SD_MODE_INIT | (eTransType>>8) | gu16_SD_MODE_DatLine[eIP] | ((U8_T)(eTransType & R_DMA_RD_CLK_STOP));
    CARD_REG(A_SD_CTL_REG(eIP))  = V_SD_CTL_INIT | (eRspType>>12) | (eCmdType>>4) | ((U8_T)(eTransType & R_ADMA_EN) | BIT09_T | R_CMD_EN);
    CARD_REG(A_SDIO_MODE_REG(eIP)) = V_SDIO_MODE_INIT | gu16_SDIO_MODE_IntMode[eIP];

    CARD_REG_CLRBIT(A_BOOT_MOD_REG(eIP), R_BOOT_MODE);
    CARD_REG_CLRBIT(A_BOOT_REG(eIP), (R_NAND_BOOT_EN | R_BOOTSRAM_ACCESS_SEL | R_IMI_SEL));

    CARD_REG(A_MMA_PRI_REG_REG(eIP)) = V_MMA_PRI_INIT;
    CARD_REG(A_DDR_MOD_REG(eIP)) = V_DDR_MODE_INIT | (eTransType==EV_CIF ? gu16_DDR_MODE_REG_ForR2N[eIP] : gu16_DDR_MODE_REG[eIP]);

    Hal_Timer_uDelay(gu16_WT_NRC[eIP]);

    if( _REG_ClearSDSTS(eIP, RT_CLEAN_SDSTS) || _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
        return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, bCloseClk, __LINE__);;

#if(EN_BIND_CARD_INT)
    Hal_CARD_INT_SetMIEIntEn(eIP, V_MIE_INT_EN_INIT);//consider do this only once
#endif

#if EN_HALT
    if(gst_RspStruct[eIP].u8Cmd == 8)
    {
        printk("stop for cmd:%d\n", gst_RspStruct[eIP].u8Cmd);
        halt(eIP);
    }
#endif

    if(eCmdType==EV_CMDREAD)
    {
        if (eTransType!=EV_CIF)
        {
            u16WaitMIEEvent |= R_DATA_END;
            u32WaitMS += WT_EVENT_READ;
        }
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_DTRX_EN );
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), (R_DTRX_EN | R_JOB_START) );
    }
    else
    {
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), (R_JOB_START) );
    }

    if(_REG_WaitEvent(eIP, EV_MIE, u16WaitMIEEvent, u32WaitMS))
        return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

    //====== Special Case for R2N CIFD Read Transfer ======
    if( (eCmdType==EV_CMDREAD) && (eTransType==EV_CIF) )
    {
        if(_BUF_CIFD_WaitEvent(eIP, eCmdType, gpu8Buf[eIP]))
           return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

        if(_REG_WaitEvent(eIP, EV_MIE, R_DATA_END, WT_EVENT_READ))
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);
    }

    if( (eRspType == EV_R1B))
    {
        if( _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, bCloseClk, __LINE__);
        if( _REG_WaitDat0HI(eIP, WT_DAT0HI_END) )
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_DAT0_BUSY, bCloseClk, __LINE__);
    }
    else if( (eRspType == EV_R3) || (eRspType == EV_R4) )  // For IP CRC bug
        CARD_REG(A_SD_STS_REG(eIP)) = R_CMDRSP_CERR; //Clear CMD CRC Error

    if( (eCmdType == EV_CMDWRITE) && (!M_REG_STSERR(eIP)) )
    {
        if( _REG_ClearSDSTS(eIP, RT_CLEAN_SDSTS) || _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, bCloseClk, __LINE__);

        CARD_REG(A_SD_CTL_REG(eIP)) = V_SD_CTL_INIT |(eCmdType>>4) | ((U8_T)(eTransType & R_ADMA_EN));
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_DTRX_EN );
        CARD_REG_SETBIT(A_SD_CTL_REG(eIP), (R_DTRX_EN | R_JOB_START) );

        //====== Special Case for R2N CIFD Write Transfer ======
        if ( (eTransType==EV_CIF) && _BUF_CIFD_WaitEvent(eIP, eCmdType, gpu8Buf[eIP]) )
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

        if(_REG_WaitEvent(eIP, EV_MIE, R_DATA_END, WT_EVENT_WRITE))
            return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

    }

    return _SDMMC_EndProcess(eIP, eCmdType, (RspErrEmType)M_REG_STSERR(eIP), bCloseClk, __LINE__);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_RunBrokenDmaAndWaitProcess
*     @author jeremy.wang (2015/7/29)
* Desc: For Broken DMA Data Transfer
*
* @param eIP : FCIE1/FCIE2/...
* @param eCmdType : READ/WRITE
*
* @return RspErrEmType  : Response Error Code
----------------------------------------------------------------------------------------------------------*/
RspErrEmType Hal_SDMMC_RunBrokenDmaAndWaitProcess(IpOrder eIP, CmdEmType eCmdType)
{
    U32_T u32WaitMS = 0;

    if(eCmdType==EV_CMDREAD)
        u32WaitMS = WT_EVENT_READ;
    else if(eCmdType==EV_CMDWRITE)
        u32WaitMS = WT_EVENT_WRITE;

    if ( _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
        return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, FALSE, __LINE__);

    CARD_REG_CLRBIT(A_SD_CTL_REG(eIP), R_CMD_EN );
    CARD_REG_SETBIT(A_SD_CTL_REG(eIP), (R_DTRX_EN | R_JOB_START) );

    if ( _REG_WaitEvent(eIP, EV_MIE, R_DATA_END, u32WaitMS) )
        return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, FALSE, __LINE__);

    return _SDMMC_EndProcess(eIP, eCmdType, (RspErrEmType)M_REG_STSERR(eIP), FALSE, __LINE__);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_ADMASetting
*     @author jeremy.wang (2015/7/13)
* Desc: For ADMA Data Transfer Settings
*
* @param pDMATable : DMA Table memory address pointer
* @param u8Item : DMA Table Item 0 ~
* @param u32SubLen :  DMA Table DMA Len
* @param u32SubBCnt : DMA Table Job Blk Cnt
* @param u32SubAddr : DMA Table DMA Addr
* @param u8MIUSel : MIU Select
* @param bEnd : End Flag
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_ADMASetting(volatile void *pDMATable, U8_T u8Item, U32_T u32SubLen, U16_T u16SubBCnt, U32_T u32SubAddr, U8_T u8MIUSel, BOOL_T bEnd)
{
    //U16_T u16Pos;

    AdmaDescStruct* pst_AdmaDescStruct = (AdmaDescStruct*) pDMATable;

    pst_AdmaDescStruct = (pst_AdmaDescStruct+u8Item);
    memset(pst_AdmaDescStruct, 0, sizeof(AdmaDescStruct));

    pst_AdmaDescStruct->u32_DmaLen = u32SubLen;
    pst_AdmaDescStruct->u32_Address = u32SubAddr;
    pst_AdmaDescStruct->u32_JobCnt = u16SubBCnt;
    pst_AdmaDescStruct->u32_MiuSel = u8MIUSel;
    pst_AdmaDescStruct->u32_End    = bEnd;

    /*sdmmc_print("\r\n");
    sdmmc_print("gpst_AdmaDescStruct Pos=(");
    sdmmc_print("0x%08X", (U32_T)pst_AdmaDescStruct);
    sdmmc_print(")\r\n");

    for(u16Pos=0; u16Pos<192 ; u16Pos++)
    {
        if( (u16Pos%12)==0)
           (sdmmc_print("\r\n"));

        (sdmmc_print("["));
        (sdmmc_print("0x%02X",  *(pu8Buf+u16Pos)));
        (sdmmc_print("]"));

    }*/


}


//***********************************************************************************************************
// SDMMC Operation Function
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_ClkCtrl
 *     @author jeremy.wang (2015/11/4)
 * Desc: Enable IP clock output
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 * @param u16DelayMs : Delay ms to Specail Purpose
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_ClkCtrl(IpOrder eIP, BOOL_T bEnable, U16_T u16DelayMs)
{
    CARD_REG(A_MIE_FUNC_CTL_REG(eIP)) = V_MIE_PATH_INIT | _REG_GetMIEFunCtlSetting(eIP);

    if( bEnable )
        CARD_REG_SETBIT(A_SD_MODE_REG(eIP), R_CLK_EN);
    else
        CARD_REG_CLRBIT(A_SD_MODE_REG(eIP), R_CLK_EN);

    Hal_Timer_mSleep(u16DelayMs);
    //Hal_Timer_mDelay(u16DelayMs);
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_Reset
*     @author jeremy.wang (2015/7/17)
* Desc:  Reset IP to avoid IP dead
*
* @param eIP : FCIE1/FCIE2/...
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_Reset(IpOrder eIP)
{
    _REG_ResetIP(eIP);
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_StopProcessCtrl
 *     @author jeremy.wang (2015/11/3)
 * Desc: Stop process to avoid long time waiting
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_StopProcessCtrl(IpOrder eIP, BOOL_T bEnable)
{
    gb_StopWProc[eIP] = bEnable;

#if(EN_BIND_CARD_INT)
    if ( gb_StopWProc[eIP] )
        Hal_CARD_INT_StopWaitMIEEventCtrl(eIP, TRUE);
    else
        Hal_CARD_INT_StopWaitMIEEventCtrl(eIP, FALSE);
#endif

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_OtherPreUse
*     @author jeremy.wang (2015/7/17)
* Desc: Use this for avoid SD/EMMC to use FCIE at the same time
*
* @param eIP : FCIE1/FCIE2/...
*
* @return BOOL_T  :  (TRUE: Other driver usnig)
----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_SDMMC_OtherPreUse(IpOrder eIP)
{

    if ( CARD_REG(A_MIE_FUNC_CTL_REG(eIP)) & R_EMMC_EN )
        return (TRUE);

    if ( !(CARD_REG(A_MIE_FUNC_CTL_REG(eIP)) & (R_SD_EN | R_SDIO_MODE)) ) //Not SD Path
        return (TRUE);

    return (FALSE);
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_ErrGroup
 *     @author jeremy.wang (2015/10/28)
 * Desc: transfer response error type to group error type
 *
 * @param eRspErrType : Response Error Type
 *
 * @return ErrGrpEmType  : Group Error Type
 ----------------------------------------------------------------------------------------------------------*/
ErrGrpEmType Hal_SDMMC_ErrGroup(RspErrEmType eRspErrType)
{

    switch((U16_T)eRspErrType)
    {
        case EV_STS_OK:
            return EV_EGRP_OK;

        case EV_STS_WR_TOUT:
        case EV_STS_NORSP:
        case EV_STS_RD_TOUT:
        case EV_STS_RIU_ERR:
        case EV_STS_DAT0_BUSY:
        case EV_STS_MIE_TOUT:
            return EV_EGRP_TOUT;

        case EV_STS_RD_CERR:
        case EV_STS_WD_CERR:
        case EV_STS_RSP_CERR:
            return EV_EGRP_COMM;

        default:
            return EV_EGRP_OTHER;
    }

}


//***********************************************************************************************************
// SDMMC Information
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_DumpMemTool
*     @author jeremy.wang (2015/7/29)
* Desc: Help us to dump memory
*
* @param u8ListNum : Each List Num include 16 bytes
* @param pu8Buf : Buffer Pointer
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_DumpMemTool(U8_T u8ListNum, volatile U8_T *pu8Buf)
{
    U16_T u16Pos=0;
    U8_T u8ListPos;
    U32_T u32BufAddr = (U32_T)pu8Buf;
    sdmmc_print("\r\n $[Prt MEM_DATA: 0x%08X ]\r\n", u32BufAddr);

    for(u8ListPos=0 ; u8ListPos <u8ListNum; u8ListPos++ )
    {
        u16Pos= u8ListPos*16;
        for(; u16Pos< (u8ListPos+1)*16; u16Pos++)
        {
            sdmmc_print("[0x%02X]", pu8Buf[u16Pos]);
        }
        sdmmc_print("\r\n");
    }

    sdmmc_print("\r\n");
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_GetDATBusLevel
*     @author jeremy.wang (2015/7/17)
* Desc: Monitor bus level for debug
*
* @param eIP : FCIE1/FCIE2/...
*
* @return U8_T  : Return DAT Bus Level (4Bits)
----------------------------------------------------------------------------------------------------------*/
U8_T Hal_SDMMC_GetDATBusLevel(IpOrder eIP)
{
    U16_T u16Temp = 0;

    u16Temp = CARD_REG(A_SD_STS_REG(eIP)) & 0x0F00;

    return (U8_T)(u16Temp>>8);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_GetMIEEvent
*     @author jeremy.wang (2015/7/31)
* Desc: Monitor MIE Event for debug
*
* @param eIP : FCIE1/FCIE2/...
*
* @return U16_T  : Return MIE Event
----------------------------------------------------------------------------------------------------------*/
U16_T Hal_SDMMC_GetMIEEvent(IpOrder eIP)
{
    return CARD_REG(A_MIE_EVENT_REG(eIP));
}


//***********************************************************************************************************
// SDMMC SDIO Setting
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SDIODeviceCtrl
 *     @author jeremy.wang (2015/11/3)
 * Desc: Enable SDIO device for do something for SDIO case
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SDIODeviceCtrl(IpOrder eIP, BOOL_T bEnable)
{
    if(bEnable)
        gb_SDIODevice[eIP] = TRUE;
    else
        gb_SDIODevice[eIP] = FALSE;

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SDIOIntDetCtrl
 *     @author jeremy.wang (2015/11/3)
 * Desc: Enable SDIO interrupt detection
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SDIOIntDetCtrl(IpOrder eIP, BOOL_T bEnable)
{
    if (gb_SDIODevice[eIP])
    {
        if(bEnable)
        {
            CARD_REG_SETBIT(A_SDIO_DET_ON(eIP), R_SDIO_DET_ON);
        }
        else
        {
            CARD_REG_CLRBIT(A_SDIO_DET_ON(eIP), R_SDIO_DET_ON);
        }
    }
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIOIntBeginSetting
 *     @author jeremy.wang (2015/11/3)
 * Desc: Set SDIO Int begin setting
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param u8Cmd : SDIO Command
 * @param u32Arg : SDIO Argument
 * @param eCmdType : CMDRSP/READ/WRITE
 * @param u16BlkCnt : Block Cnt
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIOIntBeginSetting(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg, CmdEmType eCmdType, U16_T u16BlkCnt)
{
    BOOL_T bSDIOAbortMode = (u8Cmd == 52) && ( (u32Arg & 0x83FFFE00) == 0x80000C00);

    gu16_SDIO_MODE_IntMode[eIP] = 0;

    if(gb_SDIODevice[eIP])
    {
        if(eCmdType != EV_CMDRSP)
        {
            if(u16BlkCnt == 1)
                gu16_SDIO_MODE_IntMode[eIP] = R_SDIO_INT_MOD1;
            else
                gu16_SDIO_MODE_IntMode[eIP] = R_SDIO_INT_MOD0 | R_SDIO_INT_MOD1;
        }
        else if ( (u8Cmd == 12) || bSDIOAbortMode)
        {
            gu16_SDIO_MODE_IntMode[eIP] = R_SDIO_INT_MOD0;
        }
    }

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIOIntEndSetting
 *     @author jeremy.wang (2015/10/29)
 * Desc:
 *
 * @param eIP :
 * @param eRspErr :
 * @param u16BlkCnt :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIOIntEndSetting(IpOrder eIP, RspErrEmType eRspErr, U16_T u16BlkCnt)
{
    U16_T u16RspErr = (U16_T)eRspErr;

    if(gb_SDIODevice[eIP])
    {
        if(u16BlkCnt>1)
        {

        }
        if(u16RspErr>0)
        {
        }
    }
}


//***********************************************************************************************************
// SDMMC Interrupt Setting
//***********************************************************************************************************

#if(EN_BIND_CARD_INT)

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_MIEIntCtrl
 *     @author jeremy.wang (2015/11/2)
 * Desc: Enable MIE event interrupt control
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_MIEIntCtrl(IpOrder eIP, BOOL_T bEnable)
{
    Hal_CARD_INT_MIEIntCtrl(eIP, bEnable);
}



#endif  // End of EN_BIND_CARD_INT
