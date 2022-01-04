#!/bin/sh

init () {
	if command -v acpi || acpi -b 2>&1 | grep -q "No support for device"
	then
		display_battery=1 
	else
		display_battery=0
	fi
	printf "bat is %s\n" "$display_battery"
	battery=""
	update_mute; update_music
	reload_all
}


update_disk () { 
	disk=$(df -h | awk '/ \/home$/{ print $4 }')
	[ -z "$disk" ] && disk=$(df -h | awk '/ \/$/{ print $4 }')
	printf "%s" "$disk"
}

update_mic() { 
	pacmd list-sources | grep -A 15 '* index' | grep -qw 'muted: no' &&\
		mic="mic" || mic="" 
	printf "%s" "$mic"
	}

update_mute () {
	muted=$(pacmd list-sinks | grep -A 15 '* index' | awk '/muted:/{ print $2 }')
	if [ "$muted" = "yes" ] ; then volume="" ; else update_volume ; fi
}

update_temp () {
	device_temp=$(cat /sys/class/thermal/thermal_zone*/type | grep -n x86 |\
		sed 's/:x86.*//g')
	device_temp=$(( device_temp - 1))
	rawtemp=$(cat /sys/class/thermal/thermal_zone"$device_temp"/temp)
	printf "%s" "$((${rawtemp:-0} / 1000))°C"
	exit
}

update_network () {
	interface=$(ip addr | awk '/ UP /{print $2}' | tr -d ':')
	[ -n "$interface" ] && network=$(wpa_cli -i "$interface" status | grep\
		-E '^ssid' | sed 's/ssid=//')
	[ -z "$network" ] && network="" && exit 0
	printf "%s" "$network" 
	exit
}

update_volume () {
	[ "$muted" = "no" ] && volume=$(pacmd list-sinks |\
		grep -A 10 '* index' | awk '/volume: front/{ print $5 }')
}

update_battery () {
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

update_interface() {
	interface=$(ip addr | awk '/ UP /{print $2}' | tr -d ':')
}


update_mail () {
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

display () {
	set_bar
	xsetroot -name "$bar"
	update_second
	printf "%s\n" "$bar"
}

update_music () {
	cmus_status=$(cmus-remote -Q 2>&1)
	music_status=""
	printf "%s" "$cmus_status" | grep "not running" && [ -z "$music_status" ] && return
	printf "%s" "$cmus_status" | grep "status playing" && music_status="play" ||\
		music_status="pause"
}	

$1
exit
