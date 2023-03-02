#!/bin/sh

# sfp_led.sh 0 1000, for fiber mode with 1000 Mbps
# sfp_led.sh 1 2500, for copper mode with 2500 Mbps

##############################
###   Control SFP LED     ####
##############################
# set_sfp_led()
# $1: on, off
function set_sfp_led(){
    case "$1" in
        "on")
            #echo "SFP LED on"
            echo 255 > /sys/class/leds/zyled-green-sfp/brightness
            ;;
        "off")
            #echo "SFP LED off"
            echo 0 > /sys/class/leds/zyled-green-sfp/brightness
            ;;
        *) 
            echo "unknow behavior"
            exit 1
    esac
}


##############################
###Annouse esmd link state####
##############################
# $1, use link state, "up" or "down"
# $2, use link speed, 2500, 1000
SPF_MODE_INDEX=/tmp/SFP_WAN.status
annouse_sfp_link_state() {
    case "$1" in
        "up")
            echo "$2" > $SPF_MODE_INDEX
            ;;
        "down")
            rm -f $SPF_MODE_INDEX
            ;;
        *) echo "unknow link state"
    esac
}

function is_sfp_present(){
    MOD_ABS_VALUE=`cat /sys/class/gpio/gpio468/value`
    if [ "$MOD_ABS_VALUE" = "0" ]; then
        return 1
    else
        return 0
    fi
}
#function is_copper_sfp_link_up(){
#    TMP_VAL=`i2cget -f -y i2c-mt65xx 0x56 0x01 i 1`;
#    TMP_VAL=`i2cget -f -y i2c-mt65xx 0x56 0x01 i 1`;
#    if [ $(( $TMP_VAL & $((1<<2)) )) -ne 0 ]; then
#        return 1;
#    else
#        return 0;
#    fi
#}
#We donot know why, but Zyxel SFP-1000T need to read twice to have correct value.
function is_copper_sfp_link_up(){
    is_sfp_present
    SFP_PRESENT=$?
    if [ "$SFP_PRESENT" = "0" ]; then
        return 0
    fi
    retry_count=0
    while [ $retry_count -le 2 ]
    do
        TMP_VAL=$(i2cget -f -y i2c-mt65xx 0x56 0x01 i 1 2>/dev/null);
        RET=$?
        if [ $RET -eq 0 ]; then
            if [ "$TMP_VAL" == "0x01" ]; then
                continue
            elif [ $(( $TMP_VAL & $((1<<2)) )) -ne 0 ]; then
                return 1
            else
                return 0
            fi
        else
            # command fail, retry if
            continue
        fi
        retry_count=$(($retry_count+1))
    done
}


function is_fiber_sfp_rx_los(){
    RX_LOS_VALUE=`cat /sys/class/gpio/gpio434/value`
    if [ "$RX_LOS_VALUE" = "1" ]; then
        echo 1
    else
        echo 0
    fi
}

kill_process() {
    echo "stop check_sfp_link.sh......."
    set_sfp_led off
    exit
}
trap kill_process KILL TERM EXIT INT

IS_COPPER_SFP=1
case $1 in
    0)
        IS_COPPER_SFP=0
        ;;
    1)
        IS_COPPER_SFP=1
        ;;
    *) echo "unknow type!!!"
esac
SGMII_2500_MODE=1
SGMII_1000_MODE=2
SFP_LINK_SPEED=2500
case $2 in
    1)
        SFP_LINK_SPEED=2500
        ;;
    2)
        SFP_LINK_SPEED=1000
        ;;
    *) echo "unknow type!!!"
esac

LATEST_SFP_LINK_STAT=0
CURRENT_LINK_STAT=0
while true
do
    sleep 1;
    if [ $IS_COPPER_SFP -eq 1 ]; then
        #echo "copper sfp mode"
        is_copper_sfp_link_up
        if [ $? -eq 1 ]; then
            CURRENT_LINK_STAT=1
        else
            CURRENT_LINK_STAT=0
        fi
    else
        #echo "fiber sfp mode"
        TMP_SFP_SIGNAL=$(is_fiber_sfp_rx_los)
        if [ $TMP_SFP_SIGNAL -eq 1 ]; then
            CURRENT_LINK_STAT=0
        else
            CURRENT_LINK_STAT=1
        fi
    fi

    if [ $LATEST_SFP_LINK_STAT -eq $CURRENT_LINK_STAT ]; then
        continue
    fi

    if [ $CURRENT_LINK_STAT -eq 1 ]; then
        echo "SFP link up"
        annouse_sfp_link_state up $SFP_LINK_SPEED
        set_sfp_led on
    else
        echo "SFP link down"
        annouse_sfp_link_state down
        set_sfp_led off
    fi

    LATEST_SFP_LINK_STAT=$CURRENT_LINK_STAT
done
