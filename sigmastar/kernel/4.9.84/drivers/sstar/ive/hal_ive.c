/*
* hal_ive.c- Sigmastar
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
#include "mdrv_ive.h"
#include "hal_ive.h"
#include "hal_ive_simulate.h"
#include "ms_platform.h"

#define HAL_MIU1_BASE               0x80000000

#define ADDR_LOW(addr)   (((u32)(addr))&0x0000FFFF)
#define ADDR_HIGH(addr)  ((((u32)(addr))&0xFFFF0000)>>16)

#if 1
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  ms_writew(val,((uint)base+(idx)*4))
#else
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  do{IVE_MSG(IVE_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)*4), val); ms_writew(val,((uint)base+(idx)*4));} while(0)
#endif

#define REMOVE_MIU1_OFFSET(input_addr)         ((u64)input_addr -0x80000000)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                                  //
//                                                                 IRQ API                                                                          //
//                                                                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************************************************
 * ive_hal_init
 *   init IVE HAL layer
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   base_addr0: base address 0
 *   base_addr1: base address 1
 *
 * Return:
 *   none
 */
void ive_hal_init(ive_hal_handle *handle, phys_addr_t base_addr0, phys_addr_t base_addr1)
{
    memset(handle, 0, sizeof(ive_hal_handle));
    handle->base_addr0 = base_addr0;
    handle->base_addr1 = base_addr1;
}


/*******************************************************************************************************************
 * ive_hal_set_irq_mask
 *   Set interrupt trigger mask
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_set_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank0.irq_mask = ~mask;
    REGW(handle->base_addr0, 0x10, handle->reg_bank0.reg10);
}

/*******************************************************************************************************************
 * ive_hal_clear_irq
 *   Clear triggered interrupt
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   none
 */
void ive_hal_clear_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
    handle->reg_bank0.woc_irq_clr = mask;
    REGW(handle->base_addr0, 0x15, handle->reg_bank0.reg15);
}

/*******************************************************************************************************************
 * ive_hal_get_irq_check
 *   Check current IRQ status
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwise mask to be checked
 *
 * Return:
 *   Checked result, bitwise
 */
IVE_HAL_IRQ_MASK ive_hal_get_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask)
{
#if !defined(IVE_SW_SIMULATE)
    handle->reg_bank0.irq_final_status = REGR(handle->base_addr0, 0x13);
#else  // defined(IVE_SW_SIMULATE)
    handle->reg_bank0.irq_final_status = IVE_HAL_IRQ_MASK_FRAME_DONE;
#endif // defined(IVE_SW_SIMULATE)

    return handle->reg_bank0.irq_final_status & mask;
}

/*******************************************************************************************************************
 * ive_hal_get_irq
 *   Get current interrupt trigger status
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: Bitwize mask, defined in IveHalIrqEvent_e
 *
 * Return:
 *   Bitwize status
 */
u16 ive_hal_get_irq(ive_hal_handle *handle)
{
#if !defined(IVE_SW_SIMULATE)
    handle->reg_bank0.irq_final_status = REGR(handle->base_addr0, 0x13);
#else  // defined(IVE_SW_SIMULATE)
    handle->reg_bank0.irq_final_status = IVE_HAL_IRQ_MASK_FRAME_DONE;
#endif // defined(IVE_SW_SIMULATE)

    return handle->reg_bank0.irq_final_status;
}


/*******************************************************************************************************************
 * ive_hal_set_operation
 *   Set operation of IVE HW
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   op_type: operation type
 *   op_mode: operation mode
 *
 * Return:
 *   none
 */
void ive_hal_set_operation(ive_hal_handle *handle, IVE_IOC_OP_TYPE op_type)
{
    handle->reg_bank1.op_type = op_type;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * ive_hal_miu_set
 *   Set miu value
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_miu_set(ive_hal_handle *handle, int miu_state)
{
    handle->reg_bank0.miu_sel = miu_state;

    REGW(handle->base_addr0, 0x03, handle->reg_bank0.reg03);
}

/*******************************************************************************************************************
 * ive_hal_set_images
 *   Set input & output image
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   input: input image
 *   output: output image
 *
 * Return:
 *   none
 */
void ive_hal_set_images(ive_hal_handle *handle, ive_ioc_image *input, ive_ioc_image *output)
{
    int miu_value;
    u64 input_addr0, input_addr1, input_addr2, output_addr0, output_addr1, output_addr2;

    IVE_MSG(IVE_MSG_DBG, "Ori -input : address[0] = 0x%p\n",(unsigned char*)input->address[0]);
    IVE_MSG(IVE_MSG_DBG, "Ori -input : address[1] = 0x%p\n",(unsigned char*)input->address[1]);
    IVE_MSG(IVE_MSG_DBG, "Ori -input : address[2] = 0x%p\n",(unsigned char*)input->address[2]);

#if defined(USE_MIU_DIRECT)
    input_addr0  = (uintptr_t)(input->address[0]);
    input_addr1  = (uintptr_t)(input->address[1]);
    input_addr2  = (uintptr_t)(input->address[2]);
    output_addr0 = (uintptr_t)(output->address[0]);
    output_addr1 = (uintptr_t)(output->address[1]);
    output_addr2 = (uintptr_t)(output->address[2]);
#else
    input_addr0  = Chip_Phys_to_MIU((uintptr_t)input->address[0]);
    input_addr1  = Chip_Phys_to_MIU((uintptr_t)input->address[1]);
    input_addr2  = Chip_Phys_to_MIU((uintptr_t)input->address[2]);
    output_addr0 = Chip_Phys_to_MIU((uintptr_t)output->address[0]);
    output_addr1 = Chip_Phys_to_MIU((uintptr_t)output->address[1]);
    output_addr2 = Chip_Phys_to_MIU((uintptr_t)output->address[2]);
#endif

    //Check address and switch MIU
    if( (input_addr0 >= HAL_MIU1_BASE) || (input_addr1 >= HAL_MIU1_BASE) || (input_addr2 >= HAL_MIU1_BASE) )
    {
        input_addr0 = REMOVE_MIU1_OFFSET(input_addr0);
        input_addr1 = REMOVE_MIU1_OFFSET(input_addr1);
        input_addr2 = REMOVE_MIU1_OFFSET(input_addr2);
        output_addr0 = REMOVE_MIU1_OFFSET(output_addr0);
        output_addr1 = REMOVE_MIU1_OFFSET(output_addr1);
        output_addr2 = REMOVE_MIU1_OFFSET(output_addr2);

        miu_value = 1;
        ive_hal_miu_set(handle, miu_value);
    }
    else
    {
        miu_value = 0;
        ive_hal_miu_set(handle, miu_value);
    }

    IVE_MSG(IVE_MSG_DBG, "Final - input_addr0 = %lld\n", input_addr0);
    IVE_MSG(IVE_MSG_DBG, "Final - input_addr1 = %lld\n", input_addr1);
    IVE_MSG(IVE_MSG_DBG, "Final - input_addr2 = %lld\n", input_addr2);
    IVE_MSG(IVE_MSG_DBG, "Final - output_addr0 = %lld\n", output_addr0);
    IVE_MSG(IVE_MSG_DBG, "Final - output_addr1 = %lld\n", output_addr1);
    IVE_MSG(IVE_MSG_DBG, "Final - output_addr2 = %lld\n", output_addr2);
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x03, REGR(handle->base_addr0, 0x03));  //MIU Register

    handle->reg_bank1.infmt   = input->format;
    handle->reg_bank1.outfmt  = output->format;

    handle->reg_bank1.frame_width  = input->width -1;
    handle->reg_bank1.frame_height = input->height -1;

    handle->reg_bank1.src1_addr_low  = ADDR_LOW( input_addr0);
    handle->reg_bank1.src1_addr_high = ADDR_HIGH(input_addr0);
    handle->reg_bank1.src2_addr_low  = ADDR_LOW( input_addr1);
    handle->reg_bank1.src2_addr_high = ADDR_HIGH(input_addr1);
    handle->reg_bank1.src3_addr_low  = ADDR_LOW( input_addr2);
    handle->reg_bank1.src3_addr_high = ADDR_HIGH(input_addr2);
    handle->reg_bank1.src1_stride    = input->stride[0] -1;
    handle->reg_bank1.src2_stride    = input->stride[1] -1;
    handle->reg_bank1.src3_stride    = input->stride[2] -1;

    handle->reg_bank1.dst1_addr_low  = ADDR_LOW( output_addr0);
    handle->reg_bank1.dst1_addr_high = ADDR_HIGH(output_addr0);
    handle->reg_bank1.dst2_addr_low  = ADDR_LOW( output_addr1);
    handle->reg_bank1.dst2_addr_high = ADDR_HIGH(output_addr1);
    handle->reg_bank1.dst3_addr_low  = ADDR_LOW( output_addr2);
    handle->reg_bank1.dst3_addr_high = ADDR_HIGH(output_addr2);
    handle->reg_bank1.dst1_stride    = output->stride[0] -1;
    handle->reg_bank1.dst2_stride    = output->stride[1] -1;
    handle->reg_bank1.dst3_stride    = output->stride[2] -1;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x06, handle->reg_bank1.reg06);
    REGW(handle->base_addr1, 0x07, handle->reg_bank1.reg07);
    REGW(handle->base_addr1, 0x08, handle->reg_bank1.reg08);
    REGW(handle->base_addr1, 0x09, handle->reg_bank1.reg09);
    REGW(handle->base_addr1, 0x0A, handle->reg_bank1.reg0A);
    REGW(handle->base_addr1, 0x0B, handle->reg_bank1.reg0B);
    REGW(handle->base_addr1, 0x0C, handle->reg_bank1.reg0C);
    REGW(handle->base_addr1, 0x0D, handle->reg_bank1.reg0D);
    REGW(handle->base_addr1, 0x0E, handle->reg_bank1.reg0E);
    REGW(handle->base_addr1, 0x0F, handle->reg_bank1.reg0F);
    REGW(handle->base_addr1, 0x10, handle->reg_bank1.reg10);
    REGW(handle->base_addr1, 0x11, handle->reg_bank1.reg11);
    REGW(handle->base_addr1, 0x12, handle->reg_bank1.reg12);
    REGW(handle->base_addr1, 0x13, handle->reg_bank1.reg13);
    REGW(handle->base_addr1, 0x14, handle->reg_bank1.reg14);
    REGW(handle->base_addr1, 0x15, handle->reg_bank1.reg15);
    REGW(handle->base_addr1, 0x16, handle->reg_bank1.reg16);
    REGW(handle->base_addr1, 0x17, handle->reg_bank1.reg17);
    REGW(handle->base_addr1, 0x18, handle->reg_bank1.reg18);
    REGW(handle->base_addr1, 0x19, handle->reg_bank1.reg19);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_mask
 *   Set mask coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   mask: mask coefficient
 *
 * Return:
 *   none
 */
static void ive_hal_set_coeff_mask(ive_hal_handle *handle, u8 *mask, u8 shift)
{
    handle->reg_bank1.mask0  = mask[0];
    handle->reg_bank1.mask1  = mask[1];
    handle->reg_bank1.mask2  = mask[2];
    handle->reg_bank1.mask3  = mask[3];
    handle->reg_bank1.mask4  = mask[4];
    handle->reg_bank1.mask5  = mask[5];
    handle->reg_bank1.mask6  = mask[6];
    handle->reg_bank1.mask7  = mask[7];
    handle->reg_bank1.mask8  = mask[8];
    handle->reg_bank1.mask9  = mask[9];
    handle->reg_bank1.mask10 = mask[10];
    handle->reg_bank1.mask11 = mask[11];
    handle->reg_bank1.mask12 = mask[12];
    handle->reg_bank1.mask13 = mask[13];
    handle->reg_bank1.mask14 = mask[14];
    handle->reg_bank1.mask15 = mask[15];
    handle->reg_bank1.mask16 = mask[16];
    handle->reg_bank1.mask17 = mask[17];
    handle->reg_bank1.mask18 = mask[18];
    handle->reg_bank1.mask19 = mask[19];
    handle->reg_bank1.mask20 = mask[20];
    handle->reg_bank1.mask21 = mask[21];
    handle->reg_bank1.mask22 = mask[22];
    handle->reg_bank1.mask23 = mask[23];
    handle->reg_bank1.mask24 = mask[24];
    handle->reg_bank1.shift  = shift;

    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x1C, handle->reg_bank1.reg1C);
    REGW(handle->base_addr1, 0x1D, handle->reg_bank1.reg1D);
    REGW(handle->base_addr1, 0x1E, handle->reg_bank1.reg1E);
    REGW(handle->base_addr1, 0x1F, handle->reg_bank1.reg1F);
    REGW(handle->base_addr1, 0x20, handle->reg_bank1.reg20);
    REGW(handle->base_addr1, 0x21, handle->reg_bank1.reg21);
    REGW(handle->base_addr1, 0x22, handle->reg_bank1.reg22);
    REGW(handle->base_addr1, 0x23, handle->reg_bank1.reg23);
    REGW(handle->base_addr1, 0x24, handle->reg_bank1.reg24);
    REGW(handle->base_addr1, 0x25, handle->reg_bank1.reg25);
    REGW(handle->base_addr1, 0x26, handle->reg_bank1.reg26);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_filter
 *   Set filter coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_filter(ive_hal_handle *handle, ive_ioc_coeff_filter *coeff)
{
    ive_hal_set_coeff_mask(handle, coeff->mask, coeff->shift);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_csc
 *   Set CSC coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: CSC coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_csc(ive_hal_handle *handle, ive_ioc_coeff_csc *coeff)
{
    handle->reg_bank1.csc_coeff0 = coeff->coeff[0];
    handle->reg_bank1.csc_coeff1 = coeff->coeff[1];
    handle->reg_bank1.csc_coeff2 = coeff->coeff[2];
    handle->reg_bank1.csc_coeff3 = coeff->coeff[3];
    handle->reg_bank1.csc_coeff4 = coeff->coeff[4];
    handle->reg_bank1.csc_coeff5 = coeff->coeff[5];
    handle->reg_bank1.csc_coeff6 = coeff->coeff[6];
    handle->reg_bank1.csc_coeff7 = coeff->coeff[7];
    handle->reg_bank1.csc_coeff8 = coeff->coeff[8];

    handle->reg_bank1.csc_offset0 = coeff->offset[0];
    handle->reg_bank1.csc_offset1 = coeff->offset[1];
    handle->reg_bank1.csc_offset2 = coeff->offset[2];

    handle->reg_bank1.csc_clamp0_low  = coeff->clamp[0].clamp_low;
    handle->reg_bank1.csc_clamp0_high = coeff->clamp[0].clamp_high;
    handle->reg_bank1.csc_clamp1_low  = coeff->clamp[1].clamp_low;
    handle->reg_bank1.csc_clamp1_high = coeff->clamp[1].clamp_high;
    handle->reg_bank1.csc_clamp2_low  = coeff->clamp[2].clamp_low;
    handle->reg_bank1.csc_clamp2_high = coeff->clamp[2].clamp_high;

    REGW(handle->base_addr1, 0x30, handle->reg_bank1.reg30);
    REGW(handle->base_addr1, 0x31, handle->reg_bank1.reg31);
    REGW(handle->base_addr1, 0x32, handle->reg_bank1.reg32);
    REGW(handle->base_addr1, 0x33, handle->reg_bank1.reg33);
    REGW(handle->base_addr1, 0x34, handle->reg_bank1.reg34);
    REGW(handle->base_addr1, 0x35, handle->reg_bank1.reg35);
    REGW(handle->base_addr1, 0x36, handle->reg_bank1.reg36);
    REGW(handle->base_addr1, 0x37, handle->reg_bank1.reg37);
    REGW(handle->base_addr1, 0x38, handle->reg_bank1.reg38);
    REGW(handle->base_addr1, 0x39, handle->reg_bank1.reg39);
    REGW(handle->base_addr1, 0x3A, handle->reg_bank1.reg3A);
    REGW(handle->base_addr1, 0x3B, handle->reg_bank1.reg3B);
    REGW(handle->base_addr1, 0x3C, handle->reg_bank1.reg3C);
    REGW(handle->base_addr1, 0x3D, handle->reg_bank1.reg3D);
    REGW(handle->base_addr1, 0x3E, handle->reg_bank1.reg3E);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_sobel
 *   Set sobel coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_sobel(ive_hal_handle *handle, ive_ioc_coeff_sobel *coeff)
{
    ive_hal_set_coeff_mask(handle, coeff->mask, 0);

    handle->reg_bank1.outfmt  = coeff->mode;
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_mag_and_ang
 *   Set mag and ang coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_mag_and_ang(ive_hal_handle *handle, ive_ioc_coeff_mag_and_ang *coeff)
{
    ive_hal_set_coeff_mask(handle, coeff->mask, 0);

    handle->reg_bank1.outfmt  = coeff->mode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_ord_stat_filter
 *   Set order statistics filter coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_ord_stat_filter(ive_hal_handle *handle, ive_ioc_coeff_ord_stat_filter *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_bernsen
 *   Set bernsen coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_bernsen(ive_hal_handle *handle, ive_ioc_coeff_bernsen *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_dilate
 *   Set dilate coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_dilate(ive_hal_handle *handle, ive_ioc_coeff_dilate *coeff)
{
    ive_hal_set_coeff_mask(handle, coeff->mask, 0);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_erode
 *   Set erode coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_erode(ive_hal_handle *handle, ive_ioc_coeff_erode *coeff)
{
    ive_hal_set_coeff_mask(handle, coeff->mask, 0);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_thresh
 *   Set thresh coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_thresh(ive_hal_handle *handle, ive_ioc_coeff_thresh *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.mask0  = coeff->min;
    handle->reg_bank1.mask1  = coeff->mid;
    handle->reg_bank1.mask2  = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_thresh_s16
 *   Set thresh s16 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_thresh_s16(ive_hal_handle *handle, ive_ioc_coeff_thresh_s16 *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.mask0  = coeff->min;
    handle->reg_bank1.mask1  = coeff->mid;
    handle->reg_bank1.mask2  = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_thresh_u16
 *   Set thresh u16 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_thresh_u16(ive_hal_handle *handle, ive_ioc_coeff_thresh_u16 *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.mask0  = coeff->min;
    handle->reg_bank1.mask1  = coeff->mid;
    handle->reg_bank1.mask2  = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->low;
    handle->reg_bank1.thresh_16bit_2 = coeff->high;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}


/*******************************************************************************************************************
 * ive_hal_set_coeff_add
 *   Set add coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_add(ive_hal_handle *handle, ive_ioc_coeff_add *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.add_weight_x = coeff->weight_x;
    handle->reg_bank1.add_weight_y = coeff->weight_y;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x2B, handle->reg_bank1.reg2B);
    REGW(handle->base_addr1, 0x2C, handle->reg_bank1.reg2C);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_sub
 *   Set sub coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_sub(ive_hal_handle *handle, ive_ioc_coeff_sub *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}


/*******************************************************************************************************************
 * ive_hal_set_coeff_16to8
 *   Set 16 to 8 coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_16to8(ive_hal_handle *handle, ive_ioc_coeff_16to8 *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.fraction = (u16)(((u32)coeff->numerator << 16) / (u32)coeff->denominator);
    handle->reg_bank1.mask0 = coeff->bias;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x2A, handle->reg_bank1.reg2A);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_map
 *   Set map coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_map(ive_hal_handle *handle, u64 map_addr)
{
#if defined(USE_MIU_DIRECT)
    u64 miu_addr = map_addr;
#else
    u64 miu_addr = Chip_Phys_to_MIU(map_addr);
#endif

    //Check  address
    if( miu_addr >= HAL_MIU1_BASE )
    {
        miu_addr = REMOVE_MIU1_OFFSET(miu_addr);
    }

    handle->reg_bank1.src1_addr_low  = ADDR_LOW( miu_addr);
    handle->reg_bank1.src1_addr_high = ADDR_HIGH(miu_addr);

    REGW(handle->base_addr1, 0x08, handle->reg_bank1.reg08);
    REGW(handle->base_addr1, 0x09, handle->reg_bank1.reg09);
}


/*******************************************************************************************************************
 * ive_hal_set_coeff_integral
 *   Set integral coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_integral(ive_hal_handle *handle, ive_ioc_coeff_integral *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_sad
 *   Set SAD coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_sad(ive_hal_handle *handle, ive_ioc_coeff_sad *coeff)
{
    handle->reg_bank1.op_mode = coeff->block_mode;
    handle->reg_bank1.outfmt  = coeff->out_mode;
    handle->reg_bank1.mask0  = coeff->min;
    handle->reg_bank1.mask1  = coeff->max;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_lbp
 *   Set LBP coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_ncc(ive_hal_handle *handle, u64 output_addr)
{
#if defined(USE_MIU_DIRECT)
    u64 miu_addr = output_addr;
#else
    u64 miu_addr = Chip_Phys_to_MIU(output_addr);
#endif

     //Check address
    if( miu_addr >= HAL_MIU1_BASE )
    {
        miu_addr = REMOVE_MIU1_OFFSET(miu_addr);
    }

    handle->reg_bank1.dst1_addr_low  = ADDR_LOW( miu_addr);
    handle->reg_bank1.dst1_addr_high = ADDR_HIGH(miu_addr);
    // handle->reg_bank1.dst1_stride = handle->reg_bank1.src1_stride;

    REGW(handle->base_addr1, 0x0A, handle->reg_bank1.reg0A);
    REGW(handle->base_addr1, 0x0B, handle->reg_bank1.reg0B);
    // REGW(handle->base_addr1, 0x15, handle->reg_bank1.reg15);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_lbp
 *   Set LBP coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_lbp(ive_hal_handle *handle, ive_ioc_coeff_lbp *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.infmt = coeff->chlmode;
    handle->reg_bank1.thresh_16bit_1 = coeff->thresh-1;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_bat
 *   Set bat coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_bat(ive_hal_handle *handle, ive_ioc_coeff_bat *coeff)
{
    handle->reg_bank1.thresh_16bit_1 = coeff->h_times;
    handle->reg_bank1.thresh_16bit_2 = coeff->v_times;

    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x29, handle->reg_bank1.reg29);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_adp_thresh
 *   Set adaptive threshold coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_adp_thresh(ive_hal_handle *handle, ive_ioc_coeff_adp_thresh *coeff)
{
    handle->reg_bank1.mask0             = coeff->u8HalfMaskx;
    handle->reg_bank1.mask1             = coeff->u8HalfMasky;
    handle->reg_bank1.shift             = coeff->s8Offset;
    handle->reg_bank1.thresh_16bit_1    = coeff->u8ValueThr;
    handle->reg_bank1.add_weight_x      = coeff->u8RateThr;

    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x26, handle->reg_bank1.reg26);
    REGW(handle->base_addr1, 0x28, handle->reg_bank1.reg28);
    REGW(handle->base_addr1, 0x2B, handle->reg_bank1.reg2B);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_matrix_transform
 *   Set matrix transform coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_matrix_transform(ive_hal_handle *handle, ive_ioc_coeff_matrix_transform *coeff)
{
    handle->reg_bank1.op_mode = coeff->ctrl_mode;

    switch( coeff->chl_mode )
    {
        case IVE_IOC_MODE_MATRIX_TRANSFORM_C1:
        handle->reg_bank1.infmt = 13;
        handle->reg_bank1.outfmt = 13;

                handle->reg_bank1.mask0 = coeff->s16MatrixArray[0] % 256;
        handle->reg_bank1.mask1 = coeff->s16MatrixArray[0] /256;
                handle->reg_bank1.mask2 = 0;
        handle->reg_bank1.mask3 = 0;
        handle->reg_bank1.mask4 = 0;
        handle->reg_bank1.mask5 = 0;
        handle->reg_bank1.mask6 = 0;
        handle->reg_bank1.mask7 = 0;
        handle->reg_bank1.mask8 = 0;
        handle->reg_bank1.mask9 = 0;
        handle->reg_bank1.mask10 = 0;
        handle->reg_bank1.mask11 = 0;
        handle->reg_bank1.mask12 = 0;
        handle->reg_bank1.mask13 = 0;
        handle->reg_bank1.mask14 = 0;
        handle->reg_bank1.mask15 = 0;
        handle->reg_bank1.mask16 = 0;
        handle->reg_bank1.mask17 = 0;
        break;

    case IVE_IOC_MODE_MATRIX_TRANSFORM_C2:
        handle->reg_bank1.infmt = 14;
        handle->reg_bank1.outfmt = 14;

        handle->reg_bank1.mask0 = coeff->s16MatrixArray[0] % 256;
        handle->reg_bank1.mask1 = coeff->s16MatrixArray[0] /256;
                handle->reg_bank1.mask2 = coeff->s16MatrixArray[1] % 256;
        handle->reg_bank1.mask3 = coeff->s16MatrixArray[1] /256;
        handle->reg_bank1.mask4 = 0;
        handle->reg_bank1.mask5 = 0;
        handle->reg_bank1.mask6 = coeff->s16MatrixArray[2] % 256;
        handle->reg_bank1.mask7 = coeff->s16MatrixArray[2] /256;
        handle->reg_bank1.mask8 = coeff->s16MatrixArray[3] % 256;
        handle->reg_bank1.mask9 = coeff->s16MatrixArray[3] /256;
        handle->reg_bank1.mask10 = 0;
        handle->reg_bank1.mask11 = 0;
        handle->reg_bank1.mask12 = 0;
        handle->reg_bank1.mask13 = 0;
        handle->reg_bank1.mask14 = 0;
        handle->reg_bank1.mask15 = 0;
        handle->reg_bank1.mask16 = 0;
        handle->reg_bank1.mask17 = 0;
        break;

    case IVE_IOC_MODE_MATRIX_TRANSFORM_C3:
        handle->reg_bank1.infmt = 15;
        handle->reg_bank1.outfmt = 15;

        handle->reg_bank1.mask0 = coeff->s16MatrixArray[0] % 256;
        handle->reg_bank1.mask1 = coeff->s16MatrixArray[0] /256;
                handle->reg_bank1.mask2 = coeff->s16MatrixArray[1] % 256;
        handle->reg_bank1.mask3 = coeff->s16MatrixArray[1] /256;
        handle->reg_bank1.mask4 = coeff->s16MatrixArray[2] % 256;
        handle->reg_bank1.mask5 = coeff->s16MatrixArray[2] /256;
        handle->reg_bank1.mask6 = coeff->s16MatrixArray[3] % 256;
        handle->reg_bank1.mask7 = coeff->s16MatrixArray[3] /256;
        handle->reg_bank1.mask8 = coeff->s16MatrixArray[4] % 256;
        handle->reg_bank1.mask9 = coeff->s16MatrixArray[4] /256;
        handle->reg_bank1.mask10 = coeff->s16MatrixArray[5] % 256;
        handle->reg_bank1.mask11 = coeff->s16MatrixArray[5] /256;
        handle->reg_bank1.mask12 = coeff->s16MatrixArray[6] % 256;
        handle->reg_bank1.mask13 = coeff->s16MatrixArray[6] /256;
        handle->reg_bank1.mask14 = coeff->s16MatrixArray[7] % 256;
        handle->reg_bank1.mask15 = coeff->s16MatrixArray[7] /256;
        handle->reg_bank1.mask16 = coeff->s16MatrixArray[8] % 256;
        handle->reg_bank1.mask17 = coeff->s16MatrixArray[8] /256;
        break;
    default :
        IVE_MSG(IVE_MSG_ERR, "Invalid input type of matrix transform!!\n");
        break;
    }



    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);

    REGW(handle->base_addr1, 0x1A, handle->reg_bank1.reg1A);
    REGW(handle->base_addr1, 0x1B, handle->reg_bank1.reg1B);
    REGW(handle->base_addr1, 0x1C, handle->reg_bank1.reg1C);
    REGW(handle->base_addr1, 0x1D, handle->reg_bank1.reg1D);
    REGW(handle->base_addr1, 0x1E, handle->reg_bank1.reg1E);
    REGW(handle->base_addr1, 0x1F, handle->reg_bank1.reg1F);
    REGW(handle->base_addr1, 0x20, handle->reg_bank1.reg20);
    REGW(handle->base_addr1, 0x21, handle->reg_bank1.reg21);
    REGW(handle->base_addr1, 0x22, handle->reg_bank1.reg22);
}

/*******************************************************************************************************************
 * ive_hal_set_coeff_image_dot
 *   Set image dot coefficient
 *
 * Parameters:
 *   handle: IVE HAL handle
 *   coeff: coefficient
 *
 * Return:
 *   none
 */
void ive_hal_set_coeff_image_dot(ive_hal_handle *handle, ive_ioc_coeff_image_dot *coeff)
{
    handle->reg_bank1.op_mode = coeff->mode;
    handle->reg_bank1.infmt = 14;

    REGW(handle->base_addr1, 0x04, handle->reg_bank1.reg04);
    REGW(handle->base_addr1, 0x05, handle->reg_bank1.reg05);
}

/*******************************************************************************************************************
 * ive_hal_start
 *   start IVE HW engine to process images
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
 void ive_hal_start(ive_hal_handle *handle)
{
#if !defined(IVE_SW_SIMULATE)
    handle->reg_bank0.sw_fire = 1;

    REGW(handle->base_addr0, 0x00, handle->reg_bank0.reg00);

    handle->reg_bank0.sw_fire = 0; // write one clear
#else  // !defined(IVE_SW_SIMULATE)
    ive_hal_run_simulate(handle);
#endif // defined(IVE_SW_SIMULATE)
}

/*******************************************************************************************************************
 * ive_hal_sw_reset
 *   reset IVE HW engine
 *
 * Parameters:
 *   handle: IVE HAL handle
 *
 * Return:
 *   none
 */
void ive_hal_sw_reset(ive_hal_handle *handle)
{
    handle->reg_bank0.sw_rst = 1;

    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);

    handle->reg_bank0.sw_rst = 0; // write one clear

    REGW(handle->base_addr0, 0x02, handle->reg_bank0.reg02);

    memset(&handle->reg_bank0, 0, sizeof(handle->reg_bank0));
    memset(&handle->reg_bank1, 0, sizeof(handle->reg_bank1));
}

void ive_hal_reg_dump(ive_hal_handle *handle)
{
    IVE_MSG(IVE_MSG_DBG, "clock reg = 0x%04x\n", REGR(0x1F207000, 0x6A));

    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x00, REGR(handle->base_addr0, 0x00));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x01, REGR(handle->base_addr0, 0x01));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x02, REGR(handle->base_addr0, 0x02));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x03, REGR(handle->base_addr0, 0x03));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x10, REGR(handle->base_addr0, 0x10));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x11, REGR(handle->base_addr0, 0x11));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x12, REGR(handle->base_addr0, 0x12));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x13, REGR(handle->base_addr0, 0x13));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x14, REGR(handle->base_addr0, 0x14));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x15, REGR(handle->base_addr0, 0x15));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x16, REGR(handle->base_addr0, 0x16));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x17, REGR(handle->base_addr0, 0x17));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x18, REGR(handle->base_addr0, 0x18));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x20, REGR(handle->base_addr0, 0x20));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x21, REGR(handle->base_addr0, 0x21));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x22, REGR(handle->base_addr0, 0x22));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x23, REGR(handle->base_addr0, 0x23));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x24, REGR(handle->base_addr0, 0x24));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x25, REGR(handle->base_addr0, 0x25));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x30, REGR(handle->base_addr0, 0x30));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x31, REGR(handle->base_addr0, 0x31));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x40, REGR(handle->base_addr0, 0x40));
    IVE_MSG(IVE_MSG_DBG, "bank 0 reg 0x%02X = 0x%04x\n", 0x41, REGR(handle->base_addr0, 0x41));

    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x00, REGR(handle->base_addr1, 0x00));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x01, REGR(handle->base_addr1, 0x01));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x02, REGR(handle->base_addr1, 0x02));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x03, REGR(handle->base_addr1, 0x03));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x04, REGR(handle->base_addr1, 0x04));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x05, REGR(handle->base_addr1, 0x05));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x06, REGR(handle->base_addr1, 0x06));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x07, REGR(handle->base_addr1, 0x07));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x08, REGR(handle->base_addr1, 0x08));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x09, REGR(handle->base_addr1, 0x09));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0A, REGR(handle->base_addr1, 0x0A));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0B, REGR(handle->base_addr1, 0x0B));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0C, REGR(handle->base_addr1, 0x0C));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0D, REGR(handle->base_addr1, 0x0D));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0E, REGR(handle->base_addr1, 0x0E));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x0F, REGR(handle->base_addr1, 0x0F));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x10, REGR(handle->base_addr1, 0x10));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x11, REGR(handle->base_addr1, 0x11));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x12, REGR(handle->base_addr1, 0x12));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x13, REGR(handle->base_addr1, 0x13));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x14, REGR(handle->base_addr1, 0x14));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x15, REGR(handle->base_addr1, 0x15));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x16, REGR(handle->base_addr1, 0x16));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x17, REGR(handle->base_addr1, 0x17));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x18, REGR(handle->base_addr1, 0x18));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x19, REGR(handle->base_addr1, 0x19));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1A, REGR(handle->base_addr1, 0x1A));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1B, REGR(handle->base_addr1, 0x1B));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1C, REGR(handle->base_addr1, 0x1C));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1D, REGR(handle->base_addr1, 0x1D));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1E, REGR(handle->base_addr1, 0x1E));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x1F, REGR(handle->base_addr1, 0x1F));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x20, REGR(handle->base_addr1, 0x20));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x21, REGR(handle->base_addr1, 0x21));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x22, REGR(handle->base_addr1, 0x22));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x23, REGR(handle->base_addr1, 0x23));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x24, REGR(handle->base_addr1, 0x24));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x25, REGR(handle->base_addr1, 0x25));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x26, REGR(handle->base_addr1, 0x26));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x27, REGR(handle->base_addr1, 0x27));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x28, REGR(handle->base_addr1, 0x28));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x29, REGR(handle->base_addr1, 0x29));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2A, REGR(handle->base_addr1, 0x2A));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2B, REGR(handle->base_addr1, 0x2B));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2C, REGR(handle->base_addr1, 0x2C));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2D, REGR(handle->base_addr1, 0x2D));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2E, REGR(handle->base_addr1, 0x2E));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x2F, REGR(handle->base_addr1, 0x2F));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x30, REGR(handle->base_addr1, 0x30));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x31, REGR(handle->base_addr1, 0x31));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x32, REGR(handle->base_addr1, 0x32));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x33, REGR(handle->base_addr1, 0x33));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x34, REGR(handle->base_addr1, 0x34));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x35, REGR(handle->base_addr1, 0x35));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x36, REGR(handle->base_addr1, 0x36));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x37, REGR(handle->base_addr1, 0x37));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x38, REGR(handle->base_addr1, 0x38));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x39, REGR(handle->base_addr1, 0x39));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x3A, REGR(handle->base_addr1, 0x3A));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x3B, REGR(handle->base_addr1, 0x3B));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x3C, REGR(handle->base_addr1, 0x3C));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x3D, REGR(handle->base_addr1, 0x3D));
    IVE_MSG(IVE_MSG_DBG, "bank 1 reg 0x%02X = 0x%04x\n", 0x3E, REGR(handle->base_addr1, 0x3E));
}
