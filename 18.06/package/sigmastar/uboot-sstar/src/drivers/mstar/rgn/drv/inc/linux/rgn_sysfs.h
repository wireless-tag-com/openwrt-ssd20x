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

#ifndef __RGN_SYSFS_H__
#define __RGN_SYSFS_H__
//=============================================================================
// enum
//=============================================================================
//=============================================================================
// struct
//=============================================================================
//=============================================================================
// Defines
//=============================================================================
//=============================================================================

//=============================================================================
#ifndef __RGN_SYSFS_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE void RgnSysfsInit(void);
INTERFACE void RgnSysfsDeInit(void);


#undef INTERFACE
#endif /* __HAL_SYSFS_H__ */
