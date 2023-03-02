
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t DNS_ENTRY_param[] =
{
	{"Index", 		dalType_int, 	0, 	0,	 	NULL,	NULL,	dal_Edit|dal_Delete},
	{"HostName", 	dalType_string, 1, 	256, 	NULL,	NULL,	dal_Add},
	{"IPv4Address", dalType_v4Addr,	0,	0,		NULL,	NULL,	dal_Add},	
	{NULL, 			0, 				0, 	0, 		NULL,	NULL,	0}
};
void zcfgFeDalShowDnsEntry(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-20s %-30s %-30s\n",
			"Index", "Host Name", "IP Address");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
		printf("%-20s %-30s %-30s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "HostName")),
			json_object_get_string(json_object_object_get(obj, "IPv4Address")));
	}
		}
		
zcfgRet_t zcfgFeDalDnsEntryAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsEntryObj = NULL;
	objIndex_t dnsEntryIid = {0};
	const char *hostname = NULL;
	const char *IPv4Address = NULL;
	zcfg_offset_t oid;
	oid = RDM_OID_DNS_ENTRY;

	hostname = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	ret = dalcmdParamCheck(NULL, hostname, oid, "HostName", "HostName",  replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if(zcfgFeObjJsonAdd(RDM_OID_DNS_ENTRY, &dnsEntryIid, NULL) == ZCFG_SUCCESS){
		if(zcfgFeObjJsonGet(RDM_OID_DNS_ENTRY, &dnsEntryIid, &dnsEntryObj) == ZCFG_SUCCESS){
			
			IPv4Address = json_object_get_string(json_object_object_get(Jobj, "IPv4Address"));
			json_object_object_add(dnsEntryObj, "HostName", json_object_new_string(hostname));
			json_object_object_add(dnsEntryObj, "IPv4Address", json_object_new_string(IPv4Address));

			ret = zcfgFeObjJsonSet(RDM_OID_DNS_ENTRY, &dnsEntryIid, dnsEntryObj, NULL);
			zcfgFeJsonObjFree(dnsEntryObj);
	}
	}
	return ret;
}
zcfgRet_t zcfgFeDalDnsEntryEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret= ZCFG_SUCCESS;
	struct json_object *dnsEntryObj = NULL;
	objIndex_t dnsEntryIid = {0};
	const char *hostname = NULL;
	const char *IPv4Address = NULL;
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_DNS_ENTRY;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	hostname = json_object_get_string(json_object_object_get(Jobj, "HostName"));

	ret = convertIndextoIid(index, &dnsEntryIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	if(json_object_object_get(Jobj, "HostName")){
		ret = dalcmdParamCheck(&dnsEntryIid, hostname, oid, "HostName", "HostName", replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}
	if(zcfgFeObjJsonGet(oid, &dnsEntryIid, &dnsEntryObj) == ZCFG_SUCCESS){
		IPv4Address = json_object_get_string(json_object_object_get(Jobj, "IPv4Address"));
		if(hostname)
			json_object_object_add(dnsEntryObj, "HostName", json_object_new_string(hostname));
		if(IPv4Address)
			json_object_object_add(dnsEntryObj, "IPv4Address", json_object_new_string(IPv4Address));

		ret = zcfgFeObjJsonSet(oid, &dnsEntryIid, dnsEntryObj, NULL);
		zcfgFeJsonObjFree(dnsEntryObj);
	}
		return ret;
	}
zcfgRet_t zcfgFeDalDnsEntryDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dnsEntryIid = {0};
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_DNS_ENTRY;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));	
	ret = convertIndextoIid(index, &dnsEntryIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	zcfgFeObjJsonDel(RDM_OID_DNS_ENTRY, &dnsEntryIid, NULL);
		
	return ret;
}
zcfgRet_t zcfgFeDalDnsEntryGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsEntryObj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t dnsEntryIid = {0};
	int count = 1;
	while(zcfgFeObjJsonGetNext(RDM_OID_DNS_ENTRY, &dnsEntryIid, &dnsEntryObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "HostName", JSON_OBJ_COPY(json_object_object_get(dnsEntryObj, "HostName")));
		json_object_object_add(paramJobj, "IPv4Address", JSON_OBJ_COPY(json_object_object_get(dnsEntryObj, "IPv4Address")));
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
        if (json_object_object_get(Jobj, "Index")) {
            if(count == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
                json_object_array_add(Jarray, paramJobj);
                zcfgFeJsonObjFree(dnsEntryObj);
                break;
            }
        }
        else {
            json_object_array_add(Jarray, paramJobj);
        }

		zcfgFeJsonObjFree(dnsEntryObj);
		count++;
	}
	return ret;
}
	
zcfgRet_t zcfgFeDalDnsEntry(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalDnsEntryEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalDnsEntryAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalDnsEntryDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method,"GET")){
		ret = zcfgFeDalDnsEntryGet(Jobj, Jarray, NULL);
	}

	return ret;
}
