/* SigmaStar trade secret */
/*
* msb250x_reg.h - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: raul.wang <raul.wang@sigmastar.com.tw>
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

#ifndef __MSB250X_USB_REGISTER
#define __MSB250X_USB_REGISTER

#define GET_REG16_ADDR(x, y)                        ((x) + ((y) << 2))
#define GET_REG8_ADDR(x, y)                         ((x) + ((y) << 1) - ((y) & 1))

#define UTMI_BASE_ADDR                              GET_REG8_ADDR(MS_BASE_REG_RIU_PA, 0x142100)
#define USBC_BASE_ADDR                              GET_REG8_ADDR(MS_BASE_REG_RIU_PA, 0x142300)
#define OTG0_BASE_ADDR                              GET_REG8_ADDR(MS_BASE_REG_RIU_PA, 0x142500)
#define REG_ADDR_BASE_PM_GPIO                       GET_REG8_ADDR(MS_BASE_REG_RIU_PA, 0x0f00)
#define REG_ADDR_BASE_PM_PWM                        GET_REG8_ADDR(MS_BASE_REG_RIU_PA, 0x0e00)

#define USB_MIU_SEL0                                ((u8) 0x70U)
#define USB_MIU_SEL1                                ((u8) 0xefU)
#define USB_MIU_SEL2                                ((u8) 0xefU)
#define USB_MIU_SEL3                                ((u8) 0xefU)

/* 00h ~ 0Fh */
#define MSB250X_OTG0_FADDR_REG			            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x00)
#define MSB250X_OTG0_PWR_REG				        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x01)
#define MSB250X_OTG0_INTRTX_REG    		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x02)
/* 03h reserved */
#define MSB250X_OTG0_INTRRX_REG    		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x04)
/* 05h reserved */
#define MSB250X_OTG0_INTRTXE_REG  		            GET_REG16_ADDR(OTG0_BASE_ADDR, 0x03)
#define MSB250X_OTG0_INTRTX1E_REG                   GET_REG8_ADDR(OTG0_BASE_ADDR, 0x06)
#define MSB250X_OTG0_INTRTX2E_REG                   GET_REG8_ADDR(OTG0_BASE_ADDR, 0x07)
/* 07h reserved */
#define MSB250X_OTG0_INTRRXE_REG  		            GET_REG16_ADDR(OTG0_BASE_ADDR, 0x04)
#define MSB250X_OTG0_INTRRX1E_REG  		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x08)
#define MSB250X_OTG0_INTRRX2E_REG  		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x09)
/* 09h reserved */
#define MSB250X_OTG0_INTRUSB_REG  		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0A)
#define MSB250X_OTG0_INTRUSBE_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0B)
#define MSB250X_OTG0_FRAME_L_REG   		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0C)
#define MSB250X_OTG0_FRAME_H_REG  		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0D)
#define MSB250X_OTG0_INDEX_REG       		        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0E)
#define MSB250X_OTG0_TESTMODE_REG  		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x0F)

#define MSB250X_OTG0_INTR_EP(x)                     (1 << x)
//#define MSB250X_OTG0_EP_FIFOSIZE_REG(x) 		    (GET_BASE_ADDR_BY_BANK(OTG0_BASE_ADDR, (0x10E + (0x10 * x))) + 1)
#define MSB250X_OTG0_EP_FIFOSIZE_REG(x)             GET_REG8_ADDR(OTG0_BASE_ADDR, (0x10F + (0x10 * x)))


/* for EP_SEL = 0, 12h ~ 1Fh */
#define MSB250X_OTG0_EP0_CSR0_REG        		    GET_REG8_ADDR(OTG0_BASE_ADDR, 0x102)
#define MSB250X_OTG0_EP0_CSR0_FLSH_REG  		    GET_REG8_ADDR(OTG0_BASE_ADDR, 0x103)
/* 14h ~ 17h reserved */
#define MSB250X_OTG0_EP0_COUNT0_REG 			    GET_REG8_ADDR(OTG0_BASE_ADDR, 0x108)
/* 19h ~ 1Eh reserved */
#define MSB250X_OTG0_EP0_CONFDATA_REG 		        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x10F)

/* 10h ~ 1Fh */
#define MSB250X_OTG0_EP_TXMAP_REG(x)  		        GET_REG16_ADDR(OTG0_BASE_ADDR, (0x80 + (0x08 * (x))))
#define MSB250X_OTG0_EP_TXMAP_L_REG(x)  		    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x100 + (0x10 * (x))))
#define MSB250X_OTG0_EP_TXMAP_H_REG(x)  		    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x101 + (0x10 * (x))))

/* for EP_SEL != 0, 12h ~ 1Fh (EP1 ~ EPx) */
#define MSB250X_OTG0_EP_TXCSR1_REG(x) 			    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x102 + (0x10 * (x))))
#define MSB250X_OTG0_EP_TXCSR2_REG(x) 			    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x103 + (0x10 * (x))))

#define MSB250X_OTG0_EP_RXMAP_REG(x) 		        GET_REG16_ADDR(OTG0_BASE_ADDR, (0x82 + (0x08 * (x))))
#define MSB250X_OTG0_EP_RXMAP_L_REG(x) 		        GET_REG8_ADDR(OTG0_BASE_ADDR, (0x104 + (0x10 * (x))))
#define MSB250X_OTG0_EP_RXMAP_H_REG(x) 		        GET_REG8_ADDR(OTG0_BASE_ADDR, (0x105 + (0x10 * (x))))

#define MSB250X_OTG0_EP_RXCSR1_REG(x) 			    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x106 + (0x10 * (x))))
#define MSB250X_OTG0_EP_RXCSR2_REG(x) 			    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x107 + (0x10 * (x))))

#define MSB250X_OTG0_EP_RXCOUNT_REG(x) 		        GET_REG16_ADDR(OTG0_BASE_ADDR, (0x84 + (0x08 * (x))))
#define MSB250X_OTG0_EP_RXCOUNT_L_REG(x) 		    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x108 + (0x10 * (x))))
#define MSB250X_OTG0_EP_RXCOUNT_H_REG(x) 		    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x109 + (0x10 * (x))))

#define MSB250X_OTG0_EP_FIFO_COUNT(x)               GET_REG8_ADDR(OTG0_BASE_ADDR, (0x108 + (0x10 * (x))))

/* for EP_SEL = 0, 12h ~ 1Fh */
#define MSB250X_OTG0_CSR0_REG         		        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x12)
#define MSB250X_OTG0_CSR0_FLSH_REG  		        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x13)
/* 14h ~ 17h reserved */
#define MSB250X_OTG0_COUNT0_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x18)
/* 19h ~ 1Eh reserved */
#define MSB250X_OTG0_CONFDATA_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1F)

/* for EP_SEL != 0, 12h ~ 1Fh (EP1 ~ EP3) */
#define MSB250X_OTG0_TXCSR1_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x12)
#define MSB250X_OTG0_TXCSR2_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x13)
#define MSB250X_OTG0_RXMAP_L_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x14)
#define MSB250X_OTG0_RXMAP_H_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x15)
#define MSB250X_OTG0_RXCSR1_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x16)
#define MSB250X_OTG0_RXCSR2_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x17)
#define MSB250X_OTG0_RXCOUNT_L_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x18)
#define MSB250X_OTG0_RXCOUNT_H_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x19)

#define MSB250X_OTG0_TXTYPE                         GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1A)
#define MSB250X_OTG0_TXINTERVAL                     GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1B)
#define MSB250X_OTG0_RXTYPE                         GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1C)
#define MSB250X_OTG0_RXINTERVAL                     GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1D)

#define MSB250X_OTG0_FIFOSIZE_REG 		            GET_REG8_ADDR(OTG0_BASE_ADDR, 0x1E)

#define OTG0_EP_FIFO_ACCESS_L(x)                    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x20 + (0x04 * (x))))

/* 30h ~ 5Fh reserved */

/* 60h */
#define MSB250X_OTG0_DEVCTL_REG 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x60)

/* 80h ~ 8Fh */
#define MSB250X_OTG0_USB_CFG0_L 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x80)
#define MSB250X_OTG0_USB_CFG0_H 			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x81)
#define MSB250X_OTG0_USB_CFG1_L  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x82)
#define MSB250X_OTG0_USB_CFG1_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x83)
#define MSB250X_OTG0_USB_CFG2_L  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x84)
#define MSB250X_OTG0_USB_CFG2_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x85)
#define MSB250X_OTG0_USB_CFG3_L  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x86)
#define MSB250X_OTG0_USB_CFG3_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x87)
#define MSB250X_OTG0_USB_CFG4_L  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x88)
#define MSB250X_OTG0_USB_CFG4_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x89)
#define MSB250X_OTG0_USB_CFG5  			            GET_REG16_ADDR(OTG0_BASE_ADDR, 0x45)
#define MSB250X_OTG0_USB_CFG5_L  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8A)
#define MSB250X_OTG0_USB_CFG5_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8B)
#define MSB250X_OTG0_USB_CFG6_L   			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8C)
#define MSB250X_OTG0_USB_CFG6_H  			        GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8D)
#define MSB250X_OTG0_USB_CFG7_L   	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8E)
#define MSB250X_OTG0_USB_CFG7_H  	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x8F)
#define MSB250X_OTG0_USB_CFG8_L   	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x90)
#define MSB250X_OTG0_USB_CFG8_H  	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x91)
#define MSB250X_OTG0_USB_CFG9_L   	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x92)
#define MSB250X_OTG0_USB_CFG9_H  	                GET_REG8_ADDR(OTG0_BASE_ADDR, 0x93)

#define MSB250X_OTG0_AUTONAK0_EP_BULKOUT            MSB250X_OTG0_USB_CFG3_L
#define MSB250X_OTG0_AUTONAK1_EP_BULKOUT            MSB250X_OTG0_USB_CFG0_H
#define MSB250X_OTG0_AUTONAK2_EP_BULKOUT            MSB250X_OTG0_USB_CFG8_L

#define MSB250X_OTG0_AUTONAK0_RX_PKT_CNT            MSB250X_OTG0_USB_CFG5_L
#define MSB250X_OTG0_AUTONAK1_RX_PKT_CNT            MSB250X_OTG0_USB_CFG1_L
#define MSB250X_OTG0_AUTONAK2_RX_PKT_CNT            MSB250X_OTG0_USB_CFG9_L

//#define MSB250X_OTG0_AUTONAK0_CTRL                  MSB250X_OTG0_USB_CFG5_H
#define MSB250X_OTG0_AUTONAK0_CTRL                  MSB250X_OTG0_USB_CFG5_L

#define MSB250X_OTG0_AUTONAK1_CTRL                  MSB250X_OTG0_USB_CFG0_H
#define MSB250X_OTG0_AUTONAK2_CTRL                  MSB250X_OTG0_USB_CFG8_L

#define CFG6_H_SHORT_MODE                           0x20
#define CFG6_H_BUS_OP_FIX                           0x40
#define CFG6_H_REG_MI_WDFIFO_CTRL                   0x80

#define MSB250X_OTG0_AUTONAK0_EN                    0x2000
#define MSB250X_OTG0_AUTONAK1_EN                    0x10
#define MSB250X_OTG0_AUTONAK2_EN                    0x0100

//#define MSB250X_OTG0_AUTONAK0_OK2Rcv                0x80
#define MSB250X_OTG0_AUTONAK0_OK2Rcv                0x8000

#define MSB250X_OTG0_AUTONAK1_OK2Rcv                0x40
#define MSB250X_OTG0_AUTONAK2_OK2Rcv                0x0400

#define MSB250X_OTG0_AUTONAK0_AllowAck              0x4000
#define MSB250X_OTG0_AUTONAK1_AllowAck              0x20
#define MSB250X_OTG0_AUTONAK2_AllowAck              0x04

#define MSB250X_OTG0_DMA_MODE_CTL                   MSB250X_OTG0_USB_CFG5_L
#define MSB250X_OTG0_DMA_MODE_CTL1	                MSB250X_OTG0_USB_CFG5_H


/* 200h */
#define MSB250X_OTG0_DMA_INTR                       GET_REG8_ADDR(OTG0_BASE_ADDR, 0x200)
#define MSB250X_OTG0_DMA_CNTL(x)                    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x204 + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_ADDR(x)                    GET_REG8_ADDR(OTG0_BASE_ADDR, (0x208 + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_ADDR_LW(x)                 GET_REG8_ADDR(OTG0_BASE_ADDR, (0x208 + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_ADDR_HW(x)                 GET_REG8_ADDR(OTG0_BASE_ADDR, (0x20A + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_COUNT(x)                   GET_REG8_ADDR(OTG0_BASE_ADDR, (0x20C + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_COUNT_LW(x)                GET_REG8_ADDR(OTG0_BASE_ADDR, (0x20C + (0x10 * (x - 1))))
#define MSB250X_OTG0_DMA_COUNT_HW(x)                GET_REG8_ADDR(OTG0_BASE_ADDR, (0x20E + (0x10 * (x - 1))))

/* new mode1 in Peripheral mode */
#define M_Mode1_P_BulkOut_EP                        0x0002
#define M_Mode1_P_OK2Rcv                            0x8000
#define M_Mode1_P_AllowAck                          0x4000
#define M_Mode1_P_Enable                            0x2000
#define M_Mode1_P_NAK_Enable                        0x2000
#define M_Mode1_P_NAK_Enable_1                      0x10
#define M_Mode1_P_AllowAck_1                        0x20
#define M_Mode1_P_OK2Rcv_1                          0x40

/* MSB250X_OTG0_PWR_REG */ /* RW */
#define MSB250X_OTG0_PWR_ISOUP		                (1 << 7)
#define MSB250X_OTG0_PWR_SOFT_CONN                  (1 << 6)
#define MSB250X_OTG0_PWR_HS_EN                      (1 << 5)
#define MSB250X_OTG0_PWR_HS_MODE                    (1 << 4)
#define MSB250X_OTG0_PWR_RESET		                (1 << 3)
#define MSB250X_OTG0_PWR_RESUME		                (1 << 2)
#define MSB250X_OTG0_PWR_SUSPEND	                (1 << 1)
#define MSB250X_OTG0_PWR_ENSUSPEND	                (1 << 0)

/* MSB250X_OTG0_INTRUSB_REG */ /* RO */
#define MSB250X_OTG0_INTRUSB_VBUS_ERR               (1 << 7)
#define MSB250X_OTG0_INTRUSB_SESS_REQ               (1 << 6)
#define MSB250X_OTG0_INTRUSB_DISCONN                (1 << 5)
#define MSB250X_OTG0_INTRUSB_CONN                   (1 << 4)
#define MSB250X_OTG0_INTRUSB_SOF                    (1 << 3)
#define MSB250X_OTG0_INTRUSB_RESET                  (1 << 2)
#define MSB250X_OTG0_INTRUSB_RESUME                 (1 << 1)
#define MSB250X_OTG0_INTRUSB_SUSPEND                (1 << 0)

/* MSB250X_OTG0_INTRUSBE_REG */ /* RW */
#define MSB250X_OTG0_INTRUSBE_VBUS_ERR              (1 << 7)
#define MSB250X_OTG0_INTRUSBE_SESS_REQ              (1 << 6)
#define MSB250X_OTG0_INTRUSBE_DISCONN               (1 << 5)
#define MSB250X_OTG0_INTRUSBE_CONN                  (1 << 4)
#define MSB250X_OTG0_INTRUSBE_SOF                   (1 << 3)
#define MSB250X_OTG0_INTRUSBE_RESET                 (1 << 2)
#define MSB250X_OTG0_INTRUSBE_BABBLE                (1 << 2)
#define MSB250X_OTG0_INTRUSBE_RESUME                (1 << 1)
#define MSB250X_OTG0_INTRUSBE_SUSPEND               (1 << 0)


/* MSB250X_OTG0_CSR0_REG */ /* RO, WO */
#define MSB250X_OTG0_CSR0_SSETUPEND                 (1 << 7)
#define MSB250X_OTG0_CSR0_SRXPKTRDY                 (1 << 6)
#define MSB250X_OTG0_CSR0_SENDSTALL                 (1 << 5)
#define MSB250X_OTG0_CSR0_SETUPEND                  (1 << 4)
#define MSB250X_OTG0_CSR0_DATAEND                   (1 << 3)
#define MSB250X_OTG0_CSR0_SENTSTALL                 (1 << 2)
#define MSB250X_OTG0_CSR0_TXPKTRDY                  (1 << 1)
#define MSB250X_OTG0_CSR0_RXPKTRDY                  (1 << 0)
#define MSB250X_OTG0_CSR0_FLUSHFIFO                 (1 << 0)

/* CSR0 in host mode */
#define MSB250X_OTG0_CSR0_STATUSPACKET              (1 << 6)
#define MSB250X_OTG0_CSR0_REQPACKET                 (1 << 5)
#define MSB250X_OTG0_CSR0_SETUPPACKET               (1 << 3)
#define MSB250X_OTG0_CSR0_RXSTALL                   (1 << 2)


/* MSB250X_OTG0_TXCSR1_REG */ /* RO, WO */
#define MSB250X_OTG0_TXCSR1_CLRDATAOTG              (1 << 6)
#define MSB250X_OTG0_TXCSR1_SENTSTALL               (1 << 5)
#define MSB250X_OTG0_TXCSR1_SENDSTALL               (1 << 4)
#define MSB250X_OTG0_TXCSR1_FLUSHFIFO               (1 << 3)
#define MSB250X_OTG0_TXCSR1_UNDERRUN                (1 << 2)
#define MSB250X_OTG0_TXCSR1_FIFONOEMPTY             (1 << 1)
#define MSB250X_OTG0_TXCSR1_TXPKTRDY                (1 << 0)

/* host mode */
#define MSB250X_OTG0_TXCSR1_RXSTALL                 (1 << 5)

/* MSB250X_OTG0_TXCSR2_REG */ /* RW */
#define MSB250X_OTG0_TXCSR2_AUTOSET                 (1 << 7)
#define MSB250X_OTG0_TXCSR2_ISOC                    (1 << 6)
#define MSB250X_OTG0_TXCSR2_MODE                    (1 << 5)
#define MSB250X_OTG0_TXCSR2_DMAREQENAB              (1 << 4)
#define MSB250X_OTG0_TXCSR2_FRCDATAOG               (1 << 3)
#define MSB250X_OTG0_TXCSR2_DMAREQMODE              (1 << 2)

/* MSB250X_OTG0_RXCSR1_REG */ /* RW, RO */
#define MSB250X_OTG0_RXCSR1_CLRDATATOG              (1 << 7)
#define MSB250X_OTG0_RXCSR1_SENTSTALL               (1 << 6)
#define MSB250X_OTG0_RXCSR1_SENDSTALL               (1 << 5)
#define MSB250X_OTG0_RXCSR1_FLUSHFIFO               (1 << 4)
#define MSB250X_OTG0_RXCSR1_DATAERROR               (1 << 3)
#define MSB250X_OTG0_RXCSR1_OVERRUN                 (1 << 2)
#define MSB250X_OTG0_RXCSR1_FIFOFULL                (1 << 1)
#define MSB250X_OTG0_RXCSR1_RXPKTRDY                (1 << 0)

/* host mode */
#define MSB250X_OTG0_RXCSR1_RXSTALL                 (1 << 6)
#define MSB250X_OTG0_RXCSR1_REQPKT                  (1 << 5)

/* MSB250X_OTG0_RXCSR2_REG */ /* RW */
#define MSB250X_OTG0_RXCSR2_AUTOCLR                 (1 << 7)
#define MSB250X_OTG0_RXCSR2_ISOC                    (1 << 6)
#define MSB250X_OTG0_RXCSR2_DMAREQEN                (1 << 5)
#define MSB250X_OTG0_RXCSR2_DISNYET                 (1 << 4)
#define MSB250X_OTG0_RXCSR2_DMAREQMD                (1 << 3)


/* MSB250X_OTG0_DEVCTL_REG */
#define MSB250X_OTG0_B_DEVIC                        (1 << 7)
#define MSB250X_OTG0_FS_DEVIC                       (1 << 6)
#define MSB250X_OTG0_LS_DEVIC                       (1 << 5)
#define MSB250X_OTG0_HOST_MODE                      (1 << 2)
#define MSB250X_OTG0_HOST_REQ                       (1 << 1)
#define MSB250X_OTG0_SESSION                        (1 << 0)

/* CH_DMA_CNTL */
#define MSB250X_OTG0_DMA_BURST_MODE                 (3 << 9)
#define MSB250X_OTG0_DMA_INT_EN                     (1 << 3)
#define MSB250X_OTG0_DMA_AUTO                       (1 << 2)
#define MSB250X_OTG0_DMA_TX                         (1 << 1)
#define MSB250X_OTG0_EN_DMA                         (1 << 0)

/* USB_CFG0_L */
#define MSB250X_OTG0_CFG0_SRST_N                    (1 << 0)

#define RXCSR2_MODE1  (MSB250X_OTG0_RXCSR2_AUTOCLR | MSB250X_OTG0_RXCSR2_DMAREQEN | MSB250X_OTG0_RXCSR2_DMAREQMD)
#define TXCSR2_MODE1  (MSB250X_OTG0_TXCSR2_DMAREQENAB | MSB250X_OTG0_TXCSR2_AUTOSET | MSB250X_OTG0_TXCSR2_DMAREQMODE)

struct otg0_ep_txcsr_h {
    __u8    bUnused:        2;
    __u8    bDMAReqMode:    1;
    __u8    bFrcDataTog:    1;
    __u8    bDMAReqEnab:    1;
    __u8    bMode:          1;
    __u8    bISO:           1;
    __u8    bAutoSet:       1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep_txcsr_l {
    __u8    bTxPktRdy:      1;
    __u8    bFIFONotEmpty:  1;
    __u8    bUnderRun:      1;
    __u8    bFlushFIFO:     1;
    __u8    bSendStall:     1;
    __u8    bSentStall:     1;
    __u8    bClrDataTog:    1;
    __u8    bIncompTx:      1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep_rxcsr_h {
    __u8    bIncompRx:      1;
    __u8    bUnused:        2;
    __u8    bDMAReqMode:    1;
    __u8    bDisNyet:       1;
    __u8    bDMAReqEnab:    1;
    __u8    bISO:           1;
    __u8    bAutoClear:     1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep_rxcsr_l {
    __u8    bRxPktRdy:      1;
    __u8    bFIFOFull:      1;
    __u8    bOverRun:       1;
    __u8    bDataError:     1;
    __u8    bFlushFIFO:     1;
    __u8    bSendStall:     1;
    __u8    bSentStall:     1;
    __u8    bClrDataTog:    1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep0_csr_h {
    __u8    bFlushFIF0:     1;
    __u8    bUnused:        7;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep0_csr_l {
    __u8    bRxPktRdy:          1;
    __u8    bTxPktRdy:          1;
    __u8    bSentStall:         1;
    __u8    bDataEnd:           1;
    __u8    bSetupEnd:          1;
    __u8    bSendStall:         1;
    __u8    bServicedRxPktRdy:  1;
    __u8    bServicedSetupEnd:  1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_usb_power {
    __u8    bEnableSuspendM:    1;
    __u8    bSuspendMode:       1;
    __u8    bResume:            1;
    __u8    bReset:             1;
    __u8    bHSMode:            1;
    __u8    bHSEnab:            1;
    __u8    bSoftConn:          1;
    __u8    bISOUpdate:         1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_usb_intr {
    __u8    bSuspend:   1;
    __u8    bResume:    1;
    __u8    bReset:     1;
    __u8    bSOF:       1;
    __u8    bConn:      1;
    __u8    bDiscon:    1;
    __u8    bSessReq:   1;
    __u8    bVBusError: 1;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_usb_cfg0_l {
    __u8    bSRST_N:        1;
    __u8    bOTG_TM_1:      1;
    __u8    bDebugSel:      4;
    __u8    bUSBOTG:        1;
    __u8    bMIUPriority:   1;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_usb_cfg0_h {
    __u8    bEP_BULKOUT_1:      4;
    __u8    bECO4NAK_EN_1:      1;
    __u8    bSetAllow_ACK_1:    1;
    __u8    bSet_OK2Rcv_1:      1;
    __u8    bDMPullDown:        1;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_usb_cfg6_h {
    __u8    bDMABugFix:             1;
    __u8    bDMAMCU_RD_Fix:         1;
    __u8    bDMAMCU_WR_Fix:         1;
    __u8    bINT_WR_CLR_EN:         1;
    __u8    bMCU_HLT_DMA_EN:        1;
    __u8    bShortMode:             1;
    __u8    bBusOPFix:              1;
    __u8    bREG_MI_WDFIFO_CTRL:    1;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

struct utmi_signal_status_l {
    __u8    bVBUSVALID:     1;
    __u8    bAVALID:        1;
    __u8    bBVALID:        1;
    __u8    bIDDIG:         1;
    __u8    bHOSTDISCON:    1;
    __u8    bSESSEND:       1;
    __u8    bLINESTATE:     2;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

struct usbc0_rst_ctrl_l {
    __u8    bUSB_RST:           1;
    __u8    bUHC_RST:           1;
    __u8    bOTG_RST:           1;
    __u8    bREG_SUSPEND:       1;
    __u8    bReserved0:         1;
    __u8    bUHC_XIU_ENABLE:    1;
    __u8    bOTG_XIU_ENABLE:    1;
    __u8    bReserved1:         1;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_dma_ctrlrequest {
    __u8    bEnableDMA:         1;
    __u8    bDirection:         1;
    __u8    bDMAMode:           1;
    __u8    bInterruptEnable:   1;
    __u8    bEndpointNumber:    4;
    __u8    bBusError:          1;
    __u8    bRurstMode:         2;
	__u8    bReserved:          5;
} __attribute__ ((packed))__attribute__ ((aligned(16)));

struct otg0_ep_tx_maxp {
    __u16   wMaximumPayload:    11;
    __u8    bMult:              2;
    __u8    bUnused:            3;
}  __attribute__ ((packed))__attribute__ ((aligned(16)));

#endif
