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

#define LEVEL_NAME_UD "UserDefined"

dal_param_t FIREWALL_param[] =
{
	{"initFirewall", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Firewall_Enable", 	dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"IPv4_Enable", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"IPv6_Enable", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{"AdvancedLevel", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Level_GUI",			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
#ifdef ABUE_CUSTOMIZATION	
	{"Level",				dalType_string, 	0, 	0, 	NULL,	"Low|Medium|High|Custom",	0},
#else	
	{"Level",				dalType_string, 	0, 	0, 	NULL,	"Low|Medium|High",	0},
#endif	
	{"Config", 				dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Level_Name", 			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Description", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Order", 				dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_Enable_Firewall", dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"RSA_Telkom", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Chain_Enable", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"Chain_Name", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"initChain", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"enableDos", 			dalType_boolean, 	0, 	0, 	NULL,	NULL,	0},
	{NULL, 			0, 					0, 	0, 	NULL}
};

zcfgRet_t zcfgFeDalShowFirewall(struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	const char *filewalllevel = NULL;
	const char *level = NULL;
	const char *wantolan = NULL;
	const char *lantowan = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return ZCFG_INTERNAL_ERROR;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	filewalllevel = json_object_get_string(json_object_object_get(obj, "Level_GUI"));
	if(!strcmp(filewalllevel, "Off")){
		level = "Low";
		wantolan = "YES";
		lantowan = "YES";
	}
	else if(!strcmp(filewalllevel, "Low")){
		level = "Medium(Recommended)";
		wantolan = "NO";
		lantowan = "YES";
	}
	else if(!strcmp(filewalllevel, "High")){
		level = "High";
		wantolan = "NO";
		lantowan = "NO";
	}
#ifdef ABUE_CUSTOMIZATION		
	else if(!strcmp(filewalllevel, "Custom")){
		level = "Custom";
		wantolan = "NO";
		lantowan = "YES";
	}
#endif

	printf("%-20s  %-10s \n", "IPv4 Firewall:", json_object_get_string(json_object_object_get(obj, "IPv4_Enable")));
	printf("%-20s  %-10s \n", "IPv6 Firewall:", json_object_get_string(json_object_object_get(obj, "IPv6_Enable")));
	printf("\n%-20s  %-10s \n", "Firewall Level:", level);
	printf("%-20s  %-10s \n", "Lan to WAN:", lantowan);
	printf("%-20s  %-10s \n", "Wan to LAN:", wantolan);
	printf("\n%-20s %-10s \n","DoS Protection Blocking :",json_object_get_string(json_object_object_get(obj, "enableDos")));
	
	return ret;
}


zcfgRet_t zcfgFeDal_Firewall_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *levelObj = NULL;
	objIndex_t levelIid = {0};
	const char *level = NULL;
	const char *name = NULL;
	const char *description = NULL;
	int order = 0;

	IID_INIT(levelIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_FIREWALL_LEVEL, &levelIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj);
	}

	if(ret == ZCFG_SUCCESS){
		name = json_object_get_string(json_object_object_get(Jobj, "Level_Name"));
		level = json_object_get_string(json_object_object_get(Jobj, "Level_GUI"));
		description = json_object_get_string(json_object_object_get(Jobj, "Description"));
		order = json_object_get_int(json_object_object_get(Jobj, "Order"));
		

		json_object_object_add(levelObj, "Order", json_object_new_int(order));
		json_object_object_add(levelObj, "Name", json_object_new_string(name));
		json_object_object_add(levelObj, "X_ZYXEL_DefaultConfig", json_object_new_string(level));
		json_object_object_add(levelObj, "Description", json_object_new_string(description));
		
		ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL_LEVEL, &levelIid, levelObj, NULL);
	}

	zcfgFeJsonObjFree(levelObj);
	return ret;
}

zcfgRet_t setFirewallEnable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *firewallObj = NULL;
	objIndex_t firewallIid = {0};
	bool v4Enable = false, origv4Enable = false;
	bool v6Enable = false, origv6Enable = false;
	bool firewallEnable = false;
	bool initFirewall = false;
	const char *config = NULL;
	const char *advancedLevel = NULL;
	
	
	IID_INIT(firewallIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL, &firewallIid, &firewallObj);

	if(ret == ZCFG_SUCCESS ){
		initFirewall = json_object_get_boolean(json_object_object_get(Jobj, "initFirewall"));

		if(initFirewall){
			config = json_object_get_string(json_object_object_get(Jobj, "Config"));
			advancedLevel = json_object_get_string(json_object_object_get(Jobj, "AdvancedLevel"));
			json_object_object_add(firewallObj, "Config", json_object_new_string(config));
			json_object_object_add(firewallObj, "AdvancedLevel", json_object_new_string(advancedLevel));
		}else{
			v4Enable = json_object_get_boolean(json_object_object_get(Jobj, "IPv4_Enable"));
			origv4Enable = json_object_get_boolean(json_object_object_get(firewallObj, "IPv4_Enable"));
			v6Enable = json_object_get_boolean(json_object_object_get(Jobj, "IPv6_Enable"));
			origv6Enable = json_object_get_boolean(json_object_object_get(firewallObj, "IPv6_Enable"));
			firewallEnable = json_object_get_boolean(json_object_object_get(Jobj, "Firewall_Enable"));

			if(json_object_object_get(Jobj, "IPv4_Enable") && !json_object_object_get(Jobj, "IPv6_Enable"))
				v6Enable = origv6Enable;
			else if(!json_object_object_get(Jobj, "IPv4_Enable") && json_object_object_get(Jobj, "IPv6_Enable"))
				v4Enable = origv4Enable;
			else if(!json_object_object_get(Jobj, "IPv4_Enable") && !json_object_object_get(Jobj, "IPv6_Enable")){
				v4Enable = origv4Enable;
				v6Enable = origv6Enable;
			}
			if(!v4Enable && !v6Enable)
				firewallEnable = false;
			else 
				firewallEnable = true;

			json_object_object_add(firewallObj, "IPv4_Enable", json_object_new_boolean(v4Enable));
			json_object_object_add(firewallObj, "IPv6_Enable", json_object_new_boolean(v6Enable));
			json_object_object_add(firewallObj, "Enable", json_object_new_boolean(firewallEnable));
		}
		ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL, &firewallIid, firewallObj, NULL);
	}

	zcfgFeJsonObjFree(firewallObj);
	return ret;
}


zcfgRet_t setFirewallLevel(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *levelObj = NULL;
	objIndex_t levelIid = {0};
	struct json_object *intfObj = NULL;
	objIndex_t intfIid = {0};
	const char *level = NULL;
	const char *clilevel = NULL;
	const char *name = NULL;
	const char *Ifname = NULL;
	bool zyxelFirewallEnable = false;
	bool found = false;
	bool RSA_TAAAA = false;
#ifdef ABUE_CUSTOMIZATION	
	struct json_object *FiremallACLJobj = NULL;
	bool enable = false;
	objIndex_t FiremallACLIid;
#endif	


	// RSA TAAAA - Firewall per Interface
	RSA_TAAAA = json_object_get_boolean(json_object_object_get(Jobj, "RSA_Telkom"));
	if(RSA_TAAAA){
		found = false;
		zyxelFirewallEnable = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_Enable_Firewall"));
		IID_INIT(intfIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &intfIid, &intfObj) == ZCFG_SUCCESS) {
				Ifname = json_object_get_string(json_object_object_get(intfObj, "X_ZYXEL_IfName"));
				if(strncmp(Ifname, "br", 2)){
					json_object_object_add(intfObj, "X_ZYXEL_Enable_Firewall", json_object_new_boolean(zyxelFirewallEnable));
					zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &intfIid, intfObj, NULL);
				}
		}
		zcfgFeJsonObjFree(intfObj);
	}
	IID_INIT(levelIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(levelObj, "Name"));
		if(!strcmp(name, LEVEL_NAME_UD)){
			found = true;
			break;
		}
	}
	if(json_object_object_get(Jobj, "Level_GUI"))
		level = json_object_get_string(json_object_object_get(Jobj, "Level_GUI"));
	else if(json_object_object_get(Jobj, "Level")){
		clilevel = json_object_get_string(json_object_object_get(Jobj, "Level"));
		if(!strcmp(clilevel, "Low"))
			level = "Off";
		else if(!strcmp(clilevel, "Medium"))
			level = "Low";
		else if(!strcmp(clilevel, "High"))
			level = "High";
#ifdef ABUE_CUSTOMIZATION 		
		else if(!strcmp(clilevel, "Custom"))
			level = "Custom";
#endif		
	}
	if(found && level){

		json_object_object_add(levelObj, "X_ZYXEL_DefaultConfig", json_object_new_string(level));
		ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL_LEVEL, &levelIid, levelObj, NULL);
	}
#ifdef ABUE_CUSTOMIZATION 
	if(!strcmp(level, "Custom"))
		enable = true;
	else
		enable = false;

	IID_INIT(FiremallACLIid);
	FiremallACLIid.level = 2;
	FiremallACLIid.idx[0] = 1;

	while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_CHAIN_RULE, &FiremallACLIid, &FiremallACLJobj) == ZCFG_SUCCESS){	
		if(json_object_object_get(FiremallACLJobj, "Enable")){
			json_object_object_add(FiremallACLJobj, "Enable", json_object_new_boolean(enable));		
			zcfgFeObjJsonSet(RDM_OID_FIREWALL_CHAIN_RULE, &FiremallACLIid, FiremallACLJobj, NULL);
		}	
	}	
	zcfgFeJsonObjFree(FiremallACLJobj);	
#endif	
	zcfgFeJsonObjFree(levelObj);
	return ret;
}

zcfgRet_t setFirewallChain(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *chainObj = NULL;
	struct json_object *levelObj = NULL;
	objIndex_t chainIid = {0};
	objIndex_t levelIid = {0};
	const char *name = NULL;
	bool enable = false;
	bool found = false;
	char chain[32] = {0};
	int chainIdx = 0;

	IID_INIT(levelIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(levelObj, "Name"));
		if(!strcmp(name, LEVEL_NAME_UD)){
			strcpy(chain, json_object_get_string(json_object_object_get(levelObj, "Chain")));
			chainIdx = atoi(&chain[15]);
			break;
		}
	}


	IID_INIT(chainIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_FIREWALL_CHAIN, &chainIid, &chainObj) == ZCFG_SUCCESS) {
		name = json_object_get_string(json_object_object_get(chainObj, "Name"));
		if(chainIid.idx[0] == chainIdx){
			found = true;
			break;
		}
	}

	if(found){
		name = json_object_get_string(json_object_object_get(Jobj, "Chain_Name"));
		enable = json_object_get_string(json_object_object_get(Jobj, "Chain_Enable"));
		json_object_object_add(chainObj, "Name", json_object_new_string(name));
		json_object_object_add(chainObj, "Enable", json_object_new_boolean(enable));
		ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL_CHAIN, &chainIid, chainObj, NULL);
	}


	zcfgFeJsonObjFree(levelObj);
	zcfgFeJsonObjFree(chainObj);
	return ret;
}

zcfgRet_t setDoSEnable(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *DosObj = NULL;
	objIndex_t DosIid = {0};
	bool enableDos = false;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL, &DosIid, &DosObj) == ZCFG_SUCCESS){
		enableDos = json_object_get_boolean(json_object_object_get(Jobj, "enableDos"));
		json_object_object_add(DosObj, "X_ZYXEL_Dos_Enable", json_object_new_boolean(enableDos));
		ret = zcfgFeObjJsonSet(RDM_OID_FIREWALL, &DosIid, DosObj, NULL);
		zcfgFeJsonObjFree(DosObj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool initChain = false;

	initChain = json_object_get_boolean(json_object_object_get(Jobj, "initChain"));
	if(initChain){
		setFirewallChain(Jobj);	
		return ret;
	}
	if(json_object_object_get(Jobj, "enableDos"))
		setDoSEnable(Jobj);
	setFirewallEnable(Jobj);
	setFirewallLevel(Jobj);	
	return ret;
}

zcfgRet_t zcfgFeDal_Firewall_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *firewallObj = NULL;
	struct json_object *levelObj = NULL;
	objIndex_t firewallIid = {0};
	objIndex_t levelIid = {0};
	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_FIREWALL, &firewallIid, &firewallObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(firewallObj, "Enable")));
		json_object_object_add(paramJobj, "IPv4_Enable", JSON_OBJ_COPY(json_object_object_get(firewallObj, "IPv4_Enable")));
		json_object_object_add(paramJobj, "IPv6_Enable", JSON_OBJ_COPY(json_object_object_get(firewallObj, "IPv6_Enable")));
		json_object_object_add(paramJobj, "enableDos", JSON_OBJ_COPY(json_object_object_get(firewallObj, "X_ZYXEL_Dos_Enable")));
		
		zcfgFeJsonObjFree(firewallObj);
	}
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_FIREWALL_LEVEL, &levelIid, &levelObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Level_GUI", JSON_OBJ_COPY(json_object_object_get(levelObj, "X_ZYXEL_DefaultConfig")));

		zcfgFeJsonObjFree(levelObj);
	}
	json_object_array_add(Jarray, paramJobj);

	
	return ret;
}


zcfgRet_t zcfgFeDalFirewall(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Firewall_Edit(Jobj, NULL);
	}else if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_Firewall_Add(Jobj, NULL);
	}else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_Firewall_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


