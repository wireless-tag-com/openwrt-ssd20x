/*
* ms_msys_dma_wrapper.c- Sigmastar
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
#include <linux/kernel.h>
#include <asm/uaccess.h>  /* for get_fs*/
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/compaction.h> /*  for sysctl_compaction_handler*/
#include <asm/cacheflush.h>

#include "registers.h"
#include "ms_platform.h"
#include "mdrv_msys_io_st.h"
#include "mdrv_msys_io.h"
#include "platform_msys.h"
#include "hal_bdma.h"
#include "hal_movedma.h"
#include "cam_os_wrapper.h"

extern struct miscdevice  sys_dev;
#if defined(CONFIG_MS_BDMA)
static CamOsTsem_t m_stBdmaDoneSem[HAL_BDMA_CH_NUM];
#endif
#if defined(CONFIG_MS_MOVE_DMA)
static CamOsTsem_t m_stMdmaDoneSem;
#endif

#if defined(CONFIG_MS_MOVE_DMA)
void msys_mdma_done(u32 argu)
{
    CamOsTsemUp(&m_stMdmaDoneSem);
}

int msys_dma_blit(MSYS_DMA_BLIT *pstMdmaCfg)
{
    HalMoveDmaParam_t       tMoveDmaParam;
    HalMoveDmaLineOfst_t    tMoveDmaLineOfst;

    tMoveDmaParam.u32SrcAddr    = pstMdmaCfg->phyaddr_src;
    tMoveDmaParam.u32SrcMiuSel  = (pstMdmaCfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (0) : (1);
    tMoveDmaParam.u32DstAddr    = pstMdmaCfg->phyaddr_dst;
    tMoveDmaParam.u32DstMiuSel  = (pstMdmaCfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? (0) : (1);
    tMoveDmaParam.u32Count      = pstMdmaCfg->length;
    tMoveDmaParam.CallBackFunc  = msys_mdma_done;
    tMoveDmaParam.CallBackArg   = 0;


    if (pstMdmaCfg->lineofst_src && pstMdmaCfg->lineofst_dst)
    {
        if((pstMdmaCfg->lineofst_src<pstMdmaCfg->width_src)
            || (pstMdmaCfg->lineofst_dst<pstMdmaCfg->width_dst) )
        {
            printk("[MDMA] CAUTION: line offset is less than width\n"
                "width_src=0x%x  lineofst_src=0x%x, DstWidth=0x%x  lineofst_dst=0x%x\n",
                pstMdmaCfg->width_src, pstMdmaCfg->lineofst_src, pstMdmaCfg->width_dst, pstMdmaCfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tMoveDmaLineOfst.u32SrcWidth    = pstMdmaCfg->width_src;
        tMoveDmaLineOfst.u32SrcOffset   = pstMdmaCfg->lineofst_src;
        tMoveDmaLineOfst.u32DstWidth    = pstMdmaCfg->width_dst;
        tMoveDmaLineOfst.u32DstOffset   = pstMdmaCfg->lineofst_dst;

        tMoveDmaParam.bEnLineOfst       = 1;
        tMoveDmaParam.pstLineOfst       = &tMoveDmaLineOfst;
    }
    else {
        tMoveDmaParam.bEnLineOfst       = 0;
        tMoveDmaParam.pstLineOfst       = NULL;
    }

    if (HAL_MOVEDMA_NO_ERR != HalMoveDma_MoveData(&tMoveDmaParam)) {
        return -1;
    }

    CamOsTsemDownInterruptible(&m_stMdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_blit);
#endif
#if defined(CONFIG_MS_BDMA)
static void msys_bdma_done(u32 u32DmaCh)
{
    CamOsTsemUp(&m_stBdmaDoneSem[u32DmaCh]);
}

int msys_dma_fill(MSYS_DMA_FILL *pstDmaCfg)
{
    HalBdmaParam_t  tBdmaParam;
    u8              u8DmaCh = HAL_BDMA_CH1;
    tBdmaParam.ePathSel     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MEM_TO_MIU0) : (HAL_BDMA_MEM_TO_MIU1);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = pstDmaCfg->length;
    tBdmaParam.pSrcAddr     = (void*)0;
    tBdmaParam.pDstAddr     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (void *)((U32)pstDmaCfg->phyaddr) : (void *)((U32)pstDmaCfg->phyaddr - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.u32Pattern   = pstDmaCfg->pattern;

    if (HAL_BDMA_PROC_DONE != HalBdma_Transfer(u8DmaCh, &tBdmaParam)) {
        return -1;
    }

    if (tBdmaParam.bIntMode) {
        CamOsTsemDownInterruptible(&m_stBdmaDoneSem[u8DmaCh]);
    }

    return 0;
}
EXPORT_SYMBOL(msys_dma_fill);

int  msys_dma_copy(MSYS_DMA_COPY *cfg)
{
    HalBdmaParam_t  tBdmaParam;
#if defined(CONFIG_ARCH_INFINITY2)
    u8              u8DmaCh = HAL_BDMA_CH1;
#else
    u8              u8DmaCh = HAL_BDMA_CH2;
#endif
    tBdmaParam.ePathSel     = ((U32)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MIU0_TO_MIU0) : (HAL_BDMA_MIU1_TO_MIU0);
    tBdmaParam.ePathSel     = ((U32)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? tBdmaParam.ePathSel : tBdmaParam.ePathSel+1;
    tBdmaParam.pSrcAddr     = ((U32)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (void *)((U32)cfg->phyaddr_src) : (void *)((U32)cfg->phyaddr_src - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pDstAddr     = ((U32)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? (void *)((U32)cfg->phyaddr_dst) : (void *)((U32)cfg->phyaddr_dst - ARM_MIU1_BASE_ADDR);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != HalBdma_Transfer(u8DmaCh, &tBdmaParam)) {
        return -1;
    }

    if (tBdmaParam.bIntMode) {
        CamOsTsemDownInterruptible(&m_stBdmaDoneSem[u8DmaCh]);
    }

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy);

#if defined(CONFIG_MS_BDMA_LINE_OFFSET_ON)
int msys_dma_fill_lineoffset(MSYS_DMA_FILL_BILT *pstDmaCfg)
{
    HalBdmaParam_t  tBdmaParam;
    HalBdmaLineOfst_t tBdmaLineOfst;
    u8              u8DmaCh = HAL_BDMA_CH1;

    tBdmaParam.ePathSel     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MEM_TO_MIU0) : (HAL_BDMA_MEM_TO_MIU1);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = pstDmaCfg->length;
    tBdmaParam.pSrcAddr     = (void*)0;
    tBdmaParam.pDstAddr     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (void *)((U32)pstDmaCfg->phyaddr) : (void *)((U32)pstDmaCfg->phyaddr - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.u32Pattern   = pstDmaCfg->pattern;

    if (pstDmaCfg->lineofst_dst)
    {
        if (pstDmaCfg->lineofst_dst < pstDmaCfg->width_dst)
        {
            printk("[BDMA] CAUTION: line offset is less than width\n"
            "DstWidth=0x%x  lineofst_dst=0x%x\n",
            pstDmaCfg->width_dst, pstDmaCfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tBdmaParam.pstLineOfst = &tBdmaLineOfst;
        tBdmaParam.pstLineOfst->u32SrcWidth  = pstDmaCfg->width_dst;
        tBdmaParam.pstLineOfst->u32SrcOffset = pstDmaCfg->lineofst_dst;
        tBdmaParam.pstLineOfst->u32DstWidth  = pstDmaCfg->width_dst;
        tBdmaParam.pstLineOfst->u32DstOffset = pstDmaCfg->lineofst_dst;

        tBdmaParam.bEnLineOfst = 1;
    }
    else {
        tBdmaParam.bEnLineOfst = 0;
        tBdmaParam.pstLineOfst = NULL;
    }

    if (HAL_BDMA_PROC_DONE != HalBdma_Transfer_LineOffset(u8DmaCh, &tBdmaParam)) {
        return -1;
    }

    if (tBdmaParam.bIntMode) {
        CamOsTsemDownInterruptible(&m_stBdmaDoneSem[u8DmaCh]);
    }

    return 0;
}
EXPORT_SYMBOL(msys_dma_fill_lineoffset);

int msys_dma_copy_lineoffset(MSYS_DMA_BLIT *cfg)
{
    HalBdmaParam_t    tBdmaParam;
    HalBdmaLineOfst_t tBdmaLineOfst;
#if defined(CONFIG_ARCH_INFINITY2)
    u8              u8DmaCh = HAL_BDMA_CH1;
#else
    u8              u8DmaCh = HAL_BDMA_CH2;
#endif

    tBdmaParam.ePathSel     = ((U32)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MIU0_TO_MIU0) : (HAL_BDMA_MIU1_TO_MIU0);
    tBdmaParam.ePathSel     = ((U32)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? tBdmaParam.ePathSel : tBdmaParam.ePathSel+1;
    tBdmaParam.pSrcAddr     = ((U32)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (void *)((U32)cfg->phyaddr_src) : (void *)((U32)cfg->phyaddr_src - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pDstAddr     = ((U32)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? (void *)((U32)cfg->phyaddr_dst) : (void *)((U32)cfg->phyaddr_dst - ARM_MIU1_BASE_ADDR);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.u32Pattern   = 0;

    if (cfg->lineofst_src && cfg->lineofst_dst)
    {
        if((cfg->lineofst_src < cfg->width_src)
        || (cfg->lineofst_dst < cfg->width_dst) )
        {
            printk("[BDMA] CAUTION: line offset is less than width\n"
            "width_src=0x%x  lineofst_src=0x%x, DstWidth=0x%x  lineofst_dst=0x%x\n",
            cfg->width_src, cfg->lineofst_src, cfg->width_dst, cfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tBdmaParam.pstLineOfst = &tBdmaLineOfst;
        tBdmaParam.pstLineOfst->u32SrcWidth  = cfg->width_src;
        tBdmaParam.pstLineOfst->u32SrcOffset = cfg->lineofst_src;
        tBdmaParam.pstLineOfst->u32DstWidth  = cfg->width_dst;
        tBdmaParam.pstLineOfst->u32DstOffset = cfg->lineofst_dst;

        tBdmaParam.bEnLineOfst = 1;
    }
    else {
        tBdmaParam.bEnLineOfst = 0;
        tBdmaParam.pstLineOfst = NULL;
    }

    if (HAL_BDMA_PROC_DONE != HalBdma_Transfer_LineOffset(u8DmaCh, &tBdmaParam)) {
        return -1;
    }

    if (tBdmaParam.bIntMode) {
        CamOsTsemDownInterruptible(&m_stBdmaDoneSem[u8DmaCh]);
    }

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_lineoffset);

#endif

#endif

static int __init ms_msys_dma_wrapper_init(void)
{
#if defined(CONFIG_MS_MOVE_DMA)
    CamOsTsemInit(&m_stMdmaDoneSem, 0);

    HalMoveDma_Initialize();
#endif

#if defined(CONFIG_MS_BDMA)
    CamOsTsemInit(&m_stBdmaDoneSem[0], 0);
    CamOsTsemInit(&m_stBdmaDoneSem[1], 0);
    CamOsTsemInit(&m_stBdmaDoneSem[2], 0);
    CamOsTsemInit(&m_stBdmaDoneSem[3], 0);

    //HalBdma_Initialize(0);
    HalBdma_Initialize(1);
    HalBdma_Initialize(2);
    HalBdma_Initialize(3);
#endif
    return 0;
}
subsys_initcall(ms_msys_dma_wrapper_init)
