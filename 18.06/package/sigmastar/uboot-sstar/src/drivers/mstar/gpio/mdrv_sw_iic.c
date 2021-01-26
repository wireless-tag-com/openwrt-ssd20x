/*
* mdrv_sw_iic.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: giggshuang <giggshuang@sigmastar.com.tw>
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

#include <common.h>
#include <command.h>
#include <MsDebug.h>

#include "drvGPIO.h"
#include "mhal_gpio.h"
#include <mdrv_sw_iic.h>
//#include "mdrv_sw_iic.h"


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
//#define OPEN_SWI2C_DEBUG

#ifdef OPEN_SWI2C_DEBUG
#define swi2cDbg  printf
#else
#define swi2cDbg(...)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

// for SW I2C
#define _INPUT                  1
#define _OUTPUT                 0
#define _HIGH                   1
#define _LOW                    0
#define SWIIC_READ              0
#define SWIIC_WRITE             1
#define I2C_CHECK_PIN_DUMMY     3200 /*6000*//*3200*/
#define I2C_ACKNOWLEDGE         _LOW
#define I2C_NON_ACKNOWLEDGE     _HIGH

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
I2C_BusCfg_t g_I2CBusCfg[SW_IIC_NUM_OF_MAX];
int gpioi2c_delay_us = 20; //default
int access_dummy_time = 1; //default

#define SWIIC_SCL_PIN(chNum, x) \
    ((x == _INPUT) ? MDrv_GPIO_Pad_Odn(g_I2CBusCfg[chNum].u8PadSCL) : MDrv_GPIO_Pad_Oen(g_I2CBusCfg[chNum].u8PadSCL))

#define SWIIC_SDA_PIN(chNum, x) \
    ((x == _INPUT) ? MDrv_GPIO_Pad_Odn(g_I2CBusCfg[chNum].u8PadSDA) : MDrv_GPIO_Pad_Oen(g_I2CBusCfg[chNum].u8PadSDA))

#define SWIIC_SCL_OUT(chNum, x) \
    ((x == _HIGH) ? MDrv_GPIO_Pull_High(g_I2CBusCfg[chNum].u8PadSCL) : MDrv_GPIO_Pull_Low(g_I2CBusCfg[chNum].u8PadSCL))

#define SWIIC_SDA_OUT(chNum, x) \
    ((x == _HIGH) ? MDrv_GPIO_Pull_High(g_I2CBusCfg[chNum].u8PadSDA) : MDrv_GPIO_Pull_Low(g_I2CBusCfg[chNum].u8PadSDA));

#define GET_SWIIC_SCL(chNum) MDrv_GPIO_Pad_Read(g_I2CBusCfg[chNum].u8PadSCL)

#define GET_SWIIC_SDA(chNum) MDrv_GPIO_Pad_Read(g_I2CBusCfg[chNum].u8PadSDA)

//#define SWII_DELAY(chNum)    (_I2CBus[chNum].DefDelay)

#define _SDA_HIGH(chNum)     SWIIC_SDA_PIN(chNum, _INPUT)
#define _SDA_LOW(chNum)     do { SWIIC_SDA_OUT(chNum, _LOW); SWIIC_SDA_PIN(chNum, _OUTPUT); } while(0)

#define _SCL_HIGH(chNum)     SWIIC_SCL_PIN(chNum, _INPUT)
#define _SCL_LOW(chNum)     do { SWIIC_SCL_OUT(chNum, _LOW); SWIIC_SCL_PIN(chNum, _OUTPUT); } while(0)


#define DISABLE 0
#define ENABLE 1

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void MDrv_SW_IIC_Delay(U8 u8ChIIC)
{
/*
    //U32 volatile u32Loop = DELAY_CYCLES(g_I2CBusCfg[u8ChIIC].u16SpeedKHz) / 2;
    U32 volatile u32Loop=gpioi2c_delay_param;

    while(u32Loop--)
    {
        __asm__ __volatile__ ("mov r0, r0");
    }
*/
    udelay(gpioi2c_delay_us);

}

void MDrv_SW_IIC_SCL(U8 u8ChIIC, U8 u8Data)
{
    if ( u8Data == _HIGH )
    {
        SWIIC_SCL_PIN(u8ChIIC, _INPUT);  //set to input
    }
    else
    {
        SWIIC_SCL_OUT(u8ChIIC, _LOW);
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
    }
}

void MDrv_SW_IIC_SDA(U8 u8ChIIC, U8 u8Data)
{
    if ( u8Data == _HIGH )
    {
        SWIIC_SDA_PIN(u8ChIIC, _INPUT);  //set to input
    }
    else
    {
        //printf(KERN_INFO "SWIIC_SDA_OUT(u8ChIIC, _LOW)\r\n");
        SWIIC_SDA_OUT(u8ChIIC, _LOW);
        SWIIC_SDA_PIN(u8ChIIC, _OUTPUT); //set to output
    }
}

void MDrv_SW_IIC_SCL_Chk(U8 u8ChIIC, U16 bSet)
{
    U16 u16Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SCL_PIN(u8ChIIC, _INPUT);
        u16Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy

        while ((GET_SWIIC_SCL(u8ChIIC) == _LOW) && (u16Dummy--));
    }
    else
    {
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);    // set SCL pin
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
    }
}

void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, U16 bSet)
{
    U16 u16Dummy;       // loop dummy

    if (bSet == _HIGH)  // if set pin high
    {
        SWIIC_SDA_PIN(u8ChIIC, _INPUT);
        u16Dummy = I2C_CHECK_PIN_DUMMY; // initialize dummy
        while ((GET_SWIIC_SDA(u8ChIIC) == _LOW) && (u16Dummy--));// check SDA pull high
    }
    else
    {
        MDrv_SW_IIC_SDA(u8ChIIC, _LOW);    // set SDA pin
        SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
    }
}

//-------------------------------------------------------------------------------------------------
// SW I2C: start signal.
// <comment>
//  SCL ________
//              \_________
//  SDA _____
//           \____________
//
// Return value: None
//-------------------------------------------------------------------------------------------------
U16 MDrv_SW_IIC_Start(U8 u8ChIIC)
{

    U16 bStatus = TRUE;    // success status

    //printf(KERN_INFO "u8ChIIC = %d \r\n", u8ChIIC);
    MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);
    MDrv_SW_IIC_Delay(u8ChIIC);
    // check pin error
    SWIIC_SCL_PIN(u8ChIIC, _INPUT);
    SWIIC_SDA_PIN(u8ChIIC, _INPUT);
    if ((GET_SWIIC_SCL(u8ChIIC) == _LOW) || (GET_SWIIC_SDA(u8ChIIC) == _LOW))
    {
        SWIIC_SCL_PIN(u8ChIIC, _OUTPUT);
        SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
        bStatus = FALSE;
    }
    else // success
    {
        MDrv_SW_IIC_SDA(u8ChIIC, _LOW);
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);
    }

    return bStatus;     //vain
}

//-------------------------------------------------------------------------------------------------
// SW I2C: stop signal.
// <comment>
//              ____________
//  SCL _______/
//                 _________
//  SDA __________/
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Stop(U8 u8ChIIC)
{
    _SCL_LOW(u8ChIIC);

    MDrv_SW_IIC_Delay(u8ChIIC);
    _SDA_LOW(u8ChIIC);

    MDrv_SW_IIC_Delay(u8ChIIC);
//    _SCL_HIGH(u8ChIIC);
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);    // <20091212 takerest> fix SCL pin error
    MDrv_SW_IIC_Delay(u8ChIIC);
//    _SDA_HIGH(u8ChIIC);
    MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);    // <20091212 takerest> fix SDA pin error
    MDrv_SW_IIC_Delay(u8ChIIC);
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data
///@param u8data \b IN: 1 byte data to send
//-------------------------------------------------------------------------------------------------
static U16 _IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack )   // Be used int IIC_SendByte
{
    U8      u8Mask = 0x80;
    U16     bAck; // acknowledge bit

    while ( u8Mask )
    {
        if (u8data & u8Mask)
        {
            MDrv_SW_IIC_SDA_Chk(u8ChIIC, _HIGH);
        }
        else
        {
            MDrv_SW_IIC_SDA_Chk(u8ChIIC, _LOW);
        }

        MDrv_SW_IIC_Delay(u8ChIIC);
        #if 0//(EXTRA_DELAY_CYCLE)
        if(SWII_DELAY(u8ChIIC) == 2)
        {
            MDrv_SW_IIC_DelayEx(u8ChIIC, 8);
        }
        #endif
        MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH); // clock
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);
        u8Mask >>= 1; // next
    }

    // recieve acknowledge
    SWIIC_SDA_PIN(u8ChIIC, _INPUT);
    if( u8Delay4Ack > 0 )
    {
        udelay( u8Delay4Ack );
    }
    else
    {
        MDrv_SW_IIC_Delay(u8ChIIC);
    }
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);

    bAck = GET_SWIIC_SDA(u8ChIIC); // recieve acknowlege
//    SWIIC_SDA(u8ChIIC, bAck);     //for I2c waveform sharp
//    SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL(u8ChIIC, _LOW);

    MDrv_SW_IIC_Delay(u8ChIIC);

    MDrv_SW_IIC_SDA(u8ChIIC, (U8)bAck);     //for I2c waveform sharp
    SWIIC_SDA_PIN(u8ChIIC, _OUTPUT);

    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);

    return (bAck);
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Send 1 bytes data, this function will retry 5 times until success.
///@param u8data \b IN: 1 byte data to send
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
U16 MDrv_SW_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack )
{
    U8 u8I;

    //printf(KERN_INFO "send byte u8data = 0x%x \r\n", u8data);

    for(u8I=0;u8I<access_dummy_time;u8I++)
    {
        if (_IIC_SendByte(u8ChIIC, u8data, u8Delay4Ack) == I2C_ACKNOWLEDGE)
            return TRUE;
    }

    //printf("IIC write byte 0x%02x fail!!\n", u8data);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// SW I2C: access start.
//
// Arguments: u8SlaveAdr - slave address
//            u8Trans - I2C_TRANS_WRITE/I2C_TRANS_READ
//-------------------------------------------------------------------------------------------------
U16 MDrv_SW_IIC_AccessStart(U8 u8ChIIC, U8 u8SlaveAdr, U8 u8Trans)
{
    U8 u8Dummy; // loop dummy
    U8 u8Delay4Ack = 0;

    if (u8Trans == SWIIC_READ) // check i2c read or write
    {
        u8SlaveAdr = u8SlaveAdr | 0x01; // read
    }
    else
    {
        u8SlaveAdr = u8SlaveAdr & 0xfe; // write
    }

    u8Dummy = access_dummy_time;

    while (u8Dummy--)
    {
        if ( MDrv_SW_IIC_Start(u8ChIIC) == FALSE)
        {
            //printf("MDrv_SW_IIC_Start=>Failed\n");
            continue;
        }

        if ( MDrv_SW_IIC_SendByte(u8ChIIC, u8SlaveAdr, u8Delay4Ack) == TRUE )   //I2C_ACKNOWLEDGE) // check acknowledge
        {
            return TRUE;
        }
        MDrv_SW_IIC_Stop(u8ChIIC);
    }

    //printf(KERN_INFO "MDrv_SW_IIC_SendByte u8SlaveAdr = 0x%x\r\n", u8SlaveAdr);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Get 1 bytes data
///@param u16Ack  \b IN: acknowledge
///@return U8:    \b OUT: get data from the device
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
U8 MDrv_SW_IIC_GetByte (U8 u8ChIIC, U16 u16Ack)
{
    U8 u8Receive = 0;
    U8 u8Mask = 0x80;

    SWIIC_SDA_PIN(u8ChIIC, _INPUT);

    while ( u8Mask )
    {
        MDrv_SW_IIC_Delay(u8ChIIC);
        MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);
        MDrv_SW_IIC_Delay(u8ChIIC);

        if (GET_SWIIC_SDA(u8ChIIC) == _HIGH)
        {
            u8Receive |= u8Mask;
        }
        u8Mask >>= 1; // next

        MDrv_SW_IIC_SCL(u8ChIIC, _LOW);

        #if 0//(EXTRA_DELAY_CYCLE)
        if(SWII_DELAY(u8ChIIC) == 2)
        {
            MDrv_SW_IIC_Delay(u8ChIIC);
            MDrv_SW_IIC_Delay(u8ChIIC);
            MDrv_SW_IIC_DelayEx(u8ChIIC, 8);
        }
        #endif
    }
    if (u16Ack)
    {
        // acknowledge
        MDrv_SW_IIC_SDA_Chk(u8ChIIC, I2C_ACKNOWLEDGE);
    }
    else
    {
        // non-acknowledge
        MDrv_SW_IIC_SDA_Chk(u8ChIIC, I2C_NON_ACKNOWLEDGE);
    }
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL_Chk(u8ChIIC, _HIGH);
    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_SCL(u8ChIIC, _LOW);
    MDrv_SW_IIC_Delay(u8ChIIC);


    MDrv_SW_IIC_Delay(u8ChIIC);
    MDrv_SW_IIC_Delay(u8ChIIC);

    return u8Receive;
}

//-------------------------------------------------------------------------------------------------
///SW I2C: Write bytes, be able to write 1 byte or several bytes to several register offsets in same slave address.
///@param u8SlaveID \b IN: Slave ID (Address)
///@param u8AddrCnt \b IN:  register NO to write, this parameter is the NO of register offsets in pu8addr buffer,
///it should be 0 when *pu8Addr = NULL.
///@param *pu8Addr \b IN: pointer to a buffer containing target register offsets to write
///@param u32BufLen \b IN: Data length (in byte) to write
///@param *pu8Buf \b IN: pointer to the data buffer for write
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf)
{
    U8  u8Dummy; // loop dummy
    S32 s32RetCountIIC;

    //check if sw i2c channel is disabled
    if(g_I2CBusCfg[u8ChIIC].u8Enable == DISABLE)
        return -1;

    printf("%s:u8ChIIC=0x%02x\n",__FUNCTION__,u8ChIIC);
    printf("%s:u8SlaveID=0x%02x\n",__FUNCTION__,u8SlaveID);
    printf("%s:u8AddrCnt=0x%02x\n",__FUNCTION__,u8AddrCnt);
    printf("%s:pu8Addr=0x%02x\n",__FUNCTION__,*pu8Addr);
    printf("%s:u32BufLen=0x%02x\n",__FUNCTION__,u32BufLen);

    u8Dummy = access_dummy_time;
    s32RetCountIIC = u32BufLen;
    #ifdef SWI2C_LOCK
    MDrv_SW_IIC_Lock();
    #endif

    //printf(KERN_INFO "u8AddrCnt = %d u32BufLen = %d\r\n" , u8AddrCnt, u32BufLen);
    while (u8Dummy--)
    {
        if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_WRITE) == FALSE)
        {
            s32RetCountIIC = -2;
            swi2cDbg(":write slave address failed,err=%d\r\n",s32RetCountIIC);
            //OALMSG(1, (L":write slave address failed,err=%d\r\n",s32RetCountIIC));

            goto SW_IIC_Write_End;
        }

        while( u8AddrCnt )
        {
              u8AddrCnt--;
              if ( MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Addr, 0) == FALSE )
              {
                s32RetCountIIC = -3;
                swi2cDbg(":write register address failed,err=%d\r\n",s32RetCountIIC);
                goto SW_IIC_Write_End;
              }
              pu8Addr++;
        }
        while (u32BufLen) // loop of writting data
        {
            u32BufLen-- ;
            if ( MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Buf, 0) == FALSE )
            {
                s32RetCountIIC = -4;
                swi2cDbg(":write data failed,err=%d\r\n",s32RetCountIIC);
                goto SW_IIC_Write_End;
            }
            pu8Buf++; // next byte pointer
        }

        break;
    }

SW_IIC_Write_End:
    MDrv_SW_IIC_Stop(u8ChIIC);

    #ifdef SWI2C_LOCK
    MDrv_SW_IIC_UnLock();
    #endif
    return s32RetCountIIC;
}


//-------------------------------------------------------------------------------------------------
///SW I2C: Read bytes, be able to read 1 byte or several bytes from several register offsets in same slave address.
///@param u8SlaveID \b IN: Slave ID (Address)
///@param u8AddrCnt \b IN:  register NO to read, this parameter is the NO of register offsets in pu8addr buffer,
///it should be 0 when *paddr = NULL.
///@param *pu8Addr \b IN: pointer to a buffer containing target register offsets to read
///@param u32BufLen \b IN: Data length (in byte) to read
///@param *pu8Buf \b IN: pointer to retun data buffer.
///@return BOOLEAN:
///- TRUE: Success
///- FALSE: Fail
//-------------------------------------------------------------------------------------------------
S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf)
{
    U8  u8Dummy; // loop dummy
    S32 s32RetCountIIC;

    //check if sw i2c channel is disabled
    if(g_I2CBusCfg[u8ChIIC].u8Enable == DISABLE)
        return -1;

    printf("%s:u8ChIIC=0x%02x\n",__FUNCTION__,u8ChIIC);
    printf("%s:u8SlaveID=0x%02x\n",__FUNCTION__,u8SlaveID);
    printf("%s:u8AddrCnt=0x%02x\n",__FUNCTION__,u8AddrCnt);
    printf("%s:pu8Addr=0x%02x\n",__FUNCTION__,*pu8Addr);
    printf("%s:u32BufLen=0x%02x\n",__FUNCTION__,u32BufLen);
    
    u8Dummy = access_dummy_time;
    s32RetCountIIC = u32BufLen;
    #ifdef SWI2C_LOCK
    MDrv_SW_IIC_Lock();
    #endif

    while (u8Dummy--)
    {
        if( u8AddrCnt > 0 )
        {
            if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_WRITE) == FALSE)
            {
                s32RetCountIIC = -2;
                swi2cDbg(":write slave address failed,err=%d\r\n",s32RetCountIIC);
                //printf(KERN_INFO "%s:write slave address failed,err=%d\n",__FUNCTION__,s32RetCountIIC);
                goto SW_IIC_Read_End;
            }

            while( u8AddrCnt )
            {
                u8AddrCnt--;
                //printf(KERN_INFO "%s:pu8Addr=0x%02x\n",__FUNCTION__,*pu8Addr);
                if (MDrv_SW_IIC_SendByte(u8ChIIC, *pu8Addr, 0) == FALSE)
                {
                    s32RetCountIIC = -3;
                    swi2cDbg(":write register address failed,err=%d\r\n",s32RetCountIIC);
                    //printf(KERN_INFO "%s:write register address failed,err=%d\n",__FUNCTION__,s32RetCountIIC);
                    goto SW_IIC_Read_End;
                }
                pu8Addr++;
            }
        }

        if (MDrv_SW_IIC_AccessStart(u8ChIIC, u8SlaveID, SWIIC_READ) == FALSE)
        {
            s32RetCountIIC = -4;
            swi2cDbg(":write slave address failed,err=%d\r\n",s32RetCountIIC);
            //printf(KERN_INFO "%s:write slave address failed,err=%d\n",__FUNCTION__,s32RetCountIIC);
            goto SW_IIC_Read_End;
        }

        while (u32BufLen--) // loop to burst read
        {
            *pu8Buf = MDrv_SW_IIC_GetByte(u8ChIIC, (U16)u32BufLen); // receive byte
             //printf(KERN_INFO "%s:pu8Buf=0x%02x\n",__FUNCTION__,*pu8Buf);
             pu8Buf++; // next byte pointer
        }

        break;
    }

SW_IIC_Read_End:
    MDrv_SW_IIC_Stop(u8ChIIC);

    #ifdef SWI2C_LOCK
    MDrv_SW_IIC_UnLock();
    #endif
    return s32RetCountIIC;
}


//-------------------------------------------------------------------------------------------------
/// IIC Set Speed by Index
/// @param  u8ChIIC            \b IN:  channel index
/// @param  u8Speed            \b IN:  u8Speed index
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed)
{

    //switch(SWII_DELAY(u8ChIIC))
    switch(u8Speed)
    {
        case 1:
            g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 100; //KHz
            break;
        case 2:
            g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 200; //KHz
            break;
        case 3:
            g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 300; //KHz
            break;
        case 4:
            g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 400; //KHz
            break;
        default:
            g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 100; //KHz
            break;
    }

}


//-------------------------------------------------------------------------------------------------
/// IIC Bus Enable or Disable
/// @param  u8ChIIC            \b IN:  channel index
/// @param  bEnable            \b IN:  enable
/// @return None
//-------------------------------------------------------------------------------------------------
void MDrv_SW_IIC_Enable(U8 u8ChIIC, U8 bEnable )
{
    g_I2CBusCfg[u8ChIIC].u8Enable = bEnable;
}


//-------------------------------------------------------------------------------------------------
/// IIC Bus Configuration
/// @param pBusCfg            \b IN:  clock selection
/// @return None
//-------------------------------------------------------------------------------------------------
U16 MDrv_SW_IIC_ConfigBus(I2C_BusCfg_t* pBusCfg)
{
    U8 u8ChIIC;


    swi2cDbg("[%s] Channel index = %d \r\n",__FUNCTION__,pBusCfg->u8ChIdx);
    swi2cDbg("[%s] Channel PadSCL = %d \r\n",__FUNCTION__,pBusCfg->u8PadSCL);
    swi2cDbg("[%s] Channel PadSDA = %d \r\n",__FUNCTION__,pBusCfg->u8PadSDA);
    swi2cDbg("[%s] Channel SpeedKHz = %d \r\n",__FUNCTION__,pBusCfg->u16SpeedKHz);
    swi2cDbg("[%s] Channel Enable = %d \r\n",__FUNCTION__,pBusCfg->u8Enable);
    u8ChIIC = pBusCfg->u8ChIdx;
    g_I2CBusCfg[u8ChIIC].u8ChIdx = pBusCfg->u8ChIdx;
    g_I2CBusCfg[u8ChIIC].u8PadSCL = pBusCfg->u8PadSCL;
    g_I2CBusCfg[u8ChIIC].u8PadSDA = pBusCfg->u8PadSDA;
    g_I2CBusCfg[u8ChIIC].u16SpeedKHz = pBusCfg->u16SpeedKHz;
    g_I2CBusCfg[u8ChIIC].u8Enable = pBusCfg->u8Enable;
    pBusCfg++;

    return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// IIC master initialization
/// @return None
/// @note   Hardware IIC. Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_IIC_Init(void)
{
    U8 u8ChIIC;
    for(u8ChIIC=0; u8ChIIC<SW_IIC_NUM_OF_MAX; u8ChIIC++)
    {
        g_I2CBusCfg[u8ChIIC].u8ChIdx = 0xFF;
        g_I2CBusCfg[u8ChIIC].u16SpeedKHz = 100; //KHz
        g_I2CBusCfg[u8ChIIC].u8Enable = DISABLE;
    }
}
