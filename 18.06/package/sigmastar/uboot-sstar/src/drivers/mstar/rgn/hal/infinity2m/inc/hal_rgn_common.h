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


#ifndef __HAL_RGN_COMMON_H__
#define __HAL_RGN_COMMON_H__
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "drv_rgn_os_header.h"
#include "rgn_debug.h"

extern unsigned long geGopOsdFlag;
//=============================================================================
// enum
//=============================================================================
typedef enum
{
    E_HAL_RGN_CMDQ_VPE_ID_0 = 0,
    E_HAL_RGN_CMDQ_DIVP_ID_1,
	E_HAL_RGN_CMDQ_LDC_ID_2,
    E_HAL_RGN_CMDQ_ID_NUM,
}HalRgnCmdqIdType_e;
typedef enum
{
    E_HAL_RGN_OSD_FLAG_PORT0 = 0x1,
    E_HAL_RGN_OSD_FLAG_PORT1 = 0x2,
    E_HAL_RGN_OSD_FLAG_PORT2 = 0x4,
    E_HAL_RGN_OSD_FLAG_PORT3 = 0x8,
    E_HAL_RGN_OSD_FLAG_DIP   = 0x10,
}HalRgnOsdFlag_e;

//=============================================================================
// struct
//=============================================================================
//=============================================================================
// Defines
//=============================================================================
#define HAL_RGN_GOP_NUM 2
#define HAL_RGN_GOP_GWIN_NUM 1
#define HAL_RGN_OSD_REAL_NUM 2
#define HAL_RGN_OSD_NUM 2
#define HAL_RGN_DISP_GWIN_CNT 0
#define RIU_32_EN 0

#ifndef bool
#define  bool   unsigned char
#endif

//=============================================================================

//=============================================================================
#ifndef __HAL_RGN_COMMON_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE void HalRgnInitCmdq(void);
INTERFACE void HalRgnUseCmdq(bool bEn);
INTERFACE void HalRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalRgnCmdqIdType_e eHalCmdqId);
INTERFACE void HalRgnWrite2Byte(u32 u32Reg, u16 u16Val,HalRgnCmdqIdType_e eCmdqId);
INTERFACE void HalRgnWrite2ByteMsk(u32 u32Reg, u16 u16Val, u16 u16Mask,HalRgnCmdqIdType_e eCmdqId);
INTERFACE u16  HalRgnRead2Byte(u32 u32Reg);
INTERFACE bool HalRgnCheckBindRation(HalRgnOsdFlag_e *enFlag);
INTERFACE void HalRgnUpdateReg(void);
INTERFACE void HalRgnInit(void);
INTERFACE void HalRgnDeinit(void);

#undef INTERFACE
#endif /* __HAL_GOP_H__ */
