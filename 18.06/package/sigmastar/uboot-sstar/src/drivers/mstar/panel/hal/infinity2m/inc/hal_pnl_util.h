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

#ifndef __HAL_PNL_UTIL_H__
#define __HAL_PNL_UTIL_H__

extern u32 UTILITY_RIU_BASE_Vir;     // This should be inited before XC library starting.



#define RIU_READ_BYTE(addr)         ( READ_BYTE( 0x1F000000+ (addr) ) )
#define RIU_READ_2BYTE(addr)        ( READ_WORD( 0x1F000000 + (addr) ) )
#define RIU_READ_4BYTE(addr)        ( READ_LONG( 0x1F000000 + (addr) ) )

#define RIU_WRITE_BYTE(addr, val)    WRITE_BYTE( (0x1F000000 + (addr)), val)
#define RIU_WRITE_2BYTE(addr, val)   WRITE_WORD( 0x1F000000 + (addr), val)
#define RIU_WRITE_4BYTE(addr, val)   WRITE_LONG( 0x1F000000 + (addr), val)
#define BANKSIZE 512
#define RIU_GET_ADDR(addr)         ((void*)(UTILITY_RIU_BASE_Vir+ ((addr)<<1)) )

#define RBYTE( u32Reg ) RIU_READ_BYTE( (u32Reg) << 1)

#define R2BYTE( u32Reg ) RIU_READ_2BYTE( (u32Reg) << 1)

#define R2BYTEMSK( u32Reg, u16mask)\
    ( ( RIU_READ_2BYTE( (u32Reg)<< 1) & u16mask  ) )

#define R4BYTE( u32Reg )\
    ( {  (RIU_READ_4BYTE( (u32Reg)<<1)) ; } )

#define R4BYTEMSK( u32Reg, u32Mask) \
    ( RIU_READ_4BYTE( (u32Reg) << 1) & u32Mask  )

#define WBYTE(u32Reg, u8Val) RIU_WRITE_BYTE( ((u32Reg) << 1), u8Val )

#define WBYTEMSK(u32Reg, u8Val, u8Mask) \
               RIU_WRITE_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)), ( RIU_READ_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)) ) & ~(u8Mask)) | ((u8Val) & (u8Mask)) )

#define W2BYTE( u32Reg, u16Val) RIU_WRITE_2BYTE( (u32Reg) << 1 , u16Val )

#define W2BYTEMSK( u32Reg, u16Val, u16Mask)\
              RIU_WRITE_2BYTE( (u32Reg)<< 1 , (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) )

#if 0
#define W4BYTE( u32Reg, u32Val)\
            ( { RIU_WRITE_2BYTE( (u32Reg) << 1, ((u32Val) & 0x0000FFFF) ); \
                RIU_WRITE_2BYTE( ( (u32Reg) + 2) << 1 , (((u32Val) >> 16) & 0x0000FFFF)) ; } )
#else
#define W4BYTE( u32Reg, u32Val)\
            ( { RIU_WRITE_4BYTE( (u32Reg)<<1, u32Val);})
#endif

#define W4BYTEMSK( u32Reg, u32Val, u32Mask)\
                  RIU_WRITE_4BYTE( (u32Reg)<< 1 , (RIU_READ_4BYTE((u32Reg) << 1) & ~(u32Mask)) | ((u32Val) & (u32Mask)) )


#define W3BYTE( u32Reg, u32Val)\
            ( { RIU_WRITE_2BYTE( (u32Reg) << 1,  u32Val); \
                RIU_WRITE_BYTE( (u32Reg + 2) << 1 ,  ((u32Val) >> 16)); } )



#define WriteLongRegBit( u32Reg, bEnable, u32Mask)      \
    RIU_WRITE_4BYTE((u32Reg)<<1,  (bEnable) ? RIU_READ_4BYTE((u32Reg)<<1)| u32Mask :  \
    RIU_READ_4BYTE((u32Reg)<<1) & (~u32Mask));


#define MApi_XC_ReadByte(u32Reg)                    RBYTE(u32Reg)
#define MApi_XC_WriteByteMask(u32Reg, u8Val, u8Msk) WBYTEMSK( u32Reg, u8Val, u8Msk )
#define MApi_XC_WriteByte(u32Reg, u8Val)            WBYTE( u32Reg, u8Val )

#define MApi_XC_R2BYTE(u32Reg)                      R2BYTE( u32Reg )
#define MApi_XC_R2BYTEMSK(u32Reg, u16mask)          R2BYTEMSK( u32Reg, u16mask)

#define MApi_XC_W2BYTE(u32Reg, u16Val)              W2BYTE( u32Reg, u16Val)
#define MApi_XC_W2BYTEMSK(u32Reg, u16Val, u16mask)  W2BYTEMSK( u32Reg, u16Val, u16mask)


#define PNL_BIT0                           0x00000001
#define PNL_BIT1                           0x00000002
#define PNL_BIT2                           0x00000004
#define PNL_BIT3                           0x00000008
#define PNL_BIT4                           0x00000010
#define PNL_BIT5                           0x00000020
#define PNL_BIT6                           0x00000040
#define PNL_BIT7                           0x00000080
#define PNL_BIT8                           0x00000100
#define PNL_BIT9                           0x00000200
#define PNL_BIT10                          0x00000400
#define PNL_BIT11                          0x00000800
#define PNL_BIT12                          0x00001000
#define PNL_BIT13                          0x00002000
#define PNL_BIT14                          0x00004000
#define PNL_BIT15                          0x00008000
#define PNL_BIT16                          0x00010000
#define PNL_BIT17                          0x00020000
#define PNL_BIT18                          0x00040000
#define PNL_BIT19                          0x00080000
#define PNL_BIT20                          0x00100000
#define PNL_BIT21                          0x00200000
#define PNL_BIT22                          0x00400000
#define PNL_BIT23                          0x00800000
#define PNL_BIT24                          0x01000000
#define PNL_BIT25                          0x02000000
#define PNL_BIT26                          0x04000000
#define PNL_BIT27                          0x08000000
#define PNL_BIT28                          0x10000000
#define PNL_BIT29                          0x20000000
#define PNL_BIT30                          0x40000000
#define PNL_BIT31                          0x80000000



extern s8 UartSendTrace(const char *strFormat, ...);
#endif /* __HAL_PNL_UTIL_H__ */
