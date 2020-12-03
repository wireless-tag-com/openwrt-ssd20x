/*
* hal_warp.c- Sigmastar
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
#include "hal_warp.h"
#include "hal_debug.h"

#include "ms_platform.h"

#if (HAL_MSG_LEVL < HAL_MSG_DBG)
#define REGR(base,idx)      ms_readl(((uint)base+(idx)))
#define REGW(base,idx,val)  ms_writel(val,((uint)base+(idx)))
#else
#define REGR(base,idx)      ms_readl(((uint)base+(idx)))
#define REGW(base,idx,val)  do{HAL_MSG(HAL_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)), val); ms_writel(val,((uint)base+(idx)));} while(0)
#endif

/*******************************************************************************************************************
 * warp_hal_set_axi
 *   Set AXI bus
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_axi(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
 {
    handle->reg_bank.axi_cfg2.fields.mwb = (u8)(AXI_MAX_WRITE_BURST_SIZE & 0xff  );
    handle->reg_bank.axi_cfg2.fields.mrb = (u8)(AXI_MAX_READ_BURST_SIZE   & 0xff);
    handle->reg_bank.axi_cfg2.fields.moutstw = (u8)(AXI_MAX_WRITE_OUTSTANDING & 0xff);
    handle->reg_bank.axi_cfg2.fields.moutstr = (u8)(AXI_MAX_READ_OUTSTANDING & 0xff);

    REGW(handle->base_addr,WARP_AXI_CFG2_REG_ADDR, handle->reg_bank.axi_cfg2.overlay); //0x0c,  AXI_CFG2
 }
/*******************************************************************************************************************
 * warp_hal_image_set
 *   Set image buffer pointer
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_image_point(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
 {
    HAL_WARP_IMAGE_DATA_T* input_data = &config->input_data;
    HAL_WARP_IMAGE_DATA_T* output_data = &config->output_data;

    //point to image plane
    handle->reg_bank.dlua_yrgb = (s32)Chip_MIU_to_Phys(input_data->data[HAL_WARP_IMAGE_PLANE_Y]); //physical address
    handle->reg_bank.dlua_uvg = (s32)Chip_MIU_to_Phys(input_data->data[HAL_WARP_IMAGE_PLANE_UV]); //physical address
    handle->reg_bank.dsua_yrgb = (s32)Chip_MIU_to_Phys(output_data->data[HAL_WARP_IMAGE_PLANE_Y]); //physical address
    handle->reg_bank.dsua_uvg = (s32)Chip_MIU_to_Phys(output_data->data[HAL_WARP_IMAGE_PLANE_UV]);; //physical address

    REGW(handle->base_addr,WARP_DLUA_YRGB_REG_ADDR, handle->reg_bank.dlua_yrgb );   //0x10, RGBA or Y base address of input
    REGW(handle->base_addr,WARP_DLUA_UV_REG_ADDR, handle->reg_bank.dlua_uvg );      //0x14, UV base address of input
    REGW(handle->base_addr,WARP_DSUA_YRGB_REG_ADDR, handle->reg_bank.dsua_yrgb );   //0x20, RGBA or Y base address of output
    REGW(handle->base_addr,WARP_DSUA_UV_REG_ADDR, handle->reg_bank.dsua_uvg );      //0x24, UV base address of output
 }
/*******************************************************************************************************************
 * warp_hal_image_size_set
 *   Set image size
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_image_size(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
 {
    HAL_WARP_IMAGE_DESC_T* input_image  = &config->input_image;
    HAL_WARP_IMAGE_DESC_T* output_image = &config->output_image;

    //point to image plane
    handle->reg_bank.ifsz.fields.ifszx = (u16)input_image->width;
    handle->reg_bank.ifsz.fields.ifszy = (u16)input_image->height;
    handle->reg_bank.ofsz.fields.ofszx= (u16)output_image->width;
    handle->reg_bank.ofsz.fields.ofszy = (u16)output_image->height;

    REGW(handle->base_addr,WARP_IFSZ_REG_ADDR, handle->reg_bank.ifsz.overlay);  //0x38, Input frame heigth and width
    REGW(handle->base_addr,WARP_OFSZ_REG_ADDR, handle->reg_bank.ofsz.overlay);  //0x3c, Output frame heigth and width
 }
/*******************************************************************************************************************
 * warp_hal_output_tile_set
 *   Set output tile size
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_output_tile(warp_hal_handle* handle, u16 tile_w, u16 tile_h)
 {
    handle->reg_bank.ocs.fields.ocsx = (u16)tile_w;
    handle->reg_bank.ocs.fields.ocsy = (u16)tile_h;

    REGW(handle->base_addr,WARP_OCS_REG_ADDR, handle->reg_bank.ocs.overlay);    //0x30, Output tile scanning block height and width
 }
 /*******************************************************************************************************************
 * warp_hal_set_disp
 *   Point to displacement  map
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_disp(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
 {
    HAL_WARP_DISPLAY_TABLE_T* disp_table = &config->disp_table; //< Displacement table descriptor

    handle->reg_bank.distba = (s32)Chip_MIU_to_Phys(disp_table->table);

    REGW(handle->base_addr,WARP_DISTBA_REG_ADDR, handle->reg_bank.distba ); //0x40, dist. table base address
 }
 /*******************************************************************************************************************
 * warp_hal_set_bb
 *   Point to  bounding box table
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_bb(warp_hal_handle* handle, s32 table_addr)
 {
    handle->reg_bank.bba = (s32)Chip_MIU_to_Phys(table_addr);

    REGW(handle->base_addr,WARP_BBA_REG_ADDR, handle->reg_bank.bba );       //0x44, B.B base address
 }
  /*******************************************************************************************************************
 * warp_hal_pers_matirx_set
 *   Set perspective transform coefficient
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
 void warp_hal_set_pers_matirx(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
 {
    handle->reg_bank.c00 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C00];
    handle->reg_bank.c01 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C01];
    handle->reg_bank.c02 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C02];
    handle->reg_bank.c10 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C10];
    handle->reg_bank.c11 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C11];
    handle->reg_bank.c12 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C12];
    handle->reg_bank.c20 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C20];
    handle->reg_bank.c21 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C21];
    handle->reg_bank.c22 = config->coeff[HAL_WARP_PERSECTIVE_COEFFS_C22];

    REGW(handle->base_addr,WARP_PDC_C00_REGS_ADDR, handle->reg_bank.c00 ); //0x48, perspective transform coefficient(0x48~0x68)
    REGW(handle->base_addr,WARP_PDC_C01_REGS_ADDR, handle->reg_bank.c01 );//0x4c
    REGW(handle->base_addr,WARP_PDC_C02_REGS_ADDR, handle->reg_bank.c02 );//0x50
    REGW(handle->base_addr,WARP_PDC_C10_REGS_ADDR, handle->reg_bank.c10 ); //0x54
    REGW(handle->base_addr,WARP_PDC_C11_REGS_ADDR, handle->reg_bank.c11 );//0x58
    REGW(handle->base_addr,WARP_PDC_C12_REGS_ADDR, handle->reg_bank.c12 );//0x5C
    REGW(handle->base_addr,WARP_PDC_C20_REGS_ADDR, handle->reg_bank.c20 ); //0x60
    REGW(handle->base_addr,WARP_PDC_C21_REGS_ADDR, handle->reg_bank.c21 );//0x64
    REGW(handle->base_addr,WARP_PDC_C22_REGS_ADDR, handle->reg_bank.c22 );//0x68
 }
/*******************************************************************************************************************
 * warp_hal_set_out_of_range
 *   Set out of range pixel fill value
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
void warp_hal_set_out_of_range(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
{
    handle->reg_bank.csfv.overlay= *((u32*)(config->fill_value));
    REGW(handle->base_addr,WARP_CSFV_REG_ADDR, handle->reg_bank.csfv.overlay ); //0x70, out of range pixel fill value
}
/*******************************************************************************************************************
 * warp_hal_set_config
 *   Set configure
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
void warp_hal_set_config(warp_hal_handle* handle, HAL_WARP_CONFIG* config)
{
    //handle->reg_bank.ctl.overlay = 0x4205; //default
    handle->reg_bank.ctl.fields.dispm   = config->op_mode;
    handle->reg_bank.ctl.fields.dxym    = config->disp_table.format;            // 0 is abs mode, 1 is relative mode
    handle->reg_bank.ctl.fields.eofie   = 1;                                    // End of frame interrupt enable
    handle->reg_bank.ctl.fields.oie     = 1;                                    // Output interrupt enable
    handle->reg_bank.ctl.fields.axierie = 1;                                    // AXI error interrupt enable
    handle->reg_bank.ctl.fields.dtibsz  = config->disp_table.resolution;        // gride size : 8x8 or 16x16
    handle->reg_bank.ctl.fields.iif     = config->input_image.format;           // RGBA, YUV422 NV16 or YUV420 NV12
    handle->reg_bank.ctl.fields.bpdu    = DEBUG_BYPASS_DISPLACEMENT_EN; // Bypass Displacement unit (Debug hook)
    handle->reg_bank.ctl.fields.bpiu    = DEBUG_BYPASS_INTERP_EN;       // Bypass Interpolation unit (Debug hook)
    handle->reg_bank.ctl.fields.go      = 1;                                    // WARP accelerator starts

    REGW(handle->base_addr,WARP_CTL_REG_ADDR, handle->reg_bank.ctl.overlay );   //0x00, warp config setting
}
/*******************************************************************************************************************
 * warp_hal_start
 *   Enable hw engine
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *   en : enable hardware engine
 *
 * Return:
 *
 */
void warp_hal_start(warp_hal_handle* handle, uint en)
{
    handle->reg_bank.ctl.fields.go = (en & 1);

    REGW(handle->base_addr,WARP_CTL_REG_ADDR, handle->reg_bank.ctl.overlay );//0x00, warp config setting
}
/*******************************************************************************************************************
 * warp_hal_get_hw_status
 *   Get hardware status
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   config: WARP configurations
 *   en : enable hardware engine
 *
 * Return:
 *
 */
void warp_hal_get_hw_status(warp_hal_handle* handle)
{
    u32 status = 0;
    status = (u32)REGR(handle->base_addr,WARP_STA_REG_ADDR);
    handle->reg_bank.sta.overlay = status;

    HAL_MSG(HAL_MSG_DBG, "REGR: addr 0x%02X,  value 0x%08X\n", WARP_STA_REG_ADDR, handle->reg_bank.sta.overlay);
}
/*******************************************************************************************************************
 * warp_hal_init
 *   init Warp HAL layer
 *
 * Parameters:
 *   handle: WARP HAL handle
 *   base_addr: base address
 *
 * Return:
 *   none
 */
void warp_hal_init(warp_hal_handle *handle, phys_addr_t base_addr)
{
    memset(handle, 0, sizeof(warp_hal_handle));
    handle->base_addr = base_addr;
}
