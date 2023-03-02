#!/bin/sh


LOCK_FILE=/tmp/sfp_wan.lock

########################  PLATFORMSETUP START
GPIOBASE=`cat /sys/class/gpio/gpiochip*/base | head -n1`
set_gpio() {
  GPIO_PIN=`expr $1 + $GPIOBASE`
  if [ -d /sys/class/gpio/gpio$GPIO_PIN ]; then
    echo "pin${GPIO_PIN} already be exported."
  else
    echo $GPIO_PIN > /sys/class/gpio/export;
    if [ $? != 0 ]; then
      echo "export pin${GPIO_PIN} fail."
    fi
  fi

  if [ "$3" != "" ]; then
    { [ "$3" = "0" ] && echo "low" || echo "high"; } \
      >"/sys/class/gpio/gpio$GPIO_PIN/direction"
  else
    echo in > /sys/class/gpio/gpio$GPIO_PIN/direction
  fi
}
# SFP: AE_MOD_ABS_1V8(GPIO_57), AE_RX_LOS_3V3(GPIO_23), AE_TX_FAULT_3V3(GPIO_28)
GPIO_INPUT_LIST="23 28 57"
# SFP: SFP_EWAN_SEL GPIO 10, HIGH for 2.5G PHY, LOW for SFP
GPIO_OUT_H_LIST="10"
# SFP: AE_TX_DIS_3V3(GPIO_26)
GPIO_OUT_L_LIST="26"
function init_sfp_gpios(){
    for i in $GPIO_INPUT_LIST; do
        set_gpio $i in
    done
    for i in $GPIO_OUT_H_LIST; do
        set_gpio $i out 1
    done
    for i in $GPIO_OUT_L_LIST; do
        set_gpio $i out 0
    done
}
########################  PLATFORMSETUP END


# CURRENT_SFP_MODE, 0 is fiber sfp, 1 is copper sfp
CURRENT_SFP_MODE=0
CURRENT_SFP_MODE_IS_FIBER=0
CURRENT_SFP_MODE_IS_COPPER=1
# sometimes SFP need to have more time to boot up, we need to wait additional time for sfp ready.
SFP_CHECK_RETRY_TIME=60

function is_sfp_present(){
    MOD_ABS_VALUE=`cat /sys/class/gpio/gpio468/value`
    if [ "$MOD_ABS_VALUE" = "0" ]; then
        echo 1
    else
        echo 0
    fi
}

function is_sfp_rx_los(){
    RX_LOS_VALUE=`cat /sys/class/gpio/gpio434/value`
    if [ "$RX_LOS_VALUE" = "1" ]; then
        echo 1
    else
        echo 0
    fi
}

# set_sfp_ethwan_sel()
# $1: sfp, ethwan
function set_sfp_ethwan_sel(){
    case "$1" in
        "sfp")
            echo "Switch SGMII1 to SFP Mode"
            echo 0 > /sys/class/gpio/gpio421/value
            ;;
        "ethwan")
            echo "Switch SGMII1 to 2.5G PHY Mode"
            echo 1 > /sys/class/gpio/gpio421/value
            ;;
        *) echo "unknow select"
    esac
}

# set_sgmii_rate()
# $1: 1000, 2500
function set_sgmii_rate(){
    case $1 in
        1000)
            # 1000Base-X force 1G
            regs w 100700e8 10 >> /dev/null
            regs w 10070128 14813 >> /dev/null
            regs w 10070020 31120008 >> /dev/null
            regs w 10070008 1a0 >> /dev/null
            regs w 10070000 1240 >> /dev/null
            regs w 100700e8 0 >> /dev/null
            regs w 15110200 0105f33b
            ;;
        2500)
            # force 2.5G (gen2)
            regs w 100700e8 10 >> /dev/null
            regs w 10070128 14817 >> /dev/null
            regs w 10070020 31120009 >> /dev/null
            regs w 10070000 140 >> /dev/null
            regs w 100700e8 0 >> /dev/null
            regs w 15110200 0105f33b
            ;;
        *) echo "unknow rate"
    esac
}

function is_cooper_sfp(){
    HAVE_COOPER_SFP=`i2cdetect -y i2c-mt65xx 0x56 0x56 | grep 50 | cut -d ':' -f 2 | sed s/[[:space:]]//g`
    if [ "$HAVE_COOPER_SFP" = "56" ]; then
        echo 1
    else
        echo 0
    fi
}

function is_fiber_sfp(){
    HAVE_COOPER_SFP=`i2cdetect -y i2c-mt65xx 0x51 0x51 | grep 50 | cut -d ':' -f 2 | sed s/[[:space:]]//g`
    if [ "$HAVE_COOPER_SFP" = "51" ]; then
        echo 1
    else
        echo 0
    fi
}

function read_sfp_vender_name(){
    VENDER_NAME=""; \
    STRING=`i2cget -f -y i2c-mt65xx 0x50 0x14 i 16 | sed -e s/0x//g`; \
    for char in $STRING; \
    do \
    VENDER_NAME=$VENDER_NAME`printf "\x$char"` ; \
    done; \
    echo $VENDER_NAME;
}

function read_sfp_part_number(){
    PART_NUM=""; \
    STRING=`i2cget -f -y i2c-mt65xx 0x50 0x28 i 16 | sed -e s/0x//g`; \
    for char in $STRING; \
    do \
    PART_NUM=$PART_NUM`printf "\x$char"` ; \
    done; \
    echo $PART_NUM;
}

function detect_sfp_mode(){
    retry_count=0
    CURRENT_SFP_MODE=$CURRENT_SFP_MODE_IS_COPPER
    while [ $retry_count -le $SFP_CHECK_RETRY_TIME ]
    do
      #is_fiber_sfp
      if [ $(is_fiber_sfp) -eq 1 ]; then
        CURRENT_SFP_MODE=$CURRENT_SFP_MODE_IS_FIBER
        echo "Fiber SFP mode"
        break
      elif [ $(is_cooper_sfp) -eq 1 ]; then
        CURRENT_SFP_MODE=$CURRENT_SFP_MODE_IS_COPPER
        echo "Copper SFP mode"
        break
      fi
      #echo "check CURRENT_SFP_MODE=$CURRENT_SFP_MODE"
      echo "Waiting SFP detection..."
      retry_count=$(($retry_count+1))
      sleep 1
    done
}

# check bit 18 to SGMII link state
function sgmii_check_link_state() {
    TMP_VAL_HEX=`regs d 0x10070000 | grep "Value at" | cut -d ':' -f 2 | sed s/[[:space:]]//g`
    if [ $(( $((TMP_VAL_HEX)) & $((1<<18)) )) -ne 0 ]; then
        return 1
    else
        return 0
    fi
}

SGMII_2500MB_DEVICE_LIST="PMG3000-D20B "
SGMII_1000MB_DEVICE_LIST="THMPRS-3511-10A THMPRS-5311-10A SFP-1000T ETB44435-7SB4-ZY ETB44451-8SB4-ZY AFM0002 AFM0003"

SGMII_2500_MODE=1
SGMII_1000_MODE=2
###################
# PMG3000-D20B have multiple firmware to support different rate
# use SGMII 1G only with firmware V100ABVJ0bX
# use SGMII 2.5G/1G with firmware V250ABVJ1bX
# in this case, we try 2500 Mbps first, if check the SGMII link state.
# if the link still down, retry with 1000 Mbps
###################
function decide_sgmii_mode_of_sfp(){
    CURRENT_SFP_PART_NUM=$(read_sfp_part_number)
    for DEVICE in $SGMII_2500MB_DEVICE_LIST; do
        #echo "check DEVICE=$DEVICE and CURRENT_SFP_PART_NUM=$CURRENT_SFP_PART_NUM"
        if [ "$DEVICE" == "$CURRENT_SFP_PART_NUM" ]; then
            if [ "PMG3000-D20B" == "$CURRENT_SFP_PART_NUM" ]; then
                # try 2500 Mbps first
                #echo "set_sgmii_rate 2500-1"
                set_sgmii_rate 2500
                # wait a seconds then check link state
                sleep 3
                sgmii_check_link_state
                if [ $? -eq 1 ]; then
                    return $SGMII_2500_MODE
                else
                    #echo "set_sgmii_rate 1000-1"
                    # re-try 1000 Mbps first
                    set_sgmii_rate 1000
                    return $SGMII_1000_MODE
                fi
            else
                #echo "set_sgmii_rate 2500-2"
                set_sgmii_rate 2500
                return $SGMII_2500_MODE
            fi
        fi
    done
    for DEVICE in $SGMII_1000MB_DEVICE_LIST; do
        #echo "check DEVICE=$DEVICE and CURRENT_SFP_PART_NUM=$CURRENT_SFP_PART_NUM"
        if [ "$DEVICE" == "$CURRENT_SFP_PART_NUM" ]; then
            #echo "set_sgmii_rate 1000-1"
            set_sgmii_rate 1000
            return $SGMII_1000_MODE
        fi
    done
    # the SFP device is not in the list, default set SGMII to 2500.
    #echo "set_sgmii_rate 2500-3"
    set_sgmii_rate 2500
    return $SGMII_2500_MODE
}

######SFP LED deamon######
SFP_LED_PROG=/bin/check_sfp_link.sh
# $1 : 0 for fiber mode, 1 for copper mode
# $2 : link rate, 1000 or 2500
start_check_sfp_link() {
    $SFP_LED_PROG $1 $2 &
    echo $! > /tmp/check_sfp_link.sh.pid
}

stop_check_sfp_link() {
    if [ -e /tmp/check_sfp_link.sh.pid ]; then
        pid=`cat /tmp/check_sfp_link.sh.pid`
    else
        pid=`ps | grep $SFP_LED_PROG | awk '{print $1}' | sed -n 1p`
    fi
    if [ $pid -ne 0 ]
    then
        kill -TERM $pid
    fi
}

##############################

# force the external PHY into power down mode
power_down_ext_phy() {
    TMP_VAL=`mii_mgr_cl45 -g -p 0x6 -d 0x0 -r 0x0 | cut -d '=' -f 2 | tr -d ' '`
    WRITE_VAL=$(( $TMP_VAL | (1 << 0xb) ))
    WRITE_VAL_HEX=$(printf "%x" $WRITE_VAL)
    mii_mgr_cl45 -s -p 0x6 -d 0x0 -r 0x0 -v $WRITE_VAL_HEX >> /dev/null
}

# force the external PHY into normal operational mode
power_up_ext_phy() {
    TMP_VAL=`mii_mgr_cl45 -g -p 0x6 -d 0x0 -r 0x0 | cut -d '=' -f 2 | tr -d ' '`
    WRITE_VAL=$(( $TMP_VAL & (~(1 << 0xb)) ))
    WRITE_VAL_HEX=$(printf "%x" $WRITE_VAL)
    mii_mgr_cl45 -s -p 0x6 -d 0x0 -r 0x0 -v $WRITE_VAL_HEX >> /dev/null
}

kill_process() {
    stop_check_sfp_link
    rm -f ${LOCK_FILE}
    echo "stop sfp_wan.sh......."
    exit
}
trap kill_process KILL TERM EXIT INT


if [ -f $LOCK_FILE ]; then
  echo "\n================ This script is already running! lock:  /tmp/sfp_wan.lock \n"
  exit 1
fi

touch $LOCK_FILE

if [ ! -f $LOCK_FILE ]; then
  echo "\n=============== Cannot create lock file!\n"
  exit 1
fi

init_sfp_gpios

# wait for system ready
sleep 10

CURRENT_CHECK_TIMES=0
LATEST_SFP_PRESENT=0
while true
do
    SFP_PRESENT=$(is_sfp_present)
    #echo "check CURRENT_SFP_MODE=$CURRENT_SFP_MODE"
    #echo "check SFP_PRESENT=$SFP_PRESENT"
    #echo "LATEST_SFP_PRESENT=$LATEST_SFP_PRESENT"
    if [ "$SFP_PRESENT" = "1" ] && [ "$LATEST_SFP_PRESENT" = "0" ]; then
        echo "sfp module was plug in!"
        LATEST_SFP_PRESENT=1
        set_sfp_ethwan_sel sfp
        power_down_ext_phy

        #wait
        sleep 3

        detect_sfp_mode
        decide_sgmii_mode_of_sfp
        SFP_LINK_SPEED=$?
        #echo "set SGMII to $SFP_LINK_SPEED"
        start_check_sfp_link $CURRENT_SFP_MODE $SFP_LINK_SPEED

        echo "===== Change to SFP Mode ====="

    elif [ "$SFP_PRESENT" = "0" ] && [ "$LATEST_SFP_PRESENT" = "1" ]; then
        # because the SFP_PRESENT of PMG3000-D20B will HIGH-LOW-HIGH when fiber plug in
        # we donot know why, but I impact the SFP module detect result!
        if [ $CURRENT_CHECK_TIMES -le 3 ]; then
            CURRENT_CHECK_TIMES=$(($CURRENT_CHECK_TIMES+1))
            sleep 1
            continue
        else
            echo "sfp module was removed!"
            LATEST_SFP_PRESENT=0
            set_sfp_ethwan_sel ethwan
            power_up_ext_phy

            #wait
            sleep 3
            set_sgmii_rate 2500

            stop_check_sfp_link

            echo "===== Change to 2.5G PHY Mode ====="
        fi
    fi
    if [ $CURRENT_CHECK_TIMES -ne 0 ]; then
        CURRENT_CHECK_TIMES=0
    fi
    sleep 3
done

rm -f ${LOCK_FILE}

