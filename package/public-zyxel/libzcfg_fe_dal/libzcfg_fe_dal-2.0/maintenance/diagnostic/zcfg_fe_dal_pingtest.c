
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zlog_api.h"
#include "zcfg_fe_dal.h"

#define IP_DIAG_IPPING 0
#define IP_DIAG_TRACE_RT 1
#define DNS_DIAG_NS_LOOKUP 2

dal_param_t PING_TEST_param[]={
	{"type",								dalType_int,	0,	0,	NULL},
	{"ProtocolVersion",						dalType_string,	0,	0,	NULL},
	{"Host",								dalType_string,	0,	0,	NULL},
	{"DiagnosticsState",					dalType_string,	0,	0,	NULL},
	{"NumberOfRepetitions",					dalType_int,	0,	0,	NULL},
	{"HostName",							dalType_string,	0,	0,	NULL},
	{"DNSServer",							dalType_string,	0,	0,	NULL},
	{NULL,									0,				0,	0,	NULL},
};

unsigned int type;
const char* ProtocolVersion;
const char* Host;
const char* DiagnosticsState;
unsigned int NumberOfRepetitions;
const char* HostName;
const char* DNSServer;
bool invalidInput = false;

struct json_object *diagIPPingJobj;
struct json_object *diagTraceRTobj;
struct json_object *dnsNSLookUpobj;

objIndex_t IpDiagIPPingIid;
objIndex_t IpDiagTraceRTIid;
objIndex_t DNSDiagNSLookUpIid;

void freeAllPingTestObjects(){
	if(diagIPPingJobj) json_object_put(diagIPPingJobj);
	if(diagTraceRTobj) json_object_put(diagTraceRTobj);
	if(dnsNSLookUpobj) json_object_put(dnsNSLookUpobj);
	return;
}

void initPingTestGlobalObjects(){
	diagIPPingJobj = NULL;
	diagTraceRTobj = NULL;
	dnsNSLookUpobj = NULL;
	return;
}

void getPingTestType(struct json_object *Jobj){
	if( NULL != json_object_object_get(Jobj, "type") ){
		type = json_object_get_int(json_object_object_get(Jobj, "type"));
	}
	/* After we use Nslookup or Traceroute function, the global variable "type" will be set to 2 or 1 respectively and saved,it will lead to the later "double free" and zhttp restart issue if we go back to homepage.
	So we should set the global variable "type" to 0 (default) after some operations.*/
	else{
		if(type!=0){
			type = 0;
		}
	}
}

void getIpDiagIPPingBasicInfo(struct json_object *Jobj){
	if( NULL != json_object_object_get(Jobj, "ProtocolVersion") ){
		ProtocolVersion = json_object_get_string(json_object_object_get(Jobj, "ProtocolVersion"));
	}
	if( NULL != json_object_object_get(Jobj, "Host") ){
		Host = json_object_get_string(json_object_object_get(Jobj, "Host"));
		invalidCharFilter((char *)Host, 41);
		if(DomainFilter(Host) == true || 
			IPFilter(Host) == true ||
			IPv6Filter(Host) == true){
			invalidInput = false;
		}
		else
			invalidInput = true;
	}
	if( NULL != json_object_object_get(Jobj, "DiagnosticsState") ){
		DiagnosticsState = json_object_get_string(json_object_object_get(Jobj, "DiagnosticsState"));
	}
	if( NULL != json_object_object_get(Jobj, "NumberOfRepetitions") ){
		NumberOfRepetitions = json_object_get_int(json_object_object_get(Jobj, "NumberOfRepetitions"));
	}
}

void getIpDiagTraceRTBasicInfo(struct json_object *Jobj){
	if( NULL != json_object_object_get(Jobj, "ProtocolVersion") ){
		ProtocolVersion = json_object_get_string(json_object_object_get(Jobj, "ProtocolVersion"));
	}
	if( NULL != json_object_object_get(Jobj, "Host") ){
		Host = json_object_get_string(json_object_object_get(Jobj, "Host"));
		invalidCharFilter((char *)Host, 41);
		if(DomainFilter(Host) == true || 
			IPFilter(Host) == true ||
			IPv6Filter(Host) == true){
			invalidInput = false;
		}
		else
			invalidInput = true;
	}
	if( NULL != json_object_object_get(Jobj, "DiagnosticsState") ){
		DiagnosticsState = json_object_get_string(json_object_object_get(Jobj, "DiagnosticsState"));
	}
	if( NULL != json_object_object_get(Jobj, "NumberOfRepetitions") ){
		NumberOfRepetitions = json_object_get_int(json_object_object_get(Jobj, "NumberOfRepetitions"));
	}
}

void getDNSDiagNSLookUpgBasicInfo(struct json_object *Jobj){
	if( NULL != json_object_object_get(Jobj, "HostName") ){
		HostName = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	}else if( NULL != json_object_object_get(Jobj, "Host") ){
		HostName = json_object_get_string(json_object_object_get(Jobj, "Host"));
	}
	invalidCharFilter((char *)HostName, 41);
	if(DomainFilter(HostName) == true || 
			IPFilter(HostName) == true ||
			IPv6Filter(HostName) == true){
			invalidInput = false;
	}
	else
		invalidInput = true;

	if( NULL != json_object_object_get(Jobj, "DiagnosticsState") ){
		DiagnosticsState = json_object_get_string(json_object_object_get(Jobj, "DiagnosticsState"));
	}
	if( NULL != json_object_object_get(Jobj, "DiagnosticsState") ){
		DNSServer = json_object_get_string(json_object_object_get(Jobj, "DNSServer"));
	}
}

zcfgRet_t zcfgFeDalIpDiagIPPingEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO("Enter.. \n ");
	getIpDiagIPPingBasicInfo(Jobj);
	if(invalidInput){
		printf("\ninvalid input...\n");
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(IpDiagIPPingIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, &diagIPPingJobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get ip daig ip ping Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(diagIPPingJobj != NULL){
		json_object_object_add(diagIPPingJobj, "ProtocolVersion", json_object_new_string(ProtocolVersion));
		json_object_object_add(diagIPPingJobj, "Host", json_object_new_string(Host));
		json_object_object_add(diagIPPingJobj, "DiagnosticsState", json_object_new_string(DiagnosticsState));
		json_object_object_add(diagIPPingJobj, "NumberOfRepetitions", json_object_new_int(NumberOfRepetitions));

		//if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, diagIPPingJobj, NULL)) != ZCFG_SUCCESS)
		const char *ipping;
		ipping = json_object_to_json_string(diagIPPingJobj);
		if((ret = zcfgFeObjWrite(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, (char *)ipping, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS)
		{
			ZLOG_INFO("(ERROR) Set ip daig ip ping fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalIpDiagTraceRTEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO("Enter.. \n ");
	getIpDiagTraceRTBasicInfo(Jobj);
	if(invalidInput){
		printf("\ninvalid input...\n");
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(IpDiagTraceRTIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_TRACE_RT, &IpDiagTraceRTIid, &diagTraceRTobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get ip daig trace RT Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(diagTraceRTobj != NULL){
		json_object_object_add(diagTraceRTobj, "ProtocolVersion", json_object_new_string(ProtocolVersion));
		json_object_object_add(diagTraceRTobj, "Host", json_object_new_string(Host));
		json_object_object_add(diagTraceRTobj, "DiagnosticsState", json_object_new_string(DiagnosticsState));
		json_object_object_add(diagTraceRTobj, "NumberOfRepetitions", json_object_new_int(NumberOfRepetitions));

		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_DIAG_TRACE_RT, &IpDiagTraceRTIid, diagTraceRTobj, NULL)) != ZCFG_SUCCESS)
		{
			ZLOG_INFO("(ERROR) Set ip daig trace RT fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalDNSDiagNSLookUpEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO("Enter.. \n ");
	getDNSDiagNSLookUpgBasicInfo(Jobj);
	if(invalidInput){
		printf("\ninvalid input...\n");
		return ZCFG_INTERNAL_ERROR;
	}


	IID_INIT(DNSDiagNSLookUpIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &DNSDiagNSLookUpIid, &dnsNSLookUpobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get DNS diag NS Lookup Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(dnsNSLookUpobj != NULL){
		if( (NULL != json_object_object_get(Jobj, "HostName")) || (NULL != json_object_object_get(Jobj, "Host"))){
			json_object_object_add(dnsNSLookUpobj, "HostName", json_object_new_string(HostName));
		}
		if( NULL != json_object_object_get(Jobj, "DiagnosticsState")){
			json_object_object_add(dnsNSLookUpobj, "DiagnosticsState", json_object_new_string(DiagnosticsState));
		}
		if( NULL != json_object_object_get(Jobj, "DNSServer")){
			json_object_object_add(dnsNSLookUpobj, "DNSServer", json_object_new_string(DNSServer));
		}
		if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &DNSDiagNSLookUpIid, dnsNSLookUpobj, NULL)) != ZCFG_SUCCESS)
		{
			ZLOG_INFO("(ERROR) Set DNS diag NS Lookup fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalPingTestEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO("input --- %s \n ",json_object_to_json_string(Jobj));

	initPingTestGlobalObjects();
	getPingTestType(Jobj);

	if(type == IP_DIAG_IPPING ){
		if((ret = zcfgFeDalIpDiagIPPingEdit(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("config I pDiag IP Ping fail\n");
			goto exit;
		}
	}else if(type == IP_DIAG_TRACE_RT){
		if((ret = zcfgFeDalIpDiagTraceRTEdit(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("config Ip Diag Trace RT fail\n");
			goto exit;
		}
	}else if(type == DNS_DIAG_NS_LOOKUP){
		if((ret = zcfgFeDalDNSDiagNSLookUpEdit(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("config DNS Diag NSL ookUp fail\n");
			goto exit;
		}
	}

exit:
	freeAllPingTestObjects();
	return ret;
}

zcfgRet_t zcfgFeDalPingTestGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	 ZLOG_INFO("input --- %s \n ",json_object_to_json_string(Jobj));
	struct json_object *paramJobj = NULL;
	paramJobj = json_object_new_object();

	getPingTestType(Jobj);

	if(type == IP_DIAG_IPPING ){
		IID_INIT(IpDiagIPPingIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_IPPING, &IpDiagIPPingIid, &diagIPPingJobj)) != ZCFG_SUCCESS) {
			ZLOG_INFO("(ERROR) Get ip daig ip ping Obj fail\n");
			ret = ZCFG_INTERNAL_ERROR;
			goto exit;
		}
		json_object_object_add(paramJobj, "DiagnosticsState", JSON_OBJ_COPY(json_object_object_get(diagIPPingJobj, "DiagnosticsState")));
	}else if(type == IP_DIAG_TRACE_RT){
		IID_INIT(IpDiagTraceRTIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_IP_DIAG_TRACE_RT, &IpDiagTraceRTIid, &diagTraceRTobj)) != ZCFG_SUCCESS) {
			ZLOG_INFO("(ERROR) Get ip daig trace RT Obj fail\n");
			ret = ZCFG_INTERNAL_ERROR;
			goto exit;
		}
		json_object_object_add(paramJobj, "DiagnosticsState", JSON_OBJ_COPY(json_object_object_get(diagTraceRTobj, "DiagnosticsState")));
	}else if(type == DNS_DIAG_NS_LOOKUP){
		IID_INIT(DNSDiagNSLookUpIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_DNS_DIAG_NS_LOOKUP_DIAG, &DNSDiagNSLookUpIid, &dnsNSLookUpobj)) != ZCFG_SUCCESS) {
			ZLOG_INFO("(ERROR) Get DNS diag NS Lookup Obj fail\n");
			ret = ZCFG_INTERNAL_ERROR;
			goto exit;
		}
		json_object_object_add(paramJobj, "DiagnosticsState", JSON_OBJ_COPY(json_object_object_get(dnsNSLookUpobj, "DiagnosticsState")));
	}
	json_object_array_add(Jarray, paramJobj);
exit:
	freeAllPingTestObjects();
	return ret;
}

zcfgRet_t zcfgFeDalPingTest(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalPingTestEdit(Jobj, NULL);
	}

	if(!strcmp(method, "GET")) {
		ret = zcfgFeDalPingTestGet(Jobj, Jarray, NULL);
	}

	return ret;
}

