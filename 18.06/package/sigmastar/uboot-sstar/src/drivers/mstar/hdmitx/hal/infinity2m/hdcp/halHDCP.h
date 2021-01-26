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

#ifndef _HAL_HDCP_H_
#define _HAL_HDCP_H_


enum HDMI_RX_PORT_SELECT_TYPE
{
    HDMI_RX_SELECT_PORTA = 0,
    HDMI_RX_SELECT_MASK,
    HDMI_RX_SELECT_PORTB,
    HDMI_RX_SELECT_PORTC,
    HDMI_RX_SELECT_PORTD,
};

/*********************************************************************/
/*                                                                                                                     */
/*                                                 Proto-type                                                    */
/*                                                                                                                     */
/*********************************************************************/
void MHal_HDCP_HDCP14TxInitHdcp(MS_U8 u8PortIdx);
void MHal_HDCP_HDCP14TxLoadKey(MS_U8* pu8KeyData, MS_BOOL bUseKmNewMode);
void MHal_HDCP_HDCP14TxSetAuthPass(MS_U8 u8PortIdx);
void MHal_HDCP_HDCP14TxEnableENC_EN(MS_U8 u8PortIdx, MS_BOOL bEnable);
void MHal_HDCP_HDCP14TxProcessAn(MS_U8 u8PortIdx, MS_BOOL bUseInternalAn, MS_U8* pu8An);
void MHal_HDCP_HDCP14TxGetAKSV(MS_U8 u8PortIdx, MS_U8* pu8Aksv);
MS_BOOL MHal_HDCP_HDCP14TxCompareRi(MS_U8 u8PortIdx, MS_U8* pu8SinkRi);
void MHal_HDCP_HDCP14TxConfigMode(MS_U8 u8PortIdx, MS_U8 u8Mode);
void MHal_HDCP_HDCP14TxGenerateCipher(MS_U8 u8PortIdx, MS_U8* pu8Bksv);
MS_BOOL MHal_HDCP_HDCP14TxProcessR0(MS_U8 u8PortIdx);
void MHal_HDCP_HDCP14TxGetM0(MS_U8 u8PortIdx, MS_U8* pu8M0);
void MHal_HDCP_HDCP14GetM0(MS_U8 u8PortIdx, MS_U8 *pu8Data);
void MHal_HDCP_HDCP14FillBksv(MS_U8 *pu8BksvData);
void MHal_HDCP_HDCP14FillKey(MS_U8 *pu8KeyData);
void MHal_HDCP_SetBank(MS_U32 u32NonPmBankAddr, MS_U32 u32PmBankAddr);
void MHal_HDCP_HDCP2TxInit(MS_U8 u8PortIdx, MS_BOOL bEnable);
void MHal_HDCP_HDCP2TxEnableEncrypt(MS_U8 u8PortIdx, MS_BOOL bEnable);
void MHal_HDCP_HDCP2TxFillCipherKey(MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128);
void MHal_HDCP_HDCP2TxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State);
void MHal_HDCP_HDCP2TxSetAuthPass(MS_U8 u8PortIdx, MS_BOOL bEnable);
void MHal_HDCP_HDCP2RxInit(MS_U8 u8PortIdx);
void MHal_HDCP_HDCP2RxProcessCipher(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey);
void MHal_HDCP_HDCP2RxSetSKEPass(MS_U8 u8PortIdx, MS_BOOL bEnable);
void MHal_HDCP_HDCP2RxFillCipherKey(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey);
void MHal_HDCP_HDCP2RxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State);
MS_U32 MHal_HDCP_HDCP1TxEncrytionStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus);
MS_U32 MHal_HDCP_HDCP2TxEncrytionStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus);
MS_U32 MHal_HDCP_HDCPTxHDMIStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus);

#endif //#ifndef _HAL_HDCP_H_
