/*
* jinclude.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: ryan.hsiao <ryan.hsiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/



/* Include auto-config file to find out which system include files we need. */

#include "jconfig.h"		/* auto configuration options */
#define JCONFIG_INCLUDED	/* so that jpeglib.h doesn't do it again */

/*
 * We need the NULL macro and size_t typedef.
 * On an ANSI-conforming system it is sufficient to include <stddef.h>.
 * Otherwise, we get them from <stdlib.h> or <stdio.h>; we may have to
 * pull in <sys/types.h> as well.
 * Note that the core JPEG library does not require <stdio.h>;
 * only the default error handler and data source/destination modules do.
 * But we must pull it in because of the references to FILE in jpeglib.h.
 * You can remove those references if you want to compile without <stdio.h>.
 */

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef NEED_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

/*
 * We need memory copying and zeroing functions, plus strncpy().
 * ANSI and System V implementations declare these in <string.h>.
 * BSD doesn't have the mem() functions, but it does have bcopy()/bzero().
 * Some systems may declare memset and memcpy in <memory.h>.
 *
 * NOTE: we assume the size parameters to these functions are of type size_t.
 * Change the casts in these macros if not!
 */

#ifdef NEED_BSD_STRINGS

#include <strings.h>
#define MEMZERO(target,size)	bzero((void *)(target), (size_t)(size))
#define MEMCOPY(dest,src,size)	bcopy((const void *)(src), (void *)(dest), (size_t)(size))

#else /* not BSD, assume ANSI/SysV string lib */

#include <linux/string.h>
#define MEMZERO(target,size)	memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)	memcpy((void *)(dest), (const void *)(src), (size_t)(size))

#endif


#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>

/*
 * In ANSI C, and indeed any rational implementation, size_t is also the
 * type returned by sizeof().  However, it seems there are some irrational
 * implementations out there, in which sizeof() returns an int even though
 * size_t is defined as long or unsigned long.  To ensure consistent results
 * we always use this SIZEOF() macro in place of using sizeof() directly.
 */

#define SIZEOF(object)	((size_t) sizeof(object))

/*
 * The modules that use fread() and fwrite() always invoke them through
 * these macros.  On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 *
 * Furthermore, macros are provided for fflush() and ferror() in order
 * to facilitate adaption by applications using an own FILE class.
 */

#define FILE        void
#define exit(x)


#if 0
#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#else
#define JFREAD(file,buf,sizeofbuf) 10
#endif

#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFFLUSH(file)	fflush(file)
#define JFERROR(file)	ferror(file)
