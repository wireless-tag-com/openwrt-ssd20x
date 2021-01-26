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

/*                                                                                                                     */
/*                                                   Includes                                                      */
/*                                                                                                                     */
/*********************************************************************/
#include "mhal_common.h"
#include <stdio.h>
#include "drvHDCP.h"
#include "halHDCP.h"
#include "drv_hdmitx_os.h"

#ifndef DRV_HDCP_C
#define DRV_HDCP_C

/*********************************************************************/
/*                                                                                                                     */
/*                                                      Defines                                                    */
/*                                                                                                                     */
/*********************************************************************/
/*********************************************************************/
/*                                                                                                                     */
/*                                                      Global                                                      */
/*                                                                                                                     */
/*********************************************************************/
static MS_BOOL g_bHdcpMMIOBaseInit = FALSE;

/*********************************************************************/
/*                                                                                                                     */
/*                                                    Functions                                                    */
/*                                                                                                                     */
/*********************************************************************/
/*********************************************************************/
/*                                                                                                                     */
/*                                  HDCP14 Tx Function Start                                              */
/*                                                                                                                     */
/*********************************************************************/
void __FUNC_HDCP14TX_Relative__(void){} // dummy function for pivot
//TBD
MS_BOOL MDrv_HDCP_HDCP14TxInitHdcp(MS_U8 u8PortIdx)
{
    MS_VIRT u32Bank, u32IRBank;
    MS_PHY u32BankSize;  // Non-PM bank
    MS_PHY u32IrBankSize;  // PM bank

    if (g_bHdcpMMIOBaseInit == FALSE)
    {
        if (FALSE == MDrv_MMIO_GetBASE(&u32Bank, &u32BankSize, MS_MODULE_BDMA))
        {
            printf("MDrv_MMIO_GetBASE (NonPM base)fail\n");
            MS_ASSERT(0);
        }

        if (FALSE == MDrv_MMIO_GetBASE(&u32IRBank, &u32IrBankSize, MS_MODULE_IR))
        {
            printf("MDrv_MMIO_GetBASE (PM base)fail\n");
            MS_ASSERT(0);
        }

        MHal_HDCP_SetBank(u32Bank, u32IRBank);
        g_bHdcpMMIOBaseInit = TRUE;
    }

    MHal_HDCP_HDCP14TxInitHdcp(u8PortIdx);

    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxLoadKey(MS_U8* pu8KeyData, MS_BOOL bUseKmNewMode)
{
    MHal_HDCP_HDCP14TxLoadKey(pu8KeyData, bUseKmNewMode);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxSetAuthPass(MS_U8 u8PortIdx)
{
    MHal_HDCP_HDCP14TxSetAuthPass(u8PortIdx);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxEnableENC_EN(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDCP_HDCP14TxEnableENC_EN(u8PortIdx, bEnable);
    return TRUE;
}
MS_BOOL MDrv_HDCP_HDCP14TxProcessAn(MS_U8 u8PortIdx, MS_BOOL bUseInternalAn, MS_U8* pu8An)
{
    MHal_HDCP_HDCP14TxProcessAn(u8PortIdx, bUseInternalAn, pu8An);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxGetAKSV(MS_U8 u8PortIdx, MS_U8* pu8Aksv)
{
    MHal_HDCP_HDCP14TxGetAKSV(u8PortIdx, pu8Aksv);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxCompareRi(MS_U8 u8PortIdx, MS_U8* pu8SinkRi)
{
    return MHal_HDCP_HDCP14TxCompareRi(u8PortIdx, pu8SinkRi);
}

MS_BOOL MDrv_HDCP_HDCP14TxConfigMode(MS_U8 u8PortIdx, MS_U8 u8Mode)
{
    MHal_HDCP_HDCP14TxConfigMode(u8PortIdx, u8Mode);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxGenerateCipher(MS_U8 u8PortIdx, MS_U8* pu8Bksv)
{
    MHal_HDCP_HDCP14TxGenerateCipher(u8PortIdx, pu8Bksv);
    return TRUE;
}

MS_BOOL MDrv_HDCP_HDCP14TxProcessR0(MS_U8 u8PortIdx)
{
    return MHal_HDCP_HDCP14TxProcessR0(u8PortIdx);
}

MS_BOOL MDrv_HDCP_HDCP14TxGetM0(MS_U8 u8PortIdx, MS_U8* pu8M0)
{
    if (pu8M0 == NULL)
    {
        return FALSE;
    }
    else
    {
        MHal_HDCP_HDCP14TxGetM0(u8PortIdx, pu8M0);
        return TRUE;
    }
}
/*********************************************************************/
/*                                                                                                                     */
/*                                  HDCP14 Rx Function Start                                              */
/*                                                                                                                     */
/*********************************************************************/
void __FUNC_HDCP14RX_Relative__(void){} // dummy function for pivot
//TBD
MS_BOOL MDrv_HDCP_HDCP14GetM0(MS_U8 u8PortIdx, MS_U8 *pu8Data)
{
    if (pu8Data == NULL)
    {
        return FALSE; //DRVAESDMA_FAIL;
    }
    else
    {
        MHal_HDCP_HDCP14GetM0(u8PortIdx, pu8Data);
        return TRUE; //DRVAESDMA_OK;
    }
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP14FillKey()
//  [Description]:
//                  HDCP 1.4 load key
//  [Arguments]:
//                  [MS_U8] pu8KeyData
//  [Return]:
//                  BOOL
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP14FillKey(MS_U8 *pu8KeyData)
{
    if(pu8KeyData == NULL)
    {
        return FALSE; //DRVAESDMA_FAIL;
    }
    else
    {
        MHal_HDCP_HDCP14FillBksv(pu8KeyData);
        MHal_HDCP_HDCP14FillKey(pu8KeyData +5);
        return TRUE; //DRVAESDMA_OK;
    }
}

/*********************************************************************/
/*                                                                                                                     */
/*                                  HDCP22 Tx Function Start                                              */
/*                                                                                                                     */
/*********************************************************************/
void __FUNC_HDCP22TX_Relative__(void){} // dummy function for pivot

MS_BOOL MDrv_HDCP_HDCP2Init(void)
{
    MS_VIRT u32Bank, u32IRBank;
    MS_PHY u32BankSize;  // Non-PM bank
    MS_PHY u32IrBankSize;  // PM bank

    if (g_bHdcpMMIOBaseInit == FALSE)
    {
        if (FALSE == MDrv_MMIO_GetBASE(&u32Bank, &u32BankSize, MS_MODULE_BDMA))
        {
            printf("MDrv_MMIO_GetBASE (NonPM base)fail\n");
            MS_ASSERT(0);
        }

        if (FALSE == MDrv_MMIO_GetBASE(&u32IRBank, &u32IrBankSize, MS_MODULE_IR))
        {
            printf("MDrv_MMIO_GetBASE (PM base)fail\n");
            MS_ASSERT(0);
        }

        MHal_HDCP_SetBank(u32Bank, u32IRBank);
        g_bHdcpMMIOBaseInit = TRUE;
    }

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2TxModuleInit()
//  [Description]:
//                  init or de-init hdcp22 tx hw module
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_BOOL] bEnable
//  [Return]:
//                  bool
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2TxModuleInit(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDCP_HDCP2TxInit(u8PortIdx, bEnable);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2TxEnableEncrypt()
//  [Description]:
//                  enable/disable Hdcp22 Tx encrytion process
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_BOOL] bEnable
//  [Return]:
//                  void
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2TxEnableEncrypt(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDCP_HDCP2TxEnableEncrypt(u8PortIdx, bEnable);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2TxFillCipherKey()
//  [Description]:
//                  Fill Cipher key (Ks^LC128, riv) for HDCP 2.2 Tx
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_U8*] pu8Riv,
//                  [MS_U8*] pu8KsXORLC128
//  [Return]:
//                  void
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2TxFillCipherKey(MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8ContentKey)
{
    MHal_HDCP_HDCP2TxFillCipherKey( u8PortIdx, pu8Riv, pu8ContentKey);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2TxGetCipherState()
//  [Description]:
//                  return Status about HDCP22 Tx Cipher engine
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_U8*] pu8State
//  [Return]:
//                  [MS_U32]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2TxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State)
{
    MHal_HDCP_HDCP2TxGetCipherState(u8PortIdx, pu8State);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2TxSetAuthPass()
//  [Description]:
//                  enable or disable authentication pass bit of hdcp22 tx hw module
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_BOOL] bEnable
//  [Return]:
//                  [MS_U32]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2TxSetAuthPass(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDCP_HDCP2TxSetAuthPass(u8PortIdx, bEnable);
    return TRUE;
}

/*********************************************************************/
/*                                                                                                                     */
/*                                  HDCP22 Rx Function Start                                              */
/*                                                                                                                     */
/*********************************************************************/
void __FUNC_HDCP22RX_Relative__(void){} // dummy function for pivot

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2RxInit()
//  [Description]:
//                  initial setting of hdcp22 rx
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2RxInit(MS_U8 u8PortIdx)
{
    MHal_HDCP_HDCP2RxInit(u8PortIdx);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2RxProcessCipher()
//  [Description]:
//                  Fill Cipher key (Ks^LC128, riv) for HDCP 2.2 Rx
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_U8*] pu8Riv,
//                  [MS_U8*] pu8ContentKey
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2RxProcessCipher(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey)
{
    MHal_HDCP_HDCP2RxProcessCipher(u8PortIdx, pu8Riv, pu8ContentKey);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2RxEnableENCEN()
//  [Description]:
//                  Enable or disable ENC_EN
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_BOOL] bEnable
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2RxSetSKEPass(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MHal_HDCP_HDCP2RxSetSKEPass(u8PortIdx, bEnable);
    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2RxProcessCipher()
//  [Description]:
//                  Fill Cipher key (Ks^LC128, riv) for HDCP 2.2 Rx
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_U8*] pu8Riv,
//                  [MS_U8*] pu8ContentKey
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2RxFillCipherKey(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey)
{
    MHal_HDCP_HDCP2RxFillCipherKey(u8PortIdx, pu8Riv, pu8ContentKey);
    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_HDCP2RxGetCipherState()
//  [Description]:
//                  return Status about HDCP22 Rx Cipher engine
//  [Arguments]:
//                  [MS_U8] u8PortIdx
//                  [MS_U8*] pu8State
//  [Return]:
//                  [MS_U32]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_HDCP2RxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State)
{
    MHal_HDCP_HDCP2RxGetCipherState(u8PortIdx, pu8State);

    return TRUE;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_GetHDCP2Status()
//  [Description]:
//                  Get HDCP22 encrytion status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32*] pu32HDCPStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_GetHDCP2Status(MS_U32 u32HdmiPort, MS_U32 *pu32HDCPStatus)
{
    MS_BOOL bGetStatusFlag = FALSE;

    if(pu32HDCPStatus != NULL)
    {
        bGetStatusFlag = TRUE;

        *pu32HDCPStatus = MHal_HDCP_HDCP2TxEncrytionStatus(u32HdmiPort, FALSE, 0);
    }

    return bGetStatusFlag;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_SetHDCP2Status()
//  [Description]:
//                  Set HDCP22 encrytion status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32] u32HDCPStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_SetHDCP2Status(MS_U32 u32HdmiPort, MS_U32 u32HDCPStatus)
{
    MS_BOOL bSetStatusFlag = FALSE;

    if(MHal_HDCP_HDCP2TxEncrytionStatus(u32HdmiPort, TRUE, u32HDCPStatus) == u32HDCPStatus)
    {
        bSetStatusFlag = TRUE;
    }

    return bSetStatusFlag;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_GetHDCP1Status()
//  [Description]:
//                  Get HDCP14 encrytion status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32*] pu32HDCPStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_GetHDCP1Status(MS_U32 u32HdmiPort, MS_U32 *pu32HDCPStatus)
{
    MS_BOOL bGetStatusFlag = FALSE;

    if(pu32HDCPStatus != NULL)
    {
        bGetStatusFlag = TRUE;

        *pu32HDCPStatus = MHal_HDCP_HDCP1TxEncrytionStatus(u32HdmiPort, FALSE, 0);
    }

    return bGetStatusFlag;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_SetHDCP1Status()
//  [Description]:
//                  Set HDCP22 encrytion status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32] u32HDCPStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_SetHDCP1Status(MS_U32 u32HdmiPort, MS_U32 u32HDCPStatus)
{
    MS_BOOL bSetStatusFlag = FALSE;

    if(MHal_HDCP_HDCP1TxEncrytionStatus(u32HdmiPort, TRUE, u32HDCPStatus) == u32HDCPStatus)
    {
        bSetStatusFlag = TRUE;
    }

    return bSetStatusFlag;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_GetHDMIStatus()
//  [Description]:
//                  Get HDMI enable status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32*] pu32HDMIStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_GetHDMIStatus(MS_U32 u32HdmiPort, MS_U32 *pu32HDMIStatus)
{
    MS_BOOL bGetStatusFlag = FALSE;

    if(pu32HDMIStatus != NULL)
    {
        bGetStatusFlag = TRUE;

        *pu32HDMIStatus = MHal_HDCP_HDCPTxHDMIStatus(u32HdmiPort, FALSE, 0);
    }

    return bGetStatusFlag;
}

//**************************************************************************
//  [Function Name]:
//                  MDrv_HDCP_SetHDCP1Status()
//  [Description]:
//                  Set HDMI enable status based on HDMI port number
//  [Arguments]:
//                  [MS_U32] u32HdmiPort
//                  [MS_U32] u32HDMIStatus
//  [Return]:
//                  [MS_BOOL]
//
//**************************************************************************
MS_BOOL MDrv_HDCP_SetHDMIStatus(MS_U32 u32HdmiPort, MS_U32 u32HDMIStatus)
{
    MS_BOOL bSetStatusFlag = FALSE;

    if(MHal_HDCP_HDCPTxHDMIStatus(u32HdmiPort, TRUE, u32HDMIStatus) == u32HDMIStatus)
    {
        bSetStatusFlag = TRUE;
    }

    return bSetStatusFlag;
}

#endif //#ifndef DRV_HDCP_C
