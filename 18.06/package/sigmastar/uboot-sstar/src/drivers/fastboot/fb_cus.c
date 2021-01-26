#include <common.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_cus.h>
#include <nand.h>

#define TRACE 0
#define cus_trace(msg...) \
	do { \
		if (TRACE) \
			printf("FastBoot Cus: "msg); \
	} while (0)

#ifdef CONFIG_MS_SPINAND
static int cmd_nand_erase(const char *partname);
static int cmd_nand_write(const char *partname, void *download_buffer, unsigned int file_size);
static int cmd_nand_probe_info(const char *partname, struct part_info *part_info);
static int cmd_ubi_erase(const char *partname);
static int cmd_ubi_write(const char *partname, void *download_buffer, unsigned int file_size);
static int cmd_ubi_probe_info(const char *partname, struct part_info *part_info);
#elif defined (CONFIG_MS_PARTITION)
static int cmd_sf_erase(const char *partname);
static int cmd_sf_write(const char *partname, void *download_buffer ,unsigned int file_size);
static int cmd_sf_probe_info(const char *partname, struct part_info *part_info);
#endif

#define PRE_PART_OPS(name, type)\
        {name, cmd_##type##_erase, cmd_##type##_write, cmd_##type##_probe_info}

const part_ops_t part_ops_infos[] =
#ifdef CONFIG_MS_SPINAND
{PRE_PART_OPS("IPL0",       nand), PRE_PART_OPS("IPL1",       nand),
 PRE_PART_OPS("IPL_CUST0",  nand), PRE_PART_OPS("IPL_CUST1",  nand),
 PRE_PART_OPS("UBOOT0",     nand), PRE_PART_OPS("UBOOT1",     nand),
 PRE_PART_OPS("KERNEL",     nand), PRE_PART_OPS("RECOVERY",   nand),
 PRE_PART_OPS("rootfs",     ubi),//UBI
 PRE_PART_OPS("nvrservice", ubi),
 PRE_PART_OPS("customer",   ubi),
};
#elif defined (CONFIG_MS_PARTITION)
{PRE_PART_OPS("IPL",        sf),
 PRE_PART_OPS("IPL_CUST",   sf),
 PRE_PART_OPS("KEY_CUST",   sf),
 PRE_PART_OPS("MXPT",       sf),
 PRE_PART_OPS("UBOOT",      sf),
 PRE_PART_OPS("UBOOT_ENV",  sf),
 PRE_PART_OPS("BOOT",       sf),
 PRE_PART_OPS("KERNEL",     sf),
 PRE_PART_OPS("rootfs",     sf),
 PRE_PART_OPS("nvrservice", sf),
 PRE_PART_OPS("customer",   sf)
};
#endif

#ifdef CONFIG_MS_SPINAND
static inline int cmd_nand_erase(const char *partname)
{
    char cmd_erase_partition[100] = "\0";

    sprintf(cmd_erase_partition, "nand erase.part %s", partname);
	cus_trace("[%s] command (%s)\n", __func__, cmd_erase_partition);

    return run_command(cmd_erase_partition, 0);
}

static inline int cmd_nand_write(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_partition[100] = "\0";

    sprintf(cmd_write_partition, "nand write 0x%p %s 0x%x", download_buffer, partname, file_size);
	cus_trace("[%s] command (%s)\n", __func__, cmd_write_partition);

    return run_command(cmd_write_partition, 0);
}

static int set_dev(int dev)
{
	if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[dev].name) {
		puts("No such device\n");
		return -1;
	}

	if (nand_curr_device == dev)
		return 0;

	printf("Device %d: %s", dev, nand_info[dev].name);
	puts("... is now current device\n");
	nand_curr_device = dev;

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	board_nand_select_device(nand_info[dev].priv, dev);
#endif

	return 0;
}

static int get_nand_part_info(const char *partname, struct part_info *part_info)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
		return ret;

	if (dev->id->type != MTD_DEV_TYPE_NAND) {
		puts("not a NAND device\n");
		return -1;
	}
#if 0
	*off = part->offset;
	*size = part->size;
	*maxsize = part->size;
	*idx = dev->id->num;
#else
    part_info->size = part->size;
    cus_trace("%s part %s size %llx\n ", __func__, partname, part_info->size);
#endif

	ret = set_dev(dev->id->num);
	if (ret)
		return ret;

	return 0;
#else
	puts("offset is not a number\n");
	return -1;
#endif
}

static inline int cmd_nand_probe_info(const char *partname, struct part_info *part_info)
{
    return get_nand_part_info(partname, part_info);//only get total size of ubi part
}

static inline int cmd_ubi_erase(const char *partname)
{
    return 0;//do nothing
}

static inline int cmd_ubi_write(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_partition[100] = "\0";

    sprintf(cmd_write_partition, "ubi write 0x%p %s 0x%x", download_buffer, partname, file_size);
	cus_trace("[%s] command (%s)\n", __func__, cmd_write_partition);

    return run_command(cmd_write_partition, 0);
}

static inline int cmd_ubi_probe_info(const char *partname, struct part_info *part_info)
{
    if (get_nand_part_info("UBI", part_info))
        return -1;

    return run_command("ubi part UBI", 0);
}
#elif defined (CONFIG_MS_PARTITION)
static inline int cmd_sf_erase(const char *partname)
{
    return run_command("sf probe 0;sf erase ${sf_part_start} ${sf_part_size}", 0);
}

static inline int cmd_sf_write(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_partition[100] = "\0";

    sprintf(cmd_write_partition, "sf write 0x%p ${sf_part_start} 0x%x", download_buffer, file_size);
	cus_trace("[%s] command (%s)\n", __func__, cmd_write_partition);

    return run_command(cmd_write_partition, 0);
}

static inline int cmd_sf_probe_info(const char *partname, struct part_info *part_info)
{
    int ret;
    char cmd_get_partinfo[100] = "\0";

    sprintf(cmd_get_partinfo, "mxp r.info %s", partname);
	cus_trace("[%s], command (%s)\n", __func__, cmd_get_partinfo);

    ret = run_command(cmd_get_partinfo, 0);
    if (ret!=CMD_RET_SUCCESS)
        return -1;

    part_info->size = getenv_hex("sf_part_size", 0);
    if (part_info->size <= 0)
        return -1;

    return 0;
}
#endif
int get_patirion_index(const char *partname)
{
    int i;

    for(i = 0;i < sizeof(part_ops_infos)/sizeof(part_ops_infos[0]); i++)
    {
        if (!strcmp(part_ops_infos[i].partname, partname))
            return i;
    }
    return -1;
}

void fastboot_cus_flash_write(const char *partname, void *download_buffer,
			unsigned int download_bytes, char *response)
{
    int ret, index;
    struct part_info part_info = {};

    index = get_patirion_index(partname);
    if (index < 0)
    {
        fastboot_fail("no such partition", response);
        return;
    }

    ret = part_ops_infos[index].probe_info(partname, &part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return;
    }

    if (part_info.size < download_bytes)
    {
        fastboot_fail("file size is to large", response);
	    return;
    }

    ret = part_ops_infos[index].erase(partname);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("no such partition", response);
	    return;
    }

    ret = part_ops_infos[index].write(partname, download_buffer, download_bytes);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("no such partition", response);
	    return;
    }
    fastboot_okay(NULL, response);
}

void fastboot_cus_erase(const char *partname, char *response)
{
    int ret, index;
    struct part_info part_info = {};

    index = get_patirion_index(partname);
    if (index < 0)
    {
        fastboot_fail("no such partition", response);
        return;
    }

    ret = part_ops_infos[index].probe_info(partname, &part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return;
    }

    ret = part_ops_infos[index].erase(partname);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("no such partition", response);
	    return;
    }
    fastboot_okay(NULL, response);
}

int fastboot_cus_get_part_info(const char *partname,
                struct part_info *part_info, char *response)
{
    int ret, index;
    char *sub;

	cus_trace("[%s] partname: (%s)\n", __func__, partname);
    //find the string "_a" or "_b"
    if(((sub = strpbrk(partname, "_")) && (strpbrk(sub + 1, "a"))) ||
       ((sub = strpbrk(partname, "_")) && (strpbrk(sub + 1, "b"))))
    {
        // no support slot
        return -1;
    }
    index = get_patirion_index(partname);
    if (index < 0)
    {
        fastboot_fail("no such partition", response);
	    return -1;
    }

    ret = part_ops_infos[index].probe_info(partname, part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return -1;
    }

    fastboot_okay(NULL, response);
    return 0;
}
