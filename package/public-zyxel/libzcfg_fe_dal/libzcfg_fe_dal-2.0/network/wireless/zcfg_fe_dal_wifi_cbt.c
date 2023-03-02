#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zos_api.h"

//#ifdef CAAAB_CUSTOMIZATION
#if defined(CAAAB_CUSTOMIZATION) || defined(ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE)

// RDM Json Object
struct json_object *cbtSsidObj = NULL;
struct json_object *cbt5GSsidObj = NULL;
struct json_object *cbt5GGuestSsidObj = NULL;
struct json_object *cbtSecObj = NULL;
struct json_object *cbt5GSecObj = NULL;
struct json_object *cbtDevInfoObj = NULL;
struct json_object *cbtSecTmpObj = NULL;
struct json_object *cbt5GSecTmpObj = NULL;
struct json_object *cbtApObj = NULL;
struct json_object *cbt5GApObj = NULL;

// RDM Iid
objIndex_t cbtSsidIid = {0};
objIndex_t cbt5GSsidIid = {0};
objIndex_t cbt5GGuestSsidIid = {0};
objIndex_t cbtSecIid = {0};
objIndex_t cbt5GSecIid = {0};
objIndex_t cbtDevInfoIid = {0};
objIndex_t cbtSecTmpIid = {0};
objIndex_t cbt5GSecTmpIid = {0};
objIndex_t cbtApIid = {0};
objIndex_t cbt5GApIid = {0};

//WiFi Access Point Sec
const char* CAAAB_X_ZYXEL_Auto_PSK;
const char* CAAAB_5G_X_ZYXEL_Auto_PSK;
const char* cbtWpaMode;
const char* cbt5GWpaMode;
bool CAAAB_X_ZYXEL_AutoGenPSK;
bool CAAAB_5G_X_ZYXEL_AutoGenPSK;

// CAAAB parameter
bool cbtSameSsid;
bool cbtWifiEnable;
const char* cbtWifiSsid;
const char* cbt5GWifiSsid;
bool cbtGuestWifiConfig;
bool cbtWifiConfig;
unsigned int cbtGuestCountdownTime = 0;
unsigned int cbtGuest5GCountdownTime = 0;
bool cbtSameSsidChange;
const char* cbtWifiPskValue;
const char* cbt5GWifiPskValue;
bool isExtraWiFi = false;
bool cbtWifiIntraBssBlock;
bool cbtWifi5GIntraBssBlock;
bool cbtWifiHideSsid;
bool cbtWifi5GHideSsid;

time_t t1, t2;

dal_param_t WIFI_CAAAB_param[]={
	// APSec
	{"wpaMode",		dalType_string,	0,	0,	NULL},
	// CAAAB parameter
	{"cbtGuestSsidSame",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuestEnable",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuestWifiSsid",		dalType_string, 0,	0,	NULL,		NULL,			0},
	{"cbtGuest5GWifiSsid",		dalType_string, 0,	0,	NULL,		NULL,			0},
	{"cbtGuestWifi",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtWifi",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuestWifiTimes",		dalType_int,	0,	0,	NULL,		NULL,			0},
	{"cbtGuestPskValue",		dalType_string,	0,	0,	NULL,		NULL,			0},
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
	// APSec
	{"wpaMode_5G",		dalType_string,	0,	0,	NULL},
	// CAAAB parameter
	{"cbt5GPskValue",		dalType_string,	0,	0,	NULL,		NULL,			0},
	{"cbtIntraBssBlock",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbt5GIntraBssBlock",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuestSsidHide",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuest5GSsidHide",		dalType_boolean,	0,	0,	NULL,		NULL,			0},
	{"cbtGuest5GWifiTimes",		dalType_int,	0,	0,	NULL,		NULL,			0},
#endif
	{NULL,		0,	0,	0,	NULL},
};
void initCbtWifiObjects(){
	
	cbtSsidObj = NULL;
	cbt5GSsidObj = NULL;
	cbt5GGuestSsidObj = NULL;
	cbtSecObj = NULL;
	cbt5GSecObj = NULL;
	cbtDevInfoObj = NULL;
	cbtSecTmpObj = NULL;
	cbt5GSecTmpObj = NULL;
	cbtApObj = NULL;
	cbt5GApObj = NULL;
	
	IID_INIT(cbtSsidIid);
	IID_INIT(cbt5GSsidIid);
	IID_INIT(cbt5GGuestSsidIid);
	IID_INIT(cbtSecIid);
	IID_INIT(cbt5GSecIid);
	IID_INIT(cbtDevInfoIid);
	IID_INIT(cbtSecTmpIid);
	IID_INIT(cbt5GSecTmpIid);
	IID_INIT(cbtApIid);
	IID_INIT(cbt5GApIid);
}

zcfgRet_t freeAllCbtWifiObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(cbtSsidObj) json_object_put(cbtSsidObj);
	if(cbt5GSsidObj) json_object_put(cbt5GSsidObj);
	if(cbt5GGuestSsidObj) json_object_put(cbt5GGuestSsidObj);
	if(cbtSecObj) json_object_put(cbtSecObj);
	if(cbt5GSecObj) json_object_put(cbt5GSecObj);
	if(cbtApObj) json_object_put(cbtApObj);
	if(cbt5GApObj) json_object_put(cbt5GApObj);
	if(cbtDevInfoObj) json_object_put(cbtDevInfoObj);
	if(cbtSecTmpObj) json_object_put(cbtSecTmpObj);
	if(cbt5GSecTmpObj) json_object_put(cbt5GSecTmpObj);

	return ret;
}

void getCbtBasicWifiInfo(struct json_object *Jobj){
	
	cbtGuestWifiConfig = json_object_get_boolean(json_object_object_get(Jobj, "cbtGuestWifi"));
	cbtWifiConfig = json_object_get_boolean(json_object_object_get(Jobj, "cbtWifi"));
	cbtWifiEnable = json_object_get_boolean(json_object_object_get(Jobj, "cbtGuestEnable")); // Enable/Disable
	cbtSameSsid = json_object_get_boolean(json_object_object_get(Jobj, "cbtGuestSsidSame")); // 2.4G/5G the same SSID
	cbtWifiSsid = json_object_get_string(json_object_object_get(Jobj, "cbtGuestWifiSsid")); // 2.4G SSID
	cbt5GWifiSsid = json_object_get_string(json_object_object_get(Jobj, "cbtGuest5GWifiSsid")); // 5G SSID
	cbtWifiPskValue = json_object_get_string(json_object_object_get(Jobj, "cbtGuestPskValue")); // 2.4G/5G password
	cbtGuestCountdownTime = json_object_get_int(json_object_object_get(Jobj, "cbtGuestWifiTimes")); // countdown Time
	cbtWpaMode = json_object_get_string(json_object_object_get(Jobj, "wpaMode"));
	isExtraWiFi = json_object_get_boolean(json_object_object_get(Jobj, "isExtraWiFiSetting"));
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
	cbt5GWpaMode = json_object_get_string(json_object_object_get(Jobj, "wpaMode_5G"));
	cbt5GWifiPskValue = json_object_get_string(json_object_object_get(Jobj, "cbt5GPskValue"));
	cbtWifiIntraBssBlock = json_object_get_boolean(json_object_object_get(Jobj, "cbtIntraBssBlock"));		// Enable/Disable
	cbtWifi5GIntraBssBlock = json_object_get_boolean(json_object_object_get(Jobj, "cbt5GIntraBssBlock"));	// Enable/Disable
	cbtWifiHideSsid = json_object_get_boolean(json_object_object_get(Jobj, "cbtGuestSsidHide"));		// Enable/Disable
	cbtWifi5GHideSsid = json_object_get_boolean(json_object_object_get(Jobj, "cbtGuest5GSsidHide"));	// Enable/Disable
	if(!cbtWifiConfig && !isExtraWiFi && !cbtSameSsid){
		cbtGuest5GCountdownTime = json_object_get_int(json_object_object_get(Jobj, "cbtGuest5GWifiTimes")); // countdown Time
	}
#endif
	
	return;
}

void cbtStrtolower(char *str, char *buf, size_t size)
{

	int n;

	for(n = 0; n < size && str[n] != '\0'; n++) {
		buf[n] = tolower(str[n]);
	}
	buf[n] = '\0';

	return;

}

// RDM_OID_WIFI_SSID
zcfgRet_t editCbtWifiSsidObjects(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx =0;
	const char *serialNum;
	char tempSerial[6] = {0};
	char temp5GSsid[32] = {0};
	char temp5GGuestSsid[32] = {0};
	char originalSsid[32] = {0};
#if defined (ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE) && defined (ZYXEL_ONESSID)
		objIndex_t oneSsidObjIid;
		rdm_WifiOneSsid_t *oneSsidObj = NULL;
#endif
	if(cbtGuestWifiConfig == true){ // CAAAB WiFi or CAAAB Guest WiFi
		
		if(isExtraWiFi){
			// 2.4G Iid
			iidArray = json_object_object_get(Jobj, "cbtGuestWifiIid");
			
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtSsidIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
				if(cbtSsidIid.idx[iidIdx] != 0)
					cbtSsidIid.level = iidIdx+1;
			}
			
			// 2.4G get data and set value to object
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &cbtSsidIid, &cbtSsidObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			json_object_object_add(cbtSsidObj, "Enable", json_object_new_boolean(cbtWifiEnable));
			json_object_object_add(cbtSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
			json_object_object_add(cbtSsidObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifiHideSsid));
			
		}else{
			
			// 2.4G Iid
			iidArray = json_object_object_get(Jobj, "cbtGuestWifiIid");
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtSsidIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
				if(cbtSsidIid.idx[iidIdx] != 0)
					cbtSsidIid.level = iidIdx+1;
			}
			// 5G Iid
			iidArray = json_object_object_get(Jobj, "cbtGuest5GWifiIid");
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbt5GSsidIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
				if(cbt5GSsidIid.idx[iidIdx] != 0)
					cbt5GSsidIid.level = iidIdx+1;
			}

			// 2.4G get data and set value to object
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &cbtSsidIid, &cbtSsidObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			json_object_object_add(cbtSsidObj, "Enable", json_object_new_boolean(cbtWifiEnable));
			json_object_object_add(cbtSsidObj, "X_ZYXEL_GuestEnableTime", json_object_new_int(cbtGuestCountdownTime));

			// 5G get data and set value to object
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &cbt5GSsidIid, &cbt5GSsidObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			json_object_object_add(cbt5GSsidObj, "Enable", json_object_new_boolean(cbtWifiEnable));
			
			cbt5GGuestSsidIid.level = 1;
			cbt5GGuestSsidIid.idx[0] = 6;
			// 5G guest et data and set value to object
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &cbt5GGuestSsidIid, &cbt5GGuestSsidObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			// The same SSID or not
			if(cbtSameSsid){ // 2.4G / 5G using the same SSID base on 2.4G SSID
				json_object_object_add(cbtSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
				json_object_object_add(cbt5GSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
#if defined (ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE) && defined (ZYXEL_ONESSID)
				IID_INIT(oneSsidObjIid);
        		if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &oneSsidObjIid, &oneSsidObj) == ZCFG_SUCCESS){
        			json_object_object_add(oneSsidObj, "Enable", json_object_new_boolean(true));
       			}
       			if(oneSsidObj)
       				zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ONE_SSID, &oneSsidObjIid, oneSsidObj, NULL);
       			zcfgFeJsonObjFree(oneSsidObj);
       			json_object_object_add(cbt5GSsidObj, "X_ZYXEL_GuestEnableTime", json_object_new_int(cbtGuestCountdownTime));
#endif
			}else{ // 2.4G / 5G using different SSID
#if defined (ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE) && defined (ZYXEL_ONESSID)
				IID_INIT(oneSsidObjIid);
        		if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ONE_SSID, &oneSsidObjIid, &oneSsidObj) == ZCFG_SUCCESS){
        			json_object_object_add(oneSsidObj, "Enable", json_object_new_boolean(false));
       			}
       			if(oneSsidObj)
       				zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ONE_SSID, &oneSsidObjIid, oneSsidObj, NULL);
       			zcfgFeJsonObjFree(oneSsidObj);
       			json_object_object_add(cbt5GSsidObj, "X_ZYXEL_GuestEnableTime", json_object_new_int(cbtGuest5GCountdownTime));
#endif
				if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_INFO, &cbtDevInfoIid, &cbtDevInfoObj)) != ZCFG_SUCCESS) {
					return ret;
				}

				serialNum = json_object_get_string(json_object_object_get(cbtDevInfoObj, "SerialNumber"));

				strncpy(tempSerial, serialNum + 8, 5);
				if(cbtWifiConfig){ // WiFi
					ZOS_SPRINTF_M(originalSsid, "Fioptics%s", tempSerial);

					if(!strcmp(cbtWifiSsid, originalSsid)) // default SSID
						ZOS_SPRINTF_M(temp5GSsid, "Fioptics%s-5G", tempSerial);
					else{ // user define SSID
#ifdef CAAAB_CUSTOMIZATION
						ZOS_SPRINTF_M(temp5GSsid, "%s-5G", cbtWifiSsid);
#else
						const char* wifi_5g_guest_ssid;
						ZOS_SPRINTF_M(temp5GSsid, "%s_5G", cbtWifiSsid);
						wifi_5g_guest_ssid = json_object_get_string(json_object_object_get(cbt5GGuestSsidObj,"SSID"));
						ZOS_SPRINTF_M(temp5GGuestSsid, "%s_5G", wifi_5g_guest_ssid);
#endif
					}
				}else{ // Guest WiFi
					ZOS_SPRINTF_M(originalSsid, "Fioptics%s-Guest", tempSerial);

					if(!strcmp(cbtWifiSsid, originalSsid)) // default SSID
						ZOS_SPRINTF_M(temp5GSsid, "Fioptics%s-Guest-5G", tempSerial);
					else // user default SSID
#ifdef CAAAB_CUSTOMIZATION
						ZOS_SPRINTF_M(temp5GSsid, "%s-5G", cbtWifiSsid);
#else
						ZOS_SPRINTF_M(temp5GSsid, "%s_5G", cbtWifiSsid);
#endif
				}
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
				const char* wifi_24g_ssid;
				const char* wifi_5g_ssid;
				wifi_24g_ssid = json_object_get_string(json_object_object_get(cbtSsidObj,"SSID"));
				wifi_5g_ssid = json_object_get_string(json_object_object_get(cbt5GSsidObj,"SSID"));
				if(json_object_get_boolean(json_object_object_get(cbtSsidObj, "X_ZYXEL_MasterMain")) && cbtWifiConfig){
					// compare with last set 2.4G SSID
					json_object_object_add(cbtSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
					if(!strcmp(wifi_5g_ssid, cbt5GWifiSsid)) // compare with last set 5G SSID
						json_object_object_add(cbt5GSsidObj, "SSID", json_object_new_string(temp5GSsid));
					else
						json_object_object_add(cbt5GSsidObj, "SSID", json_object_new_string(cbt5GWifiSsid));
					json_object_object_add(cbt5GGuestSsidObj, "SSID", json_object_new_string(temp5GGuestSsid));
				}
				else{
					json_object_object_add(cbtSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
					json_object_object_add(cbt5GSsidObj, "SSID", json_object_new_string(cbt5GWifiSsid));
				}
#else
				json_object_object_add(cbtSsidObj, "SSID", json_object_new_string(cbtWifiSsid));
				json_object_object_add(cbt5GSsidObj, "SSID", json_object_new_string(temp5GSsid));
#endif
			}

			// TR-181 parameter
			if(cbtWifiConfig){ // WiFi, X_ZYXEL_MasterMain

				cbtSameSsidChange = json_object_get_boolean(json_object_object_get(cbtSsidObj, "X_ZYXEL_MasterMain"));
				if(cbtSameSsidChange == cbtSameSsid)
					cbtSameSsidChange = false;
				else
					cbtSameSsidChange = true;

				// X_ZYXEL_MasterMain
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterMain", json_object_new_boolean(cbtSameSsid));
				json_object_object_add(cbt5GSsidObj, "X_ZYXEL_MasterMain", json_object_new_boolean(cbtSameSsid));

				// X_ZYXEL_MasterMainSSID, X_ZYXEL_MasterMainPassword
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterMainSSID", json_object_new_string(cbtWifiSsid));
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterMainPassword", json_object_new_string(cbtWifiPskValue));
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
				//5G
				json_object_object_add(cbt5GSsidObj, "X_ZYXEL_MasterMainPassword", json_object_new_string(cbt5GWifiPskValue));
				if(cbtSameSsid)
					json_object_object_add(cbt5GSsidObj, "X_ZYXEL_MasterMainSSID", json_object_new_string(cbtWifiSsid));
				else
					json_object_object_add(cbt5GSsidObj, "X_ZYXEL_MasterMainSSID", json_object_new_string(cbt5GWifiSsid));
					
				
#endif
			}else{ // Guest WiFi, X_ZYXEL_MasterGuest

				cbtSameSsidChange = json_object_get_boolean(json_object_object_get(cbtSsidObj, "X_ZYXEL_MasterGuest"));
				if(cbtSameSsidChange == cbtSameSsid)
					cbtSameSsidChange = false;
				else
					cbtSameSsidChange = true;

				// X_ZYXEL_MasterGuest
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterGuest", json_object_new_boolean(cbtSameSsid));
				json_object_object_add(cbt5GSsidObj, "X_ZYXEL_MasterGuest", json_object_new_boolean(cbtSameSsid));
				// X_ZYXEL_MasterGuestSSID, X_ZYXEL_MasterGuestPassword
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterGuestSSID", json_object_new_string(cbtWifiSsid));
				json_object_object_add(cbtSsidObj, "X_ZYXEL_MasterGuestPassword", json_object_new_string(cbtWifiPskValue));
			}
		
		}

	}

	return ret;
}

// RDM_OID_WIFI_ACCESS_POINT_SEC
zcfgRet_t editCbtWifiSecObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx =0;
	char *psk_lowercase = NULL;
	struct json_object *guiCustObj = NULL;
	objIndex_t guiObjIid;
	int cbtGuestWifiNumber = 0;

	if(cbtGuestWifiConfig == true){ // CAAAB Wi-Fi
		if(isExtraWiFi){
			iidArray = json_object_object_get(Jobj, "cbtGuestWifiIid"); // 2.4G Iid
			
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtSecIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbtSecIid.idx[iidIdx] != 0)
					cbtSecIid.level = iidIdx+1;
			}
			
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtApIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbtApIid.idx[iidIdx] != 0)
					cbtApIid.level = iidIdx+1;
			}
			
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbtSecIid, &cbtSecObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &cbtApIid, &cbtApObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			
			if(!strcmp(cbtWpaMode,"wpa2psk") || !strcmp(cbtWpaMode,"wpapsk") || !strcmp(cbtWpaMode,"wpawpa2psk")){
				if(!strcmp(cbtWpaMode,"wpa2psk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
				}
				else if(!strcmp(cbtWpaMode,"wpapsk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
				}
				else{
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
				}
				
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));

				CAAAB_X_ZYXEL_Auto_PSK = json_object_get_string(json_object_object_get(cbtSecObj, "X_ZYXEL_Auto_PSK"));
				if(strcmp(CAAAB_X_ZYXEL_Auto_PSK, cbtWifiPskValue) == 0){ // The same string means using auto generate psk
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = true;
					// 2.4G KeyPassphrase and PreSharedKey
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
					json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(CAAAB_X_ZYXEL_Auto_PSK));
					json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(""));
				}else{
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = false;
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
					if(strlen(cbtWifiPskValue) == 64) {
						psk_lowercase = (char*)malloc(strlen(cbtWifiPskValue)+1);
						memset(psk_lowercase, 0, strlen(cbtWifiPskValue)+1);
						cbtStrtolower(cbtWifiPskValue, psk_lowercase, strlen(cbtWifiPskValue));
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(""));
						json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						free(psk_lowercase);
					}
					else {
						// 2.4G PSK
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
					}
				}
					
			}else{
				json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("None"));
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
			}
			json_object_object_add(cbtApObj, "IsolationEnable", json_object_new_boolean(cbtWifiIntraBssBlock));
			json_object_object_add(cbtApObj, "X_ZYXEL_BssIdIsolation", json_object_new_boolean(cbtWifiIntraBssBlock));
			json_object_object_add(cbtApObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifiHideSsid));
		}else{
			iidArray = json_object_object_get(Jobj, "cbtGuestWifiIid"); // 2.4G Iid
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtSecIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbtSecIid.idx[iidIdx] != 0)
					cbtSecIid.level = iidIdx+1;
			}

			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbtApIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbtApIid.idx[iidIdx] != 0)
					cbtApIid.level = iidIdx+1;
			}

			iidArray = json_object_object_get(Jobj, "cbtGuest5GWifiIid"); // 5G Iid
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbt5GSecIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbt5GSecIid.idx[iidIdx] != 0)
					cbt5GSecIid.level = iidIdx+1;
			}
		
			for(iidIdx=0;iidIdx<6;iidIdx++){
				cbt5GApIid.idx[iidIdx] = json_object_get_int(json_object_array_get_idx(iidArray, iidIdx));
				if(cbt5GApIid.idx[iidIdx] != 0)
					cbt5GApIid.level = iidIdx+1;
			}
		
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbtSecIid, &cbtSecObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbt5GSecIid, &cbt5GSecObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &cbtApIid, &cbtApObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &cbt5GApIid, &cbt5GApObj)) != ZCFG_SUCCESS) {
				return ret;
			}
			
			//2.4G 5G Guest Iid when apply by main page
			if(cbtWifiConfig){
				IID_INIT(guiObjIid);
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &guiObjIid, &guiCustObj) == ZCFG_SUCCESS){
					cbtGuestWifiNumber = json_object_get_int(json_object_object_get(guiCustObj, "CbtGuestWifi"));
					free(guiCustObj);
				}else{
					zcfgLog(ZCFG_LOG_ERR, "%s: fail to get RDM_OID_GUI_CUSTOMIZATION\n", __FUNCTION__);
					cbtGuestWifiNumber = 1; // set 1 when get fail
				}

				cbtSecTmpIid.idx[0] = cbtGuestWifiNumber + 1;
				cbt5GSecTmpIid.idx[0] = cbtGuestWifiNumber + 5;

				for(iidIdx = 1; iidIdx < 6; iidIdx++){
					cbtSecTmpIid.idx[iidIdx] = cbtSecIid.idx[iidIdx];
					cbt5GSecTmpIid.idx[iidIdx] = cbt5GSecIid.idx[iidIdx];
				}
				cbtSecTmpIid.level = cbtSecIid.level;
				cbt5GSecTmpIid.level = cbt5GSecIid.level;
				if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbtSecTmpIid, &cbtSecTmpObj)) != ZCFG_SUCCESS)
					return ret;

				if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbt5GSecTmpIid, &cbt5GSecTmpObj)) != ZCFG_SUCCESS)
					return ret;
			}

#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
			//2.4G 
			if(!strcmp(cbtWpaMode,"wpa2psk") || !strcmp(cbtWpaMode,"wpapsk") || !strcmp(cbtWpaMode,"wpawpa2psk")){
				
				if(!strcmp(cbtWpaMode,"wpa2psk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
				}
				else if(!strcmp(cbtWpaMode,"wpapsk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
				}
				else{
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
				}

				// CAAAB guest WiFi security mode is the same with WiFi 2.4G security mode
				if(cbtWifiConfig){
					if(!strcmp(cbtWpaMode,"wpa2psk")){
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
					}
					else if(!strcmp(cbtWpaMode,"wpapsk")){
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
					}
					else{
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
					}
					json_object_object_add(cbtSecTmpObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));
					json_object_object_add(cbt5GSecTmpObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));

				}
				//WiFi PSK 
				CAAAB_X_ZYXEL_Auto_PSK = json_object_get_string(json_object_object_get(cbtSecObj, "X_ZYXEL_Auto_PSK"));
				if(strcmp(CAAAB_X_ZYXEL_Auto_PSK, cbtWifiPskValue) == 0) // 2.4G : The same string means using auto generate psk
				{
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = true;
					// 2.4G KeyPassphrase and PreSharedKey
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
					json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(CAAAB_X_ZYXEL_Auto_PSK));
					json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(""));
				}else {
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = false;
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));

					if(strlen(cbtWifiPskValue) == 64) {
						psk_lowercase = (char*)malloc(strlen(cbtWifiPskValue)+1);
						memset(psk_lowercase, 0, strlen(cbtWifiPskValue)+1);
						cbtStrtolower(cbtWifiPskValue, psk_lowercase, strlen(cbtWifiPskValue));
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(""));
						json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
						free(psk_lowercase);
					}else{
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
					}
				}
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));
			}else{//None Secrity mode
				json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("None"));
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
				//Guest Security Type must follow 2.4G main Security type
				if(cbtWifiConfig){
					json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("None"));
					json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("None"));
					json_object_object_add(cbtSecTmpObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
					json_object_object_add(cbt5GSecTmpObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));

				}

			}
			//5G security type
			if(!strcmp(cbt5GWpaMode,"wpa2psk") || !strcmp(cbt5GWpaMode,"wpapsk") || !strcmp(cbt5GWpaMode,"wpawpa2psk")){
				
				if(!strcmp(cbt5GWpaMode,"wpa2psk")){
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
				}
				else if(!strcmp(cbt5GWpaMode,"wpapsk")){
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
				}
				else{
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
				}

				//WiFi PSK 
				CAAAB_5G_X_ZYXEL_Auto_PSK = json_object_get_string(json_object_object_get(cbt5GSecObj, "X_ZYXEL_Auto_PSK"));
				if(strcmp(CAAAB_5G_X_ZYXEL_Auto_PSK, cbt5GWifiPskValue) == 0) // The same string means using auto generate psk
				{
					// X_ZYXEL_AutoGenPSK
					CAAAB_5G_X_ZYXEL_AutoGenPSK = true;
					// 5G KeyPassphrase and PreSharedKey
					json_object_object_add(cbt5GSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_5G_X_ZYXEL_AutoGenPSK));
					json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(CAAAB_5G_X_ZYXEL_Auto_PSK));
					json_object_object_add(cbt5GSecObj, "PreSharedKey", json_object_new_string(""));
				}else {
					// X_ZYXEL_AutoGenPSK
					CAAAB_5G_X_ZYXEL_AutoGenPSK = false;
					json_object_object_add(cbt5GSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_5G_X_ZYXEL_AutoGenPSK));

					if(strlen(cbt5GWifiPskValue) == 64) {
						psk_lowercase = (char*)malloc(strlen(cbt5GWifiPskValue)+1);
						memset(psk_lowercase, 0, strlen(cbt5GWifiPskValue)+1);
						cbtStrtolower(cbt5GWifiPskValue, psk_lowercase, strlen(cbt5GWifiPskValue));
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(""));
						json_object_object_add(cbt5GSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						free(psk_lowercase);
					}else {
						// 5G PSK
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(cbt5GWifiPskValue));
					}
				}
				json_object_object_add(cbt5GSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));
			}else{//None Secrity mode
				json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("None"));
				json_object_object_add(cbt5GSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
			}
			//When enable one SSID, use 2.4G password to instead 5G password
			if(cbtSameSsid){
				json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
				json_object_object_add(cbtApObj, "IsolationEnable", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbtApObj, "X_ZYXEL_BssIdIsolation", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbt5GApObj, "IsolationEnable", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbt5GApObj, "X_ZYXEL_BssIdIsolation", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbtApObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifiHideSsid));
				json_object_object_add(cbt5GApObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifiHideSsid));
			}else{
				json_object_object_add(cbtApObj, "IsolationEnable", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbtApObj, "X_ZYXEL_BssIdIsolation", json_object_new_boolean(cbtWifiIntraBssBlock));
				json_object_object_add(cbt5GApObj, "IsolationEnable", json_object_new_boolean(cbtWifi5GIntraBssBlock));
				json_object_object_add(cbt5GApObj, "X_ZYXEL_BssIdIsolation", json_object_new_boolean(cbtWifi5GIntraBssBlock));
				json_object_object_add(cbtApObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifiHideSsid));
				json_object_object_add(cbt5GApObj, "SSIDAdvertisementEnabled", json_object_new_boolean(!cbtWifi5GHideSsid));
			}
#else
			if(!strcmp(cbtWpaMode,"wpa2psk") || !strcmp(cbtWpaMode,"wpapsk") || !strcmp(cbtWpaMode,"wpawpa2psk")){
				
				if(!strcmp(cbtWpaMode,"wpa2psk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
				}
				else if(!strcmp(cbtWpaMode,"wpapsk")){
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
				}
				else{
					json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
					json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
				}

				// CAAAB guest WiFi security mode is the same with WiFi security mode
				if(cbtWifiConfig){ 

					if(!strcmp(cbtWpaMode,"wpa2psk")){
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA2-Personal"));
					}
					else if(!strcmp(cbtWpaMode,"wpapsk")){
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA-Personal"));
					}
					else{
						json_object_object_add(cbtSecTmpObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
						json_object_object_add(cbt5GSecTmpObj, "ModeEnabled", json_object_new_string("WPA-WPA2-Personal"));
						zcfgLog(ZCFG_LOG_ERR, "%s: fail to set Guest WiFi Security type\n", __FUNCTION__);
					}

				}
				//WiFi PSK 
				CAAAB_X_ZYXEL_Auto_PSK = json_object_get_string(json_object_object_get(cbtSecObj, "X_ZYXEL_Auto_PSK"));
				if(strcmp(CAAAB_X_ZYXEL_Auto_PSK, cbtWifiPskValue) == 0) // The same string means using auto generate psk
				{
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = true;

					// 2.4G KeyPassphrase and PreSharedKey
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
					json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(CAAAB_X_ZYXEL_Auto_PSK));
					json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(""));

					// 5G KeyPassphrase and PreSharedKey
					//if(cbtSameSsid == true){
						json_object_object_add(cbt5GSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(CAAAB_X_ZYXEL_Auto_PSK));
						json_object_object_add(cbt5GSecObj, "PreSharedKey", json_object_new_string(""));
					/*}else{ // 20170720 Max Noted: remove CAAAB 5G wifi password add "-5G" at the end of 2.4G password
						CAAAB_X_ZYXEL_AutoGenPSK = false;

						json_object_object_add(cbt5GSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
						ZOS_SPRINTF_M(tmpPsk, "%s%s", CAAAB_X_ZYXEL_Auto_PSK, "-5G");
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(tmpPsk));
					}*/
				}else {
					// X_ZYXEL_AutoGenPSK
					CAAAB_X_ZYXEL_AutoGenPSK = false;
					json_object_object_add(cbtSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));
					json_object_object_add(cbt5GSecObj, "X_ZYXEL_AutoGenPSK", json_object_new_boolean(CAAAB_X_ZYXEL_AutoGenPSK));

					if(strlen(cbtWifiPskValue) == 64) {
						psk_lowercase = (char*)malloc(strlen(cbtWifiPskValue)+1);
						memset(psk_lowercase, 0, strlen(cbtWifiPskValue)+1);
						cbtStrtolower(cbtWifiPskValue, psk_lowercase, strlen(cbtWifiPskValue));
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(""));
						json_object_object_add(cbtSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(""));
						json_object_object_add(cbt5GSecObj, "PreSharedKey", json_object_new_string(psk_lowercase));
						free(psk_lowercase);
					}else {
						// 2.4G PSK
						json_object_object_add(cbtSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
							
						// 5G PSK
						//if(cbtSameSsid == true){
						json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(cbtWifiPskValue));
						/*}else{ // 20170720 Max Noted: remove CAAAB 5G wifi password add "-5G" at the end of 2.4G password
							ZOS_SPRINTF_M(tmpPsk, "%s%s", cbtWifiPskValue, "-5G");
							json_object_object_add(cbt5GSecObj, "KeyPassphrase", json_object_new_string(tmpPsk));
						}*/
					}
				}
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));
				json_object_object_add(cbt5GSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("WEPEncryption"));
			}else{//None Secrity mode
				json_object_object_add(cbtSecObj, "ModeEnabled", json_object_new_string("None"));
				json_object_object_add(cbt5GSecObj, "ModeEnabled", json_object_new_string("None"));
				json_object_object_add(cbtSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
				json_object_object_add(cbt5GSecObj, "X_ZYXEL_WEPEncryption", json_object_new_string("None"));
			}
#endif
		}
	}
	
	return ret;
}

zcfgRet_t setAllCbtWifiObjects(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;

	if(cbtWifiConfig == true){ // Guest WiFi follow WiFi security mode
		if(cbtSecTmpObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbtSecTmpIid, cbtSecTmpObj, NULL);
		if(cbt5GSecTmpObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbt5GSecTmpIid, cbt5GSecTmpObj, NULL);
	}

	// 2.4G and 5G APSec
	if(cbtSecObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbtSecIid, cbtSecObj, NULL);
	if(cbt5GSecObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &cbt5GSecIid, cbt5GSecObj, NULL);

	if(cbtApObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &cbtApIid, cbtApObj, NULL);
	if(cbt5GApObj) zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &cbt5GApIid, cbt5GApObj, NULL);

	// 2.4G SSID
	if(cbtSsidObj) zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &cbtSsidIid, cbtSsidObj, NULL);
	// 5G SSID
	if(cbt5GSsidObj) zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &cbt5GSsidIid, cbt5GSsidObj, NULL);
	if(cbt5GGuestSsidObj && cbtWifiConfig) zcfgFeObjJsonSet(RDM_OID_WIFI_SSID, &cbt5GGuestSsidIid, cbt5GGuestSsidObj, NULL);


	return ret;
}

zcfgRet_t zcfgFeDalCbtWifiEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	initCbtWifiObjects();
	getCbtBasicWifiInfo(Jobj);

	t2 = time(NULL);
	printf("line=%d, time=%d\n", __LINE__, (int) difftime(t2, t1));

	if((ret = editCbtWifiSsidObjects(Jobj)) != ZCFG_SUCCESS)
		goto dalwlan_edit_fail;
	
	if((ret = editCbtWifiSecObject(Jobj)) != ZCFG_SUCCESS)
		goto dalwlan_edit_fail;

	t2 = time(NULL);
	printf("before set, time=%d seconds\n", (int) difftime(t2, t1));
	
	setAllCbtWifiObjects(Jobj);
	
	sleep(5); //CAAAB case , the WiFi 2.4 and 5G setting at same time. The delay 5sec for next get.

dalwlan_edit_fail:
	freeAllCbtWifiObjects();
	
	return ret;
}


void cbtDalWlanGetMainSsidIndex(int *main2gFound, int *main5gFound)
{
	char *str;
	struct json_object *obj = NULL;
	objIndex_t iid;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &iid, &obj) == ZCFG_SUCCESS) {
		if (json_object_get_int(json_object_object_get(obj, "X_ZYXEL_MainSSID")) == true) {
			str = (char*)json_object_get_string(json_object_object_get(obj, "LowerLayers"));
			if (strncmp(str, "WiFi.Radio.1", strlen("WiFi.Radio.1")) == 0) {
				*main2gFound = iid.idx[0];
			}
			else if (strncmp(str, "WiFi.Radio.2", strlen("WiFi.Radio.2")) == 0) {
				*main5gFound = iid.idx[0];
			}
		}
		json_object_put(obj);
		obj = NULL;
	}
	return;
}

zcfgRet_t zcfgFeDalCbtWifi(const char *method, struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	t1 = time(NULL);
	
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDalCbtWifiEdit(Jobj, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	t2 = time(NULL);
	printf("zcfgFeDalCbtWifi() Total time=%d seconds ret=%d\n", (int) difftime(t2, t1), ret);

	return ret;
}
#endif
