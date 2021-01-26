/*
* sar.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: sylvia.nain. <sylvia.nain@sigmastar.com.tw>
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

#ifndef _SAR_H_
#define _SAR_H_


void sar_hw_init(void);
int  sar_get_value(int ch);

#endif /* SAR_H_ */
