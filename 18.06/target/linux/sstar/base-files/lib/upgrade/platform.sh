#/bin/sh

. /lib/sstar.sh

RAMFS_COPY_DATA=/lib/sstar.sh

platform_check_image() {
	return 0
}

wt_nand_upgrade() {
	local tar_file="$1"
	local board_dir=$(tar tzf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	if [ -n "${board_dir}" ]; then
		rm -rf /tmp/${board_dir}
		tar xzf $tar_file ${board_dir}/kernel -C /tmp

		export CI_KERNPART="KERNEL"
		export CI_UBIPART="ubi"

		mtd write /tmp/${board_dir}/kernel KERNEL
		mtd write /tmp/${board_dir}/kernel RECOVERY
		rm -rf /tmp/${board_dir}/kernel

		tar xzf $tar_file ${board_dir}/root -C /tmp

		local mtdnum="$(find_mtd_index "$CI_UBIPART")"

		if [ ! "$mtdnum" ]; then
			echo "cannot find mtd device $CI_UBIPART"
			umount -a
			reboot -f
		fi

		local mtddev="/dev/mtd${mtdnum}"
		ubidetach -f -p "${mtddev}" || true
		sync
		ubiformat "${mtddev}" -y -f /tmp/${board_dir}/root
		sync

		if [ "$SAVE_CONFIG" -eq 1 ]; then
			mkdir -p /tmp/new_overlay
			ubiattach -p "${mtddev}"
			echo "save config"
			mount -t ubifs ubi0:rootfs_data /tmp/new_overlay
			cp /tmp/sysupgrade.tgz /tmp/new_overlay/
			sync
			umount /tmp/new_overlay
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
