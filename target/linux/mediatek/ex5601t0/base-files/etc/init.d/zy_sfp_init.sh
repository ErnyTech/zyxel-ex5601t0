#!/bin/sh /etc/rc.common

START=50

# ZYXEL initial setup.


start()
{
	/bin/sfp_wan.sh &
	return 0
}

stop()
{
    killall /bin
    ps | grep sfp_wan.sh | awk '{print $1}' | xargs kill -9
	return 0
}
