#!/bin/sh /etc/rc.common
# Copyright (C) 2006 OpenWrt.org

# use the number after /etc/init.d/fstab
START=59

function amixer_sset_on() { 
		mkdir -p /var/tmp/osal
		amixer sset 'O018 I150_Switch' on
		amixer sset 'O019 I151_Switch' on
		amixer sset 'O124 I032_Switch' on
		amixer sset 'O125 I033_Switch' on
}

function exportd2key()	{
		
		key=`fw_printenv d2key`
		if [[ -n "$key" ]];then
			fw_printenv d2key | sed -e "s/d2key=//g" > /var/d2-vport.key
			exit
		fi

}

boot(){
		start
		return 0
}

start() {
		amixer_sset_on
		exportd2key
}

stop() {
		return 0
}