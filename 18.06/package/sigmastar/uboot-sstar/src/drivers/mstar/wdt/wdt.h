/*
* wdt.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: Ken.Chang <ken-ms.chang@sigmastar.com.tw>
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


#ifndef _WDT_H_
#define _WDT_H_

void wdt_reset(void);
void wdt_stop(void);
void wdt_start(U32 u32LaunchTim);
void wdt_set_timeout(U32 u32Timeout);

#endif /* WDT_H_ */
