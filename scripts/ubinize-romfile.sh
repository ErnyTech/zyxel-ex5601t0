#!/bin/sh

. $TOPDIR/scripts/functions.sh
GENNANDIMG_DIR=$BIN_DIR/NAND/genNANDimg

ubinize_param=""
romfile=""
vol_size=""
tmp_path=""
ENDIAN=""
outfile=""
err=""
ubinize_seq=""

ubivol() {
	volid=$1
	name=$2
	image=$3
	autoresize=$4
	size="$5"
	echo "[$name]"
	echo "mode=ubi"
	echo "vol_id=$volid"
	echo "vol_type=dynamic"
	echo "vol_name=$name"
	if [ "$image" ]; then
		echo "image=$image"
		[ -n "$size" ] && echo "vol_size=${size}"
	else
		echo "vol_size=1MiB"
	fi
	if [ "$autoresize" ]; then
		echo "vol_flags=autoresize"
	fi
}

ubivol_static() {
	volid=$1
	name=$2
	image=$3
	size="$4"
	echo "[$name]"
	echo "mode=ubi"
	echo "vol_id=$volid"
	echo "vol_type=static"
	echo "vol_name=$name"
	if [ "$image" ]; then
		echo "image=$image"
		[ -n "$size" ] && echo "vol_size=${size}"
	else
		echo "vol_size=1MiB"
	fi
}

ubilayout() {
	local vol_id=0
	local rootsize=
	local romfilesize=

	# add for romfile
	if [ "$1" ]; then
		#ubivol $vol_id romfile "$1"
		echo "ubilayout : vol_size = $2"
		#ubivol_static $vol_id romfile "$1" "2MiB"
		ubivol_static $vol_id romfile "$1" "$2"
		vol_id=$(( $vol_id + 1 ))
	fi

}

set_ubinize_seq() {
	if [ -n "$SOURCE_DATE_EPOCH" ] ; then
		ubinize_seq="-Q $SOURCE_DATE_EPOCH"
	fi
}

while [ "$1" ]; do
	case "$1" in
	"--romfile")
		romfile="$2"
		shift
		shift
		continue
		;;
	"--vol_size")
		vol_size="$2"
		shift
		shift
		continue
		;;
	"--tmp_path")
		tmp_path="$2"
		shift
		shift
		continue
		;;
	"--ENDIAN")
		if [ "$2" == "Little" ]; then
			ENDIAN="1"
		elif [ "$2" = "Big" ]; then
			ENDIAN="0"
		fi
		shift
		shift
		continue
		;;
	"-"*)
		ubinize_param="$@"
		break
		;;
	*)
		if [ ! "$outfile" ]; then
			outfile=$1
			shift
			continue
		fi
		;;
	esac
done

if [ ! -r "$romfile" -a ! "$outfile" ]; then
	echo "syntax: $0 [--romfile romfile] out [ubinize opts]"
	exit 1
fi

ubinize="$( which ubinize )"
if [ ! -x "$ubinize" ]; then
	echo "ubinize tool not found or not usable"
	exit 1
fi

ubinizecfg="$( mktemp 2> /dev/null )"
if [ -z "$ubinizecfg" ]; then
	# try OSX signature
	ubinizecfg="$( mktemp -t 'ubitmp' )"
fi


#********************************************
# Padding func $1:name  $2:"block size" or "partition size"
#Padding()
#{
#        if [ -f  $1 ]; then
#                echo "!! file exist 1:$1  2:$2"
#                SIZE=`wc -c $1 | awk '{ print $1 }'` ; \
#                TEMP_SIZE=$2 ; \
#        PAD=`expr $TEMP_SIZE - $SIZE % $TEMP_SIZE` ; \
#        else
#                echo "!! file not exist  1:$1  2:$2"
#                PAD=$2 ; \
#        fi
#        dd if=/dev/zero count=1 bs=$PAD 2> /dev/null | \
#        tr \\000 \\377 >> $1
#}
#********************************************

#********************************************
cp $romfile $tmp_path/ROMFILE_TMP.img
# to create romfile header (8 bytes)
dd if=/dev/zero of=$tmp_path/romfile_header count=1 bs=8
cat $tmp_path/romfile_header $tmp_path/ROMFILE_TMP.img > $tmp_path/ROMFILE_TMP2.img
$tmp_path/../staging_dir/host/bin/romfileHeaderModify $tmp_path/ROMFILE_TMP2.img $tmp_path/ROMFILE.img $ENDIAN
#Padding $tmp_path/ROMFILE.img `printf "%d" $vol_size`
rm $tmp_path/ROMFILE_TMP.img $tmp_path/ROMFILE_TMP2.img
echo "Padding ROMFILE partition finish."
#********************************************

ubilayout "$tmp_path/ROMFILE.img" `printf "%d" $vol_size` > "$ubinizecfg"

set_ubinize_seq
cat "$ubinizecfg"
ubinize $ubinize_seq -o "$outfile" $ubinize_param "$ubinizecfg"
err="$?"
[ ! -e "$outfile" ] && err=2
rm "$ubinizecfg" romfile_header ROMFILE.img

exit $err
