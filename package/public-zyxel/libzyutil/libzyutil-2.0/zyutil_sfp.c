#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <byteswap.h>
#include "zos_api.h"

#include <math.h>	// for log10

#ifdef BROADCOM_PLATFORM
#include <net/if.h>
#include "bcmnet.h"
#endif
#include "zyutil.h"
#include "zyutil_sfp.h"

#define DEFAULT_I2C_BUS				"/dev/i2c-0"
#define PROC_SFP_PRESENT			"/proc/sfp/present"
#define MAX_READ_BYTES				32

#define DDMI1_ADDRESS				(0xA0>>1)
#define DDMI2_ADDRESS				(0xA2>>1)

#define DDMI_START_OFFSET			0
#define DDMI_MAX_LENGTH				256
#define DDMI1_VENDOR_NAME_OFFSET	20
#define DDMI1_VENDOR_NAME_LENGTH	16
#define DDMI1_VENDOR_PN_OFFSET		40
#define DDMI1_VENDOR_PN_LENGTH		16
#define DDMI1_VENDOR_SN_OFFSET		68
#define DDMI1_VENDOR_SN_LENGTH		16
#define DDMI1_DM_TYPE_OFFSET		92
#define DDMI1_DM_TYPE_LENGTH		1	// Diagnostic monitoring type
    #define INTERNAL_CALIBRATED_OFFEST	5
    #define INTERNAL_CALIBRATED_MASK	(0x1 << INTERNAL_CALIBRATED_OFFEST)
    #define EXTERNAL_CALIBRATED_OFFEST	4
    #define EXTERNAL_CALIBRATED_MASK	(0x1 << EXTERNAL_CALIBRATED_OFFEST)

#define DDMI2_RX_POWER_CALIB_4_OFFSET   56
#define DDMI2_RX_POWER_CALIB_4_LENGTH   4
#define DDMI2_RX_POWER_CALIB_3_OFFSET   60
#define DDMI2_RX_POWER_CALIB_3_LENGTH   4
#define DDMI2_RX_POWER_CALIB_2_OFFSET   64
#define DDMI2_RX_POWER_CALIB_2_LENGTH   4
#define DDMI2_RX_POWER_CALIB_1_OFFSET   68
#define DDMI2_RX_POWER_CALIB_1_LENGTH   4
#define DDMI2_RX_POWER_CALIB_0_OFFSET   72
#define DDMI2_RX_POWER_CALIB_0_LENGTH   4
#define DDMI2_TX_POWER_SLOPE_OFFSET		80
#define DDMI2_TX_POWER_SLOPE_LENGTH		2
#define DDMI2_TX_POWER_OFFSET_OFFSET	82
#define DDMI2_TX_POWER_OFFSET_LENGTH	2
#define DDMI2_TEMP_SLOPE_OFFSET			84
#define DDMI2_TEMP_SLOPE_LENGTH			2
#define DDMI2_TEMP_OFFSET_OFFSET		86
#define DDMI2_TEMP_OFFSET_LENGTH		2
#define DDMI2_TEMPERATURE_OFFSET		96
#define DDMI2_TEMPERATURE_LENGTH		2
#define DDMI2_TX_POWER_OFFSET			102
#define DDMI2_TX_POWER_LENGTH			2
#define DDMI2_RX_POWER_OFFSET			104
#define DDMI2_RX_POWER_LENGTH			2
#define DDMI2_STATUS_CONTROL_OFFSET		110
#define DDMI2_STATUS_CONTROL_LENGTH		1
//GPON SN
#define DDMI2_GPON_SN_OFFSET		    233
#define DDMI2_GPON_SN_LENGTH		    8

    #define DDMI_DATA_READY_BAR_OFFSET	0
    #define DDMI_DATA_READY_BAR_MASK	(0x1 << DDMI_DATA_READY_BAR_OFFSET)	// low is ready.

//DDMI value store as big endian, you may need to do byte swap!
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define DDMI16_TO_HOST(x) bswap_16(x)
#define DDMI32_TO_HOST(x) bswap_32(x)
#else
#define DDMI16_TO_HOST(x) (x)
#define DDMI32_TO_HOST(x) (x)
#endif


//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

#define FETCH_BIT_N(v, n)    (v>>n) & 1

int zyUtil_i2c_open()
{
    int fd = open(DEFAULT_I2C_BUS, O_RDWR);

    if (fd < 0) {
        printf("ERROR: open(%d) failed\n", fd);
        return -1;
    }
    return fd;
}

int zyUtil_i2c_close(int fd)
{
    close(fd);
    return 0;
}

int zyUtil_i2c_setSlaveAddress(int fd, int slave_address)
{
    int status = 0;

    /* Use I2C_SLAVE_FORCE to replace I2C_SLAVE,
    *  because I2C_SLAVE will return -EBUSY when the I2C device already register to driver.
    */
    status = ioctl(fd, I2C_SLAVE_FORCE/*I2C_SLAVE*/, slave_address);
    if (status < 0)
    {
        printf("ERROR: ioctl(fd, I2C_SLAVE_FORCE, 0x%02X) failed\n", slave_address);
        return -1;
    }
    DEBUG_PRINT("\nSUCCESS: ioctl(fd, I2C_SLAVE_FORCE, 0x%02X) passed\n", slave_address);
    return 0;
}

static int _zyUtil_i2c_read_bus(int fd, unsigned char *buf, int bufsize)
{
    return read(fd, buf, bufsize);
}

static int _zyUtil_i2c_write_bus(int fd, unsigned char *buf, int bufsize)
{
    return write(fd, buf, bufsize);
}

int zyUtil_i2c_read_register(int fd, unsigned char reg, unsigned char *buf, int bufsize)
{
    int ret;
    unsigned char *current_buf = buf;
    int current_read_bytes=0;
    int remain_read_bytes=bufsize;

    ret = _zyUtil_i2c_write_bus(fd, &reg, 1);
    if (ret < 0) {
        printf("Failed to write reg: %02x.\n", reg);
        return ret;
    }

    DEBUG_PRINT("wrote %d bytes\n", ret);
    ret = 0;
    do
    {
        if(remain_read_bytes > MAX_READ_BYTES)
            current_read_bytes = MAX_READ_BYTES;
        else
            current_read_bytes = remain_read_bytes;

        ret = _zyUtil_i2c_read_bus(fd, current_buf, current_read_bytes);
        if(ret < 0)
            return ret;
        else
        {
            current_buf += ret;
            remain_read_bytes -= ret;
        }
    }while( remain_read_bytes != 0 );

    return bufsize;
}

int zyUtil_is_sfp_present(void)
{
    FILE *fp = NULL;
    int value=0;
    char str[3] = {0};

    fp = ZOS_FOPEN(PROC_SFP_PRESENT, "r");
    if ( !fp )
    {
        printf("%s : Cannot open %s.\n", __func__, PROC_SFP_PRESENT);
        return -1;
    }

    memset(str, 0, 3);
    if (fgets(str, 2, fp) == NULL)
    {
        ZOS_FCLOSE(fp);
        printf("%s : Get string fail.\n", __func__);
        return -1;
    }

    sscanf(str, "%d", &value);
    ZOS_FCLOSE(fp);

    if( (value != 1) &&  (value!=0) )
    {
        printf("%s : return value out off range.\n", __func__);
        return -1;
    }
    DEBUG_PRINT("%s : return value is %d.\n", __func__, value);
    return value;
}

int zyUtil_is_sfp_txpower(void)
{
    FILE *fp = NULL;
    char str[256] ={0};
	int value = 0;

	if (!strcmp(ZYXEL_PRODUCT_NAME, "EX5601-T0")){
		fp = popen("cat /sys/class/gpio/gpio434/value", "r");
		
		if(fp){
			if( fgets(str, sizeof(str), fp) == NULL )
			{
				DEBUG_PRINT("[%s] : cannot get the return value.\n", __func__);
				pclose(fp);
				return -1;
			}

			pclose(fp);
			if(sscanf(str, "%d", &value)==1)
			{
				if( value == 0 )
					return 1;
				else
					return 0;
			}
			
		}
	}
	return 0;
}



/*!
 *  @return     1 means up, 0 means down, -1 means access fail.
 *
 */
int zyUtil_is_sfp_linkup(void)
{
    FILE *fp = NULL;
    char str[256] ={0};
	int value = 0;

	if(!strcmp(ZYXEL_PRODUCT_NAME, "EX5601-T0"))
	{    fp = popen("cat /sys/class/gpio/gpio468/value", "r");    }
	else
	{
 #ifdef BCM_SDK_504L02
		fp = popen("ethctl phy int 7 1 | awk '{print $5}'", "r");
 #else
		fp = popen("ethctl phy int 6 1 | awk '{print $5}'", "r");
 #endif //BCM_SDK_504L02
	}
	if(fp){
		if( fgets(str, sizeof(str), fp) == NULL )
		{
			DEBUG_PRINT("[%s] : cannot get the return value.\n", __func__);
			pclose(fp);
			return -1;
		}

		pclose(fp);
		if(sscanf(str, "%d", &value)==1)
		{
			if( value == 0 )
				return 1;
			else
				return 0;
		}
		
	}
	else
	{
		fp = popen("ethctl phy int 6 1 | awk '{print $5}'", "r");
		if(fp){
			if( fgets(str, sizeof(str), fp) == NULL )
			{
				DEBUG_PRINT("[%s] : cannot get the return value.\n", __func__);
				pclose(fp);
				return -1;
			}
			pclose(fp);
		}
		if(sscanf(str, "0x%x", &value)==1)
		{
			DEBUG_PRINT("[%s] : mii register 0x1 is 0x%x.\n", __func__, value);
			if( value == 0xffff )
				return 0;
			else{
				value = FETCH_BIT_N(value , 2);
				DEBUG_PRINT("[%s] : bit2 value is %d.\n", __func__, value);
			}
		}
		else{
			DEBUG_PRINT("[%s] : read mii error.\n", __func__);
			return -1;
		}
	}
    return value;
}

// Reference from https://github.com/mozilla-b2g/i2c-tools/blob/master/lib/smbus.c
__s32 i2c_smbus_access(int file, char read_write, __u8 command,
		       int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	__s32 err;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	err = ioctl(file, I2C_SMBUS, &args);
	if (err == -1)
		err = -errno;
	return err;
}

__s32 i2c_smbus_read_byte(int file)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

// Reference from https://github.com/mozilla-b2g/i2c-tools/blob/master/tools/i2cdetect.c
int zyUtil_bosa_exist(void)
{
    int status = 0;
	int res = 0;
    int busfd;

    busfd = zyUtil_i2c_open();
    // I2C_SLAVE will return -EBUSY when the I2C device already register to driver.
    res = ioctl(busfd, I2C_SLAVE, DDMI1_ADDRESS);
    DEBUG_PRINT("\nSUCCESS: ioctl(fd, I2C_SLAVE, 0x%02X) return %d(errno=%d)\n", DDMI1_ADDRESS, res, errno);
    if( res < 0 )
    {
        if( errno == EBUSY )
            status = 1;
        goto end;
    }

    // Probe the address
    res = i2c_smbus_read_byte(busfd);
    if( res < 0 )
    {
        DEBUG_PRINT("%s : cannot read data from i2c device %d.\n", __func__, DDMI1_ADDRESS);
        status = 0;
    }
    else
        status = 1;

end:
    zyUtil_i2c_close(busfd);

    return status;
}

int _printString(unsigned char *_string, int length)
{
    int i=0;
    for(i=0;i<length;i++)
        DEBUG_PRINT("0x%02X ", _string[i]);
    return 0;
}


void zyUtilConvertSfpSnToString(unsigned char *_string2, int length, char *sn_buf)
{
	int i = 0;
	char sn_tmp[DDMI_STRING_MAX_LENGTH +1] = {0};
	char temp[3] = {0};

	for(i=0;i<length;i++){
		zos_snprintf(temp, sizeof(temp), "%02X" ,_string2[i]);
		ZOS_STRCAT(sn_tmp, temp, sizeof(sn_tmp));
	}
	memcpy(sn_buf , sn_tmp, DDMI_STRING_MAX_LENGTH);
}

void dumpString(unsigned char *_string, int length)
{
	int i;
	for(i=0;i<length;i++){
		/* cut garbled ending, ASC char out of 32~126*/
		if( _string[i] < 32 || _string[i] > 126 )
			_string[i] = '\0';
	}
	if(0<length)
		_string[length-1] = '\0';
}

#ifdef WIND_ITALY_CUSTOMIZATION //show GPON serial

int dump_DDMI_state(sfp_ddmi_state_t *ddmi_state)
{
    dumpString( &ddmi_state->sfp_vendor, DDMI_STRING_MAX_LENGTH);
    dumpString( &ddmi_state->sfp_pn, DDMI_STRING_MAX_LENGTH);

    printf("DDMI state:\n");
    printf("sfp/bosa present   = %d\n",        ddmi_state->present);
    printf("DDMI vendor name   = %s\n",        ddmi_state->sfp_vendor);
    printf("DDMI part number   = %s\n",        ddmi_state->sfp_pn);   
    printf("DDMI rx power      = %f dbm\n",    ddmi_state->rx_power);
    printf("DDMI tx power      = %f dbm\n",    ddmi_state->tx_power);
    printf("DDMI temperature   = %f C\n",    ddmi_state->temperature);

    char gpon_sn_string[DDMI_STRING_MAX_LENGTH + 1] = {0};
    zyUtilConvertSfpSnToString( ddmi_state->sfp_sn, DDMI2_GPON_SN_LENGTH, gpon_sn_string);
    //gpon_sn_string[DDMI_STRING_MAX_LENGTH]='\0';
    printf("DDMI GPON serial   = %s\n\n", gpon_sn_string);
    return 0;
}

#else
int dump_DDMI_state(sfp_ddmi_state_t *ddmi_state)
{
	dumpString( ddmi_state->sfp_vendor, DDMI_STRING_MAX_LENGTH);
	dumpString( ddmi_state->sfp_pn, DDMI_STRING_MAX_LENGTH);
	dumpString( ddmi_state->sfp_sn, DDMI_STRING_MAX_LENGTH);

    printf("DDMI state:\n");
    printf("sfp/bosa present   = %d\n",        ddmi_state->present);
    printf("DDMI vendor name   = %s\n",        ddmi_state->sfp_vendor);
    printf("DDMI part number   = %s\n",        ddmi_state->sfp_pn);
    printf("DDMI serial number = %s\n",        ddmi_state->sfp_sn);
    printf("DDMI rx power      = %f dbm\n",    ddmi_state->rx_power);
    printf("DDMI tx power      = %f dbm\n",    ddmi_state->tx_power);
    printf("DDMI temperature   = %f C\n",    ddmi_state->temperature);

    return 0;
}
#endif

float _extCalibrationRxPower_uW(unsigned short rxPower, float calibate0, float calibate1, float calibate2, float calibate3, float calibate4)
{
    return (calibate0 +
            calibate1 * (float)(pow((double)rxPower, 1)) +
            calibate2 * (float)(pow((double)rxPower, 2)) +
            calibate3 * (float)(pow((double)rxPower, 3)) +
            calibate4 * (float)(pow((double)rxPower, 4)));
}

double _uw_to_dbm(double input)
{
    return (double)10*log10((double)input/(double)10000);
}

float _read_DDMI_INT32(unsigned char *buf, int offset)
{
    float value=0;
    memcpy(&value, buf+offset, 4);
    return DDMI32_TO_HOST(value);
}

short int _read_DDMI_INT16(unsigned char *buf, int offset)
{
    short int value=0;
    memcpy(&value, buf+offset, 2);
    return DDMI16_TO_HOST(value);
}

unsigned short _read_DDMI_UINT16(unsigned char *buf, int offset)
{
    short int value=0;
    memcpy(&value, buf+offset, 2);
    return DDMI16_TO_HOST(value);
}

int zyUtil_sfp_read_ddmi_state(sfp_ddmi_state_t *ddmi_state)
{
    int busfd;
    unsigned char ddmi1_data[DDMI_MAX_LENGTH];
    unsigned char ddmi2_data[DDMI_MAX_LENGTH];

    int isIntCalibrated=0, isExtCalibrated=0;
    unsigned short _rx_power=0, _tx_power=0;
    short int _temp=0;

    memset(ddmi1_data, 0x0, sizeof(unsigned char)*DDMI_MAX_LENGTH);
    memset(ddmi2_data, 0x0, sizeof(unsigned char)*DDMI_MAX_LENGTH);

    // Read out DDMI value from SFP through I2C
    busfd = zyUtil_i2c_open();
    if( busfd < 0 )
        return -1;
    // Read DDMI1
    if( zyUtil_i2c_setSlaveAddress(busfd, DDMI1_ADDRESS) != 0 ) {
        zyUtil_i2c_close(busfd);
        return -1;
    }
    if( zyUtil_i2c_read_register(busfd, DDMI_START_OFFSET, ddmi1_data, DDMI_MAX_LENGTH) < 0) {
        zyUtil_i2c_close(busfd);
        return -1;
    }
    // Read DDMI2
    if( zyUtil_i2c_setSlaveAddress(busfd, DDMI2_ADDRESS) != 0 ) {
        zyUtil_i2c_close(busfd);
        return -1;
    }
    if( zyUtil_i2c_read_register(busfd, DDMI_START_OFFSET, ddmi2_data, DDMI_MAX_LENGTH) < 0) {
        zyUtil_i2c_close(busfd);
        return -1;
    }
    zyUtil_i2c_close(busfd);

    // Check if the DDMI data is ready
    if ( ddmi2_data[DDMI2_STATUS_CONTROL_OFFSET] & DDMI_DATA_READY_BAR_MASK )
    {
        // true means the DDMI data is not ready.
        printf("The DDMI data is not ready.\n");
        return -1;
    }
    // copy the result of sfp present.
    ddmi_state->present = ((zyUtil_is_sfp_present() > 0) || (zyUtil_bosa_exist() > 0))?1:0;
    // Copy vendor name
    memcpy(ddmi_state->sfp_vendor, ddmi1_data+DDMI1_VENDOR_NAME_OFFSET, DDMI1_VENDOR_NAME_LENGTH);
    DEBUG_PRINT("\nVendor name: ");_printString(ddmi1_data+DDMI1_VENDOR_NAME_OFFSET, DDMI1_VENDOR_NAME_LENGTH);
    // Copy part number
    memcpy(ddmi_state->sfp_pn, ddmi1_data+DDMI1_VENDOR_PN_OFFSET, DDMI1_VENDOR_PN_LENGTH);
    DEBUG_PRINT("\npart number: ");_printString(ddmi1_data+DDMI1_VENDOR_PN_OFFSET, DDMI1_VENDOR_PN_LENGTH);
	
    // Check internal/external calibration state
    DEBUG_PRINT("\nddmi1_data[%d]=0x%02x\n", DDMI1_DM_TYPE_OFFSET, ddmi1_data[DDMI1_DM_TYPE_OFFSET]);
    if ( ddmi1_data[DDMI1_DM_TYPE_OFFSET] && INTERNAL_CALIBRATED_MASK )
    {
        isIntCalibrated = 1;
        DEBUG_PRINT("isIntCalibrated=%d\n", isIntCalibrated);
    }
    if ( ddmi1_data[DDMI1_DM_TYPE_OFFSET] && EXTERNAL_CALIBRATED_MASK )
    {
        isExtCalibrated = 1;
        DEBUG_PRINT("isExtCalibrated=%d\n", isExtCalibrated);
    }

    // prepare the value.
    DEBUG_PRINT("_rx_power is 0x%02X 0x%02X\n", ddmi2_data[DDMI2_RX_POWER_OFFSET], ddmi2_data[DDMI2_RX_POWER_OFFSET+1]);
    DEBUG_PRINT("_tx_power is 0x%02X 0x%02X\n", ddmi2_data[DDMI2_TX_POWER_OFFSET], ddmi2_data[DDMI2_TX_POWER_OFFSET+1]);
    DEBUG_PRINT("_temp is 0x%02X 0x%02X\n", ddmi2_data[DDMI2_TEMPERATURE_OFFSET], ddmi2_data[DDMI2_TEMPERATURE_OFFSET+1]);

    _rx_power = _read_DDMI_UINT16(ddmi2_data, DDMI2_RX_POWER_OFFSET);
    _tx_power = _read_DDMI_UINT16(ddmi2_data, DDMI2_TX_POWER_OFFSET);
    _temp = _read_DDMI_INT16(ddmi2_data, DDMI2_TEMPERATURE_OFFSET);
    DEBUG_PRINT("_rx_power=%hi, _tx_power=%hi, _temp=%hu\n", _rx_power, _tx_power, _temp);

    if(isIntCalibrated)
    {
        /*the valid value is -40~+8.2, set -41 means power = 0(can't operation for log)*/
        // calculate rx power
        ddmi_state->rx_power = _rx_power?(float)(_uw_to_dbm((double)_rx_power)):(float)(-41);
        // calculate tx power
        ddmi_state->tx_power = _tx_power?(float)(_uw_to_dbm((double)_tx_power)):(float)(-41);
        // calculate temperature
        ddmi_state->temperature = (float)_temp/(float)256;
    }else if (isExtCalibrated)
    {
        unsigned short _slope=0;
        short int _offset=0;
        float _rx_cal0=0, _rx_cal1=0, _rx_cal2=0,_rx_cal3=0, _rx_cal4=0;

        // calculate rx power
        _rx_cal0 = _read_DDMI_INT32(ddmi2_data, DDMI2_RX_POWER_CALIB_0_OFFSET);
        _rx_cal1 = _read_DDMI_INT32(ddmi2_data, DDMI2_RX_POWER_CALIB_1_OFFSET);
        _rx_cal2 = _read_DDMI_INT32(ddmi2_data, DDMI2_RX_POWER_CALIB_2_OFFSET);
        _rx_cal3 = _read_DDMI_INT32(ddmi2_data, DDMI2_RX_POWER_CALIB_3_OFFSET);
        _rx_cal4 = _read_DDMI_INT32(ddmi2_data, DDMI2_RX_POWER_CALIB_4_OFFSET);
        DEBUG_PRINT("_rx_cal0=%f, _rx_cal1=%f, _rx_cal2=%f, _rx_cal3=%f, _rx_cal4=%f\n", _rx_cal0, _rx_cal1, _rx_cal2, _rx_cal3, _rx_cal4);
        ddmi_state->rx_power = _rx_power?(float)_uw_to_dbm(_extCalibrationRxPower_uW(_rx_power, _rx_cal0, _rx_cal1, _rx_cal2, _rx_cal3, _rx_cal4)):(float)(-41);
        // calculate tx power
        _slope = _read_DDMI_UINT16(ddmi2_data, DDMI2_TX_POWER_SLOPE_OFFSET);
        _offset = _read_DDMI_INT16(ddmi2_data, DDMI2_TX_POWER_OFFSET_OFFSET);
        DEBUG_PRINT("_tx__slope=%hu, _tx_offset=%hi\n", _slope, _offset);
        ddmi_state->tx_power = _tx_power?(float)_uw_to_dbm((double)((float)(((float)_slope/(float)256)*((float)_tx_power)) + (float)_offset)):(float)(-41);
        // calculate temperature.
        _slope = _read_DDMI_UINT16(ddmi2_data, DDMI2_TEMP_SLOPE_OFFSET);
        _offset = _read_DDMI_INT16(ddmi2_data, DDMI2_TEMP_OFFSET_OFFSET);
        DEBUG_PRINT("_temp__slope=%hu, _temp_offset=%hi\n", _slope, _offset);
        ddmi_state->temperature = ((float)((float)_slope/(float)256))*((float)_temp) + (float)_offset;
    }else
    {
        ddmi_state->rx_power = -41;
        ddmi_state->tx_power = -41;
        ddmi_state->temperature = 0;
    }

	//check filber is connected or not(rx lose bit). fix bug for sfp tx reading. workaround.
	if (ddmi2_data[DDMI2_STATUS_CONTROL_OFFSET]){
		ddmi_state->rx_power = -41;
		ddmi_state->tx_power = -41;
	}

#ifdef WIND_ITALY_CUSTOMIZATION //show GPON serial
	memcpy(ddmi_state->sfp_sn, ddmi2_data+DDMI2_GPON_SN_OFFSET, DDMI2_GPON_SN_LENGTH);
#else
		// Copy serial name
	memcpy(ddmi_state->sfp_sn, ddmi1_data+DDMI1_VENDOR_SN_OFFSET, DDMI1_VENDOR_SN_LENGTH);
	DEBUG_PRINT("\nSerial number: ");_printString(ddmi1_data+DDMI1_VENDOR_SN_OFFSET, DDMI1_VENDOR_SN_LENGTH);
#endif //WIND_ITALY_CUSTOMIZATION

    return 0;
}

#ifdef BROADCOM_PLATFORM
int zyUtil_netdevice_sfp_info(char* ifname, int* intIfBitRate, int* intIfDuplex)
{
    struct ifreq i = {0};
    IOCTL_MIB_INFO out = {0};
    ZOS_STRNCPY(i.ifr_name, ifname, IFNAMSIZ);
    i.ifr_data = (char *)&out;
    int32_t sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(ioctl(sock, SIOCMIBINFO, &i) != 0){
        perror("ioctl error");
      close(sock);
        return -1;
    }
    close(sock);
   *intIfDuplex = (int)out.ulIfDuplex;
   *intIfBitRate = (int)out.ulIfSpeed;
    return 0;
}
#endif

