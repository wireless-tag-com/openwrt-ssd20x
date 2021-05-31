#!/bin/sh
append DRIVERS "esp"

scan_esp() {
	return 0
}

disable_esp() {
	local device="$1"

	killall hostapd
	killall wpa_supplicant
	sleep 1
	ifconfig wlan0 0.0.0.0

	set_wifi_down "$device"
}

enable_esp() {
	local device="$1"

	wifi_fixup_hwmode $device

	config_get hwmode "$device" hwmode g
	config_get channel "$device" channel 0

	config_get vifs "$device" vifs

	# bring up vifs
	for vif in $vifs; do
		config_get_bool disabled "$vif" disabled 0
		[ $disabled -eq 0 ] || continue

		config_get ifname "$vif" ifname
		config_get ssid "$vif" ssid
		config_get encryption "$vif" encryption
		config_get key "$vif" key
		config_get mode "$vif" mode

		local net_cfg bridge
		net_cfg="$(find_net_config "$vif")"
		[ -z "$net_cfg" ] || {
			bridge="$(bridge_interface "$net_cfg")"
		}

		if [ "$mode" == "ap" ]; then
			local hostapd_conf="/var/run/hostapd-$ifname.conf"
			config_get hidden "$vif" hidden 0

			rm -rf "$hostapd_conf"

			if [ "$encryption" == "none" ]; then
			cat >> "$hostapd_conf" <<EOF
interface=$ifname
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
driver=nl80211
ssid=$ssid
hw_mode=$hwmode
channel=$channel
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=$hidden

EOF

			else
			cat >> "$hostapd_conf" <<EOF
interface=$ifname
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
driver=nl80211
ssid=$ssid
hw_mode=$hwmode
channel=$channel
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=$hidden
wpa=3
wpa_passphrase=$key
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

EOF

			fi

			ifconfig "$ifname" up
			ifconfig "$ifname" 0.0.0.0
			/usr/sbin/hostapd -s -P /var/run/wifi-$ifname.pid -B "$hostapd_conf"
		else
			local wpa_supplicant_conf="/var/run/wpa_supplicant-$ifname.conf"

			rm -rf "$wpa_supplicant_conf"

			if [ "$encryption" == "none" ]; then
	cat >> "$wpa_supplicant_conf" <<EOF
ctrl_interface=/var/run/wpa_supplicant
update_config=1

network={
	scan_ssid=1
	ssid="$ssid"
	key_mgmt=NONE
}
EOF

			else
	cat >> "$wpa_supplicant_conf" <<EOF
ctrl_interface=/var/run/wpa_supplicant
update_config=1

network={
	scan_ssid=1
	ssid="$ssid"
	psk="${key}"
}
EOF
			fi

			local wpa_bridge=""
			if [ -n "$bridge" ]; then
				wpa_bridge="-b $bridge"
				ifconfig "$ifname" up
				ifconfig "$ifname" 0.0.0.0 
				brctl addif "$bridge" "$ifname"
			fi

			sleep 1

			/usr/sbin/wpa_supplicant -B \
				-P "/var/run/wpa_supplicant-${ifname}.pid" \
				-D nl80211 \
				-i "$ifname" \
				-c "$wpa_supplicant_conf" \
				-C "/var/run/wpa_supplicant" \
				"$wpa_bridge"
		fi

		sleep 1

		[ -z "$net_cfg" ] || {
			config_set "$vif" bridge "$bridge"
			start_net "$ifname" "$net_cfg"
		}

		set_wifi_up "$vif" "$ifname"
	done
}

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			case "$(readlink -f /sys/class/ieee80211/$phy/device)" in
				*$devpath) return;;
			esac
		done
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}

	return 0
}

check_mac80211_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}

detect_esp() {
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band="g"
		channel="11"
		htmode=""
		ht_capab=""

		htmode=HT20

		[ -n "$htmode" ] && ht_capab="set wireless.radio${devidx}.htmode=$htmode"

		if [ -x /usr/bin/readlink -a -h /sys/class/ieee80211/${dev} ]; then
			path="$(readlink -f /sys/class/ieee80211/${dev}/device)"
		else
			path=""
		fi
		if [ -n "$path" ]; then
			path="${path##/sys/devices/}"
			case "$path" in
				platform*/pci*) path="${path##platform/}";;
			esac
			dev_id="set wireless.radio${devidx}.path='$path'"
		else
			dev_id="set wireless.radio${devidx}.macaddr=$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		local mac_addr=$(cat /sys/class/ieee80211/${dev}/macaddress)
		local suffix=$(echo $mac_addr | sed 's/://g')
		local mac_suffix=${suffix:8:4}

		uci -q batch <<-EOF
			set wireless.radio${devidx}=wifi-device
			set wireless.radio${devidx}.type=esp
			set wireless.radio${devidx}.channel=${channel}
			set wireless.radio${devidx}.hwmode=11${mode_band}
			${dev_id}
			${ht_capab}
			set wireless.radio${devidx}.disabled=0

			set wireless.wlan0=wifi-iface
			set wireless.wlan0.device=radio${devidx}
			set wireless.wlan0.ifname=wlan0
			set wireless.wlan0.network=lan
			set wireless.wlan0.mode=ap
			set wireless.wlan0.ssid=WT8089-${mac_suffix}
			set wireless.wlan0.encryption=none
			set wireless.wlan0.disabled=0

	EOF
		uci -q commit wireless

		devidx=$(($devidx + 1))
	done
}
