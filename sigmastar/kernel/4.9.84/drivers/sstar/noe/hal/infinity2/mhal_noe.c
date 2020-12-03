/*
* mhal_noe.c- Sigmastar
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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mhal_noe.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "mdrv_types.h"
#include "ms_platform.h"
#include "registers.h"
#include "irqs.h"
#include "mhal_noe_reg.h"
#include "mhal_noe.h"
#include "mhal_noe_lro.h"



//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef enum {
    E_NOE_INTERFACE_RGMII_MODE = 0,
    E_NOE_INTERFACE_MII_MODE,
    E_NOE_INTERFACE_RMII_MODE,
    E_NOE_INTERFACE_INVALID_MODE,
}EN_NOE_INTERFACE;


typedef enum {
    E_MHAL_NOE_DELAY_NS_0,
    E_MHAL_NOE_DELAY_NS_2,
}EN_MHAL_NOE_DELAY_TIME;

struct _mhal_mii_info{
    unsigned char noe_mii_force_mode;
    u32 phy_addr;
    EN_NOE_SPEED speed;
    EN_NOE_DUPLEX duplex;
    EN_NOE_INTERFACE mode;
};

struct _mhal_intr_info {
    void __iomem *fe_tx_int_status;
    void __iomem *fe_tx_int_enable;
    void __iomem *fe_rx_int_status;
    void __iomem *fe_rx_int_enable;
    MS_U32 reg_int_val_p;
    MS_U32 reg_int_val_q;
    unsigned int tx_mask;
    unsigned int rx_mask;
};

struct _mhal_noe_config{
    struct _mhal_mii_info mii_info[E_NOE_GE_MAC_MAX];
    EN_NOE_HAL_LOG noe_dbg_enable;
    struct _mhal_intr_info intr_info;
    EN_NOE_SEL noe_internal_phy_enable;
    EN_NOE_SEL_PIN_MUX pin_mux;
    MS_U8 bAN_Supported; /* 0: FALSE, 1:TRUE*/
    EN_MHAL_NOE_DELAY_TIME delay;
    MS_BOOL bGPHY_Inited;
    MS_BOOL bQoS_SetMode;
    EN_NOE_HAL_STATUS status;
};


typedef void (*_MHAL_DMA_GET_INFO) (void *);
typedef void (*_MHAL_PINMUX_SEL) (void );
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define NOE_WAIT_IDLE_THRESHOLD (1000)
#define NOE_QOS_SET_REG(x)    (*((volatile u32 *)(NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + x)))
#define NOE_AUTO_POLLING_IS_NOT_SUPPORTED()  (0)

#define NOE_IS_INVALID_MUX(mux)  ((mux <= E_NOE_SEL_PIN_MUX_INVALID) || (mux >= E_NOE_SEL_PIN_MUX_MAX))


#define MHAL_NOE_WARN_INFO(fmt, args...) printk(fmt, ##args)

#define MHAL_NOE_DBG_INFO(fmt, args...) \
{\
    if (noe_config.noe_dbg_enable & E_NOE_HAL_LOG_DBG)\
        printk(fmt, ##args);\
}

#define MHAL_NOE_INTR_INFO(fmt, args...) \
{\
    if (noe_config.noe_dbg_enable & E_NOE_HAL_LOG_INTR)\
        printk(fmt, ##args);\
}

//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------
static void _MHal_NOE_PDMA_Get_Dbg(void *dbg_info);
static void _MHal_NOE_QDMA_Get_Cnt(void *cnt_info);
static void _MHal_NOE_QDMA_Get_Fq(void *fq_info);
static void _MHal_NOE_QDMA_Get_Sch(void *rate_info);
static void _MHal_NOE_QDMA_Get_Fc(void *fc_info);
static void _MHal_NOE_QDMA_Get_Fsm(void *fsm_info);
static void _MHal_NOE_QDMA_Get_Vq(void *vq_info);
static void _MHal_NOE_QDMA_Get_Pq(void *pq_info);
static void _MHal_NOE_GE1_To_CT0(void);
static void _MHal_NOE_GE2_To_CT1(void);
static void _MHal_NOE_GE1_To_CT0_GE2_To_CT1(void);
static void _MHal_NOE_Set_Auto_Polling(EN_NOE_SEL enable);

//--------------------------------------------------------------------------------------------------
//  Variable
//--------------------------------------------------------------------------------------------------

void __iomem *ethdma_sysctl_base;
EXPORT_SYMBOL(ethdma_sysctl_base);


//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------

static _MHAL_DMA_GET_INFO _mhal_noe_qdma_get_info_pfn[E_NOE_QDMA_INFO_MAX] = {
    [E_NOE_QDMA_INFO_CNT] = _MHal_NOE_QDMA_Get_Cnt,
    [E_NOE_QDMA_INFO_FQ] = _MHal_NOE_QDMA_Get_Fq,
    [E_NOE_QDMA_INFO_SCH] = _MHal_NOE_QDMA_Get_Sch,
    [E_NOE_QDMA_INFO_FC] = _MHal_NOE_QDMA_Get_Fc,
    [E_NOE_QDMA_INFO_FSM] = _MHal_NOE_QDMA_Get_Fsm,
    [E_NOE_QDMA_INFO_VQ] = _MHal_NOE_QDMA_Get_Vq,
    [E_NOE_QDMA_INFO_PQ] = _MHal_NOE_QDMA_Get_Pq,
};

static _MHAL_DMA_GET_INFO _mhal_noe_pdma_get_info_pfn[E_NOE_PDMA_INFO_MAX] = {
    [E_NOE_PDMA_INFO_DBG] = _MHal_NOE_PDMA_Get_Dbg,
};

static _MHAL_PINMUX_SEL _mhal_noe_pinmux_pfn[E_NOE_SEL_PIN_MUX_MAX] = {
    [E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP] = _MHal_NOE_GE1_To_CT0,
    [E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP] = _MHal_NOE_GE2_To_CT1,
    [E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP_GE2_TO_CHIPTOP] = _MHal_NOE_GE1_To_CT0_GE2_To_CT1,
};

static struct _mhal_noe_config  noe_config = {
    .mii_info = {
        [E_NOE_GE_MAC1] = {
            .noe_mii_force_mode = NOE_DISABLE,
            .phy_addr = NOE_INVALID_PHY_ADDR,
            .speed = E_NOE_SPEED_INVALID,
            .duplex = E_NOE_DUPLEX_INVALID,
            .mode = E_NOE_INTERFACE_INVALID_MODE,
        },
        [E_NOE_GE_MAC2] = {
            .noe_mii_force_mode = NOE_DISABLE,
            .phy_addr = NOE_INVALID_PHY_ADDR,
            .speed = E_NOE_SPEED_INVALID,
            .duplex = E_NOE_DUPLEX_INVALID,
            .mode = E_NOE_INTERFACE_INVALID_MODE,
        },
    },
    .noe_dbg_enable = NOE_DISABLE,
    .intr_info = {
        .fe_tx_int_status = NULL,
        .fe_tx_int_enable = NULL,
        .fe_rx_int_status = NULL,
        .fe_rx_int_enable = NULL,
    },
    .bAN_Supported = FALSE,
    .delay = E_MHAL_NOE_DELAY_NS_2,
    .bGPHY_Inited = FALSE,
    .bQoS_SetMode = TRUE,
    .pin_mux = E_NOE_SEL_PIN_MUX_INVALID,
};

static inline void _MHal_NOE_Switch_MDIO(u32 addr)
{
}

static inline void _MHal_NOE_Reset_SW(void)
{
    unsigned int val;
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x00 << 1) = 0x0002;
    udelay(1);
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x00 << 1) = 0x0000;

    val = MHal_NOE_Read_Reg(RSTCTRL);
    val = val | RSTCTL_SYS_RST;
    MHal_NOE_Write_Reg(RSTCTRL, val);
    udelay(10);
    MHal_NOE_Write_Reg(RSTSTAT, RSTSTAT_SWSYSRST);
}

void MHal_NOE_Reset_SW(void)
{
    _MHal_NOE_Reset_SW();
}

static void _MHal_NOE_Init_Clock(void)
{
    /* NOE */
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN1, 0x26) = 0x0400;
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN1, 0x28) = 0x0004;
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN1, 0x2a) = 0x0000;

    /* Reset */
    //_MHal_NOE_Reset_SW();

    /* GMACPLL */
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN1, 0x15 << 1) = 0x0000;
    NOE_RIU_REG(NOE_RIU_BANK_ANA_MISC_GMAC, 0x69 << 1) = 0x0014;
    NOE_RIU_REG(NOE_RIU_BANK_ANA_MISC_GMAC, 0x68 << 1) = 0xBC4F;
    MHal_NOE_Write8_Reg(NOE_RIU_BANK_CLKGEN0, (0x63 << 1), 0x00);
    NOE_RIU_REG(NOE_RIU_BANK_ANA_MISC_GMAC, 0x63 << 1) &= 0xFF00;
}

static void _MHal_NOE_Set_MDIO(void)
{
    NOE_RIU8_REG(0x1224, 0x04) |= 0x03;
    NOE_RIU8_REG(0x1026, 0x0a) |= 0x40;

}

static void _MHal_NOE_GE1_To_CT0(void)
{
    // Clock
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN2, 0x02) = (NOE_RIU_REG(NOE_RIU_BANK_CLKGEN2, 0x02) & 0x00FF) | 0x0C00;
    NOE_RIU_REG(0x1026, 0x0a) |= 0x00c0;
    NOE_RIU8_REG(0x1026, 0x00)= 0x00;
    NOE_RIU8_REG(0x121f, 0x23)= 0x00;
    NOE_RIU_REG(0x1224, 0x6e) = 0x1108;
    NOE_RIU_REG(0x1224, 0x22) = 0x0088;
    NOE_RIU_REG(0x1224, 0x24) = 0x2191;
#ifdef CONFIG_NOE_RGMII_TX_DELAY_2NS
    NOE_RIU_REG(0x1224, 0x26) = 0x6EE2;
#else
    NOE_RIU_REG(0x1224, 0x26) = 0x6CE2;
#endif
    NOE_RIU_REG(0x1224, 0x20) = 0xa200;
    NOE_RIU_REG(0x1224, 0x04) = 0x0103;
}





static void _MHal_NOE_GE2_To_CT1(void)
{
    // Clock
    NOE_RIU_REG(NOE_RIU_BANK_CLKGEN2, 0xA0) = (NOE_RIU_REG(NOE_RIU_BANK_CLKGEN2, 0xA0) & 0x00FF) | 0x0C00;

    NOE_RIU8_REG(0x1026, 0x0b) |= 0x02;
    NOE_RIU8_REG(0x1026, 0x00) = 0x00;
    NOE_RIU8_REG(0x121f, 0x23) = 0x00;
    NOE_RIU_REG(0x1224, 0xee) = 0x1108;
    NOE_RIU_REG(0x1224, 0xa2) = 0x0088;
    NOE_RIU_REG(0x1224, 0xa4) = 0x2191;
#ifdef CONFIG_NOE_RGMII_TX_DELAY_2NS
    NOE_RIU_REG(0x1224, 0xa6) = 0x6EE2;
#else
    NOE_RIU_REG(0x1224, 0xa6) = 0x6CE2;
#endif
    NOE_RIU_REG(0x1224, 0xa0) = 0xa200;
    NOE_RIU_REG(0x1224, 0x84) = 0x0103;
}


static void _MHal_NOE_GE1_To_CT0_GE2_To_CT1(void)
{
    _MHal_NOE_GE1_To_CT0();
    _MHal_NOE_GE2_To_CT1();
}

static inline void _MHal_NOE_Enable_Mdio(EN_NOE_SEL enable)
{

}


void MHal_NOE_Write_Bits_Zero(void __iomem *addr, MS_U32 bit, MS_U32 len)
{
    int reg_val;
    int i;

    reg_val = MHal_NOE_Read_Reg(addr);
    for (i = 0; i < len; i++)
        reg_val &= ~(1 << (bit + i));
    MHal_NOE_Write_Reg(addr, reg_val);
}

void MHal_NOE_Write_Bits_One(void __iomem *addr, MS_U32 bit, MS_U32 len)
{
    unsigned int reg_val;
    unsigned int i;

    reg_val = MHal_NOE_Read_Reg(addr);
    for (i = 0; i < len; i++)
        reg_val |= 1 << (bit + i);
    MHal_NOE_Write_Reg(addr, reg_val);
}

EN_NOE_RET MHal_NOE_Set_Pin_Mux(EN_NOE_SEL_PIN_MUX mux)
{
    if (NOE_IS_INVALID_MUX(mux)) {
        return E_NOE_RET_INVALID_PARAM;
    }
    MHAL_NOE_DBG_INFO("mux=%d\n",mux);

    noe_config.pin_mux = mux;
    if (_mhal_noe_pinmux_pfn[mux] == NULL) {
        return E_NOE_RET_INVALID_PARAM;
    }
    _mhal_noe_pinmux_pfn[mux]();
    _MHal_NOE_Set_MDIO();
    return E_NOE_RET_TRUE;
}

void MHal_NOE_Set_MAC_Address(EN_NOE_GE_MAC ge, unsigned char p[6])
{
    unsigned long reg_value;
    reg_value = (p[0] << 8) | (p[1]);
    if (ge == E_NOE_GE_MAC1) {
        MHal_NOE_Write_Reg(GDMA1_MAC_ADRH, reg_value);
    }
    else if(ge == E_NOE_GE_MAC2) {
        MHal_NOE_Write_Reg(GDMA2_MAC_ADRH, reg_value);
    }
    reg_value = (unsigned long)((p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5]);
    if(ge == E_NOE_GE_MAC1) {
        MHal_NOE_Write_Reg(GDMA1_MAC_ADRL, reg_value);
    }
    else if(ge == E_NOE_GE_MAC2) {
        MHal_NOE_Write_Reg(GDMA2_MAC_ADRL, reg_value);
    }
    MHAL_NOE_DBG_INFO("GMAC%d_MAC_ADRH -- : 0x%08x\n", ge+1, (ge == E_NOE_GE_MAC1)?MHal_NOE_Read_Reg(GDMA1_MAC_ADRH):MHal_NOE_Read_Reg(GDMA2_MAC_ADRH));
    MHAL_NOE_DBG_INFO("GMAC%d_MAC_ADRL -- : 0x%08x\n", ge+1, (ge == E_NOE_GE_MAC1)?MHal_NOE_Read_Reg(GDMA1_MAC_ADRL):MHal_NOE_Read_Reg(GDMA2_MAC_ADRL));
}

void MHal_NOE_Get_MAC_Address(EN_NOE_GE_MAC ge, unsigned char* p)
{
    unsigned long reg_value;
    unsigned long address;

    if (ge == E_NOE_GE_MAC1) {
        address = (unsigned long)GDMA1_MAC_ADRH;
    }
    else if(ge == E_NOE_GE_MAC2) {
        address = (unsigned long)GDMA2_MAC_ADRH;
    }

    reg_value = MHal_NOE_Read_Reg(address);
    p[0]= (reg_value>>8)&0xFF;
    p[1]= (reg_value)&0xFF;

    if (ge == E_NOE_GE_MAC1) {
        address = (unsigned long)GDMA1_MAC_ADRL;
    }
    else if(ge == E_NOE_GE_MAC2) {
        address = (unsigned long)GDMA2_MAC_ADRL;
    }

    reg_value = MHal_NOE_Read_Reg(address);
    p[5]= (reg_value)&0xFF;
    p[4]= (reg_value>>8)&0xFF;
    p[3]= (reg_value>>16)&0xFF;
    p[2]= (reg_value>>24)&0xFF;
}

void MHal_NOE_Stop(void)
{
    unsigned int reg_value;

    MHAL_NOE_DBG_INFO("%s()...",__FUNCTION__);
    reg_value = MHal_NOE_Read_Reg(DMA_GLO_CFG);
    reg_value &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
    MHal_NOE_Write_Reg(DMA_GLO_CFG, reg_value);

    MHAL_NOE_DBG_INFO("Done\n");
}


void MHal_NOE_Reset_FE(void)
{
    unsigned int val;
    val = MHal_NOE_Read_Reg(RSTCTRL);
    val = val | RSTCTL_FE_RST;
    MHal_NOE_Write_Reg(RSTCTRL, val);

    val = val & ~(RSTCTL_FE_RST);
    MHal_NOE_Write_Reg(RSTCTRL, val);
}


void MHal_NOE_Set_DBG(EN_NOE_HAL_LOG level)
{
    noe_config.noe_dbg_enable = level;
}

void MHal_NOE_Reset_GMAC(void)
{
}

static EN_NOE_RET _MHal_NOE_Write_Mii_Mgr(u32 addr, u32 reg, u32 write_data)
{
    unsigned long t_start = jiffies;
    u32 data;

    _MHal_NOE_Switch_MDIO(addr);
    _MHal_NOE_Enable_Mdio(E_NOE_SEL_ENABLE);

    /* make sure previous write operation is complete */
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            break;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Write operation ongoing\n");
            return E_NOE_RET_DEVICE_BUSY;
        }
    }
    data = (0x01 << 16) | (1 << 18) | (addr << 20) | (reg << 25) | write_data;
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, data);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, (data | (1 << 31))); /*start*/
    /* MHAL_NOE_DBG_INFO("\n Set Command [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0); */

    t_start = jiffies;

    /* make sure write operation is complete */
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            return E_NOE_RET_TRUE;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Write operation Time Out\n");
            return E_NOE_RET_TIMEOUT;
        }
    }
    return E_NOE_RET_FALSE;
}

static EN_NOE_RET _MHal_NOE_Read_Mii_Mgr(u32 addr, u32 reg, u32 *read_data)
{
    MS_U32 status = 0;
    unsigned long t_start = jiffies;
    MS_U32 data = 0;

    _MHal_NOE_Switch_MDIO(addr);
    /* We enable mdio gpio purpose register, and disable it when exit. */
    _MHal_NOE_Enable_Mdio(E_NOE_SEL_ENABLE);

    /* make sure previous read operation is complete */
    while (1) {
        /* 0 : Read/write operation complete */
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            break;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation is ongoing !!\n");
            return E_NOE_RET_DEVICE_BUSY;
        }
    }

    data = (0x01 << 16) | (0x02 << 18) | (addr << 20) | (reg << 25);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, data);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

    /* make sure read operation is complete */
    t_start = jiffies;
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            status = MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0);
            *read_data = (MS_U32)(status & 0x0000FFFF);
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            return E_NOE_RET_TRUE;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation Time Out!!\n");
            return E_NOE_RET_TIMEOUT;
        }
    }
    return E_NOE_RET_FALSE;
}


static void _MHal_NOE_Set_Auto_Polling(EN_NOE_SEL enable)
{
    if (enable == E_NOE_SEL_ENABLE)
    {
        MHal_NOE_Write_Reg(ESW_PHY_POLLING, MHal_NOE_Read_Reg(ESW_PHY_POLLING)|(1 << 31));
    }
    else{
        MHal_NOE_Write_Reg(ESW_PHY_POLLING, MHal_NOE_Read_Reg(ESW_PHY_POLLING)&(~(1 << 31)));
    }
}


static MS_U8 MHal_NOE_Is_Mii_Mgr_Force_Mode(u32 addr)
{
    unsigned char i = 0;
    for (i = 0; i < E_NOE_GE_MAC_MAX; i++)
    {
        if (noe_config.mii_info[i].phy_addr == addr) {
            return noe_config.mii_info[i].noe_mii_force_mode;
        }
    }
    return NOE_DISABLE;
}

static EN_NOE_RET _MHal_NOE_Set45_Mii_Mgr_Addr(u32 port_num, u32 dev_addr, u32 reg_addr)
{
    unsigned long t_start = jiffies;
    MS_U32 data = 0;

    _MHal_NOE_Enable_Mdio(E_NOE_SEL_ENABLE);

    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            break;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation is ongoing !!\n");
            return E_NOE_RET_DEVICE_BUSY;
        }
    }
    data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | reg_addr;
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, data);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

    t_start = jiffies;
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            return E_NOE_RET_TRUE;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Write operation Time Out\n");
            return E_NOE_RET_TIMEOUT;
        }
    }
    return E_NOE_RET_FALSE;
}


EN_NOE_RET MHal_NOE_Write45_Mii_Mgr(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
    unsigned long t_start = jiffies;
    u32 data = 0;
    MS_U32 an_status = 0;
    an_status = (MHal_NOE_Read_Reg(ESW_PHY_POLLING) & (1 << 31));
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_DISABLE);
    }

    /* set address first */
    _MHal_NOE_Set45_Mii_Mgr_Addr(port_num, dev_addr, reg_addr);
    udelay(10);

    _MHal_NOE_Enable_Mdio(E_NOE_SEL_ENABLE);
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            break;
        }
        else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation is ongoing !!\n");
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_DEVICE_BUSY;
        }
    }

    data = (dev_addr << 25) | (port_num << 20) | (0x01 << 18) | (0x00 << 16) | write_data;
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, data);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

    t_start = jiffies;

    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TRUE;
        } else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Write operation Time Out\n");
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TIMEOUT;
        }
    }
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
    }
    return E_NOE_RET_FALSE;
}

EN_NOE_RET MHal_NOE_Read45_Mii_Mgr(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
    u32 status = 0;
    unsigned long t_start = jiffies;
    u32 data = 0;
    MS_U32 an_status = 0;
    an_status = (MHal_NOE_Read_Reg(ESW_PHY_POLLING) & (1 << 31));
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_DISABLE);
    }

    /* set address first */
    _MHal_NOE_Set45_Mii_Mgr_Addr(port_num, dev_addr, reg_addr);
    udelay(10);

    _MHal_NOE_Enable_Mdio(E_NOE_SEL_ENABLE);

    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            break;
        }
        else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation is ongoing !!\n");
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_DEVICE_BUSY;
        }
    }
    data = (dev_addr << 25) | (port_num << 20) | (0x03 << 18) | (0x00 << 16) | reg_addr;
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, data);
    MHal_NOE_Write_Reg(MDIO_PHY_CONTROL_0, (data | (1 << 31)));
    t_start = jiffies;
    while (1) {
        if (!(MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
            *read_data = (MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0) & 0x0000FFFF);
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TRUE;
        }
        else if (time_after(jiffies, t_start + 5 * HZ)) {
            _MHal_NOE_Enable_Mdio(E_NOE_SEL_DISABLE);
            MHAL_NOE_DBG_INFO("\n MDIO Read operation Time Out!!\n");
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TIMEOUT;
        }
        status = MHal_NOE_Read_Reg(MDIO_PHY_CONTROL_0);
    }
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
    }
    return E_NOE_RET_FALSE;
}

EN_NOE_RET MHal_NOE_Write_Mii_Mgr(u32 addr, u32 reg, u32 write_data)
{
    MS_U32 an_status = 0;
    an_status = (MHal_NOE_Read_Reg(ESW_PHY_POLLING) & (1 << 31));
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_DISABLE);
    }
    if (MHal_NOE_Is_Mii_Mgr_Force_Mode(addr) == NOE_ENABLE) {
        if (addr == 31) {
            if(_MHal_NOE_Write_Mii_Mgr(addr, 0x1f, (reg>>6)&0x3FFF) == E_NOE_RET_TRUE) {
                if(_MHal_NOE_Write_Mii_Mgr(addr, (reg>>2) & 0xF, write_data & 0xFFFF) == E_NOE_RET_TRUE) {
                    if(_MHal_NOE_Write_Mii_Mgr(addr, (0x1 << 4), write_data >> 16) == E_NOE_RET_TRUE) {
                        if (an_status) {
                            _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
                        }
                        return E_NOE_RET_TRUE;
                    }
                }
            }
        }
        else {
            if (_MHal_NOE_Write_Mii_Mgr(addr, reg, write_data) == E_NOE_RET_TRUE) {
                if (an_status) {
                    _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
                }
                return E_NOE_RET_TRUE;
            }
        }
    }
    else {
        if (_MHal_NOE_Write_Mii_Mgr(addr, reg, write_data) == E_NOE_RET_TRUE) {
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TRUE;
        }
    }

    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
    }
    return E_NOE_RET_FALSE;
}

EN_NOE_RET MHal_NOE_Read_Mii_Mgr(u32 addr, u32 reg, u32 *read_data)
{
    u32 low_word;
    u32 high_word;
    u32 an_status = 0;

    an_status = (MHal_NOE_Read_Reg(ESW_PHY_POLLING) & (1 << 31));
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_DISABLE);
    }

    if (MHal_NOE_Is_Mii_Mgr_Force_Mode(addr) == NOE_ENABLE) {
        if (addr == 31) {
            if (_MHal_NOE_Write_Mii_Mgr(addr, 0x1f, (reg >> 6) & 0x3FFF) == E_NOE_RET_TRUE) {
                if (_MHal_NOE_Read_Mii_Mgr(addr, (reg >> 2) & 0xF, &low_word) == E_NOE_RET_TRUE ) {
                    if (_MHal_NOE_Read_Mii_Mgr(addr, (0x1 << 4), &high_word) == E_NOE_RET_TRUE) {
                        *read_data = (high_word << 16) | (low_word & 0xFFFF);
                        if (an_status) {
                            _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
                        }
                        return E_NOE_RET_TRUE;
                    }
                }
            }
        }
        else {
            if (_MHal_NOE_Read_Mii_Mgr(addr, reg, read_data) == E_NOE_RET_TRUE) {
                if (an_status) {
                    _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
                }
                return E_NOE_RET_TRUE;
            }
        }
    }
    else
    {
        if (_MHal_NOE_Read_Mii_Mgr(addr, reg, read_data) == E_NOE_RET_TRUE) {
            if (an_status) {
                _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
            }
            return E_NOE_RET_TRUE;
        }
    }
    if (an_status) {
        _MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
    }

    return E_NOE_RET_FALSE;
}



MS_BOOL MHal_NOE_Support_Auto_Polling(void)
{
    return TRUE;
}

void MHal_NOE_Set_Auto_Polling(EN_NOE_SEL enable)
{
    MS_U32 reg_value;

    /* FIXME: we don't know how to deal with PHY end addr */
    reg_value = MHal_NOE_Read_Reg(ESW_PHY_POLLING);
    reg_value &= ~(0x1f);
    reg_value &= ~(0x1f << 8);

    if (E_NOE_SEL_ENABLE == enable) {
        reg_value |= (1 << 31);
    }
    else {
        reg_value &= 0x7FFFFFFF;
    }

    /* setup PHY address for auto polling (Start Addr). */
    reg_value |= noe_config.mii_info[E_NOE_GE_MAC1].phy_addr & 0x1F;

    /* setup PHY address for auto polling (End Addr). */
    reg_value |= (noe_config.mii_info[E_NOE_GE_MAC2].phy_addr & 0x1F) << 8;


    MHAL_NOE_DBG_INFO("[%s][%d] reg_value=0x%x\n", __FUNCTION__, __LINE__, reg_value);
    MHal_NOE_Write_Reg(ESW_PHY_POLLING, reg_value);
}

void MHal_NOE_Force_Link_Mode(EN_NOE_GE_MAC ge, EN_NOE_SPEED speed, EN_NOE_DUPLEX duplex)
{
    MS_U32 reg_value = 0;

    if(ge == E_NOE_GE_MAC1) {
        reg_value = MHal_NOE_Read_Reg(NOE_MAC_P1_MCR);
    }
    else if(ge == E_NOE_GE_MAC2) {
        reg_value = MHal_NOE_Read_Reg(NOE_MAC_P2_MCR);
    }

    MHAL_NOE_DBG_INFO("GMAC%d: 0x%x \n", ge+1  , reg_value);
    if ((duplex == E_NOE_DUPLEX_INVALID) && (speed == E_NOE_SPEED_INVALID)) {
        reg_value &= (~0x8031);
    }
    else {
        reg_value &= (~(0x2|0xC));
        reg_value |= (0x01 | 0x30 | 0xE000);

        if(speed == E_NOE_SPEED_1000) {
            reg_value |= 0x8;
            if (duplex == E_NOE_DUPLEX_FULL) {
                reg_value |= 0x2;
            }
        }
        else if(speed == E_NOE_SPEED_100) {
            reg_value |= 0x4;
            if (duplex == E_NOE_DUPLEX_FULL) {
                reg_value |= 0x2;
            }
        }
        else {
            if (duplex == E_NOE_DUPLEX_FULL) {
                reg_value |= 0x2;
            }
        }
    }

    if(ge == E_NOE_GE_MAC1) {
        MHal_NOE_Write_Reg(NOE_MAC_P1_MCR, reg_value);
        MHAL_NOE_DBG_INFO("GMAC1: 0x%x \n",MHal_NOE_Read_Reg(NOE_MAC_P1_MCR));
    }
    else if(ge == E_NOE_GE_MAC2) {
        MHal_NOE_Write_Reg(NOE_MAC_P2_MCR, reg_value);
        MHAL_NOE_DBG_INFO("GMAC2: 0x%x \n",MHal_NOE_Read_Reg(NOE_MAC_P2_MCR));
    }
}

static void _MHal_NOE_Set_Interface_Mode(EN_NOE_GE_MAC ge, EN_NOE_INTERFACE mode)
{
    MS_U32 val = 0;
    if(ge == E_NOE_GE_MAC1) {
        val = MHal_NOE_Read_Reg(SYSCFG1) & (~(0x3 << 12)); /* E_NOE_INTERFACE_RGMII_MODE */
        if (mode == E_NOE_INTERFACE_MII_MODE)
            val = val | BIT(12);
        else if (mode == E_NOE_INTERFACE_RMII_MODE)
            val = val | BIT(13);
        MHal_NOE_Write_Reg(SYSCFG1, val);
        noe_config.mii_info[E_NOE_GE_MAC1].mode = mode;
    }
    else if(ge == E_NOE_GE_MAC2) {
        val = MHal_NOE_Read_Reg(SYSCFG1) & (~(0x3 << 14)); /* E_NOE_INTERFACE_RGMII_MODE */
        if (mode == E_NOE_INTERFACE_MII_MODE)
            val = val | BIT(14);
        else if (mode == E_NOE_INTERFACE_RMII_MODE)
            val = val | BIT(15)| BIT(14);
        MHal_NOE_Write_Reg(SYSCFG1, val);
        noe_config.mii_info[E_NOE_GE_MAC2].mode = mode;
    }
}

void MHal_NOE_Init_Mii_Mgr(EN_NOE_GE_MAC ge, u32 phy_addr, unsigned char force_mode)
{
    if (ge >= E_NOE_GE_MAC_MAX) {
        return;
    }
    noe_config.mii_info[ge].noe_mii_force_mode = force_mode;
    noe_config.mii_info[ge].phy_addr = phy_addr;
}


EN_NOE_RET MHal_NOE_LRO_Set_Ring_Cfg(EN_NOE_RING ring_no, unsigned int sip, unsigned int dip, unsigned int sport, unsigned int dport)
{
    MS_U32 ring_idx;
    if ((ring_no != E_NOE_RING_NO1) &&
        (ring_no != E_NOE_RING_NO2) &&
        (ring_no != E_NOE_RING_NO3)) {
        MHAL_NOE_DBG_INFO("invalid ring_no=%d\n", ring_no);
        return E_NOE_RET_INVALID_PARAM;
    }


    if (ring_no == E_NOE_RING_NO1)
        ring_idx = ADMA_RX_RING1;
    else if (ring_no == E_NOE_RING_NO2)
        ring_idx = ADMA_RX_RING2;
    else if (ring_no == E_NOE_RING_NO3)
        ring_idx = ADMA_RX_RING3;

    MHAL_NOE_DBG_INFO("set_fe_lro_ring%d_cfg()\n", ring_idx);

    /* 1. Set RX ring mode to force port */
    SET_PDMA_RXRING_MODE(ring_idx, PDMA_RX_FORCE_PORT);

    /* 2. Configure lro ring */
    /* 2.1 set src/destination TCP ports */
    SET_PDMA_RXRING_TCP_SRC_PORT(ring_idx, sport);
    SET_PDMA_RXRING_TCP_DEST_PORT(ring_idx, dport);
    /* 2.2 set src/destination IPs */
    if(ring_idx == E_NOE_RING_NO1) {
        MHal_NOE_Write_Reg(LRO_RX_RING1_SIP_DW0, sip);
        MHal_NOE_Write_Reg(LRO_RX_RING1_DIP_DW0, dip);
    }
    else if(ring_idx == E_NOE_RING_NO2) {
        MHal_NOE_Write_Reg(LRO_RX_RING2_SIP_DW0, sip);
        MHal_NOE_Write_Reg(LRO_RX_RING2_DIP_DW0, dip);
    }
    else if(ring_idx == E_NOE_RING_NO3) {
        MHal_NOE_Write_Reg(LRO_RX_RING3_SIP_DW0, sip);
        MHal_NOE_Write_Reg(LRO_RX_RING3_DIP_DW0, dip);
    }
    /* 2.3 IPv4 force port mode */
    SET_PDMA_RXRING_IPV4_FORCE_MODE(ring_idx, 1);
    /* 2.4 IPv6 force port mode */
    SET_PDMA_RXRING_IPV6_FORCE_MODE(ring_idx, 1);

    /* 3. Set Age timer: 10 msec. */
    SET_PDMA_RXRING_AGE_TIME(ring_idx, HW_LRO_AGE_TIME);

    /* 4. Valid LRO ring */
    SET_PDMA_RXRING_VALID(ring_idx, 1);

    return E_NOE_RET_TRUE;
}


void MHal_NOE_LRO_Set_Cfg(void)
{
    unsigned int reg_val = 0;

    MHAL_NOE_DBG_INFO("set_fe_lro_glo_cfg()\n");

    /* 1 Set max AGG timer: 10 msec. */
    SET_PDMA_LRO_MAX_AGG_TIME(HW_LRO_AGG_TIME);

    /* 2. Set max LRO agg count */
    SET_PDMA_LRO_MAX_AGG_CNT(HW_LRO_MAX_AGG_CNT);

    /* PDMA prefetch enable setting */
    SET_PDMA_LRO_RXD_PREFETCH_EN(ADMA_RXD_PREFETCH_EN | ADMA_MULTI_RXD_PREFETCH_EN);
    /* 2.1 IPv4 checksum update enable */
    SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

    /* 3. Polling relinguish */
    while (1) {
        if (MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {
            MHAL_NOE_DBG_INFO("Polling HW LRO RELINGUISH...\n");
        }
        else
            break;
    }

    /* 4. Enable LRO */
    reg_val = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0);
    reg_val |= PDMA_LRO_EN;
    MHal_NOE_Write_Reg(ADMA_LRO_CTRL_DW0, reg_val);
}


void MHal_NOE_LRO_Set_Ip(unsigned int lan_ip)
{
    /* Set IP: LAN IP */
    MHal_NOE_Write_Reg(LRO_RX_RING0_DIP_DW0, lan_ip);
    MHal_NOE_Write_Reg(LRO_RX_RING0_DIP_DW1, 0);
    MHal_NOE_Write_Reg(LRO_RX_RING0_DIP_DW2, 0);
    MHal_NOE_Write_Reg(LRO_RX_RING0_DIP_DW3, 0);
    SET_PDMA_RXRING_MYIP_VALID(E_NOE_RING_NO0, 1);
}


void MHal_NOE_LRO_Set_Auto_Learn_Cfg(void)
{
    unsigned int reg_val = 0;

    /* Set RX ring1~3 to auto-learn modes */
    SET_PDMA_RXRING_MODE(E_NOE_RING_NO1, PDMA_RX_AUTO_LEARN);
    SET_PDMA_RXRING_MODE(E_NOE_RING_NO2, PDMA_RX_AUTO_LEARN);
    SET_PDMA_RXRING_MODE(E_NOE_RING_NO3, PDMA_RX_AUTO_LEARN);

    /* Valid LRO ring */
    SET_PDMA_RXRING_VALID(E_NOE_RING_NO0, 1);
    SET_PDMA_RXRING_VALID(E_NOE_RING_NO1, 1);
    SET_PDMA_RXRING_VALID(E_NOE_RING_NO2, 1);
    SET_PDMA_RXRING_VALID(E_NOE_RING_NO3, 1);

    /* Set AGE timer */
    SET_PDMA_RXRING_AGE_TIME(E_NOE_RING_NO1, HW_LRO_AGE_TIME);
    SET_PDMA_RXRING_AGE_TIME(E_NOE_RING_NO2, HW_LRO_AGE_TIME);
    SET_PDMA_RXRING_AGE_TIME(E_NOE_RING_NO3, HW_LRO_AGE_TIME);

    /* Set max AGG timer */
    SET_PDMA_RXRING_AGG_TIME(E_NOE_RING_NO1, HW_LRO_AGG_TIME);
    SET_PDMA_RXRING_AGG_TIME(E_NOE_RING_NO2, HW_LRO_AGG_TIME);
    SET_PDMA_RXRING_AGG_TIME(E_NOE_RING_NO3, HW_LRO_AGG_TIME);

    /* Set max LRO agg count */
    SET_PDMA_RXRING_MAX_AGG_CNT(E_NOE_RING_NO1, HW_LRO_MAX_AGG_CNT);
    SET_PDMA_RXRING_MAX_AGG_CNT(E_NOE_RING_NO2, HW_LRO_MAX_AGG_CNT);
    SET_PDMA_RXRING_MAX_AGG_CNT(E_NOE_RING_NO3, HW_LRO_MAX_AGG_CNT);

    /* IPv6 LRO enable */
    SET_PDMA_LRO_IPV6_EN(1);

    /* IPv4 checksum update enable */
    SET_PDMA_LRO_IPV4_CSUM_UPDATE_EN(1);

    /* TCP push option check disable */
    /* SET_PDMA_LRO_IPV4_CTRL_PUSH_EN(0); */
    /* PDMA prefetch enable setting */
    SET_PDMA_LRO_RXD_PREFETCH_EN(ADMA_RXD_PREFETCH_EN | ADMA_MULTI_RXD_PREFETCH_EN);
    /* switch priority comparison to packet count mode */
    SET_PDMA_LRO_ALT_SCORE_MODE(PDMA_LRO_ALT_PKT_CNT_MODE);

    /* bandwidth threshold setting */
    SET_PDMA_LRO_BW_THRESHOLD(HW_LRO_BW_THRE);

    /* auto-learn score delta setting */
    MHal_NOE_Write_Reg(LRO_ALT_SCORE_DELTA, HW_LRO_REPLACE_DELTA);

    /* Set ALT timer to 20us: (unit: 20us) */
    SET_PDMA_LRO_ALT_REFRESH_TIMER_UNIT(HW_LRO_TIMER_UNIT);
    /* Set ALT refresh timer to 1 sec. (unit: 20us) */
    SET_PDMA_LRO_ALT_REFRESH_TIMER(HW_LRO_REFRESH_TIME);

    /* the least remaining room of SDL0 in RXD for lro aggregation */
    SET_PDMA_LRO_MIN_RXD_SDL(HW_LRO_SDL_REMAIN_ROOM);

    /* Polling relinguish */
    while (1) {
        if (MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0) & PDMA_LRO_RELINGUISH) {
            MHAL_NOE_DBG_INFO("Polling HW LRO RELINGUISH...\n");
        }
        else
            break;
    }

    /* Enable HW LRO */
    reg_val = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0);
    reg_val |= PDMA_LRO_EN;
    MHal_NOE_Write_Reg(ADMA_LRO_CTRL_DW0, reg_val);

}




void MHal_NOE_LRO_Update_Calc_Idx(MS_U32 ring_no, MS_U32 dma_owner_idx)
{
    MHal_NOE_Write_Reg(CONFIG_NOE_REG_RX_CALC_IDX0 + (ring_no << 4) , dma_owner_idx);
}


void MHal_NOE_LRO_Reset_Rx_Ring(EN_NOE_RING rx_ring_no, u32 phy_addr, u32 desc_num, u32 desc_idx)
{
    if (rx_ring_no == E_NOE_RING_NO3) {
        MHal_NOE_Write_Reg(RX_BASE_PTR3, phy_addr);
        MHal_NOE_Write_Reg(RX_MAX_CNT3, desc_num);
        MHal_NOE_Write_Reg(RX_CALC_IDX3, desc_idx);
        MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX3);
    }
    else if (rx_ring_no == E_NOE_RING_NO2) {
        MHal_NOE_Write_Reg(RX_BASE_PTR2, phy_addr);
        MHal_NOE_Write_Reg(RX_MAX_CNT2, desc_num);
        MHal_NOE_Write_Reg(RX_CALC_IDX2, desc_idx);
        MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX2);
    }
    else if (rx_ring_no == E_NOE_RING_NO1) {
        MHal_NOE_Write_Reg(RX_BASE_PTR1, phy_addr);
        MHal_NOE_Write_Reg(RX_MAX_CNT1, desc_num);
        MHal_NOE_Write_Reg(RX_CALC_IDX1, desc_idx);
        MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX1);
    }
}


EN_NOE_RET MHal_NOE_LRO_Get_Ring_Info(EN_NOE_DMA dma, EN_NOE_DIR dir, EN_NOE_RING ring_no, struct noe_dma_info * info)
{
    EN_NOE_RET ret = E_NOE_RET_FALSE;
    if (E_NOE_DIR_TX == dir) {
        if (dma & E_NOE_DMA_PACKET) {
            if (ring_no == E_NOE_RING_NO0) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(TX_BASE_PTR0);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(TX_MAX_CNT0);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(TX_CTX_IDX0);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(TX_DTX_IDX0);
            }
            else if (ring_no == E_NOE_RING_NO1) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(TX_BASE_PTR1);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(TX_MAX_CNT1);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(TX_CTX_IDX1);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(TX_DTX_IDX1);
            }
            else if (ring_no == E_NOE_RING_NO2) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(TX_BASE_PTR2);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(TX_MAX_CNT2);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(TX_CTX_IDX2);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(TX_DTX_IDX2);
            }
            else if (ring_no == E_NOE_RING_NO3) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(TX_BASE_PTR3);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(TX_MAX_CNT3);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(TX_CTX_IDX3);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(TX_DTX_IDX3);
            }
        }
        else if (dma & E_NOE_DMA_QUEUE) {
            ret = E_NOE_RET_TRUE;
            info->adr_st.ctx_adr = MHal_NOE_Read_Reg(QTX_CTX_PTR);
            info->adr_st.dtx_adr = MHal_NOE_Read_Reg(QTX_DTX_PTR);
            info->adr_st.crx_adr = MHal_NOE_Read_Reg(QTX_CRX_PTR);
            info->adr_st.drx_adr = MHal_NOE_Read_Reg(QTX_DRX_PTR);

        }
    }
    else if (E_NOE_DIR_RX == dir){
        if (dma & E_NOE_DMA_PACKET) {
            if (ring_no == E_NOE_RING_NO0) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(RX_BASE_PTR0);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(RX_MAX_CNT0);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(RX_CALC_IDX0);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(RX_DRX_IDX0);
            }
            else if (ring_no == E_NOE_RING_NO1) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(RX_BASE_PTR1);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(RX_MAX_CNT1);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(RX_CALC_IDX1);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(RX_DRX_IDX1);
            }
            else if (ring_no == E_NOE_RING_NO2) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(RX_BASE_PTR2);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(RX_MAX_CNT2);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(RX_CALC_IDX2);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(RX_DRX_IDX2);
            }
            else if (ring_no == E_NOE_RING_NO3) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(RX_BASE_PTR3);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(RX_MAX_CNT3);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(RX_CALC_IDX3);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(RX_DRX_IDX3);
            }
        }
        else if (dma & E_NOE_DMA_QUEUE) {
             if (ring_no == E_NOE_RING_NO0) {
                ret = E_NOE_RET_TRUE;
                info->ring_st.base_adr = MHal_NOE_Read_Reg(QRX_BASE_PTR_0);
                info->ring_st.max_cnt = MHal_NOE_Read_Reg(QRX_MAX_CNT_0);
                info->ring_st.cpu_idx = MHal_NOE_Read_Reg(QRX_CRX_IDX_0);
                info->ring_st.dma_idx = MHal_NOE_Read_Reg(QRX_DRX_IDX_0);
            }
        }
    }

    return ret;
}



void MHal_NOE_LRO_Get_Calc_Idx(struct noe_lro_calc_idx *calc_idx)
{
    calc_idx->ring1 = MHal_NOE_Read_Reg(RX_CALC_IDX1);
    calc_idx->ring2 = MHal_NOE_Read_Reg(RX_CALC_IDX2);
    calc_idx->ring3 = MHal_NOE_Read_Reg(RX_CALC_IDX3);
}

void MHal_NOE_Dump_Mii_Mgr(int port_no, int from, int to, int is_local, int page_no)
{
/// do nothing
}



void MHal_NOE_Init_Sep_Intr(EN_NOE_DMA dma, EN_NOE_DIR dir)
{
    if (E_NOE_DIR_TX == dir) {
        if (E_NOE_DMA_PACKET == dma) {
            noe_config.intr_info.fe_tx_int_status = (void __iomem *)CONFIG_NOE_REG_FE_INT_STATUS;
            noe_config.intr_info.fe_tx_int_enable = (void __iomem *)CONFIG_NOE_REG_FE_INT_ENABLE;
        }
        else if (E_NOE_DMA_QUEUE & dma) {
            noe_config.intr_info.fe_tx_int_status = (void __iomem *)QFE_INT_STATUS;
            noe_config.intr_info.fe_tx_int_enable = (void __iomem *)QFE_INT_ENABLE;
        }
    }
    else if (E_NOE_DIR_RX == dir) {
        if (E_NOE_DMA_PACKET == dma) {
            noe_config.intr_info.fe_rx_int_status = (void __iomem *)CONFIG_NOE_REG_FE_INT_STATUS;
            noe_config.intr_info.fe_rx_int_enable = (void __iomem *)CONFIG_NOE_REG_FE_INT_ENABLE;
        }
        else if (E_NOE_DMA_QUEUE & dma) {
            noe_config.intr_info.fe_rx_int_status = (void __iomem *)QFE_INT_STATUS;
            noe_config.intr_info.fe_rx_int_enable = (void __iomem *)QFE_INT_ENABLE;
        }
    }
}



MS_U8 MHal_NOE_Get_Sep_Intr_Status(EN_NOE_DIR dir)
{
    unsigned int reg_int_val;
    if (E_NOE_DIR_TX == dir) {
        reg_int_val = MHal_NOE_Read_Reg(noe_config.intr_info.fe_tx_int_status);
        if (reg_int_val & TX_INT_ALL) {
            return NOE_TRUE;
        }
    }
    else if (E_NOE_DIR_RX == dir) {
        reg_int_val = MHal_NOE_Read_Reg(noe_config.intr_info.fe_rx_int_status);
        if (reg_int_val & RX_INT_ALL) {
            return NOE_TRUE;
        }
    }

    return NOE_FALSE;
}

void MHal_NOE_Clear_Sep_Intr_Status(EN_NOE_DIR dir)
{
    if (E_NOE_DIR_TX == dir) {
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_tx_int_status, TX_INT_ALL);
    }
    else if (E_NOE_DIR_RX == dir) {
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_rx_int_status, RX_INT_ALL);
    }
}

void MHal_NOE_Clear_Sep_Intr_Specific_Status(EN_NOE_DIR dir, EN_NOE_INTR_CLR_STATUS status)
{
    MS_U32 reg_int_val;
    if (E_NOE_DIR_TX == dir) {
        reg_int_val = MHal_NOE_Read_Reg(noe_config.intr_info.fe_tx_int_status);
        if(status == E_NOE_INTR_CLR_EXCEPT_RX) {
            reg_int_val &= ~(RX_INT_ALL);
        }
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_tx_int_status, reg_int_val);
    }
    else if (E_NOE_DIR_RX == dir) {
        reg_int_val = MHal_NOE_Read_Reg(noe_config.intr_info.fe_rx_int_status);
        if(status == E_NOE_INTR_CLR_EXCEPT_TX) {
            reg_int_val &= ~(TX_INT_ALL);
        }
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_rx_int_status, reg_int_val);
    }
}

void MHal_NOE_Enable_Sep_Intr(EN_NOE_DIR dir)
{
    unsigned int reg_int_mask;

    if (E_NOE_DIR_TX == dir) {
        reg_int_mask = MHal_NOE_Read_Reg(noe_config.intr_info.fe_tx_int_enable);
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_tx_int_enable, reg_int_mask | noe_config.intr_info.tx_mask);
    }
    else if (E_NOE_DIR_RX == dir) {
        reg_int_mask = MHal_NOE_Read_Reg(noe_config.intr_info.fe_rx_int_enable);
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_rx_int_enable, reg_int_mask | noe_config.intr_info.rx_mask);
    }
}


void MHal_NOE_Disable_Sep_Intr(EN_NOE_DIR dir)
{
    unsigned int reg_int_mask;
    if (E_NOE_DIR_TX == dir) {
        /* Disable TX interrupt */
        reg_int_mask = MHal_NOE_Read_Reg(noe_config.intr_info.fe_tx_int_enable);
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_tx_int_enable, reg_int_mask & ~(TX_INT_ALL));
    }
    else if (E_NOE_DIR_RX == dir) {
        /* Disable RX interrupt */
        reg_int_mask = MHal_NOE_Read_Reg(noe_config.intr_info.fe_rx_int_enable);
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_rx_int_enable, reg_int_mask & ~(RX_INT_ALL));
    }
    else if (E_NOE_DIR_BOTH == dir) {
        /* Disable TX interrupt */
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_tx_int_enable, 0);
        /* Disable RX interrupt */
        MHal_NOE_Write_Reg(noe_config.intr_info.fe_rx_int_enable, 0);
    }
}


void MHal_NOE_Enable_Sep_Delay_Intr(EN_NOE_DMA dma, EN_NOE_DELAY delay_intr)
{
    if (E_NOE_DMA_PACKET & dma) {
        if(delay_intr == E_NOE_DLY_ENABLE) {
            MHal_NOE_Write_Reg(CONFIG_NOE_REG_DLY_INT_CFG, DELAY_INT_INIT);
            MHal_NOE_Write_Reg(CONFIG_NOE_REG_FE_INT_ENABLE, FE_INT_DLY_INIT);
        }
        else{
            MHal_NOE_Write_Reg(CONFIG_NOE_REG_FE_INT_ENABLE, FE_INT_ALL);
        }
    }
    else if (E_NOE_DMA_QUEUE & dma){
        if(delay_intr == E_NOE_DLY_ENABLE) {
            MHal_NOE_Write_Reg(QDMA_DELAY_INT, DELAY_INT_INIT);
            MHal_NOE_Write_Reg(QFE_INT_ENABLE, QFE_INT_DLY_INIT);
            MHAL_NOE_DBG_INFO("[%s][%d] QFE_INT_ENABLE = %0X\n",__FUNCTION__,__LINE__, MHal_NOE_Read_Reg(QFE_INT_ENABLE));
        }
        else {
            MHal_NOE_Write_Reg(QFE_INT_ENABLE, QFE_INT_ALL);
            MHAL_NOE_DBG_INFO("[%s][%d] QFE_INT_ENABLE = %0X\n",__FUNCTION__,__LINE__, MHal_NOE_Read_Reg(QFE_INT_ENABLE));
        }
    }
}


void MHal_NOE_Set_Grp_Intr(unsigned char delay_intr, unsigned char rx_only)
{
    if (delay_intr == NOE_TRUE) {
        /* PDMA setting */
        MHal_NOE_Write_Reg(PDMA_INT_GRP1, TX_DLY_INT);
        MHal_NOE_Write_Reg(PDMA_INT_GRP2, RX_DLY_INT);
        /* QDMA setting */
        MHal_NOE_Write_Reg(QDMA_INT_GRP1, RLS_DLY_INT);
        MHal_NOE_Write_Reg(QDMA_INT_GRP2, RX_DLY_INT);
    }
    else {
        /* PDMA setting */
        MHal_NOE_Write_Reg(PDMA_INT_GRP1, TX_DONE_INT0);
        MHal_NOE_Write_Reg(PDMA_INT_GRP2, RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3);

        if (rx_only) {
            /* QDMA setting */
            MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x220, RLS_DLY_INT);
            MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x224, RX_DLY_INT);
        }
        else {
            /* QDMA setting */
            MHal_NOE_Write_Reg(QFE_INT_ENABLE, QFE_INT_ALL);
            MHal_NOE_Write_Reg(QDMA_INT_GRP1, RLS_DONE_INT);
            MHal_NOE_Write_Reg(QDMA_INT_GRP2, RX_DONE_INT0 | RX_DONE_INT1);
        }
    }

    MHal_NOE_Write_Reg(FE_INT_GRP, 0x21021001);
    /* FE[0] */
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x1D << 1) |= 0x00A0;
    /* FE[1] */
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x33 << 1) |= 0x0060;
}



void MHal_NOE_Set_Vlan_Info(void)
{
    /*VLAN_IDX 0 = VLAN_ID 0
     * .........
     * VLAN_IDX 15 = VLAN ID 15
     *
     */
    /* frame engine will push VLAN tag
     * regarding to VIDX feild in Tx desc.
     */
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xa8, 0x00010000);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xac, 0x00030002);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xb0, 0x00050004);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xb4, 0x00070006);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xb8, 0x00090008);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xbc, 0x000b000a);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xc0, 0x000d000c);
    MHal_NOE_Write_Reg(NOE_REG_FRAME_ENGINE_BASE + 0xc4, 0x000f000e);
}


void MHal_NOE_Offoad_Checksum(EN_NOE_GE_MAC ge, unsigned char offload)
{
    unsigned int reg_val, reg_csg;
    unsigned int reg_val2 = 0;

    reg_csg = MHal_NOE_Read_Reg(CDMA_CSG_CFG);
    reg_val = MHal_NOE_Read_Reg(GDMA1_FWD_CFG);
    reg_val2 = MHal_NOE_Read_Reg(GDMA2_FWD_CFG);
    if(ge == E_NOE_GE_MAC1) {
        /* set unicast/multicast/broadcast frame to cpu */
        reg_val &= ~0xFFFF;
        reg_val |= GDMA1_FWD_PORT;
        reg_csg &= ~0x7;
        if (offload == NOE_ENABLE) {
            /* enable ipv4 header checksum check */
            reg_val |= GDM1_ICS_EN;
            reg_csg |= ICS_GEN_EN;

            /* enable tcp checksum check */
            reg_val |= GDM1_TCS_EN;
            reg_csg |= TCS_GEN_EN;

            /* enable udp checksum check */
            reg_val |= GDM1_UCS_EN;
            reg_csg |= UCS_GEN_EN;
        }
        else {
            /* disable ipv4 header checksum check */
            reg_val &= ~GDM1_ICS_EN;
            reg_csg &= ~ICS_GEN_EN;

            /* disable tcp checksum check */
            reg_val &= ~GDM1_TCS_EN;
            reg_csg &= ~TCS_GEN_EN;

            /* disable udp checksum check */
            reg_val &= ~GDM1_UCS_EN;
            reg_csg &= ~UCS_GEN_EN;
        }
        MHal_NOE_Write_Reg(GDMA1_FWD_CFG, reg_val);
        MHal_NOE_Write_Reg(CDMA_CSG_CFG, reg_csg);
    }
    else if(ge == E_NOE_GE_MAC2) {

        if (offload == NOE_ENABLE) {
            reg_val2 &= ~0xFFFF;
            reg_val2 |= GDMA2_FWD_PORT;
            reg_val2 |= GDM1_ICS_EN;
            reg_val2 |= GDM1_TCS_EN;
            reg_val2 |= GDM1_UCS_EN;
        }
        else {
            reg_val2 &= ~GDM1_ICS_EN;
            reg_val2 &= ~GDM1_TCS_EN;
            reg_val2 &= ~GDM1_UCS_EN;
        }
        MHal_NOE_Write_Reg(GDMA2_FWD_CFG, reg_val2);
    }
}


void MHal_NOE_GLO_Reset(void)
{
    /*FE_RST_GLO register definition -
     *Bit 0: PSE Rest
     *Reset PSE after re-programming PSE_FQ_CFG.
     */
    MS_U32 reg_val = 0x1;
    MHal_NOE_Write_Reg(FE_RST_GL, reg_val);
    MHal_NOE_Write_Reg(FE_RST_GL, 0);   /* update for RSTCTL issue */

    MHAL_NOE_DBG_INFO("CDMA_CSG_CFG = %0X\n", MHal_NOE_Read_Reg(CDMA_CSG_CFG));
    MHAL_NOE_DBG_INFO("GDMA1_FWD_CFG = %0X\n", MHal_NOE_Read_Reg(GDMA1_FWD_CFG));
    MHAL_NOE_DBG_INFO("GDMA2_FWD_CFG = %0X\n", MHal_NOE_Read_Reg(GDMA2_FWD_CFG));
}


void MHal_NOE_LRO_Set_Prefetch(void)
{
    /* enable RXD prefetch of ADMA */
    unsigned int reg_val = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0);
    reg_val &= ~PDMA_LRO_RXD_PREFETCH_EN;
    reg_val |= (ADMA_RXD_PREFETCH_EN | ADMA_MULTI_RXD_PREFETCH_EN);
    MHal_NOE_Write_Reg(ADMA_LRO_CTRL_DW0, reg_val);
    MHAL_NOE_DBG_INFO("[%s][%d]ADMA_LRO_CTRL_DW0=0x%X\n",__FUNCTION__,__LINE__,MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0));
}

EN_NOE_RET MHal_NOE_Dma_Is_Idle(EN_NOE_DMA dma)
{
    unsigned int reg_val, loop_cnt = 0;
    void __iomem *phy_adr = PDMA_GLO_CFG;

    MHAL_NOE_DBG_INFO("[%s][%d]dma = 0x%x \n",__FUNCTION__,__LINE__,dma);
    if (E_NOE_DMA_PACKET & dma) {
        phy_adr = PDMA_GLO_CFG;
    }
    else if (E_NOE_DMA_QUEUE & dma) {
        phy_adr = QDMA_GLO_CFG;
    }
    else {
        return E_NOE_RET_FALSE;
    }
    while (1) {
        if (loop_cnt++ > NOE_WAIT_IDLE_THRESHOLD)
            break;
        reg_val = MHal_NOE_Read_Reg(phy_adr);
        if ((reg_val & RX_DMA_BUSY)) {
            MHAL_NOE_DBG_INFO("\n  RX_DMA_BUSY !!! ");
            continue;
        }
        if ((reg_val & TX_DMA_BUSY)) {
            MHAL_NOE_DBG_INFO("\n  TX_DMA_BUSY !!! ");
            continue;
        }
        return E_NOE_RET_TRUE;
    }
    return E_NOE_RET_FALSE;
}

void MHal_NOE_Dma_Init_Global_Config(EN_NOE_DMA dma)
{
    unsigned int reg_val;
    if (E_NOE_DMA_PACKET == dma) {
        reg_val = (TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN | PDMA_BT_SIZE_16DWORDS | MULTI_EN | ADMA_RX_BT_SIZE_32DWORDS);
        reg_val |= (RX_2B_OFFSET);
        MHal_NOE_Write_Reg(PDMA_GLO_CFG, reg_val);
        MHAL_NOE_DBG_INFO("[%s][%d]PDMA_GLO_CFG=0x%x\n",__FUNCTION__,__LINE__,MHal_NOE_Read_Reg(PDMA_GLO_CFG));
    }
    else if (E_NOE_DMA_QUEUE & dma) {
        reg_val = MHal_NOE_Read_Reg(QDMA_GLO_CFG);
        reg_val &= 0x000000FF;

        MHal_NOE_Write_Reg(QDMA_GLO_CFG, reg_val);
        reg_val = MHal_NOE_Read_Reg(QDMA_GLO_CFG);

        /* Enable randon early drop and set drop threshold automatically */
        if (E_NOE_DMA_QUEUE_WITH_SFQ != dma)
            MHal_NOE_Write_Reg(QDMA_FC_THRES, 0x4444);

        MHal_NOE_Write_Reg(QDMA_HRED2, 0x0);

        reg_val = (TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN | PDMA_BT_SIZE_16DWORDS);
        reg_val |= (RX_2B_OFFSET);
        MHal_NOE_Write_Reg(QDMA_GLO_CFG, reg_val);

        MHAL_NOE_DBG_INFO("Enable QDMA TX NDP coherence check and re-read mechanism\n");
        reg_val = MHal_NOE_Read_Reg(QDMA_GLO_CFG);
        reg_val = reg_val | 0x400 | 0x100000;
        MHal_NOE_Write_Reg(QDMA_GLO_CFG, reg_val);
        MHAL_NOE_DBG_INFO("***********QDMA_GLO_CFG=0x%x\n", MHal_NOE_Read_Reg(QDMA_GLO_CFG));
    }
}

void MHal_NOE_DMA_Init(EN_NOE_DMA dma, EN_NOE_DIR dir, EN_NOE_RING ring_no, struct noe_dma_info *dma_info)
{
    int i = 0;
    int page, queue;

    MHAL_NOE_DBG_INFO("dma,dir  = 0x%0X,0x%0X  \n",dma,dir);
    if ((dma & E_NOE_DMA_QUEUE) && (dir == E_NOE_DIR_TX)) {
        MHAL_NOE_DBG_INFO("ctx = 0x%0X \n",(MS_U32) dma_info->adr_st.ctx_adr);
        MHAL_NOE_DBG_INFO("dtx = 0x%0X \n",(MS_U32) dma_info->adr_st.dtx_adr);
        MHAL_NOE_DBG_INFO("crx = 0x%0X \n",(MS_U32) dma_info->adr_st.crx_adr);
        MHAL_NOE_DBG_INFO("drx = 0x%0X \n",(MS_U32) dma_info->adr_st.drx_adr);
    }
    else {
        MHAL_NOE_DBG_INFO("base_adr = 0x%0X\n",(MS_U32)dma_info->ring_st.base_adr);
        MHAL_NOE_DBG_INFO("max_cnt  = 0x%0X\n",dma_info->ring_st.max_cnt);
        MHAL_NOE_DBG_INFO("cpu_idx  = 0x%0X \n\n",dma_info->ring_st.cpu_idx);
    }


    if(dma & E_NOE_DMA_PACKET) {
        if (dir == E_NOE_DIR_RX) {
            if (ring_no == E_NOE_RING_NO0) {
                /* Tell the adapter where the RX rings are located. */
                MHal_NOE_Write_Reg(RX_BASE_PTR0, dma_info->ring_st.base_adr);
                MHal_NOE_Write_Reg(RX_MAX_CNT0, dma_info->ring_st.max_cnt);
                MHal_NOE_Write_Reg(RX_CALC_IDX0, dma_info->ring_st.cpu_idx);
                MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX0);
            }
            else if (ring_no == E_NOE_RING_NO3) {
                MHal_NOE_Write_Reg(RX_BASE_PTR3, dma_info->ring_st.base_adr);
                MHal_NOE_Write_Reg(RX_MAX_CNT3, dma_info->ring_st.max_cnt);
                MHal_NOE_Write_Reg(RX_CALC_IDX3, dma_info->ring_st.cpu_idx);
                MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX3);
            }
            else if (ring_no == E_NOE_RING_NO2) {
                MHal_NOE_Write_Reg(RX_BASE_PTR2, dma_info->ring_st.base_adr);
                MHal_NOE_Write_Reg(RX_MAX_CNT2, dma_info->ring_st.max_cnt);
                MHal_NOE_Write_Reg(RX_CALC_IDX2, dma_info->ring_st.cpu_idx);
                MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX2);
            }
            else if (ring_no == E_NOE_RING_NO1) {
                MHal_NOE_Write_Reg(RX_BASE_PTR1, dma_info->ring_st.base_adr);
                MHal_NOE_Write_Reg(RX_MAX_CNT1, dma_info->ring_st.max_cnt);
                MHal_NOE_Write_Reg(RX_CALC_IDX1, dma_info->ring_st.cpu_idx);
                MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DRX_IDX1);
            }
        }
        else if (dir == E_NOE_DIR_TX) {
            /* Tell the adapter where the TX rings are located. */
            MHal_NOE_Write_Reg(TX_BASE_PTR0, dma_info->ring_st.base_adr);
            MHal_NOE_Write_Reg(TX_MAX_CNT0, dma_info->ring_st.max_cnt);
            MHal_NOE_Write_Reg(TX_CTX_IDX0, dma_info->ring_st.cpu_idx);
            MHal_NOE_Write_Reg(PDMA_RST_CFG, PST_DTX_IDX0);
        }
    }
    else if(dma & E_NOE_DMA_QUEUE) {
        if (dir == E_NOE_DIR_RX) {
            /* Tell the adapter where the RX rings are located. */
            MHal_NOE_Write_Reg(QRX_BASE_PTR_0, dma_info->ring_st.base_adr);
            MHal_NOE_Write_Reg(QRX_MAX_CNT_0, dma_info->ring_st.max_cnt);
            MHal_NOE_Write_Reg(QRX_CRX_IDX_0, dma_info->ring_st.cpu_idx);
            MHal_NOE_Write_Reg(QDMA_RST_CFG, PST_DRX_IDX0);
        }
        else if (dir == E_NOE_DIR_TX) {
            MHal_NOE_Write_Reg(QTX_CTX_PTR, dma_info->adr_st.ctx_adr);
            MHal_NOE_Write_Reg(QTX_DTX_PTR, dma_info->adr_st.dtx_adr);
            MHal_NOE_Write_Reg(QTX_CRX_PTR, dma_info->adr_st.crx_adr);
            MHal_NOE_Write_Reg(QTX_DRX_PTR, dma_info->adr_st.drx_adr);
            MHal_NOE_Write_Reg(QTX_SCH_1, 0x80000000);
            for (i = 0; i < NUM_PQ; i++) {
                page = i / QUEUE_OFFSET;
                queue = i & (QUEUE_OFFSET - 1);
                MHal_NOE_Write_Reg(QDMA_PAGE, page);
                MHal_NOE_Write_Reg(QTX_CFG_0 + QUEUE_OFFSET * queue, (NUM_PQ_RESV | (NUM_PQ_RESV << 8)));
            }
            MHal_NOE_Write_Reg(QDMA_PAGE, 0);
        }
    }
}



void MHal_NOE_Get_Intr_Info(struct noe_intr_info *info)
{
    info->fe_intr_enable = MHal_NOE_Read_Reg(FE_INT_ENABLE);
    info->fe_intr_mask = MHal_NOE_Read_Reg(INT_MASK);
    info->fe_intr_status = MHal_NOE_Read_Reg(FE_INT_STATUS);
    info->delay_intr_cfg = MHal_NOE_Read_Reg(DLY_INT_CFG);
    info->qfe_intr_enable = MHal_NOE_Read_Reg(QFE_INT_ENABLE);
    info->qfe_intr_mask = MHal_NOE_Read_Reg(QDMA_INT_MASK);
    info->qfe_intr_status = MHal_NOE_Read_Reg(QFE_INT_STATUS);
}



void MHal_NOE_Get_Mac_Info(EN_NOE_GE_MAC ge, struct noe_mac_info *info)
{
    void __iomem *reg_base;

    if(ge == E_NOE_GE_MAC1) {
        reg_base = NOE_REG_FRAME_ENGINE_BASE + 0x2400;
        info->stat.control = MHal_NOE_Read_Reg(NOE_MAC_P1_MCR);
        info->stat.status = MHal_NOE_Read_Reg(NOE_MAC_P1_SR);
    }
    else if (ge == E_NOE_GE_MAC2) {
        reg_base = NOE_REG_FRAME_ENGINE_BASE + 0x2440;
        info->stat.control = MHal_NOE_Read_Reg(NOE_MAC_P2_MCR);
        info->stat.status = MHal_NOE_Read_Reg(NOE_MAC_P2_SR);
    }
    else
        return;

    info->rx.good_cnt = MHal_NOE_Read_Reg(reg_base);
    info->rx.good_pkt = MHal_NOE_Read_Reg(reg_base + 0x08);
    info->rx.overflow_err = MHal_NOE_Read_Reg(reg_base + 0x10);
    info->rx.fcs_err = MHal_NOE_Read_Reg(reg_base + 0x14);
    info->rx.ser_cnt = MHal_NOE_Read_Reg(reg_base + 0x18);
    info->rx.ler_pkt = MHal_NOE_Read_Reg(reg_base + 0x1C);
    info->rx.chk_err = MHal_NOE_Read_Reg(reg_base + 0x20);
    info->rx.flow_ctrl = MHal_NOE_Read_Reg(reg_base + 0x24);

    info->tx.skip_cnt = MHal_NOE_Read_Reg(reg_base + 0x28);
    info->tx.collision_cnt = MHal_NOE_Read_Reg(reg_base + 0x2C);
    info->tx.good_cnt = MHal_NOE_Read_Reg(reg_base + 0x30);
    info->tx.good_pkt = MHal_NOE_Read_Reg(reg_base + 0x38);
}

void MHal_NOE_Get_Pse_Info(struct noe_pse_info *info)
{
    info->min_free_cnt = (MHal_NOE_Read_Reg(FE_PSE_FREE) & 0xff0000) >> 16;
    info->free_cnt = MHal_NOE_Read_Reg(FE_PSE_FREE) & 0x00ff;
    info->fq_drop_cnt = MHal_NOE_Read_Reg(FE_DROP_FQ);
    info->fc_drop_cnt = MHal_NOE_Read_Reg(FE_DROP_FC);
    info->ppe_drop_cnt = MHal_NOE_Read_Reg(FE_DROP_PPE);
}

static void _MHal_NOE_PDMA_Get_Dbg(void *dbg_info)
{
    struct noe_pdma_dbg *info = (struct noe_pdma_dbg *)dbg_info;
    info->rx[0] = MHal_NOE_Read_Reg(INT_MASK + 0x10);
    info->rx[1] = MHal_NOE_Read_Reg(INT_MASK + 0x14);
    info->tx[0] = MHal_NOE_Read_Reg(INT_MASK + 0x8);
    info->tx[1] = MHal_NOE_Read_Reg(INT_MASK + 0xC);
}


static void _MHal_NOE_QDMA_Get_Cnt(void *cnt_info)
{
    struct noe_qdma_cnt *info = (struct noe_qdma_cnt *) cnt_info;
    unsigned int page_no = 0, queue_no = 0;
    if (queue_no >= NUM_PQ)
        return;

    page_no = info->pq_no / QUEUE_OFFSET;
    queue_no = info->pq_no & (QUEUE_OFFSET - 1);

    MHal_NOE_Write_Reg(QTX_MIB_IF, 0x90000000);
    MHal_NOE_Write_Reg(QDMA_PAGE, page_no);
    info->pkt_cnt = MHal_NOE_Read_Reg(QTX_CFG_0 + queue_no * QUEUE_OFFSET);
    info->drop_cnt = MHal_NOE_Read_Reg(QTX_SCH_0 + queue_no * QUEUE_OFFSET);

    MHal_NOE_Write_Reg(QDMA_PAGE, 0);
    MHal_NOE_Write_Reg(QTX_MIB_IF, 0x0);
}

static void _MHal_NOE_QDMA_Get_Fq(void *fq_info)
{
    struct noe_qdma_fq *info = (struct noe_qdma_fq *) fq_info;
    info->sw_fq = (MHal_NOE_Read_Reg(QDMA_FQ_CNT) & 0xFFFF0000) >> 16;
    info->hw_fq = MHal_NOE_Read_Reg(QDMA_FQ_CNT) & 0x0000FFFF;
}

static void _MHal_NOE_QDMA_Get_Sch(void *rate_info)
{
    struct noe_qdma_sch *info = (struct noe_qdma_sch *) rate_info;
    unsigned int temp = MHal_NOE_Read_Reg(QDMA_TX_SCH);
    unsigned int max_rate, i;
    info->sch[0].max_en = (temp & 0x00000800) >> 11;
    max_rate = (temp & 0x000007F0) >> 4;
    for (i = 0; i < (temp & 0x0000000F); i++)
        max_rate *= 10;
    info->sch[0].max_rate = max_rate;

    info->sch[1].max_en = (temp & 0x08000000) >> 27;
    max_rate = (temp & 0x07F00000) >> 20;
    for (i = 0; i < (temp & 0x000F0000); i++)
        max_rate *= 10;
    info->sch[1].max_rate = max_rate;
}

static void _MHal_NOE_QDMA_Get_Fc(void *fc_info)
{
    struct noe_qdma_fc *info = (struct noe_qdma_fc *) fc_info;
    unsigned int temp = MHal_NOE_Read_Reg(QDMA_FC_THRES);
    info->sw.en = (temp & 0x1000000) >> 24;
    info->sw.ffa = (temp & 0x2000000) >> 25;
    info->sw.mode = (temp & 0x30000000) >> 28;
    info->sw.fst_vq_en = (temp & 0x08000000) >> 27;
    info->sw.fst_vq_mode = (temp & 0xC0000000) >> 30;
    info->hw.en = (temp & 0x10000) >> 16;
    info->hw.ffa = (temp & 0x20000) >> 17;
    info->hw.mode = (temp & 0x300000) >> 20;
    info->hw.fst_vq_en = (temp & 0x080000) >> 19;
    info->hw.fst_vq_mode = (temp & 0xC00000) >> 22;
}

static void _MHal_NOE_QDMA_Get_Fsm(void *fsm_info)
{
    struct noe_qdma_fsm *info = (struct noe_qdma_fsm *) fsm_info;
    unsigned int temp = MHal_NOE_Read_Reg(QDMA_DMA);
    info->vqtb = (temp & 0x0F000000) >> 24;
    info->fq = (temp & 0x000F0000) >> 16;
    info->tx = (temp & 0x00000F00) >> 8;
    info->rx = temp & 0x0000001F;
    temp = MHal_NOE_Read_Reg(QDMA_BMU);
    info->rls = (temp & 0x07FF0000) >> 16;
    info->fwd = temp & 0x00007FFF;
}

static void _MHal_NOE_QDMA_Get_Vq(void *vq_info)
{
    struct noe_qdma_vq *info = (struct noe_qdma_vq *) vq_info;
    unsigned int temp = MHal_NOE_Read_Reg(VQTX_NUM);
    info->vq_num[E_NOE_VQ_NO0]=temp & 0xF;
    info->vq_num[E_NOE_VQ_NO1]=(temp & 0xF0) >> 4;
    info->vq_num[E_NOE_VQ_NO2]=(temp & 0xF00) >> 8;
    info->vq_num[E_NOE_VQ_NO3]=(temp & 0xF000) >> 12;
}

static void _MHal_NOE_QDMA_Get_Pq(void *pq_info)
{
    struct noe_qdma_pq *info = (struct noe_qdma_pq *) pq_info;
    unsigned int temp, i, rate, queue, queue_no;
    queue = info->queue;
    queue_no = queue % 16;

    if (queue < 16) {
        MHal_NOE_Write_Reg(QDMA_PAGE, 0);
    }
    else if (queue > 15 && queue <= 31) {
        MHal_NOE_Write_Reg(QDMA_PAGE, 1);
    }
    else if (queue > 31 && queue <= 47) {
        MHal_NOE_Write_Reg(QDMA_PAGE, 2);
    }
    else if (queue > 47 && queue <= 63) {
        MHal_NOE_Write_Reg(QDMA_PAGE, 3);
    }



    temp = MHal_NOE_Read_Reg(QTX_CFG_0 + QUEUE_OFFSET * queue_no);
    info->txd_cnt = (temp & 0xffff0000) >> 16;
    info->hw_resv = (temp & 0xff00) >> 8;
    info->sw_resv = (temp & 0xff);
    temp = MHal_NOE_Read_Reg(QTX_CFG_0 + (QUEUE_OFFSET * queue_no) + 0x4);
    info->sch = (temp >> 31) + 1;
    info->min_en = (temp & 0x8000000) >> 27;
    rate = (temp & 0x7f00000) >> 20;
    for (i = 0; i < (temp & 0xf0000) >> 16; i++)
        rate *= 10;
    info->min_rate = rate;
    info->max_en = (temp & 0x800) >> 11;
    rate = (temp & 0x7f0) >> 4;
    for (i = 0; i < (temp & 0xf); i++)
        rate *= 10;
    info->max_rate = rate;
    info->weight = (temp & 0xf000) >> 12;
    info->queue_head = MHal_NOE_Read_Reg(QTX_HEAD_0 + QUEUE_OFFSET * queue_no);
    info->queue_tail = MHal_NOE_Read_Reg(QTX_TAIL_0 + QUEUE_OFFSET * queue_no);

    MHal_NOE_Write_Reg(QDMA_PAGE, 0);
}

void MHal_NOE_Get_Qdma_Info(EN_NOE_QDMA_INFO_TYPE type, void *info)
{
    if ((info == NULL) || (type >= E_NOE_QDMA_INFO_MAX) || (_mhal_noe_qdma_get_info_pfn[type] == NULL))
        return;
    _mhal_noe_qdma_get_info_pfn[type](info);
}


void MHal_NOE_IO_Enable_Coherence(MS_BOOL qos)
{
    MS_U32 reg_val = MHal_NOE_Read_Reg(REG_NOE_IOC_ETH);

    reg_val |= IOC_ETH_PDMA;
    if (qos) {
        reg_val |= IOC_ETH_QDMA;
    }
    MHal_NOE_Write_Reg(REG_NOE_IOC_ETH, reg_val);
}



void MHal_NOE_Enable_Link_Intr(void)
{
    MS_U32 val;

    if (NOE_IS_INVALID_MUX(noe_config.pin_mux)) {
        MHAL_NOE_WARN_INFO("Invalid mux = %d.\n", noe_config.pin_mux);
        return ;
    }

    val = MHal_NOE_Read_Reg(FE_INT_ENABLE2);
    if (noe_config.pin_mux == E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP)
        val |= (0x1 << 24);
    else if (noe_config.pin_mux == E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP)
        val |= (0x1 << 25);
    else
        val |= (0x3 << 24) ;

    MHal_NOE_Write_Reg(FE_INT_ENABLE2, val);
}

void MHal_NOE_Clear_Link_Intr_Status(void)
{
    u32 val = MHal_NOE_Read_Reg(FE_INT_STATUS2) & (MAC1_LINK | MAC2_LINK);
    MHal_NOE_Write_Reg(FE_INT_STATUS2, val );
}

MS_BOOL MHal_NOE_Get_Link_Intr_Status(void)
{
    int val = 0;
    MS_BOOL link_changed = FALSE;
    val = MHal_NOE_Read_Reg(FE_INT_STATUS2);
    if ((val & MAC2_LINK) || (val & MAC1_LINK)) {
        link_changed = TRUE;
    }
    MHal_NOE_Clear_Link_Intr_Status();
    return link_changed;
}

void MHal_NOE_Disable_Link_Intr(void)
{
    MS_U32 mask = 0;

    if (NOE_IS_INVALID_MUX(noe_config.pin_mux)) {
        MHAL_NOE_WARN_INFO("Invalid mux = %d.\n", noe_config.pin_mux);
        return ;
    }

    mask = MHal_NOE_Read_Reg(FE_INT_ENABLE2);
    if (noe_config.pin_mux == E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP)
        mask &= ~(MAC1_LINK);
    else if (noe_config.pin_mux == E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP)
        mask &= ~(MAC2_LINK);
    else
        mask &= ~(MAC1_LINK | MAC2_LINK) ;

    MHal_NOE_Write_Reg(FE_INT_ENABLE2, mask);

}


void MHal_NOE_DMA_Update_Calc_Idx(EN_NOE_DIR dir, MS_U32 owner_idx)
{
    if (dir == E_NOE_DIR_RX)
        MHal_NOE_Write_Reg(CONFIG_NOE_REG_RX_CALC_IDX0, owner_idx);
    else if (dir == E_NOE_DIR_TX)
        MHal_NOE_Write_Reg(TX_CTX_IDX0, owner_idx);
}

MS_U32 MHal_NOE_DMA_Get_Calc_Idx(EN_NOE_DIR dir)
{
    if (dir == E_NOE_DIR_RX)
        return MHal_NOE_Read_Reg(CONFIG_NOE_REG_RX_CALC_IDX0);
    else if (dir == E_NOE_DIR_TX)
        return MHal_NOE_Read_Reg(TX_CTX_IDX0);
    return NOE_INVALID_CALC_IDX;
}


void MHal_NOE_Get_Intr_Status(EN_NOE_DMA dma, MS_U32 *recv, MS_U32 *xmit)
{
    MS_U32 reg_int_val;
    noe_config.intr_info.reg_int_val_q = 0;
    noe_config.intr_info.reg_int_val_p = MHal_NOE_Read_Reg(CONFIG_NOE_REG_FE_INT_STATUS);
    if (dma & E_NOE_DMA_QUEUE) {
        noe_config.intr_info.reg_int_val_q = MHal_NOE_Read_Reg(QFE_INT_STATUS);
    }
    reg_int_val = noe_config.intr_info.reg_int_val_p | noe_config.intr_info.reg_int_val_q;
    if ((reg_int_val & noe_config.intr_info.rx_mask))
        *recv = 1;
    if (reg_int_val & noe_config.intr_info.tx_mask)
        *xmit = 1;

}

void MHal_NOE_Clear_Intr_Status(EN_NOE_DMA dma)
{
    /* Clear Interrupt */
    if (dma & E_NOE_DMA_QUEUE) {
        MHal_NOE_Write_Reg(QFE_INT_STATUS, noe_config.intr_info.reg_int_val_q);
    }
    /* QWERT */
    MHal_NOE_Write_Reg(CONFIG_NOE_REG_FE_INT_STATUS, noe_config.intr_info.reg_int_val_p);
}


void MHal_NOE_Enable_Intr_Status(EN_NOE_DMA dma)
{
    MS_U32 reg_int_mask;
    reg_int_mask = MHal_NOE_Read_Reg(CONFIG_NOE_REG_FE_INT_ENABLE);

    MHal_NOE_Write_Reg(CONFIG_NOE_REG_FE_INT_ENABLE, reg_int_mask & ~(noe_config.intr_info.rx_mask));

    if (dma & E_NOE_DMA_QUEUE) {
        reg_int_mask = MHal_NOE_Read_Reg(QFE_INT_ENABLE);
        MHal_NOE_Write_Reg(QFE_INT_ENABLE, reg_int_mask & ~(noe_config.intr_info.rx_mask));
    }

}

void MHal_NOE_MAC_Get_Link_Status(EN_NOE_GE_MAC ge, struct noe_mac_link_status *link_status)
{
    u32 data = 0;
    if (NOE_AUTO_POLLING_IS_NOT_SUPPORTED()) {
        if ((ge == E_NOE_GE_MAC1) || (ge == E_NOE_GE_MAC2)) {
            if (noe_config.mii_info[ge].phy_addr != NOE_INVALID_PHY_ADDR) {
                MHal_NOE_Read_Mii_Mgr(noe_config.mii_info[ge].phy_addr, GPHY_REG_STATUS, &data);
                /*
                the register indicates whether the link was lost since the last read.
                for the current link status, read this register twice.
                */
                MHal_NOE_Read_Mii_Mgr(noe_config.mii_info[ge].phy_addr, GPHY_REG_STATUS, &data);
                link_status->link_up = ((data & BIT(2)) == BIT(2))? TRUE: FALSE;
            }
        }
    }
    else {
        if (ge == E_NOE_GE_MAC1) {
            link_status->link_up = (MHal_NOE_Read_Reg(NOE_MAC_P1_SR) & 0x1)?TRUE:FALSE;
        }
        else if (ge == E_NOE_GE_MAC2) {
            link_status->link_up = (MHal_NOE_Read_Reg(NOE_MAC_P2_SR) & 0x1)?TRUE:FALSE;
        }
    }
}

EN_NOE_RET MHal_NOE_Need_Reset(void)
{
    unsigned int val_1, val_2, info;
    unsigned int tmp[4];
    val_1 = MHal_NOE_Read_Reg(NOE_REG_SYSCTL_BASE);
    val_2 = MHal_NOE_Read_Reg(NOE_REG_SYSCTL_BASE + 4);
    tmp[3] = ((val_1 >> 16) & 0xff) - 0x30;
    tmp[2] = ((val_1 >> 24) & 0xff) - 0x30;
    tmp[1] = ((val_2 >> 0) & 0xff) - 0x30;
    tmp[0] = ((val_2 >> 8) & 0xff) - 0x30;
    info = (tmp[3] * 1000) + (tmp[2] * 100) + (tmp[1] * 10) + tmp[0];
    /* reset is not necessary */
    return E_NOE_RET_FALSE;
}

static void _MHal_NOE_FE_Reset(void)
{
    MS_U32 adma_rx_dbg0_r = 0;
    MS_U32 dbg_rx_curr_state, rx_fifo_wcnt;
    MS_U32 dbg_cdm_lro_rinf_afifo_rempty, dbg_cdm_eof_rdy_afifo_empty;
    MS_U32 reg_tmp, loop_count;

    /* do CDM/PDMA reset */
    reg_tmp = MHal_NOE_Read_Reg(FE_GLO_MISC);
    MHal_NOE_Write_Reg(FE_GLO_MISC, reg_tmp | reg_tmp);
    mdelay(10);
    reg_tmp = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW3);
    MHal_NOE_Write_Reg(ADMA_LRO_CTRL_DW3, reg_tmp | (0x1 << 14));
    loop_count = 0;
    do {
        adma_rx_dbg0_r = MHal_NOE_Read_Reg(ADMA_RX_DBG0);
        dbg_rx_curr_state = (adma_rx_dbg0_r >> 16) & 0x7f;
        rx_fifo_wcnt = (adma_rx_dbg0_r >> 8) & 0x3f;
        dbg_cdm_lro_rinf_afifo_rempty = (adma_rx_dbg0_r >> 7) & 0x1;
        dbg_cdm_eof_rdy_afifo_empty = (adma_rx_dbg0_r >> 6) & 0x1;
        loop_count++;
        if (loop_count >= 100) {
            MHAL_NOE_DBG_INFO("[%s] loop_count timeout!!!\n", __func__);
            break;
        }
        mdelay(10);
    } while (((dbg_rx_curr_state != 0x17) && (dbg_rx_curr_state != 0x00)) || (rx_fifo_wcnt != 0) || (!dbg_cdm_lro_rinf_afifo_rempty) || (!dbg_cdm_eof_rdy_afifo_empty));
    reg_tmp = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW3);
    MHal_NOE_Write_Reg(ADMA_LRO_CTRL_DW3, reg_tmp & 0xffffbfff);
    reg_tmp = MHal_NOE_Read_Reg(FE_GLO_MISC);
    MHal_NOE_Write_Reg(FE_GLO_MISC, reg_tmp & 0xfffffffe);

}

void MHal_NOE_Do_Reset(void)
{

    MS_U32 adma_rx_dbg0_r = 0;
    MS_U32 dbg_rx_curr_state, rx_fifo_wcnt;
    MS_U32 dbg_cdm_lro_rinf_afifo_rempty, dbg_cdm_eof_rdy_afifo_empty;

    adma_rx_dbg0_r = MHal_NOE_Read_Reg(ADMA_RX_DBG0);
    dbg_rx_curr_state = (adma_rx_dbg0_r >> 16) & 0x7f;
    rx_fifo_wcnt = (adma_rx_dbg0_r >> 8) & 0x3f;
    dbg_cdm_lro_rinf_afifo_rempty = (adma_rx_dbg0_r >> 7) & 0x1;
    dbg_cdm_eof_rdy_afifo_empty = (adma_rx_dbg0_r >> 6) & 0x1;

    /* check if PSE P0 hang */
    if (dbg_cdm_lro_rinf_afifo_rempty && dbg_cdm_eof_rdy_afifo_empty && (rx_fifo_wcnt & 0x20) && ((dbg_rx_curr_state == 0x17) || (dbg_rx_curr_state == 0x00))) {
        _MHal_NOE_FE_Reset();
    }
}


void MHal_NOE_DMA_SFQ_Init(struct noe_sfq_base *adr_info)
{
    MS_U32 reg_val;
    reg_val = MHal_NOE_Read_Reg(VQTX_GLO);
    reg_val = reg_val | VQTX_MIB_EN;
    /* Virtual table extends to 32bytes */
    MHal_NOE_Write_Reg(VQTX_GLO, reg_val);
    reg_val = MHal_NOE_Read_Reg(VQTX_GLO);
    MHal_NOE_Write_Reg(VQTX_NUM, (VQTX_NUM_0) | (VQTX_NUM_1) | (VQTX_NUM_2) | (VQTX_NUM_3) | (VQTX_NUM_4) | (VQTX_NUM_5) | (VQTX_NUM_6) | (VQTX_NUM_7));

    /* 10 s change hash algorithm */
    MHal_NOE_Write_Reg(VQTX_HASH_CFG, 0xF002710);
    MHal_NOE_Write_Reg(VQTX_VLD_CFG, 0xeca86420);
    MHal_NOE_Write_Reg(VQTX_HASH_SD, 0x0D);
    MHal_NOE_Write_Reg(QDMA_FC_THRES, 0x9b9b4444);
    MHal_NOE_Write_Reg(QDMA_HRED1, 0);
    MHal_NOE_Write_Reg(QDMA_HRED2, 0);
    MHal_NOE_Write_Reg(QDMA_SRED1, 0);
    MHal_NOE_Write_Reg(QDMA_SRED2, 0);
    MHal_NOE_Write_Reg(VQTX_0_3_BIND_QID, (VQTX_0_BIND_QID) | (VQTX_1_BIND_QID) | (VQTX_2_BIND_QID) | (VQTX_3_BIND_QID));
    MHal_NOE_Write_Reg(VQTX_4_7_BIND_QID, (VQTX_4_BIND_QID) | (VQTX_5_BIND_QID) | (VQTX_6_BIND_QID) | (VQTX_7_BIND_QID));
    MHAL_NOE_DBG_INFO("VQTX_0_3_BIND_QID =%x\n", MHal_NOE_Read_Reg(VQTX_0_3_BIND_QID));
    MHAL_NOE_DBG_INFO("VQTX_4_7_BIND_QID =%x\n", MHal_NOE_Read_Reg(VQTX_4_7_BIND_QID));
    MHal_NOE_Write_Reg(VQTX_TB_BASE0, (MS_U32)adr_info->phy_adr[0]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE1, (MS_U32)adr_info->phy_adr[1]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE2, (MS_U32)adr_info->phy_adr[2]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE3, (MS_U32)adr_info->phy_adr[3]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE4, (MS_U32)adr_info->phy_adr[4]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE5, (MS_U32)adr_info->phy_adr[5]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE6, (MS_U32)adr_info->phy_adr[6]);
    MHal_NOE_Write_Reg(VQTX_TB_BASE7, (MS_U32)adr_info->phy_adr[7]);
}

MS_U32 MHal_NOE_DMA_Get_Queue_Cfg(MS_U32 pq_no)
{
    int page_no = 0, queue_no = 0;

    if (noe_config.bQoS_SetMode == TRUE) {
        return NOE_QOS_SET_REG(pq_no);
    }
    else {
        if (pq_no >= NUM_PQ)
            return 0;

        page_no = pq_no / QUEUE_OFFSET;
        queue_no = pq_no & (QUEUE_OFFSET - 1);
        MHal_NOE_Write_Reg(QDMA_PAGE, page_no);
        return MHal_NOE_Read_Reg(QTX_CFG_0 + queue_no * QUEUE_OFFSET);
    }
}

void MHal_NOE_DMA_Set_Queue_Cfg(MS_U32 pq_no, MS_U32 cfg)
{
    int page = 0, queue = 0;

    if (noe_config.bQoS_SetMode == TRUE) {
        NOE_QOS_SET_REG(pq_no) = cfg;
        MHAL_NOE_DBG_INFO(" 0x%x = 0x%x\n", QDMA_RELATED + pq_no, cfg);
    }
    else {
        if (pq_no >= NUM_PQ)
            return ;

        page = pq_no / QUEUE_OFFSET;
        queue = pq_no & (QUEUE_OFFSET - 1);
        MHAL_NOE_DBG_INFO("page: 0x%x, queue = 0x%x\n", page, queue);
        MHal_NOE_Write_Reg(QDMA_PAGE, page);
        MHal_NOE_Write_Reg(QTX_CFG_0 + QUEUE_OFFSET * queue, cfg);
    }
}


void MHal_NOE_DMA_Enable_Specific_Intr(EN_NOE_DMA dma, EN_NOE_INTR_INFO e_intr)
{
    MS_U32 reg_val;
    if (dma & E_NOE_DMA_QUEUE) {
        reg_val = MHal_NOE_Read_Reg(QFE_INT_ENABLE);
        if (e_intr == E_NOE_INTR_INFO_RLS_DLY) {
            MHal_NOE_Write_Reg(QFE_INT_ENABLE, reg_val | RLS_DLY_INT);
        }
        else {
            MHal_NOE_Write_Reg(QFE_INT_ENABLE, reg_val | RLS_DONE_INT);
        }
    }
}


void MHal_NOE_DMA_FQ_Init(struct noe_fq_base *info)
{
    MHal_NOE_Write_Reg(QDMA_FQ_HEAD, (MS_U32) info->head);
    MHal_NOE_Write_Reg(QDMA_FQ_TAIL, (MS_U32) info->tail);
    MHal_NOE_Write_Reg(QDMA_FQ_CNT, ((info->txd_num << 16) | info->page_num));
    MHal_NOE_Write_Reg(QDMA_FQ_BLEN, info->page_size << 16);
}

MS_U32 MHal_NOE_QDMA_Get_Tx(void)
{
    return MHal_NOE_Read_Reg(QTX_DRX_PTR);
}

void MHal_NOE_QDMA_Update_Tx(EN_NOE_QDMA_TX_TYPE type, MS_U32 adr)
{
    if (type == E_NOE_QDMA_TX_FORWARD)
        MHal_NOE_Write_Reg(QTX_CTX_PTR, adr);
    else if (type == E_NOE_QMDA_TX_RELEASE)
        MHal_NOE_Write_Reg(QTX_CRX_PTR, adr);
}

void MHal_NOE_LRO_Control(EN_NOE_LRO_CTRL_TYPE type, MS_U32 param)
{
    if (type == E_NOE_LRO_CTRL_AGG_CNT) {
        SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING1, param);
        SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING2, param);
        SET_PDMA_RXRING_MAX_AGG_CNT(ADMA_RX_RING3, param);
    }
    else if (type == E_NOE_LRO_CTRL_AGG_TIME) {
        SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING1, param);
        SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING2, param);
        SET_PDMA_RXRING_AGG_TIME(ADMA_RX_RING3, param);
    }
    else if (type == E_NOE_LRO_CTRL_AGE_TIME) {
        SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING1, param);
        SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING2, param);
        SET_PDMA_RXRING_AGE_TIME(ADMA_RX_RING3, param);
    }
    else if (type == E_NOE_LRO_CTRL_BW_THRESHOLD) {
        SET_PDMA_LRO_BW_THRESHOLD(param);
    }
    else if (type == E_NOE_LRO_CTRL_SWITCH) {
        if (param == NOE_DISABLE) {
            SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 0);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 0);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 0);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 0);
        }
        else {
            SET_PDMA_RXRING_VALID(ADMA_RX_RING0, 1);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING1, 1);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING2, 1);
            SET_PDMA_RXRING_VALID(ADMA_RX_RING3, 1);
        }
    }
}


void MHal_NOE_Get_Pdma_Info(EN_NOE_PDMA_INFO_TYPE type, void *info)
{
    if ((type >= E_NOE_PDMA_INFO_MAX) || (_mhal_noe_pdma_get_info_pfn[type] == NULL))
        return;
    _mhal_noe_pdma_get_info_pfn[type](info);
}

void _MHal_NOE_Disable_WED_Interrupt(void)
{
    /* only enable FE[0] interrupt for NOE */
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x1D << 1) = 0xFF1F;
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x1E << 1) = 0xFFFF;

    /* only enable FE[1] interrupt for NOE */
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x33 << 1) = 0xFF1F;
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x34 << 1) = 0xFFFF;

    /* nly enable FE[2] interrupt for NOE */
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x27 << 1) |= 0xFFFF;
    NOE_RIU_REG(NOE_RIU_BANK_NOE_MISC, 0x28 << 1) |= 0xFFFF;
}

void MHal_NOE_Init(struct noe_sys *info)
{
    ethdma_sysctl_base = info->sysctl_base;
    _MHal_NOE_Init_Clock();
    _MHal_NOE_Disable_WED_Interrupt();
    /* GMAC1/2 RGMII mode First */
    _MHal_NOE_Set_Interface_Mode(E_NOE_GE_MAC1, E_NOE_INTERFACE_RGMII_MODE);
    _MHal_NOE_Set_Interface_Mode(E_NOE_GE_MAC2, E_NOE_INTERFACE_RGMII_MODE);

    noe_config.bAN_Supported = 1;

}

void MHAL_NOE_Get_Interrupt(struct noe_irq *info)
{
    info->num = 2;
    info->irq[E_NOE_IRQ_0] = INT_IRQ_115_NOE_IRQ0;
    info->irq[E_NOE_IRQ_1] = INT_IRQ_116_NOE_IRQ1;
    info->irq[E_NOE_IRQ_2] = INT_IRQ_117_NOE_IRQ2;
}

void MHAL_NOE_LRO_Get_Control(EN_NOE_RING ring, struct lro_ctrl *info)
{
    MS_U32 reg1, reg2, reg3;

    if ((ring >=E_NOE_RING_MAX) || (info == NULL)) {
        return;
    }

    reg1 = MHal_NOE_Read_Reg(LRO_RX_RING0_CTRL_DW1 + (ring * 0x40));
    reg2 = MHal_NOE_Read_Reg(LRO_RX_RING0_CTRL_DW2 + (ring * 0x40));
    reg3 = MHal_NOE_Read_Reg(LRO_RX_RING0_CTRL_DW3 + (ring * 0x40));


    info->agg_cnt = ((reg3 & 0x03) << PDMA_LRO_AGG_CNT_H_OFFSET) | ((reg2 >> PDMA_LRO_RING_AGG_CNT1_OFFSET) & 0x3f);
    info->agg_time = (reg2 >> PDMA_LRO_RING_AGG_OFFSET) & 0xffff;
    info->age_time = ((reg2 & 0x03f) << PDMA_LRO_AGE_H_OFFSET) | ((reg1 >> PDMA_LRO_RING_AGE1_OFFSET) & 0x3ff);
    info->threshold = MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW2);
}


void MHAL_NOE_LRO_Get_Table(MS_U32 entry, struct lro_tbl *info)
{
    int i = 0;
    MS_U32 reg_val;

    if (entry >= MHAL_NOE_MAX_ENTRIES_IN_LRO_TABLE) {
        info->valid = FALSE;
        return;
    }

    MHal_NOE_Write_Reg(PDMA_FE_ALT_CF8, 0);
    reg_val = MHal_NOE_Read_Reg(PDMA_FE_ALT_SEQ_CFC);

    info->valid = (reg_val & (1 << entry))?TRUE:FALSE;

    if (entry > 4)
        entry = entry - 1;
    entry = (entry * 9) + 1;

    /* read valid entries of the auto-learn table */
    MHal_NOE_Write_Reg(PDMA_FE_ALT_CF8, entry);

    for (i = 0; i < MHAL_NOE_MAX_INFO_FOR_EACH_ENTRY_IN_LRO_TABLE; i++) {
        info->tlb_info[i] = MHal_NOE_Read_Reg(PDMA_FE_ALT_SEQ_CFC);
    }

    if (MHal_NOE_Read_Reg(ADMA_LRO_CTRL_DW0) & PDMA_LRO_ALT_SCORE_MODE)
        info->priority = info->tlb_info[6] >> 20;   /* packet count */
    else
        info->priority = info->tlb_info[7];         /* byte count */
}

void MHal_NOE_Set_Intr_Mask(EN_NOE_DELAY e_dly)
{
    if (e_dly != E_NOE_DLY_DISABLE) {
        noe_config.intr_info.tx_mask = RLS_DLY_INT;
        noe_config.intr_info.rx_mask = RX_DLY_INT;
    }
    else {
        noe_config.intr_info.tx_mask = TX_DONE_INT0;
        noe_config.intr_info.rx_mask = RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3;
    }
}


void MHal_NOE_Set_WoL(MS_BOOL enable)
{
    unsigned int reg_value = 0;

    if (enable) {
        reg_value = MHal_NOE_Read_Reg(MAC1_WOL);
        reg_value |= (WOL_INT_CLR | WOL_INT_EN | WOL_EN);
        MHal_NOE_Write_Reg(MAC1_WOL, reg_value);

    }
    else {
        reg_value = MHal_NOE_Read_Reg(MAC1_WOL);
        reg_value &= ~(WOL_INT_EN | WOL_EN);
        MHal_NOE_Write_Reg(MAC1_WOL, reg_value);
    }

}

void MHal_NOE_Dma_DeInit_Global_Config(EN_NOE_DMA dma)
{
    if (E_NOE_DMA_PACKET == dma) {
        MHal_NOE_Write_Reg(PDMA_GLO_CFG, 0);
    }
    else if (E_NOE_DMA_QUEUE & dma) {
        MHal_NOE_Write_Reg(QDMA_GLO_CFG, 0);
    }

    noe_config.status = E_NOE_HAL_STATUS_SUSPEND;
}
