#!/bin/sh

SSTAR_BOARD_NAME=

sstar_board_detect() {
	local cmd_board_name=$(cat /proc/cmdline | awk $'{print $1}')
	local board_name=${cmd_board_name##wt_board=}

	mkdir -p /tmp/sysinfo
	echo "$board_name" >/tmp/sysinfo/board_name

	[ ! -e /tmp/sysinfo/model -a -e /proc/device-tree/model ] && \
		echo "$(cat /proc/device-tree/model)" > /tmp/sysinfo/model
}

sstar_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}
