/*
* hal_card_regs.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: truman.yang <truman.yang@sigmastar.com.tw>
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

/***************************************************************************************************************
 *
 * FileName hal_card_regs.h
 *     @author jeremy.wang (2015/08/12)
 * Desc:
 * 	   This file is the header file of hal_card_regs.c.
 *
 * 	   For Base RegisterSetting:
 * 	   (1) BASE Register Address
 *	   (2) BASE Register Operation
 * 	   (3) BASE FCIE Reg Meaning Position (Including File)
 *
 *	   P.S. If you want to use only IP for single card or dual cards,
 * 			please modify FCIE1 and FCIE2 setting to the same reg position.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_REGS_H
#define __HAL_CARD_REGS_H

#include "hal_card_base.h"

//***********************************************************************************************************
// (1) BASE Register Address
//***********************************************************************************************************


//###########################################################################################################
#if (D_PROJECT == D_PROJECT__C3)
//###########################################################################################################

#define A_RIU_BASE			(0x1F000000)

#define A_FCIE1_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9000) 		// SDIO0_0_BANK
#define A_FCIE1_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9080)     	// SDIO0_1_BANK
#define A_FCIE1_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9100)     	// SDIO0_2_BANK

#define A_FCIE2_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9400) 		// SDIO2_0_BANK
#define A_FCIE2_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9480)     	// SDIO2_1_BANK
#define A_FCIE2_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9500)     	// SDIO2_2_BANK

#define A_FCIE3_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9200) 		// SDIO1_0_BANK
#define A_FCIE3_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9280)     	// SDIO1_1_BANK
#define A_FCIE3_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0xA9300)     	// SDIO1_2_BANK

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__C4)
//###########################################################################################################

#define A_RIU_BASE			(0x1F000000)

#define A_FCIE1_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A00) 		// SDIO0_0_BANK
#define A_FCIE1_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A80)     	// SDIO0_1_BANK
#define A_FCIE1_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0B00)     	// SDIO0_2_BANK

#define A_FCIE2_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A00)
#define A_FCIE2_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A80)
#define A_FCIE2_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0B00)

#define A_FCIE3_0_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A00)
#define A_FCIE3_1_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0A80)
#define A_FCIE3_2_BANK		GET_CARD_REG_ADDR(A_RIU_BASE, 0x0B00)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF)
//###########################################################################################################

#define A_RIU_BASE		    (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //FCIE0_0_BANK
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)      //FCIE0_1_BANK
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)      //FCIE0_2_BANK

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF3)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //FCIE0_0_BANK
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)      //FCIE0_1_BANK
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)      //FCIE0_2_BANK

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF5)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK 1410h
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK 1411h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK 1412h

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //FCIE0_0_BANK 1402h
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)      //FCIE0_1_BANK 1403h
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)      //FCIE0_2_BANK 1404h

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0980)      //SDIO0_0_BANK 1413h
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A00)      //SDIO0_1_BANK 1414h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A80)      //SDIO0_2_BANK 1415h

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //FCIE0_0_BANK 1402h
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)      //FCIE0_1_BANK 1403h
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)      //FCIE0_2_BANK 1404h

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF2m)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK 1410h
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK 1411h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK 1412h

#define A_FCIE2_0_BANK      A_FCIE1_0_BANK
#define A_FCIE2_1_BANK      A_FCIE1_1_BANK
#define A_FCIE2_2_BANK      A_FCIE1_2_BANK

#define A_FCIE3_0_BANK      A_FCIE1_0_BANK
#define A_FCIE3_1_BANK      A_FCIE1_1_BANK
#define A_FCIE3_2_BANK      A_FCIE1_2_BANK

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6E)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0980)      //SDIO0_0_BANK 1413h
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A00)      //SDIO0_1_BANK 1414h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A80)      //SDIO0_2_BANK 1415h

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK 1410h
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK 1411h
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK 1412h

#define A_FCIE3_0_BANK      A_FCIE1_0_BANK
#define A_FCIE3_1_BANK      A_FCIE1_1_BANK
#define A_FCIE3_2_BANK      A_FCIE1_2_BANK

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6B0)
//###########################################################################################################

#define A_RIU_BASE          (0x1F000000)

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0980)      //SDIO0_0_BANK 1413h (SD on i6)
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A00)      //SDIO0_1_BANK 1414h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A80)      //SDIO0_2_BANK 1415h

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //FCIE0_0_BANK 1410h (SD1 on i6)
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //FCIE0_1_BANK 1411h
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //FCIE0_2_BANK 1412h

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

//###########################################################################################################
#endif


//***********************************************************************************************************
// (2) BASE Register Operation
//***********************************************************************************************************

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
    //#if (D_PROJECT == D_PROJECT__C3) || (D_PROJECT == D_PROJECT__C4) || (D_PROJECT == D_PROJECT__iNF)
        #include "../../../mstar/include/ms_platform.h"  //IO_ADDRESS       // IO Mapping Address
		#define IO_MAPADDR(Reg_Addr)	IO_ADDRESS(Reg_Addr)
    //#endif
//###########################################################################################################
#else
//###########################################################################################################
#define IO_MAPADDR(Reg_Addr)	Reg_Addr
//###########################################################################################################
#endif

#define D_MIU_WIDTH                            8        // Special MIU WIDTH for FCIE4
#define REG_OFFSET_BITS                        2	// Register Offset Byte  (2= 4byte = 32bits)
#define GET_CARD_REG_ADDR(x, y)                ((x)+((y) << REG_OFFSET_BITS))

#define CARD_REG(Reg_Addr)                     (*(volatile U16_T*)(IO_MAPADDR(Reg_Addr)) )
#define CARD_REG_L8(Reg_Addr)                  (*(volatile U8_T*)(IO_MAPADDR(Reg_Addr)) )
#define CARD_REG_H8(Reg_Addr)                  (*( (volatile U8_T*)(IO_MAPADDR(Reg_Addr))+1) )

#define CARD_REG_SETBIT(Reg_Addr, Value)       CARD_REG(Reg_Addr) |= (Value)
#define CARD_REG_CLRBIT(Reg_Addr, Value)       CARD_REG(Reg_Addr) &= (~(Value))

#define CARD_REG_L8_SETBIT(Reg_Addr, Value)    CARD_REG_L8(Reg_Addr) |= (Value)
#define CARD_REG_H8_SETBIT(Reg_Addr, Value)    CARD_REG_H8(Reg_Addr) |= (Value)
#define CARD_REG_L8_CLRBIT(Reg_Addr, Value)    CARD_REG_L8(Reg_Addr) &= (~(Value))
#define CARD_REG_H8_CLRBIT(Reg_Addr, Value)    CARD_REG_H8(Reg_Addr) &= (~(Value))

#define CARD_BANK(Bank_Addr)                   IO_MAPADDR(Bank_Addr)

volatile void* Hal_CREG_GET_REG_BANK(IPEmType eIP, U8_T u8Bank);
void Hal_CREG_SET_PORT(IPEmType eIP, PortEmType ePort);
volatile PortEmType Hal_CREG_GET_PORT(IPEmType eIP);

#define GET_CARD_BANK                          Hal_CREG_GET_REG_BANK
#define GET_CARD_PORT                          Hal_CREG_GET_PORT
#define SET_CARD_PORT                          Hal_CREG_SET_PORT

//***********************************************************************************************************
// (3) BASE FCIE Reg Meaning Position (Including File)
//***********************************************************************************************************

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__04)
//###########################################################################################################
#include "hal_card_regs_v4.h"
//###########################################################################################################
#elif (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
#include "hal_card_regs_v5.h"
//###########################################################################################################
#endif


#endif //End of __HAL_CARD_REGS_H


