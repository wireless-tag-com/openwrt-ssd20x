/*
* cedric_r2.h- Sigmastar
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

#ifndef __CEDRIC_R2_H__
#define __CEDRIC_R2_H__


//------------------------------------------------------------------------------
//  Macro
//------------------------------------------------------------------------------
#include "asm/arch/mach/ms_types.h"

#define HK_CNTROL_CAR_BACKING
//display related
//rtk qstart related--mailbox

#define DETECT_METHOD 1
// 0 for interrupt from rtk
// 1 for polling mailbox

#define QST_ALWAYS_RUN 1


#define TOKEN_GET 0x08
#define CAR_MODE 0x04


#define MBX_TX (0X41) //HOSTKEEPER_MBX
#define MBX_RX (0x42) //CARBACKING_MBX
#define MBX_TOUCH_X (0X43)
#define MBX_TOUCH_Y (0X44)


#define CEDRIC_BASE_REG_MBX_PA			GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x81980)
#define REG_MBX_BASE    				GET_REG_ADDR(CEDRIC_BASE_REG_MBX_PA,0X40)
#define REG_MBX_RX    					GET_REG_ADDR(CEDRIC_BASE_REG_MBX_PA,MBX_RX)
#define REG_MBX_TX    					GET_REG_ADDR(CEDRIC_BASE_REG_MBX_PA,MBX_TX)
#define REG_MBX_TOUCH_X					GET_REG_ADDR(CEDRIC_BASE_REG_MBX_PA,MBX_TOUCH_X)
#define REG_MBX_TOUCH_Y					GET_REG_ADDR(CEDRIC_BASE_REG_MBX_PA,MBX_TOUCH_Y)


#define LOOP_THRESHOLD 8
#define CarBackingState  (1<<2)



typedef struct
{
/*
h0040	h0040	15	0	reg_mb0_0
*/
	U16 reg_mb0_0;
    U16 u16REG_RESERVED_40;

/*

h0041	h0041	15	0	reg_mb0_1
*/
	union
    {
        volatile U16 reg_mb0_1; /* 00h */
        struct
		{
			volatile U16 token :4;
			volatile U16 touch_flag :1;
								//#define VALID 		1
								//#define INVALID 		0
			volatile U16 bt_flag :1;
            volatile U16 suspend_flag :1;
            volatile U16 Os_flag :1;
            volatile U16 boot_type :1;
            volatile U16 pq_bin_loaded :1;            	
			volatile U16 reserve41 :6;
								//bit 3~f reserve
		};
    };
    U16 u16REG_RESERVED_41;

/*

h0042	h0042	15	0	reg_mb0_2
*/
    union
    {
        volatile U16 reg_mb0_2; /* 00h */
        struct
		{
			volatile U16 disp_inited :1;
								//#define DISP_INIT 1
								//#define DISP_NON 0

			volatile U16 cvbs_in :1;		//represent cvbs in is ok or not
								//#define CVBS_IN_OK 1
								//#define CVBS_IN_NON 0

			volatile U16 carbacking_status :1;
								//value as enum CAR_BACKING_MODE

			volatile U16 token_switch :1;
								//#define TOKEN_CP 0	//co processor
								//#define TOKEN_HK 1	//host keeper
            volatile U16 app_launch :4; ////max support 15 apps

        	volatile U16 bt_call_rsp :2;    // give response to ARM OS
        	volatile U16 mmi_task_running :1; //mmi task exist or not
        	volatile U16 reserve42 :5;

		};
    };
    U16 u16REG_RESERVED_42;

/*
h0043	h0043	15	0	reg_mb0_3
*/
	U16 u16MBX_TOUCH_X;
    U16 u16REG_RESERVED_43;

/*
h0044	h0044	15	0	reg_mb0_4
*/
	U16 u16MBX_TOUCH_Y;
    U16 u16REG_RESERVED_44;

/*
h0045	h0045	15	0	reg_mb0_5
*/
	U16 reg_mb0_5;
    U16 u16REG_RESERVED_45;

/*
h0046	h0046	15	0	reg_mb0_6
*/
	U16 reg_mb0_6;
    U16 u16REG_RESERVED_46;

/*
h0047	h0047	15	0	reg_mb0_7
*/
	U16 reg_mb0_7;
    U16 u16REG_RESERVED_47;

/*
h0048	h0048	15	0	reg_mb0_8
*/
	U16 reg_mb0_8;
    U16 u16REG_RESERVED_48;

/*
h0049	h0049	15	0	reg_mb0_9
*/
	U16 reg_mb0_9;
    U16 u16REG_RESERVED_49;

/*
h004a	h004a	15	0	reg_mb0_a
*/
	U16 reg_mb0_a;
    U16 u16REG_RESERVED_4a;

/*
h004b	h004b	15	0	reg_mb0_b
*/
	U16 reg_mb0_b;
    U16 u16REG_RESERVED_4b;

/*
h004c	h004c	15	0	reg_mb0_c
*/
	U16 reg_mb0_c;
    U16 u16REG_RESERVED_4c;

/*
h004d	h004d	15	0	reg_mb0_d
*/
	U16 reg_mb0_d;
    U16 u16REG_RESERVED_4d;

/*
h004e	h004e	15	0	reg_mb0_e
*/
	U16 reg_mb0_e;
    U16 u16REG_RESERVED_4e;

/*
h004f	h004f	15	0	reg_mb0_f
*/
	U16 reg_mb0_f;
    U16 u16REG_RESERVED_4f;

/*
h0050	h0050	15	0	reg_mb0_10
*/
	U16 reg_mb0_10;
    U16 u16REG_RESERVED_50;

/*
h0051	h0051	15	0	reg_mb0_11
*/
	U16 reg_mb0_11;
    U16 u16REG_RESERVED_51;

/*
h0052	h0052	15	0	reg_mb0_12
*/
	U16 reg_mb0_12;
    U16 u16REG_RESERVED_52;

/*
h0053	h0053	15	0	reg_mb0_13
*/
	U16 reg_mb0_13;
    U16 u16REG_RESERVED_53;

/*
h0054	h0054	15	0	reg_mb0_14
*/
	U16 reg_mb0_14;
    U16 u16REG_RESERVED_54;

/*
h0055	h0055	15	0	reg_mb0_15
*/
	U16 reg_mb0_15;
    U16 u16REG_RESERVED_55;

/*
h0056	h0056	15	0	reg_mb0_16
*/
	U16 reg_mb0_16;
    U16 u16REG_RESERVED_56;

/*
h0057	h0057	15	0	reg_mb0_17
*/
	U16 reg_mb0_17;
    U16 u16REG_RESERVED_57;

/*
h0058	h0058	15	0	reg_mb0_18
*/
	U16 reg_mb0_18;
    U16 u16REG_RESERVED_58;

/*
h0059	h0059	15	0	reg_mb0_19
*/
	U16 reg_mb0_19;
    U16 u16REG_RESERVED_59;

/*
h005A	h005A	15	0	reg_mb0_1A
*/
	U16 reg_mb0_1A;
    U16 u16REG_RESERVED_5a;

/*
h005B	h005B	15	0	reg_mb0_1B
*/
	U16 reg_mb0_1B;
    U16 u16REG_RESERVED_5b;

/*
h005C	h005C	15	0	reg_mb0_1C
*/
	U16 reg_mb0_1C;
    U16 u16REG_RESERVED_5c;

/*
h005D	h005D	15	0	reg_mb0_1D
*/
	U16 reg_mb0_1D;
    U16 u16REG_RESERVED_5d;

/*
h005E	h005E	15	0	reg_mb0_1E
*/
	U16 reg_mb0_1E;
    U16 u16REG_RESERVED_5e;

/*
h005f	h005f	15	0	reg_mb0_1f
*/
	U16 reg_mb0_1f;
    U16 u16REG_RESERVED_5f;

/*
h0060	h0060	15	0	reg_mb0_20
*/
	U16 reg_mb0_20;
    U16 u16REG_RESERVED_60;

/*
h0061	h0061	15	0	reg_mb0_21
*/
	U16 reg_mb0_21;
    U16 u16REG_RESERVED_61;

/*
h0062	h0062	15	0	reg_mb0_22
*/
	U16 reg_mb0_22;
    U16 u16REG_RESERVED_62;

/*
h0063	h0063	15	0	reg_mb0_23
*/
	U16 reg_mb0_23;
    U16 u16REG_RESERVED_63;

/*
h0064	h0064	15	0	reg_mb0_24
*/
	U16 reg_mb0_24;
    U16 u16REG_RESERVED_64;

/*
h0065	h0065	15	0	reg_mb0_25
*/
	U16 reg_mb0_25;
    U16 u16REG_RESERVED_65;

/*
h0066	h0066	15	0	reg_mb0_26
*/
	U16 reg_mb0_26;
    U16 u16REG_RESERVED_66;

/*
h0067	h0067	15	0	reg_mb0_27
*/
	U16 reg_mb0_27;
    U16 u16REG_RESERVED_67;

/*
h0068	h0068	15	0	reg_mb0_28
*/
	U16 reg_mb0_28;
    U16 u16REG_RESERVED_68;

/*
h0069	h0069	15	0	reg_mb0_29
*/
	U16 reg_mb0_29;
    U16 u16REG_RESERVED_69;

/*
h006A	h006A	15	0	reg_mb0_2A
*/
	U16 reg_mb0_2A;
    U16 u16REG_RESERVED_6a;

/*
h006B	h006B	15	0	reg_mb0_2B
*/
	U16 reg_mb0_2B;
    U16 u16REG_RESERVED_6b;

/*
h006C	h006C	15	0	reg_mb0_2C
*/
	U16 reg_mb0_2C;
    U16 u16REG_RESERVED_6c;

/*
h006D	h006D	15	0	reg_mb0_2D
*/
	U16 reg_mb0_2D;
    U16 u16REG_RESERVED_6d;

/*
h006E	h006E	15	0	reg_mb0_2E
*/
	U16 reg_mb0_2E;
    U16 u16REG_RESERVED_6e;

/*
h006F	h006F	15	0	reg_mb0_2F
*/
	U16 reg_mb0_2F;
    U16 u16REG_RESERVED_6f;

/*
*/
}REG_MBX_st,*PREG_MBX_st;




enum
{
	BACK,
	FORWARD,
	DIR_NONE,
	DIR_NUM = DIR_NONE,
};
enum
{
	NON_event = 0,
	RTOS_event,
	USER_event,
	GOP_refresh_event,
};

enum
{
	HK_PREPARE,
	HK_START ,
	HK_ALIVE,
	USR_OP_START,
	USR_OP_BACK,
	USR_OP_FORWARD,

};


#endif //__CEDRIC_QST__