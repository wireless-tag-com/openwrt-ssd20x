#include <common.h>
#include <cmd_osd.h>
#include "asm/arch/mach/ms_types.h"

#ifdef CONFIG_MS_DISPLAY
#include "../drivers/mstar/displaylogo/displaylogo.h"

//static U8 sg_u8UPDLineNumber = 1;  //osd

extern U16 _getPanelWidth(void);
extern U16 _getPanelHeight(void);

static void PaintTextTransfer(char text_trans, char *Text_Transfer)
{
    Text_Transfer[0] = 0xfe;

    switch(text_trans)
    {
    case 'a':
    case 'A':
        Text_Transfer[0] = 0xfc;
        Text_Transfer[1] = 0x12;
        Text_Transfer[2] = 0x12;
        Text_Transfer[3] = 0x12;
        Text_Transfer[4] = 0xfc;
        break;
    case 'b':
    case 'B':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0x6c;
        break;
    case 'c':
    case 'C':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x82;
        Text_Transfer[3] = 0x82;
        Text_Transfer[4] = 0x82;
        break;
    case 'd':
    case 'D':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x82;
        Text_Transfer[3] = 0x82;
        Text_Transfer[4] = 0x7c;
        break;
    case 'e':
    case 'E':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0x92;
        break;
    case 'f':
    case 'F':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x12;
        Text_Transfer[2] = 0x12;
        Text_Transfer[3] = 0x12;
        Text_Transfer[4] = 0x12;
        break;
    case 'g':
    case 'G':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x82;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0xf2;
        break;
    case 'h':
    case 'H':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x10;
        Text_Transfer[2] = 0x10;
        Text_Transfer[3] = 0x10;
        Text_Transfer[4] = 0xfe;
        break;
    case 'i':
    case 'I':
        Text_Transfer[0] = 0x82;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0xfe;
        Text_Transfer[3] = 0x82;
        Text_Transfer[4] = 0x82;
        break;
    case 'j':
    case 'J':
        Text_Transfer[0] = 0x42;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x82;
        Text_Transfer[3] = 0x7e;
        Text_Transfer[4] = 0x2;
        break;
    case 'k':
    case 'K':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x10;
        Text_Transfer[2] = 0x28;
        Text_Transfer[3] = 0x44;
        Text_Transfer[4] = 0x82;
        break;
    case 'l':
    case 'L':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x80;
        Text_Transfer[2] = 0x80;
        Text_Transfer[3] = 0x80;
        Text_Transfer[4] = 0x80;
        break;
    case 'm':
    case 'M':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x04;
        Text_Transfer[2] = 0x08;
        Text_Transfer[3] = 0x04;
        Text_Transfer[4] = 0xfe;
        break;
    case 'n':
    case 'N':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x04;
        Text_Transfer[2] = 0x38;
        Text_Transfer[3] = 0x40;
        Text_Transfer[4] = 0xfe;
        break;
    case 'o':
    case 'O':
        Text_Transfer[0] = 0x7c;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x82;
        Text_Transfer[3] = 0x82;
        Text_Transfer[4] = 0x7c;
        break;
    case 'p':
    case 'P':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x12;
        Text_Transfer[2] = 0x12;
        Text_Transfer[3] = 0x12;
        Text_Transfer[4] = 0x1e;
        break;
    case 'q':
    case 'Q':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0xa2;
        Text_Transfer[3] = 0x42;
        Text_Transfer[4] = 0x5f;
        break;
    case 'r':
    case 'R':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x12;
        Text_Transfer[2] = 0x32;
        Text_Transfer[3] = 0x52;
        Text_Transfer[4] = 0x9e;
        break;
    case 's':
    case 'S':
        Text_Transfer[0] = 0x8c;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0x62;
        break;
    case 't':
    case 'T':
        Text_Transfer[0] = 0x2;
        Text_Transfer[1] = 0x2;
        Text_Transfer[2] = 0xfe;
        Text_Transfer[3] = 0x2;
        Text_Transfer[4] = 0x2;
        break;
    case 'u':
    case 'U':
        Text_Transfer[0] = 0x7e;
        Text_Transfer[1] = 0x80;
        Text_Transfer[2] = 0x80;
        Text_Transfer[3] = 0x80;
        Text_Transfer[4] = 0x7e;
        break;
    case 'v':
    case 'V':
        Text_Transfer[0] = 0x3e;
        Text_Transfer[1] = 0x40;
        Text_Transfer[2] = 0x80;
        Text_Transfer[3] = 0x40;
        Text_Transfer[4] = 0x3e;
        break;
    case 'w':
    case 'W':
        Text_Transfer[0] = 0x7e;
        Text_Transfer[1] = 0x80;
        Text_Transfer[2] = 0xfc;
        Text_Transfer[3] = 0x80;
        Text_Transfer[4] = 0x7e;
        break;
    case 'x':
    case 'X':
        Text_Transfer[0] = 0xc6;
        Text_Transfer[1] = 0x28;
        Text_Transfer[2] = 0x10;
        Text_Transfer[3] = 0x28;
        Text_Transfer[4] = 0xc6;
        break;
    case 'y':
    case 'Y':
        Text_Transfer[0] = 0x6;
        Text_Transfer[1] = 0x8;
        Text_Transfer[2] = 0xf0;
        Text_Transfer[3] = 0x8;
        Text_Transfer[4] = 0x6;
        break;
    case 'z':
    case 'Z':
        Text_Transfer[0] = 0xc2;
        Text_Transfer[1] = 0xa2;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x8a;
        Text_Transfer[4] = 0x86;
        break;
    case ':':
        Text_Transfer[0] = 0x0;
        Text_Transfer[1] = 0x44;
        Text_Transfer[2] = 0xee;
        Text_Transfer[3] = 0x44;
        Text_Transfer[4] = 0x0;
        break;
    case '.':
        Text_Transfer[0] = 0x0;
        Text_Transfer[1] = 0xc0;
        Text_Transfer[2] = 0xc0;
        Text_Transfer[3] = 0x0;
        Text_Transfer[4] = 0x0;
        break;
    case '1':
        Text_Transfer[0] = 0x84;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0xfe;
        Text_Transfer[3] = 0x80;
        Text_Transfer[4] = 0x80;
        break;
    case '2':
        Text_Transfer[0] = 0xe2;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0x8e;
        break;
    case '3':
        Text_Transfer[0] = 0x92;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0xee;
        break;
    case '4':
        Text_Transfer[0] = 0x1e;
        Text_Transfer[1] = 0x10;
        Text_Transfer[2] = 0x10;
        Text_Transfer[3] = 0x10;
        Text_Transfer[4] = 0xfe;
        break;
    case '5':
        Text_Transfer[0] = 0x9e;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0x62;
        break;
    case '6':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0xf2;
        break;
    case '7':
        Text_Transfer[0] = 0xe;
        Text_Transfer[1] = 0x2;
        Text_Transfer[2] = 0x2;
        Text_Transfer[3] = 0x2;
        Text_Transfer[4] = 0xfe;
        break;
    case '8':
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0xfe;
        break;
    case '9':
        Text_Transfer[0] = 0x9e;
        Text_Transfer[1] = 0x92;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x92;
        Text_Transfer[4] = 0xfe;
        break;
    case '0':
        Text_Transfer[0] = 0x7c;
        Text_Transfer[1] = 0xa2;
        Text_Transfer[2] = 0x92;
        Text_Transfer[3] = 0x8a;
        Text_Transfer[4] = 0x7c;
        break;
    case ' ':
        Text_Transfer[0] = 0x0;
        Text_Transfer[1] = 0x0;
        Text_Transfer[2] = 0x0;
        Text_Transfer[3] = 0x0;
        Text_Transfer[4] = 0x0;
        break;
    case '%':
        Text_Transfer[0] = 0x46;
        Text_Transfer[1] = 0x26;
        Text_Transfer[2] = 0x10;
        Text_Transfer[3] = 0xc8;
        Text_Transfer[4] = 0xc4;
        break;
    case '<':
        Text_Transfer[0] = 0x10;
        Text_Transfer[1] = 0x28;
        Text_Transfer[2] = 0x44;
        Text_Transfer[3] = 0x82;
        Text_Transfer[4] = 0x00;
        break;
    case '>':
        Text_Transfer[0] = 0x00;
        Text_Transfer[1] = 0x82;
        Text_Transfer[2] = 0x44;
        Text_Transfer[3] = 0x28;
        Text_Transfer[4] = 0x10;
        break;
    case '-':
        Text_Transfer[0] = 0x10;
        Text_Transfer[1] = 0x10;
        Text_Transfer[2] = 0x10;
        Text_Transfer[3] = 0x10;
        Text_Transfer[4] = 0x10;
        break;
    case '+':
        Text_Transfer[0] = 0x10;
        Text_Transfer[1] = 0x10;
        Text_Transfer[2] = 0x7c;
        Text_Transfer[3] = 0x10;
        Text_Transfer[4] = 0x10;
        break;
    case '_':
        Text_Transfer[0] = 0x80;
        Text_Transfer[1] = 0x80;
        Text_Transfer[2] = 0x80;
        Text_Transfer[3] = 0x80;
        Text_Transfer[4] = 0x80;
        break;
    case '\'':
        Text_Transfer[0] = 0x00;
        Text_Transfer[1] = 0x00;
        Text_Transfer[2] = 0x03;
        Text_Transfer[3] = 0x00;
        Text_Transfer[4] = 0x00;
        break;
    case '@':
        Text_Transfer[0] = 0x00;
        Text_Transfer[1] = 0xfc;
        Text_Transfer[2] = 0xfc;
        Text_Transfer[3] = 0xfc;
        Text_Transfer[4] = 0x00;
        break;

    default:
        //Text_Transfer[0] = 0xfe;
        Text_Transfer[1] = 0xfe;
        Text_Transfer[2] = 0xfe;
        Text_Transfer[3] = 0xfe;
        Text_Transfer[4] = 0xfe;
        break;
    }

}


static void PaintLCDText(int DisplayPanelType,
                         char *LCDDisplay_Text,
                         S16 num,
                         S16 TextRow,
                         U32 color1,
                         BOOL bErase,
                         U32 color2,
                         U16 u16TextSize,
                         U8 u8Pos_x,
                         U8 u8Pos_y)
{
    S16 i = 0, j = 0, k = 0, m = 0, n = 0, text_x = 0, text_y = 9;
    volatile U32 *frameBuffer = (volatile U32 *)GOP_DISP_ADDR;
    volatile U32 color[2] = {color2, color1};
    U8 TEXT[9][301] = {0};
    U16 Boot_DisX = 0, Boot_DisY = 0;
    U8 Text_Transfer[5] = {0};
    U16 u16Factor_x = 0;
    U16 u16Factor_y = 0;

    for(i = 0; i < num; i++)
    {
        PaintTextTransfer(LCDDisplay_Text[i], Text_Transfer);
        for(j = 0; j < 8; j++)
            for(k = 0; k < 5; k++)
                TEXT[j][1+6*i+k] = ((Text_Transfer[k] >> j) & 0x1) ? 1 : 0;
    }
    text_x = i * 6 + 1;

    switch(u8Pos_x)
    {
    case POS_LEFT:
        u16Factor_x = 0;
        break;
    case POS_RIGHT:
        u16Factor_x = 1;
        break;
    case POS_CENTER:
        u16Factor_x = 2;
        break;
    default:
        u16Factor_x = 2;
        break;
    }

    switch(u8Pos_y)
    {
    case POS_TOP:
        u16Factor_y = 0;
        break;
    case POS_BOTTOM:
        u16Factor_y = 3;
        break;
    case POS_CENTER:
        u16Factor_y = 2;
        break;
    default:
        u16Factor_y = 3;
        break;
    }

    if (!u16Factor_x)
    {
        Boot_DisX = 0;
    }
    else
    {
        Boot_DisX = (U16)(_getPanelWidth() / u16Factor_x - text_x * u16TextSize / u16Factor_x);
    }
    Boot_DisY = (U16)(_getPanelHeight() / 4 * u16Factor_y + u16TextSize * (TextRow - 1) * 9 + 1);

    for(i = 0; i < text_y; i++)
        for(j = 0; j < text_x; j++)
            for(m = 0; m < u16TextSize; m++)
                for(n = 0; n < u16TextSize; n++)
                    if(TEXT[i][j] || bErase)
                        frameBuffer[_getPanelWidth()*(Boot_DisY+i*u16TextSize+m) + (Boot_DisX+j*u16TextSize+n) ] = color[TEXT[i][j]];


}


static void BootTEXTStr
(
    U8 *pu8Str,
    U16 u16TextRow,
    U16 u16TextSize,
    BOOL bErase,
    U32 u32TextColor,
    U32 u32BkColor,
    U8 u8Posx,
    U8 u8Posy
)
{
    S16 i = 0;
    U8 au8PrintStr[UPD_MAX_LINE_WIDTH + 1];

    memset(au8PrintStr, 0, UPD_MAX_LINE_WIDTH + 1);

    // max line width is 40
    for(i = 0; i < UPD_MAX_LINE_WIDTH; i++)
    {
        if(pu8Str[i] == '\0')
        {
            break;
        }
        else
        {
            au8PrintStr[i] = pu8Str[i];
        }
    }

    PaintLCDText(RESOL_320_240,
                 au8PrintStr,
                 i,
                 u16TextRow,
                 u32TextColor,
                 bErase,
                 u32BkColor,
                 u16TextSize,
                 u8Posx,
                 u8Posy
                );
}


//osd
void UPDPrintLineSize(char *pu8Str, U32 u32TextColor, U16 u16Size, U8 u8Posx, U8 u8LineNumber)
{
    BootTEXTStr(pu8Str, u8LineNumber, u16Size, 0, u32TextColor, UPD_BACK_GROUND_COLOR, u8Posx, POS_TOP);
    //sg_u8UPDLineNumber += u16Size;
}

void DrawProgressBar(U32 u32X, U32 u32Y, U32 u32Percentage, U8 u8Style)
{
    volatile U32 *frameBuffer = (volatile U32 *)GOP_DISP_ADDR;
    int i = 0, j = 0;
    int avail_value = 0;

    for (i = 0; i < PROGRESS_BAR_HEIGHT + 6; i++)
    {
        frameBuffer[_getPanelWidth()*(i + u32Y - 3) + (u32X - 3)] = COLOR_RED;
        frameBuffer[_getPanelWidth()*(i + u32Y - 3) + (PROGRESS_BAR_WIDTH + u32X  + 3)] = COLOR_RED;
    }
    
    for (j = 0; j < PROGRESS_BAR_WIDTH + 6; j++)
    {
        frameBuffer[_getPanelWidth()*(u32Y - 3) + (j + u32X - 3)] = COLOR_RED;
        frameBuffer[_getPanelWidth()*(PROGRESS_BAR_HEIGHT + u32Y + 3) + (j + u32X - 3)] = COLOR_RED;
    }

    avail_value = u32Percentage * (PROGRESS_BAR_WIDTH / 100);
    for (i = 0; i < avail_value ; i++)
        for (j = 0; j < PROGRESS_BAR_HEIGHT; j++)
        {
            frameBuffer[_getPanelWidth()*(j + u32Y) + (i + u32X)] = COLOR_RED;
        }
        
     for (i = avail_value; i < PROGRESS_BAR_WIDTH; i++)
        for (j = 0; j < PROGRESS_BAR_HEIGHT; j++)
        {
            frameBuffer[_getPanelWidth()*(j + u32Y) + (i + u32X)] = COLOR_BLACK;
        }

        
}
#else

//osd
void UPDPrintLineSize(char *pu8Str, U32 u32TextColor, U16 u16Size, U8 u8Posx, U8 u8LineNumber)
{
	puts(pu8Str);
    //BootTEXTStr(pu8Str, u8LineNumber, u16Size, 0, u32TextColor, UPD_BACK_GROUND_COLOR, u8Posx, POS_TOP);
    //sg_u8UPDLineNumber += u16Size;
}

void DrawProgressBar(U32 u32X, U32 u32Y, U32 u32Percentage, U8 u8Style)
{
	puts(".");
}


#endif

