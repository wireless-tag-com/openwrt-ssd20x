/*
* hal_ceva_reg.h- Sigmastar
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
#ifndef __HAL_CEVA_REG_H__
#define __HAL_CEVA_REG_H__

#include <linux/kernel.h>

typedef struct
{
    union
    {
        struct
        {
            u16 reg_forbidden:1;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 reg_div_num_bus:4;
            u16 reg_div_num_wdog:4;
            u16 reg_div_num_warp:4;
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 reg_rstz_ceva_core:1;
            u16 reg_rstz_ceva_sys:1;
            u16 reg_rstz_ceva_ocem:1;
            u16 reg_rstz_ceva_global:1;
            u16 reg_rstz_miu:1;
            u16 reg_rstz_mcu:1;
            u16 reg_rstz_mcu2ceva:1;
            u16 reg_rstz_isp2ceva:1;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 reg_ceva2riu_int_en:16;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 reg_ceva2riu_int_en2:16;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 reg_ceva_int0_en:16;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 reg_ceva_int1_en:16;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 reg_ceva_int2_en:16;
        };
        u16 reg07;
    };

    union
    {
        struct
        {
            u16 reg_ceva_nmi_en:16;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 reg_ceva_vint_en:16;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 reg_uop_int_wc:1;
        };
        u16 reg0a;
    };

    union
    {
        struct
        {
            u16 reg_mcci_rd_wc_low:16;
        };
        u16 reg0b;
    };

    union
    {
        struct
        {
            u16 reg_mcci_rd_wc_high:16;
        };
        u16 reg0c;
    };

    union
    {
        struct
        {
            u16 reg_mcci_rd_ind_low:16;
        };
        u16 reg0d;
    };

    union
    {
        struct
        {
            u16 reg_mcci_rd_ind_high:16;
        };
        u16 reg0e;
    };

    union
    {
        struct
        {
            u16 reg_warp_r1_sel:1;
            u16 reg_warp_r2_sel:1;
            u16 reg_warp_w1_sel:1;
            u16 reg_warp_w1_sel2:1;
            u16 reg_rq_mask_miu:4;
            u16 reg_rq_mask_imi:4;
        };
        u16 reg0f;
    };

    union
    {
        struct
        {
            u16 reg_ceva_dbus_low:16;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 reg_ceva_dbus_high:8;
            u16 reg_ceva_dbus_sel:8;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 reg_clktest_in:8;
            u16 reg_clktest_out:8;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 reg_ceva2mcu_ldz_en:1;
            u16 reg_ceva2isp_ldz_en:1;
            u16 reg_ceva_eflag_wc:1;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 reg_mi02ceva_last_done_z:1;
            u16 reg_mi12ceva_last_done_z:1;
            u16 reg_mi22ceva_last_done_z:1;
            u16 reg_mi32ceva_last_done_z:1;
            u16 reg_imi02ceva_last_done_z:1;
            u16 reg_imi12ceva_last_done_z:1;
            u16 reg_imi22ceva_last_done_z:1;
            u16 reg_imi32ceva_last_done_z:1;
            u16 reg_ceva2mcu_last_done_z:1;
            u16 reg_ceva2isp_last_done_z:1;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 reg_ceva_boot:1;
            u16 reg_ceva_csysreq:1;
            u16 reg_ceva_core_rcvr:1;
            u16 reg_ceva_external_wait:1;
            u16 reg_ceva_mcache_invalidate_strap:1;
            u16 reg_ceva_acu_lock:1;
            u16 reg_ceva_acu_slv_acc:1;
            u16 reg_ceva_ddma_dbg_match_ack:1;
            u16 reg_ceva_next_ddma:1;
            u16 reg_ceva_bs_reg_tdo:1;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 reg_ceva_vector_low:16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 reg_ceva_vector_high:16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 reg_ceva_is:16;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 reg_ceva_is2:16;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 reg_ceva_psu_dsp_idle:1;
            u16 reg_ceva_psu_core_idle:1;
            u16 reg_ceva_psu_core_wait:1;
            u16 reg_ceva_psu_cactive:1;
            u16 reg_ceva_psu_csysack:1;
            u16 reg_ceva_epp_aps:1;
            u16 reg_ceva_iop_aps:1;
            u16 reg_ceva_edp_aps:1;
        };
        u16 reg1a;
    };

    union
    {
        struct
        {
            u16 reg_ceva_ocm_gp_out:4;
            u16 reg_ceva_ocm_jtag_state:4;
            u16 reg_ceva_ocm_core_rst:1;
            u16 reg_ceva_ocm_debug:1;
            u16 reg_ceva_cverbit:1;
            u16 reg_ceva_seq_eotbit:1;
            u16 reg_ceva_seq_om:2;
        };
        u16 reg1b;
    };

    union
    {
        struct
        {
            u16 reg_ceva_gpin_low:16;
        };
        u16 reg1c;
    };

    union
    {
        struct
        {
            u16 reg_ceva_gpin_high:16;
        };
        u16 reg1d;
    };

    union
    {
        struct
        {
            u16 reg_ceva_gpout_low:16;
        };
        u16 reg1e;
    };

    union
    {
        struct
        {
            u16 reg_ceva_gpout_high:16;
        };
        u16 reg1f;
    };

    union
    {
        struct
        {
            u16 reg_qman_desc_en:8;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 reg_xiu_be_err:1;
            u16 reg_ceva2mcu_bresp:1;
            u16 reg_ceva2mcu_rresp:1;
            u16 reg_ceva2mcu_wstrb:1;
            u16 reg_ceva2isp_bresp:1;
            u16 reg_ceva2isp_rresp:1;
            u16 reg_ceva2isp_wstrb:1;
            u16 reg_epp_awlen:1;
            u16 reg_epp_arlen:1;
            u16 reg_edp_awlen:1;
            u16 reg_edp_arlen:1;
            u16 reg_axir1_arlen:1;
            u16 reg_axir2_arlen:1;
            u16 reg_axiw1_awlen:1;
            u16 reg_apb_pslverr:1;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 reg_axi_maxlen:8;
            u16 reg_ceva_apbs_paddr:3;
            u16 reg_ceva_jt_ap:1;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 reg_forbidden_1:1;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_rstz_warp:3;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_warp2cmdq_trig_en:16;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_warp2cmdq_trig_en2:16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_dummy_0:16;
        };
        u16 reg60;
    };

    union
    {
        struct
        {
            u16 reg_dummy_1:16;
        };
        u16 reg61;
    };

    union
    {
        struct
        {
            u16 reg_dummy_2:16;
        };
        u16 reg62;
    };

    union
    {
        struct
        {
            u16 reg_dummy_3:16;
        };
        u16 reg63;
    };

}ceva_hal_reg_sys;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_uncache_no_pack_miu0_en:1;
            u16 reg_uncache_no_pack_imi_en:1;
            u16 reg_w_pack_always_no_hit_miu0:1;
            u16 reg_w_pack_always_no_hit_imi:1;
            u16 reg_w_always_flush_miu0:1;
            u16 reg_w_always_flush_imi:1;
            u16 reg_miu_access_mode:1;
            u16 reg_l3_dynamic_gat_en:1;
            u16 reg_l3_clk_latency:2;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_max_miu0:7;
            u16 reg_clk_miu2x_sel:1;
            u16 reg_mcu_req_max_imi:7;
            u16 reg_wriu_32b_en:1;
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_prior_miu0:6;
            u16 reg_clk_miu2x_en_switch:1;
            u16 reg_clk_miu1x_en_switch:1;
            u16 reg_mcu_req_prior_imi:6;
            u16 reg_clk_miu2x_switch_bypass_waiting:1;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 reg_w_pack_timeout:16;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 reg_dummy_44:16;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 reg_dummy_45:16;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 reg_timeout_cnt:16;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 reg_status_clear:1;
            u16 reg_ro_resp_flag_ms0:2;
            u16 reg_ro_resp_flag_ms1:2;
            u16 reg_req_wait_cyc:4;
            u16 reg_acp_req_mask:1;
        };
        u16 reg07;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_low:16;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_high:8;
            u16 reg_debug_port_sel:3;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 reg_testout_sel:2;
            u16 reg_ca9miu_strb_inv:1;
        };
        u16 reg0b;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_low:16;
        };
        u16 reg0c;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_high:16;
        };
        u16 reg0d;
    };

    union
    {
        struct
        {
            u16 reg_acp_pack_timeout:4;
            u16 reg_acp_idle:1;
        };
        u16 reg0e;
    };

    union
    {
        struct
        {
            u16 reg_testbus_en:1;
            u16 reg_ckg_alldft:1;
            u16 reg_ro_miucmd_come_before_set_miu2x:1;
            u16 reg_ro_clk_miu2x_state:2;
        };
        u16 reg0f;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_0:16;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_1:16;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_2:16;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_3:16;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_4:16;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_5:16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_6:16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_7:16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 reg_dummy50_ro:16;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 reg_rom_security_oneway_prot:1;
            u16 reg_rom_memory_en:1;
            u16 reg_rom_lightsleep:1;
            u16 reg_rom_shutdown:1;
            u16 reg_hemcu_iso_ctrl:2;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w:1;
            u16 reg_map_nodefine_hit_r:1;
            u16 reg_rxiu_nodefine_hit:1;
            u16 reg_rxiu_timeout_int:1;
        };
        u16 reg1a;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_low:16;
        };
        u16 reg1b;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_high:16;
        };
        u16 reg1c;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_low:16;
        };
        u16 reg1d;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_high:16;
        };
        u16 reg1e;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_low:16;
        };
        u16 reg1f;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_high:16;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_low:16;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_high:16;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 reg_nodefine_hit_clear:1;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_enable:1;
            u16 reg_protect_area_offset_start:4;
            u16 reg_protect_area_offset_end:4;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_low:16;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_high:10;
        };
        u16 reg26;
    };

    union
    {
        struct
        {
            u16 reg_pcie_noe_flush_en:1;
            u16 reg_pcie_wflush_en:1;
        };
        u16 reg37;
    };

    union
    {
        struct
        {
            u16 reg_c_riu_s2:4;
            u16 reg_c_riu_s0:4;
            u16 reg_c_delayriu_w_pulse:1;
            u16 reg_c_resetbridge:1;
            u16 reg_c_resetmcu:1;
        };
        u16 reg38;
    };

    union
    {
        struct
        {
            u16 reg_c_riurwtimeout:16;
        };
        u16 re79;
    };

    union
    {
        struct
        {
            u16 reg_c_maskint:1;
            u16 reg_c_forceint:1;
        };
        u16 reg3a;
    };

    union
    {
        struct
        {
            u16 reg_nonbuf_always_flush:1;
        };
        u16 reg3b;
    };

    union
    {
        struct
        {
            u16 reg_keep_write_pcie_miu_order:1;
        };
        u16 reg3c;
    };

    union
    {
        struct
        {
            u16 reg_rom_tc:4;
            u16 reg_rom_pd:1;
            u16 reg_rom_cs:1;
        };
        u16 reg3d;
    };

    union
    {
        struct
        {
            u16 reg_64b_wful_on:1;
            u16 reg_fpkfifo_no_merge:1;
        };
        u16 reg3e;
    };

    union
    {
        struct
        {
            u16 reg_clear_crossdie:1;
        };
        u16 reg3f;
    };
#if 1
}ceva_hal_reg_bus;
#else
// Maybe we can use a single structure for all axi2miu buses, maybe not...
}ceva_hal_reg_bus_axi2miu0;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_uncache_no_pack_miu0_en:1;
            u16 reg_uncache_no_pack_imi_en:1;
            u16 reg_w_pack_always_no_hit_miu0:1;
            u16 reg_w_pack_always_no_hit_imi:1;
            u16 reg_w_always_flush_miu0:1;
            u16 reg_w_always_flush_imi:1;
            u16 reg_miu_access_mode:1;
            u16 reg_l3_dynamic_gat_en:1;
            u16 reg_l3_clk_latency:2;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_max_miu0:7;
            u16 reg_clk_miu2x_sel:1;
            u16 reg_mcu_req_max_imi:7;
            u16 reg_wriu_32b_en:1;
        };
        u16 reg41;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_prior_miu0:6;
            u16 reg_clk_miu2x_en_switch:1;
            u16 reg_clk_miu1x_en_switch:1;
            u16 reg_mcu_req_prior_imi:6;
            u16 reg_clk_miu2x_switch_bypass_waiting:1;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_w_pack_timeout:16;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_dummy_44:16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_dummy_45:16;
        };
        u16 reg45;
    };

    union
    {
        struct
        {
            u16 reg_timeout_cnt:16;
        };
        u16 reg46;
    };

    union
    {
        struct
        {
            u16 reg_status_clear:1;
            u16 reg_ro_resp_flag_ms0:2;
            u16 reg_ro_resp_flag_ms1:2;
            u16 reg_req_wait_cyc:4;
            u16 reg_acp_req_mask:1;
        };
        u16 reg47;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_low:16;
        };
        u16 reg48;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_high:8;
            u16 reg_debug_port_sel:3;
        };
        u16 reg49;
    };

    union
    {
        struct
        {
            u16 reg_testout_sel:2;
            u16 reg_ca9miu_strb_inv:1;
        };
        u16 reg4b;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_low:16;
        };
        u16 reg4c;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_high:16;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 reg_acp_pack_timeout:4;
            u16 reg_acp_idle:1;
        };
        u16 reg4e;
    };

    union
    {
        struct
        {
            u16 reg_testbus_en:1;
            u16 reg_ckg_alldft:1;
            u16 reg_ro_miucmd_come_before_set_miu2x:1;
            u16 reg_ro_clk_miu2x_state:2;
        };
        u16 reg4f;
    };

     union
    {
        struct
        {
            u16 reg_force_axi_rd_0:16;
        };
        u16 reg50;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_1:16;
        };
        u16 reg51;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_2:16;
        };
        u16 reg52;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_3:16;
        };
        u16 reg53;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_4:16;
        };
        u16 reg54;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_5:16;
        };
        u16 reg55;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_6:16;
        };
        u16 reg56;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_7:16;
        };
        u16 reg57;
    };

    union
    {
        struct
        {
            u16 reg_dummy50_ro:16;
        };
        u16 reg58;
    };

    union
    {
        struct
        {
            u16 reg_rom_security_oneway_prot:1;
            u16 reg_rom_memory_en:1;
            u16 reg_rom_lightsleep:1;
            u16 reg_rom_shutdown:1;
            u16 reg_hemcu_iso_ctrl:2;
        };
        u16 reg59;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w:1;
            u16 reg_map_nodefine_hit_r:1;
            u16 reg_rxiu_nodefine_hit:1;
            u16 reg_rxiu_timeout_int:1;
        };
        u16 reg5a;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_low:16;
        };
        u16 reg5b;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_high:16;
        };
        u16 reg5c;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_low:16;
        };
        u16 reg5d;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_high:16;
        };
        u16 reg5e;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_low:16;
        };
        u16 reg5f;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_high:16;
        };
        u16 reg60;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_low:16;
        };
        u16 reg61;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_high:16;
        };
        u16 reg62;
    };

    union
    {
        struct
        {
            u16 reg_nodefine_hit_clear:1;
        };
        u16 reg63;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_enable:1;
            u16 reg_protect_area_offset_start:4;
            u16 reg_protect_area_offset_end:4;
        };
        u16 reg64;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_low:16;
        };
        u16 reg65;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_high:10;
        };
        u16 reg66;
    };

    union
    {
        struct
        {
            u16 reg_pcie_noe_flush_en:1;
            u16 reg_pcie_wflush_en:1;
        };
        u16 reg77;
    };

    union
    {
        struct
        {
            u16 reg_c_riu_s2:4;
            u16 reg_c_riu_s0:4;
            u16 reg_c_delayriu_w_pulse:1;
            u16 reg_c_resetbridge:1;
            u16 reg_c_resetmcu:1;
        };
        u16 reg78;
    };

    union
    {
        struct
        {
            u16 reg_c_riurwtimeout:16;
        };
        u16 re79;
    };

    union
    {
        struct
        {
            u16 reg_c_maskint:1;
            u16 reg_c_forceint:1;
        };
        u16 reg7a;
    };

    union
    {
        struct
        {
            u16 reg_nonbuf_always_flush:1;
        };
        u16 reg7b;
    };

    union
    {
        struct
        {
            u16 reg_keep_write_pcie_miu_order:1;
        };
        u16 reg7c;
    };

    union
    {
        struct
        {
            u16 reg_rom_tc:4;
            u16 reg_rom_pd:1;
            u16 reg_rom_cs:1;
        };
        u16 reg7d;
    };

    union
    {
        struct
        {
            u16 reg_64b_wful_on:1;
            u16 reg_fpkfifo_no_merge:1;
        };
        u16 reg7e;
    };

    union
    {
        struct
        {
            u16 reg_clear_crossdie:1;
        };
        u16 reg7f;
    };
}ceva_hal_reg_bus_axi2miu1;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_uncache_no_pack_miu0_en:1;
            u16 reg_uncache_no_pack_imi_en:1;
            u16 reg_w_pack_always_no_hit_miu0:1;
            u16 reg_w_pack_always_no_hit_imi:1;
            u16 reg_w_always_flush_miu0:1;
            u16 reg_w_always_flush_imi:1;
            u16 reg_miu_access_mode:1;
            u16 reg_l3_dynamic_gat_en:1;
            u16 reg_l3_clk_latency:2;
        };
        u16 reg00;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_max_miu0:7;
            u16 reg_clk_miu2x_sel:1;
            u16 reg_mcu_req_max_imi:7;
            u16 reg_wriu_32b_en:1;
        };
        u16 reg01;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_prior_miu0:6;
            u16 reg_clk_miu2x_en_switch:1;
            u16 reg_clk_miu1x_en_switch:1;
            u16 reg_mcu_req_prior_imi:6;
            u16 reg_clk_miu2x_switch_bypass_waiting:1;
        };
        u16 reg02;
    };

    union
    {
        struct
        {
            u16 reg_w_pack_timeout:16;
        };
        u16 reg03;
    };

    union
    {
        struct
        {
            u16 reg_dummy_44:16;
        };
        u16 reg04;
    };

    union
    {
        struct
        {
            u16 reg_dummy_45:16;
        };
        u16 reg05;
    };

    union
    {
        struct
        {
            u16 reg_timeout_cnt:16;
        };
        u16 reg06;
    };

    union
    {
        struct
        {
            u16 reg_status_clear:1;
            u16 reg_ro_resp_flag_ms0:2;
            u16 reg_ro_resp_flag_ms1:2;
            u16 reg_req_wait_cyc:4;
            u16 reg_acp_req_mask:1;
        };
        u16 reg07;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_low:16;
        };
        u16 reg08;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_high:8;
            u16 reg_debug_port_sel:3;
        };
        u16 reg09;
    };

    union
    {
        struct
        {
            u16 reg_testout_sel:2;
            u16 reg_ca9miu_strb_inv:1;
        };
        u16 reg0b;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_low:16;
        };
        u16 reg0c;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_high:16;
        };
        u16 reg0d;
    };

    union
    {
        struct
        {
            u16 reg_acp_pack_timeout:4;
            u16 reg_acp_idle:1;
        };
        u16 reg0e;
    };

    union
    {
        struct
        {
            u16 reg_testbus_en:1;
            u16 reg_ckg_alldft:1;
            u16 reg_ro_miucmd_come_before_set_miu2x:1;
            u16 reg_ro_clk_miu2x_state:2;
        };
        u16 reg0f;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_0:16;
        };
        u16 reg10;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_1:16;
        };
        u16 reg11;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_2:16;
        };
        u16 reg12;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_3:16;
        };
        u16 reg13;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_4:16;
        };
        u16 reg14;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_5:16;
        };
        u16 reg15;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_6:16;
        };
        u16 reg16;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_7:16;
        };
        u16 reg17;
    };

    union
    {
        struct
        {
            u16 reg_dummy50_ro:16;
        };
        u16 reg18;
    };

    union
    {
        struct
        {
            u16 reg_rom_security_oneway_prot:1;
            u16 reg_rom_memory_en:1;
            u16 reg_rom_lightsleep:1;
            u16 reg_rom_shutdown:1;
            u16 reg_hemcu_iso_ctrl:2;
        };
        u16 reg19;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w:1;
            u16 reg_map_nodefine_hit_r:1;
            u16 reg_rxiu_nodefine_hit:1;
            u16 reg_rxiu_timeout_int:1;
        };
        u16 reg1a;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_low:16;
        };
        u16 reg1b;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_high:16;
        };
        u16 reg1c;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_low:16;
        };
        u16 reg1d;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_high:16;
        };
        u16 reg1e;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_low:16;
        };
        u16 reg1f;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_high:16;
        };
        u16 reg20;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_low:16;
        };
        u16 reg21;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_high:16;
        };
        u16 reg22;
    };

    union
    {
        struct
        {
            u16 reg_nodefine_hit_clear:1;
        };
        u16 reg23;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_enable:1;
            u16 reg_protect_area_offset_start:4;
            u16 reg_protect_area_offset_end:4;
        };
        u16 reg24;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_low:16;
        };
        u16 reg25;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_high:10;
        };
        u16 reg26;
    };

    union
    {
        struct
        {
            u16 reg_pcie_noe_flush_en:1;
            u16 reg_pcie_wflush_en:1;
        };
        u16 reg37;
    };

    union
    {
        struct
        {
            u16 reg_c_riu_s2:4;
            u16 reg_c_riu_s0:4;
            u16 reg_c_delayriu_w_pulse:1;
            u16 reg_c_resetbridge:1;
            u16 reg_c_resetmcu:1;
        };
        u16 reg38;
    };

    union
    {
        struct
        {
            u16 reg_c_riurwtimeout:16;
        };
        u16 re79;
    };

    union
    {
        struct
        {
            u16 reg_c_maskint:1;
            u16 reg_c_forceint:1;
        };
        u16 reg3a;
    };

    union
    {
        struct
        {
            u16 reg_nonbuf_always_flush:1;
        };
        u16 reg3b;
    };

    union
    {
        struct
        {
            u16 reg_keep_write_pcie_miu_order:1;
        };
        u16 reg3c;
    };

    union
    {
        struct
        {
            u16 reg_rom_tc:4;
            u16 reg_rom_pd:1;
            u16 reg_rom_cs:1;
        };
        u16 reg3d;
    };

    union
    {
        struct
        {
            u16 reg_64b_wful_on:1;
            u16 reg_fpkfifo_no_merge:1;
        };
        u16 reg3e;
    };

    union
    {
        struct
        {
            u16 reg_clear_crossdie:1;
        };
        u16 reg3f;
    };
}ceva_hal_reg_bus_axi2miu2;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_uncache_no_pack_miu0_en:1;
            u16 reg_uncache_no_pack_imi_en:1;
            u16 reg_w_pack_always_no_hit_miu0:1;
            u16 reg_w_pack_always_no_hit_imi:1;
            u16 reg_w_always_flush_miu0:1;
            u16 reg_w_always_flush_imi:1;
            u16 reg_miu_access_mode:1;
            u16 reg_l3_dynamic_gat_en:1;
            u16 reg_l3_clk_latency:2;
        };
        u16 reg40;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_max_miu0:7;
            u16 reg_clk_miu2x_sel:1;
            u16 reg_mcu_req_max_imi:7;
            u16 reg_wriu_32b_en:1;
        };
        u16 reg41;
    };

    union
    {
        struct
        {
            u16 reg_mcu_req_prior_miu0:6;
            u16 reg_clk_miu2x_en_switch:1;
            u16 reg_clk_miu1x_en_switch:1;
            u16 reg_mcu_req_prior_imi:6;
            u16 reg_clk_miu2x_switch_bypass_waiting:1;
        };
        u16 reg42;
    };

    union
    {
        struct
        {
            u16 reg_w_pack_timeout:16;
        };
        u16 reg43;
    };

    union
    {
        struct
        {
            u16 reg_dummy_44:16;
        };
        u16 reg44;
    };

    union
    {
        struct
        {
            u16 reg_dummy_45:16;
        };
        u16 reg45;
    };

    union
    {
        struct
        {
            u16 reg_timeout_cnt:16;
        };
        u16 reg46;
    };

    union
    {
        struct
        {
            u16 reg_status_clear:1;
            u16 reg_ro_resp_flag_ms0:2;
            u16 reg_ro_resp_flag_ms1:2;
            u16 reg_req_wait_cyc:4;
            u16 reg_acp_req_mask:1;
        };
        u16 reg47;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_low:16;
        };
        u16 reg48;
    };

    union
    {
        struct
        {
            u16 reg_debug_port_out_high:8;
            u16 reg_debug_port_sel:3;
        };
        u16 reg49;
    };

    union
    {
        struct
        {
            u16 reg_testout_sel:2;
            u16 reg_ca9miu_strb_inv:1;
        };
        u16 reg4b;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_low:16;
        };
        u16 reg4c;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_debug_high:16;
        };
        u16 reg4d;
    };

    union
    {
        struct
        {
            u16 reg_acp_pack_timeout:4;
            u16 reg_acp_idle:1;
        };
        u16 reg4e;
    };

    union
    {
        struct
        {
            u16 reg_testbus_en:1;
            u16 reg_ckg_alldft:1;
            u16 reg_ro_miucmd_come_before_set_miu2x:1;
            u16 reg_ro_clk_miu2x_state:2;
        };
        u16 reg4f;
    };

     union
    {
        struct
        {
            u16 reg_force_axi_rd_0:16;
        };
        u16 reg50;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_1:16;
        };
        u16 reg51;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_2:16;
        };
        u16 reg52;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_3:16;
        };
        u16 reg53;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_4:16;
        };
        u16 reg54;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_5:16;
        };
        u16 reg55;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_6:16;
        };
        u16 reg56;
    };

    union
    {
        struct
        {
            u16 reg_force_axi_rd_7:16;
        };
        u16 reg57;
    };

    union
    {
        struct
        {
            u16 reg_dummy50_ro:16;
        };
        u16 reg58;
    };

    union
    {
        struct
        {
            u16 reg_rom_security_oneway_prot:1;
            u16 reg_rom_memory_en:1;
            u16 reg_rom_lightsleep:1;
            u16 reg_rom_shutdown:1;
            u16 reg_hemcu_iso_ctrl:2;
        };
        u16 reg59;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w:1;
            u16 reg_map_nodefine_hit_r:1;
            u16 reg_rxiu_nodefine_hit:1;
            u16 reg_rxiu_timeout_int:1;
        };
        u16 reg5a;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_low:16;
        };
        u16 reg5b;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_w_addr_high:16;
        };
        u16 reg5c;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_low:16;
        };
        u16 reg5d;
    };

    union
    {
        struct
        {
            u16 reg_map_nodefine_hit_r_addr_high:16;
        };
        u16 reg5e;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_low:16;
        };
        u16 reg5f;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_nodefine_hit_addr_high:16;
        };
        u16 reg60;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_low:16;
        };
        u16 reg61;
    };

    union
    {
        struct
        {
            u16 reg_rxiu_timeout_adr_high:16;
        };
        u16 reg62;
    };

    union
    {
        struct
        {
            u16 reg_nodefine_hit_clear:1;
        };
        u16 reg63;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_enable:1;
            u16 reg_protect_area_offset_start:4;
            u16 reg_protect_area_offset_end:4;
        };
        u16 reg64;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_low:16;
        };
        u16 reg65;
    };

    union
    {
        struct
        {
            u16 reg_protect_area_base_high:10;
        };
        u16 reg66;
    };

    union
    {
        struct
        {
            u16 reg_pcie_noe_flush_en:1;
            u16 reg_pcie_wflush_en:1;
        };
        u16 reg77;
    };

    union
    {
        struct
        {
            u16 reg_c_riu_s2:4;
            u16 reg_c_riu_s0:4;
            u16 reg_c_delayriu_w_pulse:1;
            u16 reg_c_resetbridge:1;
            u16 reg_c_resetmcu:1;
        };
        u16 reg78;
    };

    union
    {
        struct
        {
            u16 reg_c_riurwtimeout:16;
        };
        u16 re79;
    };

    union
    {
        struct
        {
            u16 reg_c_maskint:1;
            u16 reg_c_forceint:1;
        };
        u16 reg7a;
    };

    union
    {
        struct
        {
            u16 reg_nonbuf_always_flush:1;
        };
        u16 reg7b;
    };

    union
    {
        struct
        {
            u16 reg_keep_write_pcie_miu_order:1;
        };
        u16 reg7c;
    };

    union
    {
        struct
        {
            u16 reg_rom_tc:4;
            u16 reg_rom_pd:1;
            u16 reg_rom_cs:1;
        };
        u16 reg7d;
    };

    union
    {
        struct
        {
            u16 reg_64b_wful_on:1;
            u16 reg_fpkfifo_no_merge:1;
        };
        u16 reg7e;
    };

    union
    {
        struct
        {
            u16 reg_clear_crossdie:1;
        };
        u16 reg7f;
    };
}ceva_hal_reg_bus_axi2miu3;
#endif
#endif // __HAL_CEVA_REG_H__
