#include <ctype.h>
#include <unistd.h>	// for sleep()
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

dal_param_t SNMP_param[]={
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
/* set RDM_OID_ZY_SNMP object */
{"Enable", 		dalType_boolean,	0,	0,	NULL},
{"GetCommunity", dalType_string, 	0, 15,	NULL},
{"SetCommunity", dalType_string, 	0, 15, NULL},
{"TrapCommunity", dalType_string, 	0, 15, NULL},
#endif
#endif
/* set RDM_OID_SYSTEM_INFO object */
{"HostName", 	dalType_string, 	0, 30, NULL},
{"DomainName",	dalType_string, 	0, 30, NULL},	// system info
#ifdef ZYXEL_LED_SWITCH_ACTIVE
{"ledAllEnable",dalType_boolean,	0,	0,	NULL},
#endif
{"Location", 	dalType_string,		0, 15, NULL},
{"Contact", 	dalType_string,		0, 50, NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
/* set RDM_OID_TRAP_ADDR object */
{"TrapDest",		dalType_v4v6Addr,	0, 0, NULL,	"None|undefined",	0},	// string "None" is for CLI use, "undefined" is for GUI use.
{"Port", 		dalType_int,		0, 65535, NULL},
#endif
#endif
{NULL, 			0, 					0, 0, NULL}
};

zcfgRet_t setSNMPObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *snmpObj = NULL;
	objIndex_t snmpIid = {0};
	bool enable = false;
	const char *ROCommunity = NULL;
	const char *RWCommunity = NULL;
	const char *TrapCommunity = NULL;

	enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	ROCommunity = json_object_get_string(json_object_object_get(Jobj, "GetCommunity"));
	RWCommunity = json_object_get_string(json_object_object_get(Jobj, "SetCommunity"));
	TrapCommunity = json_object_get_string(json_object_object_get(Jobj, "TrapCommunity"));

	IID_INIT(snmpIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SNMP, &snmpIid, &snmpObj);

	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(snmpObj, "Enable", json_object_new_boolean(enable));
		if(json_object_object_get(Jobj, "GetCommunity"))
		json_object_object_add(snmpObj, "ROCommunity", json_object_new_string(ROCommunity));
		if(json_object_object_get(Jobj, "SetCommunity"))
		json_object_object_add(snmpObj, "RWCommunity", json_object_new_string(RWCommunity));
		if(json_object_object_get(Jobj, "TrapCommunity"))
		json_object_object_add(snmpObj, "TrapCommunity", json_object_new_string(TrapCommunity));

		zcfgFeObjJsonSet(RDM_OID_ZY_SNMP, &snmpIid, snmpObj, NULL);
	}

	zcfgFeJsonObjFree(snmpObj);
	return ret;
}

zcfgRet_t setSysInfoObj(struct json_object *Jobj, boolean *domainNameChange){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object * SysInfoObj = NULL;
	objIndex_t SysInfoIid = {0};
	const char *HostName = NULL;
	const char *Location = NULL;
	const char *Contact = NULL;
	const char *domainName = NULL;
#ifdef ZYXEL_LED_SWITCH_ACTIVE
	bool ledAllEnable =true;
#endif
	const char *oridomainname = NULL;

	HostName = json_object_get_string(json_object_object_get(Jobj, "HostName"));
	Location = json_object_get_string(json_object_object_get(Jobj, "Location"));
	Contact = json_object_get_string(json_object_object_get(Jobj, "Contact"));
	domainName = json_object_get_string(json_object_object_get(Jobj, "DomainName"));

	invalidCharFilter((char *)HostName, 31);
	if(HostNameFilter(HostName) == false){
		printf("invalid HostName...\n");
		return false;		
	}

#ifdef ZYXEL_LED_SWITCH_ACTIVE	
	ledAllEnable = json_object_get_boolean(json_object_object_get(Jobj, "ledAllEnable"));
#endif
	IID_INIT(SysInfoIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SYSTEM_INFO, &SysInfoIid, &SysInfoObj);

	if(ret == ZCFG_SUCCESS){
		oridomainname = json_object_get_string(json_object_object_get(SysInfoObj, "DomainName"));
		if(strcmp(oridomainname,domainName))
			*domainNameChange = true;

		if(json_object_object_get(Jobj, "HostName"))
			json_object_object_add(SysInfoObj, "HostName", json_object_new_string(HostName));
		if(json_object_object_get(Jobj, "Location"))
			json_object_object_add(SysInfoObj, "Location", json_object_new_string(Location));
		if(json_object_object_get(Jobj, "Contact"))
			json_object_object_add(SysInfoObj, "Contact", json_object_new_string(Contact));
		if(json_object_object_get(Jobj, "DomainName"))
			json_object_object_add(SysInfoObj, "DomainName", json_object_new_string(domainName));
#ifdef ZYXEL_LED_SWITCH_ACTIVE		
		if(json_object_object_get(Jobj, "ledAllEnable")){
			json_object_object_add(SysInfoObj, "ledAllEnable", json_object_new_boolean(ledAllEnable));
		}
#endif
		zcfgFeObjJsonSet(RDM_OID_SYSTEM_INFO, &SysInfoIid, SysInfoObj, NULL);
	}

	zcfgFeJsonObjFree(SysInfoObj);
	return ret;
}

zcfgRet_t setTrap_addrObj(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *trapObj = NULL;
	objIndex_t trapIid = {0};
	const char *host = NULL;
	int port = 0;
	bool found = false;

	
	IID_INIT(trapIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_TRAP_ADDR, &trapIid, &trapObj) == ZCFG_SUCCESS){
		found = true;
		break;
	}

	if(found == false && trapObj == NULL){
		IID_INIT(trapIid);
		ret = zcfgFeObjJsonAdd(RDM_OID_TRAP_ADDR, &trapIid, NULL);
		if(ret == ZCFG_SUCCESS){
			zcfgFeObjJsonGet(RDM_OID_TRAP_ADDR, &trapIid, &trapObj);
			found = true;
		}
	}

	if(found == true && trapObj != NULL){
		host = json_object_get_string(json_object_object_get(Jobj, "TrapDest"));
		port = json_object_get_int(json_object_object_get(Jobj, "Port"));
		if(json_object_object_get(Jobj, "TrapDest")){
			if(!strcmp(host, "undefined") || !strcmp(host, "None")){
				ret = zcfgFeObjJsonDel(RDM_OID_TRAP_ADDR, &trapIid, NULL);
			}else{
				if(json_object_object_get(Jobj, "TrapDest"))
					json_object_object_add(trapObj, "Host", json_object_new_string(host));
				if(json_object_object_get(Jobj, "Port"))
					json_object_object_add(trapObj, "Port", json_object_new_int(port));
				else if(json_object_object_get(Jobj, "TrapDest") && !json_object_object_get(Jobj, "Port"))
					json_object_object_add(trapObj, "Port", json_object_new_int(162));
				zcfgFeObjJsonSet(RDM_OID_TRAP_ADDR, &trapIid, trapObj, NULL);
			}
		}
	}

	zcfgFeJsonObjFree(trapObj);
	return ret;
}

zcfgRet_t zcfgFeDal_SNMP_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	boolean domainNameChange=false;
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
	setSNMPObj(Jobj);
	setTrap_addrObj(Jobj);
#endif
#endif
	ret = setSysInfoObj(Jobj, &domainNameChange);

	/*
	Systeminfo backend(beSysInfoConfigLoad) will Power down/up lan eth interface to trigger Lan Host send DHCP request, so waiting 10 secs for it if user change domain name.
	*/
	if(domainNameChange) 
		sleep(10);

	return ret;
}

void zcfgFeDalShowSNMP(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
		if(json_object_get_boolean(json_object_object_get(obj,"Enable")))
			printf("%-20s: Enabled\n","SNMP Agent");
		else
			printf("%-20s: Disabled\n","SNMP Agent");

		printf("%-20s: %s\n","Get Community",json_object_get_string(json_object_object_get(obj, "GetCommunity")));
		printf("%-20s: %s\n","Set Community",json_object_get_string(json_object_object_get(obj, "SetCommunity")));
		printf("%-20s: %s\n","Trap Community",json_object_get_string(json_object_object_get(obj, "TrapCommunity")));
#endif
#endif
		printf("%-20s: %s\n","System Name",json_object_get_string(json_object_object_get(obj, "HostName")));
		printf("%-20s: %s\n","System Location",json_object_get_string(json_object_object_get(obj, "Location")));
		printf("%-20s: %s\n","System Contact",json_object_get_string(json_object_object_get(obj, "Contact")));
		printf("%-20s: %s\n","Domain Name",json_object_get_string(json_object_object_get(obj, "DomainName")));
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
		printf("%-20s: %s\n","Trap Destination",json_object_get_string(json_object_object_get(obj, "TrapDest")));
#endif
#endif
	}
}

zcfgRet_t zcfgFeDal_SNMP_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *retObj = NULL;

	/* RDM_OID_ZY_SNMP */
	struct json_object *snmpObj = NULL;
	objIndex_t snmpIid = {0};
	bool enable = false;
	const char *ROCommunity = NULL;
	const char *RWCommunity = NULL;
	const char *TrapCommunity = NULL;

	/* RDM_OID_SYSTEM_INFO */
	struct json_object * SysInfoObj = NULL;
	objIndex_t SysInfoIid = {0};
	const char *HostName = NULL;
	const char *Location = NULL;
	const char *Contact = NULL;
	const char *domainname = NULL;
#ifdef ZYXEL_LED_SWITCH_ACTIVE	
	bool ledAllEnable = true;
#endif
	/* RDM_OID_TRAP_ADDR */
	struct json_object *trapObj = NULL;
	objIndex_t trapIid = {0};
	const char *host = NULL;
	int port = 0;
	char dest[32] = {0};

	retObj = json_object_new_object();
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
	IID_INIT(snmpIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_SNMP, &snmpIid, &snmpObj);
	if(ret == ZCFG_SUCCESS){
		enable = json_object_get_boolean(json_object_object_get(snmpObj, "Enable"));
		ROCommunity = json_object_get_string(json_object_object_get(snmpObj, "ROCommunity"));
		RWCommunity = json_object_get_string(json_object_object_get(snmpObj, "RWCommunity"));
		TrapCommunity = json_object_get_string(json_object_object_get(snmpObj, "TrapCommunity"));		
		json_object_object_add(retObj, "Enable", json_object_new_boolean(enable));
		json_object_object_add(retObj, "GetCommunity", json_object_new_string(ROCommunity));
		json_object_object_add(retObj, "SetCommunity", json_object_new_string(RWCommunity));
		json_object_object_add(retObj, "TrapCommunity", json_object_new_string(TrapCommunity));
		zcfgFeJsonObjFree(snmpObj);
	}
#endif
#endif
	IID_INIT(SysInfoIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SYSTEM_INFO, &SysInfoIid, &SysInfoObj);
	if(ret == ZCFG_SUCCESS){
		HostName = json_object_get_string(json_object_object_get(SysInfoObj, "HostName"));
		Location = json_object_get_string(json_object_object_get(SysInfoObj, "Location"));
		Contact = json_object_get_string(json_object_object_get(SysInfoObj, "Contact"));
		domainname = json_object_get_string(json_object_object_get(SysInfoObj, "DomainName"));
#ifdef ZYXEL_LED_SWITCH_ACTIVE
		ledAllEnable = json_object_get_boolean(json_object_object_get(SysInfoObj, "ledAllEnable"));
#endif
		json_object_object_add(retObj, "HostName", json_object_new_string(HostName));
		json_object_object_add(retObj, "DomainName", json_object_new_string(domainname));
#ifdef ZYXEL_LED_SWITCH_ACTIVE
		json_object_object_add(retObj, "ledAllEnable", json_object_new_boolean(ledAllEnable));
#endif
		json_object_object_add(retObj, "Location", json_object_new_string(Location));
		json_object_object_add(retObj, "Contact", json_object_new_string(Contact));
		zcfgFeJsonObjFree(SysInfoObj);
	}
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#ifndef ABQA_CUSTOMIZATION
	IID_INIT(trapIid);
	ret = zcfgFeObjJsonGetNext(RDM_OID_TRAP_ADDR, &trapIid, &trapObj);
	if(ret == ZCFG_SUCCESS){
		host = json_object_get_string(json_object_object_get(trapObj, "Host"));
		port = json_object_get_int(json_object_object_get(trapObj, "Port"));
		if(port != 162){
			sprintf(dest,"%s:%d",host,port);
			json_object_object_add(retObj, "TrapDest", json_object_new_string(dest));
		}
		else
			json_object_object_add(retObj, "TrapDest", json_object_new_string(host));
		zcfgFeJsonObjFree(trapObj);
	}
	else if(trapObj==NULL){
		json_object_object_add(retObj, "TrapDest", json_object_new_string(""));
		dbg_printf("Fail to get object..\n");
	}
#endif
#endif
	json_object_array_add(Jarray, retObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalSNMP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_SNMP_Edit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_SNMP_Get(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

