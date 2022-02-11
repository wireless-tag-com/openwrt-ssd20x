/*
 *  fs/partitions/emmc.h
 */

#define EMMC_PARTITION_MAGIC  0x5840
#define EMMC_RESERVED_FOR_MAP 64		//keep the same with uboot

/* type field value for A/UX or other Unix partitions */
//#define APPLE_AUX_TYPE	"Apple_UNIX_SVR2"

struct emmc_partition {
    __be16	signature;	/* expected to be MAC_PARTITION_MAGIC */
    __be16	res1;
    __be32	map_count;	/* # blocks in partition map */
    __be32	start_block;	/* absolute starting block # of partition */
    __be32	block_count;	/* number of blocks in partition */
    char	name[32];	/* partition name */
    char	type[32];	/* string type description */
    __be32	data_start;	/* rel block # of first data block */
    __be32	data_count;	/* number of data blocks */
    __be32	status;		/* partition status bits */
    __be32	boot_start;
    __be32	boot_size;
    __be32	boot_load;
    __be32	boot_load2;
    __be32	boot_entry;
    __be32	boot_entry2;
    __be32	boot_cksum;
    char	processor[16];	/* identifies ISA of boot */
    /* there is more stuff after this that we don't need */
};

#define EMMC_STATUS_BOOTABLE	8	/* partition is bootable */

#define EMMC_DRIVER_MAGIC 0x1630
#define EMMC_PARTITIONTABLE_VERSION2 0x2000
#define EMMC_PARTITIONTABLE_VERSION3 0x4000


/* Driver descriptor structure, in block 0 */
struct emmc_driver_desc {
    __be16   signature;           /* expected to be EMMC_DRIVER_MAGIC  */
    __be16   block_size;             /* block size of device */
    __be32   block_count;           /* number of blocks on device */
    __be16   dev_type;            /* device type */
    __be16   dev_id;                /* device id */
    __be32   data;                   /* reserved */
    __be16   version;               /* version number of partition table */
    __be16   drvr_cnt;             /* number of blocks reserved for partition table */
    /* ... more stuff */
};

int emmc_partition(struct parsed_partitions *state);
