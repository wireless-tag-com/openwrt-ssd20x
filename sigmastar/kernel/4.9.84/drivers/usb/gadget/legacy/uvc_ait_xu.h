#ifndef _UVC_AIT_XU_H_
#define _UVC_AIT_XU_H_

#include <linux/usb/video.h>
#include "u_uvc.h"

/*************************************************************/
/*															 */
/*						  MACROS							 */
/*															 */
/*************************************************************/
/* ----------------------
* AIT XU descriptor GUID.
*/
#define USE_AIT_XU

#define UUID_LE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)        \
{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
   (b) & 0xff, ((b) >> 8) & 0xff,                   \
   (c) & 0xff, ((c) >> 8) & 0xff,                   \
   (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }

#define UUID_BE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)        \
{ ((a) >> 24) & 0xff, ((a) >> 16) & 0xff, ((a) >> 8) & 0xff, (a) & 0xff, \
   ((b) >> 8) & 0xff, (b) & 0xff,                   \
   ((c) >> 8) & 0xff, (c) & 0xff,                   \
   (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }


#define UVC_AIT_EU1_GUID	UUID_LE(0x23E49ED0, 0x1178, 0x4f31, 0xAE, 0x52, 0xD2, 0xFB, 0x8A, 0x8D, 0x3B, 0x48)
#define UVC_CUS_EU2_GUID	UUID_LE(0x2C49D16A, 0x32B8, 0x4485, 0x3E, 0xA8, 0x64, 0x3A, 0x15, 0x23, 0x62, 0xF2)

#endif//_UVC_AIT_XU_H_
