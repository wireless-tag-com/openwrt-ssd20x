/*
* mhal_miu.c- Sigmastar
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
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif
#include "MsTypes.h"
#include "mdrv_types.h"
#include "mdrv_miu.h"
#include "mdrv_system.h"
#include "regMIU.h"
#include "mhal_miu.h"
#include "registers.h"
#include "ms_platform.h"
//-------------------------------------------------------------------------------------------------
//  Macro Define
//-------------------------------------------------------------------------------------------------

#define MIU_CLIENT_GP0                  \
/* 0 */    MIU_CLIENT_NONE,             \
/* 1 */    MIU_CLIENT_MIIC0_RW,         \
/* 2 */    MIU_CLIENT_MIIC1_RW,         \
/* 3 */    MIU_CLIENT_DUMMY_G0C3,       \
/* 4 */    MIU_CLIENT_JPE_R,            \
/* 5 */    MIU_CLIENT_JPE_W,            \
/* 6 */    MIU_CLIENT_BACH_RW,          \
/* 7 */    MIU_CLIENT_AESDMA_RW,        \
/* 8 */    MIU_CLIENT_DUMMY_G08,        \
/* 9 */    MIU_CLIENT_DUMMY_G09,        \
/* A */    MIU_CLIENT_MCU51_RW,         \
/* B */    MIU_CLIENT_URDMA_RW,         \
/* C */    MIU_CLIENT_BDMA_RW,          \
/* D */    MIU_CLIENT_MOVDMA0_RW,       \
/* E */    MIU_CLIENT_DUMMY_G0CE,       \
/* F */    MIU_CLIENT_DUMMY_G0CF

#define MIU_CLIENT_GP1                  \
/* 0 */    MIU_CLIENT_CMDQ0_R,          \
/* 1 */    MIU_CLIENT_SATA_RW,          \
/* 2 */    MIU_CLIENT_EMAC_RW,          \
/* 3 */    MIU_CLIENT_EMAC1_RW,         \
/* 4 */    MIU_CLIENT_DUMMY_G1C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G1C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G1C6,       \
/* 7 */    MIU_CLIENT_USB20_1_RW,       \
/* 8 */    MIU_CLIENT_USB20_2_RW,       \
/* 9 */    MIU_CLIENT_USB20_3_RW,       \
/* A */    MIU_CLIENT_GE_RW,            \
/* B */    MIU_CLIENT_DUMMY_G1CB,       \
/* C */    MIU_CLIENT_DUMMY_G1CC,       \
/* D */    MIU_CLIENT_DUMMY_G1CD,       \
/* E */    MIU_CLIENT_DUMMY_G1CE,       \
/* F */    MIU_CLIENT_SDIO30_RW

#define MIU_CLIENT_GP2                  \
/* 0 */    MIU_CLIENT_DIP0_R,           \
/* 1 */    MIU_CLIENT_DIP0_W,           \
/* 2 */    MIU_CLIENT_GOP0_R,           \
/* 3 */    MIU_CLIENT_GOP1_R,           \
/* 4 */    MIU_CLIENT_MOP_ROT0_Y_R,     \
/* 5 */    MIU_CLIENT_MOP_ROT0_C_R,     \
/* 6 */    MIU_CLIENT_MOP_ROT1_Y_R,     \
/* 7 */    MIU_CLIENT_MOP_ROT1_C_R,     \
/* 8 */    MIU_CLIENT_MOP_S_Y_R,        \
/* 9 */    MIU_CLIENT_MOP_S_C_R,        \
/* A */    MIU_CLIENT_MOP_G_Y_R,        \
/* B */    MIU_CLIENT_MOP_G_C_R,        \
/* C */    MIU_CLIENT_DUMMY_G2CC,       \
/* D */    MIU_CLIENT_DUMMY_G2CD,       \
/* E */    MIU_CLIENT_WAVE511_RW,       \
/* F */    MIU_CLIENT_DIAMOND_RW

#define MIU_CLIENT_GP3                  \
/* 0 */    MIU_CLIENT_DUMMY_G3C0,       \
/* 1 */    MIU_CLIENT_DUMMY_G3C1,       \
/* 2 */    MIU_CLIENT_DUMMY_G3C2,       \
/* 3 */    MIU_CLIENT_DUMMY_G3C3,       \
/* 4 */    MIU_CLIENT_DUMMY_G3C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G3C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G3C6,       \
/* 7 */    MIU_CLIENT_DUMMY_G3C7,       \
/* 8 */    MIU_CLIENT_DUMMY_G3C8,       \
/* 9 */    MIU_CLIENT_DUMMY_G3C9,       \
/* A */    MIU_CLIENT_DUMMY_G3CA,       \
/* B */    MIU_CLIENT_DUMMY_G3CB,       \
/* C */    MIU_CLIENT_DUMMY_G3CC,       \
/* D */    MIU_CLIENT_DUMMY_G3CD,       \
/* E */    MIU_CLIENT_DUMMY_G3CE,       \
/* F */    MIU_CLIENT_DUMMY_G3CF

#define MIU_CLIENT_GP4                  \
/* 0 */    MIU_CLIENT_DUMMY_G4C0,       \
/* 1 */    MIU_CLIENT_DUMMY_G4C1,       \
/* 2 */    MIU_CLIENT_DUMMY_G4C2,       \
/* 3 */    MIU_CLIENT_DUMMY_G4C3,       \
/* 4 */    MIU_CLIENT_DUMMY_G4C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G4C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G4C6,       \
/* 7 */    MIU_CLIENT_DUMMY_G4C7,       \
/* 8 */    MIU_CLIENT_DUMMY_G4C8,       \
/* 9 */    MIU_CLIENT_DUMMY_G4C9,       \
/* A */    MIU_CLIENT_DUMMY_G4CA,       \
/* B */    MIU_CLIENT_DUMMY_G4CB,       \
/* C */    MIU_CLIENT_DUMMY_G4CC,       \
/* D */    MIU_CLIENT_DUMMY_G4CD,       \
/* E */    MIU_CLIENT_DUMMY_G4CE,       \
/* F */    MIU_CLIENT_DUMMY_G4CF

#define MIU_CLIENT_GP5                  \
/* 0 */    MIU_CLIENT_DUMMY_G5C0,       \
/* 1 */    MIU_CLIENT_DUMMY_G5C1,       \
/* 2 */    MIU_CLIENT_DUMMY_G5C2,       \
/* 3 */    MIU_CLIENT_DUMMY_G5C3,       \
/* 4 */    MIU_CLIENT_DUMMY_G5C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G5C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G5C6,       \
/* 7 */    MIU_CLIENT_DUMMY_G5C7,       \
/* 8 */    MIU_CLIENT_DUMMY_G5C8,       \
/* 9 */    MIU_CLIENT_DUMMY_G5C9,       \
/* A */    MIU_CLIENT_DUMMY_G5CA,       \
/* B */    MIU_CLIENT_DUMMY_G5CB,       \
/* C */    MIU_CLIENT_DUMMY_G5CC,       \
/* D */    MIU_CLIENT_DUMMY_G5CD,       \
/* E */    MIU_CLIENT_DUMMY_G5CE,       \
/* F */    MIU_CLIENT_DUMMY_G5CF

#define MIU_CLIENT_GP6                  \
/* 0 */    MIU_CLIENT_DUMMY_G6C0,       \
/* 1 */    MIU_CLIENT_DUMMY_G6C1,       \
/* 2 */    MIU_CLIENT_DUMMY_G6C2,       \
/* 3 */    MIU_CLIENT_DUMMY_G6C3,       \
/* 4 */    MIU_CLIENT_DUMMY_G6C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G6C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G6C6,       \
/* 7 */    MIU_CLIENT_DUMMY_G6C7,       \
/* 8 */    MIU_CLIENT_DUMMY_G6C8,       \
/* 9 */    MIU_CLIENT_DUMMY_G6C9,       \
/* A */    MIU_CLIENT_DUMMY_G6CA,       \
/* B */    MIU_CLIENT_DUMMY_G6CB,       \
/* C */    MIU_CLIENT_DUMMY_G6CC,       \
/* D */    MIU_CLIENT_DUMMY_G6CD,       \
/* E */    MIU_CLIENT_DUMMY_G6CE,       \
/* F */    MIU_CLIENT_DUMMY_G6CF

#define MIU_CLIENT_GP7                  \
/* 0 */    MIU_CLIENT_MIPS_RW,          \
/* 1 */    MIU_CLIENT_DUMMY_G7C1,       \
/* 2 */    MIU_CLIENT_DUMMY_G7C2,       \
/* 3 */    MIU_CLIENT_DUMMY_G7C3,       \
/* 4 */    MIU_CLIENT_DUMMY_G7C4,       \
/* 5 */    MIU_CLIENT_DUMMY_G7C5,       \
/* 6 */    MIU_CLIENT_DUMMY_G7C6,       \
/* 7 */    MIU_CLIENT_DUMMY_G7C7,       \
/* 8 */    MIU_CLIENT_DUMMY_G7C8,       \
/* 9 */    MIU_CLIENT_DUMMY_G7C9,       \
/* A */    MIU_CLIENT_DUMMY_G7CA,       \
/* B */    MIU_CLIENT_DUMMY_G7CB,       \
/* C */    MIU_CLIENT_DUMMY_G7CC,       \
/* D */    MIU_CLIENT_DUMMY_G7CD,       \
/* E */    MIU_CLIENT_DUMMY_G7CE,       \
/* F */    MIU_CLIENT_DUMMY_G7CF

#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < ARM_MIU1_BASE_ADDR) \
                                                        {MiuSel = E_MIU_0; Offset = PhysAddr - ARM_MIU0_BASE_ADDR;} \
                                                     else if ((PhysAddr >= ARM_MIU1_BASE_ADDR) && (PhysAddr < ARM_MIU2_BASE_ADDR)) \
                                                        {MiuSel = E_MIU_1; Offset = PhysAddr - ARM_MIU1_BASE_ADDR;} \
                                                     else \
                                                        {MiuSel = E_MIU_2; Offset = PhysAddr - ARM_MIU2_BASE_ADDR;}

#define MIU_HAL_ERR(fmt, args...)   printk(KERN_ERR "miu hal error %s:%d" fmt,__FUNCTION__,__LINE__,## args)

//-------------------------------------------------------------------------------------------------
//  Local Variable
//-------------------------------------------------------------------------------------------------

const eMIUClientID clientTbl[MIU_MAX_DEVICE][MIU_MAX_TBL_CLIENT] =
{
    {
        MIU_CLIENT_GP0,
        MIU_CLIENT_GP1,
        MIU_CLIENT_GP2,
        MIU_CLIENT_GP3,
        MIU_CLIENT_GP4,
        MIU_CLIENT_GP5,
        MIU_CLIENT_GP6,
        MIU_CLIENT_GP7
    },
    {
        MIU_CLIENT_GP0,
        MIU_CLIENT_GP1,
        MIU_CLIENT_GP2,
        MIU_CLIENT_GP3,
        MIU_CLIENT_GP4,
        MIU_CLIENT_GP5,
        MIU_CLIENT_GP6,
        MIU_CLIENT_GP7
    }
};

static MS_U16 clientId_KernelProtect[IDNUM_KERNELPROTECT] =
{
    MIU_CLIENT_MIPS_RW,
    MIU_CLIENT_USB20_1_RW,
    MIU_CLIENT_USB20_2_RW,
    MIU_CLIENT_USB20_3_RW,
    MIU_CLIENT_MIIC1_RW,
    MIU_CLIENT_EMAC_RW,
    MIU_CLIENT_EMAC1_RW,
    MIU_CLIENT_SATA_RW,
    MIU_CLIENT_SDIO30_RW,
    MIU_CLIENT_AESDMA_RW,
    MIU_CLIENT_URDMA_RW,
    MIU_CLIENT_BDMA_RW,
    MIU_CLIENT_MOVDMA0_RW,
    MIU_CLIENT_GE_RW,
    0,
};

#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
static MS_U32 m_u32MiuMapBase = 0xFD200000UL;   //default set to arm 32bit platform
#elif defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
static ptrdiff_t m_u32MiuMapBase;
#endif

static MS_BOOL IDEnables[MIU_MAX_DEVICE][MIU_MAX_PROTECT_BLOCK][MIU_MAX_PROTECT_ID] = {{{0},{0},{0},{0}}, {{0},{0},{0},{0}}}; //ID enable for protect block 0~3
static MS_U16 IDList[MIU_MAX_DEVICE][MIU_MAX_PROTECT_ID] = {{0}, {0}}; //IDList for protection

//-------------------------------------------------------------------------------------------------
//  MTLB HAL internal function
//-------------------------------------------------------------------------------------------------

static MS_U32 HAL_MIU_BA2PA(MS_U32 u32BusAddr)
{
    MS_PHYADDR u32PhyAddr = 0x0UL;

    // pa = ba - offset
    if ((u32BusAddr >= ARM_MIU0_BUS_BASE) && (u32BusAddr < ARM_MIU1_BUS_BASE))
        u32PhyAddr = u32BusAddr - ARM_MIU0_BUS_BASE + ARM_MIU0_BASE_ADDR;
    else if ((u32BusAddr >= ARM_MIU1_BUS_BASE) && (u32BusAddr < ARM_MIU2_BUS_BASE))
        u32PhyAddr = u32BusAddr - ARM_MIU1_BUS_BASE + ARM_MIU1_BASE_ADDR;
    else
        u32PhyAddr = u32BusAddr - ARM_MIU2_BUS_BASE + ARM_MIU2_BASE_ADDR;

    return u32PhyAddr;
}

static MS_S16 HAL_MIU_GetClientIndex(MS_U8 u8MiuSel, eMIUClientID eClientID)
{
    MS_U8 idx = 0;

    if (MIU_MAX_DEVICE <= u8MiuSel) {
        MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
        return (-1);
    }

    for (idx = 0; idx < MIU_MAX_TBL_CLIENT; idx++) {
        if (eClientID == clientTbl[u8MiuSel][idx])
            return idx;
    }
    return (-1);
}

static MS_U8 HAL_MIU_ReadByte(MS_U32 u32RegProtectId)
{
#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U8*)(m_u32MiuMapBase))[(u32RegProtectId << 1) - (u32RegProtectId & 1)];
}

static MS_U16 HAL_MIU_Read2Byte(MS_U32 u32RegProtectId)
{
#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U16*)(m_u32MiuMapBase))[u32RegProtectId];
}

static MS_BOOL HAL_MIU_WriteByte(MS_U32 u32RegProtectId, MS_U8 u8Val)
{
    if (!u32RegProtectId) {
        MIU_HAL_ERR("%s reg err\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U8*)(m_u32MiuMapBase))[(u32RegProtectId << 1) - (u32RegProtectId & 1)] = u8Val;

    return TRUE;
}

static MS_BOOL HAL_MIU_Write2Byte(MS_U32 u32RegProtectId, MS_U16 u16Val)
{
    if (!u32RegProtectId) {
        MIU_HAL_ERR("%s reg err\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U16*)(m_u32MiuMapBase))[u32RegProtectId] = u16Val;

    return TRUE;
}

static void HAL_MIU_WriteByteMask(MS_U32 u32RegOffset, MS_U8 u8Mask, MS_BOOL bEnable)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(u32RegOffset);

    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_MIU_WriteByte(u32RegOffset, u8Val);
}

static void HAL_MIU_Write2BytesMask(MS_U32 u32RegOffset, MS_U16 u16Mask, MS_BOOL bEnable)
{
    MS_U16 u16Val = HAL_MIU_Read2Byte(u32RegOffset);

    u16Val = (bEnable) ? (u16Val | u16Mask) : (u16Val & ~u16Mask);
    HAL_MIU_Write2Byte(u32RegOffset, u16Val);
}

static void HAL_MIU_SetProtectIDReg(MS_U32 u32RegBase, MS_U8 u8MiuSel, MS_U16 u16ClientID)
{
    MS_S16 sVal = HAL_MIU_GetClientIndex(u8MiuSel, (eMIUClientID)u16ClientID);
    MS_S16 sIDVal = 0;

    if (0 > sVal) {
        sVal = 0;
    }

    sIDVal = HAL_MIU_ReadByte(u32RegBase);
    sIDVal &= 0x80;
    sIDVal |= sVal;
    HAL_MIU_WriteByte(u32RegBase, sIDVal);
}

static MS_BOOL HAL_MIU_SetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegProtectIdEn)
{
    MS_U32 u32index0, u32index1;
    MS_U16 u16ID = 0;
    MS_U16 u16IdEnable = 0;
    MS_U8 u8isfound0, u8isfound1;

    // Reset IDEnables for protect u8Blockx
    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u16ID = pu8ProtectId[u32index0];

        // Unused ID
        if (u16ID == 0)
           continue;

        u8isfound0 = FALSE;

        for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
        {
            if (IDList[u8MiuSel][u32index1] == u16ID)
            {
                // ID reused former setting
                IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                u8isfound0 = TRUE;
                break;
            }
        }

        // Need to create new ID in IDList
        if (u8isfound0 != TRUE)
        {
            u8isfound1 = FALSE;

            for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
            {
                if (IDList[u8MiuSel][u32index1] == 0)
                {
                    IDList[u8MiuSel][u32index1] = u16ID;
                    IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                    u8isfound1 = TRUE;
                    break;
                }
            }

            // ID overflow
            if (u8isfound1 == FALSE) {
                return FALSE;
            }
        }
    }

    u16IdEnable = 0;

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        if (IDEnables[u8MiuSel][u8Blockx][u32index0] == 1) {
            u16IdEnable |= (1 << u32index0);
        }
    }

    HAL_MIU_Write2Byte(u32RegProtectIdEn, u16IdEnable);

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectIDReg(u32RegAddrID + u32index0, u8MiuSel, IDList[u8MiuSel][u32index0]);
    }

    return TRUE;
}

static MS_BOOL HAL_MIU_ResetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegProtectIdEn)
{
    MS_U32 u32index0, u32index1;
    MS_U8 u8isIDNoUse = 0;
    MS_U16 u16IdEnable = 0;

    // Reset IDEnables for protect u8Blockx
    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    u16IdEnable = 0x0UL;

    HAL_MIU_Write2Byte(u32RegProtectIdEn, u16IdEnable);

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u8isIDNoUse  = FALSE;

        for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_BLOCK; u32index1++)
        {
            if (IDEnables[u8MiuSel][u32index1][u32index0] == 1)
            {
                // Protect ID is still be used
                u8isIDNoUse = FALSE;
                break;
            }
            u8isIDNoUse  = TRUE;
        }

        if (u8isIDNoUse == TRUE) {
            IDList[u8MiuSel][u32index0] = 0;
        }
    }

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectIDReg(u32RegAddrID + u32index0, u8MiuSel, IDList[u8MiuSel][u32index0]);
    }

    return TRUE;
}

#define GET_HIT_BLOCK(regval)       BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_NO)
#define GET_HIT_CLIENT(regval)      BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_ID)

MS_BOOL HAL_MIU_GetHitProtectInfo(MS_U8 u8MiuSel, MIU_PortectInfo *pInfo)
{
    MS_U16 u16Ret = 0;
    MS_U16 u16LoAddr = 0;
    MS_U16 u16HiAddr = 0;
    MS_U32 u32RegBase = (u8MiuSel) ? MIU1_REG_BASE : MIU_REG_BASE;
    MS_U32 u32EndAddr = 0;
    char clientName[40];
    if (!pInfo) {
        return FALSE;
    }

    u16Ret      = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_STATUS);
    u16LoAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_LOADDR);
    u16HiAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_HIADDR);

    HAL_MIU_ClientIdToName((MS_U8)(GET_HIT_CLIENT(u16Ret)), clientName);

    if (REG_MIU_PROTECT_HIT_FALG & u16Ret)
    {
        pInfo->bHit         = TRUE;
        pInfo->u8Block      = (MS_U8)GET_HIT_BLOCK(u16Ret);
        pInfo->u8Group      = (MS_U8)(GET_HIT_CLIENT(u16Ret) >> 4);
        pInfo->u8ClientID   = (MS_U8)(GET_HIT_CLIENT(u16Ret) & 0x0F);
        pInfo->uAddress     = (MS_U32)((u16HiAddr << 16) | u16LoAddr);
        pInfo->uAddress     = pInfo->uAddress * MIU_PROTECT_ADDRESS_UNIT;

        u32EndAddr = (pInfo->uAddress + MIU_PROTECT_ADDRESS_UNIT - 1);

        if (pInfo->u8Block == 6 || pInfo->u8Block == 7)
        {
            printk(KERN_EMERG "MIU%u %s Out of Range Client:%s ID:%u-%u Hitted_Addr:0x%x<->0x%x (Valid Range: 0x00000000 ~ 0x%08X)\n",
                   u8MiuSel, (pInfo->u8Block == 6)? "Read" : "Write", clientName,
                   pInfo->u8Group, pInfo->u8ClientID,
                   pInfo->uAddress, u32EndAddr, HAL_MIU_ProtectDramSize());
        }
        else
        {
            printk(KERN_EMERG "MIU%u Block:%u Client:%s ID:%u-%u Hitted_Addr:0x%x<->0x%x\n",
                   u8MiuSel, pInfo->u8Block, clientName,
                   pInfo->u8Group, pInfo->u8ClientID,
                   pInfo->uAddress, u32EndAddr);
        }

        // Clear log
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, TRUE);
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, FALSE);

        if ((MIU_CLIENT_WAVE511_RW == GET_HIT_CLIENT(u16Ret)) && (pInfo->u8Block == 6))
        {
             pInfo->bHit = FALSE;
        }

        if (MIU_CLIENT_WAVE511_RW == GET_HIT_CLIENT(u16Ret)) {
            printk(KERN_EMERG "ignor, miu already protect, MIU%u Block:%u Client:%s ID:%u-%u Hitted_Addr:0x%x<->0x%x\n",
                   u8MiuSel, pInfo->u8Block, clientName,
                   pInfo->u8Group, pInfo->u8ClientID,
                   pInfo->uAddress, u32EndAddr);
            pInfo->bHit = FALSE;
        }

    }

    return TRUE;
}

MS_BOOL HAL_MIU_GetProtectIdEnVal(MS_U8 u8MiuSel, MS_U8 u8BlockId, MS_U8 u8ProtectIdIndex)
{
    return IDEnables[u8MiuSel][u8BlockId][u8ProtectIdIndex];
}

MS_U16* HAL_MIU_GetDefaultKernelProtectClientID(void)
{
     if (IDNUM_KERNELPROTECT > 0) {
         return (MS_U16 *)&clientId_KernelProtect[0];
     }
     return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_Protect()
/// @brief \b Function \b Description: Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 4)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDList to write
/// @param u32Start        \b IN     : Starting bus address
/// @param u32End          \b IN     : End bus address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Protect(    MS_U8   u8Blockx,
                            MS_U16  *pu8ProtectId,
                            MS_U32  u32BusStart,
                            MS_U32  u32BusEnd,
                            MS_BOOL bSetFlag)
{
    MS_U32 u32RegProtectId = 0;
    MS_U32 u32RegBase = 0;
    MS_U32 u32RegStartAddr = 0;
    MS_U32 u32RegStartEnd = 0;
    MS_U32 u32RegAddrMSB = 0;
    MS_U32 u32RegProtectIdEn = 0;
    MS_U32 u32RegRWProtectEn = 0;
    MS_U32 u32StartOffset = 0;
    MS_U32 u32EndOffset = 0;
    MS_U8  u8MiuSel = 0;
    MS_U16 u16Data = 0;
    MS_U16 u16Data1 = 0;
    MS_U16 u16Data2 = 0;
    MS_U8  u8Data = 0;
    MS_U32 u32Start = 0, u32End = 0;

    u32Start = HAL_MIU_BA2PA(u32BusStart);
    u32End   = HAL_MIU_BA2PA(u32BusEnd);

    // Get MIU selection and offset
    _phy_to_miu_offset(u8MiuSel, u32EndOffset, u32End);
    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32Start);

    u32Start = u32StartOffset;
    u32End   = u32EndOffset;

    // Parameter check
    if (u8Blockx >= E_MIU_BLOCK_NUM)
    {
        MIU_HAL_ERR("Err: Blk Num out of range\n");
        return FALSE;
    }
    else if (((u32Start & ((1 << MIU_PAGE_SHIFT) -1)) != 0) ||
             ((u32End & ((1 << MIU_PAGE_SHIFT) -1)) != 0))
    {
        MIU_HAL_ERR("Err: Protected addr not 8KB aligned\n");
        return FALSE;
    }
    else if (u32Start >= u32End)
    {
        MIU_HAL_ERR("Err: Invalid end addr\n");
        return FALSE;
    }


    if (u8MiuSel == E_MIU_0)
    {
        u32RegProtectId = MIU_PROTECT_ID0;
        u32RegBase = MIU_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u8Data = 1 << 0;
                u32RegRWProtectEn = MIU_PROTECT0_EN;
                u32RegStartAddr = MIU_PROTECT0_START;
                u32RegStartEnd = MIU_PROTECT0_END;
                u32RegProtectIdEn = MIU_PROTECT0_ID_ENABLE;
                u32RegAddrMSB = MIU_PROTECT0_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u8Data = 1 << 1;
                u32RegRWProtectEn = MIU_PROTECT1_EN;
                u32RegStartAddr = MIU_PROTECT1_START;
                u32RegStartEnd = MIU_PROTECT1_END;
                u32RegProtectIdEn = MIU_PROTECT1_ID_ENABLE;
                u32RegAddrMSB = MIU_PROTECT1_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u8Data = 1 << 2;
                u32RegRWProtectEn = MIU_PROTECT2_EN;
                u32RegStartAddr = MIU_PROTECT2_START;
                u32RegStartEnd = MIU_PROTECT2_END;
                u32RegProtectIdEn = MIU_PROTECT2_ID_ENABLE;
                u32RegAddrMSB = MIU_PROTECT2_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u8Data = 1 << 3;
                u32RegRWProtectEn = MIU_PROTECT3_EN;
                u32RegStartAddr = MIU_PROTECT3_START;
                u32RegStartEnd = MIU_PROTECT3_END;
                u32RegProtectIdEn = MIU_PROTECT3_ID_ENABLE;
                u32RegAddrMSB = MIU_PROTECT3_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            case E_MIU_BLOCK_4:
                u8Data = 1 << 0;
                u32RegRWProtectEn = MIU_PROTECT4_EN;
                u32RegStartAddr = MIU_PROTECT4_START;
                u32RegStartEnd = MIU_PROTECT4_END;
                u32RegProtectIdEn = MIU_PROTECT4_ID_ENABLE;
                u32RegAddrMSB = MIU_PROTECT4_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            default:
                return FALSE;
        }
    }
    else if (u8MiuSel == E_MIU_1)
    {
        u32RegProtectId = MIU1_PROTECT_ID0;
        u32RegBase = MIU1_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u8Data = 1 << 0;
                u32RegRWProtectEn = MIU1_PROTECT0_EN;
                u32RegStartAddr = MIU1_PROTECT0_START;
                u32RegStartEnd = MIU1_PROTECT0_END;
                u32RegProtectIdEn = MIU1_PROTECT0_ID_ENABLE;
                u32RegAddrMSB = MIU1_PROTECT0_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u8Data = 1 << 1;
                u32RegRWProtectEn = MIU1_PROTECT1_EN;
                u32RegStartAddr = MIU1_PROTECT1_START;
                u32RegStartEnd = MIU1_PROTECT1_END;
                u32RegProtectIdEn = MIU1_PROTECT1_ID_ENABLE;
                u32RegAddrMSB = MIU1_PROTECT1_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u8Data = 1 << 2;
                u32RegRWProtectEn = MIU1_PROTECT2_EN;
                u32RegStartAddr = MIU1_PROTECT2_START;
                u32RegStartEnd = MIU1_PROTECT2_END;
                u32RegProtectIdEn = MIU1_PROTECT2_ID_ENABLE;
                u32RegAddrMSB = MIU1_PROTECT2_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u8Data = 1 << 3;
                u32RegRWProtectEn = MIU1_PROTECT3_EN;
                u32RegStartAddr = MIU1_PROTECT3_START;
                u32RegStartEnd = MIU1_PROTECT3_END;
                u32RegProtectIdEn = MIU1_PROTECT3_ID_ENABLE;
                u32RegAddrMSB = MIU1_PROTECT3_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            case E_MIU_BLOCK_4:
                u8Data = 1 << 0;
                u32RegRWProtectEn = MIU1_PROTECT4_EN;
                u32RegStartAddr = MIU1_PROTECT4_START;
                u32RegStartEnd = MIU1_PROTECT4_END;
                u32RegProtectIdEn = MIU1_PROTECT4_ID_ENABLE;
                u32RegAddrMSB = MIU1_PROTECT4_MSB;
                u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            default:
                return FALSE;
        }
    }
    else
    {
        MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
        return FALSE;
    }

    // Disable MIU write protect
    HAL_MIU_WriteByteMask(u32RegRWProtectEn, u8Data, DISABLE);

    if (bSetFlag)
    {
        // Set Protect IDList
        if (HAL_MIU_SetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegProtectId, u32RegProtectIdEn) == FALSE)
        {
            return FALSE;
        }

        // Set BIT29,30 of start/end address
        u16Data2 = u16Data2 | (MS_U16)((u32Start >> 29) << ((u8Blockx%4)*4));   // u16Data2 for start_ext addr
        u16Data1 = u16Data2 | (MS_U16)(((u32End - 1) >> 29) << ((u8Blockx%4)*4+2));
        HAL_MIU_Write2Byte(u32RegAddrMSB, u16Data1);

        // Start Address
        u16Data = (MS_U16)(u32Start >> MIU_PAGE_SHIFT);     // 8k unit
        HAL_MIU_Write2Byte(u32RegStartAddr, u16Data);

        // End Address
        u16Data = (MS_U16)((u32End >> MIU_PAGE_SHIFT)-1);   // 8k unit;
        HAL_MIU_Write2Byte(u32RegStartEnd, u16Data);

        // Enable MIU write protect
        HAL_MIU_WriteByteMask(u32RegRWProtectEn, u8Data, ENABLE);
    }
    else
    {
        // Reset Protect IDList
        HAL_MIU_ResetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegProtectId, u32RegProtectIdEn);
    }

    // Clear log
    HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, TRUE);
    HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, FALSE);

    // Mask PWR IRQ
    HAL_MIU_Write2BytesMask(REG_MIU_PROTECT_PWR_IRQ_MASK_OFFSET, REG_MIU_PROTECT_PWR_IRQ_MASK_BIT, TRUE);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ParseOccupiedResource
/// @brief \b Function  \b Description: Parse occupied resource to software structure
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_ParseOccupiedResource(void)
{
    MS_U8  u8MiuSel = 0;
    MS_U8  u8Blockx = 0;
    MS_U8  u8ClientID = 0;
    MS_U16 u16IdEnable = 0;
    MS_U32 u32index = 0;
    MS_U32 u32RegProtectId = 0;
    MS_U32 u32RegProtectIdEn = 0;

    for (u8MiuSel = E_MIU_0; u8MiuSel < MIU_MAX_DEVICE; u8MiuSel++)
    {
        for (u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)
        {
            if (u8MiuSel == E_MIU_0)
            {
                u32RegProtectId = MIU_PROTECT_ID0;

                switch (u8Blockx)
                {
                    case E_MIU_BLOCK_0:
                        u32RegProtectIdEn = MIU_PROTECT0_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_1:
                        u32RegProtectIdEn = MIU_PROTECT1_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_2:
                        u32RegProtectIdEn = MIU_PROTECT2_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_3:
                        u32RegProtectIdEn = MIU_PROTECT3_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_4:
                        u32RegProtectIdEn = MIU_PROTECT4_ID_ENABLE;
                        break;
                    default:
                        return FALSE;
                }
            }
            else if (u8MiuSel == E_MIU_1)
            {
                u32RegProtectId = MIU1_PROTECT_ID0;

                switch (u8Blockx)
                {
                case E_MIU_BLOCK_0:
                     u32RegProtectIdEn = MIU1_PROTECT0_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_1:
                     u32RegProtectIdEn = MIU1_PROTECT1_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_2:
                     u32RegProtectIdEn = MIU1_PROTECT2_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_3:
                     u32RegProtectIdEn = MIU1_PROTECT3_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_4:
                     u32RegProtectIdEn = MIU1_PROTECT4_ID_ENABLE;
                     break;
                 default:
                     return FALSE;
                }
            }
            else
            {
                MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel);
                return FALSE;
            }

            u16IdEnable = HAL_MIU_Read2Byte(u32RegProtectIdEn);

            for (u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
            {
                IDEnables[u8MiuSel][u8Blockx][u32index] = ((u16IdEnable >> u32index) & 0x1UL) ? 1: 0;
            }
        }

        for (u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
        {
            u8ClientID = HAL_MIU_ReadByte(u32RegProtectId + u32index) & 0x7F;
            IDList[u8MiuSel][u32index] = clientTbl[u8MiuSel][u8ClientID];
        }
    }

    return TRUE;
}

unsigned int HAL_MIU_ProtectDramSize(void)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(MIU_PROTECT_DDR_SIZE);

    u8Val = (u8Val >> 4) & 0xF;

    if (0 == u8Val) {
        MIU_HAL_ERR("MIU protect size undefined. Using 0x40000000\n");
        return 0x40000000;
    }
    return (0x1 << (20 + u8Val));
}

int HAL_MIU_ClientIdToName(MS_U16 clientId, char *clientName)
{
    int iRet = 0;

    if (!clientName) {
        iRet = -1;
        MIU_HAL_ERR("Wrong clientName\n");
        return iRet;
    }

    switch(clientId)
    {
        // group 0
        case MIU_CLIENT_NONE:
            strcpy(clientName, "NONE");
            break;
        case MIU_CLIENT_MIIC0_RW:
            strcpy(clientName, "MIIC0_RW");
            break;
        case MIU_CLIENT_MIIC1_RW:
            strcpy(clientName, "MIIC1_RW");
            break;
        case MIU_CLIENT_DUMMY_G0C3:
            strcpy(clientName, "DUMMY_G0C3");
            break;
        case MIU_CLIENT_JPE_W:
            strcpy(clientName, "JPE_W");
            break;
        case MIU_CLIENT_JPE_R:
            strcpy(clientName, "JPE_R");
            break;
        case MIU_CLIENT_BACH_RW:
            strcpy(clientName, "BACH_RW");
            break;
        case MIU_CLIENT_AESDMA_RW:
            strcpy(clientName, "AESDMA_RW");
            break;
        case MIU_CLIENT_DUMMY_G08:
            strcpy(clientName, "DUMMY_G0C8");
            break;
        case MIU_CLIENT_DUMMY_G09:
            strcpy(clientName, "DUMMY_G0C9");
            break;
        case MIU_CLIENT_MCU51_RW:
            strcpy(clientName, "MCU51_RW");
            break;
        case MIU_CLIENT_URDMA_RW:
            strcpy(clientName, "URDMA_RW");
            break;
        case MIU_CLIENT_BDMA_RW:
            strcpy(clientName, "BDMA_RW");
            break;
        case MIU_CLIENT_MOVDMA0_RW:
            strcpy(clientName, "MOVDMA0_RW");
            break;
        case MIU_CLIENT_DUMMY_G0CE:
            strcpy(clientName, "DUMMY_G0CE");
            break;
        case MIU_CLIENT_DUMMY_G0CF:
            strcpy(clientName, "DUMMY_G0CF");
            break;
        // group 1
        case MIU_CLIENT_CMDQ0_R:
            strcpy(clientName, "CMDQ0_R");
            break;
        case MIU_CLIENT_SATA_RW:
            strcpy(clientName, "SATA_RW");
            break;
        case MIU_CLIENT_EMAC_RW:
            strcpy(clientName, "EMAC_RW");
            break;
        case MIU_CLIENT_EMAC1_RW:
            strcpy(clientName, "EMAC1_RW");
            break;
        case MIU_CLIENT_DUMMY_G1C4:
            strcpy(clientName, "DUMMY_G1C4");
            break;
        case MIU_CLIENT_DUMMY_G1C5:
            strcpy(clientName, "DUMMY_G1C5");
            break;
        case MIU_CLIENT_DUMMY_G1C6:
            strcpy(clientName, "DUMMY_G1C6");
            break;
        case MIU_CLIENT_USB20_1_RW:
            strcpy(clientName, "USB20_1_RW");
            break;
        case MIU_CLIENT_USB20_2_RW:
            strcpy(clientName, "USB20_2_RW");
            break;
        case MIU_CLIENT_USB20_3_RW:
            strcpy(clientName, "USB20_3_RW");
            break;
        case MIU_CLIENT_GE_RW:
            strcpy(clientName, "GE_RW");
            break;
        case MIU_CLIENT_DUMMY_G1CB:
            strcpy(clientName, "DUMMY_G1CB");
            break;
        case MIU_CLIENT_DUMMY_G1CC:
            strcpy(clientName, "DUMMY_G1CC");
            break;
        case MIU_CLIENT_DUMMY_G1CD:
            strcpy(clientName, "DUMMY_G1CD");
            break;
        case MIU_CLIENT_DUMMY_G1CE:
            strcpy(clientName, "DUMMY_G1CE");
            break;
        case MIU_CLIENT_SDIO30_RW:
            strcpy(clientName, "SDIO30_RW");
            break;
        // group 2
        case MIU_CLIENT_DIP0_R:
            strcpy(clientName, "DIP0_R");
            break;
        case MIU_CLIENT_DIP0_W:
            strcpy(clientName, "DIP0_W");
            break;
        case MIU_CLIENT_GOP0_R:
            strcpy(clientName, "GOP0_R");
            break;
        case MIU_CLIENT_GOP1_R:
            strcpy(clientName, "GOP1_R");
            break;
        case MIU_CLIENT_MOP_ROT0_Y_R:
            strcpy(clientName, "MOP_ROT0_Y_R");
            break;
        case MIU_CLIENT_MOP_ROT0_C_R:
            strcpy(clientName, "MOP_ROT0_C_R");
            break;
        case MIU_CLIENT_MOP_ROT1_Y_R:
            strcpy(clientName, "MOP_ROT1_Y_R");
            break;
        case MIU_CLIENT_MOP_ROT1_C_R:
            strcpy(clientName, "MOP_ROT1_C_R");
            break;
        case MIU_CLIENT_MOP_S_Y_R:
            strcpy(clientName, "MOP_S_Y_R");
            break;
        case MIU_CLIENT_MOP_S_C_R:
            strcpy(clientName, "MOP_S_C_R");
            break;
        case MIU_CLIENT_MOP_G_Y_R:
            strcpy(clientName, "MOP_G_Y_R");
            break;
        case MIU_CLIENT_MOP_G_C_R:
            strcpy(clientName, "MOP_G_C_R");
            break;
        case MIU_CLIENT_DUMMY_G2CC:
            strcpy(clientName, "DUMMY_G2CC");
            break;
        case MIU_CLIENT_DUMMY_G2CD:
            strcpy(clientName, "DUMMY_G2CD");
            break;
        case MIU_CLIENT_WAVE511_RW:
            strcpy(clientName, "WAVE511_RW");
            break;
        case MIU_CLIENT_DIAMOND_RW:
            strcpy(clientName, "DIAMOND_RW");
            break;
        // DIAMOND
        case MIU_CLIENT_MIPS_RW:
            strcpy(clientName, "CPU_RW");
            break;
        default:
            MIU_HAL_ERR("Wrong clientId %d\n", clientId);
            iRet = -1;
            break;
    }
    return iRet;
}
EXPORT_SYMBOL(HAL_MIU_ClientIdToName);


int HAL_MIU_Enable_Goup1_Smaller_Group2(unsigned char u8Enable)
{

    if (u8Enable)
    {
        HAL_MIU_Write2Byte(REG_MIU_GROUP_PRIORITY, MIU_GROUP_PRIORITY_GROUP1_SMALLER_GROUP2);
    }
    else
    {
        HAL_MIU_Write2Byte(REG_MIU_GROUP_PRIORITY, MIU_GROUP_PRIORITY_GROUP1_NO_CHANGE_GROUP2);
    }

    return 0;
}


int clientId_KernelProtectToName(MS_U16 clientId, char *clientName)
{
    return HAL_MIU_ClientIdToName(clientId, clientName);
}
EXPORT_SYMBOL(clientId_KernelProtectToName);


//-------------------------------------------------------------------------------------------------
int HAL_MMU_SetRegion(unsigned short u16Region)
{
    MS_U16 u16CtrlRegVal;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x2)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    if (u16Region >> 5)
    {
        MIU_HAL_ERR("Region over range(0x%x)\n", u16Region);
        return -1;
    }

    u16CtrlRegVal = HAL_MIU_Read2Byte(REG_MMU_CTRL) & ~(BITS_RANGE(REG_MMU_CTRL_REGION_MASK));
    u16CtrlRegVal |= (u16Region << 8);

    HAL_MIU_Write2Byte(REG_MMU_CTRL, u16CtrlRegVal);

    return 0;
}


int HAL_MMU_SetRegionReplaceable(unsigned short u16Region, unsigned short u16ReplaceRegion)
{
    MS_U16 u16CtrlRegVal;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x1)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    if ((u16Region >> 5) || (u16ReplaceRegion >> 5))
    {
        MIU_HAL_ERR("Region(0x%X) or ReplaceRegion(0x%X) over range \n", u16Region,u16ReplaceRegion);
        return -1;
    }

    u16CtrlRegVal = HAL_MIU_Read2Byte(REG_MMU_CTRL) & ~(BITS_RANGE(REG_MMU_CTRL_REGION_MASK));
    u16CtrlRegVal |= (u16Region << 8);

    HAL_MIU_Write2Byte(REG_MMU_CTRL, u16CtrlRegVal);


    u16CtrlRegVal = HAL_MIU_Read2Byte(REG_MMU_REPLACE_MSB) & ~(BITS_RANGE(REG_MMU_REPLACE_MSB_MASK));
    u16CtrlRegVal |= u16ReplaceRegion;

    HAL_MIU_Write2Byte(REG_MMU_REPLACE_MSB, u16CtrlRegVal);

    return 0;
}




int HAL_MMU_Map(unsigned short u16PhyAddrEntry, unsigned short u16VirtAddrEntry)
{
    MS_U16 u16RegVal;

    if ((u16PhyAddrEntry >> 9) || (u16VirtAddrEntry >> 9))
    {
        MIU_HAL_ERR("Entry over range(Phy:0x%x, Virt:0x%x)\n", u16PhyAddrEntry, u16VirtAddrEntry);
        return -1;
    }

    // reg_mmu_wdata
    HAL_MIU_Write2Byte(REG_MMU_W_DATA, u16VirtAddrEntry);

    // reg_mmu_entry
    u16RegVal = REG_MMU_RW_ENTRY_MODE | u16PhyAddrEntry;
    HAL_MIU_Write2Byte(REG_MMU_RW_ENTRY, u16RegVal);

    // reg_mmu_access
    HAL_MIU_Write2Byte(REG_MMU_ACCESS, BIT0);

    return 0;
}

unsigned short HAL_MMU_MapQuery(unsigned short u16PhyAddrEntry)
{
    MS_U16 u16RegVal;

    if (u16PhyAddrEntry >> 9)
    {
        MIU_HAL_ERR("Entry over range(Phy:0x%x)\n", u16PhyAddrEntry);
        return -1;
    }

    // reg_mmu_entry
    HAL_MIU_Write2Byte(REG_MMU_RW_ENTRY, u16PhyAddrEntry);

    // reg_mmu_access
    HAL_MIU_Write2Byte(REG_MMU_ACCESS, BIT0);

    // delay 100ns for SRAM read latency(designer comfirmed)
    ndelay(100);

    // reg_mmu_rdata
    u16RegVal = HAL_MIU_Read2Byte(REG_MMU_R_DATA);

    return u16RegVal;
}

int HAL_MMU_UnMap(unsigned short u16PhyAddrEntry)
{
    MS_U16 u16RegVal;

    if (u16PhyAddrEntry >> 9)
    {
        MIU_HAL_ERR("Entry over range(Phy:0x%x)\n", u16PhyAddrEntry);
        return -1;
    }

    // reg_mmu_wdata
    HAL_MIU_Write2Byte(REG_MMU_W_DATA, MMU_INVALID_ENTRY_VAL);

    // reg_mmu_entry
    u16RegVal = REG_MMU_RW_ENTRY_MODE | u16PhyAddrEntry;
    HAL_MIU_Write2Byte(REG_MMU_RW_ENTRY, u16RegVal);

    // reg_mmu_access
    HAL_MIU_Write2Byte(REG_MMU_ACCESS, BIT0);

    return 0;
}

static MS_U16 u16ClientIdMap[MMU_MAX_CLIENT_NUM]={0};
static MS_U16 u16ClientSel=0;
int HAL_MMU_AddClientId(unsigned short u16ClientId)
{
    MS_U16 u16ClientIdx, u16RegVal;
    MS_U32 u32RegOffset;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x2)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    for (u16ClientIdx = 0; u16ClientIdx < MMU_MAX_CLIENT_NUM; u16ClientIdx++)
    {
        if (u16ClientIdMap[u16ClientIdx] == u16ClientId)
            return 0;

        if (u16ClientIdMap[u16ClientIdx] == 0)
            break;
    }

    if (u16ClientIdx != MMU_MAX_CLIENT_NUM)
    {
        switch (u16ClientIdx)
        {
            case 0:
            case 1:
                u32RegOffset = REG_MMU_CLIENT_ID_0_1;
                break;
            case 2:
            case 3:
                u32RegOffset = REG_MMU_CLIENT_ID_2_3;
                break;
            case 4:
            case 5:
                u32RegOffset = REG_MMU_CLIENT_ID_4_5;
                break;
            case 6:
            case 7:
                u32RegOffset = REG_MMU_CLIENT_ID_6_7;
                break;
            default:
                break;
        }

        u16RegVal = HAL_MIU_Read2Byte(u32RegOffset);
        if (!(u16ClientIdx & 0x1)) // ID=0,2,4,6
        {
            u16RegVal &= ~0x3F;
            u16RegVal |= (u16ClientId & 0x3F);
        }
        else // ID=1,3,5,7
        {
            u16RegVal &= ~0x3F00;
            u16RegVal |= ((u16ClientId << 8) & 0x3F00);
        }

        // Add client ID
        u16ClientIdMap[u16ClientIdx] = u16ClientId;
        HAL_MIU_Write2Byte(u32RegOffset, u16RegVal);

        // Enable this client ID
        u16ClientSel |= (1 << u16ClientIdx);
        HAL_MIU_Write2Byte(REG_MMU_CLIENT_ID_SEL, u16ClientSel);

        return 0;
    }
    else
        MIU_HAL_ERR("Client ID is full(max:%d)\n", MMU_MAX_CLIENT_NUM);

    return -1;
}

int HAL_MMU_RemoveClientId(unsigned short u16ClientId)
{
    MS_U16 u16ClientIdx, u16RegVal;
    MS_U32 u32RegOffset;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x2)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    for (u16ClientIdx = 0; u16ClientIdx < MMU_MAX_CLIENT_NUM; u16ClientIdx++)
    {
        if (u16ClientIdMap[u16ClientIdx] == u16ClientId)
            break;
    }

    if (u16ClientIdx != MMU_MAX_CLIENT_NUM)
    {
        switch (u16ClientIdx)
        {
            case 0:
            case 1:
                u32RegOffset = REG_MMU_CLIENT_ID_0_1;
                break;
            case 2:
            case 3:
                u32RegOffset = REG_MMU_CLIENT_ID_2_3;
                break;
            case 4:
            case 5:
                u32RegOffset = REG_MMU_CLIENT_ID_4_5;
                break;
            case 6:
            case 7:
                u32RegOffset = REG_MMU_CLIENT_ID_6_7;
                break;
            default:
                break;
        }

        u16RegVal = HAL_MIU_Read2Byte(u32RegOffset);
        if (!(u16ClientIdx & 0x1)) // ID=0,2,4,6
        {
            u16RegVal &= ~0x3F;
        }
        else // ID=1,3,5,7
        {
            u16RegVal &= ~0x3F00;
        }

        // Remove client ID
        u16ClientIdMap[u16ClientIdx] = 0;
        HAL_MIU_Write2Byte(u32RegOffset, u16RegVal);

        // Disable this client ID
        u16ClientSel &= ~(1 << u16ClientIdx);
        HAL_MIU_Write2Byte(REG_MMU_CLIENT_ID_SEL, u16ClientSel);

        return 0;
    }

    return 0;
}

int HAL_MMU_Enable(unsigned char u8Enable)
{
    MS_U16 u16CtrlRegVal;
    MS_U16 u16IrqRegVal;

    u16CtrlRegVal = HAL_MIU_Read2Byte(REG_MMU_CTRL);
    u16IrqRegVal = HAL_MIU_Read2Byte(REG_MMU_IRQ_CTRL);

    if (u8Enable)
    {
        u16CtrlRegVal |= REG_MMU_CTRL_ENABLE;
        // Enable IRQ
        //u16IrqRegVal |= (REG_MMU_IRQ_RW_MASK | REG_MMU_IRQ_RD_MASK | REG_MMU_IRQ_WR_MASK);
        u16IrqRegVal &= ~(REG_MMU_IRQ_RW_MASK | REG_MMU_IRQ_RD_MASK | REG_MMU_IRQ_WR_MASK);
    }
    else
    {
        u16CtrlRegVal &= ~REG_MMU_CTRL_ENABLE;
        // Disable IRQ
        //u16IrqRegVal &= ~(REG_MMU_IRQ_RW_MASK | REG_MMU_IRQ_RD_MASK | REG_MMU_IRQ_WR_MASK);
        u16IrqRegVal |= (REG_MMU_IRQ_RW_MASK | REG_MMU_IRQ_RD_MASK | REG_MMU_IRQ_WR_MASK);
    }

    HAL_MIU_Write2Byte(REG_MMU_CTRL, u16CtrlRegVal);
    HAL_MIU_Write2Byte(REG_MMU_IRQ_CTRL, u16IrqRegVal);

    return 0;
}

int HAL_MMU_Reset(void)
{
    MS_U16 u16RetryNum=100;

    HAL_MIU_Write2Byte(REG_MMU_CTRL, 0x0);
    HAL_MIU_Write2Byte(REG_MMU_CTRL, REG_MMU_CTRL_RESET | REG_MMU_CTRL_RESET_INIT_VAL);

    do {
        if (HAL_MIU_Read2Byte(REG_MMU_CTRL) & REG_MMU_CTRL_INIT_DONE)
        {
            HAL_MIU_Write2Byte(REG_MMU_CTRL, 0x0);
            return 0;
        }

        u16RetryNum--;
    } while (u16RetryNum > 0);

    MIU_HAL_ERR("Reset timeout!\n");

    return -1;
}

unsigned int HAL_MMU_Status(unsigned short *u16PhyAddrEntry, unsigned short *u16ClientId, unsigned char *u8IsWriteCmd)
{
    MS_U16 u16IrqRegVal;
    unsigned int u32Status=0;

    u16IrqRegVal = HAL_MIU_Read2Byte(REG_MMU_IRQ_CTRL);

    if (u16IrqRegVal & REG_MMU_IRQ_RW_FLAG)
    {
        *u16PhyAddrEntry = HAL_MIU_Read2Byte(REG_MMU_COLLISION_ENTRY);
        u16IrqRegVal |= REG_MMU_IRQ_RW_CLR;
        u32Status |= E_HAL_MMU_STATUS_RW_COLLISION;
    }

    if ((u16IrqRegVal & REG_MMU_IRQ_RD_FLAG) || (u16IrqRegVal & REG_MMU_IRQ_WR_FLAG))
    {
        *u16ClientId = BITS_RANGE_VAL(u16IrqRegVal, REG_MMU_IRQ_INVALID_ID_MASK);
        *u8IsWriteCmd = (u16IrqRegVal & REG_MMU_IRQ_INVALID_RW)?1:0;

        if (u16IrqRegVal & REG_MMU_IRQ_RD_FLAG)
        {
            u16IrqRegVal |= REG_MMU_IRQ_RD_CLR;
            u32Status |= E_HAL_MMU_STATUS_R_INVALID;
        }

        if (u16IrqRegVal & REG_MMU_IRQ_WR_FLAG)
        {
            u16IrqRegVal |= REG_MMU_IRQ_WR_CLR;
            u32Status |= E_HAL_MMU_STATUS_W_INVALID;
        }
    }

    // Clear IRQ
    if (u32Status != E_HAL_MMU_STATUS_NORMAL)
    {
        HAL_MIU_Write2Byte(REG_MMU_IRQ_CTRL, u16IrqRegVal);
        HAL_MIU_Write2Byte(REG_MMU_IRQ_CTRL, 0x0);
    }

    if (!u32Status)
        MIU_HAL_ERR("Unknown MIU MMU error: 0x%x\n", u16IrqRegVal);

    return u32Status;
}
