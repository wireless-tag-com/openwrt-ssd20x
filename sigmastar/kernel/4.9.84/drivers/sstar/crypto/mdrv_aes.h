/*
* mdrv_aes.h- Sigmastar
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

#ifndef _MSTAR_AES_H_
#define _MSTAR_AES_H_

/* driver logic flags */
#define AES_MODE_ECB 0
#define AES_MODE_CBC 1
#define AES_MODE_CTR 2
#define AES_DIR_DECRYPT 0
#define AES_DIR_ENCRYPT 1
#define AESDMA_ALLOC_MEMSIZE (16) //1MB  1024*1024
#define AESDMA_ALLOC_MEMSIZE_TEMP (16) //64byte for SHA

struct infinity_aes_op
{
	void *src;
	void *dst;//16
	u32 mode;
	u32 dir;
	u32 flags;
    u32 keylen;//24
	int len;
    u8 *iv;//8
	u8 key[AES_KEYSIZE_256];
	union {
		struct crypto_blkcipher *blk;
		struct crypto_cipher *cip;
	} fallback;
    u16 engine;

};


struct aesdma_alloc_dmem
{
    dma_addr_t  aesdma_phy_addr ;
    dma_addr_t  aesdma_phy_SHABuf_addr;
    const char* DMEM_AES_ENG_INPUT;
    const char* DMEM_AES_ENG_SHABUF;
    u8 *aesdma_vir_addr;
    u8 *aesdma_vir_SHABuf_addr;
};

extern struct platform_device *psg_mdrv_aesdma;

typedef enum
{
    E_MSOS_PRIORITY,            ///< Priority-order suspension
    E_MSOS_FIFO,                ///< FIFO-order suspension
} MsOSAttribute;

int infinity_aes_crypt_pub(struct infinity_aes_op *op, u32 in_addr, u32 out_addr);
u32 infinity_random(void);

#endif
