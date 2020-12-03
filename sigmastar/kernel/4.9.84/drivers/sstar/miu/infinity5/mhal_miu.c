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
//-------------------------------------------------------------------------------------------------
//  Macro Define
//-------------------------------------------------------------------------------------------------

#define MIU_CLIENT_GP0                  \
/* 0 */    MIU_CLIENT_NONE,             \
/* 1 */    MIU_CLIENT_DIP0_R,           \
/* 2 */    MIU_CLIENT_DIP0_W,           \
/* 3 */    MIU_CLIENT_LDC_R,            \
/* 4 */    MIU_CLIENT_SC2_FRAME_W,      \
/* 5 */    MIU_CLIENT_SC3_FRAME_W,      \
/* 6 */    MIU_CLIENT_RSC_R,            \
/* 7 */    MIU_CLIENT_SC1_DBG_R,        \
/* 8 */    MIU_CLIENT_CMDQ0_R,          \
/* 9 */    MIU_CLIENT_MOVDMA0_RW,       \
/* A */    MIU_CLIENT_EMAC_RW,          \
/* B */    MIU_CLIENT_2DGE_RW,          \
/* C */    MIU_CLIENT_3DNR0_R,          \
/* D */    MIU_CLIENT_3DNR0_W,          \
/* E */    MIU_CLIENT_GOP4_R,           \
/* F */    MIU_CLIENT_DUMMY_G0CF

#define MIU_CLIENT_GP1                  \
/* 0 */    MIU_CLIENT_ISP_DMAG0_RW,     \
/* 1 */    MIU_CLIENT_ISP_DMAG1_RW,     \
/* 2 */    MIU_CLIENT_ISP_DMAG2_RW,     \
/* 3 */    MIU_CLIENT_GOP2_R,           \
/* 4 */    MIU_CLIENT_GOP3_R,           \
/* 5 */    MIU_CLIENT_ISP_DMAG_RW,      \
/* 6 */    MIU_CLIENT_ISP_AF_STA1_W,    \
/* 7 */    MIU_CLIENT_ISP_MLOAD_RW,     \
/* 8 */    MIU_CLIENT_CMDQ1_R,          \
/* 9 */    MIU_CLIENT_MOVDMA1_RW,       \
/* A */    MIU_CLIENT_MCU51_RW,         \
/* B */    MIU_CLIENT_DLA_RW,           \
/* C */    MIU_CLIENT_IVE_RW,           \
/* D */    MIU_CLIENT_DUMMY_G1CD,       \
/* E */    MIU_CLIENT_DUMMY_G1CE,       \
/* F */    MIU_CLIENT_DUMMY_G1CF

#define MIU_CLIENT_GP2                  \
/* 0 */    MIU_CLIENT_DUMMY_G2C0,       \
/* 1 */    MIU_CLIENT_SC_ROT_R,         \
/* 2 */    MIU_CLIENT_SC_AIP_W,         \
/* 3 */    MIU_CLIENT_SC0_FRAME_W,      \
/* 4 */    MIU_CLIENT_SC0_SNAPSHOT_W,   \
/* 5 */    MIU_CLIENT_SC1_FRAME_W,      \
/* 6 */    MIU_CLIENT_GOP0_R,           \
/* 7 */    MIU_CLIENT_3DNR1_R,          \
/* 8 */    MIU_CLIENT_3DNR1_W,          \
/* 9 */    MIU_CLIENT_CMDQ2_R,          \
/* A */    MIU_CLIENT_BDMA_RW,          \
/* B */    MIU_CLIENT_AESDMA_RW,        \
/* C */    MIU_CLIENT_USB20_RW,         \
/* D */    MIU_CLIENT_USB20_H_RW,       \
/* E */    MIU_CLIENT_MIIC1_RW,         \
/* F */    MIU_CLIENT_URDMA_RW

#define MIU_CLIENT_GP3                  \
/* 0 */    MIU_CLIENT_VEN_R,            \
/* 1 */    MIU_CLIENT_VEN_W,            \
/* 2 */    MIU_CLIENT_JPE_W,            \
/* 3 */    MIU_CLIENT_JPE_R,            \
/* 4 */    MIU_CLIENT_DIP1_R,           \
/* 5 */    MIU_CLIENT_DIP1_W,           \
/* 6 */    MIU_CLIENT_GOP1_R,           \
/* 7 */    MIU_CLIENT_BACH_RW,          \
/* 8 */    MIU_CLIENT_BACH2_RW,         \
/* 9 */    MIU_CLIENT_CMDQ3_R,          \
/* A */    MIU_CLIENT_SDIO_RW,          \
/* B */    MIU_CLIENT_FCIE_RW,          \
/* C */    MIU_CLIENT_MIIC2_RW,         \
/* D */    MIU_CLIENT_MIIC3_RW,         \
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
    MIU_CLIENT_MCU51_RW,
    MIU_CLIENT_USB20_H_RW,
    MIU_CLIENT_USB20_RW,
    MIU_CLIENT_MIIC1_RW,
    MIU_CLIENT_MIIC2_RW,
    MIU_CLIENT_MIIC3_RW,
    MIU_CLIENT_IVE_RW,
    MIU_CLIENT_EMAC_RW,
    MIU_CLIENT_AESDMA_RW,
    MIU_CLIENT_SDIO_RW,
    MIU_CLIENT_FCIE_RW,
    MIU_CLIENT_URDMA_RW,
    MIU_CLIENT_BDMA_RW,
    MIU_CLIENT_MOVDMA0_RW,
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
    static MS_U16 u16HitCnt[MIU_MAX_DEVICE] = {0, };

    if (!pInfo) {
        return FALSE;
    }

    u16Ret      = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_STATUS);
    u16LoAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_LOADDR);
    u16HiAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_HIADDR);

    if (REG_MIU_PROTECT_HIT_FALG & u16Ret)
    {
        pInfo->bHit         = TRUE;
        pInfo->u8Block      = (MS_U8)GET_HIT_BLOCK(u16Ret);
        pInfo->u8Group      = (MS_U8)(GET_HIT_CLIENT(u16Ret) >> 4);
        pInfo->u8ClientID   = (MS_U8)(GET_HIT_CLIENT(u16Ret) & 0x0F);
        pInfo->uAddress     = (MS_U32)((u16HiAddr << 16) | u16LoAddr);
        pInfo->uAddress     = pInfo->uAddress * MIU_PROTECT_ADDRESS_UNIT;

        u32EndAddr = (pInfo->uAddress + MIU_PROTECT_ADDRESS_UNIT - 1);

        HAL_MIU_ClientIdToName((MS_U8)(GET_HIT_CLIENT(u16Ret)), clientName);
        printk(KERN_EMERG "MIU%u Block:%u Client:%s ID:%u-%u Hitted_Addr:0x%x<->0x%x\n",
               u8MiuSel, pInfo->u8Block, clientName,
               pInfo->u8Group, pInfo->u8ClientID,
               pInfo->uAddress, u32EndAddr);

        // Clear log
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, TRUE);
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, FALSE);

        // FIXME: Workaround for the unknown SC3_FRAME_W request even DMA disabled
        if ((u16HitCnt[u8MiuSel] == 0) && (MIU_CLIENT_SC3_FRAME_W == GET_HIT_CLIENT(u16Ret)))
        {
            pInfo->bHit = FALSE;
        }
        // FIXME: Workaround for the unknown VEN_R request addr over DRAM size
        if (MIU_CLIENT_VEN_R == GET_HIT_CLIENT(u16Ret))
        {
            pInfo->bHit = FALSE;
        }
        u16HitCnt[u8MiuSel]++;
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

    // Write_enable
    u8Data = 1 << u8Blockx;

    if (u8MiuSel == E_MIU_0)
    {
        u32RegAddrMSB = MIU_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegProtectId = MIU_PROTECT0_ID0;
        u32RegRWProtectEn = MIU_PROTECT_EN_INTERNAL;
        u32RegBase = MIU_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegStartAddr = MIU_PROTECT0_START;
                u32RegProtectIdEn = MIU_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegStartAddr = MIU_PROTECT1_START;
                u32RegProtectIdEn = MIU_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegStartAddr = MIU_PROTECT2_START;
                u32RegProtectIdEn = MIU_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegStartAddr = MIU_PROTECT3_START;
                u32RegProtectIdEn = MIU_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
                return FALSE;
        }
    }
    else if (u8MiuSel == E_MIU_1)
    {
        u32RegAddrMSB = MIU1_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegProtectId = MIU1_PROTECT0_ID0;
        u32RegRWProtectEn = MIU1_PROTECT_EN;
        u32RegBase = MIU1_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegStartAddr = MIU1_PROTECT0_START;
                u32RegProtectIdEn = MIU1_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegStartAddr = MIU1_PROTECT1_START;
                u32RegProtectIdEn = MIU1_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegStartAddr = MIU1_PROTECT2_START;
                u32RegProtectIdEn = MIU1_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegStartAddr = MIU1_PROTECT3_START;
                u32RegProtectIdEn = MIU1_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
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
        u16Data2 = u16Data2 | (MS_U16)((u32Start >> 29) << (u8Blockx*4));   // u16Data2 for start_ext addr
        u16Data1 = u16Data2 | (MS_U16)(((u32End - 1) >> 29) << (u8Blockx*4+2));
        HAL_MIU_Write2Byte(u32RegAddrMSB, u16Data1);

        // Start Address
        u16Data = (MS_U16)(u32Start >> MIU_PAGE_SHIFT);     // 8k unit
        HAL_MIU_Write2Byte(u32RegStartAddr, u16Data);

        // End Address
        u16Data = (MS_U16)((u32End >> MIU_PAGE_SHIFT)-1);   // 8k unit;
        HAL_MIU_Write2Byte(u32RegStartAddr + 2, u16Data);

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
                u32RegProtectId = MIU_PROTECT0_ID0;

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
                    default:
                        return FALSE;
                }
            }
            else if (u8MiuSel == E_MIU_1)
            {
                u32RegProtectId = MIU1_PROTECT0_ID0;

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
        case MIU_CLIENT_DIP0_R:
            strcpy(clientName, "DIP0_R");
            break;
        case MIU_CLIENT_DIP0_W:
            strcpy(clientName, "DIP0_W");
            break;
        case MIU_CLIENT_LDC_R:
            strcpy(clientName, "LDC_R");
            break;
        case MIU_CLIENT_SC2_FRAME_W:
            strcpy(clientName, "SC2_FRAME_W");
            break;
        case MIU_CLIENT_SC3_FRAME_W:
            strcpy(clientName, "SC3_FRAME_W");
            break;
        case MIU_CLIENT_RSC_R:
            strcpy(clientName, "RSC_R");
            break;
        case MIU_CLIENT_SC1_DBG_R:
            strcpy(clientName, "SC_DBG_R");
            break;
        case MIU_CLIENT_CMDQ0_R:
            strcpy(clientName, "CMDQ0_R");
            break;
        case MIU_CLIENT_MOVDMA0_RW:
            strcpy(clientName, "MDMA0_RW");
            break;
        case MIU_CLIENT_EMAC_RW:
            strcpy(clientName, "EMAC_RW");
            break;
        case MIU_CLIENT_2DGE_RW:
            strcpy(clientName, "2DGE_RW");
            break;
        case MIU_CLIENT_3DNR0_R:
            strcpy(clientName, "3DNR0_R");
            break;
        case MIU_CLIENT_3DNR0_W:
            strcpy(clientName, "3DNR0_W");
            break;
        case MIU_CLIENT_GOP4_R:
            strcpy(clientName, "GOP4_R");
            break;
        case MIU_CLIENT_DUMMY_G0CF:
            strcpy(clientName, "DUMMY_G0CF");
            break;
        // group 1
        case MIU_CLIENT_ISP_DMAG0_RW:
            strcpy(clientName, "ISP_DMAG0_RW");
            break;
        case MIU_CLIENT_ISP_DMAG1_RW:
            strcpy(clientName, "ISP_DMAG1_RW");
            break;
        case MIU_CLIENT_ISP_DMAG2_RW:
            strcpy(clientName, "ISP_DMAG2_RW");
            break;
        case MIU_CLIENT_GOP2_R:
            strcpy(clientName, "GOP2_R");
            break;
        case MIU_CLIENT_GOP3_R:
            strcpy(clientName, "GOP3_R");
            break;
        case MIU_CLIENT_ISP_DMAG_RW:
            strcpy(clientName, "ISP_DMAG_RW");
            break;
        case MIU_CLIENT_ISP_AF_STA1_W:
            strcpy(clientName, "ISP_AF_STA1_W");
            break;
        case MIU_CLIENT_ISP_MLOAD_RW:
            strcpy(clientName, "ISP_MLOAD_RW");
            break;
        case MIU_CLIENT_CMDQ1_R:
            strcpy(clientName, "CMDQ1_R");
            break;
        case MIU_CLIENT_MOVDMA1_RW:
            strcpy(clientName, "MDMA1_RW");
            break;
        case MIU_CLIENT_MCU51_RW:
            strcpy(clientName, "MCU51_RW");
            break;
        case MIU_CLIENT_DLA_RW:
            strcpy(clientName, "DLA_RW");
            break;
        case MIU_CLIENT_IVE_RW:
            strcpy(clientName, "IVE_RW");
            break;
        case MIU_CLIENT_DUMMY_G1CD:
            strcpy(clientName, "DUMMY_G1CD");
            break;
        case MIU_CLIENT_DUMMY_G1CE:
            strcpy(clientName, "DUMMY_G1CE");
            break;
        case MIU_CLIENT_DUMMY_G1CF:
            strcpy(clientName, "DUMMY_G1CF");
            break;
        // group 2
        case MIU_CLIENT_DUMMY_G2C0:
            strcpy(clientName, "DUMMY_G2C0");
            break;
        case MIU_CLIENT_SC_ROT_R:
            strcpy(clientName, "SC_ROT_R");
            break;
        case MIU_CLIENT_SC_AIP_W:
            strcpy(clientName, "SC_AIP_W");
            break;
        case MIU_CLIENT_SC0_FRAME_W:
            strcpy(clientName, "SC0_FRAME_W");
            break;
        case MIU_CLIENT_SC0_SNAPSHOT_W:
            strcpy(clientName, "SC0_SNP_W");
            break;
        case MIU_CLIENT_SC1_FRAME_W:
            strcpy(clientName, "SC1_FRAME_W");
            break;
        case MIU_CLIENT_GOP0_R:
            strcpy(clientName, "GOP0_R");
            break;
        case MIU_CLIENT_3DNR1_R:
            strcpy(clientName, "3DNR1_R");
            break;
        case MIU_CLIENT_3DNR1_W:
            strcpy(clientName, "3DNR1_W");
            break;
        case MIU_CLIENT_CMDQ2_R:
            strcpy(clientName, "CMDQ2_R");
            break;
        case MIU_CLIENT_BDMA_RW:
            strcpy(clientName, "BDMA_RW");
            break;
        case MIU_CLIENT_AESDMA_RW:
            strcpy(clientName, "AESDMA_RW");
            break;
        case MIU_CLIENT_USB20_RW:
            strcpy(clientName, "USB20_RW");
            break;
        case MIU_CLIENT_USB20_H_RW:
            strcpy(clientName, "USB20_H_RW");
            break;
        case MIU_CLIENT_MIIC1_RW:
            strcpy(clientName, "MIIC1_RW");
            break;
        case MIU_CLIENT_URDMA_RW:
            strcpy(clientName, "URDMA_RW");
            break;
        // group 3
        case MIU_CLIENT_VEN_R:
            strcpy(clientName, "VEN_R");
            break;
        case MIU_CLIENT_VEN_W:
            strcpy(clientName, "VEN_W");
            break;
        case MIU_CLIENT_JPE_W:
            strcpy(clientName, "JPE_W");
            break;
        case MIU_CLIENT_JPE_R:
            strcpy(clientName, "JPE_R");
            break;
        case MIU_CLIENT_DIP1_R:
            strcpy(clientName, "DIP1_R");
            break;
        case MIU_CLIENT_DIP1_W:
            strcpy(clientName, "DIP1_W");
            break;
        case MIU_CLIENT_GOP1_R:
            strcpy(clientName, "GOP1_R");
            break;
        case MIU_CLIENT_BACH_RW:
            strcpy(clientName, "BACH_RW");
            break;
        case MIU_CLIENT_BACH2_RW:
            strcpy(clientName, "BACH2_RW");
            break;
        case MIU_CLIENT_CMDQ3_R:
            strcpy(clientName, "CMDQ3_R");
            break;
        case MIU_CLIENT_SDIO_RW:
            strcpy(clientName, "SDIO_RW");
            break;
        case MIU_CLIENT_FCIE_RW:
            strcpy(clientName, "FCIE_RW");
            break;
        case MIU_CLIENT_MIIC2_RW:
            strcpy(clientName, "MIIC2_RW");
            break;
        case MIU_CLIENT_MIIC3_RW:
            strcpy(clientName, "MIIC3_RW");
            break;
        case MIU_CLIENT_DUMMY_G3CE:
            strcpy(clientName, "DUMMY_G3CE");
            break;
        case MIU_CLIENT_DUMMY_G3CF:
            strcpy(clientName, "DUMMY_G3CF");
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

int clientId_KernelProtectToName(MS_U16 clientId, char *clientName)
{
    return HAL_MIU_ClientIdToName(clientId, clientName);
}
EXPORT_SYMBOL(clientId_KernelProtectToName);
