/*
* cam_drv_poll.h- Sigmastar
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
#ifndef CAM_DRV_POLL_H_
#define CAM_DRV_POLL_H_

#include <cam_os_wrapper.h>
#include <cam_dev_wrapper.h>

s32  CamDrvPollRegEventGrp(void);
void CamDrvPollDeRegEventGrp(u32 nEventID);
void CamDrvPollSetEvent(u32 nEventID, u32 nEventBits);
s32  CamDrvPollEvent(u32 nEventID, u32 nWaitBits, struct file *filp, poll_table *tPoll);


#endif /* CAM_DRV_POLL_H_ */
