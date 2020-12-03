/*
* mdrv_sha.c- Sigmastar
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
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/hw_random.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <crypto/scatterwalk.h>
#include <crypto/algapi.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
//#include <asm/vio.h>
#include "ms_platform.h"
#include "ms_msys.h"
#include "halAESDMA.h"
#include <linux/mm.h>

#define LOOP_CNT                        10000
#define INFINITY_SHA_BUFFER_SIZE        1024

#define SHA_DEBUG_FLAG (0)

#if (SHA_DEBUG_FLAG == 1)
#define SHA_DBG(fmt, arg...) printk(KERN_ERR fmt, ##arg)//KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define SHA_DBG(fmt, arg...)
#endif

//memory probe from aes.c
extern struct aesdma_alloc_dmem
{
    dma_addr_t  aesdma_phy_addr ;
    dma_addr_t  aesdma_phy_SHABuf_addr;
    const char* DMEM_AES_ENG_INPUT;
    const char* DMEM_AES_ENG_SHABUF;
    u8 *aesdma_vir_addr;
    u8 *aesdma_vir_SHABuf_addr;
}ALLOC_DMEM;

extern struct platform_device *psg_mdrv_aesdma;
extern void enableClock(void);
extern void disableClock(void);
extern void allocMem(U32 len);
extern void _ms_aes_mem_free(void);

extern void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr );
static  u8 gu8WorkMode=0;

static void* alloc_dmem(const char* name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length=size;
    if(0!=msys_request_dmem(&dmem)){
        return NULL;
    }
    *addr=dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}

void allocTempMem(U32 len)
{
    if (!(ALLOC_DMEM.aesdma_vir_SHABuf_addr = alloc_dmem(ALLOC_DMEM.DMEM_AES_ENG_SHABUF,
                                             len,
                                             &ALLOC_DMEM.aesdma_phy_SHABuf_addr))){
        printk("[input]unable to allocate aesdma memory\n");
    }
    memset(ALLOC_DMEM.aesdma_vir_SHABuf_addr, 0, len);
}

struct infinity_sha256_ctx
{
    u8 digest[SHA256_DIGEST_SIZE];
    u32 u32digest_len;
    u32 u32Bufcnt;
};

static int infinity_sha256_init(struct shash_desc *desc)
{
    struct infinity_sha256_ctx *infinity_ctx = crypto_tfm_ctx(&desc->tfm->base);
    struct sha256_state *sctx = shash_desc_ctx(desc);


    SHA_DBG(" %s %d \n",__FUNCTION__,__LINE__);
 
    HAL_SHA_Reset();

    HAL_SHA_SelMode(1);

    gu8WorkMode = 0;
    infinity_ctx->u32Bufcnt =0;
    memset(sctx, 0, sizeof(struct sha256_state));
    return 0;
}

/*static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}*/

static int infinity_sha256_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);
    struct infinity_sha256_ctx *infinity_ctx = crypto_tfm_ctx(&desc->tfm->base);
    u64 leftover = 0;
    u32 u32InputCopied = 0;
    u32 u32loopCnt;
    //U32 msg_cnt = 0;

    SHA_DBG(" %s %d sctx->count:%llu len:%d\n", __FUNCTION__, __LINE__, sctx->count, len);


    leftover = infinity_ctx->u32Bufcnt + len;
    while(leftover >= SHA256_BLOCK_SIZE)
    {
        SHA_DBG(" %s %d leftover:%llu \n",__FUNCTION__, __LINE__, leftover);

        memcpy(ALLOC_DMEM.aesdma_vir_SHABuf_addr + infinity_ctx->u32Bufcnt, data+u32InputCopied, SHA256_BLOCK_SIZE-infinity_ctx->u32Bufcnt);

        //hexdump(ALLOC_DMEM.aesdma_vir_SHABuf_addr, SHA256_BLOCK_SIZE);

        if (sctx->count)
        {
            SHA_DBG(" ----%s set init vt\n",__FUNCTION__ );
            HAL_SHA_Write_InitValue_BE((U32)sctx->state);
            HAL_SHA_SetInitHashMode(1);
        }
        else
        {
            
            HAL_SHA_SetInitHashMode(0);
        }
        Chip_Flush_MIU_Pipe();
        HAL_SHA_SetAddress(Chip_Phys_to_MIU(ALLOC_DMEM.aesdma_phy_SHABuf_addr));
        HAL_SHA_SetLength(SHA256_BLOCK_SIZE);
        HAL_SHA_ManualMode(1);
        
        HAL_SHA_Start();
        udelay(1);  //sha256 cost about 1~1.4us

        u32loopCnt = 0;
        while(((HAL_SHA_GetStatus() & SHARNG_CTRL_SHA_READY) != SHARNG_CTRL_SHA_READY) )
        {
            u32loopCnt++;

            if(u32loopCnt>LOOP_CNT)
            {
                printk("ERROR!! %s %d %d \n",__FUNCTION__, __LINE__, SHA256_BLOCK_SIZE);
                break;
            }
        }

        HAL_SHA_Out((U32)sctx->state);
        HAL_SHA_Clear();
        u32InputCopied += SHA256_BLOCK_SIZE;
        leftover -= SHA256_BLOCK_SIZE;
        infinity_ctx->u32Bufcnt = 0;
        sctx->count += SHA256_BLOCK_SIZE;

    }

    if (leftover)
    {
        memcpy(ALLOC_DMEM.aesdma_vir_SHABuf_addr+infinity_ctx->u32Bufcnt, data+u32InputCopied, leftover);
        infinity_ctx->u32Bufcnt = leftover;
    }

    return 0;
}

static const u8 padding[64+56] = { 0x80, };

static int infinity_sha256_final(struct shash_desc *desc, u8 *out)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);
    struct infinity_sha256_ctx *infinity_ctx = crypto_tfm_ctx(&desc->tfm->base);
    int index, padlen;
    __be64 bits;

    SHA_DBG(" %s %d %llu \n", __FUNCTION__, __LINE__, sctx->count );
    bits = cpu_to_be64((sctx->count+infinity_ctx->u32Bufcnt) << 3);

    /* Pad out to 56 mod 64 */
    index = infinity_ctx->u32Bufcnt;
    padlen = (index < 56) ? (56 - index) : ((64+56) - index);
    infinity_sha256_update(desc, padding, padlen);

    /* Append length field bytes */
    infinity_sha256_update(desc, (const u8 *)&bits, sizeof(bits));

    //HAL_SHA_Out((U32)sctx->state);
    //hexdump((unsigned char *)sctx->state, SHA256_DIGEST_SIZE);
    memcpy(out, sctx->state, SHA256_DIGEST_SIZE);
    HAL_SHA_Reset();

    return 0;
}

static int infinity_sha256_export(struct shash_desc *desc, void *out)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);
    struct infinity_sha256_ctx *infinity_ctx = crypto_tfm_ctx(&desc->tfm->base);
    struct sha256_state *octx = out;
    SHA_DBG(" %s %d \n",__FUNCTION__,__LINE__);
    octx->count = sctx->count + infinity_ctx->u32digest_len;
    memcpy(octx->buf, sctx->buf, sizeof(octx->buf));
    /* if no data has been processed yet, we need to export SHA256's
     * initial data, in case this context gets imported into a software
     * context */
    if(infinity_ctx->u32digest_len)
    {
        memcpy(octx->state, infinity_ctx->digest, SHA256_DIGEST_SIZE);
    }
    else
    {
        memset(octx->state, 0, SHA256_DIGEST_SIZE);
    }

    return 0;
}

static int infinity_sha256_import(struct shash_desc *desc, const void *in)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);
    struct infinity_sha256_ctx *infinity_ctx = crypto_tfm_ctx(&desc->tfm->base);
    const struct sha256_state *ictx = in;
    SHA_DBG(" %s %d \n",__FUNCTION__,__LINE__);
    memcpy(sctx->buf, ictx->buf, sizeof(ictx->buf));
    sctx->count = ictx->count & 0x3f;
    infinity_ctx->u32digest_len = sctx->count;
    if (infinity_ctx->u32digest_len) {
        memcpy(infinity_ctx->digest, ictx->state, SHA256_DIGEST_SIZE);
    }
    return 0;
}

int infinity_sha_init(u8 sha256_mode)
{
    HAL_SHA_Reset();

    if (sha256_mode)
    {
        HAL_SHA_SelMode(1);
    }
    else
    {
        HAL_SHA_SelMode(0);
    }

    return 0;
}

int infinity_sha_update(u32 *in, u32 len, u32 *state, u32 count, u8 once)
{
    u32 loop = 0;
    U32 msg_cnt = 0;

    SHA_DBG(" %s len %d count %d\n",__FUNCTION__, len, count);
    if (count)
    {
        HAL_SHA_Write_InitValue((U32)state);
        HAL_SHA_WriteWordCnt(count >> 2); // in unit of 4-bytes
        HAL_SHA_SetInitHashMode(1);
    }
    else
    {
        HAL_SHA_SetInitHashMode(0);
    }

    Chip_Flush_MIU_Pipe();
    HAL_SHA_SetAddress((U32)in);
    HAL_SHA_SetLength(len);
    if (once)
    {
        HAL_SHA_ManualMode(0);
    }
    else {
        HAL_SHA_ManualMode(1);
    }

    HAL_SHA_Start();
    udelay(1);  //sha256 cost about 1~1.4us

    while(((HAL_SHA_GetStatus() & SHARNG_CTRL_SHA_READY) != SHARNG_CTRL_SHA_READY) && (loop < LOOP_CNT))
    {
        loop++;
        //usleep_range(20, 80);
    }
    HAL_SHA_ReadOut((U32)state);
    msg_cnt = HAL_SHA_ReadWordCnt() << 2;
    SHA_DBG("msg calculated %d -> %d\n", count, (u32)msg_cnt);
    SHA_DBG("x%x, x%x, x%x, x%x, x%x, x%x, x%x, x%x\n", state[0], state[1], state[2], state[3], state[4], state[5], state[6], state[7]);
    HAL_SHA_Clear();
    return 0;
}

int infinity_sha_final(void)
{
    HAL_SHA_Reset();

    return 0;
}

struct shash_alg infinity_shash_sha256_alg = {
	.digestsize = SHA256_DIGEST_SIZE,
	.init       = infinity_sha256_init,
	.update     = infinity_sha256_update,
	.final      = infinity_sha256_final,
	.export     = infinity_sha256_export,
	.import     = infinity_sha256_import,
	.descsize   = sizeof(struct sha256_state),
	.statesize  = sizeof(struct sha256_state),
	.base       = {
		.cra_name        = "sha256",
		.cra_driver_name = "sha256-infinity",
		.cra_priority    = 00,
		.cra_flags       = CRYPTO_ALG_TYPE_SHASH |
						CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize   = SHA256_BLOCK_SIZE,
		.cra_module      = THIS_MODULE,
		.cra_ctxsize     = sizeof(struct infinity_sha256_ctx),
	}
};



int infinity_sha_create(void)
{
    int ret = -1;

    SHA_DBG(" %s %d \n",__FUNCTION__,__LINE__);

    allocTempMem(4096);

    ret = crypto_register_shash(&infinity_shash_sha256_alg);

    return ret;
}

int infinity_sha_destroy(void)
{
    _ms_aes_mem_free();

//    crypto_unregister_alg(&infinity_shash_sha256_alg);
    return 0;
}
