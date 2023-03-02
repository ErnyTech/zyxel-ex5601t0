#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <json/json.h>
#include <stdio.h>
#include <time.h>

#include <sys/stat.h>
#ifdef ABOG_CUSTOMIZATION
#include <stdlib.h>
#include <dirent.h>
#endif

#include <ctype.h>
#include <unistd.h>
#define _GNU_SOURCE
#include "zcfg_common.h"
#include "zcfg_debug.h"
#include "zhttpd_oid.h"

#include "zcfg_rdm_oid.h"
#include "zcfg_msg.h"
#include "zlog_api.h"
#include "zos_api.h"

#define MID_SIZE_MAX  32

#include "zyutil.h"

#include "libzyutil_wrapper.h"

#define IPTABLESRESTORE		"iptables-restore"
#define IP6TABLESRESTORE	"ip6tables-restore"

#if 0	//move to libzyutil_brcm_wrapper.c
#ifdef BROADCOM_PLATFORM
#include "board.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"

#define BOARD_DEVICE_NAME  "/dev/brcmboard"
#endif
#endif

/*
 *  Function Name: zyUtilIfaceHwAddrGet
 *  Description: Used to get the MAC Addfree of interface
 *  Parameters:
 *
 */
void zyUtilIfaceHwAddrGet(char *ifname, char *hwaddr)
{
	char buf[MID_SIZE_MAX+60];
	char sysCmd[1024] = {0};
	int i = 0, j = 0;
	char *p = NULL;
	FILE *fp = NULL;

	memset(hwaddr, 0, strlen(hwaddr)); //clear the string before "/0"

	zos_snprintf(sysCmd, sizeof(sysCmd),"ifconfig %s > /var/hwaddr ", ifname);
	ZOS_SYSTEM(sysCmd);

	fp = ZOS_FOPEN("/var/hwaddr", "r");
	if(fp != NULL) {
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			for(i = 2; i < (int)(sizeof(buf) - 5); i++) {
				if (buf[i]==':' && buf[i+3]==':') {
					p = buf+(i-2);
					for(j = 0; j < MID_SIZE_MAX-1; j++, p++) {
						if (isalnum(*p) || *p ==':') {
							hwaddr[j] = *p;
						}
						else {
							break;
						}
					}
					hwaddr[j] = '\0';
					break;
				}
			}
		}
		ZOS_FCLOSE(fp);
	}
}

bool zyUtilIsAppRunning(char *appName)
{
	char sysCmd[64] = {0};
	int size = 0;
	FILE *fp = NULL;
	bool isRunning = false;

	if(strlen(appName) == 0)
		return false;

	zos_snprintf(sysCmd, sizeof(sysCmd),"ps | grep %s | grep -v grep > /tmp/app", appName);
	ZOS_SYSTEM(sysCmd);

	fp = ZOS_FOPEN("/tmp/app", "r");
	if(fp != NULL) {
		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);

		if(size > 0)
			isRunning = true;

		ZOS_FCLOSE(fp);
	}

	return isRunning;
}

bool zyUtilCheckIfaceUp(char *devname)
{
	int skfd;
	int ret = false;
	struct ifreq intf;

	if (devname == NULL || (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return ret;
	}

	ZOS_STRNCPY(intf.ifr_name, devname, IFNAMSIZ);

	/*   if interface is br0:0 and
	 *   there is no binding IP address then return down
	 */

	if (strchr(devname, ':') != NULL) {
		if (ioctl(skfd, SIOCGIFADDR, &intf) < 0) {
			close(skfd);
			return ret;
		}
	}

	// if interface flag is down then return down
	if (ioctl(skfd, SIOCGIFFLAGS, &intf) != -1) {
		if ((intf.ifr_flags & IFF_UP) != 0)
			ret = true;
	}

	close(skfd);

	return ret;
}

void zyUtilAddIntfToBridge(char *ifName, char *bridgeName)
{
	char sysCmd[64] = {0};

	if(ifName == NULL || ifName[0] == 0) return;
	if(bridgeName == NULL || bridgeName[0] == 0) return;
	//zos_snprintf(sysCmd, sizeof(sysCmd), "brctl delif %s %s 2>/dev/null", bridgeName, ifName);
	//ZOS_SYSTEM(sysCmd);
#ifdef ZYXEL_IFCONFIG_BY_IOCTL
	if (ZOS_IFCONFIG_SET_IP(ifName, "0.0.0.0", NULL, NULL, IFCONFIG_NONE) == ZOS_SUCCESS){
	    ZOS_IFCONFIG_UP(ifName);
#else
	zos_snprintf(sysCmd, sizeof(sysCmd), "ifconfig %s 0.0.0.0 2>/dev/null", ifName);
	if(ZOS_SYSTEM(sysCmd) == ZOS_SUCCESS){
		zos_snprintf(sysCmd, sizeof(sysCmd),"ifconfig %s up", ifName);
		ZOS_SYSTEM(sysCmd);
#endif
		zos_snprintf(sysCmd, sizeof(sysCmd), "brctl addif %s %s", bridgeName, ifName);
		ZOS_SYSTEM(sysCmd);
		zos_snprintf(sysCmd, sizeof(sysCmd), "sendarp -s %s -d %s", bridgeName, bridgeName);
		ZOS_SYSTEM(sysCmd);
		zos_snprintf(sysCmd, sizeof(sysCmd), "sendarp -s %s -d %s", bridgeName, ifName);
		ZOS_SYSTEM(sysCmd);
#if defined (ZYXEL_VMG8924_B10D_TAAAD_ISOLATE_DMZ)
		ZOS_SYSTEM("kill $(ps | grep \"/etc/isolateDMZ_Wind_Italy_VMG8924_b10d.sh\" | awk 'FNR==1' | awk '{print $1}')");
		ZOS_SYSTEM("/etc/isolateDMZ_Wind_Italy_VMG8924_b10d.sh remap &"); // check dmz port and isolate dmz port
#elif defined (ZYXEL_VMG8825_T50K_TAAAD_ISOLATE_DMZ)
		ZOS_SYSTEM("kill $(ps | grep \"/etc/isolateDMZ_VMG8825_T50K.sh\" | awk 'FNR==1' | awk '{print $1}')");
		ZOS_SYSTEM("/etc/isolateDMZ_VMG8825_T50K.sh &");
#endif
	}
}

void zyUtilDelIntfFromBridge(char *ifName, char *bridgeName)
{
	char sysCmd[64] = {0};

	zos_snprintf(sysCmd, sizeof(sysCmd), "brctl delif %s %s", bridgeName, ifName);
	ZOS_SYSTEM(sysCmd);
	zos_snprintf(sysCmd, sizeof(sysCmd), "sendarp -s %s -d %s", bridgeName, bridgeName);
	ZOS_SYSTEM(sysCmd);
	zos_snprintf(sysCmd, sizeof(sysCmd), "sendarp -s %s -d %s", bridgeName, ifName);
	ZOS_SYSTEM(sysCmd);
#if defined (ZYXEL_VMG8924_B10D_TAAAD_ISOLATE_DMZ)
	ZOS_SYSTEM("kill $(ps | grep \"/etc/isolateDMZ_Wind_Italy_VMG8924_b10d.sh\" | awk 'FNR==1' | awk '{print $1}')");
	ZOS_SYSTEM("/etc/isolateDMZ_Wind_Italy_VMG8924_b10d.sh remap &"); // check dmz port and isolate dmz port
#elif defined (ZYXEL_VMG8825_T50K_TAAAD_ISOLATE_DMZ)
	ZOS_SYSTEM("kill $(ps | grep \"/etc/isolateDMZ_VMG8825_T50K.sh\" | awk 'FNR==1' | awk '{print $1}')");
	ZOS_SYSTEM("/etc/isolateDMZ_VMG8825_T50K.sh &");
#endif
}

#if 0	//move to libzyutil_brcm_wrapper.c
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

int zyUtilGetGPIO(int gpioNum)
{
	int ret = 0;
	ret = boardIoctl(BOARD_IOCTL_GET_GPIO, 0, "", gpioNum, 0, "");
	return ret;
}
#endif

// tzlink: timezone link
void zyUtilSyncTimezone(const char *tzlink)
{
	struct stat st = {0};

	if(tzlink && !stat(tzlink, &st)) {
		char cmd[300] = {0}, buf[300] = {0};
		FILE *p = NULL;

		snprintf(cmd, sizeof(cmd), "find %s | xargs grep GMT", tzlink);
		p = popen((const char *)cmd, "r");
		if(p) {
			fgets(buf, sizeof(buf), p);
			pclose(p);
		}

		if(strlen(buf)) {
			//printf("%s\n", buf);
			setenv("TZ", buf, 1);
			tzset();
		}

		unlink(tzlink);
	}
}

zcfgRet_t zyUtilGetDomainNameFromUrl(char *url, char *domainName, int domainNameLength)
{
	char *domainNameStartPtr = NULL;
	char *domainNameEndPtr = NULL;
	char tmpdomainName[256];

	if(!strcmp(url, "\0")){
		zcfgLog(ZCFG_LOG_INFO, "url string is empty \n");
		return ZCFG_SUCCESS;
	}

	domainNameStartPtr = strstr(url, "://");
	if (NULL == domainNameStartPtr) {
		domainNameStartPtr = url;
	}
	else {
		domainNameStartPtr += strlen("://");
	}


	if(domainNameStartPtr[0] == '[' && strstr(domainNameStartPtr, "]"))
		return ZCFG_SUCCESS;

	domainNameEndPtr = strstr(domainNameStartPtr, ":");
	if (NULL == domainNameEndPtr) {
		domainNameEndPtr = strstr(domainNameStartPtr, "/");
		if (NULL == domainNameEndPtr) {
			domainNameEndPtr = url + strlen(url);
		}
	}

	ZOS_STRNCPY(tmpdomainName, domainNameStartPtr, (domainNameEndPtr-domainNameStartPtr+1));
#if defined(ZYXEL_DNS_TRAFFIC_ROUTE_FUZZY_COMPARE)
	domainNameStartPtr = tmpdomainName;
#else
	domainNameStartPtr = strstr(tmpdomainName, "*.");
	if (NULL == domainNameStartPtr) {
		domainNameStartPtr = tmpdomainName;
	}
	else {
		domainNameStartPtr += strlen("*.");
	}
#endif

	domainNameEndPtr = tmpdomainName + strlen(tmpdomainName);

	ZOS_STRNCPY(domainName, domainNameStartPtr, (domainNameEndPtr-domainNameStartPtr+1));

	return ZCFG_SUCCESS;
}

bool zyUtilRetrieveSystemDefaultRoutIface(char *interfaceName)
{
#ifdef ZYXEL_ECHO_REPLACEMENT
    ZOS_ECHO_STR_TO_FILE("/var/tr143NullInterface", "\n", 1, "w");
#else
	ZOS_SYSTEM("echo > /var/tr143NullInterface");//If interface is null
#endif
	FILE *fp = NULL;
	char defaultGateway[256] = {0};

	fp = popen("route", "r");
	if(fp != NULL)
	{
	while ((fgets(defaultGateway, 255, fp)) != NULL)
	{
		if (strstr(defaultGateway, "default") != NULL)
		{
			if(strstr(defaultGateway, "eth") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "eth"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "eth")) + 1));
			else if(strstr(defaultGateway, "ptm") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "ptm"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "ptm")) + 1));
			else if(strstr(defaultGateway, "atm") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "atm"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "atm")) + 1));
			else if(strstr(defaultGateway, "ppp") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "ppp"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "ppp")) + 1));
			else if(strstr(defaultGateway, "br") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "br"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "br")) + 1));
//#ifdef ZYXEL_ECONET_WAN_INTERFACE_NAMING_ENABLE
			else if(strstr(defaultGateway, "nas") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "nas"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "nas")) + 1));
			else if(strstr(defaultGateway, "ppp") != NULL)
				ZOS_STRNCPY(interfaceName, strstr(defaultGateway, "ppp"), ((strstr(defaultGateway, "\n")) - (strstr(defaultGateway, "ppp")) + 1));
//#endif
			else
			{
				pclose(fp);
				fp = NULL;
				return false;
			}

			pclose(fp);
			fp = NULL;
			return true;
		}
		else{
			ZOS_STRNCPY(interfaceName, "br0", 4);
		}
	}
	pclose(fp);
	fp = NULL;
	}

	return false;
}

#if 0
zcfgRet_t zyUtilAppStopByPidfile(char *pidFile)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char sysCmd[32] = {0};
	FILE *fptr = NULL;
	int pid = 0;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((fptr = ZOS_FOPEN(pidFile, "r")) != NULL) {
		if ( fscanf(fptr, "%d", &pid) != 1)
			printf("%s : fail to get the content from file!\n", __FUNCTION__);
		printf("%s : Stop process with PID %d\n", __FUNCTION__, pid);
		zos_snprintf(sysCmd, sizeof(sysCmd), "kill -9 %d", pid);
		zcfgLog(ZCFG_LOG_DEBUG, "Command %s\n", sysCmd);
		if(ZOS_SYSTEM(sysCmd) != ZOS_SUCCESS)
			ret = ZCFG_INTERNAL_ERROR;
		ZOS_FCLOSE(fptr);
	}

	return ret;
}
#endif

zcfgRet_t zyUtilStopScriptByName(char *scriptName)
{
	zcfgRet_t   ret = ZCFG_SUCCESS;
	char cmd[80] = {0};
	char ptmp[10] = {0};
	FILE *fp = NULL;
	int ppid = -1, i=0;
	cmd[0] = '\0';
	
	sprintf(cmd, "pgrep -f %s", scriptName);
	if((fp = popen(cmd, "r"))){
		if(fgets(ptmp, sizeof(ptmp), fp) != NULL){
			ppid = atoi(ptmp);
		}
		pclose(fp);
	}
	
	if(ppid > 0)
	{
		kill(ppid, SIGTERM);
		while(!kill(ppid, 0) && i < 5){sleep(1);i++;}
		kill(ppid, SIGKILL);
	}
	else
	{
		ZLOG_WARNING("fail to get pid of script = %s", scriptName);
	}
	
	
	return ret;
}

zcfgRet_t zyUtilAppStopByPidfile(char *pidFile)
{
	zcfgRet_t   ret = ZCFG_SUCCESS;
	FILE        *fptr = NULL;
	int         pid = 0;

	fptr = ZOS_FOPEN(pidFile, "r");
	if (fptr == NULL)
	{
	    ZLOG_WARNING("fail to open pid file = %s", pidFile);
	    return ret;
	}

	if ( fscanf(fptr, "%d", &pid) != 1 )
		ZLOG_WARNING("fail to get the content from file!\n");
	ZOS_FCLOSE(fptr);

#if 1 //SAAAD_RELEASE_DHCP_PPP_BEFORE_IFDOWN
#ifdef USA_PRODUCT 
	//Jack: solution for 4f6e5e9c5817c95e80cbd7c928f9d50ec6d3d017
	if (strstr(pidFile,"pppd") != NULL || strstr(pidFile,"/var/dhcpc") != NULL)
#else
	if (strstr(pidFile,"pppd") != NULL || strstr(pidFile,"/var/dhcpc") != NULL || strstr(pidFile,"dhcp6c") != NULL)
#endif
	{
		kill(pid, SIGTERM);
		sleep(1);
	}
#endif

	if (kill(pid, SIGKILL) == 0)
	{
        ZLOG_DEBUG("kill process with PID %d", pid);
	}
	else
	{
        ZLOG_WARNING("fail to kill process with PID %d", pid);
	}

	return ret;
}

zcfgRet_t zyUtilMacStrToNum(const char *macStr, uint8_t *macNum)
{
	char    *pToken = NULL;
	char    *pLast = NULL;
	char    *buf;
	int     i;

	if (macNum == NULL)
	{
	    ZLOG_ERROR("macNum == NULL");
		return ZCFG_INVALID_ARGUMENTS;
	}

	if (macStr == NULL)
	{
	    ZLOG_ERROR("macStr == NULL");
		return ZCFG_INVALID_ARGUMENTS;
	}

	buf = (char *)ZOS_MALLOC(MAC_STR_LEN + 1);
	if (buf == NULL)
	{
		ZLOG_ERROR("memory insufficient");
		return ZCFG_INTERNAL_ERROR;
	}

	/* need to copy since strtok_r updates string */
	ZOS_STRNCPY(buf, macStr, (MAC_STR_LEN + 1));

	/* Mac address has the following format
	 * xx:xx:xx:xx:xx:xx where x is hex number
	 */
	pToken = strtok_r(buf, ":", &pLast);
	macNum[0] = (uint8_t)strtol(pToken, (char **)NULL, 16);
	for (i = 1; i < MAC_ADDR_LEN; ++i)
	{
		pToken = strtok_r(NULL, ":", &pLast);
		macNum[i] = (uint8_t)strtol(pToken, (char **)NULL, 16);
	}

	ZOS_FREE(buf);

	return ZCFG_SUCCESS;
}

// array: has n units, [0,..,n-1]
// compRout: compare rout
// value: search unit by value
// l, r: array section, l=0, r=n-1 on starting
// return: searched array unit
void *zyUtilSortedArraySearch(void *array, compRout cr, void *value, int l, int r)
{
	int m = 0, ll, rr;
	void *unit = NULL;

	if(!array || !value || !cr)
		return NULL;

	if(l > r){
		return NULL;

	}else if(l == r){
		cr(array, l, value, &unit);
		return unit;
	}

	m = (l + r)/2;
	int rst = cr(array, m, value, &unit);
	if(rst == 0){
		return unit;
	}else if(rst > 0){
		ll = m+1; rr = r;
		return zyUtilSortedArraySearch(array, cr, value, ll, rr);
	}else{
		ll = l; rr = m-1;
		return zyUtilSortedArraySearch(array, cr, value, ll, rr);
	}
}

zcfgRet_t zyUtilMacNumToStr(const uint8_t *macNum, char *macStr)
{
	if(macNum == NULL || macStr == NULL) {
		return ZCFG_INVALID_ARGUMENTS;
	}

	zos_snprintf(macStr, MAC_ADDRESS_WITH_COLON, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
			(uint8_t) macNum[0], (uint8_t) macNum[1], (uint8_t) macNum[2],
			(uint8_t) macNum[3], (uint8_t) macNum[4], (uint8_t) macNum[5]);

   return ZCFG_SUCCESS;
}

/*!
*  replace string
*  @param[IN/OUT]	source
*  @param[IN]	find_str //keyword
*  @param[IN]	rep_str //replace
*/
void zyUtilStrReplace (char *source, char* find_str, char*rep_str){
	char result[65] = {0};
	char* str_temp = NULL;
	char* str_location = NULL;
	int rep_str_len = 0;
	int find_str_len = 0;
	int gap = 0;

	ZOS_STRNCPY(result, source, sizeof(result));
	find_str_len = strlen(find_str);
	rep_str_len = strlen(rep_str);
	str_temp = source;
	str_location = strstr(str_temp, find_str);

	while(str_location != NULL){
		gap += (str_location - str_temp);
		result[gap] = '\0';
		ZOS_STRCAT(result, rep_str, sizeof(result));
		gap += rep_str_len;
		str_temp = str_location + find_str_len;
		ZOS_STRCAT(result, str_temp,sizeof(result));
		str_location = strstr(str_temp, find_str);
	}

	result[strlen(result)] = '\0';
	memset(source, 0 , strlen(source));
	ZOS_STRNCPY(source, result, (strlen(result)+1));
}


#if 1
void zyUtilBackslashInsert(char *str)
{
	char tmpStr[513] = {};
	int i = 0, j = 0, sym = 0;

	for (i=0; i<strlen(str); i++) {
		if ((48 <= *(str+i) && *(str+i) <= 57) || (65 <= *(str+i) && *(str+i) <= 90) || (97 <= *(str+i) && *(str+i) <= 122))
			sym = 0;
		else
			sym = 1;
		if (sym == 1) {
			*(tmpStr+j) = 0x5c;
			j++;
		}
		*(tmpStr+j) = *(str+i);
		j++;
	}
	ZOS_STRNCPY(str, tmpStr, (strlen(tmpStr)+1));

	return;
}

void zyUtilBackslashInForEcho(char *str)
{
	char tmpStr[513] = {};
	int i = 0, j = 0;

	for (i=0; i<strlen(str); i++) {
		if ((*(str+i) == 34 ) || (*(str+i) == 36) || (*(str+i) == 92) || (*(str+i) == 96)){
			*(tmpStr+j) = 0x5c;
			j++;
		}
		*(tmpStr+j) = *(str+i);
		j++;
	}
	ZOS_STRNCPY(str, tmpStr, (strlen(tmpStr)+1));

	return;
}

#endif

#if 0	//move to libzyutil_brcm_wrapper.c
#ifdef BROADCOM_PLATFORM
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
#if 0 //move to libzyutil_brcm_wrapper.c
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
		ioctlParms.string = nvramData;
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
		ioctlParms.string = nvramData;
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
#endif
zcfgRet_t zyUtilIGetSerailNum(char *sn)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	strcpy(sn, nvramData.SerialNumber);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBaseMAC(char *mac)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

        sprintf(mac ,"%02X%02X%02X%02X%02X%02X",
			nvramData.ucaBaseMacAddr[0],
			nvramData.ucaBaseMacAddr[1],
        	nvramData.ucaBaseMacAddr[2],
        	nvramData.ucaBaseMacAddr[3],
        	nvramData.ucaBaseMacAddr[4],
        	nvramData.ucaBaseMacAddr[5]
	);
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

    if(num) *num =  nvramData.ulNumMacAddrs;
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
	if(!strcmp(ZYXEL_PRODUCT_NAME, "VMG3926-B10A") || !strcmp(ZYXEL_PRODUCT_NAME, "VMG8924-B10B")){
		int ret = 0, gpioNum = 11;
		ret = zyUtilGetGPIO(gpioNum);
		if(ret){//VMG3926
			strcpy(pdname, "VMG3926-B10A");
		}
		else{//VMG8924
			strcpy(pdname, "VMG8924-B10B");
		}
	}
	else{
		NVRAM_DATA nvramData;
		memset(&nvramData, 0, sizeof(NVRAM_DATA));

		if(nvramDataGet(&nvramData) < 0)
			return ZCFG_INTERNAL_ERROR;

		strcpy(pdname, nvramData.ProductName);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
	NVRAM_DATA nvramData;
#ifdef EAAAA_CUSTOMIZATION
	char fwId[60] = "VMG3926-B10A_Elisa3_";
#endif
	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;
#ifdef EAAAA_CUSTOMIZATION
	strcat(fwId, nvramData.FirmwareVersion);
	strcpy(fwversion, fwId);
#else
	strcpy(fwversion, nvramData.FirmwareVersion);
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetSerialNumber(char *serianumber)
{
	NVRAM_DATA nvramData;

	memset(&nvramData, 0, sizeof(NVRAM_DATA));

	if(nvramDataGet(&nvramData) < 0)
		return ZCFG_INTERNAL_ERROR;

	strcpy(serianumber, nvramData.SerialNumber);
	return ZCFG_SUCCESS;
}
#endif
#endif


// str 'csvStr' 'string'
// 'x,y,z' 'x, y, z'
char *zyUtilStrCsvString(const char *csvStr, const char *string)
{
	char *token, *tmp = NULL;
	char *p = NULL;
	int cmp;
	char *_csvStr_local = strdup(csvStr); //prevent strtok_r modify the original string.
	token = strtok_r(_csvStr_local, ",", &tmp);
	int running = token ? 1 : 0;

	while(running) {
		//p = jumpDummyHead(token);
		p = token;
		cmp = strcmp(p, string);

		token = strtok_r(NULL, ",", &tmp);
		running = (!token || !cmp) ? 0 : 1;
	}

	if(_csvStr_local)ZOS_FREE(_csvStr_local);
	return p;
}

char *zyUtilStrCsvStripString(const char *csvStr, const char *string)
{
	char *token, *tmp = NULL;
	char *p = NULL;
	int csvStrLength = 0;
	char *stripCsvStr = NULL, *outputCsvStr;
	bool strip = false;
	char *_csvStr_local = NULL;

	if(!csvStr || !string || (csvStrLength = strlen(csvStr)) == 0 || strlen(string) == 0)
		return NULL;

	_csvStr_local = strdup(csvStr); //prevent strtok_r modify the original string.
	stripCsvStr = malloc(csvStrLength+1);
	memset(stripCsvStr, 0, csvStrLength+1);
	outputCsvStr = NULL;

	token = strtok_r(_csvStr_local, ",", &tmp);
	int running = token ? 1 : 0;

	while(running) {
		p = token;
		outputCsvStr = (strlen(p) && strcmp(p, string) != 0) ? ZOS_STRCAT(stripCsvStr, p, (csvStrLength+1)) : NULL;
		outputCsvStr = (outputCsvStr) ? ZOS_STRCAT(stripCsvStr, ",", (csvStrLength+1)) : NULL;
		strip = (!outputCsvStr && !strip) ? true : false;

		token = strtok_r(NULL, ",", &tmp);
		running = (!token) ? 0 : 1;
	}

	if((csvStrLength = strlen(stripCsvStr))){
		*(stripCsvStr+(csvStrLength-1)) = '\0';
	}
	if(!strip){
		free(stripCsvStr);
	}

	if(_csvStr_local)ZOS_FREE(_csvStr_local);
	return strip ? stripCsvStr : NULL;
}

char *zyUtilCsvStringStripTerm(const char *csv, const char *term)
{
	char *new_Csv = NULL;
	char *str_1 = NULL, *str_2 = NULL;
	int csvLen = strlen(csv);


	if (!csv || !term || (csvLen <= 0))
		return NULL;

	str_1 = strstr(csv, term);
	if (!str_1)
	{
		return NULL;
	}

	new_Csv = ZOS_MALLOC(csvLen);
	if (new_Csv == NULL)
	{
	    ZLOG_ERROR("memory insufficient");
	    return NULL;
	}

	if (str_1 == csv)
	{
		str_2 = str_1 + strlen(term);
		if (str_2[0] != '\0')
		{
			str_2++;
			ZOS_STRNCPY(new_Csv, str_2, csvLen);
		}
	}
	else
	{
		str_2 = str_1 + strlen(term);
		ZOS_STRNCPY(new_Csv, csv, (str_1-csv));
		if (str_2[0] != '\0')
		{
			ZOS_STRCAT(new_Csv, str_2, csvLen);
		}
	}

	return new_Csv;
}

void zyUtilStrCsvAppend(char **csv, const char *string)
{
	int n = 0, csvLen;

	if(!csv || !string || !strlen(string)){
		return;
	}

	if(*csv && (csvLen = strlen(*csv)))
	{
		//csvLen = strlen(*csv);
		n = strlen(string) + 2;
		char *newcsv = ZOS_MALLOC(csvLen+n);
		ZOS_STRNCPY(newcsv, *csv, csvLen+n);
		free(*csv);
		char *ptr = newcsv + csvLen;
		zos_snprintf(ptr, n, ",%s", string);
		*csv = newcsv;
	}
	else{
		if(*csv)
			free(*csv);
		n = strlen(string) + 1;
		*csv = ZOS_MALLOC(n);
		ZOS_STRNCPY(*csv, string, n);
	}
}

int zyUtilValidIpAddr(const char *ipAddress)
{
	struct in6_addr ipv6addr;
	struct in_addr ipaddr;

	if(ipAddress && (strlen(ipAddress) > 0) && strstr(ipAddress, ":")){
		if(inet_pton(AF_INET6, (const char *)ipAddress, (void *)&ipv6addr) != 1)
			return -1;
		else return AF_INET6;
	}else if(ipAddress && (strlen(ipAddress) > 0)) {
		if(inet_aton((const char *)ipAddress, &ipaddr) == 0)
			return AF_UNSPEC; //hostname ?
		else return AF_INET;
	}else
		return -1;
}

int zyUtilIp4AddrDomainCmp(const char *ipaddr1, const char *addrmask, const char *ipaddr2)
{
	in_addr_t addr1 = inet_addr(ipaddr1);
	in_addr_t mask = inet_addr(addrmask);

	in_addr_t addr2 = inet_addr(ipaddr2);

	if( (addr1 == INADDR_NONE) || (mask == INADDR_NONE) || (addr2 == INADDR_NONE) )
		return -1;

	return ((addr1 & mask) == (addr2 & mask)) ? 0 : 1;
}

zcfgRet_t zyUtilIGetBridgeIPAddr(char *ipAddress)
{
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;
	FILE *fp = NULL;
	char tmpStr[64] = {'\0'};
	char brIP[16] = {'\0'};
	char tmpPath[] = "/tmp/brIP";

	zos_snprintf(tmpStr, sizeof(tmpStr), "ifconfig br0 > %s", tmpPath);
	ZOS_SYSTEM(tmpStr);
	memset(tmpStr, 0, sizeof(tmpStr));

	if ((fp = ZOS_FOPEN(tmpPath, "r")) != NULL)
	{
		while (fgets(tmpStr, sizeof(tmpStr), fp) != NULL)
		{
			if (!strstr(tmpStr, "inet addr"))
				continue;
			sscanf(tmpStr, "          inet addr:%s", brIP);
			ZOS_STRNCPY(ipAddress, brIP, 16);
printf("%s\n", brIP);
			ret = ZCFG_SUCCESS;
		}
		ZOS_FCLOSE(fp);
		//ulink(tmpPath);
	}
	return ret;
}

zcfgRet_t zyUtilIGetUPnPuuid(char *uuid)
{
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;
	FILE *fp_uuid = NULL;
	char uuidStr[96];
	char MACAddr[16] = {'\0'};
	char tmpStr[64] = {'\0'};

	uuidStr[0]='\0';
	if (zyUtilIGetBaseMAC(MACAddr) == ZCFG_SUCCESS)
	{
		if ((fp_uuid = ZOS_FOPEN(SYSTEM_UUID_PATH, "r")) != NULL)
		{
			if (fgets(tmpStr, sizeof(tmpStr), fp_uuid) != NULL)
			{
				ZOS_STRNCPY(uuidStr, tmpStr, 25);
				for(int i=0; i<sizeof(MACAddr); i++)
					MACAddr[i] = tolower(MACAddr[i]);
				ZOS_STRCAT(uuidStr, MACAddr, sizeof(uuidStr));
				ZOS_STRNCPY(uuid, uuidStr, 37); //uuid size 36 + EOL
				ret = ZCFG_SUCCESS;
			}
			ZOS_FCLOSE(fp_uuid);
		}
	}
	return ret;
}

#ifdef ZCFG_LOG_USE_DEBUGCFG
int zyUtilQueryUtilityEid(const char *name)
{
	//(!name || !strlen(name))



	if(!strcmp(name, "esmd")){

		return ZCFG_EID_ESMD;
	}
	else if(!strcmp(name, "tr69"))
        {

		return ZCFG_EID_TR69;
	}





	return 0;
}

// return: ZCFG_SUCCESS, it's debug config msg and processed
zcfgRet_t zyUtilHandleRecvdMsgDebugCfg(const char *msg)
{
	zcfgMsg_t *recvdMsg = (zcfgMsg_t *)msg;
	const char *recvdMsgData = NULL;
	struct json_object *debugObj = NULL;

	if(!recvdMsg)
		return ZCFG_INTERNAL_ERROR;

	if((recvdMsg->type != ZCFG_MSG_LOG_SETTING_CONFIG) || !recvdMsg->length)
		return ZCFG_REQUEST_REJECT;

	recvdMsgData = (const char *)(recvdMsg + 1);

	if(!recvdMsgData || !(debugObj = json_tokener_parse(recvdMsgData)))
		return ZCFG_INVALID_OBJECT;

	//printf("%s: debugobj %s\n", __FUNCTION__, recvdMsgData);
	struct json_object *levelObj = json_object_object_get(debugObj, "Level");
	if(!levelObj){
		json_object_put(debugObj);
		return ZCFG_INTERNAL_ERROR;
	}
	bool enable = json_object_get_boolean(json_object_object_get(debugObj, "Enable"));
	myDebugCfgObjLevel =  (enable == true) ? json_object_get_int(levelObj) : 0;
	const char *utilityName = json_object_get_string(json_object_object_get(debugObj, "Name"));
	if(utilityName && strlen(utilityName))
		printf("%s: debugobj %s\n", __FUNCTION__, utilityName);

	// it might require lock while utilizing myDebugCfgObj somewhere
	json_object_put(myDebugCfgObj);
	myDebugCfgObj = debugObj;

	return ZCFG_SUCCESS;
}
#else
int zyUtilQueryUtilityEid(const char *name)
{

	return 0;
}

zcfgRet_t zyUtilHandleRecvdMsgDebugCfg(const char *msg)
{

	return ZCFG_REQUEST_REJECT;
}
#endif

/**
 * zyUtilMailSend function parse receive massage and send email
 *
 * const char *msg retable json format string
 * json format string need "Event" key for event
 * Event string format:
 *             MAILSEND_EVENT_<your definition>
 *
 * MAILSEND_EVENT_3G_UP: 3G connection event
 * MAILSEND_EVENT_WIFI_UP: Wifi connection event
 * MAILSEND_EVENT_LOG_ALERT: Log alert event
 *
 ***/
zcfgRet_t zyUtilMailSend(const char *msg) {
	struct json_object *sendObj = NULL;
	struct json_object *value = NULL;
	char sysCmd[2084] = {0};
	char *bodypath = "/tmp/mailbody";
	time_t time_stamp = time(NULL);
	struct tm *tPtr = localtime(&time_stamp);
	const char *filepath = NULL;
	int files = 0, delfile = 0;
	int rm_ret;
	FILE *fp = NULL;
	char buf[128] = {0};
	bool isSendSuccess = false;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	// Create a file and waiting for entering content, if file create fail
	// the mail will send only subject without body.
	FILE *mailbody = ZOS_FOPEN(bodypath, "w");
	if(!mailbody)
		printf("Cannot create file for mail body, mail will send without content!");

	//printf("Got message: %s\n", msg);
	sendObj = json_tokener_parse(msg);

	if(mailbody != NULL)
	{
		fprintf(mailbody, "%s\n", json_object_get_string(json_object_object_get(sendObj, "body")));
		fprintf(mailbody, "\nSend at\t%s", asctime(tPtr));
		ZOS_FCLOSE_S(mailbody);
		mailbody = NULL;
	}

	json_object_object_foreach(sendObj, keys, val) {
		if(!strcmp(keys,"body")) continue;
#ifdef ZYXEL_SEND_NEW_CONNECTION_NOTIFICATION
		if(!strcmp(keys,"EmailBodySuffix")) continue;
#endif
		// check if there are any file need to attach, set true
		if(!strcmp(keys,"FilePath")) {
			files = true;
			filepath = json_object_get_string(json_object_object_get(sendObj, "FilePath"));
			continue;
		}
		// check if the file need to delete or not
		if(!strcmp(keys, "Delete")) {
			if(!strcmp(json_object_get_string(json_object_object_get(sendObj, "Delete")),"true")) {
				delfile = 1;
				continue;
			}
			continue;
		}
		zos_snprintf(sysCmd, sizeof(sysCmd), "sed -i \"s/\\#{%s}/%s/g\" %s", keys, json_object_get_string(val), bodypath);
		ZOS_SYSTEM(sysCmd);
		memset(sysCmd, 0, sizeof(sysCmd));
	}

	// mailsend command
	char mailFrom[128] = {0}, security[128] = {0}, smtp[128] = {0}, mailTo[128] = {0}, user[128] = {0}, passwd[128] = {0}, subject[128] = {0};
	int port = 0;
	ZOS_STRNCPY(mailFrom, (char *)json_object_get_string(json_object_object_get(sendObj, "mailfrom")), sizeof(mailFrom));
	port = json_object_get_int(json_object_object_get(sendObj, "port"));
	ZOS_STRNCPY(security, (char *)json_object_get_string(json_object_object_get(sendObj, "security")), sizeof(security));
	ZOS_STRNCPY(smtp, (char *)json_object_get_string(json_object_object_get(sendObj, "SMTP")),sizeof(smtp));
	ZOS_STRNCPY(mailTo, (char *)json_object_get_string(json_object_object_get(sendObj, "mailto")),sizeof(mailTo));
	ZOS_STRNCPY(user, (char *)json_object_get_string(json_object_object_get(sendObj, "user")), sizeof(user));
	ZOS_STRNCPY(passwd, (char *)json_object_get_string(json_object_object_get(sendObj, "pass")), sizeof(passwd));
	ZOS_STRNCPY(subject, (char *)json_object_get_string(json_object_object_get(sendObj, "subject")), sizeof(subject));
	zos_snprintf(sysCmd, sizeof(sysCmd), "mailsend -f %s -port %d -%s -smtp %s -domain %s -t %s -cs \"utf-8\" -auth -user %s -pass %s -sub \"%s\" ",
			mailFrom,
			port,
			security,
			smtp,
			smtp,
			mailTo,
			user,
			passwd,
			subject);

	if(files) {
		char attachCmd[2048] ={0};
		char buff[1024] = {0};
		fp = popen("cat /tmp/mailbody", "r");
		if(fp != NULL)
		{
			if ( fgets(buff, sizeof(buff), fp) != NULL)
			{
				/* mailsend can choose option between -attach file or -mime-type.
				* It can attach multi files, but if it need mail body, use option -M only
				* And now mailsend can only attach one text file */
				zos_snprintf(attachCmd, sizeof(attachCmd), " -attach \"%s,text/plain\" -M \"%s\"", filepath, buff);
				ZOS_STRCAT(sysCmd, attachCmd, sizeof(sysCmd));
			} else
				zcfgLog(ZCFG_LOG_ERR, "%s fail to get content from file!\n", __FUNCTION__);
			ZOS_FCLOSE(fp);
			fp = NULL;
		}
	} else {
		char *normalCmd = " -mime-type \"text/plain\" -msg-body \"/tmp/mailbody\"";
		ZOS_STRCAT(sysCmd, normalCmd, sizeof(sysCmd));
	}

        ZOS_STRCAT(sysCmd, " -log /tmp/mailLog", sizeof(sysCmd));

	printf("Sending Email...\n");
	//printf("%s\n", sysCmd);
	ZOS_SYSTEM(sysCmd);

	chmod("/tmp/mailLog", strtol("0777", 0, 8));
	if ((fp = ZOS_FOPEN("/tmp/mailLog", "r")) == NULL) {
		printf("fopen error!\n");
	}
	else
	{
		while (fgets(buf, sizeof(buf), fp) != NULL){
			if (strstr(buf, "Mail sent successfully") != NULL ){
				isSendSuccess = true;
				break;
			}
		}
		ZOS_FCLOSE(fp);
		fp = NULL;
	}
	unlink("/tmp/mailLog");

	if(delfile) {
		if((rm_ret = unlink(filepath)) != 0)
			printf("Cannot delete file (%s).  ret=%d\n", filepath, rm_ret);
	}
	if((rm_ret = unlink(bodypath)) != 0)
		printf("Cannot delete file (%s).  ret=%d\n", bodypath, rm_ret);

	json_object_put(sendObj);
	json_object_put(value);

	if(isSendSuccess)
		return ZCFG_SUCCESS;
	else
		return ZCFG_INTERNAL_ERROR;
}

bool zyUtilFileExists(const char * filename)
{
	FILE *file = ZOS_FOPEN(filename, "r");
	if ( file != NULL )
	{
		ZOS_FCLOSE(file);
		return true;
	}

	return false;
}

/*ex: objNameAbbr: RDM_OID_DEVICE*/
int32_t zyUtilOidGet(const char *objNameAbbr)
{
	if(objNameAbbr == NULL)
		return -1;

	zHttpOidParallel *oidDictionaryAddr = &oidDictionary[0];
	do{
		if(strcmp(objNameAbbr, oidDictionaryAddr->abbreviation) == 0 )
			return oidDictionaryAddr->oid;
		oidDictionaryAddr++;
	}while(oidDictionaryAddr->abbreviation != NULL);

	return -1;
}

#if 0
/*zyUtilIptablesLockInit() should be only called once, for now it is called by beIptablesInit() in zcfg_be_iptables.c*/
void zyUtilIptablesLockInit(){
	union semun sem_union;

	iptables_sem_id = semget(ZCFG_IPTABLES_SEM_KEY,1,IPC_CREAT|0666);

	sem_union.val = 1;
	semctl(iptables_sem_id, 0, SETVAL, sem_union);
}

void zyUtilIptablesLock(){
	struct sembuf sem_b;

	if(iptables_sem_id < 0)
		iptables_sem_id = semget(ZCFG_IPTABLES_SEM_KEY,1,IPC_CREAT|0666);

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	semop(iptables_sem_id, &sem_b, 1);
}

void zyUtilIptablesUnLock(){
	struct sembuf sem_b;

	if(iptables_sem_id < 0)
		iptables_sem_id = semget(ZCFG_IPTABLES_SEM_KEY,1,IPC_CREAT|0666);

	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	semop(iptables_sem_id, &sem_b, 1);
}
#endif

bool zyUtilInitIptablesConfig(iptCfg_t **ipt)
{
    if (ipt == NULL)
    {
        ZLOG_ERROR("ipt == NULL");
        return false;
    }

	*ipt = ZOS_MALLOC(sizeof(iptCfg_t));
	if (*ipt == NULL)
	{
	    ZLOG_ERROR("memory insufficient");
	    return false;
	}

	(*ipt)->fp_m = open_memstream(&(*ipt)->buf_m, &(*ipt)->len_m);
	(*ipt)->fp_f = open_memstream(&(*ipt)->buf_f, &(*ipt)->len_f);
	(*ipt)->fp_n = open_memstream(&(*ipt)->buf_n, &(*ipt)->len_n);
	(*ipt)->fp_m6 = open_memstream(&(*ipt)->buf_m6, &(*ipt)->len_m6);
	(*ipt)->fp_f6 = open_memstream(&(*ipt)->buf_f6, &(*ipt)->len_f6);

	return true;
}

void zyUtilRestoreIptablesConfig(iptCfg_t *ipt, const char *file, const char *func, int line){
	FILE *fp = NULL;
	char cmd[64] = {0};
	char fileName[32] = {0};
	int ret = 0;
	int fd;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s, called from %s, %s():%d\n", __FUNCTION__, file, func, line);

	ZOS_FCLOSE(ipt->fp_m);
	ZOS_FCLOSE(ipt->fp_f);
	ZOS_FCLOSE(ipt->fp_n);
	ZOS_FCLOSE(ipt->fp_m6);
	ZOS_FCLOSE(ipt->fp_f6);

	//iptables-restore
	if(ipt->len_m || ipt->len_f || ipt->len_n){
		ZOS_STRNCPY(fileName, "/tmp/iptablesConfig-XXXXXX", sizeof(fileName));
		fd = mkstemp(fileName);
		fp = fdopen(fd, "w+");
		if(fp != NULL)
		{
			if(ipt->len_m){
				fprintf(fp, "*mangle\n");
				fprintf(fp, "%s", ipt->buf_m);
				fprintf(fp, "COMMIT\n");
			}
			if(ipt->len_f){
				fprintf(fp, "*filter\n");
				fprintf(fp, "%s", ipt->buf_f);
				fprintf(fp, "COMMIT\n");
			}
			if(ipt->len_n){
				fprintf(fp, "*nat\n");
				fprintf(fp, "%s", ipt->buf_n);
				fprintf(fp, "COMMIT\n");
			}
			ZOS_FCLOSE_S(fp);
			fp = NULL;
		}
#ifdef ABUU_CUSTOMIZATION
		zos_snprintf(cmd, sizeof(cmd), "%s --noflush %s 2>/dev/null", IPTABLESRESTORE, fileName);
#else
		zos_snprintf(cmd, sizeof(cmd), "%s --noflush %s", IPTABLESRESTORE, fileName);
#endif
		ret = ZOS_SYSTEM(cmd);
		if(ret != ZOS_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s fail, ret = %d, called from %s, %s():%d\n", IPTABLESRESTORE, ret, file, func, line);
#ifndef ABUU_CUSTOMIZATION
			printf("%s fail, ret = %d, called from %s, %s():%d\n", IPTABLESRESTORE, ret, file, func, line);
#endif
			zos_snprintf(cmd, sizeof(cmd), "cat %s", fileName);
			ZOS_SYSTEM(cmd);
		}

		unlink(fileName);
	}

	//ip6tables-restore
	if(ipt->len_m6 || ipt->len_f6){
		ZOS_STRNCPY(fileName, "/tmp/iptablesConfig-XXXXXX", sizeof(fileName));
		fd = mkstemp(fileName);
		fp = fdopen(fd, "w+");
		if(fp != NULL)
		{
			if(ipt->len_m6){
				fprintf(fp, "*mangle\n");
				fprintf(fp, "%s", ipt->buf_m6);
				fprintf(fp, "COMMIT\n");
			}
			if(ipt->len_f6){
				fprintf(fp, "*filter\n");
				fprintf(fp, "%s", ipt->buf_f6);
				fprintf(fp, "COMMIT\n");
			}
			ZOS_FCLOSE_S(fp);
			fp = NULL;
		}
#ifdef ABUU_CUSTOMIZATION
		zos_snprintf(cmd, sizeof(cmd), "%s --noflush %s 2>/dev/null", IP6TABLESRESTORE, fileName);
#else
		zos_snprintf(cmd, sizeof(cmd), "%s --noflush %s", IP6TABLESRESTORE, fileName);
#endif
		ret = ZOS_SYSTEM(cmd);
		if(ret != ZOS_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s fail, ret = %d, called from %s, %s():%d\n",IP6TABLESRESTORE, ret, file, func, line);
#ifndef ABUU_CUSTOMIZATION
			printf("%s fail, ret = %d, called from %s, %s():%d\n", IP6TABLESRESTORE, ret, file, func, line);
#endif
			zos_snprintf(cmd, sizeof(cmd), "cat %s", fileName);
			ZOS_SYSTEM(cmd);
		}

		unlink(fileName);
	}

	ZOS_FREE(ipt->buf_m);
	ZOS_FREE(ipt->buf_f);
	ZOS_FREE(ipt->buf_n);
	ZOS_FREE(ipt->buf_m6);
	ZOS_FREE(ipt->buf_f6);
	ZOS_FREE(ipt);
}

// return:
// -3: auth file does not exist
// -2: user account does not exist
// -1: auth fail
//  0: auth success
//  1: not support sha512 hash account password
int zyUtilAuthUserAccount(const char *username, const char *password)
{
	static unsigned int fileSerialNumber = 19475;
	char accInfo[512] = {0};
	char accFile[32] = {0};
	char syscmd[512];
	int fd, ret=-1;
	ssize_t r;

#ifndef SHA512_PASSWD
	return 1;
#endif

	if (username == NULL || password == NULL) return -3;
	fileSerialNumber += 91;
	zos_snprintf(accFile, sizeof(accFile), "/var/tmp/tmp%d", fileSerialNumber);
	fd = open(accFile, O_RDWR|O_CREAT, 0600);
	if (fd < 0)
	{
		printf("%s.%d: cant not open file! \n", __func__, __LINE__);
		return -3;
	}

	zos_snprintf(syscmd, sizeof(syscmd), "%s\r\n%s\r\n", username, password);
	if( write(fd, syscmd, strlen(syscmd)) == -1 )
	{
		printf("%s.%d: cant not write file! \n", __func__, __LINE__);
		close(fd);
		return -3;
	}
	close(fd);

	zos_snprintf(syscmd, sizeof(syscmd), "login -z %s", accFile);
	ZOS_SYSTEM_S(syscmd);

	fd = open(accFile, O_RDONLY);
	if (fd < 0)
	{
		// zos_snprintf(syscmd, sizeof(syscmd), "rm %s", accFile);
		// ZOS_SYSTEM(syscmd);
		remove(accFile);
		printf("%s.%d: cant not open file! \n", __func__, __LINE__);
		return -3;
	}

	r = read(fd, &accInfo[0], 512);
	if ( r == -1)
		printf("%s.%d: fail to read out the content form file! \n", __func__, __LINE__);
	close(fd);

	sscanf(&accInfo[0], "result:%d", &ret);

	// zos_snprintf(syscmd, sizeof(syscmd), "rm %s", accFile);
	// ZOS_SYSTEM(syscmd);
	remove(accFile);

	return ret;
}

char *zyGetAccountInfoFromFile(char *username, char *filePath){
    static char accountInfo[513] = {0};
    FILE *fp = NULL;

    fp = ZOS_FOPEN(filePath, "r");
    if(fp == NULL){
        printf("%s: cannot open file '%s'!\n", __FUNCTION__, filePath);
        return NULL;
    }

    memset(accountInfo, 0, sizeof(accountInfo));
    while(fgets(accountInfo, sizeof(accountInfo), fp) != NULL){
        if((strncmp(username, accountInfo, strlen(username)) == 0) &&
           (accountInfo[strlen(username)] == 0x3A ) // 0x3A = ":"
          )
        {
            ZOS_FCLOSE(fp);
            return accountInfo;
        }
    }

    printf("%s: cannot find account '%s' in '%s'!", __FUNCTION__, username, filePath);
    ZOS_FCLOSE(fp);
    return NULL;
}

void zyRemoveAccountInfoFromFile(char *username, char *filePath)
{
    char    strBuff[513] = {0};
    char    *tmpFilePath = NULL;
    FILE    *fp = NULL;
    FILE    *tmpFp = NULL;

    fp = ZOS_FOPEN(filePath, "r");
    if (fp == NULL)
    {
        ZLOG_ERROR("fail to open file %s", filePath);
        return;
    }

    tmpFilePath = ZOS_MALLOC(sizeof(char) * (strlen(filePath) + 4 + 1));
    if (tmpFilePath == NULL)
    {
        ZLOG_ERROR("memory insufficient");
        if(fp != NULL) {ZOS_FCLOSE(fp); fp = NULL;}
        return;
    }

    zos_snprintf(tmpFilePath, (strlen(filePath) + 4 + 1), "%s.tmp", filePath);
    tmpFp = ZOS_FOPEN(tmpFilePath, "w");
    if (tmpFp == NULL)
    {
        ZLOG_ERROR("fail to create tmp file %s", tmpFilePath);
        ZOS_FCLOSE(fp);
        ZOS_FREE(tmpFilePath);
        return;
    }

    while (fgets(strBuff, sizeof(strBuff), fp) != NULL)
    {
        if (strncmp(username, strBuff, strlen(username)) != 0)
        {
            fputs(strBuff, tmpFp);
        }
        memset(strBuff, 0, sizeof(strBuff));
    }

    ZOS_FCLOSE_S(tmpFp);
    ZOS_FCLOSE(fp);

    // zos_snprintf(strBuff, sizeof(strBuff), "mv %s %s", tmpFilePath, filePath);
    // ZOS_SYSTEM(strBuff);
    // zos_snprintf(strBuff, sizeof(strBuff), "chmod 644 %s", filePath);
    // ZOS_SYSTEM(strBuff);
	ZOS_MOVE_FILE(filePath, tmpFilePath);
	chmod(filePath, 0644);

    ZOS_FREE(tmpFilePath);
}

#ifdef ABOG_CUSTOMIZATION
#define REMOTE_MANAGEMENT_FILE_FOLDER   "/tmp/rmm/"
void zyTouchFile(int pid, char *ip)
{
	char    cmd[128] = {0};
	char    *addr = NULL;
	FILE    *fp = NULL;
	char    filename[32] ={0};

	fp = ZOS_FOPEN("/tmp/isWANRoute.sh", "r");
	if (fp != NULL)
	{
		ZOS_FCLOSE(fp);
		fp = NULL;
		fp = ZOS_FOPEN("/tmp/isWANRoute.sh", "w");
		if (fp == NULL)
		{
			ZLOG_ERROR("fp == NULL");
		}
		else
		{
		fprintf(fp, "route=`ip r get $1`\n");
		fprintf(fp, "if expr \"$1\" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then\n");
		fprintf(fp, "	iface=`echo $route | awk '{print substr($3,1,3)}'`\n");
		//fprintf(fp, "	echo iface is $iface\n");
		fprintf(fp, "	if [ \"$iface\" == \"nas\" ]; then\n");
		//fprintf(fp, "		echo $1 comes from wan side\n");
		fprintf(fp, "		exit 1\n");
		fprintf(fp, "	fi\n");
		fprintf(fp, "	iface=`echo $route | awk '{print substr($5,1,3)}'`\n");
		//fprintf(fp, "	echo iface is $iface\n");
		fprintf(fp, "	if [ \"$iface\" == \"nas\" ]; then\n");
		//fprintf(fp, "		echo $1 comes from wan side\n");
		fprintf(fp, "		exit 1\n");
		fprintf(fp, "	fi\n");
		//fprintf(fp, "	echo $1 comes from br side\n");
		fprintf(fp, "else\n");
		fprintf(fp, "	iface=`echo $route | awk '{print substr($5,1,3)}'`\n");
		//fprintf(fp, "	echo iface is $iface\n");
		fprintf(fp, "	if [ \"$iface\" == \"nas\" ]; then\n");
		//fprintf(fp, "		echo $1 comes from wan side\n");
		fprintf(fp, "		exit 1\n");
		fprintf(fp, "	fi\n");
		fprintf(fp, "	iface=`echo $route | awk '{print substr($7,1,3)}'`\n");
		//fprintf(fp, "	echo iface is $iface\n");
		fprintf(fp, "	if [ \"$iface\" == \"nas\" ]; then\n");
		//fprintf(fp, "		echo $1 comes from wan side\n");
		fprintf(fp, "		exit 1\n");
		fprintf(fp, "	fi\n");
		//fprintf(fp, "	echo $1 comes from br side\n");
		fprintf(fp, "fi\n");
		fprintf(fp, "exit 0\n");
#if 0
		fprintf(fp, "route=`ip r get $1`\n");
		fprintf(fp, "iface=`echo $route | awk '{print substr($3,1,2)}'`\n");
		fprintf(fp, "if [ \"$iface\" == \"nas\" ]; then\n");
		//fprintf(fp, "	echo $1 comes from wan side\n");
		fprintf(fp, "	exit 1\n");
		fprintf(fp, "fi\n");
		//fprintf(fp, "	echo $1 comes from br side\n");
		fprintf(fp, "exit 0\n");
#endif
		ZOS_FCLOSE_S(fp);
		fp = NULL;
		}
	}

	chmod("/tmp/isWANRoute.sh", S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);

	if (strstr(ip, "."))
	{
		addr = strstr(ip, ":");
		if (addr)
			addr[0] = 0;
	}
	zos_snprintf(cmd, sizeof(cmd), "/tmp/isWANRoute.sh %s", ip);
	if (addr)
		addr[0] = ':';

	if ( ZOS_SYSTEM(cmd) == ZOS_FAIL )
		return;

	DIR* dir = opendir(REMOTE_MANAGEMENT_FILE_FOLDER);
	if (dir)
	{
		/* Directory exists. */
		closedir(dir);
	}
	else
	{
		// zos_snprintf(cmd, sizeof(cmd), "mkdir %s", REMOTE_MANAGEMENT_FILE_FOLDER);
		// ZOS_SYSTEM(cmd);
		ZOS_MKDIR(REMOTE_MANAGEMENT_FILE_FOLDER);
	}
	//zos_snprintf(cmd, sizeof(cmd), "touch %s%d", REMOTE_MANAGEMENT_FILE_FOLDER, pid);
	//ZOS_SYSTEM(cmd);
	//zyxel "touch" replacement
	zos_snprintf(filename, sizeof(filename), "%s%d", REMOTE_MANAGEMENT_FILE_FOLDER, pid);
	ZOS_TOUCH_FILE(filename);
#ifdef ZYXEL_ECHO_REPLACEMENT
	ZOS_ECHO_STR_TO_FILE("/proc/tc3162/led_pwr_green", "0\n", 2, "w");
    ZOS_ECHO_STR_TO_FILE("/proc/tc3162/led_pwr_green", "1\n", 2, "w");
#else
	ZOS_SYSTEM("echo 0 > /proc/tc3162/led_pwr_green");
	ZOS_SYSTEM("echo 1 > /proc/tc3162/led_pwr_green");
#endif
}

static int zyCountFilesInRemoteManageFolder(void)
{
	int file_count = 0;
	DIR *dirp;
	struct dirent *entry;

	dirp = opendir(REMOTE_MANAGEMENT_FILE_FOLDER); /* There should be error handling after this */
	if (dirp == NULL)
	{
		return 0;
	}

	while ((entry = readdir(dirp)) != NULL)
	{
		if (entry->d_type == DT_REG) /* If the entry is a regular file */
		{
			++file_count;
		}
	}
	closedir(dirp);
	return file_count;
}

void zyRemoveFile(int pid)
{
	char cmd[128] = {0};
	char rmfile[128] = {0};

	DIR* dir = opendir(REMOTE_MANAGEMENT_FILE_FOLDER);
	if (dir)
	{
		/* Directory exists. */
		closedir(dir);
	}
	else
	{
		return;
	}
	//zos_snprintf(cmd, sizeof(cmd), "rm %s%d &>/dev/null", REMOTE_MANAGEMENT_FILE_FOLDER, pid);
	//ZOS_SYSTEM(cmd);
	//zyxel "rm" replacement
	zos_snprintf(rmfile, sizeof(cmd), "%s%d", REMOTE_MANAGEMENT_FILE_FOLDER, pid);
	unlink(rmfile);

	if (zyCountFilesInRemoteManageFolder() == 0)
	{
#ifdef ZYXEL_ECHO_REPLACEMENT
	    ZOS_ECHO_STR_TO_FILE("/proc/tc3162/led_pwr_green", "0\n", 2, "w");
	    ZOS_ECHO_STR_TO_FILE("/proc/tc3162/led_pwr_green", "2\n", 2, "w");
#else
		ZOS_SYSTEM("echo 0 > /proc/tc3162/led_pwr_green");
		ZOS_SYSTEM("echo 2 > /proc/tc3162/led_pwr_green");
#endif
	}
}
#endif
#ifdef CONFIG_ABZQ_CUSTOMIZATION
void zySetLoginInfo (char *username, char *remoAddr_Type, char *addr, char *app)
{
	char cmd [60] = {0};
	if(access(LOGIN_INFO_FILE_NAME, F_OK) < 0){
		snprintf (cmd, sizeof(cmd), "touch %s", LOGIN_INFO_FILE_NAME);
		system (cmd);

		snprintf (cmd, sizeof(cmd), "chmod 777 %s", LOGIN_INFO_FILE_NAME);
		system (cmd);
	}

	snprintf (cmd, sizeof(cmd), "echo >> %s \"%s\"", LOGIN_INFO_FILE_NAME, username);
	system (cmd);

	snprintf (cmd, sizeof(cmd), "echo >> %s \"%s\"", LOGIN_INFO_FILE_NAME, remoAddr_Type);
	system (cmd);

	snprintf (cmd, sizeof(cmd), "echo >> %s \"%s\"", LOGIN_INFO_FILE_NAME, addr);
	system (cmd);

	snprintf (cmd, sizeof(cmd), "echo >> %s \"%s\"", LOGIN_INFO_FILE_NAME, app);
	system (cmd);

	return;
}
#endif //CHT_CUSTOMIZATION_SECU_LOG_FORMA



/*!
 *  Check if it is an alphanumeric character.
 *
 *  @param[in] unsigned char c  Input character.
 *  @return                     1:alphanumeric; 0:non-alphanumeric
 */
static int _common_isalnum(unsigned char c);
static int _common_isalnum(unsigned char c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

/*!
 *  Check if it is Hex. value.
 *
 *  @param[in] unsigned char c  Input character.
 *  @return                     1:hex; 0:non-hex
 */
static int _common_isHex(unsigned char ch);
static int _common_isHex(unsigned char ch)
{
    /* refer to ui_zycommands.c */
	return (((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'F')));
}

/*!
 *  Check serial number format.
 *
 *  @param[in] char *str        Input string.
 *  @param[in] int len          Lenth of input string.
 *  @return                     0:ok; -1:error.
 */
int zyUtilChkSnFormat(char *str, int len)
{
    int i = 0, ret = -1;

#ifdef ZYXEL_NEW_ATSN
    if (len != 12)
#else
    if (len != SERIAL_NUM_STR_LEN)
#endif
    {
        return ret;
    }

    ret = 0;
#ifdef ZYXEL_NEW_ATSN
    for (i = 0; i < 12; ++i)
#else
    for (i = 0; i < SERIAL_NUM_STR_LEN; ++i)
#endif
    {
        if ( ! _common_isalnum(str[i]) )
        {
            ret = -1; //format error
            return ret;
        }
    }

    return ret;
}

/*!
 *  Check GPON serial number format.
 *
 *  @param[in] char *str        Input string.
 *  @param[in] int inputLen     Lenth of input string.
 *  @param[in] int chkLen       Lenth want to check.
 *  @return                     0:ok; -1:error.
 */
int zyUtilChkGPONSnFormat(char *str, int inputLen, int chkLen)
{
    int i = 0, ret = -1;

    if (inputLen == chkLen)
    {
        for (i = 0; i < 4; i++)
        {
            if ( ! _common_isalnum(str[i]) )
            {
                return ret;
            }
        }
        for (i = 4; i < chkLen; i++) /* refer to ui_zycommands.c */
        {
            if ( ! _common_isHex(str[i]) )
            {
                return ret;
            }
        }
        ret = 0;
    }

    return ret;
}

/*!
 *  Check password format.
 *
 *  @param[in] char *str        Input string.
 *  @param[in] int inputLen     Lenth of input string.
 *  @param[in] int chkLen       Lenth want to check.
 *  @return                     0:ok; -1:error.
 */
int zyUtilChkPwdFormat(char *str, int inputLen, int chkLen)
{
    int i = 0, ret = -1;

    if (inputLen == chkLen)
    {
        for (i = 0; i < chkLen; i++)
        {
            if ( ! _common_isalnum(str[i]) )
            {
                return ret;
            }
        }
        ret = 0;
    }

    return ret;
}

/*!
 *  Check the string is printable.
 *
 *  @param[in] char *str        Input string.
 *  @param[in] int inputLen     Lenth of input string.
 *  @return                     0:ok; -1:error.
 */
int zyUtilIsPrintableString(char *tmp, int len){
    int i=0;
    for( i=0 ; i<len ; i++ )
    {
        if( (tmp[i] < 0x20) || (tmp[i] > 0x7e) )
            return -1;
    }
    return 0;
}

/*!
 *  Check the string is alphanumeric.
 *
 *  @param[in] char *str        Input string.
 *  @param[in] int inputLen     Lenth of input string.
 *  @return                     0:ok; -1:error.
 */
int zyUtilIsAlphanumericString(char *tmp, int len){
    int i=0;
    for( i=0 ; i<len ; i++ )
    {
        if ( ! _common_isalnum(tmp[i]) )
        {
            return -1;
        }
    }
    return 0;
}

/*!
*  replace string
*  @param[IN/OUT]	source
*  @param[IN]	find_str //keyword
*  @param[IN]	rep_str //replace
*/
void str_replace (char *source, char* find_str, char*rep_str){
	char result[65] = {0};
	char* str_temp = NULL;
	char* str_location = NULL;
	int rep_str_len = 0;
	int find_str_len = 0;
	int gap = 0;

	ZOS_STRNCPY(result, source, sizeof(result));
	find_str_len = strlen(find_str);
	rep_str_len = strlen(rep_str);
	str_temp = source;
	str_location = strstr(str_temp, find_str);

	while(str_location != NULL){
		gap += (str_location - str_temp);
		result[gap] = '\0';
		ZOS_STRCAT(result, rep_str, sizeof(result));
		gap += rep_str_len;
		str_temp = str_location + find_str_len;
		ZOS_STRCAT(result, str_temp, sizeof(result));
		str_location = strstr(str_temp, find_str);
	}

	result[strlen(result)] = '\0';
	memset(source, 0 , strlen(source));
	ZOS_STRNCPY(source, result, (strlen(result)+1));
}

int zyUtilReadPidfile(char *pidFile)
{
	FILE        *fptr = NULL;
	int         pid = 0;

	fptr = ZOS_FOPEN(pidFile, "r");
	if (fptr == NULL)
	{
	    ZLOG_WARNING("fail to open pid file = %s", pidFile);
	    return 0;
	}

	if ( fscanf(fptr, "%d", &pid) != 1)
		ZLOG_WARNING("fail to get the content from file!");
	ZOS_FCLOSE(fptr);

	return pid;
}

#if defined(CONFIG_ABZQ_CUSTOMIZATION)
#define IPV4_SUBNET_STR_LEN 16
int getSubnetPrefixLen4(char* subnet, char* subnetMaskLen)
{
    char *str = NULL;
    char *delim = ".";
    char subnet_arr[IPV4_SUBNET_STR_LEN];
    char *subnet_ptr = NULL;
    int byte = 0;
    int prefixLen = 0;

    memset(subnet_arr, 0, IPV4_SUBNET_STR_LEN);
    strncpy(subnet_arr, subnet, IPV4_SUBNET_STR_LEN);
    subnet_ptr = subnet_arr;
    str = strtok(subnet_ptr, delim);

    while (str) {
        byte = atoi(str);
        if (byte < 0 || byte > 255) {//invalid byte
            return -1;
        }
        while (byte > 0 && byte <= 255) {
            byte = byte >> 1;
            prefixLen++;
        }
        str = strtok(NULL, delim);
    }
    sprintf(subnetMaskLen, "%d", prefixLen);
    return 0;
}
#endif //CONFIG_ABZQ_CUSTOMIZATION

/*!
 *  send msg to dstEid
 *
 *  @param [in]   msgType       message type, define in zcfg_msg_type.h
 *  @param [in]   srcEid        source process eid
 *  @param [in]   dstEid        destination process eid
 *  @param [in]   payloadLen    message payload lengh
 *  @param [in]   payload       message payload
 *  @param [out]  returnStr     response message payload string
 *
 *  @return ZCFG_SUCCESS        successfully send and get reply
 *          others              failed
 *
 */
zcfgRet_t zyUtilMsgSend(uint32_t msgType, zcfgEid_t srcEid, zcfgEid_t dstEid, int payloadLen, const char *payload, char **returnStr)
{
    zcfgRet_t ret;
    void *sendBuf = NULL;
    void *recvBuf = NULL;
    zcfgMsg_t *sendMsgHdr = NULL;
    char *recv_str = NULL;
    int buf_size = 0;

    buf_size = sizeof(zcfgMsg_t)+payloadLen;
    sendBuf = malloc(buf_size);

    if (sendBuf == NULL)
    {
        ZLOG_ERROR("sendMsg == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    sendMsgHdr = (zcfgMsg_t*)sendBuf;

    sendMsgHdr->type = msgType;
    sendMsgHdr->length = payloadLen;
    sendMsgHdr->srcEid = srcEid; //eid defined at zcfg_eid.h
    sendMsgHdr->dstEid = dstEid;

    if (payload != NULL)
    {
        memcpy(sendBuf+sizeof(zcfgMsg_t), payload, payloadLen);
    }

    ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

    if (ret == ZCFG_SUCCESS)
    {
        recv_str = (char *)recvBuf+sizeof(zcfgMsg_t);
        //printf("Receive message : %s\n", recv_str);
        ZLOG_DEBUG("Receive message : %s\n", recv_str);

        *returnStr = recv_str;
        /*switch(msgType){
        case ZCFG_MSG_XXX:
        //printf("reply msgType = %d\n", msgType);
        break;
        }*/
        free(recvBuf);
    }
    else
    {
        ZLOG_ERROR("zcfgMsgSendAndGetReply error");
    }

    return ret;
}
