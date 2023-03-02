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

dal_param_t DEFAULT_PSK_param[] = {
	{ NULL,	0,	0,	0,	NULL}
};


zcfgRet_t zcfgFeDalDefaultPskGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *paramJobj = NULL;
	struct json_object *wifiObj = NULL;	
	char defPsk[14] = {0};

	
	IID_INIT(iid);
	iid.level = 1; 
	iid.idx[0] = 1;

	if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &iid, &wifiObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	char *tempPsk = (char *)json_object_get_string(json_object_object_get(wifiObj, "X_ZYXEL_Auto_PSK"));
	ZOS_STRNCPY(defPsk, tempPsk, sizeof(defPsk)); 	//sizeof(defPsk) should be 13+1

	paramJobj = json_object_new_object();
	json_object_object_add(paramJobj, "DefaultPSK", json_object_new_string(defPsk));                               
	json_object_array_add(Jarray, paramJobj);

	return ret;
}


void zcfgFeDalShowDefaultPsk(struct json_object *Jarray) {
	struct json_object *obj = NULL;

	if (json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	printf("%-20s %-20s \n","DefaultPSK:", json_object_get_string(json_object_object_get(obj, "DefaultPSK")));	
}


zcfgRet_t zcfgFeDalDefaultPsk(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if (!strcmp(method, "GET"))
		ret = zcfgFeDalDefaultPskGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
