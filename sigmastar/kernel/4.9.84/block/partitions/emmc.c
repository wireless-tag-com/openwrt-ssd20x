/*
 *  fs/partitions/emmc.c
 *
 *  Code extracted from drivers/block/genhd.c
 *  Copyright (C) 1991-1998  Linus Torvalds
 *  Re-organised Feb 1998 Russell King
 */

#include <linux/ctype.h>
#include "check.h"
#include "emmc.h"

int emmc_partition(struct parsed_partitions *state)
{
    int slot = 1;
    Sector sect;
    unsigned char *data;
    int blk, blocks_in_map;
    unsigned secsize;
    struct emmc_partition *part;
    struct emmc_driver_desc *md;
       printk("emmc_partition()\n");
    /* Get 0th block and look at the first partition map entry. */
    md = read_part_sector(state, 0, &sect);
    if (!md)
        return -1;

    if (md->signature != EMMC_DRIVER_MAGIC) {
            //can not found the partiton map!
            printk("0x%x\n",md->signature);
            put_dev_sector(sect);
            return 0;
    }

    if(md->version == EMMC_PARTITIONTABLE_VERSION2 || md->version == EMMC_PARTITIONTABLE_VERSION3)
        blocks_in_map = md->drvr_cnt;
    else
        blocks_in_map = EMMC_RESERVED_FOR_MAP;

    //  secsize = be16_to_cpu(md->block_size);
       secsize = 512;
    put_dev_sector(sect);
    data = read_part_sector(state, secsize/512, &sect);
    if (!data)
        return -1;
    part = (struct emmc_partition *) (data + secsize%512);
    if (part->signature != EMMC_PARTITION_MAGIC) {
        put_dev_sector(sect);
        return 0;		/* not a emmc disk */
    }
    printk(" [emmc]");
    for (blk = 1; blk <= blocks_in_map; ++blk) {
        int pos = blk * secsize;
        put_dev_sector(sect);
        data = read_part_sector(state, pos/512, &sect);
        if (!data)
            return -1;
        part = (struct emmc_partition *) (data + pos%512);
        if (part->signature != EMMC_PARTITION_MAGIC)
            break;
             printk("Start_block=%d, block_count=%d\n",part->start_block,part->block_count);
        put_partition(state, slot,
            (part->start_block) * (secsize/512),
            (part->block_count) * (secsize/512));
        strcpy(state->parts[slot].info.volname, part->name); /* put parsed partition name into state */

        if (!strncasecmp(part->type, "Linux_RAID", 10))
            state->parts[slot].flags = ADDPART_FLAG_RAID;

        ++slot;
    }

    put_dev_sector(sect);
    printk("\n");
    return 1;
}
