#ifndef _IPL_RTK_H
#define _IPL_RTK_H

typedef struct rtk_image_header {
    U32     ivt[8];     // Interrupt Vector Table, 32byte
    U32     marker;     // RTK header, 0x5F4B5452 (“RTK_”)
    U32     size;       // Image size (include header)
    U32     checksum;  // Checksum (exclude header)
    U32     reserved;  // Reserved
} __attribute__((packed)) rtk_image_header_t;

#define RTK_MAKER                   0x5F4B5452
#define RTK_IMAGE_HEADER_OFFSET     0
#define RTK_IMAGE_HEADER_IVT        0
#define RTK_IMAGE_HEADER_MAKER      32
#define RTK_IMAGE_HEADER_SIZE       36
#define RTK_IMAGE_HEADER_CHECKSUM   40

#define image_get_rtk_header_size()     (sizeof(rtk_image_header_t))
#define image_get_rtk_header(a)         ((rtk_image_header_t) (a + RTK_IMAGE_HEADER_OFFSET))
#define image_get_rtk_maker(a)          (*((volatile U32*) (a + RTK_IMAGE_HEADER_OFFSET + RTK_IMAGE_HEADER_MAKER)))
#define image_get_rtk_size(a)           (*((volatile U32*) (a + RTK_IMAGE_HEADER_OFFSET + RTK_IMAGE_HEADER_SIZE)))

#endif

