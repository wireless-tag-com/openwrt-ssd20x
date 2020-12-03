/*
* cam_drv_poll_test.c- Sigmastar
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cam_os_wrapper.h>
#include <cam_dev_wrapper.h>
#include <mdrv_pollsample_io.h>
#ifdef CAM_OS_RTK
#include "sys_sys_core.h"
#include "sys_sys_isw_cli.h"
#endif
#define POLL_LOOP_CNT   10

static void _CamDrvPollShowTestMenu(void)
{
    CamOsPrintf("cam_drv_poll test menu: \n");
    CamOsPrintf("\t0) Poll sample test\r\n");
}

static void _TestCamDrvPollThread(void *arg)
{
    int handle[1];
    struct pollfd fds[1];
    int nRetFd = -1;
    int nTime = 2000;  /* msec */
    int cnt = 0, timeoutCnt = 0;

    memset(fds, 0, sizeof(fds));
    handle[0] = CamDevOpen("/dev/pollsample");
    if (handle[0] == CAM_OS_FAIL)
    {
          CamOsPrintf("CamDevOpen /dev/pollsample failed\n\r");
          return;
    }
    CamOsPrintf("CamDevOpen /dev/pollsample fd=%d\n\r", handle[0]);


    if (0 != CamDevIoctl(handle[0], IOCTL_POLLSAMPLE_START_TIMER, (void*)&nTime))
    {
        CamOsPrintf("IOCTL_POLLSAMPLE_START_TIMER failed\n\r");
        CamDevClose(handle[0]);
        return;
    }

    do {
         fds[0].fd = handle[0];
         fds[0].events = CAM_DEV_POLLOUT; //CAM_DEV_POLLPRI;
         if ((nRetFd = CamDevPoll(&fds[0], 1, 5000)) < 0)
         {
             CamOsPrintf("CamDevPoll failed\n\r");
         }

         CamOsPrintf("CamDevPoll(POLLPRI) done nRetFd=%d, events=0x%x\n\r", nRetFd, fds[0].revents);
         if (nRetFd != 0)
         {
             ++cnt;
         }
         else
         {
             ++timeoutCnt;
         }
    } while ((cnt < 2) && (timeoutCnt < 2));

    do {
        fds[0].fd = handle[0];
        fds[0].events = CAM_DEV_POLLIN | CAM_DEV_POLLPRI;
        if ((nRetFd = CamDevPoll(&fds[0], 1, 10000)) < 0)
        {
            CamOsPrintf("CamDevPoll failed\n\r");
        }

        CamOsPrintf("CamDevPoll done nRetFd=%d, events=0x%x\n\r", nRetFd, fds[0].revents);
        if (fds[0].revents & CAM_DEV_POLLIN)
        {
            CamOsPrintf("CamDevPoll got CAM_DEV_POLLIN\n\r");
        }
        if (fds[0].revents & CAM_DEV_POLLPRI)
        {
          CamOsPrintf("CamDevPoll got CAM_DEV_POLLPRI\n\r");
        }
     } while ((++cnt < POLL_LOOP_CNT) && nRetFd > 0);

     CamDevClose(handle[0]);
}

void TestCamDrvPoll_byCamDev(void)
{
    CamOsThread thread;
    CamOsRet_e eRet = CAM_OS_OK;

    eRet = CamOsThreadCreate(&thread, NULL, (void *)_TestCamDrvPollThread, NULL);
    if (CAM_OS_OK != eRet)
    {
        CamOsPrintf("%s : Create poll thread fail(err=%d)\n\r", __FUNCTION__, (int)eRet);
    }
    CamOsThreadJoin(thread);
}

#ifdef CAM_OS_RTK
extern int CamDevPollSampleInit(void);
extern void CamDevPollSampleRelease(void);

void TestCamDrvPoll_Rtk(void)
{
    CamDevPollSampleInit();
    TestCamDrvPoll_byCamDev();
    CamDevPollSampleRelease();
}

int CamDrvPollTest(CLI_t *pCli, char *p)
{
    int i, ParamCnt, ret = 0;
    u32  case_num = 0;
    char *pEnd;

    ParamCnt = CliTokenCount(pCli);

    if(ParamCnt < 1)
    {
        _CamDrvPollShowTestMenu();
        return eCLI_PARSE_INPUT_ERROR;
    }

    for(i = 0; i < ParamCnt; i++)
    {
        pCli->tokenLvl++;
        p = CliTokenPop(pCli);
        if(i == 0)
        {
            //CLIDEBUG(("p: %s, len: %d\n", p, strlen(p)));
            //*pV = _strtoul(p, &pEnd, base);
            case_num = strtoul(p, &pEnd, 10);
            //CLIDEBUG(("*pEnd = %d\n", *pEnd));
            if(p == pEnd || *pEnd)
            {
                cliPrintf("Invalid input\n");
                return eCLI_PARSE_ERROR;
            }
        }
        /* else{
             value[i-1] = p;
        }*/

    }

    switch(case_num)
    {
        case 0:
            TestCamDrvPoll_Rtk();
            break;
        default:
            _CamDrvPollShowTestMenu();
            ret = -1;
            break;
    }

    if(ret < 0)
        return eCLI_PARSE_ERROR;

    return eCLI_PARSE_OK;
}
#else // Linux
int main(int argc, char *argv[])
{
    int ret=0;

    if (argc != 2)
    {
        _CamDrvPollShowTestMenu();
        return -1;
    }

    switch (atoi(argv[1]))
    {
        case 0:
            TestCamDrvPoll_byCamDev();
            break;
        default:
            ret = -1;
            break;
    }

    if (ret < 0)
    {
        _CamDrvPollShowTestMenu();
        return -1;
    }

    return 0;
}
#endif
