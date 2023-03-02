
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t NAT_CONF_param[] =
{
	//Dmz
	{"PortMappingNumberOfEntries",		dalType_int,		0,	100,	NULL,	NULL,					dalcmd_Forbid},
	{"DmzHost",							dalType_v4Addr,		0,	32,		NULL,	NULL,					0},
#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
	{"DmzHost_Enable",			dalType_boolean,	0,	0,		NULL,	NULL,					0},
#endif
	//Alg
	{"Nat",								dalType_boolean,	0,	0,		NULL,	NULL,					0},
	{"Sip",								dalType_boolean,	0,	0,		NULL,	NULL,					0},
	{"Rtsp",							dalType_boolean,	0,	0,		NULL,	NULL,					0},
	{"Pptp",							dalType_boolean,	0,	0,		NULL,	NULL,					0},
	{"Ipsec",							dalType_boolean,	0,	0,		NULL,	NULL,					0},
	//Session
	{"MaxSessionPerSystem",				dalType_int,		0,	20480,	NULL,	NULL,					dalcmd_Forbid},
	{"MaxSessionPerHost",				dalType_int,		0,	20480,	NULL,	NULL,					0},
	{NULL,								0,					0,	0,		NULL,	NULL,					0},	
};

zcfgRet_t zcfgFeDalNatConfGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *natDmzAlgJobj = NULL;
	struct json_object *natSessionJobj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t objIid;
	objIndex_t subnetIid;
	struct json_object *subnetObj = NULL;
	struct json_object *subnetparamJobj = NULL;
	struct json_object *subnetInfoObj = NULL;

	IID_INIT(objIid);
	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &natDmzAlgJobj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "DMZHost", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_DMZHost")));
		json_object_object_add(paramJobj, "NAT_ALG", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_NAT_ALG")));
		json_object_object_add(paramJobj, "SIP_ALG", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_SIP_ALG")));
		json_object_object_add(paramJobj, "RTSP_ALG", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_RTSP_ALG")));
		json_object_object_add(paramJobj, "PPTP_ALG", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_PPTP_ALG")));
		json_object_object_add(paramJobj, "IPSEC_ALG", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_IPSEC_ALG")));
		#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
		json_object_object_add(paramJobj, "DmzHost_Enable", JSON_OBJ_COPY(json_object_object_get(natDmzAlgJobj, "X_ZYXEL_DMZHost_Enable")));
		#endif
		zcfgFeJsonObjFree(natDmzAlgJobj);		
	}

	IID_INIT(objIid);
	if(zcfgFeObjJsonGet(RDM_OID_NAT_SESSION_CTL, &objIid, &natSessionJobj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "MaxSessionPerHost", JSON_OBJ_COPY(json_object_object_get(natSessionJobj, "MaxSessionPerHost")));
#ifdef ABOG_CUSTOMIZATION
				json_object_object_add(paramJobj, "TcpEstablishedConnectionTimeout", JSON_OBJ_COPY(json_object_object_get(natSessionJobj, "TcpEstablishedConnectionTimeout")));
				json_object_object_add(paramJobj, "UdpConnectionTimeout", JSON_OBJ_COPY(json_object_object_get(natSessionJobj, "UdpConnectionTimeout")));
#endif
		zcfgFeJsonObjFree(natSessionJobj);
	}


	subnetInfoObj = json_object_new_array();
	IID_INIT(subnetIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_SRV_SUBNET, &subnetIid, &subnetObj) == ZCFG_SUCCESS){

		if(strstr(json_object_get_string(json_object_object_get(subnetObj, "Interface")),"WiFi.SSID.") && 
			json_object_get_boolean(json_object_object_get(subnetObj, "Enable"))){
			subnetparamJobj = json_object_new_object();
			json_object_object_add(subnetparamJobj, "MinAddress", JSON_OBJ_COPY(json_object_object_get(subnetObj, "MinAddress")));
			json_object_object_add(subnetparamJobj, "SubnetMask", JSON_OBJ_COPY(json_object_object_get(subnetObj, "SubnetMask")));
			json_object_array_add(subnetInfoObj, subnetparamJobj);
		}
		zcfgFeJsonObjFree(subnetObj);
	}
	json_object_object_add(paramJobj, "subnetInfo", subnetInfoObj);
	subnetInfoObj = NULL;

	json_object_array_add(Jarray, paramJobj);
	
	return ret;
}

void zcfgFeDalShowNatConf(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("Dmz Configuration\n");
		printf("%-50s %s \n","Default Server Address :", json_object_get_string(json_object_object_get(obj, "DMZHost")));
		printf("\nAlg Configuration\n");
		printf("%-50s %d \n","NAT ALG :", json_object_get_boolean(json_object_object_get(obj, "NAT_ALG")));
		printf("%-50s %d \n","SIP ALG :", json_object_get_boolean(json_object_object_get(obj, "SIP_ALG")));
		printf("%-50s %d \n","RTSP ALG :", json_object_get_boolean(json_object_object_get(obj, "RTSP_ALG")));
		printf("%-50s %d \n","PPTP ALG :", json_object_get_boolean(json_object_object_get(obj, "PPTP_ALG")));
		printf("%-50s %d \n","IPSEC ALG :", json_object_get_boolean(json_object_object_get(obj, "IPSEC_ALG")));
		#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
		printf("%-50s %d \n","X_ZYXEL_DMZHost_Enable :", json_object_get_boolean(json_object_object_get(obj, "DmzHost_Enable")));
		#endif
		printf("\nSession Configuration\n");
		printf("%-50s %s \n","MAX NAT Session Per Host (0 - 20480) :", json_object_get_string(json_object_object_get(obj, "MaxSessionPerHost")));
	}
}

zcfgRet_t zcfgFeDalNatConfEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *natDmzAlgJobj = NULL;
	struct json_object *natSessionJobj = NULL;
	bool nat = false,sip = false, rtsp = false, pptp = false, ipsec = false;
#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
	bool dmzhost_enable = false;
#endif
	const char *dmzhost;
	int host = 0;
	
	IID_INIT(objIid);
	ret = zcfgFeObjJsonGet(RDM_OID_NAT, &objIid, &natDmzAlgJobj);
	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "DmzHost") != NULL){
			dmzhost = json_object_get_string(json_object_object_get(Jobj, "DmzHost"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_DMZHost", json_object_new_string(dmzhost));
		}
		#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
		if(json_object_object_get(Jobj, "DmzHost_Enable") != NULL){
			dmzhost_enable = json_object_get_boolean(json_object_object_get(Jobj, "DmzHost_Enable"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_DMZHost_Enable", json_object_new_boolean(dmzhost_enable));
		}
		#endif
		if(json_object_object_get(Jobj, "PortMappingNumberOfEntries") != NULL){
			json_object_object_add(natDmzAlgJobj, "PortMappingNumberOfEntries", JSON_OBJ_COPY(json_object_object_get(Jobj, "PortMappingNumberOfEntries")));
		}

		if(json_object_object_get(Jobj, "Nat") != NULL){
			nat = json_object_get_boolean(json_object_object_get(Jobj, "Nat"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_NAT_ALG", json_object_new_boolean(nat));
		}

		if(json_object_object_get(Jobj, "Sip") != NULL){
			sip = json_object_get_boolean(json_object_object_get(Jobj, "Sip"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_SIP_ALG", json_object_new_boolean(sip));
		}

		if(json_object_object_get(Jobj, "Rtsp") != NULL){
			rtsp = json_object_get_boolean(json_object_object_get(Jobj, "Rtsp"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_RTSP_ALG", json_object_new_boolean(rtsp));
		}

		if(json_object_object_get(Jobj, "Pptp") != NULL){
			pptp = json_object_get_boolean(json_object_object_get(Jobj, "Pptp"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_PPTP_ALG", json_object_new_boolean(pptp));
		}

		if(json_object_object_get(Jobj, "Ipsec") != NULL){
			ipsec = json_object_get_boolean(json_object_object_get(Jobj, "Ipsec"));
			json_object_object_add(natDmzAlgJobj, "X_ZYXEL_IPSEC_ALG", json_object_new_boolean(ipsec));
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &objIid, natDmzAlgJobj, NULL)) != ZCFG_SUCCESS){
			printf("%s: Write object fail\n", __FUNCTION__);
			return ret;
		}
		zcfgFeJsonObjFree(natDmzAlgJobj);
	}

	else{
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(objIid);
	ret = zcfgFeObjJsonGet(RDM_OID_NAT_SESSION_CTL, &objIid, &natSessionJobj);
	if(ret == ZCFG_SUCCESS){		
		if(json_object_object_get(Jobj, "MaxSessionPerHost") != NULL){
			host = json_object_get_int(json_object_object_get(Jobj, "MaxSessionPerHost"));
			json_object_object_add(natSessionJobj, "MaxSessionPerHost", json_object_new_int(host));
		}

		if(json_object_object_get(Jobj, "MaxSessionPerSystem") != NULL){
			json_object_object_add(natSessionJobj, "MaxSessionPerSystem", JSON_OBJ_COPY(json_object_object_get(Jobj, "MaxSessionPerSystem")));
		}
#ifdef ABOG_CUSTOMIZATION
		if(json_object_object_get(Jobj, "TcpEstablishedConnectionTimeout") != NULL){
			json_object_object_add(natSessionJobj, "TcpEstablishedConnectionTimeout", JSON_OBJ_COPY(json_object_object_get(Jobj, "TcpEstablishedConnectionTimeout")));
		}
		
		if(json_object_object_get(Jobj, "UdpConnectionTimeout") != NULL){
			json_object_object_add(natSessionJobj, "UdpConnectionTimeout", JSON_OBJ_COPY(json_object_object_get(Jobj, "UdpConnectionTimeout")));
		}
#endif
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_SESSION_CTL, &objIid, natSessionJobj, NULL)) != ZCFG_SUCCESS){
			printf("%s: Write object fail\n", __FUNCTION__);
			return ret;
		}
		zcfgFeJsonObjFree(natSessionJobj);
	}
	else{
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}
	return ret;
}

zcfgRet_t zcfgFeDalNatConf(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;
	if(!strcmp(method, "PUT")) 
		ret = zcfgFeDalNatConfEdit(Jobj, replyMsg);
	if(!strcmp(method, "GET")) 
		ret = zcfgFeDalNatConfGet(Jobj, Jarray, NULL);
	return ret;
}
