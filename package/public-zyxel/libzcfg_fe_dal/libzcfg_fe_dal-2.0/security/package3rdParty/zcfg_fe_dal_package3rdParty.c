
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t Package_3rdParty_param[] =
{
	//Package3rdParty
	{"Index",			dalType_int,		0,	0,	NULL,	NULL,	dal_Edit|dal_Delete},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},	
	{"Version",			dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Script",			dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"Data", 			dalType_string, 	0,	0,	NULL,	NULL,	0},
	{"FixCustID",		dalType_string, 	0,	0,	NULL,	NULL,	0},
	{"Hash",		    dalType_string, 	0,	0,	NULL,	NULL,	0},
	{"Status",			dalType_string, 	0,	0,	NULL,	NULL,	0},
	{NULL,				0,					0,	0,	NULL,	NULL,	0}
};

zcfgRet_t zcfgFeDalPackage3rdPartyGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *Package3rdPartyObj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
	struct json_object *enable = NULL;
	struct json_object *name = NULL;
	struct json_object *version = NULL;
	struct json_object *script = NULL;
	struct json_object *data = NULL;
	struct json_object *custID = NULL;
	struct json_object *hash = NULL;
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
	char buffer[1024] = {0};
	char *scriptName = NULL;
	char sysCmd[512] = {0};
	FILE *fp = NULL;
#else
	struct json_object *status = NULL;
#endif	
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_PACKAGE, &objIid, &Package3rdPartyObj) == ZCFG_SUCCESS){
		index = objIid.idx[0];
		
		name = json_object_object_get(Package3rdPartyObj, "Name");
		version = json_object_object_get(Package3rdPartyObj, "Version");
		script = json_object_object_get(Package3rdPartyObj, "Script");
		enable = json_object_object_get(Package3rdPartyObj, "Enable");
		data = json_object_object_get(Package3rdPartyObj, "Data");
		custID = json_object_object_get(Package3rdPartyObj, "FixCustID");
		hash = json_object_object_get(Package3rdPartyObj, "Hash");
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
		scriptName = json_object_get_string(json_object_object_get(Package3rdPartyObj, "Script"));
		//printf("zcfgFeDalPackage3rdPartyGet : scriptName = %s\n", scriptName);
		if((!strcmp(scriptName, "avast-agent.sh")) || (!strcmp(scriptName, "avast-smarthome.sh"))){
			sprintf(sysCmd, "sh /misc/etc/init.d/%s status", scriptName);
			//printf("zcfgFeDalPackage3rdPartyGet : syscmd get status cmd = %s\n", sysCmd);
			if((fp = popen(sysCmd, "r")) != NULL && fgets(buffer, sizeof(buffer), fp) != NULL) {
				printf("zcfgFeDalPackage3rdPartyGet : syscmd get status buffer: %s\n", buffer);
				//json_object_object_add(Package3rdPartyObj, "Status", json_object_new_string(buffer));
				//zcfgFeObjJsonSetWithoutApply(RDM_OID_PACKAGE, &objIid, Package3rdPartyObj, NULL);
			}
		}
	
#else
		status = json_object_object_get(Package3rdPartyObj, "Status");
#endif		
		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(index));
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(name));
		json_object_object_add(pramJobj, "Version", JSON_OBJ_COPY(version));
		json_object_object_add(pramJobj, "Script", JSON_OBJ_COPY(script));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
		json_object_object_add(pramJobj, "Data", JSON_OBJ_COPY(data));
		json_object_object_add(pramJobj, "FixCustID", JSON_OBJ_COPY(custID));
		json_object_object_add(pramJobj, "Hash", JSON_OBJ_COPY(hash));
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
		json_object_object_add(pramJobj, "Status", json_object_new_string(buffer));
#else
		json_object_object_add(pramJobj, "Status", JSON_OBJ_COPY(status));
#endif		
		json_object_array_add(Jarray, pramJobj);
		
		zcfgFeJsonObjFree(Package3rdPartyObj);
	}
	dbg_printf("%s : Jarray=%s\n",__FUNCTION__,json_object_to_json_string(Jarray));
	return ret;
}

zcfgRet_t zcfgFeDalPackage3rdPartyEditArray(struct json_object *Jobj, char *replyMsg){ 
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *package3rdPartyObj = NULL;
	struct json_object *reqObj = NULL;
	int reqObjIdx = 1;
	objIndex_t package3rdPartyIid = {0};
	
	bool enable = false;
	int Index = 0;

	enum json_type jsonType;
	//printf("### zcfgFeDalPackage3rdPartyEdit ### zcfgFeDalPackage3rdPartyEdit \n");
	while((reqObj = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){

		Index = json_object_get_int(json_object_object_get(reqObj, "Index"));
		enable = json_object_get_boolean(json_object_object_get(reqObj, "Enable"));
		//printf("### objIndex ### Index = %d\n", Index);
		//printf("### objIndex ### enable = %d\n", enable);
		reqObjIdx++;
	
		IID_INIT(package3rdPartyIid);
		package3rdPartyIid.level = 1;
		package3rdPartyIid.idx[0] = Index;
	
		ret = zcfgFeObjJsonGet(RDM_OID_PACKAGE, &package3rdPartyIid, &package3rdPartyObj);
		if(ret == ZCFG_SUCCESS) {
			json_object_object_add(package3rdPartyObj, "Enable", json_object_new_boolean(enable));
			ret = zcfgFeObjJsonBlockedSet(RDM_OID_PACKAGE, &package3rdPartyIid, package3rdPartyObj, NULL);
		}
		
		zcfgFeJsonObjFree(package3rdPartyObj);
		zcfgFeJsonObjFree(reqObj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalPackage3rdPartyEdit(struct json_object *Jobj, char *replyMsg)
{
 	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *Package3rdPartyJobj = NULL;
	struct json_object *pramJobj = NULL;
	int objIndex;
	bool Enable = false;

	objIndex_t objIid={0};
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	//printf("### objIndex ### objIndex = %d/n", objIndex);
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_PACKAGE, NULL, NULL, replyMsg );
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_PACKAGE, &objIid, &Package3rdPartyJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(json_object_object_get(Jobj, "Enable") != NULL){
			Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(Package3rdPartyJobj, "Enable", json_object_new_boolean(Enable));
	}
	
	ret = zcfgFeObjJsonBlockedSet(RDM_OID_PACKAGE, &objIid, Package3rdPartyJobj, NULL);
	json_object_put(Package3rdPartyJobj);
	return ret;
}

zcfgRet_t zcfgFeDalPackage3rdParty(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {	
		if(json_object_get_type(Jobj) == json_type_array){ // for GUI, tmp solution
			ret = zcfgFeDalPackage3rdPartyEditArray(Jobj, replyMsg);
		}
		else{
			ret = zcfgFeDalPackage3rdPartyEdit(Jobj, replyMsg);
		}
	}
#if 0 // temp
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalPackage3rdPartyAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalPackage3rdPartyDelete(Jobj, replyMsg);
	}
#endif	
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalPackage3rdPartyGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

