
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

#define TYPE_IEEE8021AG 0
#define TYPE_LOOPBACK 1
#define TYPE_LINKTRACE 2

dal_param_t IEEE_8021AG_param[]={
	{"EthOAM_TMS",							dalType_boolean,0,	0,	NULL},
	{"type",								dalType_int,	0,	0,	NULL},
	{"Enabled",								dalType_boolean,0,	0,	NULL},
	{"MDLevel",								dalType_int,	0,	0,	NULL},
	{"Interface",							dalType_string,	0,	0,	NULL},
	{"MAID",								dalType_string,	0,	0,	NULL},
	{"MDName",								dalType_string,	0,	0,	NULL},
	{"LMPID",								dalType_int,	0,	0,	NULL},
	{"RMPID",								dalType_int,	0,	0,	NULL},
	{"CCMEnabled",							dalType_boolean,0,	0,	NULL},
	{"Y1731Mode",							dalType_boolean,0,	0,	NULL},
	{"VLANID",								dalType_int,	0,	0,	NULL},
	{"DestMacAddress",						dalType_string,	0,	0,	NULL},
	{"SendLoopBack",						dalType_boolean,0,	0,	NULL},
	{"SendLinkTrace",						dalType_boolean,0,	0,	NULL},
	{NULL,									0,				0,	0,	NULL},
};


struct json_object *ieee8021agobj;

objIndex_t ieee8021agIid;

uint32_t flag1 = 0;

bool EthOAM_TMS;
unsigned int type;
bool Enabled;
unsigned int MDLevel;
char *Interface;
char *MAID;
char *MDName;
unsigned int LMPID;
unsigned int RMPID;
bool CCMEnabled;
bool Y1731Mode;
unsigned int VLANID;
char *DestMacAddress;
bool SendLoopBack;
bool SendLinkTrace;

void freeAll8021agObjects(){
	if(ieee8021agobj) json_object_put(ieee8021agobj);

	return;
}

void init8021agGlobalObjects(){
	ieee8021agobj = NULL;

	return;
}

void get8021agBasicInfo(struct json_object *Jobj){
	EthOAM_TMS = json_object_get_boolean(json_object_object_get(Jobj, "EthOAM_TMS"));
	MDLevel = json_object_get_int(json_object_object_get(Jobj, "MDLevel"));
	if(EthOAM_TMS == true){
		type = json_object_get_int(json_object_object_get(Jobj, "type"));
		Enabled = json_object_get_boolean(json_object_object_get(Jobj, "Enabled"));
		Y1731Mode = json_object_get_boolean(json_object_object_get(Jobj, "Y1731Mode"));
		if(Y1731Mode == false){
			MAID = (char *)json_object_get_string(json_object_object_get(Jobj, "MAID"));
		}
		MDName = (char *)json_object_get_string(json_object_object_get(Jobj, "MDName"));
		Interface = (char *)json_object_get_string(json_object_object_get(Jobj, "Interface"));
		LMPID = json_object_get_int(json_object_object_get(Jobj, "LMPID"));
		RMPID = json_object_get_int(json_object_object_get(Jobj, "RMPID"));
		CCMEnabled = json_object_get_boolean(json_object_object_get(Jobj, "CCMEnabled"));
		VLANID = json_object_get_int(json_object_object_get(Jobj, "VLANID"));
	}

	return;
}

void getLoopBackBasicInfo(struct json_object *Jobj){
	MDLevel = json_object_get_int(json_object_object_get(Jobj, "MDLevel"));
	DestMacAddress = (char *)json_object_get_string(json_object_object_get(Jobj, "DestMacAddress"));
	VLANID = json_object_get_int(json_object_object_get(Jobj, "VLANID"));
	SendLoopBack = json_object_get_boolean(json_object_object_get(Jobj, "SendLoopBack"));

	return;
}

void getLinkTraceBasicInfo(struct json_object *Jobj){
	MDLevel = json_object_get_int(json_object_object_get(Jobj, "MDLevel"));
	DestMacAddress = (char *)json_object_get_string(json_object_object_get(Jobj, "DestMacAddress"));
	VLANID = json_object_get_int(json_object_object_get(Jobj, "VLANID"));
	SendLinkTrace = json_object_get_boolean(json_object_object_get(Jobj, "SendLinkTrace"));

	return;
}

void getCfgTypeInfo(struct json_object *Jobj){
	type = json_object_get_int(json_object_object_get(Jobj, "type"));

	return;
}

zcfgRet_t zcfgFeDal8021agCfg(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" Enter..\n ");
	get8021agBasicInfo(Jobj);

	IID_INIT(ieee8021agIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8021AG, &ieee8021agIid, &ieee8021agobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.1ag Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(ieee8021agobj != NULL){
		if(EthOAM_TMS == true){
			if(Enabled == true){
				json_object_object_add(ieee8021agobj, "Enabled", json_object_new_boolean(Enabled));
				json_object_object_add(ieee8021agobj, "Y1731Mode", json_object_new_boolean(Y1731Mode));
				json_object_object_add(ieee8021agobj, "MDLevel", json_object_new_int(MDLevel));
				json_object_object_add(ieee8021agobj, "Interface", json_object_new_string(Interface));
				json_object_object_add(ieee8021agobj, "MDName", json_object_new_string(MDName));
				if(Y1731Mode == false){
					json_object_object_add(ieee8021agobj, "MAID", json_object_new_string(MAID));
				}
				json_object_object_add(ieee8021agobj, "LMPID", json_object_new_int(LMPID));
				json_object_object_add(ieee8021agobj, "RMPID", json_object_new_int(RMPID));
				json_object_object_add(ieee8021agobj, "CCMEnabled", json_object_new_boolean(CCMEnabled));
				json_object_object_add(ieee8021agobj, "VLANID", json_object_new_int(VLANID));
			}else{
				json_object_object_add(ieee8021agobj, "Enabled", json_object_new_boolean(Enabled));
			}
		}else{
			json_object_object_add(ieee8021agobj, "MDLevel", json_object_new_int(MDLevel));
		}

		if((ret = zcfgFeObjJsonSet(RDM_OID_IEEE8021AG, &ieee8021agIid, ieee8021agobj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Set IEEE 802.1ag fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalLoopBack(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" Enter..\n ");
	getLoopBackBasicInfo(Jobj);

	IID_INIT(ieee8021agIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8021AG, &ieee8021agIid, &ieee8021agobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.1ag Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(ieee8021agobj != NULL){
		json_object_object_add(ieee8021agobj, "MDLevel", json_object_new_int(MDLevel));
		json_object_object_add(ieee8021agobj, "DestMacAddress", json_object_new_string(DestMacAddress));
		json_object_object_add(ieee8021agobj, "VLANID", json_object_new_int(VLANID));
		json_object_object_add(ieee8021agobj, "SendLoopBack", json_object_new_boolean(SendLoopBack));

		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_IEEE8021AG, &ieee8021agIid, ieee8021agobj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Set IEEE 802.1ag fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDalLinktrace(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" Enter..\n ");
	getLinkTraceBasicInfo(Jobj);

	IID_INIT(ieee8021agIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8021AG, &ieee8021agIid, &ieee8021agobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.1ag Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(ieee8021agobj != NULL){
		json_object_object_add(ieee8021agobj, "MDLevel", json_object_new_int(MDLevel));
		json_object_object_add(ieee8021agobj, "DestMacAddress", json_object_new_string(DestMacAddress));
		json_object_object_add(ieee8021agobj, "VLANID", json_object_new_int(VLANID));
		json_object_object_add(ieee8021agobj, "SendLinkTrace", json_object_new_boolean(SendLinkTrace));

		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_IEEE8021AG, &ieee8021agIid, ieee8021agobj, NULL)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) Set IEEE 802.1ag fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ret;
}

zcfgRet_t zcfgFeDal8021agEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	ZLOG_INFO(" input --- %s \n ",json_object_to_json_string(Jobj));
	init8021agGlobalObjects();
	getCfgTypeInfo(Jobj);
	
	if(Y1731Mode){
		if(sizeof(MDName)>13){
			return ZCFG_REQUEST_REJECT;
		}
	}
	
	if(type == TYPE_IEEE8021AG){
		if((ret = zcfgFeDal8021agCfg(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO(" config 8021ag fail\n");
			goto exit;
		}
	}else if(type == TYPE_LOOPBACK){
		if((ret = zcfgFeDalLoopBack(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("%(ERROR) config LoopBack fail\n");
			goto exit;
		}
	}else if(type == TYPE_LINKTRACE){
		if((ret = zcfgFeDalLinktrace(Jobj)) != ZCFG_SUCCESS){
			ZLOG_INFO("(ERROR) config Linktrace fail\n");
			goto exit;
		}
	}

exit:
	freeAll8021agObjects();
	return ret;
}

zcfgRet_t zcfgFeDal8021agGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
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
		atm_Ifname_p = (char *)json_object_get_string(json_object_object_get(obj, "X_ZYXEL_IfName"));
		
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

	IID_INIT(ieee8021agIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IEEE8021AG, &ieee8021agIid, &ieee8021agobj)) != ZCFG_SUCCESS) {
		ZLOG_INFO("(ERROR) Get IEEE 802.1ag Obj fail\n");
		return ZCFG_INTERNAL_ERROR;
	}
	json_object_object_add(ieee8021agobj, "interfaceList",JSON_OBJ_COPY(interfaceList));;
	json_object_array_add(Jarray, JSON_OBJ_COPY(ieee8021agobj));

	if(interfaceList) json_object_put(interfaceList);

	freeAll8021agObjects();
	return ret;
}

zcfgRet_t zcfgFe8021ag(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
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
		ret = zcfgFeDal8021agEdit(Jobj, NULL);
	}
	if(!strcmp(method, "GET")) {
		ret = zcfgFeDal8021agGet(Jobj, Jarray, NULL);;
	}

	return ret;
}

