/*
* irqs.h- Sigmastar
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
/*------------------------------------------------------------------------------
    Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/

#ifndef __IRQS_H
#define __IRQS_H

/* [GIC irqchip]
        ID   0 -  15 : SGI
        ID  16 -  31 : PPI
        ID  32 -  95 : SPI:MS_IRQ0
        ID  96 -  159 : SPI:MS_FIQ0
        ID  160 - 191 : SPI:Debug/Profiling(ARM_INTERNAL named in I3)
        ID  192 - 255 : SPI:MS_IRQ1
        ID  256 - 319 : SPI:MS_FIQ1
*/

#define GIC_SGI_NR                 16
#define GIC_PPI_NR                 16
#define GIC_SPI_ARM_INTERNAL_NR    32
#define GIC_HWIRQ_MS_START        (GIC_SGI_NR + GIC_PPI_NR)

//for GIC_ID
#define GIC_ID_LEGACY_FIQ       0x1C
#define GIC_ID_LEGACY_IRQ       0x1F
#define GIC_ID_LOCAL_TIMER_IRQ  0x1D


/*  The folloing list are used in dtsi and get number by of_irq,
    if need to get the interrupt number for request_irq(), manual calculate the number is
    GIC_SGI_NR+GIC_PPI_NR+X=32+X
 NOTE: We count from GIC_SPI_ARM_INTERNAL because interrupt delcaration in dts is from SPI 0 */

/* MS_NON_PM_IRQ SPI_ID:0-63 */
#define GIC_SPI_MS_IRQ_START                (0)
#define INT_IRQ_00_INT_UART0                (GIC_SPI_MS_IRQ_START +  0) //	gic	32	spi	0
#define INT_IRQ_01_MIIC1_INT                (GIC_SPI_MS_IRQ_START +  1) //	gic	33	spi	1
#define INT_IRQ_02_USB30M1_SS_INT           (GIC_SPI_MS_IRQ_START +  2)	//	gic	34	spi	2
#define INT_IRQ_03_USB30M1_HS_UHC_INT       (GIC_SPI_MS_IRQ_START +  3)	//	gic	35	spi	3
#define INT_IRQ_04_TILDE_REG_TOP_GPIO_IN_2  (GIC_SPI_MS_IRQ_START +  4)	//	gic	36	spi	4
#define INT_IRQ_05_U3D_INT                  (GIC_SPI_MS_IRQ_START +  5)	//	gic	37	spi	5
#define INT_IRQ_06_USB_INT                  (GIC_SPI_MS_IRQ_START +  6)	//	gic	38	spi	6
#define INT_IRQ_07_UHC_INT                  (GIC_SPI_MS_IRQ_START +  7)	//	gic	39	spi	7
#define INT_IRQ_08_REG_CMDQ_DUMMY_15		(GIC_SPI_MS_IRQ_START +  8)	//	gic	40	spi	8
#define INT_IRQ_09_GMAC_INT                 (GIC_SPI_MS_IRQ_START +  9)	//	gic	41	spi	9
#define INT_IRQ_10_DISP_INT                 (GIC_SPI_MS_IRQ_START + 10)	//	gic	42	spi	10
#define INT_IRQ_11_RESERVED                 (GIC_SPI_MS_IRQ_START + 11)	//	gic	43	spi	11
#define INT_IRQ_12_MSPI_INT                 (GIC_SPI_MS_IRQ_START + 12)	//	gic	44	spi	12
#define INT_IRQ_13_EVD_INT                  (GIC_SPI_MS_IRQ_START + 13)	//	gic	45	spi	13
#define INT_IRQ_14_SATA_PHY_IRQ             (GIC_SPI_MS_IRQ_START + 14)	//	gic	46	spi	14
#define INT_IRQ_15_SATA_INTRQ               (GIC_SPI_MS_IRQ_START + 15)	//	gic	47	spi	15
#define INT_IRQ_16_SATA_P1_INTRQ            (GIC_SPI_MS_IRQ_START + 16)	//	gic	48	spi	16
#define INT_IRQ_17_VE_INT                   (GIC_SPI_MS_IRQ_START + 17)	//	gic	49	spi	17
#define INT_IRQ_18_IRQ_AEON2HI              (GIC_SPI_MS_IRQ_START + 18)	//	gic	50	spi	18
#define INT_IRQ_19_DC_INT                   (GIC_SPI_MS_IRQ_START + 19)	//	gic	51	spi	19
#define INT_IRQ_20_GOP_INT                  (GIC_SPI_MS_IRQ_START + 20)	//	gic	52	spi	20
#define INT_IRQ_21_PCM2MCU_INT              (GIC_SPI_MS_IRQ_START + 21)	//	gic	53	spi	21
#define INT_IRQ_22_MIIC0_INT                (GIC_SPI_MS_IRQ_START + 22)	//	gic	54	spi	22
#define INT_IRQ_23_RTC0_INT                 (GIC_SPI_MS_IRQ_START + 23)	//	gic	55	spi	23
#define INT_IRQ_24_KEYPAD_INT               (GIC_SPI_MS_IRQ_START + 24)	//	gic	56	spi	24
#define INT_IRQ_25_PM_INT                   (GIC_SPI_MS_IRQ_START + 25)	//	gic	57	spi	25
#define INT_IRQ_26_MFE_INT                  (GIC_SPI_MS_IRQ_START + 26)	//	gic	58	spi	26
#define INT_IRQ_27_REG_CMDQ2_DUMMY_15       (GIC_SPI_MS_IRQ_START + 27)	//	gic	59	spi	27
#define INT_IRQ_28_REG_CMDQ3_DUMMY_15       (GIC_SPI_MS_IRQ_START + 28)	//	gic	60	spi	28
#define INT_IRQ_29_RTC1_INT                 (GIC_SPI_MS_IRQ_START + 29)	//	gic	61	spi	29
#define INT_IRQ_30_SECGMAC_INT              (GIC_SPI_MS_IRQ_START + 30)	//	gic	62	spi	30
#define INT_IRQ_31_RESERVED                 (GIC_SPI_MS_IRQ_START + 31)	//	gic	63	spi	31
#define INT_IRQ_32_REG_CMDQ4_DUMMY_15       (GIC_SPI_MS_IRQ_START + 32)	//	gic	64	spi	32
#define INT_IRQ_33_USB_INT1                 (GIC_SPI_MS_IRQ_START + 33)	//	gic	65	spi	33
#define INT_IRQ_34_UHC_INT1                 (GIC_SPI_MS_IRQ_START + 34)	//	gic	66	spi	34
#define INT_IRQ_35_MIU_INT                  (GIC_SPI_MS_IRQ_START + 35)	//	gic	67	spi	35
#define INT_IRQ_36_ERROR_RESP_INT           (GIC_SPI_MS_IRQ_START + 36)	//	gic	68	spi	36
#define INT_IRQ_37_OTG_INT_P0               (GIC_SPI_MS_IRQ_START + 37)	//	gic	69	spi	37
#define INT_IRQ_38_U3_PCIE_PHY_IRQ_OUT      (GIC_SPI_MS_IRQ_START + 38)	//	gic	70	spi	38
#define INT_IRQ_39_INT_UART1                (GIC_SPI_MS_IRQ_START + 39)	//	gic	71	spi	39
#define INT_IRQ_40_HVD_INT                  (GIC_SPI_MS_IRQ_START + 40)	//	gic	72	spi	40
#define INT_IRQ_41_REG_TOP_GPIO_IN_4        (GIC_SPI_MS_IRQ_START + 41)	//	gic	73	spi	41
#define INT_IRQ_42_TILDE_REG_TOP_GPIO_IN_4  (GIC_SPI_MS_IRQ_START + 42)	//	gic	74	spi	42
#define INT_IRQ_43_GMAC_TX_INT              (GIC_SPI_MS_IRQ_START + 43)	//	gic	75	spi	43
#define INT_IRQ_44_SECGMAC_TX_INT           (GIC_SPI_MS_IRQ_START + 44)	//	gic	76	spi	44
#define INT_IRQ_45_JPD_INT                  (GIC_SPI_MS_IRQ_START + 45)	//	gic	77	spi	45
#define INT_IRQ_46_DISP1_INT                (GIC_SPI_MS_IRQ_START + 46)	//	gic	78	spi	46
#define INT_IRQ_47_PWD_STATUS_INT           (GIC_SPI_MS_IRQ_START + 47)	//	gic	79	spi	47
#define INT_IRQ_48_INT_BDMA_0              	(GIC_SPI_MS_IRQ_START + 48)	//	gic	80	spi	48
#define INT_IRQ_49_INT_BDMA_1              	(GIC_SPI_MS_IRQ_START + 49)	//	gic	81	spi	49
#define INT_IRQ_50_UART2MCU_INTR            (GIC_SPI_MS_IRQ_START + 50)	//	gic	82	spi	50
#define INT_IRQ_51_URDMA2MCU_INTR           (GIC_SPI_MS_IRQ_START + 51)	//	gic	83	spi	51
#define INT_IRQ_52_DVI_HDMI_HDCP_INT        (GIC_SPI_MS_IRQ_START + 52)	//	gic	84	spi	52
#define INT_IRQ_53_CEC_IRQ_OUT              (GIC_SPI_MS_IRQ_START + 53)	//	gic	85	spi	53
#define INT_IRQ_54_HDMITX_IRQ_LEVEL         (GIC_SPI_MS_IRQ_START + 54)	//	gic	86	spi	54
#define INT_IRQ_55_FCIE_INT                 (GIC_SPI_MS_IRQ_START + 55)	//	gic	87	spi	55
#define INT_IRQ_56_RESERVED                 (GIC_SPI_MS_IRQ_START + 56)	//	gic	88	spi	56
#define INT_IRQ_57_REG_CMDQ5_DUMMY          (GIC_SPI_MS_IRQ_START + 57)	//	gic	89	spi	57
#define INT_IRQ_58_SAR1_INT                 (GIC_SPI_MS_IRQ_START + 58)	//	gic	90	spi	58
#define INT_IRQ_59_IRQ_DAC_PLUG_DET         (GIC_SPI_MS_IRQ_START + 59)	//	gic	91	spi	59
#define INT_IRQ_60_REG_TOP_GPIO_IN_2        (GIC_SPI_MS_IRQ_START + 60)	//	gic	92	spi	60
#define INT_IRQ_61_RESERVED                 (GIC_SPI_MS_IRQ_START + 61)	//	gic	93	spi	61
#define INT_IRQ_62_FI_QUEUE_INT             (GIC_SPI_MS_IRQ_START + 62)	//	gic	94	spi	62
#define INT_IRQ_63_IRQ_FRM_PM               (GIC_SPI_MS_IRQ_START + 63)	//	gic	95	spi	63
#define GIC_SPI_MS_IRQ_END                  (GIC_SPI_MS_IRQ_START + 64)

#define GIC_SPI_MS_IRQ_NR                   (GIC_SPI_MS_IRQ_END - GIC_SPI_MS_IRQ_START)

/* MS_NON_PM_FIQ SPI_ID:64-127 */
#define GIC_SPI_MS_FIQ_START                (GIC_SPI_MS_IRQ_END)
#define INT_FIQ_00_INT_TIMER0               (GIC_SPI_MS_FIQ_START +  0) //	gic	96	spi	64
#define INT_FIQ_01_INT_TIMER1               (GIC_SPI_MS_FIQ_START +  1) //	gic	97	spi	65
#define INT_FIQ_02_INT_WDT                  (GIC_SPI_MS_FIQ_START +  2)	//	gic	98	spi	66
#define INT_FIQ_03_INT_SEC_TIMER0           (GIC_SPI_MS_FIQ_START +  3)	//	gic	99	spi	67
#define INT_FIQ_04_INT_SEC_TIMER1           (GIC_SPI_MS_FIQ_START +  4)	//	gic	100	spi	68
#define INT_FIQ_05_RESERVED                 (GIC_SPI_MS_FIQ_START +  5)	//	gic	101	spi	69
#define INT_FIQ_06_RESERVED                 (GIC_SPI_MS_FIQ_START +  6)	//	gic	102	spi	70
#define INT_FIQ_07_RESERVED                 (GIC_SPI_MS_FIQ_START +  7)	//	gic	103	spi	71
#define INT_FIQ_08_RESERVED                 (GIC_SPI_MS_FIQ_START +  8)	//	gic	104	spi	72
#define INT_FIQ_09_RESERVED                 (GIC_SPI_MS_FIQ_START +  9)	//	gic	105	spi	73
#define INT_FIQ_10_RESERVED                 (GIC_SPI_MS_FIQ_START + 10)	//	gic	106	spi	74
#define INT_FIQ_11_RESERVED                 (GIC_SPI_MS_FIQ_START + 11)	//	gic	107	spi	75
#define INT_FIQ_12_RESERVED                 (GIC_SPI_MS_FIQ_START + 12)	//	gic	108	spi	76
#define INT_FIQ_13_LAN_ESD_INT              (GIC_SPI_MS_FIQ_START + 13)	//	gic	109	spi	77
#define INT_FIQ_14_RESERVED                 (GIC_SPI_MS_FIQ_START + 14)	//	gic	110	spi	78
#define INT_FIQ_15_RESERVED                 (GIC_SPI_MS_FIQ_START + 15)	//	gic	111	spi	79
#define INT_FIQ_16_RESERVED                 (GIC_SPI_MS_FIQ_START + 16)	//	gic	112	spi	80
#define INT_FIQ_17_RESERVED                 (GIC_SPI_MS_FIQ_START + 17)	//	gic	113	spi	81
#define INT_FIQ_18_RESERVED                 (GIC_SPI_MS_FIQ_START + 18)	//	gic	114	spi	82
#define INT_FIQ_19_XIU_TIMEOUT_INT          (GIC_SPI_MS_FIQ_START + 19)	//	gic	115	spi	83
#define INT_FIQ_20_RESERVED                 (GIC_SPI_MS_FIQ_START + 20)	//	gic	116	spi	84
#define INT_FIQ_21_VE_VBI_F0_INT            (GIC_SPI_MS_FIQ_START + 21)	//	gic	117	spi	85
#define INT_FIQ_22_VE_VBI_F1_INT            (GIC_SPI_MS_FIQ_START + 22)	//	gic	118	spi	86
#define INT_FIQ_23_RESERVED                 (GIC_SPI_MS_FIQ_START + 23)	//	gic	119	spi	87
#define INT_FIQ_24_VE_DONE_TT_IRQ           (GIC_SPI_MS_FIQ_START + 24)	//	gic	120	spi	88
#define INT_FIQ_25_RESERVED                 (GIC_SPI_MS_FIQ_START + 25)	//	gic	121	spi	89
#define INT_FIQ_26_RESERVED                 (GIC_SPI_MS_FIQ_START + 26)	//	gic	122	spi	90
#define INT_FIQ_27_IR_INT                   (GIC_SPI_MS_FIQ_START + 27)	//	gic	123	spi	91
#define INT_FIQ_28_RESERVED                 (GIC_SPI_MS_FIQ_START + 28)	//	gic	124	spi	92
#define INT_FIQ_29_RESERVED                 (GIC_SPI_MS_FIQ_START + 29)	//	gic	125	spi	93
#define INT_FIQ_30_RESERVED                 (GIC_SPI_MS_FIQ_START + 30)	//	gic	126	spi	94
#define INT_FIQ_31_RESERVED                 (GIC_SPI_MS_FIQ_START + 31)	//	gic	127	spi	95
#define INT_FIQ_32_IR_INT_RC                (GIC_SPI_MS_FIQ_START + 32)	//	gic	128	spi	96
#define INT_FIQ_33_RESERVED                 (GIC_SPI_MS_FIQ_START + 33)	//	gic	129	spi	97
#define INT_FIQ_34_VE_SW_WR2BUF_INT         (GIC_SPI_MS_FIQ_START + 34)	//	gic	130	spi	98
#define INT_FIQ_35_RESERVED                 (GIC_SPI_MS_FIQ_START + 35)	//	gic	131	spi	99
#define INT_FIQ_36_REG_HST0TO3_INT          (GIC_SPI_MS_FIQ_START + 36)	//	gic	132	spi	100
#define INT_FIQ_37_REG_HST0TO2_INT          (GIC_SPI_MS_FIQ_START + 37)	//	gic	133	spi	101
#define INT_FIQ_38_REG_HST0TO1_INT          (GIC_SPI_MS_FIQ_START + 38)	//	gic	134	spi	102
#define INT_FIQ_39_REG_HST0TO4_IN           (GIC_SPI_MS_FIQ_START + 39)	//	gic	135	spi	103
#define INT_FIQ_40_REG_HST1TO3_INT          (GIC_SPI_MS_FIQ_START + 40)	//	gic	136	spi	104
#define INT_FIQ_41_REG_HST1TO2_INT          (GIC_SPI_MS_FIQ_START + 41)	//	gic	137	spi	105
#define INT_FIQ_42_REG_HST1TO0_INT          (GIC_SPI_MS_FIQ_START + 42)	//	gic	138	spi	106
#define INT_FIQ_43_REG_HST1TO4_INT          (GIC_SPI_MS_FIQ_START + 43)	//	gic	139	spi	107
#define INT_FIQ_44_REG_HST2TO3_INT          (GIC_SPI_MS_FIQ_START + 44)	//	gic	140	spi	108
#define INT_FIQ_45_REG_HST2TO1_INT          (GIC_SPI_MS_FIQ_START + 45)	//	gic	141	spi	109
#define INT_FIQ_46_REG_HST2TO0_INT          (GIC_SPI_MS_FIQ_START + 46)	//	gic	142	spi	110
#define INT_FIQ_47_REG_HST2TO4_INT          (GIC_SPI_MS_FIQ_START + 47)	//	gic	143	spi	111
#define INT_FIQ_48_REG_HST3TO2_INT          (GIC_SPI_MS_FIQ_START + 48)	//	gic	144	spi	112
#define INT_FIQ_49_REG_HST3TO1_INT          (GIC_SPI_MS_FIQ_START + 49)	//	gic	145	spi	113
#define INT_FIQ_50_REG_HST3TO0_INT          (GIC_SPI_MS_FIQ_START + 50) //	gic	146	spi	114
#define INT_FIQ_51_REG_HST3TO4_INT          (GIC_SPI_MS_FIQ_START + 51) //	gic	147	spi	115
#define INT_FIQ_52_RESERVED                 (GIC_SPI_MS_FIQ_START + 52) //	gic	148	spi	116
#define INT_FIQ_53_RESERVED                 (GIC_SPI_MS_FIQ_START + 53) //	gic	149	spi	117
#define INT_FIQ_54_HDMITX_IRQ_EDGE          (GIC_SPI_MS_FIQ_START + 54) //	gic	150	spi	118
#define INT_FIQ_55_RESERVED                 (GIC_SPI_MS_FIQ_START + 55) //	gic	151	spi	119
#define INT_FIQ_56_RESERVED                 (GIC_SPI_MS_FIQ_START + 56) //	gic	152	spi	120
#define INT_FIQ_57_REG_HST4TO3_INT          (GIC_SPI_MS_FIQ_START + 57) //	gic	153	spi	121
#define INT_FIQ_58_REG_HST4TO2_INT          (GIC_SPI_MS_FIQ_START + 58) //	gic	154	spi	122
#define INT_FIQ_59_REG_HST4TO1_INT          (GIC_SPI_MS_FIQ_START + 59) //	gic	155	spi	123
#define INT_FIQ_60_REG_HST4TO0_INT          (GIC_SPI_MS_FIQ_START + 60) //	gic	156	spi	124
#define INT_FIQ_61_RESERVED                 (GIC_SPI_MS_FIQ_START + 61) //	gic	157	spi	125
#define INT_FIQ_62_RESERVED                 (GIC_SPI_MS_FIQ_START + 62) //	gic	158	spi	126
#define INT_FIQ_63_FIQ_FRM_PM               (GIC_SPI_MS_FIQ_START + 63) //	gic	159	spi	127    //Timer2!!
#define GIC_SPI_MS_FIQ_END                  (GIC_SPI_MS_FIQ_START + 64)
#define GIC_SPI_MS_FIQ_NR                   (GIC_SPI_MS_FIQ_END - GIC_SPI_MS_FIQ_START)

/*  SPI_ID:128-159 */
#define GIC_SPI_MS_EXT_START                (GIC_SPI_MS_FIQ_END)
#define INT_160_1_b0                        (GIC_SPI_MS_EXT_START +  0) //	gic	160	spi	128
#define INT_161_1_b0                        (GIC_SPI_MS_EXT_START +  1) //	gic	161	spi	129
#define INT_162_IRQ_IN_0                    (GIC_SPI_MS_EXT_START +  2) //	gic	162	spi	130
#define INT_163_IRQ_IN_1                    (GIC_SPI_MS_EXT_START +  3) //	gic	163	spi	131
#define INT_164_FIQ_IN_0                    (GIC_SPI_MS_EXT_START +  4) //	gic	164	spi	132
#define INT_165_FIQ_IN_1                    (GIC_SPI_MS_EXT_START +  5) //	gic	165	spi	133
#define INT_166_INT_MERGE_0                 (GIC_SPI_MS_EXT_START +  6) //	gic	166	spi	134
#define INT_167_INT_MERGE_1                 (GIC_SPI_MS_EXT_START +  7) //	gic	167	spi	135
#define INT_168_REG_DUMMY_ECO_1_0           (GIC_SPI_MS_EXT_START +  8) //	gic	168	spi	136
#define INT_169_REG_DUMMY_ECO_1_1           (GIC_SPI_MS_EXT_START +  9) //	gic	169	spi	137
#define INT_170_1_b0                        (GIC_SPI_MS_EXT_START + 10) //	gic	170	spi	138
#define INT_171_RXIU_TIMEOUT_INT            (GIC_SPI_MS_EXT_START + 11) //	gic	171	spi	139
#define INT_172_SCUEVABORT_INTR             (GIC_SPI_MS_EXT_START + 12) //	gic	172	spi	140
#define INT_173_L2_INTR                     (GIC_SPI_MS_EXT_START + 13) //	gic	173	spi	141
#define INT_174_DEFLAGS0_INTR_0             (GIC_SPI_MS_EXT_START + 14) //	gic	174	spi	142
#define INT_175_DEFLAGS0_INTR_1             (GIC_SPI_MS_EXT_START + 15) //	gic	175	spi	143
#define INT_176_DEFLAGS0_INTR_2             (GIC_SPI_MS_EXT_START + 16) //	gic	176	spi	144
#define INT_177_DEFLAGS0_INTR_3             (GIC_SPI_MS_EXT_START + 17) //	gic	177	spi	145
#define INT_178_DEFLAGS0_INTR_4             (GIC_SPI_MS_EXT_START + 18) //	gic	178	spi	146
#define INT_179_DEFLAGS0_INTR_5             (GIC_SPI_MS_EXT_START + 19) //	gic	179	spi	147
#define INT_180_DEFLAGS0_INTR_6             (GIC_SPI_MS_EXT_START + 20) //	gic	180	spi	148
#define INT_181_CTI_INTR_XOR_0              (GIC_SPI_MS_EXT_START + 21) //	gic	181	spi	149
#define INT_182_PMU_IRQ_0                   (GIC_SPI_MS_EXT_START + 22) //	gic	182	spi	150
#define INT_183_DEFLAGS1_INTR_0             (GIC_SPI_MS_EXT_START + 23) //	gic	183	spi	151
#define INT_184_DEFLAGS1_INTR_1             (GIC_SPI_MS_EXT_START + 24) //	gic	184	spi	152
#define INT_185_DEFLAGS1_INTR_2             (GIC_SPI_MS_EXT_START + 25) //	gic	185	spi	153
#define INT_186_DEFLAGS1_INTR_3             (GIC_SPI_MS_EXT_START + 26) //	gic	186	spi	154
#define INT_187_DEFLAGS1_INTR_4             (GIC_SPI_MS_EXT_START + 27) //	gic	187	spi	155
#define INT_188_DEFLAGS1_INTR_5             (GIC_SPI_MS_EXT_START + 28) //	gic	188	spi	156
#define INT_189_DEFLAGS1_INTR_6             (GIC_SPI_MS_EXT_START + 29) //	gic	189	spi	157
#define INT_190_CTI_INTR_XOR_1			    (GIC_SPI_MS_EXT_START + 30) //	gic	190	spi	158
#define INT_191_PMU_IRQ_1                   (GIC_SPI_MS_EXT_START + 31) //	gic	191	spi	159
#define GIC_SPI_MS_EXT_END                  (GIC_SPI_MS_EXT_START + 32) //	gic	192	spi	160
#define GIC_SPI_MS_EXT_NR                  (GIC_SPI_MS_EXT_END - GIC_SPI_MS_EXT_START)

/*IRQ1 :0*/
#define GIC_SPI_MS_IRQ1_START               (GIC_SPI_MS_EXT_END)
#define INT_IRQ_64_MIIC2_INT                (GIC_SPI_MS_IRQ1_START +  0) //	gic	192	spi	160
#define INT_IRQ_65_MIIC3_INT                (GIC_SPI_MS_IRQ1_START +  1) //	gic	193	spi	161
#define INT_IRQ_66_MIIC4_INT                (GIC_SPI_MS_IRQ1_START +  2) //	gic	194	spi	162
#define INT_IRQ_67_RESERVED                 (GIC_SPI_MS_IRQ1_START +  3) //	gic	195	spi	163
#define INT_IRQ_68_HDMITX_PHY_INT           (GIC_SPI_MS_IRQ1_START +  4) //	gic	196	spi	164
#define INT_IRQ_69_GE_INT                   (GIC_SPI_MS_IRQ1_START +  5) //	gic	197	spi	165
#define INT_IRQ_70_MIU_SECURITY_INT         (GIC_SPI_MS_IRQ1_START +  6) //	gic	198	spi	166
#define INT_IRQ_71_U3_PHY_IRQ_OUT           (GIC_SPI_MS_IRQ1_START +  7) //	gic	199	spi	167
#define INT_IRQ_72_G3D2MCU_IRQ_DFT          (GIC_SPI_MS_IRQ1_START +  8) //	gic	200	spi	168
#define INT_IRQ_73_CMDQ_INT                 (GIC_SPI_MS_IRQ1_START +  9) //	gic	201	spi	169
#define INT_IRQ_74_CMDQ3_INT                (GIC_SPI_MS_IRQ1_START + 10) //	gic	202	spi	170
#define INT_IRQ_75_SCDC_INT_PM              (GIC_SPI_MS_IRQ1_START + 11) //	gic	203	spi	171
#define INT_IRQ_76_MSPI2_INT                (GIC_SPI_MS_IRQ1_START + 12) //	gic	204	spi	172
#define INT_IRQ_77_CEVA2RIU_INT             (GIC_SPI_MS_IRQ1_START + 13) //	gic	205	spi	173
#define INT_IRQ_78_WARP2RIU_INT             (GIC_SPI_MS_IRQ1_START + 14) //	gic	206	spi	174
#define INT_IRQ_79_DCSUB_INT                (GIC_SPI_MS_IRQ1_START + 15) //	gic	207	spi	175
#define INT_IRQ_80_SDIO_INT                 (GIC_SPI_MS_IRQ1_START + 16) //	gic	208	spi	176
#define INT_IRQ_81_USB30_SS_INT             (GIC_SPI_MS_IRQ1_START + 17) //	gic	209	spi	177
#define INT_IRQ_82_MIU_CMA_CLR_INT          (GIC_SPI_MS_IRQ1_START + 18) //	gic	210	spi	178
#define INT_IRQ_83_VIF_INT                  (GIC_SPI_MS_IRQ1_START + 19) //	gic	211	spi	179
#define INT_IRQ_84_USB30_HS_USB_INT         (GIC_SPI_MS_IRQ1_START + 20) //	gic	212	spi	180
#define INT_IRQ_85_USB30_HS_UHC_INT         (GIC_SPI_MS_IRQ1_START + 21) //	gic	213	spi	181
#define INT_IRQ_86_RXIU_TIMEOUT_NODEF_INT   (GIC_SPI_MS_IRQ1_START + 22) //	gic	214	spi	182
#define INT_IRQ_87_MIPI_TX_INT              (GIC_SPI_MS_IRQ1_START + 23) //	gic	215	spi	183
#define INT_IRQ_88_RESERVED                 (GIC_SPI_MS_IRQ1_START + 24) //	gic	216	spi	184
#define INT_IRQ_89_CMDQ5_INT                (GIC_SPI_MS_IRQ1_START + 25) //	gic	217	spi	185
#define INT_IRQ_90_CMDQ4_INT                (GIC_SPI_MS_IRQ1_START + 26) //	gic	218	spi	186
#define INT_IRQ_91_RESERVED                 (GIC_SPI_MS_IRQ1_START + 27) //	gic	219	spi	187
#define INT_IRQ_92_MIU_TLB_INT              (GIC_SPI_MS_IRQ1_START + 28) //	gic	220	spi	188
#define INT_IRQ_93_DIPW_INT                 (GIC_SPI_MS_IRQ1_START + 29) //	gic	221	spi	189
#define INT_IRQ_94_EMAC_INT                 (GIC_SPI_MS_IRQ1_START + 30) //	gic	222	spi	190
#define INT_IRQ_95_RESERVED                 (GIC_SPI_MS_IRQ1_START + 31) //	gic	223	spi	191
#define INT_FIQ_64_SEC_GUARD_INT            (GIC_SPI_MS_IRQ1_START + 32) //	gic	224	spi	192
#define INT_FIQ_65_SD_CDZ_IN                (GIC_SPI_MS_IRQ1_START + 33) //	gic	225	spi	193
#define INT_IRQ_96_CORE0_MHE_INT            (GIC_SPI_MS_IRQ1_START + 34) //	gic	226	spi	194
#define INT_IRQ_97_CORE1_MFE_INT            (GIC_SPI_MS_IRQ1_START + 35) //	gic	227	spi	195
#define INT_IRQ_98_CORE1_MHE_INT            (GIC_SPI_MS_IRQ1_START + 36) //	gic	228	spi	196
#define INT_IRQ_99_INT_UART2                (GIC_SPI_MS_IRQ1_START + 37) //	gic	229	spi	197
#define INT_IRQ_100_MSPI3_INT               (GIC_SPI_MS_IRQ1_START + 38) //	gic	230	spi	198
#define INT_IRQ_101_SD_INT                  (GIC_SPI_MS_IRQ1_START + 39) //	gic	231	spi	199
#define INT_IRQ_102_SD_OSP_INT              (GIC_SPI_MS_IRQ1_START + 40) //	gic	232	spi	200
#define INT_IRQ_103_JPE_IRQ                 (GIC_SPI_MS_IRQ1_START + 41) //	gic	233	spi	201
#define INT_IRQ_104_MIPI_CSI2_INT_0         (GIC_SPI_MS_IRQ1_START + 42) //	gic	234	spi	202
#define INT_IRQ_105_MIPI_CSI2_INT_1         (GIC_SPI_MS_IRQ1_START + 43) //	gic	235	spi	203
#define INT_IRQ_106_MIPI_CSI2_INT_2         (GIC_SPI_MS_IRQ1_START + 44) //	gic	236	spi	204
#define INT_IRQ_107_MIPI_CSI2_INT_3         (GIC_SPI_MS_IRQ1_START + 45) //	gic	237	spi	205
#define INT_IRQ_108_DMA2CPU_INT             (GIC_SPI_MS_IRQ1_START + 46) //	gic	238	spi	206
#define INT_IRQ_109_ISP_GOP_INT             (GIC_SPI_MS_IRQ1_START + 47) //	gic	239	spi	207
#define INT_IRQ_110_AU_INT                  (GIC_SPI_MS_IRQ1_START + 48) //	gic	240	spi	208
#define INT_IRQ_111_AU_INT_GEN              (GIC_SPI_MS_IRQ1_START + 49) //	gic	241	spi	209
#define INT_IRQ_112_CMDQ_INT2               (GIC_SPI_MS_IRQ1_START + 50) //	gic	242	spi	210
#define INT_IRQ_113_IMI_TOP_IRQ_0           (GIC_SPI_MS_IRQ1_START + 51) //	gic	243	spi	211
#define INT_IRQ_114_IMI_TOP_IRQ_1           (GIC_SPI_MS_IRQ1_START + 52) //	gic	244	spi	212
#define INT_IRQ_115_NOE_IRQ0                (GIC_SPI_MS_IRQ1_START + 53) //	gic	245	spi	213
#define INT_IRQ_116_NOE_IRQ1                (GIC_SPI_MS_IRQ1_START + 54) //	gic	246	spi	214
#define INT_IRQ_117_NOE_IRQ2                (GIC_SPI_MS_IRQ1_START + 55) //	gic	247	spi	215
#define INT_IRQ_118_ISP_INT                 (GIC_SPI_MS_IRQ1_START + 56) //	gic	248	spi	216
#define INT_IRQ_119_ISP_DMA_INT             (GIC_SPI_MS_IRQ1_START + 57) //	gic	249	spi	217
#define INT_IRQ_120_IVE_INT                 (GIC_SPI_MS_IRQ1_START + 58) //	gic	250	spi	218
#define INT_IRQ_121_sc_top_int_0            (GIC_SPI_MS_IRQ1_START + 59) //	gic	251	spi	219
#define INT_IRQ_122_sc_top_int_1            (GIC_SPI_MS_IRQ1_START + 60) //	gic	252	spi	220
#define INT_IRQ_123_sc_top_int_2            (GIC_SPI_MS_IRQ1_START + 61) //	gic	253	spi	221
#define INT_IRQ_124_NOT_ALLOW               (GIC_SPI_MS_IRQ1_START + 62) //	gic	254	spi	222
#define INT_IRQ_125_NOT_ALLOW               (GIC_SPI_MS_IRQ1_START + 63) //	gic	255	spi	223
//#define INT_IRQ_126_NOT_ALLOW               (GIC_SPI_MS_IRQ1_START + 64) //	gic	256	spi	224
//#define INT_IRQ_127_NOT_ALLOW               (GIC_SPI_MS_IRQ1_START + 65) // gic	257	spi	225
#define GIC_SPI_MS_IRQ1_END                 (GIC_SPI_MS_IRQ1_START + 64)  //	gic	256	spi	224
#define GIC_SPI_MS_IRQ1_NR                  (GIC_SPI_MS_IRQ1_END - GIC_SPI_MS_IRQ1_START)




#define GIC_SPI_MS_FIQ1_START               (GIC_SPI_MS_IRQ1_END)
//#define INT_FIQ_64_SEC_GUARD_INT            (GIC_SPI_MS_FIQ1_START +  0)
//#define INT_FIQ_65_SD_CDZ_IN                (GIC_SPI_MS_FIQ1_START +  1)
#define INT_FIQ_66_RESERVED                 (GIC_SPI_MS_FIQ1_START +  2)
#define INT_FIQ_67_RESERVED                 (GIC_SPI_MS_FIQ1_START +  3)
#define INT_FIQ_68_RESERVED                 (GIC_SPI_MS_FIQ1_START +  4)
#define INT_FIQ_69_RESERVED                 (GIC_SPI_MS_FIQ1_START +  5)
#define INT_FIQ_70_RESERVED                 (GIC_SPI_MS_FIQ1_START +  6)
#define INT_FIQ_71_RESERVED                 (GIC_SPI_MS_FIQ1_START +  7)
#define INT_FIQ_72_RESERVED                 (GIC_SPI_MS_FIQ1_START +  8)
#define INT_FIQ_73_RESERVED                 (GIC_SPI_MS_FIQ1_START +  9)
#define INT_FIQ_74_RESERVED                 (GIC_SPI_MS_FIQ1_START + 10)
#define INT_FIQ_75_RESERVED                 (GIC_SPI_MS_FIQ1_START + 11)
#define INT_FIQ_76_USB_INT_P0               (GIC_SPI_MS_FIQ1_START + 12)
#define INT_FIQ_77_UHC_INT_P0               (GIC_SPI_MS_FIQ1_START + 13)
#define INT_FIQ_78_USB30_SS_INT             (GIC_SPI_MS_FIQ1_START + 14)
#define INT_FIQ_79_OTG_INT_P0               (GIC_SPI_MS_FIQ1_START + 15)
#define INT_FIQ_80_USB_INT_P1               (GIC_SPI_MS_FIQ1_START + 16)
#define INT_FIQ_81_UHC_INT_P1               (GIC_SPI_MS_FIQ1_START + 17)
#define INT_FIQ_82_RESERVED                 (GIC_SPI_MS_FIQ1_START + 18)
#define INT_FIQ_83_RESERVED                 (GIC_SPI_MS_FIQ1_START + 19)
#define INT_FIQ_84_USB31_HS_USB_INT         (GIC_SPI_MS_FIQ1_START + 20)
#define INT_FIQ_85_USB30_HS_UHC_INT         (GIC_SPI_MS_FIQ1_START + 21)
#define INT_FIQ_86_RESERVED                 (GIC_SPI_MS_FIQ1_START + 22)
#define INT_FIQ_87_RESERVED                 (GIC_SPI_MS_FIQ1_START + 23)
#define INT_FIQ_88_RESERVED                 (GIC_SPI_MS_FIQ1_START + 24)
#define INT_FIQ_89_RESERVED                 (GIC_SPI_MS_FIQ1_START + 25)
#define INT_FIQ_90_RESERVED                 (GIC_SPI_MS_FIQ1_START + 26)
#define INT_FIQ_91_RESERVED                 (GIC_SPI_MS_FIQ1_START + 27)
#define INT_FIQ_92_RESERVED                 (GIC_SPI_MS_FIQ1_START + 28)
#define INT_FIQ_93_RESERVED                 (GIC_SPI_MS_FIQ1_START + 29)
#define INT_FIQ_94_RESERVED                 (GIC_SPI_MS_FIQ1_START + 30)
#define INT_FIQ_95_RESERVED                 (GIC_SPI_MS_FIQ1_START + 31)
#define INT_FIQ_96_NOT_ALLOW                (GIC_SPI_MS_FIQ1_START + 32)
#define INT_FIQ_97_NOT_ALLOW                (GIC_SPI_MS_FIQ1_START + 33)
#define INT_FIQ_98_NOT_ALLOW                (GIC_SPI_MS_FIQ1_START + 34)
#define INT_FIQ_99_NOT_ALLOW                (GIC_SPI_MS_FIQ1_START + 35)
#define INT_FIQ_100_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 36)
#define INT_FIQ_101_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 37)
#define INT_FIQ_102_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 38)
#define INT_FIQ_103_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 39)
#define INT_FIQ_104_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 40)
#define INT_FIQ_105_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 41)
#define INT_FIQ_106_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 42)
#define INT_FIQ_107_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 43)
#define INT_FIQ_108_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 44)
#define INT_FIQ_109_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 45)
#define INT_FIQ_110_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 46)
#define INT_FIQ_111_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 47)
#define INT_FIQ_112_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 48)
#define INT_FIQ_113_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 49)
#define INT_FIQ_114_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 50)
#define INT_FIQ_115_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 51)
#define INT_FIQ_116_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 52)
#define INT_FIQ_117_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 53)
#define INT_FIQ_118_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 54)
#define INT_FIQ_119_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 55)
#define INT_FIQ_120_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 56)
#define INT_FIQ_121_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 57)
#define INT_FIQ_122_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 58)
#define INT_FIQ_123_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 59)
#define INT_FIQ_124_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 60)
#define INT_FIQ_125_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 61)
#define INT_FIQ_126_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 62)
#define INT_FIQ_127_NOT_ALLOW               (GIC_SPI_MS_FIQ1_START + 63)
#define GIC_SPI_MS_FIQ1_END                 (GIC_SPI_MS_FIQ1_START + 64)
#define GIC_SPI_MS_FIQ1_NR                  (GIC_SPI_MS_FIQ1_END - GIC_SPI_MS_FIQ1_START)


#if 1
/*   [PMSLEEP irqchip]
        ID   0 -  31 : MS_PM_IRQ    */
/*  Not used in dtsi,
if need to get the interrupt number for request_irq(), use gpio_to_irq() to obtain irq number.
Or manual calculate the number is
GIC_SGI_NR+GIC_PPI_NR+GIC_SPI_ARM_INTERNAL_NR+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR+X=256+X   */

/* INT_PMSLEEP_DVI_CK_DET *///rm dvi_clk_det, because can not find gpio index
/* MS_PM_SLEEP_FIQ 0-31 */
//#define PMSLEEP_FIQ_START           0
//#define INT_PMSLEEP_IR              (PMSLEEP_FIQ_START +  0)
//#define INT_PMSLEEP_DVI_CK_DET      (PMSLEEP_FIQ_START +  1)
//#define INT_PMSLEEP_GPIO_0          (PMSLEEP_FIQ_START +  2)
//#define INT_PMSLEEP_GPIO_1          (PMSLEEP_FIQ_START +  3)
//#define INT_PMSLEEP_GPIO_2          (PMSLEEP_FIQ_START +  4)
//#define INT_PMSLEEP_GPIO_3          (PMSLEEP_FIQ_START +  5)
//#define INT_PMSLEEP_GPIO_4          (PMSLEEP_FIQ_START +  6)
//#define INT_PMSLEEP_GPIO_5          (PMSLEEP_FIQ_START +  7)
//#define INT_PMSLEEP_GPIO_6          (PMSLEEP_FIQ_START +  8)
//#define INT_PMSLEEP_GPIO_7          (PMSLEEP_FIQ_START +  9)
//#define INT_PMSLEEP_GPIO_8          (PMSLEEP_FIQ_START + 10)
//#define INT_PMSLEEP_GPIO_9          (PMSLEEP_FIQ_START + 11)
//#define INT_PMSLEEP_GPIO_10         (PMSLEEP_FIQ_START + 12)
//#define INT_PMSLEEP_GPIO_11         (PMSLEEP_FIQ_START + 13)
//#define INT_PMSLEEP_GPIO_12         (PMSLEEP_FIQ_START + 14)
//#define INT_PMSLEEP_GPIO_13         (PMSLEEP_FIQ_START + 15)
//#define INT_PMSLEEP_GPIO_14         (PMSLEEP_FIQ_START + 16)
//#define INT_PMSLEEP_GPIO_15         (PMSLEEP_FIQ_START + 17)
//#define INT_PMSLEEP_18_RESERVED     (PMSLEEP_FIQ_START + 18)
//#define INT_PMSLEEP_19_RESERVED     (PMSLEEP_FIQ_START + 19)
//#define INT_PMSLEEP_20_RESERVED     (PMSLEEP_FIQ_START + 20)
//#define INT_PMSLEEP_21_RESERVED     (PMSLEEP_FIQ_START + 21)
//#define INT_PMSLEEP_IRIN            (PMSLEEP_FIQ_START + 22)
//#define INT_PMSLEEP_UART_RX         (PMSLEEP_FIQ_START + 23)
//#define INT_PMSLEEP_CEC             (PMSLEEP_FIQ_START + 24)
//#define INT_PMSLEEP_25_DUMMY        (PMSLEEP_FIQ_START + 25)
//#define INT_PMSLEEP_SPI_CZ          (PMSLEEP_FIQ_START + 26)
//#define INT_PMSLEEP_SPI_CK          (PMSLEEP_FIQ_START + 27)
//#define INT_PMSLEEP_SPI_DI          (PMSLEEP_FIQ_START + 28)
//#define INT_PMSLEEP_SPI_DO          (PMSLEEP_FIQ_START + 29)
//#define INT_PMSLEEP_30_RESERVED     (PMSLEEP_FIQ_START + 30)
//#define INT_PMSLEEP_31_RESERVED     (PMSLEEP_FIQ_START + 31)
//#define INT_PMSLEEP_32_RESERVED     (PMSLEEP_FIQ_START + 32)
//#define INT_PMSLEEP_33_RESERVED     (PMSLEEP_FIQ_START + 33)
//#define INT_PMSLEEP_34_RESERVED     (PMSLEEP_FIQ_START + 34)
//#define INT_PMSLEEP_35_RESERVED     (PMSLEEP_FIQ_START + 35)
//#define INT_PMSLEEP_36_RESERVED     (PMSLEEP_FIQ_START + 36)
//#define INT_PMSLEEP_37_RESERVED     (PMSLEEP_FIQ_START + 37)
//#define INT_PMSLEEP_38_RESERVED     (PMSLEEP_FIQ_START + 38)
//#define INT_PMSLEEP_39_RESERVED     (PMSLEEP_FIQ_START + 39)
//#define INT_PMSLEEP_40_RESERVED     (PMSLEEP_FIQ_START + 40)
//#define INT_PMSLEEP_41_RESERVED     (PMSLEEP_FIQ_START + 41)
//#define INT_PMSLEEP_42_RESERVED     (PMSLEEP_FIQ_START + 42)
//#define INT_PMSLEEP_43_RESERVED     (PMSLEEP_FIQ_START + 43)
//#define INT_PMSLEEP_44_RESERVED     (PMSLEEP_FIQ_START + 44)
//#define INT_PMSLEEP_45_RESERVED     (PMSLEEP_FIQ_START + 45)
//#define INT_PMSLEEP_46_RESERVED     (PMSLEEP_FIQ_START + 46)
//#define INT_PMSLEEP_47_RESERVED     (PMSLEEP_FIQ_START + 47)
//#define INT_PMSLEEP_48_RESERVED     (PMSLEEP_FIQ_START + 48)
//#define INT_PMSLEEP_49_RESERVED     (PMSLEEP_FIQ_START + 49)
//#define INT_PMSLEEP_50_RESERVED     (PMSLEEP_FIQ_START + 50)
//#define INT_PMSLEEP_51_RESERVED     (PMSLEEP_FIQ_START + 51)
//#define INT_PMSLEEP_52_RESERVED     (PMSLEEP_FIQ_START + 52)
//#define INT_PMSLEEP_53_RESERVED     (PMSLEEP_FIQ_START + 53)
//#define INT_PMSLEEP_54_RESERVED     (PMSLEEP_FIQ_START + 54)
//#define INT_PMSLEEP_55_RESERVED     (PMSLEEP_FIQ_START + 55)
//#define INT_PMSLEEP_56_RESERVED     (PMSLEEP_FIQ_START + 56)
//#define INT_PMSLEEP_57_RESERVED     (PMSLEEP_FIQ_START + 57)
//#define INT_PMSLEEP_58_RESERVED     (PMSLEEP_FIQ_START + 58)
//#define INT_PMSLEEP_59_RESERVED     (PMSLEEP_FIQ_START + 59)
//#define INT_PMSLEEP_60_RESERVED     (PMSLEEP_FIQ_START + 60)
//#define INT_PMSLEEP_61_RESERVED     (PMSLEEP_FIQ_START + 61)
//#define INT_PMSLEEP_62_RESERVED     (PMSLEEP_FIQ_START + 62)
//#define INT_PMSLEEP_63_RESERVED     (PMSLEEP_FIQ_START + 63)
//#define INT_PMSLEEP_64_RESERVED     (PMSLEEP_FIQ_START + 64)
//#define INT_PMSLEEP_65_RESERVED     (PMSLEEP_FIQ_START + 65)
//#define INT_PMSLEEP_66_RESERVED     (PMSLEEP_FIQ_START + 66)
//#define INT_PMSLEEP_67_RESERVED     (PMSLEEP_FIQ_START + 67)
//#define INT_PMSLEEP_68_RESERVED     (PMSLEEP_FIQ_START + 68)
//#define INT_PMSLEEP_69_RESERVED     (PMSLEEP_FIQ_START + 69)
//#define INT_PMSLEEP_70_RESERVED     (PMSLEEP_FIQ_START + 70)
//#define INT_PMSLEEP_71_RESERVED     (PMSLEEP_FIQ_START + 71)
//#define INT_PMSLEEP_72_RESERVED     (PMSLEEP_FIQ_START + 72)
//#define INT_PMSLEEP_73_RESERVED     (PMSLEEP_FIQ_START + 73)
//#define INT_PMSLEEP_74_RESERVED     (PMSLEEP_FIQ_START + 74)
//#define INT_PMSLEEP_75_RESERVED     (PMSLEEP_FIQ_START + 75)
//#define INT_PMSLEEP_76_RESERVED     (PMSLEEP_FIQ_START + 76)
//#define INT_PMSLEEP_77_RESERVED     (PMSLEEP_FIQ_START + 77)
//#define PMSLEEP_FIQ_END             (PMSLEEP_FIQ_START + 30)
//#define PMSLEEP_FIQ_NR              (PMSLEEP_FIQ_END - PMSLEEP_FIQ_START)
/*pm_gpio bank 0xf*/
#define PM_GPIO_INT_START        (0)
#define INT_PMSLEEP_GPIO_0  (PM_GPIO_INT_START + 0) //pm_sleep_gpio0
#define INT_PMSLEEP_GPIO_1  (PM_GPIO_INT_START + 1)
#define INT_PMSLEEP_GPIO_2  (PM_GPIO_INT_START + 2)
#define INT_PMSLEEP_GPIO_3  (PM_GPIO_INT_START + 3)
#define INT_PMSLEEP_GPIO_4  (PM_GPIO_INT_START + 4)
#define INT_PMSLEEP_GPIO_5  (PM_GPIO_INT_START + 5)
#define INT_PMSLEEP_GPIO_6  (PM_GPIO_INT_START + 6)
#define INT_PMSLEEP_GPIO_7  (PM_GPIO_INT_START + 7)
#define INT_PMSLEEP_GPIO_8  (PM_GPIO_INT_START + 8)
#define INT_PMSLEEP_GPIO_9  (PM_GPIO_INT_START + 9)
#define INT_PMSLEEP_GPIO_10 (PM_GPIO_INT_START + 10)
#define INT_PMSLEEP_GPIO_11 (PM_GPIO_INT_START + 11)
#define INT_PMSLEEP_GPIO_12 (PM_GPIO_INT_START + 12)
#define INT_PMSLEEP_GPIO_13 (PM_GPIO_INT_START + 13)
#define INT_PMSLEEP_GPIO_14 (PM_GPIO_INT_START + 14)
#define INT_PMSLEEP_GPIO_15 (PM_GPIO_INT_START + 15) //pm_sleep_gpio15
#define INT_PMSLEEP_GPIO_16 (PM_GPIO_INT_START + 16)
#define INT_PMSLEEP_GPIO_17 (PM_GPIO_INT_START + 17)
#define INT_PMSLEEP_GPIO_18 (PM_GPIO_INT_START + 18)
#define INT_PMSLEEP_GPIO_19 (PM_GPIO_INT_START + 19) //pm_sleep_gpio19
#define INT_PMSLEEP_GPIO_20_IR  (PM_GPIO_INT_START + 20)
//#define INT_PMSLEEP_GPIO_21_UART_RX (PM_GPIO_INT_START + 21) //?? check ??
#define INT_PMSLEEP_GPIO_22_CEC (PM_GPIO_INT_START + 22)
//#define GPIO_23_un_connect  (PM_GPIO_INT_START + 23)
#define INT_PMSLEEP_GPIO_24_SPI_CZ  (PM_GPIO_INT_START + 24)
#define INT_PMSLEEP_GPIO_25_SPI_CK  (PM_GPIO_INT_START + 25)
#define INT_PMSLEEP_GPIO_26_SPI_DI  (PM_GPIO_INT_START + 26)
#define INT_PMSLEEP_GPIO_27_SPI_DO  (PM_GPIO_INT_START + 27)
//#define GPIO_28 (PM_GPIO_INT_START + 28)
//#define GPIO_29 (PM_GPIO_INT_START + 29)
//#define GPIO_30 (PM_GPIO_INT_START + 30)
//#define GPIO_31 (PM_GPIO_INT_START + 31)
//#define GPIO_32 (PM_GPIO_INT_START + 32)
//#define GPIO_33 (PM_GPIO_INT_START + 33)
//#define GPIO_34 (PM_GPIO_INT_START + 34)
//#define GPIO_35 (PM_GPIO_INT_START + 35)
//#define GPIO_36 (PM_GPIO_INT_START + 36)
//#define GPIO_37 (PM_GPIO_INT_START + 37)
//#define GPIO_38 (PM_GPIO_INT_START + 38)
//#define GPIO_39_PAD_GT0_MDC (PM_GPIO_INT_START + 39)
//#define GPIO_40_PAD_GT0_MDIO    (PM_GPIO_INT_START + 40)
//#define GPIO_41_PAD_GT0_RX_CLK  (PM_GPIO_INT_START + 41)
//#define GPIO_42_PAD_GT0_RX_CTL  (PM_GPIO_INT_START + 42)
//#define GPIO_43_PAD_GT0_RX_D0    (PM_GPIO_INT_START + 43)
//#define GPIO_44_PAD_GT0_RX_D1   (PM_GPIO_INT_START + 44)
//#define GPIO_45_PAD_GT0_RX_D2   (PM_GPIO_INT_START + 45)
//#define GPIO_46_PAD_GT0_RX_D3   (PM_GPIO_INT_START + 46)
//#define GPIO_47_PAD_GT0_TX_CLK  (PM_GPIO_INT_START + 47)
//#define GPIO_48_PAD_GT0_TX_CTL  (PM_GPIO_INT_START + 48)
//#define GPIO_49_PAD_GT0_TX_D0   (PM_GPIO_INT_START + 49)
//#define GPIO_50_PAD_GT0_TX_D1   (PM_GPIO_INT_START + 50)
//#define GPIO_51_PAD_GT0_TX_D2   (PM_GPIO_INT_START + 51)
//#define GPIO_52_PAD_GT0_TX_D3   (PM_GPIO_INT_START + 52)
//#define GPIO_53_PAD_GT1_MDC    (PM_GPIO_INT_START + 53)
//#define GPIO_54_PAD_GT1_MDIO    (PM_GPIO_INT_START + 54)
//#define GPIO_55_PAD_GT1_RX_CLK  (PM_GPIO_INT_START + 55)
//#define GPIO_56_PAD_GT1_RX_CTL  (PM_GPIO_INT_START + 56)
//#define GPIO_57_PAD_GT1_RX_D0   (PM_GPIO_INT_START + 57)
//#define GPIO_58_PAD_GT1_RX_D1   (PM_GPIO_INT_START + 58)
//#define GPIO_59_PAD_GT1_RX_D2   (PM_GPIO_INT_START + 59)
//#define GPIO_60_PAD_GT1_RX_D3   (PM_GPIO_INT_START + 60)
//#define GPIO_61_PAD_GT1_TX_CLK   (PM_GPIO_INT_START + 61)
//#define GPIO_62_PAD_GT1_TX_CTL   (PM_GPIO_INT_START + 62)
//#define GPIO_63_PAD_GT1_TX_D0   (PM_GPIO_INT_START + 63)
//#define GPIO_64_PAD_GT1_TX_D1   (PM_GPIO_INT_START + 64)
//#define GPIO_65_PAD_GT1_TX_D2   (PM_GPIO_INT_START + 65)
//#define GPIO_66_PAD_GT1_TX_D3   (PM_GPIO_INT_START + 66)
#define INT_PMSLEEP_GPIO_67_PAD_PM_HDMI_CEC (PM_GPIO_INT_START + 67)
#define INT_PMSLEEP_GPIO_68_PAD_PM_SPI_WPZ  (PM_GPIO_INT_START + 68)
#define INT_PMSLEEP_GPIO_69_PAD_PM_SPI_HOLDZ    (PM_GPIO_INT_START + 69)
#define INT_PMSLEEP_GPIO_70_PAD_PM_SPI_RSTZ (PM_GPIO_INT_START + 70)
#define INT_PMSLEEP_GPIO_71_PAD_PM_SD_CDZ	    (PM_GPIO_INT_START + 71)
#define INT_PMSLEEP_GPIO_72_PAD_VID0    (PM_GPIO_INT_START + 72)          //279
#define INT_PMSLEEP_GPIO_73_PAD_VID1    (PM_GPIO_INT_START + 73)          //280
#define INT_PMSLEEP_GPIO_74_PAD_LED0    (PM_GPIO_INT_START + 74)          //281
#define INT_PMSLEEP_GPIO_75_PAD_LED1    (PM_GPIO_INT_START + 75)          //282
#define INT_PMSLEEP_GPIO_76_PAD_PM_SPI_CZ1  (PM_GPIO_INT_START + 76)
#define INT_PMSLEEP_GPIO_77_PAD_PM_SPI_CZ2  (PM_GPIO_INT_START + 77)
#define INT_PMSLEEP_GPIO_78_PAD_PM_SD30_CDZ (PM_GPIO_INT_START + 78) //277
#define INT_PMSLEEP_GPIO_79_PAD_PM_SD20_CDZ (PM_GPIO_INT_START + 79) //278
#define PM_GPIO_INT_END            (PM_GPIO_INT_START + 80)
#define PM_GPIO_INT_NR              (PM_GPIO_INT_END - PM_GPIO_INT_START)


#define PMSLEEP_IRQ_START           (PM_GPIO_INT_END)
#define INT_PMSLEEP_IRQ_00_CEC      (PMSLEEP_IRQ_START +  0)
#define INT_PMSLEEP_IRQ_01_SAR      (PMSLEEP_IRQ_START +  1)
#define INT_PMSLEEP_IRQ_02_WOL      (PMSLEEP_IRQ_START +  2)
#define INT_PMSLEEP_IRQ_03_SYNC_DET (PMSLEEP_IRQ_START +  3)
#define INT_PMSLEEP_IRQ_04_RTC0     (PMSLEEP_IRQ_START +  4)
#define INT_PMSLEEP_IRQ_05_DDC      (PMSLEEP_IRQ_START +  5)
#define INT_PMSLEEP_IRQ_06_RTC2     (PMSLEEP_IRQ_START +  6)
#define INT_PMSLEEP_IRQ_07_RTC1     (PMSLEEP_IRQ_START +  7)
#define PMSLEEP_IRQ_END             (PMSLEEP_IRQ_START +  8)    // PMSLEEP_IRQ_END = 38
#define PMSLEEP_IRQ_NR              (PMSLEEP_IRQ_END - PMSLEEP_IRQ_START)



#endif
#endif // __ARCH_ARM_ASM_IRQS_H
