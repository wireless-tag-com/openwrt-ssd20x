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

#ifndef _HAL_HDMITX_CHIP_H_
#define _HAL_HDMITX_CHIP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_HDMITX_CTX_INST_MAX        2


#define HAL_HDMITX_CLK_NUM              3

#define CLK_MHZ(x)                      (x*1000000)
#define HAL_HDMITX_CLK_HDMI             0
#define HAL_HDMITX_CLK_DISP_432         CLK_MHZ(432)
#define HAL_HDMITX_CLK_DISP_216         CLK_MHZ(216)

#define HAL_HDMITX_CLK_ON_SETTING \
{ \
    1, 1, 1,\
}

#define HAL_HDMITX_CLK_OFF_SETTING \
{ \
    0, 0, 0,\
}


#define HAL_HDMITX_CLK_RATE_SETTING \
{ \
    HAL_HDMITX_CLK_HDMI, \
    HAL_HDMITX_CLK_DISP_432, \
    HAL_HDMITX_CLK_DISP_216, \
}

#define HAL_HDMITX_CLK_MUX_ATTR \
{ \
    1, 0, 0, \
}


//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------


#endif

