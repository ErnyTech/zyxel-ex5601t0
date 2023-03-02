
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WIFI_MACFILTER_param[] =
{
	{"Index",			dalType_int,			1,	8,	NULL,	NULL,	dalcmd_Forbid},
	{"SSID",			dalType_wifiSSID,		0,	0,	NULL,	NULL,	dal_Add|dal_Delete},
	{"MacAddress",		dalType_MacAddrHyphen,	0,	0,	NULL,	NULL,	dal_Add|dal_Delete|dal_Edit_ignore},
	{"FilterLists",		dalType_string,			0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"MACMode",		dalType_string,			0,	0,	NULL,	"Disable|Deny|Allow",	dal_Edit},
	{"FilterMode",		dalType_string,			0,	0,	NULL,	NULL,	dalcmd_Forbid},	// for webgui
	{NULL,		0,	0,	0,	NULL}
};

bool StrToJarray(char *list, char *separator, struct json_object *Jarray){
	char *buf, *ptr, *tmp_ptr = NULL;
	if(list == NULL || separator == NULL)
		return false;

	buf = strdup(list);
	ptr = strtok_r(buf, separator, &tmp_ptr);
	if(ptr==NULL) {return false;}
	json_object_array_add(Jarray,json_object_new_string(ptr));
	while((ptr = strtok_r(NULL, separator, &tmp_ptr)) != NULL){
		json_object_array_add(Jarray,json_object_new_string(ptr));
	}
	free(buf);
	return true;
}
bool JarrayToStr(struct json_object *Jarray, char *separator, char *list){
	if(Jarray == NULL || separator == NULL)
		return false;

	char tmpStr[512]={0};
	int len,i;
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		strcat(tmpStr,json_object_get_string(json_object_array_get_idx(Jarray,i)));
		strcat(tmpStr,",");
	}
	tmpStr[strlen(tmpStr)-1]='\0';
	strcpy(list,tmpStr);	 
	return true;
}

void zcfgFeDalShowWifiMACFilter(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	int mode;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%s\n",json_object_get_string(json_object_object_get(obj, "ssid")));
		printf("-----------------------------------\n");
		mode = json_object_get_int(json_object_object_get(obj, "mode"));
		if(mode==0)
			printf("MAC Restrict Mode: Disabled\n");
		if(mode==1)
			printf("MAC Restrict Mode: Deny\n");
		if(mode==2)
			printf("MAC Restrict Mode: Allow\n");
		printf("MAC address List: %s\n",json_object_get_string(json_object_object_get(obj, "list")));
		printf("\n");
	}
}

zcfgRet_t zcfgFeDalWifiMACFilterGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_STA_FILTER, &iid, &obj) == ZCFG_SUCCESS) {
		struct json_object *ssidObj = NULL;
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_SSID, &iid, &ssidObj)) != ZCFG_SUCCESS);
		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "ssid", JSON_OBJ_COPY(json_object_object_get(ssidObj, "SSID")));
		json_object_object_add(pramJobj, "mode", JSON_OBJ_COPY(json_object_object_get(obj, "FilterMode")));
		json_object_object_add(pramJobj, "list", JSON_OBJ_COPY(json_object_object_get(obj, "FilterLists")));
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(obj);
		zcfgFeJsonObjFree(ssidObj);
	}
	return ret;
}

extern zcfgRet_t getwifiSSIDObj(struct json_object **ssidObj);	//implement in zcfg_fe_dal.c

zcfgRet_t zcfgFeDalWifiMACFilterEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiMACFilterJobj = NULL, *wifissidObj = NULL, *obj = NULL, *arrayobj= NULL;
	int index, objLen = 0, idx = 0, i, arrayLen=0;
	const char *mode = NULL, *ssid = NULL;

	
if(json_object_get_type(Jobj) == json_type_array){
	arrayLen = json_object_array_length(Jobj);
	for(i=0;i<arrayLen;i++){
		
			arrayobj = json_object_array_get_idx(Jobj, i);	

			if(!json_object_object_get(arrayobj, "Index")){ 
					ssid = json_object_get_string(json_object_object_get(arrayobj, "SSID"));
					getwifiSSIDObj(&wifissidObj);
					objLen = json_object_array_length(wifissidObj);
					for(i=0;i<objLen;i++){
						obj = json_object_array_get_idx(wifissidObj, i);
						if(!strcmp(ssid,json_object_get_string(json_object_object_get(obj,"SSID")))){
							idx = json_object_get_int(json_object_object_get(obj,"idx"));
							json_object_object_add(arrayobj, "Index", json_object_new_int(idx));
							break;
						}
					}
					zcfgFeJsonObjFree(wifissidObj);
				}
			
			
			index = json_object_get_int(json_object_object_get(arrayobj, "Index"));		
			IID_INIT(objIid);
			
			if(json_object_object_get(arrayobj, "MACMode")){
				mode = json_object_get_string(json_object_object_get(arrayobj, "MACMode"));
				if(!strcmp(mode, "Disable")){
					json_object_object_add(arrayobj, "FilterMode", json_object_new_int(0));
					}
				else if(!strcmp(mode, "Deny")){
					json_object_object_add(arrayobj, "FilterMode", json_object_new_int(1));
					}
				else if(!strcmp(mode, "Allow")){
					json_object_object_add(arrayobj, "FilterMode", json_object_new_int(2));
					}
			}
		 
			objIid.level = 1;
			objIid.idx[0] = index;
			if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_STA_FILTER, &objIid, &WifiMACFilterJobj)) != ZCFG_SUCCESS) {
				return ret;
			}
		    
			
			if(json_object_object_get(arrayobj,"FilterLists"))
				json_object_object_add(WifiMACFilterJobj, "FilterLists",JSON_OBJ_COPY(json_object_object_get(arrayobj,"FilterLists")));
			if(json_object_object_get(arrayobj,"FilterMode"))
				json_object_object_add(WifiMACFilterJobj, "FilterMode", JSON_OBJ_COPY(json_object_object_get(arrayobj,"FilterMode")));


			if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_STA_FILTER, &objIid, WifiMACFilterJobj, NULL)) != ZCFG_SUCCESS);
			
			json_object_put(WifiMACFilterJobj);
		}
		return ret;
	}
	
else{	
	if(!json_object_object_get(Jobj, "Index")){
		ssid = json_object_get_string(json_object_object_get(Jobj, "SSID"));
		getwifiSSIDObj(&wifissidObj);
		objLen = json_object_array_length(wifissidObj);
		for(i=0;i<objLen;i++){
			obj = json_object_array_get_idx(wifissidObj, i);
			if(!strcmp(ssid,json_object_get_string(json_object_object_get(obj,"SSID")))){
				idx = json_object_get_int(json_object_object_get(obj,"idx"));
				json_object_object_add(Jobj, "Index", json_object_new_int(idx));
				break;
			}
		}
		zcfgFeJsonObjFree(wifissidObj);
	}
	
	

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	IID_INIT(objIid);

	if(json_object_object_get(Jobj, "MACMode")){
		mode = json_object_get_string(json_object_object_get(Jobj, "MACMode"));
		if(!strcmp(mode, "Disable")){
			json_object_object_add(Jobj, "FilterMode", json_object_new_int(0));
		}
		else if(!strcmp(mode, "Deny")){
			json_object_object_add(Jobj, "FilterMode", json_object_new_int(1));
		}
		else if(!strcmp(mode, "Allow")){
			json_object_object_add(Jobj, "FilterMode", json_object_new_int(2));
		}
	}
		
	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_STA_FILTER, &objIid, &WifiMACFilterJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(json_object_object_get(Jobj,"FilterLists"))
		json_object_object_add(WifiMACFilterJobj, "FilterLists",JSON_OBJ_COPY(json_object_object_get(Jobj,"FilterLists")));
	if(json_object_object_get(Jobj,"FilterMode"))
		json_object_object_add(WifiMACFilterJobj, "FilterMode", JSON_OBJ_COPY(json_object_object_get(Jobj,"FilterMode")));

   
	if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_STA_FILTER, &objIid, WifiMACFilterJobj, NULL)) != ZCFG_SUCCESS);

	json_object_put(WifiMACFilterJobj);

	
	return ret;
   }
}

zcfgRet_t zcfgFeDalWifiMACFilterAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiMACFilterJobj = NULL, *curObj = NULL, *wifissidObj = NULL, *obj = NULL;

	int index, objLen = 0, i, idx = 0, count = 0;
	bool rrr = false;
	char *macAddr=NULL;

	char *curlist;
	char newlist[512]={0};
	const char *ssid = NULL;
	char *tmp = NULL, *ptr = NULL, *macString = NULL;

	if(!json_object_object_get(Jobj, "Index")){
		ssid = json_object_get_string(json_object_object_get(Jobj, "SSID"));
		getwifiSSIDObj(&wifissidObj);
		objLen = json_object_array_length(wifissidObj);
		for(i=0;i<objLen;i++){
			obj = json_object_array_get_idx(wifissidObj, i);
			if(!strcmp(ssid,json_object_get_string(json_object_object_get(obj,"SSID")))){
				idx = json_object_get_int(json_object_object_get(obj,"idx"));
				json_object_object_add(Jobj, "Index", json_object_new_int(idx));
				break;
			}
		}
		zcfgFeJsonObjFree(wifissidObj);
	}

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	macAddr = (char *)json_object_get_string(json_object_object_get(Jobj, "MacAddress"));

	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_STA_FILTER, &objIid, &WifiMACFilterJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	curObj = json_object_new_array();
	curlist = (char *)json_object_get_string(json_object_object_get(WifiMACFilterJobj, "FilterLists"));
	rrr = StrToJarray(curlist,",",curObj);
	if ( rrr == false) {
		printf("%s_%d: input data for StrToJarray is incorrect!\n", __FUNCTION__, __LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	tmp = strdup(curlist);
	macString = strtok_r(tmp, ",", &ptr);
	while(macString != NULL){
		count++;
		macString = strtok_r(NULL, ",", &ptr);
	}
	if(count>24){
		if(replyMsg)
			strcat(replyMsg, "A maximum of 25 MAC Authentication rules can be configured.");
		free(tmp);
		json_object_put(WifiMACFilterJobj);
		json_object_put(curObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	if(!findInList(curlist,macAddr,",")){
		json_object_array_add(curObj,json_object_new_string(macAddr));
	}

	rrr = JarrayToStr(curObj,",",newlist);
	if ( rrr == false) {
		printf("%s_%d: input data for StrToJarray is incorrect!\n", __FUNCTION__, __LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if(newlist!=NULL)
		json_object_object_add(WifiMACFilterJobj, "FilterLists",json_object_new_string(newlist));

	if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_STA_FILTER, &objIid, WifiMACFilterJobj, NULL)) != ZCFG_SUCCESS);

	free(tmp);
	json_object_put(WifiMACFilterJobj);
	json_object_put(curObj);
	return ret;
}

zcfgRet_t zcfgFeDalWifiMACFilterDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiMACFilterJobj = NULL, *curObj = NULL, *newObj = NULL, *wifissidObj = NULL, *obj = NULL;
	int index, i, len, objLen = 0, idx = 0;
	bool rrr = false;
	char *macAddr=NULL;
	char *curlist;
	char newlist[512]={0};
	const char *ssid = NULL;

	if(!json_object_object_get(Jobj, "Index")){
		ssid = json_object_get_string(json_object_object_get(Jobj, "SSID"));
		getwifiSSIDObj(&wifissidObj);
		objLen = json_object_array_length(wifissidObj);
		for(i=0;i<objLen;i++){
			obj = json_object_array_get_idx(wifissidObj, i);
			if(!strcmp(ssid,json_object_get_string(json_object_object_get(obj,"SSID")))){
				idx = json_object_get_int(json_object_object_get(obj,"idx"));
				json_object_object_add(Jobj, "Index", json_object_new_int(idx));
				break;
			}
		}
		zcfgFeJsonObjFree(wifissidObj);
	}

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	macAddr = (char *)json_object_get_string(json_object_object_get(Jobj, "MacAddress"));

	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_STA_FILTER, &objIid, &WifiMACFilterJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	curObj = json_object_new_array();
	newObj = json_object_new_array();
	curlist = (char *)json_object_get_string(json_object_object_get(WifiMACFilterJobj, "FilterLists"));
	rrr = StrToJarray(curlist,",",curObj);
	if ( rrr == false) {
		printf("%s_%d: input data for StrToJarray is incorrect!\n", __FUNCTION__, __LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}

	len = json_object_array_length(curObj);
	for(i=0;i<len;i++){
		if(strcmp(macAddr,json_object_get_string(json_object_array_get_idx(curObj,i)))){
			json_object_array_add(newObj,JSON_OBJ_COPY(json_object_array_get_idx(curObj,i)));
		}
	}
	rrr = JarrayToStr(newObj,",",newlist);
	if ( rrr == false) {
		printf("%s_%d: input data for StrToJarray is incorrect!\n", __FUNCTION__, __LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if(newlist!=NULL)
		json_object_object_add(WifiMACFilterJobj, "FilterLists",json_object_new_string(newlist));

	if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_STA_FILTER, &objIid, WifiMACFilterJobj, NULL)) != ZCFG_SUCCESS);

	json_object_put(WifiMACFilterJobj);
	json_object_put(curObj);
	json_object_put(newObj);
	return ret;
}

zcfgRet_t zcfgFeDalWifiMACFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWifiMACFilterEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalWifiMACFilterAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalWifiMACFilterDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiMACFilterGet(Jobj, Jarray, replyMsg);
	}
	else 
		printf("Unknown method:%s\n", method);

	return ret;
}
