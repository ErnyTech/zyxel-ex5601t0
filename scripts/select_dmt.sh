#!/bin/sh

ROM_ID=$1
DMT_LIST=`cat .config | grep "CONFIG_ZYXEL_CUSTOM_DMT_LIST=" | cut -d'=' -f2 | sed 's/"//g'`
echo "ROM_ID=$ROM_ID"
echo "DMT_LIST=$DMT_LIST"
if [[ "$ROM_ID" != "" && "$DMT_LIST" != "" ]] ; then
	DMT=`echo $DMT_LIST | awk -F',' '{split($0, a, ",")} END{for(i=1; i<=NF; i++) printf("%s\n",a[i])}' | grep $ROM_ID | cut -d':' -f2`
	echo "DMT=$DMT"
if [[ "$DMT" != "" ]] ; then
	echo "Select $DMT modem code!"
else
	echo "Select default modem code!"
fi
	grep CONFIG_ZYXEL_CUSTOM_DMT $TOPDIR/.config | sed -i s/CONFIG_ZYXEL_CUSTOM_DMT=.*/CONFIG_ZYXEL_CUSTOM_DMT=\"$DMT\"/g $TOPDIR/.config
fi
