/*
* halAESDMA.c- Sigmastar
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

#include "ms_platform.h"

#ifndef _HAL_AESDMA_H_
#include "halAESDMA.h"
#include "mstar_chip.h"
#endif
#include <linux/sysctl.h>
#if 0
void HAL_AESDMA_DisableXIUSelectCA9(void)
{
    //disable AESDMA XIU select CA9
    RIU[(AESDMA_BASE_ADDR+(0x5F<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x5F<<1))])&(~AESDMA_CTRL_XIU_SEL_CA9));
}

void HAL_AESDMA_WB2DMADisable(void)
{
    // WB2DMA read & write disable
    RIU[(DMA_SECURE_BASE_ADDR+(0x79<<1))]= ((RIU[(DMA_SECURE_BASE_ADDR+(0x79<<1))])&(~(DMA_SECURE_CTRL_WB2DMA_R_EN|DMA_SECURE_CTRL_WB2DMA_W_EN)));
}

void HAL_AESDMA_ShaFromOutput(void)
{
    // SHA from AESDMA output(XIU)
    RIU[(AESDMA_BASE_ADDR+(0x5F<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x5F<<1))])|(AESDMA_CTRL_SHA_FROM_OUT));
}

void HAL_AESDMA_ShaFromInput(void)
{
    // SHA from AESDMA input(XIU)
    RIU[(AESDMA_BASE_ADDR+(0x5F<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x5F<<1))])|(AESDMA_CTRL_SHA_FROM_IN));
}
#endif
void HAL_AESDMA_INTMASK(void)
{
	RIU[(AESDMA_BASE_ADDR+(0x5e<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x5e<<1))]) | (1<<7));
}

void HAL_AESDMA_INTDISABLE(void)
{
	RIU[(AESDMA_BASE_ADDR+(0x5e<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x5e<<1))]) & ~(1<<7));
}


void HAL_AESDMA_SetXIULength(U32 u32Size)
{
    // AESDMA XIU length (byte):54~55
    RIU[(AESDMA_BASE_ADDR+(0x54<<1))]= (U16)((0x0000ffff)&(u32Size));
    RIU[(AESDMA_BASE_ADDR+(0x55<<1))]= (U16)(((0xffff0000)&(u32Size))>>16);
}

void HAL_AESDMA_UseHwKey(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x79<<1))]= (((RIU[(AESDMA_BASE_ADDR+(0x79<<1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_USE_HW_KEY);
}

void HAL_AESDMA_UseEfuseKey(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x79<<1))]= (((RIU[(AESDMA_BASE_ADDR+(0x79<<1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_USE_EFUSE_KEY);
}

void HAL_AESDMA_UseCipherKey(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x79<<1))]= (((RIU[(AESDMA_BASE_ADDR+(0x79<<1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_USE_CIPHER_KEY);
}

void HAL_AESDMA_CipherEncrypt(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x51<<1))]) & (~AESDMA_CTRL_CIPHER_DECRYPT));
}

void HAL_AESDMA_CipherDecrypt(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x51<<1))])|(AESDMA_CTRL_CIPHER_DECRYPT));
}

void HAL_AESDMA_Enable(U16 u16engine)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x51<<1))]) | u16engine);
}

void HAL_AESDMA_Disable(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x51<<1))])&(~AESDMA_CTRL_AES_EN));
}

void HAL_AESDMA_FileOutEnable(U8 u8FileOutEnable)
{
    // AESDMA fout_en
    if(u8FileOutEnable==1)
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))]) | AESDMA_CTRL_FOUT_EN);
        /*MCM*/
        //RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))]) | 0x1000);
        //RIU[(AESDMA_BASE_ADDR+(0x77<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x77<<1))]) | 0x003F);
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])&(~AESDMA_CTRL_FOUT_EN));
    }
}

void HAL_AESDMA_SetFileinAddr(U32 u32addr)
{
    u32addr = u32addr & (~0xF0000000); //mask bit to avoid MIU/IMI wrap

    //SHA_SetLength:5c~5d(sha_message_length)
    RIU[(AESDMA_BASE_ADDR+(0x52<<1))]= (U16)((0x0000ffff)&(u32addr));
    RIU[(AESDMA_BASE_ADDR+(0x53<<1))]= (U16)(((0xffff0000)&(u32addr))>>16);
}

void HAL_AESDMA_SetFileoutAddr(U32 u32addr, U32 u32Size)
{
    u32addr = u32addr & (~0xF0000000); //mask bit to avoid MIU/IMI wrap

    //SHA_SetLength:5c~5d(sha_message_length)
    RIU[(AESDMA_BASE_ADDR+(0x56<<1))]= (U16)((0x0000ffff)&(u32addr));
    RIU[(AESDMA_BASE_ADDR+(0x57<<1))]= (U16)(((0xffff0000)&(u32addr))>>16);

    RIU[(AESDMA_BASE_ADDR+(0x58<<1))]= (U16)((0x0000ffff)&(u32addr+u32Size-1));
    RIU[(AESDMA_BASE_ADDR+(0x59<<1))]= (U16)(((0xffff0000)&(u32addr+u32Size-1))>>16);
}

void HAL_AESDMA_SetCipherKey(U16 *pu16Key)
{
    int i;

    for(i=0;i<8;i++)
    {
        RIU[(AESDMA_BASE_ADDR+((0x67-i)<<1))] =((pu16Key[i] & 0x00FF)<<8)|((pu16Key[i] & 0xFF00)>>8);
    }
}

#define OFFSET_REG_ENG3_CTRL_S 0x77
#define KEY_LEN_AES128 0
#define KEY_LEN_AES192 0x00001000
#define KEY_LEN_AES256 0x00002000
#define KEY_LEN_MASK   0x00003000
#define BASE_RIU_ADDR_AESDMA (0x1F000000+AESDMA_BASE_ADDR*2)

void HAL_AESDMA_SetCipherKey2(U16 *pu16Key, U32 len)
{
    int i;

    if(len==16)
        OUTREGMSK16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), KEY_LEN_AES128, KEY_LEN_MASK );
    else if(len==24)
        OUTREGMSK16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), KEY_LEN_AES192, KEY_LEN_MASK );
    else if(len==32)
        OUTREGMSK16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), KEY_LEN_AES256, KEY_LEN_MASK );

    CLRREG16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), 0x800 );

    for(i=0;i<8;i++)
    {
        RIU[(AESDMA_BASE_ADDR+((0x67-i)<<1))] =((pu16Key[i] & 0x00FF)<<8)|((pu16Key[i] & 0xFF00)>>8);
    }

    if(len==24)
    {
        SETREG16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), 0x800 );
        for(i=0;i<4;i++)
        {
            RIU[(AESDMA_BASE_ADDR+((0x67-i)<<1))] =((pu16Key[i+8] & 0x00FF)<<8)|((pu16Key[i+8] & 0xFF00)>>8);
        }
    }
    else if(len==32)
    {
        SETREG16(BASE_RIU_ADDR_AESDMA+((OFFSET_REG_ENG3_CTRL_S)<<2), 0x800 );
        for(i=0;i<8;i++)
        {
            RIU[(AESDMA_BASE_ADDR+((0x67-i)<<1))] =((pu16Key[i+8] & 0x00FF)<<8)|((pu16Key[i+8] & 0xFF00)>>8);
        }
    }
}

void HAL_AESDMA_SetIV(U16 *pu16IV)
{
    int i;

    for(i=0;i<8;i++)
    {
        RIU[(AESDMA_BASE_ADDR+((0x6F-i)<<1))] = ((pu16IV[i] & 0x00FF)<<8)|((pu16IV[i] & 0xFF00)>>8);
    }
}

void HAL_AESDMA_SetChainModeECB(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))] = (RIU[(AESDMA_BASE_ADDR+(0x51<<1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_ECB;
}

void HAL_AESDMA_SetChainModeCTR(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))] = (RIU[(AESDMA_BASE_ADDR+(0x51<<1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_CTR;
}

void HAL_AESDMA_SetChainModeCBC(void)
{
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))] = (RIU[(AESDMA_BASE_ADDR+(0x51<<1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_CBC;
}

void HAL_AESDMA_Reset(void)
{
    int i;

    RIU[(AESDMA_BASE_ADDR+(0x50<<1))] = AESDMA_CTRL_SW_RST;
    RIU[(AESDMA_BASE_ADDR+(0x50<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x79<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x51<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x52<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x53<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x54<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x55<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x56<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x57<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x58<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x59<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(0x79<<1))] = (((RIU[(AESDMA_BASE_ADDR+(0x79<<1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_USE_CIPHER_KEY);

    for(i=0;i<8;i++)
    {
        RIU[(AESDMA_BASE_ADDR+((0x67-i)<<1))] = 0;
    }
    for(i=0;i<8;i++)
    {
        RIU[(AESDMA_BASE_ADDR+((0x6F-i)<<1))] = 0;
    }
}

void HAL_AESDMA_Start(U8 u8AESDMAStart)
{
    // AESDMA file start
    if(u8AESDMAStart==1)
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])|(AESDMA_CTRL_FILE_ST));
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])&(~AESDMA_CTRL_FILE_ST));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR+(0x50<<1))]= ((RIU[(AESDMA_BASE_ADDR+(0x50<<1))])&(~AESDMA_CTRL_FILE_ST));
    }
}

U16 HAL_AESDMA_GetStatus(void)
{
    return RIU[(AESDMA_BASE_ADDR+(0x7F<<1))];
}

void HAL_RSA_ClearInt(void)
{
    //RSA interrupt clear
    RIU[(RSA_BASE_ADDR+(0x27<<1))]= ((RIU[(RSA_BASE_ADDR+(0x27<<1))])|(RSA_INT_CLR));
}

void HAL_RSA_Reset(void)
{
    //RSA Rst
    RIU[(RSA_BASE_ADDR+(0x28<<1))]= ((RIU[(RSA_BASE_ADDR+(0x28<<1))])|(RSA_CTRL_RSA_RST));
    RIU[(RSA_BASE_ADDR+(0x28<<1))]= ((RIU[(RSA_BASE_ADDR+(0x28<<1))])&(~RSA_CTRL_RSA_RST));
}

void HAL_RSA_Ind32Ctrl(U8 u8dirction)
{
    //[1] reg_ind32_direction 0: Read. 1: Write
    if(u8dirction==1)
    {
        RIU[(RSA_BASE_ADDR+(0x21<<1))]= ((RIU[(RSA_BASE_ADDR+(0x21<<1))])|(RSA_IND32_CTRL_DIRECTION_WRITE));
    }
    else
    {
        RIU[(RSA_BASE_ADDR+(0x21<<1))]= ((RIU[(RSA_BASE_ADDR+(0x21<<1))])&(~RSA_IND32_CTRL_DIRECTION_WRITE));
    }
    //[2] reg_addr_auto_inc : Set 1 to enable address auto-increment after finishing read/write
    RIU[(RSA_BASE_ADDR+(0x21<<1))]= ((RIU[(RSA_BASE_ADDR+(0x21<<1))])|(RSA_IND32_CTRL_ADDR_AUTO_INC));

    //[3] Set 1 to enable access auto-start after writing Data[31:16]
    RIU[(RSA_BASE_ADDR+(0x21<<1))]= ((RIU[(RSA_BASE_ADDR+(0x21<<1))])|(RSA_IND32_CTRL_ACCESS_AUTO_START));
}

void HAL_RSA_LoadSignInverse(U32 *ptr_Sign, U8 u8Signlentgh)
{
        // RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_E+i))>>8)&0xFF00)|(((*(ptr_E+i))>>24)&0xFF);
       // RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_E+i))>>8)&0xFF)|(((*(ptr_E+i))<<8)&0xFF00);
    U32 i;
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
    U16 dummy;
#endif
	U8 lentgh = u8Signlentgh;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_A_BASE_ADDR; //RSA A addr
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    for( i = 0; i < lentgh ; i++ )
    {
        //RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_Sign+63-i))>>8)&0xFF00)|(((*(ptr_Sign+63-i))>>24)&0xFF);
        //RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_Sign+63-i))>>8)&0xFF)|(((*(ptr_Sign+63-i))<<8)&0xFF00);

//        RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_Sign + i))>>8)&0xFF00)|(((*(ptr_Sign + i))>>24)&0xFF);
//        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_Sign + i))>>8)&0xFF)|(((*(ptr_Sign + i))<<8)&0xFF00);

		RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_Sign + (lentgh-1) - i))>>8)&0xFF00)|(((*(ptr_Sign + (lentgh-1) - i))>>24)&0xFF);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
		dummy = RIU[(RSA_BASE_ADDR+(0x23<<1))];
#endif
		RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_Sign + (lentgh-1) - i))>>8)&0xFF)|(((*(ptr_Sign + (lentgh-1) - i))<<8)&0xFF00);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
        dummy = RIU[(RSA_BASE_ADDR+(0x24<<1))];
#endif
   }

    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop
}
/*
void HAL_RSA_LoadSignInverse_2byte(U16 *ptr_Sign)
{

    S32 i;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_A_BASE_ADDR; //RSA A addr
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    for( i = 127; i >= 0; i -= 2 )
    {
        RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)( ((*(ptr_Sign+i) << 8) & 0xFF00) | ((*(ptr_Sign+i) >> 8) & 0xFF) );
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)( ((*(ptr_Sign+i-1) << 8) & 0xFF00) | ((*(ptr_Sign+i-1) >> 8) & 0xFF) );
    }

    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop
}*/

void HAL_RSA_LoadKeyE(U32 *ptr_E, U8 u8Elentgh)
{
    U32 i;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_E_BASE_ADDR; //RSA E addr
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    //RIU[(POR_STATUS_BASE_ADDR+(0xA<<1))]=(U16)((0x0000ffff)&(U32)(ptr_E)); //write ptr_E addr to por_status(0x10050A)

    for( i = 0; i < u8Elentgh ; i++ )
    {
		RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_E + i))>>8)&0xFF00)|(((*(ptr_E  + i))>>24)&0xFF);
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_E + i))>>8)&0xFF)|(((*(ptr_E  + i))<<8)&0xFF00);
	}

    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop
}

void HAL_RSA_LoadKeyN(U32 *ptr_N ,U8 u8Nlentgh)
{
    U32 i;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_N_BASE_ADDR; //RSA N addr
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    //RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

    for( i = 0; i < u8Nlentgh; i++ )
    {
        RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_N+i))>>8)&0xFF00)|(((*(ptr_N+i))>>24)&0xFF);
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_N+i))>>8)&0xFF)|(((*(ptr_N+i))<<8)&0xFF00);
    }

    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop
}

void HAL_RSA_LoadKeyNInverse(U32 *ptr_N,U8 u8Nlentgh)
{
    U32 i;
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
    U16 dummy;
#endif
	U8 lentgh = u8Nlentgh;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_N_BASE_ADDR; //RSA N addr
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    //RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

    for( i = 0; i < lentgh; i++ )
    {
        RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_N + (lentgh-1) - i))>>8)&0xFF00)|(((*(ptr_N + (lentgh-1) - i))>>24)&0xFF);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
        dummy = RIU[(RSA_BASE_ADDR+(0x23<<1))];
#endif
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_N + (lentgh-1) - i))>>8)&0xFF)|(((*(ptr_N + (lentgh-1) - i))<<8)&0xFF00);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
        dummy = RIU[(RSA_BASE_ADDR+(0x24<<1))];
#endif
	}
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop
}

void HAL_RSA_LoadKeyEInverse(U32 *ptr_E,U8 u8Elentgh)
{
    U32 i;
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
    U16 dummy;
#endif
	U8 lentgh = u8Elentgh;

    RIU[(RSA_BASE_ADDR+(0x22<<1))]= RSA_E_BASE_ADDR;
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START)); //RSA start

    //RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

	for( i = 0; i < lentgh; i++ )
	{
		RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_E + (lentgh-1) - i))>>8)&0xFF00)|(((*(ptr_E + (lentgh-1) - i))>>24)&0xFF);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
		dummy = RIU[(RSA_BASE_ADDR+(0x23<<1))];
#endif
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_E + (lentgh-1) - i))>>8)&0xFF)|(((*(ptr_E + (lentgh-1) - i))<<8)&0xFF00);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
        dummy = RIU[(RSA_BASE_ADDR+(0x24<<1))];
#endif
	}
	RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START)); //RSA stop

}

void HAL_RSA_SetKeyLength(U16 u16keylen)
{
    //[13:8] n_len_e: key length, if hardware key set, this register is ignored and hardware internal using 3f
    RIU[(RSA_BASE_ADDR+(0x28<<1))] = ((RIU[(RSA_BASE_ADDR+(0x28<<1))])|(u16keylen<<8));
}

void HAL_RSA_SetKeyType(U8 u8hwkey, U8 u8pubkey)
{
    //[1] hw_key_e : 0 : software key, 1: hardware key
    //[2] e_pub_e : 0: pvivate key, 1: public key
    if(u8hwkey==1)
    {
        RIU[(RSA_BASE_ADDR+(0x28<<1))] = ((RIU[(RSA_BASE_ADDR+(0x28<<1))])|(RSA_SEL_HW_KEY));
    }
    else
    {
        RIU[(RSA_BASE_ADDR+(0x28<<1))] = ((RIU[(RSA_BASE_ADDR+(0x28<<1))])&(~RSA_SEL_HW_KEY));
    }

    if(u8pubkey==1)
    {
        RIU[(RSA_BASE_ADDR+(0x28<<1))] = ((RIU[(RSA_BASE_ADDR+(0x28<<1))])|(RSA_SEL_PUBLIC_KEY));
    }
    else
    {
        RIU[(RSA_BASE_ADDR+(0x28<<1))] = ((RIU[(RSA_BASE_ADDR+(0x28<<1))])&(~RSA_SEL_PUBLIC_KEY));
    }

}

void HAL_RSA_ExponetialStart(void)
{
    //RSA exp start
    RIU[(RSA_BASE_ADDR+(0x27<<1))] = ((RIU[(RSA_BASE_ADDR+(0x27<<1))])|(RSA_EXP_START));
}

U16 HAL_RSA_GetStatus(void)
{
    return RIU[(RSA_BASE_ADDR+(0x29<<1))];
}

void HAL_RSA_FileOutStart(void)
{
    //RSA ind32_start
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])|(RSA_IND32_START));
}

void HAL_RSA_FileOutEnd(void)
{
    //RSA ind32_start
    RIU[(RSA_BASE_ADDR+(0x20<<1))]= ((RIU[(RSA_BASE_ADDR+(0x20<<1))])&(~RSA_IND32_START));
}

void HAL_RSA_SetFileOutAddr(U32 u32offset)
{
    //RSA ind32_addr
    RIU[(RSA_BASE_ADDR+(0x22<<1))] = (U16)(RSA_Z_BASE_ADDR + u32offset);
}

U32 HAL_RSA_FileOut(void)
{
	U32 output;
	U32 dummy;
	//output = (U16)(((RIU[(RSA_BASE_ADDR+(0x26<<1))] >>8 )& 0xff )|((RIU[(RSA_BASE_ADDR+(0x26<<1))] << 8 )& 0xff00 )) ;
	//output = output | ((((RIU[(RSA_BASE_ADDR+(0x25<<1))]>>8)& 0xff)|((RIU[(RSA_BASE_ADDR+(0x25<<1))]<<8)& 0xff00)) << 16);
    dummy = RIU[(RSA_BASE_ADDR+(0x26<<1))];
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
    dummy = RIU[(RSA_BASE_ADDR+(0x26<<1))];
#endif
    output = (U16)(((dummy >> 8) & 0xFF) | ((dummy << 8) & 0xFF00));

    dummy = RIU[(RSA_BASE_ADDR+(0x25<<1))];
#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
    dummy = RIU[(RSA_BASE_ADDR+(0x25<<1))];
#endif
    output = output | ((((dummy >> 8) & 0xFF) | ((dummy << 8) & 0xFF00)) << 16);

    return output;
}

void HAL_SHA_Reset(void)
{
    //SHA_Reset
    RIU[SHARNG_BASE_ADDR+(0x08<<1)]= (RIU[SHARNG_BASE_ADDR+(0x08<<1)]|(SHARNG_CTRL_SHA_RST));
    RIU[SHARNG_BASE_ADDR+(0x08<<1)]= (RIU[SHARNG_BASE_ADDR+(0x08<<1)]&(~SHARNG_CTRL_SHA_RST));

    //Clear reg_dma4_ctrl_s
    RIU[SHARNG_BASE_ADDR+(0x5d<<1)]= 0;
}

void HAL_SHA_SetAddress(U32 u32Address)
{
    /**/
   // u32Address = u32Address & (~0xF0000000); //mask bit to avoid MIU/IMI wrap
    if (u32Address < ARM_MIU1_BASE_ADDR) {
        RIU[(SHARNG_BASE_ADDR+(0x0E<<1))]= 0x80;
    }
    else {
        u32Address -= ARM_MIU1_BASE_ADDR;
        RIU[(SHARNG_BASE_ADDR+(0x0E<<1))]= 0x20;
    }

    //SHA_SetLength:5c~5d(sha_message_length)
    RIU[(SHARNG_BASE_ADDR+(0x0A<<1))]= (U16)((0x0000ffff)&(u32Address));
    RIU[(SHARNG_BASE_ADDR+(0x0B<<1))]= (U16)(((0xffff0000)&(u32Address))>>16);

    // Bypass scatter & gather address
    RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(1<<11));
}

void HAL_SHA_SetLength(U32 u32Size)
{
    //SHA_SetLength:5c~5d(sha_message_length)
    RIU[(SHARNG_BASE_ADDR+(0x0C<<1))]= (U16)((0x0000ffff)&(u32Size));
    RIU[(SHARNG_BASE_ADDR+(0x0D<<1))]= (U16)(((0xffff0000)&(u32Size))>>16);
}

void HAL_SHA_SelMode(U8 u8sha256)
{
    //SHA_SelMode:58~59(sha_ctrl & sha_scattergather_size)
    if(u8sha256==1)
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(SHARNG_CTRL_SHA_SEL_SHA256));
    }
    else
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])&(~SHARNG_CTRL_SHA_SEL_SHA256));
    }
}

void HAL_SHA_ManualMode(U8 bManualMode)
{
    if(bManualMode==1)
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(SHARNG_CTRL_WORK_MODE_MANUAL));
    }
    else
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])&(~SHARNG_CTRL_WORK_MODE_MANUAL));
    }
}

U16 HAL_SHA_GetStatus(void)
{
    return (RIU[(SHARNG_BASE_ADDR+(0x0F<<1))]);
}

void HAL_SHA_Clear(void)
{
    //Set "1" to idle state after reg_read_sha_ready = 1
    RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])&(~SHARNG_CTRL_SHA_FIRE_ONCE));
    RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])&(~SHARNG_CTRL_SHA_CLR));
    RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(SHARNG_CTRL_SHA_CLR));

    RIU[(SHARNG_BASE_ADDR+(0x0F<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x0F<<1))])&(~SHARNG_CTRL_SHA_BUSY));
}

void HAL_SHA_Start(void)
{
    RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(SHARNG_CTRL_SHA_FIRE_ONCE));
}

void HAL_SHA_Out(U32 u32Buf)
{
    U32 index;

//	output = (U16)(((RIU[(RSA_BASE_ADDR+(0x26<<1))] >>8 )& 0xff )|((RIU[(RSA_BASE_ADDR+(0x26<<1))] << 8 )& 0xff00 )) ;
//	output = output | ((((RIU[(RSA_BASE_ADDR+(0x25<<1))]>>8)& 0xff)|((RIU[(RSA_BASE_ADDR+(0x25<<1))]<<8)& 0xff00)) << 16);

    //SHA_Out
    for( index = 0; index < 16; index++ )
    {
        *((U16 *)u32Buf +(15-index)) = (U16)(RIU[(SHARNG_BASE_ADDR + (0x10<<1) + index*2)]>>8&0xff)|(U16)(RIU[(SHARNG_BASE_ADDR + (0x10<<1) + index*2)]<<8&0xff00);
    }
}
void HAL_SHA_Write_InitValue_BE(U32 u32Buf)
{
    U32 index;

    for( index = 0; index < 16; index++ )
    {
         RIU[(SHARNG_BASE_ADDR+(0x10<<1)+index*2)] = __cpu_to_be16(*((U16 *)u32Buf + (15-index)));
    }
}

void HAL_SHA_ReadOut(U32 u32Buf)
{
    U32 index;

    //SHA_Out
    for( index = 0; index < 16; index++ )
    {
        *((U16 *)u32Buf + index) = RIU[(SHARNG_BASE_ADDR+(0x10<<1)+index*2)];
#ifdef CONFIG_SRAM_DUMMY_ACCESS_SHA
		*((U16 *)u32Buf + index) = RIU[(SHARNG_BASE_ADDR+(0x10<<1)+index*2)];
#endif
    }
}

void HAL_SHA_SetInitHashMode(U8 uMode)
{
    if(uMode)
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])|(SHARNG_CTRL_SHA_INIT_HASH));
    }
    else
    {
        RIU[(SHARNG_BASE_ADDR+(0x08<<1))]= ((RIU[(SHARNG_BASE_ADDR+(0x08<<1))])&(~SHARNG_CTRL_SHA_INIT_HASH));
    }
}

void HAL_SHA_Write_InitValue(U32 u32Buf)
{
    U32 index;
#ifdef CONFIG_SRAM_DUMMY_ACCESS_SHA
    U16 dummy;
#endif

    //SHA_Out
    for( index = 0; index < 16; index++ )
    {
        RIU[(SHARNG_BASE_ADDR+(0x10<<1)+index*2)] = *((U16 *)u32Buf + index);
#ifdef CONFIG_SRAM_DUMMY_ACCESS_SHA
		dummy = RIU[(SHARNG_BASE_ADDR+(0x10<<1)+index*2)];
#endif
    }
}

U32 HAL_SHA_ReadWordCnt(void)
{
    return RIU[(SHARNG_BASE_ADDR+(0x2E<<1))] | (RIU[(SHARNG_BASE_ADDR+(0x2F<<1))] << 16);
}

void HAL_SHA_WriteWordCnt(U32 u32Val)
{
    RIU[(SHARNG_BASE_ADDR+(0x2E<<1))] = (U16)(u32Val & 0xFFFF);
    RIU[(SHARNG_BASE_ADDR+(0x2F<<1))] = (U16)(u32Val >> 16);
}
void HAL_MCM(U32 u32Val)
{
    RIU[(0x113200+(0x01<<1))]= (U16)u32Val;
}

U16 HAL_RNG_Read(void)
{
    u8 try_count = 20;
    u16 rng = 0xFFFF;

    if (((RIU[(RSA_BASE_ADDR+(0x00<<1))]) & 0x80) == 0)
    {
        RIU[(RSA_BASE_ADDR+(0x00<<1))] = ((RIU[(RSA_BASE_ADDR+(0x00<<1))])|(0x80));
    }
    do {
        if ((RIU[(RSA_BASE_ADDR+(0x03<<1))]) & 0x1)
        {
            rng = RIU[(RSA_BASE_ADDR+(0x02<<1))];
            break;
        }
    } while(try_count--);

    if (rng == 0xFFFF)
    {
        rng = RIU[(RSA_BASE_ADDR+(0x02<<1))];
    }

    return rng;
}

