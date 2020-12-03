/*
* mdrv_ive.h- Sigmastar
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
#include <linux/cdev.h>
#include <linux/mutex.h>

#include "drv_ive.h"
#include "hal_ive.h"
#include "mdrv_ive_io_st.h"

#ifndef _MDRV_IVE_H_
#define _MDRV_IVE_H_

// SW simulation for driver debug without real HW
// #define IVE_SW_SIMULATE

// Defines reference kern levels of printfk
#define IVE_MSG_ERR     3
#define IVE_MSG_WRN     4
#define IVE_MSG_DBG     5

#define IVE_MSG_LEVL    IVE_MSG_WRN


#define IVE_MSG_ENABLE

#if defined(IVE_MSG_ENABLE)
#define IVE_MSG_FUNC_ENABLE

#define IVE_STRINGIFY(x) #x
#define IVE_TOSTRING(x) IVE_STRINGIFY(x)

#if defined(IVE_MSG_FUNC_ENABLE)
#define IVE_MSG_TITLE   "[IVE, %s] "
#define IVE_MSG_FUNC    __func__
#else   // NOT defined(IVE_MSG_FUNC_ENABLE)
#define IVE_MSG_TITLE   "[IVE] %s"
#define IVE_MSG_FUNC    ""
#endif  // NOT defined(IVE_MSG_FUNC_ENABLE)

#define IVE_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)

#define IVE_MSG(dbglv, _fmt, _args...)                          \
    do if(dbglv <= IVE_MSG_LEVL) {                              \
        printk(KERN_SOH IVE_TOSTRING(dbglv) IVE_MSG_TITLE  _fmt, IVE_MSG_FUNC, ## _args);   \
    } while(0)

#else   // NOT defined(IVE_MSG_ENABLE)
#define     IVE_ASSERT(arg)
#define     IVE_MSG(dbglv, _fmt, _args...)
#endif  // NOT defined(IVE_MSG_ENABLE)



///////////////////////////////////////////////////////////////////////////////////////////////////
// Driver Data Structure
//
//   Enum and structure fo IVE dirver
//   We declare enum & structure here because enum and structures are
//   shared in all three layers (mdrv, drv, and hal_simulation)
//
//   It is not good way for modulization, but most members of structure are not pointer
//   because it can simplify the memory managent
//   and container_of() is a key funtion to access data strucure in operators
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************************************************
 * There are 2 state enumeration
 *   IVE_DRV_STATE: indicate the state of HW
 *   IVE_FILE_STATE: indicate the state of File
 *
 *   File and HW has different state because it support multi instance
 *   and file request may be queued if HW is busy
 */
#define USE_MIU_DIRECT

typedef enum
{
    IVE_DRV_STATE_READY         = 0,
    IVE_DRV_STATE_PROCESSING    = 1,
    IVE_DRV_STATE_DONE          = 2
} IVE_DRV_STATE;

typedef enum
{
    IVE_FILE_STATE_READY        = 0,
    IVE_FILE_STATE_PROCESSING   = 1,
    IVE_FILE_STATE_DONE         = 2,
    IVE_FILE_STATE_IN_QUEUE     = 3
} IVE_FILE_STATE;

typedef union
{
    ive_ioc_coeff_ncc       ncc_buffer;
    __u8                    map_buffer[256];
} ive_work_buffer;

// Data structure for driver
typedef struct
{
    struct platform_device  *pdev;          // platform device data
    ive_hal_handle          hal_handle;     // HAL handle for real HW configuration
    IVE_DRV_STATE           dev_state;      // HW state
    struct list_head        request_list;   // request list to queue waiting requst
} ive_drv_handle;

// Device data
typedef struct {
    struct platform_device  *pdev;          // platform device data
    struct cdev             cdev;           // character device
    struct clk              **clk;          // clock
    int                     clk_num;        // clock number
    unsigned int            irq;            // IRQ number
    ive_drv_handle          drv_handle;     // device handle
    struct work_struct      work_queue;     // work queue for post process after ISR
    struct mutex            mutex;          // for critical section
} ive_dev_data;

// File private data
typedef struct{
    ive_dev_data            *dev_data;      // Device data
    ive_ioc_config          ioc_config;     // IO configuation, i.e. one device file can service one request at the same time
    IVE_FILE_STATE          state;          // File state
    wait_queue_head_t       wait_queue;     // Wait queue for polling operation
    ive_ioc_config          *user_io_config;// IO configuation pointer from user space
} ive_file_data;

#endif //_MDRV_IVE_H_
