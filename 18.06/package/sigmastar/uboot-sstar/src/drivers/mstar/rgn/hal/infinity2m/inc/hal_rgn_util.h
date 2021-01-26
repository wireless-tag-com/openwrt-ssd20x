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


#ifndef __HAL_GOP_UTIL_H__
#define __HAL_GOP_UTIL_H__

#define WRITE_GOP_REG(addr, type, data)  ((*(volatile type *)(addr)) = (data))
#define READ_GOP_REG(addr, type)         ((*(volatile type *)(addr)))
/* macro to get at MMIO space when running virtually */
#define RGN_IO_ADDRESS(x)           ( (u32)(x) + RGN_IO_OFFSET )
//#define __io_address(n)       ((void __iomem __force *)RGN_IO_ADDRESS(n))

/* read register by byte */
#define rgn_readb(a) (*(volatile unsigned char *)RGN_IO_ADDRESS(a))

/* read register by word */
#define rgn_readw(a) (*(volatile unsigned short *)RGN_IO_ADDRESS(a))

/* read register by long */
#define rgn_readl(a) (*(volatile unsigned int *)RGN_IO_ADDRESS(a))

/* write register by byte */
#define rgn_writeb(v,a) (*(volatile unsigned char *)RGN_IO_ADDRESS(a) = (v))

/* write register by word */
#define rgn_writew(v,a) (*(volatile unsigned short *)RGN_IO_ADDRESS(a) = (v))

/* write register by long */
#define rgn_writel(v,a) (*(volatile unsigned int *)RGN_IO_ADDRESS(a) = (v))



#define READ_BYTE(x)         rgn_readb(x)
#define READ_WORD(x)         rgn_readw(x)
#define READ_LONG(x)         rgn_readl(x)
#define WRITE_BYTE(x, y)     rgn_writeb((u8)(y), x)
#define WRITE_WORD(x, y)     rgn_writew((u16)(y), x)
#define WRITE_LONG(x, y)     rgn_writel((u32)(y), x)

#define RIU_READ_BYTE(addr)         ( READ_BYTE( 0x1F000000+ (addr) ) )
#define RIU_READ_2BYTE(addr)        ( READ_WORD( 0x1F000000 + (addr) ) )
#define RIU_WRITE_BYTE(addr, val)    WRITE_BYTE( (0x1F000000 + (addr)), val)
#define RIU_WRITE_2BYTE(addr, val)   WRITE_WORD( 0x1F000000 + (addr), val)
#define RIU_WRITE_4BYTE(addr, val)   WRITE_LONG( 0x1F000000 + (addr), val)


#define R2BYTE( u32Reg ) RIU_READ_2BYTE( (u32Reg) << 1)

#define R2BYTEMSK( u32Reg, u16mask)\
    ( ( RIU_READ_2BYTE( (u32Reg)<< 1) & u16mask  ) )
#define R4BYTE( u32Reg )\
    ( { ((RIU_READ_2BYTE( (u32Reg) << 1)) | ((u32)(RIU_READ_2BYTE( ( (u32Reg) + 2 ) << 1) ) << 16)) ; } )


#define WBYTEMSK(u32Reg, u8Val, u8Mask) \
    RIU_WRITE_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)), ( RIU_READ_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)) ) & ~(u8Mask)) | ((u8Val) & (u8Mask)) )

#define W2BYTE( u32Reg, u16Val) RIU_WRITE_2BYTE( (u32Reg) << 1 , u16Val )
//#define W2BYTEMSK( u32Reg, u32ValandMask) RIU_WRITE_4BYTE( (u32Reg) << 1 , u32ValandMask )
#if RIU_32_EN
#define W2BYTEMSK( u32Reg, u16Val, u16Mask)\
    RIU_WRITE_4BYTE( (u32Reg)<< 1 , (u32)((u16Val) | ((u16Mask)<<16)) )
#else
#define W2BYTEMSK( u32Reg, u16Val, u16Mask)\
    RIU_WRITE_2BYTE( (u32Reg)<< 1 , (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) )
#endif
#define W4BYTE( u32Reg, u32Val)\
    ( { RIU_WRITE_2BYTE( (u32Reg) << 1, ((u32Val) & 0x0000FFFF) ); \
        RIU_WRITE_2BYTE( ( (u32Reg) + 2) << 1 , (((u32Val) >> 16) & 0x0000FFFF)) ; } )

#define W3BYTE( u32Reg, u32Val)\
    ( { RIU_WRITE_2BYTE( (u32Reg) << 1,  u32Val); \
        RIU_WRITE_BYTE( (u32Reg + 2) << 1 ,  ((u32Val) >> 16)); } )




#define RGN_BIT0                           0x00000001
#define RGN_BIT1                           0x00000002
#define RGN_BIT2                           0x00000004
#define RGN_BIT3                           0x00000008
#define RGN_BIT4                           0x00000010
#define RGN_BIT5                           0x00000020
#define RGN_BIT6                           0x00000040
#define RGN_BIT7                           0x00000080
#define RGN_BIT8                           0x00000100
#define RGN_BIT9                           0x00000200
#define RGN_BIT10                          0x00000400
#define RGN_BIT11                          0x00000800
#define RGN_BIT12                          0x00001000
#define RGN_BIT13                          0x00002000
#define RGN_BIT14                          0x00004000
#define RGN_BIT15                          0x00008000
#define RGN_BIT16                          0x00010000
#define RGN_BIT17                          0x00020000
#define RGN_BIT18                          0x00040000
#define RGN_BIT19                          0x00080000
#define RGN_BIT20                          0x00100000
#define RGN_BIT21                          0x00200000
#define RGN_BIT22                          0x00400000
#define RGN_BIT23                          0x00800000
#define RGN_BIT24                          0x01000000
#define RGN_BIT25                          0x02000000
#define RGN_BIT26                          0x04000000
#define RGN_BIT27                          0x08000000
#define RGN_BIT28                          0x10000000
#define RGN_BIT29                          0x20000000
#define RGN_BIT30                          0x40000000
#define RGN_BIT31                          0x80000000

#endif /* __HAL_GOP_UTIL_H__ */
