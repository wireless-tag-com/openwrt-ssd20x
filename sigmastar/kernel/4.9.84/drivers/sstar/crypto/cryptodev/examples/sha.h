/*
* sha.h- Sigmastar
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
#ifndef SHA_H
# define SHA_H

#include <stdint.h>

struct cryptodev_ctx {
	int cfd;
	struct session_op sess;
	uint16_t alignmask;
};

int sha_ctx_init(struct cryptodev_ctx* ctx, int cfd, const uint8_t *key, unsigned int key_size);
void sha_ctx_deinit();
int sha_hash(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest);

#endif
