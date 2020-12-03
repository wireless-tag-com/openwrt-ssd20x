/*
* prof.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/types.h>
#include "ms_platform.h"
#include "ms_types.h"
#include <linux/io.h>

#if 1
#define RECORD_IPL_ADDR 0xF900F000 //IPL
#define RECORD_BL_ADDR  0xF900F400 //BL
#define RECORD_ADDR     0xF900F800 //LINUX 0XA0


#define MAX_RECORD 800  /*max:(0x18000-0xF800)/40~=870*/
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
    u64 usticks=div64_u64(cycles, 6/*us_ticks_factor*/);
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
        strncpy(tc->tt[tc->count].name, name, MAX_LANGTH-1);
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
        strncpy(tc->tt[tc->count].name, name, MAX_LANGTH-1);
        tc->count++;
    } 
}

void recode_show(void)
{
    struct timecrecord *tc;
    int i=0;

    tc = (struct timecrecord *) (RECORD_IPL_ADDR); // IMI SRAM
    if( tc->count <= MAX_RECORD && tc->count>0)
    {
        printk(KERN_CRIT"IPL: 0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk(KERN_CRIT"%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk(KERN_CRIT"Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

    tc = (struct timecrecord *) (RECORD_BL_ADDR); // IMI SRAM
    if( tc->count <= MAX_RECORD && tc->count>0)
    {
        printk(KERN_CRIT"BL: 0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk(KERN_CRIT"%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk(KERN_CRIT"Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

    tc = (struct timecrecord *) (RECORD_ADDR); // IMI SRAM
    if( tc->count <= MAX_RECORD && tc->count>0)
    {
        printk(KERN_CRIT"Linux:0x%p\n", tc);
        for( i=0; i<tc->count; i++)
        {
            tc->tt[i].name[MAX_LANGTH-1]='\0';

            printk(KERN_CRIT"%03d time:%8u, diff:%8u, %s, %d\n", 
                i, 
                tc->tt[i].timestamp_us, 
                tc->tt[i].timestamp_us-tc->tt[i?i-1:i].timestamp_us,
                tc->tt[i].name,
                tc->tt[i].mark);

        }
        printk(KERN_CRIT"Total cost:%8u(us)\n",  tc->tt[tc->count-1].timestamp_us - tc->tt[0].timestamp_us );
    }

}
#else
void recode_timestamp(int mark, const char* name){}
void recode_show(void){}
unsigned int read_timestamp(void){return 0;}
void recode_timestamp_ext(int mark, const char* name, unsigned int timestamp){}
#endif
