/*
* region_substract.c- Sigmastar
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

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/hashtable.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <asm/string.h>

#include "region_substract.h"

#undef DBG_INFO
#undef DBG_ERR
#define DBG_INFO(...)
#define DBG_ERR(...)

/*  1 if two RECTs overlap.
 *  0 if two RECTs do not overlap.
 */
#define EXTENTCHECK(r1, r2) \
        ((r1)->right > (r2)->left && \
         (r1)->left < (r2)->right && \
         (r1)->bottom > (r2)->top && \
         (r1)->top < (r2)->bottom)


/*
 * Allocate a new clipping rect and add it to the region.
 */
#define NEWCLIPRECT(region, rect) \
       {\
            rect = ClipRectAlloc(region->heap);\
            rect->next = NULL;\
            rect->prev = region->tail;\
            if (region->tail)\
                region->tail->next = rect;\
            region->tail = rect;\
            if (region->head == NULL)\
                region->head = rect;\
       }

/**
 * \def CopyRegion
 * \brief Is an alias of \a ClipRgnCopy
 * \sa ClipRgnCopy
 */
#define CopyRegion              ClipRgnCopy

/* MAX/MIN/ABS macors */
/**
 * \def MAX(x, y)
 * \brief A macro returns the maximum of \a x and \a y.
 */
#ifndef MAX
#define MAX(x, y)           (((x) > (y))?(x):(y))
#endif
/**
 * \def MIN(x, y)
 * \brief A macro returns the minimum of \a x and \a y.
 */
#ifndef MIN
#define MIN(x, y)           (((x) < (y))?(x):(y))
#endif
/**
 * \def ABS(x)
 * \brief A macro returns the absolute value of \a x.
 */
#ifndef ABS
#define ABS(x)              (((x)<0) ? -(x) : (x))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


typedef void (*voidProcp1)(CLIPRGN *region, const CLIPRECT *r1, const CLIPRECT *r1End,
                          const CLIPRECT *r2, const CLIPRECT *r2End, int top, int bottom);
typedef void (*voidProcp2)(CLIPRGN *region, const CLIPRECT *r, const CLIPRECT *rEnd,
                          int top, int bottom);

static struct kmem_cache *cliprect_cache=NULL;


void* ClipRectAlloc (PBLOCKHEAP heap)
{
    return kmem_cache_alloc(cliprect_cache,GFP_KERNEL);
}
void FreeClipRect (PBLOCKHEAP heap, void* data)
{
    kmem_cache_free(cliprect_cache,data);
}

/**
 * \fn void SetRectEmpty (RECT* prc)
 * \brief Empties a rectangle.
 *
 *
 * \param prc The pointer to the rectangle.
 *
 * \sa SetRect
 */
static inline void SetRectEmpty (RECT* prc)
{
    (prc)->left = (prc)->top = (prc)->right = (prc)->bottom = 0;
}

BOOL IsEmptyClipRgn (const CLIPRGN* pRgn)
{
     if (pRgn->head == NULL)
         return TRUE;

     return FALSE;
}
EXPORT_SYMBOL(IsEmptyClipRgn);
BOOL IsRectEmpty (const RECT* prc)
{
    if( prc->left == prc->right ) return TRUE;
    if( prc->top == prc->bottom ) return TRUE;
    return FALSE;
}

void EmptyClipRgn (PCLIPRGN pRgn)
{
    PCLIPRECT pCRect, pTemp;

    pCRect = pRgn->head;
    while (pCRect) {
        pTemp = pCRect->next;
        FreeClipRect (pRgn->heap, pCRect);
        pCRect = pTemp;
    }

    pRgn->type = NULLREGION;
    SetRectEmpty (&pRgn->rcBound);
    pRgn->head = NULL;
    pRgn->tail = NULL;
}
EXPORT_SYMBOL(EmptyClipRgn);

/* Init a region */
void InitClipRgn (PCLIPRGN pRgn, PBLOCKHEAP heap)
{
     pRgn->type = NULLREGION;
     SetRectEmpty (&pRgn->rcBound);
     pRgn->head = NULL;
     pRgn->tail = NULL;

     pRgn->heap = heap;   // read-only field.
}
EXPORT_SYMBOL(InitClipRgn);
/* Reset a region */
BOOL SetClipRgn (PCLIPRGN pRgn, const RECT* pRect)
{
    PCLIPRECT pClipRect;

    if (IsRectEmpty (pRect))
        return FALSE;

    // empty rgn first
    EmptyClipRgn (pRgn);

    // get a new clip rect from free list
    pClipRect = ClipRectAlloc (pRgn->heap);
    if (pClipRect == NULL)
        return FALSE;

    // set clip rect
    pClipRect->rc = *pRect;
    pClipRect->next = NULL;
    pClipRect->prev = NULL;

    pRgn->type = SIMPLEREGION;
    pRgn->head = pRgn->tail = pClipRect;
    pRgn->rcBound = *pRect;

    return TRUE;
}

/* Re-calculate the rcBound of a region */
static void REGION_SetExtents (CLIPRGN *region)
{
    PCLIPRECT cliprect;
    RECT *pExtents;

    if (region->head == NULL) {
        region->rcBound.left = 0; region->rcBound.top = 0;
        region->rcBound.right = 0; region->rcBound.bottom = 0;
        return;
    }

    pExtents = &region->rcBound;

    /*
     * Since head is the first rectangle in the region, it must have the
     * smallest top and since tail is the last rectangle in the region,
     * it must have the largest bottom, because of banding. Initialize left and
     * right from head and tail, resp., as good things to initialize them
     * to...
     */
    pExtents->left = region->head->rc.left;
    pExtents->top = region->head->rc.top;
    pExtents->right = region->tail->rc.right;
    pExtents->bottom = region->tail->rc.bottom;

    cliprect = region->head;
    while (cliprect) {
        if (cliprect->rc.left < pExtents->left)
            pExtents->left = cliprect->rc.left;
        if (cliprect->rc.right > pExtents->right)
            pExtents->right = cliprect->rc.right;

        cliprect = cliprect->next;
    }
}

BOOL ClipRgnCopy (PCLIPRGN pDstRgn, const CLIPRGN* pSrcRgn)
{
    PCLIPRECT pcr;
    PCLIPRECT pnewcr, prev;

    // return false if the destination region is not an empty one.
    if (pDstRgn == pSrcRgn)
        return FALSE;

    EmptyClipRgn (pDstRgn);
    if (!(pcr = pSrcRgn->head))
        return TRUE;

    pnewcr = ClipRectAlloc (pDstRgn->heap);

    pDstRgn->head = pnewcr;
    pnewcr->rc = pcr->rc;

    prev = NULL;
    while (pcr->next) {

        pnewcr->next = ClipRectAlloc (pDstRgn->heap);
        pnewcr->prev = prev;

        prev = pnewcr;
        pcr = pcr->next;
        pnewcr = pnewcr->next;

        pnewcr->rc = pcr->rc;
    }

    pnewcr->prev = prev;
    pnewcr->next = NULL;
    pDstRgn->tail = pnewcr;

    pDstRgn->type = pSrcRgn->type;
    pDstRgn->rcBound = pSrcRgn->rcBound;

    return TRUE;
}

/***********************************************************************
 *           REGION_Coalesce
 *
 *      Attempt to merge the rects in the current band with those in the
 *      previous one. Used only by REGION_RegionOp.
 *
 * Results:
 *      The new index for the previous band.
 *
 * Side Effects:
 *      If coalescing takes place:
 *          - rectangles in the previous band will have their bottom fields
 *            altered.
 *          - some clipping rect will be deleted.
 *
 */
static CLIPRECT* REGION_Coalesce (
             CLIPRGN *region,      /* Region to coalesce */
             CLIPRECT *prevStart,  /* start of previous band */
             CLIPRECT *curStart    /* start of current band */
) {
    CLIPRECT *newStart;         /* Start of new band */
    CLIPRECT *pPrevRect;        /* Current rect in previous band */
    CLIPRECT *pCurRect;         /* Current rect in current band */
    CLIPRECT *temp;             /* Temporary clipping rect */
    int curNumRects;            /* Number of rectangles in current band */
    int prevNumRects;           /* Number of rectangles in previous band */
    int bandtop;                /* top coordinate for current band */

    if (prevStart == NULL) prevStart = region->head;
    if (curStart == NULL) curStart = region->head;

    if (prevStart == curStart)
        return prevStart;

    newStart = pCurRect = curStart;

    pPrevRect = prevStart;
    temp = prevStart;
    prevNumRects = 0;
    while (temp != curStart) {
        prevNumRects ++;
        temp = temp->next;
    }

    /*
     * Figure out how many rectangles are in the current band. Have to do
     * this because multiple bands could have been added in REGION_RegionOp
     * at the end when one region has been exhausted.
     */
    pCurRect = curStart;
    bandtop = pCurRect->rc.top;
    curNumRects = 0;
    while (pCurRect && (pCurRect->rc.top == bandtop)) {
        curNumRects ++;
        pCurRect = pCurRect->next;
    }

    if (pCurRect) {
        /*
         * If more than one band was added, we have to find the start
         * of the last band added so the next coalescing job can start
         * at the right place... (given when multiple bands are added,
         * this may be pointless -- see above).
         */
        temp = region->tail;
        while (temp->prev->rc.top == temp->rc.top) {
            temp = temp->prev;
        }
        newStart = temp;
    }

    if ((curNumRects == prevNumRects) && (curNumRects != 0)) {
        pCurRect = curStart;
        /*
         * The bands may only be coalesced if the bottom of the previous
         * matches the top scanline of the current.
         */
        if (pPrevRect->rc.bottom == pCurRect->rc.top) {
            /*
             * Make sure the bands have rects in the same places. This
             * assumes that rects have been added in such a way that they
             * cover the most area possible. I.e. two rects in a band must
             * have some horizontal space between them.
             */
            do {
                if ((pPrevRect->rc.left != pCurRect->rc.left) ||
                    (pPrevRect->rc.right != pCurRect->rc.right))
                {
                    /*
                     * The bands don't line up so they can't be coalesced.
                     */
                    return newStart;
                }
                pPrevRect = pPrevRect->next;
                pCurRect = pCurRect->next;
            } while (--prevNumRects);

            /*
             * If only one band was added to the region, we have to backup
             * newStart to the start of the previous band.
             */
            if (pCurRect == NULL) {
                newStart = prevStart;
            }

            /*
             * The bands may be merged, so set the bottom of each rect
             * in the previous band to that of the corresponding rect in
             * the current band.
             */
            pCurRect = curStart;
            pPrevRect = prevStart;
            do {
                pPrevRect->rc.bottom = pCurRect->rc.bottom;
                pPrevRect = pPrevRect->next;

                if (pCurRect->next)
                    pCurRect->next->prev = pCurRect->prev;
                else
                    region->tail = pCurRect->prev;
                if (pCurRect->prev)
                    pCurRect->prev->next = pCurRect->next;
                else
                    region->head = pCurRect->next;

                temp = pCurRect->next;
                FreeClipRect (region->heap, pCurRect);
                pCurRect = temp;
            } while (--curNumRects);

        }
    }
    return (newStart);
}

/***********************************************************************
 *           REGION_RegionOp
 *
 *      Apply an operation to two regions. Called by Union,
 *      Xor, Subtract, Intersect...
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      The new region is overwritten.
 *
 * Notes:
 *      The idea behind this function is to view the two regions as sets.
 *      Together they cover a rectangle of area that this function divides
 *      into horizontal bands where points are covered only by one region
 *      or by both. For the first case, the nonOverlapFunc is called with
 *      each the band and the band's upper and lower rcBound. For the
 *      second, the overlapFunc is called to process the entire band. It
 *      is responsible for clipping the rectangles in the band, though
 *      this function provides the boundaries.
 *      At the end of each band, the new region is coalesced, if possible,
 *      to reduce the number of rectangles in the region.
 *
 */
static void
REGION_RegionOp(
            CLIPRGN *newReg, /* Place to store result */
            const CLIPRGN *reg1,   /* First region in operation */
            const CLIPRGN *reg2,   /* 2nd region in operation */
            voidProcp1 overlapFunc,     /* Function to call for over-lapping bands */
            voidProcp2 nonOverlap1Func, /* Function to call for non-overlapping bands in region 1 */
            voidProcp2 nonOverlap2Func  /* Function to call for non-overlapping bands in region 2 */
) {
    CLIPRGN my_dst;
    CLIPRGN* pdst;
    const CLIPRECT *r1;                 /* Pointer into first region */
    const CLIPRECT *r2;                 /* Pointer into 2d region */
    const CLIPRECT *r1BandEnd;          /* End of current band in r1 */
    const CLIPRECT *r2BandEnd;          /* End of current band in r2 */
    int ybot;                           /* Bottom of intersection */
    int ytop;                           /* Top of intersection */
    CLIPRECT* prevBand;                 /* start of previous band in newReg */
    CLIPRECT* curBand;                  /* start of current band in newReg */
    int top;                            /* Top of non-overlapping band */
    int bot;                            /* Bottom of non-overlapping band */

    /*
     * Initialization:
     *  set r1, r2, r1End and r2End appropriately, preserve the important
     * parts of the destination region until the end in case it's one of
     * the two source regions, then mark the "new" region empty, allocating
     * another array of rectangles for it to use.
     */
    r1 = reg1->head;
    r2 = reg2->head;


    if (newReg == reg1 || newReg == reg2) {
        InitClipRgn (&my_dst, newReg->heap);
        pdst = &my_dst;
    }
    else {
        EmptyClipRgn (newReg);
        pdst = newReg;
    }


    /*
     * Initialize ybot and ytop.
     * In the upcoming loop, ybot and ytop serve different functions depending
     * on whether the band being handled is an overlapping or non-overlapping
     * band.
     *  In the case of a non-overlapping band (only one of the regions
     * has points in the band), ybot is the bottom of the most recent
     * intersection and thus clips the top of the rectangles in that band.
     * ytop is the top of the next intersection between the two regions and
     * serves to clip the bottom of the rectangles in the current band.
     *  For an overlapping band (where the two regions intersect), ytop clips
     * the top of the rectangles of both regions and ybot clips the bottoms.
     */
    if (reg1->rcBound.top < reg2->rcBound.top)
        ybot = reg1->rcBound.top;
    else
        ybot = reg2->rcBound.top;

    /*
     * prevBand serves to mark the start of the previous band so rectangles
     * can be coalesced into larger rectangles. qv. miCoalesce, above.
     * In the beginning, there is no previous band, so prevBand == curBand
     * (curBand is set later on, of course, but the first band will always
     * start at index 0). prevBand and curBand must be indices because of
     * the possible expansion, and resultant moving, of the new region's
     * array of rectangles.
     */
    prevBand = pdst->head;

    do {
        curBand = pdst->tail;

        /*
         * This algorithm proceeds one source-band (as opposed to a
         * destination band, which is determined by where the two regions
         * intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
         * rectangle after the last one in the current band for their
         * respective regions.
         */
        r1BandEnd = r1;
        while (r1BandEnd && (r1BandEnd->rc.top == r1->rc.top))
            r1BandEnd = r1BandEnd->next;

        r2BandEnd = r2;
        while (r2BandEnd && (r2BandEnd->rc.top == r2->rc.top))
            r2BandEnd = r2BandEnd->next;

        /*
         * First handle the band that doesn't intersect, if any.
         *
         * Note that attention is restricted to one band in the
         * non-intersecting region at once, so if a region has n
         * bands between the current position and the next place it overlaps
         * the other, this entire loop will be passed through n times.
         */
        if (r1->rc.top < r2->rc.top) {
            top = MAX (r1->rc.top, ybot);
            bot = MIN (r1->rc.bottom, r2->rc.top);

            if ((top != bot) && (nonOverlap1Func != NULL))
                (* nonOverlap1Func) (pdst, r1, r1BandEnd, top, bot);

            ytop = r2->rc.top;
        }
        else if (r2->rc.top < r1->rc.top) {
            top = MAX (r2->rc.top, ybot);
            bot = MIN (r2->rc.bottom, r1->rc.top);

            if ((top != bot) && (nonOverlap2Func != NULL))
                (* nonOverlap2Func) (pdst, r2, r2BandEnd, top, bot);

            ytop = r1->rc.top;
        }
        else {
            ytop = r1->rc.top;
        }

        /*
         * If any rectangles got added to the region, try and coalesce them
         * with rectangles from the previous band. Note we could just do
         * this test in miCoalesce, but some machines incur a not
         * inconsiderable cost for function calls, so...
         */
        if (pdst->tail != curBand) {
            if(curBand)curBand=curBand->next;
            prevBand = REGION_Coalesce (pdst, prevBand, curBand);
        }

        /*
         * Now see if we've hit an intersecting band. The two bands only
         * intersect if ybot > ytop
         */
        ybot = MIN (r1->rc.bottom, r2->rc.bottom);
        curBand = pdst->tail;
        if (ybot > ytop)
            (* overlapFunc) (pdst, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);

        if (pdst->tail != curBand) {
            if(curBand)curBand=curBand->next;
            prevBand = REGION_Coalesce (pdst, prevBand, curBand);
        }

        /*
         * If we've finished with a band (bottom == ybot) we skip forward
         * in the region to the next band.
         */
        if (r1->rc.bottom == ybot)
            r1 = r1BandEnd;
        if (r2->rc.bottom == ybot)
            r2 = r2BandEnd;
    } while (r1 && r2);

    /*
     * Deal with whichever region still has rectangles left.
     */
    curBand = pdst->tail;
    if (r1) {
        if (nonOverlap1Func != NULL) {
            do {
                r1BandEnd = r1;
                while ((r1BandEnd) && (r1BandEnd->rc.top == r1->rc.top)) {
                    r1BandEnd = r1BandEnd->next;
                }
                (* nonOverlap1Func) (pdst, r1, r1BandEnd,
                                     MAX (r1->rc.top, ybot), r1->rc.bottom);
                r1 = r1BandEnd;
            } while (r1);
        }
    }
    else if ((r2) && (nonOverlap2Func != NULL))
    {
        do {
            r2BandEnd = r2;
            while ((r2BandEnd) && (r2BandEnd->rc.top == r2->rc.top)) {
                 r2BandEnd = r2BandEnd->next;
            }
            (* nonOverlap2Func) (pdst, r2, r2BandEnd,
                                MAX (r2->rc.top, ybot), r2->rc.bottom);
            r2 = r2BandEnd;
        } while (r2);
    }

    if (pdst->tail != curBand) {
        if(curBand)curBand=curBand->next;
        (void) REGION_Coalesce (pdst, prevBand, curBand);
    }

    /*
     * A bit of cleanup. To keep regions from growing without bound,
     * we shrink the array of rectangles to match the new number of
     * rectangles in the region. This never goes to 0, however...
     *
     * Only do this stuff if the number of rectangles allocated is more than
     * twice the number of rectangles in the region (a simple optimization...).
     */

    if (pdst != newReg) {
        EmptyClipRgn (newReg);
        *newReg = my_dst;
    }
}

/***********************************************************************
 *             REGION_SubtractNonO1
 *
 *      Deal with non-overlapping band for subtraction. Any parts from
 *      region 2 we discard. Anything from region 1 we add to the region.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      region may be affected.
 *
 */
static void
REGION_SubtractNonO1 (CLIPRGN *region, const CLIPRECT *r, const CLIPRECT *rEnd,
                        int top, int bottom)
{
    CLIPRECT *newcliprect;

    while (r && r != rEnd) {
        NEWCLIPRECT(region, newcliprect);
        newcliprect->rc.left = r->rc.left;
        newcliprect->rc.top = top;
        newcliprect->rc.right = r->rc.right;
        newcliprect->rc.bottom = bottom;
        r = r->next;
    }
}


/***********************************************************************
 *             REGION_SubtractO
 *
 *      Overlapping band subtraction. x1 is the left-most point not yet
 *      checked.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      region may have rectangles added to it.
 *
 */
static void
REGION_SubtractO (CLIPRGN *region, const CLIPRECT *r1, const CLIPRECT *r1End,
                        const CLIPRECT *r2, const CLIPRECT *r2End, int top, int bottom)
{
    CLIPRECT *newcliprect;
    int left;

    left = r1->rc.left;
    while (r1 && r2 && (r1 != r1End) && (r2 != r2End)) {
        if (r2->rc.right <= left) {
            /*
             * Subtrahend missed the boat: go to next subtrahend.
             */
            r2 = r2->next;
        }
        else if (r2->rc.left <= left)
        {
            /*
             * Subtrahend preceeds minuend: nuke left edge of minuend.
             */
            left = r2->rc.right;
            if (left >= r1->rc.right)
            {
                /*
                 * Minuend completely covered: advance to next minuend and
                 * reset left fence to edge of new minuend.
                 */
                r1 = r1->next;
                if (r1 != r1End)
                    left = r1->rc.left;
            }
            else
            {
                /*
                 * Subtrahend now used up since it doesn't extend beyond
                 * minuend
                 */
                r2 = r2->next;
            }
        }
        else if (r2->rc.left < r1->rc.right)
        {
            /*
             * Left part of subtrahend covers part of minuend: add uncovered
             * part of minuend to region and skip to next subtrahend.
             */
            NEWCLIPRECT(region, newcliprect);
            newcliprect->rc.left = left;
            newcliprect->rc.top = top;
            newcliprect->rc.right = r2->rc.left;
            newcliprect->rc.bottom = bottom;
            left = r2->rc.right;
            if (left >= r1->rc.right)
            {
                /*
                 * Minuend used up: advance to new...
                 */
                r1 = r1->next;
                if (r1 != r1End)
                    left = r1->rc.left;
            }
            else
            {
                /*
                 * Subtrahend used up
                 */
                r2 = r2->next;
            }
        }
        else
        {
            /*
             * Minuend used up: add any remaining piece before advancing.
             */
            if (r1->rc.right > left)
            {
                NEWCLIPRECT(region, newcliprect);
                newcliprect->rc.left = left;
                newcliprect->rc.top = top;
                newcliprect->rc.right = r1->rc.right;
                newcliprect->rc.bottom = bottom;
            }
            r1 = r1->next;
            if (r1 != r1End)
                left = r1->rc.left;
        }
    }

    /*
     * Add remaining minuend rectangles to region.
     */
    while (r1 && r1 != r1End)
    {
        NEWCLIPRECT(region, newcliprect);
        newcliprect->rc.left = left;
        newcliprect->rc.top = top;
        newcliprect->rc.right = r1->rc.right;
        newcliprect->rc.bottom = bottom;
        r1 = r1->next;
        if (r1 != r1End)
            left = r1->rc.left;
    }
}

/***********************************************************************
 *             SubtractRegion
 *
 *      Subtract rgnS from rgnM and leave the result in rgnD.
 *      S stands for subtrahend, M for minuend and D for difference.
 *
 * Results:
 *      TRUE.
 *
 * Side Effects:
 *      regD is overwritten.
 *
 */
BOOL SubtractRegion (CLIPRGN *rgnD, const CLIPRGN *rgnM, const CLIPRGN *rgnS)
{
    /* check for trivial reject */
    if ( (!(rgnM->head)) || (!(rgnS->head))  ||
            (!EXTENTCHECK (&rgnM->rcBound, &rgnS->rcBound)) ) {
        CopyRegion (rgnD, rgnM);
        return TRUE;
    }

    REGION_RegionOp (rgnD, rgnM, rgnS, REGION_SubtractO,
                REGION_SubtractNonO1, NULL);

    /*
     * Can't alter newReg's rcBound before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the rcBound of those regions being the unaltered. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    REGION_SetExtents (rgnD);
    rgnD->type = (rgnD->head) ? COMPLEXREGION : NULLREGION;

    return TRUE;
}
EXPORT_SYMBOL(SubtractRegion);

/***********************************************************************
 *             REGION_UnionNonO
 *
 *      Handle a non-overlapping band for the union operation. Just
 *      Adds the rectangles into the region. Doesn't have to check for
 *      subsumption or anything.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      region->numRects is incremented and the final rectangles overwritten
 *      with the rectangles we're passed.
 *
 */
static void
REGION_UnionNonO (CLIPRGN *region, const CLIPRECT *r, const CLIPRECT *rEnd, int top, int bottom)
{
    CLIPRECT *newcliprect;

    while (r && r != rEnd) {
        NEWCLIPRECT (region, newcliprect);
        newcliprect->rc.left = r->rc.left;
        newcliprect->rc.top = top;
        newcliprect->rc.right = r->rc.right;
        newcliprect->rc.bottom = bottom;

        r = r->next;
    }
}

/***********************************************************************
 *             REGION_UnionO
 *
 *      Handle an overlapping band for the union operation. Picks the
 *      left-most rectangle each time and merges it into the region.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      Rectangles are overwritten in region->rects and region->numRects will
 *      be changed.
 *
 */
static void
REGION_UnionO(CLIPRGN *region, const CLIPRECT *r1, const CLIPRECT *r1End,
                           const CLIPRECT *r2, const CLIPRECT *r2End, int top, int bottom)
{
    CLIPRECT *newcliprect;

#define MERGERECT(r) \
    if ((region->head) &&  \
        (region->tail->rc.top == top) &&  \
        (region->tail->rc.bottom == bottom) &&  \
        (region->tail->rc.right >= r->rc.left))  \
    {  \
        if (region->tail->rc.right < r->rc.right)  \
        {  \
            region->tail->rc.right = r->rc.right;  \
        }  \
    }  \
    else  \
    {  \
        NEWCLIPRECT(region, newcliprect);  \
        newcliprect->rc.top = top;  \
        newcliprect->rc.bottom = bottom;  \
        newcliprect->rc.left = r->rc.left;  \
        newcliprect->rc.right = r->rc.right;  \
    }  \
    r = r->next;

    while (r1 && r2 && (r1 != r1End) && (r2 != r2End))
    {
        if (r1->rc.left < r2->rc.left)
        {
            MERGERECT(r1);
        }
        else
        {
            MERGERECT(r2);
        }
    }

    if (r1 && r1 != r1End)
    {
        do {
            MERGERECT(r1);
        } while (r1 && r1 != r1End);
    }
    else while (r2 && r2 != r2End)
    {
        MERGERECT(r2);
    }
}


/***********************************************************************
 *             UnionRegion
 */
BOOL UnionRegion (CLIPRGN *dst, const CLIPRGN *src1, const CLIPRGN *src2)
{
    /*  checks all the simple cases */

    /*
     * Region 1 and 2 are the same or region 1 is empty
     */
    if ( (src1 == src2) || (!(src1->head)) ) {
        if (dst != src2)
            CopyRegion (dst, src2);
        return TRUE;
    }

    /*
     * if nothing to union (region 2 empty)
     */
    if (!(src2->head)) {
        if (dst != src1)
            CopyRegion (dst, src1);
        return TRUE;
    }

    /*
     * Region 1 completely subsumes region 2
     */
    if ((src1->head == src1->tail) &&
        (src1->rcBound.left <= src2->rcBound.left) &&
        (src1->rcBound.top <= src2->rcBound.top) &&
        (src1->rcBound.right >= src2->rcBound.right) &&
        (src1->rcBound.bottom >= src2->rcBound.bottom))
    {
        if (dst != src1)
            CopyRegion (dst, src1);
        return TRUE;
    }

    /*
     * Region 2 completely subsumes region 1
     */
    if ((src2->head == src2->tail) &&
        (src2->rcBound.left <= src1->rcBound.left) &&
        (src2->rcBound.top <= src1->rcBound.top) &&
        (src2->rcBound.right >= src1->rcBound.right) &&
        (src2->rcBound.bottom >= src1->rcBound.bottom))
    {
        if (dst != src2)
            CopyRegion(dst, src2);
        return TRUE;
    }

    REGION_RegionOp (dst, src1, src2, REGION_UnionO,
                REGION_UnionNonO, REGION_UnionNonO);

    REGION_SetExtents (dst);
    dst->type = (dst->head) ? COMPLEXREGION : NULLREGION ;
#if 0
    dst->rcBound.left = MIN (src1->rcBound.left, src2->rcBound.left);
    dst->rcBound.top = MIN (src1->rcBound.top, src2->rcBound.top);
    dst->rcBound.right = MAX (src1->rcBound.right, src2->rcBound.right);
    dst->rcBound.bottom = MAX (src1->rcBound.bottom, src2->rcBound.bottom);
    dbg_dumpRegion (dst);
#endif

    return TRUE;
}

/* Adds a rectangle to a region */
BOOL AddClipRect (PCLIPRGN region, const RECT *rect)
{
    CLIPRGN my_region;
    CLIPRECT my_cliprect;

    if (IsRectEmpty (rect))
        return FALSE;

    my_cliprect.rc = *rect;
    my_cliprect.next = NULL;
    my_cliprect.prev = NULL;

    my_region.type = SIMPLEREGION;
    my_region.rcBound = *rect;
    my_region.head = &my_cliprect;
    my_region.tail = &my_cliprect;
    my_region.heap = NULL;

    UnionRegion (region, region, &my_region);

    return TRUE;
}
EXPORT_SYMBOL(AddClipRect);

void dbg_dumpRegion (CLIPRGN* region)
{
    CLIPRECT *cliprect;

    if (!(cliprect = region->head)) {
        DBG_INFO ("region: %p is a null region.\n", region);
    }
    else {
        DBG_INFO ("start of region: %p.\n", region);
        DBG_INFO ("head of region: %p.\n", region->head);
        DBG_INFO ("tail of region: %p.\n", region->tail);
        DBG_INFO ("Bound of region: (%d, %d, %d, %d)\n",
                        region->rcBound.left,
                        region->rcBound.top,
                        region->rcBound.right,
                        region->rcBound.bottom);
        while (cliprect) {
            DBG_INFO ("cliprect %p: (%d, %d, %d, %d)\n", cliprect,
                            cliprect->rc.left, cliprect->rc.top,
                            cliprect->rc.right,
                            cliprect->rc.bottom);

            cliprect = cliprect->next;
        }
        DBG_INFO ("end of region: %p.\n", region);
    }

}
EXPORT_SYMBOL(dbg_dumpRegion);
int region_substract_init(void)
{
    cliprect_cache=kmem_cache_create("cliprect-cache",
                                                            sizeof(CLIPRECT),0,0,NULL);
    if(!cliprect_cache){
        DBG_ERR("cliprect mem cache init fail\n");
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(region_substract_init);
int region_substract_deinit(void)
{
    kmem_cache_destroy(cliprect_cache);
    cliprect_cache=NULL;
    return 0;
}
EXPORT_SYMBOL(region_substract_deinit);

