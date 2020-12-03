/*
* mstar_mci.c- Sigmastar
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
#include "mstar_mci.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define MCI_RETRY_CNT_CMD_TO        100
#define MCI_RETRY_CNT_CRC_ERR       200 // avoid stack overflow
#define MCI_RETRY_CNT_OK_CLK_UP     10

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static void mstar_mci_send_command(struct mstar_mci_host *pSstarHost_st, struct mmc_command *pCmd_st);
static void mstar_mci_completed_command(struct mstar_mci_host *pSstarHost_st);
#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
static void mstar_mci_completed_command_FromRAM(struct mstar_mci_host *pSstarHost_st);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static int mstar_mci_pre_dma_transfer(struct mstar_mci_host *pSstarHost_st, struct mmc_data *data, struct mstar_mci_host_next *next);
static void mstar_mci_pre_req(struct mmc_host *pMMCHost_st, struct mmc_request *mrq, bool is_first_req);
static void mstar_mci_post_req(struct mmc_host * pMMCHost_st,struct mmc_request * mrq,int err);
#endif
#endif

static void mstar_mci_request(struct mmc_host *pMMCHost_st, struct mmc_request *pMRQ_st);
static void mstar_mci_set_ios(struct mmc_host *pMMCHost_st, struct mmc_ios *pIOS_st);
static  s32 mstar_mci_get_ro(struct mmc_host *pMMCHost_st);

u32 mstar_mci_WaitD0High(u32 u32_us);
static U32 u32_ok_cnt=0;

/*****************************************************************************
 * Define Static Global Variables
 ******************************************************************************/
/* MSTAR Multimedia Card Interface Operations */
static const struct mmc_host_ops sg_mstar_mci_ops =
{
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    .pre_req = mstar_mci_pre_req,
    .post_req = mstar_mci_post_req,
    #endif
    #endif
    .request =	mstar_mci_request,
    .set_ios =	mstar_mci_set_ios,
    .get_ro =	mstar_mci_get_ro,
};

static struct task_struct *sgp_eMMCThread_st = NULL;
static ulong wr_seg_size = 0;
static ulong wr_split_threshold = 0;

u8 u8_enable_sar5 = 0;

// ===============================
// for /sys files
U32 gu32_pwrsvr_gpio_enable = 0;
U32 gu32_pwrsvr_gpio_addr = 0;
U32 gu32_pwrsvr_gpio_bit = 0;
U32 gu32_pwrsvr_gpio_trigger = 0;
U32 gu32_emmc_sanitize = 0;

/*****************************************************************************
 * for profiling
 ******************************************************************************/
#if defined(CONFIG_MMC_MSTAR_MMC_EMMC_LIFETEST)
static struct proc_dir_entry * writefile;
const char procfs_name[] = "StorageBytes";

int procfile_read(char* buffer, char ** buffer_location, off_t offset,
					int buffer_length, int *eof, void *data)
{
	int ret;
	if(offset > 0)
		ret = 0;
	else
		ret = sprintf(buffer,"TotalWriteBytes %llu GB %llu MB\nTotalReadBytes %llu GB %llu MB\n",
				g_eMMCDrv.u64_CNT_TotalWBlk/1024/1024/2, (g_eMMCDrv.u64_CNT_TotalWBlk/1024/2) % 1024,
				g_eMMCDrv.u64_CNT_TotalRBlk/1024/1024/2, (g_eMMCDrv.u64_CNT_TotalRBlk/1024/2) % 1024);
	return ret;
}
#endif

/******************************************************************************
 * Functions
 ******************************************************************************/
static int mstar_mci_get_dma_dir(struct mmc_data *data)
{
	#if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
		return DMA_ACP;
	#else
    if (data->flags & MMC_DATA_WRITE)
        return DMA_TO_DEVICE;
    else
        return DMA_FROM_DEVICE;
	#endif
}

static void mstar_mci_pre_dma_read(struct mstar_mci_host *pSstarHost_st)
{
    /* Define Local Variables */
    struct scatterlist *pSG_st = 0;
    struct mmc_command *pCmd_st = 0;
    struct mmc_data *pData_st = 0;
    //int sg_count = 0;
    u32 u32_dmalen = 0;
    //u32 u32_sg_count;
    dma_addr_t dmaaddr = 0;

    pCmd_st = pSstarHost_st->cmd;
    pData_st = pCmd_st->data;

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    mstar_mci_pre_dma_transfer(pSstarHost_st, pData_st, NULL);
    #else
    dma_map_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    pSG_st = &pData_st->sg[0];
    dmaaddr = (u32)sg_dma_address(pSG_st);
    u32_dmalen = sg_dma_len(pSG_st);
	u32_dmalen = ((u32_dmalen&0x1FF)?1:0) + u32_dmalen/512;

	eMMC_FCIE_MIU0_MIU1_SEL(dmaaddr);

	if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
	{
		REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_R : TOGGLE_CNT_256_CLK_R);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
		REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
		eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
		REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
	}
	REG_FCIE_W(FCIE_JOB_BL_CNT, u32_dmalen);
    REG_FCIE_W(FCIE_SDIO_ADDR0,(((u32)dmaaddr) & 0xFFFF));
    REG_FCIE_W(FCIE_SDIO_ADDR1,(((u32)dmaaddr) >> 16));
	REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    eMMC_FCIE_FifoClkRdy(0);
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

	#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
    g_eMMCDrv.u64_CNT_TotalRBlk += u32_dmalen;
	g_eMMCDrv.u32_RBlk_tmp = u32_dmalen;
	#endif

}

static U32 mstar_mci_post_dma_read(struct mstar_mci_host *pSstarHost_st)
{
    /* Define Local Variables */
    struct mmc_command	*pCmd_st = 0;
    struct mmc_data		*pData_st = 0;
    struct scatterlist	*pSG_st = 0;
    int i;
    u32 dmalen = 0;
    dma_addr_t dmaaddr = 0;
    int err = eMMC_ST_SUCCESS;

    pCmd_st = pSstarHost_st->cmd;
    pData_st = pCmd_st->data;
    pSG_st = &(pData_st->sg[0]);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
    #endif

    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_MIU_LAST_DONE, eMMC_READ_WAIT_TIME) != eMMC_ST_SUCCESS ||
       (REG_FCIE(FCIE_SD_STATUS)&BIT_SD_R_CRC_ERR))
    {
        if((REG_FCIE(FCIE_SD_STATUS)&BIT_SD_R_CRC_ERR))
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS) );
        else
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: r timeout, MIE EVENT 0x%X\n", REG_FCIE(FCIE_MIE_EVENT));
        err = eMMC_ST_ERR_TIMEOUT_MIULASTDONE;
        return err;
    }

    pData_st->bytes_xfered += pSG_st->length;

    for(i=1; i<pData_st->sg_len; i++)
    {
		eMMC_FCIE_ClearEvents_Reg0();

        pSG_st = &(pData_st->sg[i]);
        dmaaddr = sg_dma_address(pSG_st);
        dmalen = sg_dma_len(pSG_st);

        eMMC_FCIE_MIU0_MIU1_SEL(dmaaddr);

		REG_FCIE_W(FCIE_JOB_BL_CNT,(dmalen/512));
        REG_FCIE_W(FCIE_SDIO_ADDR0,(((u32)dmaaddr) & 0xFFFF));
        REG_FCIE_W(FCIE_SDIO_ADDR1,(((u32)dmaaddr) >> 16));
		REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

        #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
		REG_FCIE_SETBIT(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
        #endif
		REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN);

		if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_MIU_LAST_DONE,
			eMMC_READ_WAIT_TIME) != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: r timeout \n");
			g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
			err = eMMC_ST_ERR_TIMEOUT_MIULASTDONE;
			goto dma_read_end;
        }
		pData_st->bytes_xfered += pSG_st->length;


		// -----------------------------------
		#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
        g_eMMCDrv.u64_CNT_TotalRBlk += (dmalen / 512);
		g_eMMCDrv.u32_RBlk_tmp += (dmalen / 512);
		#endif
    }

    dma_read_end:

	#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
	if(g_eMMCDrv.u32_RBlk_tmp < 0x200)
		g_eMMCDrv.au32_CNT_MinRBlk[g_eMMCDrv.u32_RBlk_tmp]++;
	else if(g_eMMCDrv.u32_RBlk_tmp > g_eMMCDrv.u32_CNT_MaxRBlk)
		g_eMMCDrv.u32_CNT_MaxRBlk = g_eMMCDrv.u32_RBlk_tmp;
	else if(g_eMMCDrv.u32_RBlk_tmp < g_eMMCDrv.u32_CNT_MinRBlk)
		g_eMMCDrv.u32_CNT_MinRBlk = g_eMMCDrv.u32_RBlk_tmp;

	g_eMMCDrv.u32_Addr_RLast += g_eMMCDrv.u32_RBlk_tmp;
	g_eMMCDrv.u32_RBlk_tmp = 0;
	#endif

    // -----------------------------------
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if(!pData_st->host_cookie)
    {
        dma_unmap_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, (int)pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    }
    #else
    dma_unmap_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

	if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    if( !err ) // success
    {
        mstar_mci_completed_command(pSstarHost_st); // copy back rsp for cmd with data

        if(
            pSstarHost_st->request->stop
            #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
            && !pSstarHost_st->request->sbc
            #endif
        )
        {
            mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->stop);
        }
        else
        {
			if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
	        {
		        //eMMC_debug(0,1,"eMMC: restore IF\n");
		        eMMC_FCIE_ErrHandler_RestoreClk();
	        }
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            mmc_request_done(pSstarHost_st->mmc, pSstarHost_st->request);
        }
    }

    return err;
}


static U32 mstar_mci_dma_write(struct mstar_mci_host *pSstarHost_st)
{
    struct mmc_command	*pCmd_st = 0;
    struct mmc_data		*pData_st = 0;
    struct scatterlist	*pSG_st = 0;
    int i;
    //int sg_count;
    u32 dmalen = 0, split_len = 0, split_unit_len = 0, split_up_len = 0;
    dma_addr_t dmaaddr = 0;
    U32 err = eMMC_ST_SUCCESS;

	if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
	{
		REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_W : TOGGLE_CNT_256_CLK_W);
		REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
	}
    pCmd_st = pSstarHost_st->cmd;
    pData_st = pCmd_st->data;
    split_unit_len = wr_seg_size << (10 - eMMC_SECTOR_BYTECNT_BITS);
    split_up_len = wr_split_threshold << (10 - eMMC_SECTOR_BYTECNT_BITS);

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    mstar_mci_pre_dma_transfer(pSstarHost_st, pData_st, NULL);
    #else
    dma_map_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    for(i=0; i<pData_st->sg_len; i++)
    {
        pSG_st = &(pData_st->sg[i]);
        dmaaddr = sg_dma_address(pSG_st);
		eMMC_FCIE_MIU0_MIU1_SEL(dmaaddr);
        dmalen = sg_dma_len(pSG_st)>>eMMC_SECTOR_BYTECNT_BITS;

        do{
		    eMMC_FCIE_ClearEvents_Reg0();
		    if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
		    {
			    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
			    eMMC_FCIE_ErrHandler_Stop();
		    }
			#if defined(eMMC_BURST_LEN_AUTOCFG) && eMMC_BURST_LEN_AUTOCFG
            split_len =
				dmalen > g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen ?
				g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen : dmalen;
			split_len =
				split_len == g_eMMCDrv.BurstWriteLen_t.u16_WorstBrustLen ?
				split_len>>1 : split_len;
			#else
            split_len = dmalen;
			#endif

            if(split_unit_len && (!split_up_len || dmalen <= split_up_len))
                split_len = dmalen > split_unit_len ?
                                split_unit_len : dmalen;

            REG_FCIE_W(FCIE_JOB_BL_CNT, split_len);
            REG_FCIE_W(FCIE_SDIO_ADDR0, (dmaaddr & 0xFFFF));
            REG_FCIE_W(FCIE_SDIO_ADDR1, (dmaaddr >> 16));
            REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    		if(0==i)
    			eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);
            REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

            #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    		REG_FCIE_SETBIT(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);
            #endif

            eMMC_CheckPowerCut();

    		REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);

    		if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_CARD_DMA_END,
    			eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: w timeout \n");
    			g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
                err = eMMC_ST_ERR_TIMEOUT_CARDDMAEND;
                goto dma_write_end;
            }

			dmalen -= split_len;
			dmaaddr += split_len << eMMC_SECTOR_BYTECNT_BITS;

		    // -----------------------------------
		    #if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
            g_eMMCDrv.u64_CNT_TotalWBlk += (split_len / 512);
		    g_eMMCDrv.u32_WBlk_tmp += (split_len / 512);
		    #endif

        }while(dmalen > 0);

        pData_st->bytes_xfered += pSG_st->length;
    }

dma_write_end:
	#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
	if(g_eMMCDrv.u32_WBlk_tmp < 0x200)
		g_eMMCDrv.au32_CNT_MinWBlk[g_eMMCDrv.u32_WBlk_tmp]++;
	else if(g_eMMCDrv.u32_WBlk_tmp > g_eMMCDrv.u32_CNT_MaxWBlk)
		g_eMMCDrv.u32_CNT_MaxWBlk = g_eMMCDrv.u32_WBlk_tmp;
	else if(g_eMMCDrv.u32_WBlk_tmp < g_eMMCDrv.u32_CNT_MinWBlk)
		g_eMMCDrv.u32_CNT_MinWBlk = g_eMMCDrv.u32_WBlk_tmp;

	g_eMMCDrv.u32_Addr_WLast += g_eMMCDrv.u32_WBlk_tmp;
	g_eMMCDrv.u32_WBlk_tmp = 0;
	#endif

	// -----------------------------------
	#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if(!pData_st->host_cookie)
    {
        dma_unmap_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, (int)pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    }
    #else
    dma_unmap_sg(mmc_dev(pSstarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    if( !err )
    {
        mstar_mci_completed_command(pSstarHost_st); // copy back rsp for cmd with data

        if(
            pSstarHost_st->request->stop
            #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
            && !pSstarHost_st->request->sbc
            #endif
        )
        {
            mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->stop);
        }
        else
        {
			if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
	        {
		        //eMMC_debug(0,1,"eMMC: restore IF\n");
		        eMMC_FCIE_ErrHandler_RestoreClk();
	        }
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            mmc_request_done(pSstarHost_st->mmc, pSstarHost_st->request);
        }
    }

    return err;
}

static void mstar_mci_completed_command(struct mstar_mci_host *pSstarHost_st)
{
    /* Define Local Variables */
    u16 u16_st, u16_i;
	u8 *pTemp;
	struct mmc_command *pCmd_st = pSstarHost_st->cmd;
	static u32 u32_retry_cnt=0, u32_run_cnt=0;

	u32_run_cnt++;
	// ----------------------------------
	// retrun response from FCIE to mmc driver
    pTemp = (u8*)&(pCmd_st->resp[0]);
    for(u16_i=0; u16_i < 15; u16_i++)
    {
        pTemp[(3 - (u16_i % 4)) + (4 * (u16_i / 4))] =
            (u8)(REG_FCIE(FCIE1_BASE+(((u16_i+1)/2)*4)) >> (8*((u16_i+1)%2)));
    }
	#if 0
	eMMC_debug(0,0,"------------------\n");
	eMMC_debug(0,1,"resp[0]: %08Xh\n", pCmd_st->resp[0]);
	eMMC_debug(0,1,"CIFC: %04Xh %04Xh %04Xh \n", REG_FCIE(FCIE_CIFC_BASE_ADDR),
		REG_FCIE(FCIE_CIFC_BASE_ADDR+4), REG_FCIE(FCIE_CIFC_BASE_ADDR+8));
	//eMMC_dump_mem(pTemp, 0x10);
	eMMC_debug(0,0,"------------------\n");
    #endif

	// ----------------------------------
    u16_st = REG_FCIE(FCIE_SD_STATUS);
	if((u16_st & BIT_SD_FCIE_ERR_FLAGS) /*|| mstar_SD_CardChange()*/
		|| (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))// || ((mmc_resp_type(pCmd_st)==MMC_RSP_R1) && (pCmd_st->resp[0]&eMMC_ERR_R1_NEED_RETRY)))
	{
		g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_ERROR_RETRY;

        if((u16_st & BIT_SD_RSP_CRC_ERR) && !(mmc_resp_type(pCmd_st) & MMC_RSP_CRC))
        {
            pCmd_st->error = 0;
			u32_retry_cnt = 0;
        }
        else
        {
			#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
			    "eMMC Err: ST:%Xh, CMD:%u, retry: %u, flag: %Xh, R1 err: %Xh\n",
				u16_st, pCmd_st->opcode, u32_retry_cnt, g_eMMCDrv.u32_DrvFlag,
				pCmd_st->resp[0]&eMMC_ERR_R1_NEED_RETRY);

			u32_ok_cnt = 0;
			if(u32_retry_cnt++ >= MCI_RETRY_CNT_CRC_ERR)
			    eMMC_FCIE_ErrHandler_Stop(); // fatal error

			if(25==pCmd_st->opcode || 12==pCmd_st->opcode)
				eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

			eMMC_hw_timer_sleep(1);
			eMMC_FCIE_ErrHandler_ReInit();

			if(0 != pCmd_st->data)
			{
				eMMC_FCIE_ErrHandler_Retry(); // slow dwon clock
				mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->cmd);
			}
			else // for CMD12
			{
				eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: no data, just reset eMMC. \n");
				REG_FCIE_W(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);
				mstar_mci_completed_command_FromRAM(pSstarHost_st);
				return;
			}

			if(0==u32_retry_cnt)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: retry ok \n");
				return;
			}

			//-----------------------------------------
			#else
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: ST:%Xh, CMD:%u, retry:%u \n",
				u16_st, pCmd_st->opcode, pCmd_st->retries);
			#endif

			// should be trivial
			if(u16_st & (BIT_SD_RSP_TIMEOUT | BIT_SD_W_FAIL))
                pCmd_st->error = -ETIMEDOUT;
            else if(u16_st & (BIT_SD_RSP_CRC_ERR | BIT_SD_R_CRC_ERR | BIT_SD_W_CRC_ERR))
            	pCmd_st->error = -EILSEQ;
            else
                pCmd_st->error = -EIO;
        }
    }
    else
    {
        pCmd_st->error = 0;

		u32_retry_cnt = 0;
    }

    if((pCmd_st->opcode == 17) || (pCmd_st->opcode == 18) //read
        ||(pCmd_st->opcode == 24) || (pCmd_st->opcode == 25) //write
        ||(pCmd_st->opcode == 12))  //stop transmission
    {
       if(pCmd_st->resp[0] & eMMC_ERR_R1_31_0)
       {
         pCmd_st->error |= -EIO;
         if(pCmd_st->opcode == 12)
             eMMC_debug(0,0, "eMMC Warn: CMD12 R1 error: %Xh \n",
                 pCmd_st->resp[0]);
         else
             eMMC_debug(0,0, "eMMC Warn: CMD%u R1 error: %Xh, arg %08x, blocks %08x\n",
                 pCmd_st->opcode, pCmd_st->resp[0], pCmd_st->arg, pCmd_st->data->blocks);
	   }
    }

	#if 0
	if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt)
	{
		eMMC_debug(0,1,"eMMC: restore IF\n");
		eMMC_FCIE_ErrHandler_RestoreClk();
	}
	#endif

    REG_FCIE_W(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);
}

#define WAIT_D0H_POLLING_TIME    HW_TIMER_DELAY_100us
u32 mstar_mci_WaitD0High(u32 u32_us)
{
    #if defined(ENABLE_FCIE_HW_BUSY_CHECK)&&ENABLE_FCIE_HW_BUSY_CHECK

    REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_SD_BUSY_DET_ON);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    // enable busy int
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, BIT_SD_BUSY_END);
	#endif
    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_SD_BUSY_END,u32_us) != eMMC_ST_SUCCESS)
    {
        return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
    }
	return eMMC_ST_SUCCESS;
	#else
	u32 u32_cnt, u32_wait;

	for(u32_cnt=0; u32_cnt<u32_us; u32_cnt+=WAIT_D0H_POLLING_TIME)
	{
		u32_wait = eMMC_FCIE_WaitD0High_Ex(WAIT_D0H_POLLING_TIME);
		if(u32_wait < WAIT_D0H_POLLING_TIME)
		{
			#if 0
			if(u32_cnt + u32_wait)
				printk("eMMC wait d0: %u us\n", u32_cnt+u32_wait);
			#endif
		    return eMMC_ST_SUCCESS;
		}

		//if(u32_cnt > HW_TIMER_DELAY_1ms)
		{
			//msleep(1);
			  //schedule_hrtimeout is more precise and can reduce idle time of emmc
        {
            ktime_t expires = ktime_add_ns(ktime_get(), 1000 * 1000);
            set_current_state(TASK_UNINTERRUPTIBLE);
            schedule_hrtimeout(&expires, HRTIMER_MODE_ABS);
        }
			u32_cnt += HW_TIMER_DELAY_1ms;
		}
	}
	return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
    #endif
}

#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static void mstar_mci_send_data(struct work_struct *work)
{
    struct mstar_mci_host *pSstarHost_st = container_of(work, struct mstar_mci_host, async_work);
    struct mmc_command *pCmd_st;
    struct mmc_data *pData_st;
    static u8 u8_retry_data=0;
    U32 err = eMMC_ST_SUCCESS;

    if( !pSstarHost_st )
        return;

    pCmd_st = pSstarHost_st->cmd;

    if( !pCmd_st )
        return;

    pData_st = pCmd_st->data;

    if( !pData_st )
        return;

	if(pData_st->flags & MMC_DATA_WRITE)
    {
        err = mstar_mci_dma_write(pSstarHost_st);
    }
    else if(pData_st->flags & MMC_DATA_READ)
    {
        err = mstar_mci_post_dma_read(pSstarHost_st);
    }

    if( err )
    {
		u32_ok_cnt = 0;

        if(pData_st->flags & MMC_DATA_WRITE)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: w, cmd.%u arg.%Xh, ST: %Xh or TO\n",
                pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
        }
        else if(pData_st->flags & MMC_DATA_READ)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: r, cmd.%u arg.%Xh, ST: %Xh or TO \n",
                pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
        }

        if(u8_retry_data < MCI_RETRY_CNT_CMD_TO)
        {
            u8_retry_data++;
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->cmd);
        }
        else
        {
            eMMC_FCIE_ErrHandler_Stop();
        }

    }
    else
    {
        if(u8_retry_data)
            eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: data retry ok \n");

		u8_retry_data =0;
    }
}
#endif

static void mstar_mci_send_command
(
	struct mstar_mci_host *pSstarHost_st, struct mmc_command *pCmd_st
)
{
    struct mmc_data *pData_st;
    u32 u32_mie_int=0, u32_sd_ctl=0, u32_sd_mode;
	u8  u8_retry_cmd=0, u8_retry_D0H=0;

    #if !(defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO)
    u8  u8_retry_data=0;
    u32 err = 0;
	#endif

	LABEL_SEND_CMD:
	g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_ERROR_RETRY;
	u32_sd_mode = g_eMMCDrv.u16_Reg10_Mode;
	pSstarHost_st->cmd = pCmd_st;
	pData_st = pCmd_st->data;

	eMMC_FCIE_ClearEvents();
	if(12!=pCmd_st->opcode)
		if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(HW_TIMER_DELAY_500ms))
		{
			u32_ok_cnt = 0;
			eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: retry wait D0 H.\n");
			u8_retry_D0H++;
			if(u8_retry_D0H < 10)
			{
				eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
				eMMC_pads_switch(g_eMMCDrv.u8_PadType);
				goto LABEL_SEND_CMD;
			}
			else
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: wait D0 H timeout\n");
				eMMC_FCIE_ErrHandler_Stop();
			}
		}
	if(u8_retry_D0H)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: wait D0 H [ok]\n");
		u8_retry_D0H = 0;
	}

    if(pData_st)
    {
		pData_st->bytes_xfered = 0;
        REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE|(u16)pData_st->blksz);

        if (pData_st->flags & MMC_DATA_READ)
        {
            u32_sd_ctl |= BIT_SD_DAT_EN;

            // enable stoping read clock when using scatter list DMA
            //if(pData_st->sg_len > 1)
            if(pCmd_st->opcode == 18)
            	u32_sd_mode |= BIT_SD_DMA_R_CLK_STOP;
            else
            	u32_sd_mode &= ~BIT_SD_DMA_R_CLK_STOP;

			mstar_mci_pre_dma_read(pSstarHost_st);
        }
    }

    u32_sd_ctl |= BIT_SD_CMD_EN;
    u32_mie_int |= BIT_SD_CMD_END;
    REG_FCIE_W(FCIE_CIFC_ADDR(0), (((pCmd_st->arg >> 24)<<8) | (0x40|pCmd_st->opcode)));
    REG_FCIE_W(FCIE_CIFC_ADDR(1), ((pCmd_st->arg & 0xFF00) | ((pCmd_st->arg>>16)&0xFF)));
    REG_FCIE_W(FCIE_CIFC_ADDR(2), (pCmd_st->arg & 0xFF));

    if(mmc_resp_type(pCmd_st) == MMC_RSP_NONE)
    {
        u32_sd_ctl &= ~BIT_SD_RSP_EN;
        REG_FCIE_W(FCIE_RSP_SIZE, 0);
    }
    else
    {
		u32_sd_ctl |= BIT_SD_RSP_EN;
        if(mmc_resp_type(pCmd_st) == MMC_RSP_R2)
        {
            u32_sd_ctl |= BIT_SD_RSPR2_EN;
            REG_FCIE_W(FCIE_RSP_SIZE, 16); /* (136-8)/8 */
        }
        else
        {
            REG_FCIE_W(FCIE_RSP_SIZE, 5); /*(48-8)/8 */
        }
    }

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
	//eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC: INT_EN: %Xh\n", u32_mie_int);
	REG_FCIE_W(FCIE_MIE_INT_EN, u32_mie_int);
	#endif

	#if 0
	//eMMC_debug(0,0,"\n");
	eMMC_debug(0,0,"cmd:%u, arg:%Xh, buf:%Xh, ST:%Xh, mode:%Xh, ctrl:%Xh \n",
		pCmd_st->opcode, pCmd_st->arg, (u32)pData_st,
		REG_FCIE(FCIE_SD_STATUS), u32_sd_mode, u32_sd_ctl);
	//eMMC_debug(0,0,"cmd:%u arg:%Xh\n", pCmd_st->opcode, pCmd_st->arg);
	//while(1);
	#endif

	#if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
    switch(pCmd_st->opcode)
    {
		case 17:
		case 18:
			if(18 == pCmd_st->opcode)
				g_eMMCDrv.u32_CNT_CMD18++;
			else if(17 == pCmd_st->opcode)
				g_eMMCDrv.u32_CNT_CMD17++;

		    if(g_eMMCDrv.u32_Addr_RLast == pCmd_st->arg)
				g_eMMCDrv.u32_Addr_RHitCnt++;
			else
				g_eMMCDrv.u32_Addr_RLast = pCmd_st->arg;

		case 24:
		case 25:
			if(25 == pCmd_st->opcode)
				g_eMMCDrv.u32_CNT_CMD25++;
			else if(24 == pCmd_st->opcode)
				g_eMMCDrv.u32_CNT_CMD24++;

		    if(g_eMMCDrv.u32_Addr_WLast == pCmd_st->arg)
				g_eMMCDrv.u32_Addr_WHitCnt++;
			else
				g_eMMCDrv.u32_Addr_WLast = pCmd_st->arg;
    }

	#endif

	// -----------------------------------
	if((pCmd_st->opcode==12)||(pCmd_st->opcode==24)||(pCmd_st->opcode==25))
	{
	    eMMC_CheckPowerCut();
    }

	REG_FCIE_W(FCIE_SD_MODE, u32_sd_mode);
    REG_FCIE_W(FCIE_SD_CTRL, u32_sd_ctl);
    // [FIXME]: retry and timing, and omre...
    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_SD_CMD_END,
		HW_TIMER_DELAY_1s) != eMMC_ST_SUCCESS ||
		(REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)))// || 0==u8_retry_cmd)
    {
        // ------------------------------------
		#if !(defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM)
		if(NULL==pData_st) // no data, no retry
		{
			mstar_mci_completed_command(pSstarHost_st);
			return;
		}
		#endif

		// ------------------------------------
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
					"eMMC Err: cmd.%u arg.%Xh St: %Xh, retry %u \n",
					pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS), u8_retry_cmd);
		if(u8_retry_cmd < MCI_RETRY_CNT_CMD_TO)
		{
			u8_retry_cmd++;
			if(12==pCmd_st->opcode)
		    {
				if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
		        {
			        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
			        eMMC_FCIE_ErrHandler_Stop();
		        }
				eMMC_FCIE_ErrHandler_ReInit();
				eMMC_FCIE_ErrHandler_Retry();
				#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
				mstar_mci_completed_command_FromRAM(pSstarHost_st);
				#endif
				return;
		    }

			if(25==pCmd_st->opcode)
				eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

			eMMC_FCIE_ErrHandler_ReInit();
			eMMC_FCIE_ErrHandler_Retry();
			u32_ok_cnt = 0;
			goto LABEL_SEND_CMD;
		}

		eMMC_FCIE_ErrHandler_Stop();
    }

	if(u8_retry_cmd)
		eMMC_debug(0,0,"eMMC: CMD retry ok\n");

	if(pData_st)
    {
        #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
        schedule_work(&pSstarHost_st->async_work);
        #else
        if(pData_st->flags & MMC_DATA_WRITE)
        {
            err = mstar_mci_dma_write(pSstarHost_st);
        }
        else if(pData_st->flags & MMC_DATA_READ)
        {
            err = mstar_mci_post_dma_read(pSstarHost_st);
        }

        if( err )
        {
            if(pData_st->flags & MMC_DATA_WRITE)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: w timeout, cmd.%u arg.%Xh, ST: %Xh \n",
                       pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
            }
            else if(pData_st->flags & MMC_DATA_READ)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: r timeout, cmd.%u arg.%Xh, ST: %Xh \n",
                       pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
            }

            if(u8_retry_data < MCI_RETRY_CNT_CMD_TO)
            {
                u8_retry_data++;
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
				u32_ok_cnt = 0;
                goto LABEL_SEND_CMD;
            }

            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {
            if(u8_retry_data)
                eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: data retry ok \n");
        }
        #endif
    }
    else
    {
		mstar_mci_completed_command(pSstarHost_st); // copy back rsp for cmd without data

        if( pCmd_st->opcode == 23 )
            mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->cmd); // CMD18 or CMD25
        else
        {
			if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
	        {
		        //eMMC_debug(0,1,"eMMC: restore IF\n");
		        eMMC_FCIE_ErrHandler_RestoreClk();
	        }
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            mmc_request_done(pSstarHost_st->mmc, pSstarHost_st->request);
        }
    }

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static int mstar_mci_pre_dma_transfer(struct mstar_mci_host *host, struct mmc_data *data, struct mstar_mci_host_next *next)
{
	int dma_len;

    /* Check if next job is already prepared */
    if (next || (!next && data->host_cookie != host->next_data.cookie))
    {
        dma_len = dma_map_sg(mmc_dev(host->mmc),
                             data->sg,
                             data->sg_len,
                             mstar_mci_get_dma_dir(data));
    }
    else
    {
        dma_len = host->next_data.dma_len;
        host->next_data.dma_len = 0;
    }

    if (dma_len == 0)
        return -EINVAL;

    if (next)
    {
        next->dma_len = dma_len;
        data->host_cookie = ++next->cookie < 0 ? 1 : next->cookie;
    }

	return 0;
}


static void mstar_mci_pre_req(struct mmc_host *mmc, struct mmc_request *mrq, bool is_first_req)
{
	struct mstar_mci_host *host = mmc_priv(mmc);

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "\n");

    if( mrq->data->host_cookie )
    {
        mrq->data->host_cookie = 0;
        return;
    }

    if (mstar_mci_pre_dma_transfer(host, mrq->data, &host->next_data))
        mrq->data->host_cookie = 0;

}

static void mstar_mci_post_req(struct mmc_host *mmc, struct mmc_request *mrq, int err)
{
	struct mstar_mci_host *host = mmc_priv(mmc);
    struct mmc_data *data = mrq->data;

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "\n");

    if (data->host_cookie)
    {
        dma_unmap_sg(mmc_dev(host->mmc),
                     data->sg,
                     data->sg_len,
                     mstar_mci_get_dma_dir(data));
    }

    data->host_cookie = 0;
}
#endif
#endif

static void mstar_mci_request(struct mmc_host *pMMCHost_st, struct mmc_request *pMRQ_st)
{
    struct mstar_mci_host *pSstarHost_st;

	eMMC_LockFCIE((U8*)__FUNCTION__);
    pSstarHost_st = mmc_priv(pMMCHost_st);
    if (!pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMMCHost_st is NULL \n");
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
		return;
    }
    if (!pMRQ_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMRQ_st is NULL \n");
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
		return;
    }

	pSstarHost_st->request = pMRQ_st;

	// ---------------------------------------------
	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM

	if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_INIT_DONE))
	{
		eMMC_Init_Device();
	}

      #ifdef CONFIG_MP_EMMC_TRIM
    switch(pSstarHost_st->request->cmd->opcode)
    {
        case 35:
            eMMC_CMD35_CMD36(pSstarHost_st->request->cmd->arg, 35);
            break;
        case 36:
            eMMC_CMD35_CMD36(pSstarHost_st->request->cmd->arg, 36);
            break;
        case 38:
            eMMC_CMD38();
            break;
        default:
            break;
    }
      #endif

	if(NULL == pSstarHost_st->request->cmd->data && 6 != pSstarHost_st->request->cmd->opcode)
	{
		pSstarHost_st->cmd = pSstarHost_st->request->cmd;
		mstar_mci_completed_command_FromRAM(pSstarHost_st);
		return;
	}
	if(6 == pSstarHost_st->request->cmd->opcode &&
		(((pSstarHost_st->request->cmd->arg & 0xff0000) == 0xB70000)||
		 ((pSstarHost_st->request->cmd->arg & 0xff0000) == 0xB90000)))
	{
		pSstarHost_st->cmd = pSstarHost_st->request->cmd;
		mstar_mci_completed_command_FromRAM(pSstarHost_st);
		return;
	}
	#endif
	// ---------------------------------------------

    #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
	if( pSstarHost_st->request->sbc)
        mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->sbc);
	else
    #endif
        mstar_mci_send_command(pSstarHost_st, pSstarHost_st->request->cmd);
}


#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
static void mstar_mci_completed_command_FromRAM(struct mstar_mci_host *pSstarHost_st)
{
    struct mmc_command *pCmd_st = pSstarHost_st->cmd;
	u16 au16_cifc[32], u16_i;
	u8 *pTemp;

	#if 0
	eMMC_debug(0,1,"\n");
	eMMC_debug(0,1,"cmd:%u, arg:%Xh\n", pCmd_st->opcode, pCmd_st->arg);
	#endif

	if(eMMC_ST_SUCCESS != eMMC_ReturnRsp((u8*)au16_cifc, (u8)pCmd_st->opcode))
	{
		pCmd_st->error = -ETIMEDOUT;
		//eMMC_debug(0,0,"eMMC Info: no rsp\n");
	}
	else
	{
		pCmd_st->error = 0;
		pTemp = (u8*)&(pCmd_st->resp[0]);
        for(u16_i=0; u16_i < 15; u16_i++)
        {
            pTemp[(3-(u16_i%4)) + 4*(u16_i/4)] =
                (u8)(au16_cifc[(u16_i+1)/2] >> 8*((u16_i+1)%2));
        }
		#if 0
	    eMMC_debug(0,1,"------------------\n");
		eMMC_dump_mem((u8*)&(pCmd_st->resp[0]), 0x10);
		eMMC_debug(0,1,"------------------\n");
		#endif
	}

	eMMC_UnlockFCIE((U8*)__FUNCTION__);
    mmc_request_done(pSstarHost_st->mmc, pSstarHost_st->request);
}
#endif

static void mstar_mci_set_ios(struct mmc_host *pMMCHost_st, struct mmc_ios *pIOS_st)
{
    /* Define Local Variables */
    struct mstar_mci_host *pSstarHost_st;
	static u8 u8_IfLock=0;

	if(0 == (REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE))
	{
		eMMC_LockFCIE((U8*)__FUNCTION__);
		u8_IfLock = 1;
		//eMMC_debug(0,1,"lock\n");
	}

	pSstarHost_st = mmc_priv(pMMCHost_st);
    if (!pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMMCHost_st is NULL \n");
        goto LABEL_END;
    }
    if (!pIOS_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pIOS_st is NULL \n");
        goto LABEL_END;
    }

	//eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "eMMC: clock: %u, bus_width %Xh \n",
	//	pIOS_st->clock, pIOS_st->bus_width);

	// ----------------------------------
	if (pIOS_st->clock == 0)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_HIGH, 1, "eMMC Warn: disable clk \n");
        eMMC_clock_gating();
    }
	// ----------------------------------
	#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    else
    {   eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
	}
	pSstarHost_st->sd_mod = (BIT_SD_DEFAULT_MODE_REG & ~BIT_SD_DATA_WIDTH_MASK) | BIT_SD_DATA_WIDTH_8;
	#else
	// ----------------------------------
    else
    {
		pSstarHost_st->sd_mod = BIT_SD_DEFAULT_MODE_REG;

        if(pIOS_st->clock > CLK_400KHz)
		{
			eMMC_clock_setting(FCIE_DEFAULT_CLK);
		}
		else
		{	eMMC_clock_setting(FCIE_SLOWEST_CLK);
		}
    }

	if (pIOS_st->bus_width == MMC_BUS_WIDTH_8)
    {
        g_eMMCDrv.u16_Reg10_Mode = (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK) | BIT_SD_DATA_WIDTH_8;
    }
    else if (pIOS_st->bus_width == MMC_BUS_WIDTH_4)
    {
        g_eMMCDrv.u16_Reg10_Mode = (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK) | BIT_SD_DATA_WIDTH_4;
    }
    else
    {   g_eMMCDrv.u16_Reg10_Mode = (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK);
    }
	#endif

	LABEL_END:
	if(u8_IfLock)
	{
		u8_IfLock = 0;
		eMMC_UnlockFCIE((U8*)__FUNCTION__);
		//eMMC_debug(0,1,"unlock\n");
	}
}

static s32 mstar_mci_get_ro(struct mmc_host *pMMCHost_st)
{
    s32 read_only;

	read_only = 0;
    if(!pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMMCHost_st is NULL \n");
        read_only = -EINVAL;
    }

    return read_only;
}

//=======================================================================
static void mstar_mci_enable(struct mstar_mci_host *pSstarHost_st)
{
	u32 u32_err;

	memset((void*)&g_eMMCDrv, '\0', sizeof(eMMC_DRIVER));

	eMMC_PlatformInit();

	g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
	g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;

    u32_err = eMMC_FCIE_Init();
	if(eMMC_ST_SUCCESS != u32_err)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    "eMMC Err: eMMC_FCIE_Init fail: %Xh \n", u32_err);
}

static void mstar_mci_disable(struct mstar_mci_host *pSstarHost_st)
{
	u32 u32_err;

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"\n");

    u32_err = eMMC_FCIE_Reset();
	if(eMMC_ST_SUCCESS != u32_err)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
		    "eMMC Err: eMMC_FCIE_Reset fail: %Xh\n", u32_err);
	eMMC_clock_gating();
}

static ssize_t fcie_pwrsvr_gpio_trigger_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_trigger);
}

static ssize_t fcie_pwrsvr_gpio_trigger_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_pwrsvr_gpio_trigger = 0;
	//struct mmc_card *card = mmc_dev_to_card(dev);

	if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_trigger))
		return -EINVAL;
    if(u32_pwrsvr_gpio_trigger > 1)
        return -EINVAL;
	gu32_pwrsvr_gpio_trigger = u32_pwrsvr_gpio_trigger;
	return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_trigger,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_trigger_show,
            fcie_pwrsvr_gpio_trigger_store);

static ssize_t fcie_pwrsvr_gpio_bit_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_bit);
}

static ssize_t fcie_pwrsvr_gpio_bit_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_pwrsvr_gpio_bit = 0;
	//struct mmc_card *card = mmc_dev_to_card(dev);

	if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_bit))
		return -EINVAL;
	if(u32_pwrsvr_gpio_bit)
    {
        if(u32_pwrsvr_gpio_bit > 0xF)
            return -EINVAL;
		gu32_pwrsvr_gpio_bit = u32_pwrsvr_gpio_bit;
    }
	return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_bit,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_bit_show,
            fcie_pwrsvr_gpio_bit_store);

static ssize_t fcie_pwrsvr_gpio_addr_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%X\n", gu32_pwrsvr_gpio_addr);
}

static ssize_t fcie_pwrsvr_gpio_addr_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_pwrsvr_gpio_addr = 0;
	//struct mmc_card *card = mmc_dev_to_card(dev);

	if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_addr))
		return -EINVAL;
	if(u32_pwrsvr_gpio_addr)
		gu32_pwrsvr_gpio_addr = u32_pwrsvr_gpio_addr;
	return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_addr,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_addr_show,
            fcie_pwrsvr_gpio_addr_store);

static ssize_t fcie_pwrsvr_gpio_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_enable);
}

static ssize_t fcie_pwrsvr_gpio_enable_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_pwrsvr_gpio_enable = 0;
	//struct mmc_card *card = mmc_dev_to_card(dev);

    if(u8_enable_sar5)
        return count;
	if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_enable))
		return -EINVAL;
	if(u32_pwrsvr_gpio_enable)
		gu32_pwrsvr_gpio_enable = u32_pwrsvr_gpio_enable;
	return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_enable,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_enable_show,
            fcie_pwrsvr_gpio_enable_store);


static ssize_t emmc_sanitize_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", gu32_emmc_sanitize);
}

static ssize_t emmc_sanitize_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_temp = 0;

	if(kstrtoul(buf, 0, &u32_temp))
		return -EINVAL;

	gu32_emmc_sanitize = u32_temp;
	//printk("%Xh\n", gu32_emmc_sanitize);

	if(gu32_emmc_sanitize)
	{
		eMMC_LockFCIE((U8*)__FUNCTION__);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: santizing ...\n");
		eMMC_Sanitize(0xAA);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: done\n");
		eMMC_UnlockFCIE((U8*)__FUNCTION__);
	}

	return count;
}

DEVICE_ATTR(emmc_sanitize,
            S_IRUSR | S_IWUSR,
            emmc_sanitize_show,
            emmc_sanitize_store);

static struct attribute *mstar_mci_attr[] =
{
    &dev_attr_fcie_pwrsvr_gpio_enable.attr,
    &dev_attr_fcie_pwrsvr_gpio_addr.attr,
    &dev_attr_fcie_pwrsvr_gpio_bit.attr,
    &dev_attr_fcie_pwrsvr_gpio_trigger.attr,
    &dev_attr_emmc_sanitize.attr,
    NULL,
};

static struct attribute_group mstar_mci_attr_grp =
{
    .attrs = mstar_mci_attr,
};

static s32 mstar_mci_probe(struct platform_device *pDev_st)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st;
    struct mstar_mci_host *pSstarHost_st;
    s32 s32_ret;

	#if IF_FCIE_SHARE_IP && defined(CONFIG_MSTAR_SDMMC)
    eMMC_debug(0,0,"eMMC: has SD 0722\n");
	#else
	eMMC_debug(0,0,"eMMC: no SD 0722\n");
	#endif

	// --------------------------------
    if (!pDev_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pDev_st is NULL \n");
        s32_ret = -EINVAL;
		goto LABEL_END;
    }

	pMMCHost_st = 0;
	pSstarHost_st = 0;
	s32_ret = 0;
	g_eMMCDrv.u8_PadType = FCIE_DEFAULT_PAD;
	g_eMMCDrv.u16_ClkRegVal = FCIE_SLOWEST_CLK;
	// --------------------------------
	eMMC_LockFCIE((U8*)__FUNCTION__);
	mstar_mci_enable(pSstarHost_st);
	eMMC_UnlockFCIE((U8*)__FUNCTION__);

    pMMCHost_st = mmc_alloc_host(sizeof(struct mstar_mci_host), &pDev_st->dev);
    if (!pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc_alloc_host fail \n");
        s32_ret = -ENOMEM;
		goto LABEL_END;
    }

    pMMCHost_st->ops = &sg_mstar_mci_ops;
	// [FIXME]->
    pMMCHost_st->f_min = CLK_400KHz;
	pMMCHost_st->f_max = CLK_200MHz;
    pMMCHost_st->ocr_avail = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | \
						 MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	pMMCHost_st->max_blk_count  = BIT_SD_JOB_BLK_CNT_MASK;
    pMMCHost_st->max_blk_size   = 512; /* sector */
    pMMCHost_st->max_req_size   = pMMCHost_st->max_blk_count * pMMCHost_st->max_blk_size;

    pMMCHost_st->max_seg_size   = pMMCHost_st->max_req_size;
	#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,20)
    pMMCHost_st->max_phys_segs  = 511;
    pMMCHost_st->max_hw_segs    = 511;
	#else
	pMMCHost_st->max_segs       = 511; // (2^12-1)*512/4096
    #endif
	//---------------------------------------
    pSstarHost_st           = mmc_priv(pMMCHost_st);
    pSstarHost_st->mmc      = pMMCHost_st;
	//---------------------------------------

    pMMCHost_st->caps = MMC_CAP_8_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_NONREMOVABLE;
    #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
    pMMCHost_st->caps |= MMC_CAP_CMD23;
    #endif

    #ifdef CONFIG_MP_EMMC_TRIM
    pMMCHost_st->caps |= MMC_CAP_ERASE;
    #endif

    #ifdef CONFIG_MP_EMMC_CACHE
    pMMCHost_st->caps2 |= MMC_CAP2_CACHE_CTRL;
    #endif
	#if LINUX_VERSION_CODE > KERNEL_VERSION(3,1,10)
	pMMCHost_st->caps2 |= MMC_CAP2_NO_SLEEP_CMD;
    #endif
    pSstarHost_st->baseaddr = (void __iomem *)FCIE0_BASE;
	// <-[FIXME]

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    pSstarHost_st->next_data.cookie = 1;
    INIT_WORK(&pSstarHost_st->async_work, mstar_mci_send_data);
    #endif
    #endif

    mmc_add_host(pMMCHost_st);
    platform_set_drvdata(pDev_st, pMMCHost_st);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    s32_ret = request_irq(E_IRQ_NFIE, eMMC_FCIE_IRQ, IRQF_SHARED, DRIVER_NAME, pSstarHost_st);
    if (s32_ret)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: request_irq fail \n");
        mmc_free_host(pMMCHost_st);
        goto LABEL_END;
    }
    #endif

    // For getting and showing device attributes from/to user space.
    s32_ret = sysfs_create_group(&pDev_st->dev.kobj, &mstar_mci_attr_grp);

	sgp_eMMCThread_st = kthread_create(mstar_mci_Housekeep, NULL, "eMMC_bg_thread");
	if(IS_ERR(sgp_eMMCThread_st))
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: create thread fail \n");
		return PTR_ERR(sgp_eMMCThread_st);
	}
	wake_up_process(sgp_eMMCThread_st);

	#if defined(CONFIG_MMC_MSTAR_MMC_EMMC_LIFETEST)
	g_eMMCDrv.u64_CNT_TotalRBlk = 0;
	g_eMMCDrv.u64_CNT_TotalWBlk = 0;
	writefile = create_proc_entry (procfs_name, 0644, NULL);
	if(writefile == NULL)
		eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Err: Can not initialize /proc/%s\n", procfs_name);
	else
	{
        writefile->read_proc = procfile_read;
        writefile->mode      = S_IFREG | S_IRUGO;
        writefile->uid       = 0;
        writefile->gid       = 0;
        writefile->size      = 0x10;
	}
	#endif

	LABEL_END:
	return s32_ret;
}

static s32 __exit mstar_mci_remove(struct platform_device *pDev_st)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st;
    struct mstar_mci_host *pSstarHost_st;
	s32 s32_ret;

	eMMC_LockFCIE((U8*)__FUNCTION__);
	pMMCHost_st = platform_get_drvdata(pDev_st);
	pSstarHost_st = mmc_priv(pMMCHost_st);
	s32_ret = 0;

    if (!pDev_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: pDev_st is NULL\n");
        s32_ret = -EINVAL;
		goto LABEL_END;
    }
    if (!pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: pMMCHost_st is NULL\n");
        s32_ret= -1;
		goto LABEL_END;
    }
	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, remove +\n");

	mmc_remove_host(pMMCHost_st);

    mstar_mci_disable(pSstarHost_st);
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    free_irq(pSstarHost_st->irq, pSstarHost_st);
    #endif

    mmc_free_host(pMMCHost_st);
    platform_set_drvdata(pDev_st, NULL);
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, remove -\n");

	LABEL_END:
	eMMC_UnlockFCIE((U8*)__FUNCTION__);

	if(sgp_eMMCThread_st)
	    kthread_stop(sgp_eMMCThread_st);

    return s32_ret;
}

#ifdef CONFIG_PM
static s32 mstar_mci_suspend(struct platform_device *pDev_st, pm_message_t state)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st;
    s32 ret;

	pMMCHost_st = platform_get_drvdata(pDev_st);
	ret = 0;

	// wait for D0 high before losing eMMC Vcc
	eMMC_LockFCIE((U8*)__FUNCTION__);
	if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
	{
	    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
	    eMMC_FCIE_ErrHandler_Stop();
	}
	eMMC_PlatformDeinit();
	eMMC_UnlockFCIE((U8*)__FUNCTION__);

    if (pMMCHost_st)
    {
		eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, suspend + \n");
        ret = mmc_suspend_host(pMMCHost_st);
    }

	eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, suspend -, %Xh\n", ret);
	return ret;
}

static s32 mstar_mci_resume(struct platform_device *pDev_st)
{
    struct mmc_host *pMMCHost_st;
    s32 ret;
    static u8 u8_IfLock=0;

	eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC, resume +\n");
	pMMCHost_st = platform_get_drvdata(pDev_st);
	ret = 0;

	if(0 == (REG_FCIE(FCIE_REG16h) & BIT_EMMC_ACTIVE))
	{
		eMMC_LockFCIE((U8*)__FUNCTION__);
		u8_IfLock = 1;
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"lock\n");
	}

	mstar_mci_enable(mmc_priv(pMMCHost_st));
	if(u8_IfLock)
	{
		u8_IfLock = 0;
		eMMC_UnlockFCIE((U8*)__FUNCTION__);
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"unlock\n");
	}

    if (pMMCHost_st)
    {
		//eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, resume +\n");
        ret = mmc_resume_host(pMMCHost_st);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC, resume -\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, resume -, %Xh\n", ret);
	return ret;
}
#endif  /* End ifdef CONFIG_PM */

/******************************************************************************
 * Define Static Global Variables
 ******************************************************************************/
static struct platform_driver sg_mstar_mci_driver =
{
	.probe = mstar_mci_probe,
    .remove = __exit_p(mstar_mci_remove),

    #ifdef CONFIG_PM
    .suspend = mstar_mci_suspend,
    .resume = mstar_mci_resume,
    #endif

    .driver  =
    {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
};

struct platform_device sg_mstar_emmc_device_st =
{
    .name =	DRIVER_NAME,
    .id = 0,
    .resource =	NULL,
    .num_resources = 0,
};

/******************************************************************************
 * Init & Exit Modules
 ******************************************************************************/
static s32 __init mstar_mci_init(void)
{
	int err = 0;
    u16 u16_regval = 0;

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"\n");

    u16_regval = REG_FCIE(FCIE_REG16h);

    if( (u16_regval & BIT_KERN_CHK_NAND_EMMC) == BIT_KERN_CHK_NAND_EMMC )
    {
        if( (u16_regval & BIT_KERN_EMMC) != BIT_KERN_EMMC )
            return 0;
    }

	if((err = platform_device_register(&sg_mstar_emmc_device_st)) < 0)
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: platform_driver_register fail, %Xh\n", err);

    if((err = platform_driver_register(&sg_mstar_mci_driver)) < 0)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: platform_driver_register fail, %Xh\n", err);

	return err;
}

static void __exit mstar_mci_exit(void)
{
    platform_driver_unregister(&sg_mstar_mci_driver);
}

static int __init write_seg_size_setup(char *str)
{
    wr_seg_size =  simple_strtoul(str, NULL, 16);
    return 1;
}

static int __init write_seg_theshold_setup(char *str)
{
    wr_split_threshold =  simple_strtoul(str, NULL, 16);
    return 1;
}

/* SAR5=ON in set_config will enable this feature */
static int __init sar5_setup_for_pwr_cut(char * str)
{
	if(str != NULL)
	{
		printk(KERN_CRIT"SAR5=%s", str);
		if(strcmp((const char *) str, "ON") == 0)
			u8_enable_sar5 = 1;
	}

	return 0;
}
early_param("SAR5", sar5_setup_for_pwr_cut);

__setup("mmc_wrsize=", write_seg_size_setup);
__setup("mmc_wrupsize=", write_seg_theshold_setup);

subsys_initcall(mstar_mci_init);
//module_init(mstar_mci_init);
module_exit(mstar_mci_exit);

MODULE_LICENSE("non-GPL");
MODULE_DESCRIPTION("Sstar Multimedia Card Interface driver");
MODULE_AUTHOR("SSTAR");
