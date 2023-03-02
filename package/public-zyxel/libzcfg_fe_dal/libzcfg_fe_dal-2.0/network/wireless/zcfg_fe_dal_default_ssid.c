#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "libzyutil_wrapper.h"
#include "zos_api.h"

//#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t DEFAULT_SSID_param[]={
{ NULL,	0,	0,	0,	NULL}
};

zcfgRet_t zcfgFeDalDefaultSsidGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *paramJobj = NULL;
	struct json_object *wifissidObj = NULL;
	
	char macAddress[13] = {0};
	char tempSSID[21] = {0};
    char tempMAC[5] = {0};
	int tmpMac;
	
	IID_INIT(iid);
	iid.level=1; 
	iid.idx[0]=1; 

    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &iid, &wifissidObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	char *tempSSIDPrefix = (char *)json_object_get_string(json_object_object_get(wifissidObj, "X_ZYXEL_SSIDPrefix"));
 	
    zyUtilIGetBaseMAC(macAddress);
    ZOS_STRNCPY(tempMAC, macAddress + 8, sizeof(tempMAC));  //sizeof(tempMAC) should be 4+1
	
	tmpMac = (int)strtol(tempMAC, NULL, 16);
	tmpMac = tmpMac + 1;
    sprintf(tempSSID, "%s%X", tempSSIDPrefix, tmpMac);
	
	paramJobj = json_object_new_object();
	json_object_object_add(paramJobj, "DefaultSSID", json_object_new_string(tempSSID));                               
	json_object_array_add(Jarray, paramJobj);

	return ret;
}

void zcfgFeDalShowDefaultSsid(struct json_object *Jarray){
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	printf("%s%s \n","DefaultSSID:", json_object_get_string(json_object_object_get(obj, "DefaultSSID")));
	
}


zcfgRet_t zcfgFeDalDefaultSsid(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "GET"))
		ret = zcfgFeDalDefaultSsidGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

