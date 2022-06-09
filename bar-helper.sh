#!/bin/sh

disk () {
	disk=$(df -hl --no-sync | awk '/home/{ print $4 }' | sed '1q')
	[ -z "$disk" ] && disk=$(df -h | awk '/ \/$/{ print $4 }')
	printf "%s" "$disk"
}

mic() {
	pacmd list-sources | grep -A 15 '* index' | grep -qw 'muted: no' &&\
		mic="mic" || mic=""
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
	network=$(wpa_cli status | grep -E '^ssid' | sed 's/ssid=//')
	[ -z "$network" ] && network="" && exit 0
	printf "%s" "$network"
	exit
}

volume () {
	muted=$(pacmd list-sinks | grep -A 15 '* index' | awk '/muted:/{ print $2 }')
	[ "$muted" = "yes" ] && volume="" && exit
	volume=$(pacmd list-sinks |\
		grep -A 10 '* index' | awk '/volume: front/{ print $5 }')
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

$1
