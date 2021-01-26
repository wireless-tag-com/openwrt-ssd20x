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

#define  MHAL_HDMIUTILTX_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "halHDMIUtilTx.h"
#include "regHDMITx.h"
//#include "drvCPU.h"


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//static MS_U32 _gHDMITx_MapBase = 0U;
//static MS_U32 _gPM_MapBase = 0U;

MS_VIRT _gHDMITx_MapBase = 0U;
MS_VIRT _gPM_MapBase = 0U;
MS_VIRT _gCoproBase = 0U;


#define REG(bank, addr)              (*((volatile MS_U16 *)(((MS_U32)_gPM_MapBase+(bank<<1U)) + ((addr)<<2U))))
#define PMREG(bank, addr)            (*((volatile MS_U16 *)(((MS_U32)_gPM_MapBase+(bank<<1U)) + ((addr)<<2U))))
#define REG8(bank, addr)             (*((volatile MS_U8 *)(((MS_U32)_gPM_MapBase+(((MS_U32)bank)<<1U)) + ((((MS_U32)addr) & 0xFFFFFFFE)<<1U) + (((MS_U32)addr)&0x00000001) )))

//++ start for IIC
#define PIN_HIGH                        1U
#define PIN_LOW                         0U
#define I2c_CHK                         0

#define I2C_ACKNOWLEDGE                 PIN_LOW
#define I2C_NON_ACKNOWLEDGE             PIN_HIGH

#define i2cSetSCL(pin_state)            ( DrvHdmitxOsSetGpioValue(gu8SwI2cSclPin, pin_state) )//( REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) = (REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) & (~BIT0)) | (pin_state << 0) )
#define i2cSetSDA(pin_state)            ( DrvHdmitxOsSetGpioValue(gu8SwI2cSdaPin, pin_state) )//( REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) = (REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) & (~BIT4)) | (pin_state << 4) )
#define i2cGeSCL()                      ( DrvHdmitxOsGetGpioValue(gu8SwI2cSclPin) )//( (REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) & BIT0) >> 0 )
#define i2cGetSDA()                     ( DrvHdmitxOsGetGpioValue(gu8SwI2cSdaPin) )//( (REG(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_01) & BIT4) >> 4 )

#define I2C_CHECK_PIN_TIME              1000U // unit: 1 us
#define I2C_CHECK_PIN_CYCLE             8U    // cycle of check pin loopp
#define MCU_MICROSECOND_NOP_NUM         1U
#define I2C_CHECK_PIN_DUMMY             100U//255 /*((I2C_CHECK_PIN_TIME / I2C_CHECK_PIN_CYCLE) * MCU_MICROSECOND_NOP_NUM)*/
#define I2C_ACCESS_DUMMY_TIME           3U

#define I2C_DEVICE_ADR_WRITE(slave_adr)   (slave_adr & ~BIT0)
#define I2C_DEVICE_ADR_READ(slave_adr)    (slave_adr | BIT0)
//--- end of IIC

#define HDMITX_RX74_SLAVE_ADDR          0x74U
#define HDMITX_EDIDROM_SLAVE_ADDR       0xA0U

//wilson@kano -- for SCDC
#define HDMITX_SCDC_SLAVE_ADDR			0xA8U


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef enum _I2cIoTransType
{
    I2C_TRANS_READ,
    I2C_TRANS_WRITE
} I2cIoTransType;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern MS_BOOL g_bDisableRegWrite;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#ifdef CAMDRV_DEBUG
static MS_BOOL bDebugUtilFlag = TRUE;
#else
static MS_BOOL bDebugUtilFlag = FALSE;
#endif
static MS_U32 u32DDCDelayCount = 1200U; //[K6] Modify i2c speed from 200 to 100kHz


MS_U8 gu8SwI2cSdaPin = 88;
MS_U8 gu8SwI2cSclPin = 87;
MS_BOOL gbSwI2cEn = 1;
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

#define DBG_HDMIUTIL(_f)                  do{ if(bDebugUtilFlag & TRUE) (_f); } while(0);


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_EEPROM_Read(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size);

//****************************************
// i2c_Delay()
//****************************************
void i2c_Delay(void)
{
    /*
     * set HDMITx I2C data rate to 50KHz
     */
    volatile MS_U32 i = u32DDCDelayCount;
    while(i-->0)
    {
        #ifdef __mips__
        __asm__ __volatile__ ("nop");
        #endif

        #ifdef __AEONR2__
        __asm__ __volatile__ ("l.nop");
        #endif

        #ifdef __arm__
        __asm__ __volatile__ ("mov r0, r0");
        #endif
    }
}


/////////////////////////////////////////
// Set I2C SCL pin high/low.
//
// Arguments: bSet - high/low bit
/////////////////////////////////////////
void i2cSetSCL_Chk(MS_U8 bSet)
{
#if I2c_CHK
    MS_U8 ucDummy; // loop dummy
#endif

    i2cSetSCL(bSet); // set SCL pin
#if I2c_CHK
    if (bSet == PIN_HIGH) // if set pin high
    {
        ucDummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((i2cGeSCL() == PIN_LOW) && (ucDummy--)) ; // check SCL pull high
    }
    else
    {
        ucDummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((i2cGeSCL() == PIN_HIGH) && (ucDummy--)) ; // check SCL pull low
    }
#endif
}

/////////////////////////////////////////
// Set I2C SDA pin high/low
//
// Arguments: bSet - high/low bit
/////////////////////////////////////////
void i2cSetSDA_Chk(MS_U8 bSet)
{
#if I2c_CHK
    MS_U8 ucDummy; // loop dummy
#endif

    i2cSetSDA(bSet); // set SDA pin

#if I2c_CHK
    if (bSet == PIN_HIGH) // if set pin high
    {
        ucDummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((i2cGetSDA() == PIN_LOW) && (ucDummy--)) ; // check SDA pull high
    }
#endif
}

/////////////////////////////////////////
// Set I2C SDA pin as input
//
// Arguments:
/////////////////////////////////////////
void i2cSetSDA_Input(void)
{
#if I2c_CHK
    volatile MS_U8 ucDummy = 70; // loop dummy
    MS_BOOL bflag = 0;
#endif

    i2cGetSDA(); // set SDA pin

#if I2c_CHK
    while (ucDummy-- > 0)
    {
        bflag = (i2cGetSDA() == PIN_HIGH) ? TRUE : FALSE;
    }
#endif
}

//////////////////////////////////////////////////////
// I2C start signal.
// <comment>
//  SCL ________
//              \_________
//  SDA _____
//           \____________
//
// Return value: None
//////////////////////////////////////////////////////
MS_BOOL i2c_Start(void)
{
    MS_BOOL bStatus = TRUE; // success status
    MS_U32 u32OldIntr;

    //disable all interrupt
    u32OldIntr = DrvHdmitxOsDisableAllInterrupts();

    i2cSetSDA_Chk(PIN_HIGH);
    i2c_Delay();
    i2cSetSCL_Chk(PIN_HIGH);
    i2c_Delay();

#if I2c_CHK
    // check pin error
    if ((i2cGeSCL() == PIN_LOW) || (i2cGetSDA() == PIN_LOW))
    {
        bStatus = FALSE;
        DBG_HDMIUTIL(printf("i2c_Start()::SCL or SDA could not pull low, SCL = %d, SDA= %d\n", i2cGeSCL(), i2cGetSDA()));
    }
    else // success
#endif
    {
        i2cSetSDA(PIN_LOW);
        i2c_Delay();
        i2cSetSCL(PIN_LOW);
        i2c_Delay(); //AWU addded
    }

    //restore interrupt
    DrvHdmitxOsRestoreAllInterrupts(u32OldIntr);

    return bStatus;
}

/////////////////////////////////////////
// I2C stop signal.
// <comment>
//              ____________
//  SCL _______/
//                 _________
//  SDA __________/
/////////////////////////////////////////
void i2c_Stop(void)
{
    MS_U32 u32OldIntr;

    //disable all interrupt
    u32OldIntr = DrvHdmitxOsDisableAllInterrupts();

    i2cSetSCL(PIN_LOW);
    i2c_Delay();
    i2cSetSDA(PIN_LOW);
    i2c_Delay();
    i2cSetSCL_Chk(PIN_HIGH);
    i2c_Delay();
    i2cSetSDA_Chk(PIN_HIGH);
    i2c_Delay();

    //restore interrupt
    DrvHdmitxOsRestoreAllInterrupts(u32OldIntr);
}

//////////////////////////////////////////////////////////////////////////
// I2C receive byte from device.
//
// Return value: receive byte
//////////////////////////////////////////////////////////////////////////
MS_U8 i2c_ReceiveByte(MS_U16 bAck)
{
    MS_U8 ucReceive = 0;
    MS_U8 ucMask = 0x80;

    MS_U32 u32OldIntr;

    //disable all interrupt
    u32OldIntr = DrvHdmitxOsDisableAllInterrupts();

    i2cSetSDA_Input();
    i2c_Delay();//AWU added

    while (ucMask)
    {

        i2cSetSCL_Chk(PIN_HIGH);
        i2c_Delay();

        if ( i2cGetSDA() == PIN_HIGH )
        {
            ucReceive |= ucMask;
        }
        i2cSetSCL_Chk(PIN_LOW);
        i2c_Delay();
        ucMask >>= 1; // next
    } // while

    if (bAck) // acknowledge
        i2cSetSDA_Chk(I2C_ACKNOWLEDGE);
    else // non-acknowledge
        i2cSetSDA_Chk(I2C_NON_ACKNOWLEDGE);

    i2c_Delay();
    i2cSetSCL_Chk(PIN_HIGH);
    i2c_Delay();
    i2cSetSCL(PIN_LOW);
    i2c_Delay();

    //restore interrupt
    DrvHdmitxOsRestoreAllInterrupts(u32OldIntr);

    return ucReceive;
}

//////////////////////////////////////////////////////////////////////////
// I2C send byte to device.
//
// Arguments: uc_val - send byte
// Return value: I2C acknowledge bit
//               I2C_ACKNOWLEDGE/I2C_NON_ACKNOWLEDGE
//////////////////////////////////////////////////////////////////////////
MS_BOOL i2c_SendByte(MS_U8 uc_val)
{
    MS_U8 ucMask = 0x80;
    MS_U8 bAck; // acknowledge bit

    MS_U32 u32OldIntr;

    //disable all interrupt
    u32OldIntr = DrvHdmitxOsDisableAllInterrupts();

    while (ucMask)
    {
        if (uc_val & ucMask)
            i2cSetSDA_Chk(PIN_HIGH);
        else
            i2cSetSDA_Chk(PIN_LOW);
        i2c_Delay();
        i2cSetSCL_Chk(PIN_HIGH); // clock
        i2c_Delay();
        i2cSetSCL_Chk(PIN_LOW);
        i2c_Delay();

        ucMask >>= 1; // next
    } // while

    // recieve acknowledge
    i2cSetSDA_Input();
    i2c_Delay();
    i2cSetSCL_Chk(PIN_HIGH);
    i2c_Delay();
    bAck = i2cGetSDA(); // recieve acknowlege
    i2cSetSCL(PIN_LOW);
    i2c_Delay();

    //restore interrupt
    DrvHdmitxOsRestoreAllInterrupts(u32OldIntr);

    return (bAck);
}

//////////////////////////////////////////////////////////////////////////
// I2C access start.
//
// Arguments: ucSlaveAdr - slave address
//            trans_t - I2C_TRANS_WRITE/I2C_TRANS_READ
//////////////////////////////////////////////////////////////////////////
MS_BOOL i2c_AccessStart(MS_U8 ucSlaveAdr, I2cIoTransType trans_t)
{
    MS_U8 ucDummy; // loop dummy

    if (trans_t == I2C_TRANS_READ) // check i2c read or write
        ucSlaveAdr = I2C_DEVICE_ADR_READ(ucSlaveAdr); // read
    else
        ucSlaveAdr = I2C_DEVICE_ADR_WRITE(ucSlaveAdr); // write

    ucDummy = I2C_ACCESS_DUMMY_TIME;
    while (ucDummy--)
    {
        if (i2c_Start() == FALSE)
            continue;

        if (i2c_SendByte(ucSlaveAdr) == I2C_ACKNOWLEDGE) // check acknowledge
            return TRUE;
        else
        {
            DBG_HDMIUTIL(printf("i2c_AccessStart()::No ACK\n"));
        }

        i2c_Stop();

        //DrvHdmitxOsMsSleep(1);//delay 1ms
        DrvHdmitxOsUsSleep(100); // delay 100us
    } // while

    return FALSE;
}

/////////////////////////////////////////////////////////////////
// I2C read bytes from device.
//
// Arguments: ucSlaveAdr - slave address
//            ucSubAdr - sub address
//            pBuf - pointer of buffer
//            ucBufLen - length of buffer
/////////////////////////////////////////////////////////////////
MS_BOOL i2cBurstReadBytes(MS_U8 ucSlaveAdr, MS_U8 ucSubAdr, MS_U8 *pBuf, MS_U16 ucBufLen)
{
    MS_U8 ucDummy; // loop dummy
    MS_BOOL result = FALSE;

    if(gbSwI2cEn)
    {
        ucDummy = I2C_ACCESS_DUMMY_TIME;
        while (ucDummy--)
        {
            if (i2c_AccessStart(ucSlaveAdr, I2C_TRANS_WRITE) == FALSE)
                continue;

            if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
            {
                DBG_HDMIUTIL(printf("i2cBurstReadBytes()::No ACK\n"));
                continue;
            }

            //i2c_Stop();//AWU added

            if (i2c_AccessStart(ucSlaveAdr, I2C_TRANS_READ) == FALSE)
                continue;

            while (ucBufLen--) // loop to burst read
            {
                *pBuf = i2c_ReceiveByte(ucBufLen); // receive byte

                //if(*pBuf) DBG_HDMITX(printf("i2cBurstReadBytes()::BINGO\n"));

                pBuf++; // next byte pointer
            } // while
            result = TRUE;
            break;
        } // while

        i2c_Stop();

        return result;
    }
    else
    {
        return DrvHdmitxOsSetI2cReadBytes(ucSlaveAdr, ucSubAdr, pBuf, ucBufLen);
    }
}

/////////////////////////////////////////////////////////////////
// I2C write bytes to device.
//
// Arguments: ucSlaveAdr - slave address
//            ucSubAdr - sub address
//            pBuf - pointer of buffer
//            ucBufLen - length of buffer
/////////////////////////////////////////////////////////////////
MS_BOOL i2cBurstWriteBytes(MS_U8 ucSlaveAdr, MS_U8 ucSubAdr, MS_U8 *pBuf, MS_U16 ucBufLen)
{
    MS_U8 ucDummy; // loop dummy
    MS_BOOL result = FALSE;

    if(gbSwI2cEn)
    {
        ucDummy = I2C_ACCESS_DUMMY_TIME;
        while (ucDummy--)
        {
            if (i2c_AccessStart(ucSlaveAdr, I2C_TRANS_WRITE) == FALSE)
                continue;

            if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
            {
                DBG_HDMIUTIL(printf("i2cBurstReadBytes()::No ACK\n"));
                continue;
            }

            while (ucBufLen--) // loop of writting data
            {
                result = i2c_SendByte(*pBuf); // send byte, fix coverity impact.

                pBuf++; // next byte pointer
            } // while
            result = TRUE;
            break;
        } // while

        i2c_Stop();

        return result;
    }
    else
    {
        return DrvHdmitxOsSetI2cWriteBytes(ucSlaveAdr, ucSubAdr, pBuf, ucBufLen, 1);
    }
}

/////////////////////////////////////////////////////////////////
// I2C read a byte from device.
//
// Arguments: ucSlaveAdr - slave address
//            ucSubAdr - sub address
//            pBuf - return buffer point
// Return value: read byte
/////////////////////////////////////////////////////////////////
MS_BOOL i2cReadByte(MS_U8 ucSlaveAdr, MS_U8 ucSubAdr, MS_U8 *pBuf)
{
    return (i2cBurstReadBytes(ucSlaveAdr, ucSubAdr, pBuf, 1));
}

/////////////////////////////////////////////////////////////////
// I2C write a byte from device.
//
// Arguments: ucSlaveAdr - slave address
//            ucSubAdr - sub address
//            uc_val - write byte
/////////////////////////////////////////////////////////////////
MS_BOOL i2cWriteByte(MS_U8 ucSlaveAdr, MS_U8 ucSubAdr, MS_U8 uc_val)
{
    return (i2cBurstWriteBytes(ucSlaveAdr, ucSubAdr, &uc_val, 1));
}


//------------------------------------------------------------------------------
/// @brief Set HDMITx register base address
/// @param[in] u32Base
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetIOMapBase(MS_VIRT u32Base, MS_VIRT u32PMBase, MS_VIRT u32CoproBase)
{
    _gHDMITx_MapBase = u32Base;
    _gPM_MapBase = u32PMBase;
    _gCoproBase = u32CoproBase;
}

MS_BOOL MHal_HDMITx_Rx74WriteByte(MS_U8 addr, MS_U8 value)
{
    //return (i2cWriteByte(HDMITX_RX74_SLAVE_ADDR, addr, value));
    return TRUE;
}

MS_BOOL MHal_HDMITx_Rx74ReadByte(MS_U8  addr, MS_U8 *pBuf)
{
    //return (i2cReadByte(HDMITX_RX74_SLAVE_ADDR, addr, pBuf));
    return TRUE;
}

MS_BOOL MHal_HDMITx_Rx74WriteBytes(MS_U8  addr, MS_U16 len, MS_U8  *buf)
{
    //return (i2cBurstWriteBytes(HDMITX_RX74_SLAVE_ADDR, addr, buf, len));
    return TRUE;
}

MS_BOOL MHal_HDMITx_Rx74ReadBytes(MS_U8  addr, MS_U16  len, MS_U8  *buf)
{
    //return (i2cBurstReadBytes(HDMITX_RX74_SLAVE_ADDR, addr, buf, len));
    return TRUE;
}

// This routine read edid
MS_BOOL _MHal_HDMITx_EdidReadBytes(MS_U8  addr, MS_U8  len, MS_U8  *buf)
{
    return (i2cBurstReadBytes(HDMITX_EDIDROM_SLAVE_ADDR, addr, buf, len));
}

//      TxEdidSetSegment
MS_BOOL _MHal_HDMITx_EdidSetSegment(MS_U8  value)
{
    MS_BOOL bRet = TRUE;

    if(gbSwI2cEn)
    {
        if(i2c_AccessStart(0x60, I2C_TRANS_WRITE) == FALSE)
        {
            bRet = FALSE;
            printf("%s %d, Start Fail\n", __FUNCTION__, __LINE__);

        }
        if(i2c_SendByte(value) == I2C_NON_ACKNOWLEDGE) // send byte
        {
            printf("%s %d, SendByte Fail\n", __FUNCTION__, __LINE__);
            bRet = FALSE;
        }
    }
    else
    {
        if(DrvHdmitxOsSetI2cWriteBytes(0x60, value, NULL, 0, 0) == FALSE)
        {
            printf("%s %d, EdidSetSegment Fail\n", __FUNCTION__, __LINE__);
            bRet = FALSE;
        }
    }

    return bRet;
}

//  This routine read the block in EDID
MS_BOOL MHal_HDMITx_EdidReadBlock(MS_U8  num, MS_U8 *buf)
{
#if 1
    MS_U8 start_addr;

    start_addr = (num & 0x01) ? 0x80 : 0x00;

    if(num > 1)
    {
        if(_MHal_HDMITx_EdidSetSegment(num / 2) == FALSE)
        {
            return FALSE;
        }
    }
    if(_MHal_HDMITx_EdidReadBytes(start_addr, 128, buf) == FALSE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#else
    MS_U8 i;

    for(i=0;i<10;i++)
    {
        i2cSetSDA_Chk(PIN_HIGH);
        i2cSetSCL_Chk(PIN_HIGH);
        i2c_Delay();
        i2cSetSDA_Chk(PIN_LOW);
        i2cSetSCL_Chk(PIN_LOW);
        i2c_Delay();
    }
    i2cSetSDA_Chk(PIN_HIGH);
    i2cSetSCL_Chk(PIN_HIGH);
    return TRUE;
#endif
}

#if 0
void TestI2C(void)
{
    MS_U8 data[16], i;
//   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,

    while(1)
    {
        for(i=0;i<12;i++)
        {
            i2c_Start();
            i2c_SendByte(HDMITX_EDIDROM_SLAVE_ADDR);    //write
            i2c_SendByte(i);
            i2c_Stop();

            i2c_Start();
            i2c_SendByte(HDMITX_EDIDROM_SLAVE_ADDR+1);  //read
            data[i] = i2c_ReceiveByte(0);
            i2c_Stop();
        }

        DrvHdmitxOsMsSleep(5);


        DBG_HDMITX(printf("TestI2C()-->  "));

        for(i=0;i<12;i++)
            DBG_HDMITX(printf("  %x", data[i]));

        DBG_HDMITX(printf("\n"));
    }

}
#endif


//------------------------------------------------------------------------------
/// @brief This routine reads HDMI Register
/// @param[in] bank register bank
/// @param[in] address register address
/// @return register value
//------------------------------------------------------------------------------
MS_U16 MHal_HDMITx_Read(MS_U32 bank, MS_U16 address)
{
    return(REG(bank, address));
}

MS_U8 MHal_HDMITx_ReadByte(MS_U32 bank, MS_U16 address)
{
    return (REG8(bank, address));
}
//------------------------------------------------------------------------------
/// @brief This routine writes HDMI Register
/// @param[in] bank register bank
/// @param[in] address register address
/// @param[in] reg_data register data
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_data)
{
    if(g_bDisableRegWrite == TRUE)
    {
        return;
    }
    REG(bank, address) = reg_data;
}

//------------------------------------------------------------------------------
/// @brief This routine writes HDMI Register with mask
/// @param[in] bank register bank
/// @param[in] address register address
/// @param[in] reg_mask mask value
/// @param[in] reg_data register data
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Mask_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data)
{
    MS_U16 reg_value;

    if(g_bDisableRegWrite == TRUE)
    {
        return;
    }
    reg_value = (REG(bank, address) & (~reg_mask)) | (reg_data & reg_mask);
    REG(bank, address) = reg_value;
}


void MHal_HDMITx_Mask_WriteByte(MS_U32 bank, MS_U16 address, MS_U8 reg_mask, MS_U8 reg_data)
{
    MS_U8 reg_value;

    if(g_bDisableRegWrite == TRUE)
    {
        return;
    }
    reg_value = (REG8(bank, address) & (~reg_mask)) | (reg_data & reg_mask);
    REG8(bank, address) = reg_value;
}

//------------------------------------------------------------------------------
// @brief This routine writes bulk HDMI Register
// @param[in] pTable the table of register bank, address, mask and value
// @param[in] num register number
// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_RegsTbl_Write(MSTHDMITX_REG_TYPE *pTable, MS_U8 num)
{
    MS_U8 i;

    for (i = 0; i < num; i++)
    {
        if (pTable->mask != 0xFFFF)
        {
            MHal_HDMITx_Mask_Write(pTable->bank, pTable->address, pTable->mask, pTable->value);
        }
        else
           MHal_HDMITx_Write(pTable->bank, pTable->address, pTable->value);

        pTable++;
    }
}

//------------------------------------------------------------------------------
/// @brief This routine reads PM Register
/// @param[in] bank register bank
/// @param[in] address register address
/// @return register value
//------------------------------------------------------------------------------
MS_U16 MHal_HDMITxPM_Read(MS_U32 bank, MS_U16 address)
{
    return(PMREG(bank, address));
}

//------------------------------------------------------------------------------
/// @brief This routine writes PM Register
/// @param[in] bank register bank
/// @param[in] address register address
/// @param[in] reg_data register data
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITxPM_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_data)
{
    if(g_bDisableRegWrite == TRUE)
    {
        return;
    }
    PMREG(bank, address) = reg_data;
}

//------------------------------------------------------------------------------
/// @brief This routine writes PM Register with mask
/// @param[in] bank register bank
/// @param[in] address register address
/// @param[in] reg_mask mask value
/// @param[in] reg_data register data
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITxPM_Mask_Write(MS_U32 bank, MS_U16 address, MS_U16 reg_mask, MS_U16 reg_data)
{
    MS_U16 reg_value;

    if(g_bDisableRegWrite == TRUE)
    {
        return;
    }
    reg_value = (PMREG(bank, address) & (~reg_mask)) | (reg_data & reg_mask);
    PMREG(bank, address) = reg_value;
}


// Read HDCP key from external EEPROM if not used internal HDCP key
MS_BOOL MHal_HDMITx_HDCPKeyReadByte(MS_U32 u32Addr, MS_U8 *pu8Buf, MS_U32 u32Size)
{
    return (MDrv_EEPROM_Read(u32Addr, pu8Buf, u32Size));
}

void MHal_HDMITx_UtilDebugEnable(MS_BOOL benable)
{
    bDebugUtilFlag = benable;
}

MS_U32 MHal_HDMITx_GetDDCDelayCount(void)
{
    return u32DDCDelayCount;
}

void MHal_HDMITx_SetDDCDelayCount(MS_U32 u32Delay)
{
    u32DDCDelayCount = u32Delay;
}

MS_BOOL MHal_HDMITx_AdjustDDCFreq(MS_U32 u32Speed_K)
{
    #define DELAY_CNT(SpeedKHz)  ((u32FactorDelay/(SpeedKHz))-((u32Parameter1+u32AdjParam)-((SpeedKHz)/u32AdjParam))+((1<<((u32Parameter2-SpeedKHz)/40))))

    MS_U32 u32FactorDelay = 50400UL;
    MS_U32 u32FactorAdjust = 11040UL;
    MS_U32 u32ParamBase1 = 130UL;
    MS_U32 u32Parameter1 = 130UL;
    MS_U32 u32Parameter2 = 440UL;
    MS_U32 u32AdjParam = 0;
    MS_U32 u32CpuSpeedMHz = 0;


    //u32CpuSpeedMHz = ((MS_U32) ((volatile MS_U16*)(_gCoproBase))[0x1EE2]);
    //u32CpuSpeedMHz = u32CpuSpeedMHz*12*4)/1000;

    u32CpuSpeedMHz = DrvHdmitxOsGetCpuSpeedMhz();
    u32FactorDelay = u32CpuSpeedMHz * 100;
    u32FactorAdjust = (u32CpuSpeedMHz>=312)? 10000UL :13000UL;

    if (u32CpuSpeedMHz > 0)
    {
        u32AdjParam = u32FactorAdjust/u32CpuSpeedMHz;
        DBG_HDMIUTIL(printf("[%s][%d] u32AdjParam = 0x%X, u32CpuSpeedMHz = 0x%X \n", __FUNCTION__, __LINE__, u32AdjParam, u32CpuSpeedMHz));
    }
    else
    {
        DBG_HDMIUTIL(printf("%s, Error parameter u32CpuSpeedMHz = 0x%X",__FUNCTION__, u32CpuSpeedMHz));
        return FALSE;
    }

    if (u32AdjParam == 0)
    {
        u32DDCDelayCount = 250;
        return FALSE;
    }

    u32Parameter2 = 440UL;
    //(2) assign base for parameter

    if(u32CpuSpeedMHz>=1000) u32ParamBase1 = 150UL;
    else if(u32CpuSpeedMHz>=900) u32ParamBase1 = 140UL;
    else if(u32CpuSpeedMHz>=780) u32ParamBase1 = 135UL;
    else if(u32CpuSpeedMHz>=720) u32ParamBase1 = 130UL;
    else if(u32CpuSpeedMHz>=650) u32ParamBase1 = 125UL;
    else if(u32CpuSpeedMHz>=600) u32ParamBase1 = 110UL;
    else if(u32CpuSpeedMHz>=560) u32ParamBase1 = 100UL;
    else if(u32CpuSpeedMHz>=530) u32ParamBase1 = 95UL;
    else if(u32CpuSpeedMHz>=500) u32ParamBase1 = 90UL;
    else if(u32CpuSpeedMHz>=480) u32ParamBase1 = 85UL;
    else if(u32CpuSpeedMHz>=430) u32ParamBase1 = 80UL;
    else if(u32CpuSpeedMHz>=400) u32ParamBase1 = 75UL;
    else if(u32CpuSpeedMHz>=384) u32ParamBase1 = 70UL;
    else if(u32CpuSpeedMHz>=360) u32ParamBase1 = 65UL;
    else if(u32CpuSpeedMHz>=336) u32ParamBase1 = 60UL;
    else if(u32CpuSpeedMHz>=312) u32ParamBase1 = 40UL;
    else if(u32CpuSpeedMHz>=240) u32ParamBase1 = 10UL;
    else if(u32CpuSpeedMHz>=216) u32ParamBase1 = 0UL;
    else u32ParamBase1 = 0UL;
    //(3) compute parameter 1 by base
    if(u32Speed_K>=100)
    {
        u32Parameter1 = u32ParamBase1 + 250; //100K level
    }
    else if (u32Speed_K>=75)
    {
        u32Parameter1 = u32ParamBase1 + 340; //75K level
    }
    else if (u32Speed_K>=50)
    {
        u32Parameter1 = u32ParamBase1 + 560; //50K level
    }
    else
    {
        u32Parameter1 = u32ParamBase1 + 860; //30K level
    }

    //u32DDCDelayCount = 250;
    DBG_HDMIUTIL(printf("[%s][%d] u32Speed_K = 0x%X , u32AdjParam = 0x%X \n", __FUNCTION__, __LINE__, u32Speed_K, u32AdjParam));

    //(4) compute delay counts
    if ((u32Speed_K>0) && (u32AdjParam>0))
    {
        u32DDCDelayCount = DELAY_CNT(u32Speed_K);
        DBG_HDMIUTIL(printf("[%s][%d] u32DDCDelayCount = 0x%X \n", __FUNCTION__, __LINE__, u32DDCDelayCount));
    }
    else
    {
        DBG_HDMIUTIL(printf("[%s][%d] Error parameter u32Speed_K = 0x%X , u32AdjParam = 0x%X",__FUNCTION__,  __LINE__, u32Speed_K, u32AdjParam));
        return FALSE;
    }
    return TRUE;
}

//////////////////////////////////// Wilson@Kano: SCDC relative
//**************************************************************************
//  [Function Name]:
//                      Mhal_HDMITx_SCDCSetTmdsConfig
//  [Description]:
//                      config clock divide ratio for HDMI 2.0 timing, set scramble bit
//  [Arguments]:
//                      [MS_BOOL] bClkRatio
//                      [MS_BOOL] bScrambleEn
//  [Return]:
//                      void
//**************************************************************************
void Mhal_HDMITx_SCDCSetTmdsConfig(MS_BOOL bClkRatio, MS_BOOL bScrambleEn)
{
	MS_U8 ucTmpVal = (bScrambleEn == TRUE)? 0x01 : 0x00;

	ucTmpVal |= ((bClkRatio == TRUE) ? 0x02 : 0x00); //bClkRatio == TRUE, then (TMDS bit period) / (TMDS clock period) = 1/40; otherwise 1/10

	i2cBurstWriteBytes(HDMITX_SCDC_SLAVE_ADDR, E_SCDC_TMDS_CONFIG_IDX, &ucTmpVal, 1);
}

//**************************************************************************
//  [Function Name]:
//                      Mhal_HDMITx_SCDCAccessField
//  [Description]:
//                      General function for read/write operation of any SCDC field
//  [Arguments]:
//                      [enMsHDMITX_SCDC_FIELD_OFFSET_LIST] enField;
//                      [MS_BOOL] bReadAction;
//                      [MS_U8] *pucData;
//                      [MS_U8] ucLen;
//  [Return]:
//                      MS_BOOL
//**************************************************************************
MS_BOOL Mhal_HDMITx_SCDCAccessField(enMsHDMITX_SCDC_FIELD_OFFSET_LIST enField, MS_BOOL bReadAction, MS_U8* pucData, MS_U8 ucLen)
{
	if (bReadAction)
	{
		return i2cBurstReadBytes(HDMITX_SCDC_SLAVE_ADDR, enField, pucData, (MS_U16)ucLen);
	}
	else
	{
		return i2cBurstWriteBytes(HDMITX_SCDC_SLAVE_ADDR, enField, pucData, (MS_U16)ucLen);
	}
}

MS_BOOL Mhal_HDMITx_GetSwI2cEn(void)
{
    return gbSwI2cEn;
}

MS_U8 Mhal_HDMITx_GetSwI2cSdaPin(void)
{
    return gu8SwI2cSdaPin;
}

MS_U8 Mhal_HDMITx_GetSwI2cSclPin(void)
{
    return gu8SwI2cSclPin;
}
