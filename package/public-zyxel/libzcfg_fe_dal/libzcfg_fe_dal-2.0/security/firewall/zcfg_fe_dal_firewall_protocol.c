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

dal_param_t FIREWALL_PROTOCOL_param[] =
{
	{"Index", 				dalType_int, 		0, 		0, 		NULL,	NULL,	dal_Edit|dal_Delete},
	{"Name", 				dalType_string,		0,		0,		NULL,	NULL,	dal_Add},
	{"Description", 		dalType_string,		0,		0,		NULL,	NULL,	dal_Add},
	{"Protocol", 			dalType_string, 	0, 		0, 		NULL,	"TCP|UDP|ICMP|ICMPv6|Other",	dal_Add},
	{"SourcePort", 			dalType_int, 		-1, 	65535, 	NULL,	NULL,	0},
	{"SourcePortEnd", 	dalType_int, 		-1, 	65535, 	NULL,	NULL,	0},
	{"DestPort", 			dalType_int, 		-1, 	65535, 	NULL,	NULL,	0},
	{"DestPortEnd", 	dalType_int, 		-1, 	65535, 	NULL,	NULL,	0},
	{"ICMPTypeCode", 		dalType_int, 		-1, 	16,		NULL,	NULL, 	dalcmd_Forbid},
	{"ICMPType", 			dalType_int, 	    -1, 	143, 	NULL,	NULL,	dalcmd_Forbid},
	{"ICMPv6Type", 			dalType_ICMPv6, 	0, 		0, 		NULL,	NULL,	0},
	{"ProtocolNumber", 		dalType_int, 		0, 		255, 	NULL,	NULL,	0},
	{NULL, 					0, 					0, 		0, 		NULL,	NULL,	0}
};

void zcfgFeDalShowFirewallProtocol(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	int protocol = 0, sourceport = 0, sourceportrange = 0, destport = 0, destportrange = 0;
	char procotolport[256] = {0}, tmp1[32] = {0}, tmp2[32] = {0};
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-10s %-15s %-30s \n","Index","Name","Description","Ports/Protocol Number");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		procotolport[0] = '\0';
		tmp1[0] = '\0';
		tmp2[0] = '\0';
		obj = json_object_array_get_idx(Jarray, i);
		protocol = json_object_get_int(json_object_object_get(obj, "Protocol"));
		sourceport = json_object_get_int(json_object_object_get(obj, "SourcePort"));
		sourceportrange= json_object_get_int(json_object_object_get(obj, "SourcePortEnd"));
		destport = json_object_get_int(json_object_object_get(obj, "DestPort"));
		destportrange = json_object_get_int(json_object_object_get(obj, "DestPortEnd"));
		if(protocol == 1){			// ICMP
			strcat(procotolport,"ICMP");
		}
		else if(protocol == 6){		//TCP
			if((sourceport == -1) && (sourceportrange == -1)){
				strcat(tmp1,"TCP: Any Port-->");
			}
			else if((sourceport != -1) && (sourceportrange == -1)){
				sprintf(tmp1,"TCP: %d-->",sourceport);
			}
			else{ 
				sprintf(tmp1,"TCP: %d:%d-->",sourceport,sourceportrange);
			}
			if((destport == -1) && (destportrange == -1)){
				strcat(tmp2,"Any Port");
			}
			else if((destport != -1) && (destportrange == -1)){
				sprintf(tmp2,"%d",destport);
			}
			else{
				sprintf(tmp2,"%d:%d",destport,destportrange);
			}
			strcat(procotolport,tmp1);
			strcat(procotolport,tmp2);
		}
		else if(protocol == 17){	//UDP
			if((sourceport == -1) && (sourceportrange == -1)){
				strcat(tmp1,"UDP: Any Port-->");
			}
			else if((sourceport != -1) && (sourceportrange == -1)){
				sprintf(tmp1,"UDP: %d-->",sourceport);
			}
			else{ 
				sprintf(tmp1,"UDP: %d:%d-->",sourceport,sourceportrange);
			}
			if((destport == -1) && (destportrange == -1)){
				strcat(tmp2,"Any Port");
			}
			else if((destport != -1) && (destportrange == -1)){
				sprintf(tmp2,"%d",destport);
			}
			else{ 
				sprintf(tmp2,"%d:%d",destport,destportrange);
			}
			strcat(procotolport,tmp1);
			strcat(procotolport,tmp2);
		}
		else if(protocol == 1){	//ICMP
			strcat(procotolport,"ICMP:");
			strcat(procotolport,json_object_get_string(json_object_object_get(obj, "ICMPType")));
		}
		else if(protocol == 58){	//ICMPv6
			strcat(procotolport,"ICMPv6:");
			strcat(procotolport,json_object_get_string(json_object_object_get(obj, "ICMPv6Type")));
		}
		else{						//Other
			sprintf(procotolport,"Other:%d",protocol);
		}
		printf("%-10s %-10s %-15s %-30s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Name")),
			json_object_get_string(json_object_object_get(obj, "Description")),
			procotolport);
	}
	
}


zcfgRet_t zcfgFeDal_Firewall_Protocol_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *protocolObj = NULL;
	objIndex_t protocolIid = {0};
	zcfg_offset_t oid;
	const char *name = NULL;
	const char *description = NULL;
	int sourceport = -1;
	int sourcePortRangeMax = -1;
	int destPort = -1;
	int destPortRangeMax = -1;
	char *protocol = NULL;
	int ICMPType = -1;
	int ICMPTypeCode = -1;
	int protocolNum = 0;

	IID_INIT(protocolIid);
	name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	oid = RDM_OID_NETWORK_SRV_GP;
	ret = dalcmdParamCheck(NULL, name, oid, "Name", "Name",  replyMsg);
	if(ret != ZCFG_SUCCESS){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
		return ret;
	}
		if(json_object_object_get(Jobj, "SourcePort"))
		sourceport = json_object_get_int(json_object_object_get(Jobj, "SourcePort"));
		else
			sourceport = -1;
		if(json_object_object_get(Jobj, "SourcePortEnd"))
			sourcePortRangeMax = json_object_get_int(json_object_object_get(Jobj, "SourcePortEnd"));
		else 
			sourcePortRangeMax = -1;
		if(json_object_object_get(Jobj, "DestPort"))
		destPort = json_object_get_int(json_object_object_get(Jobj, "DestPort"));
		else
			destPort = -1;
		if(json_object_object_get(Jobj, "DestPortEnd"))
			destPortRangeMax = json_object_get_int(json_object_object_get(Jobj, "DestPortEnd"));
		else
			destPortRangeMax = -1;
		protocol = (char *)json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		if(!strcmp(protocol,"TCP")){
			protocolNum = 6;
		}
		else if(!strcmp(protocol,"UDP")){
			protocolNum = 17;
		}
		else if(!strcmp(protocol,"ICMP")){
			protocolNum = 1;
		}
		else if(!strcmp(protocol,"ICMPv6")){
			protocolNum = 58;
		}
		else if(!strcmp(protocol,"Other")){
			protocolNum = json_object_get_int(json_object_object_get(Jobj, "ProtocolNumber"));
			if(!json_object_object_get(Jobj, "ProtocolNumber")){
				strcpy(replyMsg, "Protocol Number is empty.");
				return ZCFG_INVALID_PARAM_VALUE;
			}
		}
	ret = zcfgFeObjJsonAdd(RDM_OID_NETWORK_SRV_GP, &protocolIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_NETWORK_SRV_GP, &protocolIid, &protocolObj);
	}
		
	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "ICMPType"))
			ICMPType = json_object_get_int(json_object_object_get(Jobj, "ICMPType"));
		if(json_object_object_get(Jobj, "ICMPTypeCode"))
			ICMPTypeCode = json_object_get_int(json_object_object_get(Jobj, "ICMPTypeCode"));
		
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));

		json_object_object_add(protocolObj, "SourcePort", json_object_new_int(sourceport));
		json_object_object_add(protocolObj, "SourcePortRangeMax", json_object_new_int(sourcePortRangeMax));
		json_object_object_add(protocolObj, "DestPort", json_object_new_int(destPort));
		json_object_object_add(protocolObj, "DestPortRangeMax", json_object_new_int(destPortRangeMax));
		json_object_object_add(protocolObj, "Protocol", json_object_new_int(protocolNum));
		if(json_object_object_get(Jobj, "ICMPType"))
			json_object_object_add(protocolObj, "ICMPType", json_object_new_int(ICMPType));
		if(json_object_object_get(Jobj, "ICMPTypeCode"))
			json_object_object_add(protocolObj, "ICMPTypeCode", json_object_new_int(ICMPTypeCode));
		json_object_object_add(protocolObj, "Name", json_object_new_string(name));
		json_object_object_add(protocolObj, "Description", json_object_new_string(description));
		
		ret = zcfgFeObjJsonSet(RDM_OID_NETWORK_SRV_GP, &protocolIid, protocolObj, NULL);
	}

	zcfgFeJsonObjFree(protocolObj);
	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_Protocol_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *protocolObj = NULL;
	objIndex_t protocolIid = {0};
	const char *name = NULL;
	const char *description = NULL;
	int sourceport = -1;
	int sourcePortRangeMax = -1;
	int destPort = -1;
	int destPortRangeMax = -1;
	char *protocol = NULL;
	int ICMPType = -1;
	int ICMPTypeCode = -1;
	int index = 0;
	int protocolNum = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_NETWORK_SRV_GP;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	ret = convertIndextoIid(index, &protocolIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	if(json_object_object_get(Jobj, "Name")){
		ret = dalcmdParamCheck(&protocolIid, name, oid, "Name", "Name", replyMsg);
		if(ret != ZCFG_SUCCESS){
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Common.duplicate_name"));
			return ret;
		}
	}
	if(zcfgFeObjJsonGetWithoutUpdate(oid, &protocolIid, &protocolObj) == ZCFG_SUCCESS){
		sourceport = json_object_get_int(json_object_object_get(Jobj, "SourcePort"));
		sourcePortRangeMax = json_object_get_int(json_object_object_get(Jobj, "SourcePortEnd"));
		destPort = json_object_get_int(json_object_object_get(Jobj, "DestPort"));
		destPortRangeMax = json_object_get_int(json_object_object_get(Jobj, "DestPortEnd"));
		protocol = (char *)json_object_get_string(json_object_object_get(Jobj, "Protocol"));
		if(!strcmp(protocol,"TCP")){
			protocolNum = 6;
		}
		else if(!strcmp(protocol,"UDP")){
			protocolNum = 17;
		}
		else if(!strcmp(protocol,"ICMP")){
			protocolNum = 1;
		}
		else if(!strcmp(protocol,"ICMPv6")){
			protocolNum = 58;
		}
		else if(!strcmp(protocol,"Other")){
			protocolNum = json_object_get_int(json_object_object_get(Jobj, "ProtocolNumber"));
			if(!json_object_object_get(Jobj, "ProtocolNumber")){
				strcpy(replyMsg, "Protocol Number is empty.");
				return ZCFG_INVALID_PARAM_VALUE;
		}
	}
		ICMPType = json_object_get_int(json_object_object_get(Jobj, "ICMPType"));
		ICMPTypeCode = json_object_get_int(json_object_object_get(Jobj, "ICMPTypeCode"));
		name = json_object_get_string(json_object_object_get(Jobj, "Name"));
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
		if(json_object_object_get(Jobj, "SourcePort"))
		json_object_object_add(protocolObj, "SourcePort", json_object_new_int(sourceport));
		if(json_object_object_get(Jobj, "SourcePortEnd"))
		json_object_object_add(protocolObj, "SourcePortRangeMax", json_object_new_int(sourcePortRangeMax));
		if(json_object_object_get(Jobj, "DestPort"))
		json_object_object_add(protocolObj, "DestPort", json_object_new_int(destPort));
		if(json_object_object_get(Jobj, "DestPortEnd"))
		json_object_object_add(protocolObj, "DestPortRangeMax", json_object_new_int(destPortRangeMax));
		if(json_object_object_get(Jobj, "Protocol"))
			json_object_object_add(protocolObj, "Protocol", json_object_new_int(protocolNum));
		if(json_object_object_get(Jobj, "ICMPType"))
		json_object_object_add(protocolObj, "ICMPType", json_object_new_int(ICMPType));
		if(json_object_object_get(Jobj, "ICMPTypeCode"))
		json_object_object_add(protocolObj, "ICMPTypeCode", json_object_new_int(ICMPTypeCode));
		if(json_object_object_get(Jobj, "Name"))
		json_object_object_add(protocolObj, "Name", json_object_new_string(name));
		if(json_object_object_get(Jobj, "Description"))
		json_object_object_add(protocolObj, "Description", json_object_new_string(description));
		
		ret = zcfgFeObjJsonSet(RDM_OID_NETWORK_SRV_GP, &protocolIid, protocolObj, NULL);
	}

	zcfgFeJsonObjFree(protocolObj);
	return ret;
}
zcfgRet_t zcfgFeDal_Firewall_Protocol_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t protocolIid = {0};
	int index = 0;
	zcfg_offset_t oid;
	oid = RDM_OID_NETWORK_SRV_GP;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));

	ret = convertIndextoIid(index, &protocolIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	ret = zcfgFeObjJsonDel(oid, &protocolIid, NULL);

	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_Protocol_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *protocolJobj = NULL;
	objIndex_t protocolIid = {0};
	int count = 1,icmptype = 0, icmptypecode = 0, protocol = 0, index = 0;
	char icmpv6[8] = {0};
	IID_INIT(protocolIid);	
	if(json_object_object_get(Jobj, "Index")){
		index =	json_object_get_int(json_object_object_get(Jobj, "Index"));		
		ret = convertIndextoIid(index, &protocolIid, RDM_OID_NETWORK_SRV_GP, NULL, NULL, replyMsg);		
		if(ret != ZCFG_SUCCESS)			
			return ret;		
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_NETWORK_SRV_GP, &protocolIid , &protocolJobj) == ZCFG_SUCCESS){
			paramJobj = json_object_new_object();
			icmptype = json_object_get_int(json_object_object_get(protocolJobj, "ICMPType"));
			icmptypecode = json_object_get_int(json_object_object_get(protocolJobj, "ICMPTypeCode"));
			protocol = json_object_get_int(json_object_object_get(protocolJobj, "Protocol"));
			json_object_object_add(paramJobj, "Iid", json_object_new_int(protocolIid.idx[0]));
			json_object_object_add(paramJobj, "Index", json_object_new_int(index));
			json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Name")));
			json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Description")));
			json_object_object_add(paramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Protocol")));
			json_object_object_add(paramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "SourcePort")));
			json_object_object_add(paramJobj, "SourcePortEnd", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "SourcePortRangeMax")));
			json_object_object_add(paramJobj, "DestPort", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "DestPort")));
			json_object_object_add(paramJobj, "DestPortEnd", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "DestPortRangeMax")));
			json_object_object_add(paramJobj, "ICMPType", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "ICMPType")));
			json_object_object_add(paramJobj, "ICMPTypeCode", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "ICMPTypeCode")));
			if(protocol == 58){	//ICMPv6
				sprintf(icmpv6,"%d,%d",icmptype,icmptypecode);
				json_object_object_add(paramJobj, "ICMPv6Type", json_object_new_string(icmpv6));
			}
			json_object_array_add(Jarray, paramJobj);
			zcfgFeJsonObjFree(protocolJobj);
		}

	}
	else{
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_NETWORK_SRV_GP, &protocolIid, &protocolJobj) == ZCFG_SUCCESS){
			paramJobj = json_object_new_object();
			icmptype = json_object_get_int(json_object_object_get(protocolJobj, "ICMPType"));
			icmptypecode = json_object_get_int(json_object_object_get(protocolJobj, "ICMPTypeCode"));
			protocol = json_object_get_int(json_object_object_get(protocolJobj, "Protocol"));
			json_object_object_add(paramJobj, "Iid", json_object_new_int(protocolIid.idx[0]));
			json_object_object_add(paramJobj, "Index", json_object_new_int(count));
			json_object_object_add(paramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Name")));
			json_object_object_add(paramJobj, "Description", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Description")));
			json_object_object_add(paramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "Protocol")));
			json_object_object_add(paramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "SourcePort")));
			json_object_object_add(paramJobj, "SourcePortEnd", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "SourcePortRangeMax")));
			json_object_object_add(paramJobj, "DestPort", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "DestPort")));
			json_object_object_add(paramJobj, "DestPortEnd", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "DestPortRangeMax")));
			json_object_object_add(paramJobj, "ICMPType", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "ICMPType")));
			json_object_object_add(paramJobj, "ICMPTypeCode", JSON_OBJ_COPY(json_object_object_get(protocolJobj, "ICMPTypeCode")));
			if(protocol == 58){	//ICMPv6
				sprintf(icmpv6,"%d,%d",icmptype,icmptypecode);
				json_object_object_add(paramJobj, "ICMPv6Type", json_object_new_string(icmpv6));
			}
			json_object_array_add(Jarray, paramJobj);
			count++;
			zcfgFeJsonObjFree(protocolJobj);
		}
	}
	return ret;
}

zcfgRet_t zcfgFeDalFirewallProtocol(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Firewall_Protocol_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_Firewall_Protocol_Add(Jobj, replyMsg);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_Firewall_Protocol_Delete(Jobj, NULL);
	}else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_Firewall_Protocol_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

