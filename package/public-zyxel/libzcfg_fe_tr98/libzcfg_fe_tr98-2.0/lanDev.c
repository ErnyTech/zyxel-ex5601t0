#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_net.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "lanDev_parameter.h"
#include "zos_api.h"

#define WLAN_MAXBITRATE (1000)
#define WIFI_24_PRIMARY_LINK 1
#define WIFI_5_PRIMARY_LINK (TOTAL_INTERFACE_24G + 1)
#define WPA2_ENTERPRISE 1
#define WPA2_PERSONAL  2

#define TKIPAES 1
#define AES 2
#define TKIP 3

#ifdef SAAAE_TR69_NO_LAN_HOST
#include "zcfg_net.h"
#define LEASEFILE DNSMASQ_DHCP_LEASE_FILE
#define ASSOCCMD "wlctl -i wl0 assoclist"
#define ASSOCCMD_5G "wlctl -i wl1 assoclist"

/*X_ZYXEL_ConnectionType*/
#define CONNTYPE_ETH "Ethernet"
#define CONNTYPE_USB "USB"
#define CONNTYPE_WIRELESS "802.11"
#define CONNTYPE_HOMEPNA "HomePNA"
#define CONNTYPE_HOMEPLUG "HomePlug"
#define CONNTYPE_OTHER "Other"

/*AddressSource*/
#define ADDRSRC_DHCP 	"DHCP"
#define ADDRSRC_STATIC 	"Static"
#define ADDRSRC_AUTO 	"AutoIP"
#define ADDRSRC_NONE 	"None"

#define TAG_DHCP 		(1 << 1)
#define TAG_WIFI 		(1 << 2)
#define TAG_LOSTWIFI 	(1 << 3)
#endif
#define ARPFILE "/proc/net/arp"

#define MODIFY_HOSTS_AND_HOST_FUN 1

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
#define WPS_DEVICENAME "DX3200-B0"
#else
#define WPS_DEVICENAME "wps_devicename"
#endif

enum {
	WPS_PIN = 1,
	WPS_PBC
};

enum {
	WPS_STA_PIN = 1,
	WPS_AP_PIN
};

extern tr98Object_t tr98Obj[];

#ifdef WIFISECURITY_AUTH_WITH_EAP

#define TR98_PARM_LAN_DEV_WLAN_CFG "{\
		\"BeaconType\": { \"Basic\": { }, \"WPA\": { }, \"11i\": { }, \"WPAand11i\": { }, \"WPA3\": { }, \"11iandWPA3\": { }, \"None\": { }, \"Emp\": { } },\
		\"DeviceOperationMode\": { \"InfrastructureAccessPoint\": { }, \"Emp\": { } },\
		\"BasicEncryptionModes\": { \"None\": { }, \"WEPEncryption\": { } },\
		\"BasicAuthenticationMode\": { \"None\": { }, \"EAPAuthentication\": { }, \"SharedAuthentication\": { } },\
  		\"WPAEncryptionModes\": { \"TKIPEncryption\": { }, \"TKIPandAESEncryption\": { }, \"Emp\": { } },\
  		\"WPAAuthenticationMode\": { \"PSKAuthentication\": { }, \"EAPAuthentication\": { }, \"Emp\": { } },\
  		\"IEEE11iEncryptionModes\": { \"AESEncryption\": { }, \"Emp\": { } },\
  		\"IEEE11iAuthenticationMode\": { \"PSKAuthentication\": { }, \"EAPAuthentication\": { }, \"Emp\": { } },\
		}"

#else

#define TR98_PARM_LAN_DEV_WLAN_CFG "{\
		\"BeaconType\": { \"Basic\": { }, \"WPA\": { }, \"11i\": { }, \"WPAand11i\": { }, \"WPA3\": { }, \"11iandWPA3\": { }, \"None\": { } },\
		\"DeviceOperationMode\": { \"InfrastructureAccessPoint\": { }, \"Emp\": { } },\
		\"BasicEncryptionModes\": { \"None\": { }, \"WEPEncryption\": { } },\
		\"BasicAuthenticationMode\": { \"None\": { } },\
  		\"WPAEncryptionModes\": { \"AESEncryption\": { }, \"TKIPEncryption\": { }, \"TKIPandAESEncryption\": { }, \"Emp\": { } },\
  		\"WPAAuthenticationMode\": { \"PSKAuthentication\": { }, \"Emp\": { } },\
  		\"IEEE11iEncryptionModes\": { \"AESEncryption\": { }, \"TKIPEncryption\": { }, \"TKIPandAESEncryption\": { }, \"Emp\": { } },\
  		\"IEEE11iAuthenticationMode\": { \"PSKAuthentication\": { }, \"Emp\": { } },\
		\"X_ZYXEL_OperatingFrequencyBand\": { \"2.4GHz\": { }, \"5GHz\": { } },\
		\"X_ZYXEL_OperatingChannelBandwidth\": { \"20MHz\": { }, \"40MHz\": { }, \"80MHz\": { } , \"160MHz\": { } },\
		}"

#endif

bool checkValidNetMask(char *netMask, uint32_t minPrefixLen)
{
	char buf[16] = {0};
	char *token = NULL, *tmp = NULL;
	int i = 0, byte = 0;
	uint32_t netMaskSum = 0;

	strcpy(buf, netMask);
	token = strtok_r(buf, ".", &tmp);
	for(i = 0; token != NULL; i++) {
		byte = atoi(token);
		if(byte > 255 || byte < 0)
			return false;

		netMaskSum = (netMaskSum<<8) | byte;
		token = strtok_r(NULL, ".", &tmp);
	}

	/* invalid netmask format. */
	if(i != 4 || (netMaskSum & (~netMaskSum>>1)))
		return false;

	/* restrict minimum prefix length. i.g. 24 = 255.255.255.0 ~ 255.255.255.255 */
	if(minPrefixLen > 0) {
		for(i = 0; netMaskSum > 0; i++)
			netMaskSum = (netMaskSum << 1);

		if(i < minPrefixLen)
			return false;
	}

	return true;
}

int getNumOfBridge()
{
	objIndex_t objIid;
	int numOfBridge = 0;
	rdm_Bridging_t *bridgingObj = NULL;

	IID_INIT(objIid);
	zcfgFeObjStructGet(RDM_OID_BRIDGING, &objIid, (void **)&bridgingObj);
	numOfBridge = bridgingObj->BridgeNumberOfEntries;
	zcfgFeObjStructFree(bridgingObj);
	return numOfBridge;
}

#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
bool setDhcpAddresses(char *ip, char *mask, char *start, char *end){
	int ipArr[4], maskArr[4], startArr[4], endArr[4];
	char tmpstr[5] = {0};
        char lanIp[16] = {0};
        char subnetMask[16] = {0};
	char *pch;
	int i;

        strcpy(lanIp, ip);
        strcpy(subnetMask, mask);

	i = 0;
	pch = strtok(lanIp, ".");
	while (pch != NULL){
		ipArr[i] = startArr[i] = endArr[i] = atoi(pch);
		pch = strtok(NULL, ".");
		i++;
	}

	i = 0;
	pch = strtok(subnetMask, ".");
	while (pch != NULL){
		maskArr[i] = atoi(pch);
		pch = strtok(NULL, ".");
		i++;
	}

	int bcast2 = 255, bcast3 = 255;
	int n2 = 0, n3=0;
	int hosts2, hosts3;
	char dhcpStart[16] = {0};
	char dhcpEnd[16] = {0};

	if (maskArr[0] == 255 && maskArr[1] == 0 &&  maskArr[2] == 0 && maskArr[3] == 0) {
	/* 255.0.0.0 is only class A supported */
		endArr[1] = 255;
		endArr[2] = 255;
	}
	else if ((maskArr[0] == 255 && maskArr[1] == 255 && maskArr[3] == 0) &&
		(maskArr[2] == 0 || maskArr[2] == 128 || maskArr[2] == 192 || maskArr[2] == 224 || maskArr[2]== 240 || maskArr[2] == 248 ||  maskArr[2] == 252)){
		/* 255.255.0.0 is only class B supported */
		n2 = ipArr[2] & maskArr[2];
		hosts2 = 255 - maskArr[2];
		bcast2 = n2 + hosts2;
	}
	else if ((maskArr[0] == 255 && maskArr[1] == 255 &&  maskArr[2] == 255 ) &&
		(maskArr[3] == 0 || maskArr[3] == 128 || maskArr[3] == 192 || maskArr[3] == 224 || maskArr[3]== 240 || maskArr[3] == 248 ||  maskArr[3] == 252)) {
		/* 255.255.255.0/128/192... class C supported */
		n3 = ipArr[3] & maskArr[3];
		hosts3 = 255 - maskArr[3];
		bcast3 = n3 + hosts3;
	}
	else
		return false;

	if (ipArr[3] == bcast3){
		printf("warning !! Invalid IP address");
		return false;
	}

	for (i = 0; i < 2; i++){/*set the first 2 octets of addresses*/
		sprintf(tmpstr, "%d.", startArr[i]);
		strcat(dhcpStart, tmpstr);
		sprintf(tmpstr, "%d.", endArr[i]);
		strcat(dhcpEnd, tmpstr);
	}

	/*set the third octets of addresses*/
	if (ipArr[2] == (bcast2-1)) {
		startArr[2] = n2;
		endArr[2] = bcast2 -2;
	}
	else if(maskArr[2] == 255){
		startArr[2] = ipArr[2];
		endArr[2] = ipArr[2];
	}
	else if(maskArr[2] == 0){
		startArr[2] = ipArr[2];
		endArr[2] = 254;
	}
	else {
		startArr[2] = ipArr[2];
		endArr[2] = bcast2 -1;
	}

	sprintf(tmpstr, "%d.", startArr[2]);
	strcat(dhcpStart, tmpstr);
	sprintf(tmpstr, "%d.", endArr[2]);
	strcat(dhcpEnd, tmpstr);

	/*set the last octets of addresses*/
	if (ipArr[3] == (bcast3-1)) {
		startArr[3] = n3 + 1;
		endArr[3] = bcast3 -2;
	}
	else {
		startArr[3] = ipArr[3] + 1;
		endArr[3] = bcast3 -1;
	}

	sprintf(tmpstr, "%d", startArr[3]);
	strcat(dhcpStart, tmpstr);
	sprintf(tmpstr, "%d", endArr[3]);
	strcat(dhcpEnd, tmpstr);

        strcpy(start, dhcpStart);
        strcpy(end, dhcpEnd);
	return true;
}
#endif

#ifdef SAAAE_TR69_NO_LAN_HOST
typedef struct hostinfo_s{
        char hwa[65];
        char hostname[65];
        char vid[256];
        int tag;
        unsigned long int lease;
}hostinfo_t;

typedef struct hostlist_s{
        struct hostlist_s *next;
        hostinfo_t hostinfo;
}hostlist_t;

hostlist_t* addHostToList(hostlist_t *list, char *hwa){
        hostlist_t *theHostlist = NULL;
        hostlist_t *tmpHostlist = NULL;
        theHostlist = malloc(sizeof(hostlist_t));
        memset(theHostlist, '\0', sizeof(hostlist_t));
        if(list){
                tmpHostlist = list;
                while(tmpHostlist->next){
                        tmpHostlist = tmpHostlist->next;
                }
                tmpHostlist->next = theHostlist;
                memcpy(&theHostlist->hostinfo, hwa, strlen(hwa));
        }
        else{
                list = theHostlist;
        }
        return theHostlist;
}

hostlist_t* findHostFromList(hostlist_t *list, char *hwa){
        hostlist_t *tmpHostlist = NULL;

        tmpHostlist = list;
        while(tmpHostlist){
                if(strcasecmp(tmpHostlist->hostinfo.hwa, hwa)==0){
                        return tmpHostlist;
                }
                tmpHostlist = tmpHostlist->next;
        }
        return NULL;
}

void freeHostList(hostlist_t *list){
	hostlist_t *theHostlist = NULL;
	theHostlist = list;
	while(theHostlist){
		list = theHostlist->next;
		free(theHostlist);
		theHostlist = list;
	}
}

zcfgRet_t lanDevHostListUpdate(hostlist_t **hostlist, int flag)
{
	FILE *leasefp = NULL;
	FILE *assocfp = NULL;
	char assoctmp[] = "/tmp/assoclist";
	char syscmd[128]={0};
	char line[512]={0};
	char ip[32]={0};
	char hwa[65]={0};
	char clid[64]={0};
	char hostname[65]={0};
	char tmpVid[256]={0};
	char *vid = NULL;
	int num = 0;
	unsigned long int lease;
	hostlist_t *theHostlist = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s()\n", __FUNCTION__);

	if(flag){
		/*DHCP lease*/
		zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, LEASEFILE);
		leasefp = fopen( LEASEFILE, "r" );
		if(leasefp != NULL){
			while (fgets(line, sizeof(line), leasefp)) {
				num = sscanf(line, "%lu %[^ ] %[^ ] %[^ ] %[^ ] %[^ ]\n",
							&lease, hwa, ip, hostname, clid, tmpVid);
				if((theHostlist = findHostFromList(hostlist, hwa))== NULL){
					theHostlist = addHostToList(hostlist, hwa );
					if(hostlist == NULL){
						hostlist = theHostlist;
					}
				}
				if(strcmp(hostname, "*")!=0){
					strncpy(theHostlist->hostinfo.hostname, hostname, sizeof(theHostlist->hostinfo.hostname)-1);
				}else{
					strncpy(theHostlist->hostinfo.hostname, "Unknown", sizeof(theHostlist->hostinfo.hostname)-1);
				}
				if(strcmp(tmpVid, "*")!=0 && num>5){
					vid = strstr(line, tmpVid);
					if(vid != NULL){
						vid = strtok(vid, "\n");
						if(strlen(vid)<sizeof(theHostlist->hostinfo.vid))
							strncpy(theHostlist->hostinfo.vid, vid, strlen(vid));
						else
							strncpy(theHostlist->hostinfo.vid, vid, sizeof(theHostlist->hostinfo.vid)-1);
					}
				}
				strncpy(theHostlist->hostinfo.hwa, hwa, sizeof(theHostlist->hostinfo.hwa)-1);

				theHostlist->hostinfo.tag |= TAG_DHCP;
				theHostlist->hostinfo.lease = lease;
			}
			fclose(leasefp);
		}
	}

	/*Wireless*/
	zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, ASSOCCMD);
	sprintf(syscmd, "%s >> %s", ASSOCCMD, assoctmp);
	system(syscmd);

	memset(syscmd, '\0', sizeof(syscmd));
	zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, ASSOCCMD_5G);
	sprintf(syscmd, "%s >> %s", ASSOCCMD_5G, assoctmp);
	system(syscmd);

	assocfp = fopen(assoctmp, "r");
	if(assocfp != NULL){
		while (fgets(line, sizeof(line), assocfp)) {
			zcfgLog(ZCFG_LOG_INFO, "%s(): wireless: %s\n", __FUNCTION__, line);
			num = sscanf(line, "assoclist %s\n",
						 hwa);
			if((theHostlist = findHostFromList(hostlist, hwa))== NULL){
				theHostlist = addHostToList(hostlist, hwa );
				if(hostlist == NULL){
					hostlist = theHostlist;
				}
			}
			if(!strcmp(theHostlist->hostinfo.hostname, "*")){
				strncpy(theHostlist->hostinfo.hostname, "Unknown", sizeof(theHostlist->hostinfo.hostname)-1);
			}
			strncpy(theHostlist->hostinfo.hwa, hwa, sizeof(theHostlist->hostinfo.hwa));
			theHostlist->hostinfo.tag |= TAG_WIFI;
		}
		fclose(assocfp);
		unlink(assoctmp);
	}

	return ZCFG_SUCCESS;
}
#endif

#if 0
static zcfgRet_t ethLinkAdd(char *ethLinkPathName, char *lowerLayer)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_EthLink_t *ethLinkObj = NULL;

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS){
		return ret;
	}

	sprintf(ethLinkPathName, "Ethernet.Link.%d", objIid.idx[0]);

	/*Set LowerLayer for Ethernet.Link*/
	if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS){
		ethLinkObj->Enable= true;
		strcpy(ethLinkObj->LowerLayers, lowerLayer);
		if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS){
			printf("Set Ethernet.Link LowerLayers Fail\n");
		}
		zcfgFeObjStructFree(ethLinkObj);
	}
	else
		return ret;

	return ret;
}
#endif

bool isSameMac(const char *mac1, const char *mac2){
	int i;
	for(i=0;i<18;i++){
		if((*mac1 == *mac2) || (*mac1 == *mac2 + 32) || (*mac1 == *mac2 - 32)){
			mac1++;
			mac2++;
			continue;
		}
		else if(*mac1 == ':' || *mac1 == '-'){
			mac1++;
			mac2++;
			continue;
		}
		else 
			return false;
		
		mac1++;
		mac2++;
	}
	return true;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.Hosts

    Related object in TR181:
    Device.Hosts
*/
zcfgRet_t lanDevHostsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
#ifdef SAAAE_TR69_NO_LAN_HOST
	FILE *arpfp = NULL;
	char line[512]={0};
	int type, flags;
	char ip[32]={0};
	char hwa[65]={0};
	char mask[32]={0};
	char dev[32]={0};
	int num = 0;
	uint32_t hostNum = 0;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*ARP table*/
	zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, ARPFILE);
	arpfp = fopen(ARPFILE, "r");
	if(arpfp == NULL){
		zcfgLog(ZCFG_LOG_ERR, "%s(): open %s fail\n", __FUNCTION__, ARPFILE);
		return ZCFG_SUCCESS;
	}
	fgets(line, sizeof(line), arpfp);
	while (fgets(line, sizeof(line), arpfp)) {
		mask[0] = '-'; mask[1] = '\0';
		dev[0] = '-'; dev[1] = '\0';
		/* All these strings can't overflow
		 * because fgets above reads limited amount of data */
		num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
					 ip, &type, &flags, hwa, mask, dev);
		if (num < 4)
			break;

		/*incomplete, skip*/
		if(flags == 0){
			continue;
		}

		if(strcmp(hwa,"00:00:00:00:00:00")==0){
			continue;
		}

		// all LAN dev should be br+ ,no eth0, eth1, eth2...
		if(strstr(dev, "br") == NULL)
			continue;

		hostNum++;
	}
	fclose(arpfp);

	/*fill up tr98 Host object*/
	*tr98Jobj = json_object_new_object();
	/*HostNumberOfEntries*/
	json_object_object_add(*tr98Jobj, "HostNumberOfEntries", json_object_new_int(hostNum));
#else
	zcfgRet_t ret;
	objIndex_t objIid;
#if MODIFY_HOSTS_AND_HOST_FUN // modify 20150809
	struct json_object *hostsObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef ABUU_CUSTOMIZATION
	objIndex_t  natSessionSTAIid;
	struct json_object *natSessionSTAJobj = NULL;
#endif
#else
	char tr181Name[32] = {0};
	char tr98TmpName[32] = {0};
	char higherLayerPath[32] = {0};
	char *ptr = NULL;
	uint32_t hostNum = 0;
	rdm_HostsHost_t *stHost = NULL;
#endif
	char mappingPathName[32] = {0};

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

#if MODIFY_HOSTS_AND_HOST_FUN // modify 20150809
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_HOSTS, &objIid, &hostsObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}
#ifdef ABUU_CUSTOMIZATION
	IID_INIT(natSessionSTAIid);
	if((ret = feObjJsonGet(RDM_OID_NAT_SESSION_CTL_ST, &natSessionSTAIid, &natSessionSTAJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get RDM_OID_NAT_SESSION_CTL_ST Fail\n", __FUNCTION__);
		return ret;
	}
#endif

	struct json_object *tr181LanDevJobj = NULL;
	int hostNumOfEntries = 0;
	char tr98LanDevPath[256+1] = {0};
	strcpy(tr98LanDevPath, tr98FullPathName);
	char *lanDev = NULL;
	if((lanDev = strstr(tr98LanDevPath, ".Hosts"))){
		*lanDev = '\0';
	}else{
		return ZCFG_NO_SUCH_OBJECT;
	}

	if(zcfgFe98To181MappingNameGet(tr98LanDevPath, mappingPathName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: tr181 mapping, %s\n", __FUNCTION__, mappingPathName);

	bool running = true;
	IID_INIT(objIid);
	while(running && (zcfgFeObjJsonGetNext(RDM_OID_LAN_DEV, &objIid, &tr181LanDevJobj) == ZCFG_SUCCESS)){
		char *linkIfaceStr = json_object_get_string(json_object_object_get(tr181LanDevJobj, "LinkInterface"));
		if(linkIfaceStr && strstr(linkIfaceStr, mappingPathName)){
			hostNumOfEntries = json_object_get_int(json_object_object_get(tr181LanDevJobj, "HostNumberOfEntries"));
			running = false;
		}
		json_object_put(tr181LanDevJobj);
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
#ifdef ZYXEL_ELIMINATE_DHCP_LEASE
		if(!strcmp(paramList->name, "X_ZYXEL_DeleteAllLease")) {
			//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramValue = hostsObj ? json_object_object_get(hostsObj, paramList->name) : NULL;
			json_object_object_add(*tr98Jobj, paramList->name, paramValue ? JSON_OBJ_COPY(paramValue) : json_object_new_boolean(false));
			paramList++;
			continue;
		}else
#endif
#ifdef ABUU_CUSTOMIZATION
		if(!strcmp(paramList->name, "X_ZYXEL_CurSessionCount")) {
			if (natSessionSTAJobj){
				paramValue = json_object_object_get(natSessionSTAJobj, "ActiveSession");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}else
#endif
		if(!strcmp(paramList->name, "HostNumberOfEntries")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(hostNumOfEntries));
			paramList++;
			continue;
		}
		
		/*Write parameter value from tr181 objects to tr98 object*/ 	
		paramValue = json_object_object_get(hostsObj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(hostsObj);
#ifdef ABUU_CUSTOMIZATION
	json_object_put(natSessionSTAJobj);
#endif
#else
	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".Hosts");
	*ptr = '\0';

	/* mappingPathName will be Ethernet.Link.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS) {
		/*No such object*/
		printf("No such object\n");
		return ret;
	}

	strcpy(tr181Name, mappingPathName);
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(tr181Name, higherLayerPath)) != ZCFG_SUCCESS)
		return ret;

	if(strstr(higherLayerPath, "IP.Interface") != NULL) {
		IID_INIT(objIid);
		objIid.level = 1;

		while(feObjStructGetNext(RDM_OID_HOSTS_HOST, &objIid, (void **)&stHost, updateFlag) == ZCFG_SUCCESS){
			if(strcmp(stHost->Layer3Interface, higherLayerPath) == 0){
				hostNum++;
			}
			zcfgFeObjStructFree(stHost);
		}
	}

	*tr98Jobj = json_object_new_object();
	json_object_object_add(*tr98Jobj, "HostNumberOfEntries", json_object_new_int(hostNum));
#endif
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t lanDevHostsObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanDevHostsObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

#ifdef ABUU_CUSTOMIZATION
		if(!strcmp(paramList->name, "X_ZYXEL_CurSessionCount")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_SESSION_CTL_ST, "ActiveSession");
		}
		else
#endif
		attrValue = zcfgFeParamAttrGetByName(RDM_OID_HOSTS, paramList->name);

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

zcfgRet_t lanDevHostsObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	uint32_t oid = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.Hosts.Host.i

    Related object in TR181:
    Device.Hosts.Host.i
    Device.Hosts.Host.i.IPv4Address.i
    Device.Hosts.Host.i.IPv6Address.i
*/
zcfgRet_t lanDevHostObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
#ifdef SAAAE_TR69_NO_LAN_HOST
	FILE *arpfp = NULL;
	char line[512]={0};
	int type, flags;
	char ip[32]={0};
	char hwa[65]={0};
	char mask[32]={0};
	char dev[32]={0};
	int num = 0;
	int idx = 0;
	uint32_t hostNum = 0;
	hostlist_t *hostlist = NULL;
	hostlist_t *theHostlist = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s()\n", __FUNCTION__);

	//Refresh Hosts device list
	lanDevHostListUpdate(&hostlist, 1);

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%d", &idx);

	/*ARP table*/
	zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, ARPFILE);
	arpfp = fopen(ARPFILE, "r");
	if(arpfp == NULL){
		zcfgLog(ZCFG_LOG_ERR, "%s(): open %s fail\n", __FUNCTION__, ARPFILE);
		return ZCFG_SUCCESS;
	}
	fgets(line, sizeof(line), arpfp);
	while (fgets(line, sizeof(line), arpfp)) {
		mask[0] = '-'; mask[1] = '\0';
		dev[0] = '-'; dev[1] = '\0';
		/* All these strings can't overflow
		 * because fgets above reads limited amount of data */
		num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
					 ip, &type, &flags, hwa, mask, dev);
		if (num < 4)
			break;

		/*incomplete, skip*/
		if(flags == 0){
			continue;
		}

		if(strcmp(hwa,"00:00:00:00:00:00")==0){
			continue;
		}

		// all LAN dev should be br+ ,no eth0, eth1, eth2...
		if(strstr(dev, "br") == NULL)
			continue;

		hostNum++;
		if(idx == hostNum) {
			theHostlist = findHostFromList(hostlist, hwa);

			/*fill up tr98 Host object*/
			*tr98Jobj = json_object_new_object();
			json_object_object_add(*tr98Jobj, "MACAddress", json_object_new_string(hwa));
			json_object_object_add(*tr98Jobj, "IPAddress", json_object_new_string(ip));
			json_object_object_add(*tr98Jobj, "ClientID", json_object_new_string(""));
			json_object_object_add(*tr98Jobj, "UserClassID", json_object_new_string(""));

			if(theHostlist){
				json_object_object_add(*tr98Jobj, "HostName", json_object_new_string(theHostlist->hostinfo.hostname));
				json_object_object_add(*tr98Jobj, "VendorClassID", json_object_new_string(theHostlist->hostinfo.vid));

				if(theHostlist->hostinfo.tag & TAG_DHCP){
					json_object_object_add(*tr98Jobj, "AddressSource", json_object_new_string(ADDRSRC_DHCP));
					json_object_object_add(*tr98Jobj, "LeaseTimeRemaining", json_object_new_int(theHostlist->hostinfo.lease));
				}else{
					json_object_object_add(*tr98Jobj, "AddressSource", json_object_new_string(ADDRSRC_STATIC));
					json_object_object_add(*tr98Jobj, "LeaseTimeRemaining", json_object_new_int(0));
				}

				if(theHostlist->hostinfo.tag & TAG_WIFI){
					json_object_object_add(*tr98Jobj, "InterfaceType", json_object_new_string(CONNTYPE_WIRELESS));
				}else if(type ==1/*ether*/ || type==2/*ppp*/){
					json_object_object_add(*tr98Jobj, "InterfaceType", json_object_new_string(CONNTYPE_ETH));
				}
				else{
					json_object_object_add(*tr98Jobj, "InterfaceType", json_object_new_string(CONNTYPE_OTHER));
				}

				if(theHostlist->hostinfo.tag & TAG_LOSTWIFI)
					json_object_object_add(*tr98Jobj, "Active", json_object_new_boolean(0));
				else
					json_object_object_add(*tr98Jobj, "Active", json_object_new_boolean(1));
			}else{
				json_object_object_add(*tr98Jobj, "HostName", json_object_new_string(""));
				json_object_object_add(*tr98Jobj, "VendorClassID", json_object_new_string(""));
				json_object_object_add(*tr98Jobj, "AddressSource", json_object_new_string(ADDRSRC_STATIC));
				json_object_object_add(*tr98Jobj, "LeaseTimeRemaining", json_object_new_int(0));
				json_object_object_add(*tr98Jobj, "Active", json_object_new_boolean(1));

				if(type ==1/*ether*/ || type==2/*ppp*/){
					json_object_object_add(*tr98Jobj, "InterfaceType", json_object_new_string(CONNTYPE_ETH));
				}
				else{
					json_object_object_add(*tr98Jobj, "InterfaceType", json_object_new_string(CONNTYPE_OTHER));
				}
			}

			break;
		}
	}
	fclose(arpfp);

	freeHostList(hostlist);
#else
	zcfgRet_t ret;
	objIndex_t objIid;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	struct json_object *hostJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *iconJobj;
	const char *MacAddress = NULL, *PhysAddress = NULL;
	char frequencyBand[8] = {0};
	bool find = false;
#if MODIFY_HOSTS_AND_HOST_FUN // modify 20150809
#else
	objIndex_t hostIid;
	char tr181Name[32] = {0};
	char tr98TmpName[128] = {0};
	char higherLayerPath[32] = {0};
	char mappingPathName[32] = {0};
	int idx = 0;
	uint32_t hostNum = 0;
	rdm_HostsHost_t *stHost = NULL;
#endif

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

#ifdef ZYXEL_HOMEPLUG_DEVICE_DISCOVER
		objIndex_t hosts_objIid;
		struct json_object *hostsObj = NULL;
		static int flag = 0;

		IID_INIT(hosts_objIid);
		if(!flag){
			if((ret = feObjJsonGet(RDM_OID_HOSTS, &hosts_objIid, &hostsObj, updateFlag)) != ZCFG_SUCCESS){
				return ret;
			}
			flag = 1;
		}
#endif


#if MODIFY_HOSTS_AND_HOST_FUN // modify 20150809
	char tr181Name[100] = {0};

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	//sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%hhu", &objIid.idx[0]);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, tr181Name) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	sscanf(tr181Name, "Hosts.Host.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_HOSTS_HOST, &objIid, &hostJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}
#else
	//strcpy(tr98TmpName, tr98FullPathName);
	//ptr = strstr(tr98TmpName, ".Hosts");
	//*ptr = '\0';
#if 0
	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		return ret;
	}

	strcpy(tr181Name, mappingPathName);
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(tr181Name, higherLayerPath)) != ZCFG_SUCCESS)
		return ret;

	if(strstr(higherLayerPath, "IP.Interface") != NULL) {
		IID_INIT(objIid);
		IID_INIT(hostIid);
		objIid.level = 1;
		sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%d", &idx);

		while(feObjStructGetNext(RDM_OID_HOSTS_HOST, &objIid, (void **)&stHost, updateFlag) == ZCFG_SUCCESS){
			if(strcmp(stHost->Layer3Interface, higherLayerPath) == 0){
				hostNum++;
				if(idx == hostNum) {
					hostIid = objIid;
					zcfgFeObjStructFree(stHost);
					break;
				}
			}
			zcfgFeObjStructFree(stHost);
		}
	}
	else
		return;
#endif
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, tr181Name) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	IID_INIT(hostIid);
	hostIid.level = 1;
	sscanf(tr181Name, "Hosts.Host.%hhu", &hostIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_HOSTS_HOST, &hostIid, &hostJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#endif

	/*fill up tr98 Host object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		if(!strcmp(paramList->name, "X_ZYXEL_SignalStrength")) {
			paramValue = json_object_object_get(hostJobj, "X_ZYXEL_RSSI");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

		paramValue = json_object_object_get(hostJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "Layer2Interface")) {
			const char *iface = json_object_get_string(json_object_object_get(hostJobj, "Layer1Interface"));
			if(iface && strlen(iface)) {
				char tr181HostIface[30] = {0};
				char tr98MappingIface[160] = {0};

				ZOS_STRNCPY(tr181HostIface, iface, sizeof(tr181HostIface));
				printf("%s: Layer1Interface: %s\n", __FUNCTION__, tr181HostIface);
				if(zcfgFe181To98MappingNameGet(tr181HostIface, tr98MappingIface) == ZCFG_SUCCESS){
					printf("%s: Layer2Interface: %s\n", __FUNCTION__, tr98MappingIface);
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98MappingIface));
				}
			}
 			paramList++;
 			continue;
		}
		else if(!strcmp(paramList->name, "MACAddress")) {
			paramValue = json_object_object_get(hostJobj, "PhysAddress");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "InterfaceType")) {
			paramValue = json_object_object_get(hostJobj, "X_ZYXEL_ConnectionType");
			const char *conntection = json_object_get_string(json_object_object_get(hostJobj, "X_ZYXEL_ConnectionType"));
			if (strstr(conntection, "Wi-Fi") != NULL){
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("802.11"));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingFrequencyBand")) {
			paramValue = json_object_object_get(hostJobj, "X_ZYXEL_ConnectionType");
			const char *band = json_object_get_string(json_object_object_get(hostJobj, "X_ZYXEL_ConnectionType"));
			if (strstr(band, "Wi-Fi") != NULL){
				sscanf(band, "Wi-Fi %s", frequencyBand);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(frequencyBand));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Internet_Lock_Enable")) {
			find = false;
			IID_INIT(objIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconJobj) == ZCFG_SUCCESS){
				PhysAddress = json_object_get_string(json_object_object_get(hostJobj, "PhysAddress"));
				MacAddress = json_object_get_string(json_object_object_get(iconJobj, "MacAddress"));
				if(MacAddress != NULL && PhysAddress != NULL && isSameMac(MacAddress, PhysAddress)){
					find = true;
					break;
				}
				json_object_put(iconJobj);
			}

			if (find == true){
				paramValue = json_object_object_get(iconJobj, "Internet_Blocking_Enable");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, "X_ZYXEL_Internet_Lock_Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					json_object_put(iconJobj);
					continue;	
				}
				json_object_put(iconJobj);
			}
		}
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
		else if(!strcmp(paramList->name, "X_ZYXEL_PriorityTraffic_Enable")) {
			find = false;
			IID_INIT(objIid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &objIid, &iconJobj) == ZCFG_SUCCESS){
				PhysAddress = json_object_get_string(json_object_object_get(hostJobj, "PhysAddress"));
				MacAddress = json_object_get_string(json_object_object_get(iconJobj, "MacAddress"));
				if(MacAddress != NULL && PhysAddress != NULL && isSameMac(MacAddress, PhysAddress)){
					find = true;
					break;
				}
				json_object_put(iconJobj);
			}
			if (find == true){
				paramValue = json_object_object_get(iconJobj, "Internet_Priority_Enable");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, "X_ZYXEL_PriorityTraffic_Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					json_object_put(iconJobj);
					continue;	
				}
				json_object_put(iconJobj);
			}

		}
#endif //ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(hostJobj);
#endif

	return ZCFG_SUCCESS;
}

#ifdef ZYXEL_ELIMINATE_DHCP_LEASE
zcfgRet_t lanDevHostsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *hostsObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_HOSTS, &objIid, &hostsObj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = hostsObj;
		hostsObj = NULL;
		hostsObj = zcfgFeJsonMultiObjAppend(RDM_OID_HOSTS, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/	
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(!strcmp(paramList->name, "X_ZYXEL_DeleteAllLease")) {
				objIndex_t hostIid;
				struct json_object *hostObj = NULL;
				IID_INIT(hostIid);
				while((ret = zcfgFeObjJsonGetNext(RDM_OID_HOSTS_HOST, &hostIid, &hostObj)) == ZCFG_SUCCESS) 
				{
					paramValue = json_object_object_get(tr98Jobj, paramList->name);
					//printf("X_ZYXEL_Global_IPv6Enable=%d\n",json_object_get_int(paramValue));
					json_object_object_add(hostObj, "X_ZYXEL_DeleteLease", JSON_OBJ_COPY(paramValue));
					ret = zcfgFeObjJsonSet(RDM_OID_HOSTS_HOST, &hostIid, hostObj, NULL);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						json_object_put(hostObj);
						return ret;
					}
					json_object_put(hostObj);
				}
			}
			
			tr181ParamValue = json_object_object_get(hostsObj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(hostsObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_HOSTS, &objIid, hostsObj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(hostsObj);
			return ret;
		}
		json_object_put(hostsObj);
	}
	
	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t lanDevHostObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ethIntfOid = 0;
	objIndex_t objIid;
	objIndex_t iconObjIid;
	struct json_object *hostJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *iconJobj = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *MacAddress = NULL, *PhysAddress = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(objIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ethIntfOid = zcfgFeObjNameToObjId(tr181Obj, &objIid);
	if((ret = zcfgFeObjJsonGet(ethIntfOid, &objIid, &hostJobj)) != ZCFG_SUCCESS)
	{
		return ret;
	}

	IID_INIT(iconObjIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &iconObjIid, &iconJobj) == ZCFG_SUCCESS){
		PhysAddress = json_object_get_string(json_object_object_get(hostJobj, "PhysAddress"));
		MacAddress = json_object_get_string(json_object_object_get(iconJobj, "MacAddress"));
		if(MacAddress != NULL && PhysAddress != NULL && isSameMac(MacAddress, PhysAddress)){
			break;
		}else	
		json_object_put(iconJobj);
	}

	if(multiJobj){
		tmpObj = hostJobj;
		hostJobj = NULL;
		hostJobj = zcfgFeJsonMultiObjAppend(RDM_OID_HOSTS_HOST, &objIid, multiJobj, tmpObj);
		if(iconJobj){
			json_object_put(tmpObj);
			tmpObj = iconJobj;
			iconJobj = NULL;
			iconJobj = zcfgFeJsonMultiObjAppend(RDM_OID_CHANGE_ICON_NAME, &iconObjIid, multiJobj, tmpObj);
		}
	
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.Hosts.Host.i */
			tr181ParamValue = json_object_object_get(hostJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(hostJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}		
			/*special case*/
			if(!strcmp(paramList->name, "X_ZYXEL_Internet_Lock_Enable")){
				tr181ParamValue = json_object_object_get(iconJobj, "Internet_Blocking_Enable");
				if(tr181ParamValue != NULL) {
					json_object_object_add(iconJobj, "Internet_Blocking_Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
			if(!strcmp(paramList->name, "X_ZYXEL_PriorityTraffic_Enable")){
				tr181ParamValue = json_object_object_get(iconJobj, "Internet_Priority_Enable");
				if(tr181ParamValue != NULL) {
					json_object_object_add(iconJobj, "Internet_Priority_Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#endif //ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
		}
		paramList++;	
	} /*Edn while*/

	/* Device.Hosts.Host.i */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_HOSTS_HOST, &objIid, hostJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Hosts.Host.i Fail\n", __FUNCTION__);
			json_object_put(hostJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Hosts.Host.i Success\n", __FUNCTION__);
		}
		json_object_put(hostJobj);

		if((ret = zcfgFeObjJsonSet(RDM_OID_CHANGE_ICON_NAME, &iconObjIid, iconJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.X_ZYXEL_Change_Icon_Name.i Fail\n", __FUNCTION__);
			json_object_put(iconJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.X_ZYXEL_Change_Icon_Name.i Success\n", __FUNCTION__);
		}
		json_object_put(iconJobj);
	}

	return ZCFG_SUCCESS;

}

zcfgRet_t lanDevHostObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanDevHostObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		attrValue = zcfgFeParamAttrGetByName(RDM_OID_HOSTS_HOST, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t lanDevHostObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	uint32_t oid = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*  TR98 Object Name : InternetGatewayDevice.LANInterfaces

    Related object in TR181:
	Device.WiFi.
	Device.USB.
	Device.Ethernet.Interface.i.
*/
zcfgRet_t lanIfacesObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	objIndex_t ethIntfIid;
	int numberOfEthIntf = 0;
	rdm_EthIface_t *ethIntfObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	objIndex_t objIid;
	int numberOfUsbIntf = 0;
	rdm_UsbIntf_t *usbIntfObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	int numberOfWlanIntf = 0;
	struct json_object *wifiJObj = NULL;
	objIndex_t wifiObjIid;
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	IID_INIT(wifiObjIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI, &wifiObjIid, &wifiJObj) == ZCFG_SUCCESS){
		numberOfWlanIntf = json_object_get_int(json_object_object_get(wifiJObj, "AccessPointNumberOfEntries"));
		json_object_put(wifiJObj);
	}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_USB
	IID_INIT(objIid);
	while(feObjStructGetNext(RDM_OID_USB_INTF, &objIid, (void **)&usbIntfObj, updateFlag) == ZCFG_SUCCESS){
		if(!usbIntfObj->Upstream){
			numberOfUsbIntf++;
		}
		zcfgFeObjStructFree(usbIntfObj);
	}
#endif

	IID_INIT(ethIntfIid);
	while(feObjStructGetNext(RDM_OID_ETH_IFACE, &ethIntfIid, (void **)&ethIntfObj, updateFlag) == ZCFG_SUCCESS){
		if(!ethIntfObj->Upstream){
			numberOfEthIntf++;
		}
		zcfgFeObjStructFree(ethIntfObj);
	}

	*tr98Jobj = json_object_new_object();
	json_object_object_add(*tr98Jobj, "LANEthernetInterfaceNumberOfEntries", json_object_new_int(numberOfEthIntf));
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	json_object_object_add(*tr98Jobj, "LANUSBInterfaceNumberOfEntries", json_object_new_int(numberOfUsbIntf));
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	json_object_object_add(*tr98Jobj, "LANWLANConfigurationNumberOfEntries", json_object_new_int(numberOfWlanIntf));
#endif

	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i

    Related object in TR181:
    Device.Ethernet.Link.i
*/
zcfgRet_t lanDevObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid, ethLinkIid;
	char *tmp = NULL, *token = NULL;
	char mappingPathName[128] = {0};
	//char ethIntfPathName[32] = {0};
	char lowerlayers[1025] = {0};
	rdm_EthLink_t *ethLink = NULL;
	rdm_BridgingBrPort_t *brPortObj = NULL;
	int numberOfEthIntf = 0;
	int numberOfWlanIntf = 0;
	int numberOfUsbIntf = 0;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	/* mappingPathName will be Ethernet.Link.i */
	//printf("TR181 object %s\n", mappingPathName);
#if 0
	IID_INIT(objIid);
	IID_INIT(ethIntfIid);
	while(feObjStructGetNext(RDM_OID_ETH_IFACE, &ethIntfIid, (void **)&ethIntfObj, updateFlag) == ZCFG_SUCCESS){
		sprintf(ethIntfPathName, "Ethernet.Interface.%d", ethIntfIid.idx[0]);	
		if(isLowestLayerIntf(ethIntfPathName, mappingPathName)){
			if(!ethIntfObj->Upstream)
				numberOfEthIntf++;
		}
		zcfgFeObjStructFree(ethIntfObj);
	}
	
	while(feObjStructGetNext(RDM_OID_IFACE_STACK, &objIid, (void **)&ifaceStack, updateFlag) == ZCFG_SUCCESS){
		/*scan of wireless lan interface number*/
		if(strstr(ifaceStack->HigherLayer, "Bridging.Bridge") != NULL){
			if (strstr(ifaceStack->LowerLayer, "WiFi.SSID") != NULL)
				numberOfWlanIntf++;
		}
		
		/*scan of usb interface number*/	
		zcfgFeObjStructFree(ifaceStack);
	}
#else
	IID_INIT(ethLinkIid);
	ethLinkIid.level = 1;
	sscanf(mappingPathName, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);

	if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_ETH_LINK, &ethLinkIid, (void **)&ethLink)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get Ethernet Link Fail\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ethLink->LowerLayers, "Bridging.Bridge.%hhu.Port.%hhu", &objIid.idx[0], &objIid.idx[1]);

	if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &objIid, (void **)&brPortObj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get Bridge Port Fail\n", __FUNCTION__);

		*tr98Jobj = json_object_new_object();
		json_object_object_add(*tr98Jobj, "LANEthernetInterfaceNumberOfEntries", json_object_new_int(0));
		json_object_object_add(*tr98Jobj, "LANUSBInterfaceNumberOfEntries", json_object_new_int(0));
		json_object_object_add(*tr98Jobj, "LANWLANConfigurationNumberOfEntries", json_object_new_int(0));

		zcfgFeObjStructFree(ethLink);
		return ZCFG_SUCCESS;
	}

	strcpy(lowerlayers, brPortObj->LowerLayers);
	zcfgFeObjStructFree(brPortObj);

	token = strtok_r(lowerlayers, ",", &tmp);
	while(token != NULL) {
		objIid.idx[0] = 0;
		objIid.idx[1] = 0;
		sscanf(token, "Bridging.Bridge.%hhu.Port.%hhu", &objIid.idx[0], &objIid.idx[1]);

		if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &objIid, (void **)&brPortObj) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get Bridge Port Fail\n", __FUNCTION__);
		}
		else {
			if(strstr(brPortObj->LowerLayers, "WiFi.SSID") != NULL) {
				numberOfWlanIntf++;
			}
			else if(strstr(brPortObj->LowerLayers, "Ethernet.Interface") != NULL) {
				numberOfEthIntf++;
			}
			else if(strstr(brPortObj->LowerLayers, "USB.Interface") != NULL) {
				numberOfUsbIntf++;
			}

			zcfgFeObjStructFree(brPortObj);
		}

		token = strtok_r(NULL, ",", &tmp);
	}
	zcfgFeObjStructFree(ethLink);
#endif

	/*fill up tr98 LANDevice object*/
	*tr98Jobj = json_object_new_object();
	json_object_object_add(*tr98Jobj, "LANEthernetInterfaceNumberOfEntries", json_object_new_int(numberOfEthIntf));
	json_object_object_add(*tr98Jobj, "LANUSBInterfaceNumberOfEntries", json_object_new_int(numberOfUsbIntf));
	json_object_object_add(*tr98Jobj, "LANWLANConfigurationNumberOfEntries", json_object_new_int(numberOfWlanIntf));

	return ZCFG_SUCCESS;
}

// InternetGatewayDevice.LANDevice.
zcfgRet_t lanDevObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	rdm_EthLink_t *ethLinkObj = NULL;
#if 0
	char ethLinkPathName[32] = {0};
	char lowerLayerPathName[32] = {0};
	int numOfBridge =0;
		
	numOfBridge = getNumOfBridge();	
	sprintf(lowerLayerPathName, "Bridging.Bridge.%d.Port.1", numOfBridge);

	/*add Device.Ethernet.Link.i*/
	if((ret = ethLinkAdd(ethLinkPathName, lowerLayerPathName)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	*idx = numOfBridge;
#endif


	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS){
		return ret;
	}

	if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS){
		ethLinkObj->Enable= false;
		if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS){
			printf("Set Ethernet.Link LowerLayers Fail\n");
		}
		zcfgFeObjStructFree(ethLinkObj);
	}
	else
		return ret;

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[0];
#else
	printf("%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		if(zcfg98To181ObjMappingAdd(e_TR98_LAN_DEV, &tr98ObjIid, RDM_OID_ETH_LINK, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added LANDevice %hhu\n", __FUNCTION__, tr98ObjIid.idx[0]);
			*idx = tr98ObjIid.idx[0];
		}
	}
#endif
	
	return ret;
}

zcfgRet_t lanDevObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[32] = {0};
	uint32_t  oid = 0;
	objIndex_t objIid;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS) {
		return ret;
	}
		
	IID_INIT(objIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);

	ret = zcfgFeObjJsonDel(oid, &objIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		printf("Delete Object Success\n");
	}
	else {
		printf("Delete Object Fail\n");	
	}

	return ret;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement

    Related object in TR181:
    Device.Ethernet.Link.i
    Device.DHCPv4.Server
    Device.DHCPv4.Relay
    Device.DHCPv4.Server.Pool.i
    Device.IP.Interface.i
*/
zcfgRet_t lanHostConfMgmtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[128] = {0};
	char ipIntfPathName[128] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint32_t  ethLinkOid, ipIntfOid= 0;
	uint32_t  ipIntfNum = 0;
	objIndex_t ethLinkIid, ipIntfIid, dhcpServPoolIid, dhcpRelayIid, objIid;
	struct json_object *ethLinkJobj = NULL;
	struct json_object *dhcpServPoolJobj = NULL;
	struct json_object *macAddr = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	rdm_IpIface_t *ipIntfObj = NULL;
	rdm_Dhcpv4SrvPool_t *dhcpServPoolObj = NULL;
	rdm_Dhcpv4Relay_t *dhcpRelayObj = NULL;
	rdm_Dhcpv4RelayFwd_t *dhcpRelayFwdObj = NULL;
	char paramName[64] = {0};
	bool dhcpRelay = false;
	uint8_t dhcpServPoolNum = 0;
	uint8_t flag = 0;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	printf("tr98TmpName %s\n", tr98TmpName);
	
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("%s: 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	else{
       	/* mappingPathName will be Ethernet.Link.i */
		printf("%s: TR181 object %s\n", __FUNCTION__, mappingPathName);
	}

	/*to get value of parameter "MACAddress" of tr98 LANHostConfigManagement object*/
	IID_INIT(ethLinkIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ethLinkOid = zcfgFeObjNameToObjId(tr181Obj, &ethLinkIid);
	if((ret = feObjJsonGet(ethLinkOid, &ethLinkIid, &ethLinkJobj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s : No such object '.Ethernet.Link.i'\n", __FUNCTION__);
		return ret;
	}
	else	
		macAddr = json_object_object_get(ethLinkJobj, "MACAddress");
		
	IID_INIT(ipIntfIid);
	while(feObjStructGetNext(RDM_OID_IP_IFACE, &ipIntfIid, (void **)&ipIntfObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS){
		if (strcmp(ipIntfObj->LowerLayers, mappingPathName) == 0){
			sprintf(ipIntfPathName, "IP.Interface.%d", ipIntfIid.idx[0]);
			ipIntfNum++;
			zcfgFeObjStructFree(ipIntfObj);
			break;
		}
		else
			zcfgFeObjStructFree(ipIntfObj);
	}

	/*to get value of parameter "DHCPRelay" of tr98 LANHostConfigManagement object*/
	IID_INIT(dhcpRelayIid);
	if((ret = feObjStructGet(RDM_OID_DHCPV4_RELAY, &dhcpRelayIid, (void **)&dhcpRelayObj, updateFlag)) == ZCFG_SUCCESS){
		if(dhcpRelayObj->Enable == true){
			if((ret = feObjStructGetNext(RDM_OID_DHCPV4_RELAY_FWD, &dhcpRelayIid, (void **)&dhcpRelayFwdObj, updateFlag)) != ZCFG_SUCCESS){
				zcfgLog(ZCFG_LOG_ERR, "%s : No such object 'DHCPv4.Relay.Forwarding.i'\n", __FUNCTION__);
				json_object_put(ethLinkJobj);
				return ret;
			}
			else{
				if (strcmp(dhcpRelayFwdObj->Interface, ipIntfPathName)==0){
					dhcpRelay = dhcpRelayFwdObj->Enable;		
				}
				else
					dhcpRelay = false;
				zcfgFeObjStructFree(dhcpRelayFwdObj);
			}	
		}
		
		zcfgFeObjStructFree(dhcpRelayObj);
	}

	/*to get value of parameter "DHCPConditionalPoolNumberOfEntries" of tr98 LANHostConfigManagement object*/
	IID_INIT(objIid);
	IID_INIT(ipIntfIid);
	IID_INIT(dhcpServPoolIid);
	while(feObjStructGetNext(RDM_OID_DHCPV4_SRV_POOL, &objIid, (void **)&dhcpServPoolObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS){
		sprintf(tr181Obj, "Device.%s", dhcpServPoolObj->Interface);
		ipIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ipIntfIid);
		if((ret = feObjStructGet(RDM_OID_IP_IFACE, &ipIntfIid, (void **)&ipIntfObj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) {
			if(strcmp(ipIntfObj->LowerLayers, mappingPathName)==0){
				if(flag == 0){
					dhcpServPoolNum++;
					memcpy(&dhcpServPoolIid, &objIid, sizeof(objIndex_t));
					flag = 1;
				}
				else
					dhcpServPoolNum++;				
			}
			zcfgFeObjStructFree(ipIntfObj);
		}		
		zcfgFeObjStructFree(dhcpServPoolObj);
	}

	if(flag && (ret = feObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, &dhcpServPoolJobj, updateFlag)) != ZCFG_SUCCESS) {
		json_object_put(ethLinkJobj);
		return ret;
	}

	/*fill up tr98 LANHostConfigManagement object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		/*special case*/
		if(!strcmp(paramList->name, "DNSServers")){
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramList->name) : NULL;
			if(paramValue != NULL) {
				char *tmp = NULL;
				tmp = json_object_get_string(paramValue);
				if(strchr(tmp,',') == NULL){
					char DNSServer[36] = {0};
					sprintf(DNSServer,"%s,0.0.0.0",tmp);
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(DNSServer));
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}
#endif
		paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramList->name) : NULL;
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(strstr(paramList->name, "LeaseTime") != NULL){
			sprintf(paramName, "%s", "LeaseTime");
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramName) : NULL;
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}
		
		if(strstr(paramList->name, "Enable") != NULL){
			sprintf(paramName, "%s", "Enable");
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramName) : NULL;
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}

		if(strstr(paramList->name, "StaticAddressNumberOfEntries") != NULL){
			sprintf(paramName, "%s", "StaticAddressNumberOfEntries");
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramName) : NULL;
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}

		if(strstr(paramList->name, "OptionNumberOfEntries") != NULL){
			sprintf(paramName, "%s", "OptionNumberOfEntries");
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramName) : NULL;
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}
			
		if(strstr(paramList->name, "DHCPServerConfigurable") != NULL){
			sprintf(paramName, "%s", "X_ZYXEL_ServerConfigurable");
			paramValue = (dhcpServPoolJobj) ? json_object_object_get(dhcpServPoolJobj, paramName) : NULL;
			if(paramValue) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
		paramList++;
	}

	/*fill up the rest parameters*/
	json_object_object_add(*tr98Jobj, "MACAddress", JSON_OBJ_COPY(macAddr));
    json_object_object_add(*tr98Jobj, "IPInterfaceNumberOfEntries", json_object_new_int(ipIntfNum));
#ifndef ZYXEL_OPAL_EXTENDER
	json_object_object_add(*tr98Jobj, "DHCPServerConfigurable", json_object_new_boolean(true));
	json_object_object_add(*tr98Jobj, "DHCPConditionalPoolNumberOfEntries", json_object_new_int(dhcpServPoolNum-1));
	if(dhcpRelay == true)
		json_object_object_add(*tr98Jobj, "DHCPRelay", json_object_new_boolean(true));
	else
		json_object_object_add(*tr98Jobj, "DHCPRelay", json_object_new_boolean(false));
#endif
	
	json_object_put(ethLinkJobj);
	json_object_put(dhcpServPoolJobj);
	return ZCFG_SUCCESS;	
	
}

zcfgRet_t lanHostConfMgmtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char ipIntfPathName[128] = {0};
	char higherLayerPathName[24] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t dhcpServPoolIid;
	struct json_object *dhcpServPoolJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	rdm_Dhcpv4SrvPool_t *dhcpServPoolObj = NULL;
	int flag = 0;


	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	printf("tr98TmpName %s\n", tr98TmpName);
	
       if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
       }
	else{
       	/* mappingPathName will be Ethernet.Link.i */
		printf("TR181 object %s\n", mappingPathName);
       }

	if(zcfgFeTr181IfaceStackHigherLayerGet(mappingPathName, higherLayerPathName) == ZCFG_SUCCESS)
		sprintf(ipIntfPathName, "%s", higherLayerPathName);
	
	IID_INIT(dhcpServPoolIid);
	while(zcfgFeObjStructGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, (void **)&dhcpServPoolObj) == ZCFG_SUCCESS){
		if (strcmp(dhcpServPoolObj->Interface, ipIntfPathName) == 0){
			flag++;
			zcfgFeObjStructFree(dhcpServPoolObj);
			break;
		}
		else
			zcfgFeObjStructFree(dhcpServPoolObj);
	}	

	if(flag == 0){
		zcfgLog(ZCFG_LOG_ERR, "%s : No such object 'DHCPv4.Server.Pool.i'\n", __FUNCTION__);
		return  ZCFG_INTERNAL_ERROR;
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, &dhcpServPoolJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = dhcpServPoolJobj;
		dhcpServPoolJobj = NULL;
		dhcpServPoolJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, multiJobj, tmpObj);
	}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	struct json_object *tr98SpvObj = NULL;
	tr98SpvObj = zcfgFeRetrieveSpv(tr98FullPathName);

    const char *tr98DnsType = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_DNS_Type"));
    if(json_object_object_get(tr98SpvObj, "DNSServers") || (tr98DnsType && !strcmp(tr98DnsType, TR181_DNS_TYPE_STATIC)))
    {
		json_object_object_add(dhcpServPoolJobj, "X_ZYXEL_DNS_Type", json_object_new_string("Static"));
	}
#endif

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from Config to Device.DHCPv4.Server.Pool.i*/
			tr181ParamValue = json_object_object_get(dhcpServPoolJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(dhcpServPoolJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			
			/*special case*/
			if(strstr(paramList->name, "Enable") != NULL){
				json_object_object_add(dhcpServPoolJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			
			if(strstr(paramList->name, "LeaseTime") != NULL){
				json_object_object_add(dhcpServPoolJobj, "LeaseTime", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			if(strstr(paramList->name, "DHCPServerConfigurable") != NULL){
				json_object_object_add(dhcpServPoolJobj, "X_ZYXEL_ServerConfigurable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

		}

		paramList++;	
	} /*Edn while*/

	/*Set Device.DHCPv4.Server.Pool.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, dhcpServPoolJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Server.Pool.i Fail\n", __FUNCTION__);
			json_object_put(dhcpServPoolJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DHCPv4.Server.Pool.i Success\n", __FUNCTION__);
		}
		json_object_put(dhcpServPoolJobj);
	}
	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i

    Related object in TR181:
    Device.IP.Interface.i
    Device.IP.Interface.i.IPv4Address.i
*/
zcfgRet_t lanIpIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ipIntfOid = 0;
	objIndex_t ipIntfIid, ipIntfv4AddrIid, igmpIid;
	struct json_object *ipIntfJobj = NULL;
	struct json_object *ipIntfv4AddrJobj = NULL;
	struct json_object *igmpJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char paramName[64] = {0};
#if 0
	objIndex_t dnsSrvIid;
	struct json_object *dnsSrvJobj = NULL;
#endif
	//char typeBuf[20] = {0};
	char ifName[16] = {0};
	char snoopingBrIfName[128] = {0};
	char *tmp_ptr = NULL, *tmp_ptr2 = NULL, *intf = NULL;
	int SnoopingMode = 0; //0 : snooping disable, 1 : standard mode, 2 : blocking mode.

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
       	/* mappingPathName will be IP.Interface.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(ipIntfIid);
	IID_INIT(ipIntfv4AddrIid);
	IID_INIT(igmpIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ipIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ipIntfIid);
	if((ret = feObjJsonGet(ipIntfOid, &ipIntfIid, &ipIntfJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	if((ret = feObjJsonSubNext(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfIid, &ipIntfv4AddrIid, &ipIntfv4AddrJobj, updateFlag)) != ZCFG_SUCCESS){
		json_object_put(ipIntfJobj);
		return ret;
	}

#if 0
	/*DNSServer*/
	IID_INIT(dnsSrvIid);
	while((ret = feObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, &dnsSrvJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(dnsSrvJobj, "Interface")));
		strcpy(typeBuf, json_object_get_string(json_object_object_get(dnsSrvJobj, "Type")));
		if(!strcmp(mappingPathName, ifBuf) && !strcmp("DHCPv6", typeBuf))
			break;
		else {
			json_object_put(dnsSrvJobj);
			dnsSrvJobj = NULL;
		}
	}
#endif

	/*fill up tr98 LANHostConfigManagement object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef ZYXEL_OPAL_EXTENDER
        if(paramList->attr & PARAMETER_ATTR_EXTENDER_HIDDEN)
        {
            paramList++;
            continue;
        }
#endif
		paramValue = json_object_object_get(ipIntfJobj, paramList->name);
		if(paramValue != NULL) {
			if(strstr(paramList->name, "X_ZYXEL_Group_WAN_IpIface") != NULL) {
#ifdef IAAAA_CUSTOMIZATION
				struct json_object *wanGroup = NULL;
				wanGroup = json_object_object_get(ipIntfJobj, "X_ZYXEL_Group_WAN_IpIface");
				if(wanGroup) {
					char wanGroupIf[256] = {0}, wanGroupIfName[256] = {0};
					char *pc1 = NULL, *pc2 = NULL;
					objIndex_t ipIid = {0};
					struct json_object *ipJobj = NULL;
					// wanGroupIfName in format "name"
					strcpy(wanGroupIf, json_object_get_string(wanGroup));
					if (strlen(wanGroupIf)) {
						pc2 = &wanGroupIf[0];
						do {
							pc1 = pc2;
							pc2 = strstr(pc1, ",");
							if (pc2 != NULL) {
								*pc2 = 0;
								++pc2;
							}
							IID_INIT(ipIid);
							ipIid.level = 1;
							sscanf(pc1, "IP.Interface.%hhu", &ipIid.idx[0]);
							if  ((ret = feObjJsonGet(RDM_OID_IP_IFACE, &ipIid,  (void **)&ipJobj, updateFlag)) == ZCFG_SUCCESS) {
								strcat(wanGroupIfName, json_object_get_string(json_object_object_get(ipJobj, "X_ZYXEL_SrvName")));
								strcat(wanGroupIfName, ",");
								zcfgFeObjStructFree(ipJobj);
							}
							if (pc2 == NULL || pc2[0] == 0)
								break;
						} while (1);
					}
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(wanGroupIfName));
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_string(""));
#else
				char *tr98WanIpIface = translateWanIpIface(TRANS_WANIPIFACE_181TO98, json_object_get_string(paramValue));
				if(tr98WanIpIface) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98WanIpIface));
					free(tr98WanIpIface);
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name,  json_object_new_string(""));
#endif
				paramList++;
				continue;
			}

			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
#if 0
			paramValue = json_object_object_get(ipIntfv4AddrJobj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#endif
			paramList++;
			continue;
		}

		/*special case*/
		memset(paramName, 0, sizeof(paramName));
		if(strstr(paramList->name, "IPAddress") != NULL){
			sprintf(paramName, "%s", "IPAddress");
			paramValue = json_object_object_get(ipIntfv4AddrJobj, paramName);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}
		
		if(strstr(paramList->name, "SubnetMask") != NULL){
			sprintf(paramName, "%s", "SubnetMask");
			paramValue = json_object_object_get(ipIntfv4AddrJobj, paramName);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;	
			}
		}

		if(strstr(paramList->name, "AddressingType") != NULL){
			sprintf(paramName, "%s", "AddressingType");
			paramValue = json_object_object_get(ipIntfv4AddrJobj, paramName);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(strstr(paramList->name, "X_ZYXEL_SnoopingMode") != NULL){
			strcpy(ifName, json_object_get_string(json_object_object_get(ipIntfJobj, "X_ZYXEL_IfName")));
			strcpy(snoopingBrIfName, json_object_get_string(json_object_object_get(igmpJobj, "SnoopingBridgeIfName")));
			intf = strtok_r(snoopingBrIfName, ",", &tmp_ptr);
			while(intf != NULL){
				if(*intf != '\0' && strstr(intf, ifName)){
					tmp_ptr2 = strchr(intf, '|');
					if(tmp_ptr2 != NULL){
						*tmp_ptr2 = '\0';
						SnoopingMode = atoi(tmp_ptr2+1);
						if(!(SnoopingMode >=0 && SnoopingMode <=2))
							SnoopingMode = 0;
					}
				}
				intf = strtok_r(tmp_ptr, ",", &tmp_ptr);
			}
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(SnoopingMode));
			paramList++;
			continue;
		}

#ifdef IPV6INTERFACE_PROFILE		
		if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")) {
			paramValue = json_object_object_get(ipIntfJobj, "IPv6Enable");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")) {
			//paramValue = json_object_object_get(ipIntfJobj, "IPv6Status");
			//json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			if(	json_object_get_boolean(json_object_object_get(ipIntfJobj, "IPv6Enable")))
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_ENABLED));
			else
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_DISABLED));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")) {
			paramValue = json_object_object_get(ipIntfJobj, "IPv6AddressNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")) {
			paramValue = json_object_object_get(ipIntfJobj, "IPv6PrefixNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#if 0
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
			if(dnsSrvJobj){
				paramValue = json_object_object_get(dnsSrvJobj, "Enable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{	
				//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(0));
			}
			paramList++;	
			continue;
		}
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
			if(dnsSrvJobj){
				paramValue = json_object_object_get(dnsSrvJobj, "DNSServer");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;	
			continue;
		}
#endif

#endif

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}
	
	json_object_put(igmpJobj);
	json_object_put(ipIntfJobj);
	json_object_put(ipIntfv4AddrJobj);
#if 0
	if(dnsSrvJobj){
		json_object_put(dnsSrvJobj);
	}
#endif
	return ZCFG_SUCCESS;
}

static char *retrieveBridgingName(objIndex_t *brIid)
{
	struct json_object *brObj = NULL;
	const char *brObjBridgeName = NULL;
	char *bridgeName = NULL;

	if(!brIid || (zcfgFeObjJsonGet(RDM_OID_BRIDGING_BR, brIid, &brObj) != ZCFG_SUCCESS))
		return NULL;

	if((brObjBridgeName = json_object_get_string(json_object_object_get(brObj, "X_ZYXEL_BridgeName")))) {
		bridgeName = malloc(strlen(brObjBridgeName) + 1);
		strcpy(bridgeName, brObjBridgeName);
	}
	json_object_put(brObj);

	return bridgeName;
}

#define LAN_DEFAULT_SUBNET "192.168"
#define LAN_DEFAULT_NETMASK "255.255.255.0"
/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i

    Related object in TR181:
    Device.IP.Interface.i
    Device.IP.Interface.i.IPv4Address.i
*/
zcfgRet_t lanIpIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ipIntfOid = 0;
	objIndex_t ipIntfIid, ipIntfv4AddrIid, igmpIid;
	struct json_object *ipIntfJobj = NULL;
	struct json_object *ipIntfv4AddrJobj = NULL;
	struct json_object *igmpJobj = NULL;
#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
	rdm_Dhcpv4SrvPool_t *dhcpServPoolObj = NULL;
	const char *setIpAddr = NULL, *setIpNetMask= NULL;
#endif
	struct json_object *dhcpServPoolJObj = NULL;
	objIndex_t dhcpServPoolIid;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
#if 0
	objIndex_t dnsSrvIid;
	struct json_object *dnsSrvJobj = NULL;
#endif
	//char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//char typeBuf[20] = {0};
	char tmpbuf[256] = {0};
	char ifName[16] = {0};
	char snoopingBrIfName[128] = {0};
	char *tmp_ptr = NULL, *intf = NULL;
	char snoopingMode[5] = {0};
	bool firstMatch = true;
	const char *ipIntfEthLink = NULL;
	struct json_object *ethLinkJobj = NULL;
	objIndex_t ethLinkIid;
	const char *setIfName = NULL;
	//bool ipAddressModify = false;
	const char *spvPubLanNetMask = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("%s:98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	else {
       	/* mappingPathName will be IP.Interface.i*/
		printf("%s: TR181 object %s\n", __FUNCTION__, mappingPathName);
	}

#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL
	setIpAddr = json_object_get_string(json_object_object_get(tr98Jobj, "IPInterfaceIPAddress"));
	setIpNetMask = json_object_get_string(json_object_object_get(tr98Jobj, "IPInterfaceSubnetMask"));
	setIfName = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_IfName"));
	if((setIpAddr || setIpNetMask) && setIfName) {
		printf("%s: Not valid parameters setting\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}
#else
	setIfName = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_IfName"));
#endif

	/* restrict public lan subnet mask as 255.255.255.x */
	spvPubLanNetMask = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_PubLan_Mask"));
	if(spvPubLanNetMask != NULL && strlen(spvPubLanNetMask) > 0 && !checkValidNetMask(spvPubLanNetMask, 24))
		return ZCFG_INVALID_ARGUMENTS;

	IID_INIT(igmpIid);
	IID_INIT(ipIntfIid);
	IID_INIT(ipIntfv4AddrIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpIid, &igmpJobj)) != ZCFG_SUCCESS)
		return ret;
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ipIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ipIntfIid);
	if((ret = zcfgFeObjJsonGet(ipIntfOid, &ipIntfIid, &ipIntfJobj)) != ZCFG_SUCCESS) {
		json_object_put(igmpJobj);
		return ret;
	}

	tr181ParamValue = json_object_object_get(ipIntfJobj, "LowerLayers");
	ipIntfEthLink = json_object_get_string(tr181ParamValue);
	IID_INIT(ethLinkIid);
	sscanf(ipIntfEthLink, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
	ethLinkIid.level = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj)) != ZCFG_SUCCESS) {
		json_object_put(igmpJobj);
		json_object_put(ipIntfJobj);
		return ret;
	}

	if((ret = zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfIid, &ipIntfv4AddrIid, &ipIntfv4AddrJobj)) != ZCFG_SUCCESS) {
		json_object_put(igmpJobj);
		json_object_put(ipIntfJobj);
		json_object_put(ethLinkJobj);
		return ret;
	}

	if(multiJobj){
		tmpObj = igmpJobj;
		igmpJobj = NULL;
		igmpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IGMP, &igmpIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
	
	if(multiJobj){
		tmpObj = ethLinkJobj;
		ethLinkJobj = NULL;
		ethLinkJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_LINK, &ethLinkIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		tmpObj = ipIntfJobj;
		ipIntfJobj = NULL;
		ipIntfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipIntfIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	if(multiJobj){
		tmpObj = ipIntfv4AddrJobj;
		ipIntfv4AddrJobj = NULL;
		ipIntfv4AddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
	if( setIpAddr || setIpNetMask || setIfName){
		IID_INIT(dhcpServPoolIid);
		while(zcfgFeObjStructGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, (void **)&dhcpServPoolObj) == ZCFG_SUCCESS){
			if (strcmp(dhcpServPoolObj->Interface, mappingPathName) == 0){
				if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, &dhcpServPoolJObj)) != ZCFG_SUCCESS)
					return ret;

				if(multiJobj){
					tmpObj = dhcpServPoolJObj;
					dhcpServPoolJObj = NULL;
					dhcpServPoolJObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, multiJobj, tmpObj);
					json_object_put(tmpObj);
                                }
					
				zcfgFeObjStructFree(dhcpServPoolObj);
				break;
			}
			else
				zcfgFeObjStructFree(dhcpServPoolObj);
		}
	}
#else
	if(setIfName) {
		IID_INIT(dhcpServPoolIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, &dhcpServPoolJObj) == ZCFG_SUCCESS) {
			const char *dhcpServIface = json_object_get_string(json_object_object_get(dhcpServPoolJObj, "Interface"));
			if(dhcpServIface && !strcmp(dhcpServIface, "mappingPathName")) {
				if(multiJobj){
					tmpObj = dhcpServPoolJObj;
					dhcpServPoolJObj = NULL;
					dhcpServPoolJObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, multiJobj, tmpObj);
					json_object_put(tmpObj);
				}

				break;
			}
			else {
				zcfgFeJsonObjFree(dhcpServPoolJObj);
			}
		}
	}
#endif
#if 0
	/*DNSServer*/
	IID_INIT(dnsSrvIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, &dnsSrvJobj)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(dnsSrvJobj, "Interface")));
		strcpy(typeBuf, json_object_get_string(json_object_object_get(dnsSrvJobj, "Type")));
		if(!strcmp(mappingPathName, ifBuf) && !strcmp("DHCPv6", typeBuf))
			break;
		else {
			json_object_put(dnsSrvJobj);
			dnsSrvJobj = NULL;
		}
	}

	
	if(multiJobj){
		if(dnsSrvJobj){
			tmpObj = dnsSrvJobj;
			dnsSrvJobj = NULL;
			dnsSrvJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, multiJobj, tmpObj);			
			json_object_put(tmpObj);
		}
	}
#endif	

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			if(strstr(paramList->name, "X_ZYXEL_IfName") != NULL){
				uint8_t brNum = 255;
				char bridgingIfaceName[30] = {0}, ipIfaceIpAddr[20] = {0};
				char startIP[16] = {0}, endIP[16] = {0};
				objIndex_t brIid;
				char *bridgeName = NULL;

				if(!setIfName || !strlen(setIfName) || !strstr(setIfName, "br")) {
					paramList++;
					continue;
				}

				printf("%s: set X_ZYXEL_IfName %s\n", __FUNCTION__, setIfName);
				json_object_object_add(ipIntfJobj, paramList->name, json_object_new_string(setIfName));
				sscanf(setIfName, "br%hhu", &brNum);

				IID_INIT(brIid);
				brIid.idx[0] = brNum+1;
				brIid.level = 1;
				bridgeName = retrieveBridgingName(&brIid);
				printf("%s: Derived bridge name %s\n", __FUNCTION__, bridgeName);
				if(bridgeName) {
					json_object_object_add(ipIntfJobj, "X_ZYXEL_SrvName", json_object_new_string(bridgeName));
					free(bridgeName);
				}

				if(ethLinkJobj) {
					sprintf(bridgingIfaceName, "Bridging.Bridge.%hhu.Port.1", brNum+1);
					printf("%s: Ethernet.Link bridge iface name %s\n", __FUNCTION__, bridgingIfaceName);
					json_object_object_add(ethLinkJobj, "Enable", json_object_new_boolean(true));
					json_object_object_add(ethLinkJobj, "LowerLayers", json_object_new_string(bridgingIfaceName));
				}

				const char *tr181SetIpAddress = json_object_get_string(json_object_object_get(ipIntfv4AddrJobj, "IPAddress"));
				if(ipIntfv4AddrJobj && !(tr181SetIpAddress && strlen(tr181SetIpAddress))) {
					sprintf(ipIfaceIpAddr, "%s.%d.1", LAN_DEFAULT_SUBNET, brNum+1);
					// Default lan ip address
					json_object_object_add(ipIntfv4AddrJobj, "Enable", json_object_new_boolean(true));
					json_object_object_add(ipIntfv4AddrJobj, "IPAddress", json_object_new_string(ipIfaceIpAddr));
					json_object_object_add(ipIntfv4AddrJobj, "SubnetMask", json_object_new_string(LAN_DEFAULT_NETMASK));
					//json_object_object_add(ipIntfv4AddrJobj, "AddressingType", json_object_new_string("Static"));
				}

				paramList++;
				continue;
			}
			else if(strstr(paramList->name, "X_ZYXEL_Group_WAN_IpIface") != NULL) {
#ifdef IAAAA_CUSTOMIZATION
				char groupIf[256] = {0};
				char tr181GroupIf[256] = {0};
                char tr181GroupIfTmp[256] = {0};
				char tmp[32];
				struct json_object *pv = NULL;
				struct json_object *ipJobj = NULL;
				objIndex_t ipIid = {0};
				char *pc1 = NULL, *pc2 = NULL;
				int idx = 0;

				strcpy(groupIf, json_object_to_json_string(paramValue));
				pc2 = &groupIf[0];
				if (*pc2 == '"') ++pc2;
				do {
					pc1 = pc2;
					// skip space char
					while (*pc1 == ' ')
						++pc1;
					// skip ,
					pc2 = strstr(pc1, ",");
					if (pc2 != NULL) {
						*pc2 = 0;
						++pc2;
						// end of param value ?
						if (*pc2 == '"') *pc2 = 0;
					} else {
						// string end with "
						pc2 = strstr(pc1, "\"");
						if (pc2 != NULL) *pc2 = 0;
					}

					if (strlen(pc1) != 0) {
						IID_INIT(ipIid);
						ipIid.level = 1;
						while ((ret = feObjJsonGetNext(RDM_OID_IP_IFACE, &ipIid, &ipJobj, true)) == ZCFG_SUCCESS) {
							pv = json_object_object_get(ipJobj, "X_ZYXEL_SrvName");
							// string pv in format "ifname"
							strcpy(tmp, json_object_to_json_string(pv)+1 /*skip first " */);
							idx = strlen(tmp);
							tmp[idx-1] = 0; // skip last "
							if (strcmp(pc1, tmp)==0) {
								sprintf(tr181GroupIfTmp, "%sIP.Interface.%d,", tr181GroupIf, ipIid.idx[0]);
                                sprintf(tr181GroupIf, "%s", tr181GroupIfTmp);
                                memset(tr181GroupIfTmp, 0, sizeof(tr181GroupIfTmp));
								json_object_put(ipJobj);
								break;
							}
							json_object_put(ipJobj);
						}
				}
					if (pc2 == NULL || pc2[0] == 0)
						break;
				} while (1);
				json_object_object_add(ipIntfJobj, paramList->name, json_object_new_string(tr181GroupIf));
#else
				const char *wanIpIfaces = json_object_get_string(paramValue);

				char *tr181WanIpIface = translateWanIpIface(TRANS_WANIPIFACE_98TO181, wanIpIfaces);
				if(tr181WanIpIface) {
					json_object_object_add(ipIntfJobj, paramList->name, json_object_new_string(tr181WanIpIface));
					free(tr181WanIpIface);
				}
#endif
				paramList++;
				continue;
			}

			/*Write value from Config to Device.DHCPv4.Server.Pool.i*/
			tr181ParamValue = json_object_object_get(ipIntfJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipIntfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			
			/*special case*/
			if(strstr(paramList->name, "IPAddress") != NULL){
				json_object_object_add(ipIntfv4AddrJobj, "IPAddress", JSON_OBJ_COPY(paramValue));
				json_object_object_add(ipIntfv4AddrJobj, "Enable", json_object_new_boolean(true));
#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
				char startIP[16] = {0}, endIP[16] = {0};
				char *lanIP = json_object_get_string(paramValue);
				char *mask = json_object_get_string(json_object_object_get(ipIntfv4AddrJobj, "SubnetMask"));

				setDhcpAddresses(lanIP, mask, startIP, endIP );
				json_object_object_add(dhcpServPoolJObj, "MinAddress", json_object_new_string(startIP));
				json_object_object_add(dhcpServPoolJObj, "MaxAddress", json_object_new_string(endIP));
#endif
				paramList++;
				continue;
			}
			
			if(strstr(paramList->name, "SubnetMask") != NULL){
				json_object_object_add(ipIntfv4AddrJobj, "SubnetMask", JSON_OBJ_COPY(paramValue));
#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
                                char startIP[16] = {0}, endIP[16] = {0};
                                char *lanIP = json_object_get_string(json_object_object_get(ipIntfv4AddrJobj, "IPAddress"));
                                char *mask = json_object_get_string(paramValue);

                                setDhcpAddresses(lanIP, mask, startIP, endIP );
                                json_object_object_add(dhcpServPoolJObj, "MinAddress", json_object_new_string(startIP));
                                json_object_object_add(dhcpServPoolJObj, "MaxAddress", json_object_new_string(endIP));
#endif
				paramList++;
				continue;
			}

			if(strstr(paramList->name, "AddressingType") != NULL){
				json_object_object_add(ipIntfv4AddrJobj, "AddressingType", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#if 0
			if(strstr(paramList->name, "X_ZYXEL_SnoopingMode") != NULL){
				strcpy(snoopingMode, json_object_get_string(paramValue));
				strcpy(ifName, json_object_get_string(json_object_object_get(ipIntfJobj, "X_ZYXEL_IfName")));
				strcpy(snoopingBrIfName, json_object_get_string(json_object_object_get(igmpJobj, "SnoopingBridgeIfName")));
				intf = strtok_r(snoopingBrIfName, ",", &tmp_ptr);
				while(intf != NULL){
					if(*intf != '\0' && strstr(intf, ifName)){
						if(!firstMatch)
							strcat(tmpbuf, ",");
						strcat(tmpbuf, ifName);
						strcat(tmpbuf, "|");
						if(!(atoi(snoopingMode) >=0 && atoi(snoopingMode) <=2))
							strcpy(snoopingMode, "0");
						strcat(tmpbuf, snoopingMode);
					}
					else if(*intf != '\0' && !strstr(intf, ifName)){
						if(!firstMatch)
							strcat(tmpbuf, ",");
						strcat(tmpbuf, intf);
					}
					firstMatch = false;
					intf = strtok_r(tmp_ptr, ",", &tmp_ptr);
				}
				json_object_object_add(igmpJobj, "SnoopingBridgeIfName", json_object_new_string(tmpbuf));
				paramList++;
				continue;
			}
#endif
#ifdef IPV6INTERFACE_PROFILE		
			if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")) {
				json_object_object_add(ipIntfJobj, "IPv6Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#if 0
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "Enable", JSON_OBJ_COPY(paramValue));
					paramList++;	
					continue;
				}
			}
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "DNSServers", JSON_OBJ_COPY(paramValue));
					paramList++;	
					continue;
				}
			}	
#endif

#endif				
		}

		paramList++;	
	} /*Edn while*/

	/*Set Device.IP.Interface.i*/
	if(multiJobj){
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIntfIid, ipIntfJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.i Fail\n", __FUNCTION__);
			goto notmultiComplete;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.IP.Interface.i Success\n", __FUNCTION__);
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_LINK, &ethLinkIid, ethLinkJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Ethernet Link Fail\n", __FUNCTION__);
			goto notmultiComplete;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Ethernet Link Success\n", __FUNCTION__);
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, ipIntfv4AddrJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.i.IPv4Address.i Fail\n", __FUNCTION__);
			goto notmultiComplete;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.IP.Interface.i.IPv4Address.i Success\n", __FUNCTION__);
		}

//#ifdef SAAAE_TR69_AUTO_CHANGE_DHCP_POOL //ZyXEL, Nick Lu, automatic change DHCP pool by LAN IP and subnet mask
		if(dhcpServPoolJObj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, dhcpServPoolJObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Server.Pool.i Fail\n", __FUNCTION__);
				goto notmultiComplete;
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DHCPv4.Server.Pool.i Success\n", __FUNCTION__);
				//json_object_put(dhcpServPoolJObj);
			}
		}
//#endif
notmultiComplete:
		json_object_put(dhcpServPoolJObj);
		json_object_put(igmpJobj);
		json_object_put(ethLinkJobj);
		json_object_put(ipIntfJobj);
		json_object_put(ipIntfv4AddrJobj);
	}

	return ZCFG_SUCCESS;
}

// InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.
zcfgRet_t lanIpIntfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	//char lowerLayerPathName[32] = {0};
	char ipifaceMappingName[MAX_TR181_PATHNAME] = {0};
	//char ifName[8] = {0};
	//char tmpChar[4] ={0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	objIndex_t objIid;
	char *ptr = NULL;
	rdm_IpIface_t *ipIntfObj = NULL;
	uint8_t lanDeviceInst = 0;
	objIndex_t dhcpvSrvIid;
	rdm_Dhcpv4SrvPool_t *dhcpvSrvObj = NULL;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	sscanf(tr98TmpName, "InternetGatewayDevice.LANDevice.%hhu", &lanDeviceInst);
	if(!lanDeviceInst) {
		printf("%s: LANDevice parsing fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("%s: 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	else{
       	/* mappingPathName will be Ethernet.Link.i */
		printf("%s: TR181 object %s\n", __FUNCTION__, mappingPathName);
	}

	// Query if there's IP.Interface already on bridge
	sprintf(tr98TmpName, "%s1", tr98FullPathName);
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ipifaceMappingName)) == ZCFG_SUCCESS){
		printf("%s: LANDevice %s is existed\n", __FUNCTION__, ipifaceMappingName);
		return ZCFG_NO_MORE_INSTANCE;
	}

#if 0
	/*get the bridge number*/
	zcfgFeTr181IfaceStackLowerLayerGet(mappingPathName, lowerLayerPathName);
	ptr = strstr(lowerLayerPathName, ".Port.1");
	*ptr = '\0';	
	ptr = strrchr(lowerLayerPathName, '.');
	strcpy(tmpChar, ptr+1);
	numOfInstance = atoi(tmpChar);

	sprintf(ifName, "br%d", numOfInstance-1);
#endif

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE, &objIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: Add Instance Fail!!\n", __FUNCTION__);
		return ret;
	}
	
	/*set LowerLayer for IP.Interface.i*/
	if((ret = zcfgFeObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIntfObj)) == ZCFG_SUCCESS){
		ipIntfObj->Enable = false;
		ipIntfObj->IPv4Enable = true;
		//ipIntfObj->X_ZYXEL_ConnectionType[0] = '\0';
		strcpy(ipIntfObj->X_ZYXEL_AccessDescription, "AddSetIpIface");
		//strcpy(ipIntfObj->X_ZYXEL_IfName, ifName);
		strcpy(ipIntfObj->LowerLayers, mappingPathName);
		if((ret = zcfgFeObjStructSet(RDM_OID_IP_IFACE, &objIid, (void *)ipIntfObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s: Set IP.Interface LowerLayers Fail\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(ipIntfObj);
	}
	else {
		return ret;
	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[0];
#else
	printf("%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		tr98ObjIid.idx[0] = lanDeviceInst;
		tr98ObjIid.level = 1;
		if(zcfg98To181ObjMappingAdd(e_TR98_IP_INTF, &tr98ObjIid, RDM_OID_IP_IFACE, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added LANDevice %hhu IPInterface %hhu\n", __FUNCTION__, tr98ObjIid.idx[0], tr98ObjIid.idx[1]);
			*idx = tr98ObjIid.idx[1];
		}
	}
	else {
		return ret;
	}
#endif

	/*add object Device.IP.Interface.i.IPv4Address.i */
	if(zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V4_ADDR, &objIid, NULL) != ZCFG_SUCCESS){
		printf("%s: Add ipIface ipv4 Fail!!\n", __FUNCTION__);
		return ZCFG_NO_MORE_INSTANCE;
	}
	
	IID_INIT(dhcpvSrvIid);
	if(zcfgFeObjStructAdd(RDM_OID_DHCPV4_SRV_POOL, &dhcpvSrvIid, NULL) != ZCFG_SUCCESS){
		printf("%s: Add DHCPv4 Server Fail!!\n", __FUNCTION__);
		return ZCFG_NO_MORE_INSTANCE;
	}

	if(zcfgFeObjStructGet(RDM_OID_DHCPV4_SRV_POOL, &dhcpvSrvIid, (void **)&dhcpvSrvObj) == ZCFG_SUCCESS){
		dhcpvSrvObj->Enable = true;
		sprintf(dhcpvSrvObj->Interface, "IP.Interface.%hhu", objIid.idx[0]);
		strcpy(dhcpvSrvObj->X_ZYXEL_DNS_Type, "DNS Proxy");
		if((ret = zcfgFeObjStructSet(RDM_OID_DHCPV4_SRV_POOL, &dhcpvSrvIid, (void *)dhcpvSrvObj, NULL)) != ZCFG_SUCCESS) {
			printf("%s: Set DHCPv4 Server Fail\n", __FUNCTION__);
		}
		zcfgFeObjStructFree(dhcpvSrvObj);
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	return ret;	
}

zcfgRet_t lanIpIntfObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ipIntfOid = 0;
	objIndex_t ipIntfIid;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
       	/* mappingPathName will be IP.Interface.i.IPv4Address.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(ipIntfIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ipIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ipIntfIid);

	ret = zcfgFeObjJsonDel(ipIntfOid, &ipIntfIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		printf("Delete Object Success\n");
	}
	else {
		printf("Delete Object Fail\n");
	}

	return ret;
}

#ifdef LANIPALIAS
// InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPAlias.
zcfgRet_t lanIpAliasObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char ipifaceMappingName[MAX_TR181_PATHNAME] = {0};
	char tr98LanIpIntfName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t ipIntfv4AddrIid;
	rdm_IpIfaceV4Addr_t	*ipIntfv4AddrObj = NULL;

	strcpy(tr98LanIpIntfName, tr98FullPathName);
	ptr = strstr(tr98LanIpIntfName, ".X_ZYXEL_IPAlias");
	*ptr = '\0';
	if((ret = zcfgFe98To181MappingNameGet(tr98LanIpIntfName, ipifaceMappingName)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "%s:  %s mapping isn't existed\n", __FUNCTION__, tr98LanIpIntfName);
		return ret;
	}

	IID_INIT(ipIntfv4AddrIid);
	sscanf(ipifaceMappingName, "IP.Interface.%hhu", &ipIntfv4AddrIid.idx[0]);
	ipIntfv4AddrIid.level = 1;
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, NULL)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "%s: Add Instance Fail!!\n", __FUNCTION__);
		return ret;
	}

	if((ret = zcfgFeObjStructGet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, (void **)&ipIntfv4AddrObj)) == ZCFG_SUCCESS){
		ipIntfv4AddrObj->X_ZYXEL_Alias = true;
		if((ret = zcfgFeObjStructSet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, (void *)ipIntfv4AddrObj, NULL)) != ZCFG_SUCCESS) {
			ztrdbg(ZTR_DL_ERR, "%s: Set Ip Alias Fail\n", __FUNCTION__);
		}

		zcfgFeObjStructFree(ipIntfv4AddrObj);
	}
	else {
		return ret;
	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = ipIntfv4AddrIid.idx[0];
#else
	ztrdbg(ZTR_DL_DEBUG, "%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;
		zcfg_name_t seqnum = 0;
		IID_INIT(tr98ObjIid);
		zcfgFe98NameToSeqnum((const char *)tr98LanIpIntfName, &seqnum, &tr98ObjIid);
		tr98ObjIid.level = 2;
		if(zcfg98To181ObjMappingAdd(e_TR98_IP_ALIAS, &tr98ObjIid, RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid) == ZCFG_SUCCESS) {
			ztrdbg(ZTR_DL_DEBUG, "%s: Added LANDevice %hhu IPInterface %hhu X_ZYXEL_IPAlias %hhu\n", __FUNCTION__, tr98ObjIid.idx[0], tr98ObjIid.idx[1], tr98ObjIid.idx[2]);
			*idx = tr98ObjIid.idx[2];
		}
	}
	else {
		return ret;
	}
#endif

	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPAlias.i

    Related object in TR181:
    Device.IP.Interface.i
    Device.IP.Interface.i.IPv4Address.i
*/
zcfgRet_t lanIpAliasObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	objIndex_t ipIntfv4AddrIid;
	struct json_object *ipIntfv4AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
		ztrdbg(ZTR_DL_DEBUG, "TR181 object %s\n", mappingPathName);
	}

	IID_INIT(ipIntfv4AddrIid);
	ipIntfv4AddrIid.level = 2;
	sscanf(mappingPathName, "IP.Interface.%hhu.IPv4Address.%hhu", &ipIntfv4AddrIid.idx[0], &ipIntfv4AddrIid.idx[1]);
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, &ipIntfv4AddrJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
 	while(paramList && paramList->name != NULL) {
		paramValue = json_object_object_get(ipIntfv4AddrJobj, paramList->name);
		if(paramValue) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		paramList++;
	}
	json_object_put(ipIntfv4AddrJobj);

	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPAlias.i

    Related object in TR181:
    Device.IP.Interface.i
    Device.IP.Interface.i.IPv4Address.i
*/
zcfgRet_t lanIpAliasObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[MAX_TR181_PATHNAME] = {0};
	objIndex_t ipIntfv4AddrIid;
	struct json_object *ipIntfv4AddrJobj = NULL;
	struct json_object *paramValue = NULL, *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		ztrdbg(ZTR_DL_ERR, "98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
		ztrdbg(ZTR_DL_DEBUG, "TR181 object %s\n", mappingPathName);
	}

	IID_INIT(ipIntfv4AddrIid);
	ipIntfv4AddrIid.level = 2;
	sscanf(mappingPathName, "IP.Interface.%hhu.IPv4Address.%hhu", &ipIntfv4AddrIid.idx[0], &ipIntfv4AddrIid.idx[1]);
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, &ipIntfv4AddrJobj, 0)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = ipIntfv4AddrJobj;
		ipIntfv4AddrJobj = NULL;
		ipIntfv4AddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList && paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue && strcmp(paramList->name, "X_ZYXEL_IfName")) {
			json_object_object_add(ipIntfv4AddrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}
		paramList++;
	}

	if(multiJobj) {

	}
	else {
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipIntfv4AddrIid, ipIntfv4AddrJobj, NULL)) != ZCFG_SUCCESS) {
			json_object_put(ipIntfv4AddrJobj);
			return ret;
		}
		json_object_put(ipIntfv4AddrJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPStaticAddress.i

    Related object in TR181:
    Device.DHCPv4.Server.Pool.i.StaticAddress.i
*/
zcfgRet_t lanDhcpStaticAddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  staticAddrOid = 0;
	objIndex_t staticAddrIid;
	struct json_object *staticAddrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
       	/* mappingPathName will be DHCPv4.Server.Pool.i.StaticAddress.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(staticAddrIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	staticAddrOid = zcfgFeObjNameToObjId(tr181Obj, &staticAddrIid);
	if((ret = feObjJsonGet(staticAddrOid, &staticAddrIid, &staticAddrJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	
	/*fill up tr98 DHCPStaticAddress object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(staticAddrJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(staticAddrJobj);	
	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpStaticAddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  staticAddrOid = 0;
	objIndex_t staticAddrIid;
	struct json_object *staticAddrJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
       	/* mappingPathName will be DHCPv4.Server.Pool.i.StaticAddress.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(staticAddrIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	staticAddrOid = zcfgFeObjNameToObjId(tr181Obj, &staticAddrIid);
	if((ret = zcfgFeObjJsonGet(staticAddrOid, &staticAddrIid, &staticAddrJobj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = staticAddrJobj;
		staticAddrJobj = NULL;
		staticAddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &staticAddrIid, multiJobj, tmpObj);
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.DHCPv4.Server.Pool.i.StaticAddress.i*/
			tr181ParamValue = json_object_object_get(staticAddrJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(staticAddrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		paramList++;
	} /*Edn while*/

	/*Set Device.DHCPv4.Server.Pool.i.StaticAddress.i */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &staticAddrIid, staticAddrJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Server.Pool.i.StaticAddress.i Fail\n", __FUNCTION__);
			json_object_put(staticAddrJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DHCPv4.Server.Pool.i.StaticAddress.i Success\n", __FUNCTION__);
		}
		json_object_put(staticAddrJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpStaticAddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	char *ptr = NULL;
	rdm_Dhcpv4SrvPool_t *dhcpServPoolObj = NULL;
	char ethLinkPathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIntfPathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t dhcpServPoolIid;
	bool found = false;
	char tr181ObjName[64] = {0};

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethLinkPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
		/* mappingPathName will be Ethernet.Link.i */
		printf("TR181 object %s\n", ethLinkPathName);
	}

	if((ret =zcfgFeTr181IfaceStackHigherLayerGet(ethLinkPathName, ipIntfPathName)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(dhcpServPoolIid);
	while(zcfgFeObjStructGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, (void **)&dhcpServPoolObj) == ZCFG_SUCCESS){
		if (!strcmp(dhcpServPoolObj->Interface, ipIntfPathName)){
			zcfgFeObjStructFree(dhcpServPoolObj);
			found = true;
			break;
		}
		else
			zcfgFeObjStructFree(dhcpServPoolObj);
	}	

	if(!found){
		zcfgLog(ZCFG_LOG_ERR, "%s : No such object 'DHCPv4.Server.Pool.i'\n", __FUNCTION__);
		return  ZCFG_INTERNAL_ERROR;
	}

	/*add object ' Device.DHCPv4.Server.Pool.i.StaticAddress.i '*/
	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV4_SRV_POOL_STATIC_ADDR, &dhcpServPoolIid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}

	*idx = dhcpServPoolIid.idx[1];

	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpStaticAddrObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  staticAddrOid = 0;
	objIndex_t staticAddrIid;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
       	/* mappingPathName will be DHCPv4.Server.Pool.i.StaticAddress.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(staticAddrIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	staticAddrOid = zcfgFeObjNameToObjId(tr181Obj, &staticAddrIid);

	ret = zcfgFeObjJsonDel(staticAddrOid, &staticAddrIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		printf("Delete Object Success\n");
	}
	else {
		printf("Delete Object Fail\n");
	}

	return ret;
}

zcfgRet_t lanDhcpStaticAddrObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanDhcpStaticAddrObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t lanDhcpStaticAddrObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret =ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}




/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPOption.i

    Related object in TR181:
    Device.DHCPv4.Server.Pool.i.Option.i
*/
zcfgRet_t lanDhcpOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  dhcpOptOid = 0;
	objIndex_t dhcpOptIid;
	struct json_object *dhcpOptJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(dhcpOptIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	dhcpOptOid = zcfgFeObjNameToObjId(tr181Obj, &dhcpOptIid);
	if((ret = feObjJsonGet(dhcpOptOid, &dhcpOptIid, &dhcpOptJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	
	/*fill up tr98 DHCPOption.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(dhcpOptJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
#if 0
			if(!strcmp(paramList->name, "Value")){
				const char *paramObjStr = json_object_get_string(paramValue);
				int paramObjStrLen = strlen(paramObjStr);
				char *outputBase64 = base64_encode((unsigned char *)paramObjStr, paramObjStrLen);
				printf("%s: encoded base64 value: %s\n", __FUNCTION__, outputBase64 ? outputBase64 : "null");
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(outputBase64));
				free(outputBase64);
			}else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
#endif
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(dhcpOptJobj);	
	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  dhcpOptOid = 0;
	objIndex_t dhcpOptIid;
	struct json_object *dhcpOptJobj = NULL;
	struct json_object *paramValue = NULL;
	// struct json_object *tr181ParamValue = NULL;
	//struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	printf("%s: %s\n", __FUNCTION__, mappingPathName);
	IID_INIT(dhcpOptIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	dhcpOptOid = zcfgFeObjNameToObjId(tr181Obj, &dhcpOptIid);
#if 0
	if((ret = zcfgFeObjJsonGet(dhcpOptOid, &dhcpOptIid, &dhcpOptJobj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = dhcpOptJobj;
		dhcpOptJobj = NULL;
		dhcpOptJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL_OPT, &dhcpOptIid, multiJobj, tmpObj);
	}
#endif

	printf("%s: %s\n", __FUNCTION__, tr181Obj);
	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);
#if 0
	char optValue[256] = {0};
	char *decodeOptValue = NULL;
	int decodeOptValueLen = 0;
	const char *valueStr = json_object_get_string(json_object_object_get(tr98Jobj, "Value"));
	if(valueStr){

		printf("%s: spv value: %s\n", __FUNCTION__, valueStr);
		strncpy(optValue, valueStr, sizeof(optValue)-1);
		printf("%s: decode value: %s\n", __FUNCTION__, optValue);
		decodeOptValue = base64_decode((unsigned char *)optValue, &decodeOptValueLen);
		printf("%s: base64 decoded: %s\n", __FUNCTION__, decodeOptValueLen ? decodeOptValue : "null" );
	}
#endif
	dhcpOptJobj = json_object_new_object();

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue){
			paramList++;
			continue;
		}

		/*Write value from Config to Device.DHCPv4.Server.Pool.i.Option.i*/
		if(!strcmp(paramList->name, "Value")){
			const char *valueStr = json_object_get_string(paramValue);
			if(strlen(valueStr) > 255){
				ret = ZCFG_INVALID_PARAM_VALUE;
			}else{
				json_object_object_add(dhcpOptJobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
		}else{
			json_object_object_add(dhcpOptJobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}

		paramList++;
	} /*Edn while*/

	//if(decodeOptValue)
		//free(decodeOptValue);

	if(ret != ZCFG_SUCCESS){
		json_object_put(dhcpOptJobj);
		return ret;
	}

	/*Set DHCPv4.Server.Pool.i.Option.i */
	if(multiJobj){
#ifdef ZYXEL_OPAL_EXTENDER
		zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SENT_OPT, &dhcpOptIid, multiJobj, dhcpOptJobj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DHCPV4_SENT_OPT, &dhcpOptIid, "partialset", json_object_new_boolean(true));
#else
		zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL_OPT, &dhcpOptIid, multiJobj, dhcpOptJobj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DHCPV4_SRV_POOL_OPT, &dhcpOptIid, "partialset", json_object_new_boolean(true));
#endif
	}
	else{
		const char *dhcpOptJobjStr = json_object_to_json_string(dhcpOptJobj);
#ifdef ZYXEL_OPAL_EXTENDER
		if((ret = zcfgFeObjWrite(RDM_OID_DHCPV4_SENT_OPT, &dhcpOptIid, dhcpOptJobjStr, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
#else
		if((ret = zcfgFeObjWrite(RDM_OID_DHCPV4_SRV_POOL_OPT, &dhcpOptIid, dhcpOptJobjStr, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
#endif
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Server.Pool.i.StaticAddress.i Fail\n", __FUNCTION__);
			json_object_put(dhcpOptJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DHCPv4.Server.Pool.i.StaticAddress.i Success\n", __FUNCTION__);
		}
	}
	json_object_put(dhcpOptJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ethLinkPathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIntfPathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t dhcpServPoolIid;
	char *ptr = NULL;
	uint32_t  ipIntfOid = 0;
	rdm_IpIface_t *ipIntfObj = NULL;
	rdm_Dhcpv4SrvPool_t *dhcpServPoolObj = NULL;
	bool found = false;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethLinkPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
		/* mappingPathName will be Ethernet.Link.i */
		printf("TR181 object %s\n", ethLinkPathName);
	}

	if((ret =zcfgFeTr181IfaceStackHigherLayerGet(ethLinkPathName, ipIntfPathName)) != ZCFG_SUCCESS)
		return ret;
	
	IID_INIT(dhcpServPoolIid);
	while(zcfgFeObjStructGetNext(RDM_OID_DHCPV4_SRV_POOL, &dhcpServPoolIid, (void **)&dhcpServPoolObj) == ZCFG_SUCCESS){
		if (!strcmp(dhcpServPoolObj->Interface, ipIntfPathName)){
			zcfgFeObjStructFree(dhcpServPoolObj);
			found = true;
			break;
		}
		else
			zcfgFeObjStructFree(dhcpServPoolObj);
	}	

	if(!found){
		zcfgLog(ZCFG_LOG_ERR, "%s : No such object 'DHCPv4.Server.Pool.i'\n", __FUNCTION__);
		return  ZCFG_INTERNAL_ERROR;
	}

	/*add object ' Device.DHCPv4.Server.Pool.i.Option.i '*/
	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV4_SRV_POOL_OPT, &dhcpServPoolIid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}
	
	*idx = dhcpServPoolIid.idx[1];
	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpOptObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  dhcpOptOid = 0;
	objIndex_t dhcpOptIid;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(dhcpOptIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	dhcpOptOid = zcfgFeObjNameToObjId(tr181Obj, &dhcpOptIid);

	ret = zcfgFeObjJsonDel(dhcpOptOid, &dhcpOptIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		printf("Delete Object Success\n");
	}
	else {
		printf("Delete Object Fail\n");	
	}

	return ret;
}

zcfgRet_t lanDhcpOptObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanDhcpOptObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t lanDhcpOptObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}




/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i

    Related object in TR181:
    Device.DHCPv4.Server.Pool.i
*/
zcfgRet_t lanDhcpCondServPoolObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{

	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	//char tr181Obj[128] = {0};
	uint32_t Oid = 0;
	objIndex_t Iid;
	struct json_object *Jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s() 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(Iid);
	//sprintf(tr181Obj, "Device.%s", mappingPathName);
	Oid = RDM_OID_DHCPV4_SRV_POOL;
	Iid.level += 1;

	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu", &Iid.idx[0]);
	if(n != 1 || (ret = feObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &Iid, &Jobj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s() feObjJsonGet Fail!! Oid=%u ret=%d\n\n", __FUNCTION__, Oid, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();

	const char *ipIface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	if(ipIface){
		struct json_object *ipIfaceAddrObj = NULL;
		objIndex_t ipIfaceAddrIid;
		IID_INIT(ipIfaceAddrIid);
		ipIfaceAddrIid.level = 2;
		int n = sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceAddrIid.idx[0]);
		ipIfaceAddrIid.idx[1] = 1;
		if((n = 1) && zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipIfaceAddrIid, &ipIfaceAddrObj) == ZCFG_SUCCESS){
			const char *ipAddr = json_object_get_string(json_object_object_get(ipIfaceAddrObj, "IPAddress"));
			json_object_object_add(*tr98Jobj, "DHCPServerIPAddress", json_object_new_string(ipAddr));
			json_object_put(ipIfaceAddrObj);
		}else{
			json_object_object_add(*tr98Jobj, "DHCPServerIPAddress", json_object_new_string(""));
		}
	}else{
		json_object_object_add(*tr98Jobj, "DHCPServerIPAddress", json_object_new_string(""));
	}

	/*fill up tr98 LANEthernetInterfaceConfig object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(Jobj, paramList->name);

		if(!paramValue){
			if(strstr(paramList->name, "Order")){
				paramValue = json_object_object_get(Jobj, "Order");
			}
			else if(strstr(paramList->name, "LeaseTime")){
				paramValue = json_object_object_get(Jobj, "LeaseTime");
			}
			else if(strstr(paramList->name, "StaticAddressNumberOfEntries")){
				paramValue = json_object_object_get(Jobj, "StaticAddressNumberOfEntries");
			}
			else if(strstr(paramList->name, "OptionNumberOfEntries")){
				paramValue = json_object_object_get(Jobj, "OptionNumberOfEntries");
			}
		}

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(Jobj);	
	return ZCFG_SUCCESS;
}

// TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i
zcfgRet_t lanDhcpCondServPoolObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{

	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	//char tr181Obj[128] = {0};
	uint32_t Oid = 0;
	objIndex_t Iid;
	struct json_object *Jobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s() 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(Iid);
	//sprintf(tr181Obj, "Device.%s", mappingPathName);
	Oid = RDM_OID_DHCPV4_SRV_POOL;
	Iid.level += 1;
	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu", &Iid.idx[0]);
	if(n != 1 || (ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &Iid, &Jobj)) != ZCFG_SUCCESS){
		return ret;
	}
	if(multiJobj){
		tmpObj = Jobj;
		Jobj = NULL;
		Jobj = zcfgFeJsonMultiObjAppend(Oid, &Iid, multiJobj, tmpObj);
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.Ethernet.Interface.i*/
			tr181ParamValue = json_object_object_get(Jobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				//paramList++;
				//continue;
			}
			else if(strstr(paramList->name, "Order")){
				json_object_object_add(Jobj, "Order", JSON_OBJ_COPY(paramValue));
			}
			else if(strstr(paramList->name, "LeaseTime")){
				json_object_object_add(Jobj, "LeaseTime", JSON_OBJ_COPY(paramValue));
			}
			else if(strstr(paramList->name, "StaticAddressNumberOfEntries")){
				json_object_object_add(Jobj, "StaticAddressNumberOfEntries", JSON_OBJ_COPY(paramValue));
			}
			else if(strstr(paramList->name, "OptionNumberOfEntries")){
				json_object_object_add(Jobj, "OptionNumberOfEntries", JSON_OBJ_COPY(paramValue));
			}		
		}

		paramList++;	
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(Oid, &Iid, Jobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Object Oid:%u Fail\n", __FUNCTION__, Oid);
			json_object_put(Jobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Object Oid:%u Success\n", __FUNCTION__, Oid);
		}
		json_object_put(Jobj);
	}
	return ZCFG_SUCCESS;
}

// TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i
zcfgRet_t lanDhcpCondServPoolObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr181ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t objIid;
	objIndex_t ipIntfIid;
	rdm_IpIface_t *ipIntfObj = NULL;
	rdm_Dhcpv4SrvPool_t *dhcpSrvPoolObj = NULL;
	char ethLinkPathName[32] = {0};
	char ipIntfPathName[32] = {0};
	char *ptr;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV4_SRV_POOL, &objIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s(): add RDM_OID_DHCPV4_SRV_POOL Fail!!\n", __FUNCTION__);
		return ret;
	}

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".LANHostConfigManagement");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethLinkPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s() 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	else{
		/* mappingPathName will be Ethernet.Link.i */
		zcfgLog(LOG_DEBUG, "%s() TR181 object %s\n", __FUNCTION__, ethLinkPathName);
	}

	IID_INIT(ipIntfIid);
	while(feObjStructGetNext(RDM_OID_IP_IFACE, &ipIntfIid, (void **)&ipIntfObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS){
		if (strcmp(ipIntfObj->LowerLayers, ethLinkPathName) == 0){
			sprintf(ipIntfPathName, "IP.Interface.%d", ipIntfIid.idx[0]);
			zcfgFeObjStructFree(ipIntfObj);
			break;
		}
		else
			zcfgFeObjStructFree(ipIntfObj);
	}

	if((ret = zcfgFeObjStructGet(RDM_OID_DHCPV4_SRV_POOL, &objIid, (void **)&dhcpSrvPoolObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s(): get RDM_OID_DHCPV4_SRV_POOL Obj Fail!!\n", __FUNCTION__);
		return ret;
	}

	strcpy(dhcpSrvPoolObj->Interface, ipIntfPathName);
	ret = zcfgFeObjStructSet(RDM_OID_DHCPV4_SRV_POOL, &objIid, (void *)dhcpSrvPoolObj, NULL);
	zcfgFeObjStructFree(dhcpSrvPoolObj);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
	{			
		zcfgLog(ZCFG_LOG_ERR, "%s(): Set RDM_OID_DHCPV4_SRV_POOL Fail. ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	sprintf(tr181ObjName, "DHCPv4.Server.Pool.%d", objIid.idx[0] );
#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	if (ret != ZCFG_DELAY_APPLY){
		if((ret = zcfgFe181To98MappingNameGet(tr181ObjName, tr98ObjName)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s(): Line:%d, 181 to 98 Mapping Fail!!\n", __FUNCTION__, __LINE__);
			return ret;
		}

		sscanf(tr98ObjName, "%*[^.].%*[^.].%*d.%*[^.].DHCPConditionalServingPool.%d", idx);
		zcfgLog(LOG_DEBUG, "%s(): *idx= %d \n", __FUNCTION__, *idx);
	}
#else
	zcfg_name_t nameSeqNum = 0;
	objIndex_t tr98ObjIid;
	*idx = 0;

	IID_INIT(tr98ObjIid);
	zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)tr181ObjName, &nameSeqNum, &tr98ObjIid);

	if(IS_SEQNUM_RIGHT(nameSeqNum)) {
		printf("Added DHCPConditionalServingPool %d\n", tr98ObjIid.idx[1]);
		*idx = tr98ObjIid.idx[1];
	}
#endif

	return ret;
}

// TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i
zcfgRet_t lanDhcpCondServPoolObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	//char tr181Obj[128] = {0};
	uint32_t  Oid = 0;
	objIndex_t Iid;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s() 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ret;
	}
	

	IID_INIT(Iid);
	//sprintf(tr181Obj, "Device.%s", mappingPathName);
	Oid = RDM_OID_DHCPV4_SRV_POOL;
	Iid.level += 1;
	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu", &Iid.idx[0]);

	ret = zcfgFeObjJsonDel(Oid, &Iid, NULL);
	if(ret == ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s() Delete Object Success\n", __FUNCTION__);
	}
	else if (ret == ZCFG_DELAY_APPLY){
		zcfgLog(ZCFG_LOG_DEBUG, "%s() Delete Object, ret = ZCFG_DELAY_APPLY\n", __FUNCTION__);	
	}
	else{
		zcfgLog(ZCFG_LOG_ERR, "%s() Delete Object Fail. ret=%d\n", __FUNCTION__, ret);
	}

	return ret;
}

#if 0
zcfgRet_t lanDhcpCondServPoolObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanDhcpCondServPoolObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mappingPathName[128] = {0};
	uint32_t Oid = 0;
	objIndex_t Iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mappingPathName);
	Oid = zcfgFeObjNameToObjId(tr181Obj, &Iid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/
		
		attrValue = zcfgFeParamAttrGetByName(Oid, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t lanDhcpCondServPoolObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mappingPathName[128] = {0};
	uint32_t Oid = 0;
	objIndex_t Iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s() 98 to 181 Mapping Fail!!\n", __FUNCTION__);
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mappingPathName);
	Oid = zcfgFeObjNameToObjId(tr181Obj, &Iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(Oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(Oid, multiAttrJobj, paramList->name, attrValue);
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, Oid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}
#endif
#if 0

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i.DHCPStaticAddress.i

    Related object in TR181:
    Device.DHCPv4.Server.Pool.i.StaticAddress.i
*/
zcfgRet_t lanDhcpCondServPoolStaticAddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{

	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpCondServPoolStaticAddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{

	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpCondServPoolStaticAddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	objIndex_t objIid;
	uint32_t  oid = 0;
	char *ptr = NULL;

       printf("Enter %s\n", __FUNCTION__);
        
	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".DHCPStaticAddress");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i */
	printf("tr98TmpName %s\n", tr98TmpName);
		        
       if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
       }
	else{
		/* mappingPathName will be DHCPv4.Server.Pool.i */
		printf("TR181 object %s\n", mappingPathName);
       }

	sprintf(tr181Obj, "Device.%s", mappingPathName);
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId(tr181Obj, &objIid);
	if((ret = zcfgFeObjStructAdd(oid, &objIid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}
	
	*idx = objIid.idx[1];
	return ZCFG_SUCCESS;
}

zcfgRet_t lanDhcpCondServPoolStaticAddrObjDel(char *tr98FullPathName)
{

	return ZCFG_SUCCESS;
}
#endif

/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i.DHCPOption.i

    Related object in TR181:
    Device.DHCPv4.Server.Pool.i.Option.i
*/
zcfgRet_t lanDhcpCondServPoolOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	char mappingPathName[128] = {0};

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: %s\n", __FUNCTION__, mappingPathName);

	struct json_object *dhcp4SvrOpt = NULL, *paramObj;
	objIndex_t objIid;
	IID_INIT(objIid);
	objIid.level += 2;
	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu.Option.%hhu", &objIid.idx[0], &objIid.idx[1]);
	zcfgRet_t rst = ZCFG_NO_SUCH_OBJECT;
	if(n != 2 || (rst = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, &dhcp4SvrOpt)) != ZCFG_SUCCESS){
		printf("%s: no DHCPv4 server pool option object\n", __FUNCTION__);
		return rst;
	}

	struct tr98Parameter_s *parmList = tr98Obj[handler].parameter;
	if(!parmList){
		return ZCFG_NO_SUCH_OBJECT;
	}

	*tr98Jobj = json_object_new_object();
	while(parmList->name){
		paramObj = json_object_object_get(dhcp4SvrOpt, parmList->name);
		if(paramObj){
			json_object_object_add(*tr98Jobj, parmList->name, JSON_OBJ_COPY(paramObj));
#if 0
			if(!strcmp(parmList->name, "Value")){
				const char *paramObjStr = json_object_get_string(paramObj);
				int paramObjStrLen = strlen(paramObjStr);
				char *outputBase64 = base64_encode((unsigned char *)paramObjStr, paramObjStrLen);
				printf("%s: encoded base64 value: %s\n", __FUNCTION__, outputBase64 ? outputBase64 : "null");
				json_object_object_add(*tr98Jobj, parmList->name, json_object_new_string(outputBase64));
				free(outputBase64);
			}else{
				json_object_object_add(*tr98Jobj, parmList->name, JSON_OBJ_COPY(paramObj));
			}
#endif
		}
		parmList++;
	}
	json_object_put(dhcp4SvrOpt);

	return ZCFG_SUCCESS;
}

//InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
zcfgRet_t lanDhcpCondServPoolOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	char mappingPathName[128] = {0};

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: %s\n", __FUNCTION__, mappingPathName);

	struct json_object *dhcp4SvrOpt = NULL, *paramObj;
	objIndex_t objIid;
	IID_INIT(objIid);
	objIid.level += 2;
	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu.Option.%hhu", &objIid.idx[0], &objIid.idx[1]);
	zcfgRet_t rst = ZCFG_NO_SUCH_OBJECT;
	if(n != 2 || (rst = zcfgFeObjJsonQueryExist(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid)) != ZCFG_SUCCESS){
		printf("%s: no DHCPv4 server pool option object\n", __FUNCTION__);
		return rst;
	}

	struct tr98Parameter_s *parmList = tr98Obj[handler].parameter;
	if(!parmList){
		return ZCFG_NO_SUCH_OBJECT;
	}

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

#if 0
	char optValue[256] = {0};
	char *decodeOptValue = NULL;
	int decodeOptValueLen = 0;
	const char *valueStr = json_object_get_string(json_object_object_get(tr98Jobj, "Value"));
	if(valueStr){
		strncpy(optValue, valueStr, sizeof(optValue)-1);
		decodeOptValue = base64_decode((unsigned char *)optValue, &decodeOptValueLen);
		printf("%s: base64 decoded: %s\n", __FUNCTION__, decodeOptValueLen ? decodeOptValue : "null" );
	}
#endif
	dhcp4SvrOpt = json_object_new_object();

	while(parmList->name){
		paramObj = json_object_object_get(tr98Jobj, parmList->name);
		if(paramObj){
			if(!strcmp(parmList->name, "Value")){
				const char *valueStr = json_object_get_string(paramObj);
				if(strlen(valueStr) > 255){
					rst = ZCFG_INVALID_PARAM_VALUE;
				}else
					json_object_object_add(dhcp4SvrOpt, parmList->name, JSON_OBJ_COPY(paramObj));
			}else{
				json_object_object_add(dhcp4SvrOpt, parmList->name, JSON_OBJ_COPY(paramObj));
			}
		}
		parmList++;
	}

	//if(decodeOptValue)
	//	free(decodeOptValue);

	if(rst != ZCFG_SUCCESS){
		json_object_put(dhcp4SvrOpt);
		return rst;
	}


	if(multiJobj){
		zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, multiJobj, dhcp4SvrOpt);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, "partialset", json_object_new_boolean(true));
	}else{
		rst = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, dhcp4SvrOpt, NULL);
	}
	json_object_put(dhcp4SvrOpt);


	return rst;
}

//InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
zcfgRet_t lanDhcpCondServPoolOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	objIndex_t objIid;
	uint32_t  oid = 0;
	char *ptr = NULL;

	printf("Enter %s\n", __FUNCTION__);

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".DHCPOption");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i */
	printf("tr98TmpName %s\n", tr98TmpName);

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}
	else{
		/* mappingPathName will be DHCPv4.Server.Pool.i */
		printf("TR181 object %s\n", mappingPathName);
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu", &objIid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, NULL)) != ZCFG_SUCCESS){
		printf("Add Instance Fail!!\n");
		return ret;
	}
	
	*idx = objIid.idx[1];
	return ZCFG_SUCCESS;
}

//InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
zcfgRet_t lanDhcpCondServPoolOptObjDel(char *tr98FullPathName)
{
	char mappingPathName[128] = {0};

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: %s\n", __FUNCTION__, mappingPathName);

	struct json_object *dhcp4SvrOpt = NULL, *paramObj;
	objIndex_t objIid;
	IID_INIT(objIid);
	objIid.level += 2;
	int n = sscanf(mappingPathName, "DHCPv4.Server.Pool.%hhu.Option.%hhu", &objIid.idx[0], &objIid.idx[1]);
	zcfgRet_t rst = ZCFG_NO_SUCH_OBJECT;
	if(n != 2 || (rst = zcfgFeObjJsonQueryExist(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid)) != ZCFG_SUCCESS){
		printf("%s: no DHCPv4 server pool option object\n", __FUNCTION__);
		return rst;
	}

	rst = zcfgFeObjJsonDel(RDM_OID_DHCPV4_SRV_POOL_OPT, &objIid, NULL);

	return rst;
}


/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANEthernetInterfaceConfig.i

    Related object in TR181:
    Device.Ethernet.Interface.i
*/
zcfgRet_t lanEthIntfConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	char *currentBitrate = NULL;
	uint32_t  ethIntfOid = 0;
	objIndex_t ethIntfIid;
	struct json_object *ethIntfJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *currentBitRateObj = NULL;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(ethIntfIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ethIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ethIntfIid);
	if((ret = feObjJsonGet(ethIntfOid, &ethIntfIid, &ethIntfJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 LANEthernetInterfaceConfig object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ethIntfJobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "Status"))
			{
				const char *paramObjStr = json_object_get_string(paramValue);
				const bool *paramObjBool = json_object_get_boolean(json_object_object_get(ethIntfJobj, "Enable"));
				if(!strcmp(paramObjStr, "Down"))
				{
					if(!paramObjBool)
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Disabled"));
					else
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("NoLink"));
				}
				else
				{
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Up"));
				}
			}
            else if (!strcmp(paramList->name, "MaxBitRate"))
			{
				if (json_object_get_int(paramValue) == 0)
				{
					json_object_object_add(*tr98Jobj, "MaxBitRate", json_object_new_string("Auto"));
				}
				else
				{
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}
			else
			{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_CurrentBitRate"))
		{
			currentBitRateObj = json_object_object_get(ethIntfJobj, "CurrentBitRate");
			if (currentBitRateObj != NULL)
			{
				currentBitrate = json_object_get_string(currentBitRateObj);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(currentBitrate));
			}
			else
			{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("0"));
			}
			paramList++;
			continue;
		}


		
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	// Remove excess release
	//json_object_put(ethIntfJobj);	
	return ZCFG_SUCCESS;
}

zcfgRet_t lanEthIntfConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ethIntfOid = 0;
	objIndex_t ethIntfIid;
	struct json_object *ethIntfJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(ethIntfIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ethIntfOid = zcfgFeObjNameToObjId(tr181Obj, &ethIntfIid);
	if((ret = zcfgFeObjJsonGet(ethIntfOid, &ethIntfIid, &ethIntfJobj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = ethIntfJobj;
		ethIntfJobj = NULL;
		ethIntfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_IFACE, &ethIntfIid, multiJobj, tmpObj);
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		printf("Parameter Name %s\n", paramList->name);
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			/*Write value from Config to Device.Ethernet.Interface.i*/
			tr181ParamValue = json_object_object_get(ethIntfJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ethIntfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}		
		}

		paramList++;
	} /*Edn while*/

	/*Set Device.Ethernet.Interface.i */
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_IFACE, &ethIntfIid, ethIntfJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Ethernet.Interface.i Fail\n", __FUNCTION__);
			json_object_put(ethIntfJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Ethernet.Interface.i Success\n", __FUNCTION__);
		}
		json_object_put(ethIntfJobj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t lanEthIntfConfObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int lanEthIntfConfObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}


	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);


	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

zcfgRet_t lanEthIntfConfObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}


	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/

	return ret;
}





/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANEthernetInterfaceConfig.i.Stats

    Related object in TR181:
    Device.Ethernet.Interface.i.Stats
*/
extern zcfgRet_t lanEthIntfConfStatObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char mappingPathName[128] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr181Obj[128] = {0};
	uint32_t  ethIntfStatOid = 0;
	objIndex_t ethIntfStatIid;
	struct json_object *ethIntfStatJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	strcpy(tr98TmpName, tr98FullPathName);

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		printf("98 to 181 Mapping Fail!!\n");
		return ret;
	}

	IID_INIT(ethIntfStatIid);
	sprintf(tr181Obj, "Device.%s", mappingPathName);
	ethIntfStatOid = zcfgFeObjNameToObjId(tr181Obj, &ethIntfStatIid);
	if((ret = feObjJsonGet(ethIntfStatOid, &ethIntfStatIid, &ethIntfStatJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 LANEthernetInterfaceConfig.i.Stats object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ethIntfStatJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(ethIntfStatJobj);
	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i
 *
 *   Related object in TR181:
 *
 *   Device.WiFi.Radio.i
 *   Device.WiFi.SSID.i
 *   Device.WiFi.AccessPoint.i
 *   Device.WiFi.AccessPoint.i.Security
 */
zcfgRet_t lanDevWlanCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char wifiSsid[32] = {0};
	char wifiRadio[32] = {0};
	char ssidRef[32] = {0};
	char ifName[16] = {0};
	objIndex_t ssidIid, radioIid, apIid;
	objIndex_t wifiIid;
	struct json_object *wifiJobj = NULL;
	struct json_object *radioJobj = NULL;
	struct json_object *ssidStatsJobj = NULL;
	struct json_object *ssidJobj = NULL;
	struct json_object *apJobj = NULL;
	struct json_object *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool freeflag = 0;
	FILE *pop = NULL;
	char lst[5] = {0};
	char clientBuf[5] = {0};
	char cmd[64] = {0};
	int clientNum = 0, ssid_attr = 0;
	char tmp[256] = {0}, *pch = NULL;
	bool backhaulSsidVisalbe = false;

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, wifiSsid)) != ZCFG_SUCCESS) {
		/*  The instance number of WLANConfiguration.i will be continuous because of the tr98 to tr181 mapping table.
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WLANConfiguration object.
		 */
		return ret; //ZCFG_NO_MORE_INSTANCE;
	}

	/*mapping InternetGatewayDevice.LANDevice.1.WLANConfiguration.i to Device.WiFi.SSID.i*/

	IID_INIT(wifiIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI, &wifiIid, &wifiJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi Fail\n", __FUNCTION__);
		json_object_put(wifiJobj);
		return ret;
	}
	backhaulSsidVisalbe = json_object_get_boolean(json_object_object_get(ssidJobj, "X_ZYXEL_BackhaulSSIDVisable"));

	IID_INIT(ssidIid);
	ssidIid.level = 1;
	sscanf(wifiSsid, "WiFi.SSID.%hhu", &ssidIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WIFI_SSID, &ssidIid, &ssidJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID Fail\n", __FUNCTION__);
		return ret;
	}

	if(!backhaulSsidVisalbe){
		ssid_attr = json_object_get_int(json_object_object_get(ssidJobj, "X_ZYXEL_Attribute"));
		if(ssid_attr != 0){
			json_object_put(ssidJobj);
			return ZCFG_INVALID_OBJECT;
		}
	}

	if((ret = feObjJsonGet(RDM_OID_WIFI_SSID_STAT, &ssidIid, &ssidStatsJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID.i.Stats Fail\n", __FUNCTION__);
		json_object_put(ssidJobj);
		return ret;
	}

	strcpy(wifiRadio, json_object_get_string(json_object_object_get(ssidJobj, "LowerLayers")));
	IID_INIT(radioIid);
	radioIid.level = 1;
	sscanf(wifiRadio, "WiFi.Radio.%hhu", &radioIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WIFI_RADIO, &radioIid, &radioJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.Radio Fail\n", __FUNCTION__);
		json_object_put(ssidJobj);
		json_object_put(ssidStatsJobj);
		return ret;
	}

	IID_INIT(apIid);
	while((ret = feObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else
			json_object_put(apJobj);
	}

	if(apJobj == NULL) {
		apJobj = json_object_new_object();
		apSecJobj = json_object_new_object();
	}
	else {
		if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj, updateFlag)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(radioJobj);
			json_object_put(ssidStatsJobj);
			json_object_put(ssidJobj);
			json_object_put(apJobj);
			return ret;
		}
	}

	/*fill up tr98 LANEthernetInterfaceConfig object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		freeflag = 0;
		paramValue = json_object_object_get(ssidJobj, paramList->name);
		if(paramValue != NULL && !strcmp(paramList->name, "Status")) {
			const char *paramStatus = json_object_get_string(paramValue);
			if(!strcmp(paramStatus, "Up"))
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			else if(strstr(paramStatus, "Err"))
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			else
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Disabled"));
			paramList++;
			continue;
		}
		else if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(radioJobj, paramList->name);
#ifdef ZYXEL_WLAN_MAXBITRATE /* Need MaxBitRate for OAAAA project, Bug #37690, Ian */
		if((paramValue != NULL) && !strcmp(paramList->name, "MaxBitRate")) {
			struct json_object *paramRate_1 = json_object_object_get(ssidJobj, "X_ZYXEL_UpstreamRate");
			struct json_object *paramRate_2 = json_object_object_get(ssidJobj, "X_ZYXEL_DownstreamRate");
			if((paramRate_1 != NULL) && (paramRate_2 != NULL)) {
				char tr98maxBitRate[10];
				uint32_t maxBitRate = 0;
				uint32_t rate_1 = json_object_get_int(paramRate_1);
				uint32_t rate_2 = json_object_get_int(paramRate_2);

				maxBitRate = (rate_1 > rate_2) ? rate_1 : rate_2;
				memset(tr98maxBitRate, 0, sizeof(tr98maxBitRate));
				/* When MaxBitRate is set as 'Auto' */
				if(maxBitRate == 0) {
					sprintf(tr98maxBitRate, "%s", "Auto");
				}
				else {
					sprintf(tr98maxBitRate, "%4.1f", (float)(maxBitRate/1000.0));
				}
				tr98maxBitRate[4] = '\0';
				zcfgLog(ZCFG_LOG_INFO, "%s: tr98maxBitRate %s\n", __FUNCTION__, tr98maxBitRate);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98maxBitRate));
			}
			paramList++;
			continue;
		}
#else
		if((paramValue != NULL) && !strcmp(paramList->name, "MaxBitRate")){
			paramValue = json_object_object_get(radioJobj, "X_ZYXEL_MaxBitRate");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(apJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(apSecJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "TotalBytesSent")) {
			paramValue = json_object_object_get(ssidStatsJobj, "BytesSent");
		}
		else if(!strcmp(paramList->name, "TotalBytesReceived")) {
			paramValue = json_object_object_get(ssidStatsJobj, "BytesReceived");
		}
		else if(!strcmp(paramList->name, "TotalPacketsSent")) {
			paramValue = json_object_object_get(ssidStatsJobj, "PacketsSent");
		}
		else if(!strcmp(paramList->name, "TotalPacketsReceived")) {
			paramValue = json_object_object_get(ssidStatsJobj, "PacketsReceived");
		}
		else if(!strcmp(paramList->name, "TotalPSKFailures")) {
			paramValue = json_object_object_get(ssidStatsJobj, "X_ZYXEL_TotalPSKFailures");
		}
		else if(!strcmp(paramList->name, "TotalAssociations")) {
			paramValue = json_object_object_get(apJobj, "AssociatedDeviceNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "RadioEnabled")) {
			paramValue = json_object_object_get(radioJobj, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingStandards")) {
			paramValue = json_object_object_get(radioJobj, "OperatingStandards");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingFrequencyBand")) {
			paramValue = json_object_object_get(radioJobj, "OperatingFrequencyBand");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingChannelBandwidth")) {
			paramValue = json_object_object_get(radioJobj, "OperatingChannelBandwidth");
		}
		else if(!strcmp(paramList->name, "PossibleDataTransmitRates")) {
			paramValue = json_object_object_get(radioJobj, "SupportedDataTransmitRates");
		}
		else if(!strcmp(paramList->name, "PeerBSSID")) {
			paramValue = json_object_new_string("");
			freeflag = 1;
		}
		else if(!strcmp(paramList->name, "WMMSupported")) {
			paramValue = json_object_object_get(apJobj, "WMMCapability");
		}
		else if(!strcmp(paramList->name, "UAPSDSupported")) {
			paramValue = json_object_object_get(apJobj, "UAPSDCapability");
		}
		else if(!strcmp(paramList->name, "LocationDescription")) {
			paramValue = json_object_new_string("");
			freeflag = 1;
		}
		else if(!strcmp(paramList->name, "MACAddressControlEnabled")) {
			paramValue = json_object_new_boolean(false);
			freeflag = 1;
		}
		else if(!strcmp(paramList->name, "Standard")) {
			char opStandards[16];

			strcpy(opStandards, json_object_get_string(json_object_object_get(radioJobj, "OperatingStandards")));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
			paramValue = json_object_object_get(radioJobj, "OperatingStandards");
#else
			if (strcmp(opStandards, "g") == 0) {
				paramValue = json_object_new_string("g-only");
				freeflag = 1;
			}
			else if (strcmp(opStandards, "b,g") == 0) {
				paramValue = json_object_new_string("g");
				freeflag = 1;
			}
			else if (strcmp(opStandards, "b,g,n") == 0) {
				paramValue = json_object_new_string("n");
				freeflag = 1;
			}
			else {
				paramValue = json_object_object_get(radioJobj, "OperatingStandards");
			}
#endif
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Auto_Channel_Skip")) {
			paramValue = json_object_object_get(radioJobj, "X_ZYXEL_Auto_Channel_Skip");
		}
#ifdef ABUU_CUSTOMIZATION
		else if(!strcmp(paramList->name, "X_ZYXEL_AirtimeFairnes")) {
			paramValue = json_object_object_get(radioJobj, "X_ZYXEL_AirtimeFairnes");
		}
#endif
		else if(!strcmp(paramList->name, "X_ZYXEL_RIFS_Advertisement")) {
			paramValue = json_object_object_get(radioJobj, "X_ZYXEL_RIFS_Advertisement");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_SingleSKU")) {
			paramValue = json_object_object_get(radioJobj, "X_ZYXEL_SingleSKU");
		}
		else if(!strcmp(paramList->name, "WEPKeyIndex")) {
			int defaultKeyIndex = json_object_get_int(json_object_object_get(apSecJobj, "X_ZYXEL_DefaultKeyID"));
			defaultKeyIndex = (defaultKeyIndex == 0) ? 1 : defaultKeyIndex;
			paramValue = json_object_new_int(defaultKeyIndex);
			freeflag = 1;
		}
#if defined(ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE) || defined(ZYXEL_WIFI_SUPPORT_WPA_ONLY_MODE)
		else if(!strcmp(paramList->name, "WPAEncryptionModes")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_WPAEncryption");
		}
		else if(!strcmp(paramList->name, "WPAAuthenticationMode")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_WPAAuthentication");
		}
#endif
		else if(!strcmp(paramList->name, "IEEE11iEncryptionModes")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_11iEncryption");
		}
		else if(!strcmp(paramList->name, "IEEE11iAuthenticationMode")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_11iAuthentication");
		}
#ifdef SUPPORT_WL_WPA_EAP
		else if(!strcmp(paramList->name, "X_ZYXEL_RadiusServerIPAddr")) {
			paramValue = json_object_object_get(apSecJobj, "RadiusServerIPAddr");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_RadiusServerPort")) {
			paramValue = json_object_object_get(apSecJobj, "RadiusServerPort");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_RadiusSecret")) {
			paramValue = json_object_object_get(apSecJobj, "RadiusSecret");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Preauth")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_Preauth");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_ReauthInterval")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_ReauthInterval");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_RekeyingInterval")) {
			paramValue = json_object_object_get(apSecJobj, "RekeyingInterval");
		}
#endif
		else if(!strcmp(paramList->name, "BasicAuthenticationMode")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_WEPAuthentication");
		}
		else if(!strcmp(paramList->name, "BasicEncryptionModes")) {
			paramValue = json_object_object_get(apSecJobj, "X_ZYXEL_WEPEncryption");
		}
		else if(!strcmp(paramList->name, "BeaconType")) {
			paramValue = json_object_object_get(apSecJobj, "ModeEnabled");
			if(paramValue != NULL) {
				if(strstr(json_object_get_string(paramValue), "WPA-WPA2") != NULL) {
					paramValue = json_object_new_string("WPAand11i");
				}
				else if(strstr(json_object_get_string(paramValue), "WPA3-Personal-Transition") != NULL) {
					paramValue = json_object_new_string("11iandWPA3");
				}
				else if(strstr(json_object_get_string(paramValue), "WPA3-Personal") != NULL) {
					paramValue = json_object_new_string("WPA3");
				}
				else if(strstr(json_object_get_string(paramValue), "WPA2") != NULL) {
					paramValue = json_object_new_string("11i");
				}
				else if(strstr(json_object_get_string(paramValue), "WPA") != NULL) {
					paramValue = json_object_new_string("WPA");
				}
				else {
					paramValue = json_object_new_string("Basic");
				}
				freeflag = 1;
			}
		}
		else if(!strcmp(paramList->name, "AuthenticationServiceMode")) {
			paramValue = json_object_new_string("None");
			freeflag = 1;
		}
		else if(!strcmp(paramList->name, "BeaconAdvertisementEnabled")) {
			paramValue = json_object_object_get(ssidJobj, "Enable");
		}
#if 0
		else if(!strcmp(paramList->name, "AutoRateFallBackEnabled")) {
			paramValue = json_object_new_string("true");
			freeflag = 1;
		}
#endif
		else if(!strcmp(paramList->name, "WEPEncryptionLevel")) {
			char wepEncryp[32] = {0};

			paramValue = json_object_new_string("Disabled,40-bit,104-bit");
			freeflag = 1;
		}
#ifdef ZYXEL_WLAN_BLOCK_OPEN	
		else if(!strcmp(paramList->name, "X_ZYXEL_WlanBlockEnable")) {
			paramValue = json_object_object_get(wifiJobj, "X_ZYXEL_WlanBlockEnable");
		}
#endif	
#ifdef ZYXEL_WLAN_WORK_24_HOURS	
		else if(!strcmp(paramList->name, "X_ZYXEL_Work24HoursEnable")) {
			paramValue = json_object_object_get(wifiJobj, "X_ZYXEL_Work24HoursEnable");
		}
#endif
#ifdef ZYXEL_CONFIG_MAX_WLAN_CLIENT	
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxAssociatedDevices")) {
			paramValue = json_object_object_get(wifiJobj, "X_ZYXEL_MaxAssociatedDevices");
		}
#endif	
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			if(freeflag) json_object_put(paramValue);
			continue;
		}

		paramValue = NULL;
		/*End of special case*/

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}

	json_object_put(radioJobj);
	json_object_put(ssidStatsJobj);
	json_object_put(ssidJobj);
	json_object_put(apJobj);
	json_object_put(apSecJobj);
	json_object_put(wifiJobj);

	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.Stats
    Related object in TR181: Device.WiFi.SSID.i.Stats.
*/
zcfgRet_t lanDevWlanCfgStatObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181PathName[128] = {0};
	char tr181Obj[128] = {0};
	uint32_t  wlanCfgStatOid = 0;
	objIndex_t wlanCfgStatIid;
	struct json_object *wlanCfgStatJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	char ifName[16] = {0};
	struct json_object *ssidJobj = NULL;
	char tmp[256] = {0}, fileName[50] = {0}, *pch = NULL;
	FILE *fp = NULL;
	unsigned int txretrans = 0;
	unsigned char QBSS[5];
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181PathName)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get TR181 mapping name fail.\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(wlanCfgStatIid);
	sprintf(tr181Obj, "Device.%s", tr181PathName);
	wlanCfgStatOid = zcfgFeObjNameToObjId(tr181Obj, &wlanCfgStatIid);
	if((ret = feObjJsonGet(wlanCfgStatOid, &wlanCfgStatIid, &wlanCfgStatJobj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get wlanCfgStatJobj fail.\n", __FUNCTION__);
		return ret;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	/* Get Interface Name */
	if( zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &wlanCfgStatIid, &ssidJobj) == ZCFG_SUCCESS) {
		strncpy(ifName, json_object_get_string(json_object_object_get(ssidJobj, "Name")), sizeof(ifName));
		json_object_put(ssidJobj);
	}
#endif

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(wlanCfgStatJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		/* X_TTNET_RetransCount */
		if(!strncmp(paramList->name, "X_TTNET_RetransCount", 20)) {
			snprintf(fileName, sizeof(fileName), "/tmp/%s_counters", ifName);
			snprintf(tmp, sizeof(tmp), "wlctl -i %s counters > %s", ifName, fileName);
			system(tmp);

			if((fp = fopen(fileName , "r")) != NULL){
				while(fgets(tmp, sizeof(tmp)-1, fp) != NULL) {
					if((pch = strstr(tmp, "txretrans")) != NULL) {
						sscanf (pch,"txretrans %u", &txretrans);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(txretrans));
						break;
					}
				}
				fclose(fp);
				unlink(fileName);
			}
		}

		/* X_TTNET_ChannelUtilization */
		if(!strncmp(paramList->name, "X_TTNET_ChannelUtilization", 20)) {
			snprintf(fileName, sizeof(fileName), "/tmp/%s_beaconinfo", ifName);
			snprintf(tmp, sizeof(tmp), "wlctl -i %s beacon_info > %s", ifName, fileName);
			system(tmp);

			if((fp = fopen(fileName , "r")) != NULL){
				while(fgets(tmp, sizeof(tmp)-1, fp) != NULL) {
					if(strstr(tmp, "QBSS Load")) {
						if(fgets(tmp, sizeof(tmp)-1, fp) != NULL) {
							sscanf(tmp, "%hhx %hhx %hhx %hhx %hhx",
								&(QBSS[0]), &(QBSS[1]), &(QBSS[2]), &(QBSS[3]), &(QBSS[4]));
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int((int)QBSS[2]) );
							break;
						}
					}
				}
				fclose(fp);
				unlink(fileName);
			}
		}
#endif

		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

	json_object_put(wlanCfgStatJobj);
	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*
 *  called by lanDevWlanCfgObjSet with CONFIG_ZCFG_BE_MODULE_WIFI compile flag
 */
static zcfgRet_t wifiSecuritySet(char index, struct json_object *apSecJobj, struct json_object *tr98Jobj, struct json_object *beaconType, int wepKeyLen, struct json_object *ssidJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char type[12] = {0};
	struct json_object *tr98ParamValue = NULL;
	const char *modeEnabled = json_object_get_string(json_object_object_get(apSecJobj, "ModeEnabled"));
	const char *modesSupported = json_object_get_string(json_object_object_get(apSecJobj, "ModesSupported"));
	const char *tr98ParmStr_1 = NULL, *tr98ParmStr_2 = NULL;
	const char *tr181AuthStr = NULL, *tr98Auth11iStr = NULL, *tr98AuthWPAStr = NULL;
	int modifyAuth = 0, tr181Auth = 0, tr98Auth11i = 0, tr98AuthWPA = 0;
	strcpy(type, json_object_get_string(beaconType));

	if(!strcmp(type, "Basic")) {
		const char *basicEncryptModes = NULL, *basicAuthMode = NULL;

		basicAuthMode = json_object_get_string(json_object_object_get(tr98Jobj, "BasicAuthenticationMode"));
		basicEncryptModes = json_object_get_string(json_object_object_get(tr98Jobj, "BasicEncryptionModes"));

		if((basicAuthMode && !strcmp(basicAuthMode, "None")) &&
			(basicEncryptModes && !strcmp(basicEncryptModes, "None")))
			json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("None"));
#if defined(ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE) || defined(ZYXEL_WIFI_SUPPORT_WPA_ONLY_MODE)
		else if(wepKeyLen == 5 || wepKeyLen == 10){
			json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WEP-64"));
		}
		else if(wepKeyLen == 0 || wepKeyLen == 13 || wepKeyLen == 26){
			json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WEP-128"));
		}
#endif
	}
#if defined(ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE) || defined(ZYXEL_WIFI_SUPPORT_WPA_ONLY_MODE)
	else if(!strcmp(type, "WPA")) {
		tr98ParamValue = json_object_object_get(tr98Jobj, "WPAAuthenticationMode");
		if(tr98ParamValue) {
			if(!strcmp(json_object_get_string(tr98ParamValue), "EAPAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-Enterprise"));
			}
			else if(!strcmp(json_object_get_string(tr98ParamValue), "PSKAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-Personal"));
			}
			else
				return ZCFG_INVALID_PARAM_VALUE;
		}

		tr98ParamValue = json_object_object_get(tr98Jobj, "WPAEncryptionModes");
		if(tr98ParamValue) {
			if(!strcmp(json_object_get_string(tr98ParamValue), "TKIPandAESEncryption")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkipaes"));
			}
#ifndef CONFIG_TAAAB_CUSTOMIZATION_TR069
			else if(!strcmp(json_object_get_string(tr98ParamValue), "TKIPEncryption")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkip"));
			}
#endif
			else if(!strcmp(json_object_get_string(tr98ParamValue), "AESEncryption")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("aes"));
		}
			else
				return ZCFG_INVALID_PARAM_VALUE;
		}
	}
#endif
	else if(!strcmp(type, "11i")) {
		int apAuth = 0;

		tr98ParmStr_1 = json_object_get_string(json_object_object_get(tr98Jobj, "IEEE11iAuthenticationMode"));

		if(strstr(modeEnabled, "WPA2-Enterprise") || strstr(modeEnabled, "WPA-WPA2-Enterprise"))
			apAuth = WPA2_ENTERPRISE;
		else if(strstr(modeEnabled, "WPA2-Personal") || strstr(modeEnabled, "WPA-WPA2-Personal"))
			apAuth = WPA2_PERSONAL;

		if(apAuth == WPA2_PERSONAL) {
			if(!strcmp(tr98ParmStr_1, "EAPAuthentication")) {
#ifndef SUPPORT_WL_WPA_EAP
				if((index == WIFI_24_PRIMARY_LINK) || (index == WIFI_5_PRIMARY_LINK))
				{
					return ZCFG_INVALID_PARAM_VALUE;
				}
#endif
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Enterprise"));
			}else json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
		}else if(apAuth == WPA2_ENTERPRISE) {
			if(!strcmp(tr98ParmStr_1, "PSKAuthentication"))
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
			else json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Enterprise"));
		}else {
			if(!strcmp(tr98ParmStr_1, "EAPAuthentication")) {
#ifndef SUPPORT_WL_WPA_EAP
				if((index == WIFI_24_PRIMARY_LINK) || (index == WIFI_5_PRIMARY_LINK))
				{
					return ZCFG_INVALID_PARAM_VALUE;
				}
#endif					

				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Enterprise"));
			}else if(!strcmp(tr98ParmStr_1, "PSKAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
			}
			else {
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}

		tr98ParmStr_1 = json_object_get_string(json_object_object_get(tr98Jobj, "IEEE11iEncryptionModes"));
		if((strlen(tr98ParmStr_1) > 0) && strcmp(tr98ParmStr_1, "AESEncryption"))
			return ZCFG_INVALID_PARAM_VALUE;

		if(!strcmp(tr98ParmStr_1, "AESEncryption"))
			json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("aes"));
	}
	else if(!strcmp(type, "WPA3")) {
		if(strstr(modesSupported, "WPA3") !=NULL) {
			json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA3-Personal"));
		}
		else {
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	else if(!strcmp(type, "11iandWPA3")) {
		if(strstr(modesSupported, "WPA3") !=NULL) {
			json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA3-Personal-Transition"));
		}
		else {
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}

#if defined(ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE) || defined(ZYXEL_WIFI_SUPPORT_WPA_ONLY_MODE)
	else if(!strcmp(type, "WPAand11i")) {
		int apAuth = 0, apEncryp = 0;

		tr98ParmStr_1 = json_object_get_string(json_object_object_get(tr98Jobj, "IEEE11iAuthenticationMode"));
		tr98ParmStr_2 = json_object_get_string(json_object_object_get(tr98Jobj, "WPAAuthenticationMode"));

		if(strstr(modeEnabled, "WPA2-Personal") || strstr(modeEnabled, "WPA-WPA2-Personal"))
			apAuth = WPA2_PERSONAL;
#ifdef ACAU_CUSTOMIZATION
		else if(strstr(modeEnabled, "WPA2-Enterprise") || strstr(modeEnabled, "WPA-WPA2-Enterprise"))
			apAuth = WPA2_ENTERPRISE;
#endif

		if(apAuth == WPA2_PERSONAL) {
			if(!strcmp(tr98ParmStr_1, "EAPAuthentication") || !strcmp(tr98ParmStr_2, "EAPAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Enterprise"));
			}else json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
		}
#ifdef ACAU_CUSTOMIZATION
		else if(apAuth == WPA2_ENTERPRISE) {
			if(!strcmp(tr98ParmStr_1, "PSKAuthentication") || !strcmp(tr98ParmStr_2, "PSKAuthentication"))
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
			else json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Enterprise"));
		}
#endif
		else {
			if(!strcmp(tr98ParmStr_1, "EAPAuthentication") && !strcmp(tr98ParmStr_2, "PSKAuthentication"))
				return ZCFG_INVALID_PARAM_VALUE;
			if(!strcmp(tr98ParmStr_1, "PSKAuthentication") && !strcmp(tr98ParmStr_2, "EAPAuthentication"))
				return ZCFG_INVALID_PARAM_VALUE;
			if(!strcmp(tr98ParmStr_1, "PSKAuthentication") || !strcmp(tr98ParmStr_2, "PSKAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
			}
#ifdef ACAU_CUSTOMIZATION
			else if(!strcmp(tr98ParmStr_1, "EAPAuthentication") || !strcmp(tr98ParmStr_2, "EAPAuthentication")) {
				json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WPA-WPA2-Enterprise"));
			}
#endif
			else {
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}


		tr98ParmStr_1 = json_object_get_string(json_object_object_get(tr98Jobj, "IEEE11iEncryptionModes"));
		tr98ParmStr_2 = json_object_get_string(json_object_object_get(tr98Jobj, "WPAEncryptionModes"));

		if(!strcmp(json_object_get_string(json_object_object_get(apSecJobj, "X_ZYXEL_Encryption")), "aes")){
			apEncryp = AES;
		}
		else if(!strcmp(json_object_get_string(json_object_object_get(apSecJobj, "X_ZYXEL_Encryption")), "tkipaes")){
			apEncryp = TKIPAES;
		}
		else if(!strcmp(json_object_get_string(json_object_object_get(apSecJobj, "X_ZYXEL_Encryption")), "tkip"))
			apEncryp = TKIP;

		if(apEncryp == AES) {
			if(!strcmp(tr98ParmStr_1, "TKIPandAESEncryption") || !strcmp(tr98ParmStr_2, "TKIPandAESEncryption")){
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkipaes"));
			}
			else if(!strcmp(tr98ParmStr_1, "TKIPEncryption") || !strcmp(tr98ParmStr_2, "TKIPEncryption"))
			{
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				return ZCFG_INVALID_PARAM_VALUE;
#else
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkip"));
#endif
			}
		}else if(apEncryp == TKIPAES) {
			if(!strcmp(tr98ParmStr_1, "AESEncryption") || !strcmp(tr98ParmStr_2, "AESEncryption"))
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("aes"));
				else if(!strcmp(tr98ParmStr_1, "TKIPEncryption") || !strcmp(tr98ParmStr_2, "TKIPEncryption"))
			{
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				return ZCFG_INVALID_PARAM_VALUE;
#else
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkip"));
#endif
			}
				}
		else if(apEncryp == TKIP) {
			if(!strcmp(tr98ParmStr_1, "AESEncryption") || !strcmp(tr98ParmStr_2, "AESEncryption"))
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("aes"));
			else if(!strcmp(tr98ParmStr_1, "TKIPandAESEncryption") || !strcmp(tr98ParmStr_2, "TKIPandAESEncryption"))
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkipaes"));
		}

		else {
			if(!strcmp(tr98ParmStr_1, "AESEncryption")) 
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("aes"));
			else if(!strcmp(tr98ParmStr_1, "TKIPandAESEncryption"))
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkipaes"));
				else if(!strcmp(tr98ParmStr_1, "TKIPEncryption"))
			{
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				return ZCFG_INVALID_PARAM_VALUE;
#else
				json_object_object_add(apSecJobj, "X_ZYXEL_Encryption", json_object_new_string("tkip"));
#endif
			}
		}
	}
#endif
	else if(!strcmp(type, "None")) {
		json_object_object_add(ssidJobj, "Enable", json_object_new_boolean(false));
	}
	else {
		ret = ZCFG_INVALID_PARAM_VALUE;
	}

	return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#define WIFI24_FIRST_AP  1
#if defined(TOTAL_INTERFACE_24G)
#define WIFI24_LAST_AP  TOTAL_INTERFACE_24G
#else
#define WIFI24_LAST_AP  4
#endif

#define WIFI5_FIRST_AP  (WIFI24_LAST_AP+1)
#if defined(TOTAL_INTERFACE_5G)
#define WIFI5_LAST_AP  (WIFI24_LAST_AP+TOTAL_INTERFACE_5G)
#else
#define WIFI5_LAST_AP  8
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
typedef enum {
	CHAN_OK=0,     //channel is ok, it doesn't require identifying Lower or Upper
	CHAN_UPPER_OK, //primary channel becomes Lower channel
	CHAN_LOWER_OK, //primary channel becomes Upper channel
	CHAN_NOT_OK,   //internal error
} wlanChanStat;

/*
 *  called by lanDevWlanConfirmRadio only
 */
//check if channel is available in channel list based on bandwidth --- ChihYuanYang
static bool WifiCheckChannelAvailable(const char *ch_list, const int channel)
{
	if(ch_list == NULL) return false;

	char channel_list[129] = {0};
	char *tmp_channel = NULL, *tmp_next = NULL;
	bool found = false;

	strcpy(channel_list, ch_list);
	tmp_channel = strtok_r(channel_list, ",", &tmp_next);
	while(tmp_channel != NULL){
		if(channel == atoi(tmp_channel)){
			found = true;
			break;
		}
		tmp_channel = strtok_r(NULL, ",", &tmp_next);
	}

	return found;
}

/*
 *  called by lanDevWlanCfgObjSet with CONFIG_ZCFG_BE_MODULE_WIFI compile flag
 */
wlanChanStat lanDevWlanConfirmRadio(objIndex_t *apIid, struct json_object *tr98Jobj, struct json_object *radioJobj)
{
	const char *beaconType = json_object_get_string(json_object_object_get(tr98Jobj, "BeaconType"));

	int tr181Channel = json_object_get_int(json_object_object_get(radioJobj, "Channel"));
	bool tr98AutoChannel = json_object_get_boolean(json_object_object_get(tr98Jobj, "AutoChannelEnable"));
	int tr98SetChannel = json_object_get_int(json_object_object_get(tr98Jobj, "Channel"));

	const char *tr181OpBandwidth = json_object_get_string(json_object_object_get(radioJobj, "OperatingChannelBandwidth"));
	const char *opBand = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_OperatingFrequencyBand"));
	const char *opBandwidth = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_OperatingChannelBandwidth"));
	const char *ExtChan = json_object_get_string(json_object_object_get(radioJobj, "ExtensionChannel"));
	const char *PossiChan = json_object_get_string(json_object_object_get(radioJobj, "PossibleChannels"));
	const char *LowerChan = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_LowerChannels"));
	const char *UpperChan = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_UpperChannels"));
	const char *BW80ChanList = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_AC80_Channels"));
	const char *BW160ChanList = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_AX160_Channels"));
	char chanlist[128] = {0};
	bool found = false;

	if(!strcmp(beaconType, "Basic")) {
		const char *basicAuthMode = json_object_get_string(json_object_object_get(tr98Jobj, "BasicAuthenticationMode"));
		const char *basicEncryptModes = json_object_get_string(json_object_object_get(tr98Jobj, "BasicEncryptionModes"));

		if(( (basicAuthMode && strcmp(basicAuthMode, "None")) || (basicEncryptModes && strcmp(basicEncryptModes, "None")) )
				&& (apIid->idx[0] == WIFI24_FIRST_AP))
			json_object_object_add(tr98Jobj, "X_ZYXEL_OperatingChannelBandwidth", json_object_new_string("20MHz"));
		else if(( (basicAuthMode && strcmp(basicAuthMode, "None")) || (basicEncryptModes && strcmp(basicEncryptModes, "None")) )
				&& (apIid->idx[0] == WIFI5_FIRST_AP))
			json_object_object_add(tr98Jobj, "X_ZYXEL_OperatingChannelBandwidth", json_object_new_string("20MHz"));
		else if(strcmp(opBandwidth, tr181OpBandwidth) && (apIid->idx[0] != WIFI24_FIRST_AP) && (apIid->idx[0] != WIFI5_FIRST_AP))
			return CHAN_NOT_OK;
	}
	else if(strcmp(opBandwidth, tr181OpBandwidth) && (apIid->idx[0] != WIFI24_FIRST_AP) && (apIid->idx[0] != WIFI5_FIRST_AP))
		return CHAN_NOT_OK;

	opBandwidth = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_OperatingChannelBandwidth"));

	if(tr98AutoChannel)
		return CHAN_OK;

	if(!strcmp(opBand, "2.4GHz")) {

		if(!strcmp(opBandwidth, "20MHz")) {
			strcpy(chanlist, PossiChan);
		}
		else if(!strcmp(opBandwidth, "40MHz")) {
			if(!strcmp(ExtChan, "BelowControlChannel")){
				strcpy(chanlist, UpperChan);
			}
			else if(!strcmp(ExtChan, "AboveControlChannel")){
				strcpy(chanlist, LowerChan);
			}
		}
		else
			return CHAN_NOT_OK;

		if(tr181Channel != tr98SetChannel){
			if(apIid->idx[0] > WIFI24_FIRST_AP && apIid->idx[0] <= WIFI24_LAST_AP){
				return CHAN_NOT_OK;
			}

			if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
				return CHAN_OK;
			}
			else if(!strcmp(opBandwidth, "40MHz")){
				// check channel is available for another channel list
				if(!strcmp(ExtChan, "BelowControlChannel")){
					strcpy(chanlist, LowerChan);
					if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
						return CHAN_LOWER_OK;
					}
					else{						
						return CHAN_NOT_OK;
					}
				}
				else if(!strcmp(ExtChan, "AboveControlChannel")){
					strcpy(chanlist, UpperChan);
					if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
						return CHAN_UPPER_OK;
					}
					else{
						return CHAN_NOT_OK;
					}					
				}
				else{
					return CHAN_NOT_OK;
				}
			}
			else{
				return CHAN_NOT_OK;
			}
		}
	}
	else if(!strcmp(opBand, "5GHz")) {

		if(!strcmp(opBandwidth, "20MHz")) {
			strcpy(chanlist, PossiChan);
		}
		else if(!strcmp(opBandwidth, "40MHz")) {
			if(!strcmp(ExtChan, "BelowControlChannel")){
				strcpy(chanlist, UpperChan);
			}
			else if(!strcmp(ExtChan, "AboveControlChannel")){
				strcpy(chanlist, LowerChan);
			}
			else{
				return CHAN_NOT_OK;
			}
		}
		else if(!strcmp(opBandwidth, "80MHz")) {
			strcpy(chanlist, BW80ChanList);
		}
		else if(!strcmp(opBandwidth, "160MHz")) {
			strcpy(chanlist, BW160ChanList);
		}
		else
			return CHAN_NOT_OK;

		if(tr181Channel != tr98SetChannel) {
			if(apIid->idx[0] > WIFI5_FIRST_AP && apIid->idx[0] <= WIFI5_LAST_AP)
				return CHAN_NOT_OK;

			if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
				return CHAN_OK;
			}
			else if(!strcmp(opBandwidth, "40MHz")){
				// check channel is available for another channel list
				if(!strcmp(ExtChan, "BelowControlChannel")){
					strcpy(chanlist, LowerChan);
					if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
						return CHAN_LOWER_OK;
					}
					else{
						return CHAN_NOT_OK;
					}
				}
				else if(!strcmp(ExtChan, "AboveControlChannel")){
					strcpy(chanlist, UpperChan);
					if(WifiCheckChannelAvailable(chanlist, tr98SetChannel)){
						return CHAN_UPPER_OK;
					}
					else{
						return CHAN_NOT_OK;
					}
				}
				else{
					return CHAN_NOT_OK;
				}
			}
			else{
				return CHAN_NOT_OK;
			}
		}
	}

	return CHAN_OK;
}
/*
 *  called by lanDevWlanCfgObjSet with CONFIG_ZCFG_BE_MODULE_WIFI compile flag
 */
zcfgRet_t lanDevWlanConfirmBW(struct json_object *tr98Jobj, struct json_object *radioJobj)
{
	const char *opBandwidth = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_OperatingChannelBandwidth"));
	const char *BW80ChanList = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_AC80_Channels"));
	const char *BW160ChanList = json_object_get_string(json_object_object_get(radioJobj, "X_ZYXEL_AX160_Channels"));

	if(!strcmp(opBandwidth, "80MHz")){
		if(!strcmp(BW80ChanList, ""))
			return ZCFG_INVALID_PARAM_VALUE;
	}
	else if(!strcmp(opBandwidth, "160MHz")){
		if(!strcmp(BW160ChanList, ""))
			return ZCFG_INVALID_PARAM_VALUE;
	}
	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI


#ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#define MAX_WEPKEY_NUM   4

/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i
 *
 *   Related object in TR181:
 *
 *   Device.WiFi.Radio.i
 *   Device.WiFi.SSID.i
 *   Device.WiFi.AccessPoint.i
 *   Device.WiFi.AccessPoint.i.Security
 */
zcfgRet_t lanDevWlanCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char wifiSsid[32] = {0};
	char wifiRadio[32] = {0};
	char ssidRef[32] = {0};
	char ExtChan40M[32] = {0};
	objIndex_t ssidIid, radioIid, apIid;
	objIndex_t wifiIid;
	struct json_object *wifiJobj = NULL;
	struct json_object *radioJobj = NULL;
	struct json_object *ssidJobj = NULL, *ssidJobjBak = NULL;
	struct json_object *apJobj = NULL, *apJobjBak = NULL;
	struct json_object *apSecJobj = NULL, *apSecJobjBak = NULL, *apWpsJobj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	int wepKeyLen = 0, wepKey64Len = 0, chan = 0, ssid_attr = 0;
	wlanChanStat ret_chan;
	struct json_object *wepKey = NULL, *wepKey64 = NULL;
	bool apWpsEnable = false;
	struct json_object *paramGuardObj = NULL;
	objIndex_t wpsIid;
	bool backhaulSsidVisalbe = false;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	char bands[32] = {0};
	char tmpRadio[32] = {0};
	objIndex_t tmpIid, onessidIid;
	struct json_object *tmpJobj = NULL;
	struct json_object *onessidJobj = NULL;
	bool onessid = false;
	bool mainssid = false;

	int guest = 0, main = 0;
#endif

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, wifiSsid) != ZCFG_SUCCESS) {
		/*  The instance number of WLANConfiguration.i will be continuous because of the tr98 to tr181 mapping table.
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WLANConfiguration object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}

	/*mapping InternetGatewayDevice.LANDevice.1.WLANConfiguration.i to Device.WiFi.SSID.i*/
	IID_INIT(wifiIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI, &wifiIid, &wifiJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi Fail\n", __FUNCTION__);
		return ret;
	}
	backhaulSsidVisalbe = json_object_get_boolean(json_object_object_get(wifiJobj, "X_ZYXEL_BackhaulSSIDVisable"));

	IID_INIT(ssidIid);
	ssidIid.level = 1;
	sscanf(wifiSsid, "WiFi.SSID.%hhu", &ssidIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &ssidIid, &ssidJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID Fail\n", __FUNCTION__);
		return ret;
	}

	if(!backhaulSsidVisalbe)
	{
		ssid_attr = json_object_get_int(json_object_object_get(ssidJobj, "X_ZYXEL_Attribute"));
		if(ssid_attr != 0){
			json_object_put(ssidJobj);
			return ZCFG_INVALID_OBJECT;
		}
	}

	strcpy(wifiRadio, json_object_get_string(json_object_object_get(ssidJobj, "LowerLayers")));
	IID_INIT(radioIid);
	radioIid.level = 1;
	sscanf(wifiRadio, "WiFi.Radio.%hhu", &radioIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &radioIid, &radioJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.Radio Fail\n", __FUNCTION__);
		json_object_put(ssidJobj);
		return ret;
	}

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	IID_INIT(onessidIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &onessidIid, &onessidJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get ONESSID Fail\n", __FUNCTION__);
		json_object_put(ssidJobj);
		json_object_put(radioJobj);
		return ret;
	}
	onessid = json_object_get_boolean(json_object_object_get(onessidJobj, "Enable"));
	strcpy(bands, json_object_get_string(json_object_object_get(radioJobj, "SupportedFrequencyBands")));
	zcfgFeJsonObjFree(onessidJobj);


	if(onessid && !strcmp(bands,"5GHz")){
		IID_INIT(tmpIid);
		while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &tmpIid, &tmpJobj)) == ZCFG_SUCCESS) {
			strcpy(tmpRadio, json_object_get_string(json_object_object_get(tmpJobj, "LowerLayers")));
			mainssid = json_object_get_boolean(json_object_object_get(tmpJobj, "X_ZYXEL_MainSSID"));
			if(mainssid && !strcmp(tmpRadio,wifiRadio)){
				guest++;
				json_object_put(tmpJobj);
				break;
			}
			else{
				json_object_put(tmpJobj);
				tmpJobj = NULL;
				guest++;
			}
		}
		main = guest++;

		if(ssidIid.idx[0]==main || ssidIid.idx[0]==guest){//5G main,5G guest 1
			struct json_object *spvObj = NULL;

			if (multiJobj) {
				spvObj = json_object_object_get(multiJobj, "spv");
				//printf("-- %s.%d spvObj:\n%s\n", __func__, __LINE__, json_object_to_json_string(spvObj));
				if (json_object_object_get(spvObj, "SSID") || json_object_object_get(spvObj, "SSIDAdvertisementEnabled") || json_object_object_get(spvObj, "IsolationEnable") ||
					json_object_object_get(spvObj, "X_ZYXEL_BssIdIsolation") || json_object_object_get(spvObj, "ModeEnabled") || json_object_object_get(spvObj, "X_ZYXEL_AutoGenPSK") ||
					json_object_object_get(spvObj, "KeyPassphrase") || json_object_object_get(spvObj, "PreSharedKey") || json_object_object_get(spvObj, "X_ZYXEL_Encryption") ||
					json_object_object_get(spvObj, "RekeyingInterval")) {
					json_object_put(ssidJobj);
					json_object_put(radioJobj);
					return ZCFG_REQUEST_REJECT;
				}
			}
		}
	}
#endif

	IID_INIT(apIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret != ZCFG_SUCCESS) {
		json_object_put(radioJobj);
		json_object_put(ssidJobj);
		return ZCFG_NO_MORE_INSTANCE;
	}
	// Sync with WebGUI
	IID_INIT(wpsIid);
	wpsIid.level = apIid.level;
	if(apIid.idx[0] >= WIFI24_FIRST_AP && apIid.idx[0] <= WIFI24_LAST_AP) {
		wpsIid.idx[0] = WIFI24_FIRST_AP;
	}
	else if(apIid.idx[0] >= WIFI5_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP) {
		wpsIid.idx[0] = WIFI5_FIRST_AP;
	}
	else {
		printf("%s: wrong access point seq num: %hhu\n", __FUNCTION__, apIid.idx[0]);
		json_object_put(radioJobj);
		json_object_put(ssidJobj);
		return ZCFG_INTERNAL_ERROR;
	}

	if(apJobj != NULL) {
		int defaultKeyIndex = 0;
		char keyParamName[20] = {0};

		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(radioJobj);
			json_object_put(ssidJobj);
			json_object_put(apJobj);
			return ret;
		}

		/*Backup for recover*/
		apJobjBak = json_tokener_parse(json_object_to_json_string(apJobj));
		apSecJobjBak = json_tokener_parse(json_object_to_json_string(apSecJobj));

		defaultKeyIndex = json_object_get_int(json_object_object_get(apSecJobj, "X_ZYXEL_DefaultKeyID"));
		defaultKeyIndex = (defaultKeyIndex > 0 && defaultKeyIndex <=MAX_WEPKEY_NUM) ? defaultKeyIndex : 1;
		if(defaultKeyIndex == 1)
			strcpy(keyParamName, "WEPKey");
		else
			sprintf(keyParamName, "X_ZYXEL_WEPKey%d", defaultKeyIndex);

		wepKey = json_object_object_get(apSecJobj, keyParamName);
		wepKeyLen = wepKey ? strlen(json_object_get_string(wepKey)) : 0;

		sprintf(keyParamName, "X_ZYXEL_WEP64Key%d", defaultKeyIndex);

		wepKey64 = json_object_object_get(apSecJobj, keyParamName);
		wepKey64Len = wepKey64 ? strlen(json_object_get_string(wepKey64)) : 0;

		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &wpsIid, &apWpsJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.WPS Fail\n", __FUNCTION__);
			json_object_put(radioJobj);
			json_object_put(ssidJobj);
			json_object_put(apJobj);
			json_object_put(apSecJobj);
			return ret;
		}

		apWpsEnable = json_object_get_boolean(json_object_object_get(apWpsJobj, "Enable"));
		json_object_put(apWpsJobj);
	}
	
	if(multiJobj){
#if (defined(ZYXEL_WLAN_BLOCK_OPEN)  || defined(ZYXEL_WLAN_WORK_24_HOURS) || defined(ZYXEL_CONFIG_MAX_WLAN_CLIENT))
		tmpObj = wifiJobj;
		wifiJobj = NULL;
		wifiJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI, &wifiIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI, &wifiIid, "apply", json_object_new_boolean(true));
#endif
		
		tmpObj = radioJobj;
		radioJobj = NULL;
		radioJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_RADIO, &radioIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_RADIO, &radioIid, "apply", json_object_new_boolean(false));
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_RADIO, &radioIid, "block", json_object_new_boolean(false));
		if(apJobj != NULL) {
			tmpObj = apJobj;
			apJobj = NULL;
			apJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT, &apIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_ACCESS_POINT, &apIid, "apply", json_object_new_boolean(false));

			tmpObj = apSecJobj;
			apSecJobj = NULL;
			apSecJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE //US will sync sec when one-ssid enabled by SSID be, so must make passkey applied
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, "apply", json_object_new_boolean(true));
#else
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, "apply", json_object_new_boolean(false));
#endif
		}

		tmpObj = ssidJobj;
		ssidJobj = NULL;
		ssidJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_SSID, &ssidIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_SSID, &ssidIid, "apply", json_object_new_boolean(true));

		zcfgFeJsonMultiObjSetDelayApply(multiJobj);
	}

	/*Backup for recover*/
	ssidJobjBak = json_tokener_parse(json_object_to_json_string(ssidJobj));

	paramGuardObj = zcfgFe98ParamGuardInit((const char *)TR98_PARM_LAN_DEV_WLAN_CFG, NULL);
#ifdef ZYXEL_WLAN_MAXBITRATE /* Need MaxBitRate for OAAAA project, Bug #37690, Ian */
	int maxBitRateSet = 0;
#endif
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			continue;
		}

		zcfgRet_t paramSecured = zcfgFe98ParamGuard(paramGuardObj, (const char *)paramList->name, paramValue);
		if(paramSecured == ZCFG_INVALID_PARAM_VALUE){
			printf("%s: %s, invalid parameter value\n", __FUNCTION__, paramList->name);
			ret = paramSecured;
			goto finish;
		}

#ifdef ZYXEL_WLAN_MAXBITRATE /* Need MaxBitRate for OAAAA project, Bug #37690, Ian */
		if((paramValue != NULL) && !strcmp(paramList->name, "MaxBitRate")) {
			struct json_object *paramRate_1 = json_object_object_get(ssidJobj, "X_ZYXEL_UpstreamRate");
			struct json_object *paramRate_2 = json_object_object_get(ssidJobj, "X_ZYXEL_DownstreamRate");
			const char *t98maxBitRate = json_object_get_string(paramValue);
			uint32_t maxBitRate = 0;
			uint32_t rate_1 = 0;
			uint32_t rate_2 = 0;
			rate_1 = json_object_get_int(paramRate_1);
			rate_2 = json_object_get_int(paramRate_2);
			float maxBitRate_f = 0;
			if(strcmp(t98maxBitRate, "Auto")) maxBitRate_f = atof(t98maxBitRate);
			if(maxBitRate_f > WLAN_MAXBITRATE) maxBitRate_f = WLAN_MAXBITRATE;
			maxBitRate = maxBitRate_f * 1000;
			if(maxBitRate != rate_1 && maxBitRate != rate_2) {
			if(paramRate_1) {
				json_object_object_add(ssidJobj, "X_ZYXEL_UpstreamRate", json_object_new_int(maxBitRate));
			}
			if(paramRate_2) {
				json_object_object_add(ssidJobj, "X_ZYXEL_DownstreamRate", json_object_new_int(maxBitRate));
			}
				maxBitRateSet++;
			}
			paramList++;
			continue;
		}else if(!strcmp(paramList->name, "AutoRateFallBackEnabled")) {
			if(strcmp(json_object_get_string(paramValue), "true")) {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}
		}
#else
		if(!strcmp(paramList->name, "MaxBitRate")){
			json_object_object_add(radioJobj, "X_ZYXEL_MaxBitRate", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif

		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(ssidJobj, paramList->name);
			if(tr181ParamValue != NULL) {
#ifdef ZYXEL_WLAN_MAXBITRATE /* Need MaxBitRate for OAAAA project, Bug #37690, Ian */
				if(!strcmp(paramList->name, "X_ZYXEL_UpstreamRate") || !strcmp(paramList->name, "X_ZYXEL_DownstreamRate")) {
					if(maxBitRateSet) {
						paramList++;
						continue;
					}
				}
#endif
#ifdef ABUU_CUSTOMIZATION
				if(!strcmp(paramList->name, "X_ZYXEL_AirtimeFairnes")){
					json_object_object_add(radioJobj, "X_ZYXEL_AirtimeFairnes", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
#endif
				json_object_object_add(ssidJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "RadioEnabled")) {
			bool tr98RadioEnable = json_object_get_boolean(paramValue);
			bool tr181RadioEnable = json_object_get_boolean(json_object_object_get(radioJobj, "Enable"));
			if((tr98RadioEnable != tr181RadioEnable) && ((apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI24_LAST_AP) ||
				(apIid.idx[0] > WIFI5_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP)) ) {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}

			json_object_object_add(radioJobj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#ifdef ZYXEL_WLAN_BLOCK_OPEN
		else if(!strcmp(paramList->name, "X_ZYXEL_WlanBlockEnable")) {
			bool trnewEnable = json_object_get_boolean(paramValue);
			bool troldEnable = json_object_get_boolean(json_object_object_get(wifiJobj, "X_ZYXEL_WlanBlockEnable"));
			if((trnewEnable != troldEnable) && (apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP))  {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}

			json_object_object_add(wifiJobj, "X_ZYXEL_WlanBlockEnable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif
#ifdef ZYXEL_WLAN_WORK_24_HOURS
		else if(!strcmp(paramList->name, "X_ZYXEL_Work24HoursEnable")) {
			bool trnew24HoursEnable = json_object_get_boolean(paramValue);
			bool trold24HoursEnable = json_object_get_boolean(json_object_object_get(wifiJobj, "X_ZYXEL_Work24HoursEnable"));
			if((trnew24HoursEnable != trold24HoursEnable) && (apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP))  {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}

			json_object_object_add(wifiJobj, "X_ZYXEL_Work24HoursEnable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif	
#ifdef ZYXEL_CONFIG_MAX_WLAN_CLIENT
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxAssociatedDevices")) {
			int trNewMaxNum = json_object_get_int(paramValue);
			int trOldMaxNum = json_object_get_int(json_object_object_get(wifiJobj, "X_ZYXEL_MaxAssociatedDevices"));
			if(((trNewMaxNum != trOldMaxNum) && (apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP)) ||( trNewMaxNum > 64))  {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto finish;
			}

			json_object_object_add(wifiJobj, "X_ZYXEL_MaxAssociatedDevices", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingStandards")) {
			json_object_object_add(radioJobj, "OperatingStandards", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingFrequencyBand")) {
			json_object_object_add(radioJobj, "OperatingFrequencyBand", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OperatingChannelBandwidth")) {
			if((ret = lanDevWlanConfirmBW(tr98Jobj, radioJobj)) == ZCFG_INVALID_PARAM_VALUE){
				goto finish;
			}
			json_object_object_add(radioJobj, "OperatingChannelBandwidth", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else {
			if(!strcmp(paramList->name, "Channel")) {
				if((ret_chan = lanDevWlanConfirmRadio(&apIid, tr98Jobj, radioJobj)) == CHAN_NOT_OK){
					ret = ZCFG_INVALID_PARAM_VALUE;
					strcpy(paramfault, "Channel");
					goto finish;
				}
				else{
					ret = ZCFG_SUCCESS;
					if(ret_chan == CHAN_LOWER_OK){ //primary channel becomes lower, extension channel should be above
						strcpy(ExtChan40M, "AboveControlChannel");
						json_object_object_add(radioJobj, "ExtensionChannel", json_object_new_string(ExtChan40M));
					}
					else if(ret_chan == CHAN_UPPER_OK){ //vise versa
						strcpy(ExtChan40M, "BelowControlChannel");
						json_object_object_add(radioJobj, "ExtensionChannel", json_object_new_string(ExtChan40M));
					}
				}
			}
			tr181ParamValue = json_object_object_get(radioJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(radioJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(apJobj != NULL) {
			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(apJobj, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(apJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}

			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(apSecJobj, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(apSecJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}

			/*special case*/
			if(!strcmp(paramList->name, "BeaconType")) {
				int wepKeyLength = (wepKeyLen > wepKey64Len) ? wepKeyLen : wepKey64Len;
				if((ret = wifiSecuritySet(ssidIid.idx[0], apSecJobj, tr98Jobj, paramValue, wepKeyLength, ssidJobj)) != ZCFG_SUCCESS) {
					goto finish;
				}
				
				const char *modeEnabled = json_object_get_string(json_object_object_get(apSecJobj, "ModeEnabled"));
#if 0
				if(apWpsEnable && ((ssidIid.idx[0] == WIFI_24_PRIMARY_LINK) || (ssidIid.idx[0] == WIFI_5_PRIMARY_LINK))) {
					const char *beaconType = json_object_get_string(paramValue);
					const char *modeEnabled = json_object_get_string(json_object_object_get(apSecJobj, "ModeEnabled"));
					if(!strcmp(beaconType, "Basic") && strcmp(modeEnabled, "None")) {
						ret = ZCFG_INVALID_PARAM_VALUE;
						goto finish;
					}
				}
#endif
			}
			else if(!strcmp(paramList->name, "WEPKeyIndex")) {
				bool autoWepKey = json_object_get_boolean(json_object_object_get(tr98Jobj, "X_ZYXEL_AutoGenWepKey"));
				int wepKeyIndex = json_object_get_int(paramValue);
				if((autoWepKey == false) && (wepKeyIndex > 0) && (wepKeyIndex <= MAX_WEPKEY_NUM))
					json_object_object_add(apSecJobj, "X_ZYXEL_DefaultKeyID", JSON_OBJ_COPY(paramValue));
				else
					json_object_object_add(apSecJobj, "X_ZYXEL_DefaultKeyID", json_object_new_int(1));
			}
			else if(!strcmp(paramList->name, "BasicAuthenticationMode")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_WEPAuthentication", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "BasicEncryptionModes")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_WEPEncryption", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "IEEE11iEncryptionModes")) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				if(!strcmp(json_object_get_string(paramValue), "TKIPEncryption")) {
					ret = ZCFG_INVALID_PARAM_VALUE;
					goto finish;
				}
				else
					json_object_object_add(apSecJobj, "X_ZYXEL_11iEncryption", JSON_OBJ_COPY(paramValue));
#else
				json_object_object_add(apSecJobj, "X_ZYXEL_11iEncryption", JSON_OBJ_COPY(paramValue));
#endif
			}
			else if(!strcmp(paramList->name, "IEEE11iAuthenticationMode")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_11iAuthentication", JSON_OBJ_COPY(paramValue));
			}
#if defined(ZYXEL_WIFI_SUPPORT_WEP_WPA_MIXMODE) || defined(ZYXEL_WIFI_SUPPORT_WPA_ONLY_MODE)
			else if(!strcmp(paramList->name, "WPAEncryptionModes")) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				if(!strcmp(json_object_get_string(paramValue), "TKIPEncryption")) {
					ret = ZCFG_INVALID_PARAM_VALUE;
					goto finish;
				}
				else
					json_object_object_add(apSecJobj, "X_ZYXEL_WPAEncryption", JSON_OBJ_COPY(paramValue));
#else
				json_object_object_add(apSecJobj, "X_ZYXEL_WPAEncryption", JSON_OBJ_COPY(paramValue));
#endif
			}
			else if(!strcmp(paramList->name, "WPAAuthenticationMode")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_WPAAuthentication", JSON_OBJ_COPY(paramValue));
			}
#endif
#ifdef SUPPORT_WL_WPA_EAP
			else if(!strcmp(paramList->name, "X_ZYXEL_RadiusServerIPAddr")) {
				json_object_object_add(apSecJobj, "RadiusServerIPAddr", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_RadiusServerPort")) {
				json_object_object_add(apSecJobj, "RadiusServerPort", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_RadiusSecret")) {
				json_object_object_add(apSecJobj, "RadiusSecret", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_Preauth")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_Preauth", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_ReauthInterval")) {
				json_object_object_add(apSecJobj, "X_ZYXEL_ReauthInterval", JSON_OBJ_COPY(paramValue));
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_RekeyingInterval")) {
				json_object_object_add(apSecJobj, "RekeyingInterval", JSON_OBJ_COPY(paramValue));
			}
#endif

			/*End of special case*/
		}
		paramList++;
	} /*Edn while*/

	/* Set Device.WiFi.Radio.i
	 *     Device.WiFi.SSID.i
	 *     Device.WiFi.AccessPoint.i
	 *     Device.WiFi.AccessPoint.i.Security
	 */
	if(multiJobj == NULL){
#if (defined(ZYXEL_WLAN_BLOCK_OPEN)  || defined(ZYXEL_WLAN_WORK_24_HOURS) || defined(ZYXEL_CONFIG_MAX_WLAN_CLIENT))
		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI, &wifiIid, wifiJobj, NULL)) != ZCFG_SUCCESS ) {
			goto finish;
		}
#endif		
		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &apIid, apJobj, NULL)) != ZCFG_SUCCESS ) {
			goto finish;
		}

		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobj, NULL)) != ZCFG_SUCCESS ) {
			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &apIid, apJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i Fail\n", __FUNCTION__);
			goto finish;
		}

		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_SSID, &ssidIid, ssidJobj, NULL)) != ZCFG_SUCCESS ) {

			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &apIid, apJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i Fail\n", __FUNCTION__);

			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);

			goto finish;
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &radioIid, radioJobj, NULL)) != ZCFG_SUCCESS ) {

			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &apIid, apJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i Fail\n", __FUNCTION__);

			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);

			if(zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_SSID, &ssidIid, ssidJobjBak, NULL) != ZCFG_SUCCESS)
				zcfgLog(ZCFG_LOG_ERR, "%s : Recover WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);

			goto finish;
		}
	}

finish:
	zcfgFe98ParamGuardRelease(paramGuardObj);

	if(multiJobj == NULL){
		json_object_put(wifiJobj);
		json_object_put(radioJobj);
		json_object_put(ssidJobj);
		json_object_put(apJobj);
		json_object_put(apSecJobj);
		json_object_put(ssidJobjBak);
		json_object_put(apJobjBak);
		json_object_put(apSecJobjBak);
	}
	else{
		if(ssidJobjBak) json_object_put(ssidJobjBak);
		if(apJobjBak) json_object_put(apJobjBak);
		if(apSecJobjBak) json_object_put(apSecJobjBak);
	}

	return ret;
}
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
zcfgRet_t lanDevWlanCfgObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;
	char attrParamName[100] = {0};
	char *attrParamNamePtr = NULL;
	uint32_t wlanCfgOid = 0;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		if(!strcmp(paramName, "TotalAssociations")) {
			strcpy(attrParamName, "AssociatedDeviceNumberOfEntries");
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_WIFI_ACCESS_POINT, attrParamName);
			wlanCfgOid = RDM_OID_WIFI_ACCESS_POINT;
			attrParamNamePtr = attrParamName;
		}
		else {
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_WIFI_ACCESS_POINT, paramName);
			wlanCfgOid = RDM_OID_WIFI_ACCESS_POINT;
			attrParamNamePtr = paramName;
		}
		break;
	}

		if(attrValue >= 0) {
			/*Write new parameter attribute from tr98 object to tr181 objects*/
			attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
			ret = zcfgFeMultiParamAttrAppend(wlanCfgOid, multiAttrJobj, attrParamNamePtr, attrValue);

			if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, wlanCfgOid, attrParamNamePtr);
			}
		}
		else {
			ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

int lanDevWlanCfgObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "TotalAssociations")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_WIFI_ACCESS_POINT, "AssociatedDeviceNumberOfEntries");
		}
		else attrValue = paramList->attr;

		//if(attrValue < 0 ) {
		//	attrValue = 0;
		//}
		break;
	}

	return attrValue;
}

zcfgRet_t lanDevWlanCfgObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	char attrParamName[100] = {0};

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(!attrParamNamePtr) {
		attrParamNamePtr = attrParamName;
		if(!strcmp(tr181ParamName, "AssociatedDeviceNumberOfEntries")) {
			strcpy(attrParamNamePtr, "TotalAssociations");
		}
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);

		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.WPS
 *
 *   Related object in TR181:
 *
 *   Device.WiFi.AccessPoint.i.WPS
 */
zcfgRet_t lanDevWlanCfgWpsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	char ConfigMethodsEnabled[256] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apWpsJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *apFirstWpsJobj = NULL;
	objIndex_t wpsFirstIid;
	bool apWpsEnable = false;
	bool supWpsV2 = false;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WPS");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(apIid);
	while((ret = feObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &apIid, &apWpsJobj, updateFlag)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.WPS Fail\n", __FUNCTION__);
			json_object_put(apJobj);
			return ret;
		}
		json_object_put(apJobj);
	}

	// Sync with WebGUI
	IID_INIT(wpsFirstIid);
	wpsFirstIid.level = apIid.level;
	if(apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI24_LAST_AP) {
		wpsFirstIid.idx[0] = WIFI24_FIRST_AP;
	}
	else if(apIid.idx[0] > WIFI5_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP) {
		wpsFirstIid.idx[0] = WIFI5_FIRST_AP;
	}
	else if(apIid.idx[0] > WIFI5_LAST_AP) {
		printf("%s: wrong access point seq num: %hhu\n", __FUNCTION__, apIid.idx[0]);
		json_object_put(apWpsJobj);
		return ZCFG_INTERNAL_ERROR;
	}

	if(wpsFirstIid.idx[0]) {
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &wpsFirstIid, &apFirstWpsJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.WPS Fail\n", __FUNCTION__);
			json_object_put(apWpsJobj);
			return ret;
		}

		apWpsEnable = json_object_get_boolean(json_object_object_get(apFirstWpsJobj, "Enable"));
		json_object_put(apFirstWpsJobj);
	}
	else {
		apWpsEnable = json_object_get_boolean(json_object_object_get(apWpsJobj, "Enable"));
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Check enable WPS 2.0 whether or not*/
		supWpsV2 = json_object_get_boolean(json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_V2"));
		/*Write parameter value from tr181 objects to tr98 object*/
		/*spetial case*/
		if(!strcmp(paramList->name, "ConfigMethodsEnabled")) {
			/* Default enable HW push button */
			if(supWpsV2){
				snprintf(ConfigMethodsEnabled, sizeof(ConfigMethodsEnabled), "PhysicalPushButton,");
			}
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_EnablePBC");
			if(paramValue != NULL) {
				if (json_object_get_boolean(paramValue) == true){
					if(supWpsV2)
						snprintf((ConfigMethodsEnabled + strlen(ConfigMethodsEnabled)), (sizeof(ConfigMethodsEnabled) - strlen(ConfigMethodsEnabled)), "VirtualPushButton,");
					else
						snprintf((ConfigMethodsEnabled + strlen(ConfigMethodsEnabled)), (sizeof(ConfigMethodsEnabled) - strlen(ConfigMethodsEnabled)), "PushButton,");
				}
			}
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_EnableStaPin");
			if(paramValue != NULL) {
				if (json_object_get_boolean(paramValue) == true)
					snprintf((ConfigMethodsEnabled + strlen(ConfigMethodsEnabled)), (sizeof(ConfigMethodsEnabled) - strlen(ConfigMethodsEnabled)), "PIN,");
			}
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin");
			if(paramValue != NULL) {
				if (json_object_get_boolean(paramValue) == true){
					if(supWpsV2)
						snprintf((ConfigMethodsEnabled + strlen(ConfigMethodsEnabled)), (sizeof(ConfigMethodsEnabled) - strlen(ConfigMethodsEnabled)), "VirtualDisplay,");
					else
						snprintf((ConfigMethodsEnabled + strlen(ConfigMethodsEnabled)), (sizeof(ConfigMethodsEnabled) - strlen(ConfigMethodsEnabled)), "Display,");
				}
			}
			if (ConfigMethodsEnabled[strlen(ConfigMethodsEnabled)-1] == ',')
				ConfigMethodsEnabled[strlen(ConfigMethodsEnabled)-1] = '\0';

			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(ConfigMethodsEnabled));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(apWpsJobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "Enable")){
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(apWpsEnable));
			}
			else {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "UUID")) {
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_UUID");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "ConfigurationState")) {
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_Configured");
			if(paramValue != NULL) {
				if(json_object_get_boolean(json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_Configured"))){
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Configured"));
				} else {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Not configured"));
				}
				paramList++;
				continue;
			}
		}

		/*Not defined in tr181, give it a default value*/
		if(!strcmp(paramList->name, "DeviceName")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(WPS_DEVICENAME));
		}else if(!strcmp(paramList->name, "DevicePassword")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
		}else if(!strcmp(paramList->name, "SetupLock")) {
#ifdef ZYXEL_TAAAB_WPSSETUPLOCK
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin");
			if(paramValue != NULL) {
				if (json_object_get_boolean(paramValue) == true)
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
#else
			printf("%s: SetupLock\n", __FUNCTION__);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
#endif
		}else if(!strcmp(paramList->name, "SetupLockedState")) {
#ifdef ZYXEL_TAAAB_WPSSETUPLOCK
			paramValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_APPinState");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
#else
			printf("%s: SetupLockedState\n", __FUNCTION__);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("UnLocked"));
#endif
		}else {
			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		}
		paramList++;
	}

	json_object_put(apWpsJobj);

	return ZCFG_SUCCESS;
}

/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.WPS
 *
 *   Related object in TR181:
 *
 *   Device.WiFi.AccessPoint.i.WPS
 */
zcfgRet_t lanDevWlanCfgWpsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	//char ConfigMethodsEnabled[64] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apWpsJobj = NULL, *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	// struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	//bool tr98WpsEnable, tr181WpsEnable;
	bool tr98WpsEnable;
	//int tr98WpsMthSupport = 0;
	// const char *tr98WpsMthEnabledStr = NULL;
	const char *tr98WpsMthSupportStr = NULL;
	//bool wpsConfigured = true, wpsOldConfigured = true;
	bool wpsConfigured = true;
	//int tr98WpsMthEnabled = 0;
	//int wpsPin = 0;
	int wpsStationPin = 0;
	char tr181ModeEnabled[25] = {0};
	const char *modeEnabled = NULL;
	struct json_object *apFirstWpsJobj = NULL;
	objIndex_t wpsFirstIid;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WPS");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(apIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if(!multiJobj){
			if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &apIid, &apWpsJobj)) != ZCFG_SUCCESS) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.WPS Fail\n", __FUNCTION__);
				json_object_put(apJobj);
				return ret;
			}
		}else{
			apWpsJobj = json_object_new_object();
		}
		json_object_put(apJobj);
	}

	// Sync with WebGUI
	IID_INIT(wpsFirstIid);
	wpsFirstIid.level = apIid.level;
	if(apIid.idx[0] > WIFI24_FIRST_AP && apIid.idx[0] <= WIFI24_LAST_AP) {
		wpsFirstIid.idx[0] = WIFI24_FIRST_AP;
	}
	else if(apIid.idx[0] > WIFI5_FIRST_AP && apIid.idx[0] <= WIFI5_LAST_AP) {
		wpsFirstIid.idx[0] = WIFI5_FIRST_AP;
	}
	else if(apIid.idx[0] > WIFI5_LAST_AP) {
		printf("%s: wrong access point seq num: %hhu\n", __FUNCTION__, apIid.idx[0]);
		json_object_put(apWpsJobj);
		return ZCFG_INTERNAL_ERROR;
	}

	if(wpsFirstIid.idx[0]) {
		bool apWpsEnable = false;

		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_WPS, &wpsFirstIid, &apFirstWpsJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.WPS Fail\n", __FUNCTION__);
			json_object_put(apWpsJobj);
			return ret;
		}

		apWpsEnable = json_object_get_boolean(json_object_object_get(apFirstWpsJobj, "Enable"));
		json_object_put(apFirstWpsJobj);

		if((paramValue = json_object_object_get(tr98Jobj, "Enable"))) {
			bool tr98ApWpsEnable = json_object_get_boolean(paramValue);
			if(tr98ApWpsEnable != apWpsEnable) {
				printf("%s: Can't set WPS Enable/Disable on this object\n", __FUNCTION__);
				json_object_put(apWpsJobj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
		json_object_put(apWpsJobj);
		return ret;
	}
	modeEnabled = json_object_get_string(json_object_object_get(apSecJobj, "ModeEnabled"));
	if(modeEnabled) {
		strncpy(tr181ModeEnabled, modeEnabled, sizeof(tr181ModeEnabled));
	}
	json_object_put(apSecJobj);

	if(multiJobj){
		tmpObj = apWpsJobj;
		apWpsJobj = NULL;
		apWpsJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT_WPS, &apIid, multiJobj, tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_ACCESS_POINT_WPS, &apIid, "partialset", json_object_new_boolean(true));
	}

	ret = ZCFG_SUCCESS;

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	tr98WpsEnable = json_object_get_boolean(json_object_object_get(tr98Jobj, "Enable"));
	tr98WpsMthSupportStr =	json_object_get_string(json_object_object_get(tr98Jobj, "ConfigMethodsSupported"));
	// tr98WpsMthEnabledStr =	json_object_get_string(json_object_object_get(tr98Jobj, "ConfigMethodsEnabled"));

#if 0
	tr181WpsEnable = json_object_get_boolean(json_object_object_get(apWpsJobj, "Enable"));
	if(!tr181WpsEnable && tr98WpsEnable && strstr(tr181ModeEnabled, "WEP")) {
		printf("%s: Couldn't enable WPS while Wifi is WEP security\n", __FUNCTION__);
		ret = ZCFG_INVALID_PARAM_VALUE;
		goto write_parameters_complete;
	}
#endif
#if 0 //don't allow ACS trigger WPS method
	if(tr98WpsEnable == false) {
		json_object_object_add(apWpsJobj, "Enable", json_object_new_boolean(tr98WpsEnable));
		goto write_parameters_complete;
	}

	tr98WpsMthSupportStr =	json_object_get_string(json_object_object_get(tr98Jobj, "ConfigMethodsSupported"));
    if(tr98WpsMthSupportStr && (strstr(tr98WpsMthSupportStr, "PIN"))) tr98WpsMthSupport |= WPS_PIN;
	if(tr98WpsMthSupportStr && (strstr(tr98WpsMthSupportStr, "PushButton"))) tr98WpsMthSupport |= WPS_PBC;


	tr98WpsMthEnabledStr =	json_object_get_string(json_object_object_get(tr98Jobj, "ConfigMethodsEnabled"));
    if(tr98WpsMthEnabledStr && (!strcmp(tr98WpsMthEnabledStr, "PIN"))) tr98WpsMthEnabled = WPS_PIN;
	else if(tr98WpsMthEnabledStr && (!strcmp(tr98WpsMthEnabledStr, "PushButton"))) tr98WpsMthEnabled = WPS_PBC;

	if( ((tr98WpsMthEnabled == WPS_PIN) && !(tr98WpsMthSupport & WPS_PIN)) ||
		((tr98WpsMthEnabled == WPS_PBC) && !(tr98WpsMthSupport & WPS_PBC)) ) {
		ret = ZCFG_INVALID_PARAM_VALUE;
		goto write_parameters_complete;
	}

	if(tr98WpsMthEnabled == WPS_PIN) wpsPin = WPS_STA_PIN;
#else

#endif

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(!paramValue) {
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "Enable")) {
			json_object_object_add(apWpsJobj, "Enable", json_object_new_boolean(tr98WpsEnable));
		}else if(!strcmp(paramList->name, "ConfigMethodsSupported")) {
			if(tr98WpsMthSupportStr)
				json_object_object_add(apWpsJobj, "ConfigMethodsSupported", json_object_new_string((const char *)tr98WpsMthSupportStr));
#if 0
		}else if(!strcmp(paramList->name, "ConfigMethodsEnabled")) {
			if(tr98WpsMthEnabledStr){
				if (strstr(tr98WpsMthEnabledStr, "PushButton"))
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnablePBC", json_object_new_boolean(true));
				else
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnablePBC", json_object_new_boolean(false));

				if (strstr(tr98WpsMthEnabledStr, "StationPIN"))
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableStaPin", json_object_new_boolean(true));
				else
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableStaPin", json_object_new_boolean(false));

				if (strstr(tr98WpsMthEnabledStr, "ApPIN"))
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin", json_object_new_boolean(true));
				else
					json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin", json_object_new_boolean(false));
			}
#endif
		}else if(!strcmp(paramList->name, "DevicePassword")) {
				char wpsStationPinStr[12] = {0};
				wpsStationPin = json_object_get_int(paramValue);
				sprintf(wpsStationPinStr, "%u", wpsStationPin);
				printf("%s: wps pin %s\n", __FUNCTION__, wpsStationPinStr);
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_StationPin", json_object_new_string(wpsStationPinStr));
		}else if(!strcmp(paramList->name, "SetupLock")) {
#ifdef ZYXEL_TAAAB_WPSSETUPLOCK
			if((json_object_get_boolean(paramValue) == true)) {
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin", json_object_new_boolean(false));
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_APPinState", json_object_new_string("LockedByRemoteManagement"));
			}
			else
			{
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin", json_object_new_boolean(true));
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_APPinState", json_object_new_string("UnLocked"));
			}
#else
			if((json_object_get_boolean(paramValue) == true)) {
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto write_parameters_complete;
			}
#endif
		}
#ifdef WPS_ACCESSPOINT_PIN
		else if(!strcmp(paramList->name, "X_ZYXEL_WPS_Configured")) {
			// tr181ParamValue = json_object_object_get(apWpsJobj, "X_ZYXEL_WPS_Configured");
#if 0 //don't allow ACS trigger WPS method
			wpsOldConfigured = json_object_get_boolean(tr181ParamValue);
#endif
			wpsConfigured = json_object_get_boolean(paramValue);
			json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_Configured", json_object_new_boolean(wpsConfigured));
		}else if(!strcmp(paramList->name, "X_ZYXEL_WPS_GenDevicePin")) {
#if 0 //don't allow ACS trigger WPS method
			if((json_object_get_boolean(paramValue) == true) && (wpsConfigured == true)) {
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableAPPin", json_object_new_boolean(true));
				json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_StationPin", json_object_new_string("00000000"));
				wpsPin = WPS_AP_PIN;
			}
#endif
			json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_GenDevicePin", JSON_OBJ_COPY(paramValue));
		}
#endif
		else if(!strcmp(paramList->name, "X_ZyXEL_TriggerWPSPushButton")){
			if(apIid.idx[0] != 1 && apIid.idx[0] != 5){
				ret = ZCFG_REQUEST_REJECT;
				goto write_parameters_complete;
			}

			json_object_object_add(apWpsJobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}else {
			//tr181ParamValue = json_object_object_get(apWpsJobj, paramList->name);
			//if(tr181ParamValue != NULL) {
				json_object_object_add(apWpsJobj, paramList->name, JSON_OBJ_COPY(paramValue));
			//}else{
			//	ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
			//}
		}
		paramList++;
	}

#if 0 //don't allow ACS trigger WPS method
	// special parameter config
	if( ((wpsOldConfigured == true) && (wpsConfigured != true)) ||
		((wpsOldConfigured == false) && (wpsConfigured == true)) ) {
		json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_StationPin", json_object_new_string("00000000"));
		json_object_object_add(apWpsJobj, "ConfigMethodsEnabled", json_object_new_string("PIN"));
		goto write_parameters_complete;
	}

	if(tr98WpsMthEnabled == WPS_PBC) {
		json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnablePBC", json_object_new_boolean(true));
		json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_StationPin", json_object_new_string("00000000"));
	}

	if((tr98WpsMthEnabled == WPS_PIN) && (wpsPin == WPS_STA_PIN)) {
		char wpsStationPinStr[10] = {0};

		if((wpsStationPin > (10000-1)) || (wpsStationPin < 0)) {
			ret = ZCFG_INVALID_PARAM_VALUE;
			goto write_parameters_complete;
		}
		sprintf(wpsStationPinStr, "%04d", wpsStationPin);

		json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_EnableStaPin", json_object_new_boolean(true));
		json_object_object_add(apWpsJobj, "X_ZYXEL_WPS_StationPin", json_object_new_string(wpsStationPinStr));
	}
#endif

write_parameters_complete:
	/*Set Device.WiFi.AccessPoint.i.Security*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else if(ret == ZCFG_SUCCESS) {
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_ACCESS_POINT_WPS, &apIid, apWpsJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(apWpsJobj);
			return ret;
		}

		json_object_put(apWpsJobj);
	}else json_object_put(apWpsJobj);

	return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.AssociatedDevice.i
 *
 *   Related object in TR181:
 *
 *   Device.Hosts.Host.i
 */
zcfgRet_t lanDevWlanCfgAssocDevObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
#ifdef SAAAE_TR69_NO_LAN_HOST
	int idx = 0;
	uint32_t hostNum = 0;
	FILE *arpfp = NULL;
	char line[512]={0};
	int type, flags;
	char ip[32]={0};
	char hwa[65]={0};
	char mask[32]={0};
	char dev[32]={0};
	int num = 0;
	hostlist_t *hostlist = NULL;
	hostlist_t *theHostlist = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s()\n", __FUNCTION__);

	//Refresh Hosts device list
	lanDevHostListUpdate(&hostlist, 0);

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.WLANConfiguration.%*d.AssociatedDevice.%d", &idx);

	/*ARP table*/
	zcfgLog(ZCFG_LOG_INFO, "%s(): open %s\n", __FUNCTION__, ARPFILE);
	arpfp = fopen(ARPFILE, "r");
	if(arpfp == NULL){
		zcfgLog(ZCFG_LOG_ERR, "%s(): open %s fail\n", __FUNCTION__, ARPFILE);
		return ZCFG_SUCCESS;
	}
	fgets(line, sizeof(line), arpfp);
	while (fgets(line, sizeof(line), arpfp)) {
		mask[0] = '-'; mask[1] = '\0';
		dev[0] = '-'; dev[1] = '\0';
		/* All these strings can't overflow
		 * because fgets above reads limited amount of data */
		num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
					 ip, &type, &flags, hwa, mask, dev);
		if (num < 4)
			break;

		/*incomplete, skip*/
		if(flags == 0){
			continue;
		}

		if(strcmp(hwa,"00:00:00:00:00:00")==0){
			continue;
		}

		// all LAN dev should be br+ ,no eth0, eth1, eth2...
		if(strstr(dev, "br") == NULL)
			continue;

		theHostlist = findHostFromList(hostlist, hwa);

		if(theHostlist){
			hostNum++;
			if(idx == hostNum) {
				/*fill up tr98 Host object*/
				*tr98Jobj = json_object_new_object();
				json_object_object_add(*tr98Jobj, "AssociatedDeviceMACAddress", json_object_new_string(hwa));
				json_object_object_add(*tr98Jobj, "AssociatedDeviceIPAddress", json_object_new_string(ip));
				break;
			}
		}
	}
	fclose(arpfp);

	freeHostList(hostlist);
#else
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	zcfgRet_t ret = ZCFG_SUCCESS;
	char wlanConfigPath[128] = {0};
	char ssidRef[32] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	objIndex_t ssidIid, apIid, assocDevStatIid;
	struct json_object *ssidJobj = NULL;
	struct json_object *apJobj = NULL;
	struct json_object *assocDevJobj = NULL;
	bool found = false;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char line[512]={0};
	FILE *arpfp = NULL;
	int type, flags;
	char ip[32]={0};
	char hwa[65]={0};
	char mask[32]={0};
	char dev[32]={0};
	int num = 0;
	const char *MACAddress = NULL;
	bool foundIP = false;
#ifdef ABZQ_CUSTOMIZATION_WLAN_INFO
	int sysUpTime = 0, connTime = 0;
	objIndex_t devInfoIid;
	struct json_object *devInfoObj = NULL;
	uint32_t LastDataDownlinkRate = 0;
	uint32_t LastDataUplinkRate = 0;
#endif

	printf("%s : Enter\n", __FUNCTION__);

	strcpy(wlanConfigPath, tr98FullPathName);
	ptr = strstr(wlanConfigPath, ".AssociatedDevice");
	*ptr = '\0';
	if((ret = zcfgFe98To181MappingNameGet(wlanConfigPath, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ssidIid);
	ssidIid.level = 1;
	sscanf(wifiSsid, "WiFi.SSID.%hhu", &ssidIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WIFI_SSID, &ssidIid, &ssidJobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID Fail\n", __FUNCTION__);
		return ret;
	}

	json_object_put(ssidJobj);

	IID_INIT(apIid);
	while((ret = feObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid)){
			found = true;
			json_object_put(apJobj);
			break;
		}
		else
			json_object_put(apJobj);
	}

	if(found){
		IID_INIT(assocDevStatIid);
		assocDevStatIid.level = 2;
		assocDevStatIid.idx[0] = apIid.idx[0];
		sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.WLANConfiguration.%*d.AssociatedDevice.%hhu.Stats", &(assocDevStatIid.idx[1]));
		if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_ASSOC_DEV, &assocDevStatIid, &assocDevJobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.AssociatedDevice.i. Fail\n", __FUNCTION__);
			return ret;
		}
	}
	else
		return ZCFG_INVALID_OBJECT;

	MACAddress = json_object_get_string(json_object_object_get(assocDevJobj, "MACAddress"));

	//get ip from arp table
	arpfp = fopen(ARPFILE, "r");
	if(arpfp != NULL && MACAddress != NULL){
		fgets(line, sizeof(line), arpfp);
		while (fgets(line, sizeof(line), arpfp)) {
			mask[0] = '-'; mask[1] = '\0';
			dev[0] = '-'; dev[1] = '\0';
			ip[0] = '\0'; hwa[0] = '\0';
			/* All these strings can't overflow
			 * because fgets above reads limited amount of data */
			num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
						 ip, &type, &flags, hwa, mask, dev);
			if (num < 4)
				break;

			/*incomplete, skip*/
			if(flags == 0){
				continue;
			}

			if(strstr(dev, "br") == NULL)
				continue;

			if(!strcasecmp(hwa, MACAddress)){
				foundIP = true;
				break;
			}
		}
	}
	if(arpfp != NULL)
		fclose(arpfp);

	/*fill up tr98 object from related tr181 objects*/
	char TransRate[5] = {0};
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/

		/*special case*/
		if(!strcmp(paramList->name, "AssociatedDeviceMACAddress")) {
			paramValue = json_object_object_get(assocDevJobj, "MACAddress");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "AssociatedDeviceIPAddress")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(ip));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "AssociatedDeviceAuthenticationState")) {
			paramValue = json_object_object_get(assocDevJobj, "AuthenticationState");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#ifdef ABZQ_CUSTOMIZATION_WLAN_INFO
		else if(!strcmp(paramList->name, "ConnectedTime")) {
			IID_INIT(devInfoIid);
			if((ret = feObjJsonGet(RDM_OID_DEV_INFO, &devInfoIid, &devInfoObj, TR98_GET_UPDATE)) == ZCFG_SUCCESS){
				sysUpTime = json_object_get_int(json_object_object_get(devInfoObj, "UpTime"));
				connTime = json_object_get_int(json_object_object_get(assocDevJobj, "ConnectedTime"));
				connTime = sysUpTime - connTime;
				zcfgFeObjStructFree(devInfoObj);
			}
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(connTime));
			paramList++;
			continue;
		}
		/* ABZQ request LastDataDownlinkRate & LastDataUplinkRate unit must be Mbps */
		/* Original unit is Kbps, therefore divide 1000 here */
		else if(!strcmp(paramList->name, "LastDataDownlinkRate")) {
			LastDataDownlinkRate =  json_object_get_int(json_object_object_get(assocDevJobj, "LastDataDownlinkRate"));
			LastDataDownlinkRate /= 1000;
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(LastDataDownlinkRate));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "LastDataUplinkRate")) {
			LastDataUplinkRate =  json_object_get_int(json_object_object_get(assocDevJobj, "LastDataUplinkRate"));
			LastDataUplinkRate /= 1000;
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(LastDataUplinkRate));
			paramList++;
			continue;
		}
#endif
		else if(!strcmp(paramList->name, "LastDataTransmitRate")) {
			paramValue = json_object_object_get(assocDevJobj, "LastDataUplinkRate");
			sprintf(TransRate, "%d", json_object_get_int(paramValue)/1000);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(TransRate));
			paramList++;
			continue;
		}
                else if(!strcmp(paramList->name, "X_ZYXEL_LastDataDownlinkRate")) {
                        paramValue = json_object_object_get(assocDevJobj, "LastDataDownlinkRate");
                        json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
                        paramList++;
                        continue;
                }
                else if(!strcmp(paramList->name, "X_ZYXEL_Retransmissions")) {
                        paramValue = json_object_object_get(assocDevJobj, "Retransmissions");
                        json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
                        paramList++;
                        continue;
                }

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		else if(!strcmp(paramList->name, "X_ZYXEL_SignalStrength")) {
			paramValue = json_object_object_get(assocDevJobj, "SignalStrength");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_RSSI")) {
			paramValue = json_object_object_get(assocDevJobj, "SignalStrength");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

		paramValue = json_object_object_get(assocDevJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}


		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(assocDevJobj);

	return ZCFG_SUCCESS;
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
#endif

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.AssociatedDevice.i.Stats
 *
 *   Related object in TR181:
 *
 *   Device.WiFi.AccessPoint.i.AssociatedDevice.i.Stats
 */
zcfgRet_t lanDevWlanCfgAssocDevStatObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char wlanConfigPath[128] = {0};
	char ssidRef[32] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	objIndex_t ssidIid, apIid, assocDevStatIid;
	struct json_object *ssidJobj = NULL;
	struct json_object *apJobj = NULL;
	struct json_object *assocDevStatJobj = NULL;
	bool found = false;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	strcpy(wlanConfigPath, tr98FullPathName);
	ptr = strstr(wlanConfigPath, ".AssociatedDevice");
	*ptr = '\0';
	if((ret = zcfgFe98To181MappingNameGet(wlanConfigPath, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ssidIid);
	ssidIid.level = 1;
	sscanf(wifiSsid, "WiFi.SSID.%hhu", &ssidIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WIFI_SSID, &ssidIid, &ssidJobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID Fail\n", __FUNCTION__);
		return ret;
	}
	json_object_put(ssidJobj);

	IID_INIT(apIid);
	while((ret = feObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj, TR98_GET_WITHOUT_UPDATE)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid)){
			found = true;
			json_object_put(apJobj);
			break;
		}
		else
			json_object_put(apJobj);
	}

	if(found){
		IID_INIT(assocDevStatIid);
		assocDevStatIid.level = 2;
		assocDevStatIid.idx[0] = apIid.idx[0];
		sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.WLANConfiguration.%*d.AssociatedDevice.%hhu.Stats", &(assocDevStatIid.idx[1]));
		if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_ASSOC_DEV_ST, &assocDevStatIid, &assocDevStatJobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.AssociatedDevice.i.Stats Fail\n", __FUNCTION__);
			return ret;
		}
	}
	else
		return ZCFG_INVALID_OBJECT;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(assocDevStatJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(assocDevStatJobj);

	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#if 0 // unused
zcfgRet_t lanDevWlanCfgAssocDevStatObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}
#endif

#if 0 //not ready
int lanDevWlanCfgAssocDevStatObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  oid = 0;
	objIndex_t iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	oid = zcfgFeObjNameToObjId(tr181Obj, &iid);

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

zcfgRet_t lanDevWlanCfgAssocDevStatObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  oid = 0;
	objIndex_t iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	oid = zcfgFeObjNameToObjId(tr181Obj, &iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
		break;
	}

	return ret;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.PreSharedKey.i
 *
 *   Related object in TR181:
 *
 *   WiFi.AccessPoint.i.Security
 */
zcfgRet_t lanDevWlanCfgPskObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	int idx = 0;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.PreSharedKey.%d", &idx);

	/*There should be only one PreSharedKey Object*/
	if(idx > 1)
		return ZCFG_NO_MORE_INSTANCE;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".PreSharedKey");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(apIid);
	while((ret = feObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if((ret = feObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj, updateFlag)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(apJobj);
			return ret;
		}
		json_object_put(apJobj);
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(apSecJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		paramValue = json_object_new_string("");
		json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		json_object_put(paramValue);
		paramList++;
	}

	json_object_put(apSecJobj);

	return ZCFG_SUCCESS;
}
/*   TR98 Object Name : InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.PreSharedKey.i
 *
 *   Related object in TR181:
 *
 *   WiFi.AccessPoint.i.Security
 */
zcfgRet_t lanDevWlanCfgPskObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	int idx = 0;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	bool setAutoGenPsk = false;
	const char *setPreSharedKey = NULL;
	const char *setKeyPassPhrase = NULL;
#ifdef ZYXEL_ONESSID
	int guest = 0;
	int main = 0;
	char wifiRadio[32] = {0};
	struct json_object *radioJobj = NULL;
	struct json_object *tmpJobj = NULL;
	struct json_object *onessidJobj = NULL;
	struct json_object *ssidJobj = NULL;
	objIndex_t onessidIid, ssidIid, radioIid, tmpIid;
	char tmpRadio[32] = {0};
	char bands[32] = {0};
	bool onessid = false;
	bool mainssid = false;
#endif
	objIndex_t mgmtIid;
	rdm_MgmtSrv_t* mgmtSrvObj = NULL;
	bool PreSharedKeyOperation = true;
	char currPSK[66]={0};
	bool setPSKtoKeyPass = false;
	struct json_object *oidJobj = NULL;
	struct json_object *iidJobj = NULL;
	char objIndexID[32]={0};
	char objID[16]={0};
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
	bool setkey = false;
#endif

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.PreSharedKey.%d", &idx);

	/*There should be only one PreSharedKey Object*/
	if(idx > 1)
		return ZCFG_NO_MORE_INSTANCE;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".PreSharedKey");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef ZYXEL_ONESSID
	IID_INIT(onessidIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &onessidIid, &onessidJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get ONESSID Fail\n", __FUNCTION__);
		return ret;
	}
	onessid = json_object_get_boolean(json_object_object_get(onessidJobj, "Enable"));
	zcfgFeJsonObjFree(onessidJobj);

	IID_INIT(ssidIid);
	ssidIid.level = 1;
	sscanf(wifiSsid, "WiFi.SSID.%hhu", &ssidIid.idx[0]);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidIid, &ssidJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.SSID Fail\n", __FUNCTION__);
		return ret;
	}
	strcpy(wifiRadio, json_object_get_string(json_object_object_get(ssidJobj, "LowerLayers")));

	IID_INIT(radioIid);
	radioIid.level = 1;
	sscanf(wifiRadio, "WiFi.Radio.%hhu", &radioIid.idx[0]);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &radioIid, &radioJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.Radio Fail\n", __FUNCTION__);
		json_object_put(ssidJobj);
		return ret;
	}
	strcpy(bands, json_object_get_string(json_object_object_get(radioJobj, "SupportedFrequencyBands")));

	if(onessid && !strcmp(bands,"5GHz")){
		IID_INIT(tmpIid);
		while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &tmpIid, &tmpJobj)) == ZCFG_SUCCESS) {
			strcpy(tmpRadio, json_object_get_string(json_object_object_get(tmpJobj, "LowerLayers")));
			mainssid = json_object_get_boolean(json_object_object_get(tmpJobj, "X_ZYXEL_MainSSID"));
			if(mainssid && !strcmp(tmpRadio,wifiRadio)){
				guest++;
				json_object_put(tmpJobj);
				break;
			}
			else {
				json_object_put(tmpJobj);
				tmpJobj = NULL;
				guest++;
			}
		}
	    main = guest++;

		if(ssidIid.idx[0]==main || ssidIid.idx[0]==guest){//5G main,5G guest 1
			json_object_put(ssidJobj);
			json_object_put(radioJobj);
			return ZCFG_REQUEST_REJECT;
		}
	}
	json_object_put(ssidJobj);
	json_object_put(radioJobj);
#endif

	setPreSharedKey = json_object_get_string(json_object_object_get(tr98Jobj, "PreSharedKey"));
	setKeyPassPhrase = json_object_get_string(json_object_object_get(tr98Jobj, "KeyPassphrase"));

	IID_INIT(apIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(apJobj);
			return ret;
		}
		json_object_put(apJobj);
	}

	IID_INIT(mgmtIid);
	if((ret = feObjStructGet(RDM_OID_MGMT_SRV, &mgmtIid, (void **)&mgmtSrvObj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS ) {
		printf("%s : Get X_ZYXEL_PreSharedKeyOperation Fail\n", __FUNCTION__);
		return ret;
	}
	PreSharedKeyOperation = mgmtSrvObj->X_ZYXEL_PreSharedKeyOperation;
	zcfgFeObjStructFree(mgmtSrvObj);

	if(multiJobj){
		tmpObj = apSecJobj;
		apSecJobj = NULL;
		apSecJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, multiJobj, tmpObj);
	}

	setAutoGenPsk = json_object_get_boolean(json_object_object_get(tr98Jobj, "X_ZYXEL_AutoGenPSK"));

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(apSecJobj, paramList->name);
			//check Key by Ron
			/*PreSharedKey format*/
			if(!strcmp(paramList->name, "PreSharedKey") && tr181ParamValue) {
				printf("setPreSharedKey : %s, line:%d\n", setPreSharedKey, __LINE__);
				printf("boolean PreSharedKeyOperation : %d, line:%d\n", PreSharedKeyOperation, __LINE__);
				// Romfile contorl parameter, X_ZYXEL_PreSharedKeyOperation: default is 1. 
				// 0 = TR69 accept SPV 8-63 char to change wifi password, 1 = TR69 accept SPV 64 byte PSK for OPAL
				if(PreSharedKeyOperation){
					if(strlen(setPreSharedKey)>0 &&strlen(setPreSharedKey)!=64){
						ret = ZCFG_INVALID_PARAM_VALUE;
						goto finish;
					}
					if(strlen(setPreSharedKey)==64){
						int i=0;
						for(; i<64; i++){
							char c = *(setPreSharedKey+i);
							if( !((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F')) ){
								ret = ZCFG_INVALID_PARAM_VALUE;
								goto finish;
							}
						}
					}
				}
				else{
					if((strlen(setPreSharedKey) !=0 && strlen(setPreSharedKey) <8)|| 63<strlen(setPreSharedKey)) {
						ret = ZCFG_INVALID_PARAM_VALUE;
						goto finish;
					}
				}
				if(strlen(setPreSharedKey) && (strcmp(setPreSharedKey, json_object_get_string(tr181ParamValue)))){
					if(!PreSharedKeyOperation && tr181ParamValue){
						ZOS_STRNCPY(currPSK, json_object_get_string(json_object_object_get(apSecJobj, "PreSharedKey")), sizeof(currPSK));
						setPSKtoKeyPass = true;
					}
					json_object_object_add(apSecJobj, "PreSharedKey", json_object_new_string(setPreSharedKey));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
					setkey= true;
#endif
				}
				paramList++;
				continue;
			}
			/*KeyPassphrase format */
            if(!strcmp(paramList->name, "KeyPassphrase") && tr181ParamValue) {
                if(strlen(setKeyPassPhrase) !=0 && strlen(setKeyPassPhrase) <8) {
                    ret = ZCFG_INVALID_PARAM_VALUE;
                    goto finish;
                }
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
                setkey= true;
#endif
            }

			if(tr181ParamValue != NULL) {
				json_object_object_add(apSecJobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
		}
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
	if(setkey)
	{
		json_object_object_add(apSecJobj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(false));
	}
#endif

	/*Set Device.WiFi.AccessPoint.i.Security*/
	if(multiJobj){
		if(!PreSharedKeyOperation && setPSKtoKeyPass && currPSK[0]){
			// 0= accept SPV 8-63 char to change wifi password, so modify multiJobj to actually mapping value to KeyPassphrase in TR181
			sprintf(objIndexID, "%d.%d.%d.%d.%d.%d.%d", apIid.level, apIid.idx[0], apIid.idx[1], apIid.idx[2], apIid.idx[3], apIid.idx[4], apIid.idx[5]);
			sprintf(objID, "%d", RDM_OID_WIFI_ACCESS_POINT_SEC);				
			oidJobj = json_object_object_get(multiJobj, objID);
			if(oidJobj != NULL) {
				iidJobj = json_object_object_get(oidJobj, objIndexID);
				if(iidJobj != NULL) {
					json_object_object_add(iidJobj, "KeyPassphrase", json_object_new_string(setPreSharedKey));
					json_object_object_add(iidJobj, "PreSharedKey", json_object_new_string(currPSK));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
					json_object_object_add(iidJobj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(false));
#endif
				}
			} 
		}
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(apSecJobj);
			return ret;
		}

		json_object_put(apSecJobj);
	}

	return ZCFG_SUCCESS;
finish:
    if(multiJobj == NULL){
        json_object_put(apJobj);
        json_object_put(apSecJobj);
    }
	strcpy(paramfault, "PreSharedKey");
    return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
zcfgRet_t lanDevWlanCfgWEPKeyObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	int idx = 0;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WEPKey.%d", &idx);

	/*There should be only one WEPKey Object*/
	if(idx > 4)
		return ZCFG_NO_MORE_INSTANCE;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WEPKey");
	*ptr = '\0';

	updateFlag = 0;
	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(apIid);
	while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(apJobj);
			return ret;
		}
		json_object_put(apJobj);
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(!strcmp(paramList->name, "WEPKey")){
			paramValue = json_object_new_string("");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			json_object_put(paramValue);
			paramList++;
			continue;
		}
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(apSecJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			json_object_put(paramValue);
			paramList++;
			continue;
		}
		paramValue = json_object_new_string("");
		json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		json_object_put(paramValue);
		paramList++;
	}

	json_object_put(apSecJobj);

	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
zcfgRet_t wifiNeiborDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_WIFI_DIAGNOSTIC, &objIid, &jobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(jobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(jobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t wifiNeiborDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *jobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_DIAGNOSTIC, &objIid, &jobj)) != ZCFG_SUCCESS)
		return ret;

	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	if(multiJobj){
		tmpObj = jobj;
		jobj = NULL;
		jobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_DIAGNOSTIC, &objIid, multiJobj, tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_WIFI_DIAGNOSTIC, &objIid, "partialset", json_object_new_boolean(true));
		json_object_put(tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "DiagnosticsState")) {
				if(strcmp(json_object_get_string(paramValue), "Requested"))
					return ZCFG_INVALID_PARAM_VALUE;

				if(zcfg_msg_eidGet() == ZCFG_EID_TR69){
					json_object_object_add(jobj, "X_ZYXEL_Creator", json_object_new_string(CREATOR_ACS));
				}

			}

			tr181ParamValue = json_object_object_get(jobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		//json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_DIAGNOSTIC, &objIid, jobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(jobj);
			return ret;
		}
		json_object_put(jobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t wifiNeiborDiagObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int wifiNeiborDiagObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  oid = 0;
	objIndex_t iid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	oid = zcfgFeObjNameToObjId(tr181Obj, &iid);

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

zcfgRet_t wifiNeiborDiagObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  oid = 0;
	objIndex_t iid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	oid = zcfgFeObjNameToObjId(tr181Obj, &iid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
		break;
	}

	return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
zcfgRet_t wifiNeiborDiagResultObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	uint32_t  oid = 0;
	objIndex_t iid;
	struct json_object *jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(iid);
	iid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_NeighboringWiFiDiagnostic.Result.%hhu", &iid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_WIFI_DIAGNOSTIC_RESULTS, &iid, &jobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(jobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/* MaxPhyRate */
		if(!strcmp(paramList->name, "MaxPhyRate"))
		{
			paramValue = json_object_object_get(jobj, "X_ZYXEL_MaxPhyRate");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/* ExtensionChannel */
		if(!strcmp(paramList->name, "ExtensionChannel"))
		{
			paramValue = json_object_object_get(jobj, "X_ZYXEL_ExtensionChannel");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/* rssi */
		if(!strcmp(paramList->name, "RSSI"))
		{
			paramValue = json_object_object_get(jobj, "SignalStrength");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		/* SignalQuality */
		if(!strcmp(paramList->name, "SignalQuality"))
		{
			paramValue = json_object_object_get(jobj, "X_ZYXEL_SignalQuality");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(jobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t wifiNeiborDiagResultObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int wifiNeiborDiagResultObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	zcfgRet_t ret;
	uint32_t  oid = RDM_OID_WIFI_DIAGNOSTIC_RESULTS;
	objIndex_t iid;
	struct json_object *jobj = NULL;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(iid);
	iid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_NeighboringWiFiDiagnostic.Result.%hhu", &iid.idx[0]);
	if((ret = feObjJsonGet(oid, &iid, &jobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS)
		return ret;

	json_object_put(jobj);

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}

	return attrValue;
}

zcfgRet_t wifiNeiborDiagResultObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	uint32_t  oid = RDM_OID_WIFI_DIAGNOSTIC_RESULTS;
	objIndex_t iid;
	struct json_object *jobj = NULL;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(iid);
	iid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_NeighboringWiFiDiagnostic.Result.%hhu", &iid.idx[0]);
	if((ret = feObjJsonGet(oid, &iid, &jobj, TR98_GET_WITHOUT_UPDATE)) != ZCFG_SUCCESS)
		return ret;

	json_object_put(jobj);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		/*special case*/

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
		break;
	}

	return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
zcfgRet_t lanDevWlanCfgWEPKeyObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	int idx = 0;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char wifiSsid[32] = {0};
	char ssidRef[32] = {0};
	int keylength = 0;
	char wepKey64ParmName[20] = {0}, wepKeyParmName[20] = {0};
	objIndex_t apIid;
	struct json_object *apJobj = NULL, *apSecJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
	bool setkey = false;
#endif
	

	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WEPKey.%d", &idx);
	/*There should be only one WEPKey Object*/
	if(idx > 4)
		return ZCFG_NO_MORE_INSTANCE;

	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WEPKey");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i.WLANConfiguration.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wifiSsid)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(apIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_WIFI_ACCESS_POINT, &apIid, &apJobj)) == ZCFG_SUCCESS) {
		strcpy(ssidRef, json_object_get_string(json_object_object_get(apJobj, "SSIDReference")));
		if(!strcmp(ssidRef, wifiSsid))
			break;
		else {
			json_object_put(apJobj);
			apJobj = NULL;
		}
	}

	if(ret == ZCFG_NO_MORE_INSTANCE)
		return ret;

	if(apJobj != NULL) {
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, &apSecJobj)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Get WiFi.AccessPoint.i.Security Fail\n", __FUNCTION__);
			json_object_put(apJobj);
			return ret;
		}
		json_object_put(apJobj);
	}

	if(multiJobj){
		tmpObj = apSecJobj;
		apSecJobj = NULL;
		apSecJobj = zcfgFeJsonMultiObjAppend(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "WEPKey")){
				keylength = strlen((const char *)json_object_get_string(paramValue));
				sprintf(wepKey64ParmName, "X_ZYXEL_WEP64Key%hhu", idx);
				sprintf(wepKeyParmName, "X_ZYXEL_WEPKey%hhu", idx);
				if((5 == keylength) || (10 == keylength)){
					json_object_object_add(apSecJobj, wepKey64ParmName, JSON_OBJ_COPY(paramValue));
					json_object_object_add(apSecJobj, "X_ZYXEL_WepModeSelect", json_object_new_string("WEP-64"));
					json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WEP-64"));
					paramList++;
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
					setkey= true;
#endif
					continue;
				}else if((13 == keylength) || (26 == keylength)){
					if(1 != idx){
						json_object_object_add(apSecJobj, wepKeyParmName, JSON_OBJ_COPY(paramValue));
					}else{
						json_object_object_add(apSecJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					}
					json_object_object_add(apSecJobj, "X_ZYXEL_WepModeSelect", json_object_new_string("WEP-128"));
					json_object_object_add(apSecJobj, "ModeEnabled", json_object_new_string("WEP-128"));
					paramList++;
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
					setkey= true;
#endif
					continue;
				}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				else
				{
					if(multiJobj){
						json_object_put(tmpObj);
					}
					else{
						json_object_put(apSecJobj);
					}					
					return ZCFG_INVALID_PARAM_VALUE;
				}
#endif
			}
			tr181ParamValue = json_object_object_get(apSecJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(apSecJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);
		paramList++;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
	if(setkey)
	{
		json_object_object_add(apSecJobj, "X_ZYXEL_AutoGenWepKey", json_object_new_boolean(false));
	}
#endif

	/*Set Device.WiFi.AccessPoint.i.Security*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apIid, apSecJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(apSecJobj);
			return ret;
		}

		json_object_put(apSecJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t lanIpIntfV6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	//char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	//char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//uint8_t prefixInstance = 0;
	char ipv6AddrName[MAX_TR181_PATHNAME] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
#if 0
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);

	*ptr = '\0';
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface) != ZCFG_SUCCESS) {
		/*	The instance number of WANIPConnection.i will be continuous. Therefore,
		 *	just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *	this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;
#endif
	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipv6AddrName) != ZCFG_SUCCESS)
		return ZCFG_NO_MORE_INSTANCE;

	IID_INIT(ipv6AddrIid);
	ipv6AddrIid.level = 2;
	if(sscanf(ipv6AddrName, "IP.Interface.%hhu.IPv6Address.%hhu", &ipv6AddrIid.idx[0], &ipv6AddrIid.idx[1]) != ipv6AddrIid.level)
		return ZCFG_NO_MORE_INSTANCE;

	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}


	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6AddrJobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "Prefix") == 0){//transfer
				strcpy(prefixObjName, json_object_get_string(paramValue));
				if(json_object_get_string(paramValue))
					strcpy(prefixObjName, json_object_get_string(paramValue));
				if(strlen(prefixObjName) && zcfgFe181To98MappingNameGet(prefixObjName, tr98Prefix) == ZCFG_SUCCESS){
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
				}
				else {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				}
				paramList++;
				continue;
#if 0
				ptr = strstr(prefixObjName, ".IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe181To98MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix),"%s.X_ZYXEL_IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
#endif
			}
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6AddrJobj);

	return ZCFG_SUCCESS;
}
/*
 *	 TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Address.i.
 *
 *	 Related object in TR181:
 *	 Device.IP.Interface.i
 *	 Device.IP.Interface.i.IPv6Address.i.
 */

zcfgRet_t lanIpIntfV6AddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t prefixInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;

	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}


	if(multiJobj){
		tmpObj = ipv6AddrJobj;
		ipv6AddrJobj = NULL;
		ipv6AddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*special case*/
			if(strcmp( paramList->name, "Prefix") == 0){//transfer
				strcpy(prefixObjName, json_object_get_string(paramValue));
				ptr = strstr(prefixObjName, ".X_ZYXEL_IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe98To181MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(ipv6AddrJobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6AddrJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6AddrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/

		}
		paramList++;
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, ipv6AddrJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
			json_object_put(ipv6AddrJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		}
		json_object_put(ipv6AddrJobj);
	}

	return ret;
}


/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t lanIpIntfV6AddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}




	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_ADDR, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Address Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];
	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t lanIpIntfV6AddrObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char ipIface[32] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}


	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_ADDR, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t lanIpIntfV6PrefixObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);

	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore,
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;

	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6PrefixJobj);

	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */

zcfgRet_t lanIpIntfV6PrefixObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;

	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}


	if(multiJobj){
		tmpObj = ipv6PrefixJobj;
		ipv6PrefixJobj = NULL;
		ipv6PrefixJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6PrefixJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, ipv6PrefixJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
			json_object_put(ipv6PrefixJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		}
		json_object_put(ipv6PrefixJobj);
	}

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t lanIpIntfV6PrefixObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}



	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_PREFIX, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Prefix Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];
	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t lanIpIntfV6PrefixObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char ipIface[32] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}


	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_RouterAdvertisement.
 *
 *   Related object in TR181:
 *   Device.RouterAdvertisement.
 *   Device.RouterAdvertisement.InterfaceSetting.i.
 */
zcfgRet_t lanIpIntfRouterAdverObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ifsetIid;
	struct json_object *ifsetJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_RouterAdvertisement");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore,
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}

	IID_INIT(ifsetIid);
	while((ret = feObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, &ifsetJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(ifsetJobj, "Interface")));
		if(!strcmp(ipIface, ifBuf))
			break;
		else {
			json_object_put(ifsetJobj);
			ifsetJobj = NULL;
		}
	}

	if(ifsetJobj == NULL)
		return ret;

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ifsetJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ifsetJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_RouterAdvertisement.
 *
 *   Related object in TR181:
 *   Device.RouterAdvertisement.
 *   Device.RouterAdvertisement.InterfaceSetting.i.
 */
zcfgRet_t lanIpIntfRouterAdverObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ifsetIid, ipIfaceIid;
	struct json_object *ifsetJobj = NULL, *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_RouterAdvertisement");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipIfaceIid);
	ipIfaceIid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj)) != ZCFG_SUCCESS) {
		printf("%s : Get IP.Interface fail\n", __FUNCTION__);
		json_object_put(ipIfaceJobj);
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(ifsetIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, &ifsetJobj)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(ifsetJobj, "Interface")));
		if(!strcmp(ipIface, ifBuf))
			break;
		else {
			json_object_put(ifsetJobj);
			ifsetJobj = NULL;
		}
	}

	if(ifsetJobj == NULL)
		return ret;


	if(multiJobj){
		if(ifsetJobj){
			tmpObj = ifsetJobj;
			ifsetJobj = NULL;
			ifsetJobj = zcfgFeJsonMultiObjAppend(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, multiJobj, tmpObj);
		}

		if(ipIfaceJobj){
			tmpObj = ipIfaceJobj;
			ipIfaceJobj = NULL;
			ipIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipIfaceIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "AdvManagedFlag")){
				if(ipIfaceJobj){
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_AdvManagedFlag", JSON_OBJ_COPY(paramValue));
				}
			}

			if(!strcmp(paramList->name, "AdvOtherConfigFlag")){
				if(ipIfaceJobj){
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_AdvOtherConfigFlag", JSON_OBJ_COPY(paramValue));
				}
			}

			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ifsetJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ifsetJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_RT_ADV_INTF_SET, &ifsetIid, ifsetJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d Fail\n", __FUNCTION__, ifsetIid.idx[0]);
			json_object_put(ifsetJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.RouterAdvertisement.InterfaceSetting.%d Fail\n", __FUNCTION__, ifsetIid.idx[0]);
		}
		json_object_put(ifsetJobj);
	}

	return ret;
}

#define zcfgStrCsvAppend(str1, str2){\
	int n = 0;\
	if((n = strlen(str1))) {char *ptr = str1+n; sprintf(ptr, ",%s", str2);}\
	else strcat(str1, str2);}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_DHCPv6Server.
 *
 *   Related object in TR181:
 *   Device.RouterAdvertisement.
 *   Device.RouterAdvertisement.InterfaceSetting.i.
 */
zcfgRet_t lanIpIntfV6SrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH*8] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *prefixBuf = NULL, *ptr = NULL;
	objIndex_t srvpoolIid;
	struct json_object *srvpoolJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	const char *prefixPtr = NULL;
	uint8_t prefixInstance = 0;
	char *token = NULL, *token_tmp = NULL;
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Server");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore,
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}

	IID_INIT(srvpoolIid);
	while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &srvpoolIid, &srvpoolJobj, updateFlag)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(srvpoolJobj, "Interface")));
		if(!strcmp(ipIface, ifBuf))
			break;
		else {
			json_object_put(srvpoolJobj);
			srvpoolJobj = NULL;
		}
	}

	if(srvpoolJobj == NULL)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(srvpoolJobj, paramList->name);
		if(paramValue != NULL) {
			/*special case*/
			if(strcmp( paramList->name, "IANAPrefixes") == 0){//transfer
				prefixPtr = json_object_get_string(paramValue);
				if(!strcmp(prefixPtr,"")){
					paramValue = json_object_object_get(srvpoolJobj, "IANAManualPrefixes");
					if(!paramValue){
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
						paramList++;
						continue;
					}
					prefixPtr = json_object_get_string(paramValue);
				}
				prefixBuf = strdup(prefixPtr);
				if(prefixBuf){
					token = strtok_r(prefixBuf, ",", &token_tmp);
					while(token){
						strcpy(prefixObjName, token);
						if(zcfgFe181To98MappingNameGet(prefixObjName, tr98ObjName) == ZCFG_SUCCESS)
							zcfgStrCsvAppend(tr98Prefix, tr98ObjName);
#if 0
						ptr = strstr(prefixObjName, ".IPv6Prefix");
						if(ptr){
							sscanf(ptr, ".IPv6Prefix.%hhu", &prefixInstance);
							*ptr = '\0';
							if(zcfgFe181To98MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
								if(strlen(tr98Prefix) >0)
									snprintf(tr98Prefix, sizeof(tr98Prefix),"%s,%s.X_ZYXEL_IPv6Prefix.%d", tr98Prefix, ipIface, prefixInstance);
								else
									snprintf(tr98Prefix, sizeof(tr98Prefix),"%s.X_ZYXEL_IPv6Prefix.%d", ipIface, prefixInstance);
							}
						}
#endif
						token = strtok_r(token_tmp, ",", &token_tmp);
					}
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
					free(prefixBuf);
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}

			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(srvpoolJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_DHCPv6Server.
 *
 *   Related object in TR181:
 *   Device.DHCPv6.Server.
 *   Device.DHCPv6.Server.Pool.i.
 */
zcfgRet_t lanIpIntfV6SrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t srvpoolIid;
	struct json_object *srvpoolJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	uint8_t prefixInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	strcpy(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Server");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(srvpoolIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &srvpoolIid, &srvpoolJobj)) == ZCFG_SUCCESS) {
		strcpy(ifBuf, json_object_get_string(json_object_object_get(srvpoolJobj, "Interface")));
		if(!strcmp(ipIface, ifBuf))
			break;
		else {
			json_object_put(srvpoolJobj);
			srvpoolJobj = NULL;
		}
	}

	if(srvpoolJobj == NULL)
		return ret;


	if(multiJobj){
		tmpObj = srvpoolJobj;
		srvpoolJobj = NULL;
		srvpoolJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_SRV_POOL, &srvpoolIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*special case*/
			if(strcmp( paramList->name, "IANAPrefixes") == 0){//transfer
				paramList++;
				continue;
			}
			else if(strcmp( paramList->name, "IANAManualPrefixes") == 0){//transfer
				strcpy(prefixObjName, json_object_get_string(paramValue));
				ptr = strstr(prefixObjName, ".X_ZYXEL_IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe98To181MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(srvpoolJobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(srvpoolJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(srvpoolJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_SRV_POOL, &srvpoolIid, srvpoolJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Server.Pool.%d Fail\n", __FUNCTION__, srvpoolIid.idx[0]);
			json_object_put(srvpoolJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Server.Pool.%d Fail\n", __FUNCTION__, srvpoolIid.idx[0]);
		}
		json_object_put(srvpoolJobj);
	}

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VendorSpecific
 *
 *   Related object in TR181: Device.X_ZYXEL_Option125VendorSpecific
 *
 */
 #ifdef ZyXEL_DHCP_OPTION125
zcfgRet_t zyExtVendorSpecObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vendorSpecJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_OPTION125, &objIid, &vendorSpecJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(vendorSpecJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(vendorSpecJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtVendorSpecObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault){
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vendorSpecJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_OPTION125, &objIid, &vendorSpecJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = vendorSpecJobj;
		vendorSpecJobj = NULL;
		vendorSpecJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_OPTION125, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(vendorSpecJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(vendorSpecJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_OPTION125, &objIid, vendorSpecJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(vendorSpecJobj);
			return ret;
		}
		json_object_put(vendorSpecJobj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_EXT.VendorSpecific.Profile.i
 *
 *   Related object in TR181: Device.X_ZYXEL_Option125VendorSpecific.VendorSpecific.i
 *
 */
zcfgRet_t zyExtVendorSpecProfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vendorSpecProfJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VendorSpecific.Profile.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, &vendorSpecProfJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(vendorSpecProfJobj, paramList->name);
		if(paramValue != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}

	json_object_put(vendorSpecProfJobj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtVendorSpecProfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *vendorSpecProfJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VendorSpecific.Profile.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, &vendorSpecProfJobj)) != ZCFG_SUCCESS){
		return ret;
	}

	if(multiJobj){
		tmpObj = vendorSpecProfJobj;
		vendorSpecProfJobj = NULL;
		vendorSpecProfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL){
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL){
			tr181ParamValue = json_object_object_get(vendorSpecProfJobj, paramList->name);
			if(tr181ParamValue != NULL){
				json_object_object_add(vendorSpecProfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, vendorSpecProfJobj, NULL)) != ZCFG_SUCCESS ){
			json_object_put(vendorSpecProfJobj);
			return ret;
		}
		json_object_put(vendorSpecProfJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zyExtVendorSpecProfObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	ret = zcfgFeObjStructAdd(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, NULL);
	*idx = objIid.idx[0];

	return ret;
}

zcfgRet_t zyExtVendorSpecProfObjDel(char *tr98FullPathName)
{
	objIndex_t objIid;

	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_EXT.VendorSpecific.Profile.%hhu", &objIid.idx[0]);

	return zcfgFeObjStructDel(RDM_OID_ZY_VENDOR_SPECIFIC, &objIid, NULL);;
}
#endif
