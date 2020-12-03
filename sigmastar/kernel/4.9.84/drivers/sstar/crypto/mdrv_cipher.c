/*
* mdrv_cipher.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <crypto/aes.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <ms_platform.h>
#include <ms_msys.h>
#include "mdrv_aes.h"
#include "mdrv_rsa.h"
#include "mdrv_cipher.h"

/*
 * Options
 */
#define CIPHER_DEBUG (1)
#if (CIPHER_DEBUG == 1)
#define CIPHER_DBG(fmt, arg...) printk(KERN_ALERT fmt, ##arg)//KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define CIPHER_DBG(fmt, arg...)
#endif

/*
 * Data types
 */
typedef enum
{
    E_AES_DIR_DECRYPT = 0,
    E_AES_DIR_ENCRYPT,
} AES_DIR;

/*
 * External
 */
extern int infinity_sha_init(u8 sha256_mode);
extern int infinity_sha_update(u32 *in, u32 len, u32 *state, u32 count, u8 once);
extern int infinity_sha_final(void);

/*
 * Local
 */
static struct mutex _mtcipher_lock;
static u8 _u8AesRefcnt = 0;

/********************************************************/
/*                                                      */
/* AES functions                                        */
/*                                                      */
/********************************************************/
int cipher_aes_init(void)
{
    if (_u8AesRefcnt == 0)
    {
        mutex_init(&_mtcipher_lock);
        _u8AesRefcnt++;
    }
    return 0;
}

EXPORT_SYMBOL(cipher_aes_init);

int cipher_aes_uninit(void)
{
    if (_u8AesRefcnt)
    {
        _u8AesRefcnt--;
        if (_u8AesRefcnt == 0)
        {
            mutex_destroy(&_mtcipher_lock);
        }
    }
    return 0;
}

EXPORT_SYMBOL(cipher_aes_uninit);

static int cipher_aes_crypto(MDRV_AES_HANDLE *handle, AES_DIR dir)
{
    int ret = 0;
    u32 in, out;
    struct infinity_aes_op op;

    in = (u32)__pa(handle->in);
    out = (u32)__pa(handle->out);
    op.len = handle->len;
    op.dir = (dir == E_AES_DIR_DECRYPT) ? AES_DIR_DECRYPT : AES_DIR_ENCRYPT;
    op.iv = handle->iv;
    memcpy(op.key, handle->key, handle->keylen);
    op.keylen = handle->keylen;
    switch(handle->mode) {
    case E_AES_ALGO_ECB:
        op.mode = AES_MODE_ECB;
        break;
    case E_AES_ALGO_CBC:
        op.mode = AES_MODE_CBC;
        break;
    case E_AES_ALGO_CTR:
        op.mode = AES_MODE_CTR;
        break;
    }
    Chip_Flush_Cache_Range((unsigned long)handle->in, handle->len);
    Chip_Inv_Cache_Range((unsigned long)handle->out, handle->len);
    ret = infinity_aes_crypt_pub(&op, in, out);
    if (ret != handle->len)
    {
        CIPHER_DBG("aes crypto failed %d\n", ret);
    }
    return ret;
}

/* [ecb] no iv */
static int cipher_ecb_crypto(MDRV_AES_HANDLE *handle, AES_DIR dir)
{
    int ret = 0;
    u32 left = handle->len;

    while (left)
    {
        handle->len = left - (left % AES_BLOCK_SIZE);
        ret = cipher_aes_crypto(handle, dir);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] ecb crypto err %d\n", ret);
            return ret;
        }
        left -= ret;
    }
    return 0;
}

/* [cbc] encrypt */
static int cipher_cbc_encrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;
    u32 left = handle->len;

    while (left)
    {
        handle->len = left - (left % AES_BLOCK_SIZE);
        ret = cipher_aes_crypto(handle, E_AES_DIR_ENCRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] cbc encrypt err %d\n", ret);
            return ret;
        }
        else if (ret > 0) {
            memcpy(handle->iv, (handle->out+(handle->len)-16), 16);
            left -= ret;
        }
    }
    return 0;
}

/* [cbc] decrypt */
static int cipher_cbc_decrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;
    u32 left = handle->len;
    u8 temp_iv[AES_BLOCK_SIZE];

    while (left)
    {
        handle->len = left - (left % AES_BLOCK_SIZE);
        memcpy(temp_iv, handle->in+(handle->len)-16, 16);
        ret = cipher_aes_crypto(handle, E_AES_DIR_DECRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] cbc decrypt err %d\n", ret);
            return ret;
        }
        else if (ret > 0)
        {
            memcpy(handle->iv, temp_iv, 16);
            left -= ret;
        }
    }
    return 0;
}

/* [ctr] decrypt = encrypt */
static int cipher_ctr_crypto(MDRV_AES_HANDLE *handle)
{
    int ret = 0;
    int counter = 0, n = 0;
    u32 left = handle->len;
    u32 temp_iv[AES_KEYSIZE_128 >> 2];

    while (left)
    {
        handle->len = left - (left % AES_BLOCK_SIZE);
        ret = cipher_aes_crypto(handle, E_AES_DIR_ENCRYPT);
        if (ret < 0)
        {
            CIPHER_DBG("[AES] ctr crypto err %d\n", ret);
            return ret;
        }
        else if (ret > 0)
        {
            memcpy(temp_iv, handle->iv, AES_BLOCK_SIZE);
            for (n = 0; n <= 3; n++)
            {
                temp_iv[n] = be32_to_cpu(temp_iv[n]);
            }
            counter = handle->len >> 4;
            // iv=iv+1;
            while(counter)
            {
                temp_iv[3] = temp_iv[3] + 1;
                if (temp_iv[3] == 0x0)
                {
                    temp_iv[2] = temp_iv[2] + 1;
                    if (temp_iv[2] == 0x0)
                    {
                        temp_iv[1] = temp_iv[1] + 1;
                        if (temp_iv[1] == 0x0)
                        {
                            temp_iv[0] = temp_iv[0] + 1;
                            if (temp_iv[0] == 0x0)
                            {
                                CIPHER_DBG("IV counter overflow!\n");
                            }
                        }
                    }
                }
                counter--;
            }
            for (n = 0; n <= 3; n++)
            {
                temp_iv[n] = cpu_to_be32(temp_iv[n]);
            }
            memcpy(handle->iv, temp_iv, AES_BLOCK_SIZE);
        }
        left -= ret;
    }
    return 0;
}

int cipher_aes_encrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;

    mutex_lock(&_mtcipher_lock);
    switch(handle->mode) {
    case E_AES_ALGO_ECB:
        ret = cipher_ecb_crypto(handle, AES_DIR_ENCRYPT);
        break;
    case E_AES_ALGO_CBC:
        ret = cipher_cbc_encrypt(handle);
        break;
    case E_AES_ALGO_CTR:
        ret = cipher_ctr_crypto(handle);
        break;
    }
    mutex_unlock(&_mtcipher_lock);
    return ret;
}

EXPORT_SYMBOL(cipher_aes_encrypt);

int cipher_aes_decrypt(MDRV_AES_HANDLE *handle)
{
    int ret = 0;

    mutex_lock(&_mtcipher_lock);
    switch(handle->mode) {
    case E_AES_ALGO_ECB:
        ret = cipher_ecb_crypto(handle, AES_DIR_DECRYPT);
        break;
    case E_AES_ALGO_CBC:
        ret = cipher_cbc_decrypt(handle);
        break;
    case E_AES_ALGO_CTR:
        ret = cipher_ctr_crypto(handle);
        break;
    }
    mutex_unlock(&_mtcipher_lock);
    return ret;
}

EXPORT_SYMBOL(cipher_aes_decrypt);

/********************************************************/
/*                                                      */
/* RSA functions                                        */
/*                                                      */
/********************************************************/
int cipher_rsa_crypto(MDRV_RSA_HANDLE *handle)
{
    struct rsa_config op;

    if (!handle->exp || !handle->modulus || !handle->in || !handle->out)
    {
        CIPHER_DBG("[RSA] invalid input\n");
        return -1;
    }
    if ((handle->mod_len != 0x10) && (handle->mod_len != 0x20) && (handle->mod_len != 0x40)) {
        CIPHER_DBG("[RSA] KenNLen %d unsupported\n", handle->mod_len);
        return -1;
    }

    op.pu32RSA_KeyE   = handle->exp;
    op.pu32RSA_KeyN   = handle->modulus;
    op.u32RSA_KeyELen = handle->exp_len;
    op.u32RSA_KeyNLen = handle->mod_len;
    op.pu32RSA_Output = handle->out;
    op.pu32RSA_Sig    = handle->in;
    op.u32RSA_SigLen  = handle->len;
    return rsa_crypto(&op);
}

EXPORT_SYMBOL(cipher_rsa_crypto);

/********************************************************/
/*                                                      */
/* SHA functions                                        */
/*                                                      */
/********************************************************/
int cipher_sha_init(MDRV_SHA_HANDLE *handle)
{
    u8 sha256 = 1;

    if ((handle->mode < E_SHA_1) && (handle->mode >= E_SHA_MODE_NUM))
    {
        CIPHER_DBG("[SHA] Unsupported mode\n");
        return -1;
    }

    handle->ctx.count = 0;
    if ((handle->mode == E_SHA_1) || (handle->mode == E_SHA_1_ONCE))
    {
        sha256 = 0;
    }
    return infinity_sha_init(sha256);
}

EXPORT_SYMBOL(cipher_sha_init);

int cipher_sha_update(MDRV_SHA_HANDLE *handle)
{
    int ret;
    u8 once = 0; // do SHA for whole message once

    if ((handle->mode == E_SHA_1_ONCE) || (handle->mode == E_SHA_256_ONCE))
    {
        once = 1;
    }
    ret = infinity_sha_update((u32 *)handle->u32DataPhy, handle->u32DataLen, handle->ctx.state, handle->ctx.count, once);
    if (ret == 0)
    {
        handle->ctx.count += handle->u32DataLen;
    }

    return ret;
}

EXPORT_SYMBOL(cipher_sha_update);

int cipher_sha_final(MDRV_SHA_HANDLE *handle)
{
    u8 i = 0, count = 32;
    u8 *hash = (u8 *)handle->u32ShaVal;
    u8 *state = (u8 *)handle->ctx.state;

    if ((handle->mode == E_SHA_1) || (handle->mode == E_SHA_1_ONCE))
    {
        count = 20;
    }
    for(i = 0; i < count; i++)
    {
        hash[i] = state[31 - i];
    }
    return infinity_sha_final();
}

EXPORT_SYMBOL(cipher_sha_final);

u32 cipher_random_num(void)
{
    return infinity_random();
}

EXPORT_SYMBOL(cipher_random_num);
