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

/*                                                                                                                     */
/*                                                   Includes                                                      */
/*                                                                                                                     */
/*********************************************************************/
//#include <stdio.h>
//#include <string.h>
#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "regHDCP.h"
#include "halHDCP.h"
#include "drv_hdmitx_os.h"

//#include "drvCPU.h"

#ifndef HAL_HDCP_C
#define HAL_HDCP_C

/*********************************************************************/
/*                                                                                                                     */
/*                                                      Defines                                                    */
/*                                                                                                                     */
/*********************************************************************/
#define DEF_HDCP_TX_FUNC_EN     1

#if(defined(CONFIG_MLOG))
#include "ULog.h"

#define HalHDCPLogInfo(format, args...)       ULOGI("HDCP", format, ##args)
#define HalHDCPLogWarning(format, args...)    ULOGW("HDCP", format, ##args)
#define HalHDCPLogDebug(format, args...)      ULOGD("HDCP", format, ##args)
#define HalHDCPLogError(format, args...)      ULOGE("HDCP", format, ##args)
#define HalHDCPLogFatal(format, args...)      ULOGF("HDCP", format, ##args)

#else

#define HalHDCPLogInfo(format, args...)       printf(format, ##args)
#define HalHDCPLogWarning(format, args...)    printf(format, ##args)
#define HalHDCPLogDebug(format, args...)      printf(format, ##args)
#define HalHDCPLogError(format, args...)      printf(format, ##args)
#define HalHDCPLogFatal(format, args...)      printf(format, ##args)

#endif


#define DEF_SIZE_OF_KSXORLC128  16
#define DEF_SIZE_OF_RIV         8
#define DEF_SIZE_OF_HDCP1X_KEY  304

MS_VIRT _gHDCPRegBase = 0x00U;
MS_VIRT _gHDCPPMRegBase = 0x00U;

#define HDCPREG(bank, addr)     (*((volatile MS_U16 *)(((MS_U32)_gHDCPRegBase + (bank << 1U)) + (addr << 2U))))
#define HDCPPMREG(bank, addr)   (*((volatile MS_U16 *)(((MS_U32)_gHDCPPMRegBase + (bank << 1U)) + (addr << 2U))))

#define DEF_HDCP14_M0_SIZE          64U //bytes

/*********************************************************************/
/*                                                                                                                     */
/*                                                    Global                                                        */
/*                                                                                                                     */
/*********************************************************************/
MS_U8 gu8Hdcp1xKey[DEF_SIZE_OF_HDCP1X_KEY]= {0x00};
static MS_BOOL gbIsKmNewMode = FALSE;

/*********************************************************************/
/*                                                                                                                     */
/*                                                    Functions                                                    */
/*                                                                                                                     */
/*********************************************************************/
/*********************************************************************/
/*                                                                                                                     */
/*                                                    Internal                                                      */
/*                                                                                                                     */
/*********************************************************************/

MS_U16 MHalHdcpRegRead(MS_U32 bank, MS_U16 address)
{
    //return HDCPREG(bank, address);
    return 0;
}

void MHalHdcpRegWrite(MS_U32 bank, MS_U16 address, MS_U16 reg_data)
{
    //HDCPREG(bank, address) = reg_data;
}

void MHalHdcpRegMaskWrite(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data)
{
    //MS_U16 reg_value;

    //reg_value = (HDCPREG(bank, address) & (~reg_mask)) | (reg_data & reg_mask);
    //HDCPREG(bank, address) = reg_value;
}

MS_U16 MHalHdcpPMRegRead(MS_U32 bank, MS_U16 address)
{
    //return HDCPPMREG(bank, address);
    return 0;
}

void MHalHdcpPMRegWrite(MS_U32 bank, MS_U16 address, MS_U16 reg_data)
{
    //HDCPPMREG(bank, address) = reg_data;
}

void MHalHdcpPMRegMaskWrite(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data)
{
    //MS_U16 reg_value;

    //reg_value = (HDCPPMREG(bank, address) & (~reg_mask)) | (reg_data & reg_mask);
    //HDCPPMREG(bank, address) = reg_value;
}

/*********************************************************************/
/*                                                                                                                     */
/*                                                    External                                                     */
/*                                                                                                                     */
/*********************************************************************/
void MHal_HDCP_HDCP14TxInitHdcp(MS_U8 u8PortIdx)
{
    u8PortIdx &= 0x0F;
    //TBD: get bank offset by port index
    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0001, 0x8000, 0x8000); // Enable HDCP encryption
    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x001C, 0x0000); //[4]: 1: km new mode; 0: km old mode
}

void MHal_HDCP_HDCP14TxLoadKey(MS_U8* pu8KeyData, MS_BOOL bUseKmNewMode)
{
    gbIsKmNewMode = bUseKmNewMode;
    if (pu8KeyData != NULL)
        memcpy(gu8Hdcp1xKey, pu8KeyData, DEF_SIZE_OF_HDCP1X_KEY);
}

void MHal_HDCP_HDCP14TxSetAuthPass(MS_U8 u8PortIdx)
{
    u8PortIdx &= 0x0F;

    //TBD: get bank offset by port index
    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x000F, 0x000C);
}

void MHal_HDCP_HDCP14TxEnableENC_EN(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    u8PortIdx &= 0x0F;

    if (bEnable == TRUE)
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x0008, 0x0008);
    else
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x0008, 0x0000);
}

void MHal_HDCP_HDCP14TxProcessAn(MS_U8 u8PortIdx, MS_BOOL bUseInternalAn, MS_U8* pu8An)
{
    MS_U8 i = 0;
    u8PortIdx &= 0x0F;

    if (bUseInternalAn == TRUE)
    {
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0001, 0x0100, 0x0100);
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x0002, 0x0002);
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x0002, 0x0000);

        DrvHdmitxOsUsSleep(1);

        for ( i = 0; i < 4; i++ )
        {
            *(pu8An + 2*i) = MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, 0x0008 + i) & 0x00FF;
            *(pu8An + 2*i + 1) = (MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, 0x0008 + i) & 0xFF00) >> 8;
        }
    }
    else
    {
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0001, 0x0100, 0x0000);

        for ( i = 0; i < 4; i++ )
        {
            MHalHdcpRegWrite(DEF_HDCP14_TX_REG_BANK, 0x0008 + i, ((*(pu8An + 2*i + 1) << 8) | (*(pu8An + 2*i))));
        }
    }
}


void MHal_HDCP_HDCP14TxGetAKSV(MS_U8 u8PortIdx, MS_U8* pu8Aksv)
{
    MS_U8 u8ByteCnt = 0;

    u8PortIdx &= 0x0F;

    for (u8ByteCnt = 0; u8ByteCnt < 5; u8ByteCnt++ )
    {
        *(pu8Aksv + u8ByteCnt) = gu8Hdcp1xKey[u8ByteCnt];
    }
}


MS_BOOL MHal_HDCP_HDCP14TxCompareRi(MS_U8 u8PortIdx, MS_U8* pu8SinkRi)
{
    MS_BOOL bRet = FALSE;
    MS_U16 u16SrcRi = 0x0000;
    u8PortIdx &= 0x0F;

    do
    {
        u16SrcRi = MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, 0x0000);

        if (u16SrcRi == *(MS_U16*)pu8SinkRi)
        {
            bRet = TRUE;
            break;
        }

        u16SrcRi = MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, 0x0000);

        if (u16SrcRi == *(MS_U16*)pu8SinkRi)
        {
            bRet = TRUE;
            break;
        }

    } while (FALSE);

    return bRet;
}


void MHal_HDCP_HDCP14TxConfigMode(MS_U8 u8PortIdx, MS_U8 u8Mode)
{
    u8PortIdx &= 0x0F;

    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0001, 0x0E00, u8Mode << 8);
}


void MHal_HDCP_HDCP14TxGenerateCipher(MS_U8 u8PortIdx, MS_U8* pu8Bksv)
{
#define DEF_HDCP1X_KEY_OFFSET 8U

    MS_U8 u8Lm[7] = {0};
    MS_U8 u8ByteCnt = 0;
    MS_U8 u8BitCnt = 0;
    MS_U8 u8LmCnt = 0;
    MS_U8 u8CarryBit = 0;
    MS_U8 u8Seed = 0;
    MS_U8 u8Tmp = 0;
    MS_U16 u16Offset = 0;

    for (u8ByteCnt = 0; u8ByteCnt < 5; u8ByteCnt++)
    {
        for (u8BitCnt = 0; u8BitCnt < 8; u8BitCnt++)
        {
            if (*(pu8Bksv + u8ByteCnt) & (1 << u8BitCnt))
            {
                u8CarryBit = 0;
                u16Offset = (u8ByteCnt * 8 + u8BitCnt) * 7 + DEF_HDCP1X_KEY_OFFSET;

                for (u8LmCnt = 0; u8LmCnt < 7; u8LmCnt++)
                {
                    u8Seed = (u8LmCnt + gu8Hdcp1xKey[7]) % 7;
                    u8Tmp = gu8Hdcp1xKey[u16Offset + u8LmCnt] ^ gu8Hdcp1xKey[u8Seed];

                    u8Lm[u8LmCnt] = u8Lm[u8LmCnt] + u8Tmp + u8CarryBit;
                    if (((u8CarryBit == 0) && (u8Lm[u8LmCnt] >= u8Tmp)) || ((u8CarryBit == 1) && (u8Lm[u8LmCnt] > u8Tmp)))
                        u8CarryBit = 0;
                    else
                        u8CarryBit = 1;
                }
            }
        }
    }

    u8Tmp = gu8Hdcp1xKey[288];

    for (u8LmCnt = 0; u8LmCnt < 7; u8LmCnt++)
    {
        if (u8LmCnt < 6)
            u8Lm[u8LmCnt] = u8Lm[u8LmCnt] ^ u8Tmp;
        else
            u8Lm[u8LmCnt] = u8Lm[u8LmCnt] ^ (~u8Tmp);

        if (u8LmCnt % 2 != 0)
            MHalHdcpRegWrite(DEF_HDCP14_TX_REG_BANK, 0x0004 + (u8LmCnt/2), (u8Lm[u8LmCnt] << 8) | u8Lm[u8LmCnt - 1]);
    }

    MHalHdcpRegWrite(DEF_HDCP14_TX_REG_BANK, 0x0007, (u8Tmp << 8) | u8Lm[6]);

#undef DEF_HDCP1X_KEY_OFFSET

}

MS_BOOL MHal_HDCP_HDCP14TxProcessR0(MS_U8 u8PortIdx)
{
    MS_U8 u8Cnt = 0;

    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x000F, 0x0000);
    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x000F, 0x0001);
    MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, 0x0002, 0x000F, 0x0000);

    while (u8Cnt-- && !(MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, 0x0002) & 0x0100));

    return ((u8Cnt == 0) ? FALSE : TRUE);
}

void MHal_HDCP_HDCP14TxGetM0(MS_U8 u8PortIdx, MS_U8* pu8M0)
{
#define DEF_HDCP1X_M0_SIZE 8
    unsigned char u8DataCnt = 0;
    MS_U16 u16BKOffset = 0x00;
    MS_U16 u16RegVal = 0x00;

    u8PortIdx &= 0x0F;

    for ( u8DataCnt = 0; u8DataCnt < (DEF_HDCP1X_M0_SIZE>>1); u8DataCnt++ )
    {
        u16RegVal = MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK + u16BKOffset, 0x0C + u8DataCnt);
        *(pu8M0 + 2*u8DataCnt) = (MS_U8)(u16RegVal & 0x00FF);
        *(pu8M0 + 2*u8DataCnt + 1) = (MS_U8)((u16RegVal & 0xFF00)>>8);

    }
#undef DEF_HDCP1X_M0_SIZE
}

void MHal_HDCP_HDCP14GetM0(MS_U8 u8PortIdx, MS_U8 *pu8Data)
{
    MS_U8 cnt = 0x00;
    MS_U16 u16BKOffset = 0x00;

    u8PortIdx &= 0x0F;
    //Kano only has 1 Tx port

    for ( cnt = 0; cnt < (DEF_HDCP14_M0_SIZE >> 4); cnt++ )
    {
        MS_U16 u16tmpData = 0x00;

        u16tmpData = MHalHdcpRegRead(DEF_HDCP14_RX_REG_BANK + u16BKOffset, 0x0E + cnt);

        *(pu8Data + cnt*2) = (MS_U8)(u16tmpData & 0x00FF);
        *(pu8Data + cnt*2 + 1) = (MS_U8)((u16tmpData & 0xFF00) >> 8);
    }
}

void MHal_HDCP_HDCP14FillBksv(MS_U8 *pu8BksvData)
{
    MS_U8 uctemp = 0;
    MS_U8 ucPortSelect = 0;
    MS_U32 ulMACBankOffset = 0;

    for(ucPortSelect = HDMI_RX_SELECT_PORTA; ucPortSelect < HDMI_RX_SELECT_MASK; ucPortSelect++)
    {
        switch(ucPortSelect)
        {
            case HDMI_RX_SELECT_PORTA:
                ulMACBankOffset = 0;
                break;

            case HDMI_RX_SELECT_PORTB:
                ulMACBankOffset = 0x300;
                break;

            case HDMI_RX_SELECT_PORTC:
                ulMACBankOffset = 0x600;
                break;

            case HDMI_RX_SELECT_PORTD:
                ulMACBankOffset = 0x900;
                break;

            default:
                break;
        };

        // Bksv
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x17, BIT(10), BIT(10));
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(15)|BIT(14), BIT(15)); // [15]: CPU write disable, [14]: 0: 74 RAM, 1 :HDCP RAM

        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x17, BMASK(9:0), 0x00); // address
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(5), BIT(5));

        for(uctemp = 0; uctemp < 5; uctemp++)
        {
            MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x18, BMASK(7:0), pu8BksvData[uctemp]); // data
            MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(4), BIT(4)); // trigger latch data

            while(MHalHdcpRegRead(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19) & BIT(7)); // wait write ready
            while(MHalHdcpRegRead(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19) & BIT(7)); // wait write ready for SW patch
        }

        // Bcaps = 0x80
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x17, BMASK(9:0), 0x40); // address
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(5), BIT(5));

        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x18, BMASK(7:0), 0x80); // data
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(4), BIT(4)); // trigger latch data

        while(MHalHdcpRegRead(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19) & BIT(7)); // wait write ready

        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x19, BIT(15)|BIT(14), 0); // [15]: CPU write disable, [14]: 0: 74 RAM, 1 :HDCP RAM

        // [10:8]: 3'b111 determine Encrp_En during Vblank in DVI mode; [5]:HDCP enable; [0]: EESS mode deglitch Vsync mode
        MHalHdcpRegMaskWrite(DEF_HDCP14_RX_REG_BANK +ulMACBankOffset, 0x00, BIT(10)|BIT(9)|BIT(8)|BIT(5)|BIT(0), BIT(10)|BIT(9)|BIT(8)|BIT(5)|BIT(0));
    }
}

void MHal_HDCP_HDCP14FillKey(MS_U8 *pu8KeyData)
{
    MS_U16 ustemp = 0;

    MHalHdcpRegMaskWrite(DEF_HDCPKEY_REG_BANK, REG_HDCPKEY_BANK_02_L, BIT(8), BIT(8));

    // HDCP key
    MHalHdcpRegMaskWrite(DEF_COMBO_GP_TOP_REG_BANK, REG_COMBO_GP_TOP_40_L, BIT(3)|BIT(2)|BIT(0), BIT(3)|BIT(2)|BIT(0)); // [2]: CPU write enable, [3]: 0: 74 RAM, 1 :HDCP RAM
    // burst write from address 0x05
    MHalHdcpRegMaskWrite(DEF_HDCPKEY_REG_BANK, REG_HDCPKEY_BANK_00_L, BMASK(9:0), 0x05); // address

    for(ustemp = 0; ustemp < 284; ustemp++)
    {
        MHalHdcpRegMaskWrite(DEF_HDCPKEY_REG_BANK, REG_HDCPKEY_BANK_01_L, BMASK(7:0), *(pu8KeyData +ustemp)); // data
    }

    MHalHdcpRegMaskWrite(DEF_COMBO_GP_TOP_REG_BANK, REG_COMBO_GP_TOP_40_L, BIT(3)|BIT(2)|BIT(0), 0); // [2]: CPU write enable, [3]: 0: 74 RAM, 1 :HDCP RAM
}

void MHal_HDCP_SetBank(MS_U32 u32NonPmBankAddr, MS_U32 u32PmBankAddr)
{
    HalHDCPLogInfo("u32NonPmBankAddr = 0x%X, u32PmBankAddr = 0x%X\r\n", (unsigned int)u32NonPmBankAddr, (unsigned int)u32PmBankAddr);
    _gHDCPRegBase = u32NonPmBankAddr;
    _gHDCPPMRegBase = u32PmBankAddr;
}

void MHal_HDCP_HDCP2TxInit(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MS_U16 u16BKOffset = 0x00;

    MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x11, bEnable ? 0x11 : 0x00); // bit 0: enable hdcp22; bit 4: enable EESS
    if (bEnable)
    {
        MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x02, 0x02); //reset hdcp22 FSM
        MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x02, 0x00);
    }
}

void MHal_HDCP_HDCP2TxEnableEncrypt(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MS_U16 u16BKOffset = 0x00;

    MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x04, bEnable ? 0x04 : 0x00); //bit 2: authentication pass
    MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x08, bEnable ? 0x08 : 0x00); //bit 3: enable hdcp22 to issue encryption enable signal
}

void MHal_HDCP_HDCP2TxFillCipherKey(MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128)
{
    MS_U8 cnt = 0;
    MS_U16 u16BKOffset = 0x00;
    //MS_U16 u16RegOffset = 0x00;

    //Kano only has 1 Tx port

    //Ks^LC128
    for ( cnt = 0; cnt < (DEF_SIZE_OF_KSXORLC128>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_TX_KEY_REG_BANK + u16BKOffset, 0x60 + (DEF_SIZE_OF_KSXORLC128 >> 1) - 1 - cnt, *(pu8KsXORLC128 + cnt*2 + 1)|(*(pu8KsXORLC128 + cnt*2)<<8));

    //Riv
    for ( cnt = 0; cnt < (DEF_SIZE_OF_RIV>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_TX_KEY_REG_BANK + u16BKOffset, 0x68 + (DEF_SIZE_OF_RIV >> 1) - 1 - cnt, *(pu8Riv + cnt*2 + 1)|(*(pu8Riv + cnt*2)<<8));

}

void MHal_HDCP_HDCP2TxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State)
{
    MS_U16 u16BKOffset = 0x00;
    //MS_U16 u16RegOffset = 0x00;


    *pu8State = MHalHdcpRegRead(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x00) & 0x0C;
}

void MHal_HDCP_HDCP2TxSetAuthPass(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MS_U16 u16BKOffset = 0x00;

    MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK + u16BKOffset, 0x0000, 0x04, bEnable ? 0x04 : 0x00); //bit 2: authentication pass
}

void MHal_HDCP_HDCP2RxInit(MS_U8 u8PortIdx)
{
    MS_U16 u16BKOffset = 0x00;

    // [1] Enable auto-clear SKE status when receiving ake_init; [2] Enable auto-clear SKE status when no hdcp22 capability
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4E, 0x0006, 0x0006);
}

void MHal_HDCP_HDCP2RxProcessCipher(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey)
{
    MS_U8 cnt = 0;
    MS_U16 u16BKOffset = 0x00;
    MS_U16 u16RegOffset = 0x00;
    MS_U16 u16RetryCnt = 0;

    //Ks^LC128
    for ( cnt = 0; cnt < (DEF_SIZE_OF_KSXORLC128>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_RX_KEY_REG_BANK + u16BKOffset, u16RegOffset + 0x30 + (DEF_SIZE_OF_KSXORLC128 >> 1) - 1 - cnt, *(pu8ContentKey + cnt*2 + 1)|(*(pu8ContentKey + cnt*2)<<8));

    //Riv
    for ( cnt = 0; cnt < (DEF_SIZE_OF_RIV>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_RX_KEY_REG_BANK + u16BKOffset, u16RegOffset + 0x38 + (DEF_SIZE_OF_RIV >> 1) - 1 - cnt, *(pu8Riv + cnt*2 + 1)|(*(pu8Riv + cnt*2)<<8));

    //Set SKE successful
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x62, 0x0001, 0x0001);
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x62, 0x0002, 0x0002);
    do
    {
        MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4E, 0x0001, 0x0000);
        DrvHdmitxOsUsSleep(1);
        MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4E, 0x0001, 0x0001);
        u16RetryCnt++;
    } while ((!(MHalHdcpRegRead(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4F) & 0x01)) && (u16RetryCnt < 2000));
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x62, 0x0001, 0x0000);
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x62, 0x0002, 0x0000);

}

void MHal_HDCP_HDCP2RxSetSKEPass(MS_U8 u8PortIdx, MS_BOOL bEnable)
{
    MS_U16 u16BKOffset = 0x00;
    //MS_U16 u16RegOffset = 0x00;

    //Set SKE successful
    MHalHdcpRegMaskWrite(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4E, 0x0001, bEnable ? 0x0001 : 0x0000);
}

void MHal_HDCP_HDCP2RxFillCipherKey(MS_U8 u8PortIdx, MS_U8* pu8Riv, MS_U8 *pu8ContentKey)
{
    MS_U8 cnt = 0;
    MS_U16 u16BKOffset = 0x00;
    MS_U16 u16RegOffset = 0x00;

    //Ks^LC128
    for ( cnt = 0; cnt < (DEF_SIZE_OF_KSXORLC128>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_RX_KEY_REG_BANK + u16BKOffset, u16RegOffset + 0x30 + (DEF_SIZE_OF_KSXORLC128 >> 1) - 1 - cnt, *(pu8ContentKey + cnt*2 + 1)|(*(pu8ContentKey + cnt*2)<<8));

    //Riv
    for ( cnt = 0; cnt < (DEF_SIZE_OF_RIV>>1); cnt++)
        MHalHdcpRegWrite(DEF_HDCP22_RX_KEY_REG_BANK + u16BKOffset, u16RegOffset + 0x38 + (DEF_SIZE_OF_RIV >> 1) - 1 - cnt, *(pu8Riv + cnt*2 + 1)|(*(pu8Riv + cnt*2)<<8));
}

void MHal_HDCP_HDCP2RxGetCipherState(MS_U8 u8PortIdx, MS_U8 *pu8State)
{
    MS_U16 u16BKOffset = 0x00;
    //MS_U16 u16RegOffset = 0x00;


    *pu8State = MHalHdcpRegRead(DEF_HDCP22_RX_REG_BANK + u16BKOffset, 0x4E) & 0x01;
}

MS_U32 MHal_HDCP_HDCP1TxEncrytionStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus)
{
    MS_U32 u32GetStatus = 0;

    if(u8SetStatusFlag) // Set HDCP1 encrytion status
    {
        MHalHdcpRegMaskWrite(DEF_HDCP14_TX_REG_BANK, REG_HDCP14_TX_02_L, BIT(3), u32SetStatus? BIT(3): 0); //bit 3: enable hdcp14 to issue encryption enable signal
    }

    // Get HDCP1 encrytion status
    if(MHalHdcpRegRead(DEF_HDCP14_TX_REG_BANK, REG_HDCP14_TX_02_L) &BIT(3))
    {
        u32GetStatus = TRUE;
    }

    return u32GetStatus;
}

MS_U32 MHal_HDCP_HDCP2TxEncrytionStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus)
{
    MS_U32 u32GetStatus = 0;

    if(u8SetStatusFlag) // Set HDCP2 encrytion status
    {
        MHalHdcpRegMaskWrite(DEF_HDCP22_TX_REG_BANK, REG_HDCP22_TX_00_L, BIT(3), u32SetStatus? BIT(3): 0); //bit 3: enable hdcp22 to issue encryption enable signal
    }

    // Get HDCP2 encrytion status
    if(MHalHdcpRegRead(DEF_HDCP22_TX_REG_BANK, REG_HDCP22_TX_00_L) &BIT(3))
    {
        u32GetStatus = TRUE;
    }

    return u32GetStatus;
}

MS_U32 MHal_HDCP_HDCPTxHDMIStatus(MS_U8 u8PortIdx, MS_U8 u8SetStatusFlag, MS_U32 u32SetStatus)
{
    MS_U32 u32GetStatus = 0;

    if(u8SetStatusFlag) // Set HDNI status
    {
        MHalHdcpRegMaskWrite(DEF_HDMITX_PHY_REG_BANK, REG_HDMITX_PHY_39_L, 0xFFFF, u32SetStatus? 0xF000: 0xFFFF);
        MHalHdcpRegMaskWrite(DEF_HDMITX_PHY_REG_BANK, REG_HDMITX_PHY_2E_L, 0xE800, u32SetStatus? 0xE800: 0x0000);
    }

    // Get HDNI status
    if((MHalHdcpRegRead(DEF_HDMITX_PHY_REG_BANK, REG_HDMITX_PHY_2E_L) &0xE800) == 0xE800)
    {
        u32GetStatus = TRUE;
    }

    return u32GetStatus;
}

#endif //#ifndef HAL_HDCP_C
