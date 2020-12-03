/*
* aes-sha1.h- Sigmastar
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

int aes_sha1_ctx_init(struct cryptodev_ctx* ctx, int cfd, 
	const uint8_t *key, unsigned int key_size,
	const uint8_t *mac_key, unsigned int mac_key_size);
void aes_sha1_ctx_deinit();

/* Note that encryption assumes that ciphertext has enough size
 * for the tag and padding to be appended. 
 *
 * Only in-place encryption and decryption are supported.
 */
int aes_sha1_encrypt(struct cryptodev_ctx* ctx, const void* iv, 
	const void* auth, size_t auth_size,
	void* plaintext, size_t size);
int aes_sha1_decrypt(struct cryptodev_ctx* ctx, const void* iv, 
	const void* auth, size_t auth_size,
	void* ciphertext, size_t size);

#endif
