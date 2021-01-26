/***************************************************************************************************************
 *
 * FileName cmd_sdmmc_ipv.c
 *     @author jeremy.wang (2015/10/07)
 * Desc:
 *	   This file is for uboot console mode
 * 	   We could do ip verification from this console support
 *
 *
 ***************************************************************************************************************/

#include "../drivers/mstar/sdmmc/inc/ms_sdmmc_verify.h"

#include <common.h>
#include <command.h>



void do_io_std_test(unsigned char u8Slot)
{
    unsigned int i;

    for (i=6;i<16;i++)
    {
        IPV_SDMMC_CIFD_RW(u8Slot, i, 0);
        IPV_SDMMC_DMA_RW(u8Slot, i, 2, 0);

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
        IPV_SDMMC_ADMA_RW(u8Slot, i, 0);
//###########################################################################################################
#endif

    }

}

void do_io_std_burntest(unsigned char u8Slot)
{

    IPV_SDMMC_CIFD_RW(u8Slot, 6, 0);
    IPV_SDMMC_DMA_RW(u8Slot, 6, 2, 0);

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
    IPV_SDMMC_ADMA_RW(u8Slot, 6, 0);
//###########################################################################################################
#endif

    IPV_SDMMC_BurnRW(u8Slot, 0, 30000, 30050); //CIFD
    IPV_SDMMC_BurnRW(u8Slot, 1, 30000, 30050); //DMA

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
    IPV_SDMMC_BurnRW(u8Slot, 2, 30000, 30050); //ADMA
//###########################################################################################################
#endif


}

void do_prepare_power_saving_mode_queue(unsigned char u8Slot)
{
    IPV_SDMMC_PowerSavingModeVerify(u8Slot);
}

int do_sdmmc_test ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char u8Val1, u8Val2, u8Val3, u8Val4;
    unsigned int  u32Val1;
	switch (argc){

    case 6:
        u8Val1 = (unsigned char)simple_strtoul(argv[2], NULL, 10);  //Slot
        u8Val2 = (unsigned char)simple_strtoul(argv[3], NULL, 10);  //IP
        u8Val3 = (unsigned char)simple_strtoul(argv[4], NULL, 10);  //PORT
        u8Val4 = (unsigned char)simple_strtoul(argv[5], NULL, 10);  //PAD

        if (strcmp(argv[1], "pad") == 0)
            IPV_SDMMC_SetPAD(u8Val1, u8Val2, u8Val3, u8Val4);
        else
            return cmd_usage(cmdtp);

        return 0;

    case 4:
        u8Val1 = (unsigned char)simple_strtoul(argv[2], NULL, 10); //Slot
        u32Val1 = (unsigned int)simple_strtoul(argv[3], NULL, 10);

        if (strcmp(argv[1], "clk") == 0)
			IPV_SDMMC_SetClock(u8Val1, u32Val1, 0);
        else if (strcmp(argv[1], "btm") == 0)
			IPV_SDMMC_SetBusTiming(u8Val1, (unsigned char)u32Val1);
		else if (strcmp(argv[1], "pat") == 0)
			IPV_SDMMC_TestPattern(u8Val1, (unsigned char)u32Val1);
        else
            return cmd_usage(cmdtp);

        return 0;

    case 3:
        u8Val1 = (unsigned char)simple_strtoul(argv[2], NULL, 10);

        if (strcmp(argv[1], "timer") == 0)
            IPV_SDMMC_TimerTest(u8Val1);
        else if (strcmp(argv[1], "cdz") == 0)
            IPV_SDMMC_CardDetect(u8Val1);
        else if (strcmp(argv[1], "init") == 0)
            IPV_SDMMC_Init(u8Val1);
        else if (strcmp(argv[1], "wibus") == 0)
            IPV_SDMMC_SetWideBus(u8Val1);
        else if (strcmp(argv[1], "hibus") == 0)
            IPV_SDMMC_SetHighBus(u8Val1);
        else if (strcmp(argv[1], "cifd") == 0)
            IPV_SDMMC_CIFD_RW(u8Val1, 6, 0);
        else if (strcmp(argv[1], "dma") == 0)
            IPV_SDMMC_DMA_RW(u8Val1, 6, 2, 0);
        else if (strcmp(argv[1], "bcifd") == 0)
            IPV_SDMMC_BurnRW(u8Val1, 0, 0, 0);
        else if (strcmp(argv[1], "bdma") == 0)
            IPV_SDMMC_BurnRW(u8Val1, 1, 0, 0);
//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
        else if (strcmp(argv[1], "adma") == 0)
            IPV_SDMMC_ADMA_RW(u8Val1, 6, 0);
        else if (strcmp(argv[1], "badma") == 0)
            IPV_SDMMC_BurnRW(u8Val1, 2, 0, 0);
//###########################################################################################################
#endif
        else if (strcmp(argv[1], "at1d") == 0)
        {
            IPV_SDMMC_TimerTest(3);
            IPV_SDMMC_CardDetect(u8Val1);
            IPV_SDMMC_Init(u8Val1);

            printf("\r\n*********************** [ Test 1bit Low Speed I/O ] ************************************\r\n");

            do_io_std_test(u8Val1);

            printf("\r\n*********************** [ Test 1bit Default Speed I/O ] ********************************\r\n");

            IPV_SDMMC_SetClock(u8Val1, 0, 0);
            IPV_SDMMC_SetBusTiming(u8Val1, 1);

            do_io_std_burntest(u8Val1);

        }
        else if (strcmp(argv[1], "at1h") == 0)
        {
            IPV_SDMMC_TimerTest(3);
            IPV_SDMMC_CardDetect(u8Val1);
            IPV_SDMMC_Init(u8Val1);

            printf("\r\n*********************** [ Test 1bit High Speed I/O ] ***********************************\r\n");

            IPV_SDMMC_SetHighBus(u8Val1);
            IPV_SDMMC_SetClock(u8Val1, 0, 0);
            IPV_SDMMC_SetBusTiming(u8Val1, 2);

            do_io_std_burntest(u8Val1);

        }
        else if (strcmp(argv[1], "at4d") == 0)
        {
            IPV_SDMMC_TimerTest(3);
            IPV_SDMMC_CardDetect(u8Val1);
            IPV_SDMMC_Init(u8Val1);

            printf("\r\n*********************** [ Test 4bit Low Speed I/O ] ************************************\r\n");

            IPV_SDMMC_SetWideBus(u8Val1);
            do_io_std_test(u8Val1);

            printf("\r\n*********************** [ Test 4bit Default Speed I/O ] ********************************\r\n");

            IPV_SDMMC_SetClock(u8Val1, 0, 0);
            IPV_SDMMC_SetBusTiming(u8Val1, 1);

             do_io_std_burntest(u8Val1);

        }
        else if (strcmp(argv[1], "at4h") == 0)
        {
            IPV_SDMMC_TimerTest(3);
            IPV_SDMMC_CardDetect(u8Val1);
            IPV_SDMMC_Init(u8Val1);

            printf("\r\n*********************** [ Test 4bit High Speed I/O ] ***********************************\r\n");

            IPV_SDMMC_SetWideBus(u8Val1);
            IPV_SDMMC_SetHighBus(u8Val1);
            IPV_SDMMC_SetClock(u8Val1, 0, 0);
            IPV_SDMMC_SetBusTiming(u8Val1, 2);

            do_io_std_burntest(u8Val1);

        }
        else if (strcmp(argv[1], "pwrsave") == 0)
        {
             printf("\r\n*********************** [ Test 4bit High Speed I/O ] ***********************************\r\n");
             do_prepare_power_saving_mode_queue(u8Val1);
        }
        /*else if (strcmp(argv[1], "psave") == 0)
            IPV_SDMMC_PowerSaving();*/
        else
            return cmd_usage(cmdtp);

        return 0;

    case 2:
        if (strcmp(argv[1], "while") == 0)
        {
            while (1) {};
        }
        else
            return cmd_usage(cmdtp);
        break;

    default:
        return cmd_usage(cmdtp);

    }

}
/**************************************************/

U_BOOT_CMD(

	mssdmmc, 6, 1, do_sdmmc_test,
	"Mstar SD/MMC IP Verification System",
    "[OPTION] <parameter>\n"
    "===============================================================================\n"
    "timer <seconds>                 -- SD/MMC Timer Test\n"
	"cdz   <slot>                    -- SD/MMC Card Detection\n"
	"init  <slot>                    -- SD/MMC Init\n"
    "wibus <slot>                    -- SD/MMC Set Wide Bus\n"
    "hibus <slot>                    -- SD/MMC Set High Bus\n"
    "\n"
    "cifd  <slot>                    -- SD/MMC CIFD R/W\n"
    "dma   <slot>                    -- SD/MMC DMA R/W\n"
    "bcifd <slot>                    -- SD/MMC CIFD Burning\n"
    "bdma  <slot>                    -- SD/MMC DMA Burning\n"
//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
    "adma  <slot>                    -- SD/MMC ADMA R/W (FCIE5)\n"
    "badma <slot>                    -- SD/MMC ADMA Burning (FCIE5)\n"
//###########################################################################################################
#endif
    "\n"
    "at1d  <slot>                    -- SD/MMC AutoTest for 1bit default speed\n"
    "at1h  <slot>                    -- SD/MMC AutoTest for 1bit high speed\n"
    "at4d  <slot>                    -- SD/MMC AutoTest for 4bit default speed\n"
    "at4h  <slot>                    -- SD/MMC AutoTest for 4bit high speed\n"
    "\n"
    "pat   <slot><pattern>           -- SD/MMC Set Pattern for R/W\n"
    "clk   <slot><reffclk>           -- SD/MMC Set Bus Clock\n"
    "btm   <slot><bustiming>         -- SD/MMC Set Bus Timing\n"
    "pad   <slot><ip><port><pad>     -- SD/MMC Set Slot/IP/PORT/PAD\n"
    "\n"
    "while                           -- While Loop for TV Tool\n"
	"pwrsave <slot>                  -- SD/MMC PowerSaving\n"
    "===============================================================================\n"

);






