/*
* cam_os_wrapper_test.h- Sigmastar
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


#ifndef __CAM_OS_WRAPPER_TEST_H__
#define __CAM_OS_WRAPPER_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CAM_OS_RTK
s32 CamOsWrapperTest(CLI_t *pCli, char *p);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_WRAPPER_TEST_H__
