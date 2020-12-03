/*
* kernel_bdma.h- Sigmastar
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
 *  kernel_bdma.h
 *--------------------------------------------------------------------------
 *  Scope: BDMA related definitions
 *
 ****************************************************************************/

#ifndef __KERNEL_BDMA_H__
#define __KERNEL_BDMA_H__

#include "cam_os_wrapper.h"

/****************************************************************************/
/*        BDMA registers                                                     */
/****************************************************************************/

typedef struct KeBdma_s
{
    // 0x0
    u32 reg_ch0_trig                :1;
    u32                             :3;
    u32 reg_ch0_stop                :1;
    u32                             :3;
    u32 reg_src_tlb                 :1;
    u32 reg_dst_tlb                 :1;
    u32                             :22;
    // 0x1
    u32 reg_ch0_queued              :1;
    u32 reg_ch0_busy                :1;
    u32 reg_ch0_int_bdma            :1;
    u32 reg_ch0_done                :1;
    u32 reg_ch0_result0             :1;
    u32 reg_xiu_bdma_ns             :1;
    u32                             :26;
    // 0x2
    u32 reg_ch0_src_sel             :4;
        #define REG_BDMA_SRC_MIU_IMI_CH0    0 //16 bytes
        #define REG_BDMA_SRC_MIU_IMI_CH1    1 //16 bytes
        #define REG_BDMA_SRC_MEM_FILL       4  //4 bytes
        #define REG_BDMA_SRC_SPI            5  //8 bytes
        #define REG_BDMA_SRC_TSP_SRAM       10 //1 byte
    u32 reg_ch0_src_dw              :3;
        #define REG_BDMA_DATA_DEPTH_1BYTE   0
        #define REG_BDMA_DATA_DEPTH_2BYTE   1
        #define REG_BDMA_DATA_DEPTH_4BYTE   2
        #define REG_BDMA_DATA_DEPTH_8BYTE   3
        #define REG_BDMA_DATA_DEPTH_16BYTE  4
    u32                             :1;
    u32 reg_ch0_dst_sel             :4;
        #define REG_BDMA_DST_MIU_IMI_CH0    0
        #define REG_BDMA_DST_MIU_IMI_CH1    1
        #define REG_BDMA_DST_PM51           6
        #define REG_BDMA_DST_SEC51          9
        #define REG_BDMA_DST_TSP_SRAM       10
        #define REG_BDMA_SDT_FSP            11 //4 byte
    u32 reg_ch0_dst_dw              :3;
    u32                             :17;
    // 0x3
    u32 reg_ch0_dec                 :1;
    u32 reg_ch0_int_en              :1;
    u32                             :2;
    u32 reg_ch0_cfg                 :4;
    u32 reg_ch0_flush_wd            :4;
    u32 reg_ch0_replace_miu         :4;
        #define REG_BDMA_CH0_MIU0           0
        #define REG_BDMA_CH0_MIU1           1
        #define REG_BDMA_CH0_IMI            2
        #define REG_BDMA_CH1_MIU0           0
        #define REG_BDMA_CH1_MIU1           4
        #define REG_BDMA_CH1_IMI            8
    u32                             :16;
    // 0x4
    u32 reg_ch0_src_a0              :16;
    u32                             :16;
    // 0x5
    u32 reg_ch0_src_a1              :16;
    u32                             :16;
    // 0x6
    u32 reg_ch0_dst_a0              :16;
    u32                             :16;
    // 0x7
    u32 reg_ch0_dst_a1              :16;
    u32                             :16;
    // 0x8
    u32 reg_ch0_size0               :16;
    u32                             :16;
    // 0x9
    u32 reg_ch0_size1               :16;
    u32                             :16;
    // 0xA
    u32 reg_ch0_cmd0_low            :16;
    u32                             :16;
    // 0xB
    u32 reg_ch0_cmd0_high           :16;
    u32                             :16;
    // 0xC
    u32 reg_ch0_cmd1_low            :16;
    u32                             :16;
    // 0xD
    u32 reg_ch0_cmd1_high           :16;
    u32                             :16;
    // 0xE
    u32 reg_ch0_cmd2_low            :16;
    u32                             :16;
    // 0xF
    u32 reg_ch0_cmd2_high           :16;
    u32                             :16;
} KeBdma_t;

extern volatile KeBdma_t* const g_ptKeBdma0;
extern volatile KeBdma_t* const g_ptKeBdma1;
extern volatile KeBdma_t* const g_ptKeBdma2;
extern volatile KeBdma_t* const g_ptKeBdma3;

#endif // __KERNEL_BDMA_H__

