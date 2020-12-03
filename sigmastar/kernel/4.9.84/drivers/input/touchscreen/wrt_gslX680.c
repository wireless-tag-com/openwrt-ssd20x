/*
 * drivers/input/touchscreen/gslX680.c
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/async.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_irq.h>

#include "wrt_gslX680.h"

#define GPIO_HIGH 1
#define GPIO_LOW 0

/* #define GSL_DEBUG */
/* #define GSL_TIMER */
#define REPORT_DATA_ANDROID_4_0  //Android4.0以上版本打开此宏

/* #define HAVE_TOUCH_KEY */

#define GSLX680_I2C_NAME "gslX680"

#define GSLX680_GPIO_INT_NAME   "gslx680_int"
#define GSLX680_GPIO_RST_NAME   "gslx680_rst"


#define GSL_DATA_REG        0x80
#define GSL_STATUS_REG      0xe0
#define GSL_PAGE_REG        0xf0

#define PRESS_MAX           255
#define MAX_FINGERS         5
#define MAX_CONTACTS        10
#define DMA_TRANS_LEN       0x20

#define WRITE_I2C_SPEED     350*1000
#define I2C_SPEED           200*1000

struct gsl_ts_data {
    u8 x_index;
    u8 y_index;
    u8 z_index;
    u8 id_index;
    u8 touch_index;
    u8 data_reg;
    u8 status_reg;
    u8 data_size;
    u8 touch_bytes;
    u8 update_data;
    u8 touch_meta_data;
    u8 finger_size;
};

static struct gsl_ts_data devices[] = {
    {
        .x_index = 6,
        .y_index = 4,
        .z_index = 5,
        .id_index = 7,
        .data_reg = GSL_DATA_REG,
        .status_reg = GSL_STATUS_REG,
        .update_data = 0x4,
        .touch_bytes = 4,
        .touch_meta_data = 4,
        .finger_size = 70,
    },
};

struct gsl_ts {
    struct i2c_client *client;
    struct input_dev *input;
    struct work_struct work;
    struct workqueue_struct *wq;
    struct gsl_ts_data *dd;
    u8 *touch_data;
    u8 device_id;
    int irq;

    struct gpio_desc *irq_gpio;
    struct gpio_desc *reset_gpio;
};

static struct gsl_ts *g_gsl_ts;

//#define GSL_DEBUG

#ifdef GSL_DEBUG
#define print_info(fmt, args...) printk(fmt, ##args)
#else
#define print_info(fmt, args...)
#endif

static int reset_gpio;
static int irq_gpio;

static u32 id_sign[MAX_CONTACTS+1] = {0};
static u8 id_state_flag[MAX_CONTACTS+1] = {0};
static u8 id_state_old_flag[MAX_CONTACTS+1] = {0};
static u16 x_old[MAX_CONTACTS+1] = {0};
static u16 y_old[MAX_CONTACTS+1] = {0};
static u16 x_new;
static u16 y_new;

int i2c_master_normal_send(const struct i2c_client *client, const char *buf, int count)
{
    int ret;
    struct i2c_adapter *adap = client->adapter;
    struct i2c_msg msg;

    msg.addr = client->addr;
    msg.flags = client->flags;
    msg.len = count;
    msg.buf = (char *)buf;
    ret = i2c_transfer(adap, &msg, 1);

    return (ret == 1) ? count : ret;
}

int i2c_master_normal_recv(const struct i2c_client *client, char *buf, int count)
{
    struct i2c_adapter *adap = client->adapter;
    struct i2c_msg msg;
    int ret;

    msg.addr = client->addr;
    msg.flags = client->flags | I2C_M_RD;
    msg.len = count;
    msg.buf = (char *)buf;

    ret = i2c_transfer(adap, &msg, 1);

    return (ret == 1) ? count : ret;
}

static u32 gsl_write_interface(struct i2c_client *client, const u8 reg, u8 *buf, u32 num)
{
    struct i2c_msg xfer_msg[1];

    buf[0] = reg;

    xfer_msg[0].addr = client->addr;
    xfer_msg[0].len = num + 1;
    xfer_msg[0].flags = client->flags & I2C_M_TEN;
    xfer_msg[0].buf = buf;

    return i2c_transfer(client->adapter, xfer_msg, 1) == 1 ? 0 : -EFAULT;
}

static int gsl_ts_write(struct i2c_client *client, u8 addr, u8 *pdata, int datalen)
{
    //int ret = 0;
    u8 tmp_buf[128];
    unsigned int bytelen;

    bytelen = 0;

    if (datalen > 125) {
        printk("%s too big datalen = %d!\n", __func__, datalen);
        return -1;
    }

    tmp_buf[0] = addr;
    bytelen++;
    if (datalen != 0 && pdata != NULL) {
        memcpy(&tmp_buf[bytelen], pdata, datalen);
        bytelen += datalen;
    }

    return i2c_master_normal_send(client, tmp_buf, bytelen);

}

static int gsl_ts_read(struct i2c_client *client, u8 addr, u8 *pdata, unsigned int datalen)
{
    int ret = 0;

    if (datalen > 126) {
        printk("%s too big datalen = %d!\n", __func__, datalen);
        return -1;
    }

    ret = gsl_ts_write(client, addr, NULL, 0);
    if (ret < 0) {
        printk("%s set data address fail!\n", __func__);
        return ret;
    }

    return i2c_master_normal_recv(client, pdata, datalen);
}

static __inline__ void fw2buf(u8 *buf, const u32 *fw)
{
    u32 *u32_buf = (int *)buf;
    *u32_buf = *fw;
}

static void gsl_load_fw(struct i2c_client *client)
{
    u8 buf[DMA_TRANS_LEN*4 + 1] = {0};
    u8 send_flag = 1;
    u8 *cur = buf + 1;
    u32 source_line = 0;
    u32 source_len;

    const struct fw_data *ptr_fw;
    printk("=============gsl_load_fw start==============\n");

    ptr_fw = GSL1680E_FW;
    source_len = ARRAY_SIZE(GSL1680E_FW);

    for (source_line = 0; source_line < source_len; source_line++) {
        /* init page trans, set the page val */
        if (GSL_PAGE_REG == ptr_fw[source_line].offset) {
            fw2buf(cur, &ptr_fw[source_line].val);
            gsl_write_interface(client, GSL_PAGE_REG, buf, 4);
            send_flag = 1;
        } else {
            if (1 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20))
                buf[0] = (u8)ptr_fw[source_line].offset;

            fw2buf(cur, &ptr_fw[source_line].val);
            cur += 4;

            if (0 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20)) {
                gsl_write_interface(client, buf[0], buf, cur - buf - 1);
                cur = buf + 1;
            }

            send_flag++;
        }
    }

    printk("=============gsl_load_fw end==============\n");
}

static void startup_chip(struct i2c_client *client)
{
    u8 tmp = 0x00;

#ifdef GSL_NOID_VERSION
    gsl_DataInit(gsl_config_data_id);
#endif

    gsl_ts_write(client, 0xe0, &tmp, 1);
    msleep(10);
}

static void reset_chip(struct i2c_client *client)
{
    u8 tmp = 0x88;
    u8 buf[4] = {0x00};

    gsl_ts_write(client, 0xe0, &tmp, sizeof(tmp));
    msleep(10);
    tmp = 0x04;
    gsl_ts_write(client, 0xe4, &tmp, sizeof(tmp));
    msleep(10);
    gsl_ts_write(client, 0xbc, buf, sizeof(buf));
    msleep(10);
}

static void clr_reg(struct i2c_client *client)
{
    u8 write_buf[4]	= {0};

    write_buf[0] = 0x88;
    gsl_ts_write(client, 0xe0, &write_buf[0], 1);
    msleep(10);
    write_buf[0] = 0x01;
    gsl_ts_write(client, 0x80, &write_buf[0], 1);
    msleep(10);
    write_buf[0] = 0x04;
    gsl_ts_write(client, 0xe4, &write_buf[0], 1);
    msleep(10);
    write_buf[0] = 0x00;
    gsl_ts_write(client, 0xe0, &write_buf[0], 1);
    msleep(10);
}

static int test_i2c(struct i2c_client *client)
{
    u8 read_buf = 0;
    u8 write_buf = 0x12;
    int ret, rc = 1;

    ret = gsl_ts_read( client, 0xf0, &read_buf, sizeof(read_buf) );
    if  (ret  < 0)
        rc --;
    else
        printk("I read reg 0xf0 is %x\n", read_buf);

    msleep(5);
    ret = gsl_ts_write(client, 0xf0, &write_buf, sizeof(write_buf));
    if(ret  >=  0 )
        printk("I write reg 0xf0 is 0x12\n");

    msleep(5);
    ret = gsl_ts_read( client, 0xf0, &read_buf, sizeof(read_buf) );
    if(ret <  0 )
        rc --;
    else
        printk("I read reg 0xf0 is 0x%x\n", read_buf);

    return rc;
}

static void init_chip(struct i2c_client *client)
{
    int ret;

    gpio_direction_output(reset_gpio, GPIO_HIGH);
    msleep(10);
    gpio_set_value(reset_gpio, GPIO_LOW);
    msleep(10);
    gpio_set_value(reset_gpio, GPIO_HIGH);
    msleep(10);

    ret= test_i2c(client);
    if(ret < 0)
    {
        printk("------gslX680 test_i2c Failed------\n");
        return;
    }

    clr_reg(client);
    reset_chip(client);
    gsl_load_fw(client);
    startup_chip(client);
    reset_chip(client);
    startup_chip(client);
}

static void check_mem_data(struct i2c_client *client)
{
    u8 read_buf[4]  = {0};

    msleep(20);
    gsl_ts_read(client, 0xb0, read_buf, sizeof(read_buf));
    // printk("#########check mem read 0xb0 = %x %x %x %x #########\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);

    if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a) {
        printk("#########check mem read 0xb0 = %x %x %x %x #########\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);

        init_chip(client);
    }
}

static void record_point(u16 x, u16 y , u8 id)
{
    u16 x_err = 0;
    u16 y_err = 0;

    id_sign[id] = id_sign[id] + 1;

    if (id_sign[id] == 1) {
        x_old[id] = x;
        y_old[id] = y;
    }

    x = (x_old[id] + x)/2;
    y = (y_old[id] + y)/2;

    if (x > x_old[id]) {
        x_err = x - x_old[id];
    } else {
        x_err = x_old[id]-x;
    }

    if (y > y_old[id]) {
        y_err = y - y_old[id];
    } else {
        y_err = y_old[id] - y;
    }

    if ((x_err > 3 && y_err > 1) || (x_err > 1 && y_err > 3)) {
        x_new = x;
        x_old[id] = x;
        y_new = y;
        y_old[id] = y;
    } else {
        if (x_err > 3) {
            x_new = x;
            x_old[id] = x;
        } else
            x_new = x_old[id];

        if (y_err > 3) {
            y_new = y;
            y_old[id] = y;
        } else
            y_new = y_old[id];
    }

    if (id_sign[id] == 1) {
        x_new = x_old[id];
        y_new = y_old[id];
    }
}

static void report_data(struct gsl_ts *ts, u16 x, u16 y, u8 pressure, u8 id)
{
    swap(x, y);

    //printk("------%s--------\n",__func__);
    //x = 1024 - x;
    y = 600 - y;
    //printk("TP_DATA: X= %d, Y= %d, ID = %d \n",x,y,id);

    if (x > SCREEN_MAX_X || y > SCREEN_MAX_Y) {
#ifdef HAVE_TOUCH_KEY
        report_key(ts, x, y);
#endif
        return;
    }

    /*上报一次触摸数据*/

#ifdef REPORT_DATA_ANDROID_4_0
    input_mt_slot(ts->input, id);
    input_report_abs(ts->input, ABS_MT_TRACKING_ID, id);
    input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, pressure);
    input_report_abs(ts->input, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR, 1);
#else
    /*数据上报给用户*/
    input_report_abs(ts->input, ABS_MT_TRACKING_ID, id);
    input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, pressure);
    input_report_abs(ts->input, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR, 1);
    //input_report_key(ts->input, BTN_TOUCH, 0);

    /*同步单次数据*/
    input_mt_sync(ts->input);
#endif
}

static inline u16 join_bytes(u8 a, u8 b)
{
    u16 ab = 0;

    ab = ab | a;
    ab = ab << 8 | b;
    return ab;
}

static void process_gslX680_data(struct gsl_ts *ts)
{
    u8 id, touches;
    u16 x, y;
    int i;
    u32 tmp1;
    u8 buf[4] = {0};
    struct gsl_touch_info cinfo;

    touches = ts->touch_data[ts->dd->touch_index];
    //printk("----------Touch Nums: %d----------\n",touches);

#ifdef GSL_NOID_VERSION
    cinfo.finger_num = touches;

    for(i = 0; i < (touches < MAX_CONTACTS ? touches : MAX_CONTACTS); i ++)
    {
        cinfo.x[i] = join_bytes( ( ts->touch_data[ts->dd->x_index  + 4 * i + 1] & 0xf),
        ts->touch_data[ts->dd->x_index + 4 * i]);
        cinfo.y[i] = join_bytes(ts->touch_data[ts->dd->y_index + 4 * i + 1],
        ts->touch_data[ts->dd->y_index + 4 * i ]);
        cinfo.id[i] = ((ts->touch_data[ts->dd->x_index  + 4 * i + 1] & 0xf0)>>4);

        //printk("tp-gsl  before: x[%d] = %d, y[%d] = %d, id[%d] = %d \n",i,cinfo.x[i],i,cinfo.y[i],i,cinfo.id[i]);
    }
    cinfo.finger_num=(ts->touch_data[3]<<24)|(ts->touch_data[2]<<16)|(ts->touch_data[1]<<8)|(ts->touch_data[0]);


    gsl_alg_id_main(&cinfo);
    tmp1=gsl_mask_tiaoping();
    //printk("[tp-gsl] tmp1=%x\n",tmp1);

    if(tmp1>0&&tmp1<0xffffffff)
    {
        buf[0]=0xa;buf[1]=0;buf[2]=0;buf[3]=0;
        gsl_ts_write(ts->client,0xf0,buf,4);
        buf[0]=(u8)(tmp1 & 0xff);
        buf[1]=(u8)((tmp1>>8) & 0xff);
        buf[2]=(u8)((tmp1>>16) & 0xff);
        buf[3]=(u8)((tmp1>>24) & 0xff);

        //printk("tmp1=%08x,buf[0]=%02x,buf[1]=%02x,buf[2]=%02x,buf[3]=%02x\n",tmp1,buf[0],buf[1],buf[2],buf[3]);
        gsl_ts_write(ts->client,0x8,buf,4);
    }
    touches = cinfo.finger_num;
#endif

    for (i = 1; i <= MAX_CONTACTS; i++) {
        if (touches == 0)
            id_sign[i] = 0;
        id_state_flag[i] = 0;
    }

    for (i = 0; i < (touches > MAX_FINGERS ? MAX_FINGERS : touches); i++)
    {
        /* 	x = join_bytes((ts->touch_data[ts->dd->x_index + 4*i + 1] & 0xf), ts->touch_data[ts->dd->x_index + 4*i]);
        y = join_bytes(ts->touch_data[ts->dd->y_index + 4*i + 1], ts->touch_data[ts->dd->y_index + 4*i]);
        id = ts->touch_data[ts->dd->id_index + 4*i] >> 4; */

        id = cinfo.id[i];
        x =  cinfo.x[i];
        y =  cinfo.y[i];

        //printk("-----x = %d  y =%d  id = %d------\n",x,y,id);
        if (1 <= id && id <= MAX_CONTACTS) {
            /*数据处理*/
            record_point(x, y, id);

            /*数据上报*/
            report_data(ts, x_new,y_new,10, id);

            id_state_flag[id] = 1;
        }
    }

    for (i = 1; i <= MAX_CONTACTS; i++) {
        if ((0 == touches) || ((0 != id_state_old_flag[i]) && (0 == id_state_flag[i]))) {
#ifdef REPORT_DATA_ANDROID_4_0
            input_mt_slot(ts->input, i);
            input_report_abs(ts->input, ABS_MT_TRACKING_ID, -1);
            input_mt_report_slot_state(ts->input, MT_TOOL_FINGER, false);
#endif
            id_sign[i] = 0;
        }
        id_state_old_flag[i] = id_state_flag[i];
    }

#ifndef REPORT_DATA_ANDROID_4_0
    if (0 == touches) {
        input_mt_sync(ts->input);
#ifdef HAVE_TOUCH_KEY
        if (key_state_flag) {
            input_report_key(ts->input, key, 0);
            input_sync(ts->input);
            key_state_flag = 0;
        }
#endif
    }
#endif
    /*数据同步*/

    input_sync(ts->input);
}

/*
    name：中断下半部处理函数
    function：获取gslX680坐标数据,解析数据，数据上报同步
*/
static void gsl_ts_xy_worker(struct work_struct *work)
{
    int rc;
    u8 read_buf[4] = {0};

    struct gsl_ts *ts = container_of(work, struct gsl_ts, work);
    print_info("----------gsl_ts_xy_worker----------\n");

    /* 从触摸芯片读取坐标数据 */
    rc = gsl_ts_read(ts->client, 0x80, ts->touch_data, ts->dd->data_size);
    print_info("----------touches: %d ----------\n", ts->touch_data[0]);

    if (rc < 0) {
        dev_err(&ts->client->dev, "read failed\n");
        goto schedule;
    }

    if (ts->touch_data[ts->dd->touch_index] == 0xff) {
        goto schedule;
    }

    rc = gsl_ts_read(ts->client, 0xbc, read_buf, sizeof(read_buf));
    if (rc < 0) {
        dev_err(&ts->client->dev, "read 0xbc failed\n");
        goto schedule;
    }
    print_info("*********reg[%x]: %x %x %x %x*********\n", 0xbc, read_buf[3], read_buf[2], read_buf[1], read_buf[0]);

    if (read_buf[3] == 0 && read_buf[2] == 0 && read_buf[1] == 0 && read_buf[0] == 0) {
        /*坐标数据解析*/
        process_gslX680_data(ts);
    } else {
        reset_chip(ts->client);
        startup_chip(ts->client);
    }
    schedule:
    enable_irq(ts->irq);
}

/*中断处理函数*/
static irqreturn_t gsl_ts_irq(int irq, void *dev_id)
{
    struct gsl_ts *ts = dev_id;

    disable_irq_nosync(ts->irq);

    /*调度任务*/
    if (!work_pending(&ts->work))
    {
        queue_work(ts->wq, &ts->work);
    }

    return IRQ_HANDLED;
}

static int gsl_ts_init_ts(struct i2c_client *client, struct gsl_ts *ts)
{
    struct input_dev *input_device;
    int rc = 0;

    ts->dd = &devices[ts->device_id];

    if (ts->device_id == 0) {
        ts->dd->data_size = MAX_FINGERS * ts->dd->touch_bytes + ts->dd->touch_meta_data;
        ts->dd->touch_index = 0;
    }

    ts->touch_data = kzalloc(ts->dd->data_size, GFP_KERNEL);
    if (!ts->touch_data) {
        pr_err("%s: Unable to allocate memory\n", __func__);
        return -ENOMEM;
    }

    /*申请input device对象*/
    input_device = input_allocate_device();
    if (!input_device) {
        rc = -ENOMEM;
        goto error_alloc_dev;
    }
    ts->input = input_device;

    /*提供用户查看信息  sys/class/input/event x/device*/
    input_device->name = GSLX680_I2C_NAME;
    input_device->id.bustype = BUS_I2C;
    input_device->dev.parent = &client->dev;
    input_set_drvdata(input_device, ts);

#ifdef REPORT_DATA_ANDROID_4_0
    __set_bit(EV_ABS, input_device->evbit);
    __set_bit(EV_KEY, input_device->evbit);
    __set_bit(EV_REP, input_device->evbit);
    __set_bit(INPUT_PROP_DIRECT, input_device->propbit);
    input_mt_init_slots(input_device, (MAX_CONTACTS+1), 0);
#else
    input_set_abs_params(input_device,ABS_MT_TRACKING_ID, 0, (MAX_CONTACTS+1), 0, 0);
    set_bit(EV_ABS, input_device->evbit);
    set_bit(EV_KEY, input_device->evbit);
    __set_bit(INPUT_PROP_DIRECT, input_device->propbit);
    input_device->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

#ifdef HAVE_TOUCH_KEY
    input_device->evbit[0] = BIT_MASK(EV_KEY);
    for (i = 0; i < MAX_KEY_NUM; i++)
    set_bit(key_array[i], input_device->keybit);
#endif

    /*产生哪些数据*/
    set_bit(ABS_MT_POSITION_X, input_device->absbit);
    set_bit(ABS_MT_POSITION_Y, input_device->absbit);
    set_bit(ABS_MT_TOUCH_MAJOR, input_device->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, input_device->absbit);

    /*设置参数最大值  最小值*/

    input_set_abs_params(input_device, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
    input_set_abs_params(input_device, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
    input_set_abs_params(input_device, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
    input_set_abs_params(input_device, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);

    client->irq = ts->irq;

    /*创建工作队列*/
    ts->wq = create_singlethread_workqueue("kworkqueue_ts");
    if (!ts->wq) {
        dev_err(&client->dev, "Could not create workqueue\n");
        goto error_wq_create;
    }
    flush_workqueue(ts->wq);

    /*中断下半部*/
    INIT_WORK(&ts->work, gsl_ts_xy_worker);

    /*2.申请input device 对象*/
    rc = input_register_device(input_device);
    if (rc)
        goto error_unreg_device;

    return 0;

error_unreg_device:
    destroy_workqueue(ts->wq);
error_wq_create:
    input_free_device(input_device);
error_alloc_dev:
    kfree(ts->touch_data);
    return rc;
}

static int gslx680_dts_parse(struct gsl_ts *ts)
{
    struct device_node *np;

    //unsigned int irq_gpio;
    struct device *dev;

    dev = &ts->client->dev;
    np = dev->of_node;

    if(0 != of_property_read_u32(np, GSLX680_GPIO_RST_NAME, &reset_gpio))
        return -EINVAL;
    ts->reset_gpio = gpio_to_desc(reset_gpio);
    if (gpio_request(reset_gpio, NULL) != 0) {
        printk(KERN_ERR "invalid reset_gpio:%d\n",reset_gpio);
        goto reset_request_failed;
    }
    if(0 != of_property_read_u32(np, GSLX680_GPIO_INT_NAME, &irq_gpio))
        return -EINVAL;
    if (gpio_request(irq_gpio, NULL) != 0) {
        printk(KERN_ERR "invalid irq_gpio:%d\n",irq_gpio);
        goto irq_request_failed;
    }
    ts->irq_gpio = gpio_to_desc(irq_gpio);
    gpio_direction_input(irq_gpio);

    ts->client->irq = of_irq_get_byname(np, GSLX680_GPIO_INT_NAME);
    ts->irq = ts->client->irq;
    printk("gslx680_irq_num:%d\n",ts->client->irq);

    return 0;

reset_request_failed:
    gpio_free(reset_gpio);

irq_request_failed:
    gpio_free(irq_gpio);

    return -1;
}

static int  gsl_ts_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
    {
    struct gsl_ts *ts;
    int rc;
    //u8 read_buf = 0;
    int ret;

    printk("----------%s:Start----------\n", __func__);
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "I2C functionality not supported\n");
        return -ENODEV;
    }

    ts = kzalloc(sizeof(*ts), GFP_KERNEL);
    if (!ts)
        return -ENOMEM;
    //printk("----------kzalloc success----------\n");
    g_gsl_ts = ts;

    /*1.保存client*/
    ts->client = client;
    i2c_set_clientdata(client, ts);
    ts->device_id = id->driver_data;

    /*2.dts解析*/
    ret = gslx680_dts_parse(ts);
    if(ret < 0)
    {
        printk("gsl_ts_probe error, probe failed!\n");
        return -1;
    }

    /*硬件初始化*/
    //msleep(20);
    init_chip(ts->client);
    check_mem_data(ts->client);

    /*3.input device对象定义*/
    rc = gsl_ts_init_ts(client, ts);
    if (rc < 0) {
        dev_err(&client->dev, "GSLX680 init failed\n");
        goto error_mutex_destroy;
    }


    /*4.申请中断*/
    rc = request_threaded_irq(client->irq, NULL, gsl_ts_irq,IRQF_ONESHOT | IRQF_TRIGGER_FALLING,client->name,ts);
    if (rc < 0) {
        printk("gsl_probe: request irq failed\n");
        goto error_req_irq_fail;
    }
    printk("----------%s:End----------\n", __func__);

    return 0;

error_req_irq_fail:
    free_irq(ts->irq, ts);

error_mutex_destroy:
    input_free_device(ts->input);
    kfree(ts);

    return rc;
}

static int  gsl_ts_remove(struct i2c_client *client)
{
    struct gsl_ts *ts = i2c_get_clientdata(client);

    printk("----------gsl_ts_remove----------\n");

    device_init_wakeup(&client->dev, 0);
    cancel_work_sync(&ts->work);
    free_irq(ts->irq, ts);
    destroy_workqueue(ts->wq);
    input_unregister_device(ts->input);

    kfree(ts->touch_data);

    gpio_free(reset_gpio);
    gpio_free(irq_gpio);
    kfree(ts);

    return 0;
}

static const struct i2c_device_id gsl_ts_id[] = {
    {GSLX680_I2C_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, gsl_ts_id);

static struct of_device_id gslX680_dt_ids[] = {
    { .compatible = "gslX680" },
};

static struct i2c_driver gsl_ts_driver = {
    .driver = {
        .name = GSLX680_I2C_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(gslX680_dt_ids),
    },
    .probe		= gsl_ts_probe,
    .remove		= gsl_ts_remove,
    .id_table	= gsl_ts_id,
};

static int __init gsl_ts_init(void)
{
    printk("----------gsl_ts_init----------\n");
    return i2c_add_driver(&gsl_ts_driver);
}
static void __exit gsl_ts_exit(void)
{
    printk("----------gsl_ts_exit----------\n");
    i2c_del_driver(&gsl_ts_driver);
    return;
}

module_init(gsl_ts_init);
module_exit(gsl_ts_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GSLX680 tp_driver for WRT D0 with 7 inch");
MODULE_AUTHOR("liub, seekdream116103@163.com");
MODULE_ALIAS("platform:rk3128");
