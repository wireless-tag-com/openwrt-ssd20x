/*
* sata_test.c- Sigmastar
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include <pthread.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
//#include <linux/delay.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <linux/netlink.h>
#include <linux/fs.h>
#include <sys/mman.h>

//#ifdef MSOS_TYPE_LINUX_KERNEL
//#include <asm/div64.h>
//#else
//    #define do_div(x,y) ((x)/=(y))
//#endif

//#include "apiXC.h"
#include "sata_test.h"
#include "mdrv_sata_io.h"
#include "mdrv_sata_io_st.h"

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------
typedef enum
{
    E_DRV_ID_SATA = 0,
    E_DRV_ID_MSYS,
    E_DRV_ID_MEM,
    E_DRV_ID_NUM,
} EN_DRV_ID_TYPE;


static int g_FD[E_DRV_ID_NUM] = {-1, -1, -1};

typedef struct __Param
{
    MS_U16 u16PortNo;
    MS_U16 u16GenNo;
    MS_U16 u16LoopbackEnable;
    MS_U16 u16TxTestMode;
    MS_U16 u16TxTestSSCEnable;
} Param;

ST_SATA_Test_Property stSATATestProperty =
{
    0,       // u16PortNo
    3,       // u16GenNo
    0,
    0,
    0
};

unsigned long long _GetSystemTimeStamp (void)
{
    struct timeval         tv;
    struct timezone tz;
    unsigned long long u64TimeStamp;
    gettimeofday(&tv, &tz);
    u64TimeStamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
    return u64TimeStamp;
}

int Open_Device(EN_DRV_ID_TYPE enDrvID)
{
    int ret = 1;
    char device_name[E_DRV_ID_NUM][50] =
    {
        {"/dev/msata"},
        {"/dev/msys"},
        {"/dev/mem"},
    };

    //FUNC_MSG("Open_Device: id=%d\n", enDrvID);
    if(enDrvID >= E_DRV_ID_NUM)
    {
        printf("ID is not correct\n");
        return 0;
    }

    //FUNC_MSG("Open_Device handle: %d\n", g_FD[enDrvID]);
    if(g_FD[enDrvID] != -1)
    {
    }
    else
    {
        //FUNC_MSG("Open_Device: %s\n", &device_name[enDrvID][0]);
        //printf("Open: %s\n", &device_name[enDrvID][0]);
        g_FD[enDrvID] = open(&device_name[enDrvID][0], O_RDWR | O_SYNC, S_IRUSR | S_IWUSR);

        if(g_FD[enDrvID] == -1)
        {
            printf("open %s fail\n", &device_name[enDrvID][0]);
            ret = 0;
        }
        else
        {
            //printf("open %s  %d sucess\n", &device_name[enDrvID][0], g_FD[enDrvID]);
            ret = 1;
        }
    }
    return ret;
}


#if 1
MS_BOOL _SysInit(void)
{
    MS_BOOL ret = TRUE;

    //FUNC_MSG("System Init Finish\n");

    return ret;
}

MS_BOOL _SysDeInit(void)
{
    MS_BOOL ret = TRUE;

    return ret;
}
#endif

MS_BOOL Test_SATA_Set_LoopbackTest(MS_U16 u16PortNo, MS_U16 u16GenNo)
{
    stSata_Loopback_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_LOOPBACK_TEST, &stInCfg);

    printf("SATA loopback test result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_BOOL Test_SATA_Set_TxTest_HFTP(MS_U16 u16PortNo, MS_U16 u16GenNo, MS_U16 u16SSCEn)
{
    stSata_Tx_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.u32SSCEnable = u16SSCEn;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_TX_TEST_HFTP, &stInCfg);

    printf("SATA SET TX TEST HFTP result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_BOOL Test_SATA_Set_TxTest_MFTP(MS_U16 u16PortNo, MS_U16 u16GenNo, MS_U16 u16SSCEn)
{
    stSata_Tx_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.u32SSCEnable = u16SSCEn;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_TX_TEST_MFTP, &stInCfg);

    printf("SATA SET TX TEST MFTP result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_BOOL Test_SATA_Set_TxTest_LFTP(MS_U16 u16PortNo, MS_U16 u16GenNo, MS_U16 u16SSCEn)
{
    stSata_Tx_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.u32SSCEnable = u16SSCEn;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_TX_TEST_LFTP, &stInCfg);

    printf("SATA SET TX TEST LFTP result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_BOOL Test_SATA_Set_TxTest_LBP(MS_U16 u16PortNo, MS_U16 u16GenNo, MS_U16 u16SSCEn)
{
    stSata_Tx_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.u32SSCEnable = u16SSCEn;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_TX_TEST_LBP, &stInCfg);

    printf("SATA SET TX TEST LBP result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_BOOL Test_SATA_Set_TxTest_SSOP(MS_U16 u16PortNo, MS_U16 u16GenNo, MS_U16 u16SSCEn)
{
    stSata_Tx_Test stInCfg;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    if((u16PortNo >= 2)  || ((u16GenNo == 0)  || (u16GenNo >= 4) ))
    {
        return FALSE;
    }

    stInCfg.u16PortNo = u16PortNo;
    stInCfg.u16GenNo = u16GenNo;
    stInCfg.u32SSCEnable = u16SSCEn;
    stInCfg.s32Result = 0;

    ioctl(g_FD[E_DRV_ID_SATA], IOCTL_SATA_SET_TX_TEST_SSOP, &stInCfg);

    printf("SATA SET TX TEST SSOP result = %d\n", stInCfg.s32Result);

    return TRUE;
}

MS_S32 fnAlloc(MS_U8 *pnMMAHeapName, MS_U32 nSize, MS_U64 *pnAddr)
{
    //kmalloc((nSize), GFP_KERNEL);
    return TRUE;
}

MS_S32 fnFree(MS_U64 nAddr)
{
    //kfree((MS_U8 *)nAddr);
    return TRUE;
}

MS_BOOL _SATAInit(void)
{
    MS_BOOL ret = TRUE;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    return ret;
}

MS_BOOL _SATADeInit(void)
{
    MS_BOOL ret = TRUE;

    if(Open_Device(E_DRV_ID_SATA) == 0)
    {
        return FALSE;
    }

    return ret;
}

MS_BOOL _SATATest(void)
{
    MS_BOOL ret = TRUE;
    MS_U16 u16PortNo = stSATATestProperty.u16PortNo;
    MS_U16 u16GenNo = stSATATestProperty.u16GenNo;
    MS_U16 u16SSCEn = stSATATestProperty.u16TxTestSSCEnable;

    if(stSATATestProperty.u16LoopbackEnable ==  TRUE)
    {
        ret = Test_SATA_Set_LoopbackTest(u16PortNo, u16GenNo);
    }
    else if((stSATATestProperty.u16TxTestMode > 0) && (stSATATestProperty.u16TxTestMode <= 5))
    {
        switch (stSATATestProperty.u16TxTestMode)
        {
            case 1:
                ret = Test_SATA_Set_TxTest_HFTP(u16PortNo, u16GenNo, u16SSCEn);
                break;
            case 2:
                ret = Test_SATA_Set_TxTest_MFTP(u16PortNo, u16GenNo, u16SSCEn);
                break;
            case 3:
                ret = Test_SATA_Set_TxTest_LFTP(u16PortNo, u16GenNo, u16SSCEn);
                break;
            case 4:
                ret = Test_SATA_Set_TxTest_LBP(u16PortNo, u16GenNo, u16SSCEn);
                break;
            case 5:
                ret = Test_SATA_Set_TxTest_SSOP(u16PortNo, u16GenNo, u16SSCEn);
                break;
        }
    }

    return ret;
}

static void UsingGuide(int argc, char **argv)
{
    fprintf(stdout,	"Usage: %s [-loopback PortNo GenNo]\n"
            "like : '%s -loopback 0 3 '\n"
            "Arguments:\n", argc ? argv[0] : NULL, argc ? argv[0] : NULL);
    /* loopback test */
    fprintf(stdout,	"-loopback\t: loopback test,\n"
            "\t '-loopback <port no.> <gen no.>', <port no.>: 0 or 1, <gen no.>: 1 ~ 3\n");

    /* Tx test */
    fprintf(stdout,	"-txmode\t: Tx test,\n"
            "\t '-txmode <port no.> <gen no.> <tx mode> <SSC>', <port no.>: 0 or 1, <gen no.>: 1 ~ 3\n"
            "\t <tx mode>: 1 :  HFTP, 2: MFTP, 3: LFTP, 4: LBP, 5: SSOP\n"
            "\t <SSC>: 0 :  disable SSC, 1: enable SSC\n");

    exit(EXIT_FAILURE);
}

static void get_opt(Param *SetParam, int argc, char **argv)
{
    char	*endptr = NULL;
    int		i = 0;

    /* Default value */
    memset(SetParam, 0x0, sizeof(Param));
    stSATATestProperty.u16LoopbackEnable = FALSE;
    stSATATestProperty.u16TxTestMode = 0;

    if(argc > 1)
    {
        if ( argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) )
            UsingGuide(argc, argv);

        for (i = 1; i < argc; i++)
        {
            /* Loopback test */
            if (!strcmp(argv[i], "-loopback"))
            {
                SetParam->u16PortNo = strtol(argv[++i], &endptr, 10);
                SetParam->u16GenNo = strtol(argv[++i], &endptr, 10);
                if ((SetParam->u16PortNo > 1)  || ((SetParam->u16GenNo == 0) || (SetParam->u16GenNo >= 4)))
                {
                    fprintf(stderr, "Invalid loopback test parameter(port=%d, gen=%d).\n", SetParam->u16PortNo, SetParam->u16GenNo);
                    UsingGuide(argc, argv);
                }
                stSATATestProperty.u16PortNo = SetParam->u16PortNo;
                stSATATestProperty.u16GenNo = SetParam->u16GenNo;
                stSATATestProperty.u16LoopbackEnable = TRUE;
            }
            /*  */
            else if (!strcmp(argv[i], "-txmode"))
            {
                SetParam->u16PortNo = strtol(argv[++i], &endptr, 10);
                SetParam->u16GenNo = strtol(argv[++i], &endptr, 10);
                SetParam->u16TxTestMode = strtol(argv[++i], &endptr, 10);
                SetParam->u16TxTestSSCEnable = strtol(argv[++i], &endptr, 10);
                if ((SetParam->u16PortNo > 1)  || ((SetParam->u16GenNo == 0) || (SetParam->u16GenNo >= 4)) || (SetParam->u16TxTestMode >= 6))
                {
                    fprintf(stderr, "Invalid Tx test parameter(port=%d, gen=%d, Tx mode=%d).\n", SetParam->u16PortNo, SetParam->u16GenNo, SetParam->u16TxTestMode);
                    UsingGuide(argc, argv);
                }
                stSATATestProperty.u16PortNo = SetParam->u16PortNo;
                stSATATestProperty.u16GenNo = SetParam->u16GenNo;
                stSATATestProperty.u16TxTestMode = SetParam->u16TxTestMode;
                stSATATestProperty.u16TxTestSSCEnable = SetParam->u16TxTestSSCEnable;
            }
            else
            {
                fprintf(stderr, "Invalid input parameter(%s).\n", argv[i]);
                UsingGuide(argc, argv);
            }
        }
    }
    else
    {
        UsingGuide(argc, argv);
    }
}

int main(int argc, char *argv[])
{
    Param			SetParam;

    get_opt(&SetParam, argc, argv);

    if (!_SysInit())
        goto SATA_TEST_EXIT;

    if (!_SATAInit())
        goto SATA_TEST_EXIT;

    if (!_SATATest())
        goto SATA_TEST_EXIT;

SATA_TEST_EXIT:
    _SATADeInit();
    _SysDeInit();

    return 0;
}
