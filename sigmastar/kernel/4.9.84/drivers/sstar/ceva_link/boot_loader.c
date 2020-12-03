/*
* boot_loader.c- Sigmastar
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
#include "boot_loader.h"
#include "drv_debug.h"

#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include <linux/delay.h>
#include <linux/io.h>
#ifdef _Measure_BootUp_Time_
#include <linux/timekeeping.h>
#endif //_Measure_BootUp_Time_
#include "file_access.h"

#define DETECT_DSP_CHECK

#define ALIGN_XM6(a) (((long)(a) + 0xFFF) & ~0xFFF)


#ifndef _FAST_DMA_BOOT_
typedef struct {
    u8 boot_copier_binary[0x200 - sizeof(phys_addr_t)*4];
    phys_addr_t dummy;
    phys_addr_t pdma_buff_addr;
    phys_addr_t image_start_addr;
    phys_addr_t entry_point_addr;
} boot_copier_t;

static const u8 boot_copier_binary[0x200] =
{
    0xCC, 0xC0, 0xD4, 0x8A, 0xFC, 0x7F, 0xBC, 0x53, 0x6B, 0xC8, 0x30, 0x70, 0x91, 0xC3, 0x56, 0x88,
    0000, 0000, 0xEF, 0x97, 0x1C, 0x36, 0x75, 0x88, 0x41, 0x33, 0x1C, 0xF2, 0x05, 0xCE, 0x01, 0x70,
    0000, 0xFE, 0x7C, 0000, 0x64, 0x80, 0xC0, 0x64, 0x40, 0000, 0000, 0xF8, 0000, 0000, 0xEF, 0x97,
    0x3A, 0x68, 0x05, 0x88, 0x5C, 0x81, 0x1C, 0xD8, 0x01, 0x64, 0xDC, 0x80, 0x61, 0x80, 0x02, 0x06,
    0x20, 0x30, 0000, 0x5F, 0x60, 0x03, 0x68, 0x95, 0000, 0000, 0000, 0xF8, 0000, 0000, 0xEF, 0x97,
    0x3A, 0x68, 0x05, 0x88, 0x5C, 0x81, 0x1C, 0xD8, 0x01, 0x64, 0x1E, 0x78, 0x05, 0x88, 0000, 0x74,
    0x5E, 0x80, 0x3C, 0x80, 0000, 0000, 0000, 0xFE, 0x1E, 0x05, 0x05, 0xC8, 0000, 0000, 0xEF, 0x97,
    0000, 0000, 0000, 0xF8, 0x5E, 0xF8, 0x0C, 0x88, 0x1E, 0x79, 0x05, 0x88, 0x8C, 0x44, 0x02, 0000,
    0xE1, 0xFE, 0x81, 0x90, 0xFF, 0xFF, 0xFF, 0xF8, 0000, 0000, 0xEF, 0x97, 0xB1, 0x06, 0x64, 0x95,
    0x6C, 0x88, 0x40, 0x1E, 0xC0, 0xFA, 0x5C, 0x81, 0x1C, 0xD8, 0x01, 0x64, 0000, 0000, 0xEF, 0x97,
    0000, 0000, 0xEF, 0x97, 0x5C, 0x81, 0x1C, 0x98, 0x01, 0x64, 0xF0, 0x01, 0xC9, 0xD7, 0xBA, 0x71,
    0x0B, 0xE8, 0x7D, 0x88, 0x02, 0x68, 0x03, 0x70, 0x6C, 0xC8, 0000, 0x71, 0xBD, 0x43, 0x83, 0xCE,
    0x02, 0x70, 0x6C, 0xC8, 0000, 0x71, 0x0D, 0xE4, 0x7D, 0x88, 0x02, 0x68, 0xA3, 0x8F, 0x1F, 0xC8,
    0x82, 0x43, 0000, 0000, 0xEF, 0x97, 0xA3, 0000, 0xA2, 0x01, 0x80, 0xD0, 0000, 0000, 0000, 0xF8,
    0xC5, 0xA0, 0x1B, 0x98, 0x1C, 0x45, 0x9F, 0x34, 0xC0, 0x20, 0x3D, 0xC0, 0x85, 0x5D, 0x84, 0x51,
    0xC8, 0x44, 0x80, 0x5D, 0000, 0000, 0xEF, 0x97, 0xA2, 0xFF, 0x81, 0x90, 0xFF, 0xFF, 0xFF, 0xF8,
    0x05, 0x88, 0x40, 0x70, 0000, 0xC1, 0x6C, 0xC8, 0x10, 0x70, 0x01, 0000, 0xEE, 0x97, 0x32, 0x37,
    0x10, 0x70, 0x01, 0x04, 0x05, 0x88, 0x40, 0x70, 0x81, 0xC1, 0x6C, 0xC8, 0x10, 0x70, 0000, 0000,
    0x02, 0000, 0x02, 0000, 0xB0, 0x53, 0x6B, 0x88, 0x20, 0x70, 0x03, 0000, 0x02, 0xC2, 0x6C, 0x88,
    0x81, 0x90, 0xFF, 0xFF, 0xFF, 0xF8, 0000, 0000, 0xEF, 0x97, 0x3D, 0x90, 0xFE, 0x8E, 0000, 0x78,
    0xEF, 0x97, 0xC0, 0xFD, 0x81, 0x90, 0xFF, 0xFF, 0xFF, 0xF8, 0000, 0000, 0xEF, 0x97, 0x61, 0xFF,
    0xFF, 0xF8, 0x60, 0xA0, 0x1B, 0x98, 0x9C, 0x44, 0x5F, 0x34, 0x61, 0x20, 0x3D, 0xC0, 0000, 0000,
    0000, 0000, 0000, 0000, 0x04, 0000, 0000, 0000, 0xEF, 0x97, 0xA2, 0xFF, 0x81, 0x90, 0xFF, 0xFF,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000
};
#else //_FAST_DMA_BOOT_
typedef struct {
    u8 boot_copier_binary[0x200];
    #if 0
    phys_addr_t dummy;
    phys_addr_t pdma_buff_addr;
    phys_addr_t image_start_addr;
    phys_addr_t entry_point_addr;
    #endif 
} boot_copier_t;

static const u8 boot_copier_binary[0x200] =
{
0x01, 0x70, 0x00, 0x00, 0xEF, 0x97, 0xCC, 0xC0, 0xD4, 0x8A, 0xFC, 0x7F, 0x91, 0xC3, 0x56, 0x88, 
0x64, 0x80, 0xC0, 0x64, 0x00, 0x00, 0xEF, 0x97, 0x00, 0x00, 0xEF, 0x97, 0x1C, 0xF3, 0x05, 0x8E, 
0x6C, 0x88, 0xCC, 0x70, 0xDC, 0x00, 0x64, 0x80, 0xC0, 0x64, 0x40, 0x00, 0x00, 0xF8, 0x7C, 0x00, 
0x10, 0x70, 0x82, 0x5D, 0x00, 0xEE, 0x7F, 0x88, 0x02, 0xC5, 0x6C, 0x88, 0x14, 0x70, 0x62, 0x76, 
0x82, 0xC1, 0x6C, 0x88, 0x10, 0x70, 0x40, 0x30, 0x00, 0xEE, 0x7F, 0x88, 0x02, 0xC1, 0x6C, 0x88, 
0x02, 0xC2, 0x6C, 0x88, 0x10, 0x70, 0x02, 0x70, 0x6C, 0x88, 0x00, 0x71, 0x00, 0xEE, 0x7F, 0x88, 
0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0xB0, 0x53, 0x6B, 0x88, 0x20, 0x70, 0x00, 0xEE, 0x7F, 0x88, 
0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 
0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x0D, 0xB4, 0x7C, 0x88, 0x90, 0x64, 0x00, 0xB0, 
0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0xA1, 0xFE, 0x01, 0x90, 0xFF, 0xFF, 0xFF, 0xF8, 
0x00, 0x71, 0x43, 0x03, 0x00, 0x90, 0x00, 0x00, 0x00, 0xF8, 0x7F, 0x34, 0xE8, 0x44, 0x63, 0x30, 
0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0x02, 0xC2, 0x6C, 0x88, 0x10, 0x70, 0x02, 0x70, 0x6C, 0x88, 
0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0xB0, 0x53, 0x6B, 0x88, 0x20, 0x70, 0x00, 0xEE, 
0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 
0xFF, 0xF8, 0x00, 0xB0, 0x14, 0x80, 0x0D, 0xB4, 0x7C, 0x88, 0x90, 0x64, 0x00, 0xB0, 0x14, 0x80, 
0xFF, 0xF8, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0xA1, 0xFE, 0x01, 0x90, 0xFF, 0xFF, 
0x0B, 0x70, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x20, 0xFD, 0x01, 0x90, 0xFF, 0xFF, 
0x6C, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0x16, 0x75, 0x6C, 0x88, 0x18, 0x70, 0x17, 0x70, 0x6C, 0xC8, 
0x7F, 0x88, 0x17, 0x5F, 0x16, 0x70, 0x6C, 0xC8, 0x1A, 0x70, 0x00, 0xEE, 0x7F, 0x88, 0xD7, 0xCE, 
0x94, 0x70, 0x6C, 0xC8, 0x1A, 0x70, 0x00, 0xEE, 0x7F, 0x88, 0xD7, 0xCE, 0x6C, 0x88, 0x00, 0xEE, 
0x00, 0xF8, 0x00, 0xEE, 0x7F, 0x88, 0x95, 0xCE, 0x6C, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0xA0, 0x32, 
0x00, 0xEE, 0x7F, 0x88, 0x16, 0x73, 0x6C, 0x88, 0x19, 0x70, 0x17, 0x70, 0x6C, 0xC8, 0x00, 0x20, 
0x1A, 0x70, 0x17, 0x74, 0x6C, 0xC8, 0x03, 0x70, 0x00, 0xEE, 0x7F, 0x88, 0xD7, 0xCE, 0x6C, 0x88, 
0x00, 0xEE, 0x7F, 0x88, 0xD7, 0xCE, 0x6C, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0x96, 0x71, 0x6C, 0x88, 
0x00, 0xEE, 0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 
0x00, 0xB0, 0x14, 0x80, 0x17, 0x74, 0x6C, 0x88, 0x03, 0x70, 0xAC, 0x53, 0x6B, 0x88, 0x34, 0x70, 
0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 
0xFF, 0xFF, 0xFF, 0xF8, 0x00, 0xB0, 0x14, 0x80, 0xFD, 0xD6, 0x68, 0x88, 0x00, 0xB0, 0x14, 0x80, 
0x6C, 0xC8, 0x18, 0x70, 0x00, 0xB0, 0x14, 0x80, 0x00, 0xB0, 0x14, 0x80, 0xA1, 0xFE, 0x01, 0x90, 
0x2A, 0xCC, 0x6C, 0x88, 0x00, 0xEE, 0x7F, 0x88, 0xEA, 0x75, 0x6C, 0x88, 0xFB, 0x72, 0x81, 0x74, 
0x00, 0xEE, 0x7F, 0x88, 0x00, 0x00, 0x80, 0x95, 0x00, 0xEE, 0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 
0x00, 0xEE, 0x7F, 0x88, 0x00, 0x00, 0x80, 0x95, 0x00, 0xEE, 0x7F, 0x88, 0x00, 0xEE, 0x7F, 0x88, 
};

#endif //_FAST_DMA_BOOT_

typedef enum
{
  E_REG_STATE_UNKNOWN = 0,      /**< Undefined state */
  E_REG_STATE_IN_PROGRESS,      /**< DSP registration in progress */
  E_REG_STATE_DONE              /**< DSP registration complete */
}dsp_reg_state_enum;

typedef struct
{
  u32 dsp_id;                               /**< The DSP ID */
  dsp_reg_state_enum    dsp_reg_state;      /**< The DSP registration state */
}dsp_info_struct;

extern void SetUpDSPTimeSlot(unsigned int SetValue);

#ifdef _Measure_BootUp_Time_
struct timeval start_time;
struct timeval end_time;
struct timeval DMA_start_time;
struct timeval DMA_end_time;

unsigned int Cacu_time(struct timeval *pStart,struct timeval *pEnd)
{
  unsigned int ret=0;
  unsigned int start,end;
  
  start=pStart->tv_sec * 1000000 + pStart->tv_usec;
  end=pEnd->tv_sec * 1000000 + pEnd->tv_usec;
  ret=end-start;
  
  return ret;
}
#endif //_Measure_BootUp_Time_

static long detect_dsp_acting(ceva_hal_handle *handle, u8 *buffer_virt)
{
    dsp_info_struct *dsp_info = (dsp_info_struct*)buffer_virt;
    int retry = 50;
    #ifdef _Measure_BootUp_Time_
    unsigned int process_time;
    #endif //_Measure_BootUp_Time_
    
    // delay 10 ms to ensure to share memory is clean by DSP
    mdelay(10);
    
    while (retry--)
    {
        if (dsp_info->dsp_id == 0 && dsp_info->dsp_reg_state == E_REG_STATE_DONE)
        {   
            #ifdef _Measure_BootUp_Time_
            do_gettimeofday(&end_time);
            process_time=Cacu_time(&start_time,&end_time);
            printk("DSP WakeUp time : %08d us \n",process_time);
            #endif //_Measure_BootUp_Time_
            XM6_MSG(XM6_MSG_DBG, "DSP is ready\n");
            return 0;
        }
        //mdelay(500);
        mdelay(30);
        XM6_MSG(XM6_MSG_DBG, "DSP is not ready (%d, %d), dummy = 0x%x\n", dsp_info->dsp_id, dsp_info->dsp_reg_state, dsp_ceva_hal_read_dummy_data(handle, 0));
    }
    dsp_ceva_hal_check_bodary_status(handle);
    return -EIO;
}

#ifdef _FAST_DMA_BOOT_

#include "ms_platform.h"
#include "hal_bdma.h"
#include "infinity2/registers.h"
#include "ms_types.h"

static void Add_DSP_Copier_Address(u8 *Data,phys_addr_t Dsp_Data_Image_Addr,phys_addr_t Dsp_Code_Image_Addr)
{
    unsigned int tmp=(unsigned int)Dsp_Data_Image_Addr;
    
    printk("Dsp_Code_Image_Addr :0x%x Dsp_Data_Image_Addr:0x%x \n", Dsp_Code_Image_Addr, Dsp_Data_Image_Addr);
    
    *(Data+0)=((tmp>>0)&0xff);
    *(Data+1)=((tmp>>8)&0xff);
    *(Data+2)=((tmp>>16)&0xff);
    *(Data+3)=((tmp>>24)&0xff);
    
    tmp=(unsigned int)Dsp_Code_Image_Addr;
    *(Data+4)=((tmp>>0)&0xff);
    *(Data+5)=((tmp>>8)&0xff);
    *(Data+6)=((tmp>>16)&0xff);
    *(Data+7)=((tmp>>24)&0xff);
    
    return;
}

int dsp_dma_transfer(phys_addr_t src_addr, phys_addr_t dst_addr, u32 size,u8 *src_virt)
{
#if 1
    HalBdmaParam_t  stBdmaParam;
    u8 u8DmaCh = HAL_BDMA_CH1;
    #ifdef _Measure_BootUp_Time_
    unsigned int process_time;
    #endif //_Measure_BootUp_Time_

    stBdmaParam.ePathSel     = HAL_BDMA_MIU0_TO_MIU0;
    stBdmaParam.eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
    stBdmaParam.eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
    stBdmaParam.bIntMode     = 0;
    stBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    stBdmaParam.u32TxCount   = size;
    stBdmaParam.pSrcAddr     = (void *)(src_addr - MIU0_BASE);
    stBdmaParam.pDstAddr     = (void *)(dst_addr - MIU0_BASE);
    stBdmaParam.pfTxCbFunc   = NULL;

    #if 0 
    printk("[Jesse]%s %d src addr=%p dest addr=%p size=%x\n", __FUNCTION__, __LINE__,
        stBdmaParam.pSrcAddr, stBdmaParam.pDstAddr, stBdmaParam.u32TxCount);
    #endif
    
    #ifdef _Measure_BootUp_Time_
    do_gettimeofday(&DMA_start_time);
    #endif //_Measure_BootUp_Time_
    
    if (HAL_BDMA_PROC_DONE != HalBdma_Transfer(u8DmaCh, &stBdmaParam)) {
        return -1;
    }
    #ifdef _Measure_BootUp_Time_
    do_gettimeofday(&DMA_end_time);
    process_time=Cacu_time(&DMA_start_time,&DMA_end_time);
    printk("ARM BDMA process time : %08d us \n",process_time);
    #endif //_Measure_BootUp_Time_
#else
    void *dst_virt_addr;

    dst_virt_addr = ioremap_nocache(dst_addr, size);
    if (dst_virt_addr == NULL)
    {
        XM6_MSG(XM6_MSG_DBG, "dsp_dma_transfer dst_virt_addr error:0x%p \n",dst_virt_addr);
        return -ENOMEM;
    }
    
    XM6_MSG(XM6_MSG_DBG, "%s %d dst_virt_addr=0x%p src_virt_addr=0x%p	size:0x%x \n", __FUNCTION__, __LINE__,dst_virt_addr,src_virt,size);
    memcpy(dst_virt_addr,src_virt,size);
    iounmap(dst_virt_addr);

#endif
    return 0;
}

#if 0
#define CODE_EXT_NAME "/mnt/CODE_EXT.bin"
#define DATA_EXT_NAME "/mnt/DATA_EXT.bin"
#define TEST_SIZE 32*1024

int Log_Bin_File(phys_addr_t dst_phy_addr,unsigned int Size,unsigned int Idx,phys_addr_t src_phy_addr)
{
	int ret=0;
	void *dst_virt_addr;
	char *pName;
	struct file *pFile;
	int wsize=0;
	unsigned int ii=0;
	/*
	void *src_virt_addr;
	unsigned int ii=0;
	unsigned char tmp0,tmp1;*/
	
	//struct resource *physical_region=NULL;
	
	printk("Log_Bin_File	aaaaaa Idx:%d	 Size:0x%x \n",Idx,Size);

  #if 0
	physical_region = request_mem_region(dst_phy_addr, Size, "BDMA_MOVE_1");
	if(physical_region == NULL){
		printk("physical_region NULL \n");
	}
	printk("Log_Bin_File  physical_region ->start:0x%x resource_size:0x%x \n",physical_region ->start,resource_size(physical_region));
	dst_virt_addr   = (void *)ioremap(physical_region ->start, resource_size(physical_region));
  #endif 
	
	
	dst_virt_addr = ioremap_nocache(dst_phy_addr, Size);
	//printk("Log_Bin_File	Idx:%d	dst_phy_addr:0x%llx  dst_virt_addr:0x%x \n",Idx,dst_phy_addr,dst_virt_addr);
	printk("Log_Bin_File	Idx:%d	dst_virt_addr:0x%p \n",Idx,dst_virt_addr);
	if (dst_virt_addr == NULL)
	{
		XM6_MSG(XM6_MSG_DBG, "Log_Bin_File  dst_virt_addr error\n");
		return -ENOMEM;
	}
	for(ii=0;ii<16;ii++){
		XM6_MSG(XM6_MSG_DBG, "Log_Bin_File  dst_virt_addr:0x%p +%d : 0x%x \n",dst_virt_addr,ii,*((unsigned char *)dst_virt_addr+ii));
	}
	
	pName=(Idx==0)?CODE_EXT_NAME:DATA_EXT_NAME;
	
	pFile=OpenFile(pName,O_CREAT|O_SYNC|O_RDWR,S_IWUSR|S_IWGRP|S_IWOTH);
	if (pFile == NULL)
	{
		XM6_MSG(XM6_MSG_DBG, "Log_Bin_File  pFile open fail\n");
		return -ENOMEM;
	}
	//wsize=WriteFile(pFile,dst_virt_addr,Size);
	wsize=WriteFile(pFile,dst_virt_addr,TEST_SIZE);
	XM6_MSG(XM6_MSG_DBG, "write file:0x%p size 0x%x		wsize:0x%x \n",pFile,Size,wsize);
	CloseFile(pFile);
	
	#if 0
	src_virt_addr = ioremap_nocache(src_phy_addr, Size);
	//printk("Log_Bin_File	Idx:%d	dst_phy_addr:0x%llx  dst_virt_addr:0x%x \n",Idx,dst_phy_addr,dst_virt_addr);
	printk("Log_Bin_File 222	Idx:%d	src_virt_addr:0x%p \n",Idx,src_virt_addr);
	if (src_virt_addr == NULL)
	{
		XM6_MSG(XM6_MSG_DBG, "Log_Bin_File  src_virt_addr error\n");
		return -ENOMEM;
	}
	for(ii=0;ii<Size;ii++){
		tmp0=*((unsigned char *)src_virt_addr+ii);
		tmp1=*((unsigned char *)dst_virt_addr+ii);
		if(tmp0!=tmp1){
			XM6_MSG(XM6_MSG_DBG, "Log_Bin_File  Error Start at:0x%x src:0x%x dst:0x%x \n",ii,tmp0,tmp1);
		}
	}
	iounmap(src_virt_addr);
	#endif
	iounmap(dst_virt_addr);
	return ret;
}
#endif

#if 0
static void dump_32byte(void *in)
{
	unsigned int ii=0;
	
	for(ii=0;ii<16;ii++){
		XM6_MSG(XM6_MSG_DBG, "dump_32byte  in:0x%p +%d : 0x%x \n",in,ii,*((unsigned char *)in+ii));
	}
}
#endif

int dsp_dma_image_ext_transfer(struct boot_config_t boot_config,u8 *image_virt)
{
    int ret;
    phys_addr_t src_phy_addr, dst_phy_addr;
    //void *dst_virt_addr;
    void *src_virt_addr;

    src_phy_addr = boot_config.boot_image.miu + MIU0_BASE + boot_config.dsp_dma_image_info.code_int_size + boot_config.dsp_dma_image_info.data_int_size;
    src_virt_addr=image_virt+boot_config.dsp_dma_image_info.code_int_size + boot_config.dsp_dma_image_info.data_int_size;
    dst_phy_addr = boot_config.dsp_code_addr;
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 000 dst_phy_addr:0x%x dsp_code_size:0x%x 	src_phy_addr:0x%x \n",dst_phy_addr,boot_config.dsp_code_size,src_phy_addr);
    #if 0
    dst_virt_addr = ioremap_nocache(dst_phy_addr, boot_config.dsp_code_size);
    if (dst_virt_addr == NULL)
    {
    	  XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 111\n");
        return -ENOMEM;
    }
    XM6_MSG(XM6_MSG_DBG, "%s %d dst_phy_addr=0x%x size=0x%x\n", __FUNCTION__, __LINE__,dst_phy_addr, boot_config.dsp_dma_image_info.code_ext_size);
    iounmap(dst_virt_addr);
    #endif
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 222\n");
    
    ret = dsp_dma_transfer(src_phy_addr, dst_phy_addr, boot_config.dsp_dma_image_info.code_ext_size,src_virt_addr);
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 333\n");
    if (ret != 0){
    		XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 555\n");
        return -EIO;
    }
    #if 0
    ret = Log_Bin_File(dst_phy_addr,boot_config.dsp_dma_image_info.code_ext_size,0,src_phy_addr);
    if (ret != 0){
        XM6_MSG(XM6_MSG_DBG, "Log_Bin_File Fail\n");
        return -EIO;
    }
    #endif

    src_phy_addr = src_phy_addr + boot_config.dsp_dma_image_info.code_ext_size;
    src_virt_addr=image_virt+boot_config.dsp_dma_image_info.code_int_size + boot_config.dsp_dma_image_info.data_int_size+ boot_config.dsp_dma_image_info.code_ext_size;
    dst_phy_addr = boot_config.dsp_data_addr;
    #if 0
    dst_virt_addr = ioremap_nocache(dst_phy_addr, boot_config.dsp_data_size);
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 666\n");
    if (dst_virt_addr == NULL)
    {
    		XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 777\n");
        return -ENOMEM;
    }
    XM6_MSG(XM6_MSG_DBG, "%s %d dst_phy_addr=0x%x size=0x%x\n", __FUNCTION__, __LINE__,dst_phy_addr, boot_config.dsp_dma_image_info.code_ext_size);
    iounmap(dst_virt_addr);
    #endif
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 888\n");
    
    ret = dsp_dma_transfer(src_phy_addr, dst_phy_addr, boot_config.dsp_dma_image_info.data_ext_size,src_virt_addr);
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer 999\n");
    if (ret != 0){
    	XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer aaa\n");
        return -EIO;
    }
    #if 0
    ret = Log_Bin_File(dst_phy_addr,boot_config.dsp_dma_image_info.data_ext_size,1,src_phy_addr);
    if (ret != 0){
        XM6_MSG(XM6_MSG_DBG, "Log_Bin_File Fail Idx:%d \n",1);
        return -EIO;
    }
    #endif
    XM6_MSG(XM6_MSG_DBG, "dsp_dma_image_ext_transfer bbb\n");

    return 0;
}

#endif //_FAST_DMA_BOOT_

int dsp_boot_up(ceva_hal_handle *handle, dev_dsp_buf *copier_buf, void *share_mem_virt, dsp_mem_info_t *dsp_mem_info)
{
    boot_copier_t *copier_virt;
    dsp_mem_info_t *info_virt;
    phys_addr_t copier_phys, info_phys;
    int offset;
    u32 size;
    
    //PLL Adjust
    if(dsp_mem_info->custom_info & 0x1)
    {
      dsp_ceva_hal_reset_xm6(handle);
      dsp_ceva_hal_set_PLL(handle,1);
    }
    SetUpDSPTimeSlot((dsp_mem_info->custom_info >> 16)&0xffff);

    XM6_MSG(XM6_MSG_DBG, "boot_image.addr  = 0x%x, size = %d\n", dsp_mem_info->boot_image.addr, dsp_mem_info->boot_image.size);
    XM6_MSG(XM6_MSG_DBG, "share_mem.addr   = 0x%x, size = %d\n", dsp_mem_info->share_mem.addr, dsp_mem_info->share_mem.size);
    XM6_MSG(XM6_MSG_DBG, "ext_heap.addr    = 0x%x, size = %d\n", dsp_mem_info->ext_heap.addr, dsp_mem_info->ext_heap.size);
    XM6_MSG(XM6_MSG_DBG, "hprintf_buf.addr = 0x%x, size = %d\n", dsp_mem_info->hprintf_buf.addr, dsp_mem_info->hprintf_buf.size);
    XM6_MSG(XM6_MSG_DBG, "dsp_log_buf.addr = 0x%x, size = %d\n", dsp_mem_info->dsp_log_buf.addr, dsp_mem_info->dsp_log_buf.size);
    XM6_MSG(XM6_MSG_DBG, "work_buffer.addr = 0x%x, size = %d\n", dsp_mem_info->work_buffer.addr, dsp_mem_info->work_buffer.size);

    size = sizeof(boot_copier_t) + sizeof(dsp_mem_info_t);
    copier_virt = (boot_copier_t*)ALIGN_XM6(copier_buf->virt);
    copier_phys = ALIGN_XM6(copier_buf->phys);
    offset = (u8*)copier_virt - copier_buf->virt;
    if ((copier_buf->size - offset) < size) {
        XM6_MSG(XM6_MSG_ERR, "copier buffer is too small\n");
        return -EINVAL;
    }

    // info is next to copier
    info_virt = (dsp_mem_info_t*)(copier_virt+1);
    info_phys = copier_phys + sizeof(boot_copier_t);

    XM6_MSG(XM6_MSG_DBG, "copier_buf = %p (0x%X), size = %d\n", copier_virt, copier_phys, sizeof(boot_copier_t));
    XM6_MSG(XM6_MSG_DBG, "info_buf   = %p (0x%X)\n", info_virt, info_phys);

    // copy copier binary and sett relative data
    memcpy(copier_virt->boot_copier_binary, boot_copier_binary, sizeof(copier_virt->boot_copier_binary));
    #ifdef _FAST_DMA_BOOT_
    Add_DSP_Copier_Address(copier_virt->boot_copier_binary+0x1F8,dsp_mem_info->boot_image.addr,dsp_mem_info->boot_image.addr+0x40000);
    #else
    copier_virt->pdma_buff_addr = 0;
    copier_virt->image_start_addr = dsp_mem_info->boot_image.addr;
    copier_virt->entry_point_addr = 0;
    XM6_MSG(XM6_MSG_DBG, "boot_copier_binary = 0x%02x       (@ 0x%p)\n", copier_virt->boot_copier_binary[0], &copier_virt->boot_copier_binary);
    XM6_MSG(XM6_MSG_DBG, "pdma_buff_addr     = 0x%08x (@ 0x%p)\n", copier_virt->pdma_buff_addr, &copier_virt->pdma_buff_addr);
    XM6_MSG(XM6_MSG_DBG, "image_start_addr   = 0x%08x (@ 0x%p)\n", copier_virt->image_start_addr, &copier_virt->image_start_addr);
    XM6_MSG(XM6_MSG_DBG, "entry_point_addr   = 0x%08x (@ 0x%p)\n", copier_virt->entry_point_addr, &copier_virt->entry_point_addr);
    #endif

    XM6_MSG(XM6_MSG_DBG, "endian check %02x %02x %02x %02x = %08x\n", copier_virt->boot_copier_binary[0], copier_virt->boot_copier_binary[1], copier_virt->boot_copier_binary[2], copier_virt->boot_copier_binary[3], *(u32*)(&copier_virt->boot_copier_binary[0]));

    dsp_ceva_hal_check_bodary_status(handle);
    
    // copy boot info
    memcpy(info_virt, dsp_mem_info, sizeof(dsp_mem_info_t));

    // wirte boot info address to dummy register, and DSP would read after boot up
    dsp_ceva_hal_write_dummy_data(handle, 0, info_phys);

    XM6_MSG(XM6_MSG_DBG, "dummy 0 = 0x%x\n", info_phys);

    // wait memory barrier
    wmb();

    // start DSP
    dsp_ceva_hal_reset_xm6(handle);
    #ifdef _Measure_BootUp_Time_
    do_gettimeofday(&start_time);
    #endif //_Measure_BootUp_Time_
    dsp_ceva_hal_enable_xm6(handle);
    dsp_ceva_hal_bootup_xm6(handle, copier_phys);
    dsp_timer_hal_init();

    return detect_dsp_acting(handle, share_mem_virt);
}

void dsp_shut_down(ceva_hal_handle *handle)
{
    dsp_ceva_hal_reset_xm6(handle);
    dsp_ceva_hal_set_PLL(handle,0);
    return;
}

#if 0
#include "file_access.h"

void MemoryClear(phys_addr_t pa_addr,unsigned int size)
{
  void *virt_addr;
	
	virt_addr = ioremap_nocache(pa_addr, size);
	memset(virt_addr,0,size);
  iounmap(virt_addr);
  
  printk("MemoryClear virt_addr:%p pa_addr:0x%x sz:%d\n",virt_addr,pa_addr,size);
  
	return;
}

void Write_Memory_to_File(phys_addr_t pa_addr,unsigned int size,struct file *PDump)
{
	void *virt_addr;
	
	virt_addr = ioremap_nocache(pa_addr, size);
	WriteFile(PDump, virt_addr, size);
  iounmap(virt_addr);
  
  printk("Write_Memory_to_File PDump:%p virt_addr:%p pa_addr:0x%x sz:%d\n", PDump, virt_addr,pa_addr,size);
  
	return;
}
void dump_memory(void)
{
	//void *dst_virt_addr;
	struct file *PDump;
	
	
	PDump=OpenFile("/mnt/dump_code_ext.bin",O_RDWR | O_CREAT | O_SYNC,0644);
	if(IS_ERR(PDump))
  {
    printk("file open fail 111  %p!", PDump);
    return;
  }
  Write_Memory_to_File(pCode_ext_Buf->phys,pCode_ext_Buf->size,PDump);
  CloseFile(PDump);
  
  PDump=OpenFile("/mnt/dump_data_ext.bin",O_RDWR | O_CREAT | O_SYNC,0644);
	if(IS_ERR(PDump))
  {
    printk("file open fail 222  %p!", PDump);
    return;
  }
  Write_Memory_to_File(pData_ext_Buf->phys,pData_ext_Buf->size,PDump);
  CloseFile(PDump);
	
	return;
}

#endif 
