/*
* drv_pollsample_module.c- Sigmastar
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
#include <cam_dev_register.h>
#include "mdrv_pollsample_module.h"

#define DRV_POLL_SAMPLE "/dev/pollsample"

int CamDevPollSampleOpen(struct inode *pInode);
int CamDevPollSampleClose(int nFd, struct inode *pInode);
int CamDevPollSampleIoctl(int nFd, unsigned long nRequest, void *pParam);
int CamDevPollSamplePoll(int nFd, short nEvents, short *pnRevent ,int nTimeout);

PollSampleDev_t tPollSampleDev;

static CamDevFuncMap_t tPollSampleFuncMap = {
    .szName = DRV_POLL_SAMPLE,
    .pOpen  = CamDevPollSampleOpen,
    .pClose = CamDevPollSampleClose,
    .pIoctl = CamDevPollSampleIoctl,
    .pPoll  = CamDevPollSamplePoll,
    .pInode = NULL,
};

int CamDevPollSampleOpen(struct inode *pInode)
{
    struct file *filp;

    filp = (struct file *)CamOsMemCalloc(1, sizeof(struct file));

    if (pollsamp_open(pInode, filp) == 0) /* success */
    {
        return (int)filp;
    }
    else
    {
        CamOsMemRelease(filp);
        return CAM_OS_FAIL;
    }
}

int CamDevPollSampleClose(int nFd, struct inode *pInode)
{
    struct file *filp = (struct file *)nFd;

    pollsamp_release(pInode, filp);
    CamOsMemRelease(filp);

    return CAM_OS_OK;
}

int CamDevPollSampleIoctl(int nFd, unsigned long nRequest, void *pParam)
{
    int nRet = CAM_OS_OK;
    struct file *filp = (struct file *)nFd;

    if (pollsamp_ioctl(filp, nRequest, (unsigned long)pParam) != 0)
    {
        nRet = CAM_OS_FAIL;
    }

    return nRet;
}

int CamDevPollSamplePoll(int nFd, short nEvents, short *pnRevent ,int nTimeout)
{
    struct file *filp = (struct file *)nFd;
    u32 nRevent;

    filp->nPollTimeout = nTimeout;
    nRevent = pollsamp_poll(filp, NULL);

    *pnRevent = nRevent & nEvents;
    CamOsPrintf("%s: revent=0x%x\n",__func__, *pnRevent);
    return (*pnRevent != 0) ? 1 : 0;
}

int CamDevPollSampleInit(void)
{
    tPollSampleDev.nTestNum = 12345;
    if ((tPollSampleFuncMap.pInode = CamOsMemAlloc(sizeof(struct inode))) == NULL)
    {
        CamOsPrintf("%s: memory allocate fail!\r\n", __FUNCTION__);
        return -1;
    }

    tPollSampleFuncMap.pInode->i_cdev = (void *)&tPollSampleDev.m_cdev;

    if (CAM_OS_OK == CamDevRegister(&tPollSampleFuncMap))
        return 0;
    else
        return -1;
}

void CamDevPollSampleRelease(void)
{
    CamDevUnregister(DRV_POLL_SAMPLE);

    if (tPollSampleFuncMap.pInode)
    {
        CamOsMemRelease(tPollSampleFuncMap.pInode);
        tPollSampleFuncMap.pInode = NULL;
    }
}
