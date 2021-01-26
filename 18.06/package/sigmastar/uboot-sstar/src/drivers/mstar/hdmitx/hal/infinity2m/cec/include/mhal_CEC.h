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

#ifndef _MHAL_CEC_H_
#define _MHAL_CEC_H_

#define CEC_DEVICE_IS_SOURCE     1

void mhal_CEC_PortSelect(MsCEC_INPUT_PORT InputPort);
void mhal_CEC_init_riu_base(MS_VIRT u32riu_base, MS_VIRT u32PMriu_base);

MS_U8 mhal_CEC_HeaderSwap(MS_U8 value);
MS_U8 mhal_CEC_SendFrame(MS_U8 header, MS_U8 opcode, MS_U8* operand, MS_U8 len);
MS_U8 mhal_CEC_SendFramex(MS_U8 header, MS_U8 opcode, MS_U8* operand, MS_U8 len);
void mhal_CEC_SetMyAddress(MS_U8 mylogicaladdress);
void mhal_CEC_INTEn(MS_BOOL bflag);
void mhal_CEC_Init(MS_U32 u32XTAL_CLK_Hz, MsCEC_DEVICELA DeviceLA, MS_U8 ucRetryCnt);
MS_BOOL mhal_CEC_IsMessageReceived(void);
MS_U8 mhal_CEC_ReceivedMessageLen(void);
MS_U8 mhal_CEC_GetMessageByte(MS_U8 idx);
void mhal_CEC_ClearRxStatus(void);
void mhal_CEC_ConfigWakeUp(MS_U8 ucRetryCount, MS_U8* ucVendorID, MS_U32 u32XTAL_CLK_Hz, MS_U8* ucPA, MsCEC_DEVICE_TYPE eType, MS_BOOL bIsSrc);
void mhal_CEC_Enabled(MS_BOOL bEnableFlag);
MS_U8 mhal_CEC_TxStatus(void);
MS_BOOL mhal_CEC_Device_Is_Tx(void);
void mhal_CEC_SetRetryCount(MS_U8 u8RetryCount);
#if ENABLE_CEC_MULTIPLE
void mhal_CEC_SetMyAddress2(MS_U8 mylogicaladdress);
MS_BOOL mhal_CEC_IsMessageReceived2(void);
MS_U8 mhal_CEC_ReceivedMessageLen2(void);
MS_U8 mhal_CEC_GetMessageByte2(MS_U8 idx);
void mhal_CEC_ClearRxStatus2(void);
#endif

#endif

