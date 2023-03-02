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
# zyledcmd.sh zyled-green-pwr on

LED_PATH=/sys/class/leds
LED_ON=1
LED_OFF=0
LED_BLINK="timer"
LED_LEST=""zyled-green-pwr" "zyled-red-pwr" "zyled-green-inet" "zyled-red-inet" "zyled-green-signal" "zyled-red-signal" "zyled-green-wps" "zyled-green-wlan""

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
		echo "$LED_PATH/$1 does not exist";exit 22;
	fi
}

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
	*) echo "unknow parameter.";echo $LED_LEST;exit 22;
esac

set_led $1 $LED_STATE

exit 0
