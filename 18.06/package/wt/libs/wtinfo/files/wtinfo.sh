#!/bin/sh

wtinfo_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}

wtinfo_get_mac() {
	local fac_mac=$(wtinfo -g mac)

	if [ -n "$fac_mac" ]; then
		echo -n "$fac_mac"
	else
		echo -n "00:86:88:00:00:00"
	fi
}

wtinfo_macaddr_add() {
	local mac=$1
	local val=$2
	local oui=${mac%:*:*:*}
	local nic=${mac#*:*:*:}

	nic=$(printf "%06X" $((0x${nic//:/} + $val & 0xffffff)) | sed 's/^\(.\{2\}\)\(.\{2\}\)\(.\{2\}\)/\1:\2:\3/')
	echo $oui:$nic
}

wtinfo_get_wan_mac() {
	local fac_mac=$(wtinfo -g mac)

	if [ -n "$fac_mac" ]; then
		local wan_mac=$(wtinfo_macaddr_add $fac_mac 1)
		echo -n "$wan_mac"
	else
		echo -n "00:88:88:00:00:01"
	fi
}

wtinfo_get_country() {
	local fac_country=$(wtinfo -g country)

	if [ -n "$fac_country" ]; then
		echo -n "$fac_country"
	else
		echo -n "CN"
	fi
}

wtinfo_get_board() {
	local fac_board=$(wtinfo -g board)

	if [ -n "$fac_board" ]; then
		echo -n "$fac_board"
	else
		wtinfo_board_name
	fi
}
