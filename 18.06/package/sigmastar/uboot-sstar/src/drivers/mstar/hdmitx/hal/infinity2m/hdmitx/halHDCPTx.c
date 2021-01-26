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

#include "mhal_common.h"
#include "halHDMIUtilTx.h"
#include "regHDMITx.h"
#include "halHDCPTx.h"
#include "drv_hdmitx_os.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define HDCP_KEY_OFFSET         8U

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

MS_BOOL bDebugHDCPFlag = FALSE;
MS_U8 HDCP_AKSV[5] = {0, 0, 0, 0, 0,};
MS_U8 HDCP_BKSV[5] = {0, 0, 0, 0, 0,};

MS_U8 HDCP_KEY[304]=
{
    0x47,0x7a,0x25,0x2f,0x1a,0xc1,0x54,0x00,0x93,0x23,0xbe,0x15,0x32,0x62,0x48,0xed,
    0x8c,0xb8,0x80,0x61,0x06,0xc1,0x93,0x15,0xab,0xd9,0x3e,0x0f,0x5b,0xb5,0x76,0xcc,
    0x25,0x01,0xbb,0x51,0xbc,0xbd,0x75,0x71,0x04,0x44,0x82,0xed,0x52,0xc6,0x68,0x11,
    0x5f,0x24,0xff,0x48,0xac,0x6b,0x79,0xcb,0xf1,0xe8,0x2c,0xc4,0x3a,0x43,0xf0,0x27,
    0x1d,0x15,0x9b,0x4e,0xa2,0x65,0x08,0x8d,0x69,0x69,0x35,0x1b,0x48,0xf0,0x79,0x57,
    0x02,0xe5,0x69,0x58,0xa3,0x3e,0x0e,0x31,0xfe,0x8d,0x70,0x3e,0x30,0x21,0xdc,0x48,
    0x9b,0x08,0xb5,0x2f,0x58,0xd2,0x21,0x67,0xe4,0x67,0x13,0x02,0x92,0x49,0x66,0x6f,
    0xb6,0xe9,0x25,0x8c,0xa0,0x1f,0xd5,0xeb,0x25,0xbc,0xc6,0xa7,0x89,0x1a,0x33,0x8d,
    0x50,0xf7,0x1d,0x88,0x18,0x7c,0xd3,0x26,0x80,0xa7,0x4c,0x40,0xae,0xb9,0x99,0x8a,
    0x97,0x11,0x60,0xb5,0xbc,0x4a,0xce,0x14,0xa5,0x2d,0x86,0x73,0xd6,0xf2,0x45,0x19,
    0x05,0xa3,0x81,0xa5,0xf3,0x8b,0x09,0xce,0x75,0x66,0xbe,0x17,0x04,0xf2,0x76,0xd9,
    0x3e,0x53,0xbf,0x02,0x91,0xf7,0xbe,0x3b,0x05,0xfc,0x38,0xd2,0x80,0x7b,0x5a,0x0c,
    0xb0,0xe8,0xb1,0x56,0x39,0xac,0x16,0x42,0xc2,0x1c,0x57,0x10,0xbd,0xd3,0x98,0x36,
    0xd0,0x50,0x98,0x2d,0xbb,0x0b,0xd6,0x51,0xf0,0x31,0x0c,0x4f,0xfc,0x0c,0xb2,0xf6,
    0x81,0x54,0xd4,0x39,0x0d,0xad,0xb8,0x73,0xf0,0x2d,0x47,0x5b,0xa0,0x01,0xe9,0x7f,
    0x96,0xeb,0xbe,0xba,0xa2,0x82,0xee,0x3a,0x0b,0x11,0xee,0x39,0x93,0xab,0x91,0x63,
    0xa9,0x25,0x63,0x24,0xcc,0x1f,0x80,0x45,0x56,0xbd,0xb3,0x82,0x7f,0xb3,0x7e,0xd9,
    0xd1,0xcd,0xd7,0x89,0x0c,0x36,0x88,0x52,0x35,0xbf,0x6c,0xed,0xb3,0xca,0x9c,0x79,
    0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

MSTHDMITX_REG_TYPE TxHdcpInitTbl[] =
{
    {HDMITX_HDCP_REG_BASE, REG_HDCP_TX_MODE_01, 0x8000, 0x8000}, // Enable HDCP encryption
    {HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x001C, 0x0000}, //[4]: 1: km new mode; 0: km old mode
};

MSTHDMITX_REG_TYPE TxHdcpEncryptOnTbl[] =
{
    {HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x0008, 0x0008},
};

MSTHDMITX_REG_TYPE TxHdcpEncryptOffTbl[] =
{
    {HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x0008, 0x0000},
};

MS_U8 TxHdcpAnTbl[] =
{
    0x03, 0x04, 0x07, 0x0C, 0x13, 0x1C, 0x27, 0x34,
};

MS_U32 sm_K160[4] =
{
    0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
};

MS_U32 sm_H160[SHA160LENGTH] =
{
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
};


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

extern MS_BOOL MDrv_EEPROM_Read(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size);

void Bytes2Word(MS_U8 const * pcBytes, MS_U32  * ruiWord)
{
    *ruiWord = ((MS_U32) pcBytes[3])
             | (((MS_U32) pcBytes[2]) << 8)
             | (((MS_U32) pcBytes[1]) << 16)
             | (((MS_U32) pcBytes[0]) << 24);
}

//#define   Bytes2Word(pcBytes) (((MS_U32)pcBytes[3]) | (((MS_U32)pcBytes[2])<<8) |       (( (MS_U32)pcBytes[1] )<<16) | (( (MS_U32)pcBytes[0] )<<24))

MS_U32 CircularShift(MS_U32 uiBits, MS_U32 uiWord)
{
    return (uiWord << uiBits) | (uiWord >> (32 - uiBits));
}

void Word2Bytes1(MS_U32 const * ruiWord, MS_U8* pcBytes)
{
	pcBytes += 3;
	*pcBytes = *ruiWord & 0xff;
	*--pcBytes = (*ruiWord >> 8) & 0xff;
	*--pcBytes = (*ruiWord >> 16) & 0xff;
	*--pcBytes = (*ruiWord >> 24) & 0xff;
}

void Word2Bytes(MS_U32 const * ruiWord, MS_U8* pcBytes)
{
    pcBytes += 3;
    *pcBytes = (*ruiWord>>24) & 0xff;
    *--pcBytes = (*ruiWord >> 16) & 0xff;
    *--pcBytes = (*ruiWord >> 8) & 0xff;
    *--pcBytes = (*ruiWord) & 0xff;
}


void MHal_HDMITx_HdcpDebugEnable(MS_BOOL benable)
{
    bDebugHDCPFlag = benable;
}

void MHal_HDMITx_GetHdcpKey(MS_BOOL useinternalkey, MS_U8 *data)
{
    if(!useinternalkey)
    {
        memcpy(&HDCP_KEY[0], data, 304*sizeof(MS_U8));
    	{
            MS_U16 i;

            if(bDebugHDCPFlag)
            {
                printf("HDCP key = \n");
                for(i=0;i<304;i++)
                {
                    printf("0x%x, ", HDCP_KEY[i]);
                    if ((i - ((i>>4)<<4) == 0) && (i != 0x00))//(i%16 == 15)
                        printf("\n");
                }
            }
        }
    }
}

void MHal_HDMITx_HdcpKeyInit(MS_BOOL useinternalkey)
{
    if(!useinternalkey)
    {
        MDrv_EEPROM_Read(0x00, HDCP_KEY, 304);
    #if 1
    	{
            MS_U16 i;

            if(bDebugHDCPFlag)
            {
                printf("HDCP key = \n");
                for(i=0;i<304;i++)
                {
                    printf("0x%x, ", HDCP_KEY[i]);
                    if ((i - ((i>>4)<<4) == 0) && (i != 0x00))//(i%16 == 15)
                        printf("\n");
                }
            }

	}
    #endif
    }
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpInit
//  [Description]:
//      This routine is the initialization for HDCP module.
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      None
//
//*******************************************************************************
void MHal_HDMITx_HdcpInit(void)
{
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpRxActive
//  [Description]:
//      This routine check the HDCP Rx active or not
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      FALSE:      no HDCP Rx
//      TRUE:       HDCP Rx active
//
//*******************************************************************************
MS_BOOL MHal_HDMITx_HdcpRxActive(void)
{
    MS_U8 regval;

    if (MHal_HDMITx_Rx74ReadByte(0x40, &regval) == TRUE)
    {
        if(regval != 0xFF)
            return TRUE;
    }
    return FALSE;
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpSetEncrypt
//  [Description]:
//      This routine set encryption for HDCP module.
//  [Arguments]:
//      arg1:   tx index
//      arg2:   0:disable 1:enable
//  [Return]:
//      None
//
//*******************************************************************************
void MHal_HDMITx_HdcpSetEncrypt(MS_BOOL enable)
{
    switch (enable)
    {
        case 0:
            //MHal_HDMITx_RegsTbl_Write(TxHdcpEncryptOffTbl, 1);
            break;
        case 1:
            //MHal_HDMITx_RegsTbl_Write(TxHdcpEncryptOnTbl, 1);
            break;
        default:
            break;
    }
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpCheckBksvLn
//  [Description]:
//      This routine check the HDCP Rx's BKSV and calculate the Key
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      TRUE:   pass
//      FALSE:  fail
//
//*******************************************************************************
MS_BOOL MHal_HDMITx_HdcpCheckBksvLn(void)
{
    MS_U8 bksv[5] = {0}, Lm[7] = {0};
    MS_U8 temp, carry_bit, seed;
    MS_U8 i, j, k, t;
    MS_U16 offset;

    if(bDebugHDCPFlag)
        printf("TxHdcpCheckBksvLn()\r\n");
    // Check BKSV 20 bit1 20 bit0
    k = 0;
    for (t = 0; t < 10; t++)
    {
        for (i = 0; i < 5; i++)
        {
            if(MHal_HDMITx_Rx74ReadByte(i, &bksv[i]) == FALSE)
            {
                if(bDebugHDCPFlag)
                    printf("Reading BKSV failed because I2C\r\n");
                return FALSE;
            }
            for (j = 0; j < 8; j++)
                k += (bksv[i] >> j) & 0x01;

			HDCP_BKSV[i] = bksv[i];
        }
        if (k == 20)
            break;
        DrvHdmitxOsMsSleep(20);
        k=0;
    }

    if(bDebugHDCPFlag)
    {
        printf("Bksv=");
        for (i = 0; i < 5; i++)
            printf("0x%x, ", bksv[i]);
        printf("\r\n");
    }

    if (k != 20)
        return FALSE;
    for (i = 0; i < 7; i++)
        Lm[i] = 0;

#if 0
    for(i=0;i<18;i++)
    {
        for(j=0;j<16;j++)
        {
            if(HDCP_KEY[(i<<4)+j]!=MHal_HDMITx_Rx74ReadByte((i<<4)+j))
            DBG_HDCP(printf("\r\nerror address=%x\r\n,",(i<<4)+j));
        }
    }
#endif

    // Generate cipher infomation from Key & BKSV
    for (i = 0; i < 5; i++)
        for (j = 0; j < 8; j++)
        {
            //if(RxBksvTbl[4-i] & (1<<j))
            if (bksv[i] & (1 << j))
            {
                carry_bit = 0;
                offset = (i * 8 + j) * 7 + HDCP_KEY_OFFSET;
                for (k = 0; k < 7; k++)
                {
                    seed = (k + HDCP_KEY[7]) % 7;
                    temp = HDCP_KEY[offset + k] ^ HDCP_KEY[seed];
                    Lm[k] = Lm[k] + temp + carry_bit;
                    if (((carry_bit == 0) && (Lm[k] >= temp)) || ((carry_bit == 1) && (Lm[k] > temp)))
                        carry_bit = 0;
                    else
                        carry_bit = 1;
                }
            }
        }
    temp = HDCP_KEY[288];

    if(bDebugHDCPFlag)
        printf("Km= ");
    for (i = 0; i < 7; i++)
    {
        if (i < 6)
            Lm[i] = Lm[i] ^ temp;
        else
            Lm[i] = Lm[i] ^ (~temp);

        if(i%2)
            MHal_HDMITx_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_LN_04+(i/2), (Lm[i]<<8) | Lm[i-1]);
        if(bDebugHDCPFlag)
            printf("0x%x, ", Lm[i]);
    }
    if(bDebugHDCPFlag)
        printf("\n");
    MHal_HDMITx_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_LN_SEED_07, (temp<<8) | Lm[6]);
    if(bDebugHDCPFlag)
        printf("Seed=0x%x\r\n", temp);

    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpWriteAn
//  [Description]:
//      This routine Write An to the HDCP Rx
//  [Arguments]:
//      arg1:   tx index
//      arg2:   use internal An or not
//  [Return]:
//      None
//
//*******************************************************************************
MS_BOOL MHal_HDMITx_HdcpWriteAn(MS_U8 internal)
{
    MS_U16 temp;
    MS_U8 i;

    if (internal == 1)
    {
        MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_MODE_01, 0x0100, 0x0100);
        MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, BIT1, BIT1);
        MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, BIT1, 0);

        //Waiting for HW An Auto Gen Ready
        DrvHdmitxOsUsSleep(1);

        if(bDebugHDCPFlag)
            printf("Internal An=");
        for (i = 0; i < 4; i++)
        {
            temp = MHal_HDMITx_Read(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_AN_08+i);
            if(MHal_HDMITx_Rx74WriteByte(0x18 + (i*2), (MS_U8)(temp & 0x00FF)) == FALSE)
            {
                if(bDebugHDCPFlag)
                    printf("Writing An failed because I2C\r\n");
                return FALSE;
            }
            if(MHal_HDMITx_Rx74WriteByte(0x18 + (i*2+1), (MS_U8)((temp & 0xFF00)>>8)) == FALSE)
            {
                if(bDebugHDCPFlag)
                    printf("Writing An failed because I2C\r\n");
                return FALSE;
            }
            if(bDebugHDCPFlag)
            {
                printf("0x%x, ", temp & 0x00FF);
                printf("0x%x, ", (temp & 0xFF00)>>8);
            }
        }
        if(bDebugHDCPFlag)
            printf("\r\n");
    }
    else
    {
        MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_MODE_01, 0x0100, 0x0000);
        for(i = 0; i < 4; i++)
            MHal_HDMITx_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_AN_08+i, (TxHdcpAnTbl[2*i+1] <<8) | (TxHdcpAnTbl[2*i]));
        if(MHal_HDMITx_Rx74WriteBytes(0x18, 8, TxHdcpAnTbl) == FALSE)
        {
            if(bDebugHDCPFlag)
                printf("Writing An failed because I2C\r\n");
            return FALSE;
        }
        if(bDebugHDCPFlag)
        {
             printf("External An=");
             for (i = 0; i < 8; i++)
                 printf("0x%x, ", TxHdcpAnTbl[i]);
             printf("\r\n");
        }
    }
    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpWriteAksv
//  [Description]:
//      This routine Write AKSV to the HDCP Rx
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      None
//
//*******************************************************************************
MS_BOOL MHal_HDMITx_HdcpWriteAksv(void)
{
    MS_U8 temp;
    MS_U8 i, k, j;

    if(bDebugHDCPFlag)
        printf("Aksv=");
    for (i = 0, k=0; i < 5; i++)
    {
        //temp = TxHdcpKeyTbl[i];
        temp = HDCP_KEY[i];
        HDCP_AKSV[i] = temp;
        if(MHal_HDMITx_Rx74WriteByte(0x10 + i, temp) == FALSE)
        {
            if(bDebugHDCPFlag)
                printf("Writing Aksv failed because I2C\r\n");
            return FALSE;
        }
        for (j = 0; j < 8; j++)
            k += (temp >> j) & 0x01; // check AKSV whether is 20 zeros and 20 ones or not.
        if(bDebugHDCPFlag)
            printf("0x%x, ", temp);
    }
    if(bDebugHDCPFlag)
        printf("\r\n");
    if(k!=20)	return FALSE;
    return TRUE;
}


//******************************************************************************
//
//  [Function Name]:
//      MHal_HDMITx_GET74
//  [Description]:
//      This routine get data from 74reg
//  [Arguments]:
//
//  [Return]:
//      None
//
//*******************************************************************************
void MHal_HDMITx_GET74(MS_U8 udata, MS_U8 *pdata)
{
   if(udata == 0x00)
       memcpy(pdata, &HDCP_BKSV, 5*sizeof(MS_U8));
   else if(udata == 0x10)
       memcpy(pdata, &HDCP_AKSV, 5*sizeof(MS_U8));
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpStartCipher
//  [Description]:
//      This routine Start HDCP Tx Cipher
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      None
//
//*******************************************************************************
void MHal_HDMITx_HdcpStartCipher(void)
{
    MS_U8 count = 0xFF;

    MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x000F, 0);
    MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x000F, BIT0);
    MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x000F, 0);

    while (count-- && !(MHal_HDMITx_Read(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02) & BIT8));
    if (count == 0)
    {
        if(bDebugHDCPFlag)
            printf("Cipher not done!!!\r\n");
    }
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpAuthPass
//  [Description]:
//      This routine finish the authentication
//  [Arguments]:
//      arg1:   tx index
//  [Return]:
//      None
//
//*******************************************************************************
void MHal_HDMITx_HdcpAuthPass(void)
{
    MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_COMMAND_02, 0x000F, 0x000C);
}


void MHal_HDMITx_HdcpSha1Init(SHA1_DATA *dig)
{
    MS_U8 i;

    for (i = 0; i < SHA160LENGTH; i++)
        dig->m_auiBuf[i] = sm_H160[i];
    dig->m_auiBits[0] = 0;
    dig->m_auiBits[1] = 0;
    memset(dig->m_aucIn, 0, BLOCKSIZE) ;  // 2006/1/18 3:31PM by Emily
}


//******************************************************************************
//
//  [Function Name]:
//      TxHdcpCheckRi
//  [Description]:
//      This routine check the hdcp Ri values
//  [Arguments]:
//      arg1:   tx index
//      arg2:   output Tx Ri value
//      arg3:   output Rx Ri value
//  [Return]:
//      None
//
//*******************************************************************************
MS_BOOL MHal_HDMITx_HdcpCheckRi(MS_U16 *ptx_ri, MS_U16 *prx_ri)
{
    *ptx_ri = MHal_HDMITx_Read(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_RI_00);
    if(MHal_HDMITx_Rx74ReadBytes(0x08, 2, (MS_U8 *)prx_ri) == TRUE)
    {
        if (*prx_ri == *ptx_ri)
            return TRUE;
    }
    if(MHal_HDMITx_Rx74ReadBytes(0x08, 2, (MS_U8 *)prx_ri) == TRUE)
    {
        if (*prx_ri == *ptx_ri)
            return TRUE;
    }
    *ptx_ri = MHal_HDMITx_Read(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_RI_00);
    if (*prx_ri == *ptx_ri)
        return TRUE;
    else
        return FALSE;
}


void _MHal_HDMITx_HdcpSha1Transform(SHA1_DATA *dig)
{
    MS_U8 * pucIn = dig->m_aucIn;
    MS_U32 auiW[80];
    MS_U8 i;
    MS_U32 temp;
    MS_U32 A, B, C, D, E;

    for (i = 0; i < 16; i++,pucIn += 4)
        auiW[i] = ((MS_U32)pucIn[3]) | ((MS_U32)pucIn[2] << 8) |
                  ((MS_U32)pucIn[1] << 16) | ((MS_U32)pucIn[0] << 24);
    for (i = 16; i < 80; i++)
        auiW[i] = ((auiW[i - 3] ^ auiW[i - 8] ^ auiW[i - 14] ^ auiW[i - 16]) << 1) |
                  ((auiW[i - 3] ^ auiW[i - 8] ^ auiW[i - 14] ^ auiW[i - 16]) >> 31);

    A = dig->m_auiBuf[0];
    B = dig->m_auiBuf[1];
    C = dig->m_auiBuf[2];
    D = dig->m_auiBuf[3];
    E = dig->m_auiBuf[4];

    for (i = 0; i < 20; i++)
    {
        temp = ((A << 5) | (A >> 27)) + ((B & C) | ((~B) & D)) + E + auiW[i] + sm_K160[0];
        E = D;
        D = C;
		C = (B << 30) | (B >> 2);
        B = A;
        A = temp;
    }
    for (i = 20; i < 40; i++)
    {
        temp = ((A << 5) | (A >> 27)) + (B ^ C ^ D) + E + auiW[i] + sm_K160[1];
        E = D;
        D = C;
		C = (B << 30) | (B >> 2);
        B = A;
        A = temp;
    }
    for (i = 40; i < 60; i++)
    {
        temp = ((A << 5) | (A >> 27)) + ((B & C) | (B & D) | (C & D)) + E + auiW[i] + sm_K160[2];
        E = D;
        D = C;
		C = (B << 30) | (B >> 2);
        B = A;
        A = temp;
    }
    for (i = 60; i < 80; i++)
    {
        temp = ((A << 5) | (A >> 27)) + (B ^ C ^ D) + E + auiW[i] + sm_K160[3];
        E = D;
        D = C;
		C = (B << 30) | (B >> 2);
        B = A;
        A = temp;
    }
    dig->m_auiBuf[0] += A;
    dig->m_auiBuf[1] += B;
    dig->m_auiBuf[2] += C;
    dig->m_auiBuf[3] += D;
    dig->m_auiBuf[4] += E;
}


void MHal_HDMITx_HdcpSha1AddData(SHA1_DATA *dig, MS_U8 const * pcData, MS_U16  iDataLength)
{
    MS_U32  uiT;

    //Update bitcount
    uiT = dig->m_auiBits[0];
    if ((dig->m_auiBits[0] = uiT + ((MS_U32) iDataLength << 3)) < uiT)
        dig->m_auiBits[1]++; //Carry from low to high
    //dig->m_auiBits[1] += ((MS_U32) iDataLength) >> 29;  //Fix coverity impact : iDataLength >> 29 is 0
    uiT = (uiT >> 3) & (BLOCKSIZE - 1); //Bytes already
    //Handle any leading odd-sized chunks
    if (uiT != 0)
    {
        MS_U8 * puc = (MS_U8 *) dig->m_aucIn + uiT;
        uiT = BLOCKSIZE - uiT;
        if (iDataLength < uiT)
        {
            memcpy(puc, pcData, iDataLength);
            return;
        }
        memcpy(puc, pcData, uiT);
        _MHal_HDMITx_HdcpSha1Transform(dig);
        pcData += uiT;
        iDataLength -= uiT;
    }
    //Process data in 64-byte chunks
    while (iDataLength >= BLOCKSIZE)
    {
        memcpy(dig->m_aucIn, pcData, BLOCKSIZE);
        _MHal_HDMITx_HdcpSha1Transform(dig);
        pcData += BLOCKSIZE;
        iDataLength -= BLOCKSIZE;
    }
    //Handle any remaining bytes of data
    memcpy(dig->m_aucIn, pcData, iDataLength);
}


void MHal_HDMITx_HdcpSha1FinalDigest(SHA1_DATA *dig, MS_U8* pcDigest)
{
    MS_U16  uiCount;
    MS_U8   *puc;
    MS_U8   i;

    //Compute number of bytes mod 64
    uiCount = (dig->m_auiBits[0] >> 3) & (BLOCKSIZE - 1);
    //Set the first char of padding to 0x80. This is safe since there is
    //always at least one byte free
    puc = dig->m_aucIn + uiCount;
    *puc++ = 0x80;
    //Bytes of padding needed to make 64 bytes
    uiCount = BLOCKSIZE - uiCount - 1;

    //Pad out to 56 mod 64
    if (uiCount < 8)
    {
        //Two lots of padding: Pad the first block to 64 bytes
        memset(puc, 0, uiCount);
        _MHal_HDMITx_HdcpSha1Transform(dig);
        //Now fill the next block with 56 bytes
        memset(dig->m_aucIn, 0, BLOCKSIZE - 8);
    }
    else
    {
        //Pad block to 56 bytes
        memset(puc, 0, uiCount - 8);
    }

    //Append length in bits and transform
    Word2Bytes1(&dig->m_auiBits[1], &dig->m_aucIn[BLOCKSIZE - 8]);
    Word2Bytes1(&dig->m_auiBits[0], &dig->m_aucIn[BLOCKSIZE - 4]);
    _MHal_HDMITx_HdcpSha1Transform(dig);

    for (i = 0; i < SHA160LENGTH; i++,pcDigest += 4)
        Word2Bytes(&dig->m_auiBuf[i], pcDigest);
}


// *************  For customer NDS **************//

MS_BOOL MHal_HDMITx_HDCP_Get_BCaps(MS_U8 *u8bcaps)
{
    return (MHal_HDMITx_Rx74ReadByte(0x40, u8bcaps));
}

MS_BOOL MHal_HDMITx_HDCP_Get_BStatus(MS_U16 *u16bstatus)
{
    MS_U8 u8data1 = 0x0;
	MS_U8 u8data2 = 0x0;
    MS_BOOL ret = TRUE;

    if(!MHal_HDMITx_Rx74ReadByte(0x42, &u8data2) || !MHal_HDMITx_Rx74ReadByte(0x41, &u8data1))
        ret = FALSE;
    *u16bstatus = ((MS_U16) u8data2<< 8) | u8data1;
    return ret;
}

MS_BOOL MHal_HDMITx_HDCP_Get_BKSV(MS_U8 *u8bksv)
{
    MS_U8 i;
    MS_BOOL ret = TRUE;

    for(i=0; i<5; i++)
    {
        if(!MHal_HDMITx_Rx74ReadByte(0x00+i, u8bksv+i))
            ret = FALSE;
    }
    return ret;
}

void MHal_HDMITx_HDCP_SHA1_Transform(unsigned int *sha1, const MS_U8 *s)
{
    SHA1_DATA tdig;

    MHal_HDMITx_HdcpSha1Init(&tdig);
    // MS_U32 M[2], w[16]
    // w[0]=M[0];
    // w[1]=M[1];
    // w[2]=0x80000000;
    // w[3]=0;w[4]=0;w[5]=0;w[6]=0;w[7]=0;w[8]=0;w[9]=0;w[10]=0;w[11]=0;w[12]=0;w[13]=0;
    // w[14]=0;w[15]=0x00000040;
    memcpy(&tdig.m_aucIn[0], s, 8*sizeof(MS_U8)); // The first 8 bytes are copy from message
    tdig.m_aucIn[8] = 0x80; // append the bit "1" and K = 439 bits "0" to the message
    memset(&tdig.m_aucIn[9], 0, 54*sizeof(MS_U8));
    tdig.m_aucIn[63] = 0x40; // apppend length of message, 64 bytes

    _MHal_HDMITx_HdcpSha1Transform(&tdig);
    memcpy(sha1, &tdig.m_auiBuf[0], 5*sizeof(MS_U32));
    //printf("@@SRM - Trans Msg= %8x%8x%8x%8x%8x\r\n", tdig.m_auiBuf[0],tdig.m_auiBuf[1],tdig.m_auiBuf[2],tdig.m_auiBuf[3],tdig.m_auiBuf[4]);
}

// *************  end  **************//

