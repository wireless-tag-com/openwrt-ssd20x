/*
* region_substract.h- Sigmastar
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
#ifndef _REGIONSUBSTRACT_H_
#define _REGIONSUBSTRACT_H_

typedef int BOOL;
typedef unsigned char BYTE;
typedef void *PBLOCKHEAP;

/**
 * A rectangle defined by coordinates of corners.
 *
 * \note The lower-right corner does not belong to the rectangle,
 * i.e. the bottom horizontal line and the right vertical line are excluded
 * from the retangle.
 *
 * \sa PRECT, GAL_Rect
 */
typedef struct _RECT
{
    /**
     * The x coordinate of the upper-left corner of the rectangle.
     */
    int left;
    /**
     * The y coordinate of the upper-left corner of the rectangle.
     */
    int top;
    /**
     * The x coordinate of the lower-right corner of the rectangle.
     */
    int right;
    /**
     * The y coordinate of the lower-right corner of the rectangle.
     */
    int bottom;
} RECT;
/**
 * \var typedef RECT* PRECT
 * \brief Data type of the pointer to a RECT.
 *
 * \sa RECT
 */
typedef RECT* PRECT;

   /**
    * \defgroup region_fns Region operations
    *
    * A Region is simply an area, as the name implies, and is implemented as
    * a "y-x-banded" array of rectangles. To explain: Each Region is made up
    * of a certain number of rectangles sorted by y coordinate first,
    * and then by x coordinate.
    *
    * Furthermore, the rectangles are banded such that every rectangle with a
    * given upper-left y coordinate (y1) will have the same lower-right y
    * coordinate (y2) and vice versa. If a rectangle has scanlines in a band, it
    * will span the entire vertical distance of the band. This means that some
    * areas that could be merged into a taller rectangle will be represented as
    * several shorter rectangles to account for shorter rectangles to its left
    * or right but within its "vertical scope".
    *
    * An added constraint on the rectangles is that they must cover as much
    * horizontal area as possible. E.g. no two rectangles in a band are allowed
    * to touch.
    *
    * Whenever possible, bands will be merged together to cover a greater
    * vertical distance (and thus reduce the number of rectangles). Two bands
    * can be merged only if the bottom of one touches the top of the other and
    * they have rectangles in the same places (of the same width, of course).
    * This maintains the y-x-banding that's so nice to have...
    *
    * Example:
    *
    * \include region.c
    *
    * @{
    */

/**
 * Clipping rectangle structure.
 */
typedef struct _CLIPRECT
{
    /**
     * The clipping rectangle itself.
     */
    RECT rc;
    /**
     * The next clipping rectangle.
     */
    struct _CLIPRECT* next;
    /**
     * The previous clipping rectangle.
     */
    struct _CLIPRECT* prev;
} CLIPRECT;
typedef CLIPRECT* PCLIPRECT;

/* Clipping Region */
#define NULLREGION      0x00
#define SIMPLEREGION    0x01
#define COMPLEXREGION   0x02

/**
 * Clipping region structure, alos used for general regions.
 */
typedef struct _CLIPRGN
{
   /**
    * Type of the region, can be one of the following:
    *   - NULLREGION\n
    *     A null region.
    *   - SIMPLEREGION\n
    *     A simple region.
    *   - COMPLEXREGION\n
    *     A complex region.
    */
    BYTE            type;
   /**
    * Reserved for alignment.
    */
    BYTE            reserved[3];
   /**
    * The bounding rect of the region.
    */
    RECT            rcBound;
   /**
    * Head of the clipping rectangle list.
    */
    PCLIPRECT       head;
   /**
    * Tail of the clipping rectangle list.
    */
    PCLIPRECT       tail;
   /**
    * The private block data heap used to allocate clipping rectangles.
    * \sa BLOCKHEAP
    */
    PBLOCKHEAP      heap;
} CLIPRGN;

/**
 * \var typedef CLIPRGN* PCLIPRGN
 * \brief Data type of the pointer to a CLIPRGN.
 *
 * \sa CLIPRGN
 */
typedef CLIPRGN* PCLIPRGN;

#define region_for_each_rect(rect, rgn) \
    for(rect=((rgn)->head)?(&((rgn)->head->rc)):NULL; \
          rect; \
          rect=(container_of(rect, CLIPRECT, rc)->next)?&(container_of(rect, CLIPRECT, rc)->next->rc):NULL)

void dbg_dumpRegion (CLIPRGN* region);
BOOL IsEmptyClipRgn (const CLIPRGN* pRgn);
void EmptyClipRgn (PCLIPRGN pRgn);
void InitClipRgn (PCLIPRGN pRgn, PBLOCKHEAP heap);
BOOL AddClipRect (PCLIPRGN region, const RECT *rect);
BOOL SubtractRegion (CLIPRGN *rgnD, const CLIPRGN *rgnM, const CLIPRGN *rgnS);
int region_substract_init(void);
int region_substract_deinit(void);

#endif