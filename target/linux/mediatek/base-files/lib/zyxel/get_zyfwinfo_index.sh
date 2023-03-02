. /lib/functions.sh

CI_ZYFWINFO="${CI_ZYFWINFO:-zyfwinfo}"
ubi_mtd_index="$(find_mtd_index "ubi")"
ubi2_mtd_index="$(find_mtd_index "ubi2")"
current_mtd_index="$(cat /sys/devices/virtual/ubi/ubi0/mtd_num)"

ubi_mknod() {
	local dir="$1"
	local dev="/dev/$(basename $dir)"

	[ -e "$dev" ] && return 0

	local devid="$(cat $dir/dev)"
	local major="${devid%%:*}"
	local minor="${devid##*:}"
	mknod "$dev" c $major $minor
}

nand_find_volume() {
	local ubidevdir ubivoldir
	ubidevdir="/sys/devices/virtual/ubi/$1"
	[ ! -d "$ubidevdir" ] && return 1
	for ubivoldir in $ubidevdir/${1}_*; do
		[ ! -d "$ubivoldir" ] && continue
		if [ "$( cat $ubivoldir/name )" = "$2" ]; then
			basename $ubivoldir
			return 0
		fi
	done
}

nand_find_ubi() {
	local ubidevdir ubidev mtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/devices/virtual/ubi/ubi*; do
		[ ! -d "$ubidevdir" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
		[ ! "$mtdnum" ] && continue
		if [ "$mtdnum" = "$cmtdnum" ]; then
			ubidev=$( basename $ubidevdir )
			ubi_mknod "$ubidevdir"
			echo $ubidev
			return 0
		fi
	done
}

get_nonboot_mtd_index() {
	if [ "$current_mtd_index" = "$ubi_mtd_index" ]; then
		nonboot_mtd_index=$ubi2_mtd_index
	elif [ "$current_mtd_index" = "$ubi2_mtd_index" ]; then
		nonboot_mtd_index=$ubi_mtd_index
	fi
}

get_zyfwinfo_index() {
	if [ "$1" = "boot" ]; then
		volume_zyfwinfo="$(nand_find_volume "ubi0" $CI_ZYFWINFO)"
	elif [ "$1" = "nonboot" ]; then
		get_nonboot_mtd_index
		if [ "$2" = "attach" ]; then
			ubiattach -m $nonboot_mtd_index
			ubidev_ubi=$(nand_find_ubi "ubi")
			ubidev_ubi2=$(nand_find_ubi "ubi2")
			if [ "${ubidev_ubi}" = "ubi0" ]; then
				ubi_unmount="$ubidev_ubi2"
			else
				ubi_unmount="$ubidev_ubi"
			fi
			echo $ubi_unmount > /tmp/ubi_unmount
			volume_zyfwinfo="$(nand_find_volume $ubi_unmount $CI_ZYFWINFO)"
		elif [ "$2" = "detach" ]; then
			ubidetach -m $nonboot_mtd_index
		fi
	fi
	if [ "${#volume_zyfwinfo}" != "0" ]; then
		index="${volume_zyfwinfo#*_}"
		return $index
	fi
}

get_zyfwinfo_index $1 $2
