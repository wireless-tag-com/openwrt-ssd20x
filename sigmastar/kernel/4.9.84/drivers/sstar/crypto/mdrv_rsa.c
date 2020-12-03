/*
* mdrv_rsa.c- Sigmastar
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <../drivers/sstar/include/ms_msys.h>
#include <../drivers/sstar/include/ms_platform.h>
#include <../drivers/sstar/include/mdrv_hvsp_io_st.h>
#include <../drivers/sstar/include/mdrv_hvsp_io.h>
#include <asm/uaccess.h>
#include "mdrv_rsa.h"
#include "halAESDMA.h"
#include <linux/sysctl.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#define RSA_DEBUG_FLAG (0)
#if (RSA_DEBUG_FLAG == 1)
#define RSA_DBG(fmt, arg...) printk(fmt, ##arg)//KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define RSA_DBG(fmt, arg...)
#endif

#define DRIVER_NAME "rsa"
//static unsigned int test_ioctl_major = 0;
//static unsigned int num_of_dev = 1;
//static struct cdev test_ioctl_cdev;
//static int ioctl_num = 0;


//static const char* DMEM_RSA_ENG_INPUT="RSA_ENG_IN";

struct rsaConfig{
	U32 *pu32RSA_Sig;
	U32 *pu32RSA_KeyN;
	U32 *pu32RSA_KeyE;
	U32 *pu32RSA_Output;
	U8 u8RSA_KeyNLen;
	U8 u8RSA_SigLen;
	U8 u8RSA_KeyELen;
	U8 u8RSAPublicKey;
};

int rsa_crypto(struct rsa_config *op)
{
    int out_size = 32, i = 0;
    unsigned long timeout;

    HAL_RSA_Reset();
    msleep(1);
    HAL_RSA_SetKeyLength((op->u32RSA_KeyNLen-1) & 0x3F);
    HAL_RSA_SetKeyType(FALSE, FALSE);

    HAL_RSA_Ind32Ctrl(1); //write
    HAL_RSA_LoadSignInverse((U32 *)op->pu32RSA_Sig,  (U8)op->u32RSA_SigLen);
    HAL_RSA_LoadKeyNInverse((U32 *)op->pu32RSA_KeyN,  (U8)op->u32RSA_KeyNLen);
    HAL_RSA_LoadKeyEInverse((U32 *)op->pu32RSA_KeyE, (U8)op->u32RSA_KeyELen);

    HAL_RSA_ExponetialStart();

#if (RSA_DEBUG_FLAG)
    RSA_DBG("IN:\n");
    for(i = 0; i < op->u32RSA_SigLen; i+=4)
    {
        RSA_DBG(" x%08X x%08X x%08X x%08X\n", *(op->pu32RSA_Sig+i), *(op->pu32RSA_Sig+i+1),*(op->pu32RSA_Sig+i+2), *(op->pu32RSA_Sig+i+3));
    }
    RSA_DBG("\n\n");
#endif

    timeout = jiffies + msecs_to_jiffies(5000);
    while((HAL_RSA_GetStatus() & RSA_STATUS_RSA_DONE) != RSA_STATUS_RSA_DONE)
    {
        if (time_after_eq(jiffies, timeout))
        {
            printk("rsa timeout!!!\n");
            goto err;
        }

    }

    if(op->u32RSA_KeyNLen == 0x40)
    {
        out_size = 64;
    }

    HAL_RSA_Ind32Ctrl(0); //read
    for(i = 0; i < out_size; i++)
    {
        HAL_RSA_SetFileOutAddr(i);
        HAL_RSA_FileOutStart();
        *(op->pu32RSA_Output + (out_size - 1) - i) = HAL_RSA_FileOut();
    }

#if (RSA_DEBUG_FLAG)
    RSA_DBG("OUT:\n");
    for(i = 0; i < out_size; i+=4)
    {
        RSA_DBG(" x%08X x%08X x%08X x%08X\n", *(op->pu32RSA_Output+i), *(op->pu32RSA_Output+i+1),*(op->pu32RSA_Output+i+2), *(op->pu32RSA_Output+i+3));
    }
    RSA_DBG("\n\n");
#endif

    HAL_RSA_FileOutEnd();
err:
    HAL_RSA_ClearInt();
    HAL_RSA_Reset();

    return 0;
}

static long rsa_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct rsaConfig *ioctl_data = filp->private_data;
    int retval = 0;
    struct rsa_config data;
    int nOutSize = 0;
	int i = 0;
	bool bRSAHwKey = FALSE;
    unsigned long timeout;

//	unsigned char* val;
    memset(&data, 0, sizeof(data));

    switch (cmd) {

        case MDrv_RSA_Reset:
            HAL_RSA_Reset();
            goto done;
            break;

        case MDrv_RSA_Setmode:
            memset(&data, 0, sizeof(data));
            if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
                retval = -EFAULT;
                goto done;
            }
//            write_lock(&ioctl_data->lock);
            ioctl_data->u8RSA_KeyNLen = data.u32RSA_KeyNLen;
//            write_unlock(&ioctl_data->lock);
			if (ioctl_data->u8RSA_KeyNLen != 0x10 && ioctl_data->u8RSA_KeyNLen != 0x20 && ioctl_data->u8RSA_KeyNLen != 0x40){
				retval = -EFAULT;
				printk("[RSAERR] Mode error\n");
				printk("[RSAERR] KenNLen != 0x10 or 0x20 or 0x40\n");
                goto done;
			}

            HAL_RSA_SetKeyLength(((ioctl_data->u8RSA_KeyNLen)-1) & 0x3F);
            HAL_RSA_SetKeyType(bRSAHwKey, FALSE);
            break;

        case MDrv_RSA_Calculate:
            memset(&data, 0, sizeof(data));
            if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
                retval = -EFAULT;
                goto done;
            }
//             write_lock(&ioctl_data->lock);
            ioctl_data->pu32RSA_Sig = (U32 *)data.pu32RSA_Sig;
            ioctl_data->pu32RSA_Output = (U32 *)data.pu32RSA_Output;
            ioctl_data->pu32RSA_KeyN = (U32 *)data.pu32RSA_KeyN;
            ioctl_data->pu32RSA_KeyE = (U32 *)data.pu32RSA_KeyE;
            ioctl_data->u8RSA_KeyELen = (U8)(data.u32RSA_KeyELen & 0xFF);
            ioctl_data->u8RSA_KeyNLen = (U8)(data.u32RSA_KeyNLen & 0xFF);
            ioctl_data->u8RSA_SigLen = (U8)(data.u32RSA_SigLen & 0xFF);
//             write_unlock(&ioctl_data->lock);

			if ((ioctl_data->u8RSA_SigLen < 1) || (ioctl_data->u8RSA_SigLen > 0xFF)) {
				 retval = -EFAULT;
				 printk("[RSAERR] SigLen is invalid\n");
                goto done;
			}

			if ((ioctl_data->u8RSA_KeyELen < 1) || (ioctl_data->u8RSA_KeyELen > 0xFF))
			{
				retval = -EFAULT;
				printk("[RSAERR] KeyELen is invalid\n");
                goto done;
			}

			if ((ioctl_data->u8RSA_KeyNLen < 1) || (ioctl_data->u8RSA_KeyNLen > 0xFF))
			{
				retval = -EFAULT;
				printk("[RSAERR] KeyNLen is invalid\n");
                goto done;
			}

			if (!((ioctl_data->pu32RSA_Sig) && (ioctl_data->pu32RSA_Output) && (ioctl_data->pu32RSA_KeyN))) {
				  retval = -EFAULT;
				  printk("[RSAERR] null pointer = 0 \n");
				 goto done;
			}

            HAL_RSA_Ind32Ctrl(1);//1:write

            if((!bRSAHwKey) && (ioctl_data->pu32RSA_Sig))
            {
                HAL_RSA_LoadSignInverse(ioctl_data->pu32RSA_Sig,  ioctl_data->u8RSA_SigLen);
            }
            else
            {
                retval = -EFAULT;
                printk("[RSAERR] RSA_Sig = NULL\n");
                goto done;
            }

            if((!bRSAHwKey) && (ioctl_data->pu32RSA_KeyN))
            {
                HAL_RSA_LoadKeyNInverse(ioctl_data->pu32RSA_KeyN,  ioctl_data->u8RSA_KeyNLen);
            }
            else
            {
                retval = -EFAULT;
                printk("[RSAERR] RSA_KeyN = NULL\n");
                goto done;
            }

            if((!bRSAHwKey) && (ioctl_data->pu32RSA_KeyE))
            {
                HAL_RSA_LoadKeyEInverse(ioctl_data->pu32RSA_KeyE, ioctl_data->u8RSA_KeyELen); //65535
            }
            else
            {
                retval = -EFAULT;
                printk("[RSAERR] RSA_KeyE = NULL\n");
                goto done;
            }

            HAL_RSA_ExponetialStart();

            timeout = jiffies + msecs_to_jiffies(1000);
            while((HAL_RSA_GetStatus() & RSA_STATUS_RSA_DONE) != RSA_STATUS_RSA_DONE)
            {
                if (time_after_eq(jiffies, timeout))
                {
                    printk("rsa timeout!!!\n");
                    break;
                }
            }

            if((bRSAHwKey) || (ioctl_data->u8RSA_KeyNLen == 64))
            {
                nOutSize = 64;
            }
            else
            {
                nOutSize = 32;
            }

            HAL_RSA_Ind32Ctrl(0);

            for( i = 0; i<nOutSize; i++)
            {
                HAL_RSA_SetFileOutAddr(i);
                HAL_RSA_FileOutStart();
                *(ioctl_data->pu32RSA_Output + (nOutSize-1) - i) = HAL_RSA_FileOut();
            }

            HAL_RSA_FileOutEnd();
            HAL_RSA_Reset();

//            read_lock(&ioctl_data->lock);

             data.pu32RSA_Output = (unsigned int*)ioctl_data->pu32RSA_Output;

//            read_unlock(&ioctl_data->lock);

            if (copy_to_user((int __user *)arg, &data, sizeof(data)) ) {
                                retval = -EFAULT;
            	goto done;
			}
            break;

        default:
            retval = -ENOTTY;
    }

    done:

    return retval;
}

//ssize_t test_ioctl_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
//{
//    struct rsaConfig *ioctl_data = filp->private_data;
//    unsigned char val;
//    int retval;
//    int i = 0;

//    read_lock(&ioctl_data->lock);
//    val = ioctl_data->val;
//    read_unlock(&ioctl_data->lock);

//    for (;i < count ;i++) {
//        if (copy_to_user(&buf[i], &val, 1)) {
//        retval = -EFAULT;
//    goto out;
//     }
// }

// retval = count;

//out:
// return retval;
//}

static int rsa_ioctl_close(struct inode *inode, struct file *filp)
{
//    printk(KERN_ALERT "%s call.\n", __func__);
        if (filp->private_data) {
            kfree(filp->private_data);
            filp->private_data = NULL;
        }

 return 0;
}

static int rsa_ioctl_open(struct inode *inode, struct file *filp)
{

    struct rsaConfig *ioctl_data;
//    printk(KERN_ALERT "%s call.\n", __func__);

    ioctl_data = kmalloc(sizeof(struct rsaConfig), GFP_KERNEL);
    if (ioctl_data == NULL)
    return -ENOMEM;

//    rwlock_init(&ioctl_data->lock);


    filp->private_data = ioctl_data;

 return 0;
}

struct file_operations rsa_fops = {
    .owner = THIS_MODULE,
    .open = rsa_ioctl_open,
    .release = rsa_ioctl_close,
    .unlocked_ioctl = rsa_ioctl,
};

struct miscdevice rsadev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rsa",
	.fops = &rsa_fops,
	.mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH,
};

//#define PFX "rsadev: "

//static int  __init rsa_ioctl_init(void)
//{

//int rc;
//	rc = misc_register(&rsadev);
//	if (unlikely(rc)) {
//		pr_err(PFX "registration of /dev/crypto failed\n");
//		return rc;
//	}

//	return 0;
//}

//static void __exit rsa_ioctl_exit(void)
//{
//    misc_deregister(&rsadev);
//}

//module_init(rsa_ioctl_init);
//module_exit(rsa_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("This is test_ioctl module.");
