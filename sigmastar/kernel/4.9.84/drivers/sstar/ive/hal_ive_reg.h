/*
* hal_ive_reg.h- Sigmastar
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
#ifndef _HAL_IVE_REG_H_
#define _HAL_IVE_REG_H_

#include <linux/kernel.h>

typedef struct
{
    union
    {
        struct
        {
            u16 sw_fire:1;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 opr_upd_mode:1;
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 sw_rst:1;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 miu_sel:1;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 irq_mask:8;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 irq_force:8;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 irq_raw_status:8;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 irq_final_status:8;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 irq_sel:8;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 woc_irq_clr:8;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set0:11;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set1:11;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 y_cnt_hit_set2:11;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 cmq_trig_mask:8;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 cmq_trig_force:8;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 cmq_trig_raw_status:8;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 cmq_trig_final_status:8;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 cmq_trig_sel:1;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 woc_cmq_trig_clr:8;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 bist_fail_rd_low:16;
        };
        u16 reg30;
    };

    union
    {
        struct
        {
            u16 bist_fail_rd_high:16;
        };
        u16 reg31;
    };

    union
    {
        struct
        {
            u16 cycle_count_low:16;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 cycle_count_high:16;
        };
        u16 reg41;
    };
} ive_hal_reg_bank0;


typedef struct
{
    union
    {
        struct
        {
            u16 nxt_cmd_addr_low:16; // not used
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 nxt_cmd_addr_high:16; // not used
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 task_id:16; // not used
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 bpp:8; // not used
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 op_type:8;
            u16 op_mode:8;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 infmt:8;
            u16 outfmt:8;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 frame_width:16;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 frame_height:16;
        };
        u16 reg07;
    };

    union
    {
        struct
        {
            u16 src1_addr_low:16;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 src1_addr_high:16;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 dst1_addr_low:16;
        };
        u16 reg0A;
    };

    union
    {
        struct
        {
            u16 dst1_addr_high:16;
        };
        u16 reg0B;
    };

    union
    {
        struct
        {
            u16 src2_addr_low:16;
        };
        u16 reg0C;
    };

    union
    {
        struct
        {
            u16 src2_addr_high:16;
        };
        u16 reg0D;
    };

    union
    {
        struct
        {
            u16 dst2_addr_low:16;
        };
        u16 reg0E;
    };

    union
    {
        struct
        {
            u16 dst2_addr_high:16;
        };
        u16 reg0F;
    };

    union
    {
        struct
        {
            u16 src3_addr_low:16;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 src3_addr_high:16;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 dst3_addr_low:16;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 dst3_addr_high:16;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 src1_stride:16;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 dst1_stride:16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 src2_stride:16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 dst2_stride:16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 src3_stride:16;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 dst3_stride:16;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 mask0:8;
            u16 mask1:8;
        };
        u16 reg1A;
    };

    union
    {
        struct
        {
            u16 mask2:8;
            u16 mask3:8;
        };
        u16 reg1B;
    };

    union
    {
        struct
        {
            u16 mask4:8;
            u16 mask5:8;
        };
        u16 reg1C;
    };

    union
    {
        struct
        {
            u16 mask6:8;
            u16 mask7:8;
        };
        u16 reg1D;
    };

    union
    {
        struct
        {
            u16 mask8:8;
            u16 mask9:8;
        };
        u16 reg1E;
    };

    union
    {
        struct
        {
            u16 mask10:8;
            u16 mask11:8;
        };
        u16 reg1F;
    };

    union
    {
        struct
        {
            u16 mask12:8;
            u16 mask13:8;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 mask14:8;
            u16 mask15:8;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 mask16:8;
            u16 mask17:8;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 mask18:8;
            u16 mask19:8;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 mask20:8;
            u16 mask21:8;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 mask22:8;
            u16 mask23:8;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 mask24:8;
            u16 shift:8;
        };
        u16 reg26;
    };

    union
    {
        struct
        {
            u16 thresh_16bit_1:16;
        };
        u16 reg28;
    };

    union
    {
        struct
        {
            u16 thresh_16bit_2:16;
        };
        u16 reg29;
    };

    union
    {
        struct
        {
            u16 fraction:16;
        };
        u16 reg2A;
    };

    union
    {
        struct
        {
            u16 add_weight_x:16;
        };
        u16 reg2B;
    };

    union
    {
        struct
        {
            u16 add_weight_y:16;
        };
        u16 reg2C;
    };

    union
    {
        struct
        {
            u16 csc_coeff0:12;
        };
        u16 reg30;
    };

    union
    {
        struct
        {
            u16 csc_coeff1:12;
        };
        u16 reg31;
    };

    union
    {
        struct
        {
            u16 csc_coeff2:12;
        };
        u16 reg32;
    };

    union
    {
        struct
        {
            u16 csc_coeff3:12;
        };
        u16 reg33;
    };

    union
    {
        struct
        {
            u16 csc_coeff4:12;
        };
        u16 reg34;
    };

    union
    {
        struct
        {
            u16 csc_coeff5:12;
        };
        u16 reg35;
    };

    union
    {
        struct
        {
            u16 csc_coeff6:12;
        };
        u16 reg36;
    };

    union
    {
        struct
        {
            u16 csc_coeff7:12;
        };
        u16 reg37;
    };

    union
    {
        struct
        {
            u16 csc_coeff8:12;
        };
        u16 reg38;
    };

    union
    {
        struct
        {
            u16 csc_offset0:12;
        };
        u16 reg39;
    };

    union
    {
        struct
        {
            u16 csc_offset1:12;
        };
        u16 reg3A;
    };

    union
    {
        struct
        {
            u16 csc_offset2:12;
        };
        u16 reg3B;
    };

    union
    {
        struct
        {
            u16 csc_clamp0_low:8;
            u16 csc_clamp0_high:8;
        };
        u16 reg3C;
    };

    union
    {
        struct
        {
            u16 csc_clamp1_low:8;
            u16 csc_clamp1_high:8;
        };
        u16 reg3D;
    };

    union
    {
        struct
        {
            u16 csc_clamp2_low:8;
            u16 csc_clamp2_high:8;
        };
        u16 reg3E;
    };

} ive_hal_reg_bank1;

#endif // _HAL_IVE_REG_H_
