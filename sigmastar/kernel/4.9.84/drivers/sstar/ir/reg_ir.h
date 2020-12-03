/*
* reg_ir.h- Sigmastar
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
#ifndef _REG_IR_H_
#define _REG_IR_H_

#define REG_IR_CTRL             	     0x40
#define REG_IR_HDC_UPB          		 0x41
#define REG_IR_HDC_LOB          		 0x42
#define REG_IR_OFC_UPB          		 0x43
#define REG_IR_OFC_LOB          		 0x44
#define REG_IR_OFC_RP_UPB       		 0x45
#define REG_IR_OFC_RP_LOB       		 0x46
#define REG_IR_LG01H_UPB        		 0x47
#define REG_IR_LG01H_LOB        		 0x48
#define REG_IR_LG0_UPB          		 0x49
#define REG_IR_LG0_LOB          		 0x4A
#define REG_IR_LG1_UPB          		 0x4B
#define REG_IR_LG1_LOB          		 0x4C
#define REG_IR_SEPR_UPB         		 0x4D
#define REG_IR_SEPR_LOB         		 0x4E
#define REG_IR_TIMEOUT_CYC_L    		 0x4F
#define REG_IR_TIMEOUT_CYC_H             0x50
    #define IR_CCB_CB                    0x9F00//ir_ccode_byte:1+ir_code_bit_num:32
#define REG_IR_SEPR_BIT_FIFO_CTRL        0x51
#define REG_IR_CCODE            		 0x52
#define REG_IR_GLHRM_NUM        		 0x53
#define REG_IR_CKDIV_NUM_KEY_DATA        0x54
#define REG_IR_SHOT_CNT_L       		 0x55
#define REG_IR_SHOT_CNT_H_FIFO_STATUS    0x56
    #define IR_RPT_FLAG                  0x0100
    #define IR_FIFO_EMPTY                0x0200
#define REG_IR_FIFO_RD_PULSE    		 0x58

#endif // _REG_IR_H_
