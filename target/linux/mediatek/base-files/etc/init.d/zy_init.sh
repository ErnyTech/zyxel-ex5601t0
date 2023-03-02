#!/bin/sh /etc/rc.common

START=49

# ZYXEL initial setup.

function zyxel_initial() {
	#enable ipv4 forwarding, ZyXEL, John
	echo 1 > /proc/sys/net/ipv4/ip_forward
	#disable ipv6 on all interface by default, Pochao
	echo 1 > /proc/sys/net/ipv6/conf/default/disable_ipv6
	#set default accept_ra when IPv6 is enable, ChihYuan
	#echo 0 > /proc/sys/net/ipv6/conf/default/accept_ra #done in S11sysctl
	# for zcfg_msg sock
	mkdir -p /var/lib/.sock
	chmod 713 /var/lib/.sock

	#mount /dev/bus for /proc/bus/usb/
	mount --bind /dev/bus /proc/bus
	ln -s /sys/kernel/debug/usb/devices /proc/bus/usb/devices

	# pppd lock dir
	#mkdir /var/lock  #done in S10boot
	#dns
	if [ ! -f "/var/fyi/sys" ]; then
		mkdir -m 755 -p /var/fyi
		mkdir -m 755 -p /var/fyi/sys
	fi
	echo 'nameserver 127.0.0.1' > /etc/resolv.conf
	ln -sf /tmp/hosts /etc/hosts
	# 3G WWAN
	mkdir /var/wwan
	# 4G LTE WNC
	test -e /etc/malmanager.cfg && cp /etc/malmanager.cfg /var/
	# usb
	mkdir /var/mount
	mkdir /var/usb	
	#account
	mkdir /var/home
	if [ ! -L "/home" ]; then
		ln -s /var/home /home
	fi
	#rout
	mkdir /var/zebra
	mkdir /var/iproute2
	ln -s /etc/iproute2/rt_tables /var/iproute2/rt_tables
	
	depmodpath=`uname -r`

	ifconfig lo up
	
	ln -sf /var/passwd /etc/passwd
	ln -sf /var/shadow /etc/shadow
	ln -sf /var/group /etc/group
	#logrotate
	mkdir -m 755 -p /var/lib
	# ZyXEL,Cj, TCP timestamp response Vulnerabilities
	# echo 0 > /proc/sys/net/ipv4/tcp_timestamps
	# ZyXEL, Mark, set unix socket maximun queue
	if [ -e "/lib/modules/$depmodpath/nciTMSkmod.ko" ]
	then
		echo "Load NComm TMS Module." > /dev/console
		mknod /dev/tms c 308 0
		insmod /lib/modules/$depmodpath/nciLservices.ko
	else
		echo "Not build NComm TMS."
	fi
	echo 64 > /proc/sys/net/unix/max_dgram_qlen
	#ZyXEL, Bell, set and update module.dep before zcmd and zcfg_be
	depmod -n > /lib/modules/$depmodpath/modules.dep &
	#Zyxel, Trevor, set watchdog for 502L07
	#wdtctl -t 80 start
	#ZyXEL, Bell, set printk log level at 2 (KERN_ALET)
	#echo 2 > /proc/sys/kernel/printk
	#ZyXEL, Frank, support ping_group_range
	echo "0 2147483647" > /proc/sys/net/ipv4/ping_group_range
	if [ -e "/lib/modules/$depmodpath/nciTMSkmod.ko" ]
	then
		echo "Load NComm TMS Module." > /dev/console
		insmod /lib/modules/$depmodpath/nciexLinuxETH.ko

	else
		echo "Not build NComm TMS."
	fi
	#Zyxel, in the USB samba upload/download stress, it will cause system has low free-memory
	#Prevent the low free-memory to cause system unstable
	if [ `cat /proc/meminfo | grep 'MemTotal:' | awk '{print $2}'` -gt 110000 ]; then
		echo 5120 > /proc/sys/vm/min_free_kbytes
	fi
	#Econet Support Ethernet OAM Feature
	if [ -e "/lib/modules/$depmodpath/nciTMSkmod.ko" ]
	then
		echo "Load NComm TMS Module." > /dev/console
		insmod /lib/modules/$depmodpath/nciTMSkmod.ko

	else
		echo "Not build NComm TMS."
	fi
	
}

start()
{
	zyxel_initial
	return 0
}

stop()
{
	return 0
}
