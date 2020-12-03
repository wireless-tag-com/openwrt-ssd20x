/*
* mdrv_system_st.h- Sigmastar
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
#ifndef __DRV_SYSTEM_ST_H__
#define __DRV_SYSTEM_ST_H__

#ifdef CONFIG_COMPAT
#include <asm/compat.h>
#endif
//------------------------------------------------------------------------------
// Data structure
//------------------------------------------------------------------------------
typedef struct IO_SYS_PANEL_INFO_s
{
    U32* pPanelInfo;
    U16  u16Len;
} IO_SYS_PANEL_INFO_t, *PIO_SYS_PANEL_INFO_t;

typedef struct IO_SYS_PANEL_Res_s
{
    U16  u16Width;
    U16  u16Height;
} IO_SYS_PANEL_GET_RES_t, *PIO_SYS_PANEL_GET_RES_t;

typedef struct IO_SYS_BOARD_INFO_s
{
    U32* pu32BoardInfo;
    U16  u16Len;
} IO_SYS_BOARD_INFO_t, *PIO_SYS_BOARD_INFO_t;

typedef struct IO_SYS_GENERAL_REG_s
{
    U16  u16Reg;
    U8   u8Value;
} IO_SYS_GENERAL_REG_t;

typedef struct IO_SYS_AEONBIN_INFO_s
{
	U8* pu8AeonStart;
	U32 u32Len;
	BOOL bRet;
} IO_SYS_AEONBIN_INFO_t;

/* PCMCIA_MAP_IOC_INFO */
typedef struct
{
    U16                  u16Addr;
    U8                   u8Value;
    U8                   u8Type;     // 1: AttribMem, 2: IOMem
    U16                  u16DataLen;
    U8 *                 u8pReadBuffer;
    U8 *                 u8pWriteBuffer;
} PCMCIA_Map_Info_t;
#ifdef CONFIG_COMPAT
typedef struct
{
    U16                  u16Addr;
    U8                   u8Value;
    U8                   u8Type;     // 1: AttribMem, 2: IOMem
    U16                  u16DataLen;
    compat_uptr_t        u8pReadBuffer;
    compat_uptr_t        u8pWriteBuffer;            
} COMPAT_PCMCIA_Map_Info_t;
#endif

typedef enum
{
    RELOAD_AEON_STOP,
    RELOAD_AEON_RESTART
} AEON_CONTROL ;

typedef enum
{
    // Analog port
    INPUT_SRC_VGA,
    INPUT_SRC_YPBPR_1,
    INPUT_SRC_YPBPR_2,

    // Digital port
    INPUT_SRC_ATV,
    INPUT_SRC_CVBS_1,
    INPUT_SRC_CVBS_2,
    INPUT_SRC_CVBS_3,

    INPUT_SRC_SVIDEO_1,
    INPUT_SRC_SVIDEO_2,

    INPUT_SRC_SCART_1,
    INPUT_SRC_SCART_2,

    // HDMI port
    INPUT_SRC_HDMI_A,
#if 1 
    INPUT_SRC_HDMI_B,
#else
    INPUT_SRC_HDMI_B1,
    INPUT_SRC_HDMI_B2,
    INPUT_SRC_HDMI_B3,
#endif
    INPUT_SRC_HDMI_C,

    // MVD port
    INPUT_SRC_DTV,
    INPUT_SRC_DTV_MLINK,

    INPUT_SRC_STORAGE,              ///< input source is Storage

    INPUT_SRC_NUM,
    INPUT_SRC_NONE = INPUT_SRC_NUM
} SYS_INPUT_SOURCE_e;


typedef struct IO_SYS_SPI_s
{
   U32 u32Start;
   U32 u32Len;
   U8 *u8data;		
} IO_SYS_SPI_t;

typedef struct IO_SYS_SPI_ERASE_s
{
   U32  u32StartAddr;
   U32  u32Size;
   BOOL bWait;
} IO_SYS_SPI_ERASE_t;

typedef struct IO_SYS_SPI_ERASE_SECTOR_s
{
  U32 u32StartAddr;
  U32 u32EndAddr;
} IO_SYS_SPI_ERASE_SECTOR_t;
#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
#ifdef CONFIG_MP_NEW_UTOPIA_32BIT
typedef struct
{
        u64                LX_MEM_ADDR;
        u64                LX_MEM_LENGTH;
        u64                LX_MEM2_ADDR;
        u64                LX_MEM2_LENGTH;
        u64                EMAC_ADDR;
        u64                EMAC_LENGTH;
        u64                DRAM_ADDR;
        u64                DRAM_LENGTH;
        u64                BB_ADDR;
        u64                BB_LENGTH;
        u64                MPOOL_MEM_ADDR;
        u64                MPOOL_MEM_LENGTH;
        u64                G3D_MEM0_ADDR;
        u64                G3D_MEM0_LENGTH;
        u64                G3D_MEM1_ADDR;
        u64                G3D_MEM1_LENGTH;
        u64                G3D_CMDQ_ADDR;
        u64                G3D_CMDQ_LENGTH;
} IO_Sys_Info_t;

typedef struct
{
        u64                LX_MEM_ADDR;
        u64                LX_MEM_LENGTH;
        u64                LX_MEM2_ADDR;
        u64                LX_MEM2_LENGTH;
        u64                LX_MEM3_ADDR;
        u64                LX_MEM3_LENGTH;
        u64                LX_MEM4_ADDR;
        u64                LX_MEM4_LENGTH;
        u64                LX_MEM5_ADDR;
        u64                LX_MEM5_LENGTH;
        u64                EMAC_ADDR;
        u64                EMAC_LENGTH;
        u64                DRAM_ADDR;
        u64                DRAM_LENGTH;
        u64                BB_ADDR;
        u64                BB_LENGTH;
        u64                MPOOL_MEM_ADDR;
        u64                MPOOL_MEM_LENGTH;
        u64                G3D_MEM0_ADDR;
        u64                G3D_MEM0_LENGTH;
        u64                G3D_MEM1_ADDR;
        u64                G3D_MEM1_LENGTH;
        u64                G3D_CMDQ_ADDR;
        u64                G3D_CMDQ_LENGTH;
}IO_Sys_Info_t_EX;
#else
typedef struct
{
        unsigned int                LX_MEM_ADDR;
        unsigned int                LX_MEM_LENGTH;
        unsigned int                LX_MEM2_ADDR;
        unsigned int                LX_MEM2_LENGTH;
        unsigned int                EMAC_ADDR;
        unsigned int                EMAC_LENGTH;
        unsigned int                DRAM_ADDR;
        unsigned int                DRAM_LENGTH;
        unsigned int                BB_ADDR;
        unsigned int                BB_LENGTH;
        unsigned int                MPOOL_MEM_ADDR;
        unsigned int                MPOOL_MEM_LENGTH;
        unsigned int                G3D_MEM0_ADDR;
        unsigned int                G3D_MEM0_LENGTH;
        unsigned int                G3D_MEM1_ADDR;
        unsigned int                G3D_MEM1_LENGTH;
        unsigned int                G3D_CMDQ_ADDR;
        unsigned int                G3D_CMDQ_LENGTH;
} IO_Sys_Info_t;

typedef struct
{
        unsigned int                LX_MEM_ADDR;
        unsigned int                LX_MEM_LENGTH;
        unsigned int                LX_MEM2_ADDR;
        unsigned int                LX_MEM2_LENGTH;
        unsigned int                LX_MEM3_ADDR;
        unsigned int                LX_MEM3_LENGTH;
        unsigned int                LX_MEM4_ADDR;
        unsigned int                LX_MEM4_LENGTH;
        unsigned int                LX_MEM5_ADDR;
        unsigned int                LX_MEM5_LENGTH;
        unsigned int                EMAC_ADDR;
        unsigned int                EMAC_LENGTH;
        unsigned int                DRAM_ADDR;
        unsigned int                DRAM_LENGTH;
        unsigned int                BB_ADDR;
        unsigned int                BB_LENGTH;
        unsigned int                MPOOL_MEM_ADDR;
        unsigned int                MPOOL_MEM_LENGTH;
        unsigned int                G3D_MEM0_ADDR;
        unsigned int                G3D_MEM0_LENGTH;
        unsigned int                G3D_MEM1_ADDR;
        unsigned int                G3D_MEM1_LENGTH;
        unsigned int                G3D_CMDQ_ADDR;
        unsigned int                G3D_CMDQ_LENGTH;
}IO_Sys_Info_t_EX;
#endif //CONFIG_MP_NEW_UTOPIA_32BIT
#elif defined(CONFIG_ARM64)
typedef struct
{
        u64                LX_MEM_ADDR;
        u64                LX_MEM_LENGTH;
        u64                LX_MEM2_ADDR;
        u64                LX_MEM2_LENGTH;
        u64                EMAC_ADDR;
        u64                EMAC_LENGTH;
        u64                DRAM_ADDR;
        u64                DRAM_LENGTH;
        u64                BB_ADDR;
        u64                BB_LENGTH;
        u64                MPOOL_MEM_ADDR;
        u64                MPOOL_MEM_LENGTH;
        u64                G3D_MEM0_ADDR;
        u64                G3D_MEM0_LENGTH;
        u64                G3D_MEM1_ADDR;
        u64                G3D_MEM1_LENGTH;
        u64                G3D_CMDQ_ADDR;
        u64                G3D_CMDQ_LENGTH;
} IO_Sys_Info_t;

typedef struct
{
        u64                LX_MEM_ADDR;
        u64                LX_MEM_LENGTH;
        u64                LX_MEM2_ADDR;
        u64                LX_MEM2_LENGTH;
        u64                LX_MEM3_ADDR;
        u64                LX_MEM3_LENGTH;
        u64                LX_MEM4_ADDR;
        u64                LX_MEM4_LENGTH;
        u64                LX_MEM5_ADDR;
        u64                LX_MEM5_LENGTH;
        u64                EMAC_ADDR;
        u64                EMAC_LENGTH;
        u64                DRAM_ADDR;
        u64                DRAM_LENGTH;
        u64                BB_ADDR;
        u64                BB_LENGTH;
        u64                MPOOL_MEM_ADDR;
        u64                MPOOL_MEM_LENGTH;
        u64                G3D_MEM0_ADDR;
        u64                G3D_MEM0_LENGTH;
        u64                G3D_MEM1_ADDR;
        u64                G3D_MEM1_LENGTH;
        u64                G3D_CMDQ_ADDR;
        u64                G3D_CMDQ_LENGTH;
}IO_Sys_Info_t_EX;

#endif

#endif // __DRV_SYSTEM_ST_H__

