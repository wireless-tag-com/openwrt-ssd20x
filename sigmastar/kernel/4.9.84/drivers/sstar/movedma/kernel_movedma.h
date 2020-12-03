/*
* kernel_movedma.h- Sigmastar
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
/***************************************************************************
 *  kernel_movedma.h
 *--------------------------------------------------------------------------
 *  Scope: General DMA related definitions
 *
 ****************************************************************************/

#ifndef __KERNEL_MOVEDMA_H__
#define __KERNEL_MOVEDMA_H__

/****************************************************************************/
/*        General DMA registers                                                     */
/****************************************************************************/

typedef struct KeMoveDma_s
{
    // 0x00
    u32 reg_dma_move_en                 :1;
    u32                                 :31;
    // 0x01
    u32 reg_move0_offset_en             :1;
    u32                                 :31;
    // 0x02
    u32 reg_dma_move0_en_status         :1;     // [RO]
    u32                                 :31;
    // 0x03
    u32 reg_move0_src_start_addr_l      :16;
    u32                                 :16;
    // 0x04
    u32 reg_move0_src_start_addr_h      :16;
    u32                                 :16;
    // 0x05
    u32 reg_move0_dest_start_addr_l     :16;
    u32                                 :16;
    // 0x06
    u32 reg_move0_dest_start_addr_h     :16;
    u32                                 :16;
    // 0x07
    u32 reg_move0_total_byte_cnt_l      :16;
    u32                                 :16;
    // 0x08
    u32 reg_move0_total_byte_cnt_h      :12;
    u32                                 :20;
    // 0x09
    u32 reg_move0_offset_src_width_l    :16;
    u32                                 :16;
    // 0x0A
    u32 reg_move0_offset_src_width_h    :12;
    u32                                 :20;
    // 0x0B
    u32 reg_move0_offset_src_offset_l   :16;
    u32                                 :16;
    // 0x0C
    u32 reg_move0_offset_src_offset_h   :12;
    u32                                 :20;
    // 0x0D
    u32 reg_move0_offset_dest_width_l   :16;
    u32                                 :16;
    // 0x0E
    u32 reg_move0_offset_dest_width_h   :12;
    u32                                 :20;
    // 0x0F
    u32 reg_move0_offset_dest_offset_l  :16;
    u32                                 :16;
    // 0x10
    u32 reg_move0_offset_dest_offset_h  :12;
    u32                                 :20;
    // 0x11
    u32 reg_dma_move0_left_byte_l       :16;    // [RO]
    u32                                 :16;
    // 0x12
    u32 reg_dma_move0_left_byte_h       :13;    // [RO]
    u32                                 :19;
    // 0x13
    u32                                 :32;
    // 0x14
    u32                                 :32;
    // 0x15
    u32                                 :32;
    // 0x16
    u32                                 :32;
    // 0x17
    u32                                 :32;
    // 0x18
    u32                                 :32;
    // 0x19
    u32                                 :32;
    // 0x1A
    u32                                 :32;
    // 0x1B
    u32                                 :32;
    // 0x1C
    u32                                 :32;
    // 0x1D
    u32                                 :32;
    // 0x1E
    u32                                 :32;
    // 0x1F
    u32                                 :32;
    // 0x20
    u32                                 :32;
    // 0x21
    u32                                 :32;
    // 0x22
    u32                                 :32;
    // 0x23
    u32 reg_dummy                       :16;
    u32                                 :16;
    // 0x24
    u32 reg_dma_mov_sw_rst              :1;     // [WO]
    u32                                 :31;
    // 0x25
    u32 reg_dma02mi_priority_mask       :1;
    u32                                 :31;
    // 0x26
    u32 reg_dma_irq_mask                :1;
    u32                                 :31;
    // 0x27
    u32 reg_dma_irq_force               :1;
    u32                                 :31;
    // 0x28
    u32 reg_dma_irq_clr                 :1;
    u32                                 :31;
    // 0x29
    u32 reg_dma_irq_select              :1;
    u32                                 :31;
    // 0x2A
    u32 reg_dma_irq_final_status        :1;     // [RO]
    u32                                 :31;
    // 0x2B
    u32 reg_dma_irq_raw_status          :1;     // [RO]
    u32                                 :31;
        #define MOVEDMA_INT_MOVE0_DONE  (0x01)
    // 0x2C
    u32 reg_dma_probe_sel               :8;
    u32                                 :24;
    // 0x2D
    u32 reg_dma_probe_l                 :16;    // [RO]
    u32                                 :16;
    // 0x2E
    u32 reg_dma_probe_h                 :8;     // [RO]
    u32                                 :24;
    // 0x2F
    u32 reg_dma_bist_fail_rd            :1;     // [RO]
    u32                                 :31;
    // 0x30
    u32 reg_dma_move0_miu_sel_en        :1;
    u32 reg_dma_move0_src_miu_sel       :1;
    u32 reg_dma_move0_dst_miu_sel       :1;
        #define REG_DMA_MOVE0_SEL_MIU0  (0)
        #define REG_DMA_MOVE0_SEL_MIU1  (1)
    u32                                 :29;
    // 0x31
    u32                                 :32;
    // 0x32
    u32                                 :32;
    // 0x33
    u32                                 :32;
    // 0x34
    u32                                 :32;
    // 0x35
    u32                                 :32;
    // 0x36
    u32                                 :32;
    // 0x37
    u32                                 :32;
    // 0x38
    u32                                 :32;
    // 0x39
    u32                                 :32;
    // 0x3A
    u32                                 :32;
    // 0x3B
    u32                                 :32;
    // 0x3C
    u32                                 :32;
    // 0x3D
    u32                                 :32;
    // 0x3E
    u32                                 :32;
    // 0x3F
    u32                                 :32;
    // 0x40
    u32 reg_dma_cmdq_irq_mask           :1;
    u32                                 :31;
    // 0x41
    u32 reg_dma_cmdq_irq_force          :1;
    u32                                 :31;
    // 0x42
    u32 reg_dma_cmdq_irq_clr            :1;
    u32                                 :31;
    // 0x43
    u32 reg_dma_cmdq_irq_select         :1;
    u32                                 :31;
    // 0x44
    u32 reg_dma_cmdq_irq_final_status   :1;     // [RO]
    u32                                 :31;
    // 0x45
    u32 reg_dma_cmdq_irq_raw_status     :1;     // [RO]
    u32                                 :31;
} KeMoveDma_t;

#endif // __KERNEL_MOVEDMA_H__

