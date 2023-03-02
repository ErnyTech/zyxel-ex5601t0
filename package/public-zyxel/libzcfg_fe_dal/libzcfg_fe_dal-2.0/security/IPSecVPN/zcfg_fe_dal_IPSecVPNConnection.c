
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t IPSecVPN_param[] =
{
	{"Index",			dalType_int,		0,	0,	NULL,	NULL,	dal_Edit|dal_Delete},
	{"Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},	
	{"Description",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Days",			dalType_string,		0,	0,	NULL,	NULL,	0},
	{"TimeStartHour",	dalType_int,		0,	23,	NULL,	NULL,	0},
	{"TimeStartMin",	dalType_int,		0,	59,	NULL,	NULL,	0},
	{"TimeStopHour",	dalType_int,		0,	23,	NULL,	NULL,	0},
	{"TimeStopMin",		dalType_int,		0,	59,	NULL,	NULL,	0},
	{NULL,				0,					0,	0,	NULL,	NULL,	0}
};

void zcfgFeDalShowIPSecVPN(struct json_object *Jarray){

	int i, len = 0;
	char *dayString = NULL;
	int index = 0, startHour = 0, startMin = 0, stopHour = 0, stopMin = 0;
	char timeStr [15] = {0};
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-6s %-15s %-30s %-15s %-30s\n", "Index", "Rule Name", "Days", "Time", "Description");
	printf("--------------------------------------------------------------------------------\n");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){

		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){

			dayString = json_object_get_string(json_object_object_get(obj, "Days"));			
			startHour = json_object_get_int(json_object_object_get(obj, "TimeStartHour"));
			startMin = json_object_get_int(json_object_object_get(obj, "TimeStartMin"));
			stopHour = json_object_get_int(json_object_object_get(obj, "TimeStopHour"));
			stopMin = json_object_get_int(json_object_object_get(obj, "TimeStopMin"));

			memset(timeStr, '\0', sizeof(timeStr));
			sprintf(timeStr, "%02d:%02d~%02d:%02d",startHour,startMin,stopHour,stopMin);

			printf("%-6d ",i+1);
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-30s ",dayString);
			printf("%-15s ",timeStr);
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "Description")));
			printf("\n");

		}
		else{
			printf("ERROR!\n");
		}
		
	}

}

zcfgRet_t zcfgFeDalIPSecVPNGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
	char path[64] = {0};
	bool Enable;
	const char *Alias, *RemoteEndpoints, *Origin, *DestIPPrefix;
	const char *X_ZYXEL_SrvName = NULL, *X_ZYXEL_IfName = NULL, *LowerLayers = NULL;
	int count = 1;
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IPSEC_PROFILE, &iid, &obj) == ZCFG_SUCCESS) {
		Alias = json_object_get_string(json_object_object_get(obj, "Alias"));
		RemoteEndpoints = json_object_get_string(json_object_object_get(obj, "RemoteEndpoints"));
		if(strcmp(Alias, "") && strcmp(RemoteEndpoints, "")){
			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(count));
			json_object_object_add(pramJobj, "Name", json_object_new_string(Alias));
			json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(obj, "Enable")));
			Enable = json_object_get_boolean(json_object_object_get(obj, "Enable"));
			if(Enable == true)
				json_object_object_add(pramJobj, "Status", json_object_new_string("Enable"));//need show status in list
			else
				json_object_object_add(pramJobj, "Status", json_object_new_string("Disable"));
			
			json_object_object_add(pramJobj, "RemoteIPSecGateWay", json_object_new_string(RemoteEndpoints));
			
			if(!strcmp("Subnet",json_object_get_string(json_object_object_get(obj, "LocalIPMode"))))
				json_object_object_add(pramJobj, "LocalIPMode", json_object_new_int(0));
			else
				json_object_object_add(pramJobj, "LocalIPMode", json_object_new_int(1));				
			json_object_object_add(pramJobj, "LocalIPAddress", JSON_OBJ_COPY(json_object_object_get(obj, "LocalIPAddress")));
			json_object_object_add(pramJobj, "LocalIPMask", JSON_OBJ_COPY(json_object_object_get(obj, "LocalIPMask")));
			if(!strcmp("Subnet",json_object_get_string(json_object_object_get(obj, "RemoteIPMode"))))
				json_object_object_add(pramJobj, "RemoteIPMode", json_object_new_int(0));
			else
				json_object_object_add(pramJobj, "RemoteIPMode", json_object_new_int(1));
			json_object_object_add(pramJobj, "RemoteIPAddress", JSON_OBJ_COPY(json_object_object_get(obj, "RemoteIPAddress")));
			json_object_object_add(pramJobj, "RemoteIPMask", JSON_OBJ_COPY(json_object_object_get(obj, "RemoteIPMask")));
			if(!strcmp("esp",json_object_get_string(json_object_object_get(obj, "Protocol"))))
				json_object_object_add(pramJobj, "Protocol", json_object_new_int(0));
			else
				json_object_object_add(pramJobj, "Protocol", json_object_new_int(1));
			json_object_object_add(pramJobj, "NATForward", JSON_OBJ_COPY(json_object_object_get(obj, "NATForward")));
			json_object_object_add(pramJobj, "KeyExchaMode", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_AuthType")));//0:Auto(IKEv1), 1:Manunal
			if(json_object_get_int(json_object_object_get(obj, "X_ZYXEL_AuthType")) == 0){
				json_object_object_add(pramJobj, "IKEv1Phase1Mode", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1Phase1Mode")));
				json_object_object_add(pramJobj, "IKEv1EncrypAlgori", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1EncrypAlgori")));
				json_object_object_add(pramJobj, "IKEv1IntegrityAlgori", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1IntegrityAlgori")));
				json_object_object_add(pramJobj, "IKEv1SelectKeyExcha", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1SelectKeyExcha")));
				json_object_object_add(pramJobj, "IKEv1KeyLifeTime", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1KeyLifeTime")));
				json_object_object_add(pramJobj, "IKEv1EncrypAlgori2", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1EncrypAlgori2")));
				json_object_object_add(pramJobj, "IKEv1IntegrityAlgori2", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1IntegrityAlgori2")));
				json_object_object_add(pramJobj, "IKEv1SelectKeyExcha2", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1SelectKeyExcha2")));
				json_object_object_add(pramJobj, "IKEv1KeyLifeTime2", JSON_OBJ_COPY(json_object_object_get(obj, "IKEv1KeyLifeTime2")));
			}
			else{
				json_object_object_add(pramJobj, "ManualEncrypAlgori", JSON_OBJ_COPY(json_object_object_get(obj, "ManualEncrypAlgori")));
				json_object_object_add(pramJobj, "ManualEncrypKey", JSON_OBJ_COPY(json_object_object_get(obj, "ManualEncrypKey")));
				json_object_object_add(pramJobj, "ManualAuthenAlgori", JSON_OBJ_COPY(json_object_object_get(obj, "ManualAuthenAlgori")));
				json_object_object_add(pramJobj, "ManualAuthenKey", JSON_OBJ_COPY(json_object_object_get(obj, "ManualAuthenKey")));
				json_object_object_add(pramJobj, "ManualSPI", JSON_OBJ_COPY(json_object_object_get(obj, "ManualSPI")));
			}
			json_object_object_add(pramJobj, "IKEAuthMode", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_IKEv1XAuthMode")));//0:PreSharedKey, 1:Certificate
			json_object_object_add(pramJobj, "PreSharedKey", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_AuthPreSharedKey")));
			json_object_object_add(pramJobj, "Certificate", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_AuthCertificate")));
			json_object_object_add(pramJobj, "idx", json_object_new_int(iid.idx[1]));

			json_object_array_add(Jarray, pramJobj);
			count++;
		}
		zcfgFeJsonObjFree(obj);
	}
	return ret;
	
}


zcfgRet_t zcfgFeDalIPSecVPNAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *IPSecVPNJobj = NULL;
	struct json_object *pramJobj = NULL;
	char *name = NULL;
	char msg[30]={0};
	char path[32] = {0};
	struct json_object *retJobj = NULL;
	int LocalIPMode = 0,RemoteIPMode = 0;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_IPSEC_PROFILE, &objIid, NULL)) != ZCFG_SUCCESS){
		if(ret == ZCFG_EXCEED_MAX_INS_NUM){
			printf("Cannot add entry since the maximum number of entries has been reached.\n");
    	}
		printf("%s: Object used all fail\n", __FUNCTION__);
		return ret;
	}	
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IPSEC_PROFILE, &objIid, &IPSecVPNJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}	

	pramJobj = json_object_object_get(Jobj, "Name");
	json_object_object_add(IPSecVPNJobj, "Alias", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "Enable");
	json_object_object_add(IPSecVPNJobj, "Enable", JSON_OBJ_COPY(pramJobj));	
	if((pramJobj = json_object_object_get(Jobj, "RemoteIPSecGateWay")) != NULL)
		json_object_object_add(IPSecVPNJobj, "RemoteEndpoints", JSON_OBJ_COPY(pramJobj));

	if(json_object_get_int(json_object_object_get(Jobj, "LocalIPMode")) == 0)
		json_object_object_add(IPSecVPNJobj, "LocalIPMode", json_object_new_string("Subnet"));
	else
		json_object_object_add(IPSecVPNJobj, "LocalIPMode", json_object_new_string("Single Address"));

	if(json_object_get_int(json_object_object_get(Jobj, "RemoteIPMode")) == 0)
		json_object_object_add(IPSecVPNJobj, "RemoteIPMode", json_object_new_string("Subnet"));
	else
		json_object_object_add(IPSecVPNJobj, "RemoteIPMode", json_object_new_string("Single Address"));

	pramJobj = json_object_object_get(Jobj, "LocalIPAddress");
	json_object_object_add(IPSecVPNJobj, "LocalIPAddress", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "LocalIPMask");
	json_object_object_add(IPSecVPNJobj, "LocalIPMask", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "RemoteIPAddress");
	json_object_object_add(IPSecVPNJobj, "RemoteIPAddress", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "RemoteIPMask");
	json_object_object_add(IPSecVPNJobj, "RemoteIPMask", JSON_OBJ_COPY(pramJobj));
	if(json_object_get_int(json_object_object_get(Jobj, "Protocol")) == 0)
		json_object_object_add(IPSecVPNJobj, "Protocol", json_object_new_string("esp"));
	else
		json_object_object_add(IPSecVPNJobj, "Protocol", json_object_new_string("ah"));
	pramJobj = json_object_object_get(Jobj, "KeyExchaMode");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthType", JSON_OBJ_COPY(pramJobj));//0:Auto(IKEv1), 1:Manunal
	if(json_object_get_int(json_object_object_get(Jobj, "KeyExchaMode")) == 0){
		pramJobj = json_object_object_get(Jobj, "IKEv1Phase1Mode");
		json_object_object_add(IPSecVPNJobj, "IKEv1Phase1Mode", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1EncrypAlgori");
		json_object_object_add(IPSecVPNJobj, "IKEv1EncrypAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1IntegrityAlgori");
		json_object_object_add(IPSecVPNJobj, "IKEv1IntegrityAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1SelectKeyExcha");
		json_object_object_add(IPSecVPNJobj, "IKEv1SelectKeyExcha", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1KeyLifeTime");
		json_object_object_add(IPSecVPNJobj, "IKEv1KeyLifeTime", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1EncrypAlgori2");
		json_object_object_add(IPSecVPNJobj, "IKEv1EncrypAlgori2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1IntegrityAlgori2");
		json_object_object_add(IPSecVPNJobj, "IKEv1IntegrityAlgori2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1SelectKeyExcha2");
		json_object_object_add(IPSecVPNJobj, "IKEv1SelectKeyExcha2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1KeyLifeTime2");
		json_object_object_add(IPSecVPNJobj, "IKEv1KeyLifeTime2", JSON_OBJ_COPY(pramJobj));
	}
	else{
		pramJobj = json_object_object_get(Jobj, "ManualEncrypAlgori");
		json_object_object_add(IPSecVPNJobj, "ManualEncrypAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualEncrypKey");
		json_object_object_add(IPSecVPNJobj, "ManualEncrypKey", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualAuthenAlgori");
		json_object_object_add(IPSecVPNJobj, "ManualAuthenAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualAuthenKey");
		json_object_object_add(IPSecVPNJobj, "ManualAuthenKey", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualSPI");
		json_object_object_add(IPSecVPNJobj, "ManualSPI", JSON_OBJ_COPY(pramJobj));
	}
	pramJobj = json_object_object_get(Jobj, "IKEAuthMode");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_IKEv1XAuthMode", JSON_OBJ_COPY(pramJobj));//0:PreSharedKey, 1:Certificate
	pramJobj = json_object_object_get(Jobj, "NATForward");
	json_object_object_add(IPSecVPNJobj, "NATForward", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "PreSharedKey");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthPreSharedKey", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "Certificate");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthCertificate", JSON_OBJ_COPY(pramJobj));

	if((ret = zcfgFeObjJsonSet(RDM_OID_IPSEC_PROFILE, &objIid, IPSecVPNJobj, NULL)) != ZCFG_SUCCESS){
		return ret;
	}
	zcfgFeJsonObjFree(IPSecVPNJobj);
	
	return ret;
}

zcfgRet_t zcfgFeDalIPSecVPNEdit(struct json_object *Jobj, char *replyMsg)
{
 	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *IPSecVPNJobj = NULL;
	struct json_object *pramJobj = NULL;
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	objIndex_t objIid={0};
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
#ifdef ZYXEL_HOME_CYBER_SECURITY_FSECURE	
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_IPSEC_PROFILE, NULL, NULL, replyMsg);
#else	
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_IPSEC_PROFILE, NULL, NULL, replyMsg);
#endif	
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_IPSEC_PROFILE, &objIid, &IPSecVPNJobj)) != ZCFG_SUCCESS) {
		return ret;
	}


	pramJobj = json_object_object_get(Jobj, "Name");
	json_object_object_add(IPSecVPNJobj, "Alias", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "Enable");
	json_object_object_add(IPSecVPNJobj, "Enable", JSON_OBJ_COPY(pramJobj));	
	if((pramJobj = json_object_object_get(Jobj, "RemoteIPSecGateWay")) != NULL)
		json_object_object_add(IPSecVPNJobj, "RemoteEndpoints", JSON_OBJ_COPY(pramJobj));

	if(json_object_get_int(json_object_object_get(Jobj, "LocalIPMode")) == 0)
		json_object_object_add(IPSecVPNJobj, "LocalIPMode", json_object_new_string("Subnet"));
	else
		json_object_object_add(IPSecVPNJobj, "LocalIPMode", json_object_new_string("Single Address"));

	if(json_object_get_int(json_object_object_get(Jobj, "RemoteIPMode")) == 0)
		json_object_object_add(IPSecVPNJobj, "RemoteIPMode", json_object_new_string("Subnet"));
	else
		json_object_object_add(IPSecVPNJobj, "RemoteIPMode", json_object_new_string("Single Address"));

	pramJobj = json_object_object_get(Jobj, "LocalIPAddress");
	json_object_object_add(IPSecVPNJobj, "LocalIPAddress", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "LocalIPMask");
	json_object_object_add(IPSecVPNJobj, "LocalIPMask", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "RemoteIPAddress");
	json_object_object_add(IPSecVPNJobj, "RemoteIPAddress", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "RemoteIPMask");
	json_object_object_add(IPSecVPNJobj, "RemoteIPMask", JSON_OBJ_COPY(pramJobj));
	if(json_object_get_int(json_object_object_get(Jobj, "Protocol")) == 0)
		json_object_object_add(IPSecVPNJobj, "Protocol", json_object_new_string("esp"));
	else
		json_object_object_add(IPSecVPNJobj, "Protocol", json_object_new_string("ah"));
	pramJobj = json_object_object_get(Jobj, "KeyExchaMode");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthType", JSON_OBJ_COPY(pramJobj));//0:Auto(IKEv1), 1:Manunal
	if(json_object_get_int(json_object_object_get(Jobj, "KeyExchaMode")) == 0){
		pramJobj = json_object_object_get(Jobj, "IKEv1Phase1Mode");
		json_object_object_add(IPSecVPNJobj, "IKEv1Phase1Mode", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1EncrypAlgori");
		json_object_object_add(IPSecVPNJobj, "IKEv1EncrypAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1IntegrityAlgori");
		json_object_object_add(IPSecVPNJobj, "IKEv1IntegrityAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1SelectKeyExcha");
		json_object_object_add(IPSecVPNJobj, "IKEv1SelectKeyExcha", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1KeyLifeTime");
		json_object_object_add(IPSecVPNJobj, "IKEv1KeyLifeTime", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1EncrypAlgori2");
		json_object_object_add(IPSecVPNJobj, "IKEv1EncrypAlgori2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1IntegrityAlgori2");
		json_object_object_add(IPSecVPNJobj, "IKEv1IntegrityAlgori2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1SelectKeyExcha2");
		json_object_object_add(IPSecVPNJobj, "IKEv1SelectKeyExcha2", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "IKEv1KeyLifeTime2");
		json_object_object_add(IPSecVPNJobj, "IKEv1KeyLifeTime2", JSON_OBJ_COPY(pramJobj));
	}
	else{
		pramJobj = json_object_object_get(Jobj, "ManualEncrypAlgori");
		json_object_object_add(IPSecVPNJobj, "ManualEncrypAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualEncrypKey");
		json_object_object_add(IPSecVPNJobj, "ManualEncrypKey", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualAuthenAlgori");
		json_object_object_add(IPSecVPNJobj, "ManualAuthenAlgori", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualAuthenKey");
		json_object_object_add(IPSecVPNJobj, "ManualAuthenKey", JSON_OBJ_COPY(pramJobj));
		pramJobj = json_object_object_get(Jobj, "ManualSPI");
		json_object_object_add(IPSecVPNJobj, "ManualSPI", JSON_OBJ_COPY(pramJobj));
	}
	pramJobj = json_object_object_get(Jobj, "IKEAuthMode");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_IKEv1XAuthMode", JSON_OBJ_COPY(pramJobj));//0:PreSharedKey, 1:Certificate
	pramJobj = json_object_object_get(Jobj, "NATForward");
	json_object_object_add(IPSecVPNJobj, "NATForward", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "PreSharedKey");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthPreSharedKey", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "Certificate");
	json_object_object_add(IPSecVPNJobj, "X_ZYXEL_AuthCertificate", JSON_OBJ_COPY(pramJobj));


	if((ret = zcfgFeObjJsonSet(RDM_OID_IPSEC_PROFILE, &objIid, IPSecVPNJobj, NULL)) != ZCFG_SUCCESS);

	json_object_put(IPSecVPNJobj);
	return ret;
}

zcfgRet_t zcfgFeDalIPSecVPNDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex;
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));

	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_IPSEC_PROFILE, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	ret = zcfgFeObjJsonDel(RDM_OID_IPSEC_PROFILE, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SchedulerRule.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret; 
	}
	return ret;
}

zcfgRet_t zcfgFeDalIPSecVPN(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalIPSecVPNEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalIPSecVPNAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalIPSecVPNDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalIPSecVPNGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

