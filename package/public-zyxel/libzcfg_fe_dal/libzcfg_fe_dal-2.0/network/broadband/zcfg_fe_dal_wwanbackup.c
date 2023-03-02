#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WWANBACKUP_param[]={
/* set 3G object */
{"Enable_3G", dalType_boolean, 0, 0, NULL},
{"DialNumber", dalType_string, 0, 0, NULL},
{"APN", dalType_string, 0, 0, NULL},
{"PIN_Code", dalType_string, 0, 0, NULL},
{"PIN_Verification", dalType_string, 0, 0, NULL},
/* set IPV4 object */
{"IPStatic", dalType_boolean,	0,	0,	NULL},
{"IPAddress", dalType_v4Addr,	0,	0,	NULL},
{"SubnetMask", dalType_v4Mask,	0,	0,	NULL},
/* set DNS object */
{"DNSStatic", dalType_boolean,	0,	0,	NULL},
{"PrimDNSServer", dalType_v4Addr, 0, 0, NULL},
{"SecDNSServer", dalType_v4Addr, 0, 0, NULL},
/* set PPP object */
{"Username", dalType_string, 0, 0, NULL},
{"Password", dalType_string, 0, 0, NULL},
#ifdef CONFIG_ZYXEL_PPP_AUTH_MENU_CHAP_PAP_CTL
{"AuthenticationProtocol", dalType_string, 0, 0, NULL, "Auto|CHAP|PAP"},
#endif
{"ConnectionTrigger", dalType_string, 0, 0, NULL, "OnDemand|AlwaysOn"},
{"IdleDisconnectTime", dalType_int, 1, 4320, NULL},
/* set Ping Check object, without apply */
{"Ping_Enable", dalType_boolean, 0, 0, NULL},
{"Ping_Interval", dalType_int, 20, 180, NULL},
{"Ping_FailLimit", dalType_int, 2, 5, NULL},
{"Ping_DefaultGateway", dalType_boolean, 0, 0, NULL},
{"Ping_Destination", dalType_string,	0,	64,	NULL},
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
{"Recovery_Interval", dalType_int, 0, 600, NULL},
{"Backup_Interval", dalType_int, 0, 600, NULL},
#endif
/* set Budget Control object, without apply */
{"Bdg_Enable", dalType_boolean, 0, 0, NULL},
{"Bdg_TimeEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_TimeValue", dalType_int, 0, 0, NULL},
{"Bdg_ByteEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_ByteValue", dalType_int, 0, 0, NULL},
{"Bdg_ByteMode", dalType_string, 0, 0, NULL, "DownUp|Down|Up"},
{"Bdg_PktEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_PktValue", dalType_int, 0, 0, NULL},
{"Bdg_PktMode", dalType_string, 0, 0, NULL, "DownUp|Down|Up"},
{"Bdg_ResetDay", dalType_int, 0, 31, NULL},
{"Bdg_TimeActionEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_TimeActionValue", dalType_int, 0, 0, NULL},
{"Bdg_ByteActionEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_ByteActionValue", dalType_int, 0, 0, NULL},
{"Bdg_PktActionEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_PktActionValue", dalType_int, 0, 0, NULL},
{"Bdg_DropWWAN", dalType_boolean, 0, 0, NULL},
{"Bdg_LogEnable", dalType_boolean, 0, 0, NULL},
{"Bdg_LogInterval", dalType_int, 0, 0, NULL},
{"ResetNow", dalType_boolean, 0, 0, NULL},
/* set Email object */
{"Email_Enable", dalType_boolean, 0, 0, NULL},
{"Email_Subject", dalType_string, 0, 0, NULL},
{"Email_To", dalType_emailAddr, 0, 0, NULL},
{"Email_SericeRef", dalType_emailList, 0, 0, NULL},

/*{"Email_UP_Enable", dalType_boolean, 0, 0, NULL},
{"Email_UP_Subject", dalType_string, 0, 0, NULL},
{"Email_UP_To", dalType_string, 0, 0, NULL},
{"Email_UP_SericeRef", dalType_string, 0, 0, NULL},

{"Email_DN_Enable", dalType_boolean, 0, 0, NULL},
{"Email_DN_Subject", dalType_string, 0, 0, NULL},
{"Email_DN_To", dalType_string, 0, 0, NULL},
{"Email_DN_SericeRef", dalType_string, 0, 0, NULL},*/

{"Email_BG_Enable", dalType_boolean, 0, 0, NULL},
{"Email_BG_Subject", dalType_string, 0, 0, NULL},
{"Email_BG_To", dalType_emailAddr, 0, 0, NULL},
{"Email_BG_SericeRef", dalType_emailList, 0, 0, NULL},
{NULL,	0,					0,	0,		NULL}

};

void getIidLevel(objIndex_t *objIid){
    uint8_t iidIdx =0;
	for(iidIdx=0;iidIdx<6;iidIdx++){
		if(objIid->idx[iidIdx] != 0)
			objIid->level = iidIdx+1;
	}
}

zcfgRet_t getIfPath(char *ipIface, char *pppIface){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object * WWANObj = NULL;
	objIndex_t WWANIid = {0};
	const char *curIpIface = NULL;
	const char *curPPPIface = NULL;
	IID_INIT(WWANIid);
	ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &WWANIid, &WWANObj);
	
	if(ret == ZCFG_SUCCESS){
		curIpIface = json_object_get_string(json_object_object_get(WWANObj, "Interface"));
		curPPPIface = json_object_get_string(json_object_object_get(WWANObj, "PPPInterface"));

		if(curIpIface)
			strcpy(ipIface, curIpIface);
		if(curPPPIface)
			strcpy(pppIface, curPPPIface);
	}

	zcfgFeJsonObjFree(WWANObj);
	return ret;
}

zcfgRet_t set3GObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object * WWANObj = NULL;
	objIndex_t WWANIid = {0};
	IID_INIT(WWANIid);
	bool enable = false;
	const char *dialNumber = NULL;
	const char *apn = NULL;
	const char *pin = NULL;
	
	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable_3G"));
	dialNumber = json_object_get_string(json_object_object_get(Jobj, "DialNumber"));
	apn = json_object_get_string(json_object_object_get(Jobj, "APN"));
	pin = json_object_get_string(json_object_object_get(Jobj, "PIN_Code"));
	
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_W_W_A_N_BACKUP, &WWANIid, &WWANObj);

	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Enable_3G"))
		json_object_object_add(WWANObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "DialNumber"))
		json_object_object_add(WWANObj, "DialNumber", json_object_new_string(dialNumber));
		if(json_object_object_get(Jobj, "APN"))
		json_object_object_add(WWANObj, "APN", json_object_new_string(apn));
		if(json_object_object_get(Jobj, "PIN_Code"))
		json_object_object_add(WWANObj, "PIN", json_object_new_string(pin));

		zcfgFeObjJsonSet(RDM_OID_ZY_W_W_A_N_BACKUP, &WWANIid, WWANObj, NULL);
	}

	zcfgFeJsonObjFree(WWANObj);
	
	return ret;
}

zcfgRet_t setIPv4Obj(struct json_object *Jobj, char *action, char *ipIface){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object * ipv4Obj = NULL;
	objIndex_t ipv4Iid = {0};
	const char *ipAddress = NULL;
	const char *subnetMask = NULL;
	bool IPStatic = false;

	IPStatic = json_object_get_boolean(json_object_object_get(Jobj, "IPStatic"));
	
	IID_INIT(ipv4Iid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv4Iid.idx[0]);
	ipv4Iid.idx[1] = 1;
	ipv4Iid.level = 2;
	
	ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4Iid, &ipv4Obj);

	if(IPStatic == true){
		if(ret == ZCFG_NO_MORE_INSTANCE && ipv4Obj == NULL){//post
			ipv4Iid.idx[1] = 0;
			ipv4Iid.level = 1;
			ret = zcfgFeObjJsonAdd(RDM_OID_IP_IFACE_V4_ADDR, &ipv4Iid, NULL);
			if(ret == ZCFG_SUCCESS){
				ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4Iid, &ipv4Obj);
			}
		}
		
		if(ret == ZCFG_SUCCESS && ipv4Obj != NULL){
			ipAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
			subnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));

			if(json_object_object_get(Jobj, "IPAddress"))
			json_object_object_add(ipv4Obj, "IPAddress", json_object_new_string(ipAddress));
			if(json_object_object_get(Jobj, "SubnetMask"))
			json_object_object_add(ipv4Obj, "SubnetMask", json_object_new_string(subnetMask));
			json_object_object_add(ipv4Obj, "AddressingType", json_object_new_string("Static"));
	
			zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4Iid, ipv4Obj, NULL);
		}

	}else{
		if(ret == ZCFG_SUCCESS && ipv4Obj != NULL){//delete
				ret = zcfgFeObjJsonDel(RDM_OID_IP_IFACE_V4_ADDR, &ipv4Iid, NULL);
			}
		}
		
	zcfgFeJsonObjFree(ipv4Obj);
	return ret;
}

zcfgRet_t setDnsObj(struct json_object *Jobj, char* action, char* ipIface){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dnsObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t dnsIid = {0};
	objIndex_t iid = {0};
	const char *CurDnsIface = NULL;
	char dnsServer[32] = {0};
	const char *primdns = NULL;
	const char *secdns = NULL;
	bool found = false;
	bool DNSStatic = false;

	DNSStatic = json_object_get_boolean(json_object_object_get(Jobj, "DNSStatic"));
	
	IID_INIT(dnsIid);
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_DNS_CLIENT_SRV, &iid, &obj) == ZCFG_SUCCESS){
		CurDnsIface = json_object_get_string(json_object_object_get(obj, "Interface"));
		if(!strcmp(ipIface,CurDnsIface)){
			memcpy(&dnsIid, &iid, sizeof(objIndex_t));
			dnsObj = obj;
			found = true;
			getIidLevel(&dnsIid);
			break;	
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}
	
	if(DNSStatic == true){
		if(found == false && dnsObj == NULL){//post
			IID_INIT(dnsIid);
			ret = zcfgFeObjJsonAdd(RDM_OID_DNS_CLIENT_SRV, &dnsIid, NULL);
			if(ret == ZCFG_SUCCESS){
				zcfgFeObjJsonGet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj);
				found = true;
			}
		}
			
		if(found == true && dnsObj != NULL){
			primdns = json_object_get_string(json_object_object_get(Jobj, "PrimDNSServer"));
			secdns = json_object_get_string(json_object_object_get(Jobj, "SecDNSServer"));

			json_object_object_add(dnsObj, "Enable", json_object_new_boolean(true));
			json_object_object_add(dnsObj, "Interface", json_object_new_string(ipIface));
			json_object_object_add(dnsObj, "Type", json_object_new_string("Static"));
			json_object_object_add(dnsObj, "X_ZYXEL_Type", json_object_new_string("Static"));

			if(json_object_object_get(Jobj, "PrimDNSServer")){
				if(!json_object_object_get(Jobj, "SecDNSServer"))
					json_object_object_add(dnsObj, "DNSServer", json_object_new_string(primdns));
				else{
					strcat(dnsServer,primdns);
					strcat(dnsServer,",");
					strcat(dnsServer,secdns);
			json_object_object_add(dnsObj, "DNSServer", json_object_new_string(dnsServer));
				}
			}
			zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsIid, dnsObj, NULL);
		}
	}else{
		if(found == true && dnsObj != NULL){//delete
				ret = zcfgFeObjJsonDel(RDM_OID_DNS_CLIENT_SRV, &dnsIid, NULL);
			}
		}

	zcfgFeJsonObjFree(dnsObj);
	return ret;
}

zcfgRet_t setPPPObj(struct json_object *Jobj, char *pppIface){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pppObj = NULL;
	objIndex_t pppIid = {0};
	const char *username = NULL;
	const char *passwrd = NULL;
	const char *connectionTrigger = NULL;
	const char *localIPAddress = NULL;
	bool ipStatic = false;
	int idleDisconnectTime = 0;
	
	IID_INIT(pppIid);
	sscanf(pppIface, "PPP.Interface.%hhu", &pppIid.idx[0]);
	pppIid.level = 1;

	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIid, &pppObj);

	if(ret == ZCFG_SUCCESS){
		username = json_object_get_string(json_object_object_get(Jobj, "Username"));
		passwrd = json_object_get_string(json_object_object_get(Jobj, "Password"));
		connectionTrigger = json_object_get_string(json_object_object_get(Jobj, "ConnectionTrigger"));
		idleDisconnectTime = json_object_get_int(json_object_object_get(Jobj, "IdleDisconnectTime"));
		ipStatic = json_object_get_boolean(json_object_object_get(Jobj, "IPStatic"));

		if(json_object_object_get(Jobj, "Username"))
		json_object_object_add(pppObj, "Username", json_object_new_string(username));
		if(json_object_object_get(Jobj, "Password"))
		json_object_object_add(pppObj, "Password", json_object_new_string(passwrd));

		if (ipStatic) {
			if(json_object_object_get(Jobj, "IPAddress")) {
				localIPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
				json_object_object_add(pppObj, "X_ZYXEL_LocalIPAddress", json_object_new_string(localIPAddress));
			}
		} else {
			json_object_object_add(pppObj, "X_ZYXEL_LocalIPAddress", json_object_new_string("0.0.0.0"));
		}
#ifdef CONFIG_ZYXEL_PPP_AUTH_MENU_CHAP_PAP_CTL
		const char *authProtocol = NULL;
		authProtocol = json_object_get_string(json_object_object_get(Jobj, "AuthenticationProtocol"));
		if(json_object_object_get(Jobj, "AuthenticationProtocol"))
			json_object_object_add(pppObj, "X_ZYXEL_AuthenticationProtocol", json_object_new_string(authProtocol));
#endif
		if(json_object_object_get(Jobj, "ConnectionTrigger")){
			if(!strcmp(connectionTrigger,"OnDemand"))
				json_object_object_add(pppObj, "IdleDisconnectTime", json_object_new_int(idleDisconnectTime*60));
			else 
				json_object_object_add(pppObj, "IdleDisconnectTime", json_object_new_int(0));
		json_object_object_add(pppObj, "ConnectionTrigger", json_object_new_string(connectionTrigger));
		}
		ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_PPP_IFACE, &pppIid, pppObj, NULL);
	}

	zcfgFeJsonObjFree(pppObj);
	return ret;
} 

zcfgRet_t setPingObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pingObj = NULL;
	objIndex_t pingIid = {0};
	bool enable = false;
	int interval = 0;
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
	int interval_backup = 0, interval_recovery = 0;
#endif
	int failLimit = 0;
	bool pingGw = false;
	const char *destination = NULL;
	
	IID_INIT(pingIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_W_W_A_N_PING_CHECK, &pingIid, &pingObj);

	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Ping_Enable"));
		interval = json_object_get_int(json_object_object_get(Jobj, "Ping_Interval"));
		failLimit = json_object_get_int(json_object_object_get(Jobj, "Ping_FailLimit"));
		pingGw = json_object_get_boolean(json_object_object_get(Jobj, "Ping_DefaultGateway"));
		destination = json_object_get_string(json_object_object_get(Jobj, "Ping_Destination"));
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
		interval_backup = json_object_get_int(json_object_object_get(Jobj, "Backup_Interval"));
		interval_recovery = json_object_get_int(json_object_object_get(Jobj, "Recovery_Interval"));	
#endif

		invalidCharFilter((char *)destination, 46);
		if(DomainFilter(destination) == false){
			if(IPFilter(destination) == false){
				printf("invalid IP Address...\n");
				return false;
			}
		}

		if(json_object_object_get(Jobj, "Ping_Enable"))
		json_object_object_add(pingObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Ping_Interval"))
		json_object_object_add(pingObj, "Interval", json_object_new_int(interval));
		if(json_object_object_get(Jobj, "Ping_FailLimit"))
		json_object_object_add(pingObj, "FailLimit", json_object_new_int(failLimit));
		if(json_object_object_get(Jobj, "Ping_DefaultGateway"))
		json_object_object_add(pingObj, "DefaultGateway", json_object_new_boolean(pingGw));
		if(json_object_object_get(Jobj, "Ping_Destination"))
		json_object_object_add(pingObj, "Destination", json_object_new_string(destination));
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
		if(json_object_object_get(Jobj, "Backup_Interval"))
			json_object_object_add(pingObj, "Backup_Interval", json_object_new_int(interval_backup));
		if(json_object_object_get(Jobj, "Recovery_Interval"))
			json_object_object_add(pingObj, "Recovery_Interval", json_object_new_int(interval_recovery));
#endif
		ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_ZY_W_W_A_N_PING_CHECK, &pingIid, pingObj, NULL);
	}

	zcfgFeJsonObjFree(pingObj);
	return ret;

}

zcfgRet_t setBudgetObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *budgetObj = NULL;
	objIndex_t budgetIid = {0};
	bool enable = false;
	bool timeEnable = false;
	int timeValue = 0;
	bool byteEnable = false;
	int  byteValue = 0;
	const char *byteMode = NULL;
	bool pktEnable = false;
	int pktValue = 0;
	const char *pktMode = NULL;
	int resetDay = 0;
	bool timeActionEnable = false;
	int timeActionValue = 0;
	bool byteActionEnable = false;
	int byteActionValue = 0;
	bool pktActionEnable =  false;
	int pktActionValue = 0; 
	bool dropWWAN = false;
	bool logEnable = false;
	int logInterval = 0;
	bool resetNow = false;		//ResetNow

	IID_INIT(budgetIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_W_W_A_N_BUDGET_CONTROL, &budgetIid, &budgetObj);
	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_Enable"));
		timeEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_TimeEnable"));
		timeValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_TimeValue"));
		byteEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_ByteEnable"));
		byteValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_ByteValue"));
		byteMode = json_object_get_string(json_object_object_get(Jobj, "Bdg_ByteMode"));
		pktEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_PktEnable"));
		pktValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_PktValue"));
		pktMode = json_object_get_string(json_object_object_get(Jobj, "Bdg_PktMode"));
		resetDay = json_object_get_int(json_object_object_get(Jobj, "Bdg_ResetDay"));
		timeActionEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_TimeActionEnable"));
		timeActionValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_TimeActionValue"));
		byteActionEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_ByteActionEnable"));
		byteActionValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_ByteActionValue"));
		pktActionEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_PktActionEnable"));
		pktActionValue = json_object_get_int(json_object_object_get(Jobj, "Bdg_PktActionValue"));
		dropWWAN = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_DropWWAN"));
		logEnable = json_object_get_boolean(json_object_object_get(Jobj, "Bdg_LogEnable"));
		logInterval = json_object_get_int(json_object_object_get(Jobj, "Bdg_LogInterval"));
		resetNow = json_object_get_int(json_object_object_get(Jobj, "ResetNow"));

		if(json_object_object_get(Jobj, "Bdg_Enable"))
		json_object_object_add(budgetObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Bdg_TimeEnable"))
		json_object_object_add(budgetObj, "TimeEnable", json_object_new_boolean(timeEnable));
		if(json_object_object_get(Jobj, "Bdg_TimeValue"))
		json_object_object_add(budgetObj, "TimeValue", json_object_new_int(timeValue));
		if(json_object_object_get(Jobj, "Bdg_ByteEnable"))
		json_object_object_add(budgetObj, "ByteEnable", json_object_new_boolean(byteEnable));
		if(json_object_object_get(Jobj, "Bdg_ByteValue"))
		json_object_object_add(budgetObj, "ByteValue", json_object_new_int(byteValue));
		if(json_object_object_get(Jobj, "Bdg_ByteMode"))
		json_object_object_add(budgetObj, "ByteMode", json_object_new_string(byteMode));
		if(json_object_object_get(Jobj, "Bdg_PktEnable"))
		json_object_object_add(budgetObj, "PktEnable", json_object_new_boolean(pktEnable));
		if(json_object_object_get(Jobj, "Bdg_PktValue"))
		json_object_object_add(budgetObj, "PktValue", json_object_new_int(pktValue));
		if(json_object_object_get(Jobj, "Bdg_PktMode"))
		json_object_object_add(budgetObj, "PktMode", json_object_new_string(pktMode));
		if(json_object_object_get(Jobj, "Bdg_ResetDay"))
		json_object_object_add(budgetObj, "ResetDay", json_object_new_int(resetDay));
		if(json_object_object_get(Jobj, "Bdg_TimeActionEnable"))
		json_object_object_add(budgetObj, "TimeActionEnable", json_object_new_boolean(timeActionEnable));
		if(json_object_object_get(Jobj, "Bdg_TimeActionValue"))
		json_object_object_add(budgetObj, "TimeActionValue", json_object_new_int(timeActionValue));
		if(json_object_object_get(Jobj, "Bdg_ByteActionEnable"))
		json_object_object_add(budgetObj, "ByteActionEnable", json_object_new_boolean(byteActionEnable));
		if(json_object_object_get(Jobj, "Bdg_ByteActionValue"))
		json_object_object_add(budgetObj, "ByteActionValue", json_object_new_int(byteActionValue));
		if(json_object_object_get(Jobj, "Bdg_PktActionEnable"))
		json_object_object_add(budgetObj, "PktActionEnable", json_object_new_boolean(pktActionEnable));
		if(json_object_object_get(Jobj, "Bdg_PktActionValue"))
		json_object_object_add(budgetObj, "PktActionValue", json_object_new_int(pktActionValue));
		if(json_object_object_get(Jobj, "Bdg_DropWWAN"))
		json_object_object_add(budgetObj, "DropWWAN", json_object_new_boolean(dropWWAN));
		if(json_object_object_get(Jobj, "Bdg_LogEnable"))
		json_object_object_add(budgetObj, "LogEnable", json_object_new_boolean(logEnable));
		if(json_object_object_get(Jobj, "Bdg_LogInterval"))
		json_object_object_add(budgetObj, "LogInterval", json_object_new_int(logInterval));
		if(json_object_object_get(Jobj, "ResetNow"))
		json_object_object_add(budgetObj, "ResetNow", json_object_new_int(resetNow));

		ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_ZY_W_W_A_N_BUDGET_CONTROL, &budgetIid, budgetObj, NULL);
	}
	zcfgFeJsonObjFree(budgetObj);
	return ret;
}

zcfgRet_t setEmailUPObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *emailUPObj = NULL;
	objIndex_t emailUPIid = {0};
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	bool enable = false;
	const char *subject = NULL;
	const char *senTo = NULL;
	const char *sericeRef = NULL;
	const char *event = NULL;
	bool found = false;
	
	IID_INIT(iid);
	IID_INIT(emailUPIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_MAIL_EVENT_CFG, &iid, &obj) == ZCFG_SUCCESS){
		event = json_object_get_string(json_object_object_get(obj, "Event"));
		if(!strcmp(event,"MAILSEND_EVENT_3G_UP")){
			memcpy(&emailUPIid, &iid, sizeof(objIndex_t));
			emailUPObj = obj;
			found = true;
			getIidLevel(&emailUPIid);
			break;	
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}

	if(!found){//POST
		IID_INIT(emailUPIid);
		ret = zcfgFeObjJsonAdd(RDM_OID_MAIL_EVENT_CFG, &emailUPIid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGet(RDM_OID_MAIL_EVENT_CFG, &emailUPIid, &emailUPObj);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(emailUPObj, "Event", json_object_new_string("MAILSEND_EVENT_3G_UP"));
				json_object_object_add(emailUPObj, "EmailBody", json_object_new_string("3G backup is up now."));
				found = true;
			}
		}
	}

	if(found){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Email_Enable"));
		subject = json_object_get_string(json_object_object_get(Jobj, "Email_Subject"));
		senTo = json_object_get_string(json_object_object_get(Jobj, "Email_To"));
		sericeRef = json_object_get_string(json_object_object_get(Jobj, "Email_SericeRef"));
		
		if(json_object_object_get(Jobj, "Email_Enable"))
		json_object_object_add(emailUPObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Email_To"))
		json_object_object_add(emailUPObj, "EmailTo", json_object_new_string(senTo));
		if(json_object_object_get(Jobj, "Email_Subject"))
		json_object_object_add(emailUPObj, "EmailSubject", json_object_new_string(subject));
		if(json_object_object_get(Jobj, "Email_SericeRef"))
		json_object_object_add(emailUPObj, "SericeRef", json_object_new_string(sericeRef));
		
		zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &emailUPIid, emailUPObj, NULL);
	}

	zcfgFeJsonObjFree(emailUPObj);
	return ret;
}

zcfgRet_t setEmailDownObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *emailDnObj = NULL;
	objIndex_t emailDnIid = {0};
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	bool enable = false;
	const char *subject = NULL;
	const char *senTo = NULL;
	const char *sericeRef = NULL;
	const char *event = NULL;
	bool found = false;

	IID_INIT(iid);
	IID_INIT(emailDnIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_MAIL_EVENT_CFG, &iid, &obj) == ZCFG_SUCCESS){
		event = json_object_get_string(json_object_object_get(obj, "Event"));
		if(!strcmp(event,"MAILSEND_EVENT_3G_DOWN")){
			memcpy(&emailDnIid, &iid, sizeof(objIndex_t));
			emailDnObj = obj;
			found = true;
			getIidLevel(&emailDnIid);
			break;	
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}

	if(!found){//POST
		IID_INIT(emailDnIid);
		ret = zcfgFeObjJsonAdd(RDM_OID_MAIL_EVENT_CFG, &emailDnIid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGet(RDM_OID_MAIL_EVENT_CFG, &emailDnIid, &emailDnObj);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(emailDnObj, "Event", json_object_new_string("MAILSEND_EVENT_3G_DOWN"));
				json_object_object_add(emailDnObj, "EmailBody", json_object_new_string("xDSL is back UP, 3G backup is down"));
				found = true;
			}
		}
	}

	if(found){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Email_Enable"));
		subject = json_object_get_string(json_object_object_get(Jobj, "Email_Subject"));
		senTo = json_object_get_string(json_object_object_get(Jobj, "Email_To"));
		sericeRef = json_object_get_string(json_object_object_get(Jobj, "Email_SericeRef"));
		
		if(json_object_object_get(Jobj, "Email_Enable"))
		json_object_object_add(emailDnObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Email_To"))
		json_object_object_add(emailDnObj, "EmailTo", json_object_new_string(senTo));
		if(json_object_object_get(Jobj, "Email_Subject"))
		json_object_object_add(emailDnObj, "EmailSubject", json_object_new_string(subject));
		if(json_object_object_get(Jobj, "Email_SericeRef"))
		json_object_object_add(emailDnObj, "SericeRef", json_object_new_string(sericeRef));
		
		zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &emailDnIid, emailDnObj, NULL);
	}

	zcfgFeJsonObjFree(emailDnObj);
	return ret;

}

zcfgRet_t setEmailBgObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *emailBgObj = NULL;
	objIndex_t emailBgIid = {0};
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	bool enable = false;
	const char *subject = NULL;
	const char *senTo = NULL;
	const char *sericeRef = NULL;
	const char *event = NULL;
	bool found = false;
	
	IID_INIT(iid);
	IID_INIT(emailBgIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_MAIL_EVENT_CFG, &iid, &obj) == ZCFG_SUCCESS){
		event = json_object_get_string(json_object_object_get(obj, "Event"));
		if(!strcmp(event,"MAILSEND_EVENT_3G_BUDGET")){
			memcpy(&emailBgIid, &iid, sizeof(objIndex_t));
			emailBgObj = obj;
			found = true;
			getIidLevel(&emailBgIid);
			break;	
		}
		else{
			zcfgFeJsonObjFree(obj);
		}
	}

	if(!found){//POST
		IID_INIT(emailBgIid);
		ret = zcfgFeObjJsonAdd(RDM_OID_MAIL_EVENT_CFG, &emailBgIid, NULL);
		if(ret == ZCFG_SUCCESS){
			ret = zcfgFeObjJsonGet(RDM_OID_MAIL_EVENT_CFG, &emailBgIid, &emailBgObj);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(emailBgObj, "Event", json_object_new_string("MAILSEND_EVENT_3G_BUDGET"));
				json_object_object_add(emailBgObj, "EmailBody", json_object_new_string("Take care of your budget. #{Msg}"));
				found = true;
			}
		}
	}

	if(found){
		enable = json_object_get_boolean(json_object_object_get(Jobj, "Email_BG_Enable"));
		subject = json_object_get_string(json_object_object_get(Jobj, "Email_BG_Subject"));
		senTo = json_object_get_string(json_object_object_get(Jobj, "Email_BG_To"));
		sericeRef = json_object_get_string(json_object_object_get(Jobj, "Email_BG_SericeRef"));
		
		if(json_object_object_get(Jobj, "Email_BG_Enable"))
		json_object_object_add(emailBgObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "Email_BG_To"))
		json_object_object_add(emailBgObj, "EmailTo", json_object_new_string(senTo));
		if(json_object_object_get(Jobj, "Email_BG_Subject"))
		json_object_object_add(emailBgObj, "EmailSubject", json_object_new_string(subject));
		if(json_object_object_get(Jobj, "Email_BG_SericeRef"))
		json_object_object_add(emailBgObj, "SericeRef", json_object_new_string(sericeRef));
		
		zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &emailBgIid, emailBgObj, NULL);
	}

	zcfgFeJsonObjFree(emailBgObj);
	return ret;
}

zcfgRet_t zcfgFeDal_WwanBackup_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *action = NULL;
	char ipIface[32] = {0};
	char pppIface[32] = {0};
	bool resetNow = false;
	resetNow = json_object_get_int(json_object_object_get(Jobj, "ResetNow"));

	ret = getIfPath(ipIface, pppIface);
	
	if(resetNow == false){
		if(ret == ZCFG_SUCCESS){
			setIPv4Obj(Jobj, action, ipIface);
			setDnsObj(Jobj, action, ipIface);
			setPPPObj(Jobj, pppIface);
		}
		setPingObj(Jobj);
		setBudgetObj(Jobj);
		set3GObj(Jobj);
		setEmailUPObj(Jobj);
		setEmailDownObj(Jobj);
		setEmailBgObj(Jobj);
	}
	else{
		resetNow = false;	
	}	
	return ret;	
}	

zcfgRet_t zcfgFeDal_WwanBackup_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *ipIfaceV4Obj = NULL, *dnsObj = NULL, *pppIfaceObj = NULL, *pingObj = NULL, *emailObj = NULL, *budgetObj = NULL;
	struct json_object *wwanBackupObj = NULL;
	objIndex_t wwanBackupIid = {0}, ipIfaceV4Iid = {0}, dnsIid = {0}, pppIfaceIid = {0}, pingIid = {0}, emailIid = {0}, budgetIid = {0};
	paramJobj = json_object_new_object();
	const char *curIpIface = NULL, *curPPPIface = NULL, *ConnectionTrigger = NULL;
	char *dnsServer = NULL, *secondaryDns = NULL, *primaryDns = NULL;
	char ipIface[32] = {0}, pppIface[32] = {0};	
	bool staticDns = false;
	int disconnectTime = 0;

	if(zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanBackupIid, &wwanBackupObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable_3G", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "Enable")));
		json_object_object_add(paramJobj, "DialNumber", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "DialNumber")));
		json_object_object_add(paramJobj, "APN", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "APN")));
		json_object_object_add(paramJobj, "PIN_Code", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "PIN")));
		json_object_object_add(paramJobj, "PIN_Verification", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "PIN_Verification")));
		json_object_object_add(paramJobj, "Manufacturer", JSON_OBJ_COPY(json_object_object_get(wwanBackupObj, "Manufacturer")));
		curIpIface = json_object_get_string(json_object_object_get(wwanBackupObj, "Interface"));
		curPPPIface = json_object_get_string(json_object_object_get(wwanBackupObj, "PPPInterface"));
		strcpy(ipIface, curIpIface);
		strcpy(pppIface, curPPPIface);

		zcfgFeJsonObjFree(wwanBackupObj);
	}

	ipIfaceV4Iid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceV4Iid.idx[0]);
	ipIfaceV4Iid.idx[1] = 1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE_V4_ADDR, &ipIfaceV4Iid, &ipIfaceV4Obj) == ZCFG_SUCCESS){
		Jadds(paramJobj, "IPAddress", Jgets(ipIfaceV4Obj, "IPAddress"));
		Jadds(paramJobj, "SubnetMask", Jgets(ipIfaceV4Obj, "SubnetMask"));
		if(!strcmp("Static", Jgets(ipIfaceV4Obj, "AddressingType"))){
			Jaddb(paramJobj,"IPStatic",true);
		}
		else{
			Jaddb(paramJobj,"IPStatic",false);
		}
		zcfgFeJsonObjFree(ipIfaceV4Obj);
	}
	else{
		Jaddb(paramJobj,"IPStatic",false);
		Jadds(paramJobj, "IPAddress", "");
		Jadds(paramJobj, "SubnetMask",  "");
	}

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DNS_CLIENT_SRV, &dnsIid, &dnsObj) == ZCFG_SUCCESS){
		if(!strcmp(ipIface,json_object_get_string(json_object_object_get(dnsObj, "Interface")))){
			dnsServer = (char *)json_object_get_string(json_object_object_get(dnsObj, "DNSServer"));
			primaryDns = strtok_r(dnsServer, ",", &secondaryDns);
			
			json_object_object_add(paramJobj, "PrimDNSServer", json_object_new_string(primaryDns));
			if(secondaryDns)
				json_object_object_add(paramJobj, "SecDNSServer", json_object_new_string(secondaryDns));
			else
				json_object_object_add(paramJobj, "SecDNSServer", json_object_new_string(""));

			if(!strcmp("Static", Jgets(dnsObj, "X_ZYXEL_Type"))){
				staticDns = true;
				Jaddb(paramJobj, "DNSStatic", true);
				json_object_object_add(paramJobj, "DNSType", json_object_new_string("Static DNS"));
			}
			else{
				staticDns = false;
				Jaddb(paramJobj, "DNSStatic", false);
			}
				
			zcfgFeJsonObjFree(dnsObj);
			break;
		}
		
		zcfgFeJsonObjFree(dnsObj);
	}
	if(!staticDns){
		Jaddb(paramJobj, "DNSStatic", false);
		Jadds(paramJobj, "PrimDNSServer", "");
		Jadds(paramJobj, "SecDNSServer", "");
	}

	pppIfaceIid.level = 1;
	sscanf(pppIface, "PPP.Interface.%hhu", &pppIfaceIid.idx[0]);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Username", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Username")));
		json_object_object_add(paramJobj, "Password", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "Password")));
#ifdef CONFIG_ZYXEL_PPP_AUTH_MENU_CHAP_PAP_CTL
		json_object_object_add(paramJobj, "AuthenticationProtocol", JSON_OBJ_COPY(json_object_object_get(pppIfaceObj, "X_ZYXEL_AuthenticationProtocol")));
#endif
		
		ConnectionTrigger = json_object_get_string(json_object_object_get(pppIfaceObj, "ConnectionTrigger"));
		Jadds(paramJobj, "ConnectionTrigger", ConnectionTrigger);
		if(!strcmp(ConnectionTrigger, "OnDemand")){
			disconnectTime = json_object_get_int(json_object_object_get(pppIfaceObj, "IdleDisconnectTime"));
			if(disconnectTime != 0)
				disconnectTime = disconnectTime/60;
			Jaddi(paramJobj, "IdleDisconnectTime", disconnectTime);
		}
		else{
			Jaddi(paramJobj, "IdleDisconnectTime", 0);
		}
			
		zcfgFeJsonObjFree(pppIfaceObj);
	}
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_W_W_A_N_PING_CHECK, &pingIid, &pingObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Ping_Enable", JSON_OBJ_COPY(json_object_object_get(pingObj, "Enable")));
		json_object_object_add(paramJobj, "Ping_Interval", JSON_OBJ_COPY(json_object_object_get(pingObj, "Interval")));
		json_object_object_add(paramJobj, "Ping_FailLimit", JSON_OBJ_COPY(json_object_object_get(pingObj, "FailLimit")));
		json_object_object_add(paramJobj, "Ping_DefaultGateway", JSON_OBJ_COPY(json_object_object_get(pingObj, "DefaultGateway")));
		json_object_object_add(paramJobj, "Ping_Destination", JSON_OBJ_COPY(json_object_object_get(pingObj, "Destination")));
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
		json_object_object_add(paramJobj, "Backup_Interval", JSON_OBJ_COPY(json_object_object_get(pingObj, "Backup_Interval")));
		json_object_object_add(paramJobj, "Recovery_Interval", JSON_OBJ_COPY(json_object_object_get(pingObj, "Recovery_Interval")));
#endif
		zcfgFeJsonObjFree(pingObj);
	}

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &emailIid, &emailObj) == ZCFG_SUCCESS){
		if(!strcmp("MAILSEND_EVENT_3G_UP", json_object_get_string(json_object_object_get(emailObj, "Event")))){
			json_object_object_add(paramJobj, "Email_Enable", JSON_OBJ_COPY(json_object_object_get(emailObj, "Enable")));
			json_object_object_add(paramJobj, "Email_To", JSON_OBJ_COPY(json_object_object_get(emailObj, "EmailTo")));
			json_object_object_add(paramJobj, "Email_Subject", JSON_OBJ_COPY(json_object_object_get(emailObj, "EmailSubject")));
			json_object_object_add(paramJobj, "Email_SericeRef", JSON_OBJ_COPY(json_object_object_get(emailObj, "SericeRef")));
		}
		if(!strcmp("MAILSEND_EVENT_3G_BUDGET", json_object_get_string(json_object_object_get(emailObj, "Event")))){
			json_object_object_add(paramJobj, "Email_BG_Enable", JSON_OBJ_COPY(json_object_object_get(emailObj, "Enable")));
			json_object_object_add(paramJobj, "Email_BG_To", JSON_OBJ_COPY(json_object_object_get(emailObj, "EmailTo")));
			json_object_object_add(paramJobj, "Email_BG_Subject", JSON_OBJ_COPY(json_object_object_get(emailObj, "EmailSubject")));
			json_object_object_add(paramJobj, "Email_BG_SericeRef", JSON_OBJ_COPY(json_object_object_get(emailObj, "SericeRef")));
		}
		zcfgFeJsonObjFree(emailObj);
	}

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_W_W_A_N_BUDGET_CONTROL, &budgetIid, &budgetObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Bdg_Enable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "Enable")));
		json_object_object_add(paramJobj, "Bdg_TimeEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "TimeEnable")));
		json_object_object_add(paramJobj, "Bdg_TimeValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "TimeValue")));
		json_object_object_add(paramJobj, "Bdg_ByteEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ByteEnable")));
		json_object_object_add(paramJobj, "Bdg_ByteValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ByteValue")));
		json_object_object_add(paramJobj, "Bdg_ByteMode", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ByteMode")));
		json_object_object_add(paramJobj, "Bdg_PktEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "PktEnable")));
		json_object_object_add(paramJobj, "Bdg_PktValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "PktValue")));
		json_object_object_add(paramJobj, "Bdg_PktMode", JSON_OBJ_COPY(json_object_object_get(budgetObj, "PktMode")));
		json_object_object_add(paramJobj, "Bdg_ResetDay", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ResetDay")));
		json_object_object_add(paramJobj, "Bdg_TimeActionEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "TimeActionEnable")));
		json_object_object_add(paramJobj, "Bdg_TimeActionValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "TimeActionValue")));
		json_object_object_add(paramJobj, "Bdg_ByteActionEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ByteActionEnable")));
		json_object_object_add(paramJobj, "Bdg_ByteActionValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "ByteActionValue")));
		json_object_object_add(paramJobj, "Bdg_PktActionEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "PktActionEnable")));
		json_object_object_add(paramJobj, "Bdg_PktActionValue", JSON_OBJ_COPY(json_object_object_get(budgetObj, "PktActionValue")));
		json_object_object_add(paramJobj, "Bdg_DropWWAN", JSON_OBJ_COPY(json_object_object_get(budgetObj, "DropWWAN")));
		json_object_object_add(paramJobj, "Bdg_LogEnable", JSON_OBJ_COPY(json_object_object_get(budgetObj, "LogEnable")));
		json_object_object_add(paramJobj, "Bdg_LogInterval", JSON_OBJ_COPY(json_object_object_get(budgetObj, "LogInterval")));
		zcfgFeJsonObjFree(budgetObj);
	}
	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowWwanBackup(struct json_object *Jarray){
	struct json_object *obj = NULL;
	const char *password = NULL, *pin = NULL, *byteMode = NULL, *pktMode = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	password = json_object_get_string(json_object_object_get(obj, "Password"));
	pin = json_object_get_string(json_object_object_get(obj, "PIN_Code"));
	if(!strcmp("DownUp", json_object_get_string(json_object_object_get(obj, "Bdg_ByteMode"))))
		byteMode = "Download/Upload";
	else if(!strcmp("Down", json_object_get_string(json_object_object_get(obj, "Bdg_ByteMode"))))
		byteMode = "Download";
	else if(!strcmp("Up", json_object_get_string(json_object_object_get(obj, "Bdg_ByteMode"))))
		byteMode = "Upload";
	if(!strcmp("DownUp", json_object_get_string(json_object_object_get(obj, "Bdg_PktMode"))))
		pktMode = "Download/Upload";
	else if(!strcmp("Down", json_object_get_string(json_object_object_get(obj, "Bdg_PktMode"))))
		pktMode = "Download";
	else if(!strcmp("Up", json_object_get_string(json_object_object_get(obj, "Bdg_PktMode"))))
		pktMode = "Upload";
	printf("Cellular Backup General: \n");
	printf("%-30s %-10s \n","Cellular Backup:", json_object_get_string(json_object_object_get(obj, "Enable_3G")));
	printf("\nTrigger Cellular backup when physical link of primary WAN is down \n");
	printf("%-30s %-10s \n","Ping Check:", json_object_get_string(json_object_object_get(obj, "Ping_Enable")));
	printf("%-30s %-10s \n","Check Cycle:", json_object_get_string(json_object_object_get(obj, "Ping_Interval")));
	printf("%-30s %-10s \n","Consecutive Fail:", json_object_get_string(json_object_object_get(obj, "Ping_FailLimit")));
#ifdef ZYXEL_CONFIGURABLE_WWANBACKUP_WAIT_TIME
	printf("%-30s %-10s \n","Backup Interval:", json_object_get_string(json_object_object_get(obj, "Backup_Interval")));
	printf("%-30s %-10s \n","Recovery Interval:", json_object_get_string(json_object_object_get(obj, "Recovery_Interval")));
#endif
	if(json_object_get_boolean(json_object_object_get(obj, "Ping_DefaultGateway")))
		printf("Ping Default Gateway. \n");
	else 
		printf("%-30s %-10s \n","Ping Host:", json_object_get_string(json_object_object_get(obj, "Ping_Destination")));
	
	printf("\nCellular Connection Settings: \n");
	if(!strcmp("",json_object_get_string(json_object_object_get(obj, "Manufacturer"))))
		printf("%-30s %-10s \n","Card Description:", "N/A");
	else
		printf("%-30s %-10s \n","Card Description:", json_object_get_string(json_object_object_get(obj, "Manufacturer")));
	printf("%-30s %-10s \n","Username:", json_object_get_string(json_object_object_get(obj, "Username")));
	if(!strcmp("",password))
		printf("%-30s \n","Password:");
	else
		printf("%-30s %-10s\n","Password :","********");
#ifdef CONFIG_ZYXEL_PPP_AUTH_MENU_CHAP_PAP_CTL
	printf("%-30s %-10s \n","Authentication:", json_object_get_string(json_object_object_get(obj, "AuthenticationProtocol")));
#endif
	if(!strcmp("",pin))
		printf("%-30s \n","PIN:");
	else
		printf("%-30s %-10s\n","PIN :","********");
	printf("%-30s %-10s \n","Dial String:", json_object_get_string(json_object_object_get(obj, "DialNumber")));
	printf("%-30s %-10s \n","APN:", json_object_get_string(json_object_object_get(obj, "APN")));

	if(strcmp("OnDemand", Jgets(obj, "ConnectionTrigger")))
		printf("%-30s %-10s \n","Connection:", "Nailed Up");
	else{
		printf("%-30s %-10s \n","Connection:", "On Demand");
		printf("%-3s %-20s %-6s minutes\n","","Max Idle Timeout:",json_object_get_string(json_object_object_get(obj, "IdleDisconnectTime")));
	}
	if(!Jgetb(obj, "IPStatic"))
		printf("Obtain an IP Address Automatically. \n");
	else{ 
		printf("Use the Following Static IP Address: \n");
		printf("%-3s %-20s %-6s\n","","IP Address:",json_object_get_string(json_object_object_get(obj, "IPAddress")));
		printf("%-3s %-20s %-6s\n","","Subnet Mask:",json_object_get_string(json_object_object_get(obj, "SubnetMask")));
	}
	if(!Jgetb(obj,"DNSStatic"))
		printf("Obtain DNS Info Dynamically. \n");
	else{ 
		printf("Use the Following Static DNS IP Address: \n");
		printf("%-3s %-20s %-6s\n","","Primary DNS Server:",json_object_get_string(json_object_object_get(obj, "PrimDNSServer")));
		printf("%-3s %-20s %-6s\n","","Secondary DNS Server:",json_object_get_string(json_object_object_get(obj, "SecDNSServer")));
	}
	if(!json_object_get_boolean(json_object_object_get(obj, "Email_Enable")))
		printf("%-30s %-10s \n","Enable E-mail Notification:", json_object_get_string(json_object_object_get(obj, "Email_Enable")));
	else{
		printf("%-30s %-10s \n","Enable E-mail Notification:", json_object_get_string(json_object_object_get(obj, "Email_Enable")));
		printf("%-30s %-10s \n","Mail Account:", json_object_get_string(json_object_object_get(obj, "Email_SericeRef")));
		printf("%-30s %-10s \n","Cellular Backup E-mail Title:", json_object_get_string(json_object_object_get(obj, "Email_Subject")));
		printf("%-30s %-10s \n","Send Notification to E-mail:", json_object_get_string(json_object_object_get(obj, "Email_To")));
	}
	
	printf("\nBudget Setup: \n");
	printf("%-30s %-10s \n","Enable Budget Control:", json_object_get_string(json_object_object_get(obj, "Bdg_Enable")));
	printf("%-10s %-8s %s %s\n","Time Budget:",json_object_get_string(json_object_object_get(obj, "Bdg_TimeEnable")),json_object_get_string(json_object_object_get(obj, "Bdg_TimeValue")),"hours per month.");
	printf("%-10s %-8s %s %s%s %s\n","Data Budget:",json_object_get_string(json_object_object_get(obj, "Bdg_ByteEnable")),json_object_get_string(json_object_object_get(obj, "Bdg_ByteValue")),"Mbytes ", byteMode,"per month.");
	printf("%-10s %-8s %s %s%s %s\n","Data Budget:",json_object_get_string(json_object_object_get(obj, "Bdg_PktEnable")),json_object_get_string(json_object_object_get(obj, "Bdg_PktValue")),"kPackets ", pktMode,"per month.");
	if(strcmp("0",json_object_get_string(json_object_object_get(obj, "Bdg_ResetDay"))))
		printf("Reset all budget counters on last day of the month.\n");
	else
		printf("Reset all budget counters on %s day of the month.\n",json_object_get_string(json_object_object_get(obj, "Bdg_ResetDay")));
	printf("\nActions before over budget:\n");	
	if(json_object_get_boolean(json_object_object_get(obj, "Bdg_TimeActionEnable")))
		printf("Enable %s of time budget.\n",json_object_get_string(json_object_object_get(obj, "Bdg_TimeActionValue")));
	else
		printf("Disable %s of time budget.\n",json_object_get_string(json_object_object_get(obj, "Bdg_TimeActionValue")));
	if(json_object_get_boolean(json_object_object_get(obj, "Bdg_ByteActionEnable")))
		printf("Enable %s of data budget (Mbytes).\n",json_object_get_string(json_object_object_get(obj, "Bdg_ByteActionValue")));
	else
		printf("Disable %s of data budget (Mbytes).\n",json_object_get_string(json_object_object_get(obj, "Bdg_ByteActionValue")));
	if(json_object_get_boolean(json_object_object_get(obj, "Bdg_PktActionEnable")))
		printf("Enable %s of data budget (Packets).\n",json_object_get_string(json_object_object_get(obj, "Bdg_PktActionValue")));
	else
		printf("Disable %s of data budget (Packets).\n",json_object_get_string(json_object_object_get(obj, "Bdg_PktActionValue")));
	printf("\nActions when over budget:\n");
	if(json_object_get_boolean(json_object_object_get(obj, "Bdg_DropWWAN")))
		printf("Current Cellular Connection Drop. \n");
	else
		printf("Current Cellular Connection Keep. \n");
	if(json_object_get_boolean(json_object_object_get(obj, "Bdg_LogEnable")))
		printf("Enable Log: Interval %s minutes. \n",json_object_get_string(json_object_object_get(obj, "Bdg_LogInterval")));
	else
		printf("Disable Log. \n");
	if(!json_object_get_boolean(json_object_object_get(obj, "Email_BG_Enable")))
		printf("%-30s %-10s \n","Enable E-mail Notification:", json_object_get_string(json_object_object_get(obj, "Email_BG_Enable")));
	else{
		printf("%-30s %-10s \n","Enable E-mail Notification:", json_object_get_string(json_object_object_get(obj, "Email_BG_Enable")));
		printf("%-30s %-10s \n","Mail Account:", json_object_get_string(json_object_object_get(obj, "Email_BG_SericeRef")));
		printf("%-30s %-10s \n","Cellular Backup E-mail Title:", json_object_get_string(json_object_object_get(obj, "Email_BG_Subject")));
		printf("%-30s %-10s \n","Send Notification to E-mail:", json_object_get_string(json_object_object_get(obj, "Email_BG_To")));
	}
	
}



zcfgRet_t zcfgFeDalWwanBackup(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_WwanBackup_Edit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_WwanBackup_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

