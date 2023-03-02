#!/bin/sh

if [ $# -ne 2 ]; then
	echo "Usage: zyautoconf.sh <input config file> <output C header file>"
	echo -e "\tThis script will extract configs from <input config file>"
	echo -e "\tOutput C header file"
	exit -1
fi

CFG_FILE=$1
OUTPUT_FILE=$2

# create C style header file
echo -e "/*\n * Automatically generated : don't edit\n */\n" > $OUTPUT_FILE

HEADER_FILE_FLAG=_$(echo $(basename $CFG_FILE) | tr '[:lower:]' '[:upper:]' | sed "s/\./_/")_

echo -e "#ifndef $HEADER_FILE_FLAG" >> $OUTPUT_FILE
echo -e "#define $HEADER_FILE_FLAG\n" >> $OUTPUT_FILE
while read cfg; do
	if [ -z "$cfg" ]; then
		continue
	fi
	# unset configuration
	if [ "`echo $cfg | cut -c 1`" = "#" ]; then
		if [ -n "`echo $cfg | grep 'CONFIG_'`" ]; then
			cfg_name=`echo $cfg | cut -d' ' -f2`
			echo "#undef  $cfg_name" >> $OUTPUT_FILE
		fi
	else
		cfg_name=`echo $cfg | cut -d'=' -f1`
		cfg_val=`echo $cfg | cut -d'=' -f2`
		if [ "$cfg_val" = "m" ]; then
			#echo "#undef  $cfg_name" >> $OUTPUT_FILE
			echo "#define ${cfg_name}_MODULE 1" >> $OUTPUT_FILE
		elif [ "$cfg_val" = "y" ]; then
			echo "#define $cfg_name 1" >> $OUTPUT_FILE
		else
			echo "#define $cfg_name $cfg_val" >> $OUTPUT_FILE
		fi
	fi
	echo "" >> $OUTPUT_FILE
done < $CFG_FILE
echo -e "\n#endif" >> $OUTPUT_FILE

