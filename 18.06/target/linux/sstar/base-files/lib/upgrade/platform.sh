#/bin/sh

. /lib/sstar.sh

RAMFS_COPY_DATA=/lib/sstar.sh

UPGRADE_UBI_STATUS=1

platform_check_image() {
	return 0
}

wt_nand_upgrade_ubi() {
	local ubi_name="$1"
	local file_name="$2"

	local mtdnum="$(find_mtd_index "${ubi_name}")"

	if [ ! "$mtdnum" ]; then
		UPGRADE_UBI_STATUS=0
		return
	fi

	local mtddev="/dev/mtd${mtdnum}"
	ubidetach -f -p "${mtddev}"
	sync
	ubiformat "${mtddev}" -y -f "${file_name}"

	if [ $? -eq 0 ]; then
		sync

		UPGRADE_UBI_STATUS=0
		if [ "$SAVE_CONFIG" -eq 1 ]; then
			mkdir -p /tmp/new_overlay
			ubiattach -p "${mtddev}"
			local ubi_index=$(nand_find_ubi "$ubi_name")
			mount -t ubifs $ubi_index:rootfs_data /tmp/new_overlay
			if [ $? -eq 0 ]; then
				cp /tmp/sysupgrade.tgz /tmp/new_overlay/
				sync
				umount /tmp/new_overlay
				UPGRADE_UBI_STATUS=0
			else
				UPGRADE_UBI_STATUS=1
			fi
			ubidetach -f -p "${mtddev}"
		fi
	else
		UPGRADE_UBI_STATUS=1
	fi
}

wt_nand_upgrade() {
	local tar_file="$1"
	local board_dir=$(tar tzf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	if [ -n "${board_dir}" ]; then
		rm -rf /tmp/${board_dir}

		tar xzf $tar_file ${board_dir}/root -C /tmp

		rm -rf $tar_file
		sync

		echo 3 >/proc/sys/vm/drop_caches

		if [ $? -eq 0 ]; then
			UPGRADE_UBI_STATUS=1
			wt_nand_upgrade_ubi "ubi2" /tmp/${board_dir}/root

			if [ $UPGRADE_UBI_STATUS -eq 0 ]; then
				UPGRADE_UBI_STATUS=1
				wt_nand_upgrade_ubi "ubi" /tmp/${board_dir}/root
			fi
		fi
	fi
}

wt_nor_upgrade()
{
	if [ "$SAVE_CONFIG" -eq 1 ]; then
		dd if="$1" bs=2048k count=1 conv=sync 2>/dev/null | mtd write - KERNEL
		dd if="$1" bs=2048k skip=1 conv=sync 2>/dev/null | mtd -j "$CONF_TAR" write - rootfs
	else
		dd if="$1" bs=2048k count=1 conv=sync 2>/dev/null | mtd write - KERNEL
		dd if="$1" bs=2048k skip=1 conv=sync 2>/dev/null | mtd write - rootfs
	fi
}

platform_pre_upgrade() {
	/etc/init.d/sstar-wifi stop
	/usr/sbin/sstar-mpp stop
}

platform_do_upgrade() {
	cat /proc/mtd | grep ubi

	if [ $? -eq 0 ]; then
		wt_nand_upgrade "$1"
	else
		wt_nor_upgrade "$1"
	fi
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog
