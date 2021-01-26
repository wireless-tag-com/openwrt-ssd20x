/*
* prof.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: Karl.Xiao <Karl.Xiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "asm/arch/mach/ms_types.h"
//#include "asm/arch/mach/platform.h"
//#include "asm/arch/mach/io.h"

#include <common.h>
#include <command.h>
#include <div64.h>
#include <linux/string.h>
#include <linux/compat.h>






#if 1
#define RECORD_IPL_ADDR 0xA000F000 //IPL
#define RECORD_BL_ADDR  0xA000F400 //BL
#define RECORD_LINUX_ADDR  0xA000F800 //LINUX
#define RECORD_ADDR     RECORD_BL_ADDR


#define MAX_RECORD 50
#define MAX_LANGTH 32
struct timestamp {
    unsigned int timestamp_us;      /* 4                 */
    unsigned int mark;              /* 4, ex:__line__    */
    unsigned char name[MAX_LANGTH]; /*32, ex:__function__*/
};
struct timecrecord {
    U32 count;
    struct timestamp tt[MAX_RECORD];
};

int g_record_inited = 0;
void* g_addr_record_ipl = 0;
void* g_addr_record_bl = 0;
void* g_addr_record_kernel= 0;

U64 arch_counter_get_cntpct(void)
{
    U64 cval;
    asm volatile("isb " : : : "memory");
    asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (cval));
    return cval;
}

unsigned int read_timestamp(void)
{
    u64 cycles=arch_counter_get_cntpct();
    u64 usticks=lldiv(cycles,6);
    return (unsigned int) usticks;
}
void recode_timestamp(int mark, const char* name)
{
    struct timecrecord *tc;

    tc = (struct timecrecord *) (RECORD_ADDR);
    if(!g_record_inited)
    {
        tc->count = 0;
        g_record_inited=1;
    }

    if(tc->count < MAX_RECORD)
    {
        tc->tt[tc->count].timestamp_us = read_timestamp();
        tc->tt[tc->count].mark = mark;
        strncpy((char *)tc->tt[tc->count].name, name, MAX_LANGTH-1);
        tc->count++;
    }
}

void recode_timestamp_ext(int mark, const char* name, unsigned int timestamp)
{
    struct timecrecord *tc;
    
    tc = (struct timecrecord *) (RECORD_ADDR);
    if(!g_record_inited)
    {
        tc->count = 0;
        g_record_inited=1;
    }

    if(tc->count < MAX_RECORD)
    {
        tc->tt[tc->count].timestamp_us = timestamp;
        tc->tt[tc->count].mark = mark;
        strncpy((char *)tc->tt[tc->count].name, name, MAX_LANGTH-1);
        tc->count++;
    } 
}

void recode_show(void)
{
    struct timecrecord *tc;
    int i=0;

    tc = (struct timecrecord *) (RECORD_IPL_ADDR); // IMI SRAM
    if( tc->count < MAX_RECORD && tc->count>0)
    {
        printk("IPL: 0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk("%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk("Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

    tc = (struct timecrecord *) (RECORD_BL_ADDR); // IMI SRAM
    if( tc->count < MAX_RECORD && tc->count>0)
    {
        printk("BL: 0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk("%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk("Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

    tc = (struct timecrecord *) (RECORD_LINUX_ADDR); // IMI SRAM
    if( tc->count < MAX_RECORD && tc->count>0)
    {
        printk("Linux:0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk("%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk("Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

}

int do_bootingtime(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    recode_show();
    return 0;
}
U_BOOT_CMD(
	btime, 2,	1,	do_bootingtime,
	"Show booting time",
	"Show booting time\n"
);
#else
void recode_timestamp(int mark, const char* name){}
void recode_show(void){}
unsigned int read_timestamp(void){return 0;}
void recode_timestamp_ext(int mark, const char* name, unsigned int timestamp){}
#endif
