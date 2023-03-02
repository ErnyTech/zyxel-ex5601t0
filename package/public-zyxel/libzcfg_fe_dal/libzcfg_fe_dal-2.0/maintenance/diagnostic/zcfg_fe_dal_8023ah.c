
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zlog_api.h"

dal_param_t IEEE_8023AH_param[]={
	{"Enabled",								dalType_boolean,0,	0,	NULL},
	{"OAMID",								dalType_int,	0,	0,	NULL},
	{"Interface",							dalType_string,	0,	0,	NULL},
	{"VariableRetrievalEnabled",			dalType_boolean,0,	0,	NULL},
	{"LinkEventsEnabled",					dalType_boolean,0,	0,	NULL},
	{"RemoteLoopbackEnabled",				dalType_boolean,0,	0,	NULL},
	{"ActiveModeEnabled",					dalType_boolean,0,	0,	NULL},
	{"AutoEventEnabled",					dalType_boolean,0,	0,	NULL},
	{NULL,									0,				0,	0,	NULL},
};

struct json_object *ieee8023ahobj;

objIndex_t ieee8023ahIid;

extern uint32_t flag1;

void freeAll8023ahObjects(){
	if(ieee8023ahobj) json_object_put(ieee8023ahobj);

	return;
}

void init8023ahGlobalObjects(){
	ieee8023ahobj = NULL;

	return;
}

bool Enabled;
unsigned int OAMID;
char *Interface;
bool VariableRetrievalEnabled;
bool LinkEventsEnabled;
bool RemoteLoopbackEnabled;
bool ActiveModeEnabled;
bool AutoEventEnabled;

void get8023ahBasicInfo(struct json_object *Jobj){
	Enabled = json_object_get_boolean(json_object_object_get(Jobj, "Enabled"));
	OAMID = json_object_get_int(json_object_object_get(Jobj, "OAMID"));
	Interface = (char *)json_object_get_string(json_object_object_get(Jobj, "Interface"));
	VariableRetrievalEnabled = json_object_get_boolean(json_object_object_get(Jobj, "VariableRetrievalEnabled"));
	LinkEventsEnabled = json_object_get_boolean(json_object_object_get(Jobj, "LinkEventsEnabled"));
	RemoteLoopbackEnabled = json_object_get_boolean(json_object_object_get(Jobj, "RemoteLoopbackEnabled"));
	ActiveModeEnabled = json_object_get_boolean(json_object_object_get(Jobj, "ActiveModeEnabled"));
	AutoEventEnabled = json_object_get_boolean(json_object_object_get(Jobj, "AutoEventEnabled"));

}

zcfgRet_t zcfgFeDal8023ahCfg(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	IID_INIT(ieee8023ahIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8023AH, &ieee8023ahIid, &ieee8023ahobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.3ah Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(ieee8023ahobj != NULL){
		json_object_object_add(ieee8023ahobj, "Enabled", json_object_new_boolean(Enabled));
		json_object_object_add(ieee8023ahobj, "OAMID", json_object_new_int(OAMID));
		json_object_object_add(ieee8023ahobj, "Interface", json_object_new_string(Interface));
		json_object_object_add(ieee8023ahobj, "VariableRetrievalEnabled", json_object_new_boolean(VariableRetrievalEnabled));
		json_object_object_add(ieee8023ahobj, "LinkEventsEnabled", json_object_new_boolean(LinkEventsEnabled));
		json_object_object_add(ieee8023ahobj, "RemoteLoopbackEnabled", json_object_new_boolean(RemoteLoopbackEnabled));
		json_object_object_add(ieee8023ahobj, "ActiveModeEnabled", json_object_new_boolean(ActiveModeEnabled));
		json_object_object_add(ieee8023ahobj, "AutoEventEnabled", json_object_new_boolean(AutoEventEnabled));

		if((ret = zcfgFeObjJsonSet(RDM_OID_IEEE8023AH, &ieee8023ahIid, ieee8023ahobj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Set IEEE 802.3ah fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDal8023ahEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" input --- %s \n ",json_object_to_json_string(Jobj));
	init8023ahGlobalObjects();
	get8023ahBasicInfo(Jobj);

	if((ret = zcfgFeDal8023ahCfg(Jobj)) != ZCFG_SUCCESS){
		ZLOG_INFO("(ERROR) config 8023ah fail\n");
			goto exit;
	}

exit:
	freeAll8023ahObjects();
	return ret;
}

zcfgRet_t zcfgFeDal8023ahGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj, *interfaceList;
	char pathString[30] = "";
	objIndex_t objIid;

	/*
	 * [#50718][ECONET][WAN] 802.1ag/802.3ah fail on ADSL interface
	 * The ADSL interface name ATM.Link.X_ZYXEL_IfName in Econet platform
	 * is not a real interfece. Need be convert from atmx to nasx,
	 * which x is index. 2018-06-26 Steve Chan
	 */
#ifdef ECONET_PLATFORM
	uint8_t atmIntfIdx = 0;
	char *atm_Ifname_p = NULL;
	char atm_nasIfname[10];
#endif //ECONET_PLATFORM

	bool atmHas = true, ptmHas = true;
	atmHas = (flag1 & HIDE_DSL) ? !atmHas : atmHas;
	ptmHas = (flag1 & HIDE_DSL) ? !ptmHas : ptmHas;

	interfaceList = json_object_new_object();

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	IID_INIT(objIid);
	while(atmHas && ZCFG_SUCCESS == zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ATM_LINK, &objIid, &obj)){
		 sprintf(pathString, "ATM.Link.%d", objIid.idx[0]);
#ifdef ECONET_PLATFORM
		/*
		 * [#50718][ECONET][WAN] 802.1ag/802.3ah fail on ADSL interface
		 * The ADSL interface name ATM.Link.X_ZYXEL_IfName in Econet platform
		 * is not a real interfece. Need be convert from atmx to nasx,
		 * which x is index. 2018-06-26 Steve Chan
		 */
		atm_Ifname_p = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"));

		if(sscanf(atm_Ifname_p, "atm%hhu", &atmIntfIdx) != 1){
			printf("<<Error>> %s() Parsing index form ATM.Link.X_ZYXEL_IfName:%s failed.\n", __FUNCTION__, atm_Ifname_p);
		}
		snprintf(atm_nasIfname, sizeof(atm_nasIfname), "nas%hhu", atmIntfIdx);
		json_object_object_add(interfaceList, pathString, json_object_new_string(atm_nasIfname));
#else //#if ECONET_PLATFORM
		json_object_object_add(interfaceList, pathString, json_object_new_string(json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"))) );
#endif //#if ECONET_PLATFORM
		json_object_put(obj);
	}
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	IID_INIT(objIid);
	while(ptmHas && ZCFG_SUCCESS == zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PTM_LINK, &objIid, &obj)){
		sprintf(pathString, "PTM.Link.%d",  objIid.idx[0]);
		json_object_object_add(interfaceList, pathString, json_object_new_string(json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"))) );
		json_object_put(obj);
	}
#endif

	IID_INIT(objIid);
	while(ZCFG_SUCCESS == zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ETH_IFACE, &objIid, &obj)){
		if( true == json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_Upstream"))  ){
			sprintf(pathString, "Ethernet.Interface.%d", objIid.idx[0]);
			json_object_object_add(interfaceList, pathString, json_object_new_string(json_object_get_string(json_object_object_get(obj, "Name"))) );
		}

		json_object_put(obj);
	}

	IID_INIT(ieee8023ahIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8023AH, &ieee8023ahIid, &ieee8023ahobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.3ah Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}
	json_object_object_add(ieee8023ahobj, "interfaceList",JSON_OBJ_COPY(interfaceList));;
	json_object_array_add(Jarray, JSON_OBJ_COPY(ieee8023ahobj));

	if(interfaceList) json_object_put(interfaceList);

	freeAll8023ahObjects();
	return ret;
}

zcfgRet_t zcfgFe8023ah(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *guiCustomObj;
	objIndex_t iid;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	retrieveGUICustomizationFlags(&flag1, NULL, NULL, NULL);

	if(!flag1){
		IID_INIT(iid);
		if(zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &iid, &guiCustomObj) == ZCFG_SUCCESS){
			flag1 = json_object_get_int(json_object_object_get(guiCustomObj, "Flag1"));
			json_object_put(guiCustomObj);
		}
	}

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal8023ahEdit(Jobj, NULL);
	}
	if(!strcmp(method, "GET")) {
		ret = zcfgFeDal8023ahGet(Jobj, Jarray, NULL);
	}

	return ret;
}

