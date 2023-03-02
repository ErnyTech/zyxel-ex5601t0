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

dal_param_t CELLWAN_PSRU_param[]={
{"Passthru_Enable", dalType_boolean, 0, 0, NULL},
{"Passthru_Mode", dalType_string, 0, 0, NULL, "Dynamic|Fixed", 0},
{"Passthru_MacAddr", dalType_string, 0, 0, NULL},
{ NULL,	0,	0,	0,	NULL}

};

zcfgRet_t zcfgFeDalCellWanPsruEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *intfZyIpPassthruObj = NULL;
	objIndex_t intfZyIpPassthruIid = {0};
	bool passthruEnable = false;
	const char *passthruMode = NULL;
	const char *passthruMacAddr = NULL;

	IID_INIT(intfZyIpPassthruIid);
	intfZyIpPassthruIid.level=1;
	intfZyIpPassthruIid.idx[0]=1; 

	passthruEnable = json_object_get_boolean(json_object_object_get(Jobj, "Passthru_Enable"));
	passthruMode = json_object_get_string(json_object_object_get(Jobj, "Passthru_Mode"));
	passthruMacAddr = json_object_get_string(json_object_object_get(Jobj, "Passthru_MacAddr"));

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &intfZyIpPassthruIid, &intfZyIpPassthruObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Passthru_Enable"))
			json_object_object_add(intfZyIpPassthruObj, "Enable", json_object_new_boolean(passthruEnable));
		if(json_object_object_get(Jobj, "Passthru_Mode"))
			json_object_object_add(intfZyIpPassthruObj, "ConnectionMode", json_object_new_string(passthruMode));
		if(json_object_object_get(Jobj, "Passthru_MacAddr"))
			json_object_object_add(intfZyIpPassthruObj, "MACAddress", json_object_new_string(passthruMacAddr));
		zcfgFeObjJsonSet(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &intfZyIpPassthruIid, intfZyIpPassthruObj, NULL);
	}

	zcfgFeJsonObjFree(intfZyIpPassthruObj);

	return ret;
}	

zcfgRet_t zcfgFeDalCellWanPsruGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *intfZyIpPassthruObj = NULL;
	objIndex_t intfZyIpPassthruIid = {0};
	paramJobj = json_object_new_object();

	IID_INIT(intfZyIpPassthruIid);
	intfZyIpPassthruIid.level=1;
	intfZyIpPassthruIid.idx[0]=1; 

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &intfZyIpPassthruIid, &intfZyIpPassthruObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Passthru_Enable", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "Enable")));
		json_object_object_add(paramJobj, "Passthru_Mode", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "ConnectionMode")));
		json_object_object_add(paramJobj, "Passthru_MacAddr", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "MACAddress")));

		zcfgFeJsonObjFree(intfZyIpPassthruObj);
	}

	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowCellWanPsru(struct json_object *Jarray){
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	
	printf("\nCellular IP Passthrough:\n");

	if(json_object_get_boolean(json_object_object_get(obj, "Passthru_Enable")))
		printf("IP Passthrough Enable.\n");
	else
		printf("IP Passthrough Disable.\n");

	if(!strcmp("",json_object_get_string(json_object_object_get(obj, "Passthru_Mode"))))
		printf("%-30s %-10s \n","Connection Mode:", "N/A");
	else if(!strcmp("Fixed",json_object_get_string(json_object_object_get(obj, "Passthru_Mode")))){
		printf("%-30s %-10s \n","Connection Mode:", json_object_get_string(json_object_object_get(obj, "Passthru_Mode")));
		printf("%-30s %-10s \n","Passthrough to fixed MAC:", json_object_get_string(json_object_object_get(obj, "Passthru_MacAddr")));
	}else
		printf("%-30s %-10s \n","Connection Mode:", json_object_get_string(json_object_object_get(obj, "Passthru_Mode")));

}


zcfgRet_t zcfgFeDalCellWanPsru(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanPsruEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanPsruGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

