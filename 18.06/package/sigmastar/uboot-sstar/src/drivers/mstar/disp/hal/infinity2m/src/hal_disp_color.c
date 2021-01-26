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

#define _HAL_DISP_COLOR_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"
#include "hal_disp_color.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define ACE_ENABLE_PC_YUV_TO_RGB    0x01
#define ACE_ENABLE_PC_SRGB          0x02
#define ACE_ENABLE_HUE_256_STEP     0x04

#define ENABLE_CBCR     1
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    s16 *tSrgbMatrix;
    bool bColorCorrectMatrixUpdate;

    u8 u8ACEConfig;
    HalDispColorYuvToRgbMatrixType_e enMatrixType;

    u8 u8VideoRCon;
    u8 u8VideoGCon;
    u8 u8VideoBCon;
    u8 u8VideoContrast;
    u8 u8VideoSaturation;

    u8 u8Brightness;

#if ( ENABLE_CBCR )
    u8 u8VideoCb;
    u8 u8VideoCr;
#endif

    u8 u8VideoHue;

    s16 sYuvToRGBMatrix[3][3];
    s16 sVideoSatHueMatrix[3][3];
    s16 sVideoContrastMatrix[3][3];
    s16 sColorCorrrectMatrix[3][3];

    // For PC
    u8 u8PCRCon;
    u8 u8PCGCon;
    u8 u8PCBCon;
    u8 u8PCContrast;
    s16 sPCConRGBMatrix[3][3];
}HalDispColorConfit;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
HalDispColorConfit g_tColorCfg[2];

s16 tSDTVYuv2rgb[3][3] =
{
    {   0x0662, 0x04A8,  0x0000 }, // 1.596,  1.164, 0
    {  -0x0341, 0x04A8, -0x0190 }, // -0.813, 1.164, -0.391
    {   0x0000, 0x04A8,  0x0812 }  // 0,      1.164, 2.018
};

s16 tHDTVYuv2rgb[3][3] =
{
    {   0x072C, 0x04A8,  0x0000 }, // 1.793,  1.164, 0
    {  -0x0223, 0x04A8, -0x00DA }, // -0.534, 1.164, -0.213
    {   0x0000, 0x04A8,  0x0876 }  // 0,      1.164, 2.115
};


s16 tByPassYuv2rgb[3][3] =
{
    {   0x0400, 0x0000, 0x0000 }, // 1.793,  1.164, 0
    {   0x0000, 0x0400, 0x0000 }, // -0.534, 1.164, -0.213
    {   0x0000, 0x0000, 0x0400 }  // 0,      1.164, 2.115
};


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
s16 sine(u16 u16Hue, bool b256Step)
{
    // sin((u8Hue * 3.1415926)/180)) * 1024

    s16 tHueToSine100[101] =
    {
        784, // 0
        772, // 1
        760, // 2
        748, // 3
        736, // 4
        724, // 5
        711, // 6
        698, // 7
        685, // 8
        671, // 9
        658, // 10
        644, // 11
        630, // 12
        616, // 13
        601, // 14
        587, // 15
        572, // 16
        557, // 17
        542, // 18
        527, // 19
        511, // 20
        496, // 21
        480, // 22
        464, // 23
        448, // 24
        432, // 25
        416, // 26
        400, // 27
        383, // 28
        366, // 29
        350, // 30
        333, // 31
        316, // 32
        299, // 33
        282, // 34
        265, // 35
        247, // 36
        230, // 37
        212, // 38
        195, // 39
        177, // 40
        160, // 41
        142, // 42
        124, // 43
        107, // 44
        89, // 45
        71, // 46
        53, // 47
        35, // 48
        17, // 49
        0, // 50
        -17, // 51
        -35, // 52
        -53, // 53
        -71, // 54
        -89, // 55
        -107, // 56
        -124, // 57
        -142, // 58
        -160, // 59
        -177, // 60
        -195, // 61
        -212, // 62
        -230, // 63
        -247, // 64
        -265, // 65
        -282, // 66
        -299, // 67
        -316, // 68
        -333, // 69
        -350, // 70
        -366, // 71
        -383, // 72
        -400, // 73
        -416, // 74
        -432, // 75
        -448, // 76
        -464, // 77
        -480, // 78
        -496, // 79
        -512, // 80
        -527, // 81
        -542, // 82
        -557, // 83
        -572, // 84
        -587, // 85
        -601, // 86
        -616, // 87
        -630, // 88
        -644, // 89
        -658, // 90
        -671, // 91
        -685, // 92
        -698, // 93
        -711, // 94
        -724, // 95
        -736, // 96
        -748, // 97
        -760, // 98
        -772, // 99
        -784, // 100
    };

    s16 tHueToSine256[257] =
    {
        806,    //0
        817,    //1
        828,    //2
        838,    //3
        848,    //4
        858,    //5
        868,    //6
        877,    //7
        886,    //8
        895,    //9
        904,    //10
        912,    //11
        920,    //12
        928,    //13
        935,    //14
        942,    //15
        949,    //16
        955,    //17
        962,    //18
        968,    //19
        973,    //20
        979,    //21
        984,    //22
        989,    //23
        993,    //24
        997,    //25
        1001,   //26
        1005,   //27
        1008,   //28
        1011,   //29
        1014,   //30
        1016,   //31
        1018,   //32
        1020,   //33
        1021,   //34
        1022,   //35
        1023,   //36
        1023,   //37
        1023,   //38
        1023,   //39
        1023,   //40
        1022,   //41
        1021,   //42
        1020,   //43
        1018,   //44
        1016,   //45
        1014,   //46
        1011,   //47
        1008,   //48
        1005,   //49
        1001,   //50
        997,    //51
        993,    //52
        989,    //53
        984,    //54
        979,    //55
        973,    //56
        968,    //57
        962,    //58
        955,    //59
        949,    //60
        942,    //61
        935,    //62
        928,    //63
        920,    //64
        912,    //65
        904,    //66
        895,    //67
        886,    //68
        877,    //69
        868,    //70
        858,    //71
        848,    //72
        838,    //73
        828,    //74
        817,    //75
        806,    //76
        795,    //77
        784,    //78
        772,    //79
        760,    //80
        748,    //81
        736,    //82
        724,    //83
        711,    //84
        698,    //85
        685,    //86
        671,    //87
        658,    //88
        644,    //89
        630,    //90
        616,    //91
        601,    //92
        587,    //93
        572,    //94
        557,    //95
        542,    //96
        527,    //97
        511,    //98
        496,    //99
        480,    //100
        464,    //101
        448,    //102
        432,    //103
        416,    //104
        400,    //105
        383,    //106
        366,    //107
        350,    //108
        333,    //109
        316,    //110
        299,    //111
        282,    //112
        265,    //113
        247,    //114
        230,    //115
        212,    //116
        195,    //117
        177,    //118
        160,    //119
        142,    //120
        124,    //121
        107,    //122
        89,     //123
        71,     //124
        53,     //125
        35,     //126
        17,     //127
        0,      //128
        -17,    //129
        -35,    //130
        -53,    //131
        -71,    //132
        -89,    //133
        -107,   //134
        -124,   //135
        -142,   //136
        -160,   //137
        -177,   //138
        -195,   //139
        -212,   //140
        -230,   //141
        -247,   //142
        -265,   //143
        -282,   //144
        -299,   //145
        -316,   //146
        -333,   //147
        -350,   //148
        -366,   //149
        -383,   //150
        -400,   //151
        -416,   //152
        -432,   //153
        -448,   //154
        -464,   //155
        -480,   //156
        -496,   //157
        -511,   //158
        -527,   //159
        -542,   //160
        -557,   //161
        -572,   //162
        -587,   //163
        -601,   //164
        -616,   //165
        -630,   //166
        -644,   //167
        -658,   //168
        -671,   //169
        -685,   //170
        -698,   //171
        -711,   //172
        -724,   //173
        -736,   //174
        -748,   //175
        -760,   //176
        -772,   //177
        -784,   //178
        -795,   //179
        -806,   //180
        -817,   //181
        -828,   //182
        -838,   //183
        -848,   //184
        -858,   //185
        -868,   //186
        -877,   //187
        -886,   //188
        -895,   //189
        -904,   //190
        -912,   //191
        -920,   //192
        -928,   //193
        -935,   //194
        -942,   //195
        -949,   //196
        -955,   //197
        -962,   //198
        -968,   //199
        -973,   //200
        -979,   //201
        -984,   //202
        -989,   //203
        -993,   //204
        -997,   //205
        -1001,  //206
        -1005,  //207
        -1008,  //208
        -1011,  //209
        -1014,  //210
        -1016,  //211
        -1018,  //212
        -1020,  //213
        -1021,  //214
        -1022,  //215
        -1023,  //216
        -1023,  //217
        -1023,  //218
        -1023,  //219
        -1023,  //220
        -1022,  //221
        -1021,  //222
        -1020,  //223
        -1018,  //224
        -1016,  //225
        -1014,  //226
        -1011,  //227
        -1008,  //228
        -1005,  //229
        -1001,  //230
        -997,   //231
        -993,   //232
        -989,   //233
        -984,   //234
        -979,   //235
        -973,   //236
        -968,   //237
        -962,   //238
        -955,   //239
        -949,   //240
        -942,   //241
        -935,   //242
        -928,   //243
        -920,   //244
        -912,   //245
        -904,   //246
        -895,   //247
        -886,   //248
        -877,   //249
        -868,   //250
        -858,   //251
        -848,   //252
        -838,   //253
        -828,   //254
        -817,   //255
        -806,   //256
    };


    if( b256Step )
    {
        if( u16Hue > 256 )
        {
            u16Hue = 256;
        }
        return tHueToSine256[u16Hue];
    }
    else
    {
        if( u16Hue > 100 )
        {
            u16Hue = 100;
        }
        return tHueToSine100[u16Hue];
    }
}

s16 cosine(u16 u16Hue, bool b256Step)
{
    // cos((u8Hue * 3.1415926)/180)) * 1024
    s16 tHueToCosine100[101] =
    {
        658, // 0
        671, // 1
        685, // 2
        698, // 3
        711, // 4
        724, // 5
        736, // 6
        748, // 7
        760, // 8
        772, // 9
        784, // 10
        795, // 11
        806, // 12
        817, // 13
        828, // 14
        838, // 15
        848, // 16
        858, // 17
        868, // 18
        877, // 19
        886, // 20
        895, // 21
        904, // 22
        912, // 23
        920, // 24
        928, // 25
        935, // 26
        942, // 27
        949, // 28
        955, // 29
        962, // 30
        968, // 31
        973, // 32
        979, // 33
        984, // 34
        989, // 35
        993, // 36
        997, // 37
        1001, // 38
        1005, // 39
        1008, // 40
        1011, // 41
        1014, // 42
        1016, // 43
        1018, // 44
        1020, // 45
        1021, // 46
        1022, // 47
        1023, // 48
        1023, // 49
        1024, // 50
        1023, // 51
        1023, // 52
        1022, // 53
        1021, // 54
        1020, // 55
        1018, // 56
        1016, // 57
        1014, // 58
        1011, // 59
        1008, // 60
        1005, // 61
        1001, // 62
        997, // 63
        993, // 64
        989, // 65
        984, // 66
        979, // 67
        973, // 68
        968, // 69
        962, // 70
        955, // 71
        949, // 72
        942, // 73
        935, // 74
        928, // 75
        920, // 76
        912, // 77
        904, // 78
        895, // 79
        886, // 80
        877, // 81
        868, // 82
        858, // 83
        848, // 84
        838, // 85
        828, // 86
        817, // 87
        806, // 88
        795, // 89
        784, // 90
        772, // 91
        760, // 92
        748, // 93
        736, // 94
        724, // 95
        711, // 96
        698, // 97
        685, // 98
        671, // 99
        658, // 100
    };

    s16 tHueToCosine256[257] =
    {
        -630, //0
        -616, //1
        -601, //2
        -587, //3
        -572, //4
        -557, //5
        -542, //6
        -527, //7
        -511, //8
        -496, //9
        -480, //10
        -464, //11
        -448, //12
        -432, //13
        -416, //14
        -400, //15
        -383, //16
        -366, //17
        -350, //18
        -333, //19
        -316, //20
        -299, //21
        -282, //22
        -265, //23
        -247, //24
        -230, //25
        -212, //26
        -195, //27
        -177, //28
        -160, //29
        -142, //30
        -124, //31
        -107, //32
        -89,  //33
        -71,  //34
        -53,  //35
        -35,  //36
        -17,  //37
        0,    //38
        17,   //39
        35,   //40
        53,   //41
        71,   //42
        89,   //43
        107,  //44
        124,  //45
        142,  //46
        160,  //47
        177,  //48
        195,  //49
        212,  //50
        230,  //51
        247,  //52
        265,  //53
        282,  //54
        299,  //55
        316,  //56
        333,  //57
        350,  //58
        366,  //59
        383,  //60
        400,  //61
        416,  //62
        432,  //63
        448,  //64
        464,  //65
        480,  //66
        496,  //67
        512,  //68
        527,  //69
        542,  //70
        557,  //71
        572,  //72
        587,  //73
        601,  //74
        616,  //75
        630,  //76
        644,  //77
        658,  //78
        671,  //79
        685,  //80
        698,  //81
        711,  //82
        724,  //83
        736,  //84
        748,  //85
        760,  //86
        772,  //87
        784,  //88
        795,  //89
        806,  //90
        817,  //91
        828,  //92
        838,  //93
        848,  //94
        858,  //95
        868,  //96
        877,  //97
        886,  //98
        895,  //99
        904,  //100
        912,  //101
        920,  //102
        928,  //103
        935,  //104
        942,  //105
        949,  //106
        955,  //107
        962,  //108
        968,  //109
        973,  //110
        979,  //111
        984,  //112
        989,  //113
        993,  //114
        997,  //115
        1001, //116
        1005, //117
        1008, //118
        1011, //119
        1014, //120
        1016, //121
        1018, //122
        1020, //123
        1021, //124
        1022, //125
        1023, //126
        1023, //127
        1024, //128
        1023, //129
        1023, //130
        1022, //131
        1021, //132
        1020, //133
        1018, //134
        1016, //135
        1014, //136
        1011, //137
        1008, //138
        1005, //139
        1001, //140
        997,  //141
        993,  //142
        989,  //143
        984,  //144
        979,  //145
        973,  //146
        968,  //147
        962,  //148
        955,  //149
        949,  //150
        942,  //151
        935,  //152
        928,  //153
        920,  //154
        912,  //155
        904,  //156
        895,  //157
        886,  //158
        877,  //159
        868,  //160
        858,  //161
        848,  //162
        838,  //163
        828,  //164
        817,  //165
        806,  //166
        795,  //167
        784,  //168
        772,  //169
        760,  //170
        748,  //171
        736,  //172
        724,  //173
        711,  //174
        698,  //175
        685,  //176
        671,  //177
        658,  //178
        644,  //179
        630,  //180
        616,  //181
        601,  //182
        587,  //183
        572,  //184
        557,  //185
        542,  //186
        527,  //187
        512,  //188
        496,  //189
        480,  //190
        464,  //191
        448,  //192
        432,  //193
        416,  //194
        400,  //195
        383,  //196
        366,  //197
        350,  //198
        333,  //199
        316,  //200
        299,  //201
        282,  //202
        265,  //203
        247,  //204
        230,  //205
        212,  //206
        195,  //207
        177,  //208
        160,  //209
        142,  //210
        124,  //211
        107,  //212
        89,   //213
        71,   //214
        53,   //215
        35,   //216
        17,   //217
        0,    //218
        -17,  //219
        -35,  //220
        -53,  //221
        -71,  //222
        -89,  //223
        -107, //224
        -124, //225
        -142, //226
        -160, //227
        -177, //228
        -195, //229
        -212, //230
        -230, //231
        -247, //232
        -265, //233
        -282, //234
        -299, //235
        -316, //236
        -333, //237
        -350, //238
        -366, //239
        -383, //240
        -400, //241
        -416, //242
        -432, //243
        -448, //244
        -464, //245
        -480, //246
        -496, //247
        -511, //248
        -527, //249
        -542, //250
        -557, //251
        -572, //252
        -587, //253
        -601, //254
        -616, //255
        -630, //256
    };

    if( b256Step )
    {
        if( u16Hue > 256 )
        {
            u16Hue = 256;
        }
        return tHueToCosine256[u16Hue];
    }
    else
    {
        if( u16Hue > 100 )
        {
            u16Hue = 100;
        }
        return tHueToCosine100[u16Hue];
    }
}

void ArrayMultiply(s16 sFirst[3][3], s16 sSecond[3][3], s16 sResult[3][3])
{
    u8 u8Row, u8Col;

    //go line by line
    for(u8Row=0; u8Row!=3; u8Row++)
    {
        // go column by column
        for(u8Col=0; u8Col!=3; u8Col++)
        {
            sResult[u8Row][u8Col] = (((long)sFirst[u8Row][0] * sSecond[0][u8Col]) +
                                     ((long)sFirst[u8Row][1] * sSecond[1][u8Col]) +
                                     ((long)sFirst[u8Row][2] * sSecond[2][u8Col])) >> 10;
        } // for
    } // for
}

void SetVideoSatHueMatrix(u8 u8Id)
{
    u16 u16Hue;
#if (ENABLE_CBCR == 0)
    s16 s16Tmp;
#endif
    bool b256Step = (g_tColorCfg[u8Id].u8ACEConfig & ACE_ENABLE_HUE_256_STEP) ? 1 : 0;
/*
    if( g_tColorCfg[u8Id].u8ACEConfig & ACE_ENABLE_HUE_256_STEP )
    {
        u16Hue = ((g_tColorCfg[u8Id].u8VideoHue <= 0x80) ? (0x80 - g_tColorCfg[u8Id].u8VideoHue) : (360-(g_tColorCfg[u8Id].u8VideoHue-0x80)));
    }
    else
    {
        u16Hue = ((g_tColorCfg[u8Id].u8VideoHue <= 50) ? (50 - g_tColorCfg[u8Id].u8VideoHue) : (360-(g_tColorCfg[u8Id].u8VideoHue-50)));
    }
*/
    u16Hue = g_tColorCfg[u8Id].u8VideoHue;
#if( ENABLE_CBCR )
    g_tColorCfg[u8Id].sVideoSatHueMatrix[2][2] = ((((long)cosine(u16Hue, b256Step) * g_tColorCfg[u8Id].u8VideoSaturation * g_tColorCfg[u8Id].u8VideoCb))>>14);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[0][0] = ((((long)cosine(u16Hue, b256Step) * g_tColorCfg[u8Id].u8VideoSaturation * g_tColorCfg[u8Id].u8VideoCr))>>14);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[2][0] = ((((long)sine(u16Hue, b256Step)   * g_tColorCfg[u8Id].u8VideoSaturation * g_tColorCfg[u8Id].u8VideoCr))>>14);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[0][2] = (((-(long)sine(u16Hue, b256Step)  * g_tColorCfg[u8Id].u8VideoSaturation * g_tColorCfg[u8Id].u8VideoCb))>>14);

#else
    s16Tmp = ((s16)g_tColorCfg[u8Id].u8VideoSaturation * 8);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[2][2] = ((((long)cosine(u16Hue, b256Step) * s16Tmp))>>10);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[0][0] = ((((long)cosine(u16Hue, b256Step) * s16Tmp))>>10);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[2][0] = ((((long)sine(u16Hue, b256Step)   * s16Tmp))>>10);
    g_tColorCfg[u8Id].sVideoSatHueMatrix[0][2] = (((-(long)sine(u16Hue, b256Step)  * s16Tmp))>>10);

#endif

    g_tColorCfg[u8Id].sVideoSatHueMatrix[1][1] = 1024;
    g_tColorCfg[u8Id].sVideoSatHueMatrix[0][1] = g_tColorCfg[u8Id].sVideoSatHueMatrix[1][0] = g_tColorCfg[u8Id].sVideoSatHueMatrix[1][2] = g_tColorCfg[u8Id].sVideoSatHueMatrix[2][1] = 0;
}

void SetVideoContrastMatrix(u8 u8Id)
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Con=%d, RCon=%d, GCon=%d BCon=%d\n",
        __FUNCTION__, __LINE__, u8Id,
        g_tColorCfg[u8Id].u8VideoContrast, g_tColorCfg[u8Id].u8VideoRCon, g_tColorCfg[u8Id].u8VideoGCon, g_tColorCfg[u8Id].u8VideoBCon);

    g_tColorCfg[u8Id].sVideoContrastMatrix[0][0] = ( (u16)g_tColorCfg[u8Id].u8VideoRCon * (g_tColorCfg[u8Id].u8VideoContrast)) >> 4;
    g_tColorCfg[u8Id].sVideoContrastMatrix[1][1] = ( (u16)g_tColorCfg[u8Id].u8VideoGCon * (g_tColorCfg[u8Id].u8VideoContrast)) >> 4;
    g_tColorCfg[u8Id].sVideoContrastMatrix[2][2] = ( (u16)g_tColorCfg[u8Id].u8VideoBCon * (g_tColorCfg[u8Id].u8VideoContrast)) >> 4;
}

void WriteColorMatrix( u8 u8Id, s16* psMatrix )
{
    u8 i,j;
    u32 u32Addr = u8Id == 0 ? REG_DISP_TOP_OP2_31_L : REG_DISP_TOP_OP2_3D_L;
    s16 s16Tmp;

    for(i=0; i!=3; i++)
    {
        for(j=0; j!=3; j++)
        {
            s16Tmp = psMatrix[i*3+j];
            if( s16Tmp >= 0 )
            {
               if( s16Tmp > 0xfff )
                {
                    s16Tmp = 0xfff;
                }
            }
            else
            {
                s16Tmp = s16Tmp * -1;
                if( s16Tmp > 0xfff )
                {
                    s16Tmp = 0xfff;
                }
                s16Tmp |= 0x1000;
            }
            W2BYTE(u32Addr, s16Tmp);
            u32Addr += 2;
        } // for
    } // for
}

void SetPCConRGBMatrix(u8 u8Id)
{
    g_tColorCfg[u8Id].sPCConRGBMatrix[0][0] = ( (u16)g_tColorCfg[u8Id].u8PCRCon * (g_tColorCfg[u8Id].u8PCContrast)) >> 4;
    g_tColorCfg[u8Id].sPCConRGBMatrix[1][1] = ( (u16)g_tColorCfg[u8Id].u8PCGCon * (g_tColorCfg[u8Id].u8PCContrast)) >> 4;
    g_tColorCfg[u8Id].sPCConRGBMatrix[2][2] = ( (u16)g_tColorCfg[u8Id].u8PCBCon * (g_tColorCfg[u8Id].u8PCContrast)) >> 4;
}

void SetVideoColorMatrix(u8 u8Id)
{
    s16 sResultTmp1[3][3];
    s16 sResultTmp2[3][3];
    s16* psOutTab = NULL;
    u32 u32CtrlReg = u8Id == 0 ? REG_DISP_TOP_OP2_30_L : REG_DISP_TOP_OP2_3C_L;

    // Adjust hue&saturation, and then YUV to RGB
    ArrayMultiply( g_tColorCfg[u8Id].sYuvToRGBMatrix, g_tColorCfg[u8Id].sVideoSatHueMatrix, sResultTmp1 );

    // Adjust contrast-RGB
    ArrayMultiply( g_tColorCfg[u8Id].sVideoContrastMatrix, sResultTmp1, sResultTmp2 );

    // Do color correction
    if( g_tColorCfg[u8Id].bColorCorrectMatrixUpdate)
    {
        ArrayMultiply( g_tColorCfg[u8Id].sColorCorrrectMatrix, sResultTmp2, sResultTmp1 );
        psOutTab = (s16*)sResultTmp1;
    }

    if(psOutTab)
    {
        WriteColorMatrix( u8Id, psOutTab );
    }

    if(g_tColorCfg[u8Id].enMatrixType == E_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS)
    {
        W2BYTEMSK(u32CtrlReg, 0x0001, 0x000F);
    }
    else
    {
        W2BYTEMSK(u32CtrlReg, 0x000B, 0x000F);
    }
}


void SetPCColorMatrix(u8 u8Id)
{
    s16 sResultTmp1[3][3];
    s16 sResultTmp2[3][3];
    s16 ColorMatrix_1[3][3];
    s16* psOutTab;
    u8 u8ColorMatrixCtl;
    u8 i,j;
    u32 u32CtrlReg = u8Id == 0 ? REG_DISP_TOP_OP2_30_L : REG_DISP_TOP_OP2_3C_L;

    psOutTab = (s16*)g_tColorCfg[u8Id].sPCConRGBMatrix;

    // YUV => RGB
    if( g_tColorCfg[u8Id].u8ACEConfig & ACE_ENABLE_PC_YUV_TO_RGB )
    {
        ArrayMultiply( g_tColorCfg[u8Id].sPCConRGBMatrix,  g_tColorCfg[u8Id].sYuvToRGBMatrix, sResultTmp1 );
        psOutTab = (s16*)sResultTmp1;
        u8ColorMatrixCtl = 0x35;//0x15; // 20050613 Daniel
    }
    else
    {
        u8ColorMatrixCtl = 0x30;
        for(i=0; i<3; i++)
        {
            for(j=0; j<3; j++)
            {
                sResultTmp1[i][j] = g_tColorCfg[u8Id].sPCConRGBMatrix[i][j];
            }
        }
    }

    // Convert to sRGB
    if( g_tColorCfg[u8Id].u8ACEConfig & ACE_ENABLE_PC_SRGB && g_tColorCfg[u8Id].tSrgbMatrix != NULL )
    {
        for ( i=0;i<3;i++ )
        {
            for ( j=0;j<3;j++ )
            {
               ColorMatrix_1[i][j]=*(g_tColorCfg[u8Id].tSrgbMatrix +(i*3)+j);
            }
        }

        ArrayMultiply( ColorMatrix_1, sResultTmp1, sResultTmp2 );
        psOutTab = (s16*)sResultTmp2;
    }

    WriteColorMatrix( u8Id, psOutTab );
    W2BYTEMSK(u32CtrlReg, u8ColorMatrixCtl, 0x000F);
}

void SetBrightnessEn(u8 u8Id, bool bEn)
{
    if(u8Id == 0)
    {
        W2BYTEMSK(REG_DISP_TOP_OP2_30_L, bEn ? 0x0100 : 0x0000, 0x0100);

    }
    else
    {
        W2BYTEMSK(REG_DISP_TOP_OP2_3C_L, bEn ? 0x0100 : 0x0000, 0x0100);
    }
}


void SetBrightness(u8 u8Id, u8 u8Brightness)
{
    if(u8Id == 0)
    {
        W2BYTEMSK(REG_DISP_TOP_OP2_3A_L, ((u16)u8Brightness) << 0, 0x00FF);
        W2BYTEMSK(REG_DISP_TOP_OP2_3A_L, ((u16)u8Brightness) << 8, 0xFF00);
        W2BYTEMSK(REG_DISP_TOP_OP2_3B_L, ((u16)u8Brightness) << 0, 0x00FF);
    }
    else
    {
        W2BYTEMSK(REG_DISP_TOP_OP2_46_L, ((u16)u8Brightness) << 0, 0x00FF);
        W2BYTEMSK(REG_DISP_TOP_OP2_46_L, ((u16)u8Brightness) << 8, 0xFF00);
        W2BYTEMSK(REG_DISP_TOP_OP2_47_L, ((u16)u8Brightness) << 0, 0x00FF);
    }
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalDispColorInitVar(void)
{
    // For video
    u8 i;

    for(i=0; i<2; i++)
    {
        g_tColorCfg[i].u8VideoRCon = 0x80;
        g_tColorCfg[i].u8VideoGCon = 0x80;
        g_tColorCfg[i].u8VideoBCon = 0x80;
        g_tColorCfg[i].u8VideoContrast = 0x80;
        g_tColorCfg[i].u8VideoSaturation = 0x80;
#if( ENABLE_CBCR )
        g_tColorCfg[i].u8VideoCb = 0x80;
        g_tColorCfg[i].u8VideoCr = 0x80;
#endif
        g_tColorCfg[i].u8VideoHue = 50;

        g_tColorCfg[i].u8Brightness = 0x80;

        g_tColorCfg[i].sVideoSatHueMatrix[0][0] = g_tColorCfg[i].sVideoSatHueMatrix[1][1] = g_tColorCfg[i].sVideoSatHueMatrix[2][2] = 1024;
        g_tColorCfg[i].sVideoSatHueMatrix[0][1] = g_tColorCfg[i].sVideoSatHueMatrix[1][0] = g_tColorCfg[i].sVideoSatHueMatrix[2][0] =
        g_tColorCfg[i].sVideoSatHueMatrix[0][2] = g_tColorCfg[i].sVideoSatHueMatrix[1][2] = g_tColorCfg[i].sVideoSatHueMatrix[2][1] = 0;

        g_tColorCfg[i].sVideoContrastMatrix[0][0] = g_tColorCfg[i].sVideoContrastMatrix[1][1] = g_tColorCfg[i].sVideoContrastMatrix[2][2] = 1024;
        g_tColorCfg[i].sVideoContrastMatrix[0][1] = g_tColorCfg[i].sVideoContrastMatrix[1][0] = g_tColorCfg[i].sVideoContrastMatrix[2][0] =
        g_tColorCfg[i].sVideoContrastMatrix[0][2] = g_tColorCfg[i].sVideoContrastMatrix[1][2] = g_tColorCfg[i].sVideoContrastMatrix[2][1] = 0;

        g_tColorCfg[i].enMatrixType = E_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS;
        g_tColorCfg[i].sYuvToRGBMatrix[0][0] = g_tColorCfg[i].sYuvToRGBMatrix[1][1] = g_tColorCfg[i].sYuvToRGBMatrix[2][2] = 1024;
        g_tColorCfg[i].sYuvToRGBMatrix[0][1] = g_tColorCfg[i].sYuvToRGBMatrix[1][0] = g_tColorCfg[i].sYuvToRGBMatrix[2][0] =
        g_tColorCfg[i].sYuvToRGBMatrix[0][2] = g_tColorCfg[i].sYuvToRGBMatrix[1][2] = g_tColorCfg[i].sYuvToRGBMatrix[2][1] = 0;

        g_tColorCfg[i].sColorCorrrectMatrix[0][0] = g_tColorCfg[i].sColorCorrrectMatrix[1][1] = g_tColorCfg[i].sColorCorrrectMatrix[2][2] = 1024;
        g_tColorCfg[i].sColorCorrrectMatrix[0][1] = g_tColorCfg[i].sColorCorrrectMatrix[1][0] = g_tColorCfg[i].sColorCorrrectMatrix[2][0] =
        g_tColorCfg[i].sColorCorrrectMatrix[0][2] = g_tColorCfg[i].sColorCorrrectMatrix[1][2] = g_tColorCfg[i].sColorCorrrectMatrix[2][1] = 0;

        // For PC
        g_tColorCfg[i].u8PCRCon = 0x80;
        g_tColorCfg[i].u8PCGCon = 0x80;
        g_tColorCfg[i].u8PCBCon = 0x80;
        g_tColorCfg[i].u8PCContrast = 0x80;

        g_tColorCfg[i].sPCConRGBMatrix[0][0] = g_tColorCfg[i].sPCConRGBMatrix[1][1] = g_tColorCfg[i].sPCConRGBMatrix[2][2] = 1024;
        g_tColorCfg[i].sPCConRGBMatrix[0][1] = g_tColorCfg[i].sPCConRGBMatrix[1][0] = g_tColorCfg[i].sPCConRGBMatrix[2][0] =
        g_tColorCfg[i].sPCConRGBMatrix[0][2] = g_tColorCfg[i].sPCConRGBMatrix[1][2] = g_tColorCfg[i].sPCConRGBMatrix[2][1] = 0;

        // PC sRGB matrix
        g_tColorCfg[i].tSrgbMatrix = NULL;

        // Color correction matrix for Video
        g_tColorCfg[i].bColorCorrectMatrixUpdate = 0;

        g_tColorCfg[i].u8ACEConfig = 0;
    }
}


void HalDispColorSetColorCorrectMatrix(u8 u8Id, s16 *psColorCorrectMatrix)
{
    u8 i, j;

    if(psColorCorrectMatrix)
    {
        for(i=0; i<3; i++)
        {
            for(j=0; j<3; j++)
            {
                g_tColorCfg[u8Id].sColorCorrrectMatrix[i][j] = *(psColorCorrectMatrix +(i*3)+j);
            }
        }

        g_tColorCfg[u8Id].bColorCorrectMatrixUpdate = 1;
    }

    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, \n %d %d %d \n %d %d %d \n %d %d %d \n",
        __FUNCTION__, __LINE__, u8Id,
        g_tColorCfg[u8Id].sColorCorrrectMatrix[0][0], g_tColorCfg[u8Id].sColorCorrrectMatrix[0][1], g_tColorCfg[u8Id].sColorCorrrectMatrix[0][2],
        g_tColorCfg[u8Id].sColorCorrrectMatrix[1][0], g_tColorCfg[u8Id].sColorCorrrectMatrix[1][1], g_tColorCfg[u8Id].sColorCorrrectMatrix[1][2],
        g_tColorCfg[u8Id].sColorCorrrectMatrix[2][0], g_tColorCfg[u8Id].sColorCorrrectMatrix[2][1], g_tColorCfg[u8Id].sColorCorrrectMatrix[2][2]);
}

void HalDispColorSeletYuvToRgbMatrix(u8 u8Id, HalDispColorYuvToRgbMatrixType_e enType, s16 *psYuv2RgbMatrix)
{
    s16 *psMatrix = NULL;
    u8 i,j;

    if(enType == E_DISP_COLOR_YUV_2_RGB_MATRIX_USER)
    {
        psMatrix = psYuv2RgbMatrix;
    }
    else if(enType == E_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV)
    {
        psMatrix = &tSDTVYuv2rgb[0][0];
    }
    else if(enType == E_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS)
    {
        psMatrix = &tByPassYuv2rgb[0][0];
    }
    else  if(enType == E_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV)
    {
        psMatrix = &tHDTVYuv2rgb[0][0];
    }
    else
    {
        DISP_ERR("%s %d, MatrixType(%d) is not correct\n", __FUNCTION__, __LINE__, enType);
        return;
    }

    if(psMatrix)
    {
        g_tColorCfg[u8Id].enMatrixType = enType;

        for(i=0; i<3; i++)
        {
            for(j=0; j<3; j++)
            {
                g_tColorCfg[u8Id].sYuvToRGBMatrix[i][j] = *(psMatrix +(i*3)+j);
            }
        }
    }

    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, \n %d %d %d \n %d %d %d \n %d %d %d \n",
        __FUNCTION__, __LINE__, u8Id,
        g_tColorCfg[u8Id].sYuvToRGBMatrix[0][0], g_tColorCfg[u8Id].sYuvToRGBMatrix[0][1], g_tColorCfg[u8Id].sYuvToRGBMatrix[0][2],
        g_tColorCfg[u8Id].sYuvToRGBMatrix[1][0], g_tColorCfg[u8Id].sYuvToRGBMatrix[1][1], g_tColorCfg[u8Id].sYuvToRGBMatrix[1][2],
        g_tColorCfg[u8Id].sYuvToRGBMatrix[2][0], g_tColorCfg[u8Id].sYuvToRGBMatrix[2][1], g_tColorCfg[u8Id].sYuvToRGBMatrix[2][2]);
}

void HalDispColorAdjustBrightness( u8 u8Id, u8 u8Brightness)
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Brightness=%d\n", __FUNCTION__, __LINE__, u8Id, u8Brightness);
    g_tColorCfg[u8Id].u8Brightness = u8Brightness;

    SetBrightnessEn(u8Id, 1);
    SetBrightness(u8Id, u8Brightness);
}

void HalDispColorAdjustHCS( u8 u8Id, u8 u8Hue, u8  u8Saturation, u8  u8Contrast )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Hue=%d, Sat=%d, Con=%d\n", __FUNCTION__, __LINE__, u8Id, u8Hue, u8Saturation, u8Contrast);

    g_tColorCfg[u8Id].u8VideoSaturation = u8Saturation;
    g_tColorCfg[u8Id].u8VideoContrast = u8Contrast;
    g_tColorCfg[u8Id].u8VideoHue = u8Hue;
    SetVideoSatHueMatrix(u8Id);
    SetVideoContrastMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
}

void HalDispColorAdjustVideoContrast( u8 u8Id, u8 u8Contrast )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Con=%d\n", __FUNCTION__, __LINE__, u8Id, u8Contrast);

    g_tColorCfg[u8Id].u8VideoContrast = u8Contrast;
    SetVideoContrastMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
}

void HalDispColorAdjustVideoSaturation( u8 u8Id, u8 u8Saturation )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Sat=%d\n", __FUNCTION__, __LINE__, u8Id, u8Saturation);

    g_tColorCfg[u8Id].u8VideoSaturation = u8Saturation;
    SetVideoSatHueMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
}

void HalDispColorAdjustVideoCbCr( u8 u8Id, u8 u8Cb, u8 u8Cr )
{
#if( ENABLE_CBCR )

    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Cb=%d, Cr=%d\n", __FUNCTION__, __LINE__, u8Id, u8Cb, u8Cr);

    g_tColorCfg[u8Id].u8VideoCb = u8Cb;
    g_tColorCfg[u8Id].u8VideoCr = u8Cr;
    SetVideoSatHueMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
#endif
}

void HalDispColorAdjustVideoHue( u8 u8Id, u8 u8Hue )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, Hue=%d\n", __FUNCTION__, __LINE__, u8Id, u8Hue);

    g_tColorCfg[u8Id].u8VideoHue = u8Hue;
    SetVideoSatHueMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
}

void HalDispColorAdjustVideoRGB( u8 u8Id, u8 u8RCon, u8 u8GCon, u8 u8BCon)
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, R=%d G=%d B=%d\n",
        __FUNCTION__, __LINE__, u8Id, u8RCon, u8GCon ,u8BCon);
    g_tColorCfg[u8Id].u8VideoRCon = u8RCon;
    g_tColorCfg[u8Id].u8VideoGCon = u8GCon;
    g_tColorCfg[u8Id].u8VideoBCon = u8BCon;
    SetVideoContrastMatrix(u8Id);
    SetVideoColorMatrix(u8Id);
}



void HalDispColorAdjustPCContrastRGB( u8 u8Id, u8 u8Contrast, u8 u8Red, u8 u8Green, u8 u8Blue )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d, R=%d G=%d B=%d\n",
        __FUNCTION__, __LINE__, u8Id, u8Red, u8Green ,u8Blue);

    g_tColorCfg[u8Id].u8PCContrast = u8Contrast;
    g_tColorCfg[u8Id].u8PCRCon = u8Red;
    g_tColorCfg[u8Id].u8PCGCon = u8Green;
    g_tColorCfg[u8Id].u8PCBCon = u8Blue;
    SetPCConRGBMatrix(u8Id);
    SetPCColorMatrix(u8Id);
}

void HalDispColorAdjustPCContrast( u8 u8Id, u8 u8Contrast )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d Con=%d\n",
        __FUNCTION__, __LINE__, u8Id, u8Contrast);

    HalDispColorAdjustPCContrastRGB( u8Id, u8Contrast, g_tColorCfg[u8Id].u8PCRCon, g_tColorCfg[u8Id].u8PCGCon, g_tColorCfg[u8Id].u8PCBCon );
}

void HalDispColorAdjustPCRGB( u8 u8Id, u8 u8Red, u8 u8Green, u8 u8Blue )
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d Id:%d R=%d G=%d B=%d\n",
        __FUNCTION__, __LINE__, u8Id, u8Red, u8Green ,u8Blue);

    HalDispColorAdjustPCContrastRGB( u8Id, g_tColorCfg[u8Id].u8PCContrast, u8Red, u8Green, u8Blue );
}


void HalDispColorSetYUV2RGBCtrl( u8 u8Id, bool bEnable )
{
    if( bEnable )
    {
        g_tColorCfg[u8Id].u8ACEConfig |= ACE_ENABLE_PC_YUV_TO_RGB;
    }
    else
    {
        g_tColorCfg[u8Id].u8ACEConfig &= ~ACE_ENABLE_PC_YUV_TO_RGB;
    }
}

void HalDispColorSetPCsRGBCtrl( u8 u8Id, bool bEnable )
{
    if( bEnable )
    {
        g_tColorCfg[u8Id].u8ACEConfig |= ACE_ENABLE_PC_SRGB;
    }
    else
    {
        g_tColorCfg[u8Id].u8ACEConfig &= ~ACE_ENABLE_PC_SRGB;
    }
}
void HalDispColorSetHue256StepCtrl( u8 u8Id, bool bEnable )
{
    if( bEnable )
    {
        g_tColorCfg[u8Id].u8ACEConfig |= ACE_ENABLE_HUE_256_STEP;
    }
    else
    {
        g_tColorCfg[u8Id].u8ACEConfig &= ~ACE_ENABLE_HUE_256_STEP;
    }
}


