#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"


dal_param_t VOIP_Status_param[] = 
{
    {"VOIPStatus",          dalType_string, 0,  0,  NULL},
    {"idx0",                dalType_int,    0,  0,  NULL},
    {"idx1",                dalType_int,    0,  0,  NULL},
    {"idx2",                dalType_int,    0,  0,  NULL},
    {"X_ZYXEL_DoRegister",  dalType_int,    0,  2,  NULL},
    {NULL,                  0,              0,  0,  NULL}
};


/*
 *  Function Name: zcfgFeDalTrafficStatusGet
 *  Description: get All Req Intf Objs to Vue GUI for Traffic status
 *
 */
zcfgRet_t zcfgFeDalVoIPStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *getAllReqIntfObjs = NULL;
	struct json_object *voipLineObj = NULL, *voipLineJarray = NULL;
	struct json_object *voipProSIPfObj = NULL, *voipProSIPfJarray = NULL;
	struct json_object *voipSIPLineStObj = NULL, *voipSIPLineStJarray = NULL;
	struct json_object *voipPhyIFObj = NULL, *voipPhyIFJarray = NULL;
	struct json_object *voipCallStObj = NULL, *voipCallStJarray = NULL;
	struct json_object *devInfoObj = NULL;		
	objIndex_t iid;

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO, &iid, &devInfoObj)) != ZCFG_SUCCESS)
		goto zcfgFeDalVoIPStatusGetFail;
	
	voipProSIPfJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PROF_SIP, &iid, &voipProSIPfObj) == ZCFG_SUCCESS) {
		Jaddi(voipProSIPfObj, "idx0", iid.idx[0]);
		Jaddi(voipProSIPfObj, "idx1", iid.idx[1]);
		Jaddi(voipProSIPfObj, "idx2", iid.idx[2]);
		json_object_array_add(voipProSIPfJarray, voipProSIPfObj);
	}
	
	voipSIPLineStJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE_STATS, &iid, &voipSIPLineStObj) == ZCFG_SUCCESS) {
		Jaddi(voipSIPLineStObj, "idx0", iid.idx[0]);
		Jaddi(voipSIPLineStObj, "idx1", iid.idx[1]);
		Jaddi(voipSIPLineStObj, "idx2", iid.idx[2]);
		json_object_array_add(voipSIPLineStJarray, voipSIPLineStObj);
	}
	
	voipPhyIFJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_PHY_INTF, &iid, &voipPhyIFObj) == ZCFG_SUCCESS) {
		Jaddi(voipPhyIFObj, "idx0", iid.idx[0]);
		Jaddi(voipPhyIFObj, "idx1", iid.idx[1]);
		Jaddi(voipPhyIFObj, "idx2", iid.idx[2]);
		json_object_array_add(voipPhyIFJarray, voipPhyIFObj);
	}

	voipCallStJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_CALL_STATUS, &iid, &voipCallStObj) == ZCFG_SUCCESS) {
		Jaddi(voipCallStObj, "idx0", iid.idx[0]);
		Jaddi(voipCallStObj, "idx1", iid.idx[1]);
		Jaddi(voipCallStObj, "idx2", iid.idx[2]);
		json_object_array_add(voipCallStJarray, voipCallStObj);
	}

	voipLineJarray = json_object_new_array();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &iid, &voipLineObj) == ZCFG_SUCCESS) {
		Jaddi(voipLineObj, "idx0", iid.idx[0]);
		Jaddi(voipLineObj, "idx1", iid.idx[1]);
		Jaddi(voipLineObj, "idx2", iid.idx[2]);
		json_object_array_add(voipLineJarray, voipLineObj);
	}
	



	getAllReqIntfObjs = json_object_new_object();
	json_object_array_add(Jarray, getAllReqIntfObjs);
	json_object_object_add(getAllReqIntfObjs, "SIPInfo", voipProSIPfJarray);
	json_object_object_add(getAllReqIntfObjs, "SIPStatInfo", voipSIPLineStJarray);
	json_object_object_add(getAllReqIntfObjs, "PhoneInfo", voipPhyIFJarray);
	json_object_object_add(getAllReqIntfObjs, "CallInfo", voipCallStJarray);
	json_object_object_add(getAllReqIntfObjs, "LineInfo", voipLineJarray);		
	json_object_object_add(getAllReqIntfObjs, "DevInfo", devInfoObj);
	return ZCFG_SUCCESS;

zcfgFeDalVoIPStatusGetFail:	
	json_object_put(devInfoObj);
	
	return ret;
}
zcfgRet_t zcfgFeDalVoIPStatusEdit(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *lineObj;
    objIndex_t lineIid;
    int DoRegister = 0;

    IID_INIT(lineIid);

    if (Jobj == NULL ||
        json_object_object_get(Jobj, "idx0") == NULL ||
        json_object_object_get(Jobj, "idx1") == NULL ||
        json_object_object_get(Jobj, "idx2") == NULL ||
        json_object_object_get(Jobj, "X_ZYXEL_DoRegister") == NULL)
    {
        return ret;
    }

    lineIid.level = 3;
    lineIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx0"));
    lineIid.idx[1] = json_object_get_int(json_object_object_get(Jobj, "idx1"));
    lineIid.idx[2] = json_object_get_int(json_object_object_get(Jobj, "idx2"));
    DoRegister = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_DoRegister"));

    if (zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE, &lineIid, &lineObj) == ZCFG_SUCCESS)
    {
        json_object_object_add(lineObj, "X_ZYXEL_DoRegister", json_object_new_int(DoRegister));
        zcfgFeObjJsonBlockedSet(RDM_OID_VOICE_LINE, &lineIid, lineObj, NULL);
        zcfgFeJsonObjFree(lineObj);
    }

    return ret;
}
 zcfgRet_t zcfgFeDalVoIPStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
 {
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (!method || !Jobj)
    {
        return ZCFG_INTERNAL_ERROR;
    }

    if (!strcmp(method, "GET"))
    {
        ret = zcfgFeDalVoIPStatusGet(Jobj, Jarray, NULL);
    }
    else if (!strcmp(method, "PUT"))
    {
        ret = zcfgFeDalVoIPStatusEdit(Jobj, replyMsg);
    }

	return ret;	 
 }
