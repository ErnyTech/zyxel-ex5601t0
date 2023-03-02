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

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t ONLINE_FW_param[]={
{ NULL,	0,	0,	0,	NULL}
};

zcfgRet_t zcfgFeDalOnlineFWInfoGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *paramJobj = NULL;
	struct json_object *Obj = NULL;

	paramJobj = json_object_new_object();
	IID_INIT(iid);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_ONLINE_F_W_UPGRADE, &iid, &Obj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "FWVersion", JSON_OBJ_COPY(json_object_object_get(Obj, "FWVersion")));
		json_object_object_add(paramJobj, "FWReleaseDate", JSON_OBJ_COPY(json_object_object_get(Obj, "FWReleaseDate")));
		json_object_object_add(paramJobj, "FWSize", JSON_OBJ_COPY(json_object_object_get(Obj, "FWSize")));
		zcfgFeJsonObjFree(Obj);
	}
	json_object_array_add(Jarray, paramJobj);

	return ret;
}

zcfgRet_t zcfgFeDalOnlineFWInfo(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "GET"))
		ret = zcfgFeDalOnlineFWInfoGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

