
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

#ifdef ZYXEL_WIFI_SUPPORT_WDS
dal_param_t WIFI_WDS_param[] =
{
	{"Index",			dalType_int,			1,	8,	NULL,	NULL,	dalcmd_Forbid},
	{"APmode",			dalType_string,		0,	0,	NULL,	"ap|wds",	0},
	{"Bridge_Restrict_Enable",		dalType_boolean,			0,	0,	NULL,	NULL,	0},
	{"MacAddress",		dalType_MacAddrHyphen,	0,	0,	NULL,	NULL,	dal_Add|dal_Delete|dal_Edit_ignore},
	{"WDSMacLists",		dalType_string,			0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{NULL,		0,	0,	0,	NULL}
};

bool WdsStrToJarray(char *list, char *separator, struct json_object *Jarray){
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
bool WdsJarrayToStr(struct json_object *Jarray, char *separator, char *list){
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

void zcfgFeDalShowWifiWds(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;


	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%s\n",json_object_get_string(json_object_object_get(obj, "Index")));
		printf("-----------------------------------\n");
		printf("%-30s : %s\n","CPE mode",json_object_get_string(json_object_object_get(obj, "APmode")));
		if(json_object_get_boolean(json_object_object_get(obj, "Enable"))){
			printf("%-15s ","Enabled");
		}
		else
		{
			printf("%-15s ","Disabled");
		}
		printf("WDS MAC address List: %s\n",json_object_get_string(json_object_object_get(obj, "WDSMacLists")));
		printf("\n");
	}
}

zcfgRet_t zcfgFeDalWifiWdsGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *pramJobj = NULL;
	struct json_object *WifiRadioJobj = NULL;
	int index;
	
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));

	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS)
		return ret;

	
	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "APmode", JSON_OBJ_COPY(json_object_object_get(WifiRadioJobj, "X_ZYXEL_WlMode")));
	json_object_object_add(pramJobj, "Bridge_Restrict_Enable", JSON_OBJ_COPY(json_object_object_get(WifiRadioJobj, "X_ZYXEL_LazyWds")));
	json_object_object_add(pramJobj, "WDSMacLists", JSON_OBJ_COPY(json_object_object_get(WifiRadioJobj, "X_ZYXEL_WdsMacList")));

	json_object_array_add(Jarray, pramJobj);

	if(WifiRadioJobj)
		zcfgFeJsonObjFree(WifiRadioJobj);

	return ret;
}

zcfgRet_t zcfgFeDalWifiWdsEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiRadioJobj = NULL, *arrayobj= NULL;
	int index, i, arrayLen=0;

	
	if(json_object_get_type(Jobj) == json_type_array){
		arrayLen = json_object_array_length(Jobj);
		for(i=0;i<arrayLen;i++){
		
			arrayobj = json_object_array_get_idx(Jobj, i);	
			
			index = json_object_get_int(json_object_object_get(arrayobj, "Index"));	
			
			IID_INIT(objIid);
			objIid.level = 1;
			objIid.idx[0] = index;
			if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
				return ret;
			}

			if(json_object_object_get(arrayobj,"APmode"))
				json_object_object_add(WifiRadioJobj, "X_ZYXEL_WlMode", JSON_OBJ_COPY(json_object_object_get(arrayobj,"APmode")));
			
			if(json_object_object_get(arrayobj,"Bridge_Restrict_Enable"))
				json_object_object_add(WifiRadioJobj,"X_ZYXEL_LazyWds", JSON_OBJ_COPY(json_object_object_get(arrayobj,"Bridge_Restrict_Enable")));
			
			if(json_object_object_get(arrayobj,"WDSMacLists"))
				json_object_object_add(WifiRadioJobj, "X_ZYXEL_WdsMacList",JSON_OBJ_COPY(json_object_object_get(arrayobj,"WDSMacLists")));

			if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &objIid, WifiRadioJobj, NULL)) != ZCFG_SUCCESS);
			
			json_object_put(WifiRadioJobj);
		}
		return ret;
	}	
	else{
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		IID_INIT(objIid);	
		objIid.level = 1;
		objIid.idx[0] = index;
		if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
			return ret;
		}

		if(json_object_object_get(Jobj,"APmode"))
			json_object_object_add(WifiRadioJobj, "X_ZYXEL_WlMode", JSON_OBJ_COPY(json_object_object_get(Jobj,"APmode")));
		
		if(json_object_object_get(Jobj,"Bridge_Restrict_Enable"))
			json_object_object_add(WifiRadioJobj,"X_ZYXEL_LazyWds", JSON_OBJ_COPY(json_object_object_get(Jobj,"Bridge_Restrict_Enable")));
		
		if(json_object_object_get(Jobj,"WDSMacLists"))
			json_object_object_add(WifiRadioJobj, "X_ZYXEL_WdsMacList",JSON_OBJ_COPY(json_object_object_get(Jobj,"WDSMacLists")));

		if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &objIid, WifiRadioJobj, NULL)) != ZCFG_SUCCESS);

		json_object_put(WifiRadioJobj);
		
		return ret;
	}
}

zcfgRet_t zcfgFeDalWifiWdsAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiRadioJobj = NULL, *curObj = NULL;
	int index, rrr, count = 0;
	char *macAddr=NULL;
	char *curlist;
	char newlist[512]={0};
	char *tmp = NULL, *ptr = NULL, *macString = NULL;


	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	macAddr = json_object_get_string(json_object_object_get(Jobj, "MacAddress"));

	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	curObj = json_object_new_array();
	curlist = json_object_get_string(json_object_object_get(WifiRadioJobj, "X_ZYXEL_WdsMacList"));
	rrr = WdsStrToJarray(curlist,",",curObj);
	
	tmp = strdup(curlist);
	macString = strtok_r(tmp, ",", &ptr);
	while(macString != NULL){
		count++;
		macString = strtok_r(NULL, ",", &ptr);
	}
	if(count>3){
		if(replyMsg)
			strcat(replyMsg, "A maximum of 4 WDS mac can be configured.");
		free(tmp);
		json_object_put(WifiRadioJobj);
		json_object_put(curObj);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	if(!findInList(curlist,macAddr,",")){
		json_object_array_add(curObj,json_object_new_string(macAddr));
	}

	rrr = WdsJarrayToStr(curObj,",",newlist);
	if(newlist!=NULL)
		json_object_object_add(WifiRadioJobj, "X_ZYXEL_WdsMacList",json_object_new_string(newlist));

	if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &objIid, WifiRadioJobj, NULL)) != ZCFG_SUCCESS);

	free(tmp);
	json_object_put(WifiRadioJobj);
	json_object_put(curObj);
	return ret;
}

zcfgRet_t zcfgFeDalWifiWdsDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WifiRadioJobj = NULL, *curObj = NULL, *newObj = NULL;
	int index, rrr, i, len;
	char *macAddr=NULL;
	char *curlist;
	char newlist[512]={0};


	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	macAddr = json_object_get_string(json_object_object_get(Jobj, "MacAddress"));

	objIid.level = 1;
	objIid.idx[0] = index;
	if((ret = zcfgFeObjJsonGet(RDM_OID_WIFI_RADIO, &objIid, &WifiRadioJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	curObj = json_object_new_array();
	newObj = json_object_new_array();
	curlist = json_object_get_string(json_object_object_get(WifiRadioJobj, "X_ZYXEL_WdsMacList"));
	rrr = WdsStrToJarray(curlist,",",curObj);
	
	len = json_object_array_length(curObj);
	for(i=0;i<len;i++){
		if(strcmp(macAddr,json_object_get_string(json_object_array_get_idx(curObj,i)))){
			json_object_array_add(newObj,JSON_OBJ_COPY(json_object_array_get_idx(curObj,i)));
		}
	}
	rrr = WdsJarrayToStr(newObj,",",newlist);
	if(newlist!=NULL)
		json_object_object_add(WifiRadioJobj, "X_ZYXEL_WdsMacList",json_object_new_string(newlist));

	if((ret = zcfgFeObjJsonSet(RDM_OID_WIFI_RADIO, &objIid, WifiRadioJobj, NULL)) != ZCFG_SUCCESS);

	json_object_put(WifiRadioJobj);
	json_object_put(curObj);
	json_object_put(newObj);
	return ret;
}

zcfgRet_t zcfgFeDalWifiWds(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWifiWdsEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalWifiWdsAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalWifiWdsDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiWdsGet(Jobj, Jarray, replyMsg);
	}
	else 
		printf("Unknown method:%s\n", method);

	return ret;
}

#endif

