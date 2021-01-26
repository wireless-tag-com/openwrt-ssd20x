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

#ifndef _HAL_HDMIUTILTX_H_
#define _HAL_HDMIUTILTX_H_


#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define DDC_I2C_HW      1
#define DDC_I2C_SW      2

#define DDC_I2C_TYPE    DDC_I2C_SW


#define BIT0  0x0001U
#define BIT1  0x0002U
#define BIT2  0x0004U
#define BIT3  0x0008U
#define BIT4  0x0010U
#define BIT5  0x0020U
#define BIT6  0x0040U
#define BIT7  0x0080U
#define BIT8  0x0100U
#define BIT9  0x0200U
#define BIT10 0x0400U
#define BIT11 0x0800U
#define BIT12 0x1000U
#define BIT13 0x2000U
#define BIT14 0x4000U
#define BIT15 0x8000U

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    MS_U32      bank;
    MS_U16     address;
    MS_U16     mask;
    MS_U16     value;
} MSTHDMITX_REG_TYPE;


/////////////Wilson:: for Kano, HDMITX 2.0 architecture @20150702
typedef enum
{
	E_SCDC_SINK_VER_IDX =           0x01,
    E_SCDC_SRC_VER_IDX =            0x02,
    E_SCDC_UPDATE_0_IDX =           0x10,
    E_SCDC_UPDATE_1_IDX=            0x11,
    E_SCDC_TMDS_CONFIG_IDX =        0x20,
    E_SCDC_SCRAMBLE_STAT_IDX =      0x21,
    E_SCDC_CONFIG_0_IDX =           0x30,
    E_SCDC_STATFLAG_0_IDX =         0x40,
    E_SCDC_STATFLAG_1_IDX =         0x41,
    E_SCDC_ERR_DET_START_IDX =      0x50,
    E_SCDC_ERR_DET_CHKSUM_IDX =     0x56,
    E_SCDC_TEST_CONFIG_0_IDX =      0xC0,
    E_SCDC_MANUFACTURER_OUI_IDX =   0xD0,
    E_SCDC_DEVICE_ID_IDX = 		    0xD3,
    E_SCDC_MANUFACTURER_SPEC_IDX =  0xDE
} enMsHDMITX_SCDC_FIELD_OFFSET_LIST;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#ifdef MHAL_HDMIUTILTX_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void MHal_HDMITx_SetIOMapBase(MS_VIRT u32Base, MS_VIRT u32PMBase, MS_VIRT u32CoproBase);

INTERFACE MS_BOOL MHal_HDMITx_Rx74WriteByte(MS_U8 addr, MS_U8 value);

INTERFACE MS_BOOL MHal_HDMITx_Rx74ReadByte(MS_U8  addr, MS_U8 *pBuf);

INTERFACE MS_BOOL MHal_HDMITx_Rx74WriteBytes(MS_U8  addr, MS_U16  len, MS_U8  *buf);

INTERFACE MS_BOOL MHal_HDMITx_Rx74ReadBytes(MS_U8  addr, MS_U16  len, MS_U8  *buf);

INTERFACE MS_BOOL MHal_HDMITx_EdidReadBlock(MS_U8  num, MS_U8 *buf);

INTERFACE MS_U16 MHal_HDMITx_Read(MS_U32 bank, MS_U16 address);

INTERFACE MS_U8 MHal_HDMITx_ReadByte(MS_U32 bank, MS_U16 address);

INTERFACE void MHal_HDMITx_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_data);

INTERFACE void MHal_HDMITx_Mask_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data);

INTERFACE void MHal_HDMITx_Mask_WriteByte(MS_U32 bank, MS_U16 address, MS_U8 reg_mask, MS_U8 reg_data);

INTERFACE void MHal_HDMITx_RegsTbl_Write(MSTHDMITX_REG_TYPE *pTable, MS_U8 num);

INTERFACE MS_U16 MHal_HDMITxPM_Read(MS_U32 bank, MS_U16 address);

INTERFACE void MHal_HDMITxPM_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_data);

INTERFACE void MHal_HDMITxPM_Mask_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data);

// Read HDCP key from external EEPROM if not used internal HDCP key
INTERFACE MS_BOOL MHal_HDMITx_HDCPKeyReadByte(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size);

INTERFACE void MHal_HDMITx_UtilDebugEnable(MS_BOOL benable);

INTERFACE MS_U32 MHal_HDMITx_GetDDCDelayCount(void);
INTERFACE void MHal_HDMITx_SetDDCDelayCount(MS_U32 u32Delay);
INTERFACE MS_BOOL MHal_HDMITx_AdjustDDCFreq(MS_U32 u32Speed_K);

//SCDC
INTERFACE void Mhal_HDMITx_SCDCSetTmdsConfig(MS_BOOL bClkRatio, MS_BOOL bScrambleEn);
INTERFACE MS_BOOL Mhal_HDMITx_SCDCAccessField(enMsHDMITX_SCDC_FIELD_OFFSET_LIST enField, MS_BOOL bReadAction, MS_U8* pucData, MS_U8 ucLen);

// Get SwI2c En
INTERFACE MS_BOOL Mhal_HDMITx_GetSwI2cEn(void);
INTERFACE MS_U8 Mhal_HDMITx_GetSwI2cSdaPin(void);
INTERFACE MS_U8 Mhal_HDMITx_GetSwI2cSclPin(void);


#ifdef __cplusplus
}
#endif

#endif // _HAL_HDMIUTILTX_H_

