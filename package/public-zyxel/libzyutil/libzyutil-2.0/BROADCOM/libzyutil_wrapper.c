#ifdef BROADCOM_PLATFORM

#include "libzyutil_wrapper.h"
#include "libzyutil_brcm_wrapper.h"

/*from common.c*/
#include "board.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "zos_api.h"


#define BOARD_DEVICE_NAME  "/dev/brcmboard"

#if defined(ZYPRJ_VMG3926) && defined(SAME_FW_FOR_TWO_BOARD) && defined(BCM_SDK_416L05)
#define PROC_NVRAM_BOARDID  "/proc/nvram/boardid"
#endif

#ifdef ZYXEL_AT_SET_KEY
//int zyUtilIsPrintableString(char *tmp, int len){
//    int i=0;
//    for( i=0 ; i<len ; i++ )
//    {
//        if( (tmp[i] < 0x20) || (tmp[i] > 0x7e) )
//            return ZCFG_INTERNAL_ERROR;
//    }
//    return ZCFG_SUCCESS;
//}
extern int zyUtilIsPrintableString(char *tmp, int len);

#ifdef ZYXEL_AT_SET_SUPERVISOR_KEY
int zyGetSupervisorPWD(char *buf, int len){
    NVRAM_DATA nvramData;
    int _string_len=0;

    if( (NVRAM_SUPER_PWD_LEN+1) != len) {
        //printf(" #### zyGetSupervisorPWD #### - Failed zyGetSupervisorPWD Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }

    memset(&nvramData, 0, sizeof(NVRAM_DATA));
    memset(buf, 0, len);

    if(nvramDataGet(&nvramData) < 0){
        //printf(" #### zyGetSupervisorPWD #### - Failed zyGetSupervisorPWD Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }
    //printf(" #### zyGetSupervisorPWD #### - nvramData.supervisor = %s\n", nvramData.supervisor);
    //printf(" #### zyGetSupervisorPWD #### - nvramData.supervisor len = %d\n", strlen(nvramData.supervisor));
    _string_len = strlen((char *)nvramData.supervisor);
    if(_string_len>NVRAM_SUPER_PWD_LEN)
        _string_len = NVRAM_SUPER_PWD_LEN;

    if ( _string_len == 0){
        //printf(" #### zyGetSupervisorPWD #### - Failed - zyGetSupervisorPWD Empty\n");
        return ZCFG_INTERNAL_ERROR;
    }else if( zyUtilIsPrintableString((char *)nvramData.supervisor, _string_len) != 0 ){
        //printf(" #### zyGetSupervisorPWD #### - Failed - zyGetSupervisorPWD is not printable string\n");
        return ZCFG_INTERNAL_ERROR;
    }
    memcpy(buf, nvramData.supervisor, NVRAM_SUPER_PWD_LEN);
    printf("%s Set to default passowrd\n", __FUNCTION__);

    return ZCFG_SUCCESS;
}

int zyGetSupervisorPWDMaxLength(void){
	return NVRAM_SUPER_PWD_LEN;
}
#endif

int zyGetAdminPWD(char *buf, int len){
    NVRAM_DATA nvramData;
    int _string_len=0;

    if( (NVRAM_ADMIN_PWD_LEN+1) != len) {
        //printf(" #### zyGetAdminPWD #### - Failed zyGetAdminPWD Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }

    memset(&nvramData, 0, sizeof(NVRAM_DATA));
    memset(buf, 0, len);

    if(nvramDataGet(&nvramData) < 0){
        //printf(" #### zyGetAdminPWD #### - Failed zyGetAdminPWD Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }
    //printf(" #### zyGetAdminPWD #### - nvramData.admin = %s\n", nvramData.admin);
    //printf(" #### zyGetAdminPWD #### - nvramData.admin len = %d\n", strlen(nvramData.admin));
    _string_len = strlen((char *)nvramData.admin);
    if(_string_len>NVRAM_ADMIN_PWD_LEN)
        _string_len = NVRAM_ADMIN_PWD_LEN;

    if ( _string_len == 0){
        //printf(" #### zyGetAdminPWD #### - Failed - zyGetAdminPWD Empty\n");
        return ZCFG_INTERNAL_ERROR;
    }else if( zyUtilIsPrintableString((char *)nvramData.admin, _string_len) != 0 ){
        //printf(" #### zyGetAdminPWD #### - Failed - zyGetAdminPWD is not printable string\n");
        return ZCFG_INTERNAL_ERROR;
    }
    memcpy(buf, nvramData.admin, NVRAM_ADMIN_PWD_LEN);
    printf("%s Set to default passowrd\n", __FUNCTION__);

    return ZCFG_SUCCESS;
}

int zyGetAdminPWDMaxLength(void){
	return NVRAM_ADMIN_PWD_LEN;
}

int zyGetPSK(char *buf, int len){
    NVRAM_DATA nvramData;
    int _string_len=0;

    if( (NVRAM_KEYPASSPHRASE_LEN+1) != len) {
        //printf(" #### zyGetPSK #### - Failed zyGetPSK Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }

    memset(&nvramData, 0, sizeof(NVRAM_DATA));
    memset(buf, 0, len);

    if(nvramDataGet(&nvramData) < 0){
        //printf(" #### zyGetPSK #### - Failed zyGetPSK Failed\n");
        return ZCFG_INTERNAL_ERROR;
    }
    //printf(" #### zyGetPSK #### - nvramData.WpaPskKey = %s\n", nvramData.WpaPskKey);
    //printf(" #### zyGetPSK #### - nvramData.WpaPskKey len = %d\n", strlen(nvramData.WpaPskKey));
    _string_len = strlen((char *)nvramData.WpaPskKey);
    if(_string_len>NVRAM_KEYPASSPHRASE_LEN)
        _string_len = NVRAM_KEYPASSPHRASE_LEN;

    if ( _string_len == 0){
        //printf(" #### zyGetPSK #### - Failed - zyGetPSK Empty\n");
        return ZCFG_INTERNAL_ERROR;
    }else if( zyUtilIsPrintableString((char *)nvramData.WpaPskKey, _string_len) != 0 ){
        //printf(" #### zyGetPSK #### - Failed - zyGetPSK is not printable string\n");
        return ZCFG_INTERNAL_ERROR;
    }
    memcpy(buf, nvramData.WpaPskKey, NVRAM_KEYPASSPHRASE_LEN);
    printf("%s Set to default Psk\n", __FUNCTION__);

    return ZCFG_SUCCESS;
}

int zyGetPSKMaxLength(void){
	return NVRAM_KEYPASSPHRASE_LEN;
}
#endif

#ifdef SUPPORT_LANVLAN
zcfgRet_t zyUtilISetIfState(char *ifName, bool up)
{
	int sockfd = 0;
	struct ifreq ifr;
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(ifName == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Cannot bring up NULL interface\n", __FUNCTION__);
		ret = ZCFG_INTERNAL_ERROR;
	}
	else {
		/* Create a channel to the NET kernel. */
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Cannot create socket to the NET kernel\n", __FUNCTION__);
			ret = ZCFG_INTERNAL_ERROR;
		}
		else {
			strncpy(ifr.ifr_name, ifName, IFNAMSIZ);
			// get interface flags
			if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) != -1) {
				if(up)
					ifr.ifr_flags |= IFF_UP;
				else
					ifr.ifr_flags &= (~IFF_UP);

				if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
					zcfgLog(ZCFG_LOG_ERR, "%s : Cannot ioctl SIOCSIFFLAGS on the socket\n", __FUNCTION__);
					ret = ZCFG_INTERNAL_ERROR;
				}
			}
			else {
				zcfgLog(ZCFG_LOG_ERR, "%s : Cannot ioctl SIOCGIFFLAGS on the socket\n", __FUNCTION__);
				ret = ZCFG_INTERNAL_ERROR;
			}

			close(sockfd);
		}
	}

	return ret;
}
#endif

static uint32_t getCrc32(unsigned char *pdata, uint32_t size, uint32_t crc)
{
	while (size-- > 0)
		crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

	return crc;
}

int nvramDataWrite(NVRAM_DATA *nvramData)
{
	int boardFd = 0;
	int rc = 0;
	unsigned int offset = 0;
	BOARD_IOCTL_PARMS ioctlParms;
	uint32_t crc = CRC32_INIT_VALUE;

	nvramData->ulCheckSum = 0;
	crc = getCrc32((unsigned char *)nvramData, sizeof(NVRAM_DATA), crc);
	nvramData->ulCheckSum = crc;

	boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

	if(boardFd != -1) {
		ioctlParms.string = (char *)nvramData;
		ioctlParms.strLen = sizeof(NVRAM_DATA);
		ioctlParms.offset = offset;
		ioctlParms.action = NVRAM;
		ioctlParms.buf    = NULL;
		ioctlParms.result = -1;

		rc = ioctl(boardFd, BOARD_IOCTL_FLASH_WRITE, &ioctlParms);
		close(boardFd);

		if(rc < 0) {
			printf("%s Set NVRAM Failure\n", __FUNCTION__);
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

int nvramDataGet(NVRAM_DATA *nvramData)
{
	int boardFd = 0;
	int rc = 0;
	unsigned int offset = 0;
	BOARD_IOCTL_PARMS ioctlParms;

	boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

	if(boardFd != -1) {
		ioctlParms.string = (char *)nvramData;
		ioctlParms.strLen = sizeof(NVRAM_DATA);
		ioctlParms.offset = offset;
		ioctlParms.action = NVRAM;
		ioctlParms.buf    = NULL;
		ioctlParms.result = -1;

		rc = ioctl(boardFd, BOARD_IOCTL_FLASH_READ, &ioctlParms);
		close(boardFd);

		if(rc < 0) {
			printf("%s Get NVRAM Failure\n", __FUNCTION__);
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

int zyFwInfoGet(ZY_FW_INFO *zyFwInfo)
{
	int boardFd = 0;
	int rc = 0;
	unsigned int offset = 0;
	BOARD_IOCTL_PARMS ioctlParms;

	boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

	if(boardFd != -1) {
		ioctlParms.string = (char *)zyFwInfo;
		ioctlParms.strLen = sizeof(ZY_FW_INFO);
		ioctlParms.offset = offset;
		ioctlParms.action = 0;
		ioctlParms.buf    = NULL;
		ioctlParms.result = -1;

		rc = ioctl(boardFd, BOARD_IOCTL_GET_ZYFWINFO, &ioctlParms);
		close(boardFd);

		if(rc < 0) {
			printf("%s Get ZY_FW_INFO Failure\n", __FUNCTION__);
			return -1;
		}
	}
	else {
		return -1;
	}

	return 0;
}

int zyFwInfoNonBootGet(ZY_FW_INFO *zyFwInfo)
{
#if defined(BCM_SDK_502L05) || defined(BCM_SDK_502L06) || defined(BCM_SDK_502L07)
	int boardFd = 0;
	int rc = 0;
	unsigned int offset = 0;
	BOARD_IOCTL_PARMS ioctlParms;

	boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

	if(boardFd != -1) {
		ioctlParms.string = (char *)zyFwInfo;
		ioctlParms.strLen = sizeof(ZY_FW_INFO);
		ioctlParms.offset = offset;
		ioctlParms.action = 0;
		ioctlParms.buf    = NULL;
		ioctlParms.result = -1;

		rc = ioctl(boardFd, BOARD_IOCTL_GET_NONBOOTZYFWINFO, &ioctlParms);
		close(boardFd);

		if(rc < 0) {
			printf("%s Get ZY_FW_INFO, NONBOOT Failure\n", __FUNCTION__);
			return -1;
		}
	}
	else {
		return -1;
	}

#else //defined(BCM_SDK_502L05) || defined(BCM_SDK_502L06) || defined(BCM_SDK_502L07)
	printf("zyFwInfoNonBootGet() only support BRCM SDK 502L05 and later, you need to add version check in this API\n");
#endif //defined(BCM_SDK_502L05) || defined(BCM_SDK_502L06) || defined(BCM_SDK_502L07)
	return 0;
}

/*-------------------------------------------------
 * eraseRomdPartition() :
 * Here, the second parameter of boardIoctl()
 *  we gave is defined in the type enumeration
 *	BOARD_IOCTL_ACTION_ERASE_PARTITION.
 *-------------------------------------------------
int eraseRomdPartition(void)
{
	int ret = 0;
	ret = boardIoctl(BOARD_IOCTL_ERASE_PARTITION, ERASE_PAR_ROMD, "", 0, 0, "");

	return ret;
}*/

zcfgRet_t zyUtilIGetBaseMAC(char *mac)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X",
		nvramData.ucaBaseMacAddr[0],
		nvramData.ucaBaseMacAddr[1],
		nvramData.ucaBaseMacAddr[2],
		nvramData.ucaBaseMacAddr[3],
		nvramData.ucaBaseMacAddr[4],
		nvramData.ucaBaseMacAddr[5]
	);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBaseMACwithColon(char *mac)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	zos_snprintf(mac , MAC_ADDRESS_WITH_COLON, "%02X:%02X:%02X:%02X:%02X:%02X",
		nvramData.ucaBaseMacAddr[0],
		nvramData.ucaBaseMacAddr[1],
		nvramData.ucaBaseMacAddr[2],
		nvramData.ucaBaseMacAddr[3],
		nvramData.ucaBaseMacAddr[4],
		nvramData.ucaBaseMacAddr[5]
	);
	return ZCFG_SUCCESS;
}

/*!
 *  Get Nth MAC.
 *  Broadcom platform do not need to use this function.
 *
 *  @param[out]    pucaMacAddr   out N`th MAC, should be 6 bytes.
 *  @param[in]     n             Get N`th MAC.
 *
 *  @note  The MAC length should be 6 bytes.
 */
zcfgRet_t zyUtilIGetNthMAC(char *mac, int nth)
{
	unsigned long macsequence = 0;
	unsigned char pucaMacAddr[NVRAM_MAC_ADDRESS_LEN]={0};
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	/* Start with the base address */
	memcpy( pucaMacAddr, nvramData.ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);

	/* Work with only least significant three bytes of base MAC address */
	macsequence = (pucaMacAddr[3] << 16) | (pucaMacAddr[4] << 8) | pucaMacAddr[5];
	macsequence = (macsequence + nth) & 0xffffff;
	pucaMacAddr[3] = (macsequence >> 16) & 0xff;
	pucaMacAddr[4] = (macsequence >> 8) & 0xff;
	pucaMacAddr[5] = (macsequence ) & 0xff;

	zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X",
		pucaMacAddr[0],
		pucaMacAddr[1],
		pucaMacAddr[2],
		pucaMacAddr[3],
		pucaMacAddr[4],
		pucaMacAddr[5]
	);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNthMACwithColon(char *mac, int nth)
{
	unsigned long macsequence = 0;
	unsigned char pucaMacAddr[NVRAM_MAC_ADDRESS_LEN]={0};
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	/* Start with the base address */
	memcpy( pucaMacAddr, nvramData.ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);

	/* Work with only least significant three bytes of base MAC address */
	macsequence = (pucaMacAddr[3] << 16) | (pucaMacAddr[4] << 8) | pucaMacAddr[5];
	macsequence = (macsequence + nth) & 0xffffff;
	pucaMacAddr[3] = (macsequence >> 16) & 0xff;
	pucaMacAddr[4] = (macsequence >> 8) & 0xff;
	pucaMacAddr[5] = (macsequence ) & 0xff;

	zos_snprintf(mac , MAC_ADDRESS_WITH_COLON, "%02X:%02X:%02X:%02X:%02X:%02X",
		pucaMacAddr[0],
		pucaMacAddr[1],
		pucaMacAddr[2],
		pucaMacAddr[3],
		pucaMacAddr[4],
		pucaMacAddr[5]
	);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	if(num) *num = nvramData.ulNumMacAddrs;
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFeatureBits(char *featurebits)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGetModelID(char *model_id)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	memcpy(model_id, nvramData.FeatureBits, 4);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
#if 0
	if(!strcmp(ZYXEL_PRODUCT_NAME, "VMG8924-B10B") || !strcmp(ZYXEL_PRODUCT_NAME, "VMG3926-B10A")){
		int ret = 0, gpioNum = 11;
		ret = zyUtilGetGPIO(gpioNum);
		if(ret){//VMG3926
			ZOS_STRNCPY(pdname, "VMG3926-B10A", NVRAM_PRODUCTNAME_LEN);
		}
		else{//VMG8924
			ZOS_STRNCPY(pdname, "VMG8924-B10B", NVRAM_PRODUCTNAME_LEN);
		}
	}
	else{
		NVRAM_DATA nvramData;
		memset(&nvramData, 0, sizeof(NVRAM_DATA));

		if(nvramDataGet(&nvramData) < 0)
			return ZCFG_INTERNAL_ERROR;

		/*Length of NVRAM_DATA->ProductName is NVRAM_PRODUCTNAME_LEN, 32, Yen-Chun,Lin, 20210324*/
		ZOS_STRNCPY(pdname, nvramData.ProductName, NVRAM_PRODUCTNAME_LEN);
	}
#else
	NVRAM_DATA nvramData;
	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	/*Length of NVRAM_DATA->ProductName is NVRAM_PRODUCTNAME_LEN, 32, Yen-Chun,Lin, 20210324*/
	ZOS_STRNCPY(pdname, nvramData.ProductName, NVRAM_PRODUCTNAME_LEN);
#if defined(ZYPRJ_VMG3926) && defined(SAME_FW_FOR_TWO_BOARD) && defined(BCM_SDK_416L05)
{
    FILE *fp = NULL;
    char str[NVRAM_BOARD_ID_STRING_LEN] = {0};

    fp = ZOS_FOPEN(PROC_NVRAM_BOARDID, "r");
    if ( !fp )
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : Cannot open %s.\n", __func__, PROC_NVRAM_BOARDID);
        return -1;
    }

    memset(str, 0, NVRAM_BOARD_ID_STRING_LEN);
    if (fgets(str, sizeof(str), fp) == NULL)
    {
        ZOS_FCLOSE(fp);
        zcfgLog(ZCFG_LOG_ERR, "%s : Get string fail.\n", __func__);
        return -1;
    }

    if (!strncmp(str, RUNTIME_BOARDID_VMG3925_B10B, strlen(RUNTIME_BOARDID_VMG3925_B10B)))
    {
    	/*Length of NVRAM_DATA->ProductName is NVRAM_PRODUCTNAME_LEN, 32, Yen-Chun,Lin, 20210324*/
        memset(pdname, 0, NVRAM_PRODUCTNAME_LEN);
        ZOS_STRNCPY(pdname, "VMG3925-B10B",  NVRAM_PRODUCTNAME_LEN);
    }
    else if (!strncmp(str, RUNTIME_BOARDID_VMG3925_B10C, strlen(RUNTIME_BOARDID_VMG3925_B10C)))
    {
    	/*Length of NVRAM_DATA->ProductName is NVRAM_PRODUCTNAME_LEN, 32, Yen-Chun,Lin, 20210324*/
        memset(pdname, 0, NVRAM_PRODUCTNAME_LEN);
        ZOS_STRNCPY(pdname, "VMG3925-B10C", NVRAM_PRODUCTNAME_LEN);
    }

    ZOS_FCLOSE(fp);
}
#endif
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetDSLVersion(char *DSLversion)
{
	char buf[64] = {0};
	char sysCmd[1024] = {0};
	FILE *fp = NULL;
	char *action = NULL;


	zos_snprintf(sysCmd, sizeof(sysCmd), "xdslctl --version 2>&1 | tee /var/DSLversion");
	ZOS_SYSTEM(sysCmd);

	fp = ZOS_FOPEN("/var/DSLversion", "r");
	if(fp != NULL){
		while(fgets(buf, sizeof(buf), fp) != NULL){
			buf[strlen(buf)-1] = '\0'; //remove '\n'
			if((action = strstr(buf,"version -")) != NULL){
				action= action + strlen("version - ");
				ZOS_STRNCPY(DSLversion, action, 64);
			}
		}
		ZOS_FCLOSE(fp);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetWiFiDriverVersion(char *wifidriverversion)
{
	zos_snprintf(wifidriverversion, 64, "-");
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBootVersion(char *bootversion)
{
	ZY_FW_INFO zyFwInfoData;
	memset(&zyFwInfoData, 0, sizeof(ZY_FW_INFO));

	if(zyFwInfoGet(&zyFwInfoData) < 0)
		return ZCFG_INTERNAL_ERROR;

	zos_snprintf(bootversion, 64, "V%d.%02d", zyFwInfoData.BuildInfo[0], zyFwInfoData.BuildInfo[1]);	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
	ZY_FW_INFO zyFwInfoData;
#ifdef EAAAA_CUSTOMIZATION
	char fwId[60] = {0};
	char produnctName[32] = {0};
	zyUtilIGetProductName(produnctName);
	zos_snprintf(fwId, sizeof(fwId), "%s_Elisa6_", produnctName);
#endif
	memset(&zyFwInfoData, 0, sizeof(ZY_FW_INFO));

	if(zyFwInfoGet(&zyFwInfoData) < 0)
		return ZCFG_INTERNAL_ERROR;
#ifdef EAAAA_CUSTOMIZATION
	ZOS_STRCAT(fwId, zyFwInfoData.FirmwareVersion, sizeof(fwId));
	ZOS_STRNCPY(fwversion, fwId, 64);
#else
	ZOS_STRNCPY(fwversion, zyFwInfoData.FirmwareVersion, NVRAM_FIRMWAREVERSION_LEN);
#endif

#ifdef ZYXEL_RMA_FW_SUPPORT
	/* PeterSu: force modify the first four chars to be "RMA_" */
	fwversion[0] = 'R';
	fwversion[1] = 'M';
	fwversion[2] = 'A';
	fwversion[3] = '_';
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetExternalFirmwareVersion(char *fwversion)
{
	ZY_FW_INFO zyFwInfoData;
	memset(&zyFwInfoData, 0, sizeof(ZY_FW_INFO));

	if(zyFwInfoGet(&zyFwInfoData) < 0)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(fwversion, zyFwInfoData.FirmwareVersionEx, NVRAM_FIRMWAREVERSION_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetSerialNumber(char *serianumber)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(serianumber, nvramData.SerialNumber, NVRAM_SERIALNUMBER_LEN);
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	/* need to append sub-sequence number */
	if(zyUtilIsAlphanumericString(nvramData.FeatureBits+NVRAM_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, NVRAM_FEATUREBITS_SUB_SERIAL_NUM_LEN) == 0)
		strncat(serianumber, nvramData.FeatureBits+NVRAM_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, NVRAM_FEATUREBITS_SUB_SERIAL_NUM_LEN);
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetSubSerialNumber(char *sub_serianumber)
{
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	if(zyUtilIsAlphanumericString(nvramData.FeatureBits+NVRAM_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, NVRAM_FEATUREBITS_SUB_SERIAL_NUM_LEN) != 0)
		return -1;

	strncpy(sub_serianumber, nvramData.FeatureBits+NVRAM_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, NVRAM_FEATUREBITS_SUB_SERIAL_NUM_LEN);

	printf("SUB-SerialNumber:%s\n",sub_serianumber);
#endif
	return ZCFG_SUCCESS;
}

int zyUtilGetGPIO(int gpioNum)
{
	int ret = 0;
	ret = boardIoctl(BOARD_IOCTL_GET_GPIO, 0, "", gpioNum, 0, "");
	return ret;
}

int zyUtilSetGPIOoff(int gpioNum)
{
	int ret = 0;
	ret = boardIoctl(BOARD_IOCTL_SET_GPIO, 0, "", gpioNum, 1, "");
	return ret;
}

int zyUtilSetGPIOon(int gpioNum)
{
	int ret = 0;
	ret = boardIoctl(BOARD_IOCTL_SET_GPIO, 0, "", gpioNum, 0, "");
	return ret;
}

zcfgRet_t zyUtilIGetDefaultGponSnPw(char *serianumber, char *password)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	if(serianumber != NULL)
		ZOS_STRNCPY(serianumber, nvramData.gponSerialNumber, NVRAM_GPON_SERIAL_NUMBER_LEN);
	if(password != NULL)
		ZOS_STRNCPY(password, nvramData.gponPassword, NVRAM_GPON_PASSWORD_LEN);

	return ZCFG_SUCCESS;
}

#if defined(ZYXEL_BOARD_DETECT_BY_GPIO) || defined(CONFIG_BCM96856)
#if defined(ZYXEL_BOARD_DETECT_BY_GPIO) && defined(ZYXEL_SUPPORT_RDPAWAN_CONFIGURE)

#define ZYXEL_DATAMODEL_AUTO     "AUTO"
#define ZYXEL_DATAMODEL_XGPON     "XGPON"
#define ZYXEL_DATAMODEL_XGS     "XGS"
#define ZYXEL_DATAMODEL_GBE_1G     "GBE_1G"
#define ZYXEL_DATAMODEL_GBE_2_5G     "GBE_2.5G"
#define ZYXEL_DATAMODEL_GBE_10G     "GBE_10G"

bool isValidateWanType(char *tmpStr){
	bool ret=false;

	if(!strcmp(tmpStr, ZYXEL_DATAMODEL_AUTO) ||!strcmp(tmpStr, ZYXEL_DATAMODEL_XGPON) || !strcmp(tmpStr, ZYXEL_DATAMODEL_XGS)
		|| !strcmp(tmpStr, ZYXEL_DATAMODEL_GBE_1G) ||!strcmp(tmpStr, ZYXEL_DATAMODEL_GBE_2_5G) || !strcmp(tmpStr, ZYXEL_DATAMODEL_GBE_10G)){
		ret=true;
	}

	return ret;
}

bool parseRdpaWANInfo(char *X_ZYXEL_RdpaWanType, char *wantype, int TYPELEN, char *wanrate, int RATELEN){
	bool ret=true;
	char buff[18];
	char delim[] = "_";
	char tmpWanType[TYPELEN];
	char tmpWanRate[RATELEN];
	tmpWanType[0] = '\0';
	tmpWanRate[0] = '\0';

	ZOS_STRNCPY(buff, X_ZYXEL_RdpaWanType, sizeof(buff));
	char *ptr = strtok(buff, delim);
	if(ptr != NULL)
	{
		zos_snprintf(tmpWanType, TYPELEN, "%s", ptr);
		ptr = strtok(NULL, delim);
		if(ptr != NULL){
			zos_snprintf(tmpWanRate, RATELEN, "%s", ptr);
		}
	}

	if(strcmp(tmpWanType, "AUTO")==0){
		zos_snprintf(wantype, TYPELEN, "%s", ZYXEL_RDPA_WAN_TYPE_VALUE_AUTO);
		zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_0000);
	}
	else if(strcmp(tmpWanType, "XGPON")==0){
		zos_snprintf(wantype, TYPELEN, "%s", ZYXEL_RDPA_WAN_TYPE_VALUE_XGPON);
		zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_0000);
	}
	else if(strcmp(tmpWanType, "XGS")==0){
		zos_snprintf(wantype, TYPELEN, "%s", ZYXEL_RDPA_WAN_TYPE_VALUE_XGS);
		zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_10G);
	}
	else if(strcmp(tmpWanType, "GBE")==0){
		zos_snprintf(wantype, TYPELEN, "%s", ZYXEL_RDPA_WAN_TYPE_VALUE_GBE);

		if(strcmp(tmpWanRate, "1G")==0)
			zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_1G);
		else if(strcmp(tmpWanRate, "2.5G")==0)
			zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_2_5G);
		else if(strcmp(tmpWanRate, "10G")==0)
			zos_snprintf(wanrate, RATELEN, "%s", ZYXEL_RDPA_WAN_RATE_10G);
		else{
			printf("\n wanrate parse error");
			ret = false;
		}
	}else{
		printf("\n wantype parse error");
		ret = false;
	}

	return ret;
}

void setRdpaWANSetting(char *wantype ,char *wanrate){

	/*Set WAN TYPE*/
	if(zyUtilISetScratchPadSetting(RDPA_WAN_TYPE_PSP_KEY, wantype, strlen(wantype)) != ZCFG_SUCCESS){
		printf("Set wan type failed.\r\n");
	}else{
		printf("Set wan type %s. \r\n", wantype);
	}
	/*Set WAN RATE*/
#if !defined(BCM_SDK_416L05)
	// SDK 416L05 didn`t have RDPA_WAN_RATE_PSP_KEY.
	if(zyUtilISetScratchPadSetting(RDPA_WAN_RATE_PSP_KEY, wanrate, strlen(wanrate)) != ZCFG_SUCCESS){
		printf("Set wan rate failed.\r\n");
	}
	else{
		printf("Set wan rate %s. \r\n", wanrate);
	}
#endif

	return;
}

int zyUtilISetScratchPadSetting(const char *key, const void *buf, int bufLen)
{
	char *currBuf = NULL;
	int count;
	int rc;

	if ((currBuf = (char *)ZOS_MALLOC(bufLen+1)) == NULL)
	{
		return ZCFG_INTERNAL_ERROR;
	}

	/*
	* Writing to the scratch pad is a non-preemptive time consuming
	* operation that should be avoided.
	* Check if the new data is the same as the old data.
	*/
	count = boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
		(char *) key, 0, bufLen, currBuf);

	if (count == bufLen)
	{
		if (memcmp(currBuf, buf, bufLen) == 0)
		{
			ZOS_FREE(currBuf);
			/* set is exactly the same as the orig data, no set needed */
			return ZCFG_SUCCESS;
		}
	}

	if( currBuf != NULL )
		ZOS_FREE(currBuf);

	rc = boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD, (char *) key, 0, bufLen, (char *) buf);
	printf("%s %d: rc:%d\n", __FUNCTION__, __LINE__, rc);

	if (rc < 0)
	{
		printf("Unable to write SCRATCH_PAD.\n");
		return ZCFG_INTERNAL_ERROR;
	}
	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t zyUtilIGetScratchPadSetting(char *psp_wantype, char *psp_wanrate)
{
    int rc;

    if (psp_wantype == NULL || psp_wanrate == NULL)
    {
        return ZCFG_INTERNAL_ERROR;
    }

    rc = boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD, (char*)RDPA_WAN_TYPE_PSP_KEY, 0, PSP_BUFLEN_16, psp_wantype);
    if (rc < 0)
    {
        printf("Unable to get WAN type");
        return ZCFG_INTERNAL_ERROR;
    }
#if !defined(BCM_SDK_416L05)
// SDK 416L05 didn`t have RDPA_WAN_RATE_PSP_KEY.
        rc = boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD, (char*)RDPA_WAN_RATE_PSP_KEY, 0, PSP_BUFLEN_16, psp_wanrate);
    if (rc < 0)
    {
        printf("Unable to get WAN rate");
        return ZCFG_INTERNAL_ERROR;
    }
#endif

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGUIGetRDPAWAN(char *psp_wantype, char *psp_wanrate)
{
	FILE *fp = NULL;
	char buf[64]={0};
	char *token=NULL, *token2=NULL;

	if (psp_wantype == NULL || psp_wanrate == NULL)
	{
	    return ZCFG_INTERNAL_ERROR;
	}

	fp = fopen(RDPA_WAN_INFO, "r");
	if ( fp != NULL ){
		if( fgets(buf, sizeof(buf), fp) != NULL){
			token = strtok(buf, ",");
			zos_snprintf(psp_wantype, PSP_BUFLEN_16, "%s", token);
			token2 = strtok(NULL, ",");
			zos_snprintf(psp_wanrate, PSP_BUFLEN_16, "%s", token2);
		}
		fclose(fp);
	}else{

		return ZCFG_INTERNAL_ERROR;
	}

	return ZCFG_SUCCESS;
}
#endif

#if defined(ABUU_CUSTOMIZATION) //IChiaHuang 20200323
//LED control for different products, added here
#if defined(ZYPRJ_DX3201_B0)
static ZY_LEDCTL_INFO_COMMON LedctlInfo_common[] =
{
    {"POWER_G", ZY_POWER_G},
    {"POWER_R", ZY_POWER_R},
    {"INET_G", ZY_INET_G},
    {"INET_R", ZY_INET_R},
    {"ADSL0", ZY_ADSL0},
    {"VDSL0", ZY_VDSL0},
    {"VOIP0_G", ZY_VOIP0_G},
    {"VOIP0_O", ZY_VOIP0_O},
    {"WLAN0", ZY_WLAN0},
    {"WPS0", ZY_WPS0},
    {"WPS1", ZY_WPS1},
    {"HW_ETH0", ZY_HW_ETH0},
    {"HW_ETH1", ZY_HW_ETH1},
    {"HW_ETH2", ZY_HW_ETH2},
    {"HW_ETH3", ZY_HW_ETH3},
    {"HW_ETH4", ZY_HW_ETH4},
    {"HW_AGGREGATE", ZY_HW_AGGREGATE},
    {"HW_WLAN1", ZY_HW_WLAN1},
    {"KHWLEDALL", ZY_KHWLEDALL},
    {"KHWLEDALLEXCEPTPOWER", ZY_KHWLEDALLEXCEPTPOWER},
    {"ZY_MAX_LED", ZY_MAX_LED}
};
#else
static ZY_LEDCTL_INFO_COMMON LedctlInfo_common[] =
{
    {"POWER_G", ZY_POWER_G},
    {"POWER_R", ZY_POWER_R},
    {"INET_G", ZY_INET_G},
    {"INET_R", ZY_INET_R},
    {"ADSL0", ZY_ADSL0},
    {"VDSL0", ZY_VDSL0},
    {"ADSL1", ZY_ADSL1},
    {"VDSL1", ZY_VDSL1},
    {"VOIP0_G", ZY_VOIP0_G},
    {"VOIP0_O", ZY_VOIP0_O},
    {"VOIP1_G", ZY_VOIP1_G},
    {"VOIP1_O", ZY_VOIP1_O},
    {"WLAN0", ZY_WLAN0},
    {"WPS0", ZY_WPS0},
    {"WLAN1", ZY_WLAN1},
    {"WPS1", ZY_WPS1},
    {"HW_ETH0", ZY_HW_ETH0},
    {"HW_ETH1", ZY_HW_ETH1},
    {"HW_ETH2", ZY_HW_ETH2},
    {"HW_ETH3", ZY_HW_ETH3},
    {"HW_ETH4", ZY_HW_ETH4},
    {"HW_AGGREGATE", ZY_HW_AGGREGATE},
    {"HW_WLAN0", ZY_HW_WLAN0},
    {"HW_WLAN1", ZY_HW_WLAN1},
    {"KHWLEDALL", ZY_KHWLEDALL},
    {"KHWLEDALLEXCEPTPOWER", ZY_KHWLEDALLEXCEPTPOWER},
    {"ZY_MAX_LED", ZY_MAX_LED}
};
#endif
#define zyLedctlInfo_common LedctlInfo_common

int zyLedIndexMaxNumGet(void)
{
    int num = 0;
    num = (sizeof(zyLedctlInfo_common)/sizeof(ZY_LEDCTL_INFO_COMMON)) - 1;
    return num;
}

char* zyLedNameMap(int index)
{
    PZY_LEDCTL_INFO_COMMON pLedInfo;
    pLedInfo = zyLedctlInfo_common;
    return pLedInfo[index].zy_LedName;
}

int boardIoctl_LED(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf, int result)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen; // led type
        IoctlParms.offset = offset; // led state
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        IoctlParms.result = result; // port
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");

    return boardFd;
}

int  get_BrcmLedState(ZY_LED_ACTION action) {
        int state;
        switch(action) {
                case ZY_LED_OFF:
                        state = kLedStateOff;
                        break;
                case ZY_LED_ON:
                        state = kLedStateOn;
                        break;
                case ZY_LED_SLOW_BLINK:
                        state = kLedStateSlowBlinkContinues;
                        break;
                case ZY_LED_FAST_BLINK:
                        state = kLedStateFastBlinkContinues;
                        break;
                case ZY_LED_UP:
                        state = kLedStateUp;
                        break;
                case ZY_LED_DOWN:
                        state = kLedStateDown;
                        break;
                default:
                        state = kLedStateOff;
                        break;
        }
        return state;
}

void led_control(ZY_LEDTYPE index, ZY_LED_ACTION action)
{
    PZY_LEDCTL_INFO pLedInfo;
    int action_tr = 0;

    action_tr = get_BrcmLedState(action);

    for(pLedInfo = zyLedctlInfo; (pLedInfo->ledType != ZY_MAX_LED); pLedInfo++)
    {
        if (index == pLedInfo->ledType)
        {
            //printf("led_control : pLedInfo->ledType = %d  index = %d action = %d action_tr = %d\n",pLedInfo->ledType, index, action, action_tr);
            if (pLedInfo->ledType == ZY_INET_R)
            {
#if defined(ZYPRJ_DX5511_B0)
                boardIoctl_LED(pLedInfo->control, 0, NULL, kLedInternetFail, action_tr, NULL, pLedInfo->port);
#else
                if(action == ZY_LED_OFF)
                {
                    boardIoctl_LED(pLedInfo->control, 0, NULL, kLedInternetData, action_tr, NULL, pLedInfo->port);
                }
                else
                {
                    boardIoctl_LED(pLedInfo->control, 0, NULL, kLedInternetData, kLedStateFail, NULL, pLedInfo->port);
                }
#endif
            }
            else if (pLedInfo->ledType == ZY_WLAN0 || pLedInfo->ledType == ZY_HW_WLAN0)
            {
                if(action == ZY_LED_DOWN)
                {
                    printf("wl0 LED DOWN\n");
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh disable wl0");
                }
                else
                {
                    printf("wl0 LED UP\n");
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh enable wl0");
                }
            }
            else if (pLedInfo->ledType == ZY_WLAN1 || pLedInfo->ledType == ZY_HW_WLAN1)
            {
                if(action == ZY_LED_DOWN)
                {
                    printf("wl1 LED DOWN\n");
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh disable wl1");
                }
                else
                {
                    printf("wl1 LED UP\n");
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh enable wl1");
                }
            }
            else if (pLedInfo->ledType == ZY_KHWLEDALL)
            {
                boardIoctl_LED(pLedInfo->control, 0, pLedInfo->zy_LedName, pLedInfo->index, action_tr, NULL, pLedInfo->port);
                if(action == ZY_LED_DOWN)
                {
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh disable");
                }
                else
                {
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh enable");
                }
            }
            else if (pLedInfo->ledType == ZY_KHWLEDALLEXCEPTPOWER)
            {
                boardIoctl_LED(pLedInfo->control, 0, pLedInfo->zy_LedName, pLedInfo->index, action_tr, NULL, pLedInfo->port);
                if(action == ZY_LED_DOWN)
                {
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh disable");
                }
                else
                {
                    ZOS_SYSTEM("/etc/init.d/zywifid_led.sh enable");
                }
            }
            else
                boardIoctl_LED(pLedInfo->control, 0, pLedInfo->zy_LedName, pLedInfo->index, action_tr, NULL, pLedInfo->port);

        }
    }
}

int zyLedSet(char *name, int action) //for userspace (LED control)
{
    PZY_LEDCTL_INFO_COMMON pLedInfo;
    for(pLedInfo = zyLedctlInfo_common; (pLedInfo->index != ZY_MAX_LED); pLedInfo++)
    {
        if( !strcmp(name, pLedInfo->zy_LedName))
        {
            led_control(pLedInfo->index, action);
            return 1;
        }
    }
    printf("led_set ERROR");
    return 0;
}

#else
void processLed(const int32_t ledType, const int32_t state)
{
	boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, ledType, state, NULL);
}

int32_t  get_BrcmLedIndex(ZYLED_INDEX index) {
	int32_t LEDindex;
	switch(index) {
		case ZYLED_POWER_GREEN:
			LEDindex = kLedPowerOn;
			break;
		case ZYLED_POWER_RED:
			LEDindex = kLedPostFail;
			break;
#ifdef ZYXEL_OPAL_EXTENDER
		case ZYLED_LINKQUALITY_GREEN:
			LEDindex = kLedQualityGreen;
			break;
		case ZYLED_LINKQUALITY_RED:
			LEDindex = kLedQualityRed;
			break;
		case ZYLED_LINKQUALITY_AMBER:
			LEDindex = kLedQualityAmber;
			break;
		case ZYLED_WIRELESS_GREEN:
			LEDindex = kLedWiFiWPS;
			break;
#endif
#if defined(LTE_SUPPORT)
		case ZYLED_CELLSIGNAL1_GREEN:
			LEDindex = kLedCellSignal1;
			break;
		case ZYLED_CELLSIGNAL2_GREEN:
			LEDindex = kLedCellSignal2;
			break;
		case ZYLED_CELLSIGNAL3_GREEN:
			LEDindex = kLedCellSignal3;
			break;
#endif
		default:
			LEDindex = kLedStateOff;
			break;
	}
	return LEDindex;
}

int32_t  get_BrcmLedState(ZYLED_ACTION action, ZYLED_RATE blink_rate) {
	int32_t state = 0;
	switch(action) {
		case ZYLED_ACTION_OFF:
			state = kLedStateOff;
			break;
		case ZYLED_ACTION_ON:
			state = kLedStateOn;
			break;
		case ZYLED_ACTION_BLINK:
			if(blink_rate == ZYLED_RATE_SLOW){
				state = kLedStateSlowBlinkContinues;
			}else if(blink_rate == ZYLED_RATE_FAST){
				state = kLedStateFastBlinkContinues;
			}
			break;
#ifdef ZYXEL_OPAL_EXTENDER
		case ZYLED_ACTION_UP:
			state = kLedStateUp;
			break;
		case ZYLED_ACTION_DOWN:
			state = kLedStateDown;
			break;
#endif
		default:
			state = kLedStateOff;
			break;
	}
	return state;
}

zcfgRet_t zyUtilSetSoftwareLedState(ZYLED_INDEX index ,ZYLED_ACTION action, ZYLED_RATE blink_rate)
{

	processLed(get_BrcmLedIndex(index), get_BrcmLedState(action, blink_rate));
	return ZCFG_SUCCESS;
}
#endif

#ifdef ZYXEL_TUTK_CLOUD_AGENT
zcfgRet_t zyUtilIGetTUTKUID(char *TUTK_UID)
{
	NVRAM_DATA nvramData;
	char tmp[21] = {0};
	
	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;
	
	if(strcmp(nvramData.TUTK_UID,""))	
	{	snprintf(tmp, sizeof(tmp), "%s", nvramData.TUTK_UID);
		strcpy(TUTK_UID, tmp);
	}else{
		return ZCFG_INTERNAL_ERROR;
	}

	return ZCFG_SUCCESS;
}
#endif

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen;	// using this to identify the gpio number.
        IoctlParms.offset = offset;
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");

    return boardFd;
}

/*
 *  This function is using for access memory with 4 bytes size
 *  type: 1 means write memory, and 0 means read memory
 *  addr: the access memory address. For example 0xFFFF0000
 *  value: read out or write into memory through it.
 */
zcfgRet_t memaccess(int type, unsigned long long addr, unsigned long *value)
{
	int siz = 4;
	BOARD_MEMACCESS_IOCTL_PARMS parms;
	int fd;

	if ((fd = open (BOARD_DEVICE_NAME, O_RDWR)) < 0)
	{
		printf ("Can't open /dev/brcmboard ");
	}

	parms.address = addr;
	parms.size = siz;
	parms.space = BOARD_MEMACCESS_IOCTL_SPACE_REG;
	parms.count = 1;
	parms.buf = (char *)value;
	if( type == 0)
		parms.op = BOARD_MEMACCESS_IOCTL_OP_READ;
	else if( type == 1)
		parms.op = BOARD_MEMACCESS_IOCTL_OP_WRITE;
	else
		return ZCFG_INTERNAL_ERROR;

	ioctl(fd, BOARD_IOCTL_MEM_ACCESS, &parms);
	close(fd);

	//printf("[%s]reg %08x value was %08x\n", __func__, addr, *value );
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


/*!
 *  ZIGBEE reset (get state)
 *
 *  @return    -1 - ioctl failed
 *              0 - Low
 *              1 - High
 *              2 - ZIGBEE not supported
 *  @note get zigbee reset state
 */
int zyzigbeereset_get(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1) 
    {
        ioctlParms.result = -2;//Because -1 to 2 has been used, the initial value of -2

        rc = ioctl(boardFd, BOARD_IOCTL_GET_ZIGBEE_RESET, &ioctlParms);
        close(boardFd);

        if (rc < 0 || (ioctlParms.result != 0 && ioctlParms.result != 1) ) 
        {
            if (ioctlParms.result == -1)
            {
                zyutil_dbg_printf("ZIGBEE not supported\n");
                return 2;
            }
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }
        zyutil_dbg_printf("get zigbee reset value: %d\n",ioctlParms.result);	
        return ioctlParms.result;
    }
    else 
    {
        return -1;
    }
}

/*!
 *  ZIGBEE reset
 *
 *  @return    -1 - ioctl failed
 *              0 - Fail to reset zigbee module
 *              1 - Reset zigbee module succeed
 *  @note ZIGBEE reset
 */
int zyzigbeereset_set(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1) 
    {
        ioctlParms.strLen =  1;//start up zigbee reset
        ioctlParms.result = -1;

        rc = ioctl(boardFd, BOARD_IOCTL_SET_ZIGBEE_RESET, &ioctlParms);
        close(boardFd);

        if(rc < 0) 
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }

        if (ioctlParms.result == 1)
        {
            zyutil_dbg_printf("Reset zigbee module succeed.\n");
            return ioctlParms.result;
        }
        else
        {
            zyutil_dbg_printf("Fail to reset zigbee module.\n");
            return ioctlParms.result;
        }
    }
    else 
    {
        return -1;
    }
}

/*!
 *  get UPS state
 *
 *  @return    -1            - ioctl failed
 *             Globe value   - UPS state
 *
 *  @note   Globe value: ups_active_status
 *         (bit3,bit2,bit1,bit0) => (miss battery, on battery, low battery, fail battery)
 *          bit0 : 1(active) - fail Self Test, 0(inactive) - battery is charged
 *          bit1 : 1(active) - battery < 45% capacity, 0(inactive) - battery full capacity
 *          bit2 : 1(active) - operating from battery, 0(inactive) - operating from utility line
 *          bit3 : 1(active) - battery is missing, 0(inactive) - battery is present
 */
int zyupsstate_get(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1) 
    {
        ioctlParms.result = -1;

        rc = ioctl(boardFd, BOARD_IOCTL_GET_UPS_STATE, &ioctlParms);
        close(boardFd);

        if (rc < 0) 
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }
        return ioctlParms.result;
    }
    else 
    {
        return -1;
    }
}

/*!
 *  sfp operation
 *  @param[in]  subvalue    - SFP operation options (0-6)
 *              input       - active/inactive
 *
 *  @return       -1        - ioctl failed
 *                 0        - inactive
 *                 1        - active
 *
 *  @note get/set sfp  
 */
int zysfpoperation(int subvalue ,int input)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;
    
    if ((subvalue > (ZYGOIOSATAT_MAX_NUM_COMMANDS - 1)) || subvalue < GET_SFP_PRESENT || (input != 0 && input != 1) )
    {
        return -1;
    }

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
    if(boardFd != -1) 
    {
        ioctlParms.result = -1;
        ioctlParms.strLen = subvalue;
        ioctlParms.offset = input;

        rc = ioctl(boardFd, BOARD_IOCTL_SET_GET_SFP, &ioctlParms);
        close(boardFd);

        if (rc < 0) 
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }

        return ioctlParms.result;
    }
    else 
    {
        return -1;
    }
}
#endif
