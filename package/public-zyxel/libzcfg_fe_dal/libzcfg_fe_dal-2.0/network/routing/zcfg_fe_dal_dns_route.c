
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

dal_param_t DNSROUTE_param[] =
{
	{"Index",								dalType_int,	0,	0,	NULL,	NULL,	dal_Edit|dal_Delete},
	{"Enable",								dalType_boolean,	0,	0,	NULL,	NULL,	dal_Add},
	{"DomainName",							dalType_string,	0,	0,	NULL,	NULL,	dal_Add},
	{"SubnetMask",							dalType_v4Mask,	0,	0,	NULL,	NULL,	dal_Add},
	{"Interface",							dalType_RtWanWWan,	0,	0,	NULL,	NULL,	dal_Add},
	{NULL,									0,				0,	0,	NULL},
};

struct json_object *dnsRTObj;

objIndex_t dnsRtIid = {0};


bool Enable;
const char* DomainName;
const char* tempDomainName;
const char* SubnetMask;
const char* Interface;

void zcfgFeDalShowDnsRoute(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-10s %-30s %-15s %-10s \n","Index","Status","Domain Name","WAN Interface","Subnet Mask");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-10s %-30s %-15s %-10s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "DomainName")),
			json_object_get_string(json_object_object_get(obj, "Interface")),
			json_object_get_string(json_object_object_get(obj, "SubnetMask")));

	}
}


void initDnsRouteGlobalObjects(){
	dnsRTObj = NULL;
	return;
}

void freeAllDnsRouteObjects(){
	if(dnsRTObj) json_object_put(dnsRTObj);

	return;
}

void getDnsRouteBasicInfo(struct json_object *Jobj){
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	DomainName = json_object_get_string(json_object_object_get(Jobj, "DomainName"));
	SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));
	Interface = json_object_get_string(json_object_object_get(Jobj, "Interface"));

	return;
}

void zcfgFeDalDnsRouteObjIidGet(struct json_object *Jobj, char *type){
	const char* path = NULL;
	int dnsRouteNum;

	path = json_object_get_string(json_object_object_get(Jobj, type));
	//printf("%s(): Edit Dns route path : %s \n ",__FUNCTION__,path);

	sscanf(path,"X_ZYXEL_EXT.DNSRouteEntry.%d",&dnsRouteNum);
	//printf("del Dns route - dnsRouteNum: %d, line: %d \n ",dnsRouteNum);
	IID_INIT(dnsRtIid);
	dnsRtIid.level = 1;
	dnsRtIid.idx[0] = dnsRouteNum;

	return;
}

zcfgRet_t zcfgFeDalDnsRouteObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s():--- Edit DnsRoute Json Object --- \n%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DNS_RT_ENTRY, &dnsRtIid, &dnsRTObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Dns route fail oid(%d)\n ",__FUNCTION__,RDM_OID_DNS_RT_ENTRY);
		return ZCFG_INTERNAL_ERROR;
	}

	if(dnsRTObj != NULL){
		if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(dnsRTObj, "Enable", json_object_new_boolean(Enable));
		if(json_object_object_get(Jobj, "DomainName"))
		json_object_object_add(dnsRTObj, "DomainName", json_object_new_string(DomainName));
		if(json_object_object_get(Jobj, "SubnetMask"))
		json_object_object_add(dnsRTObj, "SubnetMask", json_object_new_string(SubnetMask));
		if(json_object_object_get(Jobj, "Interface"))
		json_object_object_add(dnsRTObj, "Interface", json_object_new_string(Interface));
	}
	return ret;
}

zcfgRet_t zcfgFeDalDnsRouteGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
zcfgRet_t zcfgFeDalDnsRouteEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *Jarray = NULL;
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(dnsRtIid);
	oid = RDM_OID_DNS_RT_ENTRY;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	initDnsRouteGlobalObjects();
	getDnsRouteBasicInfo(Jobj);
	Jarray = json_object_new_array();
	zcfgFeDalDnsRouteGet(NULL,Jarray, NULL);
	ret = convert_index_to_iid_for_specified_string(index, &dnsRtIid, oid, "Adder", "Default",replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;	
	if(json_object_object_get(Jobj, "DomainName")){
#ifdef ABUU_CUSTOMIZATION
		ret = paramDuplicateCheck(&dnsRtIid, DomainName, Interface, oid, "DomainName", "DomainName", "Interface", "Interface", replyMsg);
#else
		ret = dalcmdParamCheck(&dnsRtIid, DomainName, oid, "DomainName", "DomainName",  replyMsg);
#endif
		if(ret != ZCFG_SUCCESS)
			return ret;
	}
	if((ret = zcfgFeDalDnsRouteObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit Dns Route \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_RT_ENTRY, &dnsRtIid, dnsRTObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_DNS_RT_ENTRY);
	}

exit:
	freeAllDnsRouteObjects();
	return ret;
}

zcfgRet_t zcfgFeDalDnsRouteAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//struct json_object *Jarray = NULL;
	initDnsRouteGlobalObjects();
	getDnsRouteBasicInfo(Jobj);
	zcfg_offset_t oid;
	oid = RDM_OID_DNS_RT_ENTRY;

	IID_INIT(dnsRtIid);

	/*ret = dalcmdParamCheck(0, DomainName, "DomainName", Jarray, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;*/
#ifdef ABUU_CUSTOMIZATION
	ret = paramDuplicateCheck(NULL, DomainName, Interface, oid, "DomainName", "DomainName", "Interface", "Interface", replyMsg);
#else
	ret = dalcmdParamCheck(NULL, DomainName, oid, "DomainName", "DomainName",  replyMsg);
#endif
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_DNS_RT_ENTRY, &dnsRtIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add Dns Route \n ",__FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalDnsRouteObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit Dns route \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_RT_ENTRY, &dnsRtIid, dnsRTObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_DNS_RT_ENTRY);
	}

exit:
	return ret;
}

zcfgRet_t zcfgFeDalDnsRouteDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(dnsRtIid);
	
	oid = RDM_OID_DNS_RT_ENTRY;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convert_index_to_iid_for_specified_string(index, &dnsRtIid, oid, "Adder", "Default",replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonDel(oid, &dnsRtIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete dns route obj fail \n", __FUNCTION__);
		goto exit;
	}

exit:
    // Frank: it should not need to free objects
	//freeAllDnsRouteObjects();
	return ret;
}

zcfgRet_t zcfgFeDalDnsRouteGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsrouteObj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t dnsrouteIid = {0};
	int count = 1;
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_RT_ENTRY, &dnsrouteIid, &dnsrouteObj) == ZCFG_SUCCESS){
		const char *dnsRoutObjAdder = json_object_get_string(json_object_object_get(dnsrouteObj, "Adder"));
		if(dnsRoutObjAdder && !strcmp(dnsRoutObjAdder, "zcmdmgmtobj"))
			continue;

		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(dnsrouteObj, "Enable")));
		json_object_object_add(paramJobj, "DomainName", JSON_OBJ_COPY(json_object_object_get(dnsrouteObj, "DomainName")));
		json_object_object_add(paramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(dnsrouteObj, "Interface")));
		json_object_object_add(paramJobj, "SubnetMask", JSON_OBJ_COPY(json_object_object_get(dnsrouteObj, "SubnetMask")));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(dnsrouteObj);
		count++;
	}
	
	return ret;

}


zcfgRet_t zcfgFeDalDnsRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalDnsRouteEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalDnsRouteAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalDnsRouteDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalDnsRouteGet(Jobj, Jarray, NULL);

	}

	return ret;
}
