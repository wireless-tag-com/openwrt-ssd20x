/*
* mstar_chip.h- Sigmastar
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
#ifdef CONFIG_ARM_LPAE

#define MSTAR_MIU0_BUS_BASE                      0x20000000UL
#define MSTAR_MIU1_BUS_BASE                      0x200000000ULL

#define ARM_MIU0_BUS_BASE                        MSTAR_MIU0_BUS_BASE
#define ARM_MIU1_BUS_BASE                        MSTAR_MIU1_BUS_BASE
#define ARM_MIU2_BUS_BASE                        0xFFFFFFFFFFFFFFFFULL
#define ARM_MIU3_BUS_BASE                        0xFFFFFFFFFFFFFFFFULL

#define ARM_MIU0_BASE_ADDR                       0x00000000UL
#define ARM_MIU1_BASE_ADDR                       0x80000000UL
#define ARM_MIU2_BASE_ADDR                       0xFFFFFFFFFFFFFFFFULL
#define ARM_MIU3_BASE_ADDR                       0xFFFFFFFFFFFFFFFFULL

#else

#define MSTAR_MIU0_BUS_BASE                      0x20000000UL
#define MSTAR_MIU1_BUS_BASE                      0xA0000000UL

#define ARM_MIU0_BUS_BASE                        MSTAR_MIU0_BUS_BASE
#define ARM_MIU1_BUS_BASE                        MSTAR_MIU1_BUS_BASE
#define ARM_MIU2_BUS_BASE                        0xFFFFFFFFUL
#define ARM_MIU3_BUS_BASE                        0xFFFFFFFFUL

#define ARM_MIU0_BASE_ADDR                       0x00000000UL
#define ARM_MIU1_BASE_ADDR                       0x80000000UL
#define ARM_MIU2_BASE_ADDR                       0xFFFFFFFFUL
#define ARM_MIU3_BASE_ADDR                       0xFFFFFFFFUL

#endif // CONFIG_ARM_LPAE

extern unsigned int query_frequency(void);
