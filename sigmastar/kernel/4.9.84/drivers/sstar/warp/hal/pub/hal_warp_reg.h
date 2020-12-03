/*
* hal_warp_reg.h- Sigmastar
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
#ifndef __HAL_WARP_REG_H__
#define __HAL_WARP_REG_H__

//#define  WARP_BASE            (0x0)
#define  REG_ADDR(a)              (uint)(a)//(uint*)(WARP_BASE + (a))

#define  WARP_CTL_REG_ADDR        REG_ADDR(0x00)
#define  WARP_STA_REG_ADDR        REG_ADDR(0x04)
#define  WARP_AXI_CFG1_REG_ADDR   REG_ADDR(0x08)
#define  WARP_AXI_CFG2_REG_ADDR   REG_ADDR(0x0C)
#define  WARP_DLUA_YRGB_REG_ADDR  REG_ADDR(0x10)
#define  WARP_DLUA_UV_REG_ADDR    REG_ADDR(0x14)
#define  WARP_DLUA_VB_REG_ADDR    REG_ADDR(0x18)
#define  WARP_CDLA_REG_ADDR       REG_ADDR(0x1C)
#define  WARP_DSUA_YRGB_REG_ADDR  REG_ADDR(0x20)
#define  WARP_DSUA_UV_REG_ADDR    REG_ADDR(0x24)
#define  WARP_DSUA_VB_REG_ADDR    REG_ADDR(0x28)
#define  WARP_CDSA_REG_ADDR       REG_ADDR(0x2C)
#define  WARP_OCS_REG_ADDR        REG_ADDR(0x30)
#define  WARP_OCSNUM_REG_ADDR     REG_ADDR(0x34)
#define  WARP_IFSZ_REG_ADDR       REG_ADDR(0x38)
#define  WARP_OFSZ_REG_ADDR       REG_ADDR(0x3C)
#define  WARP_DISTBA_REG_ADDR     REG_ADDR(0x40)
#define  WARP_BBA_REG_ADDR        REG_ADDR(0x44)
#define  WARP_PDC_C00_REGS_ADDR   REG_ADDR(0x48)
#define  WARP_PDC_C01_REGS_ADDR   REG_ADDR(0x4C)
#define  WARP_PDC_C02_REGS_ADDR   REG_ADDR(0x50)
#define  WARP_PDC_C10_REGS_ADDR   REG_ADDR(0x54)
#define  WARP_PDC_C11_REGS_ADDR   REG_ADDR(0x58)
#define  WARP_PDC_C12_REGS_ADDR   REG_ADDR(0x5C)
#define  WARP_PDC_C20_REGS_ADDR   REG_ADDR(0x60)
#define  WARP_PDC_C21_REGS_ADDR   REG_ADDR(0x64)
#define  WARP_PDC_C22_REGS_ADDR   REG_ADDR(0x68)
#define  WARP_PDOFFSET_REG_ADDR   REG_ADDR(0x6C)
#define  WARP_CSFV_REG_ADDR       REG_ADDR(0x70)

typedef struct {
    int go              : 1;
    int reserved1       : 1;
    int dispm           : 1;
    int dxym            : 1;
    int afbcm           : 1;
    int eofie           : 1;
    int oie             : 1;
    int reserved2       : 1;
    int axierie         : 1;
    int dtibsz          : 2;
    int reserved3       : 2;
    int iif             : 3;
    int bpdu            : 1;
    int bpiu            : 1;
    int reserved4       :14;
} wrp_ctl_reg_bits_t;

typedef union
{
    wrp_ctl_reg_bits_t fields;
    unsigned int overlay;
} wrp_ctl_t;

typedef struct {
    unsigned int idle            : 1;
    unsigned int oc              : 1;
    unsigned int reserved1       : 1;
    unsigned int axierr          : 1;
    unsigned int reserved2       :12;
    unsigned int version         : 8;
    unsigned int id              : 8;
} wrp_sta_reg_bits_t;

typedef union
{
    wrp_sta_reg_bits_t fields;
    unsigned int overlay;
} wrp_sta_t;

typedef struct {
    char reserved1;
    unsigned char rrc_c;
    unsigned char wrc;
    unsigned char rrc;
} wrp_axi_cfg1_reg_bits_t;

typedef union
{
    wrp_axi_cfg1_reg_bits_t fields;
    unsigned int overlay;
} wrp_axi_cfg1_t;

typedef struct {
    unsigned char mwb;
    unsigned char mrb;
    unsigned char moutstw;
    unsigned char moutstr;
} wrp_axi_cfg2_reg_bits_t;

typedef union
{
    wrp_axi_cfg2_reg_bits_t fields;
    unsigned int overlay;
} wrp_axi_cfg2_t;

typedef struct {
    unsigned short ocsx;
    unsigned short ocsy;
} wrp_ocs_reg_bits_t;

typedef union
{
    wrp_ocs_reg_bits_t fields;
    unsigned int overlay;
} wrp_ocs_t;

typedef struct {
    unsigned short ifszx;
    unsigned short ifszy;
} wrp_ifsz_reg_bits_t;

typedef union
{
    wrp_ifsz_reg_bits_t fields;
    unsigned int overlay;
} wrp_ifsz_t;

typedef struct {
    unsigned short ofszx;
    unsigned short ofszy;
} wrp_ofsz_reg_bits_t;

typedef union
{
    wrp_ofsz_reg_bits_t fields;
    unsigned int overlay;
} wrp_ofsz_t;

typedef struct {
    unsigned char yfv_rgbfv;
    unsigned char uvfv;
    unsigned short reserved;
} wrp_csfv_reg_bits_t;

typedef union
{
    wrp_csfv_reg_bits_t fields;
    unsigned int overlay;
} wrp_csfv_t;

typedef struct
{
    wrp_ctl_t           ctl;        //0x00, WRP_CTL
    wrp_sta_t           sta;        //0x04, WRP_STA
    wrp_axi_cfg1_t      axi_cfg1;   //0x08, AXI_CFG1
    wrp_axi_cfg2_t      axi_cfg2;   //0x0C, AXI_CFG2
    int                 dlua_yrgb;  //0x10, DLUA_YRGB
    int                 dlua_uvg;   //0x14, DLUA_UV
    int                 cdla;       //0x1C, CDLA
    int                 dsua_yrgb;  //0x20, DSUA_YRGB
    int                 dsua_uvg;   //0x24, DSUA_UV
    int                 dsua_vb;
    int                 cdsa;       //0x2C, CDSA
    wrp_ocs_t           ocs;        //0x30, OCS
    int                 ocsnum;
    wrp_ifsz_t          ifsz;       //0x38, IFSZ
    wrp_ofsz_t          ofsz;       //0x3C, OFSZ
    int                 distba;     //0x40, DUSTBA
    int                 bba;        //0x44, BBA
    int                 c00;        //0x48, PDC_C00
    int                 c01;        //0x4C, PDC_C01
    int                 c02;        //0x50, PDC_C02
    int                 c10;        //0x54, PDC_C10
    int                 c11;        //0x58, PDC_C11
    int                 c12;        //0x5C, PDC_C12
    int                 c20;        //0x60, PDC_C20
    int                 c21;        //0x64, PDC_C21
    int                 c22;        //0x68, PDC_C22
    wrp_csfv_t          csfv;       //0x70, CSFV

} warp_hal_reg_bank;

#endif // __HAL_WARP_REG_H__
