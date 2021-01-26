/*
* drvDeviceInfo.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "MsTypes.h"
#include "drvDeviceInfo.h"
#include "regSERFLASH.h"
#include "drvSERFLASH.h"
#include "halSERFLASH.h"




//
//  Sprcial Block Table (List)
//
static ST_SPECIAL_BLOCKS _stSpecialBlocks_EN25F10 =
{
    .u16Start     = 0,  // Start block# of special block size
    .u16End       = 3,  // End   block# of special block size
    .au32SizeList =     // List of special size;Total size must be equal to block size
    {
        SIZE_32KB,
        SIZE_32KB,
        SIZE_32KB,
        SIZE_32KB,
    }
};

static ST_SPECIAL_BLOCKS _stSpecialBlocks_EN25B32B =
{
    .u16Start     = 0,  // Start block# of special block size
    .u16End       = 4,  // End   block# of special block size
    .au32SizeList =     // List of special size;Total size must be equal to block size
    {
        SIZE_4KB,
        SIZE_4KB,
        SIZE_8KB,
        SIZE_16KB,
        SIZE_32KB,
    }
};

static ST_SPECIAL_BLOCKS _stSpecialBlocks_EN25B64B =
{
    .u16Start     = 0,  // Start block# of special block size
    .u16End       = 4,  // End   block# of special block size
    .au32SizeList =     // List of special size;Total size must be equal to block size
    {
        SIZE_4KB,
        SIZE_4KB,
        SIZE_8KB,
        SIZE_16KB,
        SIZE_32KB,
    }
};
//-------------------------------------------------------------------------------------------------
//  Write Protect Table (List)
//-------------------------------------------------------------------------------------------------

static ST_WRITE_PROTECT _pstWriteProtectTable_W25X32[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x003F0000,     0x003FFFFF  },
    {   BITS(5:2, 2),   0x003E0000,     0x003FFFFF  },
    {   BITS(5:2, 3),   0x003C0000,     0x003FFFFF  },
    {   BITS(5:2, 4),   0x00380000,     0x003FFFFF  },
    {   BITS(5:2, 5),   0x00300000,     0x003FFFFF  },
    {   BITS(5:2, 6),   0x00200000,     0x003FFFFF  },
    {   BITS(5:2, 9),   0x00000000,     0x0000FFFF  },
    {   BITS(5:2, 10),  0x00000000,     0x0001FFFF  },
    {   BITS(5:2, 11),  0x00000000,     0x0003FFFF  },
    {   BITS(5:2, 12),  0x00000000,     0x0007FFFF  },
    {   BITS(5:2, 13),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 14),  0x00000000,     0x001FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x003FFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_W25X64[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x007E0000,     0x007FFFFF  },
    {   BITS(5:2, 2),   0x007C0000,     0x007FFFFF  },
    {   BITS(5:2, 3),   0x00780000,     0x007FFFFF  },
    {   BITS(5:2, 4),   0x00700000,     0x007FFFFF  },
    {   BITS(5:2, 5),   0x00600000,     0x007FFFFF  },
    {   BITS(5:2, 6),   0x00400000,     0x007FFFFF  },
    {   BITS(5:2, 9),   0x00000000,     0x0001FFFF  },
    {   BITS(5:2, 10),  0x00000000,     0x0003FFFF  },
    {   BITS(5:2, 11),  0x00000000,     0x0007FFFF  },
    {   BITS(5:2, 12),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 13),  0x00000000,     0x001FFFFF  },
    {   BITS(5:2, 14),  0x00000000,     0x003FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x007FFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};


static ST_WRITE_PROTECT _pstWriteProtectTable_S25FL032K_CMP0[]=
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x01),   0x003F0000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x02),   0x003E0000,     0x003FFFFF  }, // 62-63
    {   BITS(6:2, 0x03),   0x003C0000,     0x003FFFFF  }, // 60-63
    {   BITS(6:2, 0x04),   0x00380000,     0x003FFFFF  }, // 56-63
    {   BITS(6:2, 0x05),   0x00300000,     0x003FFFFF  }, // 48-63
    {   BITS(6:2, 0x06),   0x00200000,     0x003FFFFF  }, // 32-63
    {   BITS(6:2, 0x09),   0x00000000,     0x0000FFFF  }, // 0
    {   BITS(6:2, 0x0A),   0x00000000,     0x0001FFFF  }, // 00-01
    {   BITS(6:2, 0x0B),   0x00000000,     0x0003FFFF  }, // 00-03
    {   BITS(6:2, 0x0C),   0x00000000,     0x0007FFFF  }, // 00-07
    {   BITS(6:2, 0x0D),   0x00000000,     0x000FFFFF  }, // 00-15
    {   BITS(6:2, 0x0E),   0x00000000,     0x001FFFFF  }, // 00-31
    {   BITS(6:2, 0x1F),   0x00000000,     0x003FFFFF  }, // 0-63
    {   BITS(6:2, 0x11),   0x003FF000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x12),   0x003FE000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x13),   0x003FC000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x14),   0x003F8000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x19),   0x00000000,     0x00000FFF  }, // 00
    {   BITS(6:2, 0x1A),   0x00000000,     0x00001FFF  }, // 00
    {   BITS(6:2, 0x1B),   0x00000000,     0x00003FFF  }, // 00
    {   BITS(6:2, 0x1C),   0x00000000,     0x00007FFF  }, // 00
}; 


ST_WRITE_PROTECT _pstWriteProtectTable_S25FL032K_CMP1[]=
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x01),   0x00000000,     0x003EFFFF  }, // 00-62
    {   BITS(6:2, 0x02),   0x00000000,     0x003DFFFF  }, // 00-61
    {   BITS(6:2, 0x03),   0x00000000,     0x003BFFFF  }, // 00-59
    {   BITS(6:2, 0x04),   0x00000000,     0x0037FFFF  }, // 00-55
    {   BITS(6:2, 0x05),   0x00000000,     0x002FFFFF  }, // 00-47
    {   BITS(6:2, 0x06),   0x00000000,     0x001FFFFF  }, // 00-31
    {   BITS(6:2, 0x09),   0x00010000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x0A),   0x00020000,     0x003FFFFF  }, // 02-63
    {   BITS(6:2, 0x0B),   0x00040000,     0x003FFFFF  }, // 04-63
    {   BITS(6:2, 0x0C),   0x00080000,     0x003FFFFF  }, // 08-63
    {   BITS(6:2, 0x0D),   0x00100000,     0x003FFFFF  }, // 16-63
    {   BITS(6:2, 0x0E),   0x00200000,     0x003FFFFF  }, // 32-63
    {   BITS(6:2, 0x1F),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x11),   0x00000000,     0x003FEFFF  }, // 00-63
    {   BITS(6:2, 0x12),   0x00000000,     0x003FDFFF  }, // 00-63
    {   BITS(6:2, 0x13),   0x00000000,     0x003FBFFF  }, // 00-62
    {   BITS(6:2, 0x14),   0x00000000,     0x003F7FFF  }, // 00-62
    {   BITS(6:2, 0x19),   0x00001000,     0x003FFFFF  }, // 00-63
    {   BITS(6:2, 0x1A),   0x00002000,     0x003FFFFF  }, // 00-63
    {   BITS(6:2, 0x1B),   0x00004000,     0x00003FFF  }, // 00-63
    {   BITS(6:2, 0x1C),   0x00008000,     0x003FFFFF  }, // 00-63
};

static ST_WRITE_PROTECT _pstWriteProtectTable_GD25Q32_CMP0[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x01),   0x003F0000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x02),   0x003E0000,     0x003FFFFF  }, // 62-63
    {   BITS(6:2, 0x03),   0x003C0000,     0x003FFFFF  }, // 60-63
    {   BITS(6:2, 0x04),   0x00380000,     0x003FFFFF  }, // 56-63
    {   BITS(6:2, 0x05),   0x00300000,     0x003FFFFF  }, // 48-63
    {   BITS(6:2, 0x06),   0x00200000,     0x003FFFFF  }, // 32-63
    
    {   BITS(6:2, 0x09),   0x00000000,     0x0000FFFF  }, // 00
    {   BITS(6:2, 0x0A),   0x00000000,     0x0001FFFF  }, // 00-01
    {   BITS(6:2, 0x0B),   0x00000000,     0x0003FFFF  }, // 00-03
    {   BITS(6:2, 0x0C),   0x00000000,     0x0007FFFF  }, // 00-07
    {   BITS(6:2, 0x0D),   0x00000000,     0x000FFFFF  }, // 00-15
    {   BITS(6:2, 0x0E),   0x00000000,     0x001FFFFF  }, // 00-31
    
    {   BITS(6:2, 0x11),   0x003FF000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x12),   0x003EE000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x13),   0x003FC000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x14),   0x003F8000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x15),   0x003F8000,     0x003FFFFF  }, // 63
    {   BITS(6:2, 0x16),   0x003F8000,     0x003FFFFF  }, // 63
   
    {   BITS(6:2, 0x19),   0x00000000,     0x00000FFF  }, // 00
    {   BITS(6:2, 0x1A),   0x00000000,     0x00001FFF  }, // 00
    {   BITS(6:2, 0x1B),   0x00000000,     0x00003FFF  }, // 00
    {   BITS(6:2, 0x1C),   0x00000000,     0x00007FFF  }, // 00
    {   BITS(6:2, 0x1D),   0x00000000,     0x00007FFF  }, // 00
    {   BITS(6:2, 0x1E),   0x00000000,     0x00007FFF  }, // 00
    
    {   BITS(6:2, 0x07),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x0F),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x17),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x1F),   0x00000000,     0x003FFFFF  }, // ALL

    {   BITS(6:2, 0x18),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x10),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x08),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
};

ST_WRITE_PROTECT _pstWriteProtectTable_GD25Q32_CMP1[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x01),   0x00000000,     0x003EFFFF  }, // 00-62
    {   BITS(6:2, 0x02),   0x00000000,     0x003DFFFF  }, // 00-61
    {   BITS(6:2, 0x03),   0x00000000,     0x003BFFFF  }, // 00-59
    {   BITS(6:2, 0x04),   0x00000000,     0x0037FFFF  }, // 00-55
    {   BITS(6:2, 0x05),   0x00000000,     0x002FFFFF  }, // 00-47
    {   BITS(6:2, 0x06),   0x00000000,     0x001FFFFF  }, // 00-31

    {   BITS(6:2, 0x09),   0x00010000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x0A),   0x00020000,     0x003FFFFF  }, // 02-63
    {   BITS(6:2, 0x0B),   0x00040000,     0x003FFFFF  }, // 04-63
    {   BITS(6:2, 0x0C),   0x00080000,     0x003FFFFF  }, // 08-63
    {   BITS(6:2, 0x0D),   0x00100000,     0x003FFFFF  }, // 16-63
    {   BITS(6:2, 0x0E),   0x00200000,     0x003FFFFF  }, // 32-63

    {   BITS(6:2, 0x11),   0x00000000,     0x003FEFFF  }, // 00-62
    {   BITS(6:2, 0x12),   0x00000000,     0x003FDFFF  }, // 00-62
    {   BITS(6:2, 0x13),   0x00000000,     0x003FBFFF  }, // 00-62
    {   BITS(6:2, 0x14),   0x00000000,     0x003F7FFF  }, // 00-62
    {   BITS(6:2, 0x15),   0x00000000,     0x003F7FFF  }, // 00-62
    {   BITS(6:2, 0x16),   0x00000000,     0x003F7FFF  }, // 00-62

    {   BITS(6:2, 0x19),   0x00001000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x1A),   0x00002000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x1B),   0x00004000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x1C),   0x00008000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x1D),   0x00008000,     0x003FFFFF  }, // 01-63
    {   BITS(6:2, 0x1E),   0x00008000,     0x003FFFFF  }, // 01-63
    
    {   BITS(6:2, 0x07),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x0F),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x17),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x1F),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    
    {   BITS(6:2, 0x18),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x10),   0x00000000,     0x003FFFFF  }, // ALL
    {   BITS(6:2, 0x08),   0x00000000,     0x003FFFFF  }, // ALL
};

static ST_WRITE_PROTECT _pstWriteProtectTable_GD25Q16_CMP0[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0xFFFFFFFF,     0xFFFFFFFF  }, // NONE
    {   BITS(6:2, 0x01),   0x001F0000,     0x001FFFFF  }, // Upper 1/32
    {   BITS(6:2, 0x02),   0x001E0000,     0x001FFFFF  }, // Upper 1/16
    {   BITS(6:2, 0x03),   0x001C0000,     0x001FFFFF  }, // Upper 1/8
    {   BITS(6:2, 0x04),   0x00180000,     0x001FFFFF  }, // Upper 1/4
    {   BITS(6:2, 0x05),   0x00100000,     0x001FFFFF  }, // Upper 1/2
    
    {   BITS(6:2, 0x09),   0x00000000,     0x0000FFFF  }, // Lower 1/32
    {   BITS(6:2, 0x0A),   0x00000000,     0x0001FFFF  }, // Lower 1/16
    {   BITS(6:2, 0x0B),   0x00000000,     0x0003FFFF  }, // Lower 1/8
    {   BITS(6:2, 0x0C),   0x00000000,     0x0007FFFF  }, // Lower 1/4
    {   BITS(6:2, 0x0D),   0x00000000,     0x000FFFFF  }, // Lower 1/2
    {   BITS(6:2, 0x0E),   0x00000000,     0x001FFFFF  }, // ALL
    
    {   BITS(6:2, 0x11),   0x001FF000,     0x001FFFFF  }, //  4KB Top Block
    {   BITS(6:2, 0x12),   0x001EE000,     0x001FFFFF  }, // 8KB Top Block
    {   BITS(6:2, 0x13),   0x001FC000,     0x001FFFFF  }, // 16KB Top Block
    {   BITS(6:2, 0x14),   0x001F8000,     0x001FFFFF  }, // 32KB Top Block

   
    {   BITS(6:2, 0x19),   0x00000000,     0x00000FFF  }, // 4KB Bottom Block
    {   BITS(6:2, 0x1A),   0x00000000,     0x00001FFF  }, // 8KB Bottom Block
    {   BITS(6:2, 0x1B),   0x00000000,     0x00003FFF  }, // 16KB Bottom Block
    {   BITS(6:2, 0x1C),   0x00000000,     0x00007FFF  }, // 32KB Bottom Block

};
static ST_WRITE_PROTECT _pstWriteProtectTable_GD25Q128_CMP0[] =
{
    //   BPX,             Lower Bound     Upper Bound
    {   BITS(6:2, 0x00),   0xFFFFFFFF,     0xFFFFFFFF  },
    {   BITS(6:2, 0x01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0x02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0x03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0x04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0x05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0x06),   0x00800000,     0x0000FFFF  },

    {   BITS(6:2, 0x09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0x0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0x0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0x0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0x0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0x0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0x0F),   0x00000000,     0x00FFFFFF  },

    {   BITS(6:2, 0x11),   0x00FFF000,     0x00FFFFFF  },
    {   BITS(6:2, 0x12),   0x00FEE000,     0x00FFFFFF  },
    {   BITS(6:2, 0x13),   0x00FFC000,     0x00FFFFFF  },
    {   BITS(6:2, 0x14),   0x00FF8000,     0x00FFFFFF  },
    {   BITS(6:2, 0x16),   0x00FF8000,     0x00FFFFFF  },

    {   BITS(6:2, 0x19),   0x00000000,     0x00000FFF  },
    {   BITS(6:2, 0x1A),   0x00000000,     0x00001FFF  },
    {   BITS(6:2, 0x1B),   0x00000000,     0x00003FFF  },
    {   BITS(6:2, 0x1C),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0x1E),   0x00000000,     0x00007FFF  },
};

#if  0//researved for GD25Q16 flash
static ST_WRITE_PROTECT _pstWriteProtectTable_GD25Q16_CMP1[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x001FFFFF  }, //  2M ALL
    {   BITS(6:2, 0x01),   0x00000000,     0x001EFFFF  }, // 1984KB  Lower 31/32
    {   BITS(6:2, 0x02),   0x00000000,     0x001DFFFF  }, // 1920KB Lower 15/16
    {   BITS(6:2, 0x03),   0x00000000,     0x001BFFFF  }, // 1792KB Lower 7/8
    {   BITS(6:2, 0x04),   0x00000000,     0x0017FFFF  }, // 1536KB Lower 3/4
    {   BITS(6:2, 0x05),   0x00000000,     0x001FFFFF  }, // 1M  Lower 1/2
 

    {   BITS(6:2, 0x09),   0x00000000,     0x0000FFFF  }, // 1984KB Upper 31/32
    {   BITS(6:2, 0x0A),   0x00000000,     0x0001FFFF  }, // 1920KB Upper 15/16
    {   BITS(6:2, 0x0B),   0x00000000,     0x0003FFFF  }, // 1792KB Upper 7/8
    {   BITS(6:2, 0x0C),   0x00000000,     0x0007FFFF  }, // 1536KB Upper 3/4
    {   BITS(6:2, 0x0D),   0x00000000,     0x000FFFFF  }, // 1M Upper 1/2


    {   BITS(6:2, 0x11),   0x00000000,     0x001FEFFF  }, // 2044KB  L - 511/512
    {   BITS(6:2, 0x12),   0x00000000,     0x001FDFFF  }, // 2040KB  L - 255/256
    {   BITS(6:2, 0x13),   0x00000000,     0x001FBFFF  }, // 2032KB  L - 127/128
    {   BITS(6:2, 0x14),   0x00000000,     0x001F7FFF  }, // 2016KB  L - 63/64

    {   BITS(6:2, 0x19),   0x00001000,     0x001FFFFF  }, // 2044KB  U - 511/512
    {   BITS(6:2, 0x1A),   0x00002000,     0x001FFFFF  }, // 2040KB  U - 255/256
    {   BITS(6:2, 0x1B),   0x00004000,     0x001FFFFF  }, // 2032KB  U - 127/128
    {   BITS(6:2, 0x1C),   0x00008000,     0x001FFFFF  }, // 2016KB  U - 63/64

       
};
#endif 

ST_WRITE_PROTECT _pstWriteProtectTable_W25Q128CV_CMP1[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x01),   0x00000000,     0x00FDFFFF  }, 
    {   BITS(6:2, 0x02),   0x00000000,     0x00FBFFFF  }, 
    {   BITS(6:2, 0x03),   0x00000000,     0x00F7FFFF  }, 
    {   BITS(6:2, 0x04),   0x00000000,     0x00DFFFFF  }, 
    {   BITS(6:2, 0x05),   0x00000000,     0x00CFFFFF  }, 
    {   BITS(6:2, 0x06),   0x00000000,     0x007FFFFF  }, 
    
    {   BITS(6:2, 0x09),   0x00FC0000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x0A),   0x00F80000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x0B),   0x00F00000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x0C),   0x00E00000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x0D),   0x00C00000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x0E),   0x00800000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x1F),   0xFFFFFFFF,     0xFFFFFFFF  }, 
    
    {   BITS(6:2, 0x11),   0x00000000,     0x00FFEFFF  }, 
    {   BITS(6:2, 0x12),   0x00000000,     0x00FFDFFF  }, 
    {   BITS(6:2, 0x13),   0x00000000,     0x00FFBFFF  }, 
    {   BITS(6:2, 0x14),   0x00000000,     0x00FF7FFF  }, 
    
    {   BITS(6:2, 0x19),   0x00001000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x1A),   0x00002000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x1B),   0x00004000,     0x00FFFFFF  }, 
    {   BITS(6:2, 0x1C),   0x00008000,     0x00FFFFFF  }, 
};


ST_WRITE_PROTECT _pstWriteProtectTable_W25Q64CV_CMP1[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x01),   0x00000000,     0x007DFFFF  }, 
    {   BITS(6:2, 0x02),   0x00000000,     0x007BFFFF  }, 
    {   BITS(6:2, 0x03),   0x00000000,     0x0077FFFF  }, 
    {   BITS(6:2, 0x04),   0x00000000,     0x006FFFFF  }, 
    {   BITS(6:2, 0x05),   0x00000000,     0x005FFFFF  }, 
    {   BITS(6:2, 0x06),   0x00000000,     0x003FFFFF  }, 
    
    {   BITS(6:2, 0x09),   0x00020000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x0A),   0x00040000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x0B),   0x00080000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x0C),   0x00100000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x0D),   0x00200000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x0E),   0x00400000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x1F),   0xFFFFFFFF,     0xFFFFFFFF  }, 
    
    {   BITS(6:2, 0x11),   0x00000000,     0x007FEFFF  }, 
    {   BITS(6:2, 0x12),   0x00000000,     0x007FDFFF  }, 
    {   BITS(6:2, 0x13),   0x00000000,     0x007FBFFF  }, 
    {   BITS(6:2, 0x14),   0x00000000,     0x007F7FFF  }, 
    
    {   BITS(6:2, 0x19),   0x00001000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x1A),   0x00002000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x1B),   0x00004000,     0x007FFFFF  }, 
    {   BITS(6:2, 0x1C),   0x00008000,     0x007FFFFF  }, 
};


ST_WRITE_PROTECT _pstWriteProtectTable_W25Q32BV_CMP1[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 0x00),   0x00000000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x01),   0x00000000,     0x003EFFFF  }, 
    {   BITS(6:2, 0x02),   0x00000000,     0x003DFFFF  }, 
    {   BITS(6:2, 0x03),   0x00000000,     0x003BFFFF  }, 
    {   BITS(6:2, 0x04),   0x00000000,     0x0037FFFF  }, 
    {   BITS(6:2, 0x05),   0x00000000,     0x002FFFFF  }, 
    {   BITS(6:2, 0x06),   0x00000000,     0x001FFFFF  }, 
    
    {   BITS(6:2, 0x09),   0x00010000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x0A),   0x00020000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x0B),   0x00040000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x0C),   0x00080000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x0D),   0x00100000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x0E),   0x00200000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x1F),   0xFFFFFFFF,     0xFFFFFFFF  }, 
    
    {   BITS(6:2, 0x11),   0x00000000,     0x003FEFFF  }, 
    {   BITS(6:2, 0x12),   0x00000000,     0x003FDFFF  }, 
    {   BITS(6:2, 0x13),   0x00000000,     0x003FBFFF  }, 
    {   BITS(6:2, 0x14),   0x00000000,     0x003F7FFF  }, 
    
    {   BITS(6:2, 0x19),   0x00001000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x1A),   0x00002000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x1B),   0x00004000,     0x003FFFFF  }, 
    {   BITS(6:2, 0x1C),   0x00008000,     0x003FFFFF  }, 
};

static ST_WRITE_PROTECT _pstWriteProtectTable_FS25Q128F2TFI[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0X02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0X03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X06),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X08),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x00FFFFFF  },
	
	{   BITS(6:2, 0X10),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x00FBFFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x00F7FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x00EFFFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x00DFFFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x00BFFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X17),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X18),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X19),   0x00040000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1A),   0x00080000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1B),   0x00100000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1C),   0x00200000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1D),   0x00400000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1E),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1F),   0x00FFFFFF,     0x00FFFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_ZB25Q128[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(5:2, 2),   0x00F80000,     0x00FFFFFF  },
    {   BITS(5:2, 3),   0x00F00000,     0x00FFFFFF  },
    {   BITS(5:2, 4),   0x00E00000,     0x00FFFFFF  },
    {   BITS(5:2, 5),   0x00C00000,     0x00FFFFFF  },
    {   BITS(5:2, 6),   0x00800000,     0x00FFFFFF  },
    {   BITS(5:2, 7),   0x00000000,     0x00FFFFFF  },
    {   BITS(5:2, 8),   0xFFFFFFFF,     0xFFFFFFFF  },
    {   BITS(5:2, 9),   0x00000000,     0x0003FFFF  },
    {   BITS(5:2, 10),  0x00000000,     0x0007FFFF  },
    {   BITS(5:2, 11),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 12),  0x00000000,     0x001FFFFF  },
    {   BITS(5:2, 13),  0x00000000,     0x003FFFFF  },
    {   BITS(5:2, 14),  0x00000000,     0x007FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x00FFFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_ZB25Q64[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x007F0000,     0x007FFFFF  },
    {   BITS(5:2, 2),   0x007E0000,     0x007FFFFF  },
    {   BITS(5:2, 3),   0x007C0000,     0x007FFFFF  },
    {   BITS(5:2, 4),   0x00780000,     0x007FFFFF  },
    {   BITS(5:2, 5),   0x00700000,     0x007FFFFF  },
    {   BITS(5:2, 6),   0x00600000,     0x007FFFFF  },
    {   BITS(5:2, 7),   0x00400000,     0x007FFFFF  },
    {   BITS(5:2, 8),   0x00200000,     0x007FFFFF  },
    {   BITS(5:2, 9),   0x00100000,     0x007FFFFF  },
    {   BITS(5:2, 10),  0x00080000,     0x007FFFFF  },
    {   BITS(5:2, 11),  0x00040000,     0x007FFFFF  },
    {   BITS(5:2, 12),  0x00020000,     0x007FFFFF  },
    {   BITS(5:2, 13),  0x00010000,     0x007FFFFF  },
    {   BITS(5:2, 14),  0x00000000,     0x007FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x007FFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_IC25LP128[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x00FEFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 2),   0x00FDFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 3),   0x00FBFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 4),   0x00F7FFFF,     0x00FFFFFF  },
    {   BITS(5:2, 5),   0x00DFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 6),   0x00CFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 7),   0x009BFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 8),   0x0037FFFF,     0x00FFFFFF  },
    {   BITS(5:2, 9),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 10),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 11),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 12),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 13),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 14),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 15),  0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0x00FFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_PN25F128B[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0X02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0X03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X06),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X08),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x00FFFFFF  },
	
	{   BITS(6:2, 0X10),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x00FBFFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x00F7FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x00EFFFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x00DFFFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x00BFFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X18),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X19),   0x00040000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1A),   0x00080000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1B),   0x00100000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1C),   0x00200000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1D),   0x00400000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1E),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x00FFFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XT25F128A[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0X02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0X03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X06),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X08),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x00FFFFFF  },
	
	{   BITS(6:2, 0X10),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x00FBFFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x00F7FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x00EFFFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x00DFFFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x00BFFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X18),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X19),   0x00040000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1A),   0x00080000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1B),   0x00100000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1C),   0x00200000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1D),   0x00400000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1E),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x00FFFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_PN25F64B[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X01),   0x007F0000,     0x007FFFFF  },
    {   BITS(6:2, 0X02),   0x007E0000,     0x007FFFFF  },
    {   BITS(6:2, 0X03),   0x007C0000,     0x007FFFFF  },
    {   BITS(6:2, 0X04),   0x00780000,     0x007FFFFF  },
    {   BITS(6:2, 0X05),   0x00700000,     0x007FFFFF  },
    {   BITS(6:2, 0X06),   0x00600000,     0x007FFFFF  },
    {   BITS(6:2, 0X07),   0x00400000,     0x007FFFFF  },
    {   BITS(6:2, 0X08),   0x00200000,     0x007FFFFF  },
    {   BITS(6:2, 0X09),   0x00100000,     0x007FFFFF  },
    {   BITS(6:2, 0X0A),   0x00080000,     0x007FFFFF  },
    {   BITS(6:2, 0X0B),   0x00040000,     0x007FFFFF  },
    {   BITS(6:2, 0X0C),   0x00020000,     0x007FFFFF  },
    {   BITS(6:2, 0X0D),   0x00010000,     0x007FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x007FFFFF  },
	
	{   BITS(6:2, 0X10),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x0000FFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x0001FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X18),   0x00000000,     0x005FFFFF  },
    {   BITS(6:2, 0X19),   0x00000000,     0x006FFFFF  },
    {   BITS(6:2, 0X1A),   0x00000000,     0x0077FFFF  },
    {   BITS(6:2, 0X1B),   0x00000000,     0x007BFFFF  },
    {   BITS(6:2, 0X1C),   0x00000000,     0x007DFFFF  },
    {   BITS(6:2, 0X1D),   0x00000000,     0x007EFFFF  },
    {   BITS(6:2, 0X1E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x007FFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XM25QH128A[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0X02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0X03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X06),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X08),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x00FFFFFF  },
	
	{   BITS(6:2, 0X10),   0xFFFFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x00FBFFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x00F7FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x00EFFFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x00DFFFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x00BFFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X18),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X19),   0x00040000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1A),   0x00080000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1B),   0x00100000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1C),   0x00200000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1D),   0x00400000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1E),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x00FFFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XM25QH64A[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X01),   0x007F0000,     0x007FFFFF  },
    {   BITS(6:2, 0X02),   0x007E0000,     0x007FFFFF  },
    {   BITS(6:2, 0X03),   0x007C0000,     0x007FFFFF  },
    {   BITS(6:2, 0X04),   0x00780000,     0x007FFFFF  },
    {   BITS(6:2, 0X05),   0x00700000,     0x007FFFFF  },
    {   BITS(6:2, 0X06),   0x00600000,     0x007FFFFF  },
    {   BITS(6:2, 0X07),   0x00400000,     0x007FFFFF  },
    {   BITS(6:2, 0X08),   0x00200000,     0x007FFFFF  },
    {   BITS(6:2, 0X09),   0x00100000,     0x007FFFFF  },
    {   BITS(6:2, 0X0A),   0x00080000,     0x007FFFFF  },
    {   BITS(6:2, 0X0B),   0x00040000,     0x007FFFFF  },
    {   BITS(6:2, 0X0C),   0x00020000,     0x007FFFFF  },
    {   BITS(6:2, 0X0D),   0x00010000,     0x007FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x007FFFFF  },
	
	{   BITS(6:2, 0X10),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x0000FFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x0001FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X18),   0x00000000,     0x005FFFFF  },
    {   BITS(6:2, 0X19),   0x00000000,     0x006FFFFF  },
    {   BITS(6:2, 0X1A),   0x00000000,     0x0077FFFF  },
    {   BITS(6:2, 0X1B),   0x00000000,     0x007BFFFF  },
    {   BITS(6:2, 0X1C),   0x00000000,     0x007DFFFF  },
    {   BITS(6:2, 0X1D),   0x00000000,     0x007EFFFF  },
    {   BITS(6:2, 0X1E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x007FFFFF  },
   
};



static ST_WRITE_PROTECT _pstWriteProtectTable_W25Q128[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x007E0000,     0x007FFFFF  },
    {   BITS(5:2, 2),   0x007C0000,     0x007FFFFF  },
    {   BITS(5:2, 3),   0x00780000,     0x007FFFFF  },
    {   BITS(5:2, 4),   0x00700000,     0x007FFFFF  },
    {   BITS(5:2, 5),   0x00600000,     0x007FFFFF  },
    {   BITS(5:2, 6),   0x00400000,     0x007FFFFF  },
    {   BITS(5:2, 9),   0x00000000,     0x0001FFFF  },
    {   BITS(5:2, 10),  0x00000000,     0x0003FFFF  },
    {   BITS(5:2, 11),  0x00000000,     0x0007FFFF  },
    {   BITS(5:2, 12),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 13),  0x00000000,     0x001FFFFF  },
    {   BITS(5:2, 14),  0x00000000,     0x003FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x007FFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XT25F64B[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0x007FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X01),   0x007E0000,     0x007FFFFF  },
    {   BITS(6:2, 0X02),   0x007C0000,     0x007FFFFF  },
    {   BITS(6:2, 0X03),   0x00780000,     0x007FFFFF  },
    {   BITS(6:2, 0X04),   0x00700000,     0x007FFFFF  },
    {   BITS(6:2, 0X05),   0x00600000,     0x007FFFFF  },
    {   BITS(6:2, 0X06),   0x00400000,     0x007FFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X08),   0x007FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0001FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x007FFFFF  },
	
	{   BITS(6:2, 0X10),   0x007FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X11),   0x007FF000,     0x007FFFFF  },
    {   BITS(6:2, 0X12),   0x007FE000,     0x007FFFFF  },
    {   BITS(6:2, 0X13),   0x007FC000,     0x007FFFFF  },
    {   BITS(6:2, 0X14),   0x007F8000,     0x007FFFFF  },
    {   BITS(6:2, 0X15),   0x007F8000,     0x007FFFFF  },
    {   BITS(6:2, 0X16),   0x007F8000,     0x007FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X18),   0x007FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X19),   0x00000000,     0x00000FFF  },
    {   BITS(6:2, 0X1A),   0x00000000,     0x00001FFF  },
    {   BITS(6:2, 0X1B),   0x00000000,     0x00003FFF  },
    {   BITS(6:2, 0X1C),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1D),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1E),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x007FFFFF  },
   
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XT25F128B[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X01),   0x00FC0000,     0x00FFFFFF  },
    {   BITS(6:2, 0X02),   0x00F80000,     0x00FFFFFF  },
    {   BITS(6:2, 0X03),   0x00F00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X04),   0x00E00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X05),   0x00C00000,     0x00FFFFFF  },
    {   BITS(6:2, 0X06),   0x00800000,     0x00FFFFFF  },
    {   BITS(6:2, 0X07),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X08),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X09),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X0A),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X0B),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X0C),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X0D),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x00FFFFFF  },

	{   BITS(6:2, 0X10),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X11),   0x00FFF000,     0x00FFFFFF  },
    {   BITS(6:2, 0X12),   0x00FFE000,     0x00FFFFFF  },
    {   BITS(6:2, 0X13),   0x00FFC000,     0x00FFFFFF  },
    {   BITS(6:2, 0X14),   0x00FF8000,     0x00FFFFFF  },
    {   BITS(6:2, 0X15),   0x00FF8000,     0x00FFFFFF  },
    {   BITS(6:2, 0X16),   0x00FF8000,     0x00FFFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 0X18),   0x00FFFFFF,     0x00FFFFFF  },
    {   BITS(6:2, 0X19),   0x00000000,     0x00000FFF  },
    {   BITS(6:2, 0X1A),   0x00000000,     0x00001FFF  },
    {   BITS(6:2, 0X1B),   0x00000000,     0x00003FFF  },
    {   BITS(6:2, 0X1C),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1D),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1E),   0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x00FFFFFF  },

};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25Q16[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 1),   0x00000000,     0x001EFFFF  },//1, 31
    {   BITS(5:2, 2),   0x00000000,     0x001DFFFF  },//2, 30~31
    {   BITS(5:2, 3),   0x00000000,     0x001BFFFF  },//3, 28~31
    {   BITS(5:2, 4),   0x00000000,     0x0017FFFF  },//4, 24~31
    {   BITS(5:2, 5),   0x00000000,     0x000FFFFF  },//5, 16~31
    {   BITS(5:2, 6),   0x00000000,     0x001FFFFF  },//6, all
    {   BITS(5:2, 7),   0x00000000,     0x001FFFFF  },//7, all
    {   BITS(5:2, 8),   0xFFFFFFFF,     0xFFFFFFFF  },//8, 31
    {   BITS(5:2, 9),   0x001FFFFF,     0x00010000  },//9, 30~31
    {   BITS(5:2, 10),  0x001FFFFF,     0x00020000  },//10, 28~31
    {   BITS(5:2, 11),  0x001FFFFF,     0x00040000  },//11, 24~31
    {   BITS(5:2, 12),  0x001FFFFF,     0x00080000  },//12, 16~31
    {   BITS(5:2, 13),  0x001FFFFF,     0x00100000  },//13, all
    {   BITS(5:2, 14),  0x001FFFFF,     0x00000000  },//14, all
    {   BITS(5:2, 15),  0x001FFFFF,     0x00000000  },//15, all
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },//0, none
};

#define FROM_BLK(x, blk_size)   ((x) * (blk_size))
#define TO_BLK(x, blk_size)     (((x) + 1) * (blk_size) - 1)

static ST_WRITE_PROTECT _pstWriteProtectTable_W25X40[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 0),  0xFFFFFFFF,                0xFFFFFFFF            },
    {   BITS(5:2, 1),  FROM_BLK(7,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  },
    {   BITS(5:2, 2),  FROM_BLK(6,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  },
    {   BITS(5:2, 3),  FROM_BLK(4,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  },
    {   BITS(5:2, 9),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(0, SIZE_64KB)  },
    {   BITS(5:2,10),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(1, SIZE_64KB)  },
    {   BITS(5:2,11),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(3, SIZE_64KB)  },
    {   BITS(5:2,15),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L8005[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 7),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 7,  00~15
    {   BITS(4:2, 6),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 6,  00~15
    {   BITS(4:2, 5),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 5,  00~15
    {   BITS(4:2, 4),   FROM_BLK(  8, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 4,  08~15
    {   BITS(4:2, 3),   FROM_BLK( 12, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 3,  12~15
    {   BITS(4:2, 2),   FROM_BLK( 14, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 2,  14~15
    {   BITS(4:2, 1),   FROM_BLK( 15, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 1,  15~15
    {   BITS(4:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25QH64A[] =
{
    //   BPX,                    Lower Bound         Upper Bound
	{   BITS(6:2, 0X00),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X01),   0x007F0000,     0x007FFFFF  },
    {   BITS(6:2, 0X02),   0x007E0000,     0x007FFFFF  },
    {   BITS(6:2, 0X03),   0x007C0000,     0x007FFFFF  },
    {   BITS(6:2, 0X04),   0x00780000,     0x007FFFFF  },
    {   BITS(6:2, 0X05),   0x00700000,     0x007FFFFF  },
    {   BITS(6:2, 0X06),   0x00600000,     0x007FFFFF  },
    {   BITS(6:2, 0X07),   0x00400000,     0x007FFFFF  },
    {   BITS(6:2, 0X08),   0x00200000,     0x007FFFFF  },
    {   BITS(6:2, 0X09),   0x00100000,     0x007FFFFF  },
    {   BITS(6:2, 0X0A),   0x00080000,     0x007FFFFF  },
    {   BITS(6:2, 0X0B),   0x00040000,     0x007FFFFF  },
    {   BITS(6:2, 0X0C),   0x00020000,     0x007FFFFF  },
    {   BITS(6:2, 0X0D),   0x00010000,     0x007FFFFF  },
    {   BITS(6:2, 0X0E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X0F),   0x00000000,     0x007FFFFF  },
	
	{   BITS(6:2, 0X10),   0xFF7FFFFF,     0x007FFFFF  },
    {   BITS(6:2, 0X11),   0x00000000,     0x0000FFFF  },
    {   BITS(6:2, 0X12),   0x00000000,     0x0001FFFF  },
    {   BITS(6:2, 0X13),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 0X14),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 0X15),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 0X16),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 0X17),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 0X18),   0x00000000,     0x005FFFFF  },
    {   BITS(6:2, 0X19),   0x00000000,     0x006FFFFF  },
    {   BITS(6:2, 0X1A),   0x00000000,     0x0077FFFF  },
    {   BITS(6:2, 0X1B),   0x00000000,     0x007BFFFF  },
    {   BITS(6:2, 0X1C),   0x00000000,     0x007DFFFF  },
    {   BITS(6:2, 0X1D),   0x00000000,     0x007EFFFF  },
    {   BITS(6:2, 0X1E),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 0X1F),   0x00000000,     0x007FFFFF  },
   
};
static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L3205D[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(62, SIZE_64KB) }, // 0~62
    {   BITS(5:2, 13),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(61, SIZE_64KB) }, // 0~61
    {   BITS(5:2, 12),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(59, SIZE_64KB) }, // 0~59
    {   BITS(5:2, 11),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(55, SIZE_64KB) }, // 0~55
    {   BITS(5:2, 10),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(47, SIZE_64KB) }, // 0~47
    {   BITS(5:2, 9),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 6),   FROM_BLK(32, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 5),   FROM_BLK(48, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 4),   FROM_BLK(56, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 3),   FROM_BLK(60, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 2),   FROM_BLK(62, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 1),   FROM_BLK(63, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 63
    {   BITS(5:2, 15),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};
static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L3206E[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 15),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 14),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(62, SIZE_64KB) }, // 0~62
    {   BITS(5:2, 13),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(61, SIZE_64KB) }, // 0~61
    {   BITS(5:2, 12),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(59, SIZE_64KB) }, // 0~59
    {   BITS(5:2, 11),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(55, SIZE_64KB) }, // 0~55
    {   BITS(5:2, 10),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(47, SIZE_64KB) }, // 0~47
    {   BITS(5:2, 9),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 8),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 7),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 6),   FROM_BLK(32, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 5),   FROM_BLK(48, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 4),   FROM_BLK(56, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 3),   FROM_BLK(60, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 2),   FROM_BLK(62, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 1),   FROM_BLK(63, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 63
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};

/* static ST_WRITE_PROTECT _pstWriteProtectTable_KH25L6406E[] =
{
    //  BPX,            Lower Bound                 Upper Bound

    {   BITS(5:2, 1),   FROM_BLK(126, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 2),   FROM_BLK(124, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 3),   FROM_BLK(120, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 4),   FROM_BLK(112, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 5),   FROM_BLK(96, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 6),   FROM_BLK(64,  SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 00~63
	{   BITS(5:2, 7),   FROM_BLK(0, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 63
    {   BITS(5:2, 8),   FROM_BLK(0, SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 9),   FROM_BLK(0, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 10),   FROM_BLK(0, SIZE_64KB),    TO_BLK(95, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 11),   FROM_BLK(0, SIZE_64KB),    TO_BLK(111, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 12),   FROM_BLK(0, SIZE_64KB),    TO_BLK(119, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 13),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(123, SIZE_64KB) }, // 00~63
	{   BITS(5:2, 14),   FROM_BLK(0, SIZE_64KB),    TO_BLK(125, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 15),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(127, SIZE_64KB) }, // 00~6
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
	
}; */

static ST_WRITE_PROTECT _pstWriteProtectTable_SST25VF032B[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(2:0, 1),   FROM_BLK(63, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 63
    {   BITS(2:0, 2),   FROM_BLK(62, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 62~63
    {   BITS(2:0, 3),   FROM_BLK(60, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(2:0, 4),   FROM_BLK(56, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 56~63
    {   BITS(2:0, 5),   FROM_BLK(48, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 48~63
    {   BITS(2:0, 6),   FROM_BLK(32, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(2:0, 7),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 00~63
    {   BITS(2:0, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L6405D[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(125, SIZE_64KB) }, // 0~125
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(123, SIZE_64KB) }, // 0~123
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(119, SIZE_64KB) }, // 0~119
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(111, SIZE_64KB) }, // 0~111
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(95,  SIZE_64KB) }, // 0~95
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(63,  SIZE_64KB) }, // 0~63
    {   BITS(5:2, 6),   FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 64~127
    {   BITS(5:2, 5),   FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 96~127
    {   BITS(5:2, 4),   FROM_BLK(112, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 112~127
    {   BITS(5:2, 3),   FROM_BLK(120, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 120~127
    {   BITS(5:2, 2),   FROM_BLK(124, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 124~127
    {   BITS(5:2, 1),   FROM_BLK(126, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 126~127
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 0~127
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L1606E[] =
{
    //  BPX,            Lower Bound                 Upper Bound
	{   BITS(5:2, 1),  FROM_BLK(31,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 31~31
	{   BITS(5:2, 2),  FROM_BLK(30,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 30~31
    {   BITS(5:2, 3),  FROM_BLK(28,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 28~31
    {   BITS(5:2, 4),  FROM_BLK(24,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 24~31
    {   BITS(5:2, 5),  FROM_BLK(16,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 16~31
    {   BITS(5:2, 6),  FROM_BLK(0,    SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 7),  FROM_BLK(0,    SIZE_64KB),   TO_BLK(31,  SIZE_64KB) }, // 0~31
    {   BITS(5:2, 8),  FROM_BLK(0,    SIZE_64KB),   TO_BLK(31,  SIZE_64KB) }, // 0~31
    {   BITS(5:2, 9),  FROM_BLK(0,    SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 10), FROM_BLK(0,    SIZE_64KB),   TO_BLK(15, SIZE_64KB) }, // 0~15
    {   BITS(5:2, 11), FROM_BLK(0,    SIZE_64KB),   TO_BLK(23, SIZE_64KB) }, // 0~23
    {   BITS(5:2, 12), FROM_BLK(0,    SIZE_64KB),   TO_BLK(27, SIZE_64KB) }, // 0~27
    {   BITS(5:2, 13), FROM_BLK(0,    SIZE_64KB),   TO_BLK(29, SIZE_64KB) }, // 0~29
    {   BITS(5:2, 14), FROM_BLK(0,    SIZE_64KB),   TO_BLK(30, SIZE_64KB) }, // 0~30
    {   BITS(5:2, 15), FROM_BLK(0,    SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // none
};
static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L6406E[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(125, SIZE_64KB) }, // 0~125
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(123, SIZE_64KB) }, // 0~123
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(119, SIZE_64KB) }, // 0~119
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(111, SIZE_64KB) }, // 0~111
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(95,  SIZE_64KB) }, // 0~95
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(63,  SIZE_64KB) }, // 0~63
    {   BITS(5:2, 6),   FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 64~127
    {   BITS(5:2, 5),   FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 96~127
    {   BITS(5:2, 4),   FROM_BLK(112, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 112~127
    {   BITS(5:2, 3),   FROM_BLK(120, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 120~127
    {   BITS(5:2, 2),   FROM_BLK(124, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 124~127
    {   BITS(5:2, 1),   FROM_BLK(126, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 126~127
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 0~127
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // none
};

// New MXIC Flash with the same RDID as MX25L6405D
ST_WRITE_PROTECT _pstWriteProtectTable_MX25L6445E[] =
{
    //   BPX,                    Lower Bound                            Upper Bound
    {   BITS(5:2, 6),   FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 064~127
    {   BITS(5:2, 5),   FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 096~127
    {   BITS(5:2, 4),   FROM_BLK(112, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 112~127
    {   BITS(5:2, 3),   FROM_BLK(120, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 120~127
    {   BITS(5:2, 2),   FROM_BLK(124, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 124~127
    {   BITS(5:2, 1),   FROM_BLK(126, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 126~127
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 8),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 000~127
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L6455E[] =
{
    //  BPX,                            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 14, 0~127
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 13, 0~127
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 12, 0~127
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 11, 0~127
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 10, 0~127
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 9,  0~127
    {   BITS(5:2, 8),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 8,  0~127
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 7,  0~127
    {   BITS(5:2, 6),   FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 6,  64~127
    {   BITS(5:2, 5),   FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 5,  96~127
    {   BITS(5:2, 4),   FROM_BLK(112, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 4,  112~127
    {   BITS(5:2, 3),   FROM_BLK(120, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 3,  120~127
    {   BITS(5:2, 2),   FROM_BLK(124, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 2,  124~127
    {   BITS(5:2, 1),   FROM_BLK(126, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 1,  126~127
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 15, 0~127
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L4006E[] =
{
       //BPX,                    Lower Bound                Upper Bound
      {   BITS(4:2, 0), 0xFFFFFFFF,                0xFFFFFFFF            }, // 0,  none
      {   BITS(4:2, 1), FROM_BLK(7,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 1, 7
      {   BITS(4:2, 2), FROM_BLK(6,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 2, 6-7
      {   BITS(4:2, 3), FROM_BLK(4,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 3, 4-7
      {   BITS(4:2, 4), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 4, all
      {   BITS(4:2, 5), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all
      {   BITS(4:2, 6), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all
      {   BITS(4:2, 7), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all

};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25Q32A[] =
{
    //   BPX,                    Lower Bound                            Upper Bound
    {   BITS(5:2, 1),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(62, SIZE_64KB) }, // 00~62
    {   BITS(5:2, 2),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(61, SIZE_64KB) }, // 00~61
    {   BITS(5:2, 3),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(59, SIZE_64KB) }, // 00~59
    {   BITS(5:2, 4),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(55, SIZE_64KB) }, // 00~55
    {   BITS(5:2, 5),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(47, SIZE_64KB) }, // 00~47
    {   BITS(5:2, 6),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(31, SIZE_64KB) }, // 00~31
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 00~63
    {   BITS(5:2, 9),   FROM_BLK(1,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 01~63
    {   BITS(5:2, 10),  FROM_BLK(2,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 02~63
    {   BITS(5:2, 11),  FROM_BLK(4,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 04~63
    {   BITS(5:2, 12),  FROM_BLK(8,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 08~63
    {   BITS(5:2, 13),  FROM_BLK(16,  SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 16~63
    {   BITS(5:2, 14),  FROM_BLK(32,  SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 00~63
    {   BITS(5:2, 8),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};
static ST_WRITE_PROTECT _pstWriteProtectTable_EN25Q64[] =
{
    //   BPX,                    Lower Bound                            Upper Bound
    {   BITS(5:2, 1),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(126, SIZE_64KB) }, // 00~126
    {   BITS(5:2, 2),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(125, SIZE_64KB) }, // 00~125
    {   BITS(5:2, 3),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(123, SIZE_64KB) }, // 00~123
    {   BITS(5:2, 4),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(119, SIZE_64KB) }, // 00~119
    {   BITS(5:2, 5),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(111, SIZE_64KB) }, // 00~111
    {   BITS(5:2, 6),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(95, SIZE_64KB) }, // 00~95
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 9),   FROM_BLK(1,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 10),  FROM_BLK(2,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 11),  FROM_BLK(4,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 12),  FROM_BLK(8,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 13),  FROM_BLK(16,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 14),  FROM_BLK(32,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 00~127
    {   BITS(5:2, 8),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_M25PX16[] =
{
    //  BPX,           Lower Bound                Upper Bound
    {   BITS(5:2, 0),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 1),  FROM_BLK(31,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 2),  FROM_BLK(30,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 3),  FROM_BLK(28,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 4),  FROM_BLK(24,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 5),  FROM_BLK(16,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 6),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 7),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 8),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 9),  FROM_BLK(0,  SIZE_64KB),  TO_BLK(0, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 10), FROM_BLK(0,  SIZE_64KB),  TO_BLK(1, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 11), FROM_BLK(0,  SIZE_64KB),  TO_BLK(3, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 12), FROM_BLK(0,  SIZE_64KB),  TO_BLK(7, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 13), FROM_BLK(0,  SIZE_64KB),  TO_BLK(15, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 14), FROM_BLK(0,  SIZE_64KB),  TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 15), FROM_BLK(0,   SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25Q128[] =
{
    //   BPX,                    Lower Bound                            Upper Bound
    {   BITS(5:2, 1),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(254, SIZE_64KB) }, // 00~254
    {   BITS(5:2, 2),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(253, SIZE_64KB) }, // 00~253
    {   BITS(5:2, 3),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(251, SIZE_64KB) }, // 00~251
    {   BITS(5:2, 4),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(247, SIZE_64KB) }, // 00~247
    {   BITS(5:2, 5),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(239, SIZE_64KB) }, // 00~239
    {   BITS(5:2, 6),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(223, SIZE_64KB) }, // 00~223
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 00~255
    {   BITS(5:2, 9),   FROM_BLK(1,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 01~255
    {   BITS(5:2, 10),  FROM_BLK(2,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 02~255
    {   BITS(5:2, 11),  FROM_BLK(4,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 04~255
    {   BITS(5:2, 12),  FROM_BLK(8,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 08~255
    {   BITS(5:2, 13),  FROM_BLK(16,  SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 16~255
    {   BITS(5:2, 14),  FROM_BLK(32,  SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 32~255
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 00~255
    {   BITS(5:2, 8),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};
ST_WRITE_PROTECT _pstWriteProtectTable_EN25QH128A[] =
{
    //   BPX,           Lower Bound                 Upper Bound
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            },
    {   BITS(5:2, 1),   FROM_BLK(252, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 2),   FROM_BLK(248, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 3),   FROM_BLK(240, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 4),   FROM_BLK(224, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 5),   FROM_BLK(192, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 6),   FROM_BLK(128, SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
    {   BITS(5:2, 8),   0xFFFFFFFF,                 0xFFFFFFFF             },
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(3,   SIZE_64KB) },
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(7,   SIZE_64KB) },
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(15,  SIZE_64KB) },
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(31,  SIZE_64KB) },
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) },
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) },
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) },
};

/*
static ST_WRITE_PROTECT _pstWriteProtectTable_EN25F32[] =
{
    {   BITS(4:2, 7),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(4:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};
*/
static ST_WRITE_PROTECT _pstWriteProtectTable_MX25L12805D[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 8),   FROM_BLK(128, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 128~255
    {   BITS(5:2, 7),   FROM_BLK(192, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 192~255
    {   BITS(5:2, 6),   FROM_BLK(224, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 224~255
    {   BITS(5:2, 5),   FROM_BLK(240, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 240~255
    {   BITS(5:2, 4),   FROM_BLK(248, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 248~255
    {   BITS(5:2, 3),   FROM_BLK(252, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 252~255
    {   BITS(5:2, 2),   FROM_BLK(254, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 254~255
    {   BITS(5:2, 1),   FROM_BLK(255, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 255
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 0~255
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25F40[] =
{
    //  BPX,          Lower Bound              Upper Bound
    {   BITS(4:2, 0), 0xFFFFFFFF,              0xFFFFFFFF           }, // 0,  none
    {   BITS(4:2, 1), FROM_BLK(7,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 1, 7
    {   BITS(4:2, 2), FROM_BLK(6,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 2, 6-7
    {   BITS(4:2, 3), FROM_BLK(4,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 3, 4-7
    {   BITS(4:2, 4), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 4, all
    {   BITS(4:2, 5), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all
    {   BITS(4:2, 6), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all
    {   BITS(4:2, 7), FROM_BLK(0,  SIZE_64KB),   TO_BLK(7, SIZE_64KB)  }, // 7, all
};

// New MXIC Flash with the same RDID as MX25L12805D
ST_WRITE_PROTECT _pstWriteProtectTable_MX25L12845E[] =
{
    //  BPX,                            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 14, 0~255
    {   BITS(5:2, 13),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 13, 0~255
    {   BITS(5:2, 12),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 12, 0~255
    {   BITS(5:2, 11),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 11, 0~255
    {   BITS(5:2, 10),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 10, 0~255
    {   BITS(5:2, 9),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 9,  0~255
    {   BITS(5:2, 8),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 8,  0~255
    {   BITS(5:2, 7),   FROM_BLK(128, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 7,  128~255
    {   BITS(5:2, 6),   FROM_BLK(192,  SIZE_64KB),  TO_BLK(255, SIZE_64KB) }, // 6,  192~255
    {   BITS(5:2, 5),   FROM_BLK(224,  SIZE_64KB),  TO_BLK(255, SIZE_64KB) }, // 5,  224~255
    {   BITS(5:2, 4),   FROM_BLK(240, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 4,  240~255
    {   BITS(5:2, 3),   FROM_BLK(248, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 3,  248~255
    {   BITS(5:2, 2),   FROM_BLK(252, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 2,  252~255
    {   BITS(5:2, 1),   FROM_BLK(254, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 1,  254~255
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 15, 0~255
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

// NOTE: AT26DF321 could protect each sector independently and BITS(5:2, 1)~BITS(5:2, 14) is no change for protection.
// This table is just used to figure out the lower bound and upper bound (no such param in function argument).
static ST_WRITE_PROTECT _pstWriteProtectTable_AT26DF321[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 14),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(62, SIZE_64KB) }, // 0~62
    {   BITS(5:2, 13),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(61, SIZE_64KB) }, // 0~61
    {   BITS(5:2, 12),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(59, SIZE_64KB) }, // 0~59
    {   BITS(5:2, 11),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(55, SIZE_64KB) }, // 0~55
    {   BITS(5:2, 10),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(47, SIZE_64KB) }, // 0~47
    {   BITS(5:2, 9),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 6),   FROM_BLK(32, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 5),   FROM_BLK(48, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 4),   FROM_BLK(56, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 3),   FROM_BLK(60, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 2),   FROM_BLK(62, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 1),   FROM_BLK(63, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 63
    {   BITS(5:2, 15),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_AT25DF321[] =
{
    {   BITS(5:2, 0),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_S25FL128P[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 7),   FROM_BLK(128, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 7,  128~255
    {   BITS(5:2, 6),   FROM_BLK(192, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 6,  192~255
    {   BITS(5:2, 5),   FROM_BLK(224, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 5,  224~255
    {   BITS(5:2, 4),   FROM_BLK(240, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 4,  240~255
    {   BITS(5:2, 3),   FROM_BLK(248, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 3,  248~255
    {   BITS(5:2, 2),   FROM_BLK(252, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 2,  252~255
    {   BITS(5:2, 1),   FROM_BLK(254, SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 1,  254~255
    {   BITS(5:2, 15),  FROM_BLK(0,   SIZE_64KB),   TO_BLK(255, SIZE_64KB) }, // 15, 0~255
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_S25FL008A[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 7),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 7,  00~15
    {   BITS(5:2, 6),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 6,  00~15
    {   BITS(5:2, 5),   FROM_BLK(  0, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 5,  00~15
    {   BITS(5:2, 4),   FROM_BLK(  8, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 4,  08~15
    {   BITS(5:2, 3),   FROM_BLK( 12, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 3,  12~15
    {   BITS(5:2, 2),   FROM_BLK( 14, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 2,  14~15
    {   BITS(5:2, 1),   FROM_BLK( 15, SIZE_64KB),   TO_BLK( 15, SIZE_64KB) }, // 1,  15~15
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25P16[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 1),   0x001F0000,     0x001FFFFF  },//1, 31
    {   BITS(4:2, 2),   0x001E0000,     0x001FFFFF  },//2, 30~31
    {   BITS(4:2, 3),   0x001C0000,     0x001FFFFF  },//3, 28~31
    {   BITS(4:2, 4),   0x00180000,     0x001FFFFF  },//4, 24~31
    {   BITS(4:2, 5),   0x00100000,     0x001FFFFF  },//5, 16~31
    {   BITS(4:2, 6),   0x00000000,     0x001FFFFF  },//6, all
    {   BITS(4:2, 7),   0x00000000,     0x001FFFFF  },//7, all
    {   BITS(4:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },//0, none
};
static ST_WRITE_PROTECT _pstWriteProtectTable_EN25F16[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 1),   0x001F0000,     0x001FFFFF  },//1, 31
    {   BITS(4:2, 2),   0x001E0000,     0x001FFFFF  },//2, 30~31
    {   BITS(4:2, 3),   0x001C0000,     0x001FFFFF  },//3, 28~31
    {   BITS(4:2, 4),   0x00180000,     0x001FFFFF  },//4, 24~31
    {   BITS(4:2, 5),   0x00100000,     0x001FFFFF  },//5, 16~31
    {   BITS(4:2, 6),   0x00000000,     0x001FFFFF  },//6, all
    {   BITS(4:2, 7),   0x00000000,     0x001FFFFF  },//7, all
    {   BITS(4:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },//0, none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25F32[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 1),   0x00000000,     0x003EFFFF  },//0~62
    {   BITS(5:2, 2),   0x00000000,     0x003DFFFF  },//0~61
    {   BITS(5:2, 3),   0x00000000,     0x003BFFFF  },//0~59
    {   BITS(5:2, 4),   0x00000000,     0x0037FFFF  },//0~55
    {   BITS(5:2, 5),   0x00000000,     0x002FFFFF  },//0~47
    {   BITS(5:2, 6),   0x00000000,     0x001FFFFF  },//0~31
    {   BITS(5:2, 7),   0x00000000,     0x003FFFFF  },//all
    {   BITS(5:2, 8),   0xFFFFFFFF,     0xFFFFFFFF  },//none
    {   BITS(5:2, 9),   0x003FFFFF,     0x00010000  },//63~1
    {   BITS(5:2, 10),  0x003FFFFF,     0x00020000  },//63~2
    {   BITS(5:2, 11),  0x003FFFFF,     0x00040000  },//63~4
    {   BITS(5:2, 12),  0x003FFFFF,     0x00080000  },//63~8
    {   BITS(5:2, 13),  0x003FFFFF,     0x00100000  },//63~16
    {   BITS(5:2, 14),  0x003FFFFF,     0x00200000  },//63~32
    {   BITS(5:2, 15),  0x00000000,     0x003FFFFF  },//all
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },//none

};


static ST_WRITE_PROTECT _pstWriteProtectTable_W25Q80[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(6:2, 1),   0x000F0000,     0x000FFFFF  },
    {   BITS(6:2, 2),   0x000E0000,     0x000FFFFF  },
    {   BITS(6:2, 3),   0x000C0000,     0x000FFFFF  },
    {   BITS(6:2, 4),   0x00080000,     0x000FFFFF  },
    {   BITS(6:2, 9),   0x00000000,     0x0000FFFF  },
    {   BITS(6:2, 10),  0x00000000,     0x0001FFFF  },
    {   BITS(6:2, 11),  0x00000000,     0x0003FFFF  },
    {   BITS(6:2, 12),  0x00000000,     0x0007FFFF  },
    {   BITS(6:2, 15),  0x00000000,     0x000FFFFF  },
    {   BITS(6:2, 17),  0x000FF000,     0x000FFFFF  },
    {   BITS(6:2, 18),  0x000FE000,     0x000FFFFF  },
    {   BITS(6:2, 19),  0x000FC000,     0x000FFFFF  },
    {   BITS(6:2, 20),  0x000F8000,     0x000FFFFF  },
    {   BITS(6:2, 25),  0x00000000,     0x00000FFF  },
    {   BITS(6:2, 26),  0x00000000,     0x00001FFF  },
    {   BITS(6:2, 27),  0x00000000,     0x00003FFF  },
    {   BITS(6:2, 28),  0x00000000,     0x00007FFF  },
    {   BITS(6:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};


static ST_WRITE_PROTECT _pstWriteProtectTable_W25X80[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 1),   0x000F0000,     0x000FFFFF  },
    {   BITS(5:2, 2),   0x000E0000,     0x000FFFFF  },
    {   BITS(5:2, 3),   0x000C0000,     0x000FFFFF  },
    {   BITS(5:2, 4),   0x00080000,     0x000FFFFF  },
    {   BITS(5:2, 9),   0x00000000,     0x0000FFFF  },
    {   BITS(5:2, 10),  0x00000000,     0x0001FFFF  },
    {   BITS(5:2, 11),  0x00000000,     0x0003FFFF  },
    {   BITS(5:2, 12),  0x00000000,     0x0007FFFF  },
    {   BITS(5:2, 13),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 15),  0x00000000,     0x000FFFFF  },
    {   BITS(5:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_EN25F80[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 1),   0x000F0000,     0x000FFFFF  },//1, 15
    {   BITS(4:2, 2),   0x000E0000,     0x000FFFFF  },//2, 14~15
    {   BITS(4:2, 3),   0x000C0000,     0x000FFFFF  },//3, 12~15
    {   BITS(4:2, 4),   0x00080000,     0x000FFFFF  },//4, 8~15
    {   BITS(4:2, 5),   0x00000000,     0x000FFFFF  },//5, all
    {   BITS(4:2, 6),   0x00000000,     0x000FFFFF  },//6, all
    {   BITS(4:2, 7),   0x00000000,     0x000FFFFF  },//7, all
    {   BITS(4:2, 0),   0xFFFFFFFF,     0xFFFFFFFF  },//0, NONE

};
static ST_WRITE_PROTECT _pstWriteProtectTable_PM25LQ032C[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(5:2, 15),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 0~63
    {   BITS(5:2, 14),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(31, SIZE_64KB) }, // 0~31
    {   BITS(5:2, 13),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(15, SIZE_64KB) }, // 0~15
    {   BITS(5:2, 12),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(7, SIZE_64KB) }, // 0~7
    {   BITS(5:2, 11),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(3, SIZE_64KB) }, // 0~3
    {   BITS(5:2, 10),  FROM_BLK(0,  SIZE_64KB),    TO_BLK(1, SIZE_64KB) }, // 0~1
    {   BITS(5:2, 9),   FROM_BLK(0,  SIZE_64KB),    TO_BLK(0, SIZE_64KB) }, // 0~0
    {   BITS(5:2, 8),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none
    {   BITS(5:2, 7),   FROM_BLK(0, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 6),   FROM_BLK(32, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 32~63
    {   BITS(5:2, 5),   FROM_BLK(48, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 48~63
    {   BITS(5:2, 4),   FROM_BLK(56, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 56~63
    {   BITS(5:2, 3),   FROM_BLK(60, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 60~63
    {   BITS(5:2, 2),   FROM_BLK(62, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 62~63
    {   BITS(5:2, 1),   FROM_BLK(63, SIZE_64KB),    TO_BLK(63, SIZE_64KB) }, // 63
    {   BITS(5:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF            }, // none

};

static ST_WRITE_PROTECT _pstWriteProtectTable_M25P16[] =
{
    //  BPX,           Lower Bound                Upper Bound
    {   BITS(4:2, 0),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(4:2, 1),  FROM_BLK(31,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 1, 7
    {   BITS(4:2, 2),  FROM_BLK(30,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 2, 6-7
    {   BITS(4:2, 3),  FROM_BLK(28,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 3, 4-7
    {   BITS(4:2, 4),  FROM_BLK(24,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 4, all
    {   BITS(4:2, 5),  FROM_BLK(16,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(4:2, 6),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(4:2, 7),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(31, SIZE_64KB)  }, // 7, all

};

static ST_WRITE_PROTECT _pstWriteProtectTable_N25Q32[] =
{
    //  BPX,           Lower Bound                Upper Bound
    {   BITS(5:2, 0),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 1),  FROM_BLK(63,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 2),  FROM_BLK(62,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 3),  FROM_BLK(60,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 4),  FROM_BLK(56,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 5),  FROM_BLK(48,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 6),  FROM_BLK(32,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 7),  FROM_BLK(0,  SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 8),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 9),  FROM_BLK(0,  SIZE_64KB),  TO_BLK(0, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 10), FROM_BLK(0,  SIZE_64KB),  TO_BLK(1, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 11), FROM_BLK(0,  SIZE_64KB),  TO_BLK(3, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 12), FROM_BLK(0,  SIZE_64KB),  TO_BLK(7, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 13), FROM_BLK(0,  SIZE_64KB),  TO_BLK(15, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 14), FROM_BLK(0,  SIZE_64KB),  TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 15), FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 7, all
};

static ST_WRITE_PROTECT _pstWriteProtectTable_N25Q64[] =
{
    //  BPX,           Lower Bound                Upper Bound
    {   BITS(5:2, 0),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 1),  FROM_BLK(127,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 2),  FROM_BLK(126,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 3),  FROM_BLK(124,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 4),  FROM_BLK(120,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 5),  FROM_BLK(112,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 6),  FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 7),  FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 8),  0xFFFFFFFF,                0xFFFFFFFF           }, // 0,  none
    {   BITS(5:2, 9),  FROM_BLK(0,  SIZE_64KB),  TO_BLK(0, SIZE_64KB)  }, // 1, 7
    {   BITS(5:2, 10), FROM_BLK(0,  SIZE_64KB),  TO_BLK(1, SIZE_64KB)  }, // 2, 6-7
    {   BITS(5:2, 11), FROM_BLK(0,  SIZE_64KB),  TO_BLK(3, SIZE_64KB)  }, // 3, 4-7
    {   BITS(5:2, 12), FROM_BLK(0,  SIZE_64KB),  TO_BLK(7, SIZE_64KB)  }, // 4, all
    {   BITS(5:2, 13), FROM_BLK(0,  SIZE_64KB),  TO_BLK(15, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 14), FROM_BLK(0,  SIZE_64KB),  TO_BLK(31, SIZE_64KB)  }, // 7, all
    {   BITS(5:2, 15), FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB)  }, // 7, all
};

static ST_WRITE_PROTECT _pstWriteProtectTable_N25Q128[] =
{
    //  BPX,           Lower Bound                Upper Bound
    {   BITS(5:2, 0),  0xFFFFFFFF,                0xFFFFFFFF           },
    {   BITS(5:2, 1),  FROM_BLK(255,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 2),  FROM_BLK(254,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 3),  FROM_BLK(252,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 4),  FROM_BLK(248,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 5),  FROM_BLK(240,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 6),  FROM_BLK(224,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 7),  FROM_BLK(192,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 8),  FROM_BLK(128,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 9),  FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 10), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 11), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 12), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 13), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 14), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
    {   BITS(5:2, 15), FROM_BLK(0  ,  SIZE_64KB),   TO_BLK(255, SIZE_64KB)  },
};

static ST_WRITE_PROTECT _pstWriteProtectTable_S25FL032[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 6),   FROM_BLK(32, SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 1,  126~127
    {   BITS(4:2, 5),   FROM_BLK(48, SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 2,  124~127
    {   BITS(4:2, 4),   FROM_BLK(56, SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 3,  120~127
    {   BITS(4:2, 3),   FROM_BLK(60, SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 4,  112~127
    {   BITS(4:2, 2),   FROM_BLK(62,  SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 5,  096~127
    {   BITS(4:2, 1),   FROM_BLK(63,  SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 6,  064~127
    {   BITS(4:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(63, SIZE_64KB) }, // 7,  000~127
    {   BITS(4:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_S25FL064[] =
{
    //  BPX,            Lower Bound                 Upper Bound
    {   BITS(4:2, 6),   FROM_BLK(126, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 1,  126~127
    {   BITS(4:2, 5),   FROM_BLK(124, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 2,  124~127
    {   BITS(4:2, 4),   FROM_BLK(120, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 3,  120~127
    {   BITS(4:2, 3),   FROM_BLK(112, SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 4,  112~127
    {   BITS(4:2, 2),   FROM_BLK(96,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 5,  096~127
    {   BITS(4:2, 1),   FROM_BLK(64,  SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 6,  064~127
    {   BITS(4:2, 7),   FROM_BLK(0,   SIZE_64KB),   TO_BLK(127, SIZE_64KB) }, // 7,  000~127
    {   BITS(4:2, 0),   0xFFFFFFFF,                 0xFFFFFFFF             }, // 0,  none
};

static ST_WRITE_PROTECT _pstWriteProtectTable_XM25QH256B[] =
{
    //   BPX,                    Lower Bound         Upper Bound
    {   BITS(5:2, 0),   0xFFFFFFFF,                    0XFFFFFFFF               },
    {   BITS(5:2, 1),   FROM_BLK(511,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 2),   FROM_BLK(510,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 3),   FROM_BLK(508,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 4),   FROM_BLK(504,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 5),   FROM_BLK(496,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 6),   FROM_BLK(480,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 7),   FROM_BLK(448,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 8),   FROM_BLK(384,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 9),   FROM_BLK(256,   SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 10),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 11),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 12),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 13),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 14),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
    {   BITS(5:2, 15),  FROM_BLK(0,     SIZE_64KB),   TO_BLK(511, SIZE_64KB) },
};

//
//  Flash Info Table (List)
//
hal_SERFLASH_t _hal_SERFLASH_table[] =   // Need to Add more sample for robust
{
    /**********************************************************************/
    /* 01.  u16FlashType                                                                                                 */
    /* 02.  u8MID                                                                                                           */
    /* 03.  u8DID0                                                                                                          */
    /* 04.  u8DID1                                                                                                          */
    /* 05.  pWriteProtectTable                                                                                          */
    /* 06.  pSpecialBlocks                                                                                                */
    /* 07.  u32FlashSize                                                                                                   */
    /* 08.  u32NumBLK                                                                                                    */
    /* 09.  u32BlockSize                                                                                                   */
    /* 10.  u16PageSize                                                                                                   */
    /* 11.  u16MaxChipWrDoneTimeout                                                                               */
    /* 12.  u8WrsrBlkProtect                                                                                             */
    /* 13.  u16DevSel                                                                                                      */
    /* 14.  u16SpiEndianSel                                                                                              */
    /* 15.  Support 2XREAD(SPI CMD is 0xBB)                                                                      */
    /**********************************************************************/
    { FLASH_IC_UNKNOWN,     0xFF,       0xFF,   0xFF,   NULL,                                 NULL,                       0x1000000,  64,     SIZE_64KB,  256,    50,    BITS(2:0, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  FALSE},
    { FLASH_IC_SST25VF032B, MID_SST,    0x25,   0x4A,   _pstWriteProtectTable_SST25VF032B,    NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(2:0, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L1655D,  MID_MXIC,   0x26,   0x15,   NULL,                                 NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_MX25L3255D,  MID_MXIC,   0x9E,   0x16,   NULL,                                 NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_MX25L6455E,  MID_MXIC,   0x26,   0x17,   _pstWriteProtectTable_MX25L6455E,     NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_MX25L12855E, MID_MXIC,   0x26,   0x18,   _pstWriteProtectTable_MX25L12845E,    NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_MX25L3205D,  MID_MXIC,   0x20,   0x16,   _pstWriteProtectTable_MX25L3205D,     NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L4006E,  MID_MXIC,   0x20,   0x13,   _pstWriteProtectTable_MX25L4006E,     NULL,                       0x80000,    8,      SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L6405D,  MID_MXIC,   0x20,   0x17,   _pstWriteProtectTable_MX25L6405D,     NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L1606E,  MID_MXIC,   0x20,   0x15,   _pstWriteProtectTable_MX25L1606E,     NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE ,  TRUE},
    { FLASH_IC_MX25L12805D, MID_MXIC,   0x20,   0x18,   _pstWriteProtectTable_MX25L12805D,    NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L3206E,  MID_MXIC,   0x20,   0x15,   _pstWriteProtectTable_MX25L3206E,     NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_MX25L8005,   MID_MXIC,   0x20,   0x14,   _pstWriteProtectTable_MX25L8005,      NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_MX25L8006E,  MID_MXIC,   0x20,   0x14,   NULL,                                 NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_MX25L6406E,  MID_MXIC,   0x20,   0x17,   _pstWriteProtectTable_MX25L6406E,     NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, TRUE ,  TRUE},
    { FLASH_IC_MX25L25645G, MID_MXIC,   0x20,   0x19,   NULL,                                 NULL,                       0x2000000,  512,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_MX25L5123G, MID_MXIC,   0x20,   0x1A,    NULL,                                 NULL,                       0x4000000,  1024,   SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_W25Q16,      MID_WB,     0x40,   0x15,   NULL,                                 NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_W25Q80,      MID_WB,     0x40,   0x14,   _pstWriteProtectTable_W25Q80,         NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_W25X32,      MID_WB,     0x30,   0x16,   _pstWriteProtectTable_W25X32,         NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE ,  TRUE},
    { FLASH_IC_W25X40,      MID_WB,     0x30,   0x13,   _pstWriteProtectTable_W25X40,         NULL,                       0x80000,    8,      SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, FALSE,  TRUE},
    { FLASH_IC_W25Q32,      MID_WB,     0x40,   0x16,   _pstWriteProtectTable_W25X32,         NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_W25X64,      MID_WB,     0x30,   0x17,   _pstWriteProtectTable_W25X64,         NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE ,  TRUE},
    { FLASH_IC_W25X80,      MID_WB,     0x30,   0x14,   _pstWriteProtectTable_W25X80,         NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_W25Q64,      MID_WB,     0x40,   0x17,   _pstWriteProtectTable_W25X64,         NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_W25Q128,     MID_WB,     0x40,   0x18,   _pstWriteProtectTable_W25Q128,        NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_W25Q256JV,   MID_WB,     0x40,   0x19, /*_pstWriteProtectTable_W25Q256*/NULL,  NULL,                       0x2000000,  512,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_AT25DF321A,  MID_ATMEL,  0x47,   0x01,   _pstWriteProtectTable_AT25DF321,      NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_AT26DF321,   MID_ATMEL,  0x47,   0x00,   _pstWriteProtectTable_AT26DF321,      NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_STM25P32,    MID_ST,     0x20,   0x16,   NULL,                                 NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25B32B,    MID_EON,    0x20,   0x16,   NULL,                                 &_stSpecialBlocks_EN25B32B, 0x400000,   68,     SIZE_64KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25B64B,    MID_EON,    0x20,   0x17,   NULL,                                 &_stSpecialBlocks_EN25B64B, 0x800000,   132,    SIZE_64KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25Q32A,    MID_EON,    0x30,   0x16,   _pstWriteProtectTable_EN25Q32A,       NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_EN25Q64,     MID_EON,    0x30,   0x17,   _pstWriteProtectTable_EN25Q64,        NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25Q128,    MID_EON,    0x30,   0x18,   _pstWriteProtectTable_EN25Q128,       NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_EN25QH128A,  MID_EON,    0x70,   0x18,   _pstWriteProtectTable_EN25QH128A,     NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
	{ FLASH_IC_EN25QH64A,   MID_EN,     0x70,   0x17,   _pstWriteProtectTable_EN25QH64A,      NULL,                       0x800000,   256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE}, //aaron add
    { FLASH_IC_EN25F10,     MID_EON,    0x31,   0x11,   NULL,                                 &_stSpecialBlocks_EN25F10,  0x20000,    4,      SIZE_32KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25F16,     MID_EON,    0x31,   0x15,   _pstWriteProtectTable_EN25F16,        NULL,                       0x200000,   32,     SIZE_64KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25F32,     MID_EON,    0x31,   0x16,   _pstWriteProtectTable_EN25F32,        NULL,                       0x400000,   64,     SIZE_64KB,  256,    384,   BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25F40,     MID_EON,    0x31,   0x13,   _pstWriteProtectTable_EN25F40,        NULL,                       0x80000,    8,      SIZE_64KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25F80,     MID_EON,    0x31,   0x14,   _pstWriteProtectTable_EN25F80,        NULL,                       0x100000,   16,     SIZE_64KB,  256,    384,   BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25P16,     MID_EON,    0x20,   0x15,   _pstWriteProtectTable_EN25P16,        NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_EN25QH16,    MID_EON,    0x70,   0x15,   _pstWriteProtectTable_EN25Q16,        NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_ZB25Q64 ,    MID_EON,    0x70,   0x17,   _pstWriteProtectTable_ZB25Q64 ,       NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_S25FL032P,   MID_SPAN,   0x02,   0x15,   _pstWriteProtectTable_S25FL032,       NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(4:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_S25FL064P,   MID_SPAN,   0x02,   0x16,   _pstWriteProtectTable_S25FL064,       NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(4:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_S25FL032K,   MID_SPAN,   0x40,   0x16,   _pstWriteProtectTable_S25FL032K_CMP0, NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(6:2, 0x1F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_S25FL128P,   MID_SPAN,   0x20,   0x18,   _pstWriteProtectTable_S25FL128P,      NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_S25FL008A,   MID_SPAN,   0x20,   0x13,   _pstWriteProtectTable_S25FL008A,      NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_XTALI, E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_S25FL128P,   MID_SPAN,   0x20,   0x18,   _pstWriteProtectTable_S25FL128P,      NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_S25FL008A,   MID_SPAN,   0x20,   0x13,   _pstWriteProtectTable_S25FL008A,      NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_XTALI, E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_PM25LQ032C,  MID_PMC,    0x46,   0x15,   _pstWriteProtectTable_PM25LQ032C,     NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x1F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE ,  TRUE},
    { FLASH_IC_GD25Q32,     MID_GD,     0x40,   0x16,   _pstWriteProtectTable_GD25Q32_CMP0,   NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(6:2, 0x1F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_GD25Q16,     MID_GD,     0x40,   0x15,   _pstWriteProtectTable_GD25Q16_CMP0,   NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(6:2, 0x1F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_GD25S80,     MID_GD,     0x40,   0x14,   NULL,                                 NULL,                       0x100000,   16,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_GD25Q64,     MID_GD,     0x40,   0x17,   NULL,                                 NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_GD25Q128,    MID_GD,     0x40,   0x18,   _pstWriteProtectTable_GD25Q128_CMP0,  NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(6:2, 0x1F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_GD25Q256C,   MID_GD,     0x40,   0x19,   NULL,                                 NULL,                       0x2000000,  512,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_M25P16,      MID_MICRON, 0x20,   0x15,   _pstWriteProtectTable_M25P16,         NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(4:2, 0x07),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
    { FLASH_IC_N25Q32,      MID_MICRON, 0xBA,   0x16,   _pstWriteProtectTable_N25Q32,         NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_D_MODE},  TRUE ,  TRUE},
    { FLASH_IC_N25Q64,      MID_MICRON, 0xBA,   0x17,   _pstWriteProtectTable_N25Q64,         NULL,                       0x800000,   128,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_D_MODE},  TRUE,   TRUE},
    { FLASH_IC_N25Q128,     MID_MICRON, 0xBA,   0x18,   _pstWriteProtectTable_N25Q128,        NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_D_MODE},  TRUE,   TRUE},
    { FLASH_IC_M25PX16,     MID_NUMONYX,0x73,   0x15,   _pstWriteProtectTable_M25PX16,        NULL,                       0x200000,   32,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_D_MODE }, TRUE,   TRUE},
    { FLASH_IC_A25LM032,    MID_AM,     0x30,   0x16,   NULL,                                 NULL,                       0x400000,   64,     SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_FAST_MODE   }, FALSE,  TRUE},
	{ FLASH_IC_IC25LP128, 	MID_IC, 	0x60,	0x18,	_pstWriteProtectTable_IC25LP128,	  NULL, 					  0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(5:2, 0x0F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},    //leaf add
    { FLASH_IC_ZB25VQ128,   MID_ZB,     0X40,   0X18,   NULL,                                 NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_DUAL_AD_MODE},  TRUE,   TRUE},
    { FLASH_IC_ZB25Q128,    MID_ZB,     0x70,   0x18,   _pstWriteProtectTable_ZB25Q128,       NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_ST,         ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
	{ FLASH_IC_FS25Q128F2TFI,	MID_FS,	0x40,	0x18,	_pstWriteProtectTable_FS25Q128F2TFI,  NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_PMC,	     ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE},    TRUE,   TRUE},
	{ FLASH_IC_XT25F64B,	MID_XT1,	0x40,	0x17,	_pstWriteProtectTable_XT25F64B,       NULL,                       0x800000,	  128,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_PMC,	     ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE},    TRUE,   TRUE},
    { FLASH_IC_XT25F128B,	MID_XT1,	0x40,	0x18,	_pstWriteProtectTable_XT25F128B,      NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_PMC,	     ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_FAST_MODE},    TRUE,   TRUE},
    { FLASH_IC_XT25F128A,	MID_XT,	    0x70,	0x18,	_pstWriteProtectTable_XT25F128A,      NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},   //aaron add
    { FLASH_IC_PN25F128B,	MID_PN,	    0x70,	0x18,	_pstWriteProtectTable_PN25F128B,      NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},   //aaron add
	{ FLASH_IC_PN25F64B,	MID_PN,	    0x70,	0x17,	_pstWriteProtectTable_PN25F64B,       NULL,                       0x800000,	  128,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},	//aaron add
	{ FLASH_IC_XM25QH128A,	MID_XMC,	    0x70,	0x18,	_pstWriteProtectTable_XM25QH128A,     NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},   //aaron add
	{ FLASH_IC_XM25QH64A,	MID_XMC,	    0x70,	0x17,	_pstWriteProtectTable_XM25QH64A,      NULL,                       0x800000,	  128,	  SIZE_64KB,  256,	  50,	 BITS(6:2, 0x1F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},	//aaron add
	{ FLASH_IC_XM25QH128B,	MID_XMC,	    0x60,	0x18,	NULL,                                 NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(5:2, 0x0F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_XM25QH64B,	MID_XMC,	    0x60,	0x17,	NULL,                                 NULL,                       0x1000000,  256,	  SIZE_64KB,  256,	  50,	 BITS(5:2, 0x0F),	 ISP_DEV_NEXTFLASH,	 ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_XM25QH256B,  MID_XMC,        0x60,   0x19,   _pstWriteProtectTable_XM25QH256B,  NULL,                      0x2000000,  512,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_NEXTFLASH,  ISP_SPI_ENDIAN_LITTLE, {E_SPI_86M  , E_DUAL_AD_MODE}, TRUE,   TRUE},
    { FLASH_IC_25Q128AS,    MID_25Q,    0X40,   0X18,   NULL,                                 NULL,                       0x1000000,  256,    SIZE_64KB,  256,    50,    BITS(5:2, 0x0F),    ISP_DEV_PMC,        ISP_SPI_ENDIAN_LITTLE, {E_SPI_54M  , E_SINGLE_MODE},  TRUE,   TRUE},
};
