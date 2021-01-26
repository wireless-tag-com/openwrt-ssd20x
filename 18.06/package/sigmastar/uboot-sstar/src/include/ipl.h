#ifndef _IPL_H
#define _IPL_H

#define IPL_HEADER_CHAR     (0x4C5049)
#define IPL__HEADER_CHAR    (0x5F4C5049)
#define IPLC_HEADER_CHAR    (0x434C5049)
#define IPLK_HEADER_CHAR    (0x4E4C5049)

#define IPL_IMAGE_MAGIC_OFFSET                  4
#define IPL_IMAGE_SIZE_OFFSET                   8
#define IPL_IMAGE_AUTH_OFFSET                   10
#define IPL_IMAGE_CID_OFFSET                    11
#define IPL_IMAGE_CHECKSUM_OFFSET               12
#define IPL_IMAGE_DATA_OFFSET                   16
#define IPL_CUST_IMAGE_KEYN_OFFSET              16
#define IPL_CUST_IMAGE_KEYAES_OFFSET            18

#define image_get_ipl_data(a)                   (U32) (a + IPL_IMAGE_DATA_OFFSET)
#define image_get_ipl_magic(a)                  (*((volatile U32*) (a + IPL_IMAGE_MAGIC_OFFSET)))
#define image_get_ipl_size(a)                   (*((volatile U16*) (a + IPL_IMAGE_SIZE_OFFSET)))
#define image_get_ipl_auth(a)                   (*((volatile U8*) (a + IPL_IMAGE_AUTH_OFFSET)))
#define image_get_ipl_cid(a)                    (*((volatile U8*) (a + IPL_IMAGE_CID_OFFSET)))
#define image_get_ipl_checksum(a)               (*((volatile U32*) (a + IPL_IMAGE_CHECKSUM_OFFSET)))


#define image_get_ipl_cust_keyn(a)              (*((volatile U16*) (a + IPL_CUST_IMAGE_KEYN_OFFSET)))
#define image_get_ipl_cust_keyaes(a)            (*((volatile U16*) (a + IPL_CUST_IMAGE_KEYAES_OFFSET)))
#define image_get_ipl_cust_keyn_data(a)         (U32) (a + image_get_ipl_size(a) - image_get_ipl_cust_keyaes(a) - image_get_ipl_cust_keyn(a))
#define image_get_ipl_cust_keyaes_data(a)       (U32) (image_get_ipl_cust_keyn_data + image_get_ipl_cust_keyn(a))


#endif

