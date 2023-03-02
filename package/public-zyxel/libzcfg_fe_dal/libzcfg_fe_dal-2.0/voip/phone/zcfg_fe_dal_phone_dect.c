
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"

#define DECT_H_S_EDIT	0
#define DECT_B_S_EDIT	1

dal_param_t VOIP_PHONEDECT_param[]={
	{"Name",									dalType_string,	0,	0,	NULL},
	{"FunctionEnable",							dalType_boolean,0,	0,	NULL},
	{"ECOModeEnable",							dalType_boolean,0,	0,	NULL},
	{"RFStrength",								dalType_int,	0,	0,	NULL},
	{"NEMoEnable",								dalType_boolean,0,	0,	NULL},
	{"EncryptionEnable",						dalType_boolean,0,	0,	NULL},
	{"PINcode",									dalType_string,	0,	0,	NULL},
	{"DoDeregisterHS",							dalType_int,	0,	0,	NULL},
	{"DoPagingHSOnOff",							dalType_int,	0,	0,	NULL},
	{"DoRegistrationModeOnOff",					dalType_int,	0,	0,	NULL},
	{"DoFirmwareUpgrade",						dalType_boolean,0,	0,	NULL},
	{NULL,										dalType_boolean,0,	0,	NULL},
};

unsigned int flag1;
bool dectEditType;
int dectHSNum;

struct json_object *multiJobj;
struct json_object *dectHSJobj;
struct json_object *dectBSJobj;

objIndex_t dectHSIid = {0};
objIndex_t dectBSIid = {0};

const char* Name;
bool ECOModeEnable;
bool FunctionEnable;
unsigned int RFStrength;
bool NEMoEnable;
bool EncryptionEnable;
const char* PINcode;
unsigned int DoPagingHSOnOff;
unsigned int DoRegistrationModeOnOff;
unsigned int DoDeregisterHS;
bool DoFirmwareUpgrade;

void freeAllDECTObjects(){
	if(multiJobj) json_object_put(multiJobj);
	if(dectHSJobj) json_object_put(dectHSJobj);
	if(dectBSJobj) json_object_put(dectBSJobj);

	return;
}

void initDECTGlobalObjects(){
	multiJobj = NULL;
	dectHSJobj = NULL;
	dectBSJobj = NULL;

	return;
}


void getDECTBasicInfo(struct json_object *Jobj){
	Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	FunctionEnable = json_object_get_boolean(json_object_object_get(Jobj, "FunctionEnable"));
	ECOModeEnable = json_object_get_boolean(json_object_object_get(Jobj, "ECOModeEnable"));
	RFStrength = json_object_get_int(json_object_object_get(Jobj, "RFStrength"));
	NEMoEnable = json_object_get_boolean(json_object_object_get(Jobj, "NEMoEnable"));
	EncryptionEnable = json_object_get_boolean(json_object_object_get(Jobj, "EncryptionEnable"));
	PINcode = json_object_get_string(json_object_object_get(Jobj, "PINcode"));
	DoDeregisterHS = json_object_get_int(json_object_object_get(Jobj, "DoDeregisterHS"));
	DoPagingHSOnOff = json_object_get_int(json_object_object_get(Jobj, "DoPagingHSOnOff"));
	DoRegistrationModeOnOff = json_object_get_int(json_object_object_get(Jobj, "DoRegistrationModeOnOff"));
	DoFirmwareUpgrade = json_object_get_boolean(json_object_object_get(Jobj, "DoFirmwareUpgrade"));
}

void zcfgFeDalDECTHSObjEditIidGet(struct json_object *Jobj){
	const char* path = NULL;
	IID_INIT(dectHSIid);

	path = json_object_get_string(json_object_object_get(Jobj, "HandsetPath"));
	if(path == NULL){
		return;
	}

	dectEditType = DECT_H_S_EDIT;

	sscanf(path,"Device.Services.VoiceService.1.X_ZYXEL_DECT.Handset.%d",&dectHSNum);
	//printf(" dectHSNum: %d \n ",dectHSNum);

	dectHSIid.level = 2;
	dectHSIid.idx[0] = 1;
	dectHSIid.idx[1] = dectHSNum;

	return;
}

void zcfgFeDalDECTBSObjEditIidGet(struct json_object *Jobj){
	const char* path = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);
	IID_INIT(dectBSIid);

	path = json_object_get_string(json_object_object_get(Jobj, "BaseStationPath"));
	if(path == NULL){
		return;
	}

	dectEditType = DECT_B_S_EDIT;
	dectBSIid.level = 1;
	dectBSIid.idx[0] = 1;

	return;
}

zcfgRet_t zcfgFeDalVoipDECTBaseStationEdit(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_DECT_B_S, &dectBSIid, &dectBSJobj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Dect BS obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(dectBSJobj != NULL){
		if(json_object_object_get(Jobj, "DoPagingHSOnOff") != NULL){
			json_object_object_add(dectBSJobj, "DoPagingHSOnOff", json_object_new_int(DoPagingHSOnOff));
		}else if(json_object_object_get(Jobj, "DoRegistrationModeOnOff") != NULL){
			json_object_object_add(dectBSJobj, "DoRegistrationModeOnOff", json_object_new_int(DoRegistrationModeOnOff));
		}else if(json_object_object_get(Jobj, "DoDeregisterHS") != NULL){
			json_object_object_add(dectBSJobj, "DoDeregisterHS", json_object_new_int(DoDeregisterHS));
		}else if(json_object_object_get(Jobj, "DoFirmwareUpgrade") != NULL){
			json_object_object_add(dectBSJobj, "DoFirmwareUpgrade", json_object_new_boolean(DoFirmwareUpgrade));
		}else{
			json_object_object_add(dectBSJobj, "FunctionEnable", json_object_new_boolean(FunctionEnable));
			json_object_object_add(dectBSJobj, "ECOModeEnable", json_object_new_boolean(ECOModeEnable));
			json_object_object_add(dectBSJobj, "RFStrength", json_object_new_int(RFStrength));
			json_object_object_add(dectBSJobj, "NEMoEnable", json_object_new_boolean(NEMoEnable));
			json_object_object_add(dectBSJobj, "EncryptionEnable", json_object_new_boolean(EncryptionEnable));
			json_object_object_add(dectBSJobj, "PINcode", json_object_new_string(PINcode));
		}

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_DECT_B_S, &dectBSIid, multiJobj, dectBSJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_DECT_B_S);
			zcfgFeJsonObjFree(dectBSJobj);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	//printf("%s(): Append DECT_B_S obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipDECTHandSetEdit(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_DECT_H_S, &dectHSIid, &dectHSJobj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Dect handset obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(dectHSJobj != NULL){
		json_object_object_add(dectHSJobj, "Name", json_object_new_string(Name));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_DECT_H_S, &dectHSIid, multiJobj, dectHSJobj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_DECT_H_S);
			zcfgFeJsonObjFree(dectHSJobj);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	//printf("%s(): Append DECT_H_S obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipDECTEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	initDECTGlobalObjects();
	getDECTBasicInfo(Jobj);
	zcfgFeDalDECTHSObjEditIidGet(Jobj);
	zcfgFeDalDECTBSObjEditIidGet(Jobj);
	multiJobj = zcfgFeJsonObjNew();

	if(dectEditType == DECT_H_S_EDIT){
		if((ret = zcfgFeDalVoipDECTHandSetEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit Dect HS obj \n", __FUNCTION__);
			goto exit;
		}
	}else if(dectEditType == DECT_B_S_EDIT){
		if((ret = zcfgFeDalVoipDECTBaseStationEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit Dect BS obj \n", __FUNCTION__);
			goto exit;
		}
	}

	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set dect object \n", __FUNCTION__);
		goto exit;
	}

exit:
	freeAllDECTObjects();
	return ret;
}

zcfgRet_t zcfgFeDalVoipPhoneDECT(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVoipDECTEdit(Jobj, NULL);
	}

	return ret;
}

