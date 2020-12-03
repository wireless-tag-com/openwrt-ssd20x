/*
* vcore_dvfs.h- Sigmastar
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
//-----------------------------------------------------------------------------
//
// Copyright (c) 2019 MStar Semiconductor, Inc.  All rights reserved.
//
//-----------------------------------------------------------------------------
// FILE
//      vcore_dvfs.h
//
// DESCRIPTION
//
//
// HISTORY
//
//-----------------------------------------------------------------------------
#ifndef __VCORE_DVFS_H
#define __VCORE_DVFS_H



#define VCORE_DVFS_IOC_MAGIC        'v'
#define VCORE_DVFS_INIT             _IO(VCORE_DVFS_IOC_MAGIC, 0)
#define VCORE_DVFS_OP               _IO(VCORE_DVFS_IOC_MAGIC, 1)

#define VCORE_DVFS_IOC_MAXNR        1



typedef enum
{
    VCORE_DVFS_DEMANDER_CEVA_PLL,
    VCORE_DVFS_DEMANDER_TOTAL,

} vcore_dvfs_demander_e;



struct vcore_dvfs_init_data {
    unsigned long   rate;
    int             active;
};



typedef int (*set_rate_hw)(unsigned long rate);
typedef int (*clk_enable_hw)(void);
typedef int (*clk_disable_hw)(void);



extern int vcore_dvfs_register(vcore_dvfs_demander_e demander, struct vcore_dvfs_init_data *init_data);
extern int vcore_dvfs_unregister(vcore_dvfs_demander_e demander);
extern int vcore_dvfs_clk_enable(vcore_dvfs_demander_e demander, clk_enable_hw clk_enable_hw);
extern int vcore_dvfs_clk_disable(vcore_dvfs_demander_e demander, clk_disable_hw clk_disable_hw);
extern int vcore_dvfs_clk_set_rate(vcore_dvfs_demander_e demander, unsigned long rate, set_rate_hw set_rate_hw);

#endif
