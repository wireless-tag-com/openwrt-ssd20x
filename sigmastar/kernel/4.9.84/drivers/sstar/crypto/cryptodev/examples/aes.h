/*
* aes.h- Sigmastar
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
#ifndef AES_H
# define AES_H

#include <stdint.h>

struct cryptodev_ctx {
	int cfd;
	struct session_op sess;
	uint16_t alignmask;
};

#define	AES_BLOCK_SIZE	16

int aes_ctx_init(struct cryptodev_ctx* ctx, int cfd, const uint8_t *key, unsigned int key_size);
void aes_ctx_deinit();
int aes_encrypt(struct cryptodev_ctx* ctx, const void* iv, const void* plaintext, void* ciphertext, size_t size);
int aes_decrypt(struct cryptodev_ctx* ctx, const void* iv, const void* ciphertext, void* plaintext, size_t size);

#endif
