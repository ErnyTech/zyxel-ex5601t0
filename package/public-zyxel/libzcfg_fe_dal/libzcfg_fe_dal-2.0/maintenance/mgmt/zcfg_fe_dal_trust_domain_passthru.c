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

dal_param_t TRUST_DOMAIN_PASSTHRU_param[] =
{
	{"Enable", 				dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"IPAddress", 			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"SubnetMask", 			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"IPAddressPrefix", 	dalType_IPPrefix,   0, 	0, 	NULL,	NULL, 	dal_Add},
	{"Index", 	 			dalType_int, 	    0, 	0, 	NULL,	NULL,	dal_Delete},
	{NULL, 					0, 				    0, 	0, 	NULL,	NULL, 	0}
};


void zcfgFeDalShowTrustDomainPassthru(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s %-45s \n",
		     "Index", "IP Address");		
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-45s \n",
				json_object_get_string(json_object_object_get(obj, "Index")),
				json_object_get_string(json_object_object_get(obj, "IPAddressPrefix")));
	}
}
zcfgRet_t zcfgFeDal_TrustDomainPassthru_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *paramJobj = NULL;
	int Index = 0;
	const char *IPAddress = 0;
	const char *SubnetMask = 0;
	char buf[64] = {0};
	
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU, &iid, &obj) == ZCFG_SUCCESS) {
		Index++;
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(Index));
		IPAddress = json_object_get_string(json_object_object_get(obj, "IPAddress"));
		SubnetMask = json_object_get_string(json_object_object_get(obj, "SubnetMask"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPAddress")), "AnyIP"))
			sprintf(buf, "%s", IPAddress);
		else
			sprintf(buf, "%s/%s", IPAddress, SubnetMask);
		json_object_object_add(paramJobj, "IPAddressPrefix", json_object_new_string(buf));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(obj);
	}
	
	return ret;
}
zcfgRet_t zcfgFeDal_TrustDomainPassthru_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *trustDomainObj = NULL;
	objIndex_t trustDomainIid = {0};
	struct json_object *paramObj = NULL;
	const char *charVal = NULL;
	char ip[64] = {0};
	char mask[8] = {0};
	bool enable = true;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
	
	IID_INIT(trustDomainIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU, &trustDomainIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU, &trustDomainIid, &trustDomainObj);
	}
	if(ret == ZCFG_SUCCESS){	
		if((paramObj = json_object_object_get(Jobj, "IPAddressPrefix")) != NULL){
			charVal = json_object_get_string(paramObj);
			sscanf(charVal, "%[^/]/%s", ip, mask);
			//printf("SourceIPPrefix ip=%s mask=%s\n", ip, mask);
			json_object_object_add(trustDomainObj, "IPAddress", json_object_new_string(ip));
			json_object_object_add(trustDomainObj, "SubnetMask", json_object_new_string(mask));
			json_object_object_add(trustDomainObj, "Enable", json_object_new_boolean(enable));
		}else
		{
			IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
			SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));
			//printf("SourceIPPrefix ip=%s mask=%s\n", IPAddress, SubnetMask);
			json_object_object_add(trustDomainObj, "IPAddress", json_object_new_string(IPAddress));
			json_object_object_add(trustDomainObj, "SubnetMask", json_object_new_string(SubnetMask));
			enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(trustDomainObj, "Enable", json_object_new_boolean(enable));
		}
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU, &trustDomainIid, trustDomainObj, NULL);
		zcfgFeJsonObjFree(trustDomainObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDal_TrustDomainPassthru_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t trustDomainIid = {0};
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));	
	ret = convertIndextoIid(index, &trustDomainIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	ret = zcfgFeObjJsonDel(RDM_OID_REMO_MGMT_TRUST_DOMAIN_IP_PASS_THRU, &trustDomainIid, NULL);
	return ret;

}

zcfgRet_t zcfgFeDalTrustDomainPassthru(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_TrustDomainPassthru_Add(Jobj, NULL);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_TrustDomainPassthru_Delete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_TrustDomainPassthru_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


