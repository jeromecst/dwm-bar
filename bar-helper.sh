#!/bin/sh

disk () {
	disk=$(df -hl --no-sync | awk '/home/{ print $4 }' | sed '1q')
	[ -z "$disk" ] && disk=$(df -h | awk '/ \/$/{ print $4 }')
	printf "%s" "$disk"
}

mic() {
	if pactl get-source-mute @DEFAULT_SOURCE@ | grep -q yes
	then
		mic=""
	else
		mic="mic"
	fi
	printf "%s" "$mic"
}

temp () {
	device_temp=$(cat /sys/class/thermal/thermal_zone*/type | grep -n x86 |\
		sed 's/:x86.*//g')
	device_temp=$(( device_temp - 1))
	rawtemp=$(cat /sys/class/thermal/thermal_zone"$device_temp"/temp)
	printf "%s" "$((${rawtemp:-0} / 1000))°C"
}

network () {
	network=$(iwctl station wlan0 show | awk '/Connected network/{print $3}')
	[ -z "$network" ] && network=$(wpa_cli status | grep -E '^ssid' | sed 's/ssid=//')
	[ -z "$network" ] && network="" && exit 0
	printf "%s" "$network"
	exit
}

volume () {
	if pactl get-sink-mute @DEFAULT_SINK@ | grep -qi yes
	then
		volume=""
		exit
	else
		volume=$(pactl get-sink-volume @DEFAULT_SINK@ | grep -Eo "[0-9]+%" |\
			head -n1)
	fi
	printf "%s" "$volume"
}

battery () {
	batstatus=$(acpi -b | awk '{print $3}' | tr -d ',')
	acstatus=$(acpi -a | awk '{print $3}' | tr -d ',')
	batpor=$(acpi -ab | awk '{print $4}' | tr -d ',')
	case "$batstatus" in
		Discharging|Charging)
			batstatus=$(printf "%s" "$batstatus" | head -n 1)
			battery="$batstatus:$batpor" ;;
		Unknown)
			[ "$acstatus" = "on-line" ] && battery="Charging:$batpor" ;;
		Full)
			battery="Full" ;;
		"")
			[ "$acstatus" = "on-line" ] && battery="AC" ;;
	esac
	printf "%s" "$battery"
}

mail () {
	nmail=$(find "$MAILPATH"/*/Inbox/new -type f | sed '/^$/d' | wc -l)
	case "$nmail" in
		0)
			nmail="" ;;
		[0-9]*)
			nmail="m:$nmail" ;;
		*)
			nmail="" ;;
	esac
	printf "%s" "$nmail"
}

music () {
	cmus_status=$(cmus-remote -Q 2>&1)
	music_status=""
	printf "%s" "$cmus_status" | grep -q "not running" && exit
	if printf "%s" "$cmus_status" | grep -q "status playing"; then
		music_status="playing"; else
		music_status="pause"
	fi
	printf "%s" "$music_status"
}	

mem () {
	free -h | awk '/Mem:/{ printf $3 }'
}

$1
