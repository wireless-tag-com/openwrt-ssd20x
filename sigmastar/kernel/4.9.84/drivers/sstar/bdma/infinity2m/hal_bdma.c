/*
* hal_bdma.c- Sigmastar
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
// Include files
/*=============================================================*/

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/version.h>
#include <linux/slab.h>
#include "ms_platform.h"
#include "ms_types.h"
#include "registers.h"
#include "kernel_bdma.h"
#include "hal_bdma.h"
#include "cam_os_wrapper.h"

////////////////////////////////////////////////////////////////////////////////
// Global variable
////////////////////////////////////////////////////////////////////////////////

volatile KeBdma_t * const g_ptKeBdma0 = (KeBdma_t *)IO_ADDRESS(BASE_REG_BDMA0_PA);
volatile KeBdma_t * const g_ptKeBdma1 = (KeBdma_t *)IO_ADDRESS(BASE_REG_BDMA1_PA);
volatile KeBdma_t * const g_ptKeBdma2 = (KeBdma_t *)IO_ADDRESS(BASE_REG_BDMA2_PA);
volatile KeBdma_t * const g_ptKeBdma3 = (KeBdma_t *)IO_ADDRESS(BASE_REG_BDMA3_PA);

static bool m_bBdmaFree[HAL_BDMA_CH_NUM] = {TRUE, TRUE, TRUE, TRUE};
static HalBdmaTxCb m_pfBdmaTxDoneCBFunc[HAL_BDMA_CH_NUM] = {NULL, NULL, NULL, NULL};
static bool m_bBdmaInited[HAL_BDMA_CH_NUM] = {0, 0, 0, 0};
static CamOsTsem_t m_stBdmaSemID[HAL_BDMA_CH_NUM];

/*=============================================================*/
// Local function definition
/*=============================================================*/

static u32 _HalBdmaVA2MiuA(void* virAddr)
{
#if 1
    return (u32)virAddr;
#else
    unsigned int phyAddr;

    phyAddr = (unsigned int)MsVA2PA(virAddr);

    return HalUtilPHY2MIUAddr(phyAddr);
#endif
}

/*=============================================================*/
// Global function definition
/*=============================================================*/

static irqreturn_t HalBdma0_ISR(int irq, void* priv)
{
    g_ptKeBdma0->reg_ch0_int_bdma = 0x1;
    g_ptKeBdma0->reg_ch0_int_en = 0x0;

    m_bBdmaFree[0] = TRUE;
    CamOsTsemUp(&m_stBdmaSemID[0]);

    if (NULL != m_pfBdmaTxDoneCBFunc[0]) {
        m_pfBdmaTxDoneCBFunc[0](0);
    }
    return IRQ_HANDLED;
}

static irqreturn_t HalBdma1_ISR(int irq, void* priv)
{
    g_ptKeBdma1->reg_ch0_int_bdma = 0x1;
    g_ptKeBdma1->reg_ch0_int_en = 0x0;

    m_bBdmaFree[1] = TRUE;
    CamOsTsemUp(&m_stBdmaSemID[1]);

    if (NULL != m_pfBdmaTxDoneCBFunc[1]) {
        m_pfBdmaTxDoneCBFunc[1](1);
    }
    return IRQ_HANDLED;
}

static irqreturn_t HalBdma2_ISR(int irq, void* priv)
{
    g_ptKeBdma2->reg_ch0_int_bdma = 0x1;
    g_ptKeBdma2->reg_ch0_int_en = 0x0;

    m_bBdmaFree[2] = TRUE;
    CamOsTsemUp(&m_stBdmaSemID[2]);

    if (NULL != m_pfBdmaTxDoneCBFunc[2]) {
        m_pfBdmaTxDoneCBFunc[2](2);
    }
    return IRQ_HANDLED;
}

static irqreturn_t HalBdma3_ISR(int irq, void* priv)
{
    g_ptKeBdma3->reg_ch0_int_bdma = 0x1;
    g_ptKeBdma3->reg_ch0_int_en = 0x0;

    m_bBdmaFree[3] = TRUE;
    CamOsTsemUp(&m_stBdmaSemID[3]);

    if (NULL != m_pfBdmaTxDoneCBFunc[3]) {
        m_pfBdmaTxDoneCBFunc[3](3);
    }
    return IRQ_HANDLED;
}

void _HalBdmaPowerOn(struct device_node *dev_node)
{
    struct clk **bdma_clks;
    struct clk *clk_parent;
    int num_parents, i;

    // If any channel of BDMA is initialized, the power is already turn on
    for (i = 0; i < HAL_BDMA_CH_NUM; i++)
    {
        if (m_bBdmaInited[i])
            return;
    }

#if CONFIG_OF
    num_parents = of_clk_get_parent_count(dev_node);
    if(num_parents > 0)
    {
        bdma_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(bdma_clks == NULL)
        {
            printk( "[BDMA]kzalloc failed!\n" );
            return;
        }

        //enable all clk
        for(i = 0; i < num_parents; i++)
        {
            bdma_clks[i] = of_clk_get(dev_node, i);
            if (IS_ERR(bdma_clks[i]))
            {
                printk( "Fail to get BDMA clk!\n" );
                clk_put(bdma_clks[i]);
                kfree(bdma_clks);
                return;
            }

            /* Get parent clock */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
            clk_parent = clk_hw_get_parent_by_index(__clk_get_hw(bdma_clks[i]), 0)->clk;
#else
            clk_parent = clk_get_parent_by_index(bdma_clks[i], 0);
#endif
            if(IS_ERR(clk_parent))
            {
                printk( "[BDMA]can't get parent clock\n" );
                clk_put(bdma_clks[i]);
                kfree(bdma_clks);
                return;
            }
            /* Set clock parent */
            clk_set_parent(bdma_clks[i], clk_parent);
            clk_prepare_enable(bdma_clks[i]);
            clk_put(bdma_clks[i]);
        }
        kfree(bdma_clks);
    }
#endif

}

//------------------------------------------------------------------------------
//  Function    : HalBdma_Initialize
//  Description :
//------------------------------------------------------------------------------
HalBdmaErr_e HalBdma_Initialize(u8 u8DmaCh)
{
    if (!m_bBdmaInited[u8DmaCh]) {

        struct device_node  *dev_node = NULL;
        irq_handler_t       pfIrqHandler = NULL;
        char                compatible[16];
        int                 iIrqNum = 0;

        CamOsSnprintf(compatible, sizeof(compatible), "sstar,bdma%d", u8DmaCh);

        dev_node = of_find_compatible_node(NULL, NULL, compatible);

        if (!dev_node) {
            return HAL_BDMA_ERROR;
        }

        _HalBdmaPowerOn(dev_node);

        /* Register interrupt handler */
        iIrqNum = irq_of_parse_and_map(dev_node, 0);

        switch(u8DmaCh) {
        case HAL_BDMA_CH0:
            pfIrqHandler = HalBdma0_ISR;
            break;
        case HAL_BDMA_CH1:
            pfIrqHandler = HalBdma1_ISR;
            break;
        case HAL_BDMA_CH2:
            pfIrqHandler = HalBdma2_ISR;
            break;
        case HAL_BDMA_CH3:
            pfIrqHandler = HalBdma3_ISR;
            break;
        default:
            return HAL_BDMA_ERROR;
            break;
        }

        if (0 != request_irq(iIrqNum, pfIrqHandler, 0, "BdmaIsr", NULL)) {
            CamOsPrintf("[BDMA] request_irq [%d] Fail\r\n", iIrqNum);
            return HAL_BDMA_ERROR;
        }
        else {
            //CamOsPrintf("[BDMA] request_irq [%d] OK\r\n", iIrqNum);
        }

        /* Initial semaphore */
        CamOsTsemInit(&m_stBdmaSemID[u8DmaCh], 1);

        m_bBdmaInited[u8DmaCh] = 1;
    }

    return HAL_BDMA_PROC_DONE;
}

//------------------------------------------------------------------------------
//  Function    : HalBdma_Transfer
//  Description :
//------------------------------------------------------------------------------
/**
 * @brief BDMA starts to transfer data
 *
 * @param [in]  ptBdmaParam      BDMA configuration parameter
 *
 * @return HalBdmaErr_e BDMA error code
 */
HalBdmaErr_e HalBdma_Transfer(u8 u8DmaCh, HalBdmaParam_t *ptBdmaParam)
{
    volatile KeBdma_t* g_ptKeBdma = g_ptKeBdma0;

    if (!m_bBdmaInited[u8DmaCh]) {
        return HAL_BDMA_PROC_DONE;
    }

    CamOsTsemDown(&m_stBdmaSemID[u8DmaCh]);

    switch(u8DmaCh) {
    case HAL_BDMA_CH0:
        g_ptKeBdma = g_ptKeBdma0;
        break;
    case HAL_BDMA_CH1:
        g_ptKeBdma = g_ptKeBdma1;
        break;
    case HAL_BDMA_CH2:
        g_ptKeBdma = g_ptKeBdma2;
        break;
    case HAL_BDMA_CH3:
        g_ptKeBdma = g_ptKeBdma3;
        break;
    default:
        return HAL_BDMA_PROC_DONE;
        break;
    }

    m_bBdmaFree[u8DmaCh] = FALSE;

    m_pfBdmaTxDoneCBFunc[u8DmaCh] = (HalBdmaTxCb)ptBdmaParam->pfTxCbFunc;

    g_ptKeBdma->reg_ch0_busy = 0x1;
    g_ptKeBdma->reg_ch0_int_bdma = 0x1;
    g_ptKeBdma->reg_ch0_done = 0x1;

    switch(ptBdmaParam->ePathSel)
    {
        case HAL_BDMA_MIU0_TO_MIU0:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_MIU0;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_TO_MIU1:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_MIU1;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_MIU0:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU0;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_MIU1:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU1;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_TO_IMI:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_IMI;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_IMI:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_IMI;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_IMI_TO_MIU0:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU0;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_IMI_TO_MIU1:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU1;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_IMI_TO_IMI:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MIU_IMI_CH0;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_IMI;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MEM_TO_MIU0:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MEM_FILL;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_4BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MEM_TO_MIU1:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MEM_FILL;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_MIU1;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_4BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MEM_TO_IMI:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_MEM_FILL;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_IMI;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_4BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_MIU0:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_SPI;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_8BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_MIU1:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_SPI;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_MIU1;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_8BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_IMI:
            g_ptKeBdma->reg_ch0_src_sel = REG_BDMA_SRC_SPI;
            g_ptKeBdma->reg_ch0_dst_sel = REG_BDMA_SRC_MIU_IMI_CH1;
            g_ptKeBdma->reg_ch0_replace_miu = REG_BDMA_CH1_IMI;
            g_ptKeBdma->reg_ch0_src_dw  = REG_BDMA_DATA_DEPTH_8BYTE;
            g_ptKeBdma->reg_ch0_dst_dw  = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        default:
            return HAL_BDMA_PROC_DONE;
            break;
    }

    // Set Source / Destination Address
    if ((HAL_BDMA_MEM_TO_MIU0 == ptBdmaParam->ePathSel) ||
        (HAL_BDMA_MEM_TO_MIU1 == ptBdmaParam->ePathSel) ||
        (HAL_BDMA_MEM_TO_IMI == ptBdmaParam->ePathSel)) {
        g_ptKeBdma->reg_ch0_cmd0_low  = (U16)(ptBdmaParam->u32Pattern & 0xFFFF);
        g_ptKeBdma->reg_ch0_cmd0_high = (U16)(ptBdmaParam->u32Pattern >> 16);
    }
    else {
        g_ptKeBdma->reg_ch0_src_a0 = (U16)(_HalBdmaVA2MiuA(ptBdmaParam->pSrcAddr) & 0xFFFF);
        g_ptKeBdma->reg_ch0_src_a1 = (U16)(_HalBdmaVA2MiuA(ptBdmaParam->pSrcAddr) >> 16);
    }

    g_ptKeBdma->reg_ch0_dst_a0 = (U16)(_HalBdmaVA2MiuA(ptBdmaParam->pDstAddr) & 0xFFFF);
    g_ptKeBdma->reg_ch0_dst_a1 = (U16)(_HalBdmaVA2MiuA(ptBdmaParam->pDstAddr) >> 16);

    // Set Transfer Size
    g_ptKeBdma->reg_ch0_size0 = (U16)(ptBdmaParam->u32TxCount & 0xFFFF);
    g_ptKeBdma->reg_ch0_size1 = (U16)(ptBdmaParam->u32TxCount >> 16);

    // Set Interrupt Enable
    if (ptBdmaParam->bIntMode) {
        g_ptKeBdma->reg_ch0_int_en = 1;
    }
    else {
        g_ptKeBdma->reg_ch0_int_en = 0;
    }

    // Trigger
    g_ptKeBdma->reg_ch0_trig = 0x1;

    // Polling mode
    if (!ptBdmaParam->bIntMode) {
        HalBdma_WaitTransferDone(u8DmaCh, ptBdmaParam);
    }

    return HAL_BDMA_PROC_DONE;
}

//------------------------------------------------------------------------------
//  Function    : HalBdma_WaitTransferDone
//  Description :
//------------------------------------------------------------------------------
/**
 * @brief BDMA wait transfer data done
 *
 * @param [in]  ptBdmaParam      BDMA configuration parameter
 *
 * @return HalBdmaErr_e BDMA error code
 */
HalBdmaErr_e HalBdma_WaitTransferDone(u8 u8DmaCh, HalBdmaParam_t *ptBdmaParam)
{
    volatile KeBdma_t* g_ptKeBdma;
    U32                u32TimeOut = 0x00FFFFFF;
    bool               bRet = FALSE;

    if (!m_bBdmaInited[u8DmaCh]) {
        return HAL_BDMA_PROC_DONE;
    }

    switch(u8DmaCh) {
    case HAL_BDMA_CH0:
        g_ptKeBdma = g_ptKeBdma0;
        break;
    case HAL_BDMA_CH1:
        g_ptKeBdma = g_ptKeBdma1;
        break;
    case HAL_BDMA_CH2:
        g_ptKeBdma = g_ptKeBdma2;
        break;
    case HAL_BDMA_CH3:
        g_ptKeBdma = g_ptKeBdma3;
        break;
    default:
        return HAL_BDMA_PROC_DONE;
        break;
    }

    // Polling mode
    if (!ptBdmaParam->bIntMode) {

        while(--u32TimeOut)
        {
            // Check done
            if (g_ptKeBdma->reg_ch0_done == 0x1)
            {
                bRet = TRUE;
                break;
            }
        }

        // Clear done
        g_ptKeBdma->reg_ch0_done = 0x1;

        m_bBdmaFree[u8DmaCh] = TRUE;
        CamOsTsemUp(&m_stBdmaSemID[u8DmaCh]);

        if (bRet == FALSE) {
            CamOsPrintf("Wait BDMA Done Fail\r\n");
            return HAL_BDMA_POLLING_TIMEOUT;
        }
    }
    else {
        // Interrupt mode
    }

    return HAL_BDMA_PROC_DONE;
}
