/*
* infinity_reg_isp0.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: eroy.yang <eroy.yang@sigmastar.com.tw>
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

#ifndef __INFINITY_REG_ISP0__
#define __INFINITY_REG_ISP0__
typedef struct {
	// h0000, bit: 0
	/* Enable ISP Engine
	# 1’b0: Disable and Reset to the Initial State
	# 1’b1: Enable
	 ( double buffer register )*/
	#define offset_of_reg_en_isp (0)
	#define mask_of_reg_en_isp (0x1)
	unsigned int reg_en_isp:1;

	// h0000, bit: 1
	/* Enable Sensor
	# 1'b0: Disable
	# 1'b1: Enable
	 ( double buffer register )*/
	#define offset_of_reg_en_sensor (0)
	#define mask_of_reg_en_sensor (0x2)
	unsigned int reg_en_sensor:1;

	// h0000, bit: 2
	/* Enable Optical Black Correction
	# 1'b0: Disable
	# 1'b1: Enable
	( double buffer register )*/
	#define offset_of_reg_en_isp_obc (0)
	#define mask_of_reg_en_isp_obc (0x4)
	unsigned int reg_en_isp_obc:1;

	// h0000, bit: 3
	/* Enable White Balance (before Denoise)
	# 1'b0: Disable
	# 1'b1: Enable
	 ( double buffer register )*/
	#define offset_of_reg_en_isp_wbg0 (0)
	#define mask_of_reg_en_isp_wbg0 (0x8)
	unsigned int reg_en_isp_wbg0:1;

	// h0000, bit: 4
	/* Enable White Balance (after Denoise)
	# 1'b0: Disable
	# 1'b1: Enable
	 ( double buffer register )*/
	#define offset_of_reg_en_isp_wbg1 (0)
	#define mask_of_reg_en_isp_wbg1 (0x10)
	unsigned int reg_en_isp_wbg1:1;

	// h0000, bit: 14
	/* */
	unsigned int :11;

	// h0000
	unsigned int /* padding 16 bit */:16;

	// h0001, bit: 0
	/* Software Reset for ISP
	# Low Active
	 ( single buffer register )*/
	#define offset_of_reg_isp_sw_rstz (2)
	#define mask_of_reg_isp_sw_rstz (0x1)
	unsigned int reg_isp_sw_rstz:1;

	// h0001, bit: 1
	/* Software Reset for Sensor
	# Low Active
	 ( single buffer register )*/
	#define offset_of_reg_sensor_sw_rstz (2)
	#define mask_of_reg_sensor_sw_rstz (0x2)
	unsigned int reg_sensor_sw_rstz:1;

	// h0001, bit: 2
	/* Power Down Sensor
	# 1'b0: Disable
	# 1'b1: Power Down
	 ( single buffer register )
	no-used*/
	#define offset_of_reg_sen_pwdn (2)
	#define mask_of_reg_sen_pwdn (0x4)
	unsigned int reg_sen_pwdn:1;

	// h0001, bit: 3
	/* Enable CLK_sensor delay for isp_if module
	# 1'b0: Disable
	# 1'b1 Enable
	 ( single buffer register )*/
	#define offset_of_reg_en_isp_clk_sensor_delay (2)
	#define mask_of_reg_en_isp_clk_sensor_delay (0x8)
	unsigned int reg_en_isp_clk_sensor_delay:1;

	// h0001, bit: 4
	/* Enable Lens Shading Compensation
	# 1'b0: Disable
	# 1'b1: Enable
	 (double buffer register )*/
	#define offset_of_reg_en_isp_lsc (2)
	#define mask_of_reg_en_isp_lsc (0x10)
	unsigned int reg_en_isp_lsc:1;

	// h0001, bit: 6
	/* */
	unsigned int :2;

	// h0001, bit: 7
	/* Enable Denoise
	# 1'b0: Disable
	# 1'b1: Enable
	 ( dobule buffer register )*/
	#define offset_of_reg_en_isp_denoise (2)
	#define mask_of_reg_en_isp_denoise (0x80)
	unsigned int reg_en_isp_denoise:1;

	// h0001, bit: 11
	/* */
	unsigned int :4;

	// h0001, bit: 12
	/* Enable isp2pad_sr_clk
	# 1'b0: Disable
	# 1'b1: Enable
	 ( single buffer register )
	(no-used)*/
	#define offset_of_reg_en_sensor_clk (2)
	#define mask_of_reg_en_sensor_clk (0x1000)
	unsigned int reg_en_sensor_clk:1;

	// h0001, bit: 14
	/* */
	unsigned int :2;

	// h0001, bit: 15
	/* Always Load Double Buffer Register (Type 2 )
	 ( single buffer register )*/
	#define offset_of_reg_load_reg (2)
	#define mask_of_reg_load_reg (0x8000)
	unsigned int reg_load_reg:1;

	// h0001
	unsigned int /* padding 16 bit */:16;

	// h0002, bit: 13
	/* */
	unsigned int :14;

	// h0002, bit: 14
	/* double buffer register set done at batch mode*/
	#define offset_of_reg_db_batch_done (4)
	#define mask_of_reg_db_batch_done (0x4000)
	unsigned int reg_db_batch_done:1;

	// h0002, bit: 15
	/* double buffer register batch mode*/
	#define offset_of_reg_db_batch_mode (4)
	#define mask_of_reg_db_batch_mode (0x8000)
	unsigned int reg_db_batch_mode:1;

	// h0002
	unsigned int /* padding 16 bit */:16;

	// h0003, bit: 3
	/* [2] b3_atv_fix*/
	#define offset_of_reg_fire_mode (6)
	#define mask_of_reg_fire_mode (0xf)
	unsigned int reg_fire_mode:4;

	// h0003, bit: 7
	/* */
	unsigned int :4;

	// h0003, bit: 8
	/* reset sensor 1*/
	#define offset_of_reg_sensor1_rst (6)
	#define mask_of_reg_sensor1_rst (0x100)
	unsigned int reg_sensor1_rst:1;

	// h0003, bit: 9
	/* power down sensor 1*/
	#define offset_of_reg_sensor1_pwrdn (6)
	#define mask_of_reg_sensor1_pwrdn (0x200)
	unsigned int reg_sensor1_pwrdn:1;

	// h0003, bit: 10
	/* reset sensor 2*/
	#define offset_of_reg_sensor2_rst (6)
	#define mask_of_reg_sensor2_rst (0x400)
	unsigned int reg_sensor2_rst:1;

	// h0003, bit: 11
	/* power down sensor 2*/
	#define offset_of_reg_sensor2_pwrdn (6)
	#define mask_of_reg_sensor2_pwrdn (0x800)
	unsigned int reg_sensor2_pwrdn:1;

	// h0003, bit: 14
	/* */
	unsigned int :4;

	// h0003
	unsigned int /* padding 16 bit */:16;

	// h0004, bit: 0
	/* isp_dp source mux
	# 1'b0: from sensor_if
	# 1'b1: from RDMA*/
	#define offset_of_reg_isp_if_rmux_sel (8)
	#define mask_of_reg_isp_if_rmux_sel (0x1)
	unsigned int reg_isp_if_rmux_sel:1;

	// h0004, bit: 3
	/* */
	unsigned int :3;

	// h0004, bit: 6
	/* WDMA source mux
	# 3'd0: from sensor_if
	# 3'd1: from isp_dp output
	# 3'd2: from isp_dp down-sample
	# 3'd3: from isp_dp output
	# 3'd4: from video stabilization*/
	#define offset_of_reg_isp_if_wmux_sel (8)
	#define mask_of_reg_isp_if_wmux_sel (0x70)
	unsigned int reg_isp_if_wmux_sel:3;

	// h0004, bit: 7
	/* */
	unsigned int :1;

	// h0004, bit: 8
	/* isp_dp source mux
	# 1'b0: from rmux
	# 1'b1: from sensor freerun vs*/
	#define offset_of_reg_isp_if_rvs_sel (8)
	#define mask_of_reg_isp_if_rvs_sel (0x100)
	unsigned int reg_isp_if_rvs_sel:1;

	// h0004, bit: 9
	/* ICP ack tie 1 enable*/
	#define offset_of_reg_isp_icp_ack_tie1 (8)
	#define mask_of_reg_isp_icp_ack_tie1 (0x200)
	unsigned int reg_isp_icp_ack_tie1:1;

	// h0004, bit: 14
	/* */
	unsigned int :5;

	// h0004, bit: 15
	/* ISP IF status reset*/
	#define offset_of_reg_isp_if_state_rst (8)
	#define mask_of_reg_isp_if_state_rst (0x8000)
	unsigned int reg_isp_if_state_rst:1;

	// h0004
	unsigned int /* padding 16 bit */:16;

	// h0005, bit: 0
	/* isp_if source mux:
	# 1'b0: from sensor
	# 1'b1: from csi*/
	#define offset_of_reg_isp_if_src_sel (10)
	#define mask_of_reg_isp_if_src_sel (0x1)
	unsigned int reg_isp_if_src_sel:1;

	// h0005, bit: 1
	/* Isp to cmdq vsync select
	# 1'b0: DVP mode, from vsync pad
	# 1'b1: use isp re-generate vsync*/
	#define offset_of_reg_isp2cmdq_vs_sel (10)
	#define mask_of_reg_isp2cmdq_vs_sel (0x2)
	unsigned int reg_isp2cmdq_vs_sel:1;

	// h0005, bit: 3
	/* */
	unsigned int :2;

	// h0005, bit: 4
	/* ISP FIFO handshaking mode enable*/
	#define offset_of_reg_isp_if_hs_mode_en (10)
	#define mask_of_reg_isp_if_hs_mode_en (0x10)
	unsigned int reg_isp_if_hs_mode_en:1;

	// h0005, bit: 7
	/* */
	unsigned int :3;

	// h0005, bit: 9
	/* isp_dp crop virtual channel selection*/
	#define offset_of_reg_isp_if_dp_vc (10)
	#define mask_of_reg_isp_if_dp_vc (0x300)
	unsigned int reg_isp_if_dp_vc:2;

	// h0005, bit: 11
	/* */
	unsigned int :2;

	// h0005, bit: 13
	/* WDMA crop virtual channel selection*/
	#define offset_of_reg_isp_if_wdma_vc (10)
	#define mask_of_reg_isp_if_wdma_vc (0x3000)
	unsigned int reg_isp_if_wdma_vc:2;

	// h0005, bit: 14
	/* */
	unsigned int :2;

	// h0005
	unsigned int /* padding 16 bit */:16;

	// h0006, bit: 14
	/* */
	unsigned int :16;

	// h0006
	unsigned int /* padding 16 bit */:16;

	// h0007, bit: 0
	/* Mask sensor/csi ready to ISP_IF*/
	#define offset_of_reg_isp_sensor_mask (14)
	#define mask_of_reg_isp_sensor_mask (0x1)
	unsigned int reg_isp_sensor_mask:1;

	// h0007, bit: 1
	/* Ready only mode*/
	#define offset_of_reg_isp_if_de_mode (14)
	#define mask_of_reg_isp_if_de_mode (0x2)
	unsigned int reg_isp_if_de_mode:1;

	// h0007, bit: 14
	/* */
	unsigned int :14;

	// h0007
	unsigned int /* padding 16 bit */:16;

	// h0008, bit: 0
	/* Enable Software Flash Strobe
	# 1'b0: Disable
	# 1'b1: Enable
	 ( single buffer register )*/
	#define offset_of_reg_en_sw_strobe (16)
	#define mask_of_reg_en_sw_strobe (0x1)
	unsigned int reg_en_sw_strobe:1;

	// h0008, bit: 1
	/* Software Strobe Setting
	 ( single buffer register )*/
	#define offset_of_reg_sw_strobe (16)
	#define mask_of_reg_sw_strobe (0x2)
	unsigned int reg_sw_strobe:1;

	// h0008, bit: 3
	/* */
	unsigned int :2;

	// h0008, bit: 4
	/* Strobe Polarity
	# 1'b0: high active
	# 1'b1: low active*/
	#define offset_of_reg_strobe_polarity (16)
	#define mask_of_reg_strobe_polarity (0x10)
	unsigned int reg_strobe_polarity:1;

	// h0008, bit: 14
	/* */
	unsigned int :11;

	// h0008
	unsigned int /* padding 16 bit */:16;

	// h0009, bit: 14
	/* Start Strobe after reference strobe start (unit:256xPCLK)
	 ( single buffer register )*/
	#define offset_of_reg_strobe_start (18)
	#define mask_of_reg_strobe_start (0x7fff)
	unsigned int reg_strobe_start:15;

	// h0009, bit: 15
	/* Reference Start for strobe signal
	# 1'b0 : vertical active start
	# 1'b1 : vertical blanking start
	 ( single buffer register )*/
	#define offset_of_reg_strobe_ref (18)
	#define mask_of_reg_strobe_ref (0x8000)
	unsigned int reg_strobe_ref:1;

	// h0009
	unsigned int /* padding 16 bit */:16;

	// h000a, bit: 14
	/* End Strobe after reference strobe start (unit:256xPCLK)
	 ( single buffer register )*/
	#define offset_of_reg_strobe_end (20)
	#define mask_of_reg_strobe_end (0x7fff)
	unsigned int reg_strobe_end:15;

	// h000a, bit: 15
	/* Enable Output Strobe Signal By Hardware Control
	# 1'b0: Disable
	# 1'b1: Enable
	 ( single buffer register )*/
	#define offset_of_reg_en_hw_strobe (20)
	#define mask_of_reg_en_hw_strobe (0x8000)
	unsigned int reg_en_hw_strobe:1;

	// h000a
	unsigned int /* padding 16 bit */:16;

	// h000b, bit: 3
	/* Hardware Flash Strobe Counter to count sensor frame number
	 ( single buffer register )*/
	#define offset_of_reg_hw_strobe_cnt (22)
	#define mask_of_reg_hw_strobe_cnt (0xf)
	unsigned int reg_hw_strobe_cnt:4;

	// h000b, bit: 13
	/* */
	unsigned int :10;

	// h000b, bit: 14
	/* Reference END for Hardware Long Strobe
	# 1'b0: vertical active start
	# 1'b1: vertical blanking start
	 ( single buffer register )*/
	#define offset_of_reg_hw_long_strobe_end_ref (22)
	#define mask_of_reg_hw_long_strobe_end_ref (0x4000)
	unsigned int reg_hw_long_strobe_end_ref:1;

	// h000b, bit: 15
	/* Hardware Flash Strobe Mode
	# 1'b0: short strobe
	# 1'b1: long strobe (enable several frames)
	 ( single buffer register )*/
	#define offset_of_reg_hw_strobe_mode (22)
	#define mask_of_reg_hw_strobe_mode (0x8000)
	unsigned int reg_hw_strobe_mode:1;

	// h000b
	unsigned int /* padding 16 bit */:16;

	// h000c, bit: 14
	/* */
	unsigned int :16;

	// h000c
	unsigned int /* padding 16 bit */:16;

	// h000d, bit: 14
	/* */
	unsigned int :16;

	// h000d
	unsigned int /* padding 16 bit */:16;

	// h000e, bit: 14
	/* */
	#define offset_of_reg_isp_bist_fail_read (28)
	#define mask_of_reg_isp_bist_fail_read (0xffff)
	unsigned int reg_isp_bist_fail_read:16;

	// h000e
	unsigned int /* padding 16 bit */:16;

	// h000f, bit: 14
	/* */
	#define offset_of_reg_isp_bist_fail_read_1 (30)
	#define mask_of_reg_isp_bist_fail_read_1 (0xffff)
	unsigned int reg_isp_bist_fail_read_1:16;

	// h000f
	unsigned int /* padding 16 bit */:16;

	// h0010, bit: 0
	/* Master Mode Definition between ISP and Sensor
	# 1'b0: Sensor: clk_sensor --> ISP: clk_sensor
	# 1'b1: ISP: clk_sensor --> Sensor: clk_sensor
	 ( single buffer register )
	(no-used)*/
	#define offset_of_reg_isp_master_mode (32)
	#define mask_of_reg_isp_master_mode (0x1)
	unsigned int reg_isp_master_mode:1;

	// h0010, bit: 1
	/* Sensor Clock Polarity
	# 1'b0: positive edge trigger
	# 1'b1: negative edge trigger
	 ( single buffer register )*/
	#define offset_of_reg_isp_clk_sensor_polarity (32)
	#define mask_of_reg_isp_clk_sensor_polarity (0x2)
	unsigned int reg_isp_clk_sensor_polarity:1;

	// h0010, bit: 3
	/* Left shift sensor input data
	# 2'b00 : no shift
	# 2'b01 : shift 2bit
	# 2'b10 : shift 4bit
	# 2'b11 : shift 6bit*/
	#define offset_of_reg_isp_sensor_format_left_sht (32)
	#define mask_of_reg_isp_sensor_format_left_sht (0xc)
	unsigned int reg_isp_sensor_format_left_sht:2;

	// h0010, bit: 5
	/* Delay Unit for CLK_Sensor
	# 2'b00: Delay 1 unit
	# 2'b01: Delay 2 unit
	# 2'b10: Delay 3 unit
	# 2'b11: Delay 4 unit
	 ( single buffer register )*/
	#define offset_of_reg_isp_clk_sensor_delay (32)
	#define mask_of_reg_isp_clk_sensor_delay (0x30)
	unsigned int reg_isp_clk_sensor_delay:2;

	// h0010, bit: 6
	/* */
	unsigned int :1;

	// h0010, bit: 7
	/* swap sensor input data*/
	#define offset_of_reg_isp_sensor_bit_swap (32)
	#define mask_of_reg_isp_sensor_bit_swap (0x80)
	unsigned int reg_isp_sensor_bit_swap:1;

	// h0010, bit: 8
	/* Sensor Hsync Polarity
	# 1'b0: high active
	# 1'b1: low active
	 ( single buffer register )*/
	#define offset_of_reg_sensor_hsync_polarity (32)
	#define mask_of_reg_sensor_hsync_polarity (0x100)
	unsigned int reg_sensor_hsync_polarity:1;

	// h0010, bit: 9
	/* Sensor Vsync Polarity
	# 1'b0: high active
	# 1'b1: low active
	# 1'b1: high active
	# 1'b0: low active
	 ( single buffer register )*/
	#define offset_of_reg_sensor_vsync_polarity (32)
	#define mask_of_reg_sensor_vsync_polarity (0x200)
	unsigned int reg_sensor_vsync_polarity:1;

	// h0010, bit: 11
	/* Sensor Format
	# 2'b00: 8 bit
	# 2'b01: 10bit
	# 2'b10: 16bit
	# 2'b11: 12bit*/
	#define offset_of_reg_isp_sensor_format (32)
	#define mask_of_reg_isp_sensor_format (0xc00)
	unsigned int reg_isp_sensor_format:2;

	// h0010, bit: 12
	/* Sensor Input Format
	# 1'b0: YUV 422 format
	# 1'b1: RGB pattern
	 ( single buffer register )*/
	#define offset_of_reg_isp_sensor_rgb_in (32)
	#define mask_of_reg_isp_sensor_rgb_in (0x1000)
	unsigned int reg_isp_sensor_rgb_in:1;

	// h0010, bit: 13
	/* */
	unsigned int :1;

	// h0010, bit: 14
	/* Sensor 8 bit to 12 bit mode
	# 2'b00: {[7:0], [7:4]}
	# 2'b01: {[7:0], 4'b0}
	not used # 2'b10: {[11:4], [11:8]}
	not used # 2'b11: {[11:4], 4'b0}
	Sensor 10 bit to 12 bit mode
	# 2'b00: {[9:0], [9:8]}
	# 2'b01: {[9:0], 2'b0}
	not used # 2'b10: {[11:2], [11:10]}
	not used # 2'b11: {[11:2], 2'b0}*/
	#define offset_of_reg_isp_sensor_format_ext_mode (32)
	#define mask_of_reg_isp_sensor_format_ext_mode (0xc000)
	unsigned int reg_isp_sensor_format_ext_mode:2;

	// h0010
	unsigned int /* padding 16 bit */:16;

	// h0011, bit: 1
	/* RGB Raw Data Organization
	# 2'h0: [R G R G ...; G B G B ...]
	# 2'h1: [G R G R ...; B G B G ...]
	# 2'h2: [B G B G ...; G R G R ...]
	# 2'h3: [G B G B ...; R G R G ...]
	 ( double buffer register )*/
	#define offset_of_reg_isp_sensor_array (34)
	#define mask_of_reg_isp_sensor_array (0x3)
	unsigned int reg_isp_sensor_array:2;

	// h0011, bit: 3
	/* */
	unsigned int :2;

	// h0011, bit: 4
	/* Sensor Input Priority as YUV input
	# 1'b0: [ C Y C Y ...]
	# 1'b1: [ Y C Y C ...]
	 ( double buffer register )*/
	#define offset_of_reg_isp_sensor_yuv_order (34)
	#define mask_of_reg_isp_sensor_yuv_order (0x10)
	unsigned int reg_isp_sensor_yuv_order:1;

	// h0011, bit: 5
	/* */
	unsigned int :1;

	// h0011, bit: 6
	/* Sensor Input Format
	# 1'b0: separate Y/C mode
	# 1'b1: YC 16bit mode*/
	#define offset_of_reg_isp_sensor_yc16bit (34)
	#define mask_of_reg_isp_sensor_yc16bit (0x40)
	unsigned int reg_isp_sensor_yc16bit:1;

	// h0011, bit: 7
	/* */
	unsigned int :1;

	// h0011, bit: 9
	/* Sensor vsync pulse delay
	# 2'd0: vsync falling edge
	# 2'd1: vsync rising edge delay 2 line
	# 2'd2: vsync rising edge delay 1 line
	# 2'd3: vsync rising edge*/
	#define offset_of_reg_isp_sensor_vs_dly (34)
	#define mask_of_reg_isp_sensor_vs_dly (0x300)
	unsigned int reg_isp_sensor_vs_dly:2;

	// h0011, bit: 11
	/* */
	unsigned int :2;

	// h0011, bit: 12
	/* Sensor hsync pulse delay
	# 1'b0: hsync rising edge
	# 1'b1: hsync falling edge*/
	#define offset_of_reg_isp_sensor_hs_dly (34)
	#define mask_of_reg_isp_sensor_hs_dly (0x1000)
	unsigned int reg_isp_sensor_hs_dly:1;

	// h0011, bit: 14
	/* */
	unsigned int :3;

	// h0011
	unsigned int /* padding 16 bit */:16;

	// h0012, bit: 11
	/* X Start Position for Crop Window
	If Sensor is YUV input, this setting must be even
	 ( double buffer register )*/
	#define offset_of_reg_isp_crop_start_x (36)
	#define mask_of_reg_isp_crop_start_x (0xfff)
	unsigned int reg_isp_crop_start_x:12;

	// h0012, bit: 14
	/* */
	unsigned int :4;

	// h0012
	unsigned int /* padding 16 bit */:16;

	// h0013, bit: 11
	/* Y Start Position for Crop Window
	 ( double buffer register )*/
	#define offset_of_reg_isp_crop_start_y (38)
	#define mask_of_reg_isp_crop_start_y (0xfff)
	unsigned int reg_isp_crop_start_y:12;

	// h0013, bit: 14
	/* */
	unsigned int :4;

	// h0013
	unsigned int /* padding 16 bit */:16;

	// h0014, bit: 11
	/* Crop Window Width (maximum = 3264)
	If Sensor is YUV input, this setting must be odd (0 base)
	 ( double buffer register )*/
	#define offset_of_reg_isp_crop_width (40)
	#define mask_of_reg_isp_crop_width (0xfff)
	unsigned int reg_isp_crop_width:12;

	// h0014, bit: 14
	/* */
	unsigned int :4;

	// h0014
	unsigned int /* padding 16 bit */:16;

	// h0015, bit: 11
	/* Crop Window High (maximum = 2448)
	 ( double buffer register )*/
	#define offset_of_reg_isp_crop_high (42)
	#define mask_of_reg_isp_crop_high (0xfff)
	unsigned int reg_isp_crop_high:12;

	// h0015, bit: 14
	/* */
	unsigned int :4;

	// h0015
	unsigned int /* padding 16 bit */:16;

	// h0016, bit: 11
	/* WDMA X Start Position for Crop Window
	If Sensor is YUV input, this setting must be even
	 ( double buffer register )*/
	#define offset_of_reg_isp_wdma_crop_start_x (44)
	#define mask_of_reg_isp_wdma_crop_start_x (0xfff)
	unsigned int reg_isp_wdma_crop_start_x:12;

	// h0016, bit: 14
	/* */
	unsigned int :4;

	// h0016
	unsigned int /* padding 16 bit */:16;

	// h0017, bit: 11
	/* WDMA Y Start Position for Crop Window
	 ( double buffer register )*/
	#define offset_of_reg_isp_wdma_crop_start_y (46)
	#define mask_of_reg_isp_wdma_crop_start_y (0xfff)
	unsigned int reg_isp_wdma_crop_start_y:12;

	// h0017, bit: 14
	/* */
	unsigned int :4;

	// h0017
	unsigned int /* padding 16 bit */:16;

	// h0018, bit: 14
	/* WDMA Crop Window Width (maximum = 3264)
	If Sensor is YUV input, this setting must be odd (0 base)
	 ( double buffer register )*/
	#define offset_of_reg_isp_wdma_crop_width (48)
	#define mask_of_reg_isp_wdma_crop_width (0xffff)
	unsigned int reg_isp_wdma_crop_width:16;

	// h0018
	unsigned int /* padding 16 bit */:16;

	// h0019, bit: 11
	/* WDMA Crop Window High (maximum = 2448)
	 ( double buffer register )*/
	#define offset_of_reg_isp_wdma_crop_high (50)
	#define mask_of_reg_isp_wdma_crop_high (0xfff)
	unsigned int reg_isp_wdma_crop_high:12;

	// h0019, bit: 14
	/* */
	unsigned int :4;

	// h0019
	unsigned int /* padding 16 bit */:16;

	// h001a, bit: 0
	/* 1'b1 : enable , rdma_width = crop_width + crop startx
	1'b0 : disable, rdma_width = crop_width*/
	#define offset_of_reg_isp_rdma_cpx_enable (52)
	#define mask_of_reg_isp_rdma_cpx_enable (0x1)
	unsigned int reg_isp_rdma_cpx_enable:1;

	// h001a, bit: 14
	/* */
	unsigned int :15;

	// h001a
	unsigned int /* padding 16 bit */:16;

	// h001b, bit: 14
	/* */
	unsigned int :16;

	// h001b
	unsigned int /* padding 16 bit */:16;

	// h001c, bit: 14
	/* */
	unsigned int :16;

	// h001c
	unsigned int /* padding 16 bit */:16;

	// h001d, bit: 14
	/* */
	unsigned int :16;

	// h001d
	unsigned int /* padding 16 bit */:16;

	// h001e, bit: 14
	/* isp debug out*/
	#define offset_of_reg_isp_debug (60)
	#define mask_of_reg_isp_debug (0xffff)
	unsigned int reg_isp_debug:16;

	// h001e
	unsigned int /* padding 16 bit */:16;

	// h001f, bit: 7
	/* isp debug out*/
	#define offset_of_reg_isp_debug_1 (62)
	#define mask_of_reg_isp_debug_1 (0xff)
	unsigned int reg_isp_debug_1:8;

	// h001f, bit: 10
	/* */
	unsigned int :3;

	// h001f, bit: 11
	/* Hardware Debug Signal Sets Selector :
	0 : from ISP_TOP
	1 : from ISP_DP*/
	#define offset_of_reg_isp_debug_mux (62)
	#define mask_of_reg_isp_debug_mux (0x800)
	unsigned int reg_isp_debug_mux:1;

	// h001f, bit: 14
	/* Hardware Debug Signal Sets Selector*/
	#define offset_of_reg_isp_debug_sel (62)
	#define mask_of_reg_isp_debug_sel (0xf000)
	unsigned int reg_isp_debug_sel:4;

	// h001f
	unsigned int /* padding 16 bit */:16;

	// h0020, bit: 11
	/* Sensor Line Counter*/
	#define offset_of_reg_isp_sensor_line_cnt (64)
	#define mask_of_reg_isp_sensor_line_cnt (0xfff)
	unsigned int reg_isp_sensor_line_cnt:12;

	// h0020, bit: 14
	/* */
	unsigned int :4;

	// h0020
	unsigned int /* padding 16 bit */:16;

	// h0021, bit: 0
	/* ISP Status*/
	#define offset_of_reg_isp_busy (66)
	#define mask_of_reg_isp_busy (0x1)
	unsigned int reg_isp_busy:1;

	// h0021, bit: 3
	/* */
	unsigned int :3;

	// h0021, bit: 4
	/* Sensor Input Vsync*/
	#define offset_of_reg_isp_sensor_vsync (66)
	#define mask_of_reg_isp_sensor_vsync (0x10)
	unsigned int reg_isp_sensor_vsync:1;

	// h0021, bit: 5
	/* Sensor Input Hsync*/
	#define offset_of_reg_isp_sensor_hsync (66)
	#define mask_of_reg_isp_sensor_hsync (0x20)
	unsigned int reg_isp_sensor_hsync:1;

	// h0021, bit: 14
	/* */
	unsigned int :9;

	// h0021, bit: 15
	/* Internal FIFO between Sensor and ISP full*/
	#define offset_of_reg_isp_fifo_ful (66)
	#define mask_of_reg_isp_fifo_ful (0x8000)
	unsigned int reg_isp_fifo_ful:1;

	// h0021
	unsigned int /* padding 16 bit */:16;

	// h0022, bit: 14
	/* */
	unsigned int :16;

	// h0022
	unsigned int /* padding 16 bit */:16;

	// h0023, bit: 11
	/* */
	unsigned int :12;

	// h0023, bit: 12
	/* Polarity for isp_debug_clk
	# 1'b0: normal
	# 1'b1: invert clock*/
	#define offset_of_reg_isp_fpga_clk_polarity (70)
	#define mask_of_reg_isp_fpga_clk_polarity (0x1000)
	unsigned int reg_isp_fpga_clk_polarity:1;

	// h0023, bit: 14
	/* */
	unsigned int :3;

	// h0023
	unsigned int /* padding 16 bit */:16;

	// h0024, bit: 3
	/* */
	unsigned int :4;

	// h0024, bit: 4
	/* Capture Enable (only for FPGA Debug Mode)
	# 1'b0: disable
	# 1'b1: enable*/
	#define offset_of_reg_capture_en (72)
	#define mask_of_reg_capture_en (0x10)
	unsigned int reg_capture_en:1;

	// h0024, bit: 5
	/* Read Data Priority for FPGA operating
	# 1'b0: normal
	# 1'b1: inverse*/
	#define offset_of_reg_fpga_read_mode (72)
	#define mask_of_reg_fpga_read_mode (0x20)
	unsigned int reg_fpga_read_mode:1;

	// h0024, bit: 6
	/* Enable dma mclk
	# 1'b0: disable
	# 1'b1: enable*/
	#define offset_of_reg_dbgsen_oen (72)
	#define mask_of_reg_dbgsen_oen (0x40)
	unsigned int reg_dbgsen_oen:1;

	// h0024, bit: 14
	/* */
	unsigned int :9;

	// h0024
	unsigned int /* padding 16 bit */:16;

	// h0025, bit: 7
	/* New Interrupt Mask*/
	#define offset_of_reg_c_irq_mask3 (74)
	#define mask_of_reg_c_irq_mask3 (0xff)
	unsigned int reg_c_irq_mask3:8;

	// h0025, bit: 14
	/* */
	unsigned int :8;

	// h0025
	unsigned int /* padding 16 bit */:16;

	// h0026, bit: 7
	/* New Force Interrupt Enable*/
	#define offset_of_reg_c_irq_force3 (76)
	#define mask_of_reg_c_irq_force3 (0xff)
	unsigned int reg_c_irq_force3:8;

	// h0026, bit: 14
	/* New Interrupt Clear*/
	#define offset_of_reg_c_irq_clr3 (76)
	#define mask_of_reg_c_irq_clr3 (0xff00)
	unsigned int reg_c_irq_clr3:8;

	// h0026
	unsigned int /* padding 16 bit */:16;

	// h0027, bit: 7
	/* New Status of Interrupt on CPU side*/
	#define offset_of_reg_irq_final_status3 (78)
	#define mask_of_reg_irq_final_status3 (0xff)
	unsigned int reg_irq_final_status3:8;

	// h0027, bit: 14
	/* New Status of Interrupt on IP side
	#[0]: AE window0 statistic done
	#[1]: AE window1 statistic done
	#[2]: AE block row count requal req_ae_int_row_num
	#[3]: menuload done
	#[4]: input line count equal reg_sw_specify_int_line
	#[5]: output count equal reg_sw_specify_int_line
	#[6]: reserved
	#[7]: reserved
	#[8]: reserved*/
	#define offset_of_reg_irq_raw_status3 (78)
	#define mask_of_reg_irq_raw_status3 (0xff00)
	unsigned int reg_irq_raw_status3:8;

	// h0027
	unsigned int /* padding 16 bit */:16;

	// h0028, bit: 14
	/* Interrupt Mask*/
	#define offset_of_reg_c_irq_mask (80)
	#define mask_of_reg_c_irq_mask (0xffff)
	unsigned int reg_c_irq_mask:16;

	// h0028
	unsigned int /* padding 16 bit */:16;

	// h0029, bit: 14
	/* Force Interrupt Enable*/
	#define offset_of_reg_c_irq_force (82)
	#define mask_of_reg_c_irq_force (0xffff)
	unsigned int reg_c_irq_force:16;

	// h0029
	unsigned int /* padding 16 bit */:16;

	// h002a, bit: 14
	/* Interrupt Clear*/
	#define offset_of_reg_c_irq_clr (84)
	#define mask_of_reg_c_irq_clr (0xffff)
	unsigned int reg_c_irq_clr:16;

	// h002a
	unsigned int /* padding 16 bit */:16;

	// h002b, bit: 14
	/* Status of Interrupt on CPU side*/
	#define offset_of_reg_irq_final_status (86)
	#define mask_of_reg_irq_final_status (0xffff)
	unsigned int reg_irq_final_status:16;

	// h002b
	unsigned int /* padding 16 bit */:16;

	// h002c, bit: 14
	/* Status of Interrupt on IP side
	# [0]: Sensor Source VREF rising edge
	# [1]: Sensor Source VREF falling edge
	# [2] Hardware Flash Strobe done
	# [3] Update double buffer register done
	# [4]: AF Statistic done
	# [5]: ISP internal FIFO full
	# [6]: ISP busy rising edge
	# [7]: ISP busy falling edge
	# [8]: AWB Statistic done
	# [9]: WDMA done
	# [10]: RDMA done
	#[11]: WDMA FIFO full
	#[12]: PAD2ISP_VSYNC rising edge
	#[13]: PAD2ISP_VSYNC falling edge
	#[14]: isp_if vsync pulse
	#[15]: AE Statistic done*/
	#define offset_of_reg_irq_raw_status (88)
	#define mask_of_reg_irq_raw_status (0xffff)
	unsigned int reg_irq_raw_status:16;

	// h002c
	unsigned int /* padding 16 bit */:16;

	// h002d, bit: 3
	/* New Interrupt Mask*/
	#define offset_of_reg_c_irq_mask2 (90)
	#define mask_of_reg_c_irq_mask2 (0xf)
	unsigned int reg_c_irq_mask2:4;

	// h002d, bit: 7
	/* */
	unsigned int :4;

	// h002d, bit: 11
	/* New Force Interrupt Enable*/
	#define offset_of_reg_c_irq_force2 (90)
	#define mask_of_reg_c_irq_force2 (0xf00)
	unsigned int reg_c_irq_force2:4;

	// h002d, bit: 14
	/* */
	unsigned int :4;

	// h002d
	unsigned int /* padding 16 bit */:16;

	// h002e, bit: 3
	/* New Interrupt Clear*/
	#define offset_of_reg_c_irq_clr2 (92)
	#define mask_of_reg_c_irq_clr2 (0xf)
	unsigned int reg_c_irq_clr2:4;

	// h002e, bit: 14
	/* */
	unsigned int :12;

	// h002e
	unsigned int /* padding 16 bit */:16;

	// h002f, bit: 3
	/* New Status of Interrupt on CPU side*/
	#define offset_of_reg_irq_final_status2 (94)
	#define mask_of_reg_irq_final_status2 (0xf)
	unsigned int reg_irq_final_status2:4;

	// h002f, bit: 7
	/* */
	unsigned int :4;

	// h002f, bit: 11
	/* New Status of Interrupt on IP side
	[0] vdos every line
	[1] vdos line #0
	[2] vdos line #1
	[3] vdos line #2*/
	#define offset_of_reg_irq_raw_status2 (94)
	#define mask_of_reg_irq_raw_status2 (0xf00)
	unsigned int reg_irq_raw_status2:4;

	// h002f, bit: 14
	/* */
	unsigned int :4;

	// h002f
	unsigned int /* padding 16 bit */:16;

	// h0030, bit: 2
	/* YUV444 to YUV422 filter mode*/
	#define offset_of_reg_c_filter (96)
	#define mask_of_reg_c_filter (0x7)
	unsigned int reg_c_filter:3;

	// h0030, bit: 3
	/* reg_ipm_h_mirror*/
	#define offset_of_reg_ipm_h_mirror (96)
	#define mask_of_reg_ipm_h_mirror (0x8)
	unsigned int reg_ipm_h_mirror:1;

	// h0030, bit: 4
	/* reg_44to42_dith_en*/
	#define offset_of_reg_44to42_dith_en (96)
	#define mask_of_reg_44to42_dith_en (0x10)
	unsigned int reg_44to42_dith_en:1;

	// h0030, bit: 5
	/* reg_rgb2yuv_dith_en*/
	#define offset_of_reg_rgb2yuv_dith_en (96)
	#define mask_of_reg_rgb2yuv_dith_en (0x20)
	unsigned int reg_rgb2yuv_dith_en:1;

	// h0030, bit: 14
	/* */
	unsigned int :10;

	// h0030
	unsigned int /* padding 16 bit */:16;

	// h0031, bit: 0
	/* Color Correction Dither Enable*/
	#define offset_of_reg_color_c_dith_en (98)
	#define mask_of_reg_color_c_dith_en (0x1)
	unsigned int reg_color_c_dith_en:1;

	// h0031, bit: 14
	/* */
	unsigned int :15;

	// h0031
	unsigned int /* padding 16 bit */:16;

	// h0032, bit: 0
	/* reg_isp_color_c_cmc_en*/
	#define offset_of_reg_isp_cc_cmc_en (100)
	#define mask_of_reg_isp_cc_cmc_en (0x1)
	unsigned int reg_isp_cc_cmc_en:1;

	// h0032, bit: 1
	/* reg_isp_color_c_y_sub_16_en*/
	#define offset_of_reg_isp_cc_y_sub_16_en (100)
	#define mask_of_reg_isp_cc_y_sub_16_en (0x2)
	unsigned int reg_isp_cc_y_sub_16_en:1;

	// h0032, bit: 2
	/* reg_isp_color_c_r_sub_16_en*/
	#define offset_of_reg_isp_cc_r_sub_16_en (100)
	#define mask_of_reg_isp_cc_r_sub_16_en (0x4)
	unsigned int reg_isp_cc_r_sub_16_en:1;

	// h0032, bit: 3
	/* reg_isp_color_c_b_sub_16_en*/
	#define offset_of_reg_isp_cc_b_sub_16_en (100)
	#define mask_of_reg_isp_cc_b_sub_16_en (0x8)
	unsigned int reg_isp_cc_b_sub_16_en:1;

	// h0032, bit: 4
	/* reg_isp_color_c_y_add_16_post_en*/
	#define offset_of_reg_isp_cc_y_add_16_post_en (100)
	#define mask_of_reg_isp_cc_y_add_16_post_en (0x10)
	unsigned int reg_isp_cc_y_add_16_post_en:1;

	// h0032, bit: 5
	/* reg_isp_color_c_r_add_16_post_en*/
	#define offset_of_reg_isp_cc_r_add_16_post_en (100)
	#define mask_of_reg_isp_cc_r_add_16_post_en (0x20)
	unsigned int reg_isp_cc_r_add_16_post_en:1;

	// h0032, bit: 6
	/* reg_isp_color_c_b_add_16_post_en*/
	#define offset_of_reg_isp_cc_b_add_16_post_en (100)
	#define mask_of_reg_isp_cc_b_add_16_post_en (0x40)
	unsigned int reg_isp_cc_b_add_16_post_en:1;

	// h0032, bit: 7
	/* reg_isp_color_c_cb_add_128_post_en*/
	#define offset_of_reg_isp_cc_cb_add_128_post_en (100)
	#define mask_of_reg_isp_cc_cb_add_128_post_en (0x80)
	unsigned int reg_isp_cc_cb_add_128_post_en:1;

	// h0032, bit: 8
	/* reg_isp_color_c_cr_add_128_post_en*/
	#define offset_of_reg_isp_cc_cr_add_128_post_en (100)
	#define mask_of_reg_isp_cc_cr_add_128_post_en (0x100)
	unsigned int reg_isp_cc_cr_add_128_post_en:1;

	// h0032, bit: 9
	/* reg_isp_color_c_rran*/
	#define offset_of_reg_isp_cc_rran (100)
	#define mask_of_reg_isp_cc_rran (0x200)
	unsigned int reg_isp_cc_rran:1;

	// h0032, bit: 10
	/* reg_isp_color_c_gran*/
	#define offset_of_reg_isp_cc_gran (100)
	#define mask_of_reg_isp_cc_gran (0x400)
	unsigned int reg_isp_cc_gran:1;

	// h0032, bit: 11
	/* reg_isp_color_c_bran*/
	#define offset_of_reg_isp_cc_bran (100)
	#define mask_of_reg_isp_cc_bran (0x800)
	unsigned int reg_isp_cc_bran:1;

	// h0032, bit: 14
	/* */
	unsigned int :4;

	// h0032
	unsigned int /* padding 16 bit */:16;

	// h0033, bit: 12
	/* Coefficient 11 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_11 (102)
	#define mask_of_reg_isp_cc_coeff_11 (0x1fff)
	unsigned int reg_isp_cc_coeff_11:13;

	// h0033, bit: 14
	/* */
	unsigned int :3;

	// h0033
	unsigned int /* padding 16 bit */:16;

	// h0034, bit: 12
	/* Coefficient 12 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_12 (104)
	#define mask_of_reg_isp_cc_coeff_12 (0x1fff)
	unsigned int reg_isp_cc_coeff_12:13;

	// h0034, bit: 14
	/* */
	unsigned int :3;

	// h0034
	unsigned int /* padding 16 bit */:16;

	// h0035, bit: 12
	/* Coefficient 13 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_13 (106)
	#define mask_of_reg_isp_cc_coeff_13 (0x1fff)
	unsigned int reg_isp_cc_coeff_13:13;

	// h0035, bit: 14
	/* */
	unsigned int :3;

	// h0035
	unsigned int /* padding 16 bit */:16;

	// h0036, bit: 12
	/* Coefficient 21 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_21 (108)
	#define mask_of_reg_isp_cc_coeff_21 (0x1fff)
	unsigned int reg_isp_cc_coeff_21:13;

	// h0036, bit: 14
	/* */
	unsigned int :3;

	// h0036
	unsigned int /* padding 16 bit */:16;

	// h0037, bit: 12
	/* Coefficient 22 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_22 (110)
	#define mask_of_reg_isp_cc_coeff_22 (0x1fff)
	unsigned int reg_isp_cc_coeff_22:13;

	// h0037, bit: 14
	/* */
	unsigned int :3;

	// h0037
	unsigned int /* padding 16 bit */:16;

	// h0038, bit: 12
	/* Coefficient 23 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_23 (112)
	#define mask_of_reg_isp_cc_coeff_23 (0x1fff)
	unsigned int reg_isp_cc_coeff_23:13;

	// h0038, bit: 14
	/* */
	unsigned int :3;

	// h0038
	unsigned int /* padding 16 bit */:16;

	// h0039, bit: 12
	/* Coefficient 31 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_31 (114)
	#define mask_of_reg_isp_cc_coeff_31 (0x1fff)
	unsigned int reg_isp_cc_coeff_31:13;

	// h0039, bit: 14
	/* */
	unsigned int :3;

	// h0039
	unsigned int /* padding 16 bit */:16;

	// h003a, bit: 12
	/* Coefficient 32 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_32 (116)
	#define mask_of_reg_isp_cc_coeff_32 (0x1fff)
	unsigned int reg_isp_cc_coeff_32:13;

	// h003a, bit: 14
	/* */
	unsigned int :3;

	// h003a
	unsigned int /* padding 16 bit */:16;

	// h003b, bit: 12
	/* Coefficient 33 for Color Correction (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_cc_coeff_33 (118)
	#define mask_of_reg_isp_cc_coeff_33 (0x1fff)
	unsigned int reg_isp_cc_coeff_33:13;

	// h003b, bit: 14
	/* */
	unsigned int :3;

	// h003b
	unsigned int /* padding 16 bit */:16;

	// h003c, bit: 0
	/* reg_isp_r2y_cmc_en*/
	#define offset_of_reg_isp_r2y_cmc_en (120)
	#define mask_of_reg_isp_r2y_cmc_en (0x1)
	unsigned int reg_isp_r2y_cmc_en:1;

	// h003c, bit: 1
	/* reg_isp_r2y_y_sub_16_en*/
	#define offset_of_reg_isp_r2y_y_sub_16_en (120)
	#define mask_of_reg_isp_r2y_y_sub_16_en (0x2)
	unsigned int reg_isp_r2y_y_sub_16_en:1;

	// h003c, bit: 2
	/* reg_isp_r2y_r_sub_16_en*/
	#define offset_of_reg_isp_r2y_r_sub_16_en (120)
	#define mask_of_reg_isp_r2y_r_sub_16_en (0x4)
	unsigned int reg_isp_r2y_r_sub_16_en:1;

	// h003c, bit: 3
	/* reg_isp_r2y_b_sub_16_en*/
	#define offset_of_reg_isp_r2y_b_sub_16_en (120)
	#define mask_of_reg_isp_r2y_b_sub_16_en (0x8)
	unsigned int reg_isp_r2y_b_sub_16_en:1;

	// h003c, bit: 4
	/* reg_isp_r2y_y_add_16_post_en*/
	#define offset_of_reg_isp_r2y_y_add_16_post_en (120)
	#define mask_of_reg_isp_r2y_y_add_16_post_en (0x10)
	unsigned int reg_isp_r2y_y_add_16_post_en:1;

	// h003c, bit: 5
	/* reg_isp_r2y_r_add_16_post_en*/
	#define offset_of_reg_isp_r2y_r_add_16_post_en (120)
	#define mask_of_reg_isp_r2y_r_add_16_post_en (0x20)
	unsigned int reg_isp_r2y_r_add_16_post_en:1;

	// h003c, bit: 6
	/* reg_isp_r2y_b_add_16_post_en*/
	#define offset_of_reg_isp_r2y_b_add_16_post_en (120)
	#define mask_of_reg_isp_r2y_b_add_16_post_en (0x40)
	unsigned int reg_isp_r2y_b_add_16_post_en:1;

	// h003c, bit: 7
	/* reg_isp_r2y_cb_add_128_post_en*/
	#define offset_of_reg_isp_r2y_cb_add_128_post_en (120)
	#define mask_of_reg_isp_r2y_cb_add_128_post_en (0x80)
	unsigned int reg_isp_r2y_cb_add_128_post_en:1;

	// h003c, bit: 8
	/* reg_isp_r2y_cr_add_128_post_en*/
	#define offset_of_reg_isp_r2y_cr_add_128_post_en (120)
	#define mask_of_reg_isp_r2y_cr_add_128_post_en (0x100)
	unsigned int reg_isp_r2y_cr_add_128_post_en:1;

	// h003c, bit: 9
	/* reg_isp_r2y_rran*/
	#define offset_of_reg_isp_r2y_rran (120)
	#define mask_of_reg_isp_r2y_rran (0x200)
	unsigned int reg_isp_r2y_rran:1;

	// h003c, bit: 10
	/* reg_isp_r2y_gran*/
	#define offset_of_reg_isp_r2y_gran (120)
	#define mask_of_reg_isp_r2y_gran (0x400)
	unsigned int reg_isp_r2y_gran:1;

	// h003c, bit: 11
	/* reg_isp_r2y_bran*/
	#define offset_of_reg_isp_r2y_bran (120)
	#define mask_of_reg_isp_r2y_bran (0x800)
	unsigned int reg_isp_r2y_bran:1;

	// h003c, bit: 14
	/* */
	unsigned int :4;

	// h003c
	unsigned int /* padding 16 bit */:16;

	// h003d, bit: 12
	/* Coefficient 11 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_11 (122)
	#define mask_of_reg_isp_r2y_coeff_11 (0x1fff)
	unsigned int reg_isp_r2y_coeff_11:13;

	// h003d, bit: 14
	/* */
	unsigned int :3;

	// h003d
	unsigned int /* padding 16 bit */:16;

	// h003e, bit: 12
	/* Coefficient 12 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_12 (124)
	#define mask_of_reg_isp_r2y_coeff_12 (0x1fff)
	unsigned int reg_isp_r2y_coeff_12:13;

	// h003e, bit: 14
	/* */
	unsigned int :3;

	// h003e
	unsigned int /* padding 16 bit */:16;

	// h003f, bit: 12
	/* Coefficient 13 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_13 (126)
	#define mask_of_reg_isp_r2y_coeff_13 (0x1fff)
	unsigned int reg_isp_r2y_coeff_13:13;

	// h003f, bit: 14
	/* */
	unsigned int :3;

	// h003f
	unsigned int /* padding 16 bit */:16;

	// h0040, bit: 12
	/* Coefficient 21 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_21 (128)
	#define mask_of_reg_isp_r2y_coeff_21 (0x1fff)
	unsigned int reg_isp_r2y_coeff_21:13;

	// h0040, bit: 14
	/* */
	unsigned int :3;

	// h0040
	unsigned int /* padding 16 bit */:16;

	// h0041, bit: 12
	/* Coefficient 22 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_22 (130)
	#define mask_of_reg_isp_r2y_coeff_22 (0x1fff)
	unsigned int reg_isp_r2y_coeff_22:13;

	// h0041, bit: 14
	/* */
	unsigned int :3;

	// h0041
	unsigned int /* padding 16 bit */:16;

	// h0042, bit: 12
	/* Coefficient 23 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_23 (132)
	#define mask_of_reg_isp_r2y_coeff_23 (0x1fff)
	unsigned int reg_isp_r2y_coeff_23:13;

	// h0042, bit: 14
	/* */
	unsigned int :3;

	// h0042
	unsigned int /* padding 16 bit */:16;

	// h0043, bit: 12
	/* Coefficient 31 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_31 (134)
	#define mask_of_reg_isp_r2y_coeff_31 (0x1fff)
	unsigned int reg_isp_r2y_coeff_31:13;

	// h0043, bit: 14
	/* */
	unsigned int :3;

	// h0043
	unsigned int /* padding 16 bit */:16;

	// h0044, bit: 12
	/* Coefficient 32 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_32 (136)
	#define mask_of_reg_isp_r2y_coeff_32 (0x1fff)
	unsigned int reg_isp_r2y_coeff_32:13;

	// h0044, bit: 14
	/* */
	unsigned int :3;

	// h0044
	unsigned int /* padding 16 bit */:16;

	// h0045, bit: 12
	/* Coefficient 33 for RGB to YUV (s2.8)
	 ( single buffer register )*/
	#define offset_of_reg_isp_r2y_coeff_33 (138)
	#define mask_of_reg_isp_r2y_coeff_33 (0x1fff)
	unsigned int reg_isp_r2y_coeff_33:13;

	// h0045, bit: 14
	/* */
	unsigned int :3;

	// h0045
	unsigned int /* padding 16 bit */:16;

	// h0046, bit: 0
	/* Enable signal for gamma_10to12*/
	#define offset_of_reg_isp_gamma_10to12_en (140)
	#define mask_of_reg_isp_gamma_10to12_en (0x1)
	unsigned int reg_isp_gamma_10to12_en:1;

	// h0046, bit: 1
	/* Enable signal for gamma_10to12 gat max*/
	#define offset_of_reg_isp_gamma_10to12_max_en (140)
	#define mask_of_reg_isp_gamma_10to12_max_en (0x2)
	unsigned int reg_isp_gamma_10to12_max_en:1;

	// h0046, bit: 13
	/* Enable signal for gamma_10to12 max_data*/
	#define offset_of_reg_isp_gamma_10to12_max_data (140)
	#define mask_of_reg_isp_gamma_10to12_max_data (0x3ffc)
	unsigned int reg_isp_gamma_10to12_max_data:12;

	// h0046, bit: 14
	/* */
	unsigned int :2;

	// h0046
	unsigned int /* padding 16 bit */:16;

	// h0047, bit: 0
	/* Enable signal for gamma_correct*/
	#define offset_of_reg_isp_gamma_crct_en (142)
	#define mask_of_reg_isp_gamma_crct_en (0x1)
	unsigned int reg_isp_gamma_crct_en:1;

	// h0047, bit: 1
	/* Enable signal for gamma_correct gat max*/
	#define offset_of_reg_isp_gamma_crct_max_en (142)
	#define mask_of_reg_isp_gamma_crct_max_en (0x2)
	unsigned int reg_isp_gamma_crct_max_en:1;

	// h0047, bit: 11
	/* Enable signal for gamma_correct max data*/
	#define offset_of_reg_isp_gamma_crct_max_data (142)
	#define mask_of_reg_isp_gamma_crct_max_data (0xffc)
	unsigned int reg_isp_gamma_crct_max_data:10;

	// h0047, bit: 14
	/* */
	unsigned int :4;

	// h0047
	unsigned int /* padding 16 bit */:16;

	// h0048, bit: 14
	/* */
	unsigned int :16;

	// h0048
	unsigned int /* padding 16 bit */:16;

	// h0049, bit: 14
	/* */
	unsigned int :16;

	// h0049
	unsigned int /* padding 16 bit */:16;

	// h004a, bit: 14
	/* */
	unsigned int :16;

	// h004a
	unsigned int /* padding 16 bit */:16;

	// h004b, bit: 14
	/* */
	unsigned int :16;

	// h004b
	unsigned int /* padding 16 bit */:16;

	// h004c, bit: 14
	/* */
	unsigned int :16;

	// h004c
	unsigned int /* padding 16 bit */:16;

	// h004d, bit: 14
	/* */
	unsigned int :16;

	// h004d
	unsigned int /* padding 16 bit */:16;

	// h004e, bit: 14
	/* */
	unsigned int :16;

	// h004e
	unsigned int /* padding 16 bit */:16;

	// h004f, bit: 14
	/* */
	unsigned int :16;

	// h004f
	unsigned int /* padding 16 bit */:16;

	// h0050, bit: 14
	/* */
	unsigned int :16;

	// h0050
	unsigned int /* padding 16 bit */:16;

	// h0051, bit: 14
	/* */
	unsigned int :16;

	// h0051
	unsigned int /* padding 16 bit */:16;

	// h0052, bit: 0
	/* Gamma Correction Dither Enable*/
	#define offset_of_reg_gamma_c_dith_en (164)
	#define mask_of_reg_gamma_c_dith_en (0x1)
	unsigned int reg_gamma_c_dith_en:1;

	// h0052, bit: 14
	/* The 35th Gamma Table*/
	#define offset_of_reg_gamma_ctl_reserved (164)
	#define mask_of_reg_gamma_ctl_reserved (0xfffe)
	unsigned int reg_gamma_ctl_reserved:15;

	// h0052
	unsigned int /* padding 16 bit */:16;

	// h0053, bit: 14
	/* */
	unsigned int :16;

	// h0053
	unsigned int /* padding 16 bit */:16;

	// h0054, bit: 14
	/* */
	unsigned int :16;

	// h0054
	unsigned int /* padding 16 bit */:16;

	// h0055, bit: 14
	/* */
	unsigned int :16;

	// h0055
	unsigned int /* padding 16 bit */:16;

	// h0056, bit: 14
	/* */
	unsigned int :16;

	// h0056
	unsigned int /* padding 16 bit */:16;

	// h0057, bit: 14
	/* */
	unsigned int :16;

	// h0057
	unsigned int /* padding 16 bit */:16;

	// h0058, bit: 14
	/* */
	unsigned int :16;

	// h0058
	unsigned int /* padding 16 bit */:16;

	// h0059, bit: 14
	/* */
	unsigned int :16;

	// h0059
	unsigned int /* padding 16 bit */:16;

	// h005a, bit: 14
	/* */
	unsigned int :16;

	// h005a
	unsigned int /* padding 16 bit */:16;

	// h005b, bit: 14
	/* */
	unsigned int :16;

	// h005b
	unsigned int /* padding 16 bit */:16;

	// h005c, bit: 14
	/* */
	unsigned int :16;

	// h005c
	unsigned int /* padding 16 bit */:16;

	// h005d, bit: 14
	/* */
	unsigned int :16;

	// h005d
	unsigned int /* padding 16 bit */:16;

	// h005e, bit: 14
	/* */
	unsigned int :16;

	// h005e
	unsigned int /* padding 16 bit */:16;

	// h005f, bit: 14
	/* */
	unsigned int :16;

	// h005f
	unsigned int /* padding 16 bit */:16;

	// h0060, bit: 0
	/* */
	#define offset_of_reg_edge_enable (192)
	#define mask_of_reg_edge_enable (0x1)
	unsigned int reg_edge_enable:1;

	// h0060, bit: 14
	/* */
	unsigned int :15;

	// h0060
	unsigned int /* padding 16 bit */:16;

	// h0061, bit: 11
	/* blank space for m5x5 window engine for the last two lines*/
	#define offset_of_reg_blank_space (194)
	#define mask_of_reg_blank_space (0xfff)
	unsigned int reg_blank_space:12;

	// h0061, bit: 14
	/* */
	unsigned int :4;

	// h0061
	unsigned int /* padding 16 bit */:16;

	// h0062, bit: 0
	/* no-used*/
	#define offset_of_reg_rdpath_en (196)
	#define mask_of_reg_rdpath_en (0x1)
	unsigned int reg_rdpath_en:1;

	// h0062, bit: 1
	/* no-used*/
	#define offset_of_reg_wrpath_en (196)
	#define mask_of_reg_wrpath_en (0x2)
	unsigned int reg_wrpath_en:1;

	// h0062, bit: 14
	/* */
	unsigned int :14;

	// h0062
	unsigned int /* padding 16 bit */:16;

	// h0063, bit: 14
	/* REV for m3isp*/
	#define offset_of_reg_reserved1_m3isp (198)
	#define mask_of_reg_reserved1_m3isp (0xffff)
	unsigned int reg_reserved1_m3isp:16;

	// h0063
	unsigned int /* padding 16 bit */:16;

	// h0064, bit: 0
	/* */
	unsigned int :1;

	// h0064, bit: 1
	/* no-used*/
	#define offset_of_reg_isp_dbgw_en (200)
	#define mask_of_reg_isp_dbgw_en (0x2)
	unsigned int reg_isp_dbgw_en:1;

	// h0064, bit: 2
	/* */
	#define offset_of_reg_isp_dp_gated_clk_en (200)
	#define mask_of_reg_isp_dp_gated_clk_en (0x4)
	unsigned int reg_isp_dp_gated_clk_en:1;

	// h0064, bit: 3
	/* denoise function*/
	#define offset_of_reg_m3isp_rawdn_gated_clk_en (200)
	#define mask_of_reg_m3isp_rawdn_gated_clk_en (0x8)
	unsigned int reg_m3isp_rawdn_gated_clk_en:1;

	// h0064, bit: 4
	/* no-used*/
	#define offset_of_reg_m3isp_kernel_gated_clk_en (200)
	#define mask_of_reg_m3isp_kernel_gated_clk_en (0x10)
	unsigned int reg_m3isp_kernel_gated_clk_en:1;

	// h0064, bit: 5
	/* no-used*/
	#define offset_of_reg_isp_dp_gated_clk_mode (200)
	#define mask_of_reg_isp_dp_gated_clk_mode (0x20)
	unsigned int reg_isp_dp_gated_clk_mode:1;

	// h0064, bit: 6
	/* */
	unsigned int :1;

	// h0064, bit: 7
	/* 0: vsync double buffer
	1: frame end double buffer*/
	#define offset_of_reg_latch_mode (200)
	#define mask_of_reg_latch_mode (0x80)
	unsigned int reg_latch_mode:1;

	// h0064, bit: 8
	/* */
	#define offset_of_reg_sensor_hsync (200)
	#define mask_of_reg_sensor_hsync (0x100)
	unsigned int reg_sensor_hsync:1;

	// h0064, bit: 9
	/* */
	#define offset_of_reg_sensor_vsync (200)
	#define mask_of_reg_sensor_vsync (0x200)
	unsigned int reg_sensor_vsync:1;

	// h0064, bit: 14
	/* */
	unsigned int :6;

	// h0064
	unsigned int /* padding 16 bit */:16;

	// h0065, bit: 0
	/* no-used*/
	#define offset_of_reg_rdpath_swrst (202)
	#define mask_of_reg_rdpath_swrst (0x1)
	unsigned int reg_rdpath_swrst:1;

	// h0065, bit: 1
	/* no-used*/
	#define offset_of_reg_wrpath_swrst (202)
	#define mask_of_reg_wrpath_swrst (0x2)
	unsigned int reg_wrpath_swrst:1;

	// h0065, bit: 2
	/* wdma/rdma sw reset*/
	#define offset_of_reg_dbgwr_swrst (202)
	#define mask_of_reg_dbgwr_swrst (0x4)
	unsigned int reg_dbgwr_swrst:1;

	// h0065, bit: 14
	/* */
	unsigned int :13;

	// h0065
	unsigned int /* padding 16 bit */:16;

	// h0066, bit: 0
	/* 0: isp_dp reset by vsync
	1: isp_dp not reset by vsync*/
	#define offset_of_reg_isp_dp_rstz_mode (204)
	#define mask_of_reg_isp_dp_rstz_mode (0x1)
	unsigned int reg_isp_dp_rstz_mode:1;

	// h0066, bit: 1
	/* rawdn_clk force disable*/
	#define offset_of_reg_m3isp_rawdn_clk_force_dis (204)
	#define mask_of_reg_m3isp_rawdn_clk_force_dis (0x2)
	unsigned int reg_m3isp_rawdn_clk_force_dis:1;

	// h0066, bit: 2
	/* isp_dp_clk force disable*/
	#define offset_of_reg_isp_dp_clk_force_dis (204)
	#define mask_of_reg_isp_dp_clk_force_dis (0x4)
	unsigned int reg_isp_dp_clk_force_dis:1;

	// h0066, bit: 3
	/* Raw data down sample to DMA enable
	(double buffer register)*/
	#define offset_of_reg_dspl2wdma_en (204)
	#define mask_of_reg_dspl2wdma_en (0x8)
	unsigned int reg_dspl2wdma_en:1;

	// h0066, bit: 6
	/* down-sampling mode
	0 : whole image
	1 : 1/4
	2 : 1/16
	3 : 1/64
	4 : 1/256*/
	#define offset_of_reg_dsp_mode (204)
	#define mask_of_reg_dsp_mode (0x70)
	unsigned int reg_dsp_mode:3;

	// h0066, bit: 7
	/* 0: Enable isp2icp frame end reset
	1: Disable*/
	#define offset_of_reg_fend_rstz_mode (204)
	#define mask_of_reg_fend_rstz_mode (0x80)
	unsigned int reg_fend_rstz_mode:1;

	// h0066, bit: 14
	/* */
	unsigned int :8;

	// h0066
	unsigned int /* padding 16 bit */:16;

	// h0067, bit: 14
	/* */
	unsigned int :16;

	// h0067
	unsigned int /* padding 16 bit */:16;

	// h0068, bit: 3
	/* hsync pipe delay for YC mode(atv mode)*/
	#define offset_of_reg_hs_pipe (208)
	#define mask_of_reg_hs_pipe (0xf)
	unsigned int reg_hs_pipe:4;

	// h0068, bit: 4
	/* Mask hsync pipe delay for YC mode(atv mode)*/
	#define offset_of_reg_s2p_hs_mask (208)
	#define mask_of_reg_s2p_hs_mask (0x10)
	unsigned int reg_s2p_hs_mask:1;

	// h0068, bit: 5
	/* hsync pipe delay polarity for YC mode
	# 1'b0: high active
	# 1'b1: low active*/
	#define offset_of_reg_s2p_hs_pol (208)
	#define mask_of_reg_s2p_hs_pol (0x20)
	unsigned int reg_s2p_hs_pol:1;

	// h0068, bit: 14
	/* */
	unsigned int :10;

	// h0068
	unsigned int /* padding 16 bit */:16;

	// h0069, bit: 14
	/* */
	unsigned int :16;

	// h0069
	unsigned int /* padding 16 bit */:16;

	// h006a, bit: 0
	/* Video stabilization enable*/
	#define offset_of_reg_vdos_en (212)
	#define mask_of_reg_vdos_en (0x1)
	unsigned int reg_vdos_en:1;

	// h006a, bit: 1
	/* Video stabilization mode:
	0: bayer mode
	1: yc mode*/
	#define offset_of_reg_vdos_yc_en (212)
	#define mask_of_reg_vdos_yc_en (0x2)
	unsigned int reg_vdos_yc_en:1;

	// h006a, bit: 3
	/* */
	unsigned int :2;

	// h006a, bit: 4
	/* Video stabilization Y/G first flag
	0: first pix is NOT Y/G
	1: first pix is Y/G*/
	#define offset_of_reg_vdos_byer_mode (212)
	#define mask_of_reg_vdos_byer_mode (0x10)
	unsigned int reg_vdos_byer_mode:1;

	// h006a, bit: 14
	/* */
	unsigned int :11;

	// h006a
	unsigned int /* padding 16 bit */:16;

	// h006b, bit: 2
	/* Number of blanking lines for sram dump*/
	#define offset_of_reg_vdos_sblock_blk (214)
	#define mask_of_reg_vdos_sblock_blk (0x7)
	unsigned int reg_vdos_sblock_blk:3;

	// h006b, bit: 7
	/* */
	unsigned int :5;

	// h006b, bit: 14
	/* */
	#define offset_of_reg_vdos_dummy (214)
	#define mask_of_reg_vdos_dummy (0xff00)
	unsigned int reg_vdos_dummy:8;

	// h006b
	unsigned int /* padding 16 bit */:16;

	// h006c, bit: 12
	/* Video stabilization X crop start, > 0*/
	#define offset_of_reg_vdos_x_st (216)
	#define mask_of_reg_vdos_x_st (0x1fff)
	unsigned int reg_vdos_x_st:13;

	// h006c, bit: 14
	/* */
	unsigned int :3;

	// h006c
	unsigned int /* padding 16 bit */:16;

	// h006d, bit: 11
	/* Video stabilization Y crop start, > 0*/
	#define offset_of_reg_vdos_y_st (218)
	#define mask_of_reg_vdos_y_st (0xfff)
	unsigned int reg_vdos_y_st:12;

	// h006d, bit: 14
	/* */
	unsigned int :4;

	// h006d
	unsigned int /* padding 16 bit */:16;

	// h006e, bit: 11
	/* Video stabilization sub block width*/
	#define offset_of_reg_vdos_sblock_width (220)
	#define mask_of_reg_vdos_sblock_width (0xfff)
	unsigned int reg_vdos_sblock_width:12;

	// h006e, bit: 14
	/* */
	unsigned int :4;

	// h006e
	unsigned int /* padding 16 bit */:16;

	// h006f, bit: 11
	/* Video stabilization sub block height*/
	#define offset_of_reg_vdos_sblock_height (222)
	#define mask_of_reg_vdos_sblock_height (0xfff)
	unsigned int reg_vdos_sblock_height:12;

	// h006f, bit: 14
	/* */
	unsigned int :4;

	// h006f
	unsigned int /* padding 16 bit */:16;

	// h0070, bit: 0
	/* Enable CFA Interpolation (Demosaic)
	# 1'b0: Disable
	# 1'b1: Enable
	 ( single buffer register )*/
	#define offset_of_reg_cfai_en (224)
	#define mask_of_reg_cfai_en (0x1)
	unsigned int reg_cfai_en:1;

	// h0070, bit: 1
	/* Enable CFAI Data Bypass (output = m33)
	# 1'b0: Disable
	# 1'b1: Enable
	 ( single buffer register )*/
	#define offset_of_reg_cfai_bypass (224)
	#define mask_of_reg_cfai_bypass (0x2)
	unsigned int reg_cfai_bypass:1;

	// h0070, bit: 2
	/* demosaic noise reduction average g enable
	0: disable
	1: enable (use average g for blending)*/
	#define offset_of_reg_cfai_dnr_gavg_ref_en (224)
	#define mask_of_reg_cfai_dnr_gavg_ref_en (0x4)
	unsigned int reg_cfai_dnr_gavg_ref_en:1;

	// h0070, bit: 3
	/* emosaic noise reduction enable
	0: disable
	1: enable */
	#define offset_of_reg_cfai_dnr_en (224)
	#define mask_of_reg_cfai_dnr_en (0x8)
	unsigned int reg_cfai_dnr_en:1;

	// h0070, bit: 7
	/* */
	unsigned int :4;

	// h0070, bit: 10
	/* demosaic noise reduction rb channel blending ratio*/
	#define offset_of_reg_cfai_dnr_rb_ratio (224)
	#define mask_of_reg_cfai_dnr_rb_ratio (0x700)
	unsigned int reg_cfai_dnr_rb_ratio:3;

	// h0070, bit: 11
	/* */
	unsigned int :1;

	// h0070, bit: 14
	/* demosaic noise reduction g channel blending ratio*/
	#define offset_of_reg_cfai_dnr_g_ratio (224)
	#define mask_of_reg_cfai_dnr_g_ratio (0x7000)
	unsigned int reg_cfai_dnr_g_ratio:3;

	// h0070, bit: 15
	/* */
	unsigned int :1;

	// h0070
	unsigned int /* padding 16 bit */:16;

	// h0071, bit: 3
	/* Demosaic DVH Edge Wieghting Adjust Slope
	(u4)*/
	#define offset_of_reg_demosaic_dvh_slope (226)
	#define mask_of_reg_demosaic_dvh_slope (0xf)
	unsigned int reg_demosaic_dvh_slope:4;

	// h0071, bit: 7
	/* */
	unsigned int :4;

	// h0071, bit: 11
	/* Demosaic DVH Edge Wieghting Threshold
	(u4)*/
	#define offset_of_reg_demosaic_dvh_thrd (226)
	#define mask_of_reg_demosaic_dvh_thrd (0xf00)
	unsigned int reg_demosaic_dvh_thrd:4;

	// h0071, bit: 14
	/* */
	unsigned int :4;

	// h0071
	unsigned int /* padding 16 bit */:16;

	// h0072, bit: 3
	/* Demosaic Complex Region Wieghting Adjust Slope
	(u0.4)*/
	#define offset_of_reg_demosaic_cpxwei_slope (228)
	#define mask_of_reg_demosaic_cpxwei_slope (0xf)
	unsigned int reg_demosaic_cpxwei_slope:4;

	// h0072, bit: 7
	/* */
	unsigned int :4;

	// h0072, bit: 14
	/* Demosaic Complex Region Wieghting Threshold
	(u8)*/
	#define offset_of_reg_demosaic_cpxwei_thrd (228)
	#define mask_of_reg_demosaic_cpxwei_thrd (0xff00)
	unsigned int reg_demosaic_cpxwei_thrd:8;

	// h0072
	unsigned int /* padding 16 bit */:16;

	// h0073, bit: 3
	/* Neighbor Check Slope M (u2.2)
	for reduce the false color on high freq. horizontal / vertical line with width 1*/
	#define offset_of_reg_nb_slope_m (230)
	#define mask_of_reg_nb_slope_m (0xf)
	unsigned int reg_nb_slope_m:4;

	// h0073, bit: 7
	/* */
	unsigned int :4;

	// h0073, bit: 11
	/* Neighbor Check Slope S (u3.1)
	for reduce the false color on high freq. horizontal / vertical line with width 1*/
	#define offset_of_reg_nb_slope_s (230)
	#define mask_of_reg_nb_slope_s (0xf00)
	unsigned int reg_nb_slope_s:4;

	// h0073, bit: 14
	/* */
	unsigned int :4;

	// h0073
	unsigned int /* padding 16 bit */:16;

	// h0074, bit: 0
	/* False color suppression enable
	0: disable
	1: enable */
	#define offset_of_reg_rgb_falsecolor_en (232)
	#define mask_of_reg_rgb_falsecolor_en (0x1)
	unsigned int reg_rgb_falsecolor_en:1;

	// h0074, bit: 1
	/* minmax rounding enable*/
	#define offset_of_reg_rgb_falsecolor_minmax_round (232)
	#define mask_of_reg_rgb_falsecolor_minmax_round (0x2)
	unsigned int reg_rgb_falsecolor_minmax_round:1;

	// h0074, bit: 3
	/* */
	unsigned int :2;

	// h0074, bit: 6
	/* blending ratio*/
	#define offset_of_reg_rgb_falsecolor_color_ratio (232)
	#define mask_of_reg_rgb_falsecolor_color_ratio (0x70)
	unsigned int reg_rgb_falsecolor_color_ratio:3;

	// h0074, bit: 7
	/* */
	unsigned int :1;

	// h0074, bit: 10
	/* blending ratio*/
	#define offset_of_reg_rgb_falsecolor_r_ratio (232)
	#define mask_of_reg_rgb_falsecolor_r_ratio (0x700)
	unsigned int reg_rgb_falsecolor_r_ratio:3;

	// h0074, bit: 11
	/* */
	unsigned int :1;

	// h0074, bit: 14
	/* blending ratio*/
	#define offset_of_reg_rgb_falsecolor_b_ratio (232)
	#define mask_of_reg_rgb_falsecolor_b_ratio (0x7000)
	unsigned int reg_rgb_falsecolor_b_ratio:3;

	// h0074, bit: 15
	/* */
	unsigned int :1;

	// h0074
	unsigned int /* padding 16 bit */:16;

	// h0075, bit: 14
	/* */
	unsigned int :16;

	// h0075
	unsigned int /* padding 16 bit */:16;

	// h0076, bit: 14
	/* */
	unsigned int :16;

	// h0076
	unsigned int /* padding 16 bit */:16;

	// h0077, bit: 14
	/* */
	unsigned int :16;

	// h0077
	unsigned int /* padding 16 bit */:16;

	// h0078, bit: 14
	/* */
	unsigned int :16;

	// h0078
	unsigned int /* padding 16 bit */:16;

	// h0079, bit: 14
	/* */
	unsigned int :16;

	// h0079
	unsigned int /* padding 16 bit */:16;

	// h007a, bit: 6
	/* isp frame start count (vsync)*/
	#define offset_of_reg_isp_frm_init_cnt (244)
	#define mask_of_reg_isp_frm_init_cnt (0x7f)
	unsigned int reg_isp_frm_init_cnt:7;

	// h007a, bit: 7
	/* */
	unsigned int :1;

	// h007a, bit: 14
	/* isp frame done count*/
	#define offset_of_reg_isp_frm_done_cnt (244)
	#define mask_of_reg_isp_frm_done_cnt (0x7f00)
	unsigned int reg_isp_frm_done_cnt:7;

	// h007a, bit: 15
	/* Clear isp frame init/done count*/
	#define offset_of_reg_isp_clr_frm_cnt (244)
	#define mask_of_reg_isp_clr_frm_cnt (0x8000)
	unsigned int reg_isp_clr_frm_cnt:1;

	// h007a
	unsigned int /* padding 16 bit */:16;

	// h007b, bit: 11
	/* input HSYNC count*/
	#define offset_of_reg_input_hs_cnt (246)
	#define mask_of_reg_input_hs_cnt (0xfff)
	unsigned int reg_input_hs_cnt:12;

	// h007b, bit: 14
	/* */
	unsigned int :4;

	// h007b
	unsigned int /* padding 16 bit */:16;

	// h007c, bit: 11
	/* input DE count*/
	#define offset_of_reg_input_de_cnt (248)
	#define mask_of_reg_input_de_cnt (0xfff)
	unsigned int reg_input_de_cnt:12;

	// h007c, bit: 14
	/* */
	unsigned int :4;

	// h007c
	unsigned int /* padding 16 bit */:16;

	// h007d, bit: 14
	/* */
	unsigned int :16;

	// h007d
	unsigned int /* padding 16 bit */:16;

	// h007e, bit: 11
	/* SW specify interrupt line number*/
	#define offset_of_reg_sw_specify_int_line (252)
	#define mask_of_reg_sw_specify_int_line (0xfff)
	unsigned int reg_sw_specify_int_line:12;

	// h007e, bit: 14
	/* */
	unsigned int :4;

	// h007e
	unsigned int /* padding 16 bit */:16;

	// h007f, bit: 7
	/* debug status*/
	#define offset_of_reg_debug_status_out (254)
	#define mask_of_reg_debug_status_out (0xff)
	unsigned int reg_debug_status_out:8;

	// h007f, bit: 14
	/* clear debug status*/
	#define offset_of_reg_debug_status_clr (254)
	#define mask_of_reg_debug_status_clr (0xff00)
	unsigned int reg_debug_status_clr:8;

	// h007f
	unsigned int /* padding 16 bit */:16;

}  __attribute__((packed, aligned(1))) infinity_reg_isp0;
#endif
