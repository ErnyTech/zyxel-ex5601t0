#!/bin/sh /etc/rc.common
# Copyright (C) 2006 OpenWrt.org

# use the number after /etc/init.d/fstab
START=45

ZYUBI_PARTITION=zyubi
DEFAULT_DATA_PARTITION_SIZE=20
DEFAULT_ROMFILE_PARTITION_SIZE=2
DEFAULT_ROMD_PARTITION_SIZE=2
DEFAULT_WWAN_PARTITION_SIZE=2
DEFAULT_MISC_PARTITION_SIZE=max

#DEFAULT_DATA_MOUNT_POINT=/mnt/data
DEFAULT_DATA_MOUNT_POINT=/data
#DEFAULT_MISC_MOUNT_POINT=/mnt/misc
DEFAULT_MISC_MOUNT_POINT=/misc

function mk_ubi_vol() {
	local ubi_number=$1
	local vol_name=$2
	local default_vol_size=$3
	local vol_desired_num=0
	local vol=`grep '^'${vol_name}'$' /sys/class/ubi/ubi${ubi_number}_*/name`

	if [ ! -z $vol ]; then 
		vol=${vol%$vol_name}
		vol=${vol//ubi${ubi_number}_/}
		vol_desired_num=${vol//[!0-9]/}
		echo "Volume ${vol_name} is exist at ubi${ubi_number} !!!!"
	else
		if [ "${default_vol_size}" != "max" ]
		then
			ubimkvol /dev/ubi${ubi_number} -s ${default_vol_size}MiB -N ${vol_name} 2>/dev/null
		else
			ubimkvol /dev/ubi${ubi_number} -m -N ${vol_name} 2>/dev/null
		fi
		if [ $? -ne 0 ];then
			echo "create VOL:${vol_name} failure!!!!"
			return 1;
		fi
		echo "Create new volume ${vol_name} at ubi${ubi_number} !!!!"
	fi

}

function check_zypartition() {
	# get MTD number by name
	if MTD=`grep $ZYUBI_PARTITION /proc/mtd`;
	then
	MTD=${MTD/mtd/}; # replace "mtd" with nothing
	MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
	fi

	if [ "$MTD" != "" ] 
	then
	echo "[$0]: $ZYUBI_PARTITION partition is on MTD devide $MTD"

		# try to attach data partition mtd to ubi, will format automatically if empty,
		if UBI=`ubiattach -m "$MTD"`;
		then
			echo ">>>>UBI attach successfully<<<<"
		else
			echo ">>>>UBI attach fail<<<<"
			echo ">>>>Erase mtd partition $ZYUBI_PARTITION<<<<"
			mtd erase /dev/mtd$MTD
			if UBI=`ubiattach -m "$MTD"`;
			then
				echo ">>>>UBI attach successfully<<<<"
			else
				echo ">>>>UBI attach fail<<<<"
				return 1;
			fi
		fi
	UBI=${UBI##*"number "}; # cut all before "number ", still need to get rid of leading space
	UBI=${UBI%%,*}; # cut all after ","
	# check and create volume "data" with 2 Mib
	if mk_ubi_vol $UBI romfile $DEFAULT_ROMFILE_PARTITION_SIZE; then
		echo ">>>>create romfile volume<<<<"
	fi
	# check and create volume "rom-d" with 2 Mib
	if mk_ubi_vol $UBI rom-d $DEFAULT_ROMD_PARTITION_SIZE; then
		echo ">>>>create rom-d volume<<<<"
	fi
	# check and create volume "wwan" with 2 Mib
	if mk_ubi_vol $UBI wwan $DEFAULT_WWAN_PARTITION_SIZE; then
		echo ">>>>create wwan volume<<<<"
	fi
	# check and create volume "data" with 20 Mib, then mount it.
	if mk_ubi_vol $UBI data $DEFAULT_DATA_PARTITION_SIZE; then
		echo ">>>>mount data partition<<<<"
		mkdir -p ${DEFAULT_DATA_MOUNT_POINT}
		mount -t ubifs ubi"$UBI":data ${DEFAULT_DATA_MOUNT_POINT};
	fi
	# check and create volume "misc" with rest storage of $ZYUBI_PARTITION partition, then mount it.
	if mk_ubi_vol $UBI misc $DEFAULT_MISC_PARTITION_SIZE; then
		echo ">>>>mount misc partition<<<<"
		mkdir -p ${DEFAULT_MISC_MOUNT_POINT}
		mount -t ubifs ubi"$UBI":misc ${DEFAULT_MISC_MOUNT_POINT};
	fi
	else
	echo "[$0]: cannot find $ZYUBI_PARTITION partition."
	fi
}

boot() {
	start
	return 0
}

start() {
	check_zypartition
}

restart() {
	# no need to unmount partition
	return 0
}

stop() {
	return 0
}
