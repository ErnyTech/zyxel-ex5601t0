#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zyutil.h"
#include "zyutil_sfp.h"
#include "zos_api.h"

#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)
#define isEmptyIns(obj) json_object_get_boolean(json_object_object_get(obj, "emptyIns"))
#ifdef ZYXEL_PPTPD_RELAY
#define pptp_uptime_file  "/var/run/pptp_uptime_file"
#include <sys/sysinfo.h>
#define TR181_PPtP2PPPOA "PPtPtoPPPoA"
#endif

dal_param_t NETWORK_MAP_param[] = 
{
	{"Internet_Blocking_Enable",	    dalType_boolean,	0,	0,	NULL},
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
	{"Internet_Priority_Enable",		dalType_boolean,	0,	0,	NULL},
	{"Device_StaticDhcp_Enable",	    dalType_boolean,	0,	0,	NULL},
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	{"BrowsingProtection",	    dalType_boolean,	0,	0,	NULL},
	{"TrackingProtection",	    dalType_boolean,	0,	0,	NULL},
	{"IOTtection",	    dalType_boolean,	0,	0,	NULL},
	{"Profile",	    dalType_string,	0,	0,	NULL},
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	{"X_ZYXEL_FirstSeen",			dalType_int,		0,	0,	NULL},
	{"X_ZYXEL_LastSeen",			dalType_int,		0,	0,	NULL},
	{"SourceVendorClassID",			dalType_string,		0,	0,	NULL},	
#endif	
	{"HostName",						dalType_string,		0,	0,	NULL},
	{"IPAddress",						dalType_string,		0,	0,	NULL},
	{"IPLinkLocalAddress6",				dalType_string,		0,	0,	NULL},
	{"PhysAddress",						dalType_string,		0,	0,	NULL},
	{"AddressSource",					dalType_string,		0,	0,	NULL},
	{"X_ZYXEL_ConnectionType",			dalType_string,		0,	0,	NULL},
	{"DeviceSource",					dalType_string,		0,	0,	NULL},
	{NULL,		0,	0,	0,	NULL}
};

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

bool genIid(struct json_object **iid, int iid1, int iid2, int iid3, int iid4, int iid5, int iid6){
	
	*iid = json_object_new_array();
	json_object_array_add(*iid, json_object_new_int(iid1));
	json_object_array_add(*iid, json_object_new_int(iid2));
	json_object_array_add(*iid, json_object_new_int(iid3));
	json_object_array_add(*iid, json_object_new_int(iid4));
	json_object_array_add(*iid, json_object_new_int(iid5));
	json_object_array_add(*iid, json_object_new_int(iid6));
	
	return true;
}
void getDHCPStatus(char *status, int *leaseTime, char *tr181Path, char *option42Enable, char *option42value, const char *type, const char *ipIfacePath, struct json_object *dhcp4PoolJarray, struct json_object *dhcp4ClientJarray){
	int len, i, len2, j, tag;
	const char *Interface = NULL, *DHCPStatus = NULL, *value = NULL;
	bool option42 = false;
	struct json_object *dhcp4PoolJobj, *dhcp4ClientJobj, *dhcp4ReqOptJarray, *dhcp4ReqOptObj;
	strcpy(status, "Disable");
	strcpy(tr181Path, "");

	if(!strcmp(type, "LAN")){
		len = json_object_array_length(dhcp4PoolJarray);
		for(i=0;i<len;i++){
			dhcp4PoolJobj = json_object_array_get_idx(dhcp4PoolJarray, i);
			Interface = Jgets(dhcp4PoolJobj, "Interface");
			if(Interface == NULL)
				continue;

			if(!strcmp(ipIfacePath, Interface)){
				sprintf(tr181Path, "Device.DHCPv4.Server.Pool.%d", i+1);
				if(Jgetb(dhcp4PoolJobj, "Enable"))
					strcpy(status, "Server");
				break;
			}
		}
	}
	else{
		len = json_object_array_length(dhcp4ClientJarray);
		for(i=0;i<len;i++){
			dhcp4ClientJobj = json_object_array_get_idx(dhcp4ClientJarray, i);
			Interface = Jgets(dhcp4ClientJobj, "Interface");
			if(Interface == NULL)
				continue;

			if(!strcmp(ipIfacePath, Interface)){
				sprintf(tr181Path, "Device.DHCPv4.Client.%d", i+1);
				DHCPStatus = Jgets(dhcp4ClientJobj, "DHCPStatus");
				if(DHCPStatus == NULL)
					strcpy(status, "");
				else
					strcpy(status, DHCPStatus);

				*leaseTime = Jgeti(dhcp4ClientJobj, "LeaseTime");

				dhcp4ReqOptJarray = Jget(dhcp4ClientJobj, "ReqOption");
				if(dhcp4ReqOptJarray != NULL){
				len2 = json_object_array_length(dhcp4ReqOptJarray);
				for( j = 0; j<len2; j++){
					dhcp4ReqOptObj = json_object_array_get_idx(dhcp4ReqOptJarray, j);
					tag = Jgeti(dhcp4ReqOptObj,"Tag");
					if(tag == 42){
						option42 = Jgetb(dhcp4ReqOptObj,"Enable");
						value = Jgets(dhcp4ReqOptObj,"Value");
						if(option42){
							strcpy(option42Enable,"true");
						}
						else{
							strcpy(option42Enable,"false");
						}
						if(value == NULL){
							strcpy(option42value, "");
						}
						else{
							strcpy(option42value, value);
						}
						break;
					}
				}
				}
				else{
					strcpy(option42Enable,"false");
					strcpy(option42value, "");
				}
				break;
			}
		}
	}
}

/*
 * return Status get PPP uptime every time.
 *  @param[IN]	   inputKey: PPP obj index
 *  @param[OUT] varValue: PPP lastConnectionUpTime
 */
int getPPPUptime(unsigned char idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pppIfaceObj = NULL;
	objIndex_t pppIfaceIid = {0};
	int X_ZYXEL_LastConnectionUpTime = 0;

	IID_INIT(pppIfaceIid);
	pppIfaceIid.idx[0] = idx;
	pppIfaceIid.level = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj)) == ZCFG_SUCCESS){
		X_ZYXEL_LastConnectionUpTime = json_object_get_int(json_object_object_get(pppIfaceObj, "X_ZYXEL_LastConnectionUpTime"));
		zcfgFeJsonObjFree(pppIfaceObj);
	}

	return X_ZYXEL_LastConnectionUpTime;
}
#ifdef ZYXEL_PPTPD_RELAY
int getPPTPUptime(unsigned char idx){
	int ConnectionUpTime = 0;
	struct sysinfo cur_up;
	sysinfo(&cur_up);
	char ptmp[256]={0};
	FILE *fp = NULL;

	memset(ptmp, 0, sizeof(ptmp));
	if((fp = fopen(pptp_uptime_file, "r")) != NULL){
		if(fgets(ptmp, sizeof(ptmp), fp) != NULL){
			ConnectionUpTime = cur_up.uptime - atoi(ptmp);
		}
		fclose(fp);
	}

	return ConnectionUpTime;
}
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
int getPPPUpCounterandDownReason(unsigned char idx,char *DownReason){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pppIfaceObj = NULL;
	objIndex_t pppIfaceIid = {0};
	int counter = 0;

	IID_INIT(pppIfaceIid);
	pppIfaceIid.idx[0] = idx;
	pppIfaceIid.level = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj)) == ZCFG_SUCCESS){
		counter = json_object_get_int(json_object_object_get(pppIfaceObj, "X_TTNET_counter"));
		strcpy(DownReason,json_object_get_string(json_object_object_get(pppIfaceObj, "X_TTNET_ConnectionDown")));
		zcfgFeJsonObjFree(pppIfaceObj);
	}

	return counter;
}

int getIPUpCounterandDownReason(unsigned char idx,char *DownReason){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t ipIfaceIid = {0};
	int counter = 0;

	IID_INIT(ipIfaceIid);
	ipIfaceIid.idx[0] = idx;
	ipIfaceIid.level = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj)) == ZCFG_SUCCESS){
		counter = json_object_get_int(json_object_object_get(ipIfaceObj, "X_TTNET_counter"));
		strcpy(DownReason,json_object_get_string(json_object_object_get(ipIfaceObj, "X_TTNET_ConnectionDown")));
		zcfgFeJsonObjFree(ipIfaceObj);
	}

	return counter;
}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
/*
 * return WLAN and ETH Port list in Jason Array.
 * Format:Jarray.i.NAME = Name
 *		 Jarray.i.SSID = Name
 *           Jarray.i.IntfPath : Ethernet.Interface.i|WiFi.SSID.i
 *           Jarray.i.X_ZYXEL_LanPort
 *           Jarray.i.Status : Up|Down
 *           Jarray.i.Channel :
 *           Jarray.i.AutoChannelEnable :
 *           Jarray.i.OperatingStandards : b,g,n,a,ac
 *           Jarray.i.ModeEnabled : None|WEP-64|WEP-128|WPA-Personal|WPA2-Personal|WPA-WPA2-Personal
 *           Jarray.i.WPSEnable : true|false
 *           Jarray.i.X_ZYXEL_MainSSID : true|false
 */
zcfgRet_t getWifiInfo(struct json_object **Jarray){
	objIndex_t iid, secIid;	
	struct json_object *Jobj = NULL;
	struct json_object *wifiRadioJarray, *wifiRadioObj, *wifiRadioStObj, *wifiSsidJarray, *wifiSsidObj;
	struct json_object *wifiApJarray, *wifiApObj, *wifiApSecObj, *wifiApWpsObj, *wifiSecObj;
	char intfPath[64] = {0};
#if 0
	char brPortPath[64] = {0};
#endif
	const char *LowerLayers, *SSID, *SSIDReference;
	unsigned char idx;
	int len, i;
#ifdef ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
	bool ssidenable = false, radioenable = false;
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
	char cmdArg[64] = {0};
	FILE *fp;
	char *Ifname = NULL;
#endif

	//update RDM_OID_WIFI_RADIO and RDM_OID_WIFI_RADIO_ST
	wifiRadioJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_RADIO, &iid, &wifiRadioObj) == ZCFG_SUCCESS) {
		json_object_array_add(wifiRadioJarray, wifiRadioObj);
		zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO_ST, &iid, &wifiRadioStObj);
		json_object_object_add(wifiRadioObj, "Stats", wifiRadioStObj);
	}

	zcfgFeWholeObjJsonGet("Device.WiFi.AccessPoint", &wifiApJarray);
	zcfgFeWholeObjJsonGet("Device.WiFi.SSID", &wifiSsidJarray);
	
	*Jarray = json_object_new_array();

	len = json_object_array_length(wifiApJarray);
	for(i=0;i<len;i++){
		Jobj = json_object_new_object();
		secIid.level = 1;
		secIid.idx[0] = i + 1;
		if(zcfgFeObjJsonGet(RDM_OID_WIFI_ACCESS_POINT_SEC, &secIid, &wifiSecObj) == ZCFG_SUCCESS){
			if(strstr(json_object_get_string(json_object_object_get(wifiSecObj, "ModeEnabled")),"WPA") != NULL){
				if(strlen(json_object_get_string(json_object_object_get(wifiSecObj, "KeyPassphrase")) ) > 0){
					json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "KeyPassphrase")));
				}
				else{
					json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "PreSharedKey")));
				}
			}
			else if(!strcmp(json_object_get_string(json_object_object_get(wifiSecObj, "ModeEnabled")),"WEP-64")){
				if(json_object_get_boolean(json_object_object_get(wifiSecObj, "X_ZYXEL_AutoGenWepKey")) == false) {
					if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 1){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEP64Key1")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 2){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEP64Key2")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 3){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEP64Key3")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 4){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEP64Key4")));
					}
				}
				else{
					json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_Auto_WepKey10")));
				}
			}
			else if(!strcmp(json_object_get_string(json_object_object_get(wifiSecObj, "ModeEnabled")),"WEP-128")){
				if(json_object_get_boolean(json_object_object_get(wifiSecObj, "X_ZYXEL_AutoGenWepKey")) == false) {
					if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 1){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "WEPKey")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 2){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEPKey2")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 3){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEPKey3")));
					}
					else if(json_object_get_int(json_object_object_get(wifiSecObj, "X_ZYXEL_DefaultKeyID")) == 4){
						json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_WEPKey4")));
					}
				}
				else{
					json_object_object_add(Jobj, "wifiPassword", JSON_OBJ_COPY(json_object_object_get(wifiSecObj, "X_ZYXEL_Auto_WepKey26")));
				}
			}
			zcfgFeJsonObjFree(wifiSecObj);
		}
		
		wifiApObj = json_object_array_get_idx(wifiApJarray, i);
		wifiApSecObj = Jget(wifiApObj, "Security");
		wifiApWpsObj = Jget(wifiApObj, "WPS");
		SSIDReference = Jgets(wifiApObj, "SSIDReference");
		sscanf(SSIDReference, "WiFi.SSID.%hhu", &idx);
		wifiSsidObj = json_object_array_get_idx(wifiSsidJarray, idx-1);
		
		sprintf(intfPath, "WiFi.SSID.%u", idx);
		SSID = json_object_get_string(json_object_object_get(wifiSsidObj, "SSID"));
		LowerLayers = json_object_get_string(json_object_object_get(wifiSsidObj, "LowerLayers"));
		sscanf(LowerLayers, "WiFi.Radio.%hhu", &idx);
		wifiRadioObj = json_object_array_get_idx(wifiRadioJarray, idx-1);
		wifiRadioStObj = Jget(wifiRadioObj, "Stats");
#ifdef ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
		ssidenable = Jgetb(wifiSsidObj, "Enable");
		radioenable = Jgetb(wifiRadioObj, "Enable");
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
       // wlan real status ; in case disable by wlan scheduler
		Ifname = (char *)json_object_get_string(json_object_object_get(wifiSsidObj, "Name"));
		sprintf(cmdArg, "wlctl -i %s bss", Ifname);
		fp = popen(cmdArg,"r");
		if(fp != NULL){
			char wlan_status_check[128] = {0};
			if ( fgets(wlan_status_check, sizeof(wlan_status_check)-1, fp) != NULL){
				if(strstr(wlan_status_check, "up")){
					json_object_object_add(Jobj, "wlstatus", json_object_new_boolean(true));
				}else{
					json_object_object_add(Jobj, "wlstatus",json_object_new_boolean(false));
				}
			}else
				json_object_object_add(Jobj, "wlstatus", json_object_new_boolean(true));
			pclose(fp);
		}else
			json_object_object_add(Jobj, "wlstatus", json_object_new_boolean(true));
#endif
#if	0
		BrPortPath = json_object_get_string(json_object_object_get(mappingObj, intfPath));
		if(BrPortPath == NULL)
			BrPortPath = "";
#endif
		
		//Jobj = json_object_new_object();
		json_object_object_add(Jobj, "SSID", json_object_new_string(SSID));
		//json_object_object_add(Jobj, "BrPortPath", json_object_new_string(BrPortPath));
		json_object_object_add(Jobj, "IntfPath", json_object_new_string(intfPath));
#if defined(ZYXEL_RADIO_MAIN_SSID_INDEPENDENT) && !defined(ZYXEL_WIND_ITALY_CUSTOMIZATION)
		if (radioenable && ssidenable )
			json_object_object_add(Jobj, "Enable", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "Enable", json_object_new_boolean(false));
#else
		replaceParam(Jobj, "Enable", wifiSsidObj, "Enable");
#endif
		replaceParam(Jobj, "MACAddress", wifiSsidObj, "MACAddress");
		replaceParam(Jobj, "X_ZYXEL_MainSSID", wifiSsidObj, "X_ZYXEL_MainSSID");

		replaceParam(Jobj, "OperatingFrequencyBand", wifiRadioObj, "OperatingFrequencyBand");
		replaceParam(Jobj, "OperatingChannelBandwidth", wifiRadioObj, "OperatingChannelBandwidth");
		replaceParam(Jobj, "Channel", wifiRadioObj, "Channel");
#ifdef ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
        replaceParam(Jobj, "RadioEnable", wifiRadioObj, "Enable");
#endif
		replaceParam(Jobj, "AutoChannelEnable", wifiRadioObj, "AutoChannelEnable");
		replaceParam(Jobj, "OperatingStandards", wifiRadioObj, "OperatingStandards");
		//replaceParam(Jobj, "X_ZYXEL_Wireless_Mode", wifiRadioObj, "X_ZYXEL_Wireless_Mode");
		replaceParam(Jobj, "X_ZYXEL_Rate", wifiRadioStObj, "X_ZYXEL_Rate");
		replaceParam(Jobj, "ModeEnabled", wifiApSecObj, "ModeEnabled");
		replaceParam(Jobj, "WPSEnable", wifiApWpsObj, "Enable");
			
		json_object_array_add(*Jarray, Jobj);
	}
	
	json_object_put(wifiRadioJarray);
	json_object_put(wifiApJarray);
	json_object_put(wifiSsidJarray);
	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

/*
 * return WAN and LAN list in Jason Array.
 * Format:Jarray.i.NAME = Name
 *           Jarray.i.IpIfacePath = IP.Interface.i
 *           Jarray.i.wanIdx
 *           Jarray.i.wanpppIdx
 *           Jarray.i.X_ZYXEL_Type = WAN| LAN
 *           Jarray.i.LinkType = ATM|PTM|ETH|PON|USB (For WAN only)
 *           Jarray.i.X_ZYXEL_ConnectionType = IP_Routed|IP_Bridged (For WAN only)
 *           Jarray.i.Encapsulation = IPoE|IPoA|PPPoE|PPPoA (For WAN only, NUL if LinkType is USB))
 *           Jarray.i.BindToIntfGrp = true|false (For WAN only)
 *           Jarray.i.Group_WAN_IpIface = IP.Interface.i,IP.Interface.i,IP.Interface.i,... (For LAN only)
 *           Jarray.i.BridgingBrPath = Bridging.Bridge.i (For LAN only)
 *
 *		 Jarray.i.IPAddress = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.SubnetMask = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.MACAddress = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.PrimaryDNSserver = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.SecondaryDNSserver = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.IPAddress = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.DHCP = Bridging.Bridge.i (For LAN only)
 *		 Jarray.i.LowerlayerUp (For PPPoE or PPPoA case)
 *		 Jarray.i.pppConnectionStatus (For PPP)
 *		 Jarray.i.wanpppIdx (For PPP)
 *		 Jarray.i.DHCPtr181Path
 */
zcfgRet_t getWanLanList_ext(struct json_object **Jarray)
{
	struct json_object *ipIfaceJarray, *pppIfaceJarray, *v4AddrJarray, *v6AddrJarray, *v6PrefixJarray, *vlanTermJarray, *atmLinkJarray, *bridgeBrJarray, *dhcp4PoolJarray, *dhcp4ClientJarray, *dnsClientJarray = NULL, *rtInfoIntfJarray = NULL;
	struct json_object *ipIfaceObj, *pppIfaceObj=NULL, *v4AddrObj, *v6AddrObj, *v6PrefixObj, *v4GwObj, *v6GwObj, *vlanTermObj, *ethLinkObj = NULL, *ethIfaceObj = NULL, *atmLinkObj, *bridgeBrObj, *dnsClientObj, *rtInfoObj;
	struct json_object *Jobj = NULL;
	struct json_object *addrJarray, *dnsv4Array = NULL, *dnsv6Array = NULL, *rtInfoArray = NULL;
	char ifacePath[32] = {0}, BridgingBrPath[32] = {0};
	int len = 0, len2 = 0, i = 0, j = 0;
	unsigned char idx;
	const char *X_ZYXEL_BridgeName, *X_ZYXEL_SrvName, *X_ZYXEL_ConnectionType, *X_ZYXEL_Group_WAN_IpIface, *LowerLayers;
	const char *X_ZYXEL_Type, *dns_tmp, *ipIface_str, *gateway4_str, *gateway6_str;
	char intfGrpWANList[512] = {0};
	char *intf = NULL, *tmp_ptr = NULL, *DefaultGWIface = NULL;
	const char *IpIfacePath = NULL, *DefaultGWList = NULL;
	char DHCPStatus[16] = {0}, DHCPtr181Path[32] = {0}, ipv6dns[128] = {0}, ipv4dns[32] = {0}, DHCPoption42Value[64] = {0}, DHCPoption42[8] = {0}; 
 	objIndex_t iid;	
	bool Enable = false, isDefaultGateway = false;
   	int X_ZYXEL_LastConnectionUpTime = 0;
	struct json_object *ipIfaceObject, *routingObj;
	int DHCPLeaseTime = 0;
	int internetUpTime = 0;
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	const char *X_TAAAB_SrvName = NULL;
#endif

	zcfgFeWholeObjJsonGet("Device.IP.Interface.", &ipIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.PPP.Interface.", &pppIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.VLANTermination.", &vlanTermJarray);
	zcfgFeWholeObjJsonGet("Device.ATM.Link.", &atmLinkJarray);
	zcfgFeWholeObjJsonGet("Device.Bridging.Bridge.", &bridgeBrJarray);
	zcfgFeWholeObjJsonGet("Device.DHCPv4.Server.Pool.", &dhcp4PoolJarray);
	zcfgFeWholeObjJsonGet("Device.DHCPv4.Client.", &dhcp4ClientJarray);
	zcfgFeWholeObjJsonGet("Device.DNS.Client.Server.", &dnsClientJarray);
	zcfgFeWholeObjJsonGet("Device.Routing.RouteInformation.InterfaceSetting.", &rtInfoIntfJarray);

	if(ipIfaceJarray==NULL||pppIfaceJarray==NULL||vlanTermJarray==NULL||bridgeBrJarray==NULL)
		return ZCFG_INTERNAL_ERROR;

	*Jarray = json_object_new_array();

	len = json_object_array_length(ipIfaceJarray);
	for(i=0;i<len;i++){
		ipIfaceObj = json_object_array_get_idx(ipIfaceJarray, i);
		Enable = json_object_get_boolean(json_object_object_get(ipIfaceObj, "Enable"));
		X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
		X_ZYXEL_ConnectionType = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType"));
		LowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
		X_ZYXEL_Group_WAN_IpIface = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
		if(json_object_object_get(ipIfaceObj, "X_ZYXEL_UpTime")){
			internetUpTime = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_UpTime"));
		}
		else{
			internetUpTime = 0;
		}
		sprintf(ifacePath, "IP.Interface.%u", i+1);
		if(X_ZYXEL_SrvName==NULL || X_ZYXEL_ConnectionType==NULL || LowerLayers==NULL){
			continue;
		}
		
		Jobj = json_object_new_object();
		Jaddb(Jobj, "Enable", Enable);
		Jadds(Jobj, "IpIfacePath", ifacePath);
		Jaddi(Jobj, "wanIdx", i+1);
		Jaddb(Jobj, "BindToIntfGrp", false);
		Jadds(Jobj, "X_ZYXEL_Type", "WAN");
		Jadds(Jobj, "Encapsulation", "");
		Jadds(Jobj, "MACAddress", "");
		Jadds(Jobj, "LowerlayerUp", "");
		Jadds(Jobj, "pppConnectionStatus", "");
		Jaddi(Jobj, "wanpppIdx", 0);
		Jadds(Jobj, "DHCPtr181Path", "");
		Jaddi(Jobj, "ipoeConnectionUpTime", internetUpTime);
		replaceParam(Jobj, "Status", ipIfaceObj, "Status");
		replaceParam(Jobj, "X_ZYXEL_ConnectionType", ipIfaceObj, "X_ZYXEL_ConnectionType");
		replaceParam(Jobj, "X_ZYXEL_SrvName", ipIfaceObj, "X_ZYXEL_SrvName");
		replaceParam(Jobj, "Name", ipIfaceObj, "Name");
		replaceParam(Jobj, "Type", ipIfaceObj, "Type");
		replaceParam(Jobj, "IPv4Enable", ipIfaceObj, "IPv4Enable");
		replaceParam(Jobj, "IPv6Enable", ipIfaceObj, "IPv6Enable");
		replaceParam(Jobj, "X_ZYXEL_IfName", ipIfaceObj, "X_ZYXEL_IfName");
		replaceParam(Jobj, "IPv6Origin", ipIfaceObj, "X_ZYXEL_IPv6Origin");
		replaceParam(Jobj, "IPv6PrefixDelegateWAN", ipIfaceObj, "X_ZYXEL_IPv6PrefixDelegateWAN");
		replaceParam(Jobj, "X_ZYXEL_DefaultGatewayIface", ipIfaceObj, "X_ZYXEL_DefaultGatewayIface");
		Jadds(Jobj, "X_ZYXEL_IPv6LocalAddress", "");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
				X_TAAAB_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_TT_SrvName"));
#ifdef TAAAB_HGW
												char *tmp_string = NULL;
												char newName[128] = {0};
												if(strcmp(X_ZYXEL_SrvName,"Default") == 0 || strcmp(X_ZYXEL_SrvName,"WWAN") == 0)
													json_object_object_add(Jobj, "HGWStyleName", json_object_new_string(X_ZYXEL_SrvName));	
												else{
												tmp_string = strrchr(X_TAAAB_SrvName,'_');
												strncpy(newName, X_TAAAB_SrvName, strlen(X_TAAAB_SrvName)-strlen(tmp_string));
												newName[strlen(X_TAAAB_SrvName)-strlen(tmp_string)] = '\0';
												json_object_object_add(Jobj, "HGWStyleName", json_object_new_string(newName));
												}
#endif
#endif
		
#if 1	/* __ZyXEL__, GraceXiao, 20171225, #32394 The IPv6 Link Local Address can not display in Status webpage. */
		//update X_ZYXEL_IPv6LocalAddress
		IID_INIT(iid);
		iid.level = 1;
		iid.idx[0] = i+1;
		if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &iid, &ipIfaceObject) == ZCFG_SUCCESS) {
			replaceParam(Jobj, "X_ZYXEL_IPv6LocalAddress", ipIfaceObject, "X_ZYXEL_IPv6LocalAddress");
			json_object_put(ipIfaceObject);
		}
#endif
		
		/*For getting Default Gateway List*/
		IID_INIT(iid);
		iid.idx[0] = 1;
		iid.level = 1;
		if(zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &iid, &routingObj) == ZCFG_SUCCESS) {
			DefaultGWList = json_object_get_string(json_object_object_get(routingObj, "X_ZYXEL_ActiveDefaultGateway"));
			
			isDefaultGateway = false;
			DefaultGWIface = strtok(DefaultGWList, ",");
			while(DefaultGWIface != NULL){
				if(!strcmp(DefaultGWIface, ifacePath)){
					isDefaultGateway = true;
				}
				DefaultGWIface = strtok(NULL, ",");
			}
			Jaddb(Jobj, "DefaultGateway", isDefaultGateway);

			zcfgFeJsonObjFree(routingObj);
		}

		/*Address info*/
		addrJarray =  json_object_new_array();
		json_object_object_add(Jobj, "IPv4Address", addrJarray);
		v4AddrJarray = Jget(ipIfaceObj, "IPv4Address");
		if(v4AddrJarray != NULL){
			len2 = json_object_array_length(v4AddrJarray);
			for(j=0;j<len2;j++){
				v4AddrObj = json_object_array_get_idx(v4AddrJarray, j);
				if(isEmptyIns(v4AddrObj))
					continue;
				Jaddi(v4AddrObj, "ipIfaceIdx", i+1);
				json_object_array_add(addrJarray, JSON_OBJ_COPY(v4AddrObj));
				break;
			}
		}

		addrJarray =  json_object_new_array();
		json_object_object_add(Jobj, "IPv6Address", addrJarray);
		v6AddrJarray = Jget(ipIfaceObj, "IPv6Address");
		if(v6AddrJarray != NULL){
			len2 = json_object_array_length(v6AddrJarray);
			for(j=0;j<len2;j++){
				v6AddrObj = json_object_array_get_idx(v6AddrJarray, j);
				if(isEmptyIns(v6AddrObj))
					continue;
				Jaddi(v6AddrObj, "ipIfaceIdx", i+1);
				json_object_array_add(addrJarray, JSON_OBJ_COPY(v6AddrObj));
			}
		}
#if 1	//	temporary solution for Bug #151507  
		else{
			if(json_object_get_boolean(json_object_object_get(ipIfaceObj, "IPv6Enable")) == true){
				FILE *ifconfigfp = NULL;
				char tmpaddr[256] = {'\0'};
				char tmp[64] = {'\0'},tmp1[64] = {'\0'},tmp2[64] = {'\0'};
				int ret=0;
				char cmd[64] = {'\0'};
				strncpy(cmd,"ifconfig ",10);
				strncat(cmd, json_object_get_string (json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName")),20);
				//printf("cmd:%s\n",cmd);
				ifconfigfp = popen(cmd, "r");
				if(ifconfigfp != NULL)
				{
					while ((fgets(tmpaddr, 255, ifconfigfp)) != NULL)
					{
						if (strstr(tmpaddr, "Scope:Global") != NULL)
						{
							//sscanf(tmpaddr,"%[^:]: %[^/]/%[a-zA-Z0-9._/:- ]",tmp,tmp1,tmp2);
							ret = sscanf(tmpaddr,"%[^:]: %[^ ] %[a-zA-Z0-9._/-:]",tmp,tmp1,tmp2);
							//printf("tmp:%s tmp1:%s tmp2:%s\n",tmp ,tmp1 ,tmp2);
						}
					}
				}
				pclose(ifconfigfp);
				
				if(ret == 3){								
					v6AddrObj =  json_object_new_object();
					json_object_object_add(v6AddrObj, "IPAddress", json_object_new_string(tmp1));
					Jadds(v6AddrObj, "Origin", "AutoConfigured");
					Jadds(v6AddrObj, "Status", "Enabled");
					json_object_array_add(addrJarray, JSON_OBJ_COPY(v6AddrObj));
					//printf("%d %s\n", __LINE__, __func__);
					//printf("Jobj=%s\n", json_object_to_json_string(Jobj));
					//printf("v6AddrObj=%s\n", json_object_to_json_string(v6AddrObj));
				}
			}
		}
#endif

		/* V6 prefix info */
		addrJarray = json_object_new_array();
		json_object_object_add(Jobj, "IPv6Prefix", addrJarray);
		v6PrefixJarray = Jget(ipIfaceObj, "IPv6Prefix");
		if(v6PrefixJarray != NULL) {
			len2 = json_object_array_length(v6PrefixJarray);
			for(j = 0; j < len2; j++) {
				v6PrefixObj = json_object_array_get_idx(v6PrefixJarray, j);
				if(isEmptyIns(v6PrefixObj))
					continue;
				json_object_array_add(addrJarray, JSON_OBJ_COPY(v6PrefixObj));
			}
		}
		
		/* V4 Gateway info */
		IID_INIT(iid);

		while(zcfgFeObjJsonGetNext(RDM_OID_ROUTING_ROUTER_V4_FWD, &iid, &v4GwObj) == ZCFG_SUCCESS)
		{
			ipIface_str = json_object_get_string (json_object_object_get(v4GwObj, "Interface"));

			if(strcmp(ipIface_str, ifacePath) == 0)
			{
				gateway4_str = json_object_get_string (json_object_object_get(v4GwObj, "GatewayIPAddress"));
				Jadds(Jobj, "v4Gateway", gateway4_str);
			}
			json_object_put(v4GwObj);
		}
        
		/* V6 Gateway info */
		IID_INIT(iid);

		while(zcfgFeObjJsonGetNext(RDM_OID_ROUTING_ROUTER_V6_FWD, &iid, &v6GwObj) == ZCFG_SUCCESS)
		{
			ipIface_str = json_object_get_string (json_object_object_get(v6GwObj, "Interface"));

			if(strcmp(ipIface_str, ifacePath) == 0)
			{
				gateway6_str = json_object_get_string (json_object_object_get(v6GwObj, "NextHop"));
				Jadds(Jobj, "v6Gateway", gateway6_str);
			}
			json_object_put(v6GwObj);
		}

		/* DNS client server */
		dnsv4Array = json_object_new_array();		
		dnsv6Array = json_object_new_array();	//	need FIX
		json_object_object_add(Jobj, "dnsv4Server", dnsv4Array);
		json_object_object_add(Jobj, "dnsv6Server", dnsv6Array);
		if(dnsClientJarray != NULL){
			len2 = json_object_array_length(dnsClientJarray);
			for(j=0; j<len2; j++){
				dnsClientObj = json_object_array_get_idx(dnsClientJarray, j);
				if(Jgets(dnsClientObj, "Interface") != NULL){
					if(!strcmp(Jgets(dnsClientObj, "Interface"),ifacePath)){
	
						dns_tmp = Jgets(dnsClientObj, "DNSServer");
						if(strchr(dns_tmp, ':')){	//ipv6 dns server
	
							strcpy(ipv6dns, dns_tmp);
							Jadds(dnsClientObj, "v6dns", ipv6dns);
							json_object_array_add(dnsv6Array, JSON_OBJ_COPY(dnsClientObj));
						}
						else{						//ipv4 dns server
	
							strcpy(ipv4dns, dns_tmp);
							Jadds(dnsClientObj, "v4dns", ipv4dns);
							json_object_array_add(dnsv4Array, JSON_OBJ_COPY(dnsClientObj));
						}
					}
				}
			}
		}

		/* Routing RT Info Intf Set (RA server) */
		rtInfoArray = json_object_new_array();		
		json_object_object_add(Jobj, "rtinfoIntf", rtInfoArray);
		if(rtInfoIntfJarray != NULL){
			len2 = json_object_array_length(rtInfoIntfJarray);
			for(j=0; j<len2; j++)
			{
				rtInfoObj = json_object_array_get_idx(rtInfoIntfJarray, j);
				if(Jgets(rtInfoObj, "Interface") != NULL){
					if(!strcmp(Jgets(rtInfoObj, "Interface"),ifacePath)){
						if(isEmptyIns(rtInfoObj))
							continue;
						json_object_array_add(rtInfoArray, JSON_OBJ_COPY(rtInfoObj));
					}
				}
			}
		}

		int count = 0;
		const char *HigherLayer = "IP.Interface.";
		while(count < 4){
			count++;

			if(LowerLayers == NULL){
				json_object_put(Jobj);
				Jobj = NULL;
				break;
			}

			if(!strncmp(HigherLayer, "IP.Interface.", 13)){
				if(!strncmp(LowerLayers, "PPP.Interface.", 14)){
					sscanf(LowerLayers, "PPP.Interface.%hhu", &idx);
					pppIfaceObj = json_object_array_get_idx(pppIfaceJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(pppIfaceObj, "LowerLayers"));
					X_ZYXEL_LastConnectionUpTime = getPPPUptime(idx);
					replaceParam(Jobj, "pppConnectionStatus", pppIfaceObj, "ConnectionStatus");
					Jaddi(Jobj, "wanpppIdx", idx);
                    			Jaddi(Jobj, "pppoeConnectionUpTime", X_ZYXEL_LastConnectionUpTime);

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
					int pppcounter = 0;
					char pppDownReason[64] = {0};
					pppcounter = getPPPUpCounterandDownReason(idx,pppDownReason);
					
					Jaddi(Jobj, "PPPoEUpCounter", pppcounter);
					if(strlen(pppDownReason) > 0)
						Jadds(Jobj, "PPPoEUpDownReason", pppDownReason);
					else
						json_object_object_add(Jobj, "PPPoEUpDownReason", json_object_new_string("N/A"));
#endif					
				}
				else if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069						
						int ipcounter = 0; 
						char ipDownReason[64] = {0};
						ipcounter = getIPUpCounterandDownReason(idx+2,ipDownReason);

						Jaddi(Jobj, "IPoEUpCounter", ipcounter);
						if(strlen(ipDownReason) > 0)
							Jadds(Jobj, "IPoEUpDownReason", ipDownReason);
						else
							json_object_object_add(Jobj, "IPoEUpDownReason", json_object_new_string("N/A"));
#endif						
				}
				else if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					if(ethLinkObj){
						zcfgFeJsonObjFree(ethLinkObj);
					}
					IID_INIT(iid);
					iid.level = 1;
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &(iid.idx[0]));
					zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &iid, &ethLinkObj);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
					replaceParam(Jobj, "MACAddress", ethLinkObj, "MACAddress");
					
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069						
						int wwancounter = 0; 
						char wwanDownReason[64] = {0};
						wwancounter = getIPUpCounterandDownReason(iid.idx[0],wwanDownReason);

						Jaddi(Jobj, "IPoEUpCounter", wwancounter);
						if(strlen(wwanDownReason) > 0)
							Jadds(Jobj, "IPoEUpDownReason", wwanDownReason);
						else
							json_object_object_add(Jobj, "IPoEUpDownReason", json_object_new_string("N/A"));
#endif						
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9) && !strcmp("IP_Routed", X_ZYXEL_ConnectionType)){
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoA"));
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp(HigherLayer, "PPP.Interface.", 14)){
				if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoE"));
#ifdef ABUU_CUSTOMIZATION
					json_object_object_add(Jobj, "Username", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Username")));
					json_object_object_add(Jobj, "Password", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Password")));
					json_object_object_add(Jobj, "ConnectionTrigger", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "ConnectionTrigger")));
					json_object_object_add(Jobj, "IdleDisconnectTime", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "IdleDisconnectTime")));
#endif
					replaceParam(Jobj, "LowerlayerUp", vlanTermObj, "Status");
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9)){
					sscanf(LowerLayers, "ATM.Link.%hhu", &idx);
					atmLinkObj = json_object_array_get_idx(atmLinkJarray, idx-1);
					//json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoA"));
					json_object_object_add(Jobj, "Encapsulation", JSON_OBJ_COPY(json_object_object_get(atmLinkObj, "LinkType")));
#ifdef ZYXEL_PPTPD_RELAY
					if (!strcmp(TR181_PPtP2PPPOA,json_object_get_string(json_object_object_get(atmLinkObj, "LinkType")))){
						X_ZYXEL_LastConnectionUpTime = getPPTPUptime(idx);
						Jaddi(Jobj, "pppoeConnectionUpTime", X_ZYXEL_LastConnectionUpTime);
					}	
#endif
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					replaceParam(Jobj, "LowerlayerUp", atmLinkObj, "Status");
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoE"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25)){
				if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					if(ethLinkObj){
						zcfgFeJsonObjFree(ethLinkObj);
					}
					IID_INIT(iid);
					iid.level = 1;
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &(iid.idx[0]));
					zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &iid, &ethLinkObj);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
					replaceParam(Jobj, "MACAddress", ethLinkObj, "MACAddress");
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.Link.", HigherLayer, 14)){
				if(!strncmp("ATM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					break;
				}
				else if(!strncmp("PTM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PTM"));
					break;
				}
				else if(!strncmp("Ethernet.Interface.", LowerLayers, 19)){
					IID_INIT(iid);
					iid.level = 1;
					sscanf(LowerLayers, "Ethernet.Interface.%hhu",  &(iid.idx[0]));
					zcfgFeObjJsonGet(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj);
					if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP")) == false)
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					else
					json_object_object_add(Jobj, "LinkType", json_object_new_string("SFP"));
					if(ethIfaceObj){
						zcfgFeJsonObjFree(ethIfaceObj);
					}
					break;
				}
				else if(!strncmp("Optical.Interface.", LowerLayers, 18)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PON"));
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));
					break;
				}
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
				else if(!strncmp("Cellular.Interface.", LowerLayers, 19)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("CELL"));
					json_object_object_add(Jobj, "cellIntfPath", json_object_new_string(LowerLayers));
					break;
				}
#endif
				else if(!strncmp("Bridging.Bridge.", LowerLayers, 16)){
					sscanf(LowerLayers, "Bridging.Bridge.%hhu.Port.1", &idx);
					bridgeBrObj = json_object_array_get_idx(bridgeBrJarray, idx-1);
					X_ZYXEL_BridgeName = json_object_get_string(json_object_object_get(bridgeBrObj, "X_ZYXEL_BridgeName"));
					if(X_ZYXEL_BridgeName == NULL){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					if(X_ZYXEL_Group_WAN_IpIface == NULL)
						X_ZYXEL_Group_WAN_IpIface = "";
						
					sprintf(BridgingBrPath, "Bridging.Bridge.%u", idx);
					json_object_object_add(Jobj, "X_ZYXEL_SrvName", json_object_new_string(X_ZYXEL_BridgeName));
					Jadds(Jobj, "X_ZYXEL_Type", "LAN");
					json_object_object_add(Jobj, "BridgingBrPath", json_object_new_string(BridgingBrPath));
					json_object_object_add(Jobj, "Group_WAN_IpIface", json_object_new_string(X_ZYXEL_Group_WAN_IpIface));
					strcat(intfGrpWANList, X_ZYXEL_Group_WAN_IpIface);
					strcat(intfGrpWANList, ",");
					break;
				}
				else if(!strcmp("", LowerLayers)){
					if(ethLinkObj){
						zcfgFeJsonObjFree(ethLinkObj);
					}
					IID_INIT(iid);
					iid.level = 1;
					sscanf(HigherLayer, "Ethernet.Link.%hhu", &(iid.idx[0]));
					zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &iid, &ethLinkObj);
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
		}

		if(Jobj != NULL){

			X_ZYXEL_Type = Jgets(Jobj, "X_ZYXEL_Type");
			getDHCPStatus(DHCPStatus, &DHCPLeaseTime, DHCPtr181Path, DHCPoption42, DHCPoption42Value, X_ZYXEL_Type, ifacePath, dhcp4PoolJarray, dhcp4ClientJarray);
 			Jadds(Jobj, "DHCPStatus", DHCPStatus);
			Jadds(Jobj, "DHCPtr181Path", DHCPtr181Path);
			Jaddi(Jobj, "DHCPLeaseTime", DHCPLeaseTime);
			if(!strcmp(DHCPoption42,"true")){
				Jaddb(Jobj, "DHCPoption42Enable", true);
			}
			else{
				Jaddb(Jobj, "DHCPoption42Enable", false);
			}
			Jadds(Jobj, "DHCPoption42Value", DHCPoption42Value);
			json_object_array_add(*Jarray, Jobj);
		}
	}

	if(strcmp(intfGrpWANList, "")){
		intf = strtok_r(intfGrpWANList, ",", &tmp_ptr);
		while(intf != NULL){
			if(*intf != '\0'){
				len = json_object_array_length(*Jarray);
				for(i=0;i<len;i++){
					Jobj = json_object_array_get_idx(*Jarray, i);
					IpIfacePath = json_object_get_string(json_object_object_get(Jobj, "IpIfacePath"));
					if(!strcmp(IpIfacePath, intf)){
						json_object_object_add(Jobj, "BindToIntfGrp", json_object_new_boolean(true));
					}
				}
			}
			intf = strtok_r(NULL, ",", &tmp_ptr);
		}
	}

	json_object_put(ipIfaceJarray);
	json_object_put(pppIfaceJarray);
	json_object_put(vlanTermJarray);
	json_object_put(atmLinkJarray);
	json_object_put(bridgeBrJarray);
	json_object_put(dhcp4PoolJarray);
	json_object_put(dhcp4ClientJarray);
	json_object_put(ethLinkObj);
	json_object_put(dnsClientJarray);
	json_object_put(rtInfoIntfJarray);
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *statusJobj = NULL;
	struct json_object *firewallInfoObj = NULL, *firewallLevelObj = NULL, *wwanObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	struct json_object *gponObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *ssidJarray = NULL;
#endif
	struct json_object *wanLanJarray = NULL;
	struct json_object *ethIfaceJarray = NULL, *ethIfaceObj = NULL;
	struct json_object *dslChannelJarray = NULL;
	struct json_object *optIntfJarray = NULL;
	struct json_object *wwanBackupJarray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	struct json_object *wwanBackupObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	struct json_object *optIbtfObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	struct json_object *dslChannelObj = NULL;
#endif
#ifdef ABUU_CUSTOMIZATION
	struct json_object *profileObj = NULL;
#endif
#if defined(ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO) || defined(SHOW_USB_PARTITION_INFOR)
	struct json_object *usbHostDevParJarray = NULL, *usbHostDevParObj = NULL;
#ifdef SHOW_USB_PARTITION_INFOR
	struct json_object *usbDevobj = NULL;
#endif
#endif
	struct json_object *voipProfJarray = NULL, *voipProfObj = NULL;
	struct json_object *voipProSIPfJarray = NULL, *voipProSIPfObj = NULL;
	struct json_object *voipLineJarray = NULL, *voipLineObj = NULL;
#ifdef ZYXEL_WEB_GUI_SYS_INFO_PAGE_SHOW_VOIP_INFO
	struct json_object *paramJobj = NULL;
	struct json_object *voipPhoneJarray = NULL, *voipPhoneObj = NULL;
	objIndex_t sipIid;
	const char *LineSelect = NULL;
#endif
	struct json_object *gponstatusObj = NULL;
	struct json_object *gponinfoObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	struct json_object *cellIntfObj = NULL;
	struct json_object *cellIntfZyIpPassthruObj = NULL;
	struct json_object *cellAccessPointJarray = NULL, *cellAccessPointObj = NULL;
#endif
	objIndex_t iid;
	struct json_object *devInfoObj = NULL, *sysInfoObj = NULL, *hostObj = NULL, *memoryObj = NULL, *cpuObj = NULL; 
	zcfgRet_t ret = ZCFG_SUCCESS;
    char WanType[16] = {0};
    char WanRate_RX[16] = {0};
    char WanRate_TX[16] = {0};
//#if (defined(CONFIG_BCM96858) || defined(CONFIG_BCM96858)) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)
#if (defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)) || (defined(CONFIG_BCM96856) && defined(XPON_SUPPORT))
    char psp_wantype[16] = {0};
    char psp_wanrate[16] = {0};
    const char *optStatus = NULL;
#endif
    int X_ZYXEL_InterfaceUpTime = 0;
#ifdef GFIBER_CUSTOMIZATION
	struct json_object *tmpSSIDobj = NULL;
	int len = 0, i = 0;
#endif

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &iid, &devInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SYSTEM_INFO, &iid, &sysInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	if(zcfgFeObjJsonGet(RDM_OID_DEV_INFO_MEM_STAT, &iid, &memoryObj) == ZCFG_SUCCESS){	
		json_object_object_add(sysInfoObj, "Total", JSON_OBJ_COPY(json_object_object_get(memoryObj, "Total")));
		json_object_object_add(sysInfoObj, "Free", JSON_OBJ_COPY(json_object_object_get(memoryObj, "Free")));
		zcfgFeJsonObjFree(memoryObj);
	}

	if(zcfgFeObjJsonGet(RDM_OID_DEV_INFO_PS_STAT, &iid, &cpuObj) == ZCFG_SUCCESS){	
		json_object_object_add(sysInfoObj, "CPUUsage", JSON_OBJ_COPY(json_object_object_get(cpuObj, "CPUUsage")));
		zcfgFeJsonObjFree(cpuObj);
	}

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL, &iid, &firewallInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_HOSTS, &iid, &hostObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
	zcfgFeJsonObjFree(hostObj);

	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL_LEVEL, &iid, &firewallLevelObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
	replaceParam(firewallInfoObj, "X_ZYXEL_DefaultConfig", firewallLevelObj, "X_ZYXEL_DefaultConfig");
	zcfgFeJsonObjFree(firewallLevelObj);


	ethIfaceJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceJarray, ethIfaceObj);
		if( true == json_object_get_boolean(json_object_object_get(ethIfaceObj,"Upstream")) && !strcmp(json_object_get_string(json_object_object_get(ethIfaceObj,"Status")),"Up")) {
			X_ZYXEL_InterfaceUpTime = json_object_get_int(json_object_object_get(ethIfaceObj,"X_ZYXEL_UpTime"));
		}
	}

	dslChannelJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &iid, &dslChannelObj) == ZCFG_SUCCESS) {
		json_object_array_add(dslChannelJarray, dslChannelObj);
	}
#endif

	optIntfJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	IID_INIT(iid);
	while (zcfgFeObjJsonGetNext(RDM_OID_OPT_INTF, &iid, &optIbtfObj) == ZCFG_SUCCESS) {
        json_object_array_add(optIntfJarray, optIbtfObj);
#if defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)
        optStatus = json_object_get_string (json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if (zyUtilIGUIGetRDPAWAN(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "XGS", 3) == 0) {
                    strncpy(WanType, "XGSPON", 6);
                }
                else if (strncmp(psp_wantype, "XGPON1", 6) == 0) {
                    strncpy(WanType, "XGPON", 5);
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "2.5Gbps", 7);
                }

                if (strncmp(psp_wanrate, "1010", 4) == 0) {
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "10Gbps", 6);
                }
            }
        }

        if (strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if (strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(CONFIG_BCM96856) && defined(XPON_SUPPORT)
        optStatus=json_object_get_string (json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if(zyUtilIGUIGetRDPAWAN(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "GPON", 4) == 0) {
                    strncpy(WanType, "GPON", 4);
                }

                if (strncmp(psp_wanrate, "0101", 4) == 0) {
                    strncpy(WanRate_RX, "2.5Gbps", 7);
                    strncpy(WanRate_TX, "1.25Gbps", 8);
                }
            }
        }

        if (strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if(strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
        strcpy(WanType, "GPON");
#else 
        strcpy(WanType, "");
        strcpy(WanRate_RX, "-1");
        strcpy(WanRate_TX, "-1");
#endif
	}
#endif //CONFIG_ZCFG_BE_MODULE_OPTICAL

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
	wwanBackupJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &iid, &wwanBackupObj) == ZCFG_SUCCESS) {
		json_object_array_add(wwanBackupJarray, wwanBackupObj);
	}
#endif
#endif

#ifdef ABUU_CUSTOMIZATION
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_MULTIPLE_CONFIG, &iid, &profileObj) != ZCFG_SUCCESS) {
		goto zcfgFeDalStatusGetFail;
	}
#endif

#if defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
	int onutype = 0;
	if(zyUtilIGetOnuType(&onutype) == ZCFG_SUCCESS){
		onutype = onutype >> 4;
		switch(onutype) { 
			case 0:
				strcpy(WanType, "AUTO");
				break; 
			case 1:
				strcpy(WanType, "GPON");//1g/2.5g
				break; 
			case 2: 
				strcpy(WanType, "EPON");// 1G_1G ,not sure,no epon device currently
				break; 
			case 3: 
				strcpy(WanType, "XEPON_1G");// 10G_1G ,not sure,no epon device currently
				break; 
			case 4: 
				strcpy(WanType, "XEPON_ASY");//10G_10G ,not sure,no epon device currently
				break;
			case 5: 
				strcpy(WanType, "XEPON_SYM"); //1G_1G ,not sure,no epon device currently
				break; 
			case 6: 
				strcpy(WanType, "XGPON"); //10g/10g
				break; 
			case 7: 
				strcpy(WanType, "XGSPON"); //10g/10g
				strcpy(WanRate_RX, "10Gbps");
                strcpy(WanRate_TX, "10Gbps");
				break; 			
			default: 
				strcpy(WanType, "UNKNOWN"); 
				break;
		} 
		//printf("%d %s\n", __LINE__, __func__);
		//printf("WanType:%s\n",WanType);
	}
#endif

#if defined(ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO) || defined(SHOW_USB_PARTITION_INFOR)
#ifdef CONFIG_ZCFG_BE_MODULE_USB
#ifdef SHOW_USB_PARTITION_INFOR
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_USB_USBHOSTS_HOST_DEV, &iid, &usbDevobj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(usbDevobj);
	}
#endif
	usbHostDevParJarray = json_object_new_array();
	objIndex_t usbIid = {0};
	IID_INIT(usbIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &usbIid, &usbHostDevParObj) == ZCFG_SUCCESS){	
		json_object_array_add(usbHostDevParJarray, usbHostDevParObj);
	}
#else
	usbHostDevParJarray = json_object_new_array();
	printf("%s : USB not support\n", __FUNCTION__);
#endif // CONFIG_ZCFG_BE_MODULE_USB
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GPON_INFO, &iid, &gponstatusObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ONU, &iid, &gponObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_STAT, &iid, &wwanObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF, &iid, &cellIntfObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &iid, &cellIntfZyIpPassthruObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	cellAccessPointJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_CELL_ACCESS_POINT, &iid, &cellAccessPointObj) == ZCFG_SUCCESS) {
		json_object_array_add(cellAccessPointJarray, cellAccessPointObj);
	}
#endif

#if BUILD_PACKAGE_ZyIMS
	voipProfJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PROF, &iid, &voipProfObj) == ZCFG_SUCCESS) {
		Jaddi(voipProfObj, "idx", iid.idx[1]);
		json_object_array_add(voipProfJarray, voipProfObj);
	}

	voipProSIPfJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PROF_SIP, &iid, &voipProSIPfObj) == ZCFG_SUCCESS) {
		json_object_array_add(voipProSIPfJarray, voipProSIPfObj);
	}

	voipLineJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &iid, &voipLineObj) == ZCFG_SUCCESS) {
		Jaddi(voipLineObj, "idx0", iid.idx[0]);
		Jaddi(voipLineObj, "idx1", iid.idx[1]);
		Jaddi(voipLineObj, "idx2", iid.idx[2]);
		json_object_array_add(voipLineJarray, voipLineObj);
	}
#ifdef ZYXEL_WEB_GUI_SYS_INFO_PAGE_SHOW_VOIP_INFO
	voipPhoneJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PHY_INTF, &iid, &voipPhoneObj) == ZCFG_SUCCESS)
	{
		LineSelect = json_object_get_string(json_object_object_get(voipPhoneObj,"X_ZYXEL_LineSelect"));
		IID_INIT(sipIid);
		sipIid.level = 3;
		sscanf(LineSelect,"Device.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu", &sipIid.idx[0], &sipIid.idx[1], &sipIid.idx[2]);
		if(zcfgFeObjJsonGet(RDM_OID_VOICE_LINE, &sipIid, &voipLineObj) == ZCFG_SUCCESS)
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(voipLineObj, "Status")));
			json_object_object_add(paramJobj, "DirectoryNumber", JSON_OBJ_COPY(json_object_object_get(voipLineObj, "DirectoryNumber")));
			zcfgFeJsonObjFree(voipLineObj);
		}
		else
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Status", json_object_new_string(""));
			json_object_object_add(paramJobj, "DirectoryNumber", json_object_new_string(""));
		}
		json_object_array_add(voipPhoneJarray, paramJobj);
		zcfgFeJsonObjFree(voipPhoneObj);
	}
#endif
#endif //BUILD_PACKAGE_ZyIMS

    gponinfoObj = json_object_new_object();
#if defined(BROADCOM_PLATFORM) && (defined(XPON_SUPPORT)|| defined(ZYXEL_SFP_MODULE_SUPPORT))
    if (zyUtil_is_sfp_present() != 0)
    {
        json_object_object_add(gponstatusObj, "sfp_presence",json_object_new_boolean(true));
    }
    else
    {
        json_object_object_add(gponstatusObj, "sfp_presence",json_object_new_boolean(false));
    }
    if (zyUtil_bosa_exist())
    {
        json_object_object_add(gponstatusObj, "bosa_presence",json_object_new_boolean(true));
    }
    else
    {
        json_object_object_add(gponstatusObj, "bosa_presence",json_object_new_boolean(false));
    }
    json_object_object_add(gponstatusObj, "Platform", json_object_new_string("BROADCOM"));//broadcom platform
    json_object_object_add(gponinfoObj, "gponStatusObj",gponstatusObj);
#endif
#if defined(ECONET_PLATFORM) && (defined(XPON_SUPPORT)|| defined(ZYXEL_SFP_MODULE_SUPPORT))
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
   json_object_object_add(gponObj, "Platform", json_object_new_string("ECONET"));// econet platform
   json_object_object_add(gponinfoObj, "gponStatusObj", JSON_OBJ_COPY(gponObj));
#endif
#endif

#if defined(MTK_PLATFORM) && (defined(XPON_SUPPORT)|| defined(ZYXEL_SFP_MODULE_SUPPORT))
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
    json_object_object_add(gponstatusObj, "Platform", json_object_new_string("MTK"));//MTK platform
    json_object_object_add(gponinfoObj, "gponStatusObj",gponstatusObj);
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	getWifiInfo(&ssidJarray);
#ifdef GFIBER_CUSTOMIZATION
	len = json_object_array_length (ssidJarray);
	for (i = 0; i < len; i++) {
		tmpSSIDobj = json_object_array_get_idx(ssidJarray, i);
		json_object_object_add(tmpSSIDobj, "wifiPassword", json_object_new_string(""));
	}
#endif
#endif
	getWanLanList_ext(&wanLanJarray);
	
	statusJobj = json_object_new_object();
	json_object_object_add(statusJobj, "DeviceInfo", devInfoObj);
	json_object_object_add(statusJobj, "SystemInfo", sysInfoObj);
	json_object_object_add(statusJobj, "FirewallInfo", firewallInfoObj);
	json_object_object_add(statusJobj, "LanPortInfo", ethIfaceJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	json_object_object_add(statusJobj, "WiFiInfo", ssidJarray);
#endif
	json_object_object_add(statusJobj, "WanLanInfo", wanLanJarray);
	json_object_object_add(statusJobj, "DslChannelInfo", dslChannelJarray);
	json_object_object_add(statusJobj, "OptIntfInfo", optIntfJarray);
#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
	json_object_object_add(statusJobj, "WWANBackupInfo", wwanBackupJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_object_add(statusJobj, "WWANStatsInfo", wwanObj);
#endif
#endif

#ifdef ABUU_CUSTOMIZATION
	if (profileObj == NULL)
		json_object_object_add(statusJobj, "profile", json_object_new_string("MU"));
	else
		json_object_object_add(statusJobj, "profile", JSON_OBJ_COPY(json_object_object_get(profileObj, "Name")));
#endif

#if defined(ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO) || defined(SHOW_USB_PARTITION_INFOR)
	json_object_object_add(statusJobj, "USBStatsInfo", usbHostDevParJarray);
#endif	

#ifdef PACKAGE_WEB_GUI_VUE
	json_object_object_add(statusJobj, "GponStatsInfo", gponinfoObj);
#else
	json_object_object_add(statusJobj, "GponStatsInfo", gponstatusObj);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	json_object_object_add(statusJobj, "GponInfo", JSON_OBJ_COPY(gponObj));
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	json_object_object_add(statusJobj, "CellIntfInfo", cellIntfObj);
	json_object_object_add(statusJobj, "CellIntfZyIpPassthruInfo", cellIntfZyIpPassthruObj);
	json_object_object_add(statusJobj, "CellAccessPointInfo", cellAccessPointJarray);
#endif
#if BUILD_PACKAGE_ZyIMS
	json_object_object_add(statusJobj, "VoipProfInfo", voipProfJarray);
	json_object_object_add(statusJobj, "VoipProfSIPInfo", voipProSIPfJarray);
	json_object_object_add(statusJobj, "VoipLineInfo", voipLineJarray);
#ifdef ZYXEL_WEB_GUI_SYS_INFO_PAGE_SHOW_VOIP_INFO
	json_object_object_add(statusJobj, "PhoneInfo", voipPhoneJarray);
#endif
#endif
    Jadds (statusJobj, "WanType", WanType);
    Jadds (statusJobj, "WanRate_RX", WanRate_RX);
    Jadds (statusJobj, "WanRate_TX", WanRate_TX);
    Jaddi (statusJobj, "X_ZYXEL_InterfaceUpTime", X_ZYXEL_InterfaceUpTime);
	
	json_object_array_add(Jarray, statusJobj);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	json_object_put(gponObj);
#endif
	return ZCFG_SUCCESS;

zcfgFeDalStatusGetFail:
	json_object_put(devInfoObj);
	json_object_put(sysInfoObj);
	json_object_put(firewallInfoObj);
	json_object_put(firewallLevelObj);
	json_object_put(hostObj);
	json_object_put(gponstatusObj);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	json_object_put(gponObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	json_object_put(cellIntfObj);
	json_object_put(cellIntfZyIpPassthruObj);
#endif
#ifdef ABUU_CUSTOMIZATION
	json_object_put(profileObj);
#endif
	return ret;
}

/*
 *  Function Name: zcfgFeDalCardPageStatusGet
 *  Description: Only get useful parameter or object to GUI.
 *
 */
zcfgRet_t zcfgFeDalCardPageStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *statusJobj = NULL;
	struct json_object *devInfoObj = NULL, *sysInfoObj = NULL;
	struct json_object *wanLanJarray = NULL;
	struct json_object *ethnewObj = NULL, *ethIfaceObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *ssidJarray = NULL, *MainSSIDobj = NULL, *GuestSSIDObj = NULL, *newssidJarray = NULL;
#endif
	struct json_object *dhcpv4SerObj = NULL;
	struct json_object *partctlObj = NULL;
	struct json_object *ethIfaceJarray = NULL, *ethwanIfaceObj = NULL;
	struct json_object *dslChannelJarray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_DSL 
	struct json_object *dslChannelObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	struct json_object *gponstatusObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	struct json_object *wwanBackupObj = NULL, *wwanObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	struct json_object *cellIntfObj = NULL;
	struct json_object *cellIntfSimObj = NULL;
	struct json_object *cellIntfZyIpPassthruObj = NULL;
#endif
	struct json_object *optIntfJarray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL 
	struct json_object *optIbtfObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	struct json_object *onessidObj = NULL;
	bool onessid = false;
#endif
	objIndex_t iid;
	int count = 0, currentBitRate = 0, isSFP =0;
	const char *lanport = NULL, *lanMacAddr = NULL, *duplexMode = NULL;
	char ethConnectionStatus[16] = {0};  
	char WanType[16] = {0};
#if (defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)) || (defined(CONFIG_BCM96856) && defined(XPON_SUPPORT))
	char psp_wantype[16] = {0};
	char psp_wanrate[16] = {0};
    const char *optStatus = NULL;
#endif
	char WanRate_RX[16] = {0};
	char WanRate_TX[16] = {0};
	bool ethPortExist = false;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	int len = 0, i = 0;
	bool MainSSID = false, Guest24GSSIDEnable = false, Main24GSSIDEnable = false, Guest5GSSIDEnable = false, Main5GSSIDEnable = false, GuestSSIDEnable = false, support24G = false, support5G = false;
	const char *guestOperatingFrequencyBand = NULL, *mainOperatingFrequencyBand = NULL;
#endif

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &iid, &devInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SYSTEM_INFO, &iid, &sysInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &iid, &dhcpv4SerObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &iid, &wwanBackupObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_STAT, &iid, &wwanObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	Jadds(devInfoObj, "PackageVersion", json_object_get_string (json_object_object_get(wwanBackupObj, "PackageVersion")));
	zcfgFeJsonObjFree(wwanBackupObj);
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GPON_INFO, &iid, &gponstatusObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF, &iid, &cellIntfObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_USIM, &iid, &cellIntfSimObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &iid, &cellIntfZyIpPassthruObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
#endif
	ethIfaceJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethwanIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceJarray, ethwanIfaceObj);
	}

	dslChannelJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &iid, &dslChannelObj) == ZCFG_SUCCESS) {
		json_object_array_add(dslChannelJarray, dslChannelObj);
	}
#endif

	optIntfJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	IID_INIT(iid);
	while (zcfgFeObjJsonGetNext(RDM_OID_OPT_INTF, &iid, &optIbtfObj) == ZCFG_SUCCESS) {
        json_object_array_add(optIntfJarray, optIbtfObj);
#if defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)
        optStatus = json_object_get_string(json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if (zyUtilIGUIGetRDPAWAN(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "XGS", 3) == 0) {
                    strncpy(WanType, "XGSPON", 6);
                }
                else if (strncmp(psp_wantype, "XGPON1", 6) == 0) {
                    strncpy(WanType, "XGPON", 5);
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "2.5Gbps", 7);
                }				

                if (strncmp(psp_wanrate, "1010", 4) == 0) {
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "10Gbps", 6);
                }
            }
        }

        if(strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if(strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(CONFIG_BCM96856) && defined(XPON_SUPPORT)
        optStatus = json_object_get_string (json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if(zyUtilIGUIGetRDPAWAN(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "GPON", 4) == 0) {
                    strncpy(WanType, "GPON",4 );
                }

                if (strncmp(psp_wanrate, "0101", 4) == 0) {
                    strncpy(WanRate_RX, "2.5Gbps", 7);
                    strncpy(WanRate_TX, "1.25Gbps", 8);
                }
            }
        }

        if(strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if(strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
        strcpy(WanType, "GPON");
#else 
        strcpy(WanType, "");
        strcpy(WanRate_RX, "-1");
        strcpy(WanRate_TX, "-1");
#endif
	}
#endif // CONFIG_ZCFG_BE_MODULE_OPTICAL

#if defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
	int onutype = 0;
	if(zyUtilIGetOnuType(&onutype) == ZCFG_SUCCESS){
		onutype = onutype >> 4;
		switch(onutype) { 
			case 0:
				strcpy(WanType, "AUTO");
				break; 
			case 1:
				strcpy(WanType, "GPON");//1g/2.5g
				break; 
			case 2: 
				strcpy(WanType, "EPON");// 1G_1G ,not sure,no epon device currently
				break; 
			case 3: 
				strcpy(WanType, "XEPON_1G");// 10G_1G ,not sure,no epon device currently
				break; 
			case 4: 
				strcpy(WanType, "XEPON_ASY");//10G_10G ,not sure,no epon device currently
				break;
			case 5: 
				strcpy(WanType, "XEPON_SYM"); //1G_1G ,not sure,no epon device currently
				break; 
			case 6: 
				strcpy(WanType, "XGPON"); //10g/10g
				break; 
			case 7: 
				strcpy(WanType, "XGSPON"); //10g/10g
				strcpy(WanRate_RX, "10Gbps");
                strcpy(WanRate_TX, "10Gbps");
				break; 			
			default: 
				strcpy(WanType, "UNKNOWN"); 
				break;
		} 
		//printf("%d %s\n", __LINE__, __func__);
		//printf("WanType:%s\n",WanType);
	}
#endif

	ethnewObj = json_object_new_object();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		lanport = json_object_get_string (json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		isSFP = json_object_get_boolean (json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP"));
#if (defined(ZCFG_VMG4927_B50A_Generic) || defined (ZCFG_EMG6726_B10A_Generic) )
		if(!strncmp (lanport, "LAN1", 4)) {
#else
		if(!strncmp (lanport, "LAN", 3)) {
#endif
			lanMacAddr = json_object_get_string (json_object_object_get(ethIfaceObj, "MACAddress"));
			Jadds (ethnewObj, "LanMac", lanMacAddr);
		}
		else if (!strcmp (lanport, "ETHWAN")  && !isSFP) {
			ethPortExist = true;
			ethConnectionStatus[0] = '\0';
			currentBitRate = json_object_get_int (json_object_object_get(ethIfaceObj, "CurrentBitRate"));
			duplexMode = json_object_get_string (json_object_object_get(ethIfaceObj, "DuplexMode"));
			if (currentBitRate > 0) {
				snprintf (ethConnectionStatus, sizeof(ethConnectionStatus), "%dM/%s", currentBitRate, duplexMode);
			}
			else {
				strcat (ethConnectionStatus, "Connection down");
			}
			Jadds (ethnewObj, "EthConnectionStatus", ethConnectionStatus);
		}
		else if (!strcmp (lanport, "ETHWAN")  && isSFP) {
			ethPortExist = true;
			ethConnectionStatus[0] = '\0';
			currentBitRate = json_object_get_int (json_object_object_get(ethIfaceObj, "CurrentBitRate"));
			duplexMode = json_object_get_string (json_object_object_get(ethIfaceObj, "DuplexMode"));
			if (currentBitRate > 0) {
				snprintf (ethConnectionStatus, sizeof(ethConnectionStatus), "%dM/%s", currentBitRate, duplexMode);
			}
			else {
				strcat (ethConnectionStatus, "Connection down");
			}
			Jadds (ethnewObj, "SFPConnectionStatus", ethConnectionStatus);
		}
		Jaddb (ethnewObj, "ethPortExist", ethPortExist);
		zcfgFeJsonObjFree (ethIfaceObj);
	}
#ifndef ABQA_CUSTOMIZATION
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PAREN_CTL_PROF, &iid, &partctlObj) == ZCFG_SUCCESS){
		count++;
		zcfgFeJsonObjFree(partctlObj);		
	}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	getWifiInfo(&ssidJarray);
	//Just GET Main SSID object to GUI
	newssidJarray = json_object_new_array();
	len = json_object_array_length (ssidJarray);
	for (i = 0; i < len; i++) {
		MainSSIDobj = json_object_array_get_idx(ssidJarray, i);
		MainSSID = json_object_get_boolean (json_object_object_get(MainSSIDobj, "X_ZYXEL_MainSSID"));
		if (MainSSID) {
			json_object_array_add(newssidJarray, JSON_OBJ_COPY(MainSSIDobj));
			GuestSSIDObj = json_object_array_get_idx(ssidJarray, i + 1);
			json_object_array_add(newssidJarray, JSON_OBJ_COPY(GuestSSIDObj));
			mainOperatingFrequencyBand = json_object_get_string (json_object_object_get(MainSSIDobj, "OperatingFrequencyBand"));
			guestOperatingFrequencyBand = json_object_get_string (json_object_object_get(GuestSSIDObj, "OperatingFrequencyBand"));
			if (!strcmp (mainOperatingFrequencyBand, "2.4GHz")) {
				Main24GSSIDEnable = json_object_get_boolean (json_object_object_get(MainSSIDobj, "Enable"));
			}
			else if (!strcmp (mainOperatingFrequencyBand, "5GHz")) {
				Main5GSSIDEnable = json_object_get_boolean (json_object_object_get(MainSSIDobj, "Enable"));
			}
			if (!strcmp (guestOperatingFrequencyBand, "2.4GHz")) {
				support24G = true;
				Guest24GSSIDEnable = json_object_get_boolean (json_object_object_get(GuestSSIDObj, "Enable"));
			}
			else if (!strcmp (guestOperatingFrequencyBand, "5GHz")) {
				support5G = true;
				Guest5GSSIDEnable = json_object_get_boolean (json_object_object_get(GuestSSIDObj, "Enable"));
			}
		}
	}
	zcfgFeJsonObjFree(ssidJarray);

#ifdef ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
		if (Guest24GSSIDEnable && Guest5GSSIDEnable){
			GuestSSIDEnable = true;
		}else{
			GuestSSIDEnable = false;
		}
#else
	//Consider if some project only support 2.4G or 5G only in the future.
	if (support24G && support5G) {	
		if (Main24GSSIDEnable && Main5GSSIDEnable && Guest24GSSIDEnable && Guest5GSSIDEnable) {
			GuestSSIDEnable = true;
		} 
	}
	else if (!support24G && support5G) {
		if (Main5GSSIDEnable && Guest5GSSIDEnable) {
			GuestSSIDEnable = true;
		}
	}
	else if (support24G && !support5G) {
		if (Main24GSSIDEnable && Guest24GSSIDEnable) {
			GuestSSIDEnable = true;
		}
	}
#endif //ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	getWanLanList_ext(&wanLanJarray);
	statusJobj = json_object_new_object();
	json_object_array_add(Jarray, statusJobj);
	json_object_object_add(statusJobj, "WanLanInfo", wanLanJarray);
	json_object_object_add(statusJobj, "DeviceInfo", devInfoObj);
	json_object_object_add(statusJobj, "SystemInfo", sysInfoObj);
	json_object_object_add(statusJobj, "LanPortInfo", ethnewObj);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	json_object_object_add(statusJobj, "WiFiInfo", newssidJarray);
#endif
	json_object_object_add(statusJobj, "Dhcp4SerPoolInfo", dhcpv4SerObj);
#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_object_add(statusJobj, "WWANStatsInfo", wwanObj);
#endif
#endif
	json_object_object_add(statusJobj, "LanInfo", ethIfaceJarray);
	json_object_object_add(statusJobj, "DslChannelInfo", dslChannelJarray);
	json_object_object_add(statusJobj, "OptIntfInfo", optIntfJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_object_add(statusJobj, "GponStatsInfo", gponstatusObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	json_object_object_add(statusJobj, "CellIntfInfo", cellIntfObj);
	json_object_object_add(statusJobj, "CellIntfSimfInfo", cellIntfSimObj);
	json_object_object_add(statusJobj, "CellIntfZyIpPassthruInfo", cellIntfZyIpPassthruObj);
#endif

#ifndef ABQA_CUSTOMIZATION
	Jaddi(statusJobj,"partctlNum",count);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	Jaddb(statusJobj,"GuestSSIDEnable",GuestSSIDEnable);
#endif
	Jadds (statusJobj, "WanType", WanType);
	Jadds (statusJobj, "WanRate_RX", WanRate_RX);
	Jadds (statusJobj, "WanRate_TX", WanRate_TX);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_ONE_SSID, &iid, &onessidObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

	onessid = json_object_get_boolean(json_object_object_get(onessidObj, "Enable"));
	if (onessid) {
		json_object_object_add(statusJobj, "oneSsidEnable", json_object_new_boolean(true));
	}
	else {
		json_object_object_add(statusJobj, "oneSsidEnable", json_object_new_boolean(false));
	}
	zcfgFeJsonObjFree(onessidObj);
#endif

	return ZCFG_SUCCESS;

zcfgFeDalCardPageStatusGetFail:
	json_object_put(devInfoObj);
	json_object_put(sysInfoObj);
	json_object_put(dhcpv4SerObj);
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_put(wwanBackupObj);
	json_object_put(wwanObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_put(gponstatusObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	json_object_put(cellIntfObj);
	json_object_put(cellIntfSimObj);
	json_object_put(cellIntfZyIpPassthruObj);
#endif
	return ret;
}

#ifdef GFIBER_CUSTOMIZATION
zcfgRet_t zcfgFeDalGfiberStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *statusJobj = NULL;
	struct json_object *firewallInfoObj = NULL, *firewallLevelObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *ssidJarray = NULL, *tmpSSIDobj = NULL;
#endif
	struct json_object *wanLanJarray = NULL;
	struct json_object *ethIfaceJarray = NULL, *ethIfaceObj = NULL;
	struct json_object *dslChannelJarray = NULL;
	struct json_object *optIntfJarray = NULL;
	struct json_object *wwanBackupJarray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	struct json_object *dslChannelObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	struct json_object *optIbtfObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	struct json_object *wwanBackupObj = NULL;
	struct json_object *wwanObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	struct json_object *gponObj = NULL;
#endif
#ifdef ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO
	struct json_object *usbHostDevParJarray = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	struct json_object *usbHostDevParObj = NULL;
#endif
	struct json_object *voipProfJarray = NULL, *voipProfObj = NULL;
	struct json_object *voipProSIPfJarray = NULL, *voipProSIPfObj = NULL;
	struct json_object *voipLineJarray = NULL, *voipLineObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	struct json_object *gponstatusObj = NULL;
#endif
	struct json_object *gponinfoObj = NULL;
	objIndex_t iid;	
	struct json_object *devInfoObj = NULL, *sysInfoObj = NULL, *hostObj = NULL, *memoryObj = NULL, *cpuObj = NULL; 
	zcfgRet_t ret = ZCFG_SUCCESS;
    char WanType[16] = {0};
    char psp_wantype[16] = {0};
    char psp_wanrate[16] = {0};
    char WanRate_RX[8] = {0};
    char WanRate_TX[8] = {0};
    const char *optStatus = NULL;
    int X_ZYXEL_InterfaceUpTime = 0, len, i;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &iid, &devInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SYSTEM_INFO, &iid, &sysInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	if(zcfgFeObjJsonGet(RDM_OID_DEV_INFO_MEM_STAT, &iid, &memoryObj) == ZCFG_SUCCESS){	
		json_object_object_add(sysInfoObj, "Total", JSON_OBJ_COPY(json_object_object_get(memoryObj, "Total")));
		json_object_object_add(sysInfoObj, "Free", JSON_OBJ_COPY(json_object_object_get(memoryObj, "Free")));
		zcfgFeJsonObjFree(memoryObj);
	}

	if(zcfgFeObjJsonGet(RDM_OID_DEV_INFO_PS_STAT, &iid, &cpuObj) == ZCFG_SUCCESS){	
		json_object_object_add(sysInfoObj, "CPUUsage", JSON_OBJ_COPY(json_object_object_get(cpuObj, "CPUUsage")));
		zcfgFeJsonObjFree(cpuObj);
	}

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL, &iid, &firewallInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_HOSTS, &iid, &hostObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
	zcfgFeJsonObjFree(hostObj);

	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL_LEVEL, &iid, &firewallLevelObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
	replaceParam(firewallInfoObj, "X_ZYXEL_DefaultConfig", firewallLevelObj, "X_ZYXEL_DefaultConfig");
	zcfgFeJsonObjFree(firewallLevelObj);

	ethIfaceJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceJarray, ethIfaceObj);
		if( true == json_object_get_boolean(json_object_object_get(ethIfaceObj,"Upstream"))) {
			X_ZYXEL_InterfaceUpTime = json_object_get_int(json_object_object_get(ethIfaceObj,"X_ZYXEL_UpTime"));
		}
	}

	dslChannelJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &iid, &dslChannelObj) == ZCFG_SUCCESS) {
		json_object_array_add(dslChannelJarray, dslChannelObj);
	}
#endif

	optIntfJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	IID_INIT(iid);
	while (zcfgFeObjJsonGetNext(RDM_OID_OPT_INTF, &iid, &optIbtfObj) == ZCFG_SUCCESS) {
		json_object_array_add(optIntfJarray, optIbtfObj);
#if defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)
        optStatus = json_object_get_string (json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if(zyUtilIGetScratchPadSetting(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "XGS", 3) == 0) {
                    strncpy(WanType, "XGSPON", 6);
                }
                else if (strncmp(psp_wantype, "XGPON1", 6) == 0) {
                    strncpy(WanType, "XGPON", 5);
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "2.5Gbps", 7);
                }

                if (strncmp(psp_wanrate, "1010", 4) == 0) {
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "10Gbps", 6);
                }
            }
        }

        if (strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if(strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
        strcpy(WanType, "GPON");
#else 
        strcpy(WanType, "");
        strcpy(WanRate_RX, "-1");
        strcpy(WanRate_TX, "-1");
#endif
	}
#endif // CONFIG_ZCFG_BE_MODULE_OPTICAL

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
	wwanBackupJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &iid, &wwanBackupObj) == ZCFG_SUCCESS) {
		json_object_array_add(wwanBackupJarray, wwanBackupObj);
	}
#endif
#endif

#ifdef ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO
	usbHostDevParJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	objIndex_t usbIid = {0};
	objIndex_t usbIid = {0};
	IID_INIT(usbIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &usbIid, &usbHostDevParObj) == ZCFG_SUCCESS){	
		json_object_array_add(usbHostDevParJarray, usbHostDevParObj);
	}
#else
	printf("%s : USB not support\n", __FUNCTION__);
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GPON_INFO, &iid, &gponstatusObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ONU, &iid, &gponObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_STAT, &iid, &wwanObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalStatusGetFail;
#endif
#endif

#if BUILD_PACKAGE_ZyIMS
	voipProfJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PROF, &iid, &voipProfObj) == ZCFG_SUCCESS) {
		Jaddi(voipProfObj, "idx", iid.idx[1]);
		json_object_array_add(voipProfJarray, voipProfObj);
	}

	voipProSIPfJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PROF_SIP, &iid, &voipProSIPfObj) == ZCFG_SUCCESS) {
		json_object_array_add(voipProSIPfJarray, voipProSIPfObj);
	}

	voipLineJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &iid, &voipLineObj) == ZCFG_SUCCESS) {
		Jaddi(voipLineObj, "idx0", iid.idx[0]);
		Jaddi(voipLineObj, "idx1", iid.idx[1]);
		Jaddi(voipLineObj, "idx2", iid.idx[2]);
		json_object_array_add(voipLineJarray, voipLineObj);
	}
#endif
    gponinfoObj = json_object_new_object();
#if defined(BROADCOM_PLATFORM) && defined(XPON_SUPPORT)
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
    if (zyUtil_is_sfp_present() != 0)
    {
        json_object_object_add(gponstatusObj, "sfp_presence",json_object_new_boolean(true));
    }
    else
    {
        json_object_object_add(gponstatusObj, "sfp_presence",json_object_new_boolean(false));
    }
    if (zyUtil_bosa_exist())
    {
        json_object_object_add(gponstatusObj, "bosa_presence",json_object_new_boolean(true));
    }
    else
    {
        json_object_object_add(gponstatusObj, "bosa_presence",json_object_new_boolean(false));
    }
    json_object_object_add(gponstatusObj, "Platform", json_object_new_string("BROADCOM"));//broadcom platform
    json_object_object_add(gponinfoObj, "gponStatusObj",gponstatusObj);
#endif
#endif

#if defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
   json_object_object_add(gponObj, "Platform", json_object_new_string("ECONET"));// econet platform
   json_object_object_add(gponinfoObj, "gponStatusObj",gponObj);
#endif


#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	getWifiInfo(&ssidJarray);
	len = json_object_array_length (ssidJarray);
	for (i = 0; i < len; i++) {
		tmpSSIDobj = json_object_array_get_idx(ssidJarray, i);
		json_object_object_add(tmpSSIDobj, "wifiPassword", json_object_new_string(""));
	}
#endif
	getWanLanList_ext(&wanLanJarray);
	
	statusJobj = json_object_new_object();
	json_object_array_add(Jarray, statusJobj);
	json_object_object_add(statusJobj, "DeviceInfo", devInfoObj);
	json_object_object_add(statusJobj, "SystemInfo", sysInfoObj);
	json_object_object_add(statusJobj, "FirewallInfo", firewallInfoObj);
	json_object_object_add(statusJobj, "LanPortInfo", ethIfaceJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	json_object_object_add(statusJobj, "WiFiInfo", ssidJarray);
#endif
	json_object_object_add(statusJobj, "WanLanInfo", wanLanJarray);
	json_object_object_add(statusJobj, "DslChannelInfo", dslChannelJarray);
	json_object_object_add(statusJobj, "OptIntfInfo", optIntfJarray);

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
	json_object_object_add(statusJobj, "WWANBackupInfo", wwanBackupJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_object_add(statusJobj, "WWANStatsInfo", wwanObj);
#endif
#endif

#ifdef ZYXEL_GUIVUE_USB_INFO_SHOW_SYSINFO
	json_object_object_add(statusJobj, "USBStatsInfo", usbHostDevParJarray);
#endif

#ifdef PACKAGE_WEB_GUI_VUE
	json_object_object_add(statusJobj, "GponStatsInfo", gponinfoObj);
#else
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_object_add(statusJobj, "GponStatsInfo", gponstatusObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	json_object_object_add(statusJobj, "GponInfo", gponObj);
#endif
#endif // PACKAGE_WEB_GUI_VUE

#if BUILD_PACKAGE_ZyIMS
	json_object_object_add(statusJobj, "VoipProfInfo", voipProfJarray);
	json_object_object_add(statusJobj, "VoipProfSIPInfo", voipProSIPfJarray);
	json_object_object_add(statusJobj, "VoipLineInfo", voipLineJarray);
#endif
    Jadds (statusJobj, "WanType", WanType);
    Jadds (statusJobj, "WanRate_RX", WanRate_RX);
    Jadds (statusJobj, "WanRate_TX", WanRate_TX);
    Jaddi (statusJobj, "X_ZYXEL_InterfaceUpTime", X_ZYXEL_InterfaceUpTime);
	
	return ZCFG_SUCCESS;

zcfgFeDalStatusGetFail:
	json_object_put(devInfoObj);
	json_object_put(sysInfoObj);
	json_object_put(firewallInfoObj);
	json_object_put(firewallLevelObj);
	json_object_put(hostObj);
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_put(gponstatusObj);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_GPON
	json_object_put(gponObj);
#endif
	return ret;

}

/*
 *  Function Name: zcfgFeDalGfiberCardStatusGet
 *  Description: Only get useful parameter or object to GUI.
 *
 */
zcfgRet_t zcfgFeDalGfiberCardStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *statusJobj = NULL;
	struct json_object *devInfoObj = NULL;
	struct json_object *wanLanJarray = NULL;
	struct json_object *ethnewObj = NULL, *ethIfaceObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *ssidJarray = NULL, *MainSSIDobj = NULL, *GuestSSIDObj = NULL, *newssidJarray = NULL;
#endif
	struct json_object *dhcpv4SerObj = NULL;
	struct json_object *partctlObj = NULL;
	struct json_object *ethIfaceJarray = NULL, *ethwanIfaceObj = NULL;
	struct json_object *dslChannelJarray = NULL;
	struct json_object *optIntfJarray = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	struct json_object *dslChannelObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	struct json_object *gponstatusObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	struct json_object *optIbtfObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	struct json_object *wwanObj = NULL, *wwanBackupObj = NULL;
#endif
	objIndex_t iid;
	int count = 0, currentBitRate = 0, len = 0, i, isSFP =0;
	const char *lanport = NULL, *lanMacAddr = NULL, *duplexMode = NULL, *optStatus = NULL;
	char ethConnectionStatus[16] = {0};
	char WanType[16] = {0};
	char psp_wantype[16] = {0};
	char psp_wanrate[16] = {0};
	char WanRate_RX[8] = {0};
	char WanRate_TX[8] = {0};
	bool ethPortExist = false;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	bool MainSSID = false, Guest24GSSIDEnable = false, Main24GSSIDEnable = false, Guest5GSSIDEnable = false, Main5GSSIDEnable = false, GuestSSIDEnable = false, support24G = false, support5G = false;
	const char *guestOperatingFrequencyBand = NULL, *mainOperatingFrequencyBand = NULL;
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	struct json_object *oneSsidJobj = NULL;
	bool oneSsidEnable = false;
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &iid, &oneSsidJobj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	oneSsidEnable = json_object_get_boolean(json_object_object_get(oneSsidJobj, "Enable"));
	zcfgFeJsonObjFree(oneSsidJobj);
#endif

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &iid, &devInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SRV_POOL, &iid, &dhcpv4SerObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;

#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &iid, &wwanBackupObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_STAT, &iid, &wwanObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
	Jadds(devInfoObj, "PackageVersion", json_object_get_string (json_object_object_get(wwanBackupObj, "PackageVersion")));
	zcfgFeJsonObjFree(wwanBackupObj);
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_GPON_INFO, &iid, &gponstatusObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalCardPageStatusGetFail;
#endif

	ethIfaceJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethwanIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceJarray, ethwanIfaceObj);
	}

#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	dslChannelJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &iid, &dslChannelObj) == ZCFG_SUCCESS) {
		json_object_array_add(dslChannelJarray, dslChannelObj);
	}
#endif

	optIntfJarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_OPT_INTF, &iid, &optIbtfObj) == ZCFG_SUCCESS) {
		json_object_array_add(optIntfJarray, optIbtfObj);
#if defined(CONFIG_BCM96858) && defined(ZYXEL_BOARD_DETECT_BY_GPIO)
        optStatus = json_object_get_string (json_object_object_get(optIbtfObj, "Status"));

        if (strcmp(optStatus, "Up") == 0) {
            if(zyUtilIGetScratchPadSetting(&psp_wantype, &psp_wanrate) == ZCFG_SUCCESS) {
                if (strncmp(psp_wantype, "XGS", 3) == 0) {
                    strncpy(WanType, "XGSPON", 6);
                }
                else if (strncmp(psp_wantype, "XGPON1", 6) == 0) {
                    strncpy(WanType, "XGPON", 5);
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "2.5Gbps", 7);
                }				

                if (strncmp(psp_wanrate, "1010", 4) == 0)
                {
                    strncpy(WanRate_RX, "10Gbps", 6);
                    strncpy(WanRate_TX, "10Gbps", 6);
                }
            }
        }

        if(strlen(WanType) == 0) {
            strncpy(WanType, "Unknown", 7);
        }

        if(strlen(WanRate_RX) == 0) {
            strncpy(WanRate_RX, "-1", 2);
            strncpy(WanRate_TX, "-1", 2);
        }
#elif defined(ECONET_PLATFORM) && defined(XPON_SUPPORT)
        strcpy(WanType, "GPON");
#else 
        strcpy(WanType, "");
        strcpy(WanRate_RX, "-1");
        strcpy(WanRate_TX, "-1");
#endif
	}
#endif // CONFIG_ZCFG_BE_MODULE_OPTICAL

	ethnewObj = json_object_new_object();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		lanport = json_object_get_string (json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		isSFP = json_object_get_boolean (json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP"));
#if (defined(ZCFG_VMG4927_B50A_Generic) || defined (ZCFG_EMG6726_B10A_Generic) )
		if(!strncmp (lanport, "LAN1", 4)) {
#else
		if(!strncmp (lanport, "LAN", 3)) {
#endif
			lanMacAddr = json_object_get_string (json_object_object_get(ethIfaceObj, "MACAddress"));
			Jadds (ethnewObj, "LanMac", lanMacAddr);
		}
		else if (!strcmp (lanport, "ETHWAN")  && !isSFP) {
			ethPortExist = true;
			ethConnectionStatus[0] = '\0';
			currentBitRate = json_object_get_int (json_object_object_get(ethIfaceObj, "CurrentBitRate"));
			duplexMode = json_object_get_string (json_object_object_get(ethIfaceObj, "DuplexMode"));
			if (currentBitRate > 0) {
				snprintf (ethConnectionStatus, sizeof(ethConnectionStatus), "%d/%s", currentBitRate, duplexMode);
			}
			else {
				strcat (ethConnectionStatus, "Connection down");
			}
			Jadds (ethnewObj, "EthConnectionStatus", ethConnectionStatus);
		}
		else if (!strcmp (lanport, "ETHWAN")  && isSFP) {
			ethPortExist = true;
			ethConnectionStatus[0] = '\0';
			currentBitRate = json_object_get_int (json_object_object_get(ethIfaceObj, "CurrentBitRate"));
			duplexMode = json_object_get_string (json_object_object_get(ethIfaceObj, "DuplexMode"));
			if (currentBitRate > 0) {
				snprintf (ethConnectionStatus, sizeof(ethConnectionStatus), "%d/%s", currentBitRate, duplexMode);
			}
			else {
				strcat (ethConnectionStatus, "Connection down");
			}
			Jadds (ethnewObj, "SFPConnectionStatus", ethConnectionStatus);
		}
		Jaddb (ethnewObj, "ethPortExist", ethPortExist);
		zcfgFeJsonObjFree (ethIfaceObj);
	}
#ifndef ABQA_CUSTOMIZATION
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PAREN_CTL_PROF, &iid, &partctlObj) == ZCFG_SUCCESS){
		count++;
		zcfgFeJsonObjFree(partctlObj);		
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	getWifiInfo(&ssidJarray);
	//Just GET Main SSID object to GUI
	newssidJarray = json_object_new_array();
	len = json_object_array_length (ssidJarray);
	for (i = 0; i < len; i++) {
		MainSSIDobj = json_object_array_get_idx(ssidJarray, i);
		json_object_object_add(MainSSIDobj, "wifiPassword", json_object_new_string(""));
		MainSSID = json_object_get_boolean (json_object_object_get(MainSSIDobj, "X_ZYXEL_MainSSID"));
		if (MainSSID) {
			json_object_array_add(newssidJarray, JSON_OBJ_COPY(MainSSIDobj));
			GuestSSIDObj = json_object_array_get_idx(ssidJarray, i + 1);
			json_object_object_add(GuestSSIDObj, "wifiPassword", json_object_new_string(""));
			json_object_array_add(newssidJarray, JSON_OBJ_COPY(GuestSSIDObj));
			mainOperatingFrequencyBand = json_object_get_string (json_object_object_get(MainSSIDobj, "OperatingFrequencyBand"));
			guestOperatingFrequencyBand = json_object_get_string (json_object_object_get(GuestSSIDObj, "OperatingFrequencyBand"));
			if (!strcmp (mainOperatingFrequencyBand, "2.4GHz")) {
				Main24GSSIDEnable = json_object_get_boolean (json_object_object_get(MainSSIDobj, "Enable"));
			}
			else if (!strcmp (mainOperatingFrequencyBand, "5GHz")) {
				Main5GSSIDEnable = json_object_get_boolean (json_object_object_get(MainSSIDobj, "Enable"));
			}
			if (!strcmp (guestOperatingFrequencyBand, "2.4GHz")) {
				support24G = true;
				Guest24GSSIDEnable = json_object_get_boolean (json_object_object_get(GuestSSIDObj, "Enable"));
			}
			else if (!strcmp (guestOperatingFrequencyBand, "5GHz")) {
				support5G = true;
				Guest5GSSIDEnable = json_object_get_boolean (json_object_object_get(GuestSSIDObj, "Enable"));
			}
		}
	}
	zcfgFeJsonObjFree(ssidJarray);

#ifdef ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
	if (Guest24GSSIDEnable && Guest5GSSIDEnable){
		GuestSSIDEnable = true;
	}
	else{
		GuestSSIDEnable = false;
	}
#else
	//Consider if some project only support 2.4G or 5G only in the future.
	if (support24G && support5G) {	
		if (Main24GSSIDEnable && Main5GSSIDEnable && Guest24GSSIDEnable && Guest5GSSIDEnable) {
			GuestSSIDEnable = true;
		} 
	}
	else if (!support24G && support5G) {
		if (Main5GSSIDEnable && Guest5GSSIDEnable) {
			GuestSSIDEnable = true;
		}
	}
	else if (support24G && !support5G) {
		if (Main24GSSIDEnable && Guest24GSSIDEnable) {
			GuestSSIDEnable = true;
		}
	}
#endif //ZYXEL_RADIO_MAIN_SSID_INDEPENDENT
#endif
	getWanLanList_ext(&wanLanJarray);
	statusJobj = json_object_new_object();
	json_object_array_add(Jarray, statusJobj);
	json_object_object_add(statusJobj, "WanLanInfo", wanLanJarray);
	json_object_object_add(statusJobj, "DeviceInfo", devInfoObj);
	json_object_object_add(statusJobj, "LanPortInfo", ethnewObj);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	json_object_object_add(statusJobj, "WiFiInfo", newssidJarray);
#endif
	json_object_object_add(statusJobj, "Dhcp4SerPoolInfo", dhcpv4SerObj);
#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_object_add(statusJobj, "WWANStatsInfo", wwanObj);
#endif
#endif
	json_object_object_add(statusJobj, "LanInfo", ethIfaceJarray);
	json_object_object_add(statusJobj, "DslChannelInfo", dslChannelJarray);
	json_object_object_add(statusJobj, "OptIntfInfo", optIntfJarray);
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_object_add(statusJobj, "GponStatsInfo", gponstatusObj);
#endif
#ifndef ABQA_CUSTOMIZATION
	Jaddi(statusJobj,"partctlNum",count);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	Jaddb(statusJobj,"GuestSSIDEnable",GuestSSIDEnable);
#endif
	Jadds (statusJobj, "WanType", WanType);
	Jadds (statusJobj, "WanRate_RX", WanRate_RX);
	Jadds (statusJobj, "WanRate_TX", WanRate_TX);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	Jaddb (statusJobj, "oneSsidEnable", oneSsidEnable);
#endif
	return ZCFG_SUCCESS;

zcfgFeDalCardPageStatusGetFail:
	json_object_put(devInfoObj);
	json_object_put(dhcpv4SerObj);
#ifdef ZyXEL_WEB_GUI_SHOW_WWAN
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	json_object_put(wwanBackupObj);	
	json_object_put(wwanObj);
#endif
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
	json_object_put(gponstatusObj);
#endif
	return ret;
}
#endif

/*
 *  Function Name: zcfgFeDalLanPortInfoGet
 *  Description: Only get useful parameter or object to GUI.
 *
 */
zcfgRet_t zcfgFeDalLanPortInfoGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *statusJobj = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ethIfaceJarray = NULL, *ethIfaceObj = NULL;
	objIndex_t iid;	

	ethIfaceJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &iid, &ethIfaceObj) == ZCFG_SUCCESS) {
		json_object_array_add(ethIfaceJarray, ethIfaceObj);
	}

	statusJobj = json_object_new_object();
	json_object_array_add(Jarray, statusJobj);
	json_object_object_add(statusJobj, "LanPortInfo", ethIfaceJarray);
	
	return ret;
}

void zcfgFeDalShowNetworkMAP(struct json_object *Jarray){
	int i, len = 0, len1 = 0;
	struct json_object *obj = NULL;
	const char *Name = NULL;
	char chartmp[64] = {0};
	const char *IPv6Address = NULL;
	struct json_object *networkMAPInfoJobj = NULL;
	struct json_object *hostsJarray;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	networkMAPInfoJobj = json_object_array_get_idx(Jarray, 0);
	if(networkMAPInfoJobj == NULL)
		return;
	hostsJarray = Jget(networkMAPInfoJobj,"lanhosts");
	if(hostsJarray == NULL)
		return;
	
	printf("%-20s %-17s %-30s %-20s %-16s %-5s\n",
		    "Name", "IP Address", "IPv6 Address", "MAC Address", "Address Source", "Connection Type");
	len = json_object_array_length(hostsJarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(hostsJarray, i);

		//if PhysAddress is empty, skip it.
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "PhysAddress")),""))
			continue;
		
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "HostName")),"")){
			Name = "Unknown";
			strcpy(chartmp,Name);
		}
		else{
			Name = json_object_get_string(json_object_object_get(obj, "HostName"));
			len1 = strlen(Name);
			strcpy(chartmp,Name);
			if(chartmp[len1-1] == '\n')
				chartmp[len1-1] = '\0';
		}
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPAddress6")),"")){
			if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPLinkLocalAddress6")),"")){
				IPv6Address = "N/A";
			}
			else{
				IPv6Address = json_object_get_string(json_object_object_get(obj, "IPLinkLocalAddress6"));
			}
		}
		else 
			IPv6Address = json_object_get_string(json_object_object_get(obj, "IPAddress6"));
		
		printf("%-20s %-17s %-30s %-20s %-16s %-5s \n",
			chartmp,
			json_object_get_string(json_object_object_get(obj, "IPAddress")),
			IPv6Address,
			json_object_get_string(json_object_object_get(obj, "PhysAddress")),
			json_object_get_string(json_object_object_get(obj, "AddressSource")),
			json_object_get_string(json_object_object_get(obj, "X_ZYXEL_ConnectionType")));
	}
}

void
zcfgFeDalLanHostsGetInit(struct json_object *obj,  struct json_object **dhcp4PoolIid, struct json_object **dhcp4StaticAddrIid )
{
	Jadds(obj, "SupportedFrequencyBands", "");
	Jadds(obj, "WiFiname", "");
	Jadds(obj, "DeviceSource", "HOSTS_HOST");
#ifdef ZYXEL_HOMEPLUG_DEVICE_DISCOVER
		if(!strcmp("HomePlug", Jgets(obj,"HostName")))
			Jadds(obj, "DeviceIcon", "connectivity-device-nas");
		else
#endif
	Jadds(obj, "DeviceIcon", "");
	Jaddb(obj, "Internet_Blocking_Enable", false);
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
	Jaddb(obj, "Internet_Priority_Enable", false);
	Jaddb(obj, "Device_StaticDhcp_Enable", false);
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	Jaddb(obj, "BrowsingProtection", false);
	Jaddb(obj, "TrackingProtection", false);
	Jaddb(obj, "IOTProtection", false);
	Jadds(obj, "Profile", "");
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	Jaddi(obj, "X_ZYXEL_FirstSeen", 0);
	Jaddi(obj, "X_ZYXEL_LastSeen", 0);
	Jadds(obj, "SourceVendorClassID", "");
#endif
	replaceParam(obj, "DeviceName", obj, "HostName");
	Jadds(obj, "curHostName", "");
	
	if(!strcmp("", Jgets(obj,"DeviceName")))
		Jadds(obj, "DeviceName", "Unknown");

	if(!strcmp("", Jgets(obj,"HostName")))
		Jadds(obj, "HostName", "Unknown");

	//DHCPv4 static address info
	Jaddb(obj, "dhcp4PoolExist", false);
	genIid(dhcp4PoolIid, 0, 0, 0, 0, 0, 0);
	json_object_object_add(obj, "dhcp4PoolIid", *dhcp4PoolIid);
	Jaddb(obj, "dhcp4StaticAddrExist", false);
	genIid(dhcp4StaticAddrIid, 0, 0, 0, 0, 0, 0);
	json_object_object_add(obj, "dhcp4StaticAddrIid", *dhcp4StaticAddrIid);
	Jaddb(obj, "dhcp4StaticAddrEnable", false);
	Jadds(obj, "dhcp4StaticAddr", "");
	Jaddi(obj, "dhcp4StaticAddrNum", 0);
	Jaddb(obj, "dhcp4StaticAddrUsedByOtherHost", false);

} /* zcfgFeDalLanHostsGetInit */


void
zcfgFeDalLanHostHostInit(struct json_object *obj )
{
	Jadds(obj, "Alias", "");
	Jadds(obj, "PhysAddress", "");
	Jadds(obj, "IPAddress", "");
	Jadds(obj, "IPAddress6", "");
	Jadds(obj, "IPLinkLocalAddress6", "N/A");
	Jadds(obj, "AddressSource", "Static");
	Jadds(obj, "DHCPClient", "");
	Jaddi(obj, "LeaseTimeRemaining", 0);
	Jadds(obj, "AssociatedDevice", "");
	Jadds(obj, "Layer1Interface", "");
	Jadds(obj, "Layer3Interface", "");
	Jadds(obj, "VendorClassID", "");
	Jadds(obj, "ClientID", "");
	Jadds(obj, "UserClassID", "");
	Jadds(obj, "HostName", "");
	Jaddb(obj, "Active", false);
	Jaddb(obj, "X_ZYXEL_DeleteLease", false);
	Jadds(obj, "X_ZYXEL_ConnectionType", "");
	Jadds(obj, "X_ZYXEL_ConnectedAP", "");
	Jadds(obj, "X_ZYXEL_HostType", "");
	Jadds(obj, "X_ZYXEL_CapabilityType", "");
	Jaddi(obj, "X_ZYXEL_PhyRate", 0);
	Jaddb(obj, "X_ZYXEL_WiFiStatus", false);
	Jaddi(obj, "X_ZYXEL_SignalStrength", 0);
	Jaddi(obj, "X_ZYXEL_SNR", 0);
	Jaddi(obj, "X_ZYXEL_RSSI", 0);
	Jadds(obj, "X_ZYXEL_SoftwareVersion", "");
	Jaddi(obj, "X_ZYXEL_LastUpdate", 0);
	Jadds(obj, "X_ZYXEL_Address6Source", "");
	Jadds(obj, "X_ZYXEL_DHCP6Client", "");
	Jaddi(obj, "X_ZYXEL_BytesSent", 0);
	Jaddi(obj, "X_ZYXEL_BytesReceived", 0);
	Jadds(obj, "X_ZYXEL_OperatingStandard", "");
	Jaddi(obj, "X_ZYXEL_LastDataDownlinkRate", 0);
	Jaddi(obj, "X_ZYXEL_LastDataUplinkRate", 0);
	Jaddi(obj, "IPv4AddressNumberOfEntries", 0);
	Jaddi(obj, "IPv6AddressNumberOfEntries", 0);
	Jadds(obj, "ClientDuid", "");
	Jadds(obj, "ExpireTime", "");
} /* zcfgFeDalLanHostHostInit */

void
zcfgFeDalLanHostsUpdatIcon(struct json_object *obj)
{
	const char *currIcon;

	currIcon = json_object_get_string(json_object_object_get(obj, "DeviceIcon"));
	if(!strcmp(currIcon, "iPC") || !strcmp(currIcon, "")){
		json_object_object_add(obj, "icon", json_object_new_string("ipc"));
	}else if(!strcmp(currIcon, "iGame")){
		json_object_object_add(obj, "icon", json_object_new_string("igame"));
	}else if(!strcmp(currIcon, "iPrinter")){
		json_object_object_add(obj, "icon", json_object_new_string("iprinter"));
	}else if(!strcmp(currIcon, "iStb")){
		json_object_object_add(obj, "icon", json_object_new_string("istb"));
	}else if(!strcmp(currIcon, "iCamera")){
		json_object_object_add(obj, "icon", json_object_new_string("icamera"));
	}else if(!strcmp(currIcon, "iServer")){
		json_object_object_add(obj, "icon", json_object_new_string("iserver"));
	}else if(!strcmp(currIcon, "iTelephone")){
		json_object_object_add(obj, "icon", json_object_new_string("itelephone"));
	}else if(!strcmp(currIcon, "iRouter")){
		json_object_object_add(obj, "icon", json_object_new_string("irouter"));
	}else if(!strcmp(currIcon, "iOthers")){
		json_object_object_add(obj, "icon", json_object_new_string("iothers"));
	}else if(!strcmp(currIcon, "iStereo")){
		json_object_object_add(obj, "icon", json_object_new_string("istereo"));
	}else if(!strcmp(currIcon, "iMobile")){
		json_object_object_add(obj, "icon", json_object_new_string("imobile"));
	}else if(!strcmp(currIcon, "iTV")){
		json_object_object_add(obj, "icon", json_object_new_string("itv"));
	}
#ifdef ZYXEL_HOMEPLUG_DEVICE_DISCOVER
		else if(!strcmp(currIcon, "connectivity-device-nas")){
			json_object_object_add(obj, "icon", json_object_new_string("connectivity-device-nas"));
		}
#endif
} /* zcfgFeDalLanHostsUpdatIcon */
/*
*	All parameters of RDM_OID_HOSTS and the following parameters
*	DeviceIcon
*	curHostName
*	DeviceName
*	dhcp4PoolExist
*	dhcp4PoolIid
*	dhcp4StaticAddrUsedByOtherHost
*	dhcp4StaticAddrExist
*	dhcp4StaticAddrNum
*	dhcp4StaticAddrIid
*	dhcp4StaticAddrEnable
*	dhcp4StaticAddr
*
*	Parameters For EAAAA and CAAAB
*	Customized X_ZYXEL_ConnectionType
*	SupportedFrequencyBands
*	WiFiname
*/
extern bool validateIPv4(const char *ipStr); // implement zcfg_fe_dal.c
zcfgRet_t zcfgFeDalLanHostsGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	struct json_object *ssidJarray = NULL;
	struct json_object *iconJobj, *iconJarray = ssidJarray;
	struct json_object *dhcp4PoolJobj, *dhcp4PoolJarray, *dhcp4PoolIid;
	struct json_object *dhcp4StaticAddrJobj, *dhcp4StaticAddrJarray, *dhcp4StaticAddrIid;
	objIndex_t iid;
	struct json_object *stbJobj = NULL;
	int i, j, len, len2, JarrayLen;
	const char *MacAddress, *IPAddress, *PhysAddress, *Layer3Interface, *Interface, *Yiaddr;
#ifdef ZYXEL_OPAL_EXTENDER
	const char *AssocDev = NULL;
	FILE *fptr;
	char dump[64] = {0};
	bool match = false;
#endif
#if (defined(CAAAB_CUSTOMIZATION) || defined (EAAAA_NETWORKMAP_CUSTOMIZATION) || defined(ZYXEL_WEB_NETWORK_MAP_NEW_STYLE))
	struct json_object *ssidJobj;
	struct json_object *radioJobj, *radioJarray;
	const char *SupportedFrequencyBands, *LowerLayers, *Layer1Interface ;
	bool X_ZYXEL_MainSSID;
	int  X_ZYXEL_Attribute = 0;
#endif
#ifdef EAAAA_NETWORKMAP_CUSTOMIZATION
	char X_ZYXEL_ConnectionType[64] = {0};
#endif
#ifdef ABPY_CUSTOMIZATION
	objIndex_t iid_tmp, devInfoIid;
	const char *ConnectedAP = NULL;
	char ClientNeighbor[66] = {0};
	struct json_object *obj_tmp = NULL, *devInfoObj = NULL;
#endif
	zcfgFeWholeObjJsonGet("Device.X_ZYXEL_Change_Icon_Name.", &iconJarray);
	zcfgFeWholeObjJsonGet("Device.X_ZYXEL_STB_VENDOR_ID.", &stbJobj);
	zcfgFeWholeObjJsonGet("Device.DHCPv4.Server.Pool.", &dhcp4PoolJarray);
#if (defined(CAAAB_CUSTOMIZATION) || defined (EAAAA_NETWORKMAP_CUSTOMIZATION) || defined(ZYXEL_WEB_NETWORK_MAP_NEW_STYLE))
	zcfgFeWholeObjJsonGet("Device.WiFi.SSID.", &ssidJarray);
	zcfgFeWholeObjJsonGet("Device.WiFi.Radio.", &radioJarray);
#endif
	//update hosts list first
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_HOSTS, &iid, &obj) == ZCFG_SUCCESS){
		zcfgFeJsonObjFree(obj);
	}
	/* Get all in LAN HOSTS HOST (there are all connected ?)
	   Find all in Change_Icon
	   if in LAN_HOSTS_HOST then update parameter.
	   if not in LAN HOSTS_HOST then add parameter.
	*/

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_HOSTS_HOST, &iid, &obj) == ZCFG_SUCCESS){
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		bool Active = false;
		Active = json_object_get_boolean(json_object_object_get(obj,"Active"));
		if(Active == false)
			continue;
#endif
		PhysAddress = Jgets(obj, "PhysAddress");
		IPAddress = Jgets(obj, "IPAddress");
		Layer3Interface = Jgets(obj, "Layer3Interface");
		Layer3Interface = strstr(Layer3Interface, "IP."); //Layer3Interface value may be "IP.Interface.1" or "Device.IP.Interface.1" skip "Device."
		
		if(Layer3Interface == NULL)//consider the host under extender
			Layer3Interface = "";

		if(PhysAddress == NULL || IPAddress == NULL || Layer3Interface == NULL)
			continue;


#ifdef ZYXEL_OPAL_EXTENDER
		AssocDev = Jgets(obj, "AssociatedDevice");
        /* In opal extender (bridge mode), it may get the ip address of the associated device
         * from the arp table. In the other case, it should allow empty ip address.
         * It is a limitation to get the ip address of the associate deveice in extender.*/
		if (AssocDev && (strstr(AssocDev,"AssociatedDevice.") != NULL))
		{
		    if (strlen(PhysAddress) < 2)
		        continue;
		}
		else
		{
			// It's not legal or empty.
			if( (strlen(PhysAddress) < 2) ||
			  ( (validateIPv4(IPAddress) == false) && (false == json_object_get_boolean(json_object_object_get(obj, "Active")))))
				continue;
		}
#else
#ifdef ZYXEL_HOMEPLUG_DEVICE_DISCOVER
		if( (strlen(PhysAddress) < 2) || ((validateIPv4(IPAddress) == false)&&(strcmp("HomePlug", Jgets(obj,"HostName"))!= 0)))//It's not legal or empty.
#else
		if( (strlen(PhysAddress) < 2) || (validateIPv4(IPAddress) == false) )//It's not legal or empty.
			continue;
#endif
#endif

		/* init the default value (original)*/
		zcfgFeDalLanHostsGetInit(obj,  &dhcp4PoolIid, &dhcp4StaticAddrIid );
		/* init the icon variable (original) */
		zcfgFeDalLanHostsUpdatIcon(obj);

#ifdef ABPY_CUSTOMIZATION
		ConnectedAP = Jgets(obj, "X_ZYXEL_ConnectedAP");
		if(!strcmp(ConnectedAP, "") || !strcmp(ConnectedAP, "Gateway")){
			IID_INIT(devInfoIid);
			if(zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &devInfoIid, &devInfoObj) == ZCFG_SUCCESS){
				json_object_object_add(obj, "X_ZYXEL_Neighbor", JSON_OBJ_COPY(json_object_object_get(devInfoObj, "ModelName")));
				zcfgFeJsonObjFree(devInfoObj);
			}
			else{
				json_object_object_add(obj, "X_ZYXEL_Neighbor", json_object_new_string("Controller"));
			}			
		}
		else if(validateMacAddr(ConnectedAP)){
			IID_INIT(iid_tmp);
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_HOSTS_HOST, &iid_tmp, &obj_tmp) == ZCFG_SUCCESS){
				if(!strcmp(ConnectedAP, Jgets(obj_tmp, "PhysAddress"))){
					ZOS_STRNCPY(ClientNeighbor, Jgets(obj_tmp, "HostName"), sizeof(ClientNeighbor));
					json_object_object_add(obj, "X_ZYXEL_Neighbor", json_object_new_string(ClientNeighbor));					
					zcfgFeJsonObjFree(obj_tmp);
					break;
				}
				zcfgFeJsonObjFree(obj_tmp);
			}
		}
		else{
			// reserved for corner case.
		}
#endif

		len = json_object_array_length(dhcp4PoolJarray);
		for(i=0;i<len;i++){
			dhcp4PoolJobj = json_object_array_get_idx(dhcp4PoolJarray, i);
			if(dhcp4PoolJobj == NULL)
				break;
			Interface = Jgets(dhcp4PoolJobj, "Interface");
			if(Interface == NULL || *Interface == '\0')
				continue;
			Interface = strstr(Interface, "IP."); //Interface value may be "IP.Interface.1" or "Device.IP.Interface.1" skip "Device."

			if(!strcmp(Interface, Layer3Interface)){
				Jaddb(obj, "dhcp4PoolExist", true);
				genIid(&dhcp4PoolIid, i+1, 0, 0, 0, 0, 0);
				json_object_object_add(obj, "dhcp4PoolIid", dhcp4PoolIid);
				
				json_object_object_add(obj, "staticIP", json_object_new_string(""));

				dhcp4StaticAddrJarray = Jget(dhcp4PoolJobj, "StaticAddress");
				if(dhcp4StaticAddrJarray == NULL){
					break;
				}

				len2 = json_object_array_length(dhcp4StaticAddrJarray);
				Jaddi(obj, "dhcp4StaticAddrNum", len2);
				for(j=0;j<len2;j++){
					dhcp4StaticAddrJobj = json_object_array_get_idx(dhcp4StaticAddrJarray, j);
					MacAddress = Jgets(dhcp4StaticAddrJobj, "Chaddr");
					if(MacAddress && isSameMac(MacAddress, PhysAddress)){
						Jaddb(obj, "dhcp4StaticAddrExist", true);
						replaceParam(obj, "dhcp4StaticAddr", dhcp4StaticAddrJobj, "Yiaddr");
						genIid(&dhcp4StaticAddrIid, i+1, j+1, 0, 0, 0, 0);
						json_object_object_add(obj, "dhcp4StaticAddrIid", dhcp4StaticAddrIid);
						replaceParam(obj, "dhcp4StaticAddrEnable", dhcp4StaticAddrJobj, "Enable");
						break;
					}
					Yiaddr = Jgets(dhcp4StaticAddrJobj, "Yiaddr");
					if(Yiaddr && !strcmp(Yiaddr, IPAddress)){ //different MAC but IP is the same
						Jaddb(obj, "dhcp4StaticAddrUsedByOtherHost", true);
						break;
					}
				}
				break;
			}
		}

#if (defined(CAAAB_CUSTOMIZATION) || defined (EAAAA_NETWORKMAP_CUSTOMIZATION) || defined(ZYXEL_WEB_NETWORK_MAP_NEW_STYLE))
		Layer1Interface = Jgets(obj, "Layer1Interface");
		if(strstr(Layer1Interface, "SSID")){
			if(strstr(Layer1Interface, "Device.WiFi."))
				sscanf(Layer1Interface, "Device.WiFi.SSID.%d", &i);
			else
				sscanf(Layer1Interface, "WiFi.SSID.%d", &i);
			
			ssidJobj = json_object_array_get_idx(ssidJarray, i-1);
			replaceParam(obj, "WiFiname", ssidJobj, "SSID");
			LowerLayers = Jgets(ssidJobj, "LowerLayers");

			X_ZYXEL_MainSSID = Jgetb(ssidJobj, "X_ZYXEL_MainSSID");
			X_ZYXEL_Attribute = Jgeti(ssidJobj, "X_ZYXEL_Attribute");
			Jaddb(obj, "X_ZYXEL_MainSSID", X_ZYXEL_MainSSID);
			Jaddi(obj, "X_ZYXEL_Attribute", X_ZYXEL_Attribute);
			
			if(LowerLayers != NULL)
				sscanf(LowerLayers, "WiFi.Radio.%d", &i);		
			
			radioJobj = json_object_array_get_idx(radioJarray, i-1);
			SupportedFrequencyBands = Jgets(radioJobj, "OperatingFrequencyBand");

			if(SupportedFrequencyBands != NULL)
					Jadds(obj, "SupportedFrequencyBands", SupportedFrequencyBands);
#ifdef EAAAA_NETWORKMAP_CUSTOMIZATION
			sprintf(X_ZYXEL_ConnectionType, "%s %s", Jgets(obj, "X_ZYXEL_ConnectionType"), SupportedFrequencyBands);
			Jadds(obj, "X_ZYXEL_ConnectionType", X_ZYXEL_ConnectionType);
#endif
		}
#ifdef EAAAA_NETWORKMAP_CUSTOMIZATION
		else if(strstr(Layer1Interface, "Radio")){
			sscanf(Layer1Interface, "Device.WiFi.Radio.%d", &i);
			radioJobj = json_object_array_get_idx(radioJarray, i-1);
			SupportedFrequencyBands = Jgets(radioJobj, "SupportedFrequencyBands");
			Jadds(obj, "SupportedFrequencyBands", SupportedFrequencyBands);
			
			sprintf(X_ZYXEL_ConnectionType, "%s %s", Jgets(obj, "X_ZYXEL_ConnectionType"), SupportedFrequencyBands);
			Jadds(obj, "X_ZYXEL_ConnectionType", X_ZYXEL_ConnectionType);
		}
#endif
#endif
#ifdef ZYXEL_OPAL_EXTENDER
		fptr = fopen("/tmp/zy1905.json","r");
		if(fptr != NULL){
			while(fgets(dump, sizeof(dump)-1, fptr) != NULL){
				if(strstr(dump, PhysAddress) != NULL && strstr(dump, "IEEE1905Id") != NULL)
				{
					match=true;
				}
				if (match && strstr(dump, "DeviceRole") != NULL){
					if (strstr(dump, "Controller") != NULL)
						Jadds(obj, "DeviceRole", "controller");
					else if(strstr(dump, "Agent") != NULL)
						Jadds(obj, "DeviceRole", "agent");

					match=false;
					break;
				}
			}//end while
			fclose(fptr);
		}
#endif
		json_object_array_add(Jarray, obj);
	}


	JarrayLen = json_object_array_length(Jarray);
			//ICON info
	len = json_object_array_length(iconJarray);
	for(i=0;i<len;i++){
		iconJobj = json_object_array_get_idx(iconJarray, i);
		if(iconJobj == NULL)
			continue;
		MacAddress = Jgets(iconJobj, "MacAddress");
		if(MacAddress == NULL)
			continue;
		/* update change icon into lan hosts host */
		for ( j = 0 ; j < JarrayLen; j ++ ) {
			obj = json_object_array_get_idx(Jarray, j);
			PhysAddress = Jgets(obj, "PhysAddress");
			if(isSameMac(PhysAddress, MacAddress)){
				Jadds(obj, "DeviceSource", "CHANGE_ICON");
				replaceParam(obj, "DeviceIcon", iconJobj, "DeviceIcon");
				replaceParam(obj, "Internet_Blocking_Enable", iconJobj, "Internet_Blocking_Enable");
	#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
				replaceParam(obj, "Internet_Priority_Enable", iconJobj, "Internet_Priority_Enable");
				replaceParam(obj, "Device_StaticDhcp_Enable", iconJobj, "Device_StaticDhcp_Enable");
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY
				replaceParam(obj, "BrowsingProtection", iconJobj, "BrowsingProtection");
				replaceParam(obj, "TrackingProtection", iconJobj, "TrackingProtection");	
				replaceParam(obj, "IOTProtection", iconJobj, "IOTProtection");
				replaceParam(obj, "Profile", iconJobj, "Profile");
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY
				replaceParam(obj, "X_ZYXEL_FirstSeen", iconJobj, "X_ZYXEL_FirstSeen");
				replaceParam(obj, "X_ZYXEL_LastSeen", iconJobj, "X_ZYXEL_LastSeen");
				replaceParam(obj, "SourceVendorClassID", iconJobj, "SourceVendorClassID");
	#endif
				if(strcmp("", Jgets(iconJobj, "HostName")))
					replaceParam(obj, "curHostName", iconJobj, "HostName");

				/* update the DeviceIcon */
				zcfgFeDalLanHostsUpdatIcon(obj);
				break;
			}
		}
		/* not found in lanhost then we add it. ( off line ? */
		if ( j == JarrayLen ) {
			obj = json_object_new_object();
			if ( obj ) {
				/* fill original RDM_OID_HOSTS_HOST */
				zcfgFeDalLanHostHostInit(obj );
				/* init the default value (original)*/
				zcfgFeDalLanHostsGetInit(obj,  &dhcp4PoolIid, &dhcp4StaticAddrIid );
				
				replaceParam(obj, "PhysAddress", iconJobj, "MacAddress");
				Jadds(obj, "DeviceSource", "CHANGE_ICON");
				replaceParam(obj, "DeviceIcon", iconJobj, "DeviceIcon");
				replaceParam(obj, "Internet_Blocking_Enable", iconJobj, "Internet_Blocking_Enable");
	#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP				
				replaceParam(obj, "Internet_Priority_Enable", iconJobj, "Internet_Priority_Enable");
				replaceParam(obj, "Device_StaticDhcp_Enable", iconJobj, "Device_StaticDhcp_Enable");
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY
				replaceParam(obj, "BrowsingProtection", iconJobj, "BrowsingProtection");
				replaceParam(obj, "TrackingProtection", iconJobj, "TrackingProtection");
				replaceParam(obj, "IOTProtection", iconJobj, "IOTProtection");
				replaceParam(obj, "Profile", iconJobj, "Profile");
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY
				replaceParam(obj, "X_ZYXEL_FirstSeen", iconJobj, "X_ZYXEL_FirstSeen");
				replaceParam(obj, "X_ZYXEL_LastSeen", iconJobj, "X_ZYXEL_LastSeen");
				replaceParam(obj, "SourceVendorClassID", iconJobj, "SourceVendorClassID");
	#endif
				if(strcmp("", Jgets(iconJobj, "HostName")))
					replaceParam(obj, "curHostName", iconJobj, "HostName");

				/* update the DeviceIcon */
				zcfgFeDalLanHostsUpdatIcon(obj);
				/* Add the obj into Jarray */
				json_object_array_add(Jarray, obj);
			}
		}
	}

	zcfgFeJsonObjFree(iconJarray);
	zcfgFeJsonObjFree(stbJobj);
	zcfgFeJsonObjFree(dhcp4PoolJarray);
#if (defined(CAAAB_CUSTOMIZATION) || defined (EAAAA_NETWORKMAP_CUSTOMIZATION) || defined(ZYXEL_WEB_NETWORK_MAP_NEW_STYLE))
	zcfgFeJsonObjFree(ssidJarray);
	zcfgFeJsonObjFree(radioJarray);
#endif

	return ret;
}

zcfgRet_t zcfgFeDalNetworkMAPEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	const char *macAddr = NULL, *curmacAddr = NULL, *GUIicon = NULL, *hostName = NULL;
	char icon[16] = {0};
	bool changeIconRuleExist = false, Internet_Blocking_Enable = false;
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP	
	bool Internet_Priority_Enable = false, Device_StaticDhcp_Enable = false;
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	const char *Profile = NULL, *SourceVendorClassID = NULL;
	bool BrowsingProtection = false, TrackingProtection = false, IOTProtection = false;
	unsigned int X_ZYXEL_FirstSeen =0, X_ZYXEL_LastSeen = 0;
#endif

	macAddr = json_object_get_string(json_object_object_get(Jobj,"MacAddress"));
	GUIicon = json_object_get_string(json_object_object_get(Jobj,"DeviceIcon"));
	hostName = json_object_get_string(json_object_object_get(Jobj,"HostName"));
	Internet_Blocking_Enable = json_object_get_boolean(json_object_object_get(Jobj,"Internet_Blocking_Enable"));
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
	Internet_Priority_Enable = json_object_get_boolean(json_object_object_get(Jobj,"Internet_Priority_Enable"));
	Device_StaticDhcp_Enable = json_object_get_boolean(json_object_object_get(Jobj,"Device_StaticDhcp_Enable"));
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	BrowsingProtection = json_object_get_boolean(json_object_object_get(Jobj,"BrowsingProtection"));
	TrackingProtection = json_object_get_boolean(json_object_object_get(Jobj,"TrackingProtection"));
	IOTProtection = json_object_get_boolean(json_object_object_get(Jobj,"IOTProtection"));
	Profile = json_object_get_string(json_object_object_get(Jobj,"Profile"));
	X_ZYXEL_FirstSeen = json_object_get_int(json_object_object_get(Jobj,"X_ZYXEL_FirstSeen"));
	X_ZYXEL_LastSeen = json_object_get_int(json_object_object_get(Jobj,"X_ZYXEL_LastSeen"));
	SourceVendorClassID = json_object_get_string(json_object_object_get(Jobj,"SourceVendorClassID"));	
#endif

	if(!strcmp(GUIicon, "ipc")){
		strcat(icon, "iPC");
	}else if(!strcmp(GUIicon, "igame")){
		strcat(icon, "iGame");
	}else if(!strcmp(GUIicon, "iprinter")){
		strcat(icon, "iPrinter");
	}else if(!strcmp(GUIicon, "istb")){
		strcat(icon, "iStb");
	}else if(!strcmp(GUIicon, "icamera")){
		strcat(icon, "iCamera");
	}else if(!strcmp(GUIicon, "iserver")){
		strcat(icon, "iServer");
	}else if(!strcmp(GUIicon, "itelephone")){
		strcat(icon, "iTelephone");
	}else if(!strcmp(GUIicon, "irouter")){
		strcat(icon, "iRouter");
	}else if(!strcmp(GUIicon, "iothers")){
		strcat(icon, "iOthers");
	}else if(!strcmp(GUIicon, "istereo")){
		strcat(icon, "iStereo");
	}else if(!strcmp(GUIicon, "imobile")){
		strcat(icon, "iMobile");
	}else if(!strcmp(GUIicon, "itv")){
		strcat(icon, "iTV");
	}
	if(macAddr!=NULL){
		IID_INIT(Iid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &Iid, &obj) == ZCFG_SUCCESS){
			curmacAddr = json_object_get_string(json_object_object_get(obj,"MacAddress"));
			if(isSameMac(macAddr, curmacAddr)){
				changeIconRuleExist = true;
				if(json_object_object_get(Jobj, "DeviceIcon")){
					json_object_object_add(obj, "DeviceIcon", json_object_new_string(icon));
				}
				if(json_object_object_get(Jobj, "HostName")){
					json_object_object_add(obj, "HostName", json_object_new_string(hostName));
				}
				if(json_object_object_get(Jobj, "MacAddress")){
					json_object_object_add(obj, "MacAddress", json_object_new_string(macAddr));
				}
				if(json_object_object_get(Jobj, "Internet_Blocking_Enable")){
					json_object_object_add(obj, "Internet_Blocking_Enable", json_object_new_boolean(Internet_Blocking_Enable));
				}
	#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
				if(json_object_object_get(Jobj, "Internet_Priority_Enable")){
					json_object_object_add(obj, "Internet_Priority_Enable", json_object_new_boolean(Internet_Priority_Enable));
				}
				if(json_object_object_get(Jobj, "Device_StaticDhcp_Enable")){
					json_object_object_add(obj, "Device_StaticDhcp_Enable", json_object_new_boolean(Device_StaticDhcp_Enable));
				}				
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY			
				if(json_object_object_get(Jobj, "BrowsingProtection")){
					json_object_object_add(obj, "BrowsingProtection", json_object_new_boolean(BrowsingProtection));
				}
				if(json_object_object_get(Jobj, "TrackingProtection")){
					json_object_object_add(obj, "TrackingProtection", json_object_new_boolean(TrackingProtection));
				}			
				if(json_object_object_get(Jobj, "IOTProtection")){
					json_object_object_add(obj, "IOTProtection", json_object_new_boolean(IOTProtection));
				}
				if(json_object_object_get(Jobj, "Profile")){
					json_object_object_add(obj, "Profile", json_object_new_string(Profile));
				}
	#endif
	#ifdef ZYXEL_HOME_CYBER_SECURITY
				if(json_object_object_get(Jobj, "X_ZYXEL_FirstSeen")){
					json_object_object_add(obj, "X_ZYXEL_FirstSeen", json_object_new_int(X_ZYXEL_FirstSeen));
				}			
				if(json_object_object_get(Jobj, "X_ZYXEL_LastSeen")){
					json_object_object_add(obj, "X_ZYXEL_LastSeen", json_object_new_int(X_ZYXEL_LastSeen));
				}
				if(json_object_object_get(Jobj, "SourceVendorClassID")){
					json_object_object_add(obj, "SourceVendorClassID", json_object_new_string(SourceVendorClassID));
				}			
	#endif			
				zcfgFeObjJsonSet(RDM_OID_CHANGE_ICON_NAME, &Iid, obj, NULL);
				zcfgFeJsonObjFree(obj);
				break;			
			}
			zcfgFeJsonObjFree(obj);
		}
	}
	IID_INIT(Iid);
	if(!changeIconRuleExist){
		ret = zcfgFeObjJsonAdd(RDM_OID_CHANGE_ICON_NAME, &Iid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &Iid, &obj);

			if(json_object_object_get(Jobj, "DeviceIcon")){
				json_object_object_add(obj, "DeviceIcon", json_object_new_string(icon));
			}
			if(json_object_object_get(Jobj, "HostName")){
				json_object_object_add(obj, "HostName", json_object_new_string(hostName));
			}
			if(json_object_object_get(Jobj, "MacAddress")){
				json_object_object_add(obj, "MacAddress", json_object_new_string(macAddr));
			}
			if(json_object_object_get(Jobj, "Internet_Blocking_Enable")){
				json_object_object_add(obj, "Internet_Blocking_Enable", json_object_new_boolean(Internet_Blocking_Enable));
			}
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
			if(json_object_object_get(Jobj, "Internet_Priority_Enable")){
				json_object_object_add(obj, "Internet_Priority_Enable", json_object_new_boolean(Internet_Priority_Enable));
			}
			if(json_object_object_get(Jobj, "Device_StaticDhcp_Enable")){
				json_object_object_add(obj, "Device_StaticDhcp_Enable", json_object_new_boolean(Device_StaticDhcp_Enable));
			}			
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY			
			if(json_object_object_get(Jobj, "BrowsingProtection")){
				json_object_object_add(obj, "BrowsingProtection", json_object_new_boolean(BrowsingProtection));
			}
			if(json_object_object_get(Jobj, "TrackingProtection")){
				json_object_object_add(obj, "TrackingProtection", json_object_new_boolean(TrackingProtection));
			}		
			if(json_object_object_get(Jobj, "IOTProtection")){
				json_object_object_add(obj, "IOTProtection", json_object_new_boolean(IOTProtection));
			}
			if(json_object_object_get(Jobj, "Profile")){
				json_object_object_add(obj, "Profile", json_object_new_string(Profile));
			}
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
			if(json_object_object_get(Jobj, "X_ZYXEL_FirstSeen")){
				json_object_object_add(obj, "X_ZYXEL_FirstSeen", json_object_new_int(X_ZYXEL_FirstSeen));
			}	
			if(json_object_object_get(Jobj, "X_ZYXEL_LastSeen")){
				json_object_object_add(obj, "X_ZYXEL_LastSeen", json_object_new_int(X_ZYXEL_LastSeen));
			}	
			if(json_object_object_get(Jobj, "SourceVendorClassID")){
				json_object_object_add(obj, "SourceVendorClassID", json_object_new_string(SourceVendorClassID));
			}
#endif

			zcfgFeObjJsonBlockedSet(RDM_OID_CHANGE_ICON_NAME, &Iid, obj, NULL);
			zcfgFeJsonObjFree(obj);
		}
	}
		
	return ret;
}


zcfgRet_t zcfgFeDalNetworkMAPPost(struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *inputObj = NULL;
	objIndex_t Iid;
	char path[64];

	IID_INIT(Iid);
	ret = zcfgFeObjJsonAdd(RDM_OID_CHANGE_ICON_NAME, &Iid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &Iid, &obj);
		if(( inputObj = json_object_object_get(Jobj, "DeviceIcon")) != NULL){
			json_object_object_add(obj, "DeviceIcon", json_object_new_string(json_object_get_string(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "HostName")) != NULL){
			json_object_object_add(obj, "HostName", json_object_new_string(json_object_get_string(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "MacAddress")) != NULL){
			json_object_object_add(obj, "MacAddress", json_object_new_string(json_object_get_string(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "Internet_Blocking_Enable")) != NULL){
			json_object_object_add(obj, "Internet_Blocking_Enable", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}
#ifdef ZYXEL_WEB_GUI_HOST_PRIORITY_STATICIP
		if(( inputObj = json_object_object_get(Jobj, "Internet_Priority_Enable")) != NULL){
			json_object_object_add(obj, "Internet_Priority_Enable", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "Device_StaticDhcp_Enable")) != NULL){
			json_object_object_add(obj, "Device_StaticDhcp_Enable", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}			
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
		if(( inputObj = json_object_object_get(Jobj, "BrowsingProtection")) != NULL){
			json_object_object_add(obj, "BrowsingProtection", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "TrackingProtection")) != NULL){
			json_object_object_add(obj, "TrackingProtection", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "IOTProtection")) != NULL){
			json_object_object_add(obj, "IOTProtection", json_object_new_boolean(json_object_get_boolean(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "Profile")) != NULL){
			json_object_object_add(obj, "Profile", json_object_new_string(json_object_get_string(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "X_ZYXEL_FirstSeen")) != NULL){
			json_object_object_add(obj, "X_ZYXEL_FirstSeen", json_object_new_int(json_object_get_int(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "X_ZYXEL_LastSeen")) != NULL){
			json_object_object_add(obj, "X_ZYXEL_LastSeen", json_object_new_int(json_object_get_int(inputObj)));
		}
		if(( inputObj = json_object_object_get(Jobj, "SourceVendorClassID")) != NULL){
			json_object_object_add(obj, "SourceVendorClassID", json_object_new_string(json_object_get_string(inputObj)));
		}
#endif

		zcfgFeObjJsonBlockedSet(RDM_OID_CHANGE_ICON_NAME, &Iid, obj, NULL);
		zcfgFeJsonObjFree(obj);
	}
	if(Jarray != NULL){
		sprintf(path, "X_ZYXEL_Change_Icon_Name.%d",Iid.idx[0]);
		obj = json_object_new_object();
		Jadds(obj, "path", path);
		json_object_array_add(Jarray, obj);
	}

	return ret;
}


zcfgRet_t zcfgFeDalNetworkMAPDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_DELETE_REJECT;
	struct json_object *obj = NULL;
	struct json_object *macObj = NULL;
	objIndex_t Iid = {0};
	const char *macAddr = NULL, *curmacAddr = NULL;
	
	macObj= json_object_object_get(Jobj,"MacAddress");
	if ( macObj != NULL ) {
		macAddr = Jgets(Jobj,"MacAddress");

		/* Remove Change ICON and HOSTS_HOST */
		IID_INIT(Iid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_CHANGE_ICON_NAME, &Iid, &obj) == ZCFG_SUCCESS){
			curmacAddr = Jgets(obj,"MacAddress");
			if(isSameMac(macAddr, curmacAddr)){
				ret = zcfgFeObjJsonDel(RDM_OID_CHANGE_ICON_NAME, &Iid, NULL);
				zcfgFeJsonObjFree(obj);
				break;
			}
			zcfgFeJsonObjFree(obj);
		}
		
		IID_INIT(Iid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_HOSTS_HOST, &Iid, &obj) == ZCFG_SUCCESS){
			curmacAddr = Jgets(obj, "PhysAddress");
			if(isSameMac(macAddr, curmacAddr)){ 
				ret = zcfgFeObjJsonDel(RDM_OID_HOSTS_HOST, &Iid, NULL);
				zcfgFeJsonObjFree(obj);
				break;
			}
			zcfgFeJsonObjFree(obj);
		}
	}

	if(ret == ZCFG_DELETE_REJECT){
//		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SchedulerRule.delete_pcp_sch_err_msg"));
//		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret; 
	}
	return ret;
} /* zcfgFeDalNetworkMAPDelete */


/*
[{
  "lanhosts":[{},...],
  "wanInfo":{"wanStatus": 1|0, "wanIfaceExist" : 1|0}
}]
*/
zcfgRet_t zcfgFeDalNetworkMAPGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	struct json_object *ipIfaceObj;
	struct json_object *v4AddrObj = NULL;
	struct json_object *v6AddrObj = NULL;
	struct json_object *networkMAPInfoJobj = NULL;
	struct json_object *hostsJarray;
	struct json_object *wanInfoJobj;
#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	struct json_object *stbJobj;
	objIndex_t iid = {0};
#endif
	struct json_object  *routingObj=NULL;
	const char *X_ZYXEL_IfName = NULL, *Status = NULL, *IPAddress = NULL;
	char wantype[9] = {0};
	bool X_ZYXEL_DefaultGatewayIface;
	int wanstatus = 0, wanIfaceExist = 0;
	objIndex_t ipIfaceIid = {0};
	objIndex_t v4AddrIid = {0};
	objIndex_t v6AddrIid = {0};
	objIndex_t routerIid = {0};
	char v6DefaultGWList[512] = {0};
	char ifacePath[32] = {0};
	const char *addrStatus = NULL;

	/*For getting IPv6 Default Gateway List*/
	IID_INIT(routerIid);
	routerIid.idx[0] = 1;
	routerIid.level = 1;
	if(zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &routerIid, &routingObj) == ZCFG_SUCCESS) {
		zos_snprintf(v6DefaultGWList, sizeof(v6DefaultGWList), "%s", json_object_get_string(json_object_object_get(routingObj, "X_ZYXEL_ActiveV6DefaultGateway")));
		zcfgFeJsonObjFree(routingObj);
	}
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS){
		zos_snprintf(ifacePath, sizeof(ifacePath), "IP.Interface.%u", ipIfaceIid.idx[0]);

		X_ZYXEL_IfName = json_object_get_string(json_object_object_get(ipIfaceObj,"X_ZYXEL_IfName"));
		if(X_ZYXEL_IfName == NULL){
			zcfgFeJsonObjFree(ipIfaceObj);
			continue;
		}
		if(strstr(X_ZYXEL_IfName, "br")){
			zcfgFeJsonObjFree(ipIfaceObj);
			continue;
		}
		wanIfaceExist = 1;
		Status = json_object_get_string(json_object_object_get(ipIfaceObj, "Status"));
		if(Status == NULL){
			Status = "";
		}
		X_ZYXEL_DefaultGatewayIface = json_object_get_boolean(json_object_object_get(ipIfaceObj,"X_ZYXEL_DefaultGatewayIface"));
#if (!defined(INTERNET_LED_DEPEND_ON_DEFAULT_GW))
		X_ZYXEL_DefaultGatewayIface = true;
#endif
		if(X_ZYXEL_DefaultGatewayIface && (!strcmp(Status, "Up") || !strcmp(Status, "LowerLayerDown"))){
#ifdef OAAAA_CUSTOMIZATION
			bool bIPv6Exist = false;
			char sysCmd[128] = "\0";
#endif

			IID_INIT(v4AddrIid);
			v4AddrIid.level = 2;
			v4AddrIid.idx[0] = ipIfaceIid.idx[0];
			v4AddrIid.idx[1] = 1;
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &v4AddrIid, &v4AddrObj) == ZCFG_SUCCESS){
				IPAddress = json_object_get_string(json_object_object_get(v4AddrObj,"IPAddress"));
				addrStatus = json_object_get_string(json_object_object_get(v4AddrObj, "Status"));
				if(addrStatus == NULL){
					addrStatus = "";
				}

				if(IPAddress != NULL && strcmp(IPAddress, "") && !strcmp(addrStatus, "Enabled")
#ifdef ZYXEL_FAKE_IP_169_254_X_X		
					&& strncmp(IPAddress, "169.254.", 8)
#endif
				){
					wanstatus = 1;
					if(strlen(wantype) == 0){
						strcpy(wantype,"ipv4");
					}
					else if(!strcmp(wantype,"ipv6")){
						wantype[0] = '\0';
						strcpy(wantype,"ipv4ipv6");
					}
				}
				zcfgFeJsonObjFree(v4AddrObj);
			}
		}

		if(strstr(v6DefaultGWList, ifacePath) && (!strcmp(Status, "Up") || !strcmp(Status, "LowerLayerDown"))){
#ifdef OAAAA_CUSTOMIZATION
			sprintf(sysCmd, "ifconfig | grep -A4 \"nas\\|ppp\" | grep \"inet6 addr\" | grep \"Scope:Global\" > /var/wan6ipcheck");
			system(sysCmd);
			FILE *fp = fopen("/var/wan6ipcheck", "r");
			if(fp) {
				char wan6ip[512] = {0};
				fread(wan6ip, 1, sizeof(wan6ip) - 1, fp);
				if(strchr(wan6ip, ':'))
					bIPv6Exist = true;
				fclose(fp);
			}

			if ( bIPv6Exist == false )
			{
				IID_INIT(v6AddrIid);
				while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, &v6AddrObj) == ZCFG_SUCCESS) {
					if ( v6AddrIid.idx[0] == ipIfaceIid.idx[0] ) {
						int bEnable = json_object_get_int(json_object_object_get(v6AddrObj, "Enable"));
						if ( bEnable ) {
							Jaddi(v6AddrObj,"Enable",false);
							zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &v6AddrIid, v6AddrObj, NULL);
						}
					}

					zcfgFeJsonObjFree(v6AddrObj);
				}
			}
			else
#endif /* OAAAA_CUSTOMIZATION */
			{
				IID_INIT(v6AddrIid);
				while(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V6_ADDR, &ipIfaceIid, &v6AddrIid, &v6AddrObj) == ZCFG_SUCCESS) {
					if( json_object_get_boolean(json_object_object_get(v6AddrObj, "Enable")) ) {
						IPAddress = json_object_get_string(json_object_object_get(v6AddrObj,"IPAddress"));
						addrStatus = json_object_get_string(json_object_object_get(v6AddrObj, "Status"));
						if(addrStatus == NULL){
							addrStatus = "";
						}
						if(IPAddress != NULL && strcmp(IPAddress, "") && !strcmp(addrStatus, "Enabled")) {
							wanstatus = 1;
							if(strlen(wantype) == 0) {
								strncpy(wantype,"ipv6", sizeof(wantype)-1);
							}
							else if(!strcmp(wantype,"ipv4")) {
								wantype[0] = '\0';
								strncpy(wantype,"ipv4ipv6", sizeof(wantype)-1);
							}
						}
						zcfgFeJsonObjFree(v6AddrObj);
						break;
					}
					zcfgFeJsonObjFree(v6AddrObj);
				}
			}
		}
		
		zcfgFeJsonObjFree(ipIfaceObj);
	}

#ifdef ZYXEL_PPTPD_RELAY
		if( access(pptp_uptime_file,F_OK) == 0 ){
			/*pptp_uptime_file record connection time, if this file exist, mean pptp connection up now.*/
			wanstatus = 1;
		}
#endif

	networkMAPInfoJobj = json_object_new_object();
	json_object_array_add(Jarray, networkMAPInfoJobj);
	wanInfoJobj = json_object_new_object();
	json_object_object_add(networkMAPInfoJobj, "wanInfo", wanInfoJobj);
	Jaddi(wanInfoJobj,"wanStatus",wanstatus);
	Jaddi(wanInfoJobj,"wanIfaceExist",wanIfaceExist);
   	Jadds(wanInfoJobj,"wanType",wantype);
	
	hostsJarray = json_object_new_array();
	json_object_object_add(networkMAPInfoJobj, "lanhosts", hostsJarray);
	zcfgFeDalLanHostsGet(NULL, hostsJarray, NULL);

#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	IID_INIT(iid);
	if(zcfgFeObjJsonGet(RDM_OID_S_T_B_VENDOR_I_D, &iid, &stbJobj) == ZCFG_SUCCESS){
		json_object_object_add(networkMAPInfoJobj, "STBVendorID", stbJobj);
	}
#endif

	return ZCFG_SUCCESS;
}


zcfgRet_t zcfgFeDalNetworkMAP(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalNetworkMAPGet(Jobj, Jarray, NULL);
	}
	else if(!strcmp(method, "PUT")){
		ret = zcfgFeDalNetworkMAPEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "POST")){
		ret = zcfgFeDalNetworkMAPPost(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")){
		ret = zcfgFeDalNetworkMAPDelete(Jobj, NULL);
	}

	return ret;
}

zcfgRet_t zcfgFeDalIPoE(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg, const char *action)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	objIndex_t v4ClientIid = {0};

	if (Jobj != NULL && json_object_object_get(Jobj,"index"))
	{
		char *index = NULL;
		char ipIfacePath[20] = {0};

		index = (char *)json_object_get_string(json_object_object_get(Jobj,"index"));
		strcpy(ipIfacePath, "IP.Interface.");
		if (index)
			strcat(ipIfacePath, index);
		if (!getSpecificObj(RDM_OID_DHCPV4_CLIENT, "Interface", json_type_string, ipIfacePath, &v4ClientIid, NULL))
		{
			return ZCFG_INTERNAL_ERROR;
		}
	}
	else
	{
		return ZCFG_INTERNAL_ERROR;
	}
	//create and send MSG
	sendMsgHdr = (zcfgMsg_t *)malloc(sizeof(zcfgMsg_t));
	if (sendMsgHdr == NULL)
	{
		return ZCFG_INTERNAL_ERROR;
	}
	if(strcmp(action,"Release") == 0){
		sendMsgHdr->type = ZCFG_MSG_WAN_IP_RELEASE;
	}
	else if(strcmp(action,"Renew") == 0){
		sendMsgHdr->type = ZCFG_MSG_WAN_IP_RENEW;
	}
	sendMsgHdr->oid = RDM_OID_DHCPV4_CLIENT;
	sendMsgHdr->length = 0;
	sendMsgHdr->srcEid = ZCFG_EID_DALCMD;
	sendMsgHdr->dstEid = ZCFG_EID_ESMD;
	memcpy(sendMsgHdr->objIid, &v4ClientIid, sizeof(objIndex_t));
	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 30000);
	if (ret == ZCFG_TIMEOUT)
		return ZCFG_SUCCESS;
	ZOS_FREE(recvBuf);
	return ret;
}

zcfgRet_t zcfgFeDalPPPoE(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg, const char *action)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	objIndex_t pppIid = {0};

	if (Jobj != NULL && json_object_object_get(Jobj,"wanpppIdx"))
	{
		int wanpppIdx = 0;
		wanpppIdx = json_object_get_int(json_object_object_get(Jobj,"wanpppIdx"));
		if (wanpppIdx)
		{
			pppIid.level = 1;
			pppIid.idx[0] = wanpppIdx;
		}
		else return ZCFG_INTERNAL_ERROR;
	}
	else
	{
		return ZCFG_INTERNAL_ERROR;
	}
	//create and send MSG
	sendMsgHdr = (zcfgMsg_t *)malloc(sizeof(zcfgMsg_t));
	if (sendMsgHdr == NULL)
	{
		return ZCFG_INTERNAL_ERROR;
	}
	if(strcmp(action,"Disconnect") == 0){
		sendMsgHdr->type = ZCFG_MSG_WAN_PPP_DISCONNECT;
	}
	else if(strcmp(action,"Connect") == 0){
		sendMsgHdr->type = ZCFG_MSG_WAN_PPP_CONNECT;
	}
	sendMsgHdr->oid = RDM_OID_PPP_IFACE;
	sendMsgHdr->length = 0;
	sendMsgHdr->srcEid = ZCFG_EID_DALCMD;
	sendMsgHdr->dstEid = ZCFG_EID_ESMD;
	memcpy(sendMsgHdr->objIid, &pppIid, sizeof(objIndex_t));
	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 30000);
	if (ret == ZCFG_TIMEOUT)
		return ZCFG_SUCCESS;
	ZOS_FREE(recvBuf);
	return ret;
}

zcfgRet_t zcfgFeDalStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalStatusGet(Jobj, Jarray, NULL);
	}
	else if(!strcmp(method, "PUT")){
		char *action = NULL;
		if (Jobj != NULL && json_object_object_get(Jobj,"action")) {
			action = (char *)json_object_get_string(json_object_object_get(Jobj,"action"));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				char intfdowncode[64]={0};
			if(!strcmp(action, "Disconnect")){
				strcpy(intfdowncode,"PPP disconnection");
				SetInterfaceDownCode(intfdowncode); 
			}
			else if(!strcmp(action, "Release")){
				strcpy(intfdowncode,"DHCP disconnection");
				SetInterfaceDownCode(intfdowncode); 
			}					
#endif			
			if(!strcmp(action, "Renew")||!strcmp(action, "Release")){
				ret = zcfgFeDalIPoE(Jobj, Jarray, replyMsg, action);
			}
			else if(!strcmp(action, "Connect")||!strcmp(action, "Disconnect")){
				ret = zcfgFeDalPPPoE(Jobj, Jarray, replyMsg, action);
			}
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalCardPageStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalCardPageStatusGet(Jobj, Jarray, NULL);
	}

	return ret;
}

#ifdef GFIBER_CUSTOMIZATION
zcfgRet_t zcfgFeDalGfiberStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalGfiberStatusGet(Jobj, Jarray, NULL);
	}
/*
	else if(!strcmp(method, "PUT")){
		char *action = NULL;
		if (Jobj != NULL && json_object_object_get(Jobj,"action")) {
			action = json_object_get_string(json_object_object_get(Jobj,"action"));
			if(!strcmp(action, "Renew")||!strcmp(action, "Release")){
				ret = zcfgFeDalIPoE(Jobj, Jarray, replyMsg, action);
			}
			else if(!strcmp(action, "Connect")||!strcmp(action, "Disconnect")){
				ret = zcfgFeDalPPPoE(Jobj, Jarray, replyMsg, action);
			}
		}
	}
*/

	return ret;
}

zcfgRet_t zcfgFeDalGfiberCardStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalGfiberCardStatusGet(Jobj, Jarray, NULL);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeDalLanPortInfo(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "GET")){
		ret = zcfgFeDalLanPortInfoGet(Jobj, Jarray, NULL);
	}

	return ret;
}

#if defined(ZYXEL_OPAL_EXTENDER) || defined(ZYXEL_ECONET_OPERATION_MODE)
zcfgRet_t zcfgFeDalExtenderConnStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t              ret = ZCFG_SUCCESS;
    objIndex_t             repIid = {0};
    objIndex_t             radioIid = {0};
    objIndex_t             routerIid = {0};
    objIndex_t             v4FwdIid = {0};
    objIndex_t             v6FwdIid = {0};
    objIndex_t             IpDiagIPPingIid = {0};
    connetedStep_t         connStep = CONNECTED_STEP_EXTENDER;
    struct json_object    *diagIPPingJobj = NULL;
    struct json_object    *repObj = NULL;
    struct json_object    *radioObj = NULL;
    struct json_object    *v4FwdObj = NULL;
    struct json_object    *v6FwdObj = NULL;
    struct json_object    *apObject = NULL;
    struct json_object    *curAPInfo = NULL;
    struct json_object    *status = NULL;
    const char            *Mode = NULL;
    const char            *DiagnosticsState = NULL;
    const char            *GatewayIPAddress = NULL;
    const char            *NextHop = NULL;
    zcfgMsg_t             *sendMsgHdr = NULL;
    char                  *tmp = NULL;
    char                  *sendBuf = NULL;
    char                  *recvBuf = NULL;
    FILE                  *fp = NULL;
    char                   extMode[16] = {0};
    char                   iface[16] = {0};
    char                   cmd[64] = {0};
    char                   buf[128] = {0};
    char                   protocol[8] = {0};

    IID_INIT(repIid);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI_REPEATER
    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REPEATER, &repIid, &repObj)) == ZCFG_SUCCESS)
    {
        Mode = json_object_get_string(json_object_object_get(repObj, "Mode"));
    }
    if (Mode == NULL || ret != ZCFG_SUCCESS)
    {
        Jaddi(Jobj, "connStep", connStep);
        Jadds(Jobj, "Mode", "");
        Jadds(Jobj, "protocol", "");
        return ret;
    }
    strncpy(extMode, Mode, sizeof(extMode));
#else
    IID_INIT(repIid);
    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_OPERATION_MODE, &repIid, &repObj)) == ZCFG_SUCCESS)
    {
        Mode = json_object_get_int(json_object_object_get(repObj, "Mode"));
    }
    if (ret != ZCFG_SUCCESS)
    {
        Jaddi(Jobj, "connStep", connStep);
        Jadds(Jobj, "Mode", "");
        Jadds(Jobj, "protocol", "");
        return ret;
    }
#endif

    zcfgFeJsonObjFree(repObj);
#ifdef ZYXEL_OPAL_EXTENDER
    if(strcmp(extMode,"repeater") == 0) /* repeater mode */
#else
    if(Mode == 2) /* repeater mode */
#endif
    {
        IID_INIT(radioIid);
        radioIid.level = 1;
        radioIid.idx[0] = 2; //5G
        if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_RADIO, &radioIid, &radioObj) == ZCFG_SUCCESS)
        {
            tmp = json_object_get_string(json_object_object_get(radioObj, "X_ZYXEL_IfName"));

            if(strcmp(tmp, ""))
            {
                strncpy(iface, tmp, sizeof(iface));
            }
            else
            {
                ret = ZCFG_INVALID_ARGUMENTS;
            }
            zcfgFeJsonObjFree(radioObj);
        }
        //send message to zywifid to get current status
        apObject = json_object_new_array();
        json_object_array_add(apObject, json_object_new_string("CurStationStatus"));

        tmp = json_object_to_json_string(apObject);
        if ((sendBuf = calloc(1, (sizeof(zcfgMsg_t)+ strlen(tmp) +1))))
        {
            sendMsgHdr = (zcfgMsg_t *)sendBuf;
            sendMsgHdr->type = ZCFG_MSG_WIFI_ST_UPDATE;
            sendMsgHdr->srcEid = ZCFG_EID_DALCMD;
            sendMsgHdr->dstEid = ZCFG_EID_WIFID;
            strcpy(sendMsgHdr->reserve, iface);
            sendMsgHdr->length = strlen(tmp)+1;
            strcpy(sendBuf+sizeof(zcfgMsg_t), tmp);

            if(zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 30000) == ZCFG_SUCCESS)
            {
                if(((zcfgMsg_t *)recvBuf)->statCode == ZCFG_SUCCESS && ((zcfgMsg_t *)recvBuf)->length > 0 &&
                    (curAPInfo = json_tokener_parse(recvBuf+sizeof(zcfgMsg_t))) > 0)
                {
                    if((status = json_object_object_get(curAPInfo, "CurStationStatus")))
                    {
                        if (json_object_get_boolean(status) == true)
                        {
                            connStep = CONNECTED_STEP_GATEWAY;
                        }
                    }
                }
            }
        }
        zcfgFeJsonObjFree(apObject);
        if (recvBuf)
        {
            ZOS_FREE(recvBuf);
        }
        recvBuf = NULL;

        if(!is_error(curAPInfo))
        {
            zcfgFeJsonObjFree(curAPInfo);
        }
    } /* repeater mode */
#ifdef ZYXEL_OPAL_EXTENDER
    else /* ap mode */
#else
    else if(Mode == 1) /* ap mode */
#endif
    {
        IID_INIT(routerIid);
        routerIid.level = 1;
        routerIid.idx[0] = 1;
        //ping v4 GW
        IID_INIT(v4FwdIid);
        while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V4_FWD, &routerIid, &v4FwdIid, &v4FwdObj) == ZCFG_SUCCESS)
        {
            GatewayIPAddress= json_object_get_string(json_object_object_get(v4FwdObj, "GatewayIPAddress"));

            if (strcmp(GatewayIPAddress,""))
            {

                IID_INIT(IpDiagIPPingIid);
                if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, &diagIPPingJobj)) == ZCFG_SUCCESS)
                {
                    if(diagIPPingJobj != NULL)
                    {
                        json_object_object_add(diagIPPingJobj, "ProtocolVersion", json_object_new_string("IPv4"));
                        json_object_object_add(diagIPPingJobj, "Host", json_object_new_string(GatewayIPAddress));
                        json_object_object_add(diagIPPingJobj, "DiagnosticsState", json_object_new_string("Requested"));
                        json_object_object_add(diagIPPingJobj, "NumberOfRepetitions", json_object_new_int(1));

                        const char *ipping;
                        ipping = json_object_to_json_string(diagIPPingJobj);
                        if((ret = zcfgFeObjWrite(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, ipping, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) == ZCFG_SUCCESS)
                        {
                            sleep(1);
                            fp = fopen("/var/diagResult", "r");
                            if(fp != NULL)
                            {
                              while (fgets(buf, 128, fp) != NULL)
                              {
                                  if (strstr(buf, "1 packets received") != NULL)
                                  {
                                      connStep = CONNECTED_STEP_GATEWAY;
                                      strcpy(protocol,"IPv4");
                                      break;
                                  }
                              }
                              fclose(fp);
                            }

                        }
                    }
                    zcfgFeJsonObjFree(diagIPPingJobj);
                }
                zcfgFeJsonObjFree(v4FwdObj);
                break;
            }
            zcfgFeJsonObjFree(v4FwdObj);
        }

        //ping v6 GW
        if (connStep == CONNECTED_STEP_EXTENDER)
        {
            IID_INIT(routerIid);
            routerIid.level = 1;
            routerIid.idx[0] = 1;

            IID_INIT(v6FwdIid);
            while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &routerIid, &v6FwdIid, &v6FwdObj) == ZCFG_SUCCESS)
            {
                NextHop= json_object_get_string(json_object_object_get(v6FwdObj, "NextHop"));
                if (NextHop)
                {
                    IID_INIT(IpDiagIPPingIid);
                    if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, &diagIPPingJobj)) == ZCFG_SUCCESS)
                    {
                        if(diagIPPingJobj != NULL)
                        {
                            json_object_object_add(diagIPPingJobj, "ProtocolVersion", json_object_new_string("IPv6"));
                            json_object_object_add(diagIPPingJobj, "Host", json_object_new_string(NextHop));
                            json_object_object_add(diagIPPingJobj, "DiagnosticsState", json_object_new_string("Requested"));
                            json_object_object_add(diagIPPingJobj, "NumberOfRepetitions", json_object_new_int(1));

                            const char *ipping;
                            ipping = json_object_to_json_string(diagIPPingJobj);
                            if((ret = zcfgFeObjWrite(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, ipping, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) == ZCFG_SUCCESS)
                            {
                                sleep(1);
                                fp = fopen("/var/diagResult", "r");
                                if(fp != NULL)
                                {
                                  while (fgets(buf, 128, fp) != NULL)
                                  {
                                      if (strstr(buf, "1 packets received") != NULL)
                                      {
                                          connStep = CONNECTED_STEP_GATEWAY;
                                          strcpy(protocol,"IPv6");
                                          break;
                                      }
                                  }
                                  fclose(fp);
                                }
                            }
                        }
                        zcfgFeJsonObjFree(diagIPPingJobj);
                    }
                    zcfgFeJsonObjFree(v6FwdObj);
                    break;

                }
                zcfgFeJsonObjFree(v6FwdObj);
            }
        }
    } /* ap mode */
    //ping internet
    Jaddi(Jobj, "connStep", connStep);
    Jadds(Jobj, "Mode", extMode);
    Jadds(Jobj, "Protocol", protocol);

    return ret;
}
zcfgRet_t zcfgFeDalExtenderNetworkMAPGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    struct json_object    *networkMAPInfoJobj = NULL;
    struct json_object    *extenderStatusJobj = NULL;

    networkMAPInfoJobj = json_object_new_object();
    json_object_array_add(Jarray, networkMAPInfoJobj);

    extenderStatusJobj = json_object_new_object();
    json_object_object_add(networkMAPInfoJobj, "extenderStatus", extenderStatusJobj);
    zcfgFeDalExtenderConnStatusGet(extenderStatusJobj, NULL, NULL);

    return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalExtenderNetworkMAP(const char *method, struct json_object *Jobj, struct json_object *Jarray,char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if(!method || !Jobj)
        return ZCFG_INTERNAL_ERROR;

    if(!strcmp(method, "GET")){
        ret = zcfgFeDalExtenderNetworkMAPGet(Jobj, Jarray, NULL);
    }
    return ret;
}
#endif

