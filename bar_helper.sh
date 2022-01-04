#!/bin/sh

init () {
	get_interface

	if command -v acpi || acpi -b 2>&1 | grep -q "No support for device"
	then
		display_battery=1 
	else
		display_battery=0
	fi
	printf "bat is %s\n" "$display_battery"
	battery=""
	get_mute; get_music
	reload_all
}


get_disk () { 
	disk=$(df -h | awk '/ \/home$/{ print $4 }')
	[ -z "$disk" ] && disk=$(df -h | awk '/ \/$/{ print $4 }')
	printf "%s" "$disk"
}
get_mic() { pacmd list-sources | grep -A 15 '* index' | grep -qw 'muted: no' &&\
	mic="mic" || mic="" ; }

get_mute () {
	muted=$(pacmd list-sinks | grep -A 15 '* index' | awk '/muted:/{ print $2 }')
	if [ "$muted" = "yes" ] ; then volume="" ; else get_volume ; fi
}

get_temp () {
	device_temp=$(cat /sys/class/thermal/thermal_zone*/type | grep -n x86 |\
		sed 's/:x86.*//g')
	device_temp=$(( device_temp - 1))
	rawtemp=$(cat /sys/class/thermal/thermal_zone"$device_temp"/temp)
	printf "%s" "$((${rawtemp:-0} / 1000))°C"
}

get_network () {
	network=""
	[ -z "$interface" ] && get_interface
	[ -n "$interface" ] && network=$(wpa_cli -i "$interface" status | grep\
		-E '^ssid' | sed 's/ssid=//')
	[ -z "$network" ] && network=""
}

get_volume () {
	[ "$muted" = "no" ] && volume=$(pacmd list-sinks |\
		grep -A 10 '* index' | awk '/volume: front/{ print $5 }')
}

get_battery () {
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
}

get_interface() {
	interface=$(ip addr | awk '/ UP /{print $2}' | tr -d ':')
}


get_mail () {
	nmail=$(find "$MAILPATH"/*/Inbox/new -type f | sed '/^$/d' | wc -l)
	case "$nmail" in
		0)
			nmail="" ;;
		[0-9]*)
			nmail="m:$nmail" ;;
		*)
			nmail="" ;;
	esac
}

set_bar () {
	bar=""
	for item in "$mic" "$music_status" "$nmail" "$disk" "$temp"\
		"$volume" "$network" "$battery"
	do
		[ -n "$item" ] && bar="$bar $item"
	done
	bar=$(printf " %s " "$bar" | cut -c 2-)
}

display () {
	set_bar
	xsetroot -name "$bar"
	get_second
	printf "%s\n" "$bar"
}

get_music () {
	cmus_status=$(cmus-remote -Q 2>&1)
	music_status=""
	printf "%s" "$cmus_status" | grep "not running" && [ -z "$music_status" ] && return
	printf "%s" "$cmus_status" | grep "status playing" && music_status="play" ||\
		music_status="pause"
}	

$1
