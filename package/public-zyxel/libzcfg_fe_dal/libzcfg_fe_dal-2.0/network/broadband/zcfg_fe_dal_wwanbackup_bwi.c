#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t Bandwidth_Improvement_param[] =
{
	{"Enable",				dalType_boolean,	0,	0,	NULL,	NULL,	0},		
	{"Status",				dalType_int,		0,	0,	NULL,	NULL,	0},	
	{"BWMInterval",			dalType_int,		0,	0,	NULL,	NULL,	0},	
	{"TriggerLording",		dalType_int, 		0,	0,	NULL,	NULL,	0},
	{"DeTriggerLording",	dalType_int, 		0,	0,	NULL,	NULL,	0},
	{NULL,		           	0,					0,	0,	NULL,	NULL,	0}
};

void zcfgFeDalShowBandwidthImprovement(struct json_object *Jarray){
	struct json_object *Jobj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	Jobj = json_object_array_get_idx(Jarray,0);

	printf("Bandwidth Improvement Setup: \n");
	printf("%-25s %u\n", "Enable", json_object_get_boolean(json_object_object_get(Jobj, "Enable")));
	printf("%-25s %lld\n", "Status", json_object_get_int(json_object_object_get(Jobj, "Status")));
	printf("%-40s %lld\n", "BWMInterval", json_object_get_int(json_object_object_get(Jobj, "BWMInterval")));
	printf("%-50s %lld\n", "TriggerLording", json_object_get_int(json_object_object_get(Jobj, "TriggerLording")));
	printf("%-50s %lld\n", "DeTriggerLording", json_object_get_int(json_object_object_get(Jobj, "DeTriggerLording")));
	
}

zcfgRet_t zcfgFeDalBandwidthImprovementGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *improvementObj = NULL;

	objIndex_t improvementIid = {0};

	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &improvementIid, &improvementObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(improvementObj, "Enable")));
		json_object_object_add(paramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(improvementObj, "Status")));
		json_object_object_add(paramJobj, "BWMInterval", JSON_OBJ_COPY(json_object_object_get(improvementObj, "BWMInterval")));
		json_object_object_add(paramJobj, "TriggerLording", JSON_OBJ_COPY(json_object_object_get(improvementObj, "TriggerLording")));
		json_object_object_add(paramJobj, "DeTriggerLording", JSON_OBJ_COPY(json_object_object_get(improvementObj, "DeTriggerLording")));
		zcfgFeJsonObjFree(improvementObj);
	}

	json_object_array_add(Jarray, paramJobj);
	return ret;

}

zcfgRet_t zcfgFeDalBandwidthImprovementEdit(struct json_object *Jobj, char *replyMsg){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *improvementObj = NULL;
	objIndex_t improvementIid = {0};
	
	bool bwi_Enable = false;
	int bwi_Status = 0;
	int bwi_BWMInterval = 0;
	int bwi_TriggerLording = 0;
	int bwi_DeTriggerLording = 0;

	IID_INIT(improvementIid);
	
	bwi_Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	bwi_Status = json_object_get_int(json_object_object_get(Jobj, "Status"));
	bwi_BWMInterval = json_object_get_int(json_object_object_get(Jobj, "BWMInterval"));
	bwi_TriggerLording = json_object_get_int(json_object_object_get(Jobj, "TriggerLording"));
	bwi_DeTriggerLording = json_object_get_int(json_object_object_get(Jobj, "DeTriggerLording"));

	//printf("zcfgFeDalBandwidthImprovementEdit: DAL BandwidthImprovement SET Enable=%d\n", bwi_Enable);
	//printf("zcfgFeDalBandwidthImprovementEdit: DAL BandwidthImprovement SET bwi_Enable=%d\n", bwi_Status);
	//printf("zcfgFeDalBandwidthImprovementEdit: DAL BandwidthImprovement SET bwi_BWMInterval=%d\n", bwi_BWMInterval);
	//printf("zcfgFeDalBandwidthImprovementEdit: DAL BandwidthImprovement SET bwi_TriggerLording=%d\n", bwi_TriggerLording);
	//printf("zcfgFeDalBandwidthImprovementEdit: DAL BandwidthImprovement SET bwi_DeTriggerLording=%d\n", bwi_DeTriggerLording);

	if (zcfgFeObjJsonGet(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &improvementIid, &improvementObj) != ZCFG_SUCCESS){	
			printf("Get RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT object fail!\n");
		return ret;
	}
	else{	
		//printf("GET RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT ZCFG_SUCCESS\n");
		if(json_object_object_get(Jobj, "Enable")){
			json_object_object_add(improvementObj, "Enable", json_object_new_boolean(bwi_Enable));
		}
		if(json_object_object_get(Jobj, "Status")){
			json_object_object_add(improvementObj, "Status", json_object_new_int(bwi_Status));
		}
		if(json_object_object_get(Jobj, "BWMInterval")){
			json_object_object_add(improvementObj, "BWMInterval", json_object_new_int(bwi_BWMInterval));
		}
		if(json_object_object_get(Jobj, "TriggerLording")){
			json_object_object_add(improvementObj, "TriggerLording", json_object_new_int(bwi_TriggerLording));
		}
		if(json_object_object_get(Jobj, "DeTriggerLording")){
			json_object_object_add(improvementObj, "DeTriggerLording", json_object_new_int(bwi_DeTriggerLording));
		}
		zcfgFeObjJsonSet(RDM_OID_ZY_BAND_WIDTH_IMPROVEMENT, &improvementIid, improvementObj, NULL);
		zcfgFeJsonObjFree(improvementObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalBandwidthImprovement(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalBandwidthImprovementEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalBandwidthImprovementGet(Jobj, Jarray, NULL);
	}
	
	return ret;
}
