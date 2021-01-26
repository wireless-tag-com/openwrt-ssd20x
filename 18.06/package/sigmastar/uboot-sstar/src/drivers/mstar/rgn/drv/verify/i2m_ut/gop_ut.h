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

#ifndef _GOP_UT_H_
#define _GOP_UT_H_

//-------------------------------------------------------------------------------------------------
//  SC Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    int argc;
    char *argv[100];
}GopUtStrConfig_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _GOP_UT_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void GopUtParsingCommand(char *str, GopUtStrConfig_t *pstStrCfg);
INTERFACE void GopTestStore(GopUtStrConfig_t *pstStringCfg);
INTERFACE int  GopTestShow(char *DstBuf);



#undef INTERFACE

#endif
