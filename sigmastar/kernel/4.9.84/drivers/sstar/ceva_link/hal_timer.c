/*
* hal_timer.c- Sigmastar
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
#include "hal_timer.h"
#include "hal_debug.h"

#define RIU_BASE_ADDR   (0x1F000000)
#define BANK_CAL(addr)  ((addr<<9) + (RIU_BASE_ADDR))

#define BANK_TIMER    (BANK_CAL(0x0030))

#if (HAL_MSG_LEVL < HAL_MSG_DBG)
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  ms_writew(val,((uint)base+(idx)*4))
#else
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  do{HAL_MSG(HAL_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)*4), val); ms_writew(val,((uint)base+(idx)*4));} while(0)
#endif

/*******************************************************************************************************************
 * dsp_timer_hal_init
 *   init device timer
 *
 * Parameters:
 *   RIU_BASE_ADDR:  RIU base address
 *
 * Return:
 *   0: OK, othes: failed
 */
#define BANK_IRQ    (BANK_CAL(0x1019))
 
 
int dsp_timer_hal_init(void)
{
    int err_state = 0;

    //REGW(BANK_IRQ, 0x2C, 0x0001); //LSB   period, (1ms, 12*1000)
#if 1
    #if 0
    //set timer 0
    REGW(BANK_TIMER, 0x12, 0x2EE0); //LSB   period, (1ms, 12*1000)
    REGW(BANK_TIMER, 0x13, 0x0000); //MSB

    REGW(BANK_TIMER, 0x10, 0x0101); //enable timer 0
    #endif
    
    #if 0
    //set timer 1
    REGW(BANK_TIMER, 0x22, 0x2EE0); //LSB   period, (1ms, 12*1000)
    REGW(BANK_TIMER, 0x23, 0x0000); //MSB

    REGW(BANK_TIMER, 0x20, 0x0101); //enable timer 0
    #endif 
    
    #if 1
    //set timer 2
    REGW(BANK_TIMER, 0x32, 0x2EE0); //LSB   period, (1ms, 12*1000)
    REGW(BANK_TIMER, 0x33, 0x0000); //MSB

    REGW(BANK_TIMER, 0x30, 0x0101); //enable timer 0
    #endif 

#endif
    return err_state;
}

void dump_bank_register(unsigned int Bank)
{
  unsigned int tmp=0;
  unsigned int xx=0;
  unsigned int int_bank;
  
  int_bank=BANK_CAL(Bank);
  
  printk("CEVA dump_bank_register: Bank:0x%04X\n",Bank);
  
  for(xx=0;xx<0x80;xx++)
  {
      tmp=REGR(int_bank,xx);
      if(xx%8 == 0)
      {
      	printk("%02X ",xx);
      }
      printk("%04X ",tmp&0xffff);
      if(xx%8 == 7)
      {
      	printk("\n");
      }
  }
  printk("\n");
  
}