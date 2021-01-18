
#include "mi_common.h"
#include "mi_gfx_datatype.h"
#include "mi_gfx.h"
#include "blitutil.h"
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define min(a, b) ((a < b) ? a : b)
void extractSquareClip_NxN(RECT src, RECT **clipA, int *clipN, int N)
{
    int clipNum = 0;
    RECT *clip = NULL;
    if (min(RECTW(src), RECTH(src)) < N)
    {
        N = min(RECTW(src), RECTH(src));
    }
    int colClipNum = (RECTW(src) + N - 1) / N;
    int rowClipNum = (RECTH(src) + N - 1) / N;
    clipNum = colClipNum * rowClipNum;
    *clipA = calloc(sizeof(RECT), clipNum);
    size_t n = 0;
    clip = *clipA;

    for (size_t i = 0; i < rowClipNum - 1; i++, n++)
    {
        for (size_t j = 0; j < colClipNum - 1; j++, n++)
        {
            clip[n].top = src.top + i * N;
            clip[n].bottom = clip[n].top + N;
            clip[n].left = src.left + j * N;
            clip[n].right = clip[n].left + N;
        }
        clip[n].top = src.top + i * N;
        clip[n].bottom = clip[n].top + N;
        clip[n].left = src.right - N;
        clip[n].right = src.right;
    }
    for (size_t j = 0; j < colClipNum - 1; j++, n++)
    {
        clip[n].top = src.bottom - N;
        clip[n].bottom = src.bottom;
        clip[n].left = src.left + j * N;
        clip[n].right = clip[n].left + N;
    }
    clip[n].top = src.bottom - N;
    clip[n].bottom = src.bottom;
    clip[n].left = src.right - N;
    clip[n].right = src.right;
    *clipN = clipNum;
}

void extractSquareClip_Even(RECT src, RECT **clipA, int *clipN)
{
    int clipNum = 0;
    RECT *clip = NULL;

    if (RECTW(src) > RECTH(src))
    {
        int i = 0;
        clipNum = (RECTW(src) + RECTH(src) - 1) / RECTH(src);
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;

        for (; i < clipNum - 1; i++)
        {
            clip[i].top = src.top;
            clip[i].bottom = src.bottom;
            clip[i].left = src.left + RECTH(src) * i;
            clip[i].right = clip[i].left + RECTH(src);
        }

        clip[i].top = src.top;
        clip[i].bottom = src.bottom;
        clip[i].left = src.right - RECTH(src);
        clip[i].right = src.right;
    }
    else if (RECTW(src) < RECTH(src))
    {
        int i = 0;
        clipNum = (RECTH(src) + RECTW(src) - 1) / RECTW(src);
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;

        for (; i < clipNum - 1; i++)
        {
            clip[i].left = src.left;
            clip[i].right = src.right;
            clip[i].top = src.top + RECTW(src) * i;
            clip[i].bottom = clip[i].top + RECTW(src);
        }

        clip[i].left = src.left;
        clip[i].right = src.right;
        clip[i].top = src.bottom - RECTW(src);
        clip[i].bottom = src.bottom;
    }
    else
    {
        clipNum = 1;
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;
        memcpy(clip, &src, sizeof(RECT));
    }

    *clipN = clipNum;
}
void extractSquareClip_MinArea(RECT src, RECT **clipA, int *clipN)
{
    int clipNum = 0;
    RECT *clip = NULL;
    int i = 0;
    unsigned char last_try = (RECTW(src) * RECTH(src)) < 128 * 128 ? 1 : 0;
    if (RECTW(src) > RECTH(src))
    {
        clipNum = (RECTW(src) + RECTH(src) - 1) / RECTH(src);
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;

        for (; i < clipNum - 1; i++)
        {
            clip[i].top = src.top;
            clip[i].bottom = src.bottom;
            clip[i].left = src.left + RECTH(src) * i;
            clip[i].right = clip[i].left + RECTH(src);
        }
        if (last_try)
        {
            clip[i].top = src.top;
            clip[i].bottom = src.bottom;
            clip[i].left = src.right - RECTH(src);
            clip[i].right = src.right;
        }
    }
    else if (RECTW(src) < RECTH(src))
    {
        clipNum = (RECTH(src) + RECTW(src) - 1) / RECTW(src);
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;

        for (; i < clipNum - 1; i++)
        {
            clip[i].left = src.left;
            clip[i].right = src.right;
            clip[i].top = src.top + RECTW(src) * i;
            clip[i].bottom = clip[i].top + RECTW(src);
        }

        if (last_try)
        {
            clip[i].left = src.left;
            clip[i].right = src.right;
            clip[i].top = src.bottom - RECTW(src);
            clip[i].bottom = src.bottom;
        }
    }
    else
    {
        clipNum = 1;
        *clipA = calloc(sizeof(RECT), clipNum);
        clip = *clipA;
        memcpy(clip, &src, sizeof(RECT));
        *clipN = clipNum;
        return;
    }
    if (!last_try)
    {
        RECT src0;
        RECT *srcClip0 = NULL;
        int clipnum0;
        if (RECTW(src) > RECTH(src))
        {
            src0.top = src.top;
            src0.bottom = src.bottom;
            src0.left = clip[i - 1].right;
            src0.right = src.right;
        }
        else
        {
            src0.top = clip[i - 1].bottom;
            src0.bottom = src.bottom;
            src0.left = src.left;
            src0.right = src.right;
        }
        extractSquareClip_MinArea(src0, &srcClip0, &clipnum0);
        clip = calloc(sizeof(RECT), clipNum - 1 + clipnum0);
        memcpy(clip, *clipA, (clipNum - 1) * sizeof(RECT));
        memcpy(clip + clipNum - 1, srcClip0, clipnum0 * sizeof(RECT));
        free(srcClip0);
        free(*clipA);
        *clipA = clip;
        *clipN = clipNum + clipnum0 - 1;
        return;
    }
    *clipN = clipNum;
}

void round_rect(int depth, RECT *update_rect)
{
    if (depth == 1)
    {
        /* round x to be 8-aligned */
        update_rect->left = update_rect->left & ~0x07;
        update_rect->right = (update_rect->right + 7) & ~0x07;
    }
    else if (depth == 2)
    {
        /* round x to be 4-aligned */
        update_rect->left = update_rect->left & ~0x03;
        update_rect->right = (update_rect->right + 3) & ~0x03;
    }
    else if (depth == 4)
    {
        /* round x to be 2-aligned */
        update_rect->left = update_rect->left & ~0x01;
        update_rect->right = (update_rect->right + 1) & ~0x01;
    }
    else
        return;
}

void _get_dst_rect_cw(RECT *dst_rect, const RECT *src_rect, Surface *pDstSurface)
{
    dst_rect->left = pDstSurface->w - src_rect->bottom;
    dst_rect->top = src_rect->left;
    dst_rect->right = pDstSurface->w - src_rect->top;
    dst_rect->bottom = src_rect->right;
}

void _get_src_rect_cw(const RECT *dst_rect, RECT *src_rect, Surface *pDstSurface)
{
    src_rect->left = dst_rect->top;
    src_rect->top = pDstSurface->w - dst_rect->right;
    src_rect->right = dst_rect->bottom;
    src_rect->bottom = pDstSurface->w - dst_rect->left;
}
void SstarBlitCW(Surface *pSrcSurface, Surface *pDstSurface, RECT *pRect)
{
    RECT src_update = *(RECT *)pRect;
    RECT dst_update;
    int dst_width, dst_height;
    int i;
    MI_U16 u16Fence;

    _get_dst_rect_cw(&dst_update, &src_update, pDstSurface);

    ///* Round the update rectangle.  */
    //round_rect(realfb_info->depth, &dst_update);

    dst_width = RECTW(dst_update);
    dst_height = RECTH(dst_update);

    if (dst_width <= 0 || dst_height <= 0)
        return;

    _get_src_rect_cw(&dst_update, &src_update, pDstSurface);

    int srcClipN = 0, dstClipN = 0;
    RECT *srcClip = NULL, *dstClip = NULL;
    extractSquareClip_MinArea(src_update, &srcClip, &srcClipN);
    //extractSquareClip(dst_update, &dstClip, &dstClipN);
    dstClip = calloc(sizeof(RECT), srcClipN);
    for (int i = 0; i < srcClipN; i++)
    {
        _get_dst_rect_cw(&dstClip[i], &srcClip[i], pDstSurface);
    }
    for (i = 0; i < srcClipN; i++)
    {

        MI_GFX_Surface_t stSrc;
        MI_GFX_Rect_t stSrcRect;
        MI_GFX_Surface_t stDst;
        MI_GFX_Rect_t stDstRect;
        MI_GFX_Opt_t stOpt;

        memset(&stOpt, 0, sizeof(stOpt));
        stDst.phyAddr = pDstSurface->phy_addr;
        stDst.eColorFmt = pDstSurface->eGFXcolorFmt;
        stDst.u32Width = pDstSurface->w;
        stDst.u32Height = pDstSurface->h;
        stDst.u32Stride = pDstSurface->pitch;

        stDstRect.s32Xpos = dstClip[i].left;
        stDstRect.s32Ypos = dstClip[i].top;
        stDstRect.u32Width = RECTW(dstClip[i]);
        stDstRect.u32Height = RECTH(dstClip[i]);

        stSrc.phyAddr = pSrcSurface->phy_addr;
        stSrc.eColorFmt = pSrcSurface->eGFXcolorFmt;

        stSrc.u32Width = pSrcSurface->w;
        stSrc.u32Height = pSrcSurface->h;
        stSrc.u32Stride = pSrcSurface->pitch;

        stSrcRect.s32Xpos = srcClip[i].left;
        stSrcRect.s32Ypos = srcClip[i].top;
        stSrcRect.u32Width = RECTW(srcClip[i]);
        stSrcRect.u32Height = RECTH(srcClip[i]);

        stOpt.stClipRect.s32Xpos = stDstRect.s32Xpos;
        stOpt.stClipRect.s32Ypos = stDstRect.s32Ypos;
        stOpt.stClipRect.u32Width = stDstRect.u32Width;
        stOpt.stClipRect.u32Height = stDstRect.u32Height;

        stOpt.u32GlobalSrcConstColor = 0xFF000000;
        stOpt.u32GlobalDstConstColor = 0xFF000000;
        stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
        stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
        stOpt.eRotate = E_MI_GFX_ROTATE_90;
        //start = clock();

        MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
    }

    MI_GFX_WaitAllDone(FALSE, u16Fence);
    free(srcClip);
    free(dstClip);
}

void _get_dst_rect_ccw(RECT *dst_rect, const RECT *src_rect, Surface *pDstSurface)
{
    dst_rect->left = src_rect->top;
    dst_rect->bottom = pDstSurface->h - src_rect->left;
    dst_rect->right = src_rect->bottom;
    dst_rect->top = pDstSurface->h - src_rect->right;
}

void _get_src_rect_ccw(const RECT *dst_rect, RECT *src_rect, Surface *pDstSurface)
{
    src_rect->left = pDstSurface->h - dst_rect->bottom;
    src_rect->top = dst_rect->left;
    src_rect->right = pDstSurface->h - dst_rect->top;
    src_rect->bottom = dst_rect->right;
}

void SstarBlitCCW(Surface *pSrcSurface, Surface *pDstSurface, RECT *pRect)
{
    RECT src_update = *pRect;
    RECT dst_update;
    int dst_width, dst_height;
    int i;
    MI_U16 u16Fence;

    _get_dst_rect_ccw(&dst_update, &src_update, pDstSurface);

    /* Round the update rectangle.  */
    //round_rect(pDstSurface->BytesPerPixel,&dst_update);

    dst_width = RECTW(dst_update);
    dst_height = RECTH(dst_update);

    if (dst_width <= 0 || dst_height <= 0)
        return;

    _get_src_rect_ccw(&dst_update, &src_update, pDstSurface);

    int srcClipN = 0;
    RECT *srcClip = NULL, *dstClip = NULL;
    extractSquareClip_MinArea(src_update, &srcClip, &srcClipN);
    //extractSquareClip_NxN(src_update, &srcClip, &srcClipN,128);
    dstClip = calloc(sizeof(RECT), srcClipN);

    for (int i = 0; i < srcClipN; i++)
    {
        _get_dst_rect_ccw(&dstClip[i], &srcClip[i], pDstSurface);
    }

    for (i = 0; i < srcClipN; i++)
    {

        MI_GFX_Surface_t stSrc;
        MI_GFX_Rect_t stSrcRect;
        MI_GFX_Surface_t stDst;
        MI_GFX_Rect_t stDstRect;
        MI_GFX_Opt_t stOpt;

        memset(&stOpt, 0, sizeof(stOpt));
        stDst.phyAddr = pDstSurface->phy_addr;
        stDst.eColorFmt = pDstSurface->eGFXcolorFmt;
        stDst.u32Width = pDstSurface->w;
        stDst.u32Height = pDstSurface->h;
        stDst.u32Stride = pDstSurface->pitch;

        stDstRect.s32Xpos = dstClip[i].left;
        stDstRect.s32Ypos = dstClip[i].top;
        stDstRect.u32Width = RECTW(dstClip[i]);
        stDstRect.u32Height = RECTH(dstClip[i]);

        stSrc.phyAddr = pSrcSurface->phy_addr;
        stSrc.eColorFmt = pSrcSurface->eGFXcolorFmt;

        stSrc.u32Width = pSrcSurface->w;
        stSrc.u32Height = pSrcSurface->h;
        stSrc.u32Stride = pSrcSurface->pitch;

        stSrcRect.s32Xpos = srcClip[i].left;
        stSrcRect.s32Ypos = srcClip[i].top;
        stSrcRect.u32Width = RECTW(srcClip[i]);
        stSrcRect.u32Height = RECTH(srcClip[i]);

        stOpt.stClipRect.s32Xpos = stDstRect.s32Xpos;
        stOpt.stClipRect.s32Ypos = stDstRect.s32Ypos;
        stOpt.stClipRect.u32Width = stDstRect.u32Width;
        stOpt.stClipRect.u32Height = stDstRect.u32Height;

        stOpt.u32GlobalSrcConstColor = 0xFF000000;
        stOpt.u32GlobalDstConstColor = 0xFF000000;
        stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
        stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
        stOpt.eRotate = E_MI_GFX_ROTATE_270;
        //start = clock();

        MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
    }

    MI_GFX_WaitAllDone(FALSE, u16Fence);
    free(srcClip);
    free(dstClip);
}


void _get_dst_rect_hvflip(RECT *dst_rect, const RECT *src_rect, Surface *pDstSurface)
{
    dst_rect->left = pDstSurface->w - src_rect->right;
    dst_rect->bottom = pDstSurface->h - src_rect->top;
    dst_rect->right = pDstSurface->w - src_rect->left;
    dst_rect->top = pDstSurface->h - src_rect->bottom;
}

void _get_src_rect_hvflip(const RECT *dst_rect, RECT *src_rect, Surface *pDstSurface)
{
    src_rect->left = pDstSurface->w - dst_rect->right;
    src_rect->top = pDstSurface->h - dst_rect->bottom;
    src_rect->right = pDstSurface->w - dst_rect->left;
    src_rect->bottom = pDstSurface->h - dst_rect->top;
}
void SstarBlitHVFlip(Surface *pSrcSurface, Surface *pDstSurface, RECT *pRect)
{

    RECT dstClip;    
    _get_dst_rect_hvflip(&dstClip,pRect,pSrcSurface);
    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stDst.phyAddr = pDstSurface->phy_addr;
    stDst.eColorFmt = pDstSurface->eGFXcolorFmt;
    stDst.u32Width = pDstSurface->w;
    stDst.u32Height = pDstSurface->h;
    stDst.u32Stride = pDstSurface->pitch;

    stDstRect.s32Xpos = dstClip.left;
    stDstRect.s32Ypos = dstClip.top;
    stDstRect.u32Width = RECTW(dstClip);
    stDstRect.u32Height = RECTH(dstClip);

    stSrc.phyAddr = pSrcSurface->phy_addr;
    stSrc.eColorFmt = pSrcSurface->eGFXcolorFmt;

    stSrc.u32Width = pSrcSurface->w;
    stSrc.u32Height = pSrcSurface->h;
    stSrc.u32Stride = pSrcSurface->pitch;

    stSrcRect.s32Xpos = pRect->left;
    stSrcRect.s32Ypos = pRect->top;
    stSrcRect.u32Width = RECTW(dstClip);
    stSrcRect.u32Height = RECTH(dstClip);

    stOpt.stClipRect.s32Xpos = stDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = stDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = stDstRect.u32Width;
    stOpt.stClipRect.u32Height = stDstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_180;
    //start = clock();

    MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
}

void SstarBlitNormal(Surface *pSrcSurface, Surface *pDstSurface, RECT *pRect)
{
    RECT src_update;
    int width, height;

    //clock_t start,end;
    src_update.left = pRect->left;
    src_update.right = pRect->right;
    src_update.top = pRect->top;
    src_update.bottom = pRect->bottom;

    /* Round the update rectangle.  */
    //round_rect(4, &src_update);

    width = RECTW(src_update);
    height = RECTH(src_update);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    MI_GFX_Surface_t stSrc;
    MI_GFX_Rect_t stSrcRect;
    MI_GFX_Surface_t stDst;
    MI_GFX_Rect_t stDstRect;
    MI_GFX_Opt_t stOpt;
    MI_U16 u16Fence;

    memset(&stOpt, 0, sizeof(stOpt));
    stDst.phyAddr = pDstSurface->phy_addr;
    stDst.eColorFmt = pDstSurface->eGFXcolorFmt;
    stDst.u32Width = pDstSurface->w;
    stDst.u32Height = pDstSurface->h;
    stDst.u32Stride = pDstSurface->pitch;

    stDstRect.s32Xpos = src_update.left;
    stDstRect.s32Ypos = src_update.top;
    stDstRect.u32Width = width;
    stDstRect.u32Height = height;

    stSrc.phyAddr = pSrcSurface->phy_addr;
    stSrc.eColorFmt = pSrcSurface->eGFXcolorFmt;

    stSrc.u32Width = pSrcSurface->w;
    stSrc.u32Height = pSrcSurface->h;
    stSrc.u32Stride = pSrcSurface->pitch;

    stSrcRect.s32Xpos = src_update.left;
    stSrcRect.s32Ypos = src_update.top;
    stSrcRect.u32Width = width;
    stSrcRect.u32Height = height;

    stOpt.stClipRect.s32Xpos = stDstRect.s32Xpos;
    stOpt.stClipRect.s32Ypos = stDstRect.s32Ypos;
    stOpt.stClipRect.u32Width = stDstRect.u32Width;
    stOpt.stClipRect.u32Height = stDstRect.u32Height;

    stOpt.u32GlobalSrcConstColor = 0xFF000000;
    stOpt.u32GlobalDstConstColor = 0xFF000000;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    stOpt.eMirror = E_MI_GFX_MIRROR_NONE;
    stOpt.eRotate = E_MI_GFX_ROTATE_0;
    //start = clock();

    MI_GFX_BitBlit(&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
}
