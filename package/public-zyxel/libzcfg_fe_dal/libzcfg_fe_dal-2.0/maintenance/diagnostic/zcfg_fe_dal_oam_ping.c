
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zlog_api.h"

#define ATM_DIAG_F4_LO 4
#define ATM_DIAG_F5_LO 5

dal_param_t OAMPING_param[]={
	{"LoopbackType",							dalType_int,	0,	0,	NULL},
	{"DiagnosticsState",						dalType_string,	0,	0,	NULL},
	{"NumberOfRepetitions",						dalType_int,	0,	0,	NULL},
	{"Interface",								dalType_string,	0,	0,	NULL},
	{"SegmentLoopback",							dalType_boolean,0,	0,	NULL},
	{"X_ZYXEL_SegmentLoopback",					dalType_boolean,0,	0,	NULL},
	{NULL,										0,				0,	0,	NULL},
};

struct json_object *atmF4Obj;
struct json_object *atmF5Obj;

void freeAllOamPingObjects(){
	if(atmF4Obj) json_object_put(atmF4Obj);
	if(atmF5Obj) json_object_put(atmF5Obj);

	return;
}

void initOamPingGlobalObjects(){
	atmF4Obj = NULL;
	atmF5Obj = NULL;

	return;
}

unsigned int LoopbackType;
unsigned int NumberOfRepetitions;
const char* DiagnosticsState;
const char* Interface;
bool SegmentLoopback;
bool X_ZYXEL_SegmentLoopback;


objIndex_t F4objIid;
objIndex_t F5objIid;

void getOamPingF4BasicInfo(struct json_object *Jobj){
	DiagnosticsState = json_object_get_string(json_object_object_get(Jobj, "DiagnosticsState"));
	NumberOfRepetitions = json_object_get_int(json_object_object_get(Jobj, "NumberOfRepetitions"));
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	SegmentLoopback = json_object_get_boolean(json_object_object_get(Jobj, "SegmentLoopback"));

	return;
}

void getOamPingF5BasicInfo(struct json_object *Jobj){
	DiagnosticsState = json_object_get_string(json_object_object_get(Jobj, "DiagnosticsState"));
	NumberOfRepetitions = json_object_get_int(json_object_object_get(Jobj, "NumberOfRepetitions"));
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
	X_ZYXEL_SegmentLoopback = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_SegmentLoopback"));
}

void getOamPingType(struct json_object *Jobj){
	if( NULL != json_object_object_get(Jobj, "LoopbackType") ){
		LoopbackType = json_object_get_int(json_object_object_get(Jobj, "LoopbackType"));
	}
	return;
}


zcfgRet_t zcfgFeDalOamPingF4LookbackEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" Enter ..\n");
	getOamPingF4BasicInfo(Jobj);

	IID_INIT(F4objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_DIAG_F4_LO, &F4objIid, &atmF4Obj)) != ZCFG_SUCCESS){
		ZLOG_INFO("(ERROR) zcfgFeObjJsonGet get atmF4Loopback fail~ \n ");
		return ZCFG_INTERNAL_ERROR;
	}

	if(atmF4Obj != NULL){
		json_object_object_add(atmF4Obj, "DiagnosticsState", json_object_new_string(DiagnosticsState));
		json_object_object_add(atmF4Obj, "NumberOfRepetitions", json_object_new_int(NumberOfRepetitions));
		json_object_object_add(atmF4Obj, "Interface", json_object_new_string(Interface));
		json_object_object_add(atmF4Obj, "SegmentLoopback", json_object_new_boolean(SegmentLoopback));

		if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_DIAG_F4_LO, &F4objIid, atmF4Obj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) fail to set object %d\n", RDM_OID_ATM_DIAG_F4_LO);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalOamPingF5LookbackEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO("Enter ..\n");
	getOamPingF5BasicInfo(Jobj);


	IID_INIT(F5objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_DIAG_F5_LO, &F5objIid, &atmF5Obj)) != ZCFG_SUCCESS){
		ZLOG_INFO("(ERROR) zcfgFeObjJsonGet get atmF5Loopback fail~ \n ");
		return ZCFG_INTERNAL_ERROR;
	}

	if(atmF5Obj != NULL){
		json_object_object_add(atmF5Obj, "DiagnosticsState", json_object_new_string(DiagnosticsState));
		json_object_object_add(atmF5Obj, "NumberOfRepetitions", json_object_new_int(NumberOfRepetitions));
		json_object_object_add(atmF5Obj, "Interface", json_object_new_string(Interface));
		json_object_object_add(atmF5Obj, "X_ZYXEL_SegmentLoopback", json_object_new_boolean(X_ZYXEL_SegmentLoopback));

		if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_DIAG_F5_LO, &F5objIid, atmF5Obj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) fail to set object %d\n", RDM_OID_ATM_DIAG_F5_LO);
			return ZCFG_INTERNAL_ERROR;
		}

	}

	return ret;
}

zcfgRet_t zcfgFeDalOamPingEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" input --- %s \n ",json_object_to_json_string(Jobj));
	initOamPingGlobalObjects();
	getOamPingType(Jobj);

	if(LoopbackType == ATM_DIAG_F4_LO){
		if((ret = zcfgFeDalOamPingF4LookbackEdit(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Oam Ping F4 Lookback fail\n");
			goto exit;
		}
	}else if(LoopbackType == ATM_DIAG_F5_LO){
		if((ret = zcfgFeDalOamPingF5LookbackEdit(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Oam Ping F5 Lookback fail\n");
			goto exit;
		}
	}

exit:
	freeAllOamPingObjects();
	return ret;
}

zcfgRet_t zcfgFeDalOamPingGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	initOamPingGlobalObjects();
	getOamPingType(Jobj);
	if( ATM_DIAG_F4_LO == LoopbackType){
		IID_INIT(F4objIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_DIAG_F4_LO, &F4objIid, &atmF4Obj)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) zcfgFeObjJsonGet get atmF4Loopback fail~ \n ");
			return ZCFG_INTERNAL_ERROR;
		}
		json_object_array_add(Jarray, JSON_OBJ_COPY(atmF4Obj));
	}else if( ATM_DIAG_F5_LO == LoopbackType){
		IID_INIT(F5objIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_DIAG_F5_LO, &F5objIid, &atmF5Obj)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) zcfgFeObjJsonGet get atmF5Loopback fail~ \n ");
			return ZCFG_INTERNAL_ERROR;
		}
		json_object_array_add(Jarray, JSON_OBJ_COPY(atmF5Obj));
	}

	freeAllOamPingObjects();
	return ret;
}

zcfgRet_t zcfgFeOamPing(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalOamPingEdit(Jobj, NULL);
	}
	if(!strcmp(method, "GET")) {
		ret =zcfgFeDalOamPingGet(Jobj, Jarray, NULL);
	}

	return ret;
}

