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

dal_param_t CELLWAN_PLMN_param[]={
{"INTF_Network_Requested", dalType_string, 0, 0, NULL},
{ NULL,	0,	0,	0,	NULL}

};

zcfgRet_t zcfgFeDalCellWanPlmnEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t intfIid = {0};
	struct json_object *intfObj = NULL;
	const char *requestedPlmn = NULL;

	IID_INIT(intfIid);
	intfIid.level=1;
	intfIid.idx[0]=1;

	requestedPlmn = json_object_get_string(json_object_object_get(Jobj, "INTF_Network_Requested"));

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF, &intfIid, &intfObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "INTF_Network_Requested"))
			json_object_object_add(intfObj, "NetworkRequested", json_object_new_string(requestedPlmn));
		zcfgFeObjJsonSet(RDM_OID_CELL_INTF, &intfIid, intfObj, NULL);
	}

	zcfgFeJsonObjFree(intfObj);

	return ret;
}	

zcfgRet_t zcfgFeDalCellWanPlmnGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *intfObj = NULL;
	objIndex_t intfIid = {0};
	paramJobj = json_object_new_object();

	IID_INIT(intfIid);
	intfIid.level=1;
	intfIid.idx[0]=1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF, &intfIid, &intfObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "INTF_Available_Networks", JSON_OBJ_COPY(json_object_object_get(intfObj, "AvailableNetworks")));
		json_object_object_add(paramJobj, "INTF_Network_Requested", JSON_OBJ_COPY(json_object_object_get(intfObj, "NetworkRequested")));
		json_object_object_add(paramJobj, "INTF_Network_In_Use", JSON_OBJ_COPY(json_object_object_get(intfObj, "NetworkInUse")));

		zcfgFeJsonObjFree(intfObj);
	}

	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowCellWanPlmn(struct json_object *Jarray){
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	
	printf("\nCellular Networks:\n");
	if(!strcmp("",json_object_get_string(json_object_object_get(obj, "INTF_Available_Networks"))))
		printf("%-30s %-10s \n","Available Networks:", "N/A");
	else
		printf("%-30s %-10s \n","Available Networks:", json_object_get_string(json_object_object_get(obj, "INTF_Available_Networks")));
	
}


zcfgRet_t zcfgFeDalCellWanPlmn(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanPlmnEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanPlmnGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

