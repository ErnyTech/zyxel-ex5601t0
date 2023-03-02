#ifdef QUALCOMM_PLATFORM

#include <ctype.h>
#include <unistd.h>
#include "zcfg_common.h"
#include "zcfg_debug.h"
#include "libzyutil_wrapper.h"
#include "libzyutil_qualcomm_wrapper.h"

#ifdef SUPPORT_LANVLAN
zcfgRet_t zyUtilISetIfState(char *ifName, bool up)
{
	zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
	zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}
#endif

#define zyUtilIGetSerailNum zyUtilIGetSerialNumber

zcfgRet_t zyUtilIGetBaseMAC(char *mac)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
	// TODO Read real data from k-data.
  zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X", 0, 1, 2, 3, 4, 5);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNthMAC(char *mac, int nth)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGetNthMACwithColon(char *mac, int nth)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGetFeatureBits(char *featurebits)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGetModelID(char *model_id)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}
zcfgRet_t zyUtilIGetDSLVersion(char *DSLversion)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}
zcfgRet_t zyUtilIGetSerialNumber(char *serianumber)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
  /* TODO, need to append sub-sequence number */
#endif
  return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetSubSerialNumber(char *sub_serianumber)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}

int zyUtilGetGPIO(int gpioNum)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return 0;
}

float transferOpticalTemperature(int input_value)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return 0;
}
zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return 0;
}
float transferOpticalPower(float input_value)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return 0;
}

zcfgRet_t zyUtilIGetBaseMACwithColon(char *mac)
{
  zcfgLog(ZCFG_LOG_DEBUG, ">> %s\n", __FUNCTION__);
  zcfgLog(ZCFG_LOG_DEBUG, "<< %s\n", __FUNCTION__);
  return ZCFG_SUCCESS;
}

/*!
 *  get size of rootfs
 *
 *  @return     size of rootfs (byte)
 *
 *  @note  If get rootfs fail, popen fail, or fgets fail, return 0.
 */
int ras_size()
{
	int size=0;
	FILE *fp = NULL;
	char buffer[64] = {0};

	fp = popen("cat /proc/mtd  | grep rootfs | awk '{print $2}'", "r");

	if (fp == NULL)
	{
		zyutil_dbg_printf("ERROR : popen fail\n");
		return 0;
	}

	if (fgets(buffer, 64, fp) == NULL)
	{
		zyutil_dbg_printf("ERROR : fgets fail\n");
		pclose(fp);
		return 0;
	}

	sscanf(buffer, "%X", &size);
	pclose(fp);

	return size;

}

/*!
 *  check upload file (FW) magic number
 *
 *  @param[in]  uploadfilename	the name of upload file
 *
 *  @return     0 - non-match
 *              1 - match
 *
 *  @note  If uploadfilename is NULL, popen fail, or fgets fail, return 0.
 */
int magic_number_check(char *uploadfilename)
{
	int magic_number=-1;
	FILE *fp = NULL;
	char buffer[64] = {0};
	char cmd[256] = {0};

	if (uploadfilename == NULL)
	{
	    zyutil_dbg_printf("ERROR : uploadfilename == NULL\n");
		return 0;
	}

	zos_snprintf(cmd, sizeof(cmd), "hexdump -n 2 /var/fw/%s", uploadfilename);
	fp = popen(cmd, "r");
	if (fp == NULL)
	{
	    zyutil_dbg_printf("ERROR : popen hexdump -n 2 /var/fw/%s fail\n", uploadfilename);
		return 0;
	}

	if (fgets(buffer, 64, fp) == NULL)
	{
	    zyutil_dbg_printf("ERROR : fgets fail\n");
	    pclose(fp);
		return 0;
	}

	sscanf(buffer, "0000000 %d ", &magic_number);
	pclose(fp);

	if (magic_number == MagicNubmer)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

#endif
