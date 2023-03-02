#!/bin/sh

# Zyxel led test script for WX5600-T0
#LED_PWR_G, GPIO_6
#LED_PWR_R, GPIO_11
#LED_INT_G, GPIO_7
#LED_INT_R, GPIO_8
#LED_SIGNAL_G, GPIO_13
#LED_SIGNAL_R, GPIO_14
#LED_WiFi_R, GPIO_12
#LED_WPS_G, GPIO_9

# $1 parameter: led name
# $2 pamameter: on, off, slow, fast

# command example
# zyledcmd.sh power-green on

LED_PATH=/sys/class/leds
LED_ON=1
LED_OFF=0
LED_BLINK="timer"
LED_LEST=""power-green" "power-red" "inet-green" "inet-red" "signal-green" "signal-red" "wps-green" "wlan-green""
LED_PATH_INET=/sys/kernel/zyinetled

function set_led() {
	if [ -d "$LED_PATH/$1" ]; then
		if [ "$2" = "$LED_BLINK" ]; then
		  echo $2 > $LED_PATH/$1/trigger
		  echo $ON_TIME > $LED_PATH/$1/delay_on
		  echo $OFF_TIME > $LED_PATH/$1/delay_off
		else
		  echo $2 > $LED_PATH/$1/brightness
		fi
	else
	    if [ -f "$LED_PATH_INET/$1" ]; then
	        echo $2 > $LED_PATH_INET/$1
	    else
		echo "$LED_PATH/$1 and $LED_PATH_INET/$1 do not exist"; exit 22;
	    fi
	fi
}

# check LED name from input
case $1 in
	power-green )
	LED_NAME="zyled-green-pwr"
	;;
	power-red )
	LED_NAME="zyled-red-pwr"
	;;
	inet-green )
	LED_NAME="green"
	;;
	inet-red )
	LED_NAME="red"
	;;
	signal-green )
	LED_NAME="zyled-green-signal"
	;;
	signal-red )
	LED_NAME="zyled-red-signal"
	;;
	wps-green )
	LED_NAME="zyled-green-wps"
	;;
	wlan-green )
	LED_NAME="zyled-green-wlan"
	;;
	*) echo "unknow parameter."; echo $LED_LEST; exit 22;
esac

# check LED on/off/slow/fast state from input
case $2 in
	on )
	LED_STATE=$LED_ON
	;;
	off )
	LED_STATE=$LED_OFF
	;;
	slow )
	LED_STATE=$LED_BLINK
	ON_TIME=250
	OFF_TIME=250
	;;
	fast )
	LED_STATE=$LED_BLINK
	ON_TIME=125
	OFF_TIME=125
	;;
	*) echo "unknow parameter.";exit 22;
esac

set_led $LED_NAME $LED_STATE

exit 0
