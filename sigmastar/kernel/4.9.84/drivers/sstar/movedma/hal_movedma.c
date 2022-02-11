/*
* hal_movedma.c- Sigmastar
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
#include "ms_platform.h"
#include "ms_types.h"
#include "registers.h"
#include "kernel_movedma.h"
#include "hal_movedma.h"
#include "cam_os_wrapper.h"

/*=============================================================*/
// Variable definition
/*=============================================================*/

volatile KeMoveDma_t* const g_ptKeMoveDma = (KeMoveDma_t *)IO_ADDRESS(BASE_REG_MOVDMA_PA);

static HalMoveDmaCBFunc *m_pfMdmaTxDoneCBFunc = NULL;
static u32 m_u32MdmaTxDoneCBArgu = 0;
static bool m_bMdmaInited = FALSE;
static CamOsTsem_t  m_stMdmaSemID;

/*=============================================================*/
// Function definition
/*=============================================================*/

static u32 _HalMoveDmaVA2Miu(void* virAddr)
{
#if 1
    return (u32)virAddr;
#else
    unsigned int phyAddr = 0;
    u32 MiuAddr = 0;

    phyAddr = (unsigned int)MsVA2PA(virAddr);

    MiuAddr = HalUtilPHY2MIUAddr(phyAddr);

    if (MiuAddr == 0xFFFFFFFF) {
        CamOsPrintf("[DMA] WrongAddr[%x][%x][%x]\r\n", (int)virAddr, phyAddr, MiuAddr);
    }
    return MiuAddr;
#endif
}

//------------------------------------------------------------------------------
//  Function    : HalMoveDma_ISR
//  Description :
//------------------------------------------------------------------------------
static irqreturn_t HalMoveDma_ISR(int irq, void* priv)
{
    u16 intsrc = 0;

    intsrc = g_ptKeMoveDma->reg_dma_irq_final_status;
    g_ptKeMoveDma->reg_dma_irq_clr = intsrc;

    if (intsrc & MOVEDMA_INT_MOVE0_DONE) {


        CamOsTsemUp(&m_stMdmaSemID);

        //CamOsPrintf("[MDMA] Done\r\n");

        if (m_pfMdmaTxDoneCBFunc) {
            (*m_pfMdmaTxDoneCBFunc)(m_u32MdmaTxDoneCBArgu);
        }
    }
    else
    {
        CamOsPrintf("[MDMA] Can't find irq status!!!!\r\n");
    }

    return IRQ_HANDLED;
}
EXPORT_SYMBOL(HalMoveDma_ISR);

//------------------------------------------------------------------------------
//  Function    : HalDmaGen_Initialize
//  Description :
//------------------------------------------------------------------------------
HalMoveDmaErr_e HalMoveDma_Initialize(void)
{
    if (!m_bMdmaInited) {

        struct device_node  *dev_node = NULL;
        irq_handler_t       pfIrqHandler = NULL;
        char                compatible[16];
        int                 iIrqNum = 0;

        CamOsSnprintf(compatible, sizeof(compatible), "sstar,movdma");

        dev_node = of_find_compatible_node(NULL, NULL, compatible);

        if (!dev_node) {
            CamOsPrintf("[MDMA] of_find_compatible_node Fail\r\n");
            return HAL_MOVEDMA_ERR_PARAM;
        }

        /* Register interrupt handler */
        iIrqNum = irq_of_parse_and_map(dev_node, 0);

        pfIrqHandler = HalMoveDma_ISR;

        if (0 != request_irq(iIrqNum, HalMoveDma_ISR, 0, "HalMoveDma_ISR", NULL)) {
            CamOsPrintf("[MDMA] request_irq [%d] Fail\r\n", iIrqNum);
            return HAL_MOVEDMA_ERR_PARAM;
        }
        else {
            //CamOsPrintf("[MDMA] request_irq [%d] OK\r\n", iIrqNum);
        }

        g_ptKeMoveDma->reg_dma_irq_mask = 0;

        /* Initial semaphore */
        CamOsTsemInit(&m_stMdmaSemID, 1);

        m_pfMdmaTxDoneCBFunc = NULL;

        m_bMdmaInited = 1;
    }

    return HAL_MOVEDMA_NO_ERR;
}
EXPORT_SYMBOL(HalMoveDma_Initialize);

//------------------------------------------------------------------------------
//  Function    : HalMoveDma_MoveData
//  Description :
//------------------------------------------------------------------------------
HalMoveDmaErr_e HalMoveDma_MoveData(HalMoveDmaParam_t *ptMoveDmaParam)
{
    CamOsRet_e eOSRet = CAM_OS_OK;

    eOSRet = CamOsTsemTimedDown(&m_stMdmaSemID, 3000);

    if (eOSRet == CAM_OS_TIMEOUT)
    {

        CamOsPrintf("MoveDMA TsemTimedDown TimeOut\r\n");
        return HAL_MOVEDMA_POLLING_TIMEOUT;
    }

    m_pfMdmaTxDoneCBFunc    = ptMoveDmaParam->CallBackFunc;
    m_u32MdmaTxDoneCBArgu   = ptMoveDmaParam->CallBackArg;

    /* Reset DMA first */
    //g_ptKeMoveDma->reg_dma_mov_sw_rst = 1;
    g_ptKeMoveDma->reg_dma_irq_mask   = 0;

    /* Set LineOffset Attribute */
    if (ptMoveDmaParam->u32Mode == HAL_MOVEDMA_LINE_OFFSET)
    {
        if(ptMoveDmaParam->pstLineOfst)
        {
            g_ptKeMoveDma->reg_move0_offset_src_width_l      = (U16)(ptMoveDmaParam->pstLineOfst->u32SrcWidth & 0xFFFF);
            g_ptKeMoveDma->reg_move0_offset_src_width_h      = (U16)(ptMoveDmaParam->pstLineOfst->u32SrcWidth >> 16);
            g_ptKeMoveDma->reg_move0_offset_src_offset_l     = (U16)(ptMoveDmaParam->pstLineOfst->u32SrcOffset & 0xFFFF);
            g_ptKeMoveDma->reg_move0_offset_src_offset_h     = (U16)(ptMoveDmaParam->pstLineOfst->u32SrcOffset >> 16);
            g_ptKeMoveDma->reg_move0_offset_dest_width_l     = (U16)(ptMoveDmaParam->pstLineOfst->u32DstWidth & 0xFFFF);
            g_ptKeMoveDma->reg_move0_offset_dest_width_h     = (U16)(ptMoveDmaParam->pstLineOfst->u32DstWidth >> 16);
            g_ptKeMoveDma->reg_move0_offset_dest_offset_l    = (U16)(ptMoveDmaParam->pstLineOfst->u32DstOffset & 0xFFFF);
            g_ptKeMoveDma->reg_move0_offset_dest_offset_h    = (U16)(ptMoveDmaParam->pstLineOfst->u32DstOffset >> 16);
            g_ptKeMoveDma->reg_move0_offset_en               = 1;
        }
        else
        {
            CamOsPrintf("pstLineOfst is null\r\n");
            g_ptKeMoveDma->reg_move0_offset_en               = 1;
            return HAL_MOVEDMA_ERR_PARAM;
        }
    }
    else
    {
        g_ptKeMoveDma->reg_move0_offset_en = 0;
    }

    g_ptKeMoveDma->reg_move0_src_start_addr_l   = (U16)(_HalMoveDmaVA2Miu((void*)ptMoveDmaParam->u32SrcAddr) & 0xFFFF);
    g_ptKeMoveDma->reg_move0_src_start_addr_h   = (U16)(_HalMoveDmaVA2Miu((void*)ptMoveDmaParam->u32SrcAddr) >> 16);
    g_ptKeMoveDma->reg_move0_dest_start_addr_l  = (U16)(_HalMoveDmaVA2Miu((void*)ptMoveDmaParam->u32DstAddr) & 0xFFFF);
    g_ptKeMoveDma->reg_move0_dest_start_addr_h  = (U16)(_HalMoveDmaVA2Miu((void*)ptMoveDmaParam->u32DstAddr) >> 16);

    if (ptMoveDmaParam->u32Mode == HAL_MOVEDMA_MSPI)
    {
        if(ptMoveDmaParam->pstMspist)
        {
            if(ptMoveDmaParam->pstMspist->u32Direction < HAL_MOVEDMA_RW_MAX)
            {
                g_ptKeMoveDma->reg_dma_spi_rw           = ptMoveDmaParam->pstMspist->u32Direction;
            }
            else
            {
                CamOsPrintf("rw para is err\r\n");
                return HAL_MOVEDMA_ERR_PARAM;
            }

            if(g_ptKeMoveDma->reg_spi_device_select < HAL_MOVEDMA_MSPI_MAX)
            {
                g_ptKeMoveDma->reg_spi_device_select    = ptMoveDmaParam->pstMspist->u32DeviceSelect;           //0 select mspi0 , 1 select mspi1
            }
            else
            {
                CamOsPrintf("mspi channel is unsupported\r\n");
                return HAL_MOVEDMA_ERR_PARAM;
            }
            g_ptKeMoveDma->reg_dma_spi_device_mode  = 1;                                                    // 1 to enable dma device mode

            if(ptMoveDmaParam->pstMspist->u32Direction == HAL_MOVEDMA_RD)
            {
                g_ptKeMoveDma->reg_move0_src_start_addr_l   = 0;
                g_ptKeMoveDma->reg_move0_src_start_addr_h   = 0;
            }
            else
            {
                g_ptKeMoveDma->reg_move0_dest_start_addr_l  = 0;
                g_ptKeMoveDma->reg_move0_dest_start_addr_h  = 0;
            }
        }
        else
        {
            CamOsPrintf("pstMspist is null\r\n");
            g_ptKeMoveDma->reg_dma_spi_device_mode  = 0;        // 1 to enable dma device mode
            return HAL_MOVEDMA_ERR_PARAM;
        }
    }
    else
    {
        g_ptKeMoveDma->reg_dma_spi_device_mode  = 0;            // 1 to enable dma device mode
    }

    g_ptKeMoveDma->reg_move0_total_byte_cnt_l   = (U16)(ptMoveDmaParam->u32Count & 0xFFFF);
    g_ptKeMoveDma->reg_move0_total_byte_cnt_h   = (U16)(ptMoveDmaParam->u32Count >> 16);

    g_ptKeMoveDma->reg_dma_move0_miu_sel_en     = 1;
    g_ptKeMoveDma->reg_dma_move0_src_miu_sel    = (ptMoveDmaParam->u32SrcMiuSel) ? (REG_DMA_MOVE0_SEL_MIU1) : (REG_DMA_MOVE0_SEL_MIU0);
    g_ptKeMoveDma->reg_dma_move0_dst_miu_sel    = (ptMoveDmaParam->u32DstMiuSel) ? (REG_DMA_MOVE0_SEL_MIU1) : (REG_DMA_MOVE0_SEL_MIU0);

#if 0
    CamOsPrintf("==========================================\r\n");
    CamOsPrintf("g_ptKeMoveDma %p\r\n",           g_ptKeMoveDma);
    CamOsPrintf("reg_move0_offset_en %x\r\n",           g_ptKeMoveDma->reg_move0_offset_en);
    CamOsPrintf("reg_move0_src_start_addr_l %x\r\n",    g_ptKeMoveDma->reg_move0_src_start_addr_l);
    CamOsPrintf("reg_move0_src_start_addr_h %x\r\n",    g_ptKeMoveDma->reg_move0_src_start_addr_h);
    CamOsPrintf("reg_move0_dest_start_addr_l %x\r\n",   g_ptKeMoveDma->reg_move0_dest_start_addr_l);
    CamOsPrintf("reg_move0_dest_start_addr_h %x\r\n",   g_ptKeMoveDma->reg_move0_dest_start_addr_h);
    CamOsPrintf("reg_move0_total_byte_cnt_l %x\r\n",    g_ptKeMoveDma->reg_move0_total_byte_cnt_l);
    CamOsPrintf("reg_move0_total_byte_cnt_h %x\r\n",    g_ptKeMoveDma->reg_move0_total_byte_cnt_h);
    CamOsPrintf("reg_move0_offset_src_width_l %x\r\n",  g_ptKeMoveDma->reg_move0_offset_src_width_l);
    CamOsPrintf("reg_move0_offset_src_width_h %x\r\n",  g_ptKeMoveDma->reg_move0_offset_src_width_h);
    CamOsPrintf("reg_move0_offset_src_offset_l %x\r\n", g_ptKeMoveDma->reg_move0_offset_src_offset_l);
    CamOsPrintf("reg_move0_offset_src_offset_h %x\r\n", g_ptKeMoveDma->reg_move0_offset_src_offset_h);
    CamOsPrintf("reg_move0_offset_dest_width_l %x\r\n", g_ptKeMoveDma->reg_move0_offset_dest_width_l);
    CamOsPrintf("reg_move0_offset_dest_width_h %x\r\n", g_ptKeMoveDma->reg_move0_offset_dest_width_h);
    CamOsPrintf("reg_move0_offset_dest_offset_l %x\r\n", g_ptKeMoveDma->reg_move0_offset_dest_offset_l);
    CamOsPrintf("reg_move0_offset_dest_offset_h %x\r\n", g_ptKeMoveDma->reg_move0_offset_dest_offset_h);
    CamOsPrintf("reg_dma_move0_left_byte_l %x\r\n",     g_ptKeMoveDma->reg_dma_move0_left_byte_l);
    CamOsPrintf("reg_dma_move0_left_byte_h %x\r\n",     g_ptKeMoveDma->reg_dma_move0_left_byte_h);
    CamOsPrintf("reg_dma_irq_mask %x\r\n",              g_ptKeMoveDma->reg_dma_irq_mask);
    CamOsPrintf("reg_dma_irq_final_status %x\r\n",      g_ptKeMoveDma->reg_dma_irq_final_status);
    CamOsPrintf("reg_dma_move0_miu_sel_en %x\r\n",      g_ptKeMoveDma->reg_dma_move0_miu_sel_en);
    CamOsPrintf("reg_dma_move0_src_miu_sel %x\r\n",     g_ptKeMoveDma->reg_dma_move0_src_miu_sel);
    CamOsPrintf("reg_dma_move0_dst_miu_sel %x\r\n",     g_ptKeMoveDma->reg_dma_move0_dst_miu_sel);
    //0x50
    CamOsPrintf("reg_dma_rw [%p]=%x\r\n",           &g_ptKeMoveDma->reg_50, g_ptKeMoveDma->reg_dma_rw);
    CamOsPrintf("reg_dma_device_mode %x\r\n",       g_ptKeMoveDma->reg_dma_device_mode);
    CamOsPrintf("reg_device_select %x\r\n",         g_ptKeMoveDma->reg_device_select);
#endif

    g_ptKeMoveDma->reg_dma_move_en = 1;

    return HAL_MOVEDMA_NO_ERR;
}

EXPORT_SYMBOL(HalMoveDma_MoveData); //for unittest

