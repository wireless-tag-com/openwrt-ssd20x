/*
* prom.h- Sigmastar
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

#ifndef _MIPS_PROM_H
#define _MIPS_PROM_H

typedef enum
{
    LINUX_MEM,
    EMAC_MEM,
    MPOOL_MEM,
    LINUX_MEM2,
    LINUX_MEM3,
    G3D_MEM0,
    G3D_MEM1,
    G3D_CMDQ,
    DRAM,
    BB,
    GMAC_MEM
}BOOT_MEM_INFO;

char *prom_getcmdline(void);
char *prom_getenv(char *name);
void prom_init_cmdline(void);
void prom_meminit(void);
void prom_fixup_mem_map(unsigned long start_mem, unsigned long end_mem);
void mips_display_message(const char *str);
void mips_display_word(unsigned int num);
void mips_scroll_message(void);
int  get_ethernet_addr(char *ethernet_addr);
void get_boot_mem_info(BOOT_MEM_INFO type, phys_addr_t *addr, phys_addr_t *len);

/* Memory descriptor management. */
#define PROM_MAX_PMEMBLOCKS    32
struct prom_pmemblock {
        unsigned long base; /* Within KSEG0. */
        unsigned int size;  /* In bytes. */
        unsigned int type;  /* free or prom memory */
};

#endif /* !(_MIPS_PROM_H) */
