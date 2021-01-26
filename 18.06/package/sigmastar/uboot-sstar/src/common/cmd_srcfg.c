#include <common.h>
#include <command.h>
#include <malloc.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"

#if defined(CONFIG_INFINITY5)
#include "../drivers/mstar/isp/snr_cfg_infinity5.h"

int do_sensor_cfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 7)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    u32 SnrIdx  = simple_strtol(argv[1], 0, 10);
    u32 BusType = simple_strtol(argv[2], 0, 10);
    u32 PadSel  = simple_strtol(argv[3], 0, 10);
    u32 MclkIdx = simple_strtol(argv[4], 0, 10);
    u32 Pdwn    = simple_strtol(argv[5], 0, 10);
    u32 Rst     = simple_strtol(argv[6], 0, 10);

    Set_SNR_BusType(SnrIdx, BusType);
    Set_SNR_IOPad(SnrIdx, PadSel);
    Set_SNR_MCLK(SnrIdx, MclkIdx>=999?0:1, MclkIdx);

    SNR_PowerDown(SnrIdx, Pdwn);
    SNR_Reset(SnrIdx, Rst);

    printf("\n Sensor configuration, ID=%u, Bus=%u, Pad=%u, MCLK=%u, PWD_PIN=%u, RST_PIN=%u \n\n",
            SnrIdx, BusType, PadSel, MclkIdx, Pdwn, Rst);
    return 0;
}

U_BOOT_CMD(
    srcfg,  CONFIG_SYS_MAXARGS,    1,    do_sensor_cfg,
    "sensor pin and mclk configuration.",
    "SensorId BusType PadSel MclkIdx PwdPin RstPin\n"
    " -SensorId Select sensor ID 0~1\n"
    " -BusType Select sensor bus type 0:Parallel, 1:MIPI_1Lane, 2:MIPI_2Lane, 3:MIPI_4Lane, 4:BT656\n"
    " -PadSel Select sensor pad 1~5\n"
    " -MclkIdx 0:27MHz, 1:72MHz, 2:61.7MHz, 3:54MHz, 4:48MHz, 5:43.2MHz, \
               6:36MHz, 7:24MHz, 8:21.6MHz, 9:12MHz, 10:5.4MHz, 999:MCLK off\n"
    " -PwdPin sensor power down pin 0:low 1:high\n"
    " -RstPin sensor reset pin 0:low 1:high\n"
);
#elif defined(CONFIG_INFINITY6)
#include "../drivers/mstar/isp/snr_cfg_infinity6.h"

int do_sensor_cfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 6)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    u32 PdnPad  = simple_strtol(argv[1], 0, 10);
    u32 RstPad  = simple_strtol(argv[2], 0, 10);
    u32 MclkIdx = simple_strtol(argv[3], 0, 10);
    u32 Pdwn    = simple_strtol(argv[4], 0, 10);
    u32 Rst     = simple_strtol(argv[5], 0, 10);

    Set_SNR_IOPad(0);
    Set_SNR_PdnPad(PdnPad);
    Set_SNR_RstPad(RstPad);
    Set_SNR_MCLK(0, MclkIdx>=999?0:1, MclkIdx);

    SNR_PowerDown(0, Pdwn);
    SNR_Reset(0, Rst);

    printf("\n Sensor configuration, PdnPad=%u, RstPad=%u, MCLK=%u, PWD_PIN=%u, RST_PIN=%u \n\n",
            PdnPad, RstPad, MclkIdx, Pdwn, Rst);
    return 0;
}

U_BOOT_CMD(
    srcfg,  CONFIG_SYS_MAXARGS,    1,    do_sensor_cfg,
    "sensor pin and mclk configuration.",
    "PdnPad RstPad MclkIdx PwdPin RstPin\n"
    " -PdnPad   Select sensor power down pad  1:PAD_SR_IO12, 2:PAD_PWM0\n"
    " -RstPad   Select sensor reset pad       1:PAD_SR_IO13, 2:PAD_PWM1\n"
    " -MclkIdx 0:27MHz, 1:72MHz, 2:61.7MHz, 3:54MHz, 4:48MHz, 5:43.2MHz, \
               6:36MHz, 7:24MHz, 8:21.6MHz, 9:12MHz, 10:5.4MHz, 999:MCLK off\n"
    " -PwdPin sensor power down pin 0:low 1:high\n"
    " -RstPin sensor reset pin 0:low 1:high\n"
);
#else
#include <../drivers/mstar/isp/isp_pub.h>

int do_sensor_cfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 5)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    u32 pad = simple_strtol(argv[1],0,10);
    u32 mclk = simple_strtol(argv[2],0,10);
    u32 pdwn = simple_strtol(argv[3],0,10);
    u32 rst = simple_strtol(argv[4],0,10);

    Select_SR_IOPad((SR_PAD_CFG)pad);
    Set_SR_MCLK(mclk>=999?0:1, mclk);

    ISP_SrPwdn(0, pdwn);
    ISP_SrRst(0, rst);
    ISP_SrPwdn(1, pdwn);
    ISP_SrRst(1, rst);
    printf("\n sensor configuration, PAD=%u,MCLK=%uMHz, PWD_PIN=%u, RST_PIN=%u \n\n",pad,mclk,pdwn,rst);
    return 0;
}

U_BOOT_CMD(
    srcfg,  CONFIG_SYS_MAXARGS,    1,    do_sensor_cfg,
    "sensor pin and mclk configuration.",
    "pad mclk pwd_pin rst_pin\n"
    " -pad Select sensor pad 0~6\n"
    " -mclk 0:5.4MHz 1:12MHz 2:21.6MHz 3:27MHz 4:36MHz 5:43.2MHz 6:54MHz 7:61.7MHz 999:MCLK off\n"
    " -pwd_pin sensor power down pin 0:low 1:high\n"
    " -rst_pin sensor reset pin 0:low 1:high\n"
);

#endif




