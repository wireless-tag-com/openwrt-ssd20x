#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>

#define WT_INFO_TOTAL_LEN		0x10000
#define WT_INFO_MTD_NAME			"wtinfo"

#define WT_MAGIC_LEN		6
#define WT_VER_LEN		4
#define WT_MD5_LEN		16
#define WT_MAC_LEN		6

static u_char wt_fac_mac_addr[WT_MAC_LEN];

typedef struct _wt_info {
	unsigned char magic[WT_MAGIC_LEN];
	unsigned char ver[WT_VER_LEN];
	unsigned char mac[WT_MAC_LEN];
	unsigned char md5[WT_MD5_LEN];
} WT_INFO;

static int __wt_get_mac(const WT_INFO *winfo, u_char *mac)
{
	const u8* p = (u8 *)winfo;

	if ((*((u32 *)p) == 0xffffffff) 
		|| (*((u32 *)p) == 0x0)) {
		printk(KERN_NOTICE "winfo: fac txt empty, use default.");
		return -1;
	}

	if ((winfo->magic[0] != 'W') &&
		(winfo->magic[1] != 'T') &&
		(winfo->magic[2] != 'I') &&
		(winfo->magic[3] != 'N') &&
		(winfo->magic[4] != 'F') &&
		(winfo->magic[5] != 'O')) {
		printk(KERN_NOTICE "wtinfo: magic error.");
		return -1;
	}

	memcpy(mac, winfo->mac, WT_MAC_LEN);

	return 0;
}

void wt_get_mac(void *mac)
{
	if (mac) {
		memcpy(mac, wt_fac_mac_addr, sizeof(wt_fac_mac_addr));
	}
}

static void wt_add_mtd_notifier(struct mtd_info *mtd)
{
	u_char *winfo;

	if (strncmp(mtd->name, WT_INFO_MTD_NAME, strlen(WT_INFO_MTD_NAME)))
		return;

	winfo = (u_char *) kmalloc(WT_INFO_TOTAL_LEN, GFP_KERNEL);
	if (winfo) {
		size_t retlen;

		mtd_read(mtd, 0, WT_INFO_TOTAL_LEN, &retlen, winfo);
		__wt_get_mac((const WT_INFO *)winfo, (u_char *)wt_fac_mac_addr);
		kfree(winfo);
	} else {
		printk(KERN_NOTICE "wtinfo: %s alloc 0x%llx memory failed\n", __func__, mtd->size);
	}
}

static void wt_remove_mtd_notifier(struct mtd_info *mtd)
{
	return;
}

static struct mtd_notifier winfo_mtd_notifier = {
	.add	= wt_add_mtd_notifier,
	.remove = wt_remove_mtd_notifier,
};

int wt_info_init(void)
{
	wt_fac_mac_addr[0] = 0x00;
	wt_fac_mac_addr[1] = 0x86;
	wt_fac_mac_addr[2] = 0x88;
	wt_fac_mac_addr[3] = 0x00;
	wt_fac_mac_addr[4] = 0x00;
	wt_fac_mac_addr[5] = 0x00;

	register_mtd_user(&winfo_mtd_notifier);

	return 0;
}

void wt_info_exit(void)
{
	unregister_mtd_user(&winfo_mtd_notifier);
}

module_init(wt_info_init);
module_exit(wt_info_exit);

EXPORT_SYMBOL(wt_get_mac);

MODULE_LICENSE("GPL"); 
