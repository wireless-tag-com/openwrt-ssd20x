/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>

#include <asm/io.h>
#include <dm/device-internal.h>

static struct spi_flash *flash;

/*
 * This function computes the length argument for the erase command.
 * The length on which the command is to operate can be given in two forms:
 * 1. <cmd> offset len  - operate on <'offset',  'len')
 * 2. <cmd> offset +len - operate on <'offset',  'round_up(len)')
 * If the second form is used and the length doesn't fall on the
 * sector boundary, than it will be adjusted to the next sector boundary.
 * If it isn't in the flash, the function will fail (return -1).
 * Input:
 *    arg: length specification (i.e. both command arguments)
 * Output:
 *    len: computed length for operation
 * Return:
 *    1: success
 *   -1: failure (bad format, bad address).
 */
static int sf_parse_len_arg(char *arg, ulong *len)
{
	char *ep;
	char round_up_len; /* indicates if the "+length" form used */
	ulong len_arg;

	round_up_len = 0;
	if (*arg == '+') {
		round_up_len = 1;
		++arg;
	}

	len_arg = simple_strtoul(arg, &ep, 16);
	if (ep == arg || *ep != '\0')
		return -1;

	if (round_up_len && flash->sector_size > 0)
		*len = ROUND(len_arg, flash->sector_size);
	else
		*len = len_arg;

	return 1;
}

/**
 * This function takes a byte length and a delta unit of time to compute the
 * approximate bytes per second
 *
 * @param len		amount of bytes currently processed
 * @param start_ms	start time of processing in ms
 * @return bytes per second if OK, 0 on error
 */
static ulong bytes_per_second(unsigned int len, ulong start_ms)
{
	/* less accurate but avoids overflow */
	if (len >= ((unsigned int) -1) / 1024)
		return len / (max(get_timer(start_ms) / 1024, 1UL));
	else
		return 1024 * len / max(get_timer(start_ms), 1UL);
}

static int do_spi_flash_probe(int argc, char * const argv[])
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char *endp;
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *new, *bus_dev;
	int ret;
#else
	struct spi_flash *new;
#endif

	if (argc >= 2) {
		cs = simple_strtoul(argv[1], &endp, 0);
		if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
			return -1;
		if (*endp == ':') {
			if (endp[1] == 0)
				return -1;

			bus = cs;
			cs = simple_strtoul(endp + 1, &endp, 0);
			if (*endp != 0)
				return -1;
		}
	}

	if (argc >= 3) {
		speed = simple_strtoul(argv[2], &endp, 0);
		if (*argv[2] == 0 || *endp != 0)
			return -1;
	}
	if (argc >= 4) {
		mode = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			return -1;
	}

#ifdef CONFIG_DM_SPI_FLASH
	/* Remove the old device, otherwise probe will just be a nop */
	ret = spi_find_bus_and_cs(bus, cs, &bus_dev, &new);
	if (!ret) {
		device_remove(new);
		device_unbind(new);
	}
	flash = NULL;
	ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
	if (ret) {
		printf("Failed to initialize SPI flash at %u:%u (error %d)\n",
		       bus, cs, ret);
		return 1;
	}

	flash = new->uclass_priv;
#else
	new = spi_flash_probe(bus, cs, speed, mode);
	if (!new) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	if (flash)
		spi_flash_free(flash);
	flash = new;
#endif

	return 0;
}


int get_sf_size(int offset)
{
    int ret = -1;

	if (NULL == flash)
    {
        printf("please do command:\"sf probe 0\" first!\n");
		return ret;
    }
	else
	{
	    if (offset > flash->size)
	    {
			printf("The offset(%d) is beyond spi flash size!\n", offset);
		    return ret;
		}
		else
		{
		    ret = flash->size - offset;
			return ret;
		}
	}
}

/**
 * Write a block of data to SPI flash, first checking if it is different from
 * what is already there.
 *
 * If the data being written is the same, then *skipped is incremented by len.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @param cmp_buf	read buffer to use to compare data
 * @param skipped	Count of skipped data (incremented by this function)
 * @return NULL if OK, else a string containing the stage which failed
 */
static const char *spi_flash_update_block(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf, char *cmp_buf, size_t *skipped)
{
	debug("offset=%#x, sector_size=%#x, len=%#zx\n",
	      offset, flash->sector_size, len);
	/* Read the entire sector so to allow for rewriting */
	if (spi_flash_read(flash, offset, flash->sector_size, cmp_buf))
		return "read";
	/* Compare only what is meaningful (len) */
	if (memcmp(cmp_buf, buf, len) == 0) {
		debug("Skip region %x size %zx: no change\n",
		      offset, len);
		*skipped += len;
		return NULL;
	}
	/* Erase the entire sector */
	if (spi_flash_erase(flash, offset, flash->sector_size))
		return "erase";
	/* Write the initial part of the block from the source */
	if (spi_flash_write(flash, offset, len, buf))
		return "write";
	/* If it's a partial sector, rewrite the existing part */
	if (len != flash->sector_size) {
		/* Rewrite the original data to the end of the sector */
		if (spi_flash_write(flash, offset + len,
				    flash->sector_size - len, &cmp_buf[len]))
			return "write";
	}

	return NULL;
}

/**
 * Update an area of SPI flash by erasing and writing any blocks which need
 * to change. Existing blocks with the correct data are left unchanged.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @return 0 if ok, 1 on error
 */
static int spi_flash_update(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf)
{
	const char *err_oper = NULL;
	char *cmp_buf;
	const char *end = buf + len;
	size_t todo;		/* number of bytes to do in this pass */
	size_t skipped = 0;	/* statistics */
	const ulong start_time = get_timer(0);
	size_t scale = 1;
	const char *start_buf = buf;
	ulong delta;

	if (end - buf >= 200)
		scale = (end - buf) / 100;
	cmp_buf = malloc(flash->sector_size);
	if (cmp_buf) {
		ulong last_update = get_timer(0);

		for (; buf < end && !err_oper; buf += todo, offset += todo) {
			todo = min_t(size_t, end - buf, flash->sector_size);
			if (get_timer(last_update) > 100) {
				printf("   \rUpdating, %zu%% %lu B/s",
				       100 - (end - buf) / scale,
					bytes_per_second(buf - start_buf,
							 start_time));
				last_update = get_timer(0);
			}
			err_oper = spi_flash_update_block(flash, offset, todo,
					buf, cmp_buf, &skipped);
		}
	} else {
		err_oper = "malloc";
	}
	free(cmp_buf);
	putc('\r');
	if (err_oper) {
		printf("SPI flash failed in %s step\n", err_oper);
		return 1;
	}

	delta = get_timer(start_time);
	printf("%zu bytes written, %zu bytes skipped", len - skipped,
	       skipped);
	printf(" in %ld.%lds, speed %ld B/s\n",
	       delta / 1000, delta % 1000, bytes_per_second(len, start_time));

	return 0;
}

static int do_spi_flash_read_write(int argc, char * const argv[])
{
	unsigned long addr;
	unsigned long offset;
	unsigned long len;
	void *buf;
	char *endp;
	int ret = 1;

	if (argc < 4)
		return -1;

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	offset = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		return -1;
	len = simple_strtoul(argv[3], &endp, 16);
	if (*argv[3] == 0 || *endp != 0)
		return -1;

	/* Consistency checking */
	if (offset + len > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return 1;
	}

	buf = map_physmem(addr, len, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	if (strcmp(argv[0], "update") == 0) {
		ret = spi_flash_update(flash, offset, len, buf);
	} else if (strncmp(argv[0], "read", 4) == 0 ||
			strncmp(argv[0], "write", 5) == 0) {
		int read;

		read = strncmp(argv[0], "read", 4) == 0;
		if (read)
			ret = spi_flash_read(flash, offset, len, buf);
		else
			ret = spi_flash_write(flash, offset, len, buf);

		printf("SF: %zu bytes @ %#x %s: %s\n", (size_t)len, (u32)offset,
		       read ? "Read" : "Written", ret ? "ERROR" : "OK");
	}

	unmap_physmem(buf, len);

	return ret == 0 ? 0 : 1;
}

static int do_spi_flash_erase(int argc, char * const argv[])
{
	unsigned long offset;
	unsigned long len;
	char *endp;
	int ret;

	if (argc < 3)
		return -1;

	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	ret = sf_parse_len_arg(argv[2], &len);
	if (ret != 1)
		return -1;

	/* Consistency checking */
	if (offset + len > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return 1;
	}

	ret = spi_flash_erase(flash, offset, len);
	printf("SF: %zu bytes @ %#x Erased: %s\n", (size_t)len, (u32)offset,
	       ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

#ifdef CONFIG_CMD_SF_TEST
enum {
	STAGE_ERASE,
	STAGE_CHECK,
	STAGE_WRITE,
	STAGE_READ,

	STAGE_COUNT,
};

static char *stage_name[STAGE_COUNT] = {
	"erase",
	"check",
	"write",
	"read",
};

struct test_info {
	int stage;
	int bytes;
	unsigned base_ms;
	unsigned time_ms[STAGE_COUNT];
};

static void show_time(struct test_info *test, int stage)
{
	uint64_t speed;	/* KiB/s */
	int bps;	/* Bits per second */

	speed = (long long)test->bytes * 1000;
	if (test->time_ms[stage])
		do_div(speed, test->time_ms[stage] * 1024);
	bps = speed * 8;

	printf("%d %s: %d ticks, %d KiB/s %d.%03d Mbps\n", stage,
	       stage_name[stage], test->time_ms[stage],
	       (int)speed, bps / 1000, bps % 1000);
}

static void spi_test_next_stage(struct test_info *test)
{
	test->time_ms[test->stage] = get_timer(test->base_ms);
	show_time(test, test->stage);
	test->base_ms = get_timer(0);
	test->stage++;
}

/**
 * Run a test on the SPI flash
 *
 * @param flash		SPI flash to use
 * @param buf		Source buffer for data to write
 * @param len		Size of data to read/write
 * @param offset	Offset within flash to check
 * @param vbuf		Verification buffer
 * @return 0 if ok, -1 on error
 */
static int spi_flash_test(struct spi_flash *flash, uint8_t *buf, ulong len,
			   ulong offset, uint8_t *vbuf)
{
	struct test_info test;
	int i;

	printf("SPI flash test:\n");
	memset(&test, '\0', sizeof(test));
	test.base_ms = get_timer(0);
	test.bytes = len;
	if (spi_flash_erase(flash, offset, len)) {
		printf("Erase failed\n");
		return -1;
	}
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("Check read failed\n");
		return -1;
	}
	for (i = 0; i < len; i++) {
		if (vbuf[i] != 0xff) {
			printf("Check failed at %d\n", i);
			print_buffer(i, vbuf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			return -1;
		}
	}
	spi_test_next_stage(&test);

	if (spi_flash_write(flash, offset, len, buf)) {
		printf("Write failed\n");
		return -1;
	}
	memset(vbuf, '\0', len);
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("Read failed\n");
		return -1;
	}
	spi_test_next_stage(&test);

	for (i = 0; i < len; i++) {
		if (buf[i] != vbuf[i]) {
			printf("Verify failed at %d, good data:\n", i);
			print_buffer(i, buf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			printf("Bad data:\n");
			print_buffer(i, vbuf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			return -1;
		}
	}
	printf("Test passed\n");
	for (i = 0; i < STAGE_COUNT; i++)
		show_time(&test, i);

	return 0;
}

static int do_spi_flash_test(int argc, char * const argv[])
{
	unsigned long offset;
	unsigned long len;
	uint8_t *buf, *from;
	char *endp;
	uint8_t *vbuf;
	int ret;

	if (argc < 3)
		return -1;
	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	len = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		return -1;

	vbuf = malloc(len);
	if (!vbuf) {
		printf("Cannot allocate memory (%lu bytes)\n", len);
		return 1;
	}
	buf = malloc(len);
	if (!buf) {
		free(vbuf);
		printf("Cannot allocate memory (%lu bytes)\n", len);
		return 1;
	}

	from = map_sysmem(CONFIG_SYS_TEXT_BASE, 0);
	memcpy(buf, from, len);
	ret = spi_flash_test(flash, buf, len, offset, vbuf);
	free(vbuf);
	free(buf);
	if (ret) {
		printf("Test failed\n");
		return 1;
	}

	return 0;
}
#endif /* CONFIG_CMD_SF_TEST */

static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *cmd;
	int ret;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "probe") == 0) {
		ret = do_spi_flash_probe(argc, argv);
		goto done;
	}

	/* The remaining commands require a selected device */
	if (!flash) {
		puts("No SPI flash selected. Please run `sf probe'\n");
		return 1;
	}

	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0 ||
	    strcmp(cmd, "update") == 0)
		ret = do_spi_flash_read_write(argc, argv);
	else if (strcmp(cmd, "erase") == 0)
		ret = do_spi_flash_erase(argc, argv);
#ifdef CONFIG_CMD_SF_TEST
	else if (!strcmp(cmd, "test"))
		ret = do_spi_flash_test(argc, argv);
#endif
	else
		ret = -1;

done:
	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_CMD_SF_TEST
#define SF_TEST_HELP "\nsf test offset len		" \
		"- run a very basic destructive test"
#else
#define SF_TEST_HELP
#endif

U_BOOT_CMD(
	sf,	5,	1,	do_spi_flash,
	"SPI flash sub-system",
	"probe [[bus:]cs] [hz] [mode]	- init flash device on given SPI bus\n"
	"				  and chip select\n"
	"sf read addr offset len	- read `len' bytes starting at\n"
	"				  `offset' to memory at `addr'\n"
	"sf write addr offset len	- write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'\n"
	"sf erase offset [+]len		- erase `len' bytes from `offset'\n"
	"				  `+len' round up `len' to block size\n"
	"sf update addr offset len	- erase and write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'"
	SF_TEST_HELP
);

#ifdef CONFIG_CMD_MSTAR_SF


DECLARE_GLOBAL_DATA_PTR;

#include <linux/list.h>

#define MTDIDS_MAXLEN		128
#define MTDPARTS_MAXLEN		512


/* special size referring to all the remaining space in a partition */
#define SIZE_REMAINING		(~0llu)

/* special offset value, it is used when not provided by user
 *
 * this value is used temporarily during parsing, later such offests
 * are recalculated */
#define OFFSET_NOT_SPECIFIED	(~0llu)

/* minimum partition size */
#define MIN_PART_SIZE		4096

/* this flag needs to be set in part_info struct mask_flags
 * field for read-only partitions */
#define MTD_WRITEABLE_CMD		1

#define SF_MTD_ID     "nor0"
#define SF_BOOT  "BOOT"

LIST_HEAD(sf_part_list);

//struct mtd_device {
//	struct list_head link;
//	struct mtdids *id;		/* parent mtd id entry */
//	u16 num_parts;			/* number of partitions on this device */
//	struct list_head parts;		/* partitions */
//};

struct part_info {
	struct list_head link;
	char *name;			/* partition name */
	u8 auto_name;			/* set to 1 for generated name */
	u64 size;			/* total size of the partition */
	u64 offset;			/* offset within device */
	void *jffs2_priv;		/* used internaly by jffs2 */
	u32 mask_flags;			/* kernel MTD mask flags */
	u32 sector_size;		/* size of sector */
	struct mtd_device *dev;		/* parent device */
};

struct mtdids {
	struct list_head link;
	u8 type;			/* device type */
	u8 num;				/* device number */
	u64 size;			/* device size */
	char *mtd_id;			/* linux kernel device id */
};


/**
 * Parses a string into a number.  The number stored at ptr is
 * potentially suffixed with K (for kilobytes, or 1024 bytes),
 * M (for megabytes, or 1048576 bytes), or G (for gigabytes, or
 * 1073741824).  If the number is suffixed with K, M, or G, then
 * the return value is the number multiplied by one kilobyte, one
 * megabyte, or one gigabyte, respectively.
 *
 * @param ptr where parse begins
 * @param retptr output pointer to next char after parse completes (output)
 * @return resulting unsigned int
 */
static u64 memsize_parse (const char *const ptr, const char **retptr)
{
	u64 ret = simple_strtoull(ptr, (char **)retptr, 0);

	switch (**retptr) {
		case 'G':
		case 'g':
			ret <<= 10;
		case 'M':
		case 'm':
			ret <<= 10;
		case 'K':
		case 'k':
			ret <<= 10;
			(*retptr)++;
		default:
			break;
	}

	return ret;
}



/**
 * Parse one partition definition, allocate memory and return pointer to this
 * location in retpart.
 *
 * @param partdef pointer to the partition definition string i.e. <part-def>
 * @param ret output pointer to next char after parse completes (output)
 * @param retpart pointer to the allocated partition (output)
 * @return 0 on success, 1 otherwise
 */
static int part_parse(const char *const partdef, const char **ret, struct part_info **retpart)
{
	struct part_info *part;
	u64 size;
	u64 offset;
	const char *name;
	int name_len;
	unsigned int mask_flags;
	const char *p;

	p = partdef;
	*retpart = NULL;
	*ret = NULL;

	/* fetch the partition size */
	if (*p == '-') {
		/* assign all remaining space to this partition */
		debug("'-': remaining size assigned\n");
		size = SIZE_REMAINING;
		p++;
	} else {
		size = memsize_parse(p, &p);
		if (size < MIN_PART_SIZE) {
			printf("partition size too small (%llx)\n", size);
			return 1;
		}
	}

	/* check for offset */
	offset = OFFSET_NOT_SPECIFIED;
	if (*p == '@') {
		p++;
		offset = memsize_parse(p, &p);
	}

	/* now look for the name */
	if (*p == '(') {
		name = ++p;
		if ((p = strchr(name, ')')) == NULL) {
			printf("no closing ) found in partition name\n");
			return 1;
		}
		name_len = p - name + 1;
		if ((name_len - 1) == 0) {
			printf("empty partition name\n");
			return 1;
		}
		p++;
	} else {
		/* 0x00000000@0x00000000 */
		name_len = 22;
		name = NULL;
	}

	/* test for options */
	mask_flags = 0;
	if (strncmp(p, "ro", 2) == 0) {
		mask_flags |= MTD_WRITEABLE_CMD;
		p += 2;
	}

	/* check for next partition definition */
	if (*p == ',') {
		if (size == SIZE_REMAINING) {
			*ret = NULL;
			printf("no partitions allowed after a fill-up partition\n");
			return 1;
		}
		*ret = ++p;
	} else if ((*p == ';') || (*p == '\0')) {
		*ret = p;
	} else {
		printf("unexpected character '%c' at the end of partition\n", *p);
		*ret = NULL;
		return 1;
	}

	/*  allocate memory */
	part = (struct part_info *)malloc(sizeof(struct part_info) + name_len);
	if (!part) {
		printf("out of memory\n");
		return 1;
	}
	memset(part, 0, sizeof(struct part_info) + name_len);
	part->size = size;
	part->offset = offset;
	part->mask_flags = mask_flags;
	part->name = (char *)(part + 1);

	if (name) {
		/* copy user provided name */
		strncpy(part->name, name, name_len - 1);
		part->auto_name = 0;
	} else {
		/* auto generated name in form of size@offset */
		sprintf(part->name, "0x%08llx@0x%08llx", size, offset);
		part->auto_name = 1;
	}

	part->name[name_len - 1] = '\0';
	INIT_LIST_HEAD(&part->link);


	*retpart = part;
	return 0;
}


/**
 * Delete all partitions from parts head list, free memory.
 *
 * @param head list of partitions to delete
 */
static void part_delall(struct list_head *head)
{
	struct list_head *entry, *n;
	struct part_info *part_tmp;

	/* clean tmp_list and free allocated memory */
	list_for_each_safe(entry, n, head) {
		part_tmp = list_entry(entry, struct part_info, link);

		list_del(entry);
		free(part_tmp);
	}
}


extern int ms_get_spi_env_offset(void);
/**
 * Performs sanity check for supplied partition. Offset and size are
 * verified to be within valid range. Partition type is checked and
 * part_validate_eraseblock() is called with the argument of part.
 *
 * @param id of the parent device
 * @param part partition to validate
 * @return 0 if partition is valid, 1 otherwise
 */
static int part_validate(struct part_info *part)
{

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	int env_offset=0;

	if (part->size == SIZE_REMAINING)
		part->size = flash->size - part->offset;

	env_offset=ms_get_spi_env_offset();

	if(0!=strncmp(SF_BOOT,part->name,sizeof(SF_BOOT)-1))
	{
		//Not a reserved region
		if(part->offset < (env_offset+CONFIG_ENV_SIZE))
		{
			printf("partition (%s) offset: 0x%08llx before RESERVED region\n",
							part->name, part->offset);
			return 1;
		}

	}
#endif
	if (part->offset > flash->size) {
		printf("partition (%s) offset: 0x%08llx beyond flash size: 0x%08llx\n",
				part->name, part->offset, (long long unsigned)flash->size);
		return 1;
	}

	if ((part->offset + part->size) <= part->offset) {
		printf("partition (%s) size too big\n", part->name);
		return 1;
	}

	if (part->offset + part->size > flash->size) {
		printf("partition (%s) offset: 0x%08llx size: 0x%08llx exceeds flash size: 0x%08llx\n", part->name,part->offset,(long long unsigned)part->size,(long long unsigned)flash->size);
		return 1;
	}


	return 0;
}


/**
 * Parse device type, name and mtd-id. If syntax is ok allocate memory and
 * return pointer to the device structure.
 *
 * @param mtd_dev pointer to the device definition string i.e. <mtd-dev>
 * @param ret output pointer to next char after parse completes (output)
 * @param retdev pointer to the allocated device (output)
 * @return 0 on success, 1 otherwise
 */
static int device_parse(const char *const mtd_dev, const char **ret /*,struct mtd_device **retdev*/)
{
//	struct mtd_device *dev;
	struct part_info *part;
	const char *mtd_id;
	unsigned int mtd_id_len;
	const char *p;
//	const char *pend;

//	struct list_head *entry, *n;
	u16 num_parts;
	u64 offset;
	int err = 1;

	printf("===device_parse===\n");

	if (ret)
		*ret = NULL;

	/* fetch <mtd-id> */
	mtd_id = p = mtd_dev;
	if (!(p = strchr(mtd_id, ':'))) {
		printf("no <mtd-id> identifier\n");
		return 1;
	}
	mtd_id_len = p - mtd_id ;
	if(0!=memcmp(mtd_id,flash->name,mtd_id_len))
	{
		printf("Can not found SF: %s\n",flash->name);
		return 1;
	}



	p++;

	/* parse partitions */
	num_parts = 0;
	offset = 0;


	while (p && (*p != '\0') && (*p != ';')) {
		err = 1;
		if ((part_parse(p, &p, &part) != 0) || (!part))
			break;

		/* calculate offset when not specified */
		if (part->offset == OFFSET_NOT_SPECIFIED)
			part->offset = offset;
		else
			offset = part->offset;

		/* verify alignment and size */
		if (part_validate(part) != 0)
			break;

		offset += part->size;

		debug("+ partition: name %-22s size 0x%08llx offset 0x%08llx\n",
					part->name, part->size,
					part->offset);


		/* partition is ok, add it to the list */
		list_add_tail(&part->link, &sf_part_list);
		num_parts++;
		err = 0;
	}
	if (err == 1) {
		part_delall(&sf_part_list);
		return 1;
	}

	if (num_parts == 0) {
		printf("no partitions for device %s\n",flash->name);
		return 1;
	}

	printf("\ntotal partitions: %d\n", num_parts);


	printf("===\n\n");
	return 0;
}

/**
 * Accept character string describing mtd partitions and call device_parse()
 * for each entry. Add created devices to the global devices list.
 *
 * @param mtdparts string specifing mtd partitions
 * @return 0 on success, 1 otherwise
 */
static int parse_mtdparts(const char *const mtdparts)
{
	const char *p = mtdparts;
	int err = 1;
	char tmp_parts[MTDPARTS_MAXLEN];

//	debug("\n---parse_mtdparts---\nmtdparts = %s\n\n", p);
//
//	/* delete all devices and partitions */
//	if (mtd_devices_init() != 0) {
//		printf("could not initialise device list\n");
//		return err;
//	}

	/* re-read 'mtdparts' variable, mtd_devices_init may be updating env */
	if (gd->flags & GD_FLG_ENV_READY) {
		p = getenv("mtdparts");
	} else {
		p = tmp_parts;
		getenv_f("mtdparts", tmp_parts, MTDPARTS_MAXLEN);
	}

	if (strncmp(p, "mtdparts=", 9) != 0) {
		printf("mtdparts variable doesn't start with 'mtdparts='\n");
		return err;
	}
	p += 9;

	while (p && (*p != '\0')) {
		err = 1;
		if (device_parse(p, &p) != 0)
			break;

		err = 0;
	}



	return 0;
}


static int do_mstar_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc,	char * const argv[])
{
	const char *cmd;
	int ret=1;
	char *p, *endp;
	char tmp_parts[MTDPARTS_MAXLEN];

	unsigned long addr;
	unsigned long offset=0xFFFF0000;
	unsigned long len=0;

	void *buf;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

//	if (strcmp(cmd, "probe") == 0) {
//		ret = do_spi_flash_probe(argc, argv);
//		goto done;
//	}

	/* The remaining commands require a selected device */
	if (!flash) {
		puts("No SPI flash selected. Please run `sf probe'\n");
		return 1;
	}

	{
		struct list_head *pentry;
		char *argv_name=argv[3];
		int printp=0;
		if (strcmp(cmd, "erase")  == 0)
		{
			argv_name=argv[2];
		}
		else if (strcmp(cmd, "print")  == 0)
		{
			printp=1;
		}


		/* re-read 'mtdparts' variable, mtd_devices_init may be updating env */
		if (gd->flags & GD_FLG_ENV_READY) {
			p = getenv("mtdparts");
		} else {
			p = tmp_parts;
			getenv_f("mtdparts", tmp_parts, MTDPARTS_MAXLEN);
		}

		if (strncmp(p, "mtdparts=", 9) != 0) {
			printf("mtdparts variable doesn't start with 'mtdparts='\n");
			return 1;
		}
		p += 9;

		if(0!=parse_mtdparts((const char*)p))
		{
			printf("parse mtdparts string error!! string=%s\n",p);
			return 1;
		}

		list_for_each(pentry, &sf_part_list) {
			struct part_info *part = list_entry(pentry, struct part_info, link);
//			printf("cmp=%s, %s\n",part->name,argv[2]);
			if(printp)
			{
				printf("+ partition: name %-22s size 0x%08llx offset 0x%08llx\n",
							part->name, part->size,
							part->offset);

			}
			else if (strcmp(part->name, argv_name) == 0)
			{
				offset=part->offset;
				len=part->size;
				goto PART_FOUND;
			}

		}
		part_delall(&sf_part_list);
		if(printp)
		{
			//done
			return 0;
		}
		printf("unable to found part %s\n",argv[2]);
		return 1;


	}

PART_FOUND:
	part_delall(&sf_part_list);

//	void *buf;




	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0) {
		int read;
		int argv_len;

		addr=simple_strtoul(argv[2], &endp, 16);

		if(argc>4)
		{
			argv_len = simple_strtoul(argv[4], &endp, 16);
			len= (len > argv_len )? argv_len : len;
		}

		buf = map_physmem(addr, len, MAP_WRBACK);
		if (!buf) {
			puts("Failed to map physical memory\n");
			return 1;
		}

		read = (strcmp(cmd, "read") == 0);
		if (read)
			ret = spi_flash_read(flash, offset, len, buf);
		else
			ret = spi_flash_write(flash, offset, len, buf);

		unmap_physmem(buf, len);
		printf("SF: %zu bytes @ %#x %s: %s\n", (size_t)len, (u32)offset,
		       read ? "Read" : "Written", ret ? "ERROR" : "OK");
	}
	else if (strcmp(cmd, "erase")  == 0)
	{
		ret = spi_flash_erase(flash, offset, len);
	}
	else
	{
		return -1;
	}




	return ret == 0 ? 0 : 1;

usage:
	return CMD_RET_USAGE;
}



U_BOOT_CMD(
	msf,	5,	1,	do_mstar_spi_flash,
	"SPI flash mtdparts sub-system",
	"msf read addr partition len	- read `len' bytes starting at\n"
	"				  `partition' to memory at `addr'\n"
	"msf write addr partition len	- write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'\n"
	"msf erase partition		- erase ` partition'\n"
	"msf print         	    	- print parsed ` partition'\n"
);


#endif
