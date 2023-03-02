#!/bin/sh

# use to control LAN led with all LAN port state
# Support lan5 is a external phy with GPY211
LAN_LED_LIST=`find /sys/class/net/lan* -type l`
#LAN_LED_LIST="lan1 lan2 lan3"

LED_BLINKING_FILEPATH="/tmp/LEDBlinking"
current_led_blinking_status=0 #0 is static , 1 is blinking

ENABLE_DEBUG_LOG=0
if [ "$ENABLE_DEBUG_LOG" == "1" ] ;then
SILENT=""
else
SILENT=">/dev/null"
fi
MESSAGE()
{
	eval echo $1 $SILENT
}

old_total_packets=0


function update_led_blinking_value(){
	value=`cat $LED_BLINKING_FILEPATH`
	if [ $? -eq 0 ]; then
	   led_blinking=$value
	fi
	return 0
}

# $1 is 1 set led blink, 0 to turn off
function set_led_blink(){
	if [ $1 -eq 1 ]; then
		echo timer > /sys/class/leds/zyled-green-lan/trigger
		echo 75 > /sys/class/leds/zyled-green-lan/delay_on
		echo 75 > /sys/class/leds/zyled-green-lan/delay_off
	else
		echo none > /sys/class/leds/zyled-green-lan/trigger
	fi
}
# $1 is 1 for on, 0 for off.
function set_led_state(){
	echo $1 > /sys/class/leds/zyled-green-lan/brightness
}

function check_netdev_packet_count(){
	total_count=0
	for net_dev in $LAN_LED_LIST; do
		net=`basename $net_dev`
		packets=$(grep $net /proc/net/dev | tr -s ' '| cut -d ' ' -f 4)
		MESSAGE $net $packets
		total_count=$(($total_count+$packets))
	done
	MESSAGE total_count=$total_count
	if [ $old_total_packets -ne $total_count ]; then
		MESSAGE "LAN packet increased!"
		old_total_packets=$total_count
		return 1
	fi
	return 0
}

# For loop start
function check_lan_interface_link_state(){
	for net_dev  in $LAN_LED_LIST; do
		net=`basename $net_dev`
		if [[ "$net" == "lan5" ]]; then
			LAN_25GPHY_LINK=`mii_mgr_cl45 -g -p 5 -d 0x0 -r 0x1 | cut -d '=' -f 2 | tr -d ' '`
			if [ $(( $LAN_25GPHY_LINK & 0x4)) -ne 0 ]; then
				MESSAGE "interface $net has up"
				return 1
			else
				MESSAGE "interface $net has no link detected"
			fi
		else
			#speed=`ethtool $net | grep Speed | cut -d ':' -f 2 | tr -d " "`
			link=`ethtool $net | grep "Link detected" | cut -d ':' -f 2 | tr -d " "`

			if [[ "$link" != "yes" ]]; then
				MESSAGE "interface $net has no link detected"
			else
				MESSAGE "interface $net has up"
				return 1
			fi
		fi
	done
	return 0
}

kill_process() {
        echo "stop ${PROCESS}......."
        set_led_state 0
        exit
}
trap kill_process KILL TERM EXIT INT
#trap kill_process 2 3 9 15

while true
do
	check_lan_interface_link_state
	if [ $? -eq 1 ]; then
		update_led_blinking_value
		if [ $led_blinking -eq 1 ]; then
			check_netdev_packet_count
			if [ $? -eq 1 ]; then
				set_led_blink 1
			else
				set_led_blink 0
				set_led_state 1
			fi
		else
			set_led_blink 0
			set_led_state 1
		fi
	else
		set_led_state 0
	fi
	sleep 1;
done
