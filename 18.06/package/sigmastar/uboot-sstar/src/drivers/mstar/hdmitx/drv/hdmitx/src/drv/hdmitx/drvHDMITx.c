/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#define  MDRV_HDMITX_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include "mhal_common.h"
#include "MsVersion.h"
#include "regHDMITx.h"
#include "halHDMIUtilTx.h"
#include "halHDMITx.h"
#include "halHDCPTx.h"
#include "drvHDMITx.h"
#include "drv_hdmitx_os.h"
//#include <bigd.h>


#include "apiHDMITx.h"
//#include "utopia_dapi.h"

//#include "utopia.h"
#include "HDMITX_private.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define HDMITX_TASK_STACK_SIZE          4096UL
//#define HDCPTX_TASK_STACK_SIZE          4096UL //wilson@kano

#define HDMITX_MUTEX_TIMEOUT            2000UL        ///< The maximum timeout
#define HDMITX_MONITOR_DELAY            50UL

#define HDCPTX_MONITOR_DELAY            10UL //wilson@kano


#define USE_INTERNAL_HDCP_KEY           1UL // Use internal HDCP key
#define USE_AUTO_GEN_AN                 1UL // Use auto An generator
#define CheckRx_Timer                   100UL // Check Rx HPD and TMDS clock status

/*
 * HDCP CTS 1A-01: NOT JUDGED
 * Refine timing to fix Ri Timer expired issue: Inactivity timer expired.
 */
#define HDCP_1ST_RiCheck_Timer          100UL //2600 // Check HDCP Ri value in both of Rx and Tx sides
#define HDCP_RiCheck_Timer              500UL //2600 // Check HDCP Ri value in both of Rx and Tx sides

//In Test Instrument, Quantumdata882, it read each DDC step with 30ms sample periods
//While doing SEC Compatibility Test, we find some REPEATER need enlarge sample rate to above 20ms to avoid timeout problem (YAMAHA RX-V2700)
//In SEC Compatibility Test, another repeater face RX Ri freeze after auth done. Enlarge sample rate to 125ms to avoid problem (YAMAHA RX-V467)
#define HDCP_DDC_SAMPLE_PERIOD          125UL //20//10 //SEC Compatibility Test (YAMAHA RX-V2700)(YAMAHA RX-V467)

#define HDCP14TX_R0_WDT		 			100UL     //wilson@kano 100 ms
#define HDCP14TX_BSTATUS_TIMEOUT_CNT    9000UL //according to QD886 log, it require 9second polling bstatus //600UL //wilson@kano
#define HDCP14TX_REP_RDY_BIT_WDT		5000UL    //wilson@kano 5sec


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#if 0
typedef    struct    {
    BIGD   p, q, g, x, y;
} dsa_context;
#endif
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
static MsHDMITX_RX_STATUS gbCurRxStatus = E_HDMITX_DVIClock_L_HPD_L;

//MDrvHDMITX_PARAMETER_LIST   gHDMITxInfo;
//MDrvHdcpTx_PARAMETER_LIST   gHdcpTxInfo;

MsHDMITX_INTERRUPT_TYPE gHDMITXIRQ = E_HDMITX_IRQ_12|E_HDMITX_IRQ_10; // HPD and Clock disconnect
MS_U8 gRepeaterFailTime = 0; // for MAX_CASCADE_EXCEEDED and MAX_DEVS_EXCEEDED
MS_U32 gRepeaterStartTime = 0; // for MAX_CASCADE_EXCEEDED and MAX_DEVS_EXCEEDED
MS_U16 gHDCPCheckRiTimer = HDCP_RiCheck_Timer;//2600;
MS_BOOL g_bDisableRegWrite = FALSE;
MS_BOOL g_bDisableTMDSCtrl = FALSE;
MS_BOOL g_bDisableAvMuteCtrl = FALSE;

MS_BOOL g_bDisableVSInfo = TRUE;

MS_BOOL g_bHDMITxTask = TRUE;
MS_BOOL g_bHdcpTxTask = TRUE; //wilson@kano
MS_BOOL g_bHpdIrqEn   = FALSE;

///MS_BOOL g_bHDCPRxValid = TRUE; //E_HDMITX_HDCP_RX_IS_NOT_VALID //E_HDMITX_HDCP_RX_IS_VALID //wilson@kano
static MS_U32 gdwPreTime = 0; //wilson@kano
static MS_U32 gdwCurTime = 0; //wilson@kano

MsHDMITX_RX_STATUS                 _hdmitx_preRX_status = E_HDMITX_DVIClock_L_HPD_L;

MS_S32 s32HDMITx_Sem = -1;
DrvHdmitxOsSemConfig_t tHDMITX_Sem;
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
//wilson@kano
MS_U8 g_u8Bstatus[2] = {0x00};
MS_U8 g_u8KSVList[635] = {0x00};
MS_BOOL g_bValidKSVList = FALSE;
static MS_U8 g_u8SupTimingNum = 0;

const MS_U8 VSDB_HDMI_IEEE_ID[3] =
{
	0x03, 0x0C, 0x00
};
//wilson@kano
const MS_U8 HFVSDB_HDMI_IEEE_ID[3] =
{
	0xD8, 0x5D, 0xC4
};


static void                           *_pHDMITxTaskStack;
static MS_S32                          _s32HDMITxTaskId = -1;
static DrvHdmitxOsTaskConfig_t         _tHDMITxTask;
static MS_S32                          _s32HDMITxEventId;
static MS_U8                           _u8HDMITx_StackBuffer[HDMITX_TASK_STACK_SIZE];

//wilson@kano
#if 0
static void                           *_pHdcpTxTaskStack;
static MS_S32                          _s32HdcpTxTaskId = -1;
static MS_S32                          _s32HdcpTxEventId;
static MS_U8                           _u8HdcpTx_StackBuffer[HDCPTX_TASK_STACK_SIZE];
#endif
//wilson@kano

static DrvHdmitxOsTimerConfig_t     _tCheckRxTimerCfg;
static DrvHdmitxOsTimerConfig_t     _tHDCPRiTimerCfg;
static MS_S32                       _s32CheckRxTimerId = -1;
static MS_S32                       _s32HDCPRiTimerId = -1;

static MS_U32                       _u32Aksv2RoInterval = 150;


MS_U8 HDMITX_EdidHeaderTbl[] =
{
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
};

MS_U8 HDMITX_IEEEHdmiIdTbl[] =
{
    0x03, 0x0C, 0x00,
};

#define HDMITX_DRV_VERSION                  /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    {'P','Q','_','_'},                  /* IP__                                             */  \
    {'0','0'},                          /* 0.0 ~ Z.Z                                        */  \
    {'0','0'},                          /* 00 ~ 99                                          */  \
    {'0','0','1','1','5','1','5','7'},  /* CL#                                              */  \
    MSIF_OS

/// Debug information
//static MSIF_Version _drv_hdmi_tx_version = {.DDI = { HDMITX_DRV_VERSION },};
static MS_HDMI_TX_INFO _info =
    {
        // TODO: ADD hdmi tx related info here.
    };
static MS_HDMI_TX_Status _hdmitx_status = {.bIsInitialized = FALSE, .bIsRunning = FALSE,};
//static MS_HDCP_TX_Status _hdcptx_status = {.bIsInitialized = FALSE, .bIsRunning = FALSE,};


#ifdef CAMDRV_DEBUG
static MS_U16 _u16DbgHDMITXSwitch = HDMITX_DBG|HDMITX_DBG_HDCP;
#else
static MS_U16 _u16DbgHDMITXSwitch = 0;
#endif

MS_BOOL bCheckEDID = TRUE; // Check EDID only when HPD from low to high

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

#define DBG_HDMITX(_f)                do{ if(_u16DbgHDMITXSwitch & HDMITX_DBG){ (_f);} } while(0)
#define DBG_HDCP(_f)                  do{ if(_u16DbgHDMITXSwitch & HDMITX_DBG_HDCP){ (_f);} } while(0)

#define EDID_DEBUG_CHECK()            (_u16DbgHDMITXSwitch & HDMITX_DBG_EDID)
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//extern MS_BOOL MDrv_EEPROM_Write(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size);
MS_BOOL MDrv_EEPROM_Read(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size)
{
    return TRUE;
}

//------------------------------------------------------------------------------
/// @brief Set event of HDMI ISR
/// @param  None
/// @return None
//------------------------------------------------------------------------------

#if HDMITX_ISR_ENABLE

static void _HDMITx_Isr(MS_U32 u32IntNum)//irqreturn_t _HDMITx_Isr(InterruptNum u32IntNum, void* dev_id)
{
//Wilson:undercheck, remove argument #2 to avoid build warning
    MS_U32 irq_status;
    u32IntNum = u32IntNum;      // prevent compile warning

    //Clear Interrupt Bit
    irq_status = MHal_HDMITx_Int_Status();
    if(irq_status & E_HDMITX_IRQ_12) // TMDS hot-plug
    {
        MHal_HDMITx_Int_Clear(E_HDMITX_IRQ_12);
    }
    else if(irq_status & E_HDMITX_IRQ_10) // Rx disconnection
    {
        MHal_HDMITx_Int_Clear(E_HDMITX_IRQ_10);
    }

    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_IRQ);
    //return IRQ_HANDLED;
}
#endif

#if 0 //ENABLE_CEC_INT
irqreturn_t _HDMITX_CEC_RxIsr(MS_U32 u32IntNum, void* dev_id)
{
    u32IntNum = u32IntNum;      // prevent compile warning

    MDrv_CEC_RxChkBuf();
    DrvHdmitxOsEnableInterrupt(E_INT_IRQ_CEC);

    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_CECRX);
    return IRQ_HANDLED;
}
#endif // ENABLE_CEC_INT


//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_Get_Semaphore()
//  [Description]:
//                  get hdmi tx semaphore
//  [Arguments]:
//                  [void] *pInstances
//  [Return]:
//                  MS_U32
//
//**************************************************************************
MS_U32 MDrv_HDMITx_Get_Semaphore(void *pInstance)
{
    if(s32HDMITx_Sem == -1)
    {
        if(DrvHdmitxOsCreateSemaphore(&tHDMITX_Sem, 1))
        {
            s32HDMITx_Sem = tHDMITX_Sem.s32Id;
        }

        if(s32HDMITx_Sem < 0)
        {
            return 0;
        }
    }

    DrvHdmitxOsObtainSemaphore(&tHDMITX_Sem);
    return 1;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_Release_Semaphore()
//  [Description]:
//                  Release hdmi tx semaphore
//  [Arguments]:
//                  [void] *pInstances
//  [Return]:
//                  MS_U32
//
//**************************************************************************
MS_U32 MDrv_HDMITx_Release_Semaphore(void *pInstance)
{
    DrvHdmitxOsReleaseSemaphore(&tHDMITX_Sem);
    return 1;
}


//**************************************************************************
//  [Function Name]:
//			_MDrv_HDCP_StopRiTimer()
//  [Description]:
//                  stop Ri timer
//  [Arguments]:
//                  void
//  [Return]:
//                  void
//
//**************************************************************************
void _MDrv_HDCPTx_StopRiTimer(void)
{
    if(_s32HDCPRiTimerId > 0)
    {
        DrvHdmitxOsStopTimer(&_tHDCPRiTimerCfg);
    }
}

/*********************************************************************/
/*                                                                                                                     */
/*                               Sorting HDCP Tx function here                                          */
/*                                                                                                                     */
/*********************************************************************/
void ____HdcpTx_Relative_Func____(void){} //Null function for navigation

/*********************************************************************/
/*                                                                                                                     */
/*                                         HDCP22 Relative                                                    */
/*                                                                                                                     */
/*********************************************************************/
MS_BOOL MDrv_HDMITx_HDCP2AccessX74(MS_U8 u8PortIdx, MS_U8 u8OffsetAddr, MS_U8 u8OpCode, MS_U8 *pu8RdBuf, MS_U8 *pu8WRBuff, MS_U16 u16RdLen, MS_U16 u16WRLen)
{
    MS_BOOL bRet = FALSE;

    //printf("[HDCP2Tx]PrtIdx = %d, Offset : 0x%02X, OpCode : %d, Data Len = 0x%X\r\n", u8PortIdx, u8OffsetAddr, u8OpCode, (u8OpCode == 0x01) ? u16RdLen : u16WRLen);
    switch (u8OffsetAddr)
    {
        case 0x50: //HDCP2Version
            bRet = MHal_HDMITx_Rx74ReadByte(u8OffsetAddr, pu8RdBuf);
        break;

        case 0x60: //Write_Message
            bRet = MHal_HDMITx_Rx74WriteBytes(u8OffsetAddr, u16WRLen, pu8WRBuff);
        break;

        case 0x70: //RxStatus
            bRet = MHal_HDMITx_Rx74ReadBytes(u8OffsetAddr, (u16RdLen > 0x02) ? 0x02 : u16RdLen, pu8RdBuf);
        break;

        case 0x80:
            bRet = MHal_HDMITx_Rx74ReadBytes(u8OffsetAddr, u16RdLen, pu8RdBuf);
        break;

        case 0xC0:
            if (u8OpCode == 0x01)
                bRet = MHal_HDMITx_Rx74ReadBytes(u8OffsetAddr, u16RdLen, pu8RdBuf);
            else
                bRet = MHal_HDMITx_Rx74WriteBytes(u8OffsetAddr, u16WRLen, pu8WRBuff);
        break;

        default:
            if (u8OpCode == E_HDCP2_OPCODE_READ)
            {
                bRet = MHal_HDMITx_Rx74ReadBytes(u8OffsetAddr, u16RdLen, pu8RdBuf);
            }
            else
            {
                bRet = MHal_HDMITx_Rx74WriteBytes(u8OffsetAddr, u16WRLen, pu8WRBuff);
            }
            break;
    }

    return bRet;
}

void MDrv_HDMITx_HDCP2TxInit(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDMITx_HDCP2TxInit(bEnable);
}

void MDrv_HDMITx_HDCP2TxEnableEncryptEnable(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDMITx_HDCP2TxEnableEncryptEnable(bEnable);
}

void MDrv_HDMITx_HDCP2TxFillCipherKey(MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128)
{
    MHal_HDMITx_HDCP2TxFillCipherKey(pu8Riv, pu8KsXORLC128);
}

//------------------------------------------------------------------------------
/// @brief This routine is to HDCP check repeater function
/// @return authentication status
//------------------------------------------------------------------------------
MsHDMITX_HDCP_AUTH_STATUS _MDrv_HDCPTx_CheckRepeater(void)
{
    MS_U16 i, j;
    MS_U8 count;
    MS_U8 testbyte[635], bstatus[2];
    SHA1_DATA   test_dig;
    MS_U8 check_error = 0;
    MS_U16 reg_value;
    MS_U8 tempV[20];
    MS_U32 u32DDCDelay = 0;

    MHal_HDMITx_HdcpSha1Init(&test_dig);

    DBG_HDCP(printf("Repeater is ready!!!\r\n"));

    // read ksv list
#if 0
    if(MHal_HDMITx_Rx74ReadByte(0x41, &count) == FALSE)
    {
        DBG_HDCP(printf("HDCP check repeater failed because I2C\r\n"));
        return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
    }
    count &= 0x7F;
#else
    if(MHal_HDMITx_Rx74ReadBytes(0x41, 2, bstatus) == FALSE)
    {
        DBG_HDCP(printf("HDCP check repeater failed because I2C\r\n"));
        return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
    }

    memcpy(g_u8Bstatus, bstatus, sizeof(bstatus));

    if((bstatus[0] & 0x80) || (bstatus[1] & 0x08)) // "MAX_DECS_EXCEEDED" or "MAX_CASCADE_EXCEEDED"
    {
        return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
    }
    count = bstatus[0];

#endif

    if(count)
    {
        if (count > 16)
        {
            //Store original delay count
            u32DDCDelay = MHal_HDMITx_GetDDCDelayCount();
            MHal_HDMITx_AdjustDDCFreq(100); //speed-up to 100KHz
        }

        if(MHal_HDMITx_Rx74ReadBytes(0x43, 5*count, (MS_U8 *)testbyte) == FALSE)
        {
            DBG_HDCP(printf("HDCP check repeater failed because I2C\r\n"));
            return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
        }

        if (count > 16)
        {
            //Set original delay count
            MHal_HDMITx_SetDDCDelayCount(u32DDCDelay);
        }

        memcpy(g_u8KSVList, testbyte, 5*count);
        g_bValidKSVList = TRUE;
    }
    DBG_HDCP(printf("KSV List:(=%d) ", count));
    if(count)
    {
        for (j = 0; j < 5*count; j++)
            DBG_HDCP(printf("0x%x, ", testbyte[j]));
        DBG_HDCP(printf("\r\n"));
        MHal_HDMITx_HdcpSha1AddData(&test_dig, testbyte, 5*count);
    }
    // read Bstatus (2 bytes)
    if(MHal_HDMITx_Rx74ReadBytes(0x41, 2, (MS_U8 *) testbyte) == FALSE)
    {
        DBG_HDCP(printf("HDCP check repeater failed because I2C\r\n"));
        return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
    }
    MHal_HDMITx_HdcpSha1AddData(&test_dig, testbyte, 2);

    // read M0 (8 byte)
    for (i = 0; i < 4; i++)
    {
        reg_value = MHal_HDMITX_GetM02Bytes(i);
        testbyte[2*i] = (MS_U8)(reg_value & 0x00FF);
        testbyte[2*i+1] = (MS_U8)((reg_value & 0xFF00) >> 8);
       // DBG_HDCP(printf(DBG_HDCP, "0x%x, ", testbyte[i]);
    }

    MHal_HDMITx_HdcpSha1AddData(&test_dig, testbyte, 8);
    MHal_HDMITx_HdcpSha1FinalDigest(&test_dig, testbyte);

    //print out SHA1 encode result
    DBG_HDCP(printf("SHA1 encode result(V)=: "));
    for (i = 0; i < 20; i++)
        DBG_HDCP(printf("0x%x, ", testbyte[i]));
    DBG_HDCP(printf("\r\n"));

    DBG_HDCP(printf("V'=: "));

    /*
     * [HDCP] 1A-01, 1B-01a: DUT kept HDCP encryption even though it did not read V' completely
     */
    if( MHal_HDMITx_Rx74ReadBytes(0x20, 20, tempV) == FALSE)
    {
        DBG_HDCP(printf("HDCP check repeater failed because I2C\r\n"));
        return E_HDMITX_HDCP_REPEATER_SHA1_FAIL;
    }

    for (i = 0; i < 20; i++)
    {
        DBG_HDCP(printf("0x%x, ", tempV[i]));
        if (tempV[i] != testbyte[i])
            check_error = 1;
    }

    DBG_HDCP(printf("\r\n"));

    return ((check_error == 0) ? E_HDMITX_HDCP_REPEATER_SHA1_PASS: E_HDMITX_HDCP_REPEATER_SHA1_FAIL);
}

//------------------------------------------------------------------------------
/// @brief This routine is to HDCP check Ri function
/// @return authentication status
//------------------------------------------------------------------------------
MsHDMITX_HDCP_AUTH_STATUS _MDrv_HDCPTX_AuthCheckRi(void)
{
    MS_U16 ptx_ri = 0, prx_ri = 0; // Fix coverity impact.

    if (MHal_HDMITx_HdcpCheckRi(&ptx_ri, &prx_ri) == TRUE)
    {
        //DBG_HDCP(printf("\nKey match: tx_ri= 0x%x, rx_ri= 0x%x\n", ptx_ri, prx_ri));
        return E_HDMITX_HDCP_SYNC_RI_PASS;
    }
    else
    {
        DBG_HDCP(printf("Tx_Ri=%x, Rx_Ri=%x\n", ptx_ri, prx_ri));
        return E_HDMITX_HDCP_SYNC_RI_FAIL;
    }
}

//-------------------------------------------------------------------------------------------------
// HDCP Ri timer callback
// @param  stTimer \b IN: Useless
// @param  u32Data \b IN: Useless
// @return None
//-------------------------------------------------------------------------------------------------
static void _MDrv_HDMITX_HDCP_Ri_TimerCallback(MS_U32 stTimer, MS_U32 u32Data)
{
    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RITIMER); //wilson@kano
}

/*
 * [HDCP] 1A and 1B: AKSV -> Ro should be large than 100msec
 * to pass Quantumdata 882 HDCP test, we delay 150ms
 *
 * 2013/11/07, in SEC Compatibility test, we meet an Repeater timeout error on PEPEATER YAMAHA RX-V2700
 * patch AKSV -> Ro large than 250ms
 */
void MDrv_HDCPTx_SetAksv2R0Interval(MS_U32 u32Interval)
{
    _u32Aksv2RoInterval = u32Interval;
}

//------------------------------------------------------------------------------
/// @brief This routine is to HDCP waiting for the active Rx
/// @return authentication status
//------------------------------------------------------------------------------
MsHDMITX_HDCP_AUTH_STATUS _MDrv_HDCPTx_AuthWaitingActiveRx(void)
{
    MHal_HDMITx_HdcpInit();
    if (!MHal_HDMITx_HdcpRxActive())
    {
        return E_HDMITX_HDCP_RX_IS_NOT_VALID; // not active
    }

    return E_HDMITX_HDCP_RX_IS_VALID; // active Rx
}


//------------------------------------------------------------------------------
/// @brief This routine is to HDCP check whether repeater is ready or not
/// @return authentication status
//------------------------------------------------------------------------------
MsHDMITX_HDCP_AUTH_STATUS _MDrv_HDCPTx_AuthCheckRepeaterReady(void)
{
    MS_U8 regval;

    if(MHal_HDMITx_Rx74ReadByte(0x40, &regval) == TRUE)
    {
        if(regval & BIT5)
            return  E_HDMITX_HDCP_REPEATER_READY; // ready
    }
    return E_HDMITX_HDCP_REPEATER_NOT_READY; // not ready
}


//**************************************************************************
//  [Function Name]:
//			_MDrv_HDCP_GetTimeDiff()
//  [Description]:
//                  calculate timer difference
//  [Arguments]:
//                  [MS_U32] dwPreTime
//                  [MS_U32] dwPostTime
//  [Return]:
//                  MS_U32
//
//**************************************************************************
MS_U32 _MDrv_HDCP_GetTimeDiff(MS_U32 dwPreTime, MS_U32 dwPostTime)
{
	if (dwPreTime > dwPostTime)
	{
		return (dwPreTime + (~dwPostTime));
	}
	else
	{
		return (dwPostTime - dwPreTime);
	}
}

//------------------------------------------------------------------------------
/// @brief This routine is to check HDCP sync setting
/// @return None
//------------------------------------------------------------------------------
MS_BOOL _MDrv_HDCPTx_HdcpSyncSetting(void* pInstance, MsHDMITX_OUTPUT_MODE tmds_mode)
{
    MS_U8 temp, regval = 0; // Fix coverity impact.
    MS_U8 tx_mode = 0;
    MS_BOOL tmp = 0;  // Fix coverity impact.
    _HDMITX_GET_VARIABLE();

    //Check Bcaps
    if(MHal_HDMITx_Rx74ReadByte(0x40, &temp) == FALSE)
        return FALSE;

    // EESS/OESS, advance cipher
    if (tmds_mode & BIT1)
    {
        tx_mode |= BIT2;      // EESS

        if (temp & BIT1)
        {
            tx_mode |= BIT3;
            tmp = MHal_HDMITx_Rx74WriteByte(0x15, 0x02);
            tmp = MHal_HDMITx_Rx74ReadByte(0x15, &regval);
            DBG_HDCP(printf("HDMI Advanced Cipher!!! %d \r\n", regval));
        }
    }
    else     //DVI
    {
        if(temp & BIT1)
        {
            tx_mode  |= BIT2;
            tx_mode  |= BIT3;
            tmp = MHal_HDMITx_Rx74WriteByte(0x15,0x02);
            DBG_HDCP(printf("Tx select EESS and enable Advance Cipher!!\r\n"));
        }
    }


    // Repeater mode?
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpCheckRepeater_flag = FALSE;
    if (temp & BIT6)
    {
        tx_mode |= BIT1;
        psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpCheckRepeater_flag = TRUE;
        DBG_HDCP(printf("Rx is a Repeater!!!\r\n"));
    }
    MHal_HDMITX_SetHDCPConfig(tx_mode);
    tmp = TRUE;
    return tmp;
}

//------------------------------------------------------------------------------
/// @brief This routine set HDMI Tx initial situation.
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDCPTx_InitVariable(void *pInstance)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHdcpTxInfo.hdcp_Rx_valid = FALSE;

    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_IDLE;

    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpUseInternalKey_flag = TRUE;
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpStartAuth_flag = FALSE;
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpAuthDone_flag = FALSE;
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpCheckRepeater_flag = FALSE;

    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_RESET;
    psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control = E_NORMAL_OUTPUT;
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPRxFail_Control = E_RXFail_NORMAL_OUTPUT;

    psHDMITXResPri->stHdcpTxInfo.revocationlist_ready = FALSE;
    psHDMITXResPri->stHdcpTxInfo.revocation_size = 0;
    psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_READY;
    psHDMITXResPri->stHdcpTxInfo.HDCP_74_check = FALSE;

    // HDCP init
    MHal_HDMITx_HdcpDebugEnable(_u16DbgHDMITXSwitch & HDMITX_DBG_HDCP ? TRUE : FALSE);

    if (psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpUseInternalKey_flag == TRUE)
    {
        MHal_HDMITx_HdcpKeyInit(USE_INTERNAL_HDCP_KEY);
    }

    memset(g_u8Bstatus, 0x00, sizeof(g_u8Bstatus));
    memset(g_u8KSVList, 0x00, sizeof(g_u8KSVList));
    g_bValidKSVList = FALSE;

}


//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_SetAuthStartFlag()
//  [Description]:
//                  Set hdcp14 Authentication Start flag
//  [Arguments]:
//                  [MS_BOOL] bFlag
//  [Return]:
//                  void
//
//**************************************************************************
void MDrv_HDCPTx_SetAuthStartFlag(void* pInstance, MS_BOOL bFlag)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpStartAuth_flag = bFlag;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_SetAuthDoneFlag()
//  [Description]:
//                  Set HDCP14 Authentication done flag
//  [Arguments]:
//                  [MS_BOOL] bFlag
//  [Return]:
//                  void
//
//**************************************************************************
void MDrv_HDCPTx_SetAuthDoneFlag(void* pInstance, MS_BOOL bFlag)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpAuthDone_flag = bFlag;
}

//**************************************************************************
//  [Function Name]:
//                      MDrv_HDCPTx_SetRxValid()
//  [Description]:
//                      Set Rx status
//  [Arguments]:
//                      [MS_BOOL] bIsRxValid
//  [Return]:
//                      void
//**************************************************************************
void MDrv_HDCPTx_SetRxValid(void* pInstance, MS_BOOL bIsRxValid) //wilson@kano
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHdcpTxInfo.hdcp_Rx_valid = bIsRxValid;
}

//**************************************************************************
//  [Function Name]:
//                      MDrv_HDCPTx_SetFSMState()
//  [Description]:
//                      Set hdcp14 FSM state
//  [Arguments]:
//                      [enHDCP14Tx_MainStates] enMainState
//                      [enHDCP14Tx_SubStates] enSubState
//  [Return]:
//                      void
//**************************************************************************
void MDrv_HDCPTx_SetFSMState(void* pInstance, enHDCP14Tx_MainStates enMainState, enHDCP14Tx_SubStates enSubState) //wilson@kano
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = enMainState;
    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = enSubState;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_SetUnHDCPRxCtrl()
//  [Description]:
//                  set signal control mode when hdcp function is not enabled
//  [Arguments]:
//                  [MDrvHDMITX_UNHDCPRX_CONTROL] enUnHDCPCtrl
//  [Return]:
//                  void
//
//**************************************************************************
void MDrv_HDCPTx_SetUnHDCPRxCtrl(void* pInstance, MDrvHDMITX_UNHDCPRX_CONTROL enUnHDCPCtrl)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control = enUnHDCPCtrl;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_SetHDCPRxFailCtrl()
//  [Description]:
//                  set signal control mode when hdcp function is failed
//  [Arguments]:
//                  [MDrvHDMITX_UNHDCPRX_CONTROL] enUnHDCPCtrl
//  [Return]:
//                  void
//
//**************************************************************************
void MDrv_HDCPTx_SetHDCPRxFailCtrl(void* pInstance, MDrvHDMITX_UNHDCPRX_CONTROL enUnHDCPCtrl)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPRxFail_Control = enUnHDCPCtrl;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_GetAuthStartFlag()
//  [Description]:
//                  Get hdcp14 Authentication Start flag
//  [Arguments]:
//                  void
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetAuthStartFlag(void* pInstance) //wilson@kano
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpStartAuth_flag;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDCPTx_GetAuthDoneFlag()
//  [Description]:
//                  Get HDCP14 Authentication done flag
//  [Arguments]:
//                  void
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetAuthDoneFlag(void* pInstance) //wilson@kano
{
    extern MS_BOOL MDrv_HDMITx_GetHdcpEnFlag(void*);
    extern MDrvHDMITX_FSM_STATE MDrv_HDMITx_GetFSMState(void*);

    _HDMITX_GET_VARIABLE();

    if (MDrv_HDMITx_GetHdcpEnFlag(pInstance) == TRUE)
        return psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpAuthDone_flag;
    else
        return (E_HDMITX_FSM_DONE == MDrv_HDMITx_GetFSMState(pInstance))? TRUE : FALSE;
}

//------------------------------------------------------------------------------
/// @brief This routine get HDCP key
/// @param[in] useinternalkey: TRUE -> from internal, FALSE -> from external, like SPI flash
/// @param[in] data: data point
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDCPTx_GetHdcpKey(void* pInstance, MS_BOOL useinternalkey, MS_U8 *data)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpUseInternalKey_flag = useinternalkey;
    MHal_HDMITx_GetHdcpKey(useinternalkey, data);
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetBksv()
//  [Description]:
//                      fetch BKsv
//  [Arguments]:
//			    [MS_U8] *pucData
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetBksv(void* pInstance, MS_U8 *pucData)
{
    _HDMITX_GET_VARIABLE();

    memcpy(pucData, &psHDMITXResPri->stHdcpTxInfo.HDCP_BKSV, 5*sizeof(MS_U8));

	return psHDMITXResPri->stHdcpTxInfo.HDCP_74_check;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetAksv()
//  [Description]:
//                      fetch AKsv
//  [Arguments]:
//			    [MS_U8] *pucData
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetAksv(void* pInstance, MS_U8 *pucData)
{
    _HDMITX_GET_VARIABLE();

    memcpy(pucData, &psHDMITXResPri->stHdcpTxInfo.HDCP_AKSV, 5*sizeof(MS_U8));

	return psHDMITXResPri->stHdcpTxInfo.HDCP_74_check;
}

//------------------------------------------------------------------------------
/// @brief This routine update revocation list
/// @argument:
///              - *data: data point
///              -  size: amount of revoked key (size 1 = 1*5 bytes, size 2= 2*5 bytes)
/// @return:
//------------------------------------------------------------------------------
void MDrv_HDCPTx_GetRevocationKeyList(void* pInstance, MS_U8 *data, MS_U16 size)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHdcpTxInfo.revocation_size= size;
    memcpy(&psHDMITXResPri->stHdcpTxInfo.hdcp_revocationlist[0], data, 5*size);
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDCPTx_GetPreState()
//  [Description]:
//                      return previous state of hdcp14 handler
//  [Arguments]:
//			    void
//  [Return]:
//			    enHDCP14Tx_SubStates
//
//**************************************************************************
enHDCP14Tx_SubStates MDrv_HDCPTx_GetPreState(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState;
}

//**************************************************************************
//  [Function Name]:
//                      MDrv_HDCPTx_GetRxValid()
//  [Description]:
//                      Get Rx status
//  [Arguments]:
//                      void
//  [Return]:
//                      MS_BOOL
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetRxValid(void* pInstance) //wilson@kano
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHdcpTxInfo.hdcp_Rx_valid;
}

//**************************************************************************
//  [Function Name]:
//                      MDrv_HDCPTx_GetUnHdcpControl()
//  [Description]:
//                      Get Un-Hdcp control value
//  [Arguments]:
//                      void
//  [Return]:
//                      MDrvHDMITX_UNHDCPRX_CONTROL
//**************************************************************************
MDrvHDMITX_UNHDCPRX_CONTROL MDrv_HDCPTx_GetUnHdcpControl(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDCPTx_GetKSVList()
//  [Description]:
//                      get BStatus, KSVList content for Repeater Mode
//  [Arguments]:
//                      [MS_U8*] pu8Bstatus;
//                      [MS_U8*] pu8KSVList;
//                      [MS_U16*] pu16KSVLength;
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_GetKSVList(MS_U8 *pu8Bstatus, MS_U8* pu8KSVList, MS_U16 u16BufLen, MS_U16 *pu16KSVLength)
{
    if (g_bValidKSVList == TRUE)
    {
        if ((pu8Bstatus != NULL) && (pu8KSVList != NULL) && (pu16KSVLength != NULL))
        {
            MS_U16 u16TmpLen = 0x00;

            u16TmpLen = (u16BufLen > g_u8Bstatus[0]) ? g_u8Bstatus[0] : u16BufLen;
            memcpy(pu8Bstatus, g_u8Bstatus, sizeof(g_u8Bstatus));
            memcpy(pu8KSVList, g_u8KSVList, u16TmpLen);
            *pu16KSVLength = g_u8Bstatus[0];
        }
    }

    return g_bValidKSVList;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDCPTx_CheckAuthFailFlag()
//  [Description]:
//                      return TRUE if hdcp14 authentication failed
//  [Arguments]:
//			    void
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDCPTx_CheckAuthFailFlag(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    if (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState == HDCP14Tx_SubFSM_AuthFail)
        return TRUE;
    else
        return FALSE;
}

//------------------------------------------------------------------------------
/// @brief This function return revocation check result
/// @argument:
/// @return:
///              - E_CHECK_NOT_READY
///              - E_CHECK_REVOKED
///              - E_CHECL_NOT_REVOKED
//------------------------------------------------------------------------------
MDrvHDMITX_REVOCATION_STATE MDrv_HDCPTx_RevocationKey_Check(void* pInstance)
{
    MDrvHDMITX_REVOCATION_STATE stRet;

    _HDMITX_DECLARE_VARIABE();
    _HDMITX_SEMAPHORE_ENTRY(pInstance);
    _HDMITX_GET_VARIABLE_WITHOUT_DECLARE();

    stRet = psHDMITXResPri->stHdcpTxInfo.revocation_state;

    _HDMITX_SEMAPHORE_RETURN(pInstance);

    return stRet;
}

//  *** Sample empty SRM ***
//  0x80 0x00                                             // SRM ID and reserved bit-field
//  0x00 0x01                                             // SRM version 0001
//  0x00                                                     // SRM generator no
//  0x00 0x00 0x2B                                     // VRL length (in bytes) 40+3 bytes.
//  0xD2 0x48 0x9E 0x49 0xD0 0x57 0xAE 0x31 0x5B 0x1A 0xBC 0xE0 0x0E 0x4F 0x6B 0x92    // DCP LLC signature
//  0xA6 0xBA 0x03 0x3B 0x98 0xCC 0xED 0x4A 0x97 0x8F 0x5D 0xD2 0x27 0x29 0x25 0x19
//  0xA5 0xD5 0xF0 0x5D 0x5E 0x56 0x3D 0x0E
MS_BOOL MDrv_HDCPTx_CheckRevokedKey(void* pInstance)
{
#if 1
    MS_U16 i;
    MS_U8 j, u8bksv[5];
    //MS_U32 u32keylen;
    // The revoked key from QD882: Bksv=0x23, 0xde, 0x5c, 0x43, 0x93
    MS_U8 u8Revokedksv[5] = {0x23, 0xDE, 0x5C, 0x43, 0x93};

    _HDMITX_GET_VARIABLE();

    if (!MHal_HDMITx_HDCP_Get_BKSV(&u8bksv[0]))
    {
        psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_READY;
        return FALSE;
    }
    //if(gHDMITxInfo.hdcp_srmlist[0] != 0x80) // no SRM list
    if (!psHDMITXResPri->stHdcpTxInfo.revocationlist_ready)
	{
    // HDCP CTS 1A-08
        for(i=0;i<5;i++)
        {
            if(u8bksv[i] != u8Revokedksv[i])
            {
                psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_REVOKED;
                return FALSE;
            }
        }
        DBG_HDCP(printf("MDrv_HDMITx_HDCP_CheckRevokedKey: Revoked Rx key!!\n"));
        psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_REVOKED;
        return TRUE;
    }
    else
    {
        // revoked key length
        //u32keylen = (gHDMITxInfo.hdcp_srmlist[5]<<16) | (gHDMITxInfo.hdcp_srmlist[6]<<8) | gHDMITxInfo.hdcp_srmlist[7];
        //u32keylen = (u32keylen >= 43) ? u32keylen-43 : 0; // 3 bytes is VRL length and 40 bytes is DCP LLC signature
        //if((u32keylen == 0) || (u32keylen%5 != 0)) // keylen is 0 or not 5 times
        if (psHDMITXResPri->stHdcpTxInfo.revocation_size == 0) // keylen is 0 or not 5 times
        {
            psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_REVOKED;
            return FALSE;
        }
        else
        {
            for(i=0 ; i<psHDMITXResPri->stHdcpTxInfo.revocation_size ; i++)
            {
                for(j=0 ; j<5 ; j++)
                {
                    if(psHDMITXResPri->stHdcpTxInfo.hdcp_revocationlist[5*i+j] != u8bksv[j])
                    break;
                }
                if(j==5) // Bksv is matched the SRM revoked keys
                {
                    psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_REVOKED;
                    return TRUE;
                }
            }
        }
    }
#endif
    psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_REVOKED;
    return FALSE;
}

//------------------------------------------------------------------------------
/// @brief This routine check whether SRM DSA signauter is valid or not
/// @argument:
///              - *data: data point
///              -   size: size of SRM list(bytes)
/// @return:
///              Ture: valid, FALSE: invalid
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDCPTx_IsSRMSignatureValid(MS_U8 *data, MS_U32 size)
{
#if 0
    MS_U8 Msg[8]; // message, hex format
    MS_U8 u8rtemp[20], u8stemp[20]; // DSA signature, hex format
    char ms[50];
    MS_U32 u32DSASigIdx;
    unsigned int SHA1[5];
    MS_S8 result;
    BIGD   m, r, s;
    dsa_context dsa;

    // ps, qs, gs is from HDCP specification v1.3
    // msg[] Source message (will be hashed by SHA-1)
    // ps[] (p) Prime Modulus, 1024 bits
    // qs[] (q) Prime Divisor, 160 bits
    // gs[] (g) Generator, 1024 bits
    // ys[] (y) Public key, 1024 bits
    char ps[] = "d3c3f5b2 fd1761b7 018d75f7 9343786b\
                17395b35 5a52c7b8 a1a24fc3 6a7058ff\
                8e7fa164 f500e0dc a0d28482 1d969e4b\
                4f34dc0c ae7c7667 b844c747 d4c6b983\
                e52ba70e 5447cf35 f404a0bc d1974c3a\
                10715509 b3721530 a73f3207 b9982049\
                5c7b9c14 3275733b 028a49fd 96891954\
                2a39951c 46edc211 8c59802b f3287527";
    char qs[] = "ee8af2ce 5e6db56a cd6d14e2 97ef3f4d f9c708e7";
    char gs[] = "92f85d1b 6a4d5213 1ae43e24 45de1ab5\
                02afdeac a9bed731 5d56d766 cd278611\
                8f5db14a bdeca9d2 5162977d a83effa8\
                8eedc6bf eb37e1a9 0e29cd0c a03d799e\
                92dd2945 f778585f f7c83564 2c21ba7f\
                b1a0b6be 81c8a5e3 c8ab69b2 1da54242\
                c98e9b8a ab4a9dc2 51fa7dac 29216fe8\
                b93f185b 2f67405b 69462442 c2ba0bd9";
    char ys[] = "c7060052 6ba0b086 3a80fbe0 a3acff0d\
                4f0d7665 8a1754a8 e7654755 f15ba78d\
                56950e48 654f0bbd e16804de 1b541874\
                db22e14f 031704db 8d5cb2a4 17c4566c\
                27ba973c 43d84e0d a2a70856 fe9ea48d\
                87259038 b16553e6 62435ff7 fd5206e2\
                7bb7ffbd 886c2410 95c8dc8d 66f662cb\
                d88f9df7 e9b3fb83 62a9f7fa 36e53799";

    m = bdNew();
    r = bdNew();
    s = bdNew();
    memcpy(&gHDMITxInfo.hdcp_srmlist[0], data, size);

    u32DSASigIdx = (gHDMITxInfo.hdcp_srmlist[5]<<16) | (gHDMITxInfo.hdcp_srmlist[6]<<8) | gHDMITxInfo.hdcp_srmlist[7];
    if(u32DSASigIdx < 43) // illegal length
        return FALSE;
    else
        u32DSASigIdx -= 43;

    memcpy(&Msg[0], &(gHDMITxInfo.hdcp_srmlist[0]), 8*sizeof(MS_U8)); // Message: The first 64 bits of SRM list
    memcpy(&u8rtemp[0], &(gHDMITxInfo.hdcp_srmlist[8+u32DSASigIdx]), 20*sizeof(MS_U8)); // r: the first 160 bits of DSA signature
    memcpy(&u8stemp[0], &(gHDMITxInfo.hdcp_srmlist[8+20+u32DSASigIdx]), 20*sizeof(MS_U8)); // s: the trailing 160 bits of DSA signature
    MHal_HDMITx_HDCP_SHA1_Transform(SHA1, Msg); // Msg: 64 bits -> 512bits, SHA-1(Msg)
    snprintf(ms, sizeof(ms), "%08x%08x%08x%08x%08x",SHA1[0],SHA1[1],SHA1[2],SHA1[3],SHA1[4]);
    //sprintf(ms,"%08x%08x%08x%08x%08x",SHA1[0],SHA1[1],SHA1[2],SHA1[3],SHA1[4]);

    _dsa_init(&dsa);

    bdConvFromHex(dsa.p, ps);
    bdConvFromHex(dsa.q, qs);
    bdConvFromHex(dsa.g, gs);
    bdConvFromHex(dsa.y, ys);
    bdConvFromHex(m,ms);
    bdConvFromOctets(r, u8rtemp, 20);
    bdConvFromOctets(s, u8stemp, 20);

    printf("@@SRM-SHA-1 Digest= ");
    bdPrint(m, BD_PRINT_TRIM | BD_PRINT_NL);
    printf("@@SRM-R= ");
    bdPrint(r, BD_PRINT_TRIM | BD_PRINT_NL);

    result = _dsa_verify(&dsa, m, r, s);

    printf("@@SRM- Verify result= ");
    result == 0 ? printf("PASS!\r\n") : printf("FAIL!\r\n");

    bdFree(&m);
    bdFree(&r);
    bdFree(&s);

    _dsa_clear(&dsa);

    if(result == 0)
        return TRUE;
    else
        return FALSE;
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
/// @brief This routine start/stop HDCP authentication
/// @param[in] bFlag: TRUE -> start authentication, FALSE -> stop authentication
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDCPTx_StartAuth(void* pInstance, MS_BOOL bFlag)
{
    extern void MDrv_HDMITx_SetFSMState(void*, MDrvHDMITX_FSM_STATE); //avoid compile error due to presented order of functions

    _HDMITX_GET_VARIABLE();

    MDrv_HDMITx_SetHDCPFlag(pInstance, bFlag);

    if (bFlag)
    {
        MDrv_HDCPTx_SetAuthStartFlag(pInstance, TRUE);
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
        psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_RESET;

        MDrv_HDCPTx_SetFSMState(pInstance, HDCP14Tx_MainState_A0, HDCP14Tx_SubFSM_IDLE);

        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
    }
    else
    {
        MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE);
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
        psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_RESET;

        if (MDrv_HDMITx_GetEdidRdyFlag(pInstance) == TRUE)
            MDrv_HDMITx_SetFSMState(pInstance, E_HDMITX_FSM_TRANSMIT);
        else
            MDrv_HDMITx_SetFSMState(pInstance, E_HDMITX_FSM_VALIDATE_EDID);

        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);

        _MDrv_HDCPTx_StopRiTimer();
    }
}

//**************************************************************************
//  [Function Name]:
//			    HDCPTx_Handler()
//  [Description]:
//                      main handler of hdcp1.4 process
//  [Arguments]:
//			    [MS_U32] u32State
//  [Return]:
//			    void
//
//**************************************************************************
void HDCPTx_Handler(void* pInstance, MS_U32 u32State) //wilson@kano
{
    static MS_BOOL bPolling = FALSE;
    static MS_BOOL bCheckDone = FALSE;

    extern MS_BOOL MDrv_HDMITx_GetTmdsEnFlag(void* ); //avoid compile error due to presented order of functions
    extern MS_BOOL MDrv_HDMITx_GetHdcpEnFlag(void* );
    extern MS_BOOL MDrv_HDMITx_GetAVMUTEStatus(void* );
    extern void MDrv_HDMITx_SetAVMUTE(void* , MS_BOOL);

    _HDMITX_GET_VARIABLE();

    if(gbCurRxStatus != E_HDMITX_DVIClock_H_HPD_H || MDrv_HDMITx_GetTmdsEnFlag(pInstance) == FALSE)
        return;

    if (MDrv_HDCPTx_GetRxValid(pInstance) == FALSE)
    {
        MHal_HDMITx_HdcpSetEncrypt(DISABLE);
        MDrv_HDCPTx_SetFSMState(pInstance, HDCP14Tx_MainState_A0, HDCP14Tx_SubFSM_IDLE);

        bPolling = FALSE;
        bCheckDone = FALSE;

        if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_HDCP_ENCRYPTION)
        {
            MHal_HDMITx_HdcpSetEncrypt(ENABLE); // for HDCP CTS
        }
        else if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_BLUE_SCREEN)
        {
            _MDrv_HDCPTx_StopRiTimer();

            MDrv_HDMITx_SetVideoOnOff(pInstance, FALSE); //wilson@kano
            MDrv_HDMITx_SetAudioOnOff(pInstance, FALSE); //wilson@kano
        }
        else //normal display
        {
            _MDrv_HDCPTx_StopRiTimer();
            MHal_HDMITx_HdcpSetEncrypt(DISABLE); // for HDCP CTS
            MDrv_HDMITx_SetVideoOnOff(pInstance, TRUE); //wilson@kano
            MDrv_HDMITx_SetAudioOnOff(pInstance, TRUE);
        }

        //return;
    }

    if (MDrv_HDMITx_GetTmdsEnFlag(pInstance) == TRUE)//((gHdcpTxInfo.hdcp_Rx_valid == TRUE) && (MDrv_HDMITx_GetTmdsEnFlag() == TRUE)) //edid is ready & tmds signal is enabled
    {
        if (MDrv_HDMITx_GetHdcpEnFlag(pInstance) == TRUE) //(gHdcpTxInfo.hdmitx_hdcp_flag == TRUE)
        {
            switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState)
        	{
        		case HDCP14Tx_MainState_A0:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
                        case HDCP14Tx_SubFSM_AuthFail:
                        {
                            if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_HDCP_ENCRYPTION)
                            {
                                MHal_HDMITx_HdcpSetEncrypt(ENABLE); // for HDCP CTS
                            }
                            else if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_BLUE_SCREEN)
                            {
                                _MDrv_HDCPTx_StopRiTimer();

                                MDrv_HDMITx_SetVideoOnOff(pInstance, FALSE); //wilson@kano
                                MDrv_HDMITx_SetAudioOnOff(pInstance, FALSE); //wilson@kano
                            }
                            else //normal display
                            {
                                _MDrv_HDCPTx_StopRiTimer();
                                MHal_HDMITx_HdcpSetEncrypt(DISABLE); // for HDCP CTS
                                MDrv_HDMITx_SetVideoOnOff(pInstance, TRUE); //wilson@kano
                                MDrv_HDMITx_SetAudioOnOff(pInstance, TRUE);
                            }

                            if (MDrv_HDMITx_GetHdcpEnFlag(pInstance) == TRUE) //(gHdcpTxInfo.hdmitx_hdcp_flag == TRUE) //if hdcp is enabled, keep re-try defalut state
                            {
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                            }

                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;//HDCP14Tx_SubFSM_PreExchangeKSV;
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_AuthFail;
                        }
                        break;

        				case HDCP14Tx_SubFSM_IDLE:
                            {
                                MS_U16 wBstatus = 0;

                                memset(g_u8Bstatus, 0x00, sizeof(g_u8Bstatus));
                                memset(g_u8KSVList, 0x00, sizeof(g_u8KSVList));
                                g_bValidKSVList = FALSE;

                                if (MDrv_HDMITx_GetOutputMode(pInstance) & 0x02) //if HDMI mode
                                {
                                    if ((bCheckDone == FALSE) && (_MDrv_HDCP_GetTimeDiff(gdwPreTime, gdwCurTime) < HDCP14TX_BSTATUS_TIMEOUT_CNT))
                                    {
                                        if (bPolling == FALSE)
                                            gdwPreTime = DrvHdmitxOsGetSystemTime(); //first time

                                        bPolling = TRUE;
                                        gdwCurTime = DrvHdmitxOsGetSystemTime();

                                        if (MHal_HDMITx_HDCP_Get_BStatus(&wBstatus) == TRUE)
                                        {
                                            psHDMITXResPri->stHdcpTxInfo.ucBStatus[1] = (MS_U8)((wBstatus & 0xFF00)>>8);
                                            psHDMITXResPri->stHdcpTxInfo.ucBStatus[0] = (MS_U8)(wBstatus & 0x00FF);

                                            if (wBstatus & BIT12)
                                            {
                                                bPolling = FALSE;
                                                bCheckDone = TRUE;
                                                //break;
                                            }
                                            else
                                            {
                                                bPolling = TRUE;
                                                bCheckDone = FALSE;
                                                DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
                                                return;
                                            }
                                        }
                                    }
                                }

                                gRepeaterFailTime = 0;
                                gRepeaterStartTime = DrvHdmitxOsGetSystemTime();

                                psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_WAITING_ACTIVE_RX;

                                MHal_HDMITx_HdcpSetEncrypt(DISABLE);

            					psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = _MDrv_HDCPTx_AuthWaitingActiveRx();

                                if (psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status == E_HDMITX_HDCP_RX_IS_NOT_VALID)
                                {
                                    MDrv_HDCPTx_SetRxValid(pInstance, FALSE);

                                    if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_HDCP_ENCRYPTION)
                                    {
                                        MHal_HDMITx_HdcpSetEncrypt(ENABLE); // for HDCP CTS
                                    }
                                    else if (psHDMITXResPri->stHdcpTxInfo.hdmitx_unHDCPRx_Control == E_BLUE_SCREEN)
                                    {
                                        _MDrv_HDCPTx_StopRiTimer();

                                        MDrv_HDMITx_SetVideoOnOff(pInstance, FALSE); //wilson@kano
                                        MDrv_HDMITx_SetAudioOnOff(pInstance, FALSE); //wilson@kano
                                    }
                                    else //normal display
                                    {
                                        _MDrv_HDCPTx_StopRiTimer();
                                        MHal_HDMITx_HdcpSetEncrypt(DISABLE); // for HDCP CTS
                                        MDrv_HDMITx_SetVideoOnOff(pInstance, TRUE); //wilson@kano
                                        MDrv_HDMITx_SetAudioOnOff(pInstance, TRUE);
                                    }

                                    if (MDrv_HDMITx_GetHdcpEnFlag(pInstance) == TRUE) //(gHdcpTxInfo.hdmitx_hdcp_flag == TRUE) //if hdcp is enabled, keep re-try defalut state
                                    {
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                                    }

                                    MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
                                }
                                else
                                {
                                    MDrv_HDCPTx_SetRxValid(pInstance, TRUE);

                                    //move to next state
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_ExchangeKSV;//HDCP14Tx_SubFSM_PreExchangeKSV;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_IDLE;
                                    gdwPreTime = DrvHdmitxOsGetSystemTime();
                                    //DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
                                }
                            }
        				break;
        				case HDCP14Tx_SubFSM_ExchangeKSV:
                            {
            					//send an and aksv, if failed then go back to state A0
            					//if (hal_HDCP14Tx_SendAn(gHdcpTxInfo.bUseInternalAn) && hal_HDCP14Tx_SendAKSV())
            					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_ExchangeKSV;

            					if (MHal_HDMITx_HdcpWriteAn(USE_AUTO_GEN_AN) == TRUE)
            					{
            					    DrvHdmitxOsMsSleep(HDCP_DDC_SAMPLE_PERIOD);

            					    if (MHal_HDMITx_HdcpWriteAksv() == TRUE)
                                    {
                                        gdwPreTime = DrvHdmitxOsGetSystemTime();//MAsm_GetSystemTime();
                						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A1andA2;
                						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_VerifyBksv;//HDCP14Tx_SubFSM_IDLE;
                                    }
                                    else
                                    {
                                        psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_TX_KEY_FAIL;
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
                                        DBG_HDCP(printf("%s::MHal_HDMITx_HdcpWriteAksv fail!! \n", __FUNCTION__));
                                    }
            					}
            					else
            					{
            					    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_TX_KEY_FAIL;

            						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
            						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
            						DBG_HDCP(printf("%s::Exchange KSV fail!! \n", __FUNCTION__));
            					}
                            }
        				break;

        				default:
        					DBG_HDCP(printf("[HDCP14TX]%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}

                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		case HDCP14Tx_MainState_A1andA2:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        				break;

        				case HDCP14Tx_SubFSM_VerifyBksv:
        					//read BCaps, set Ainfo(optional)
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_VerifyBksv;

        					if(_MDrv_HDCPTx_HdcpSyncSetting(pInstance, MDrv_HDMITx_GetOutputMode(pInstance)) == FALSE)
                            {
                                DBG_HDCP(printf("%s::I2C Read BCaps ERROR!\n", __FUNCTION__));

        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail; //HDCP14Tx_SubFSM_IDLE;
                                psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_TX_KEY_FAIL;
                                psHDMITXResPri->stHdcpTxInfo.HDCP_74_check = FALSE;

                                break;
                            }
        					else
        					{
        					    if (MHal_HDMITx_HdcpCheckBksvLn() == TRUE)
                                {
                                    MHal_HDMITx_GET74(0x00, psHDMITXResPri->stHdcpTxInfo.HDCP_BKSV);
                                    MHal_HDMITx_GET74(0x10, psHDMITXResPri->stHdcpTxInfo.HDCP_AKSV);
                                    psHDMITXResPri->stHdcpTxInfo.HDCP_74_check = TRUE;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A3;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                                }
                                else
                                {
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail; //HDCP14Tx_SubFSM_IDLE;
                                    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_TX_KEY_FAIL;
                                    psHDMITXResPri->stHdcpTxInfo.HDCP_74_check = FALSE;
                                }
        					}
        				break;

        				default:
        					DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}
                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		case HDCP14Tx_MainState_A3:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        					//check revoke list
        					if (MDrv_HDCPTx_CheckRevokedKey(pInstance) == TRUE)
        					{
        						DBG_HDCP(printf("%s::Revoke BKSV !!\n", __FUNCTION__));
                                psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_RX_KEY_REVOKED;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail; //HDCP14Tx_SubFSM_IDLE;
                                MDrv_HDCPTx_SetRxValid(pInstance, FALSE);
                                break;
        					}
        					else
        					{
        						MHal_HDMITx_HdcpStartCipher();
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A3;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_CheckR0;
        						//gdwCurTime = _MDrv_HDCP_GetTimeDiff();
        						//gdwPreTime = gdwCurTime;
        					}
        				break;

        				case HDCP14Tx_SubFSM_CheckR0:
        					//validate receiver
        					/**************************************************************************/
        					// [HDCP] 1A and 1B: AKSV -> Ro should be large than 100msec
        					// to pass Quantumdata 882 HDCP test, we delay 150ms
        					//
        					// 2013/11/07, in SEC Compatibility test, we meet an Repeater timeout error on PEPEATER YAMAHA RX-V2700
        					// patch AKSV -> Ro large than 250ms
        					/***************************************************************************/

        					//wait 100ms(at least) before check R0
        					#if 1
                            gdwCurTime = DrvHdmitxOsGetSystemTime();
        					if (_MDrv_HDCP_GetTimeDiff(gdwPreTime, gdwCurTime) < HDCP14TX_R0_WDT)
        					{
            						//gdwPreTime = gdwCurTime;
        						break;
        					}
                            gdwPreTime = gdwCurTime;
                            #else
                            if (u32State & E_HdcpTX_EVENT_RITIMER)
                            #endif
                            {
            					if (_MDrv_HDCPTX_AuthCheckRi() == E_HDMITX_HDCP_SYNC_RI_PASS) //R0 = R0'
            					{
            						if (psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpCheckRepeater_flag == TRUE)
            						{
            							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A6;
            							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_CheckR0;
            						}
            						else
            						{
            							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A4;
            							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                                        psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_SYNC_RI_PASS;
                                        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_CheckR0;
            						}
            					}
            					else //E_HDMITX_HDCP_SYNC_RI_FAIL
            					{
            						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
            						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
                                    psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_SYNC_RI_FAIL;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_CheckR0;
            					}
                            }

        				break;

        				default:
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
        					DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}
                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		case HDCP14Tx_MainState_A4:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A4;
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthDone;
                            DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        				break;

        				case HDCP14Tx_SubFSM_AuthDone:
                            MDrv_HDMITx_SetVideoOnOff(pInstance, TRUE);
                            MDrv_HDMITx_SetAudioOnOff(pInstance, TRUE);

                            if (MDrv_HDMITx_GetAVMUTEStatus(pInstance) == TRUE)
                            {
                                MDrv_HDMITx_SetAVMUTE(pInstance, FALSE);
                            }

        					//authentication done;
        					MHal_HDMITx_HdcpAuthPass();//hal_HDCP14Tx_SetAuthDone(TRUE);
                            MDrv_HDCPTx_SetAuthDoneFlag(pInstance, TRUE);
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A4;
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_CheckRi;
                            psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_AUTH_DONE;
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_AuthDone;

                            if (_s32HDCPRiTimerId > 0) //already created
                            {
                                DrvHdmitxOsDeleteTimer(&_tHDCPRiTimerCfg);
                                _s32HDCPRiTimerId = -1;
                            }

                            _tHDCPRiTimerCfg.s32Id = -1;
                            _tHDCPRiTimerCfg.u32TimerOut = gHDCPCheckRiTimer;
                            _tHDCPRiTimerCfg.pfnFunc = _MDrv_HDMITX_HDCP_Ri_TimerCallback;


                            if(DrvHdmitxOsCreateTimer(&_tHDCPRiTimerCfg))
                            {
                                _s32HDCPRiTimerId = _tHDCPRiTimerCfg.s32Id;

                            }
                            else
                            {
                                _s32HDCPRiTimerId = -1;
                            }

        					//gdwCurTime = _MDrv_HDCP_GetTimeDiff();
        					//gdwPreTime = gdwCurTime;
        				break;

        				case HDCP14Tx_SubFSM_CheckRi:
        					//gdwCurTime = _MDrv_HDCP_GetTimeDiff();

        					if (u32State & E_HdcpTX_EVENT_RITIMER)//if (_MDrv_HDCP_GetTimeDiff(gdwPreTime, gdwCurTime) > HDCP14TX_CHECK_RI_WDT)
        					{
        						if (_MDrv_HDCPTX_AuthCheckRi() == E_HDMITX_HDCP_SYNC_RI_FAIL) //if (hal_HDCP14Tx_CompareRi() == FALSE) //Ri failed
        						{
        							DBG_HDCP(printf("%s::Check Ri failed! %d\n", __FUNCTION__, (unsigned int)gdwCurTime));

        							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
        							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
        							MHal_HDMITx_HdcpSetEncrypt(DISABLE);//hal_HDCP14Tx_EnableEncryption(FALSE);
        							psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_SYNC_RI_FAIL;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_CheckRi;
        						}
        						//gdwPreTime = gdwCurTime;
        					}
        				break;

        				default:
        				break;
        			}
        		break;

        		case HDCP14Tx_MainState_A5:
        		break;

        		case HDCP14Tx_MainState_A6:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        					//gHdcpTxInfo.bRepeaterMode = (gHdcpTxInfo.ucBCaps & 0x40) ? TRUE : FALSE; //BCaps: b'[6] downstream is repeater
        					if (psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpCheckRepeater_flag == TRUE) //if (gHdcpTxInfo.bRepeaterMode == TRUE)
        					{
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A8;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
        					}
        					else
        					{
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A4;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
        					}
        				break;
        				default:
        					DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}
                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		case HDCP14Tx_MainState_A7:
        			DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        		break;

        		case HDCP14Tx_MainState_A8:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        					//TBD: set up 5 sec WDT and polling Rx ready bit
                            gdwPreTime = DrvHdmitxOsGetSystemTime();
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A8;
                            psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_PollingRdyBit;
        				break;

        				case HDCP14Tx_SubFSM_PollingRdyBit:
        					//TBD: if ready then go A9; else go to A0
                            gdwCurTime = DrvHdmitxOsGetSystemTime();
        					if (_MDrv_HDCP_GetTimeDiff(gdwPreTime, gdwCurTime) < HDCP14TX_REP_RDY_BIT_WDT)
        					{
        						if (_MDrv_HDCPTx_AuthCheckRepeaterReady() == E_HDMITX_HDCP_REPEATER_NOT_READY)
        						{
        							//gdwPreTime = gdwCurTime;
        							break;
        						}
        						else
        						{
        							//go to state A9 and start check KSV list
        							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A9;
        							psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthWithRepeater;//HDCP14Tx_SubFSM_IDLE;
                                    psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_PollingRdyBit;
        						}
        					}
        					else //timeout
        					{
        						DBG_HDCP(printf("%s::Polling Repeater Ready Bit Timeout !!\n", __FUNCTION__));
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
                                psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_REPEATER_TIMEOUT;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_PollingRdyBit;
        					}
        				break;

        				default:
        					DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}
                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		case HDCP14Tx_MainState_A9:
        			switch (psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState)
        			{
        				case HDCP14Tx_SubFSM_IDLE:
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A9;
        					psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthWithRepeater;
        				break;

        				case HDCP14Tx_SubFSM_AuthWithRepeater:
        					if (_MDrv_HDCPTx_CheckRepeater() == E_HDMITX_HDCP_REPEATER_SHA1_PASS)//if (hal_HDCP14Tx_AuthWithRepeater())
        					{
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A4;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_AuthWithRepeater;
        					}
        					else
        					{
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
        						psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_AuthFail;
                                psHDMITXResPri->stHdcpTxInfo.hdcp14tx_PreSubState = HDCP14Tx_SubFSM_AuthWithRepeater;
        					}
        				break;

        				default:
        					DBG_HDCP(printf("%s::Invalid State!\n", __FUNCTION__));
        				break;
        			}
                    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);
        		break;

        		default:
        		break;
        	}

            //DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN);

        } //gHdcpTxInfo.hdmitx_hdcp_flag == TRUE;
        else
        {
            MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
            psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_AUTH_FAIL;
        }
    } //((gHdcpTxInfo.hdcp_Rx_valid == TRUE) && (MDrv_HDMITx_GetTmdsEnFlag() == TRUE))
    else
    {
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState = HDCP14Tx_MainState_A0;
        psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState = HDCP14Tx_SubFSM_IDLE;
        psHDMITXResPri->stHdcpTxInfo.hdmitx_HDCPAuth_Status = E_HDMITX_HDCP_RX_IS_NOT_VALID;
        psHDMITXResPri->stHdcpTxInfo.revocation_state = E_CHECK_NOT_READY;
        psHDMITXResPri->stHdcpTxInfo.HDCP_74_check = FALSE;
    }
}

//**************************************************************************
//  [Function Name]:
//			()
//  [Description]:
//
//  [Arguments]:
//
//  [Return]:
//
//
//**************************************************************************
#if 0

static void _HDCPTx_Task(void) //wilson@kano
{
    MS_U32 u32Events = 0;

    while (g_bHdcpTxTask == TRUE)
    {
        DrvHdmitxOsWaitEvent(_s32HdcpTxEventId, E_HdcpTX_EVENT_RUN | E_HdcpTX_EVENT_IRQ | E_HdcpTX_EVENT_RITIMER, &u32Events, E_DRV_HDMITX_OS_EVENT_MD_OR_CLEAR, 5000);

        if (gHdcpTxInfo.hdcp_Rx_valid == TRUE)
            HDCPTx_Handler(u32Events);

        DrvHdmitxOsMsSleep(HDCPTX_MONITOR_DELAY);
    } // Task loop
}

#endif

//------------------------------------------------------------------------------
/// @brief This routine initializes HDCP Tx module
/// @return None
/// HDCP driver is in @ref E_TASK_PRI_HIGH level
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDCPTx_Init(void *pInstance) //wilson@kano
{
    #if 1
    MDrv_HDCPTx_InitVariable(pInstance);
    #else
    _hdcptx_status.bIsInitialized = TRUE;
    _hdcptx_status.bIsRunning     = TRUE;

    if (_s32HdcpTxTaskId >= 0)
    {
        DBG_HDMITX(printf("MDrv_HdcpTx_Init: already initialized.\n"));
        return TRUE;
    }

    DBG_HDMITX(printf("%s\n", __FUNCTION__));

    MDrv_HDCPTx_InitVariable();

    _s32HdcpTxEventId = DrvHdmitxOsCreateEventGroup("HDCPTx_Event");
    if (_s32HdcpTxEventId < 0)
    {
        ///MsOS_DeleteMutex(_s32HDMITxMutexId);
        return FALSE;
    }

    _pHdcpTxTaskStack = _u8HdcpTx_StackBuffer;
    g_bHdcpTxTask = TRUE;
    _s32HdcpTxTaskId = MsOS_CreateTask((TaskEntry)_HDCPTx_Task,
                                    (MS_U32)NULL,
                                    E_TASK_PRI_HIGH,
                                    TRUE,
                                    _pHdcpTxTaskStack,
                                    HDCPTX_TASK_STACK_SIZE,
                                    "HDCPTx_Task");
    if (_s32HdcpTxTaskId < 0)
    {
        DrvHdmitxOsDeleteEventGroup(_s32HdcpTxEventId);
        printf("create hdmi task failed\r\n");
        ///MsOS_DeleteMutex(_s32HDMITxMutexId);
        return FALSE;
    }
    #endif

    return TRUE;
}


/*********************************************************************/
/*                                                                                                                     */
/*                               Sorting HDMI Tx function here                                           */
/*                                                                                                                     */
/*********************************************************************/
void ____HDMITx_Relative_Func____(void){} //Null function for navigation

//-------------------------------------------------------------------------------------------------
// Check Rx timer callback
// @param  stTimer \b IN: Useless
// @param  u32Data \b IN: Useless
// @return None
//-------------------------------------------------------------------------------------------------
static void _MDrv_HDMITX_CheckRx_TimerCallback(MS_U32 stTimer, MS_U32 u32Data)
{
    // Check HDMI receiver status

    gbCurRxStatus = MHal_HDMITx_GetRXStatus();

    if(g_bHpdIrqEn)
    {
        if(gbCurRxStatus == E_HDMITX_DVIClock_L_HPD_L || gbCurRxStatus == E_HDMITX_DVIClock_H_HPD_L)
        {
            DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RXTIMER);
        }
    }
    else
    {
        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RXTIMER);
    }
}

void _MDrv_HDMITX_InitEdidField(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    //edid relative //assign initial value
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk0, 0x00, HDMITX_EDID_BLK_SIZE*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1, 0x00, HDMITX_EDID_BLK_SIZE*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID, 0x00, 3*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen, 0x00, HDMITX_CEA_DAT_BLK_TYPE_NUM*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr, 0x00, 2*sizeof(MS_U8));
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi = FALSE;

	//HF-VSDB, scdc relative
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDBVerInfo = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportIndependView = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportDualView = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3DOsdDisparity = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportLTEScramble = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC = FALSE;

    //read request
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_50Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_60Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_50Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_60Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_NoID;

    // data block content
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudioDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VendorDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SpeakerAllocDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VESA_DTCDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList, 0x00, 7*sizeof(MS_U8)); //this field has only 3 bits
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming, 0x00, 32*sizeof(MS_U8));

    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB, 0x00, 32*sizeof(MS_U8));
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB_Len = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB_Len = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3dMultiPresent = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Hdmi3DLen = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen = 0x00;

    //for 420
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support420ColorFmt = FALSE;
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk, 0x00, 32*sizeof(MS_U8));

    //for colorimetry
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ExtColorimetry = 0x00;

    //for HDR
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.bSupportHDR = FALSE;
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDRStaticDataBlk, 0x00, 32*sizeof(MS_U8));

    // VCDB
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoCapDataBlk = 0x00;
}

/**************************************************************************/
//  [Function Name]:
//                      _MDrv_HDMIRx_MapVideoTimingToVIC()
//  [Description]:
//                      mapping timing table to VIC code
//  [Arguments]:
//                      [MsHDMITX_VIDEO_TIMING] timing
//  [Return]:
//                      MsHDMITX_AVI_VIC
//**************************************************************************/
static MsHDMITX_AVI_VIC _MDrv_HDMIRx_MapVideoTimingToVIC(MsHDMITX_VIDEO_TIMING timing)
{
    //msdbg Todo: 6.6G report 3840x2160_60P VIc

    switch(timing)
    {
        case E_HDMITX_RES_640x480p:
            return E_HDMITX_VIC_640x480p_60_4_3;

        case E_HDMITX_RES_720x480i:
            return E_HDMITX_VIC_720x480i_60_16_9;

        case E_HDMITX_RES_720x576i:
            return E_HDMITX_VIC_720x576i_50_16_9;

        case E_HDMITX_RES_720x480p:
            return E_HDMITX_VIC_720x480p_60_16_9;

        case E_HDMITX_RES_720x576p:
            return E_HDMITX_VIC_720x576p_50_16_9;

        case E_HDMITX_RES_1280x720p_50Hz:
            return E_HDMITX_VIC_1280x720p_50_16_9;

        case E_HDMITX_RES_1280x720p_60Hz:
            return E_HDMITX_VIC_1280x720p_60_16_9;

        case E_HDMITX_RES_1920x1080i_50Hz:
            return E_HDMITX_VIC_1920x1080i_50_16_9;

        case E_HDMITX_RES_1920x1080i_60Hz:
            return E_HDMITX_VIC_1920x1080i_60_16_9;

        case E_HDMITX_RES_1920x1080p_24Hz:
            return E_HDMITX_VIC_1920x1080p_24_16_9;

        case E_HDMITX_RES_1920x1080p_25Hz:
            return E_HDMITX_VIC_1920x1080p_25_16_9;

        case E_HDMITX_RES_1920x1080p_30Hz:
            return E_HDMITX_VIC_1920x1080p_30_16_9;

        case E_HDMITX_RES_1920x1080p_50Hz:
            return E_HDMITX_VIC_1920x1080p_50_16_9;

        case E_HDMITX_RES_1920x1080p_60Hz:
            return E_HDMITX_VIC_1920x1080p_60_16_9;

        case E_HDMITX_RES_1920x2205p_24Hz: // for 3D timing
            return E_HDMITX_VIC_1920x1080p_24_16_9;

        case E_HDMITX_RES_1280x1470p_50Hz: // for 3D timing
            return E_HDMITX_VIC_1280x720p_50_16_9;

        case E_HDMITX_RES_1280x1470p_60Hz: // for 3D timing
            return E_HDMITX_VIC_1280x720p_60_16_9;

        case E_HDMITX_RES_3840x2160p_24Hz:
            return E_HDMITX_VIC_3840x2160p_24_16_9;

        case E_HDMITX_RES_3840x2160p_25Hz:
            return E_HDMITX_VIC_3840x2160p_25_16_9;

        case E_HDMITX_RES_3840x2160p_30Hz:
            return E_HDMITX_VIC_3840x2160p_30_16_9;

        case E_HDMITX_RES_3840x2160p_50Hz:
            return E_HDMITX_VIC_3840x2160p_50_16_9;

        case E_HDMITX_RES_3840x2160p_60Hz:
            return E_HDMITX_VIC_3840x2160p_60_16_9;

        case E_HDMITX_RES_4096x2160p_24Hz:
            return E_HDMITX_VIC_4096x2160p_24_256_135;

        case E_HDMITX_RES_4096x2160p_25Hz:
            return E_HDMITX_VIC_4096x2160p_25_256_135;

        case E_HDMITX_RES_4096x2160p_30Hz:
            return E_HDMITX_VIC_4096x2160p_30_256_135;

        case E_HDMITX_RES_4096x2160p_50Hz:
            return E_HDMITX_VIC_4096x2160p_50_256_135;

        case E_HDMITX_RES_4096x2160p_60Hz:
            return E_HDMITX_VIC_4096x2160p_60_256_135;

        case E_HDMITX_RES_1280x720p_24Hz:
            return E_HDMITX_VIC_1280x720p_24_16_9;

        case E_HDMITX_RES_1280x720p_25Hz:
            return E_HDMITX_VIC_1280x720p_25_16_9;

        case E_HDMITX_RES_1280x720p_30Hz:
            return E_HDMITX_VIC_1280x720p_30_16_9;

        default:
            printf("[HDMITx]: Error video timing: %d\n", timing);
            return E_HDMITX_VIC_NOT_AVAILABLE;
    }
}

/**************************************************************************/
//  [Function Name]:
//                      _MDrv_HDMIRx_VideoFormat()
//  [Description]:
//                      CE Video Format or IT Video Format
//  [Arguments]:
//                      [MsHDMITX_VIDEO_TIMING] timing
//  [Return]:
//                      MsHDMITX_VIDEO_FORMAT
//**************************************************************************/
MsHDMITX_VIDEO_FORMAT _MDrv_HDMIRx_VideoFormat(MsHDMITX_VIDEO_TIMING idx)
{
    MsHDMITX_VIDEO_FORMAT eFmt = E_HDMITX_VIDEO_FORMAT_CE;

    if((_MDrv_HDMIRx_MapVideoTimingToVIC(idx) == E_HDMITX_VIC_640x480p_60_4_3)||
        (_MDrv_HDMIRx_MapVideoTimingToVIC(idx) == E_HDMITX_VIC_NOT_AVAILABLE))
    {
        eFmt = E_HDMITX_VIDEO_FORMAT_IT;
    }

    return eFmt;
}

//**************************************************************************/
//  [Function Name]:
//                      _MDrv_HDMITx_ParseDeepColorInfo()
//  [Description]:
//                      mapping EDID field to color depth information
//  [Arguments]:
//                      [MS_U8] u8DataByte
//  [Return]:
//                      void
//**************************************************************************/
void _MDrv_HDMITx_ParseDeepColorInfo(void* pInstance, MS_U8 u8DataByte)
{
#define DBG_DEEPCOLOR_INFO 1

    _HDMITX_GET_VARIABLE();

	//check Deep Color information
	if ( u8DataByte & 0x40 )
    {
        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_48Bits;
        #if (DBG_DEEPCOLOR_INFO == 1)
			printf("%s::Support Color Depth 48-bit\r\n", __FUNCTION__);
		#endif
    }
	else if ( u8DataByte & 0x20 )
	{
		psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_36Bits;

		#if (DBG_DEEPCOLOR_INFO == 1)
			printf("%s::Support Color Depth 36-bit\r\n", __FUNCTION__);
		#endif
	}
	else if ( u8DataByte & 0x10 )
	{
		psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_30Bits;

		#if (DBG_DEEPCOLOR_INFO == 1)
			printf("%s::Support Color Depth 30-bit\r\n", __FUNCTION__);
		#endif
	}
	else
		psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_24Bits;

#undef DBG_DEEPCOLOR_INFO

}


//**************************************************************************/
//  [Function Name]:
//                      _MDrv_HDMITx_ParseHdmiVSDB
//  [Description]:
//                      Parsing HDMI VSDB part
//  [Arguments]:
//                      [MS_U8]* pu8BlkData
//                      [MS_U8] u8BlkLen
//                      [MS_U8] u8SupTimingNum
//  [Return]:
//                      [MS_BOOL] TRUE: successful; FALSE: failed
//**************************************************************************/
MS_BOOL _MDrv_HDMITx_ParseHdmiVSDB(void* pInstance, MS_U8* pu8BlkData, MS_U8 u8BlkLen, MS_U8 u8SupTimingNum)
{
#define DBG_HDMIVSDB_INFO 1U
	MS_U8 i = 0;

    _HDMITX_GET_VARIABLE();

	//get Physical Address
	psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[0] = *(pu8BlkData + 3);
	psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[1] = *(pu8BlkData + 4);
    #if (DBG_HDMIVSDB_INFO == 1)
        DBG_HDMITX(printf("Physcal Address = [%d, %d, %d, %d]\r\n", (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[0] & 0xF0) >> 4, \
                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[0] & 0x0F, (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[1] & 0xF0) >> 4, \
                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr[1] & 0x0F));
    #endif

	if (u8BlkLen > 5)
	{
		psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = (*(pu8BlkData + 5) & 0x80)? TRUE : FALSE;

		#if (DBG_HDMIVSDB_INFO == 1)
			DBG_HDMITX(printf("%s::Support AI = %d !\n", __FUNCTION__, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI));
		#endif

		//check Deep Color information
		_MDrv_HDMITx_ParseDeepColorInfo(pInstance, *(pu8BlkData + 5));

		if (u8BlkLen > 7)
		{
			if ( *(pu8BlkData + 7) & 0x20 ) // HDMI_Video_Present
			{
				MS_U8 u8IdxOffset = 0;

				if ((*(pu8BlkData + 7) & 0xC0) == 0xC0) //Latency_Field_present & I_Latency_Field_Present
					u8IdxOffset = 4;
				else if ((*(pu8BlkData + 7) & 0xC0) == 0x40) //I_Latency_Field_Present
					u8IdxOffset = 2;
				else
					u8IdxOffset = 0;

                if( (*(pu8BlkData + 6)) > psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate)
                {
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate = *(pu8BlkData + 6);
                }

				if (u8BlkLen > (u8IdxOffset + 9))
				{
					if ( (*(pu8BlkData + u8IdxOffset + 8) & 0x80) || ( ((*(pu8BlkData + u8IdxOffset + 9) & 0xE0) >> 5) > 0 ))//3D present and VIC LEN
					{
						MS_U8 u8_3dMultiPresent = 0;
						MS_U8 u8_Hdmi3DLen = 0;
						MS_U8 u8_HdmiVICLen = 0;
						MS_U16 u16_3dStructAll = 0;
						MS_U16 u16_3dMask = 0;

                        //psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent = TRUE;
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent = (*(pu8BlkData + u8IdxOffset + 8) & 0x80) ? TRUE : FALSE;
						u8_3dMultiPresent = (*(pu8BlkData + u8IdxOffset + 8) & 0x60) >> 5;
						u8_Hdmi3DLen = (*(pu8BlkData + u8IdxOffset + 9) & 0x1F);
						u8_HdmiVICLen = (*(pu8BlkData + u8IdxOffset + 9) & 0xE0) >> 5;
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3dMultiPresent = u8_3dMultiPresent;
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Hdmi3DLen = u8_Hdmi3DLen;
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen = u8_HdmiVICLen;

                        if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_60Hz && psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent)
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_60Hz = TRUE;

                        if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_50Hz && psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent)
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_50Hz = TRUE;

                        #if (DBG_HDMIVSDB_INFO == 1)
                            DBG_HDMITX(printf("%s::3dmulti_present=%x, 3DLen=%d, VicLen=%d !\n",
                                __FUNCTION__, u8_3dMultiPresent, u8_Hdmi3DLen, u8_HdmiVICLen));
                        #endif

						//fetch VIC list
						if ( u8_HdmiVICLen > 0 )
						{
							if ( u8BlkLen > (u8IdxOffset + 9 + u8_HdmiVICLen))
							{
								for ( i = 0; i < u8_HdmiVICLen; i++ )
									psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i] = *(pu8BlkData + u8IdxOffset + 10 + i);

								#if (DBG_HDMIVSDB_INFO == 1)
									DBG_HDMITX(printf("%s::HDMI VIC list\r\n",__FUNCTION__));
									for ( i = 0; i < u8_HdmiVICLen; i++ )
										DBG_HDMITX(printf("0x%02X ", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i]));
									DBG_HDMITX(printf("\n"));
								#endif

								u8IdxOffset += u8_HdmiVICLen; //HDMI_VIC_LEN
							}
						}

                        #if (DBG_HDMIVSDB_INFO == 1)
                            DBG_HDMITX(printf("3D_MultiPresent = 0x%02X\r\n", u8_3dMultiPresent));
                        #endif

						if (u8_3dMultiPresent == 0x01) // 3D_Structure_ALL_15..0
						{
						    //check length
							if (u8BlkLen > (u8IdxOffset + 11))
							{
								u16_3dStructAll = (*(pu8BlkData + u8IdxOffset + 10) << 8) |	(*(pu8BlkData + u8IdxOffset + 11));
								for ( i = 0; i < ((u8SupTimingNum > 16) ? 16 : u8SupTimingNum); i++ )
								{
									psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[i].Video3DInfo = u16_3dStructAll;
                                    #if (DBG_HDMIVSDB_INFO == 1)
                                        DBG_HDMITX(printf("SupVidTiming[%02d].Video3DInfo = 0x%02X\r\n", i, u16_3dStructAll));
                                    #endif
							    }
								u8_Hdmi3DLen -= 2;
							}

						}
						else if (u8_3dMultiPresent == 0x02) // 3D_Structure_ALL_15..0 and 3D_Mask_ALL_15..0
						{
							//check Length
							if (u8BlkLen > (u8IdxOffset + 13))
							{
								u16_3dStructAll = (*(pu8BlkData + u8IdxOffset + 10) << 8) |	(*(pu8BlkData + u8IdxOffset + 11));
								u16_3dMask = (*(pu8BlkData + u8IdxOffset + 12) << 8) | (*(pu8BlkData + u8IdxOffset + 13));

								for ( i = 0; i < ((u8SupTimingNum > 16) ? 16 : u8SupTimingNum); i++ )
								{
									if ((u16_3dMask >> i) & 0x01)
										psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[i].Video3DInfo = u16_3dStructAll;
									else
										psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[i].Video3DInfo = 0;

                                    #if (DBG_HDMIVSDB_INFO == 1)
                                        DBG_HDMITX(printf("SupVidTiming[%02d].Video3DInfo = 0x%02X\r\n", i, u16_3dStructAll));
                                    #endif
								}
								u8_Hdmi3DLen -= 4;
							}
						}

						if (u8_Hdmi3DLen > 1) // 3D structure remains
						{
							if (u8BlkLen > (u8IdxOffset + 11))
							{
								MS_U8 u8VicOrder = 0;
								MS_U8 u83DStruct = 0;

								for ( i = 0; i < u8_Hdmi3DLen; i++ )
								{
									if (u8_3dMultiPresent == 0x01)
									{
										u8VicOrder = (*(pu8BlkData + u8IdxOffset + 12 + i) & 0xF0) >> 4;
										u83DStruct = (*(pu8BlkData + u8IdxOffset + 12 + i) & 0x0F);
									}
									else if (u8_3dMultiPresent == 0x02)
									{
										u8VicOrder = (*(pu8BlkData + u8IdxOffset + 14 + i) & 0xF0) >> 4;
										u83DStruct = (*(pu8BlkData + u8IdxOffset + 14 + i) & 0x0F);
									}
									else
									{
										u8VicOrder = (*(pu8BlkData + u8IdxOffset + 10 + i) & 0xF0) >> 4;
										u83DStruct = (*(pu8BlkData + u8IdxOffset + 10 + i) & 0x0F);
									}

									switch (u83DStruct)
									{
										case 0:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_FramePacking;
										break;

										case 1:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_FieldAlternative;
										break;

										case 2:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_LineAlternative;
										break;

										case 3:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_SidebySide_FULL;
										break;

										case 4:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_L_Dep;
										break;

										case 5:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_L_Dep_Graphic_Dep;
										break;

										case 6:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_TopandBottom;
										break;

										case 7:
										break;

										case 8:
											psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[u8VicOrder].Video3DInfo |= E_HDMITX_EDID_3D_SidebySide_Half_horizontal;
											i++;
										break;

										default:
											i++;
										break;
									}
								}
							}
						}

					}
				}
			}
		}
	}
#undef DBG_HDMIVSDB_INFO

	return TRUE;

}

//**************************************************************************/
//  [Function Name]:
//                      _drv_HDMITx_ParseHFVSDB()
//  [Description]:
//                      Parsing HF-VSDB ( new for HDMI 2.0 spec )
//  [Arguments]:
//                      [MS_U8]* pu8BlkData
//                      [MS_U8] u8BlkLen
//                      [MS_U8] u8SupTimingNum
//  [Return]:
//                      [MS_BOOL] TRUE: successful; FALSE: failed
//
//**************************************************************************/
MS_BOOL _MDrv_HDMITx_ParseHFVSDB(void* pInstance, MS_U8* pu8BlkData, MS_U8 u8BlkLen, MS_U8 u8SupTimingNum)
{
#define DBG_HFVSDB_INFO 1
    MS_U8 i = 0;
    _HDMITX_GET_VARIABLE();

	//check IEEE HDMI identifier //NOTE: current index is BYTE 1
	for ( i = 0; i < sizeof(HFVSDB_HDMI_IEEE_ID); i++ )
	{
		if ( *(pu8BlkData + i) != HFVSDB_HDMI_IEEE_ID[i] )
			return FALSE;
	}
    #if (DBG_HFVSDB_INFO == 1)
		printf("%s::IEEE HDMI Identifier PASS!\n", __FUNCTION__);
	#endif


    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDBVerInfo = *(pu8BlkData + 3);
    //psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate = *(pu8BlkData + 4);
    if( (*(pu8BlkData + 4)) > psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate)
    {
        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate = *(pu8BlkData + 4);
    }

    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC = (*(pu8BlkData + 5) & 0x80) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportRR = (*(pu8BlkData + 5) & 0x40) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportLTEScramble = (*(pu8BlkData + 5) & 0x08) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportIndependView = (*(pu8BlkData + 5) & 0x04) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportDualView = (*(pu8BlkData + 5) & 0x02) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3DOsdDisparity = (*(pu8BlkData + 5) & 0x01) ? TRUE : FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo = (*(pu8BlkData + 6) & 0x07);

    #if (DBG_HFVSDB_INFO == 1)
        printf("[HF-VSDB] Version = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDBVerInfo);
        printf("[HF-VSDB] Max_TMDS_Character_Rate = %d Mhz\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate*5);
        printf("[HF-VSDB] SCDC_Present = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC);
        printf("[HF-VSDB] RR_Capable = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportRR);
        printf("[HF-VSDB] LTF_340Mcsc_Scramble = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportLTEScramble);
        printf("[HF-VSDB] Independent_View = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportIndependView);
        printf("[HF-VSDB] Dual_View = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportDualView);
        printf("[HF-VSDB] 3D_OSD_Disparity = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3DOsdDisparity);
        printf("[HF-VSDB] DC_420_Info = %d\r\n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo);
    #endif

#undef DBG_HFVSDB_INFO

    return TRUE;
}

//**************************************************************************/
//  [Function Name]:
//                      drv_HDMITx_ParseCEAExtEdid()
//  [Description]:
//                      Parsing CEA extend EDID part
//  [Arguments]:
//                      [MS_U8]* pu8BlkData
//  [Return]:
//                      [MS_BOOL] TRUE: successful; FALSE: failed
//**************************************************************************/
MS_BOOL _MDrv_HDMITx_ParseCEAExtEdid(void* pInstance, MS_U8* pu8BlkData) //wilson@kano
{
#define DBG_PARSE_CEA_EDID 1
    MS_U8 u8DescOffset = 0;
    MS_U8 u8CurIdx = 0;
    MS_U8 i = 0;
    MS_U8 u8BlkLen = 0;
    MS_U8 u8BlkTagCode = 0;

    _HDMITX_GET_VARIABLE();

    //support CEA Extension Version 3
    if ((*(pu8BlkData) == 0x02) && (*(pu8BlkData + 1) < 0x04))
    {
        u8DescOffset = *(pu8BlkData + 2);

        if ((u8DescOffset < 0x05) || (u8DescOffset == 0xFF)) //invalid offset value;
            return FALSE;

        //psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi = TRUE;

        u8CurIdx = 0x04; //start of data block collection

        while (u8CurIdx < u8DescOffset)
        {
            u8BlkTagCode = (*(pu8BlkData + u8CurIdx) & 0xE0) >> 5;
            u8BlkLen = *(pu8BlkData + u8CurIdx) & 0x1F;

            switch (u8BlkTagCode)
            {
                case CEATag_AudioBlk:
                {
                    #if (DBG_PARSE_CEA_EDID == 1)
                    DBG_HDMITX(printf("%s::Short Audio Block:%d \n", __FUNCTION__, u8BlkLen));
                    #endif

                    u8CurIdx++;

                    for ( i = 0; i < u8BlkLen; i++ )
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudioDataBlk[i] = *(pu8BlkData + u8CurIdx + i);

                    u8CurIdx += u8BlkLen;
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8BlkTagCode] = u8BlkLen;
                }
                break;

                case CEATag_VideoBlk:
                {
                    MS_U8 ucOffset = 0; // for multiple video block present
                    #if (DBG_PARSE_CEA_EDID == 1)
                    DBG_HDMITX(printf("%s::Short Video Block: %d\r\n", __FUNCTION__, u8BlkLen));
                    #endif

                    u8CurIdx++;
                    ucOffset = g_u8SupTimingNum;
                    for ( i = 0; i < u8BlkLen; i++ )
                    {
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[ucOffset + i] = *(pu8BlkData + u8CurIdx + i);
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming = psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[ucOffset + i] & 0x7F;

                        if ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming <= E_HDMITX_VIC_1920x1080p_60_16_9) |
                            (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming == E_HDMITX_VIC_2880x480p_60_4_3) |
                            (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming == E_HDMITX_VIC_2880x480p_60_16_9))
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_60Hz = TRUE;
                        else if ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming <= E_HDMITX_VIC_1920x1080p_50_16_9) |
                            (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming == E_HDMITX_VIC_2880x576p_50_4_3) |
                            (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming == E_HDMITX_VIC_2880x576p_50_16_9)|
                            (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[g_u8SupTimingNum].VideoTiming == E_HDMITX_VIC_1920x1080i_50_16_9_1250_total))
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_50Hz = TRUE;

                        g_u8SupTimingNum++;
                    }

                    u8CurIdx += u8BlkLen;
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8BlkTagCode] = u8BlkLen;
                }
                break;
                case CEATag_VendorSpecific:
                {
                    MS_BOOL bIsVSDB = TRUE;
                    #if (DBG_PARSE_CEA_EDID == 1)
                    DBG_HDMITX(printf("%s::VSDB\r\n", __FUNCTION__));
                    #endif

                    u8CurIdx++;

                    //check IEEE HDMI identifier
                    for ( i = 0; i < sizeof(VSDB_HDMI_IEEE_ID); i++ )
                    {
                        if ( *(pu8BlkData + u8CurIdx + i) != VSDB_HDMI_IEEE_ID[i] )
                        {
                            bIsVSDB = FALSE;
                            break;
                        }
                    }

                    if (bIsVSDB == FALSE)
                    {
                        //check if HF-VSDB
                        if (_MDrv_HDMITx_ParseHFVSDB(pInstance, pu8BlkData + u8CurIdx, u8BlkLen, g_u8SupTimingNum))
                        {
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB_Len = u8BlkLen;
                            memcpy(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB, pu8BlkData + u8CurIdx, u8BlkLen);
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi = TRUE;
                            DBG_HDMITX(printf("HF-VSDB Presented!!\n"));
                        }
                        else
                        {
                            printf("HF-VSDB Parsing Failed!!\n");
                        }
                    }
                    else
                    {
                        psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi = TRUE;

                        if (_MDrv_HDMITx_ParseHdmiVSDB(pInstance, pu8BlkData + u8CurIdx, u8BlkLen, g_u8SupTimingNum))
                        {
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB_Len = u8BlkLen;
                            memcpy(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB, pu8BlkData + u8CurIdx, u8BlkLen);

                            psHDMITXResPri->stHDMITxInfo.output_mode |= (MS_U8)BIT1; // default is DVI mode

                            switch (*(pu8BlkData + 3) & 0x30)
                            {
                                case 0x00:
                                    if (psHDMITXResPri->stHDMITxInfo.input_color == E_HDMITX_VIDEO_COLOR_YUV444 ||
                                        psHDMITXResPri->stHDMITxInfo.input_color == E_HDMITX_VIDEO_COLOR_RGB444)
                                    {
                                        psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;
                                        #if (DBG_PARSE_CEA_EDID == 1)
                                        DBG_HDMITX(printf("%s::Not Support YCbCr!!\n", __FUNCTION__));
                                        #endif
                                    }
                                break;

                                case 0x20:
                                case 0x30:
                                    if (psHDMITXResPri->stHDMITxInfo.input_color == E_HDMITX_VIDEO_COLOR_YUV444 ||
                                        psHDMITXResPri->stHDMITxInfo.input_color == E_HDMITX_VIDEO_COLOR_RGB444)
                                    {
                                        psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_YUV444;
                                        #if (DBG_PARSE_CEA_EDID == 1)
                                        DBG_HDMITX(printf("%s::Output YUV444!!\n", __FUNCTION__));
                                    }
                                    #endif
                                break;

                                case 0x10:
                                    if (psHDMITXResPri->stHDMITxInfo.input_color == E_HDMITX_VIDEO_COLOR_YUV444)
                                    {
                                        psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_YUV422;
                                        #if (DBG_PARSE_CEA_EDID == 1)
                                        DBG_HDMITX(printf("%s::Output YUV422!!\n",__FUNCTION__));
                                        #endif
                                    }
                                break;

                                default:
                                break;
                            }
                        }
                    }
                    u8CurIdx += u8BlkLen;
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8BlkTagCode] = u8BlkLen;
                }
                break;

                case CEATag_UseExtTag:
                {
                    MS_U8 ucExtTagCode = 0x00;

                    ucExtTagCode = *(pu8BlkData + (++u8CurIdx));
                    u8CurIdx++;
                    DBG_HDMITX(printf("%s::Extend Tag Code = 0x%02X, block Len = 0x%X\r\n", __FUNCTION__, ucExtTagCode, u8BlkLen));

                    switch (ucExtTagCode)
                    {
                        case CEAExtTag_VideoCap_DB:
                            DBG_HDMITX(printf("Video Capability Data Block::\n"));

                            if (u8BlkLen != 0x02) //valid length
                            {
                                printf("Invalid Length of Video Capability Data Block\r\n");
                                return FALSE;
                            }
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoCapDataBlk = *(pu8BlkData + u8CurIdx);
                            u8CurIdx++;

                            DBG_HDMITX(printf("\r\n"));
                        break;

                        case CEAExtTag_YCbCr420VideoDB:
                            DBG_HDMITX(printf("YCbCr420 Video Data Block::\n"));
                            for ( i = 0; i < (u8BlkLen - 1); i++ )
                            {
                                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i] = *(pu8BlkData + u8CurIdx);
                                u8CurIdx++;
                                DBG_HDMITX(printf("0x%02X, ", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i]));
                            }
                            DBG_HDMITX(printf("\r\n"));
                        break;

                        case CEAExtTag_YCbCr420CapMapDB:
                            DBG_HDMITX(printf("YCbCr420 Capability Map Data Block::\n"));
                            for ( i = 0; i < (u8BlkLen - 1); i++ )
                            {
                                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk[i] = *(pu8BlkData + u8CurIdx);
                                u8CurIdx++;
                                DBG_HDMITX(printf("0x%02X, ", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk[i]));
                            }
                            DBG_HDMITX(printf("\r\n"));
                        break;

                        case CEAExtTag_ColorimetryDB:
                            DBG_HDMITX(printf("Colorimetry Data Block::\n"));

                            if (u8BlkLen != 0x03) //valid length
                            {
                                DBG_HDMITX(printf("Invalid Length of Colorimetry Data Block\r\n"));
                                return FALSE;
                            }
                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ExtColorimetry = *(pu8BlkData + u8CurIdx);
                            u8CurIdx+=2; //u8CurIdx++;
                            //TBD: MD parsing
                        break;

                        case CEAExtTag_HDRStaticMetaDB:
                            DBG_HDMITX(printf("HDR Static Metadata Data Block::\n"));

                            for ( i = 0; i < (u8BlkLen - 1); i++ )
                            {
                                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDRStaticDataBlk[i] = *(pu8BlkData + u8CurIdx);
                                u8CurIdx++;
                                DBG_HDMITX(printf("0x%02X, ", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDRStaticDataBlk[i]));
                                if ((i+1)%16 == 0)
                                    DBG_HDMITX(printf("\r\n"));
                            }
                            DBG_HDMITX(printf("\r\n"));

                            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.bSupportHDR = TRUE;
                        break;

                        case CEAExtTag_VSVideo_DB:
                            DBG_HDMITX(printf("Vendor Specific Video Data Block::\n"));

                            for ( i = 0; i < (u8BlkLen - 1); i++ )
                            {
                                psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VSVideoBlk[i] = *(pu8BlkData + u8CurIdx);
                                u8CurIdx++;
                                DBG_HDMITX(printf("0x%02X, ", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VSVideoBlk[i]));
                                if ((i+1)%16 == 0)
                                    DBG_HDMITX(printf("\r\n"));
                            }
                            DBG_HDMITX(printf("\r\n"));
                        break;

                        default:
                            DBG_HDMITX(printf("drv_HDMITx_ParseCEAExtEdid()::Extend Tag Code Unsupport!!\n"));
                            u8CurIdx += (u8BlkLen - 1);
                        break;
                    }
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8BlkTagCode] = u8BlkLen;
                }
                break;

                case CEATag_SpeakerAlloc:
                case CEATag_VESA_DTCBlk:
                case CEATag_Reserved_1:
                default:
                    u8CurIdx += (u8BlkLen + 1);
                    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8BlkTagCode] = u8BlkLen;
                break;
            }
        }
    }

    return TRUE;
}

//**************************************************************************/
//  [Function Name]:
//                      MDrv_HDMITx_EdidChecking()
//  [Description]:
//                      Parsing and storing information from sink device's EDID
//  [Arguments]:
//                      void
//  [Return]:
//                      [MS_BOOL] TRUE: successful; FALSE: failed
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_EdidChecking(void* pInstance)
{
    MS_U8 i = 0;
    MS_U8 u8EDIDRetryCnt = 3;
    MS_U8 u8ExtBlkNum = 0;
    MS_U8 u8ValidBlkNum = 0;
    MS_U8 blkData[HDMITX_EDID_BLK_SIZE] = {0x00};
    const MS_U8 EDID_HEADER_TAG[8] =
    {
    	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
    };
    _HDMITX_GET_VARIABLE();

    if (!bCheckEDID)
        return TRUE;

    //init buf
    g_u8SupTimingNum = 0;
    _MDrv_HDMITX_InitEdidField(pInstance);
    //memset(&gHDMITxInfo.RxEdidInfo, 0x00, sizeof(stHDMITX_RxEDID_Info));
    //memset(blkData, 0x00, HDMITX_EDID_BLK_SIZE);

    psHDMITXResPri->stHDMITxInfo.output_mode &= (~(MS_U8)BIT1); //default is DVI mode;
    psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;

    while (MHal_HDMITx_EdidReadBlock(0x00, blkData) == FALSE)
    {
        u8EDIDRetryCnt--;

        if (u8EDIDRetryCnt == 0)
        {
            //reset buf
            memset(blkData, 0x00, HDMITX_EDID_BLK_SIZE);

            DBG_HDMITX(printf("[HDMITx] %s %d, Check EDID fail!!\n", __FUNCTION__, __LINE__));
            break;
        }

        DrvHdmitxOsMsSleep(100);
    }
    if (u8EDIDRetryCnt == 0)
    {
        //return FALSE;
    }

    if (EDID_DEBUG_CHECK())
    {
        MS_U8 *pu8Str = NULL;

        pu8Str = DrvHdmitxOsMemAlloc(512*8);

        if(pu8Str)
        {
            DBG_HDMITX(printf("%s %d::block 0 ==>\n", __FUNCTION__, __LINE__));
            for ( i = 0; i < HDMITX_EDID_BLK_SIZE; i++ )
            {
                HDMITX_SPRINTF_STRCAT(pu8Str, "0x%02X, ", blkData[i]);

                if ( ((i+1)%16) == 0 )
                {
                    HDMITX_SPRINTF_STRCAT(pu8Str, "\n");
                }
            }
            DBG_HDMITX(printf("%s\n", pu8Str));
            DrvHdmitxOsMemRelease(pu8Str);
        }
    }

    //validiate EDID tag
    for ( i = 0; i < sizeof(EDID_HEADER_TAG); i++ )
    {
        if (blkData[i] != EDID_HEADER_TAG[i])
        {
            (printf("%s::invalid EDID header!!\n", __FUNCTION__));
            return FALSE;
        }
    }

    //store block 0
    memcpy(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk0, blkData, HDMITX_EDID_BLK_SIZE*sizeof(MS_U8));

    //Get Manufacturer ID
    //****************************************
    //  EISA 3-character ID
    //  08h_b'[6..2]: first char
    //  08h_b'[1..0] & 09h_b'[7..5]: second char
    //  09h_b'[4..0]: third char
    //****************************************
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID[0] = (blkData[8] & 0x7C) >> 2;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID[1] = ((blkData[8] & 0x03) << 3) | ((blkData[9] & 0xE0) >> 5);
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID[2] = blkData[9] & 0x1F;

    //check if there are more Extended block
    u8ExtBlkNum = blkData[0x7E];

    if (EDID_DEBUG_CHECK())
    {
        DBG_HDMITX(printf("%s::Extension Block Number = %d\r\n", __FUNCTION__, u8ExtBlkNum));

        if (u8ExtBlkNum > 4)
        {
            DBG_HDMITX(printf("%s::Number of Extension Block > 4!!\n", __FUNCTION__));
        }

    }
#if 0
    for ( i = 1; i <= ((u8ExtBlkNum >= 4) ? 4 : u8ExtBlkNum); i++ )
    {
        u8EDIDRetryCnt = 3;

        while (MHal_HDMITx_EdidReadBlock(i, blkData) == FALSE)
        {
            u8EDIDRetryCnt--;

            if ((u8EDIDRetryCnt == 0) || (u8EDIDRetryCnt > 3))
            {
                //reset buf
                memset(blkData, 0x00, HDMITX_EDID_BLK_SIZE);

                DBG_HDMITX(printf("[HDMITx] Check EDID fail--1!!\n"));


                break;
            }
            DrvHdmitxOsMsSleep(100);
        }

        if (u8EDIDRetryCnt != 0)
        {
            u8ValidBlkNum++;
        }
    }

    if ((u8ValidBlkNum == 0) && (! psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode))
    {
        //return FALSE;
    }
#endif

    u8ValidBlkNum = (u8ExtBlkNum >= 4) ? 4 : u8ExtBlkNum;
    for ( i = 1; i <= u8ValidBlkNum; i++)
    {
        u8EDIDRetryCnt = 3;
        while (MHal_HDMITx_EdidReadBlock(i, blkData) == FALSE)
        {
            u8EDIDRetryCnt--;

            if ((u8EDIDRetryCnt == 0) || (u8EDIDRetryCnt > 3))
            {
                //reset buf
                memset(blkData, 0x00, HDMITX_EDID_BLK_SIZE);

                DBG_HDMITX(printf("[HDMITx]%s %d, Check EDID fail--2!!\n", __FUNCTION__, __LINE__));
                break;
            }
            DrvHdmitxOsMsSleep(100);
        }

        if (u8EDIDRetryCnt != 0)
        {
            //dump block content
            if (EDID_DEBUG_CHECK())
            {
                MS_U8 j = 0;
                MS_U8 *pu8Str;

                DBG_HDMITX(printf("%s::Block #%d\r\n", __FUNCTION__, i));

                pu8Str = DrvHdmitxOsMemAlloc(512*8);
                if(pu8Str)
                {
                    for ( j = 0; j < HDMITX_EDID_BLK_SIZE; j++ )
                    {
                        HDMITX_SPRINTF_STRCAT(pu8Str, "0x%02X, ", blkData[j]);
                        if ( (j+1) % 16 == 0 )
                        {
                            HDMITX_SPRINTF_STRCAT(pu8Str, "\n");
                        }
                    }
                    DBG_HDMITX(printf("%s\n", pu8Str));
                    DrvHdmitxOsMemRelease(pu8Str);
                }
            }

            if ( i == 1 )
            {
                memcpy(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1, &blkData, 128); //store block #1
            }

            if ( !_MDrv_HDMITx_ParseCEAExtEdid(pInstance, blkData) ) //invalid CEA Extension block
            {
                continue;
            }
        }
        else
        {
            DBG_HDMITX(printf("%s::Access EDID fail @ Block 0x%02X!!\n", __FUNCTION__, i));

            if (! psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode)
            {
                //return FALSE;
            }
            else
            {
                continue;
            }
        }
    }

    //TBD: if API layer force output mode
    if (psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode) // AP force output mode
    {
        psHDMITXResPri->stHDMITxInfo.output_mode = psHDMITXResPri->stHDMITxInfo.force_output_mode;
        if((psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI) || (psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI_HDCP)) // DVI mode
        {
            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = FALSE;
            psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;
        }
    }

    // AP force output color format
    if((psHDMITXResPri->stHDMITxInfo.output_mode == E_HDMITX_HDMI || psHDMITXResPri->stHDMITxInfo.output_mode == E_HDMITX_HDMI_HDCP) && (psHDMITXResPri->stHDMITxInfo.hdmitx_force_output_color))
    {
        psHDMITXResPri->stHDMITxInfo.output_color = psHDMITXResPri->stHDMITxInfo.force_output_color;
    }

    // YUV444 -> RGB444
    #if 0
    if( (psHDMITXResPri->stHDMITxInfo.input_color != psHDMITXResPri->stHDMITxInfo.output_color) && (psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_RGB444) )
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
    else // bypass
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;
    #endif

    psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = TRUE;
    bCheckEDID = FALSE;

    Mhal_HDMITx_SetSCDCCapability(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC);

    return TRUE;

}

//------------------------------------------------------------------------------
/// @brief This routine set HDMI Tx initial situation.
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_InitVariable(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.bHDMITxTaskIdCreated = FALSE;
    psHDMITXResPri->stHDMITxInfo.bHDMITxEventIdCreated = FALSE;
    psHDMITXResPri->stHDMITxInfo.bCheckRxTimerIdCreated = FALSE;

    psHDMITXResPri->stHDMITxInfo.hdmitx_enable_flag = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag = TRUE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag = TRUE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag = TRUE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;

    psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PENDING;
    psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_PENDING;
    psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status = E_HDMITX_DVIClock_L_HPD_L;
    _hdmitx_preRX_status = psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status;
    psHDMITXResPri->stHDMITxInfo.hdmitx_RB_swap_flag = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_force_output_color = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_AFD_override_mode = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag = FALSE;
    psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag = TRUE; // CEC

    psHDMITXResPri->stHDMITxInfo.output_mode = E_HDMITX_DVI;
    psHDMITXResPri->stHDMITxInfo.force_output_mode = E_HDMITX_HDMI;
    psHDMITXResPri->stHDMITxInfo.ext_colorimetry = E_HDMITX_EXT_COLORIMETRY_XVYCC601;
    psHDMITXResPri->stHDMITxInfo.colorimetry = E_HDMITX_COLORIMETRY_MAX;
    psHDMITXResPri->stHDMITxInfo.enYCCQuantRange = E_HDMITX_YCC_QUANT_LIMIT;
    psHDMITXResPri->stHDMITxInfo.enRGBQuantRange = E_HDMITX_RGB_QUANT_DEFAULT;
    psHDMITXResPri->stHDMITxInfo.output_colordepth_val = E_HDMITX_VIDEO_CD_NoID;//E_HDMITX_VIDEO_CD_24Bits;
    psHDMITXResPri->stHDMITxInfo.output_video_prevtiming = E_HDMITX_RES_MAX;
    psHDMITXResPri->stHDMITxInfo.output_video_timing = E_HDMITX_RES_720x480p;
    psHDMITXResPri->stHDMITxInfo.force_output_color = E_HDMITX_VIDEO_COLOR_RGB444;
    psHDMITXResPri->stHDMITxInfo.input_color = E_HDMITX_VIDEO_COLOR_RGB444;
    psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;
    psHDMITXResPri->stHDMITxInfo.input_range = E_HDMITX_YCC_QUANT_LIMIT;
    psHDMITXResPri->stHDMITxInfo.output_range = E_HDMITX_YCC_QUANT_LIMIT;
    psHDMITXResPri->stHDMITxInfo.output_aspect_ratio = E_HDMITX_VIDEO_AR_4_3;
    psHDMITXResPri->stHDMITxInfo.output_scan_info = E_HDMITX_VIDEO_SI_NoData;
    psHDMITXResPri->stHDMITxInfo.output_afd_ratio = E_HDMITX_VIDEO_AFD_SameAsPictureAR;
    psHDMITXResPri->stHDMITxInfo.output_activeformat_present = 1;
    psHDMITXResPri->stHDMITxInfo.output_audio_frequncy = E_HDMITX_AUDIO_48K;
    psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag = FALSE;
#ifdef OBERON
    psHDMITXResPri->stHDMITxInfo.output_audio_channel = E_HDMITX_AUDIO_CH_8;
#else // Uranus, Uranus4
    psHDMITXResPri->stHDMITxInfo.output_audio_channel = E_HDMITX_AUDIO_CH_2;
#endif
    psHDMITXResPri->stHDMITxInfo.output_audio_type = E_HDMITX_AUDIO_PCM;

    // HDMI Tx Pre-emphasis and Double termination
    memset(&psHDMITXResPri->stHDMITxInfo.analog_setting, 0, sizeof(psHDMITXResPri->stHDMITxInfo.analog_setting));

    _MDrv_HDMITX_InitEdidField(pInstance);
#if 0
    //edid relative //assign initial value
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk0, 0x00, HDMITX_EDID_BLK_SIZE*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1, 0x00, HDMITX_EDID_BLK_SIZE*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID, 0x00, 3*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen, 0x00, HDMITX_CEA_DAT_BLK_TYPE_NUM*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr, 0x00, 2*sizeof(MS_U8));
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi = FALSE;

	//HF-VSDB, scdc relative
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDBVerInfo = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportIndependView = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportDualView = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3DOsdDisparity = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportLTEScramble = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC = FALSE;

    //read request
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo = 0x00;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.b3DPresent = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_50Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support2D_60Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_50Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_60Hz = FALSE;
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo = E_HDMITX_VIDEO_CD_NoID;

    // data block content
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudioDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VendorDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SpeakerAllocDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VESA_DTCDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList, 0x00, 7*sizeof(MS_U8)); //this field has only 3 bits
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming, 0x00, 32*sizeof(MS_U8));

    //for 420
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk, 0x00, 32*sizeof(MS_U8));
    memset(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk, 0x00, 32*sizeof(MS_U8));

    //for colorimetry
    psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ExtColorimetry = 0x00;
#endif

    // Set DDC speed to 50KHz as default
    MHal_HDMITx_AdjustDDCFreq(100); // real speed is 80%
}

//------------------------------------------------------------------------------
/// @brief This routine sends HDMI related packets.
/// @param[in] packet_type packet type
/// @param[in] packet_process packet behavior
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SendPacket(MsHDMITX_PACKET_TYPE packet_type, MsHDMITX_PACKET_PROCESS packet_process)
{
    MHal_HDMITx_SendPacket(packet_type, packet_process);
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetTmdsEnFlag()
//  [Description]:
//                  return flag which indicate if tmds signal is enabled or not
//  [Arguments]:
//			void
//  [Return]:
//			MS_BOOL
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_GetTmdsEnFlag(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetTmdsEnFlag()
//  [Description]:
//                  return flag which indicate if tmds signal is enabled or not
//  [Arguments]:
//			void
//  [Return]:
//			MS_BOOL
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_GetHdcpEnFlag(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag;
}


//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetOutputMode()
//  [Description]:
//                  return current hdmi output mode value
//  [Arguments]:
//			void
//  [Return]:
//			MsHDMITX_OUTPUT_MODE
//
//**************************************************************************
MsHDMITX_OUTPUT_MODE MDrv_HDMITx_GetOutputMode(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHDMITxInfo.output_mode;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetRxStatus()
//  [Description]:
//                  return current HDMI status
//  [Arguments]:
//			void
//  [Return]:
//			[MS_BOOL]
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_GetRxStatus(void)
{
    if (MHal_HDMITx_GetRXStatus() == E_HDMITX_DVIClock_H_HPD_H)
        return TRUE;
    else
        return FALSE;

    //return (gbCurRxStatus == E_HDMITX_DVIClock_H_HPD_H)? TRUE : FALSE;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetFullRxStatus()
//  [Description]:
//                  return current HPD and clock status
//  [Arguments]:
//			void
//  [Return]:
//			[MS_BOOL]
//
//**************************************************************************/
MsHDMITX_RX_STATUS MDrv_HDMITx_GetFullRxStatus(void* pInstance)
{
    MsHDMITX_RX_STATUS enRxStatus = E_HDMITX_DVIClock_L_HPD_L;

    enRxStatus = MHal_HDMITx_GetRXStatus();
    return enRxStatus;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetDeepColorMode()
//  [Description]:
//                  return current deep color mode
//  [Arguments]:
//			void
//  [Return]:
//			[MsHDMITX_VIDEO_COLORDEPTH_VAL]
//
//**************************************************************************/
MsHDMITX_VIDEO_COLORDEPTH_VAL MDrv_HDMITx_GetDeepColorMode(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.output_colordepth_val;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetOutputTiming()
//  [Description]:
//                  return current output timing
//  [Arguments]:
//			void
//  [Return]:
//			[MsHDMITX_VIDEO_TIMING]
//
//**************************************************************************/
MsHDMITX_VIDEO_TIMING MDrv_HDMITx_GetOutputTiming(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHDMITxInfo.output_video_timing;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetOutputTiming()
//  [Description]:
//                  return current output timing
//  [Arguments]:
//			void
//  [Return]:
//			[MsHDMITX_VIDEO_TIMING]
//
//**************************************************************************/
MsHDMITX_VIDEO_COLOR_FORMAT MDrv_HDMITx_GetColorFormat(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    return psHDMITXResPri->stHDMITxInfo.output_color;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetRxDCInfoFromEDID()
//  [Description]:
//                  Get deep color information from EDID
//  [Arguments]:
//			void
//  [Return]:
//			[HDMITX_VIDEO_COLORDEPTH_VAL]
//
//**************************************************************************/
MsHDMITX_VIDEO_COLORDEPTH_VAL MDrv_HDMITx_GetRxDCInfoFromEDID(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ColorDepthInfo;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetEdidRdyFlag()
//  [Description]:
//                  return if sink EDID is ready
//  [Arguments]:
//			void
//  [Return]:
//			[MS_BOOL]
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_GetEdidRdyFlag(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetRxVideoFormatFromEDID()
//  [Description]:
//                  return short video descriptor content of sink EDID
//  [Arguments]:
//                  [MS_U8]* pu8Buffer
//                  [MS_U8] u8BufSize
//  [Return]:
//			void
//
//**************************************************************************/
void MDrv_HDMITx_GetRxVideoFormatFromEDID(void* pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize)
{
    _HDMITX_GET_VARIABLE();
    memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk, (u8BufSize > 32) ? 32*sizeof(MS_U8) : u8BufSize*sizeof(MS_U8));
}

//**************************************************************************/
//  [Function Name]:
//                      MDrv_HDMITx_GetDataBlockLengthFromEDID()
//  [Description]:
//                      return length of specific data block
//  [Arguments]:
//                  [MS_U8]* pu8Length
//                  [MS_U8] u8TagCode
//  [Return]:
//			void
//
//**************************************************************************/
void MDrv_HDMITx_GetDataBlockLengthFromEDID(void* pInstance, MS_U8 *pu8Length, MS_U8 u8TagCode)
{
    _HDMITX_GET_VARIABLE();
    *pu8Length = (u8TagCode < HDMITX_CEA_DAT_BLK_TYPE_NUM) ? psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[u8TagCode] : 0x00;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetRxAudioFormatFromEDID()
//  [Description]:
//                  return short audio descriptor content of sink EDID
//  [Arguments]:
//                  [MS_U8]* pu8Buffer
//                  [MS_U8] u8BufSize
//  [Return]:
//			void
//
//**************************************************************************/
void MDrv_HDMITx_GetRxAudioFormatFromEDID(void* pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize)
{
    _HDMITX_GET_VARIABLE();
    if (pu8Buffer != NULL)
        memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudioDataBlk, (u8BufSize > 32) ? 32 : u8BufSize);
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_EdidGetHDMISupportFlag()
//  [Description]:
//                  return flag which indicate if sink EDID support HDMI
//  [Arguments]:
//                  void
//  [Return]:
//			MS_BOOL
//
//**************************************************************************/
MS_BOOL MDrv_HDMITx_EdidGetHDMISupportFlag(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi;
}

//**************************************************************************/
//  [Function Name]:
//			MDrv_HDMITx_GetRxIDManufacturerName()
//  [Description]:
//                  return flag which indicate if sink EDID support HDMI
//  [Arguments]:
//                  [MS_U8] *pu8Buffer
//  [Return]:
//			void
//
//**************************************************************************/
void MDrv_HDMITx_GetRxIDManufacturerName(void* pInstance, MS_U8 *pu8Buffer)
{
    _HDMITX_GET_VARIABLE();
    memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID, sizeof(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ManufacturerID));
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_GetEDIDData()
//  [Description]:
//                  return data content of EDID block
//  [Arguments]:
//                  [MS_U8] *pu8Buffer
//                  [MS_BOOL] BlockIdx
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_GetEDIDData(void* pInstance, MS_U8 *pu8Buffer, MS_BOOL BlockIdx)
{
    _HDMITX_GET_VARIABLE();
    if (BlockIdx)
	{
		memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1, 128*sizeof(MS_U8));
	}
	else
	{
		memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk0, 128*sizeof(MS_U8));
	}
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_GetRx3DStructureFromEDID()
//  [Description]:
//                  return current hdmi output mode value
//  [Arguments]:
//                  [MsHDMITX_VIDEO_TIMING] timing
//                  [MsHDMITX_EDID_3D_STRUCTURE_ALL] *p3DStructure
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_GetRx3DStructureFromEDID(void* pInstance, MsHDMITX_VIDEO_TIMING timing, MsHDMITX_EDID_3D_STRUCTURE_ALL *p3DStructure)
{
    MS_U8 i;
    MsHDMITX_AVI_VIC vic = E_HDMITX_VIC_NOT_AVAILABLE;
    _HDMITX_GET_VARIABLE();

    *p3DStructure = 0; // Default all not support

    vic = _MDrv_HDMIRx_MapVideoTimingToVIC(timing);

    if (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_50Hz)
    {
        switch(vic)
        {
            case E_HDMITX_VIC_1920x1080p_24_16_9:
                *p3DStructure |= E_HDMITX_EDID_3D_FramePacking;
                *p3DStructure |= E_HDMITX_EDID_3D_TopandBottom;
                break;
            case E_HDMITX_VIC_1280x720p_50_16_9:
                *p3DStructure |= E_HDMITX_EDID_3D_FramePacking;
                *p3DStructure |= E_HDMITX_EDID_3D_TopandBottom;
                break;
            case E_HDMITX_VIC_1920x1080i_50_16_9:
            case E_HDMITX_VIC_1920x1080i_50_16_9_1250_total:
                *p3DStructure |= E_HDMITX_EDID_3D_SidebySide_Half_horizontal;
                break;
            default:
                break;
        }

    }

    if (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.Support3D_60Hz)
    {
        switch(vic)
        {
    		case E_HDMITX_VIC_1920x1080p_24_16_9:
    			*p3DStructure |= E_HDMITX_EDID_3D_FramePacking;
    			*p3DStructure |= E_HDMITX_EDID_3D_TopandBottom;
    			break;
    		case E_HDMITX_VIC_1280x720p_60_16_9:
    			*p3DStructure |= E_HDMITX_EDID_3D_FramePacking;
    			*p3DStructure |= E_HDMITX_EDID_3D_TopandBottom;
    			break;
    		case E_HDMITX_VIC_1920x1080i_60_16_9:
    			*p3DStructure |= E_HDMITX_EDID_3D_SidebySide_Half_horizontal;
                break;
            default:
                break;
        }
    }

    for (i=0;i < 32;i++)
    {
        if (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[i].VideoTiming == vic)
        {
            *p3DStructure |= psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupVidTiming[i].Video3DInfo;
        }
    }
}

//------------------------------------------------------------------------------
/// @brief This routine Get Audio CTS value
/// @return CTS
//------------------------------------------------------------------------------
MS_U32 MDrv_HDMITx_GetAudioCTS(void)
{
    return MHal_HDMITx_GetAudioCTS();
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetAVMUTEState()
//  [Description]:
//                      return AVMUTE flag
//  [Arguments]:
//			    void
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_GetAVMUTEStatus(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag;
}

/// Get version (without Mutex protect)
/*
MS_BOOL  MDrv_HDMITx_GetLibVer(const MSIF_Version **ppVersion)
{
    if (!ppVersion)
    {
        return FALSE;
    }

    *ppVersion = &_drv_hdmi_tx_version;
    return TRUE;
}
*/

MS_BOOL  MDrv_HDMITx_GetInfo(MS_HDMI_TX_INFO *pInfo)
{
    if (!pInfo)
    {
        return FALSE;
    }

    memcpy((void*)pInfo, (void*)&_info, sizeof(MS_HDMI_TX_INFO));
    return TRUE;
}

MS_BOOL  MDrv_HDMITx_GetStatus(MS_HDMI_TX_Status *pStatus)
{
    if (!pStatus)
    {
        return FALSE;
    }

    memcpy((void*)pStatus, (void*)&_hdmitx_status, sizeof(MS_HDMI_TX_Status ));
    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetFSMState()
//  [Description]:
//                  set state of hdmi tx FSM
//  [Arguments]:
//			[MDrvHDMITX_FSM_STATE] enState
//  [Return]:
//			void
//
//**************************************************************************
MDrvHDMITX_FSM_STATE MDrv_HDMITx_GetFSMState(void* pInstance)
{
    _HDMITX_GET_VARIABLE();
    return psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetEDIDPhyAdr()
//  [Description]:
//                      Get sink physical address from EDID
//  [Arguments]:
//			    [MS_U8]* pucData
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetEDIDPhyAdr(void* pInstance, MS_U8* pucData)
{
    _HDMITX_GET_VARIABLE();
    memcpy(pucData, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.PhyAddr, 0x02);
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetVICListFromEDID()
//  [Description]:
//                      Get VIC list from EDID
//  [Arguments]:
//                      [MS_U8] *pu8Buffer,
//                      [MS_U8] u8BufSize,
//  [Return]:
//			    MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_GetVICListFromEDID(void* pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize)
{
    _HDMITX_GET_VARIABLE();
    memcpy(pu8Buffer, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList, (u8BufSize > 7) ? 7 : u8BufSize);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetColorFormatFromEDID()
//  [Description]:
//                      Get color format from EDID
//  [Arguments]:
//			    [MS_U8]* pucData
//  [Return]:
//			    void
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_GetColorFormatFromEDID(void* pInstance, MsHDMITX_VIDEO_TIMING timing, MsHDMITX_EDID_COLOR_FORMAT *pColorFmt)
{
    MS_U8 i = 0;
    MsHDMITX_AVI_VIC enVICCode = (0x01 << E_HDMITX_VIC_NOT_AVAILABLE);
    _HDMITX_GET_VARIABLE();

    enVICCode = _MDrv_HDMIRx_MapVideoTimingToVIC(timing);

    if ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1[0] == 0x02) && (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1[1] <= 0x04))
    {
        //YCbCr 420 only
        if( enVICCode != E_HDMITX_VIC_NOT_AVAILABLE)
        {
            for ( i = 0; i < 32; i++ )
            {
                if (enVICCode == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i])
                {
                    *pColorFmt = (0x01 << E_HDMITX_VIDEO_COLOR_YUV420);
                    return TRUE;
                }
            }
        }

        switch (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1[0x03] & 0x30)
        {
            case 0x00:
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
            break;

            case 0x10:
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_YUV422);
            break;

            case 0x20:
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_YUV444);
            break;

            case 0x30:
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_YUV444);
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_YUV422);
            break;

            default:
                *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
            break;
        }

        for ( i = 0; i < sizeof(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk); i++ )
        {
            if (enVICCode != E_HDMITX_VIC_NOT_AVAILABLE && enVICCode == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i])
            {
                if (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk[i >> 3] & (0x01 << (i % 8)))
                {
                    *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_YUV420);
                }
            }
        }
    }
    else
    {
        *pColorFmt |= (0x01 << E_HDMITX_VIDEO_COLOR_RGB444);
    }
    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetAudioDataBlock()
//  [Description]:
//                      Get Audio data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetAudioDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudioDataBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetVideoDataBlock()
//  [Description]:
//                      Get video data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetVideoDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetVSDB()
//  [Description]:
//                      Get vendor-specific data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetVSDB(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        *pu32RealLen = (u32DataLen > psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB_Len) ? psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB_Len : u32DataLen;
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB, *pu32RealLen);
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetHFVSDB()
//  [Description]:
//                      Get vendor-specific data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetHFVSDB(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        *pu32RealLen = (u32DataLen > psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB_Len) ? psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB_Len : u32DataLen;
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HF_VSDB, *pu32RealLen);
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetColorimetryDataBlock()
//  [Description]:
//                      Get colorimetry data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetColorimetryDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        *pu8Data = psHDMITXResPri->stHDMITxInfo.RxEdidInfo.ExtColorimetry;
        *pu32RealLen = 0x01;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_GetHDRStaticMetaDataBlock()
//  [Description]:
//                      Get HDR static meta data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_GetHDRStaticMetaDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDRStaticDataBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_Get420VideoDataBlock()
//  [Description]:
//                      Get YCbCr420 video data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_Get420VideoDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_Get420CapabilityMapDataBlock()
//  [Description]:
//                      Get YCbCr420 Capability Map data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_Get420CapabilityMapDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//                      MDrv_HDMITx_GetVSVideoDataBlock()
//  [Description]:
//                      Get Vendor specific video data block from EDID
//  [Arguments]:
//                      [MS_U8]* pu8Data: buff to store data block content
//                      [MS_U32] u32DataLen: buff length
//                      [MS_U32]* pu32RealLen: real length of edid data block
//  [Return]:
//                      void
//
//**************************************************************************
void MDrv_HDMITx_GetVSVideoDataBlock(void* pInstance, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen)
{
    _HDMITX_GET_VARIABLE();

    if (pu8Data != NULL)
    {
        memcpy(pu8Data, psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VSVideoBlk, (u32DataLen > 32) ? 32 : u32DataLen);
        *pu32RealLen = (u32DataLen > 32) ? 32 : u32DataLen;
    }
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_SetGCPParameter()
//  [Description]:
//                      set general control packet content
//  [Arguments]:
//			    void
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_SetGCPParameter(void* pInstance)
{
    stGC_PktPara stGCPktPara;

    _HDMITX_GET_VARIABLE();

    stGCPktPara.enAVMute = (psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag == TRUE) ? E_HDMITX_GCP_SET_AVMUTE : E_HDMITX_GCP_CLEAR_AVMUTE;
    stGCPktPara.enColorDepInfo = psHDMITXResPri->stHDMITxInfo.output_colordepth_val;
    MHal_HDMITx_SetGCPParameter(stGCPktPara);
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_SetAVIInfoPktPara()
//  [Description]:
//                      set AVI infoframe packet content
//  [Arguments]:
//			    void
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_SetAVIInfoPktPara(void* pInstance)
{
    stAVIInfo_PktPara stAVIInfoPara;

    _HDMITX_GET_VARIABLE();

    stAVIInfoPara.enableAFDoverWrite = psHDMITXResPri->stHDMITxInfo.hdmitx_AFD_override_mode;
	stAVIInfoPara.A0Value = psHDMITXResPri->stHDMITxInfo.output_activeformat_present;
    stAVIInfoPara.enColorimetry = psHDMITXResPri->stHDMITxInfo.colorimetry;
    stAVIInfoPara.enExtColorimetry = psHDMITXResPri->stHDMITxInfo.ext_colorimetry;
    stAVIInfoPara.enYCCQuantRange = psHDMITXResPri->stHDMITxInfo.enYCCQuantRange;
    stAVIInfoPara.enRGBQuantRange = psHDMITXResPri->stHDMITxInfo.enRGBQuantRange;
	stAVIInfoPara.enVidTiming = psHDMITXResPri->stHDMITxInfo.output_video_timing;
	stAVIInfoPara.enAFDRatio = psHDMITXResPri->stHDMITxInfo.output_afd_ratio;
	stAVIInfoPara.enScanInfo = psHDMITXResPri->stHDMITxInfo.output_scan_info;
	stAVIInfoPara.enAspectRatio =  psHDMITXResPri->stHDMITxInfo.output_aspect_ratio;

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag)
    {
        stAVIInfoPara.enColorFmt = psHDMITXResPri->stHDMITxInfo.output_color;
    }
    else
    {
        if(!MHal_HDMITx_CSC_Support_R2Y(pInstance))
        {
            stAVIInfoPara.enColorFmt = E_HDMITX_VIDEO_COLOR_RGB444;
        }
        else
        {
            stAVIInfoPara.enColorFmt = psHDMITXResPri->stHDMITxInfo.output_color;
        }
    }

    MHal_HDMITx_SetAVIInfoParameter(stAVIInfoPara);
}

//**************************************************************************
//  [Function Name]:
//			    MDrv_HDMITx_SetAudInfoPktPara()
//  [Description]:
//                      Set Audio Infoframe packet content
//  [Arguments]:
//			    void
//  [Return]:
//			    void
//
//**************************************************************************
void MDrv_HDMITx_SetAudInfoPktPara(void* pInstance)
{
    stAUDInfo_PktPara stAudInfoPara;

    _HDMITX_GET_VARIABLE();

    stAudInfoPara.enAudChCnt = psHDMITXResPri->stHDMITxInfo.output_audio_channel;
    stAudInfoPara.enAudFreq = psHDMITXResPri->stHDMITxInfo.output_audio_frequncy;
    stAudInfoPara.enAudType = psHDMITXResPri->stHDMITxInfo.output_audio_type;
    MHal_HDMITx_SetAudioInfoParameter(stAudInfoPara);
    //stAudInfoPara.bChStatus = gHDMITxInfo.output_a
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx TMDS signal
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetTMDSOnOff(void* pInstance, MS_BOOL state)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag = state;

    if (!g_bDisableTMDSCtrl)
    {
        MHal_HDMITx_SetTMDSOnOff(psHDMITXResPri->stHDMITxInfo.hdmitx_RB_swap_flag, psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag);
    }
}


//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx TMDS signal
/// @return None
//------------------------------------------------------------------------------
MsHDMITx_TMDS_STATUS MDrv_HDMITx_GetTMDSStatus(void* pInstance)
{
    MsHDMITx_TMDS_STATUS enTMDSStatus = E_HDMITX_TMDS_OFF;
    enTMDSStatus = MHal_HDMITx_GetTMDSStatus();
    return enTMDSStatus;
}

//------------------------------------------------------------------------------
/// @brief This routine config HDMI Tx output mode (DVI / HDMI)
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetHDMITxMode(void* pInstance, MsHDMITX_OUTPUT_MODE mode)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_mode = mode;

    if (psHDMITXResPri->stHDMITxInfo.output_mode & 0x01) // check HDCP
    {
        MDrv_HDMITx_SetHDCPFlag(pInstance, TRUE); //gHDMITxInfo.hdmitx_hdcp_flag = TRUE;
    }
    else
    {
        MDrv_HDMITx_SetHDCPFlag(pInstance, FALSE); //gHDMITxInfo.hdmitx_hdcp_flag = FALSE;
    }

    if ( MHal_HDMITx_IsSupportDVIMode() == FALSE)
    {
        /***********************************************/
        //SW Patch,
        //KAISERIN_CHIP_TOP_BASE DO NOT support YUV2RGB
        /***********************************************/
        psHDMITXResPri->stHDMITxInfo.output_mode = (psHDMITXResPri->stHDMITxInfo.output_mode | BIT1);
    }
    // HDMI / DVI

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode) // AP force output mode
    {
        psHDMITXResPri->stHDMITxInfo.output_mode = psHDMITXResPri->stHDMITxInfo.force_output_mode;
        if((psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI) || (psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI_HDCP)) // DVI mode
        {
            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = FALSE;
            psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;
        }
    }

    if (psHDMITXResPri->stHDMITxInfo.output_mode & BIT1) // HDMI
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag = TRUE;
        MHal_HDMITx_SetHDMImode(TRUE, psHDMITXResPri->stHDMITxInfo.output_colordepth_val);
        MDrv_HDMITx_SendPacket(E_HDMITX_GC_PACKET, E_HDMITX_CYCLIC_PACKET);
        //MDrv_HDMITx_SendPacket(E_HDMITX_NULL_PACKET, E_HDMITX_CYCLIC_PACKET);
        if (g_bDisableVSInfo == TRUE)
        {
            MDrv_HDMITx_SendPacket(E_HDMITX_VS_INFOFRAME, E_HDMITX_STOP_PACKET);
        }
        MDrv_HDMITx_SendPacket(E_HDMITX_SPD_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
        DBG_HDMITX(printf("MDrv_HDMITx_SetHDMITxMode: HDMI mode = %d \n", psHDMITXResPri->stHDMITxInfo.output_mode));
    }
    else // DVI
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag = FALSE;
        MHal_HDMITx_SetHDMImode(FALSE, psHDMITXResPri->stHDMITxInfo.output_colordepth_val);
        DBG_HDMITX(printf("MDrv_HDMITx_SetHDMITxMode: DVI mode = %d \n", psHDMITXResPri->stHDMITxInfo.output_mode));
    }
}

//**************************************************************************
//  [Function Name]:
//			HDMITx_Handler()
//  [Description]:
//                  HDMI TX FSM
//  [Arguments]:
//			[MS_U32] u32State
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetDeepColorMode(void* pInstance, MsHDMITX_VIDEO_COLORDEPTH_VAL enDeepColorMode)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_colordepth_val = enDeepColorMode;
    MDrv_HDMITx_SetGCPParameter(pInstance);
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetRBChannelSwap()
//  [Description]:
//                  set R, B channel swap flag
//  [Arguments]:
//			[MS_BOOL] state
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetRBChannelSwap(void* pInstance, MS_BOOL state) //wilson@kano
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_RB_swap_flag = state;
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx video output
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetVideoOnOff(void* pInstance, MS_BOOL state)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag = state;

    #if 0
    if (psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag == FALSE) //force RGB color format for pattern gen
    {
        MDrv_HDMITx_SetColorFormat(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, E_HDMITX_VIDEO_COLOR_RGB444);
    }
    #endif

    MDrv_HDMITx_ColorandRange_Transform(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color, psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);

    if (psHDMITXResPri->stHDMITxInfo.output_video_timing >= E_HDMITX_RES_1280x720p_50Hz)
        MHal_HDMITx_SetVideoOnOff(psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag, psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag, TRUE);
    else
        MHal_HDMITx_SetVideoOnOff(psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag, psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag, FALSE);
}

//------------------------------------------------------------------------------
/// @brief This routine sets video color formatt
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetColorFormat(void* pInstance, MsHDMITX_VIDEO_COLOR_FORMAT enInColor, MsHDMITX_VIDEO_COLOR_FORMAT enOutColor)
{
    #if 0
    MS_BOOL bflag = FALSE;

    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.input_color = enInColor;
    psHDMITXResPri->stHDMITxInfo.output_color = enOutColor;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag == FALSE)
    {
        MDrv_HDMITx_ColorandRange_Transform(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color, psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);
        return;
    }

    // YUV444 -> RGB444
    if ((enInColor != enOutColor) /*&& (enOutColor == E_HDMITX_VIDEO_COLOR_RGB444)*/)
    {
            psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
    }
    else
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;
    }


    bflag = (psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_YUV422) ? TRUE : FALSE;

    if(psHDMITXResPri->stHDMITxInfo.input_range != psHDMITXResPri->stHDMITxInfo.output_range)
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
    }

	if (psHDMITXResPri->stHDMITxInfo.output_video_timing >= E_HDMITX_RES_1280x720p_50Hz)
    {
        MHal_HDMITx_SetColorFormat(psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag, bflag, TRUE);
    }
    else
    {
        MHal_HDMITx_SetColorFormat(psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag, bflag, FALSE);
    }

	// send packet
    MDrv_HDMITx_SendPacket(E_HDMITX_AVI_INFOFRAME, E_HDMITX_CYCLIC_PACKET);

    if (psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_YUV420)
        MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing); //wilson: for 420;

    printf("A InColor = %x, OutColor = %x\r\n", enInColor, enOutColor);
    #else
        _HDMITX_GET_VARIABLE();
        DBG_HDMITX(printf("B InColor = %x, OutColor = %x\r\n", enInColor, enOutColor));
        MDrv_HDMITx_ColorandRange_Transform(pInstance, enInColor, enOutColor, psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);
        //MDrv_HDMITx_ColorandRange_Transform(pInstance, enInColor, enOutColor, E_HDMITX_YCC_QUANT_FULL, E_HDMITX_YCC_QUANT_FULL);
    #endif
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_ColorandRange_Transform()
//  [Description]:
//                  This routine sets video color range and format
//  [Arguments]:
//			[MsHDMITX_VIDEO_COLOR_FORMAT] Input/Output color format
//                  [MsHDMITX_VIDEO_COLOR_RANGE] Input/Outpu color range
//  [Return]:
//			[MS_BOOL] Color Range Transforming supported or not
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_ColorandRange_Transform(void* pInstance, MsHDMITX_VIDEO_COLOR_FORMAT incolor, MsHDMITX_VIDEO_COLOR_FORMAT outcolor, MsHDMITX_YCC_QUANT_RANGE inange, MsHDMITX_YCC_QUANT_RANGE outrange)
{
    MS_BOOL bRet = FALSE;
    _HDMITX_DECLARE_VARIABE()
    //_HDMITX_SEMAPHORE_ENTRY(pInstance);
    _HDMITX_GET_VARIABLE_WITHOUT_DECLARE();
    psHDMITXResPri->stHDMITxInfo.input_color = incolor;
    psHDMITXResPri->stHDMITxInfo.output_color = outcolor;
    psHDMITXResPri->stHDMITxInfo.input_range = inange;
    psHDMITXResPri->stHDMITxInfo.output_range = outrange;
    psHDMITXResPri->stHDMITxInfo.enYCCQuantRange = outrange;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag == FALSE)
    {
        if(!MHal_HDMITx_CSC_Support_R2Y(pInstance))
        {
            incolor = E_HDMITX_VIDEO_COLOR_RGB444;
            outcolor = E_HDMITX_VIDEO_COLOR_RGB444;
        }
        else
        {
            incolor = E_HDMITX_VIDEO_COLOR_RGB444;
        }
    }

    if( (incolor == E_HDMITX_VIDEO_COLOR_RGB444) && (outcolor != E_HDMITX_VIDEO_COLOR_RGB444) )
    {
        //Old chip limited by R2Y
        if(!MHal_HDMITx_CSC_Support_R2Y(pInstance))
        {
            printf("Can not support for R2Y\r\n");
            return FALSE;
        }
    }

    if(!MDrv_HDMITx_EdidGetHDMISupportFlag(pInstance) && (!psHDMITXResPri->stHDMITxInfo.hdmitx_force_output_color))
    {
        if(outcolor != E_HDMITX_VIDEO_COLOR_RGB444)
        {
            DBG_HDMITX(printf("DVI mode needs RGB only\r\n"));
            return FALSE;
        }
    }

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag == FALSE)
    {
        bRet = MHal_HDMITx_ColorandRange_Transform(incolor, outcolor, psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);
    }
    else
    {
        bRet = MHal_HDMITx_ColorandRange_Transform(psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color, psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);
    }

    if(inange != outrange)
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
    }
    else if(outcolor != incolor)
    {
        if(MHal_HDMITx_CSC_Support_R2Y(pInstance))
        {
            if(outcolor == E_HDMITX_VIDEO_COLOR_RGB444)//Input Y -> Output R
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
            else if(incolor != E_HDMITX_VIDEO_COLOR_RGB444)//Input Y -> Output Y
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;
            else//Input R -> Output Y
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
        }
        else
        {
            if(outcolor == E_HDMITX_VIDEO_COLOR_RGB444)//Input Y -> Output R
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
            else if(incolor != E_HDMITX_VIDEO_COLOR_RGB444)//Input Y -> Output Y
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;
            else//Input R -> Output Y
                psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;

        }
    }
    else
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;
    }

    MDrv_HDMITx_SendPacket(E_HDMITX_AVI_INFOFRAME, E_HDMITX_CYCLIC_PACKET);

    //if (psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_YUV420)
       //MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing); //wilson: for 420;

    return bRet;
}


//------------------------------------------------------------------------------
/// @brief This routine set HDMI Tx AVMUTE
/// @argument:
///              - bflag: True=> SET_AVMUTE, FALSE=>CLEAR_AVMUTE
///
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetAVMUTE(void* pInstance, MS_BOOL bflag)
{
    _HDMITX_GET_VARIABLE();

    if(!g_bDisableAvMuteCtrl)
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag = bflag;
        MDrv_HDMITx_SetGCPParameter(pInstance); //update packet content; wilson@kano

        if (bflag)
        {
            //MDrv_HDMITx_SendPacket(E_HDMITX_GC_PACKET, E_HDMITX_CYCLIC_PACKET|E_HDMITX_GCP_SET_AVMUTE);
            MDrv_HDMITx_SendPacket(E_HDMITX_GC_PACKET, E_HDMITX_CYCLIC_PACKET);
            DBG_HDMITX(printf("\nMDrv_HDMITx_SetAVMUTE \n"));
        }
        else
        {
            MDrv_HDMITx_SendPacket(E_HDMITX_GC_PACKET, E_HDMITX_CYCLIC_PACKET);
            DBG_HDMITX(printf("\nMDrv_HDMITx_ClearAVMUTE \n"));
        }
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets VS infoframe
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_Set_VS_InfoFrame(void* pInstance, MsHDMITX_VIDEO_VS_FORMAT enVSFmt, MsHDMITX_VIDEO_4k2k_VIC enVid4K2KVic, MsHDMITX_VIDEO_3D_STRUCTURE enVid3DStruct)
{
    stVSInfo_PktPara stVSPara;

    _HDMITX_GET_VARIABLE();

    stVSPara.enVSFmt = enVSFmt;
    stVSPara.en4k2kVIC = enVid4K2KVic;
    stVSPara.en3DStruct = enVid3DStruct;

    MHal_HDMITx_SetVSInfoParameter(stVSPara);
    MDrv_HDMITx_SendPacket(E_HDMITX_VS_INFOFRAME, E_HDMITX_CYCLIC_PACKET);

    //to pass compliance test of 3D output format (test with LG, SAMSUNG, Toshiba and Sony TV)
    MDrv_HDMITx_SetAVMUTE(pInstance, TRUE);
    MHal_HDMITx_SetTMDSOnOff(psHDMITXResPri->stHDMITxInfo.hdmitx_RB_swap_flag, FALSE);

    DrvHdmitxOsMsSleep(20); //delay 1 frame/field interval

    MHal_HDMITx_SetTMDSOnOff(psHDMITXResPri->stHDMITxInfo.hdmitx_RB_swap_flag, TRUE);
    MDrv_HDMITx_SetAVMUTE(pInstance, FALSE);
}

//------------------------------------------------------------------------------
/// @brief This routine sets video output mode (color/repetition/regen)
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetVideoOutputMode(void* pInstance, MsHDMITX_VIDEO_TIMING enVidTiming)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_video_timing = enVidTiming;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);

    if (psHDMITXResPri->stHDMITxInfo.output_video_timing >= E_HDMITX_RES_MAX)
    {
        printf("[%s][%d] Unsupport HDMI mode 0x%08x\n", __FUNCTION__, __LINE__, psHDMITXResPri->stHDMITxInfo.output_video_timing);
        return;
    }

    // 41.6 ms * 128 = 5.324 s, 40 ms * 128 = 5.12 s
    if( (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_1920x1080p_24Hz) || (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_1920x1080p_25Hz) || (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_1920x2205p_24Hz)
      ||(psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_24Hz) || (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_25Hz) || (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_24Hz)) // 24/25 Hz
        gHDCPCheckRiTimer = 5400;
    // 33.3 ms * 128 = 4.26 s
    else if ((psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_1920x1080p_30Hz)||(psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_30Hz)) // 30 Hz
        gHDCPCheckRiTimer = 4300;
    // 16.6 ms * 128 = 2.124 s, 20 ms * 128 = 2.56 s
    else // 50/60 Hz
        gHDCPCheckRiTimer = HDCP_RiCheck_Timer;//2600;

    MHal_HDMITx_SetVideoOutputMode(psHDMITXResPri->stHDMITxInfo.output_video_timing, psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag, psHDMITXResPri->stHDMITxInfo.output_colordepth_val, &psHDMITXResPri->stHDMITxInfo.analog_setting, psHDMITXResPri->stHDMITxInfo.ubSSCEn);
    MDrv_HDMITx_SendPacket(E_HDMITX_AVI_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
}

//------------------------------------------------------------------------------
/// @brief This routine sets video aspect ratio
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetVideoOutputAsepctRatio(void* pInstance, MsHDMITX_VIDEO_ASPECT_RATIO enOutputAR)
{
    _HDMITX_GET_VARIABLE();
    // send packet
    psHDMITXResPri->stHDMITxInfo.output_aspect_ratio = enOutputAR;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);
    MDrv_HDMITx_SendPacket(E_HDMITX_AVI_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
    if (enOutputAR == E_HDMITX_VIDEO_AR_21_9)
        MDrv_HDMITx_SendPacket(E_HDMITX_VS_INFOFRAME, E_HDMITX_STOP_PACKET);
}

//------------------------------------------------------------------------------
/// @brief This routine sets video scan info and AFD
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetVideoOutputOverscan_AFD(void* pInstance, MS_U8 ucA0, MS_BOOL bAFDOverwrite, MsHDMITX_VIDEO_SCAN_INFO enScanInfo, MsHDMITX_VIDEO_AFD_RATIO enAFDRatio)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.output_activeformat_present = ucA0;
    psHDMITXResPri->stHDMITxInfo.hdmitx_AFD_override_mode = bAFDOverwrite;
    psHDMITXResPri->stHDMITxInfo.output_scan_info = enScanInfo;
    psHDMITXResPri->stHDMITxInfo.output_afd_ratio = enAFDRatio;

    // send packet
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);
    MDrv_HDMITx_SendPacket(E_HDMITX_AVI_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
}

//------------------------------------------------------------------------------
/// @brief This routine turns On/off Audio module
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetAudioOnOff(void* pInstance, MS_BOOL state)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag = state;

    MDrv_HDMITx_SetAudInfoPktPara(pInstance);

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag) // turn on
    {
        MHal_HDMITx_SetAudioOnOff(TRUE);
        MDrv_HDMITx_SendPacket(E_HDMITX_ACR_PACKET, E_HDMITX_CYCLIC_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_AS_PACKET, E_HDMITX_CYCLIC_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_AUDIO_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ACP_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ISRC1_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ISRC2_PACKET, E_HDMITX_STOP_PACKET);
    }

    else // turn off
    {
        MHal_HDMITx_SetAudioOnOff(FALSE);
        MDrv_HDMITx_SendPacket(E_HDMITX_ACR_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_AS_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_AUDIO_INFOFRAME, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ACP_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ISRC1_PACKET, E_HDMITX_STOP_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_ISRC2_PACKET, E_HDMITX_STOP_PACKET);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets audio sampling freq.
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetAudioFrequency(void* pInstance, MsHDMITX_AUDIO_FREQUENCY enAudFreq)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_audio_frequncy = enAudFreq;
    MDrv_HDMITx_SetAudInfoPktPara(pInstance);

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag)
    {
        MHal_HDMITx_SetAudioFrequency(psHDMITXResPri->stHDMITxInfo.output_audio_frequncy, psHDMITXResPri->stHDMITxInfo.output_audio_channel, psHDMITXResPri->stHDMITxInfo.output_audio_type, psHDMITXResPri->stHDMITxInfo.output_video_timing);
        MDrv_HDMITx_SendPacket(E_HDMITX_ACR_PACKET, E_HDMITX_CYCLIC_PACKET);
        MDrv_HDMITx_SendPacket(E_HDMITX_AUDIO_INFOFRAME, E_HDMITX_CYCLIC_PACKET);
    }
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetAudioChCnt()
//  [Description]:
//                  set audio channel number
//  [Arguments]:
//			[MsHDMITX_AUDIO_CHANNEL_COUNT] enChCnt
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetAudioChCnt(void* pInstance, MsHDMITX_AUDIO_CHANNEL_COUNT enChCnt)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_audio_channel = enChCnt;
    MDrv_HDMITx_SetAudInfoPktPara(pInstance);
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetAudioFmt()
//  [Description]:
//                  set audio output format
//  [Arguments]:
//			[MsHDMITX_AUDIO_CODING_TYPE] enAudFmt
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetAudioFmt(void* pInstance, MsHDMITX_AUDIO_CODING_TYPE enAudFmt)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.output_audio_type = enAudFmt;
    MDrv_HDMITx_SetAudInfoPktPara(pInstance);
}

//------------------------------------------------------------------------------
/// @brief This routine sets audio source format.
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetAudioSourceFormat(MsHDMITX_AUDIO_SOURCE_FORMAT fmt)
{
    MHal_HDMITx_SetAudioSourceFormat(fmt);
}

//------------------------------------------------------------------------------
/// @brief This routine sets HDMI Tx HDCP encryption On/Off
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_SetHDCPOnOff(void* pInstance, MS_BOOL state)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag = state;
    MHal_HDMITx_SetHDCPOnOff(state, ((psHDMITXResPri->stHDMITxInfo.output_mode & BIT1) ? TRUE:FALSE));
}

void MDrv_HDMITx_SetHDCPFlag(void* pInstance, MS_BOOL bEnableHdcp)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag = bEnableHdcp;
}
//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetFSMState()
//  [Description]:
//                  set state of hdmi tx FSM
//  [Arguments]:
//			[MDrvHDMITX_FSM_STATE] enState
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetFSMState(void* pInstance, MDrvHDMITX_FSM_STATE enState)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = enState;
}

///@brief setdbug level
///@u16DbgSwitch  1 means hdmitx only, 2 mean
MS_BOOL  MDrv_HDMITx_SetDbgLevel(MS_U16 u16DbgSwitch)
{
    _u16DbgHDMITXSwitch = u16DbgSwitch;
    MHal_HDMITx_HdcpDebugEnable(_u16DbgHDMITXSwitch & HDMITX_DBG_HDCP ? TRUE : FALSE);
    MHal_HDMITx_UtilDebugEnable(_u16DbgHDMITXSwitch & HDMITX_DBG_UTILTX ? TRUE : FALSE);

    return TRUE;
}

void MDrv_HDMITx_SetHPDGpioPin(MS_U8 u8pin)
{
    MHal_HDMITx_SetHPDGpioPin(u8pin);
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetCECOnOff()
//  [Description]:
//                  set CEC enable flag
//  [Arguments]:
//			[MS_BOOL] bflag
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetCECOnOff(void* pInstance, MS_BOOL bflag)
{
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag = bflag;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_SetAVIInfoExtColorimetry()
//  [Description]:
//                  set extended colorimetry field of aviinfoframe packet
//  [Arguments]:
//
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_SetAVIInfoExtColorimetry(void* pInstance, MsHDMITX_EXT_COLORIMETRY enExtColorimetry, MsHDMITX_YCC_QUANT_RANGE enYccQuantRange)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.ext_colorimetry = enExtColorimetry;
    psHDMITXResPri->stHDMITxInfo.colorimetry = E_HDMITX_COLORIMETRY_EXTEND;
    psHDMITXResPri->stHDMITxInfo.enYCCQuantRange = enYccQuantRange;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);
}

//**************************************************************************
//  [Function Name]:
//      MDrv_HDMITx_SetAVIInfoExtColorimetry()
//  [Description]:
//      set extended colorimetry field of aviinfoframe packet
//  [Arguments]:
//
//  [Return]:
//
//
//**************************************************************************
MS_U8 MDrv_HDMITx_SetAVIInfoColorimetry(void* pInstance, MsHDMITX_COLORIMETRY enColorimetry)
{
    MS_U8 ubRet = TRUE;
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.colorimetry = enColorimetry;
    MDrv_HDMITx_SetAVIInfoPktPara(pInstance);

    if(psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_RGB444 )
    {
        if(psHDMITXResPri->stHDMITxInfo.colorimetry != E_HDMITX_COLORIMETRY_NO_DATA && psHDMITXResPri->stHDMITxInfo.colorimetry != E_HDMITX_COLORIMETRY_MAX)
        {
            //ubRet = FALSE;
        }
    }

    return ubRet;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_GetCECStatus()
//  [Description]:
//                  get CEC enable flag
//  [Arguments]:
//			void
//  [Return]:
//			MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_GetCECStatus(void* pInstance)
{
    MS_BOOL bRet;

    _HDMITX_DECLARE_VARIABE();
    _HDMITX_SEMAPHORE_ENTRY(pInstance);

    _HDMITX_GET_VARIABLE_WITHOUT_DECLARE();
    bRet = psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag;

    _HDMITX_SEMAPHORE_RETURN(pInstance);

    return bRet;
}

//------------------------------------------------------------------------------
/// @brief This routine will power on/off HDMITx clock (power saving)
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_Power_OnOff(MS_BOOL bEnable)
{
    MHal_HDMITx_Power_OnOff(bEnable);
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx Module
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_TurnOnOff(void *pInstance, MS_BOOL state)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_enable_flag = state;
    psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag = state;

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_enable_flag) // turn on
    {
        MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE); //wilson@kano
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE); //wilson@kano

        //MDrv_HDMITx_PLLOnOff();
    #if HDMITX_ISR_ENABLE
        MHal_HDMITx_Int_Enable(gHDMITXIRQ);
    #endif
        MHal_HDMITx_VideoInit();
        MHal_HDMITx_AudioInit();
        //MDrv_HDMITx_SetTMDSOnOff();
        //MDrv_HDMITx_Exhibit();
    }
    else // turn off
    {
        MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE); //wilson@kano
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE); //wilson@kano

    #if HDMITX_ISR_ENABLE
        MHal_HDMITx_Int_Disable(gHDMITXIRQ);
        MHal_HDMITx_Int_Clear(gHDMITXIRQ);
    #endif
        MDrv_HDMITx_SetTMDSOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag);

         psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_PENDING;
         psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PENDING;
         psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status = E_HDMITX_DVIClock_L_HPD_L;
         _hdmitx_preRX_status = psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status;

        if(_s32CheckRxTimerId > 0)
        {
            DrvHdmitxOsStopTimer(&_tCheckRxTimerCfg);
        }
    }
}

//------------------------------------------------------------------------------
/// @brief This routine control HDMI packet generation
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_EnablePacketGen(MS_BOOL bflag)
{
    MHal_HDMITx_EnablePacketGen(bflag);
}

//------------------------------------------------------------------------------
/// @brief This function clear settings of user defined packet
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_PKT_User_Define_Clear(void)
{
    MHal_HDMITx_PKT_User_Define_Clear();
}

//------------------------------------------------------------------------------
/// @brief This function set user defined hdmi packet
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_PKT_User_Define(MsHDMITX_PACKET_TYPE packet_type, MS_BOOL def_flag,
                                 MsHDMITX_PACKET_PROCESS def_process, MS_U8 def_fcnt)
{
    MHal_HDMITx_PKT_User_Define(packet_type, def_flag, def_process, def_fcnt);

    if ((packet_type == E_HDMITX_VS_INFOFRAME) )
    {
        if(def_process == E_HDMITX_CYCLIC_PACKET)
        {
            g_bDisableVSInfo = FALSE;
        }
        else
        {
           g_bDisableVSInfo = TRUE;
        }
    }

    if(_hdmitx_status.bIsInitialized) // fix code dump
    {
        MDrv_HDMITx_SendPacket(packet_type, def_process);
    }
}

//------------------------------------------------------------------------------
/// @brief This function set user defined hdmi packet content
/// @return None
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_PKT_Content_Define(MsHDMITX_PACKET_TYPE packet_type, MS_U8* data, MS_U8 length)
{
    return MHal_HDMITx_PKT_Content_Define(packet_type, data, length);
}

//------------------------------------------------------------------------------
/// @brief This routine Mute Audio FIFO
/// @param[in] bflag: True: mute audio, False: unmute audio
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_MuteAudioFIFO(MS_BOOL bflag)
{
    MHal_HDMITx_MuteAudioFIFO(bflag);
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_AnalogTuning()
//  [Description]:
//                  loading analog setting
//  [Arguments]:
//			[HDMITX_ANALOG_TUNING] *pInfo
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_AnalogTuning(void* pInstance, drvHDMITX_ANALOG_TUNING *pInfo)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_txcurrent  = pInfo->tm_txcurrent;
    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_pren2      = pInfo->tm_pren2;
    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_precon     = pInfo->tm_precon;
    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_pren       = pInfo->tm_pren;
    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_tenpre     = pInfo->tm_tenpre;
    psHDMITXResPri->stHDMITxInfo.analog_setting.tm_ten        = pInfo->tm_ten;
}


void MDrv_HDMITx_AnalogDrvCur(void *pInstance, drvHDMITX_ANALOG_DRC_CUR_CONFIG *pDrvCurCfg)
{
    _HDMITX_GET_VARIABLE();

    DBG_HDMITX(printf("AnalogDrvCur: Tap1(0x%02x, 0x%02x, 0x%02x, 0x%02x) Tap1(0x%02x, 0x%02x, 0x%02x, 0x%02x)\n",
        pDrvCurCfg->u8DrvCurTap1Ch0, pDrvCurCfg->u8DrvCurTap1Ch1, pDrvCurCfg->u8DrvCurTap1Ch2, pDrvCurCfg->u8DrvCurTap1Ch3,
        pDrvCurCfg->u8DrvCurTap2Ch0, pDrvCurCfg->u8DrvCurTap2Ch1, pDrvCurCfg->u8DrvCurTap2Ch2, pDrvCurCfg->u8DrvCurTap2Ch3));

    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap1Ch0 = pDrvCurCfg->u8DrvCurTap1Ch0;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap1Ch1 = pDrvCurCfg->u8DrvCurTap1Ch1;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap1Ch2 = pDrvCurCfg->u8DrvCurTap1Ch2;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap1Ch3 = pDrvCurCfg->u8DrvCurTap1Ch3;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap2Ch0 = pDrvCurCfg->u8DrvCurTap2Ch0;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap2Ch1 = pDrvCurCfg->u8DrvCurTap2Ch1;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap2Ch2 = pDrvCurCfg->u8DrvCurTap2Ch2;
    psHDMITXResPri->stHDMITxInfo.analog_setting.u8DrvCurTap2Ch3 = pDrvCurCfg->u8DrvCurTap2Ch3;

    MHal_HDMITx_SetAnalogDrvCur(&psHDMITXResPri->stHDMITxInfo.analog_setting);
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx output force mode
/// @argument:
///              - bflag: TRUE: force mode, FALSE: auto mode
///              - output_mode: E_HDMITX_DVI: DVI, E_HDMITX_HDMI: HDMI
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_ForceHDMIOutputMode(void* pInstance, MS_BOOL bflag, MsHDMITX_OUTPUT_MODE output_mode)
{
    _HDMITX_GET_VARIABLE();

    psHDMITXResPri->stHDMITxInfo.hdmitx_force_mode = bflag;
    if(bflag)
    {
        psHDMITXResPri->stHDMITxInfo.force_output_mode = output_mode;
        psHDMITXResPri->stHDMITxInfo.output_mode = psHDMITXResPri->stHDMITxInfo.force_output_mode;
        if((psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI) || (psHDMITXResPri->stHDMITxInfo.force_output_mode == E_HDMITX_DVI_HDCP)) // DVI mode
        {
            //gHDMITxInfo.hdmitx_audio_supportAI = FALSE;
            psHDMITXResPri->stHDMITxInfo.RxEdidInfo.AudSupportAI = FALSE;
            psHDMITXResPri->stHDMITxInfo.output_color = E_HDMITX_VIDEO_COLOR_RGB444;
            MDrv_HDMITx_SetAVIInfoPktPara(pInstance);
        }
    }
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx output force color format
/// @argument:
///              - bflag: TRUE: force output color format, FALSE: auto mode
///              - output_mode: HDMITX_VIDEO_COLOR_RGB444: RGB, HDMITX_VIDEO_COLOR_YUV444: YUV
/// @return Ture: Set force output color format successfully
///             FALSE: Fail to set force output color format
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_ForceHDMIOutputColorFormat(void* pInstance, MS_BOOL bflag, MsHDMITX_VIDEO_COLOR_FORMAT output_color)
{
    _HDMITX_DECLARE_VARIABE();
    _HDMITX_SEMAPHORE_ENTRY(pInstance);
    _HDMITX_GET_VARIABLE_WITHOUT_DECLARE();

    if (bflag)
    {
        if((output_color != E_HDMITX_VIDEO_COLOR_RGB444) && (psHDMITXResPri->stHDMITxInfo.output_mode == E_HDMITX_DVI || psHDMITXResPri->stHDMITxInfo.output_mode == E_HDMITX_DVI_HDCP))
        {
            printf("Set force output color format failed!\n");
            _HDMITX_SEMAPHORE_RETURN(pInstance);
            return FALSE;
        }

        psHDMITXResPri->stHDMITxInfo.hdmitx_force_output_color = 1;
        psHDMITXResPri->stHDMITxInfo.force_output_color = output_color;
        psHDMITXResPri->stHDMITxInfo.output_color = psHDMITXResPri->stHDMITxInfo.force_output_color;

        if( (psHDMITXResPri->stHDMITxInfo.input_color != psHDMITXResPri->stHDMITxInfo.output_color) && (psHDMITXResPri->stHDMITxInfo.output_color == E_HDMITX_VIDEO_COLOR_RGB444) )
            psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = TRUE;
        else // bypass
            psHDMITXResPri->stHDMITxInfo.hdmitx_csc_flag = FALSE;

        MDrv_HDMITx_SetAVIInfoPktPara(pInstance);
    }
    else
    {
        psHDMITXResPri->stHDMITxInfo.hdmitx_force_output_color = 0;
    }

    _HDMITX_SEMAPHORE_RETURN(pInstance);
    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_DisableRegWrite()
//  [Description]:
//                  set flag which indicate if we are going to disable register write
//  [Arguments]:
//			[MS_BOOL] bFlag
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_DisableRegWrite(MS_BOOL bFlag)
{
    g_bDisableRegWrite = bFlag;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_RxBypass_Mode()
//  [Description]:
//                  set Rx bypass mode
//  [Arguments]:
//                  [MsHDMITX_INPUT_FREQ] freq
//                  [MS_BOOL] bflag
//  [Return]:
//			MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_RxBypass_Mode(void* pInstance, MsHDMITX_INPUT_FREQ freq, MS_BOOL bflag)
{
    MS_BOOL bRet = FALSE;

    _HDMITX_DECLARE_VARIABE();
    _HDMITX_SEMAPHORE_ENTRY(pInstance);
    _HDMITX_GET_VARIABLE_WITHOUT_DECLARE();

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag == bflag)
    {
        bRet = FALSE;
    }
    else
    {
        MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE);
        MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
#if HDMITX_ISR_ENABLE
        MHal_HDMITx_Int_Disable(gHDMITXIRQ);
        MHal_HDMITx_Int_Clear(gHDMITXIRQ);
#endif
        //MDrv_HDMITx_SetTMDSOnOff();
        //MDrv_HDMITx_PLLOnOff();
        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_PENDING;
        psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag = bflag;
        if(!bflag)
        {
            psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status = E_HDMITX_DVIClock_L_HPD_L;
            _hdmitx_preRX_status = psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status;
        }

        if (_s32CheckRxTimerId > 0)
        {
            DrvHdmitxOsStopTimer(&_tCheckRxTimerCfg);
        }

        _MDrv_HDCPTx_StopRiTimer();

        bRet = MHal_HDMITx_RxBypass_Mode(freq, bflag);
    }
    _HDMITX_SEMAPHORE_RETURN(pInstance);
    return bRet;


}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_Disable_RxBypass()
//  [Description]:
//                  disable Rx bypass mode
//  [Arguments]:
//                  void
//  [Return]:
//			MS_BOOL
//
//**************************************************************************
MS_BOOL MDrv_HDMITx_Disable_RxBypass(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

#if HDMITX_ISR_ENABLE
    MHal_HDMITx_Int_Enable(gHDMITXIRQ);
#endif
    MHal_HDMITx_VideoInit();
    MHal_HDMITx_AudioInit();
    //MDrv_HDMITx_SetTMDSOnOff();
    _HDMITX_SEMAPHORE_ENTRY(pInstance);

    psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag = FALSE;

    MDrv_HDMITx_Exhibit(pInstance);

    _HDMITX_SEMAPHORE_RETURN(pInstance);

    return MHal_HDMITx_Disable_RxBypass();
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI PLL
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_PLLOnOff(void)
{
    //MHal_HDMITx_PLLOnOff(gHDMITxInfo.hdmitx_enable_flag);
}

//------------------------------------------------------------------------------
/// @brief This routine return CHIP capability of DVI mode
/// @return TRUE, FALSE
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_IsSupportDVIMode(void)
{
    return MHal_HDMITx_IsSupportDVIMode();
}

//------------------------------------------------------------------------------
/// @brief This routine return CHIP capability of HDCP mode
/// @return TRUE, FALSE
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_IsSupportHDCP(void)
{
    return MHal_HDMITx_IsSupportHDCP();
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_DisableTMDSCtrl()
//  [Description]:
//                  set flag which indicate if enable tmds control or not
//  [Arguments]:
//			[MS_BOOL] bFlag
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_DisableTMDSCtrl(MS_BOOL bFlag)
{
    g_bDisableTMDSCtrl = bFlag;
}

//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_DisableAvMuteCtrl()
//  [Description]:
//                  set flag which indicate if enable avmute control or not
//  [Arguments]:
//			[MS_BOOL] bFlag
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_DisableAvMuteCtrl(MS_BOOL bFlag)
{
    g_bDisableAvMuteCtrl = bFlag;
}



int _HPD_Isr(int eIntNum, void* pstDevParam)
{
    //printf("%s %d, HPD=%d\n", __FUNCTION__, __LINE__, DrvHdmitxOsGetGpioValue( DrvHdmitxOsGetHpdGpinPin() ));
    DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_HPD_ISR);

    return 1;
}


//**************************************************************************
//  [Function Name]:
//			MDrv_HDMITx_Exhibit()
//  [Description]:
//                  setup environment and make HDMI process start
//  [Arguments]:
//			void
//  [Return]:
//			void
//
//**************************************************************************
void MDrv_HDMITx_Exhibit(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    if (psHDMITXResPri->stHDMITxInfo.output_mode & BIT0) // check HDCP
        psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag = TRUE;
    else
        psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag = FALSE;

    if (((MDrv_HDMITx_GetColorFormat(pInstance) != E_HDMITX_VIDEO_COLOR_YUV420) && (MDrv_HDMITx_GetColorFormat(pInstance) != E_HDMITX_VIDEO_COLOR_YUV422)) &&
        ((MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_1920x2205p_24Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_1280x1470p_50Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_1280x1470p_60Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_3840x2160p_50Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_3840x2160p_60Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_4096x2160p_50Hz) ||
        (MDrv_HDMITx_GetOutputTiming(pInstance) == E_HDMITX_RES_4096x2160p_60Hz)))
    {
        psHDMITXResPri->stHDMITxInfo.output_colordepth_val = E_HDMITX_VIDEO_CD_24Bits;
    }

    MDrv_HDMITx_SetDeepColorMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_colordepth_val);
    MDrv_HDMITx_SetColorFormat(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color);
    MDrv_HDMITx_SetHDMITxMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_mode);
    MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing);

    MDrv_HDMITx_SetAudioFrequency(pInstance, psHDMITXResPri->stHDMITxInfo.output_audio_frequncy);

    // The 1st HDCP authentication
    if (MDrv_HDCPTx_GetAuthDoneFlag(pInstance) == FALSE)
    {
         if( (!MDrv_HDMITx_GetEdidRdyFlag(pInstance)) || (psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status != E_HDMITX_DVIClock_H_HPD_H))
        {
            MDrv_HDMITx_SetFSMState(pInstance, E_HDMITX_FSM_VALIDATE_EDID);
            MDrv_HDCPTx_SetRxValid(pInstance, FALSE);
        }
        _MDrv_HDCPTx_StopRiTimer();

        psHDMITXResPri->stHDMITxInfo.output_video_prevtiming = psHDMITXResPri->stHDMITxInfo.output_video_timing;
        //psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status = E_HDMITX_DVIClock_L_HPD_L;
        //_hdmitx_preRX_status = psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status;

        if (psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag)
        {
            MDrv_HDCPTx_StartAuth(pInstance, TRUE);
        }

        // Create timer for timing monitor
            if(_s32CheckRxTimerId < 0)
            {
                if(g_bHpdIrqEn)
                {
                    DrvHdmitxOsGpioIrq(DrvHdmitxOsGetHpdGpinPin(), (InterruptCb)_HPD_Isr, 1);
                }

                _tCheckRxTimerCfg.s32Id = -1;
                _tCheckRxTimerCfg.u32TimerOut = CheckRx_Timer;
                _tCheckRxTimerCfg.pfnFunc = _MDrv_HDMITX_CheckRx_TimerCallback;
                if(DrvHdmitxOsCreateTimer(&_tCheckRxTimerCfg))
                {
                    _s32CheckRxTimerId = _tCheckRxTimerCfg.s32Id;
                }

                psHDMITXResPri->stHDMITxInfo.bCheckRxTimerIdCreated = TRUE;

                if(_s32CheckRxTimerId < 0)
                {
                    DBG_HDMITX(printf("MDrv_HDMITX_Init: Create Check Rx timer fail!\n"));
                    psHDMITXResPri->stHDMITxInfo.bCheckRxTimerIdCreated = FALSE;
                }

                DBG_HDMITX(printf("MDrv_HDMITx_Exhibit: Create Check Rx timer success!\n"));
            }
            else
            {
                DrvHdmitxOsStopTimer(&_tCheckRxTimerCfg);
                DrvHdmitxOsStartTimer(&_tCheckRxTimerCfg);

                if(g_bHpdIrqEn)
                {
                    DrvHdmitxOsGpioIrq(DrvHdmitxOsGetHpdGpinPin(), (InterruptCb)_HPD_Isr, 0);
                    DrvHdmitxOsGpioIrq(DrvHdmitxOsGetHpdGpinPin(), (InterruptCb)_HPD_Isr, 1);

                    if(psHDMITXResPri->stHDMITxInfo.hdmitx_enable_flag)
                    {
                        // To trig _Hdmitx_Task when don turnoff and the turn on, but no hpd interrupt
                        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_HPD_ISR);
                    }
                }
            }
        //return;
    }

    // HDCP re-authentication while timing changed or input source changed
    //else if((gHDMITxInfo.output_video_prevtiming != gHDMITxInfo.output_video_timing) && gHDMITxInfo.hdmitx_hdcp_flag)
    else if (psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag)
    {
        //MDrv_HDCPTx_SetRxValid(pInstance, FALSE);
        _MDrv_HDCPTx_StopRiTimer();
        MDrv_HDCPTx_StartAuth(pInstance, TRUE);
        psHDMITXResPri->stHDMITxInfo.output_video_prevtiming = psHDMITXResPri->stHDMITxInfo.output_video_timing;

        DBG_HDMITX(printf("MDrv_HDMITx_Exhibit: HDCP reauthentication!!\n"));
    }
    else // turn off HDCP encryption
    {
        MDrv_HDCPTx_StartAuth(pInstance, FALSE);//MDrv_HDCPTx_SetRxValid(pInstance, FALSE);
        _MDrv_HDCPTx_StopRiTimer();

        MHal_HDMITx_HdcpSetEncrypt(DISABLE);
        DBG_HDMITX(printf("MDrv_HDMITx_Exhibit: Turn off HDCP!!\n"));
    }

    if (MDrv_HDCPTx_GetAuthDoneFlag(pInstance) == TRUE)
    {
        MDrv_HDMITx_SetVideoOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag);
        MDrv_HDMITx_SetAudioOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag);
    }
    //MDrv_HDMITx_SetHDCPOnOff();
    //MDrv_HDMITx_SetTMDSOnOff();

    // Fix garbage while turn HDCP ON -> OFF
    if( ((MDrv_HDCPTx_GetAuthDoneFlag(pInstance) == TRUE)||(MDrv_HDMITx_GetFSMState(pInstance) == E_HDMITX_FSM_DONE)) && (!psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag) )
    {
        if (psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag)
        {
            MDrv_HDMITx_SetAVMUTE(pInstance, FALSE);
            psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag = FALSE;
        }
    }
}

MS_BOOL MDrv_HDMITx_CheckOver3G(void* pInstance)
{
    _HDMITX_GET_VARIABLE();

    if ((psHDMITXResPri->stHDMITxInfo.output_color != E_HDMITX_VIDEO_COLOR_YUV420) &&
        (
            (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_50Hz) ||
            (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_60Hz) ||
            (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_50Hz) ||
            (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_60Hz) ||
            (
                (
                    (psHDMITXResPri->stHDMITxInfo.output_colordepth_val != E_HDMITX_VIDEO_CD_24Bits) && \
                    (psHDMITXResPri->stHDMITxInfo.output_colordepth_val != E_HDMITX_VIDEO_CD_NoID)
                ) &&
                (
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_24Hz) || \
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_25Hz) || \
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_3840x2160p_30Hz) || \
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_24Hz) || \
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_25Hz) || \
                    (psHDMITXResPri->stHDMITxInfo.output_video_timing == E_HDMITX_RES_4096x2160p_30Hz)
                )
            )
         )
        )
        return TRUE;
    else
        return FALSE;
}

//**************************************************************************
//  [Function Name]:
//			HDMITx_Handler()
//  [Description]:
//                  HDMI TX FSM
//  [Arguments]:
//			[MS_U32] u32State
//  [Return]:
//			void
//
//**************************************************************************
void HDMITx_Handler(void* pInstance, MS_U32 u32State) //wilson@kano
{
    static MS_U32 u32HpdOnTime, u32TransmitTime, u32ValidateEdidTime;
    static MS_U32 bHpdRestart = 0;
    MS_U32 u32IRQStatus;

    _HDMITX_GET_VARIABLE();

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag)
    {
        //RXTXBypass
        return;
    }

    if (u32State & (E_HDMITX_EVENT_RXTIMER|E_HDMITX_EVENT_HPD_ISR))
    {
        MsHDMITX_RX_STATUS enCurRxStatus;

        if(u32State & E_HDMITX_EVENT_HPD_ISR)
        {
            enCurRxStatus = MHal_HDMITx_GetRXStatus();
            gbCurRxStatus = enCurRxStatus;
        }
        else
        {
            enCurRxStatus = gbCurRxStatus;
        }

        if (psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status != enCurRxStatus)
        {
            DBG_HDMITX(printf("E_HDMITX_FSM_CHECK_HPD:: pre= %d, cur = %d\n", psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status, enCurRxStatus));

            MDrv_HDCPTx_SetRxValid(pInstance, FALSE);
            _MDrv_HDCPTx_StopRiTimer();
            MHal_HDMITx_HdcpSetEncrypt(DISABLE);

            if ((enCurRxStatus == E_HDMITX_DVIClock_H_HPD_H) || (enCurRxStatus == E_HDMITX_DVIClock_L_HPD_H))
            {
                bHpdRestart = 1;
                u32HpdOnTime = DrvHdmitxOsGetSystemTime();
                // update EDID when (1) HPD L -> H or (2) DVI clock L -> H with HPD always high
                bCheckEDID = TRUE;
                psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_VALIDATE_EDID;
                psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = FALSE;

                if(enCurRxStatus == E_HDMITX_DVIClock_H_HPD_H)
                {
                    psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag = TRUE;
                }

                // some monitor will reset HPD when TMDS is disabled too long.
                // enable TMDS immediate when HPD=1 to resolve it.
                DBG_HDMITX(printf(PRINT_YELLOW "%s %d, TMDS=%x\n" PRINT_NONE, __FUNCTION__, __LINE__, MHal_HDMITx_Read(0x112600, 0x16)));
                MDrv_HDMITx_SetTMDSOnOff(pInstance, TRUE);
                DBG_HDMITX(printf(PRINT_YELLOW "%s %d, TMDS=%x\n" PRINT_NONE, __FUNCTION__, __LINE__, MHal_HDMITx_Read(0x112600, 0x16)));

            }
            else
            {
                bCheckEDID = FALSE;
                psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PENDING;
                //gHDMITxInfo.hdmitx_tmds_flag = FALSE;
                psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = FALSE;

                MHal_HDMITx_HdcpSetEncrypt(DISABLE); // HDCP test
                MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
                MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE);
                MDrv_HDMITx_SetTMDSOnOff(pInstance, FALSE);
            }

            psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status = enCurRxStatus;
            _hdmitx_preRX_status = psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status;

            DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
        }
        else // To avoid two events are coming together - HDCP CTS 1A-02
        {
            if (u32State & E_HDMITX_EVENT_RUN)
                DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
        }
    }
    else if (u32State & E_HDMITX_EVENT_RUN)
    {
        if ((psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status == E_HDMITX_DVIClock_H_HPD_H) || (psHDMITXResPri->stHDMITxInfo.hdmitx_preRX_status == E_HDMITX_DVIClock_L_HPD_H)) //rx attached
        {
            if (psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state != psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate)
            {
                DBG_HDMITX(printf("HDMI handler preState:curState = %d : %d\r\n", psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate, psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state));
            }
            psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state; //update state

            switch (psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state)
            {
                case E_HDMITX_FSM_PENDING: //idle state
                    {
                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_PENDING;
                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PENDING;
                    }
                break;

            	case E_HDMITX_FSM_VALIDATE_EDID:
                    {
                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_VALIDATE_EDID;
                        psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = FALSE;

                        MHal_HDMITx_HdcpSetEncrypt(DISABLE);
                        MDrv_HDCPTx_SetFSMState(pInstance, HDCP14Tx_MainState_A0, HDCP14Tx_SubFSM_IDLE);

                        if (MDrv_HDMITx_EdidChecking(pInstance))
                        {
                            DBG_HDMITX(printf("\nMDrv_HDMITX_Process()::EDID OK\n"));
                            Mhal_HDMITx_SetSCDCCapability(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportSCDC);

                            psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready = TRUE;

                            //gHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PRE_TRANSMIT; //for future usage
                            MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing);
                            psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_TRANSMIT;

                            //MDrv_HDCPTx_SetRxValid(pInstance, TRUE);
                            if (psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag != TRUE) // HDCP off
                            {
                                MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE);
                                MDrv_HDMITx_SetHDCPOnOff(pInstance, FALSE);
                                _MDrv_HDCPTx_StopRiTimer();
                            }
                        }
                        else //invalid EDID
                        {
                            Mhal_HDMITx_SetSCDCCapability(FALSE);
                            DBG_HDMITX(printf("\nMDrv_HDMITX_Process()::EDID FAIL!!\n"));
                            MDrv_HDCPTx_SetRxValid(pInstance, FALSE);

                            psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_PENDING;
                        }

                        u32ValidateEdidTime = DrvHdmitxOsGetSystemTime();

                        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
                    }
                break;

            	case E_HDMITX_FSM_PRE_TRANSMIT:
                    {
                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_PRE_TRANSMIT;
                    }
                break;

            	case E_HDMITX_FSM_TRANSMIT:
                    {
                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_TRANSMIT;

                        MDrv_HDMITx_SetHDMITxMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_mode);
                        MDrv_HDMITx_SetColorFormat(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color);
                        //MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing);
                        MDrv_HDMITx_SetAudioFrequency(pInstance, psHDMITXResPri->stHDMITxInfo.output_audio_frequncy);
                        MDrv_HDCPTx_SetRxValid(pInstance, TRUE);

                        if ((MDrv_HDCPTx_GetUnHdcpControl(pInstance) == E_NORMAL_OUTPUT)
                                ||(psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag == FALSE))
                        {
                            MDrv_HDMITx_SetVideoOnOff(pInstance, TRUE); //psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag = TRUE;
                            MDrv_HDMITx_SetAudioOnOff(pInstance, TRUE);
                        }
                        else
                        {
                            MDrv_HDMITx_SetAVMUTE(pInstance, TRUE);
                            //Fix Panasonic UITA 2000 HDCP test fail, Video should be show after ENC_EN was detects
                            MDrv_HDMITx_SetVideoOnOff(pInstance, FALSE); //psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag = FALSE;
                            MDrv_HDMITx_SetAudioOnOff(pInstance, FALSE);//psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag = FALSE;
                        }


                        // Enahbe TMDS in HPD=1
                        //MDrv_HDMITx_SetTMDSOnOff(pInstance, TRUE);

                        if (psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag == FALSE)
                        {
                            if (psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag == TRUE)
                            {
                                MDrv_HDMITx_SetAVMUTE(pInstance, FALSE);
                            }
                        }
                        else
                        {
                            MDrv_HDCPTx_SetAuthStartFlag(pInstance, TRUE);
                            if (psHDMITXResPri->stHDMITxInfo.output_mode & BIT0)
                            {
                                DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HdcpTX_EVENT_RUN); //start hdcp authentication flow
                            }
                        }

                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_DONE;

                        u32TransmitTime = DrvHdmitxOsGetSystemTime();
                        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);

                    }
                break;

            	case E_HDMITX_FSM_DONE:
                    {
                        #if 0
                        MS_U8 ucSCDCStatus = 0x00;

                        psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_prestate = E_HDMITX_FSM_DONE;
                        //MDrv_HDCPTx_SetFSMState(pInstance, HDCP14Tx_MainState_A0, HDCP14Tx_SubFSM_IDLE);

                        //TBD: adding SCDC checking routine here;
                        if (Mhal_HDMITx_SCDCAccessField(E_SCDC_SCRAMBLE_STAT_IDX, TRUE, &ucSCDCStatus, 0x01))
                        {
                            //TBD
                        }
                        #endif

                        if(bHpdRestart)
                        {
                            DBG_HDMITX(printf(PRINT_YELLOW "%s %d, HpdTIme=%d, EdidTime:%d, TransmitTime=%d, DoneTime=%d\n" PRINT_NONE,
                                __FUNCTION__, __LINE__,
                                u32HpdOnTime,
                                u32ValidateEdidTime - u32HpdOnTime,
                                u32TransmitTime - u32HpdOnTime,
                                DrvHdmitxOsGetSystemTime()-u32HpdOnTime));
                            bHpdRestart =0;
                        }
                        DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
                    }
                break;

                default:
                break;
            }
        }

        //DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);

    }
    else if (u32State & E_HDMITX_EVENT_IRQ)
    {
        u32IRQStatus = MHal_HDMITx_Int_Status();

        if (u32IRQStatus & E_HDMITX_IRQ_12) // TMDS hot-plug
        {
            DBG_HDMITX(printf("MDrv_HDMITX_Process()::E_HDMITX_IRQ_12\n"));
            //gHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_CHECK_HPD;
            MHal_HDMITx_Int_Clear(E_HDMITX_IRQ_12);
            //DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
        }
        else if (u32IRQStatus & E_HDMITX_IRQ_10) // Rx disconnection
        {
            DBG_HDMITX(printf("MDrv_HDMITX_Process()::E_HDMITX_IRQ_10\n"));
            //gHDMITxInfo.hdmitx_fsm_state = E_HDMITX_FSM_CHECK_HPD;
            MHal_HDMITx_Int_Clear(E_HDMITX_IRQ_10);
            //DrvHdmitxOsSetEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN);
        }
    }
}

//**************************************************************************
//  [Function Name]:
//			()
//  [Description]:
//
//  [Arguments]:
//
//  [Return]:
//
//
//**************************************************************************
static INTERFACED void* pu32HDMITxTaskInst = NULL;
extern MS_BOOL _HDMITXOpen(void** ppInstance);

static void _HDMITx_Task(void* pInstance) //wilson@kano
{
    MS_U32                 u32Events = 0;

    //_HDMITX_DECLARE_VARIABE();

    while (g_bHDMITxTask == TRUE)
    {
        if (pu32HDMITxTaskInst == NULL)
        {
            if (_HDMITXOpen(&pu32HDMITxTaskInst)!= TRUE)
            {
                DBG_HDMITX(printf("%s: Get Task Instance FAIL!\n", __FUNCTION__));
                return;
            }
        }
        pInstance = pu32HDMITxTaskInst;

#if HDMITX_ISR_ENABLE
        // Task wake up
        //DrvHdmitxOsWaitEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN|E_HDMITX_EVENT_IRQ, &u32Events, E_DRV_HDMITX_OS_EVENT_MD_OR_CLEAR, MSOS_WAIT_FOREVER);
        //Loop polling MAD audio sampling rate and config it for HDMI TX
        DrvHdmitxOsWaitEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN|E_HDMITX_EVENT_IRQ|E_HDMITX_EVENT_RXTIMER|E_HDMITX_EVENT_CECRX | E_HdcpTX_EVENT_RUN | E_HdcpTX_EVENT_IRQ | E_HdcpTX_EVENT_RITIMER, &u32Events,  E_DRV_HDMITX_OS_EVENT_MD_OR_CLEAR, 5000);

        if (1)
        {
#else
            DrvHdmitxOsWaitEvent(_s32HDMITxEventId, E_HDMITX_EVENT_RUN|E_HDMITX_EVENT_IRQ|E_HDMITX_EVENT_RXTIMER|E_HDMITX_EVENT_HPD_ISR|E_HDMITX_EVENT_CECRX | E_HdcpTX_EVENT_RUN | E_HdcpTX_EVENT_IRQ | E_HdcpTX_EVENT_RITIMER, &u32Events, E_DRV_HDMITX_OS_EVENT_MD_OR_CLEAR, 5000);
#endif
            _HDMITX_SEMAPHORE_ENTRY(pInstance);
            //_HDMITX_GET_VARIABLE_WITHOUT_DECLARE();

            HDMITx_Handler(pInstance, u32Events);

            if ( (u32Events & E_HdcpTX_EVENT_RUN) | (u32Events & E_HdcpTX_EVENT_IRQ) | (u32Events & E_HdcpTX_EVENT_RITIMER))
            {
                if (MDrv_HDCPTx_GetAuthStartFlag(pInstance) == TRUE)
                {
                    HDCPTx_Handler(pInstance, u32Events);
                }
            }

#if HDMITX_ISR_ENABLE
        }
        else
        {
            DBG_HDMITX(printf("%s: obtain mutex failed.\n", __FUNCTION__));
        }

        // Even if not E_DMD_EVENT_ENABLE, the interrupt should be enable.
        DrvHdmitxOsEnableInterrupt(E_INT_IRQ_HDMI_LEVEL);
#endif

        _HDMITX_SEMAPHORE_RETURN(pInstance);

        DrvHdmitxOsMsSleep(HDMITX_MONITOR_DELAY);
    } // Task loop
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
// HDMI Tx output is DVI / HDMI mode
//------------------------------------------------------------------------------
/// @return None
//------------------------------------------------------------------------------
void MDrv_HDMITx_InitSeq(void* pInstance)
{
    //MHal_HDMITx_SetChipVersion((MDrv_SYS_GetChipRev() >= 0x01) ? (MS_U8)0x01 : (MS_U8)0x00); // U01: 0x00; U02: 0x01
    MHal_HDMITx_InitSeq();
    MDrv_HDMITx_InitVariable(pInstance);
    MHal_HDMITx_Init_PadTop();
}

//------------------------------------------------------------------------------
/// @brief This routine initializes HDMI Tx module
/// @return None
/// HDMI driver is in @ref E_TASK_PRI_HIGH level
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_Init(void *pInstance)//MS_BOOL MDrv_HDMITx_Init(void)
{
    //MS_VIRT u32PMRIUBaseAddress = 0;
    //MS_PHY u32PMBankSize = 0;

    _HDMITX_GET_VARIABLE();

    _hdmitx_status.bIsInitialized = TRUE;
    _hdmitx_status.bIsRunning     = TRUE;

    DBG_HDMITX(printf("%s %d\n", __FUNCTION__, __LINE__));

    if(!DrvHdmitxOsGetMmioBase(&psHDMITXResPri->stHDMITxInfo.u32PMRIUBaseAddress, &psHDMITXResPri->stHDMITxInfo.u32PMBankSize, E_HDMITX_OS_MMIO_PM ))
    {
        printf("MApi_XC_Init Get PM BASE failure\n");
        return FALSE;
    }

    //MS_VIRT u32CoproRIUBase = 0x00;
    if( !DrvHdmitxOsGetMmioBase(&psHDMITXResPri->stHDMITxInfo.u32CoproRIUBase, &psHDMITXResPri->stHDMITxInfo.u32PMBankSize, E_HDMITX_OS_MMIO_NONEPM))
    {
        printf("_COPRO_Init: IOMap failure\n");
        return FALSE;
    }
    else
    {
        MHal_HDMITx_SetIOMapBase(psHDMITXResPri->stHDMITxInfo.u32PMRIUBaseAddress, psHDMITXResPri->stHDMITxInfo.u32PMRIUBaseAddress, psHDMITXResPri->stHDMITxInfo.u32CoproRIUBase);
    }

    MDrv_HDMITx_Power_OnOff(TRUE);

    if (psHDMITXResPri->bInit == TRUE)
    {
        DBG_HDMITX(printf("MDrv_HDMITx_Init: bInit already initialized.\n"));
        return TRUE;
    }

    if (psHDMITXResPri->stHDMITxInfo.bHDMITxTaskIdCreated) //if (_s32HDMITxTaskId >= 0)
    {
        DBG_HDMITX(printf("MDrv_HDMITx_Init: bHDMITxTaskIdCreated already initialized.\n"));
        return TRUE;
    }

#if HDMITX_ISR_ENABLE
    DrvHdmitxOsDisableInterrupt(E_INT_IRQ_HDMI_LEVEL);
    DrvHdmitxOsDetachInterrupt(E_INT_IRQ_HDMI_LEVEL);

    MHal_HDMITx_Int_Disable(gHDMITXIRQ);
    MHal_HDMITx_Int_Clear(gHDMITXIRQ);

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag)
    {
    #if 0 //ENABLE_CEC_INT
        DrvHdmitxOsDisableInterrupt(E_INT_IRQ_CEC);
        DrvHdmitxOsDetachInterrupt(E_INT_IRQ_CEC);
    #endif
    }

#endif // #if HDMITX_ISR_ENABLE

    MDrv_HDMITx_InitSeq(pInstance);

    if (psHDMITXResPri->stHDMITxInfo.bHDMITxEventIdCreated == FALSE)
    {
        _s32HDMITxEventId = DrvHdmitxOsCreateEventGroup("HDMITx_Event");
        psHDMITXResPri->stHDMITxInfo.bHDMITxEventIdCreated = TRUE;
    }

    if (_s32HDMITxEventId < 0)
    {
        ///MsOS_DeleteMutex(_s32HDMITxMutexId);
        psHDMITXResPri->stHDMITxInfo.bHDMITxEventIdCreated = FALSE;
        return FALSE;
    }

    _pHDMITxTaskStack = _u8HDMITx_StackBuffer;
    g_bHDMITxTask = TRUE;

    _HDMITX_SEMAPHORE_RETURN(pInstance);


    if(DrvHdmitxOsCreateTask(&_tHDMITxTask,
                             (TaskEntryCb)_HDMITx_Task,
                             NULL,
                             "HDMITx_Task",
                             TRUE))
    {
        _s32HDMITxTaskId = _tHDMITxTask.s32Id;
    }

    _HDMITX_SEMAPHORE_ENTRY(pInstance);

    psHDMITXResPri->stHDMITxInfo.bHDMITxTaskIdCreated = TRUE;

    if (_s32HDMITxTaskId < 0)
    {
        printf("create hdmi task failed\r\n");
        DrvHdmitxOsDeleteEventGroup(_s32HDMITxEventId);
        ///MsOS_DeleteMutex(_s32HDMITxMutexId);
        psHDMITXResPri->stHDMITxInfo.bHDMITxTaskIdCreated = FALSE;
        return FALSE;
    }

#if HDMITX_ISR_ENABLE
    DrvHdmitxOsAttachInterrupt(E_INT_IRQ_HDMI_LEVEL, _HDMITx_Isr);
    DrvHdmitxOsEnableInterrupt(E_INT_IRQ_HDMI_LEVEL);                                    // Enable TSP interrupt
    //MDrv_HDMITx_Int_Enable();

    if (psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag)
    {
    }
#endif // #if HDMITX_ISR_ENABLE


    if(Mhal_HDMITx_GetSwI2cEn() == FALSE)
    {
        DBG_HDMITX(printf("%s %d, HW I2c\n", __FUNCTION__,__LINE__));
        if(DrvHdmitxOsSetI2cAdapter( DrvHdmitxOsGetI2cId() ) == 0) //ToDo bus number ??
        {
            printf("%s %d, I2C Adapter fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }
    else
    {
        DBG_HDMITX(printf("%s %d, SW I2c\n", __FUNCTION__,__LINE__));
        DrvHdmitxOsGpioRequestOutput( Mhal_HDMITx_GetSwI2cSdaPin() );
        DrvHdmitxOsGpioRequestOutput( Mhal_HDMITx_GetSwI2cSclPin() );
        DrvHdmitxOsSetGpioValue(Mhal_HDMITx_GetSwI2cSdaPin(), 1); // keep sda to high
        DrvHdmitxOsSetGpioValue(Mhal_HDMITx_GetSwI2cSclPin(), 1); // keep scl to high
    }

    return TRUE;
}

//------------------------------------------------------------------------------
/// @brief This routine initializes HDMI + HDCP Tx module
/// @return None
/// HDMI, HDCP driver is in @ref E_TASK_PRI_HIGH level
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITxHDCPTx_Init(void* pInstance) //wilson@kano
{
    DBG_HDCP(printf("start hdmi, hdcp init\r\n"));
    if (MDrv_HDMITx_Init(pInstance) == TRUE)
    {
        DBG_HDMITX(printf("hdmitx init done\r\n"));

        if (MDrv_HDCPTx_Init(pInstance) == TRUE)
        {
            DBG_HDCP(printf("hdcptx init done\r\n"));
            return TRUE;
        }
        else
        {
            printf("hdcptx init fail\r\n");
            return FALSE;
        }
    }
    else
    {
        printf("hdmitx init fail\r\n");
        return FALSE;
    }
}

//------------------------------------------------------------------------------
/// @brief to speed up the process of resume from power saving mode
/// @return power state
//------------------------------------------------------------------------------
MS_U32 MDrv_HDMITx_SetPowerState(void* pInstance, MsHDMITX_POWER_MODE u16PowerState)
{
    static MsHDMITX_POWER_MODE _prev_u16PowerState = E_HDMITX_POWER_MODE_MECHANICAL;
    MS_U32 u32Return = 1;
	static MDrvHDMITX_PARAMETER_LIST preHDMITxInfo;

    _HDMITX_GET_VARIABLE();

    if ((psHDMITXResPri->stHDMITxInfo.bHDMITxTaskIdCreated == FALSE) ||
        (psHDMITXResPri->stHDMITxInfo.bHDMITxEventIdCreated == FALSE) ||
        (psHDMITXResPri->stHDMITxInfo.bCheckRxTimerIdCreated == FALSE))
    {
        printf("[%s,%5d] HDMI TX is not initialized!!\r\n", __FUNCTION__, __LINE__);
        return u32Return;
    }

    if (u16PowerState == E_HDMITX_POWER_MODE_SUSPEND)
    {
        _prev_u16PowerState = u16PowerState;
		//store previous state
       	memcpy(&preHDMITxInfo, &psHDMITXResPri->stHDMITxInfo, sizeof(psHDMITXResPri->stHDMITxInfo));


        #if HDMITX_ISR_ENABLE
            DrvHdmitxOsDisableInterrupt(E_INT_IRQ_HDMI_LEVEL);
            DrvHdmitxOsDetachInterrupt(E_INT_IRQ_HDMI_LEVEL);

            MHal_HDMITx_Int_Disable(gHDMITXIRQ);
            MHal_HDMITx_Int_Clear(gHDMITXIRQ);

            if(psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag)
            {
                #if 0 //ENABLE_CEC_INT
                DrvHdmitxOsDisableInterrupt(E_INT_IRQ_CEC);
                DrvHdmitxOsDetachInterrupt(E_INT_IRQ_CEC);
                #endif
            }
        #endif // #if HDMITX_ISR_ENABLE

        MDrv_HDMITx_Power_OnOff(FALSE);
        u32Return = 0;// UTOPIA_STATUS_SUCCESS;//SUSPEND_OK;
    }
    else if (u16PowerState == E_HDMITX_POWER_MODE_RESUME)
    {

        if (_prev_u16PowerState == E_HDMITX_POWER_MODE_SUSPEND)
        {
			//store back tx info
			MDrv_HDMITx_Power_OnOff(TRUE);
            MDrv_HDMITx_InitSeq(pInstance);
            MDrv_HDCPTx_InitVariable(pInstance);

            #if HDMITX_ISR_ENABLE
                DrvHdmitxOsAttachInterrupt(E_INT_IRQ_HDMI_LEVEL, _HDMITx_Isr);
                DrvHdmitxOsEnableInterrupt(E_INT_IRQ_HDMI_LEVEL);                                    // Enable TSP interrupt
                //MDrv_HDMITx_Int_Enable();

                if (psHDMITXResPri->stHDMITxInfo.hdmitx_CECEnable_flag)
                {
                    #if 0 //ENABLE_CEC_INT
                    DBG_HDMITX(printf("\nEnable CEC INT!!\r\n"));
                    DrvHdmitxOsAttachInterrupt(E_INT_IRQ_CEC, _HDMITX_CEC_RxIsr);
                    DrvHdmitxOsEnableInterrupt(E_INT_IRQ_CEC);
                    #endif
                }
            #endif // #if HDMITX_ISR_ENABLE

			memcpy(&psHDMITXResPri->stHDMITxInfo, &preHDMITxInfo, sizeof(psHDMITXResPri->stHDMITxInfo));

            MDrv_HDCPTx_SetAuthStartFlag(pInstance, FALSE);
            MDrv_HDCPTx_SetAuthDoneFlag(pInstance, FALSE);
            MDrv_HDMITx_TurnOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_enable_flag);
            MDrv_HDMITx_SetColorFormat(pInstance, psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color);
            MDrv_HDMITx_SetVideoOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag);
            MDrv_HDMITx_SetHDMITxMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_mode);
            MDrv_HDMITx_SetVideoOutputMode(pInstance, psHDMITXResPri->stHDMITxInfo.output_video_timing);
            MDrv_HDMITx_SetAudioFrequency(pInstance, psHDMITXResPri->stHDMITxInfo.output_audio_frequncy);
            MDrv_HDMITx_SetAudioOnOff(pInstance, psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag);
            MDrv_HDMITx_Exhibit(pInstance);

            _prev_u16PowerState = u16PowerState;
            u32Return = 0;//UTOPIA_STATUS_SUCCESS;//RESUME_OK;
        }
        else
        {
            u32Return = 1;//UTOPIA_STATUS_FAIL;//SUSPEND_FAILED;
        }
    }
    else
    {
        u32Return = 1;//UTOPIA_STATUS_FAIL;
    }

    return u32Return;// for success
}

//------------------------------------------------------------------------------
/// @brief to compare ri value
/// @return ri is match or not
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_Hdcp1xCompareRi(void)
{
    MS_U16 ptx_ri = 0, prx_ri = 0;

    return MHal_HDMITx_HdcpCheckRi(&ptx_ri, &prx_ri);
}

//------------------------------------------------------------------------------
/// @brief to speed up the process of resume from power saving mode
/// @return power state
//------------------------------------------------------------------------------
MS_BOOL MDrv_HDMITx_SSC_Enable(void* pInstance, MS_U8 ubSSCEn)
{
    //pInstance = pInstance;
    _HDMITX_GET_VARIABLE();
    psHDMITXResPri->stHDMITxInfo.ubSSCEn = ubSSCEn;

    return psHDMITXResPri->stHDMITxInfo.ubSSCEn;
}

//------------------------------------------------------------------------------
/// @brief Check legitimacy of user setting of timing
/// @return error code
//------------------------------------------------------------------------------
MsHDMITX_TIMING_ERROR MDrv_HDMITx_CheckLegalTiming(void* pInstance, MsHDMITX_OUTPUT_MODE eOutputMode, MsHDMITX_VIDEO_TIMING idx, MsHDMITX_VIDEO_COLOR_FORMAT incolor_fmt, MsHDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, MsHDMITX_VIDEO_COLORDEPTH_VAL color_depth)
{
    MS_U32 uiPxlClk = 0;
    MsHDMITX_AVI_VIC eVIC = E_HDMITX_VIC_NOT_AVAILABLE;
    MsHDMITX_TIMING_ERROR err = E_HDMITX_TIMING_ERR_NONE;

    //pInstance = pInstance;
    _HDMITX_GET_VARIABLE();

    if(psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag)
    {
        //return E_HDMITX_TIMING_ERR_CFG_ERR;
        DBG_HDMITX(printf("bypass mode\r\n"));
        err |= E_HDMITX_TIMING_ERR_CFG_ERR;
    }

    if(MHal_HDMITx_GetRXStatus() != E_HDMITX_DVIClock_H_HPD_H)
    {
        //return E_HDMITX_TIMING_ERR_CFG_ERR;
        DBG_HDMITX(printf("RXstatus Low\r\n"));
        err |= E_HDMITX_TIMING_ERR_CFG_ERR;
    }

    if(bCheckEDID)
    {
        //if(!MDrv_HDMITx_EdidChecking(pInstance))
        {
            //return E_HDMITX_TIMING_ERR_EDID_ERR;
            DBG_HDMITX(printf("EDID falied\r\n"));
            err |= E_HDMITX_TIMING_ERR_EDID_ERR;
        }
    }

    if(!psHDMITXResPri->stHDMITxInfo.RxEdidInfo.SupportHdmi)
    {
        if( (eOutputMode == E_HDMITX_HDMI) || (eOutputMode == E_HDMITX_HDMI_HDCP) )
        {
            err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
        }
    }

    uiPxlClk = MHal_HDMITx_GetPixelClk_ByTiming(idx, outcolor_fmt, color_depth);
    eVIC = _MDrv_HDMIRx_MapVideoTimingToVIC(idx);

    err |= MDrv_HDMITx_CheckEDIDTiming(pInstance,eVIC, uiPxlClk, incolor_fmt, outcolor_fmt, color_depth);
    err |= MDrv_HDMITx_CheckEDIDColorFormat(pInstance, idx, eVIC, incolor_fmt, outcolor_fmt, color_depth);
    err |= MDrv_HDMITx_CheckEDIDColorDepth(pInstance, outcolor_fmt, color_depth);

    return err;
}

MsHDMITX_TIMING_ERROR MDrv_HDMITx_CheckEDIDTiming(void* pInstance, MsHDMITX_AVI_VIC eVIC, MS_U32 uiPxlClk, MsHDMITX_VIDEO_COLOR_FORMAT incolor_fmt, MsHDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, MsHDMITX_VIDEO_COLORDEPTH_VAL color_depth)
{
    MS_U8 i = 0;
    MS_U8 ubGot = 0;
    MsHDMITX_TIMING_ERROR err = E_HDMITX_TIMING_ERR_NONE;

    _HDMITX_GET_VARIABLE();

    // Check timing
    {
        //Check HW Limitation
        if(uiPxlClk > MHal_HDMITx_GetMaxPixelClk())
        {
            err |= E_HDMITX_TIMING_ERR_TIMING;
            err |= E_HDMITX_TIMING_ERR_HW_LIMIT;
            DBG_HDMITX(printf("Pixel clock too high\r\n"));
        }

        //Check EDID capability
        {
            //Check VIC
            {
                for(i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[CEATag_VideoBlk]; i++)
                {
                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] < 128)
                    {
                        if( (eVIC == E_HDMITX_VIC_720x480p_60_16_9 && ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == E_HDMITX_VIC_720x480p_60_4_3)) ||
                            (eVIC == E_HDMITX_VIC_720x480i_60_16_9 && ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == E_HDMITX_VIC_720x480i_60_4_3)) ||
                            (eVIC == E_HDMITX_VIC_720x576p_50_16_9 && ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == E_HDMITX_VIC_720x576p_50_4_3))  ||
                            (eVIC == E_HDMITX_VIC_720x576i_50_16_9 && ((psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == E_HDMITX_VIC_720x576i_50_4_3)) )
                        {
                            ubGot = 1;
                            break;
                        }
                        if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == eVIC) //7bits VIC
                        {
                            ubGot = 1;
                            break;
                        }
                    }
                    else if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] > 128) && (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] <= 192) )
                    {
                        if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0x7F) == eVIC) //7bits VIC
                        {
                            ubGot = 1;
                            break;
                        }
                    }
                    else
                    {
                        switch(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i])
                        {
                            case 0:
                            case 128:
                            case 254:
                            case 255:
                            break;

                            default:
                            {
                                if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i] & 0xFF) == eVIC) //8bits VIC
                                {
                                    ubGot = 1;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

                //Check VIC in 420 VideoDataBlock
                if(!ubGot)
                {
                    if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV420)
                    {
                        for ( i = 0; i < 32; i++ )
                        {
                            if (eVIC == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i])
                            {
                                ubGot = 1;
                                break;
                            }
                        }
                    }
                }

                if(!ubGot)
                {
                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_TIMING;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("No 4K VIC\r\n"));
                    }
                    else
                    {
                        //Check HDMI_VIC
                        switch(eVIC)
                        {
                            case E_HDMITX_VIC_3840x2160p_24_16_9:
                            case E_HDMITX_VIC_3840x2160p_24_64_27:
                            {
                                for(i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen; i++)
                                {
                                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i] == 1)
                                    {
                                        ubGot = 1;
                                        break;
                                    }
                                }
                            }
                            break;

                            case E_HDMITX_VIC_3840x2160p_25_16_9:
                            case E_HDMITX_VIC_3840x2160p_25_64_27:
                            {
                                for(i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen; i++)
                                {
                                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i] == 2)
                                    {
                                        ubGot = 1;
                                        break;
                                    }
                                }
                            }
                            break;

                            case E_HDMITX_VIC_3840x2160p_30_16_9:
                            case E_HDMITX_VIC_3840x2160p_30_64_27:
                            {
                                for(i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen; i++)
                                {
                                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i] == 3)
                                    {
                                        ubGot = 1;
                                        break;
                                    }
                                }
                            }
                            break;

                            case E_HDMITX_VIC_4096x2160p_24_256_135:
                            {
                                for(i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICLen; i++)
                                {
                                    if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HdmiVICList[i] == 4)
                                    {
                                        ubGot = 1;
                                        break;
                                    }
                                }
                            }
                            break;

                            default:
                            {
                                ubGot = 0;
                            }
                            break;
                        }

                        if(!ubGot)
                        {
                            err |= E_HDMITX_TIMING_ERR_TIMING;
                            err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                            DBG_HDMITX(printf("Timing not supported\r\n"));
                        }
                    }
                }
            }

            //Check VSDB and HFVSDB
            {
                if(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate != 0)
                {
                    if(uiPxlClk > psHDMITXResPri->stHDMITxInfo.RxEdidInfo.MaxTmdsCharRate * 5 *1000)
                    {
                        err |= E_HDMITX_TIMING_ERR_TIMING;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Clock hihger than sink\r\n"));
                    }
                }
            }

            //Check SW LIMMIT
            {
                //Check 6.6G
            }

            //Check BT2020 for 4K/8K
            {

            }
        }
    }

    return err;
}

MsHDMITX_TIMING_ERROR MDrv_HDMITx_CheckEDIDColorFormat(void* pInstance, MsHDMITX_VIDEO_TIMING idx, MsHDMITX_AVI_VIC eVIC, MsHDMITX_VIDEO_COLOR_FORMAT incolor_fmt, MsHDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, MsHDMITX_VIDEO_COLORDEPTH_VAL color_depth)
{
    MS_U8 i = 0;
    MS_U8 ub420Supported = 0;
    MsHDMITX_TIMING_ERROR err = E_HDMITX_TIMING_ERR_NONE;

    //pInstance = pInstance;
    _HDMITX_GET_VARIABLE();

    //Check color format
    {
        //Check R2Y supported
        if(!MHal_HDMITx_CSC_Support_R2Y(pInstance))
        {
            if( (incolor_fmt == E_HDMITX_VIDEO_COLOR_RGB444) && (outcolor_fmt != E_HDMITX_VIDEO_COLOR_RGB444) )
            {
                err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                err |= E_HDMITX_TIMING_ERR_HW_LIMIT;
                DBG_HDMITX(printf("HW not supporting R2Y\r\n"));
            }
        }

        //Check EDID
        if(outcolor_fmt != E_HDMITX_VIDEO_COLOR_YUV420)
        {
            switch (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1[0x03] & 0x30)
            {
                case 0x00: //RGB
                {
                    if(outcolor_fmt != E_HDMITX_VIDEO_COLOR_RGB444)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Sink RGB only\r\n"));
                    }
                }
                break;

                case 0x10: //422
                {
                    if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV444)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Sink 422 only\r\n"));
                    }
                }
                break;

                case 0x20: //444
                {
                    if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV422)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Sink 444 only\r\n"));
                    }
                }
                break;

                case 0x30:
                {
                    //RGB YUV444 YUV422 OK
                }
                break;

                default:
                {
                }
                break;
            }

            for ( i = 0; i < 32; i++ )
            {
                if (eVIC == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i])
                {
                    if(outcolor_fmt != E_HDMITX_VIDEO_COLOR_YUV420)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Sink 420 only 1\r\n"));
                    }
                    else
                    {
                        ub420Supported = 1;
                    }

                    break;
                }
            }
        }
        else if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV420)
        {
            //420 checking
            for ( i = 0; i < 32; i++ )
            {
                if (eVIC == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420VidDataBlk[i])
                {
                    if(outcolor_fmt != E_HDMITX_VIDEO_COLOR_YUV420)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("Sink 420 only 2\r\n"));
                    }
                    else
                    {
                        ub420Supported = 1;
                    }

                    break;
                }
            }

            if( (outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV420) && (!ub420Supported) )
            {
                for ( i = 0; i < psHDMITXResPri->stHDMITxInfo.RxEdidInfo.CEADataBlkLen[CEATag_VideoBlk]; i++ )
                {
                    if (eVIC == psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoDataBlk[i])
                    {
                        if (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YCbCr420CapMapDataBlk[i >> 3] & (0x01 << (i % 8)))
                        {
                            ub420Supported = 1;
                        }

                        break;
                    }
                }

                if(!ub420Supported)
                {
                    err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                    err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                    DBG_HDMITX(printf("420 not supported\r\n"));
                }
            }

            switch(idx)
            {
                case E_HDMITX_RES_3840x2160p_24Hz:
                case E_HDMITX_RES_3840x2160p_25Hz:
                case E_HDMITX_RES_3840x2160p_30Hz:
                case E_HDMITX_RES_3840x2160p_50Hz:
                case E_HDMITX_RES_3840x2160p_60Hz:
                case E_HDMITX_RES_4096x2160p_24Hz:
                case E_HDMITX_RES_4096x2160p_25Hz:
                case E_HDMITX_RES_4096x2160p_30Hz:
                case E_HDMITX_RES_4096x2160p_50Hz:
                case E_HDMITX_RES_4096x2160p_60Hz:

                break;
                default:
                {
                    err |= E_HDMITX_TIMING_ERR_COLOR_FMT;
                    err |= E_HDMITX_TIMING_ERR_SW_LIMIT;
                }
                break;
            }
        }
    }

    return err;
}

MsHDMITX_TIMING_ERROR MDrv_HDMITx_CheckEDIDColorDepth(void* pInstance, MsHDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, MsHDMITX_VIDEO_COLORDEPTH_VAL color_depth)
{
    MS_U8 ubCDSupported = 0;
    MsHDMITX_TIMING_ERROR err = E_HDMITX_TIMING_ERR_NONE;

    _HDMITX_GET_VARIABLE();

    //Color Depth
    {
        if( (outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV422) && (color_depth != E_HDMITX_VIDEO_CD_NoID) )
        {
            err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
            DBG_HDMITX(printf("422 wrong color depth\r\n"));
        }

        if( (color_depth != E_HDMITX_VIDEO_CD_NoID) && (color_depth != E_HDMITX_VIDEO_CD_24Bits) )
        {
            ubCDSupported = psHDMITXResPri->stHDMITxInfo.RxEdidInfo.HDMI_VSDB[5] & 0x78;

            if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV444)
            {
                if( (ubCDSupported & 0x08) == 0)
                {
                    err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                    err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                    DBG_HDMITX(printf("YUV no color depth\r\n"));
                }

                if(color_depth == E_HDMITX_VIDEO_CD_48Bits)
                {
                    if( (ubCDSupported & 0x40) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("YUV no 16bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_36Bits)
                {
                    if( (ubCDSupported & 0x20) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("YUV no 12bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_30Bits)
                {
                    if( (ubCDSupported & 0x10) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("YUV no 10bits\r\n"));
                    }
                }
            }
            else if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_RGB444)
            {
                if(color_depth == E_HDMITX_VIDEO_CD_48Bits)
                {
                    if( (ubCDSupported & 0x40) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("RGB no 16bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_36Bits)
                {
                    if( (ubCDSupported & 0x20) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("RGB no 12bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_30Bits)
                {
                    if( (ubCDSupported & 0x10) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("RGB no 10bits\r\n"));
                    }
                }
            }
            else if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_YUV420)
            {
                if(color_depth == E_HDMITX_VIDEO_CD_48Bits)
                {
                    if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo & 0x04) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("420 no 16bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_36Bits)
                {
                    if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo & 0x02) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("420 no 12bits\r\n"));
                    }
                }

                if(color_depth == E_HDMITX_VIDEO_CD_30Bits)
                {
                    if( (psHDMITXResPri->stHDMITxInfo.RxEdidInfo.YUV420DeepColorInfo & 0x01) == 0)
                    {
                        err |= E_HDMITX_TIMING_ERR_COLOR_DEPTH;
                        err |= E_HDMITX_TIMING_ERR_SINK_LIMIT;
                        DBG_HDMITX(printf("420 no 10bits\r\n"));
                    }
                }
            }
        }
    }

    return err;
}

MsHDMITX_VIDEO_COLOR_FORMAT MDrv_HDMITx_GetAVIInfo_ColorFmt(void* pInstance)
{
    MsHDMITX_VIDEO_COLOR_FORMAT enOutColorFmt = E_HDMITX_VIDEO_COLOR_RGB444;

    enOutColorFmt = MHal_HDMITx_GetAVIInfo_ColorFmt();
    return enOutColorFmt;
}

MsHDMITX_VIDEO_COLORDEPTH_VAL MDrv_HDMITx_GetGC_ColorDepth(void* pInstance)
{
    MsHDMITX_VIDEO_COLORDEPTH_VAL enOutColorDepth= E_HDMITX_VIDEO_CD_NoID;

    enOutColorDepth = MHal_HDMITx_GetGC_ColorDepth();
    return enOutColorDepth;
}

MsHDMITX_EDID_QUANT_RANGE MDrv_HDMITx_CheckEDIDQuantizationRange(void* pInstance, MsHDMITX_VIDEO_TIMING idx, MsHDMITX_VIDEO_COLOR_FORMAT outcolor_fmt)
{
    MsHDMITX_EDID_QUANT_RANGE eOutRange = E_HDMITX_EDID_QUANT_LIMIT; // default, CE video format

    //pInstance = pInstance;
    _HDMITX_GET_VARIABLE();

    if(outcolor_fmt == E_HDMITX_VIDEO_COLOR_RGB444) // RGB
    {
        if(!(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoCapDataBlk & 0x40)) // QS = 0 (RGB) - default color range
        {
            if(_MDrv_HDMIRx_VideoFormat(idx) == E_HDMITX_VIDEO_FORMAT_IT) // IT video format
            {
                eOutRange = E_HDMITX_EDID_QUANT_FULL;
            }
        }
        else
        {
            eOutRange = E_HDMITX_EDID_QUANT_BOTH;
        }
    }
    else // YUV
    {
        if(!(psHDMITXResPri->stHDMITxInfo.RxEdidInfo.VideoCapDataBlk & 0x80)) // QY = 0 (YCC) - default color range
        {
            if(_MDrv_HDMIRx_VideoFormat(idx) == E_HDMITX_VIDEO_FORMAT_IT) // IT video format
            {
                eOutRange = E_HDMITX_EDID_QUANT_FULL;
            }
        }
        else
        {
            eOutRange = E_HDMITX_EDID_QUANT_BOTH;
        }
    }

    return eOutRange;
}

MS_U32 MDrv_HDMITx_GetClkNum(void* pInstance)
{
    MS_U32 u32ClkNum;
    u32ClkNum = MHal_HDMITX_GetClkNum();

    DBG_HDMITX(printf("%s %d, ClkNum:%d\n", __FUNCTION__, __LINE__, u32ClkNum));
    return u32ClkNum;
}

MS_BOOL MDrv_HDMITx_GetDefaultClkRate(void* pInstance, MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    return MHal_HDMITX_GetDefaultClkRate(pu32ClkRate, u32Size);
}

MS_BOOL MDrv_HDMITx_GetDefaultClkOnOff(void* pInstance, MS_BOOL bEn, MS_BOOL *pbClkOnOff, MS_U32 u32Size)
{
    return MHal_HDMITx_GetDefualtClkOnOff(bEn, pbClkOnOff, u32Size);
}

MS_BOOL MDrv_HDMITx_GetDefaultClkMuxAttr(void* pInstance, MS_BOOL *pbClkMuxAttr, MS_U32 u32Size)
{
    return MHal_HDMITx_GetDefaultClkMuxAttr(pbClkMuxAttr, u32Size);
}

MS_BOOL MDrv_HDMITx_SetClk(void* pInstance, MS_BOOL *pbEn, MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    MS_BOOL bRet;

    if(pbEn && pu32ClkRate && u32Size)
    {
        MS_U32 i;
        bRet = MHal_HDMITx_SetClk(pbEn, pu32ClkRate, u32Size);
        for(i=0; i<u32Size; i++)
        {
            DBG_HDMITX(printf("%s %d, En:%d, ClkRate:%d\n",
                __FUNCTION__, __LINE__, pbEn[i], pu32ClkRate[i]));
        }
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

MS_BOOL MDrv_HDMITx_GetClk(void* pInstance, MS_BOOL *pbEn, MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    MS_BOOL bRet = TRUE;

    if(pbEn && pu32ClkRate && u32Size)
    {
        MS_U32 i;

        bRet = MHal_HDMITx_GetClk(pbEn, pu32ClkRate, u32Size);

        for(i=0; i<u32Size; i++)
        {
            DBG_HDMITX(printf("%s %d, En:%d, ClkRate:%d\n",
                __FUNCTION__, __LINE__, pbEn[i], pu32ClkRate[i]));
        }

    }
    else
    {
        bRet = FALSE;
        printf("%s %d, Param is Null, En:%p, ClkRate:%p, size:%d\n",
            __FUNCTION__, __LINE__, pbEn, pu32ClkRate, u32Size);
    }
    return bRet;
}


#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
//**************************************************************************
//  [Function Name]:
//                  mdrv_mhl_MDCMDEchoCommand()
//  [Description]
//                  MHL MDCMD echo command function
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void MDrv_HDMITx_MDCMDEchoCommand(void* pInstance, MS_U64* u64ReqHdl, char* pcCmdLine)
{
    char pch[] = "=,";
    char* psep;

    MdbPrint(u64ReqHdl, "LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
    MdbPrint(u64ReqHdl, "pcCmdLine: %s\n", pcCmdLine);

    psep = strsep(&pcCmdLine, pch);

    if(strncmp("HDMITXOn", psep, 8) == 0)
    {

    }
}

//**************************************************************************
//  [Function Name]:
//                  mdrv_mhl_MDCMDGetInfo()
//  [Description]
//                  MHL MDCMD get info function
//  [Arguments]:
//
//  [Return]:
//
//**************************************************************************
void MDrv_HDMITx_MDCMDGetInfo(void* pInstance, MS_U64* u64ReqHdl)
{
    int i = 0;
    _HDMITX_GET_VARIABLE();

    {
        MdbPrint(u64ReqHdl, "LINE:%d, MDBCMD_GETINFO\n", __LINE__);
        MdbPrint(u64ReqHdl, "----------------  HDMITx Info -----------------\n");
        //Basic Status
        MdbPrint(u64ReqHdl, "HDMITx Dis_TMDSCtrl=%d\n",   g_bDisableTMDSCtrl );
        MdbPrint(u64ReqHdl, "HDMITx Dis_RegCtrl=%d\n",  g_bDisableRegWrite);
        MdbPrint(u64ReqHdl, "HDMITx RxStatus: Real = %d, Flag=%d\n", MHal_HDMITx_GetRXStatus(), gbCurRxStatus);
        MdbPrint(u64ReqHdl, "HDMITx TMDS Status: Real=%d, Flag=%d\n", MHal_HDMITx_GetTMDSStatus(), psHDMITXResPri->stHDMITxInfo.hdmitx_tmds_flag);
        MdbPrint(u64ReqHdl, "HDMITx FSM=%d\n",  psHDMITXResPri->stHDMITxInfo.hdmitx_fsm_state);
        MdbPrint(u64ReqHdl, "HDMITx TxRxByPass=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_bypass_flag);

        //Video
        MdbPrint(u64ReqHdl, "----------------  HDMITx Video Info -----------\n");
        MdbPrint(u64ReqHdl, "HDMITx OutputMode=%d\n",   psHDMITXResPri->stHDMITxInfo.output_mode);
        MdbPrint(u64ReqHdl, "HDMITx OutputTiming=%d\n",   psHDMITXResPri->stHDMITxInfo.output_video_timing);
        MdbPrint(u64ReqHdl, "HDMITx InColor=%d, OutColor=%d\n",  psHDMITXResPri->stHDMITxInfo.input_color, psHDMITXResPri->stHDMITxInfo.output_color);
        MdbPrint(u64ReqHdl, "HDMITx InRange=%d, OutRange=%d\n",  psHDMITXResPri->stHDMITxInfo.input_range, psHDMITXResPri->stHDMITxInfo.output_range);
        MdbPrint(u64ReqHdl, "HDMITx ColorDepth=%d\n",   psHDMITXResPri->stHDMITxInfo.output_colordepth_val);
        MdbPrint(u64ReqHdl, "HDMITx Colorimetry=%d, ExtColorimetry=%d\n",  psHDMITXResPri->stHDMITxInfo.colorimetry, psHDMITXResPri->stHDMITxInfo.ext_colorimetry);
        MdbPrint(u64ReqHdl, "HDMITx AVMute=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_avmute_flag);
        MdbPrint(u64ReqHdl, "HDMITx TxPatternOn=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_video_flag);
        MdbPrint(u64ReqHdl, "HDMITx ACTIVFMT_PRESENT=%d\n",   psHDMITXResPri->stHDMITxInfo.output_activeformat_present );
        MdbPrint(u64ReqHdl, "HDMITx AFDOverWrite=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_AFD_override_mode);
        MdbPrint(u64ReqHdl, "HDMITx ScanInfo=%d\n",   psHDMITXResPri->stHDMITxInfo.output_scan_info);
        MdbPrint(u64ReqHdl, "HDMITx AFDRatio=%d\n",   psHDMITXResPri->stHDMITxInfo.output_afd_ratio);
        MdbPrint(u64ReqHdl, "HDMITx AspectRation=%d\n",   psHDMITXResPri->stHDMITxInfo.output_aspect_ratio);

        //Audio
        MdbPrint(u64ReqHdl, "----------------  HDMITx Audio Info -----------\n");
        MdbPrint(u64ReqHdl, "HDMITx AudioOn=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_audio_flag);
        MdbPrint(u64ReqHdl, "HDMITx AudioSampleRate=%d\n",   psHDMITXResPri->stHDMITxInfo.output_audio_frequncy);
        MdbPrint(u64ReqHdl, "HDMITx AudioChannelCnt=%d\n",   psHDMITXResPri->stHDMITxInfo.output_audio_channel);
        MdbPrint(u64ReqHdl, "HDMITx AudioNon-PCM=%d\n",   psHDMITXResPri->stHDMITxInfo.output_audio_type);

        //EDID
        MdbPrint(u64ReqHdl, "----------------  HDMITx EDID Info ------------\n");
        MdbPrint(u64ReqHdl, "HDMITx EDID Valid=%d\n",   psHDMITXResPri->stHDMITxInfo.hdmitx_edid_ready);

        MdbPrint(u64ReqHdl, "**********       HDMITx EDID0       ***********\n");
        for(i = 0; i < HDMITX_EDID_BLK_SIZE; i ++)
        {
            MdbPrint(u64ReqHdl, "0x%02x, \n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk0[i]);

            if ( (i+1)%16 == 0 )
            {
                MdbPrint(u64ReqHdl, "\n");
            }
        }

        MdbPrint(u64ReqHdl, "**********       HDMITx EDID1       ***********\n");
        for(i = 0; i < HDMITX_EDID_BLK_SIZE; i ++)
        {
            MdbPrint(u64ReqHdl, "0x%02x, \n", psHDMITXResPri->stHDMITxInfo.RxEdidInfo.EdidBlk1[i]);

            if ( (i+1)%16 == 0 )
            {
                MdbPrint(u64ReqHdl, "\n");
            }
        }

        //HDCPTx
        MdbPrint(u64ReqHdl, "----------------  REE HDCPTx Info -------------\n");
        MdbPrint(u64ReqHdl, "REE HDCPTx START=%d\n",  psHDMITXResPri->stHdcpTxInfo.hdmitx_HdcpStartAuth_flag);
        MdbPrint(u64ReqHdl, "REE HDCPTx On=%d\n",  psHDMITXResPri->stHDMITxInfo.hdmitx_hdcp_flag);
        MdbPrint(u64ReqHdl, "REE HDCPTx FSM: Main=%d, Sub=%d\n",  psHDMITXResPri->stHdcpTxInfo.hdcp14tx_MainState, psHDMITXResPri->stHdcpTxInfo.hdcp14tx_SubState);
        MdbPrint(u64ReqHdl, "REE HDCPTx TVHDCP14Cap=%d\n",   psHDMITXResPri->stHdcpTxInfo.hdcp_Rx_valid);

    }
}

#endif

//------------------------------------------------------------------------------
/// HDCP SRM DSA Signature Verification
//------------------------------------------------------------------------------
#if 0
void    _dsa_init(dsa_context *dsa)
{
    dsa->p = bdNew();
    dsa->q = bdNew();
    dsa->g = bdNew();
    dsa->x = bdNew();
    dsa->y = bdNew();
}

void    _dsa_clear(dsa_context *dsa)
{
    bdFree(&dsa->p);
    bdFree(&dsa->q);
    bdFree(&dsa->g);
    bdFree(&dsa->x);
    bdFree(&dsa->y);
}

//
// FUNCTION dsa_verify
//
// m: H(m), hash of the message, in general this is SHA-1
// r: signature
// s: signature
//
// verify method:
// v = ( (g^u1 * y^u2) % p ) % q
// check v = r
//
MS_S8 _dsa_verify(dsa_context *dsa, BIGD m, BIGD r, BIGD s)
{
    BIGD w;
    BIGD u1;
    BIGD u2;
    BIGD v;
    BIGD tmp1, tmp2, tmp3;
    MS_S8      result;

    w    = bdNew();
    u1   = bdNew();
    u2   = bdNew();
    v    = bdNew();
    tmp1 = bdNew();
    tmp2 = bdNew();
    tmp3 = bdNew();

    bdModInv(w,s,dsa->q);        // w = inv(s) % q
    bdModMult(u1,m,w,dsa->q);        // u1 = m*w % q
    bdModMult(u2,r,w,dsa->q);        // u2 = r*w % q
    bdModExp(tmp1,dsa->g,u1,dsa->p);    // tmp1 = (g^u1) % p
    bdModExp(tmp2,dsa->y,u2,dsa->p);    // tmp2 = (y^u2) % p
    bdModMult(tmp3,tmp1,tmp2,dsa->p);    // tmp3 = (g^u1 % p) * (y^u2 % p)
    bdModulo(v,tmp3,dsa->q);        // v = tmp3 % q
    result = bdCompare(v,r);        // compare (v, r)

    printf("@@SRM- W= ");
    bdPrint(w, BD_PRINT_TRIM | BD_PRINT_NL);
#if 0
    printf("@@SRM- u1= ");
    bdPrint(u1, BD_PRINT_TRIM | BD_PRINT_NL);
    printf("@@SRM- u2= ");
    bdPrint(u2, BD_PRINT_TRIM | BD_PRINT_NL);
    printf("@@SRM- G^u1= ");
    bdPrint(tmp1, BD_PRINT_TRIM | BD_PRINT_NL);
    printf("@@SRM- Y^u2= ");
    bdPrint(tmp2, BD_PRINT_TRIM | BD_PRINT_NL);
    printf("@@SRM- Product= ");
    bdPrint(tmp3, BD_PRINT_TRIM | BD_PRINT_NL);
#endif
    printf("@@SRM- V= ");
    bdPrint(v, BD_PRINT_TRIM | BD_PRINT_NL);

    bdFree(&w);
    bdFree(&u1);
    bdFree(&u2);
    bdFree(&v);
    bdFree(&tmp1);
    bdFree(&tmp2);
    bdFree(&tmp3);

    return result;            // 0: verified, non-zero: failed to verify
}

#endif


