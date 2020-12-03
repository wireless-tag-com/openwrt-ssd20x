/*
* vcore_dvfs.c- Sigmastar
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>

//
#include "mdrv_gpio.h"
#include "vcore_dvfs.h"



#define VCORE_DVFS_DEBUG (0)

//
#if VCORE_DVFS_DEBUG
#define VCORE_DVFS_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define VCORE_DVFS_DBG(fmt, arg...)
#endif
#define VCORE_DVFS_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#define vcore_dvfs_success  (0)
#define vcore_dvfs_fail     (-1)

//
DEFINE_MUTEX(vcore_dvfs_mutex_lock);

//#define VCORE_DVFS_MUTEX_INIT(x)    mutex_init(x) // Use DEFINE_MUTEX, so don't need this.
#define VCORE_DVFS_MUTEX_LOCK(x)    mutex_lock(x)
#define VCORE_DVFS_MUTEX_UNLOCK(x)  mutex_unlock(x)

#define IF_NOT_ININTED()        (vcore_dvfs_info.inited != true)
#define IF_IDX_VALID(x)         ((0 <= x) && (x < vcore_dvfs_info.voltages_total_num))
#define IF_SEQ_VALID(x)         (IF_IDX_VALID(x))
#define IF_DEMENDER_VALID(x)    ((0 <= x) && (x < VCORE_DVFS_DEMANDER_TOTAL))



struct rate_voltage {
    unsigned long rate;
    int voltage;
};


//
#define VOLTAGE_BY_INIT_0   (0)
#define VOLTAGE_BY_INIT_1   (-1)

#define RATE_CEVA_PLL_0     (500000000)
#define RATE_CEVA_PLL_1     (600000000)
#define RATE_CEVA_PLL_TOTAL (2)

static struct rate_voltage rate_voltage_ceva_pll[RATE_CEVA_PLL_TOTAL] =
{
    {RATE_CEVA_PLL_0, VOLTAGE_BY_INIT_0},
    {RATE_CEVA_PLL_1, VOLTAGE_BY_INIT_1},
};

static struct rate_voltage *rate_voltage_all_table[VCORE_DVFS_DEMANDER_TOTAL] =
{
    rate_voltage_ceva_pll,
};

static int rate_voltage_num_table[VCORE_DVFS_DEMANDER_TOTAL] =
{
    RATE_CEVA_PLL_TOTAL,
};

//
struct vcore_dvfs_info {
    unsigned int    	gpio_total_num;
    unsigned int    	*gpio_pins;
    int             	*voltages;
    int             	*voltages_sort_idxs;
    unsigned int    	voltages_total_num;
    int             	voltage_current;
    int                 voltage_init;
    int                 voltage_init_seq;
    int             	inited;
};

struct vcore_dvfs {
    vcore_dvfs_demander_e   demander;
    int                     active;
    unsigned long           rate;
    int                     voltage;
    struct rate_voltage     *rate_vol;
    int                     rate_vol_num;
    struct list_head        list;
};



static LIST_HEAD(vcore_dvfs_list);

static struct vcore_dvfs_info vcore_dvfs_info =
{
    .voltage_init       = -1,
    .voltage_init_seq   = -1,
    .inited             = false,
};


static int vcore_dvfs_bubble_sort(int* value, int* idxs, int num);
static int vcore_dvfs_get_curr_idx(int* idx);
static int vcore_dvfs_set_curr_voltage_by_idx(int idx);
static int vcore_dvfs_get_demander_voltage_by_rate(vcore_dvfs_demander_e demander, unsigned long rate);
static int vcore_dvfs_get_idx_by_voltage(int voltage);
static int vcore_dvfs_get_voltage_by_idx(int idx);
static int vcore_dvfs_get_seq_by_idx(int idx);
/* // mask it, just avoid warning.
static int vcore_dvfs_get_idx_by_seq(int seq);
*/
static int vcore_dvfs_init(void);
static int vcore_dvfs_update_voltage(void);

static int vcore_dvfs_bubble_sort(int* value, int* idxs, int num)
{
    int c, d, swap, swap_idx;

    for (c = 0; c < (num - 1); c++)
    {
        for (d = 0; d < (num - c - 1); d++)
        {
            if (value[d] > value[d+1]) // small -> large
            {
                // value
                swap        = value[d];
                value[d]    = value[d+1];
                value[d+1]  = swap;

                // idx
                swap_idx    = idxs[d];
                idxs[d]     = idxs[d+1];
                idxs[d+1]   = swap_idx;
            }
        }
    }

    return vcore_dvfs_success;
}

static int vcore_dvfs_get_curr_idx(int* idx)
{
    int i = 0, idx_i = 0;
    int *gpio_values = NULL;

    gpio_values = kzalloc(sizeof(int) * vcore_dvfs_info.gpio_total_num, GFP_KERNEL);
    if (!gpio_values)
        goto fail;

    for (i = 0; i < vcore_dvfs_info.gpio_total_num; i++)
    {
        gpio_values[i] = MDrv_GPIO_Pad_Read(vcore_dvfs_info.gpio_pins[i]);
        idx_i += (( gpio_values[i] & 1 ) << i );
    }

    if (gpio_values)
    {
        kfree(gpio_values);
        gpio_values = NULL;
    }

    if (IF_IDX_VALID(idx_i) != true)
    {
        goto fail;
    }

    *idx = idx_i;

    return vcore_dvfs_success;

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_get_curr_idx FAIL !\n");
    
    return vcore_dvfs_fail;
}

static int vcore_dvfs_set_curr_voltage_by_idx(int idx)
{
    int i = 0;

    if (IF_IDX_VALID(idx) != true)
    {
        goto fail;
    }

    for (i = 0; i < vcore_dvfs_info.gpio_total_num; i++)
    {
        if ((idx >> i) & 0x1)
        {
            MDrv_GPIO_Set_High(vcore_dvfs_info.gpio_pins[i]);
        }
        else
        {
            MDrv_GPIO_Set_Low(vcore_dvfs_info.gpio_pins[i]);
        }
    }

    return vcore_dvfs_success;

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_set_curr_voltage_by_idx FAIL !\n");

    return vcore_dvfs_fail;
}

static int vcore_dvfs_get_demander_voltage_by_rate(vcore_dvfs_demander_e demander, unsigned long rate)
{
    int j = 0;

    for (j = 0; j < rate_voltage_num_table[demander]; j++)
    {
        if (rate_voltage_all_table[demander][j].rate == rate)
        {
            return rate_voltage_all_table[demander][j].voltage;
        }
    }

    VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_get_demander_voltage_by_rate FAIL !\n");

    return vcore_dvfs_fail;
}

static int vcore_dvfs_get_idx_by_voltage(int voltage)
{
    int i = 0, idx = -1;

    for (i = 0; i < vcore_dvfs_info.voltages_total_num; i++)
    {
        if (voltage == vcore_dvfs_info.voltages[i])
        {
            idx = i;
            break;
        }
    }

    if (IF_IDX_VALID(idx) != true)
    {
        VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_get_idx_by_voltage FAIL !\n");
        
        return vcore_dvfs_fail;
    }

    return idx;
}

static int vcore_dvfs_get_voltage_by_idx(int idx)
{
    if (IF_IDX_VALID(idx) != true)
    {
        VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_get_voltage_by_idx FAIL !\n");
        
        return vcore_dvfs_fail;
    }

    return vcore_dvfs_info.voltages[idx];
}

static int vcore_dvfs_get_seq_by_idx(int idx)
{
    int i = 0, seq = vcore_dvfs_fail;

    for (i = 0; i < vcore_dvfs_info.voltages_total_num; i++)
    {
        if (idx == vcore_dvfs_info.voltages_sort_idxs[i])
        {
            seq = i;
            break;
        }
    }

    if (IF_SEQ_VALID(seq) != true)
    {
        VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_get_seq_by_idx FAIL !\n");
        
        return vcore_dvfs_fail;
    }

    return seq;
}

/* // mask it, just avoid warning.
static int vcore_dvfs_get_idx_by_seq(int seq)
{
    int idx = vcore_dvfs_fail;

    if (IF_SEQ_VALID(seq) != true)
    {
        return vcore_dvfs_fail;
    }

    idx = vcore_dvfs_info.voltages_sort_idxs[seq];

    if (IF_IDX_VALID(idx) != true)
    {
        return vcore_dvfs_fail;
    }

    return idx;
}
*/

static int vcore_dvfs_init(void)
{
    int ret = vcore_dvfs_success;
    struct device_node *np = NULL;
    int i = 0, j = 0, curr_idx = 0;
    int *vol_sort = NULL;

    if ((np = of_find_node_by_name(NULL, "vcore_info")))
    {
        // get gpio_total_num from DTS
        ret = of_property_read_u32(np, "gpio_total_num", &vcore_dvfs_info.gpio_total_num);
        if ( ret || (vcore_dvfs_info.gpio_total_num < 1))
            goto fail;

        // get gpio_pins from DTS
        vcore_dvfs_info.gpio_pins = kzalloc(sizeof(unsigned int) * vcore_dvfs_info.gpio_total_num, GFP_KERNEL);
        if (!vcore_dvfs_info.gpio_pins)
        {
            ret = -ENOMEM;
            goto fail;
        }

        ret = of_property_read_u32_array(np, "gpio_pins", vcore_dvfs_info.gpio_pins, vcore_dvfs_info.gpio_total_num);
        if (ret)
            goto fail;

        // cal voltages_total_num
        vcore_dvfs_info.voltages_total_num = (1 << vcore_dvfs_info.gpio_total_num);

        // get voltages from DTS
        vcore_dvfs_info.voltages = kzalloc(sizeof(int) * vcore_dvfs_info.voltages_total_num, GFP_KERNEL);
        if (!vcore_dvfs_info.voltages)
        {
            ret = -ENOMEM;
            goto fail;
        }

        ret = of_property_read_u32_array(np, "voltages", vcore_dvfs_info.voltages, vcore_dvfs_info.voltages_total_num);
        if (ret)
            goto fail;

        // voltages_sort, voltages_sort_idxs
        vol_sort = kzalloc(sizeof(int) * vcore_dvfs_info.voltages_total_num, GFP_KERNEL);
        if (!vol_sort)
        {
            ret = -ENOMEM;
            goto fail;
        }

        vcore_dvfs_info.voltages_sort_idxs = kzalloc(sizeof(int) * vcore_dvfs_info.voltages_total_num, GFP_KERNEL);
        if (!vcore_dvfs_info.voltages_sort_idxs)
        {
            ret = -ENOMEM;
            goto fail;
        }
        
        for (i = 0; i < vcore_dvfs_info.voltages_total_num; i++)
        {
            vol_sort[i] = vcore_dvfs_info.voltages[i];
            vcore_dvfs_info.voltages_sort_idxs[i] = i;
        }

        // sort
        vcore_dvfs_bubble_sort(vol_sort, vcore_dvfs_info.voltages_sort_idxs, vcore_dvfs_info.voltages_total_num);
        kfree(vol_sort);
        vol_sort = NULL;

        // get vcore init seq from IPL
        if (vcore_dvfs_get_curr_idx(&curr_idx) == vcore_dvfs_success)
        {
            vcore_dvfs_info.voltage_init = vcore_dvfs_get_voltage_by_idx(curr_idx);
            vcore_dvfs_info.voltage_init_seq = vcore_dvfs_get_seq_by_idx(curr_idx);
            vcore_dvfs_info.voltage_current = vcore_dvfs_info.voltage_init;
        }
        else
        {
            goto fail;
        }

        // update table if voltage is decided by init.
        for (i = 0; i < VCORE_DVFS_DEMANDER_TOTAL; i++)
        {
            for (j = 0; j < rate_voltage_num_table[i]; j++)
            {
                if (rate_voltage_all_table[i][j].voltage == VOLTAGE_BY_INIT_0)
                {
                    rate_voltage_all_table[i][j].voltage = vcore_dvfs_info.voltage_init;
                }
                else if (rate_voltage_all_table[i][j].voltage == VOLTAGE_BY_INIT_1)
                {
                    if (IF_SEQ_VALID(vcore_dvfs_info.voltage_init_seq + 1) != true)
                    {
                        goto fail;
                    }

                    rate_voltage_all_table[i][j].voltage = vcore_dvfs_get_voltage_by_idx(vcore_dvfs_info.voltages_sort_idxs[(vcore_dvfs_info.voltage_init_seq + 1)]);
                }
            }
        }
        
        // init done
        vcore_dvfs_info.inited = true;

        VCORE_DVFS_DBG("[VCORE_DVFS]vcore_dvfs_init SUCCESS\n");
    }
    else
    {
        VCORE_DVFS_ERR("[VCORE_DVFS]can't get vcore_info node\n");
        goto fail;
    }

    return vcore_dvfs_success;

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_init FAIL !\n");

    if (vcore_dvfs_info.gpio_pins)
    {
        kfree(vcore_dvfs_info.gpio_pins);
        vcore_dvfs_info.gpio_pins = NULL;
    }

    if (vcore_dvfs_info.voltages)
    {
        kfree(vcore_dvfs_info.voltages);
        vcore_dvfs_info.voltages = NULL;
    }

    if (vol_sort)
    {
        kfree(vol_sort);
        vol_sort = NULL;
    }

    if (vcore_dvfs_info.voltages_sort_idxs)
    {
        kfree(vcore_dvfs_info.voltages_sort_idxs);
        vcore_dvfs_info.voltages_sort_idxs = NULL;
    }

    return vcore_dvfs_fail;
}

static int vcore_dvfs_update_voltage(void)
{
    struct vcore_dvfs *vcore_dvfs_i;
    int max_vol = 0, idx = 0;

    max_vol = vcore_dvfs_info.voltage_init;

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
	    if (vcore_dvfs_i->active == true)
        {
            if (max_vol < vcore_dvfs_i->voltage)
            {
                max_vol = vcore_dvfs_i->voltage;
            }
        }
    }

    idx = vcore_dvfs_get_idx_by_voltage(max_vol);
    if (IF_IDX_VALID(idx) != true)
    {
        goto fail;
    }

    if (vcore_dvfs_set_curr_voltage_by_idx(idx))
    {
        goto fail;
    }

    vcore_dvfs_info.voltage_current = max_vol;

    return vcore_dvfs_success;

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_update_voltage FAIL !\n");

    return vcore_dvfs_fail;
}

int vcore_dvfs_register(vcore_dvfs_demander_e demander, struct vcore_dvfs_init_data *init_data)
{
    struct vcore_dvfs *vcore_dvfs_i;

    VCORE_DVFS_MUTEX_LOCK(&vcore_dvfs_mutex_lock);

    if (IF_NOT_ININTED())
    {
        vcore_dvfs_init();
    }  

    if (IF_DEMENDER_VALID(demander) != true)
    {
        goto fail;
    }

    list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
    {
        if (vcore_dvfs_i->demander == demander)
        {
            goto fail;
        }
    }

    vcore_dvfs_i = NULL;

    vcore_dvfs_i = kzalloc(sizeof(*vcore_dvfs_i), GFP_KERNEL);
    if (!vcore_dvfs_i)
    {
        goto fail;
    }

    vcore_dvfs_i->demander = demander;
    vcore_dvfs_i->rate_vol = rate_voltage_all_table[demander];
    vcore_dvfs_i->rate_vol_num = rate_voltage_num_table[demander];
    vcore_dvfs_i->rate = init_data->rate;

    vcore_dvfs_i->voltage = vcore_dvfs_get_demander_voltage_by_rate(demander, vcore_dvfs_i->rate);
    if ( vcore_dvfs_i->voltage < 0 )
    {
        goto fail;
    }

    vcore_dvfs_i->active = init_data->active;

    list_add(&vcore_dvfs_i->list, &vcore_dvfs_list);

    if (vcore_dvfs_i->active == true)
    {
        vcore_dvfs_update_voltage();
    }
    
    VCORE_DVFS_DBG("[VCORE_DVFS]%s SUCCESS, demander = %d\n", __FUNCTION__, (int)demander);

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_success;

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]%s FAIL !\n", __FUNCTION__);

    if (vcore_dvfs_i)
    {
        kfree(vcore_dvfs_i);
        vcore_dvfs_i = NULL;
    }

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_fail;
}

int vcore_dvfs_unregister(vcore_dvfs_demander_e demander)
{
    struct vcore_dvfs *vcore_dvfs_i;
    
    VCORE_DVFS_MUTEX_LOCK(&vcore_dvfs_mutex_lock);

    if (IF_NOT_ININTED())
    {
        goto fail;
    }

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
		if (vcore_dvfs_i->demander == demander)
        {
            list_del(&vcore_dvfs_i->list);
            kfree(vcore_dvfs_i);
            vcore_dvfs_i = NULL;

            VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
            return vcore_dvfs_success;
        }
    }

fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]%s FAIL !\n", __FUNCTION__);

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_fail;
}

int vcore_dvfs_clk_enable(vcore_dvfs_demander_e demander, clk_enable_hw clk_enable_hw)
{
    struct vcore_dvfs *vcore_dvfs_i;

    VCORE_DVFS_MUTEX_LOCK(&vcore_dvfs_mutex_lock);

    if (IF_NOT_ININTED())
    {
        goto fail;
    }

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
		if (vcore_dvfs_i->demander == demander)
        {
            vcore_dvfs_i->active = true;
            if (vcore_dvfs_update_voltage() != vcore_dvfs_success)
            {
                goto fail;
            }

            if (clk_enable_hw)
            {
                if (clk_enable_hw() != vcore_dvfs_success)
                {
                    goto fail;
                }
            }
 
            VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
            return vcore_dvfs_success;
        }
    }
    
fail:

    vcore_dvfs_i->active = false;
    vcore_dvfs_update_voltage(); //

    VCORE_DVFS_ERR("[VCORE_DVFS]%s FAIL !\n", __FUNCTION__);

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_fail;
}

int vcore_dvfs_clk_disable(vcore_dvfs_demander_e demander, clk_disable_hw clk_disable_hw)
{
    struct vcore_dvfs *vcore_dvfs_i;

    VCORE_DVFS_MUTEX_LOCK(&vcore_dvfs_mutex_lock);

    if (IF_NOT_ININTED())
    {
        goto fail;
    }

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
		if (vcore_dvfs_i->demander == demander)
        {
            vcore_dvfs_i->active = false;
            if (vcore_dvfs_update_voltage() != vcore_dvfs_success)
            {
                goto fail;
            }
            
            if (clk_disable_hw)
            {
                clk_disable_hw();
            }
 
            VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
            return vcore_dvfs_success;
        }
    }
    
fail:

/*
    vcore_dvfs_i->active = true;
    if (vcore_dvfs_update_voltage() != vcore_dvfs_success)
    {
        // Fatal Error
    }
*/

    VCORE_DVFS_ERR("[VCORE_DVFS]%s FAIL !\n", __FUNCTION__);

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_fail;
}

int vcore_dvfs_clk_set_rate(vcore_dvfs_demander_e demander, unsigned long rate, set_rate_hw set_rate_hw)
{
    struct vcore_dvfs *vcore_dvfs_i;
    unsigned long old_rate;

    
    VCORE_DVFS_MUTEX_LOCK(&vcore_dvfs_mutex_lock);

    if (IF_NOT_ININTED())
    {
        goto fail;
    }

    VCORE_DVFS_DBG("[VCORE_DVFS]%s !\n",__FUNCTION__);

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
		if (vcore_dvfs_i->demander == demander)
        {
            old_rate = vcore_dvfs_i->rate;
        
            vcore_dvfs_i->rate = rate;
            vcore_dvfs_i->voltage = vcore_dvfs_get_demander_voltage_by_rate(demander, vcore_dvfs_i->rate);
            if ( vcore_dvfs_i->voltage < 0 )
            {
                goto fail;
            }
        
            if (vcore_dvfs_i->active == true)
            {
                if (rate < old_rate)
                {
                    set_rate_hw(rate);

                    if (vcore_dvfs_update_voltage() != vcore_dvfs_success)
                    {
                        goto fail;
                    }
                }
                else if (rate > old_rate)
                {
                    if (vcore_dvfs_update_voltage() != vcore_dvfs_success)
                    {
                        goto fail;
                    }
                    
                    set_rate_hw(rate);
                }         
                else if (rate == old_rate)
                {
                    set_rate_hw(rate);
                }
            }
            else
            {
                set_rate_hw(rate);
            }
 
            VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
            return vcore_dvfs_success;
        }
    }
    
fail:

    VCORE_DVFS_ERR("[VCORE_DVFS]%s FAIL !\n",__FUNCTION__);

    VCORE_DVFS_MUTEX_UNLOCK(&vcore_dvfs_mutex_lock);
    return vcore_dvfs_fail;
}

EXPORT_SYMBOL(vcore_dvfs_register);
EXPORT_SYMBOL(vcore_dvfs_unregister);
EXPORT_SYMBOL(vcore_dvfs_clk_enable);
EXPORT_SYMBOL(vcore_dvfs_clk_disable);
EXPORT_SYMBOL(vcore_dvfs_clk_set_rate);



static int vcore_dvfs_open(struct inode *inode, struct file *file);
static long vcore_dvfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static const struct file_operations vcore_dvfs_fops =
{
    .open     = vcore_dvfs_open,
    .unlocked_ioctl = vcore_dvfs_ioctl,
};

static long vcore_dvfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int s32Err= 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((VCORE_DVFS_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> VCORE_DVFS_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        s32Err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        s32Err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (s32Err)
    {
        return -EFAULT;
    }

    switch(cmd)
    {
        case VCORE_DVFS_INIT:
            break;

        case VCORE_DVFS_OP:
            break;

        default:
            printk("ioctl: unknown command\n");
            return -ENOTTY;
    }

    return 0;
}

static int vcore_dvfs_open(struct inode *inode, struct file *file)
{
    int ret=0;
    if (ret)
    {
        VCORE_DVFS_ERR("[VCORE_DVFS]vcore_dvfs_open FAIL !\n");
        return -EINVAL;
    }

    return 0;
}

static int vcore_dvfs_probe(struct platform_device *pdev)
{
    VCORE_DVFS_DBG("[VCORE_DVFS]vcore_dvfs_probe \n");
    
    return 0;
}

static int vcore_dvfs_remove(struct platform_device *pdev)
{
    struct vcore_dvfs *vcore_dvfs_i;

    VCORE_DVFS_DBG("[VCORE_DVFS]vcore_dvfs_remove \n");

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
        list_del(&vcore_dvfs_i->list);
        kfree(vcore_dvfs_i);
    }

    if (vcore_dvfs_info.gpio_pins)
    {
        kfree(vcore_dvfs_info.gpio_pins);
        vcore_dvfs_info.gpio_pins = NULL;
    }

    if (vcore_dvfs_info.voltages)
    {
        kfree(vcore_dvfs_info.voltages);
        vcore_dvfs_info.voltages = NULL;
    }

    if (vcore_dvfs_info.voltages_sort_idxs)
    {
        kfree(vcore_dvfs_info.voltages_sort_idxs);
        vcore_dvfs_info.voltages_sort_idxs = NULL;
    }

    vcore_dvfs_info.inited = false;

/*
    Set to init voltage first here !?
    vcore_dvfs_update_voltage();
*/

    return 0;
}

#ifdef CONFIG_PM
static int vcore_dvfs_suspend(struct platform_device *dev, pm_message_t state)
{
    VCORE_DVFS_DBG("[VCORE_DVFS]vcore_dvfs_suspend \n");
    return 0;
}

static int vcore_dvfs_resume(struct platform_device *dev)
{
    VCORE_DVFS_DBG("[VCORE_DVFS]vcore_dvfs_resume \n");
    return 0;
}
#else
#define vcore_dvfs_suspend NULL
#define vcore_dvfs_resume  NULL
#endif /* CONFIG_PM */

static const struct of_device_id vcore_dvfs_of_match_table[] =
{
    { .compatible = "sstar,vcore_dvfs" },
    {}
};
MODULE_DEVICE_TABLE(of, vcore_dvfs_of_match_table);

static struct platform_driver vcore_dvfs_driver =
{
    .probe		= vcore_dvfs_probe,
    .remove		= vcore_dvfs_remove,
#ifdef CONFIG_PM
    .suspend	= vcore_dvfs_suspend,
    .resume		= vcore_dvfs_resume,
#endif
    .driver		= {
        .owner	= THIS_MODULE,
        .name	= "vcore_dvfs",
        .of_match_table = vcore_dvfs_of_match_table,
    },
};



#if 0 // Only for debug
static ssize_t dbg_debug_register_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    static int val = 0;

    val++;
    if (val % 2)
    {
        vcore_dvfs_clk_enable(VCORE_DVFS_DEMANDER_TEST, NULL);
    }
    else
    {
        vcore_dvfs_clk_disable(VCORE_DVFS_DEMANDER_TEST, NULL);
    }

    return sprintf(buf, "%d\n", val);
}


static ssize_t dbg_debug_register_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct vcore_dvfs_init_data init_data;
        
    if (NULL != buf)
    {
        printk("VCORE_DVFS_DEBUG: %s\r\n",__FUNCTION__);
        init_data.rate = 333;
        init_data.active = false; // true;
        vcore_dvfs_register(VCORE_DVFS_DEMANDER_TEST, &init_data);
    }

    return count;
}

static ssize_t dbg_debug_set_rate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

static ssize_t dbg_debug_set_rate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct clk		*clk;
    unsigned long rate = 0;
    int ret;
    struct device_node *np = NULL;
    
    if (NULL != buf)
    {
        np = of_find_node_by_name(NULL, "xm6");

        if (np == NULL)
        {
            printk("VCORE_DVFS_DEBUG: np == NULL\r\n");
            return -1;
        }

        clk = of_clk_get(np, 0);

        if (clk == NULL)
        {
            printk("VCORE_DVFS_DEBUG: clk == NULL\r\n");
            return -1;
        }

        if (buf[0] == '5')
        {
            rate = 500000000;
        }
        else
        {
            rate = 600000000;
        }
        //rate = atol(buf);

        printk("VCORE_DVFS_DEBUG: rate = %lu\r\n",rate);

        ret = clk_set_rate(clk, rate);
    }

    return count;
}
#endif

static ssize_t dbg_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i = 0;

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "---VCORE DVFS INFO---\r\n");
    str += scnprintf(str, end - str, "gpio_total_num = %d\r\n",vcore_dvfs_info.gpio_total_num);
    for (i = 0; i < vcore_dvfs_info.gpio_total_num; i++)
    {
        str += scnprintf(str, end - str, "gpio_pins[%d] = %d\r\n",i, vcore_dvfs_info.gpio_pins[i]);
    }
    
    str += scnprintf(str, end - str, "voltages_total_num = %d\r\n",vcore_dvfs_info.voltages_total_num);

    for (i = 0; i < vcore_dvfs_info.voltages_total_num; i++)
    {
        str += scnprintf(str, end - str, "voltages[%d] = %d\r\n",i, vcore_dvfs_info.voltages[i]);
    }
    
    for (i = 0; i < vcore_dvfs_info.voltages_total_num; i++)
    {
        str += scnprintf(str, end - str, "voltages_sort_idxs[%d] = %d\r\n",i, vcore_dvfs_info.voltages_sort_idxs[i]);
    } 
    
    str += scnprintf(str, end - str, "voltage_current = %d\r\n",vcore_dvfs_info.voltage_current);
    str += scnprintf(str, end - str, "voltage_init = %d\r\n",vcore_dvfs_info.voltage_init);
    str += scnprintf(str, end - str, "voltage_init_seq = %d\r\n",vcore_dvfs_info.voltage_init_seq);
    
    return (str - buf);
}

static ssize_t dbg_demander_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct vcore_dvfs *vcore_dvfs_i;
    
    str += scnprintf(str, end - str, "---VCORE DVFS DEMANDER INFO---\r\n");

	list_for_each_entry(vcore_dvfs_i, &vcore_dvfs_list, list)
	{
        str += scnprintf(str, end - str, "------\r\n");
	    str += scnprintf(str, end - str, "demander = %d\r\n",vcore_dvfs_i->demander);
        str += scnprintf(str, end - str, "active = %d\r\n",vcore_dvfs_i->active);
        str += scnprintf(str, end - str, "rate = %lu\r\n",vcore_dvfs_i->rate);
        str += scnprintf(str, end - str, "voltage = %d\r\n",vcore_dvfs_i->voltage);
        str += scnprintf(str, end - str, "rate_vol_num = %d\r\n",vcore_dvfs_i->rate_vol_num);
    }

    return (str - buf);
}

static ssize_t dbg_rate_vol_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0, j = 0;

    str += scnprintf(str, end - str, "---VCORE DVFS RATE VOL INFO---\r\n");

    for (i = 0; i < VCORE_DVFS_DEMANDER_TOTAL; i++)
    {
        str += scnprintf(str, end - str, "Demander %d:\r\n", i);
        
        for (j = 0; j < rate_voltage_num_table[i]; j++)
        {
            str += scnprintf(str, end - str, "[%d]rate = %lu\t vol=%d\r\n", j, rate_voltage_all_table[i][j].rate, rate_voltage_all_table[i][j].voltage);
        }
    }

    return (str - buf);
}

static ssize_t dbg_gpio_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i = 0;
    int idx_i = 0;
    int *gpio_values = NULL;
    
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "---VCORE DVFS GPIO INFO---\r\n");

    gpio_values = kzalloc(sizeof(int) * vcore_dvfs_info.gpio_total_num, GFP_KERNEL);
    if (!gpio_values)
        goto fail;

    for (i = 0; i < vcore_dvfs_info.gpio_total_num; i++)
    {
        gpio_values[i] = MDrv_GPIO_Pad_Read(vcore_dvfs_info.gpio_pins[i]);
        idx_i += (( gpio_values[i] & 1 ) << i );
    }

    str += scnprintf(str, end - str, "IDX = %d\r\n", idx_i);
    str += scnprintf(str, end - str, "( ");
    for (i = 0; i < vcore_dvfs_info.gpio_total_num; i++)
    {
        str += scnprintf(str, end - str, "%d ", gpio_values[(vcore_dvfs_info.gpio_total_num - i - 1)]);
    }
    str += scnprintf(str, end - str, ")\r\n");
    str += scnprintf(str, end - str, "voltage = %d\r\n", vcore_dvfs_get_voltage_by_idx(idx_i));

    if (gpio_values)
    {
        kfree(gpio_values);
        gpio_values = NULL;
    }

fail:
    
    if (gpio_values)
    {
        kfree(gpio_values);
        gpio_values = NULL;
    }
    
    return (str - buf);
}

static ssize_t dbg_gpio_info_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (NULL != buf)
    {
/*
        if (buf[0] == '5')
        {
        }
        else
        {
        }
*/
        return count;
    }

    return 0;
}

static DEVICE_ATTR(vcore_dvfs_info, 0444, dbg_info_show, NULL);
static DEVICE_ATTR(vcore_dvfs_demander_info, 0444, dbg_demander_info_show, NULL);
static DEVICE_ATTR(vcore_dvfs_rate_vol_info, 0444, dbg_rate_vol_info_show, NULL);
static DEVICE_ATTR(vcore_dvfs_gpio_info, 0644, dbg_gpio_info_show, dbg_gpio_info_store);

static struct miscdevice vcore_dvfs_miscdev = {MISC_DYNAMIC_MINOR, "vcore_dvfs", &vcore_dvfs_fops};

#if 0
module_platform_driver(vcore_dvfs_driver);
#else
static int __init vcore_dvfs_driver_init(void)
{
    struct device *dev;

    misc_register(&vcore_dvfs_miscdev);
    
    dev = vcore_dvfs_miscdev.this_device;

    device_create_file(dev, &dev_attr_vcore_dvfs_info);
    device_create_file(dev, &dev_attr_vcore_dvfs_demander_info);
    device_create_file(dev, &dev_attr_vcore_dvfs_rate_vol_info);
    device_create_file(dev, &dev_attr_vcore_dvfs_gpio_info);

    return platform_driver_register(&vcore_dvfs_driver);
}

//module_init(vcore_dvfs_driver_init);
subsys_initcall(vcore_dvfs_driver_init);

static void __exit vcore_dvfs_driver_exit(void)
{
    misc_deregister(&vcore_dvfs_miscdev);
    
    return platform_driver_unregister(&vcore_dvfs_driver);
}
module_exit(vcore_dvfs_driver_exit);
#endif

MODULE_AUTHOR("SStar Semiconductor, Inc.");
MODULE_DESCRIPTION("infinity Vcore Dvfs Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:vcore_dvfs");
