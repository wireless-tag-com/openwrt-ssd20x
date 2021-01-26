/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _HAL_DISP_CHIP_H_
#define _HAL_DISP_CHIP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_CTX_MAX_INST   2

// Device Ctx
#define HAL_DISP_DEVICE_MAX     1

// IRQ CTX
#define HAL_DISP_DEVICE_IRQ_MAX 3

// Inernal Isr
#define HAL_DISP_INTERNAL_ISR_SUPPORT           DISP_OS_INTERNAL_ISR_SUPPORT
#define HAL_DISP_DEVICE_IRQ_INTERNAL_ISR_IDX    1
#define E_HAL_DISP_IRQ_TYPE_INTERNAL_TIMEZONE   (E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_POSITIVE | E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_POSITIVE | E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_NEGATIVE)

// Vga HPD Isr
#define HAL_DISP_VGA_HPD_ISR_SUPPORT            1
#define HAL_DISP_DEVICE_IRQ_VGA_HPD_ISR_IDX     2
#define E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF      (E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON | E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF)

#define HAL_DISP_VIDLAYER_MAX   2 // MOPG & MOPS

#define HAL_DISP_MOPG_GWIN_NUM  16 //MOPG 16 Gwins
#define HAL_DISP_MOPS_GWIN_NUM  1  //MOPS 1 Gwin

#define HAL_DISP_INPUTPORT_NUM  (HAL_DISP_MOPG_GWIN_NUM+HAL_DISP_MOPS_GWIN_NUM) // MOP: 16Gwin_MOPG + 1Gwin_MOPS
#define HAL_DISP_INPUTPORT_MAX  (HAL_DISP_VIDLAYER_MAX  * HAL_DISP_INPUTPORT_NUM)

#define CLK_MHZ(x)                  (x*1000000)
#define HAL_DISP_CLK_MOP_RATE       CLK_MHZ(320)
#define HAL_DISP_CLK_HDMI_RATE      0
#define HAL_DISP_CLK_DAC_RATE       0
#define HAL_DISP_CLK_DISP_432_RATE  CLK_MHZ(432)
#define HAL_DISP_CLK_DISP_216_RATE  CLK_MHZ(216)

#if defined(HDMITX_VGA_SUPPORTED)

#define HAL_DISP_CLK_NUM            5

#define HAL_DISP_CLK_ON_SETTING \
{ \
    1, 1, 1, 1, 1, \
}

#define HAL_DISP_CLK_OFF_SETTING \
{ \
    0, 0, 0, 0, 0,\
}


#define HAL_DISP_CLK_RATE_SETTING \
{ \
    HAL_DISP_CLK_MOP_RATE, \
    HAL_DISP_CLK_HDMI_RATE, \
    HAL_DISP_CLK_DAC_RATE, \
    HAL_DISP_CLK_DISP_432_RATE, \
    HAL_DISP_CLK_DISP_216_RATE, \
}

#define HAL_DISP_CLK_MUX_ATTR \
{ \
    0, 1, 1, 0, 0, \
}

#define HAL_DISP_CLK_NAME \
{   \
    "mop", \
    "hdmi", \
    "dac", \
    "disp432", \
    "disp216", \
}
#else

#define HAL_DISP_CLK_NUM            3

#define HAL_DISP_CLK_ON_SETTING \
{ \
    1, 1, 1,\
}

#define HAL_DISP_CLK_OFF_SETTING \
{ \
    0, 0, 0,\
}


#define HAL_DISP_CLK_RATE_SETTING \
{ \
    HAL_DISP_CLK_MOP_RATE, \
    HAL_DISP_CLK_DISP_432_RATE, \
    HAL_DISP_CLK_DISP_216_RATE, \
}

#define HAL_DISP_CLK_MUX_ATTR \
{ \
    0, 0, 0, \
}
#define HAL_DISP_CLK_NAME \
{   \
    "mop", \
    "disp432", \
    "disp216", \
}

#endif
//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

#endif

