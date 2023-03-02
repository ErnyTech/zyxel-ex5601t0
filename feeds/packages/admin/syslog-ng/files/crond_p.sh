#!/bin/sh /etc/rc.common

START=50

export PATH=/home/bin:/home/scripts:/opt/bin:/bin:/sbin:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/lib/public:/lib/private:/usr/lib:/lib:/usr/lib/ebtables:/usr/lib/iptables

EXEC_NAME=crond
EXEC_PATH=/usr/sbin
CRONTAB_DIR=/var/spool/cron/crontabs


start_crond() {
	if [ ! -d $CRONTAB_DIR ]; then 
			mkdir -m 755 -p $CRONTAB_DIR
		fi
		ln -s /etc/crontab $CRONTAB_DIR/root 2>/dev/null
        $EXEC_PATH/$EXEC_NAME -c $CRONTAB_DIR -b
}

stop_crond() {
	killall -9 $EXEC_NAME
}

start()	{
	PID=$(pgrep '^$EXEC_NAME$')
	if [ "$PID" == "" ] ; then
		start_crond
	else
		echo 'crond is already running !!!'
	fi
}

stop()	{
	stop_crond
}

restart()	{
	stop_crond
	start_crond
}