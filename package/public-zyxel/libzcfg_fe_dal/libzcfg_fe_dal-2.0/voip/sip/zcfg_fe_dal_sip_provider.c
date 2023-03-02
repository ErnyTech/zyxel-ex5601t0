
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zcfg_fe_dal_voip.h"
#include "zos_api.h"

dal_param_t VOIP_PROFILE_param[]={
	{"Index",									dalType_int   , 0,	0,	NULL, NULL, dal_Edit|dal_Delete},
	{"ProfleEnable",							dalType_string,0,	0,	NULL, "Enabled|Disabled"},
	{"ProfName",								dalType_string,	0,	0,	NULL},
	{"UserAgentPort",							dalType_int,	1025,	65535,	NULL},
	{"ProxyServer",								dalType_string,	0,	0,	NULL},
	{"ProxyServerPort",							dalType_int,	1025,	65535,	NULL},
	{"RegistrarServer",							dalType_string,	0,	0,	NULL},
	{"RegistrarServerPort",						dalType_int,	1025,	65535,	NULL},
	{"UserAgentDomain",							dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_ConferenceURI",                                             dalType_string, 0,  0,  NULL},
	{"Rfc3262Support",							dalType_boolean,0,	0,	NULL},
	{"VoipIOPFlags",							dalType_int,	0,	0,	NULL,	NULL,	dalcmd_Forbid},	// need modify, change two parameter of GUI, type: bool //close
	{"ReplaceDialDigit",						dalType_boolean,0,	0,	NULL},	// new parameter for VoipIOPFlag
	{"RemoveRouteHeader",						dalType_boolean,0,	0,	NULL},	// new parameter for VoipIOPFlag
	{"VoipSigIOPFlags",							dalType_int,	0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"BoundIfName",								dalType_string,	0,	0,	NULL, "Any_WAN|Multi_WAN"},
	{"BoundIfList",								dalType_MultiRtWanWWan,	0,	0,	NULL},
	{"OutboundProxy",							dalType_string,	0,	0,	NULL},
	{"OutboundProxyPort",						dalType_int,	1025,	65535,	NULL},
	{"Option120Use",							dalType_boolean,0,	0,	NULL},
	{"LocalPort",								dalType_int,	1026,	65482,	NULL},	// need change parameter name -> LocalPort
	//{"LocalPortMin",							dalType_int,	0,	0,	NULL},	// need change parameter name -> LocalPort
	//{"LocalPortMax",							dalType_int,	0,	0,	NULL},	// need delete and this parameter = LocalPort + 18
	{"SRTPEnable",								dalType_boolean,0,	0,	NULL},
	{"Crypto_Suite_Name",						dalType_string,	0,	0,	NULL, "AES_CM_128_HMAC_SHA1_80|AES_CM_128_HMAC_SHA1_32|AES_CM_128_NULL|NULL_CIPHER_HMAC_SHA1_80"},
	{"DTMFMode",								dalType_string,	0,	0,	NULL, "RFC2833|InBand|SIPInfo"},	// need change parameter name -> DTMFMode
	//{"DTMFMethod",								dalType_string,	0,	0,	NULL, "RFC2833|InBand|SIPInfo"},	// need change parameter name -> DTMFMode
	//{"DTMFMethodG711",							dalType_string,	0,	0,	NULL, "RFC2833|InBand|SIPInfo"},	// value equal as DTMFMethod, delete this parameter and deal in DAL
	{"TransportType",							dalType_string,	0,	0,	NULL, "TCP|UDP|TLS"},	// need change parameter name -> TransportType
	//{"RegistrarServerTransport",				dalType_string,	0,	0,	NULL, "TCP|UDP"},	// need change parameter name -> TransportType
	//{"UserAgentTransport",						dalType_string,	0,	0,	NULL, "TCP|UDP"},	// delete and copy value from RegistrarServerTransport
	//{"ProxyServerTransport",					dalType_string,	0,	0,	NULL, "TCP|UDP"},	// delete and copy value from RegistrarServerTransport
	{"IgnoreDirectIP",							dalType_boolean,0,	0,	NULL},
	{"FaxMode",									dalType_string,	0,	0,	NULL, "G711PassThrough|T38Relay"},
	{"DSCPMark",								dalType_int,	0,	63,	NULL},
	{"RTPDSCPMark",								dalType_int,	0,	63,	NULL},
	{"RegisterExpires",							dalType_int,	20,	65535,	NULL},
	{"RegisterRetryInterval",					dalType_int,	30,	65535,	NULL},
	{"InviteExpires",							dalType_int,	100, 3600,	NULL},
	{"MinSE",									dalType_int,	90,	 1800,	NULL},
	{"InterDigitTimerStd",						dalType_int,	0,	0,	NULL},
	//{"Rfc3263Support",							dalType_boolean,0,	0,	NULL},	//not support CLI yet
	{"DNSSrvEnable",							dalType_boolean,0,	0,	NULL},	
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
	{"DnsMethod",								dalType_string, 0,	0,	NULL, "NAPTR|SRV|BASIC"},
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
	{"FAXPassThrough",							dalType_string,	0,	0,	NULL, "Auto"},	// set in DAL = Auto
	{"ModemPassThrough",						dalType_string,	0,	0,	NULL, "Auto"},	// set in DAL = Auto
	{"CALL_RETURN",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CLIR_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CLIR_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CA_ACTIVATE",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CA_DISABLE",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CW_ACTIVATE",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CW_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"IVR",										dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"INTERNALCALL",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"TRANSFER_ACTIVE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFU_ACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFU_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFNR_ACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFNR_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFB_ACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CFB_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"ONESHOT_CW_ACTIVATE",						dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"ONESHOT_CW_DEACTIVATE",					dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"DND_ENABLE",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"DND_DISABLE",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CCBS_DEACTIVATE",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"OUTGOING_SIP",							dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CALL_REDIAL",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"CALL_REDIAL",								dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{"X_SINGLE_INTERNAL_CALL",					dalType_string,	0,	0,	NULL, NULL, dalcmd_Forbid},
	{NULL,										0,				0,	0,	NULL},
};

typedef struct parm_s {
    char *DAL_FeatureKey;
    char *RDM_FeatureKey;
}parm_t;

parm_t Call_Feature_param[] = {
    {"CALL_RETURN",                 "X_ZYXEL_CALL_RETURN"           },
    {"CLIR_DEACTIVATE",             "CLIR_DEACTIVATE"               },
    {"CLIR_DEACTIVATE",             "CLIR_ACTIVATE"                 },
    {"CA_ACTIVATE",                 "CA_ACTIVATE"                   },
    {"CA_DISABLE",                  "X_ZYXEL_CA_DISABLE"            },
    {"CW_ACTIVATE",                 "CW_ACTIVATE"                   },
    {"CW_DEACTIVATE",               "CW_DEACTIVATE"                 },
    {"IVR",                         "X_ZYXEL_IVR"                   },
    {"INTERNALCALL",                "X_ZYXEL_INTERNALCALL"          },
    {"TRANSFER_ACTIVE",             "X_ZYXEL_TRANSFER_ACTIVE"       },
    {"CFU_ACTIVATE",                "CFU_ACTIVATE"                  },
    {"CFU_DEACTIVATE",              "CFU_DEACTIVATE"                },
    {"CFNR_ACTIVATE",               "CFNR_ACTIVATE"                 },
    {"CFNR_DEACTIVATE",             "CFNR_DEACTIVATE"               },
    {"CFB_ACTIVATE",                "CFB_ACTIVATE"                  },
    {"CFB_DEACTIVATE",              "CFB_DEACTIVATE"                },
    {"ONESHOT_CW_ACTIVATE",         "X_ZYXEL_ONESHOT_CW_ACTIVATE"   },
    {"ONESHOT_CW_DEACTIVATE",       "X_ZYXEL_ONESHOT_CW_DEACTIVATE" },
    {"DND_ENABLE",                  "X_ZYXEL_DND_ENABLE"            },
    {"DND_DISABLE",                 "X_ZYXEL_DND_DISABLE"           },
    {"CCBS_DEACTIVATE",             "CCBS_DEACTIVATE"               },
    {"OUTGOING_SIP",                "X_ZYXEL_OUTGOING_SIP"          },
    {"CALL_REDIAL",                 "X_ZYXEL_CALL_REDIAL"           },
    {"X_SINGLE_INTERNAL_CALL",      "X_SINGLE_INTERNAL_CALL"        },
};

const char* ProfName;
const char* ProfleEnable;
const char* X_ZYXEL_BoundIfName;
const char* X_ZYXEL_BoundIfList;
const char* DTMFMode;
const char* DTMFMethod;
const char* DTMFMethodG711;
bool DigitMapEnable;
const char* DigitMap;
unsigned int X_ZYXEL_VoipIOPFlags;
unsigned int X_ZYXEL_VoipSigIOPFlags;
bool ReplaceDialDigit;
bool RemoveRouteHeader;
const char* X_ZYXEL_FaxMode;
const char* FAXPassThrough;
const char* ModemPassThrough;
bool X_ZYXEL_IgnoreDirectIP;
unsigned int UserAgentPort;
const char* ProxyServer;
unsigned int ProxyServerPort;
const char* ProxyServerTransport;
const char* RegistrarServerTransport;
const char* UserAgentTransport;
const char* RegistrarServer;
unsigned int RegistrarServerPort;
const char* TransportType;
const char* UserAgentDomain;
const char* X_ZYXEL_ConferenceURI;
const char* OutboundProxy;
unsigned int OutboundProxyPort;
bool X_ZYXEL_Option120Use;
unsigned int RegisterExpires;
unsigned int RegisterRetryInterval;
unsigned int InviteExpires;
unsigned int X_ZYXEL_MinSE;
bool X_ZYXEL_Rfc3263Support;
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
const char* X_ZYXEL_Rfc3263_DNS_Method;
const char* X_ZYXEL_REGISTER_RequestURIBase;
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
bool X_ZYXEL_Rfc3262Support;
unsigned int LocalPortMin;
unsigned int LocalPortMax;
unsigned int DSCPMark;
unsigned int RTPDSCPMark;
bool SRTPEnable;
const char* X_ZYXEL_Crypto_Suite_Name;
unsigned int InterDigitTimerStd;
const char* X_ZYXEL_CALL_RETURN;
const char* CLIR_DEACTIVATE;
const char* CLIR_ACTIVATE;
const char* CA_ACTIVATE;
const char* X_ZYXEL_CA_DISABLE;
const char* CW_ACTIVATE;
const char* CW_DEACTIVATE;
const char* X_ZYXEL_IVR;
const char* X_ZYXEL_INTERNALCALL;
const char* X_ZYXEL_TRANSFER_ACTIVE;
const char* CFU_ACTIVATE;
const char* CFU_DEACTIVATE;
const char* CFNR_ACTIVATE;
const char* CFNR_DEACTIVATE;
const char* CFB_ACTIVATE;
const char* CFB_DEACTIVATE;
const char* X_ZYXEL_ONESHOT_CW_ACTIVATE;
const char* X_ZYXEL_ONESHOT_CW_DEACTIVATE;
const char* X_ZYXEL_DND_ENABLE;
const char* X_ZYXEL_DND_DISABLE;
const char* CCBS_DEACTIVATE;
const char* X_ZYXEL_OUTGOING_SIP;
const char* X_ZYXEL_CALL_REDIAL;
const char* X_SINGLE_INTERNAL_CALL;
const char* PrefixRange;
const char* FacilityAction;

struct json_object *multiJobj;
struct json_object *guiCustObj;
struct json_object *profObj;
struct json_object *profSipObj;
struct json_object *profRtpObj;
struct json_object *profRtpRtcpObj; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
struct json_object *profRtpSrtpObj;
struct json_object *profRtpRedundancyObj; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
struct json_object *profNumPlanpObj;
struct json_object *profNumPlanpInfoObj;
struct json_object *profFaxT38Obj; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
struct json_object *singleProObj;

bool free_multiJobj = true;

unsigned int flag1;

objIndex_t cFlagIid = {0};
objIndex_t profIid = {0};
objIndex_t profPrefixInfoIid = {0};

extern zcfgRet_t zcfgFeDalVoipLineGetDefault(struct json_object* defaultJobj, char *replyMsg);


void zcfgFeDalShowVoipProfile(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL, *obj_idx = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	obj_idx = json_object_array_get_idx(Jarray, 0);
	if(len == 1 && json_object_get_boolean(json_object_object_get(obj_idx, "ShowDetail"))){
		obj = json_object_array_get_idx(Jarray, 0);
		printf("SIP Service Provider Selection:\n");
		printf("%-2s %-30s %-10s \n","", "Service Provider Selection:", json_object_get_string(json_object_object_get(obj, "ProfName")));
		printf("General:\n");
		printf("%-2s %-30s %-10s \n","", "SIP Service Provider:", json_object_get_string(json_object_object_get(obj, "ProfleEnable")));
		printf("%-2s %-30s %-10s \n","", "SIP Service Provider Name:", json_object_get_string(json_object_object_get(obj, "ProfName")));
		printf("%-2s %-30s %-10s \n","", "SIP Local Port:", json_object_get_string(json_object_object_get(obj, "UserAgentPort")));
		printf("%-2s %-30s %-10s \n","", "SIP Proxy Server Address:", json_object_get_string(json_object_object_get(obj, "ProxyServer")));
		printf("%-2s %-30s %-10s \n","", "SIP Proxy Server Port:", json_object_get_string(json_object_object_get(obj, "ProxyServerPort")));
		printf("%-2s %-30s %-10s \n","", "SIP REGISTRAR Server Address:", json_object_get_string(json_object_object_get(obj, "RegistrarServer")));
		printf("%-2s %-30s %-10s \n","", "SIP REGISTRAR Server Port:", json_object_get_string(json_object_object_get(obj, "RegistrarServerPort")));
		printf("%-2s %-30s %-10s \n","", "SIP Service Domain:", json_object_get_string(json_object_object_get(obj, "UserAgentDomain")));
		printf("%-2s %-30s %-10s \n","", "URI of the network conference bridge:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_ConferenceURI")));
		printf("RFC Support:\n");
		printf("%-2s %-30s %-10s \n","", "PRACK (RFC 3262, Require: 100rel):", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Rfc3262Support")));
		printf("VoIP IOP Flags:\n");
		printf("%s %s %d \n",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags")),__func__,__LINE__);

		if(!strcmp("0",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags")))){
			printf("%-2s %-50s %-10s \n","", "Replace dial digit '#' to '%23' in SIP messages:", "Disable");
			printf("%-2s %-50s %-10s \n","", "Remove the 'Route' header in SIP messages:", "Disable");
		}
		else if(!strcmp("1",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags")))){
			printf("%-2s %-40s %-10s \n","", "Replace dial digit '#' to '%23' in SIP messages:", "Enable");
			printf("%-2s %-40s %-10s \n","", "Remove the 'Route' header in SIP messages:", "Disable");
		}
		else if(!strcmp("4",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags")))){
			printf("%-2s %-40s %-10s \n","", "Replace dial digit '#' to '%23' in SIP messages:", "Disable");
			printf("%-2s %-40s %-10s \n","", "Remove the 'Route' header in SIP messages:", "Enable");
		}
		else if(!strcmp("5",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags")))){
			printf("%-2s %-40s %-10s \n","", "Replace dial digit '#' to '%23' in SIP messages:", "Enable");
			printf("%-2s %-40s %-10s \n","", "Remove the 'Route' header in SIP messages:", "Enable");
		}
		printf("Bound Interface Name:\n");
		if(!strcmp("Any_WAN",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_BoundIfName"))))
			printf("%-2s %-30s %-10s \n","", "Bound Interface Name:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_BoundIfName")));
		else if(!strcmp("Multi_WAN",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_BoundIfName"))))
			printf("%-2s %-30s %-10s %-10s\n","", "Bound Interface Name:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_BoundIfName")), json_object_get_string(json_object_object_get(obj, "BoundIfList")));
		printf("Outbound Proxy:\n");
		printf("%-2s %-30s %-10s \n","", "Outbound Proxy Address:", json_object_get_string(json_object_object_get(obj, "OutboundProxy")));
		printf("%-2s %-30s %-10s \n","", "Outbound Proxy Port:", json_object_get_string(json_object_object_get(obj, "OutboundProxyPort")));
		printf("%-2s %-30s %-10s \n","", "Use DHCP Option 120 First:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Option120Use")));
		printf("RTP Port Range:\n");
		printf("%-2s %-20s %-10s \n","", "Start Port:", json_object_get_string(json_object_object_get(obj, "LocalPortMin")));
		printf("%-2s %-20s %-10s \n","", "End Port:", json_object_get_string(json_object_object_get(obj, "LocalPortMax")));
		printf("SRTP Support:\n");
		printf("%-2s %-20s %-10s \n","", "SRTP Support:", json_object_get_string(json_object_object_get(obj, "SRTPEnable")));
		printf("%-2s %-20s %-10s \n","", "End Port:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Crypto_Suite_Name")));
		printf("%-20s %-10s \n", "DTMF Mode:", json_object_get_string(json_object_object_get(obj, "DTMFMethod")));
		printf("%-20s %-10s \n", "Transport Type:", json_object_get_string(json_object_object_get(obj, "ProxyServerTransport")));
		printf("%-20s %-10s \n", "Ignore Direct IP:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IgnoreDirectIP")));
		printf("%-20s %-10s \n", "FAX Option:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_FaxMode")));
		printf("QoS Tag:\n");
		printf("%-2s %-30s %-10s \n","", "SIP DSCP Mark Setting:", json_object_get_string(json_object_object_get(obj, "DSCPMark")));
		printf("%-2s %-30s %-10s \n","", "RTP DSCP Mark Setting:", json_object_get_string(json_object_object_get(obj, "RTPDSCPMark")));
		printf("Timer Setting:\n");
		printf("%-2s %-35s %-10s \n","", "SIP Register Expiration Duration:", json_object_get_string(json_object_object_get(obj, "RegisterExpires")));
		printf("%-2s %-35s %-10s \n","", "SIP Register Fail Re-try Timer:", json_object_get_string(json_object_object_get(obj, "RegisterRetryInterval")));
		printf("%-2s %-35s %-10s \n","", "Session Expires (SE):", json_object_get_string(json_object_object_get(obj, "InviteExpires")));
		printf("%-2s %-35s %-10s \n","", "Min-SE:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_MinSE")));
		printf("%-20s %-10d \n", "Dialing Interval Selection:", atoi(json_object_get_string(json_object_object_get(obj, "InterDigitTimerStd")))/1000);
		printf("%-20s %-10s \n", "Enable DNS SRV:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Rfc3263Support")));
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
		printf("%-20s %-10s \n", "DNS Method:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Rfc3263_DNS_Method")));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
	}
	else{
	printf("%-10s %-30s %-30s %-30s %-10s\n",
		"Index","SIP Service Provider Name","SIP Proxy Server Address","REGISTER Server Address","SIP Service Domain");
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-30s %-30s %-30s %-10s %-10s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "ProfName")),
			json_object_get_string(json_object_object_get(obj, "ProxyServer")),
			json_object_get_string(json_object_object_get(obj, "RegistrarServer")),
			json_object_get_string(json_object_object_get(obj, "UserAgentDomain")),
			json_object_get_string(json_object_object_get(obj, "X_ZYXEL_ConferenceURI")));
	}
	}

}


void zcfgFeDalCustomflagGet(){
	IID_INIT(cFlagIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &cFlagIid, &guiCustObj) == ZCFG_SUCCESS){
		flag1 = json_object_get_int(json_object_object_get(guiCustObj, "Flag1"));
		zcfgFeJsonObjFree(guiCustObj);
		return;
	}

	return;
}

void freeAllProfObjects(){
	if(multiJobj && free_multiJobj) json_object_put(multiJobj);
	if(guiCustObj) json_object_put(guiCustObj);
	if(profObj) json_object_put(profObj);
	if(profSipObj) json_object_put(profSipObj);
	if(profRtpObj) json_object_put(profRtpObj);
	if(profRtpRtcpObj) json_object_put(profRtpRtcpObj); //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if(profRtpSrtpObj) json_object_put(profRtpSrtpObj);
	if(profRtpRedundancyObj) json_object_put(profRtpRedundancyObj); //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if(profNumPlanpObj) json_object_put(profNumPlanpObj);
	if(profNumPlanpInfoObj) json_object_put(profNumPlanpInfoObj);
	if(profFaxT38Obj) json_object_put(profFaxT38Obj); //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if(singleProObj) json_object_put(singleProObj);

	return;
}

void getProfBasicInfo(struct json_object *Jobj){
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	int index = 0, origVoipIOPFlags = 0;

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	Iid.level = 2;
	Iid.idx[0] = 1;
	Iid.idx[1] = index;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF, &Iid, &obj) == ZCFG_SUCCESS){
		origVoipIOPFlags = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_VoipIOPFlags"));
		zcfgFeJsonObjFree(obj);
	}
	
	ProfName = json_object_get_string(json_object_object_get(Jobj, "ProfName"));
	ProfleEnable = json_object_get_string(json_object_object_get(Jobj, "ProfleEnable"));
	X_ZYXEL_BoundIfName = json_object_get_string(json_object_object_get(Jobj, "BoundIfName"));
	X_ZYXEL_BoundIfList = json_object_get_string(json_object_object_get(Jobj, "BoundIfList"));
	DTMFMode = json_object_get_string(json_object_object_get(Jobj, "DTMFMode"));
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		DigitMapEnable = json_object_get_boolean(json_object_object_get(Jobj, "DigitMapEnable"));
		DigitMap = json_object_get_string(json_object_object_get(Jobj, "DigitMap"));
	}
	if(json_object_object_get(Jobj, "VoipIOPFlags"))
	X_ZYXEL_VoipIOPFlags = json_object_get_int(json_object_object_get(Jobj, "VoipIOPFlags"));
	else if(json_object_object_get(Jobj, "ReplaceDialDigit") && json_object_object_get(Jobj, "RemoveRouteHeader")){		
		ReplaceDialDigit = json_object_get_boolean(json_object_object_get(Jobj, "ReplaceDialDigit"));
		RemoveRouteHeader = json_object_get_boolean(json_object_object_get(Jobj, "RemoveRouteHeader"));
		if(ReplaceDialDigit)
			origVoipIOPFlags |= (1<<0);
		else
			origVoipIOPFlags &= ~(1<<0);
		if(RemoveRouteHeader)
			origVoipIOPFlags |= (1<<2);
		else
			origVoipIOPFlags &= ~(1<<2);
		X_ZYXEL_VoipIOPFlags = origVoipIOPFlags;
	}

	if(json_object_object_get(Jobj, "VoipSigIOPFlags"))
		X_ZYXEL_VoipSigIOPFlags = json_object_get_int(json_object_object_get(Jobj, "VoipSigIOPFlags"));
	
	X_ZYXEL_FaxMode = json_object_get_string(json_object_object_get(Jobj, "FaxMode"));
	//FAXPassThrough = json_object_get_string(json_object_object_get(Jobj, "FAXPassThrough"));
	FAXPassThrough = "Auto";
	//ModemPassThrough = json_object_get_string(json_object_object_get(Jobj, "ModemPassThrough"));
	ModemPassThrough = "Auto";
	X_ZYXEL_IgnoreDirectIP = json_object_get_boolean(json_object_object_get(Jobj, "IgnoreDirectIP"));
	UserAgentPort = json_object_get_int(json_object_object_get(Jobj, "UserAgentPort"));
	ProxyServer = json_object_get_string(json_object_object_get(Jobj, "ProxyServer"));
	ProxyServerPort = json_object_get_int(json_object_object_get(Jobj, "ProxyServerPort"));
	RegistrarServer = json_object_get_string(json_object_object_get(Jobj, "RegistrarServer"));
	RegistrarServerPort = json_object_get_int(json_object_object_get(Jobj, "RegistrarServerPort"));
	TransportType = json_object_get_string(json_object_object_get(Jobj, "TransportType"));
	UserAgentDomain = json_object_get_string(json_object_object_get(Jobj, "UserAgentDomain"));
	X_ZYXEL_ConferenceURI = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_ConferenceURI"));
	OutboundProxy = json_object_get_string(json_object_object_get(Jobj, "OutboundProxy"));
	OutboundProxyPort = json_object_get_int(json_object_object_get(Jobj, "OutboundProxyPort"));
	X_ZYXEL_Option120Use = json_object_get_boolean(json_object_object_get(Jobj, "Option120Use"));
	RegisterExpires = json_object_get_int(json_object_object_get(Jobj, "RegisterExpires"));
	RegisterRetryInterval = json_object_get_int(json_object_object_get(Jobj, "RegisterRetryInterval"));
	InviteExpires = json_object_get_int(json_object_object_get(Jobj, "InviteExpires"));
	X_ZYXEL_MinSE = json_object_get_int(json_object_object_get(Jobj, "MinSE"));
	DSCPMark = json_object_get_int(json_object_object_get(Jobj, "DSCPMark"));
	X_ZYXEL_Rfc3263Support = json_object_get_boolean(json_object_object_get(Jobj, "DNSSrvEnable"));
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
	X_ZYXEL_Rfc3263_DNS_Method = json_object_get_string(json_object_object_get(Jobj, "DnsMethod"));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
	X_ZYXEL_Rfc3262Support = json_object_get_boolean(json_object_object_get(Jobj, "Rfc3262Support"));
	LocalPortMin = json_object_get_int(json_object_object_get(Jobj, "LocalPort"));
	LocalPortMax = LocalPortMin + 18;
	RTPDSCPMark = json_object_get_int(json_object_object_get(Jobj, "RTPDSCPMark"));
	if(!(HIDE_VOICE_SRTP&flag1)){
		SRTPEnable = json_object_get_boolean(json_object_object_get(Jobj, "SRTPEnable"));
		X_ZYXEL_Crypto_Suite_Name = json_object_get_string(json_object_object_get(Jobj, "Crypto_Suite_Name"));
	}
	InterDigitTimerStd = json_object_get_int(json_object_object_get(Jobj, "InterDigitTimerStd"));
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		X_ZYXEL_CALL_RETURN = json_object_get_string(json_object_object_get(Jobj, "CALL_RETURN"));
		CLIR_DEACTIVATE  = json_object_get_string(json_object_object_get(Jobj, "CLIR_DEACTIVATE"));
		CLIR_ACTIVATE  = json_object_get_string(json_object_object_get(Jobj, "CLIR_ACTIVATE"));
		CA_ACTIVATE  = json_object_get_string(json_object_object_get(Jobj, "CA_ACTIVATE"));
		X_ZYXEL_CA_DISABLE  = json_object_get_string(json_object_object_get(Jobj, "CA_DISABLE"));
		CW_ACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CW_ACTIVATE"));
		CW_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CW_DEACTIVATE"));
		X_ZYXEL_IVR = json_object_get_string(json_object_object_get(Jobj, "IVR"));
		X_ZYXEL_INTERNALCALL = json_object_get_string(json_object_object_get(Jobj, "INTERNALCALL"));
		X_ZYXEL_TRANSFER_ACTIVE = json_object_get_string(json_object_object_get(Jobj, "TRANSFER_ACTIVE"));
		CFU_ACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFU_ACTIVATE"));
		CFU_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFU_DEACTIVATE"));
		CFNR_ACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFNR_ACTIVATE"));
		CFNR_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFNR_DEACTIVATE"));
		CFB_ACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFB_ACTIVATE"));
		CFB_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CFB_DEACTIVATE"));
		X_ZYXEL_ONESHOT_CW_ACTIVATE = json_object_get_string(json_object_object_get(Jobj, "ONESHOT_CW_ACTIVATE"));
		X_ZYXEL_ONESHOT_CW_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "ONESHOT_CW_DEACTIVATE"));
		X_ZYXEL_DND_ENABLE = json_object_get_string(json_object_object_get(Jobj, "DND_ENABLE"));
		X_ZYXEL_DND_DISABLE = json_object_get_string(json_object_object_get(Jobj, "DND_DISABLE"));
		CCBS_DEACTIVATE = json_object_get_string(json_object_object_get(Jobj, "CCBS_DEACTIVATE"));
		X_ZYXEL_OUTGOING_SIP = json_object_get_string(json_object_object_get(Jobj, "OUTGOING_SIP"));
		X_ZYXEL_CALL_REDIAL = json_object_get_string(json_object_object_get(Jobj, "CALL_REDIAL"));
		X_SINGLE_INTERNAL_CALL = json_object_get_string(json_object_object_get(Jobj, "X_SINGLE_INTERNAL_CALL"));
	}
	return;
}

bool checkMaxProfileNum(){
	unsigned int profNum = 0;
	objIndex_t getProfIid;
	struct json_object* targetobj = NULL;

	IID_INIT(getProfIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &getProfIid, &targetobj) == ZCFG_SUCCESS) {
		profNum++;
		zcfgFeJsonObjFree(targetobj);
	}

	if(profNum >= ZYXEL_SIP_PROFILE_NUM_MAX){
		printf("%s():<WARRNING>Max Profile number(%d) Currect Profile number(%d) - User can't add more profile.\n", __FUNCTION__, ZYXEL_SIP_PROFILE_NUM_MAX, profNum);
		return false;
	}

	return true;
}

void zcfgFeDalProfObjEditIidGet(struct json_object *Jobj){
	//const char* tmpprofName = NULL;
	const char* editPath = NULL;
	int profileNum=0;
	//printf("%s() Enter \n ",__FUNCTION__);

	//printf("%s() ProfName : %s \n ",__FUNCTION__,ProfName);
	if(json_object_object_get(Jobj, "EditPath") != NULL){
		editPath = json_object_get_string(json_object_object_get(Jobj, "EditPath"));
		sscanf(editPath,"VoiceProfile.%d",&profileNum);
	}
	else if(json_object_object_get(Jobj, "Index") != NULL){
		profileNum = json_object_get_int(json_object_object_get(Jobj, "Index"));
	}
		

	profIid.level = 2;
	profIid.idx[0] = 1;
	profIid.idx[1] = profileNum;

	return;
}


void initProfGlobalObjects(){
	multiJobj = NULL;
	guiCustObj = NULL;
	profObj = NULL;
	profSipObj = NULL;
	profRtpObj = NULL;
	profRtpRtcpObj = NULL; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	profRtpSrtpObj = NULL;
	profRtpRedundancyObj = NULL; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	profNumPlanpObj = NULL;
	profNumPlanpInfoObj = NULL;
	profFaxT38Obj = NULL; //Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	singleProObj = NULL;
	return;
}

zcfgRet_t zcfgFeDalVoipProfileGetDefault(struct json_object* defaultJobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *ProfleEnable = NULL;
	int DSCPMark = 0;
	int RTPDSCPMark = 0;
	bool SRTPEnable = false;
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
	char DnsMethod[16];
	bool DnsSrvEnable = NULL;
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
    char facilityAction[1024] = {0};
    char prefixRange[1024] = {0};

    zcfgFeDalCustomflagGet();

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF);
		return ret;
	}
	ProfleEnable = json_object_get_string(json_object_object_get(defaultJobj, "Enable"));
	json_object_object_add(defaultJobj, "ProfEnable", json_object_new_string(ProfleEnable));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "Enable");

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF_SIP, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF_SIP);
		return ret;
	}
	DSCPMark = json_object_get_int(json_object_object_get(defaultJobj, "DSCPMark"));
	json_object_object_add(defaultJobj, "SIPDSCPMark", json_object_new_int(DSCPMark));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "DSCPMark");

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF_RTP, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF_RTP);
		return ret;
	}
	RTPDSCPMark = json_object_get_int(json_object_object_get(defaultJobj, "DSCPMark"));
	json_object_object_add(defaultJobj, "RTPDSCPMark", json_object_new_int(RTPDSCPMark));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "DSCPMark");

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF_RTP_SRTP, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF_RTP_SRTP);
		return ret;
	}
	SRTPEnable = json_object_get_boolean(json_object_object_get(defaultJobj, "Enable"));
	json_object_object_add(defaultJobj, "SRTPEnable", json_object_new_boolean(SRTPEnable));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "Enable");

#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
	ZOS_STRNCPY(DnsMethod, json_object_get_string(json_object_object_get(defaultJobj, "X_ZYXEL_Rfc3263_DNS_Method")), sizeof(DnsMethod));
	DnsSrvEnable = json_object_get_boolean(json_object_object_get(defaultJobj, "X_ZYXEL_Rfc3262Support"));
	if ( (0 != strcmp(DnsMethod, "NAPTR"))
		&& (0 != strcmp(DnsMethod, "SRV"))
		&& (0 != strcmp(DnsMethod, "BASIC")) )
	{
		if (DnsSrvEnable)
		{
			strcpy(DnsMethod, "SRV");
		}
		else
		{
			strcpy(DnsMethod, "BASIC");
		}
	}
	json_object_object_add(defaultJobj, "X_ZYXEL_Rfc3263_DNS_Method", json_object_new_string(DnsMethod));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF_NUM_PLAN, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF_NUM_PLAN);
		return ret;
	}
	/*if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_PROF_FAX_T38, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_PROF_FAX_T38);
		return ret;
	}*/

    if (SHOW_NOT_USE_OFTEN_VOIP_SETTING & flag1)
    {
        strcpy(facilityAction, "X_ZYXEL_CALL_RETURN,CLIR_DEACTIVATE,CLIR_ACTIVATE,CA_ACTIVATE,X_ZYXEL_CA_DISABLE,X_ZYXEL_INTERNALCALL,X_ZYXEL_TRANSFER_ACTIVE,CFU_ACTIVATE,CFU_DEACTIVATE,CFNR_ACTIVATE,CFNR_DEACTIVATE,CFB_ACTIVATE,CFB_DEACTIVATE,X_ZYXEL_CALL_REDIAL,X_SINGLE_INTERNAL_CALL,CW_ACTIVATE,CW_DEACTIVATE,X_ZYXEL_ONESHOT_CW_ACTIVATE,X_ZYXEL_ONESHOT_CW_DEACTIVATE,X_ZYXEL_DND_ENABLE,X_ZYXEL_DND_DISABLE,CCBS_DEACTIVATE,X_ZYXEL_OUTGOING_SIP,X_ZYXEL_IVR");
        json_object_object_add(defaultJobj, "facilityAction", json_object_new_string(facilityAction));

#ifdef WIND_ITALY_CUSTOMIZATION
        strcpy(prefixRange, ",,,,,,,,,,,,,,,*43#,#43#,,,,,,,");
#else
        strcpy(prefixRange, ",,,,,####,*98#,,,,,,,,**,,,,,,,,,****");
#endif
        json_object_object_add(defaultJobj, "prefixRange", json_object_new_string(prefixRange));
    }

	return ret;
}

zcfgRet_t zcfgFeDalProfObjAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	IID_INIT(profIid);
	//printf("%s() Enter \n ",__FUNCTION__);

	profIid.level = 1;
	profIid.idx[0] = 1;

	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_VOICE_PROF, &profIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add prof fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}


zcfgRet_t zcfgFeDalProfObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF, &profIid, &profObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>Prof Obj :%s \n ",__FUNCTION__,json_object_to_json_string(profObj));

	if(profObj != NULL){
		if(json_object_object_get(Jobj, "ProfName") != NULL)
			json_object_object_add(profObj, "Name", json_object_new_string(ProfName));
		if(json_object_object_get(Jobj, "ProfleEnable") != NULL)
			json_object_object_add(profObj, "Enable", json_object_new_string(ProfleEnable));
		if(json_object_object_get(Jobj, "BoundIfName") != NULL)
			json_object_object_add(profObj, "X_ZYXEL_BoundIfName", json_object_new_string(X_ZYXEL_BoundIfName));
		if(json_object_object_get(Jobj, "BoundIfList") != NULL)
			json_object_object_add(profObj, "X_ZYXEL_BoundIfList", json_object_new_string(X_ZYXEL_BoundIfList));
		if(json_object_object_get(Jobj, "DTMFMode") != NULL){
			json_object_object_add(profObj, "DTMFMethod", json_object_new_string(DTMFMode));
			json_object_object_add(profObj, "DTMFMethodG711", json_object_new_string(DTMFMode));
		}
		if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
			if(json_object_object_get(Jobj, "DigitMapEnable") != NULL)
				json_object_object_add(profObj, "DigitMapEnable", json_object_new_boolean(DigitMapEnable));
			if(json_object_object_get(Jobj, "DigitMap") != NULL)
				json_object_object_add(profObj, "DigitMap", json_object_new_string(DigitMap));
		}
		if(json_object_object_get(Jobj, "VoipIOPFlags") != NULL || (json_object_object_get(Jobj, "ReplaceDialDigit") && json_object_object_get(Jobj, "RemoveRouteHeader")))
			json_object_object_add(profObj, "X_ZYXEL_VoipIOPFlags", json_object_new_int(X_ZYXEL_VoipIOPFlags));

		if(json_object_object_get(Jobj, "VoipSigIOPFlags") != NULL)
			json_object_object_add(profObj, "X_ZYXEL_VoipSigIOPFlags", json_object_new_int(X_ZYXEL_VoipSigIOPFlags));

		if(json_object_object_get(Jobj, "FaxMode") != NULL)
			json_object_object_add(profObj, "X_ZYXEL_FaxMode", json_object_new_string(X_ZYXEL_FaxMode));
			json_object_object_add(profObj, "FAXPassThrough", json_object_new_string(FAXPassThrough));
			json_object_object_add(profObj, "ModemPassThrough", json_object_new_string(ModemPassThrough));
		if(json_object_object_get(Jobj, "IgnoreDirectIP") != NULL)
			json_object_object_add(profObj, "X_ZYXEL_IgnoreDirectIP", json_object_new_boolean(X_ZYXEL_IgnoreDirectIP));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF, &profIid, multiJobj, profObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF);
			zcfgFeJsonObjFree(profObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profObj);
	}
	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);

	return ret;
}

zcfgRet_t zcfgFeDalProfSipObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_SIP, &profIid, &profSipObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof sip obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>Prof sip Obj :%s \n ",__FUNCTION__,json_object_to_json_string(profSipObj));

	if(profSipObj != NULL){
		if(json_object_object_get(Jobj, "UserAgentPort") != NULL)
			json_object_object_add(profSipObj, "UserAgentPort", json_object_new_int(UserAgentPort));
		if(json_object_object_get(Jobj, "ProxyServer") != NULL)
			json_object_object_add(profSipObj, "ProxyServer", json_object_new_string(ProxyServer));
		if(json_object_object_get(Jobj, "ProxyServerPort") != NULL)
			json_object_object_add(profSipObj, "ProxyServerPort", json_object_new_int(ProxyServerPort));
		if(json_object_object_get(Jobj, "RegistrarServer") != NULL)
			json_object_object_add(profSipObj, "RegistrarServer", json_object_new_string(RegistrarServer));
		if(json_object_object_get(Jobj, "RegistrarServerPort") != NULL)
			json_object_object_add(profSipObj, "RegistrarServerPort", json_object_new_int(RegistrarServerPort));
		if(json_object_object_get(Jobj, "TransportType") != NULL){
			json_object_object_add(profSipObj, "ProxyServerTransport", json_object_new_string(TransportType));
			json_object_object_add(profSipObj, "RegistrarServerTransport", json_object_new_string(TransportType));
			json_object_object_add(profSipObj, "UserAgentTransport", json_object_new_string(TransportType));
		}
		if(json_object_object_get(Jobj, "UserAgentDomain") != NULL)
			json_object_object_add(profSipObj, "UserAgentDomain", json_object_new_string(UserAgentDomain));
		if(json_object_object_get(Jobj, "X_ZYXEL_ConferenceURI") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_ConferenceURI", json_object_new_string(X_ZYXEL_ConferenceURI));
		if(json_object_object_get(Jobj, "OutboundProxy") != NULL)
			json_object_object_add(profSipObj, "OutboundProxy", json_object_new_string(OutboundProxy));
		if(json_object_object_get(Jobj, "OutboundProxyPort") != NULL)
			json_object_object_add(profSipObj, "OutboundProxyPort", json_object_new_int(OutboundProxyPort));
		if(json_object_object_get(Jobj, "Option120Use") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_Option120Use", json_object_new_boolean(X_ZYXEL_Option120Use));
		if(json_object_object_get(Jobj, "RegisterExpires") != NULL)
			json_object_object_add(profSipObj, "RegisterExpires", json_object_new_int(RegisterExpires));
		if(json_object_object_get(Jobj, "RegisterRetryInterval") != NULL)
			json_object_object_add(profSipObj, "RegisterRetryInterval", json_object_new_int(RegisterRetryInterval));
		if(json_object_object_get(Jobj, "InviteExpires") != NULL)
			json_object_object_add(profSipObj, "InviteExpires", json_object_new_int(InviteExpires));
		if(json_object_object_get(Jobj, "MinSE") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_MinSE", json_object_new_int(X_ZYXEL_MinSE));
		if(json_object_object_get(Jobj, "DSCPMark") != NULL)
			json_object_object_add(profSipObj, "DSCPMark", json_object_new_int(DSCPMark));
		if(json_object_object_get(Jobj, "DNSSrvEnable") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_Rfc3263Support", json_object_new_boolean(X_ZYXEL_Rfc3263Support));
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
		if(json_object_object_get(Jobj, "DnsMethod") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_Rfc3263_DNS_Method", json_object_new_string(X_ZYXEL_Rfc3263_DNS_Method));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
		if(json_object_object_get(Jobj, "Rfc3262Support") != NULL)
			json_object_object_add(profSipObj, "X_ZYXEL_Rfc3262Support", json_object_new_boolean(X_ZYXEL_Rfc3262Support));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_SIP, &profIid, multiJobj, profSipObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_SIP);
			zcfgFeJsonObjFree(profSipObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profSipObj);
	}
	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);

	return ret;
}

//Amber.20170914.B: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
zcfgRet_t zcfgFeDalProfFaxT38ObjAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_FAX_T38, &profIid, &profFaxT38Obj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof fax T38 obj fail~ \n ", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(profFaxT38Obj != NULL){
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_FAX_T38, &profIid, multiJobj, profFaxT38Obj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_FAX_T38);
			zcfgFeJsonObjFree(profFaxT38Obj);
			return ZCFG_INTERNAL_ERROR;
		}
	}
	zcfgFeJsonObjFree(profFaxT38Obj);

	return ret;
}
//Amber.20170914.E: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI

zcfgRet_t zcfgFeDalProfFaxT38ObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	return ret;
}

zcfgRet_t zcfgFeDalProfRTPObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP, &profIid, &profRtpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof rtp obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>Prof rtp Obj :%s \n ",__FUNCTION__,json_object_to_json_string(profRtpObj));


	if(profRtpObj != NULL){
		if(json_object_object_get(Jobj, "LocalPort") != NULL){
			json_object_object_add(profRtpObj, "LocalPortMin", json_object_new_int(LocalPortMin));
			json_object_object_add(profRtpObj, "LocalPortMax", json_object_new_int(LocalPortMax));
		}
		if(json_object_object_get(Jobj, "RTPDSCPMark") != NULL)
			json_object_object_add(profRtpObj, "DSCPMark", json_object_new_int(RTPDSCPMark));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_RTP, &profIid, multiJobj, profRtpObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_RTP);
			zcfgFeJsonObjFree(profRtpObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profRtpObj);
	}
	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);

	return ret;
}

//Amber.20170914.B: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
zcfgRet_t zcfgFeDalProfRTPRTCPObjAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP_RTCP, &profIid, &profRtpRtcpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof RTP-RTCP obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	
	if(profRtpRtcpObj != NULL){
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_RTP_RTCP, &profIid, multiJobj, profRtpRtcpObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_RTP_RTCP);
			zcfgFeJsonObjFree(profRtpRtcpObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profRtpRtcpObj);
	}

	return ret;
}
//Amber.20170914.E: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI

zcfgRet_t zcfgFeDalProfRTPSRTPObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP_SRTP, &profIid, &profRtpSrtpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof RTP-SRTP obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>Prof rtp-srtp Obj :%s \n ",__FUNCTION__,json_object_to_json_string(profRtpSrtpObj));


	if(profRtpSrtpObj != NULL){
		if(json_object_object_get(Jobj, "SRTPEnable") != NULL)
			json_object_object_add(profRtpSrtpObj, "Enable", json_object_new_boolean(SRTPEnable));
		if(json_object_object_get(Jobj, "Crypto_Suite_Name") != NULL)
			json_object_object_add(profRtpSrtpObj, "X_ZYXEL_Crypto_Suite_Name", json_object_new_string(X_ZYXEL_Crypto_Suite_Name));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_RTP_SRTP, &profIid, multiJobj, profRtpSrtpObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_RTP_SRTP);
			zcfgFeJsonObjFree(profRtpSrtpObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profRtpSrtpObj);
	}
	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);

	return ret;
}

//Amber.20170914.B: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
zcfgRet_t zcfgFeDalProfRTPRedundancyObjAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP_REDUNDANCY, &profIid, &profRtpRedundancyObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof RTP-RTCP obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	
	if(profRtpRedundancyObj != NULL){
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_RTP_REDUNDANCY, &profIid, multiJobj, profRtpRedundancyObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_RTP_REDUNDANCY);
			zcfgFeJsonObjFree(profRtpRedundancyObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profRtpRedundancyObj);
	}

	return ret;
}
//Amber.20170914.E: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI

zcfgRet_t zcfgFeDalProfNumberingPlanObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_NUM_PLAN, &profIid, &profNumPlanpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof Numbering Plan obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>Prof Numbering Plan Obj :%s \n ",__FUNCTION__,json_object_to_json_string(profNumPlanpObj));


	if(profNumPlanpObj != NULL){
		if(json_object_object_get(Jobj, "InterDigitTimerStd") != NULL)
			json_object_object_add(profNumPlanpObj, "InterDigitTimerStd", json_object_new_int(InterDigitTimerStd*1000));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_NUM_PLAN, &profIid, multiJobj, profNumPlanpObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_NUM_PLAN);
			zcfgFeJsonObjFree(profNumPlanpObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(profNumPlanpObj);
	}
	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);

	return ret;
}

zcfgRet_t zcfgFeDalConvertFeatureKey(char **DAL_key, char **RDM_key)
{
    int i;

    if ((*DAL_key == NULL) && (*RDM_key == NULL))
    {
        return ZCFG_INVALID_PARAM_VALUE;
    }

    if (*DAL_key == NULL)
    {
        for (i = 0; Call_Feature_param[i].RDM_FeatureKey != NULL; i++)
        {
            if(!strcmp(Call_Feature_param[i].RDM_FeatureKey, *RDM_key))
            {
                *DAL_key = Call_Feature_param[i].DAL_FeatureKey;
                break;
            }
        }
    }
    else if (*RDM_key == NULL)
    {
        for (i = 0; Call_Feature_param[i].RDM_FeatureKey != NULL; i++)
        {
            if(!strcmp(Call_Feature_param[i].DAL_FeatureKey, *DAL_key))
            {
                *RDM_key = Call_Feature_param[i].RDM_FeatureKey;
                break;
            }
        }
    }

    return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalProfNumberingPlanPrefixInfoObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
    char* facilityAction = NULL;
    char* facilityAction_DAL = NULL;
	const char* key = NULL;
	struct json_object *setJson = NULL;
	profNumPlanpInfoObj = NULL;

	//printf("%s() Enter \n ",__FUNCTION__);

	IID_INIT(profPrefixInfoIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profIid, &profPrefixInfoIid, &profNumPlanpInfoObj) == ZCFG_SUCCESS){
		if(profNumPlanpInfoObj != NULL){
			facilityAction = (char *)json_object_get_string(json_object_object_get(profNumPlanpInfoObj, "FacilityAction"));

            if(facilityAction != NULL)
            {
                facilityAction_DAL = NULL;
                zcfgFeDalConvertFeatureKey(&facilityAction_DAL, &facilityAction);
				key = json_object_get_string(json_object_object_get(Jobj, facilityAction_DAL));
            }
            if(key != NULL)
            {
				json_object_object_add(profNumPlanpInfoObj, "PrefixRange", json_object_new_string(key));
            }

			if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profPrefixInfoIid, multiJobj, profNumPlanpInfoObj)) == NULL){
				printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO);
				zcfgFeJsonObjFree(profNumPlanpInfoObj);
				return ZCFG_INTERNAL_ERROR;
			}
			zcfgFeJsonObjFree(profNumPlanpInfoObj);
		}
	}

	//printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipProfileEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	//printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	initProfGlobalObjects();
	zcfgFeDalCustomflagGet();
	getProfBasicInfo(Jobj);
	multiJobj = zcfgFeJsonObjNew();
	zcfgFeDalProfObjEditIidGet(Jobj);

	if((ret = zcfgFeDalProfObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof obj fail \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalProfSipObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof sip obj fail \n", __FUNCTION__);
		goto exit;
	}
/*
	if((ret = zcfgFeDalProfFaxT38ObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof FaxT38 obj fail \n", __FUNCTION__);
		goto exit;
	}
*/
	if((ret = zcfgFeDalProfRTPObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof rtp fail obj \n", __FUNCTION__);
		goto exit;
	}
	if(!(HIDE_VOICE_SRTP&flag1)){
		if((ret = zcfgFeDalProfRTPSRTPObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s: Edit prof rtp-srtp obj fail \n", __FUNCTION__);
			goto exit;
		}
	}
	if((ret = zcfgFeDalProfNumberingPlanObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof NumPaln obj fail \n", __FUNCTION__);
		goto exit;
	}
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		if((ret = zcfgFeDalProfNumberingPlanPrefixInfoObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s: Edit prof NumPaln-prefixInfo obj fail \n", __FUNCTION__);
			goto exit;
		}
	}
	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_VOICE_PROF);
		goto exit;
	}

	printf("%s(): Exit.. \n ",__FUNCTION__);
exit:
	freeAllProfObjects();
	return ret;
}

extern void initLineGlobalObjects(); // implement in zcfg_fe_dal_sip_account.c
extern zcfgRet_t getLineBasicInfo(struct json_object *Jobj, char *replyMsg);
extern void zcfgFeDalCustomizationflagGet();
extern zcfgRet_t zcfgFeDalLineObjAdd(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineObjEdit(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineSipObjEdit(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineCallFeatureObjEdit(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineCodecListObjEdit(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineProcessingEdit(struct json_object *Jobj);
extern zcfgRet_t zcfgFeDalLineMailCfgAdd(struct json_object *Jobj);
extern void freeAllLineObjects();
zcfgRet_t zcfgFeDalVoipAddOneAccount(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char fail_reason[128] = {0};

	initLineGlobalObjects();
	ret = getLineBasicInfo(Jobj, fail_reason);
	if(ret != ZCFG_SUCCESS){
		goto exit;
	}
	zcfgFeDalCustomizationflagGet();

	char buf[64];
	sprintf(buf,"VoiceProfile.%d",profIid.idx[1]);

	json_object_object_add(Jobj, "AddPath", json_object_new_string(buf));
	//printf("%s(): %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	if((ret = zcfgFeDalLineObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to add line obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineSipObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line sip obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCallFeatureObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit call feature obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCodecListObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit codec list obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineProcessingEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line processing obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineMailCfgAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to add mail config obj \n", __FUNCTION__);
		goto exit;
	}

exit:
	freeAllLineObjects();
	return ret;
}

extern zcfgRet_t zcfgFeDalVoipLineAdd(struct json_object *Jobj,struct json_object * prepareMultiJobj,char *replyMsg); // implement in zcfg_fe_dal_sip_account.c
zcfgRet_t zcfgFeDalVoipProfileAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char path[16] = {0};
	struct json_object *defaultJobj;
	int X_ZYXEL_RemoteRingbackToneIndex, X_ZYXEL_MusicOnHoldToneIndex, TransmitGain, ReceiveGain;
	const char *X_ZYXEL_Url_Type = NULL;
		
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	//printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	if(checkMaxProfileNum() == false){
		return ZCFG_INTERNAL_ERROR;
	}
	initProfGlobalObjects();
	zcfgFeDalCustomflagGet();
	getProfBasicInfo(Jobj);

	/*Default parameters for new Account */
	defaultJobj = json_object_new_object();
	ret = zcfgFeDalVoipLineGetDefault(defaultJobj, NULL);
	if((ret == ZCFG_SUCCESS) && (defaultJobj!=NULL)){
#if 0
		json_object_object_foreach(defaultJobj,key,val){
			if(json_object_object_get(Jobj, key) == NULL){
				json_object_object_add(Jobj, key,  JSON_OBJ_COPY(val));
				printf("111 find in Jobj, key=%s val=%s\n", key, json_object_to_json_string(val));
			}
			else
				printf("can find in Jobj, key=%s val=%s\n", key, json_object_to_json_string(val));
		}
#endif

		json_object_object_add(Jobj, "Enable", json_object_new_boolean(false));
		json_object_object_add(Jobj, "PhyReferenceList", json_object_new_string(""));
		json_object_object_add(Jobj, "SIP_Account_Number", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "DirectoryNumber")));
		json_object_object_add(Jobj, "AuthUserName", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "AuthUserName")));
		json_object_object_add(Jobj, "AuthPassword", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "AuthPassword")));
		
		X_ZYXEL_Url_Type = json_object_get_string(json_object_object_get(defaultJobj, "X_ZYXEL_Url_Type"));
		if(!strcmp(X_ZYXEL_Url_Type, "URL_Sip"))
			json_object_object_add(Jobj, "Url_Type", json_object_new_string("SIP"));
		else if(!strcmp(X_ZYXEL_Url_Type, "URL_Tel"))
			json_object_object_add(Jobj, "Url_Type", json_object_new_string("TEL"));
		//json_object_object_add(Jobj, "Url_Type", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_Url_Type")));
		
		json_object_object_add(Jobj, "AnonymousCallEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "AnonymousCallEnable")));
		json_object_object_add(Jobj, "CallTransferEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallTransferEnable")));
		json_object_object_add(Jobj, "CallWaitingEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallWaitingEnable")));
		json_object_object_add(Jobj, "CallWaitingRejectTime", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_CallWaitingRejectTime")));
		json_object_object_add(Jobj, "CallForwardUnconditionalEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardUnconditionalEnable")));
		json_object_object_add(Jobj, "CallForwardUnconditionalNumber", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardUnconditionalNumber")));
		json_object_object_add(Jobj, "CallForwardOnBusyEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardOnBusyEnable")));
		json_object_object_add(Jobj, "CallForwardOnBusyNumber", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardOnBusyNumber")));
		json_object_object_add(Jobj, "CallForwardOnNoAnswerEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardOnNoAnswerEnable")));
		json_object_object_add(Jobj, "CallForwardOnNoAnswerNumber", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardOnNoAnswerNumber")));
		json_object_object_add(Jobj, "CallForwardOnNoAnswerRingCount", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "CallForwardOnNoAnswerRingCount")));
		json_object_object_add(Jobj, "AnonymousCallBlockEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "AnonymousCallBlockEnable")));
		json_object_object_add(Jobj, "DoNotDisturbEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "DoNotDisturbEnable")));
		json_object_object_add(Jobj, "MWIEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "MWIEnable")));
		json_object_object_add(Jobj, "MwiExpireTime", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MwiExpireTime")));
		json_object_object_add(Jobj, "MissedCallEmailEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MissedCallEmailEnable")));
		//X_ZYXEL_MissedCallEmailEvent = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_MissedCallEmailEvent"));
		json_object_object_add(Jobj, "MissedCallEmailEvent", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MissedCallEmailEvent")));
		json_object_object_add(Jobj, "RemoteRingbackTone", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_RemoteRingbackTone")));
		
		X_ZYXEL_RemoteRingbackToneIndex = json_object_get_int(json_object_object_get(defaultJobj, "X_ZYXEL_RemoteRingbackToneIndex"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 66)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("Default"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 0)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("No1"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 1)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("No2"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 2)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("No3"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 3)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("No4"));
		if(X_ZYXEL_RemoteRingbackToneIndex == 4)
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("No5"));
		
		json_object_object_add(Jobj, "MusicOnHoldTone", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MusicOnHoldTone")));
		
		X_ZYXEL_MusicOnHoldToneIndex = json_object_get_int(json_object_object_get(defaultJobj, "X_ZYXEL_MusicOnHoldToneIndex"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 66)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("Default"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 0)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("No1"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 1)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("No2"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 2)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("No3"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 3)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("No4"));
		if(X_ZYXEL_MusicOnHoldToneIndex == 4)
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("No5"));
		
		//X_ZYXEL_CCBSEnable = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_CCBSEnable"));
		json_object_object_add(Jobj, "X_ZYXEL_CCBSEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_CCBSEnable")));
		json_object_object_add(Jobj, "X_ZYXEL_WarmLineEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_WarmLineEnable")));
		json_object_object_add(Jobj, "X_ZYXEL_HotLineEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_HotLineEnable")));
		json_object_object_add(Jobj, "X_ZYXEL_HotLineNum", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_HotLineNum")));
		json_object_object_add(Jobj, "X_ZYXEL_WarmLineNumber", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_WarmLineNumber")));
		json_object_object_add(Jobj, "X_ZYXEL_TimerFirstDigitWarmLine", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_TimerFirstDigitWarmLine")));
		json_object_object_add(Jobj, "EchoCancellationEnable", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "EchoCancellationEnable")));

		TransmitGain = json_object_get_int(json_object_object_get(defaultJobj, "TransmitGain"));
		if(TransmitGain == -3)
			json_object_object_add(Jobj, "TransmitGain", json_object_new_string("Minimum"));
		if(TransmitGain == 0)
			json_object_object_add(Jobj, "TransmitGain", json_object_new_string("Middle"));
		if(TransmitGain == 6)
			json_object_object_add(Jobj, "TransmitGain", json_object_new_string("Maximum"));
		//json_object_object_add(Jobj, "TransmitGain", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "TransmitGain")));

		ReceiveGain = json_object_get_int(json_object_object_get(defaultJobj, "ReceiveGain"));
		if(ReceiveGain == -3)
			json_object_object_add(Jobj, "ReceiveGain", json_object_new_string("Minimum"));
		if(ReceiveGain == 0)
			json_object_object_add(Jobj, "ReceiveGain", json_object_new_string("Middle"));
		if(ReceiveGain == 6)
			json_object_object_add(Jobj, "ReceiveGain", json_object_new_string("Maximum"));
		//json_object_object_add(Jobj, "ReceiveGain", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "ReceiveGain")));
		
		json_object_object_add(Jobj, "primaryCodec", json_object_new_string("G.711MuLaw"));
		json_object_object_add(Jobj, "secondarydCodec", json_object_new_string("G.711ALaw"));
		json_object_object_add(Jobj, "thirdCodec", json_object_new_string("G.722"));
		json_object_object_add(Jobj, "SilenceSuppression", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "SilenceSuppression")));
		json_object_object_add(Jobj, "ComfortNoise", json_object_new_boolean(false));
		//X_ZYXEL_ComfortNoise = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_ComfortNoise"));
		json_object_object_add(Jobj, "X_ZYXEL_ComfortNoise", json_object_new_boolean(false));
		json_object_object_add(Jobj, "MailAccount", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MissedCallEmailServerAddr")));
		json_object_object_add(Jobj, "Send_Notification_to_Email", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MissedCallEmailToAddr")));
		json_object_object_add(Jobj, "Missed_Call_Email_Title", JSON_OBJ_COPY(json_object_object_get(defaultJobj, "X_ZYXEL_MissedCallEmailTitle")));

		json_object_put(defaultJobj);
	}
	else
		return ZCFG_INTERNAL_ERROR;
	
	multiJobj = zcfgFeJsonObjNew();

	if((ret = zcfgFeDalProfObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to add prof obj \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalProfObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof obj fail \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalProfSipObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof sip obj fail \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalProfRTPObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof rtp fail obj \n", __FUNCTION__);
		goto exit;
	}

	//Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if((ret = zcfgFeDalProfRTPRTCPObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof rtp-rtcp fail obj \n", __FUNCTION__);
		goto exit;
	}

	if(!(HIDE_VOICE_SRTP&flag1)){
		if((ret = zcfgFeDalProfRTPSRTPObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s: Edit prof rtp-srtp obj fail \n", __FUNCTION__);
			goto exit;
		}
	}

	//Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if((ret = zcfgFeDalProfRTPRedundancyObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof rtp-redundancy fail obj \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalProfNumberingPlanObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof NumPaln obj fail \n", __FUNCTION__);
		goto exit;
	}

	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		if((ret = zcfgFeDalProfNumberingPlanPrefixInfoObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s: Edit prof NumPaln-prefixInfo obj fail \n", __FUNCTION__);
			goto exit;
		}
	}

	//Amber.20170914: Add for Content of  RTP-RTCP, RTP-REDUNDANCY, FAX_T38 won’t be created in config while adding new SIP provider via GUI/CLI
	if((ret = zcfgFeDalProfFaxT38ObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof FaxT38 obj fail \n", __FUNCTION__);
		goto exit;
	}

/*
	if((ret = zcfgFeDalProfFaxT38ObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof FaxT38 obj fail \n", __FUNCTION__);
		goto exit;
	}
*/
	
	sprintf(path,"VoiceProfile.%d",profIid.idx[1]);
	json_object_object_add(Jobj, "AddPath", json_object_new_string(path));
	ret = zcfgFeDalVoipLineAdd(Jobj, multiJobj, NULL);
	free_multiJobj = false;

exit:
	freeAllProfObjects();
	free_multiJobj = true;
	//printf("%s(): Exit..ret(%d) \n ",__FUNCTION__, ret);
	return ret;
}

zcfgRet_t zcfgFeDalVoipProfileDel(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* path;
	int profileNum;
	//printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	if(json_object_object_get(Jobj, "DelPath") != NULL){
		path = json_object_get_string(json_object_object_get(Jobj, "DelPath"));
		//printf("%s(): Delete prof path : %s \n ",__FUNCTION__, path);
		sscanf(path,"VoiceProfile.%d",&profileNum);
	}
	else if(json_object_object_get(Jobj, "Index") != NULL){ //for dalcmd
		profileNum = json_object_get_int(json_object_object_get(Jobj, "Index"));
	}
	else
		return ret;

	IID_INIT(profIid);
	profIid.level = 2;
	profIid.idx[0] = 1;
	profIid.idx[1] = profileNum;

	if((ret = zcfgFeObjJsonDel(RDM_OID_VOICE_PROF, &profIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete prof obj fail \n", __FUNCTION__);
		return ret;
	}

	//printf("%s(): Exit.. \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);

	ProfName = json_object_get_string(json_object_object_get(profObj, "Name"));
	ProfleEnable = json_object_get_string(json_object_object_get(profObj, "Enable"));
	X_ZYXEL_BoundIfName = json_object_get_string(json_object_object_get(profObj, "X_ZYXEL_BoundIfName"));
	X_ZYXEL_BoundIfList = json_object_get_string(json_object_object_get(profObj, "X_ZYXEL_BoundIfList"));
	DTMFMethodG711 = json_object_get_string(json_object_object_get(profObj, "DTMFMethodG711"));
	DTMFMethod = json_object_get_string(json_object_object_get(profObj, "DTMFMethod"));
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		DigitMapEnable = json_object_get_boolean(json_object_object_get(profObj, "DigitMapEnable"));
		DigitMap = json_object_get_string(json_object_object_get(profObj, "DigitMap"));
	}
	X_ZYXEL_VoipIOPFlags = json_object_get_int(json_object_object_get(profObj, "X_ZYXEL_VoipIOPFlags"));
	X_ZYXEL_VoipSigIOPFlags = json_object_get_int(json_object_object_get(profObj, "X_ZYXEL_VoipSigIOPFlags"));
	X_ZYXEL_FaxMode = json_object_get_string(json_object_object_get(profObj, "X_ZYXEL_FaxMode"));
	FAXPassThrough = json_object_get_string(json_object_object_get(profObj, "FAXPassThrough"));
	ModemPassThrough = json_object_get_string(json_object_object_get(profObj, "ModemPassThrough"));
	X_ZYXEL_IgnoreDirectIP = json_object_get_boolean(json_object_object_get(profObj, "X_ZYXEL_IgnoreDirectIP"));

	json_object_object_add(paserobj, "profileIdx", json_object_new_int(profIid.idx[1]));
	json_object_object_add(paserobj, "ProfName", json_object_new_string(ProfName));
	json_object_object_add(paserobj, "ProfleEnable", json_object_new_string(ProfleEnable));
	json_object_object_add(paserobj, "X_ZYXEL_BoundIfName", json_object_new_string(X_ZYXEL_BoundIfName));
	json_object_object_add(paserobj, "X_ZYXEL_BoundIfList", json_object_new_string(X_ZYXEL_BoundIfList));
	json_object_object_add(paserobj, "BoundIfList", json_object_new_string(X_ZYXEL_BoundIfList));
	json_object_object_add(paserobj, "DTMFMethodG711", json_object_new_string(DTMFMethodG711));
	json_object_object_add(paserobj, "DTMFMethod", json_object_new_string(DTMFMethod));
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		json_object_object_add(paserobj, "DigitMapEnable", json_object_new_boolean(DigitMapEnable));
		json_object_object_add(paserobj, "DigitMap", json_object_new_string(DigitMap));
	}
	json_object_object_add(paserobj, "X_ZYXEL_VoipIOPFlags", json_object_new_int(X_ZYXEL_VoipIOPFlags));
	json_object_object_add(paserobj, "X_ZYXEL_VoipSigIOPFlags", json_object_new_int(X_ZYXEL_VoipSigIOPFlags));
	json_object_object_add(paserobj, "X_ZYXEL_FaxMode", json_object_new_string(X_ZYXEL_FaxMode));
	json_object_object_add(paserobj, "FAXPassThrough", json_object_new_string(FAXPassThrough));
	json_object_object_add(paserobj, "ModemPassThrough", json_object_new_string(ModemPassThrough));
	json_object_object_add(paserobj, "X_ZYXEL_IgnoreDirectIP", json_object_new_boolean(X_ZYXEL_IgnoreDirectIP));

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfSipObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_SIP, &profIid, &profSipObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof sip fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if(profSipObj != NULL){
		UserAgentPort = json_object_get_int(json_object_object_get(profSipObj, "UserAgentPort"));
		ProxyServer = json_object_get_string(json_object_object_get(profSipObj, "ProxyServer"));
		ProxyServerPort = json_object_get_int(json_object_object_get(profSipObj, "ProxyServerPort"));
		ProxyServerTransport = json_object_get_string(json_object_object_get(profSipObj, "ProxyServerTransport"));
		RegistrarServer = json_object_get_string(json_object_object_get(profSipObj, "RegistrarServer"));
		RegistrarServerPort = json_object_get_int(json_object_object_get(profSipObj, "RegistrarServerPort"));
		RegistrarServerTransport = json_object_get_string(json_object_object_get(profSipObj, "RegistrarServerTransport"));
		UserAgentDomain = json_object_get_string(json_object_object_get(profSipObj, "UserAgentDomain"));
		UserAgentTransport = json_object_get_string(json_object_object_get(profSipObj, "UserAgentTransport"));
		X_ZYXEL_ConferenceURI = json_object_get_string(json_object_object_get(profSipObj, "X_ZYXEL_ConferenceURI"));
		OutboundProxy = json_object_get_string(json_object_object_get(profSipObj, "OutboundProxy"));
		OutboundProxyPort = json_object_get_int(json_object_object_get(profSipObj, "OutboundProxyPort"));
		X_ZYXEL_Option120Use = json_object_get_boolean(json_object_object_get(profSipObj, "X_ZYXEL_Option120Use"));
		RegisterExpires = json_object_get_int(json_object_object_get(profSipObj, "RegisterExpires"));
		RegisterRetryInterval = json_object_get_int(json_object_object_get(profSipObj, "RegisterRetryInterval"));
		InviteExpires = json_object_get_int(json_object_object_get(profSipObj, "InviteExpires"));
		X_ZYXEL_MinSE = json_object_get_int(json_object_object_get(profSipObj, "X_ZYXEL_MinSE"));
		DSCPMark = json_object_get_int(json_object_object_get(profSipObj, "DSCPMark"));
		X_ZYXEL_Rfc3263Support = json_object_get_boolean(json_object_object_get(profSipObj, "X_ZYXEL_Rfc3263Support"));
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
		X_ZYXEL_Rfc3263_DNS_Method = json_object_get_string(json_object_object_get(profSipObj, "X_ZYXEL_Rfc3263_DNS_Method"));
		X_ZYXEL_REGISTER_RequestURIBase = json_object_get_string(json_object_object_get(profSipObj, "X_ZYXEL_REGISTER_RequestURIBase"));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
		X_ZYXEL_Rfc3262Support = json_object_get_boolean(json_object_object_get(profSipObj, "X_ZYXEL_Rfc3262Support"));
		json_object_object_add(paserobj, "UserAgentPort", json_object_new_int(UserAgentPort));
		json_object_object_add(paserobj, "ProxyServer", json_object_new_string(ProxyServer));
		json_object_object_add(paserobj, "ProxyServerPort", json_object_new_int(ProxyServerPort));
		json_object_object_add(paserobj, "ProxyServerTransport", json_object_new_string(ProxyServerTransport));
		json_object_object_add(paserobj, "RegistrarServer", json_object_new_string(RegistrarServer));
		json_object_object_add(paserobj, "RegistrarServerPort", json_object_new_int(RegistrarServerPort));
		json_object_object_add(paserobj, "RegistrarServerTransport", json_object_new_string(RegistrarServerTransport));
		json_object_object_add(paserobj, "UserAgentDomain", json_object_new_string(UserAgentDomain));
		json_object_object_add(paserobj, "UserAgentTransport", json_object_new_string(UserAgentTransport));
		json_object_object_add(paserobj, "X_ZYXEL_ConferenceURI", json_object_new_string(X_ZYXEL_ConferenceURI));
		json_object_object_add(paserobj, "OutboundProxy", json_object_new_string(OutboundProxy));
		json_object_object_add(paserobj, "OutboundProxyPort", json_object_new_int(OutboundProxyPort));
		json_object_object_add(paserobj, "X_ZYXEL_Option120Use", json_object_new_boolean(X_ZYXEL_Option120Use));
		json_object_object_add(paserobj, "RegisterExpires", json_object_new_int(RegisterExpires));
		json_object_object_add(paserobj, "RegisterRetryInterval", json_object_new_int(RegisterRetryInterval));
		json_object_object_add(paserobj, "InviteExpires", json_object_new_int(InviteExpires));
		json_object_object_add(paserobj, "X_ZYXEL_MinSE", json_object_new_int(X_ZYXEL_MinSE));
		json_object_object_add(paserobj, "DSCPMark", json_object_new_int(DSCPMark));
		json_object_object_add(paserobj, "X_ZYXEL_Rfc3263Support", json_object_new_boolean(X_ZYXEL_Rfc3263Support));
#ifdef ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT
		json_object_object_add(paserobj, "X_ZYXEL_Rfc3263_DNS_Method", json_object_new_string(X_ZYXEL_Rfc3263_DNS_Method));
		json_object_object_add(paserobj, "X_ZYXEL_REGISTER_RequestURIBase", json_object_new_string(X_ZYXEL_REGISTER_RequestURIBase));
#endif //ZYXEL_VOICE_RFC3263_DNS_NAPTR_SUPPORT.
		json_object_object_add(paserobj, "X_ZYXEL_Rfc3262Support", json_object_new_boolean(X_ZYXEL_Rfc3262Support));

		zcfgFeJsonObjFree(profSipObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfRTPObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP, &profIid, &profRtpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof sip fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(profRtpObj != NULL){
		LocalPortMin = json_object_get_int(json_object_object_get(profRtpObj, "LocalPortMin"));
		LocalPortMax = json_object_get_int(json_object_object_get(profRtpObj, "LocalPortMax"));
		RTPDSCPMark = json_object_get_int(json_object_object_get(profRtpObj, "DSCPMark"));

		json_object_object_add(paserobj, "LocalPortMin", json_object_new_int(LocalPortMin));
		json_object_object_add(paserobj, "LocalPortMax", json_object_new_int(LocalPortMax));
		json_object_object_add(paserobj, "RTPDSCPMark", json_object_new_int(RTPDSCPMark));

		zcfgFeJsonObjFree(profRtpObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfRTPSRTPObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_RTP_SRTP, &profIid, &profRtpSrtpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof sip fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(profRtpSrtpObj != NULL){
		SRTPEnable = json_object_get_boolean(json_object_object_get(profRtpSrtpObj, "Enable"));
		X_ZYXEL_Crypto_Suite_Name = json_object_get_string(json_object_object_get(profRtpSrtpObj, "X_ZYXEL_Crypto_Suite_Name"));
		json_object_object_add(paserobj, "SRTPEnable", json_object_new_boolean(SRTPEnable));
		json_object_object_add(paserobj, "X_ZYXEL_Crypto_Suite_Name", json_object_new_string(X_ZYXEL_Crypto_Suite_Name));
		zcfgFeJsonObjFree(profRtpSrtpObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfNumberingPlanObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF_NUM_PLAN, &profIid, &profNumPlanpObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get prof numbering plan fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(profNumPlanpObj != NULL){
		InterDigitTimerStd = json_object_get_int(json_object_object_get(profNumPlanpObj, "InterDigitTimerStd"));
		json_object_object_add(paserobj, "InterDigitTimerStd", json_object_new_int(InterDigitTimerStd));

		zcfgFeJsonObjFree(profNumPlanpObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleProfNumberingPlanPrefixInfoObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char prefixRange[1024] = {};
	char facilityAction[1024] = {};
	char temp[1024] = {};
	//printf("%s() Enter..\n", __FUNCTION__);
	int featureNum = 0;

	IID_INIT(profPrefixInfoIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profIid, &profPrefixInfoIid, &profNumPlanpInfoObj) == ZCFG_SUCCESS){
		if(profNumPlanpInfoObj != NULL){
			PrefixRange = json_object_get_string(json_object_object_get(profNumPlanpInfoObj, "PrefixRange"));
			memset(temp, 0 , sizeof(temp));
			if(profPrefixInfoIid.idx[2] == 1){
				sprintf(prefixRange,"%s", PrefixRange);
			}else{
				sprintf(temp,"%s,%s", prefixRange, PrefixRange);
				strncpy(prefixRange, temp, sizeof(prefixRange));
			}
			memset(temp, 0 , sizeof(temp));
			FacilityAction = json_object_get_string(json_object_object_get(profNumPlanpInfoObj, "FacilityAction"));
			if(profPrefixInfoIid.idx[2] == 1){
				sprintf(facilityAction,"%s", FacilityAction);
			}else{
				sprintf(temp,"%s,%s",facilityAction , FacilityAction);
				strncpy(facilityAction, temp, sizeof(facilityAction));
			}

			featureNum++;
			zcfgFeJsonObjFree(profNumPlanpInfoObj);
		}
	}

	json_object_object_add(paserobj, "prefixRange", json_object_new_string(prefixRange));
	json_object_object_add(paserobj, "facilityAction", json_object_new_string(facilityAction));
	json_object_object_add(paserobj, "featureKeyNum", json_object_new_int(featureNum));

	return ret;
}

zcfgRet_t zcfgFeDalVoipProfileGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int idx = 1, index = 0;

	zcfgFeDalCustomflagGet();
	initProfGlobalObjects();

	IID_INIT(profIid);
	if(json_object_object_get(Jobj, "Index")){
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &profIid, RDM_OID_VOICE_PROF, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF, &profIid, &profObj) == ZCFG_SUCCESS) {
			if(profObj != NULL){
				singleProObj = json_object_new_object();
				json_object_object_add(singleProObj, "ShowDetail", json_object_new_boolean(true));

				if(singleProObj == NULL){
					printf("%s:Create new json Obj fail \n", __FUNCTION__);
					goto exit;
				}
				
				if((ret = zcfgFeDalparseSingleProfObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleProfSipObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof SIP Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleProfRTPObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof RTP Obj \n", __FUNCTION__);
					goto exit;
				}
				if(!(HIDE_VOICE_SRTP&flag1)){
					if((ret = zcfgFeDalparseSingleProfRTPSRTPObj(singleProObj)) != ZCFG_SUCCESS){
						printf("%s:(ERROR) fail to parse single Prof SRTP Obj \n", __FUNCTION__);
						goto exit;
					}
				}
				if((ret = zcfgFeDalparseSingleProfNumberingPlanObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof numbering plan Obj \n", __FUNCTION__);
					goto exit;
				}
				if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
					if((ret = zcfgFeDalparseSingleProfNumberingPlanPrefixInfoObj(singleProObj)) != ZCFG_SUCCESS){
						printf("%s:(ERROR) fail to parse single Prof numbering plan Obj \n", __FUNCTION__);
						goto exit;
					}
				}
				json_object_object_add(singleProObj, "Index", json_object_new_int(idx));
				if( ret == ZCFG_SUCCESS ){
					json_object_array_add(Jarray, JSON_OBJ_COPY(singleProObj));
				}


				zcfgFeJsonObjFree(singleProObj);
				zcfgFeJsonObjFree(profObj);
			}
		}
	}
	else{
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &profIid, &profObj) == ZCFG_SUCCESS) {
		if(profObj != NULL){
			singleProObj = json_object_new_object();
			if(singleProObj == NULL){
				printf("%s:Create new json Obj fail \n", __FUNCTION__);
				goto exit;
			}
			
			if((ret = zcfgFeDalparseSingleProfObj(singleProObj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Prof Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleProfSipObj(singleProObj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Prof SIP Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleProfRTPObj(singleProObj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Prof RTP Obj \n", __FUNCTION__);
				goto exit;
			}
			if(!(HIDE_VOICE_SRTP&flag1)){
				if((ret = zcfgFeDalparseSingleProfRTPSRTPObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof SRTP Obj \n", __FUNCTION__);
					goto exit;
				}
			}
			if((ret = zcfgFeDalparseSingleProfNumberingPlanObj(singleProObj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Prof numbering plan Obj \n", __FUNCTION__);
				goto exit;
			}
			if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
				if((ret = zcfgFeDalparseSingleProfNumberingPlanPrefixInfoObj(singleProObj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Prof numbering plan Obj \n", __FUNCTION__);
					goto exit;
				}
			}
				json_object_object_add(singleProObj, "Index", json_object_new_int(idx));
			if( ret == ZCFG_SUCCESS ){
				json_object_array_add(Jarray, JSON_OBJ_COPY(singleProObj));
			}
				idx++;

			zcfgFeJsonObjFree(singleProObj);
			zcfgFeJsonObjFree(profObj);
		}
	}
	}
	

	//printf("%s(): Jobj :%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
exit:
	freeAllProfObjects();
	return ret;
}


zcfgRet_t zcfgFeDalVoipProfile(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *defaultJobj = NULL;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalVoipProfileAdd(Jobj, NULL);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVoipProfileEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalVoipProfileDel(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalVoipProfileGet(Jobj, Jarray, NULL);
	}
	else if(!strcmp(method, "DEFAULT")) {
		defaultJobj = json_object_new_object();
		ret = zcfgFeDalVoipProfileGetDefault(defaultJobj, NULL);
		if((ret == ZCFG_SUCCESS) && (defaultJobj!=NULL)){
			json_object_array_add(Jarray, JSON_OBJ_COPY(defaultJobj));
			json_object_put(defaultJobj);
		}
	}
	return ret;
}

