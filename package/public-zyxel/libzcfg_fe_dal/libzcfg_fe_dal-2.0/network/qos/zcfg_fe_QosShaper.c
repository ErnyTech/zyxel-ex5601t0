#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <string.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_Qos.h"

typedef struct s_QosShaper {
	int Index;
	bool Enable;
	char *Interface;
	int ShapingRate;
}s_QosShaper;

typedef struct s_QosShaperDel {
	int Index;
	int UpdateOrDel; //Update: 0, Del: 1
}s_QosShaperDel;

dal_param_t QosShaper_param[] = {
	{"Index",						dalType_int, 			0, 0, 		  NULL,		NULL,	dal_Edit|dal_Delete},
	{"Enable",						dalType_boolean,		0, 0, 		  NULL,		NULL,	dal_Add},
	{"Interface",					dalType_ShaperIntf    , 0, 0,         NULL,		NULL,	dal_Add},
	{"ShapingRate",					dalType_int, 			0, QOS_MAX_SHAPING_RATE,   NULL,  	NULL,	dal_Add},
	{NULL,							0,						0, 0,	      NULL,		NULL,	0},
};
extern uint32_t StrToIntConv(char *str);

void ShaperInfoInit(s_QosShaper *QosShaper_Info)
{
    QosShaper_Info->Index = 0;
    QosShaper_Info->Enable = 0;
    QosShaper_Info->Interface = NULL;
    QosShaper_Info->ShapingRate = 0;
}

void ShaperDelInfoInit(s_QosShaperDel *QosShaperDel_Info)
{
    QosShaperDel_Info->Index = 0;
    QosShaperDel_Info->UpdateOrDel = 0;
}


void getQosShaperBasicInfo(struct json_object *Jobj, s_QosShaper *QosShaper_Info) {
	char *rest_str = NULL;
	QosShaper_Info->Index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if(json_object_object_get(Jobj, "Enable")!= NULL)
		QosShaper_Info->Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(json_object_object_get(Jobj, "Interface")!= NULL)
		QosShaper_Info->Interface = strtok_r((char *)json_object_get_string(json_object_object_get(Jobj, "Interface")),",", &rest_str);
	if(json_object_object_get(Jobj, "ShapingRate")!= NULL)
		QosShaper_Info->ShapingRate = json_object_get_int(json_object_object_get(Jobj, "ShapingRate"));
	return;
}

void getQosShaperDelInfo(struct json_object *Jobj, s_QosShaperDel *QosShaperDel_Info) {
	if(json_object_object_get(Jobj, "Index")!= NULL)
		QosShaperDel_Info->Index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	else
		QosShaperDel_Info->Index = 0;
	QosShaperDel_Info->UpdateOrDel = json_object_get_int(json_object_object_get(Jobj, "UpdateOrDel"));
	return;
}

void prepareQosShaperObj(struct json_object *Jobj, s_QosShaper *QosShaper_Info, json_object *shaperObj) {	
	if(json_object_object_get(Jobj, "Enable")!= NULL){
		json_object_object_add(shaperObj, "Enable", json_object_new_boolean(QosShaper_Info->Enable));
	}
	if(json_object_object_get(Jobj, "Interface")!= NULL){
		json_object_object_add(shaperObj, "Interface", json_object_new_string(QosShaper_Info->Interface));
	}
	//Shaping rate range is between 0 and 1024000 
	if(json_object_object_get(Jobj, "ShapingRate")!= NULL){
		json_object_object_add(shaperObj, "ShapingRate", json_object_new_int(QosShaper_Info->ShapingRate));
	}
	return;
}

static json_object* getObjByPathName(char* pathName) {
	int pathNode = -1;
	char *pathNode_ptr = NULL;
	char *tmp = NULL;
	char *rest_str = NULL;
	char *pathName_cpy = NULL;
	json_object *obj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	json_object *atmLinkObj = NULL;
	objIndex_t atmLinkIid = {0};
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	json_object *ptmLinkObj = NULL;
	objIndex_t ptmLinkIid = {0};
#endif
	json_object *ethIfaceObj = NULL;
	json_object *bridgePortObj = NULL;
	json_object *ipIfaceObj = NULL;
	json_object *pppIfaceObj = NULL;
	objIndex_t ethIfaceIid = {0};
	objIndex_t bridgePortIid = {0};
	objIndex_t ipIfaceIid = {0};
	objIndex_t pppIfaceIid = {0};

	pathName_cpy = (char*) malloc(strlen(pathName)+1);
	memcpy(pathName_cpy, pathName, strlen(pathName)+1);
	// find the pathNode
	tmp = strtok_r(pathName_cpy, ".", &rest_str);
	while(tmp != NULL) {
		tmp = strtok_r(rest_str, ".", &rest_str);
		if(tmp != NULL) {
			pathNode_ptr = tmp;
		}
	}
	if(pathNode_ptr != NULL) {
		pathNode = StrToIntConv(pathNode_ptr);
#if(QOS_DAL_DEBUG == 1)			
		printf("getObjByPathName pathNode %d\n", pathNode);
#endif		
	}
	free(pathName_cpy);

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	if(strstr(pathName, "ATM") != NULL) {
		IID_INIT(atmLinkIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj) == ZCFG_SUCCESS) {
			if(atmLinkIid.idx[0] == pathNode) {
				obj = atmLinkObj;
				break;
			}
			zcfgFeJsonObjFree(atmLinkObj);
		}
	}
	else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	if(strstr(pathName, "PTM") != NULL) {
		IID_INIT(ptmLinkIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkObj) == ZCFG_SUCCESS) {
			if(ptmLinkIid.idx[0] == pathNode) {
				obj = ptmLinkObj;
				break;
			}
			zcfgFeJsonObjFree(ptmLinkObj);
		}
	}	
	else
#endif
	if(strstr(pathName, "Ethernet") != NULL) {
		IID_INIT(ethIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj) == ZCFG_SUCCESS) {
			if(ethIfaceIid.idx[0] == pathNode) {
				obj = ethIfaceObj;
				break;
			}
			zcfgFeJsonObjFree(ethIfaceObj);
		}
	}	
	else if(strstr(pathName, "Bridging") != NULL) {
		IID_INIT(bridgePortIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &bridgePortIid, &bridgePortObj) == ZCFG_SUCCESS) {
			if(bridgePortIid.idx[0] == pathNode) {
				obj = bridgePortObj;
				break;
			}
			zcfgFeJsonObjFree(bridgePortObj);
		}
	}	
	else if(strstr(pathName, "IP.Interface") != NULL) {
		IID_INIT(ipIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			if(ipIfaceIid.idx[0] == pathNode) {
				obj = ipIfaceObj;
				break;
			}
			zcfgFeJsonObjFree(ipIfaceObj);
		}
	}
	else if(strstr(pathName, "PPP.Interface") != NULL) {
		IID_INIT(pppIfaceIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceObj) == ZCFG_SUCCESS) {
			if(pppIfaceIid.idx[0] == pathNode) {
				obj = pppIfaceObj;
				break;
			}
			zcfgFeJsonObjFree(pppIfaceObj);
		}
	}
	return obj;
}

zcfgRet_t checkQosShaperInterface() {
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *Shaper_Interface = NULL;
	char *Shaper_Interface_cpy = NULL;
	json_object *obj = NULL;
	json_object *shaperObj = NULL;
	objIndex_t shaperIid = {0};
	IID_INIT(shaperIid);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_SHAPER, &shaperIid, &shaperObj) == ZCFG_SUCCESS) {
		Shaper_Interface = (char*)json_object_get_string(json_object_object_get(shaperObj, "Interface"));
		Shaper_Interface_cpy = (char*)malloc(strlen(Shaper_Interface)+1);
		memcpy(Shaper_Interface_cpy, Shaper_Interface, strlen(Shaper_Interface)+1);
		obj = getObjByPathName(Shaper_Interface_cpy);
		if(obj == NULL) {
			// if the interface doesn't exist, remove the corresponding shaper object
#if(QOS_DAL_DEBUG == 1)			
			printf("remove the interface: %s\n", Shaper_Interface); 
#endif			
			zcfgFeObjJsonDel(RDM_OID_QOS_SHAPER, &shaperIid, NULL);
		}
		else {
			zcfgFeJsonObjFree(obj);
		}
		free(Shaper_Interface_cpy);
		zcfgFeJsonObjFree(shaperObj);
	}
	return ret;
}

void zcfgFeDalShowQosShaper(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-7s %-12s %-20s %-12s\n",
		    "Index", "Enable", "Interface", "Rate Limit");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			printf("%-7s %-12d %-20s %-12s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_boolean(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "IfName")),
			json_object_get_string(json_object_object_get(obj, "ShapingRate")));
		}
	}
}

zcfgRet_t zcfgFeDalQosShaperGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *pramJobj = NULL;
    struct json_object *shaperObj = NULL;
    struct json_object *wanobj = NULL;
    struct json_object *lanObj = NULL;
    struct json_object *tmpObj = NULL;
    objIndex_t shaperIid = {0};
    objIndex_t wanIid = {0};
    objIndex_t lanIid;
    const char *Interface;
    const char *newInt;
    char ipIntfPath[32] = {0};
    int count = 1;
    int idx = -1;

    if ((Jobj != NULL) && ((tmpObj = json_object_object_get(Jobj, "idx")) != NULL))
    {
        idx = json_object_get_int(tmpObj);
    }

    IID_INIT(shaperIid);
    while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_SHAPER, &shaperIid, &shaperObj) == ZCFG_SUCCESS)
    {
        if (idx == -1 || shaperIid.idx[0] == idx)
        {
            pramJobj = json_object_new_object();
            IID_INIT(wanIid);
            wanIid.level = 1;
            if ((tmpObj = json_object_object_get(shaperObj, "Interface")) != NULL)
            {
                Interface = json_object_get_string(tmpObj);
                if(strstr(Interface, "PPP") != NULL)
                {
                    while ((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &wanIid, &wanobj)) == ZCFG_SUCCESS)
                    {
                        if ((tmpObj = json_object_object_get(wanobj, "LowerLayers")) != NULL)
                        {
                            if (!strcmp(Interface, json_object_get_string(tmpObj)))
                            {
                                snprintf(ipIntfPath, sizeof(ipIntfPath), "IP.Interface.%d", wanIid.idx[0]);
                                Interface = ipIntfPath;
                                break;
                            }
                        }
                        zcfgFeJsonObjFree(wanobj);
                    }
                }
                else if (strstr(Interface, "IP") != NULL)
                {
                    sscanf(Interface, "IP.Interface.%hhu", &wanIid.idx[0]);
                    if(wanIid.idx[0] > 0){
						ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &wanIid, &wanobj);
					}
                }
                else
                {
                    IID_INIT(lanIid);
                    lanIid.level = 1;
                    sscanf(Interface, "Ethernet.Interface.%hhu", &lanIid.idx[0]);
                    ret = zcfgFeObjJsonGet(RDM_OID_ETH_IFACE, &lanIid, &lanObj);
                }

                if (wanobj && ((tmpObj = json_object_object_get(wanobj, "X_ZYXEL_SrvName")) != NULL))
                {
                    #ifdef TAAAB_HGW
					char *tmp_string = NULL;
					char newName[128] = {0};
                    newInt = json_object_get_string(tmpObj);
					if(strcmp(newInt,"Default") == 0 || strcmp(newInt,"WWAN") == 0)
                    json_object_object_add(pramJobj, "IfName",json_object_new_string(newInt));
					else{
					tmp_string = strrchr(newInt,'_');
					strncpy(newName, newInt, strlen(newInt)-strlen(tmp_string));	
					newName[strlen(newInt)-strlen(tmp_string)] = '\0';
					json_object_object_add(pramJobj, "IfName",json_object_new_string(newName));
					}
					#else
                    newInt = json_object_get_string(tmpObj);
                    json_object_object_add(pramJobj, "IfName",json_object_new_string(newInt));
					#endif
                    zcfgFeJsonObjFree(wanobj);
                }
                else if (lanObj && ((tmpObj = json_object_object_get(lanObj, "X_ZYXEL_LanPort")) != NULL))
                {
                    newInt = json_object_get_string(tmpObj);
                    json_object_object_add(pramJobj, "IfName",json_object_new_string(newInt));
                    zcfgFeJsonObjFree(lanObj);
                }

                json_object_object_add(pramJobj, "Interface",json_object_new_string(Interface));
                json_object_object_add(pramJobj, "Index", json_object_new_int(count));
                json_object_object_add(pramJobj, "idx", json_object_new_int(shaperIid.idx[0]));
                json_object_object_add(pramJobj, "ShapingRate", JSON_OBJ_COPY(json_object_object_get(shaperObj, "ShapingRate")));
                json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(shaperObj, "Enable")));
                json_object_array_add(Jarray, pramJobj);
            }
        }
        count++;
        zcfgFeJsonObjFree(shaperObj);
    }
    return ret;
}


zcfgRet_t zcfgFeDalQosShaperAdd(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	s_QosShaper QosShaper_Info;
	json_object *shaperObj = NULL;
	objIndex_t shaperIid = {0};
	IID_INIT(shaperIid);

	ShaperInfoInit(&QosShaper_Info);
	getQosShaperBasicInfo(Jobj, &QosShaper_Info);

	zcfg_offset_t oid;
	oid = RDM_OID_QOS_SHAPER;
	if(QosShaper_Info.Interface != NULL){
		ret = dalcmdParamCheck(NULL, QosShaper_Info.Interface , oid, "Interface", "Interface",  replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}else{
		strcat(replyMsg, " Interface is NULL.");
		return ZCFG_INVALID_PARAM_VALUE;
	}

	ret = zcfgFeObjJsonAdd(RDM_OID_QOS_SHAPER, &shaperIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		ret = zcfgFeObjJsonGet(RDM_OID_QOS_SHAPER, &shaperIid, &shaperObj);
	}

	if(ret == ZCFG_SUCCESS) {
		prepareQosShaperObj(Jobj,&QosShaper_Info, shaperObj);
		zcfgFeObjJsonSet(RDM_OID_QOS_SHAPER, &shaperIid, shaperObj, NULL);
		zcfgFeJsonObjFree(shaperObj);
	}
	
	else{
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}

	return ret;
}

zcfgRet_t zcfgFeDalQosShaperEdit(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	s_QosShaper QosShaper_Info;
	json_object *shaperObj = NULL;
	objIndex_t shaperIid = {0};
	IID_INIT(shaperIid);

	ShaperInfoInit(&QosShaper_Info);
	getQosShaperBasicInfo(Jobj, &QosShaper_Info);
	zcfg_offset_t oid;
	oid = RDM_OID_QOS_SHAPER;
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		shaperIid.level = 1;
		shaperIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else
	{
	ret = convertIndextoIid(QosShaper_Info.Index, &shaperIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}

	if(QosShaper_Info.Interface != NULL){
		ret = dalcmdParamCheck(&shaperIid, QosShaper_Info.Interface , oid, "Interface", "Interface",  replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}

	// PUT RDM_OID_QOS_SHAPER
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_SHAPER, &shaperIid, &shaperObj) == ZCFG_SUCCESS) {
		prepareQosShaperObj(Jobj,&QosShaper_Info, shaperObj);
		zcfgFeObjJsonSet(RDM_OID_QOS_SHAPER, &shaperIid, shaperObj, NULL);
		zcfgFeJsonObjFree(shaperObj);
		return ret;
	}
	else{
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Shaper fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_SHAPER);
		return ZCFG_INTERNAL_ERROR;
	}
}

zcfgRet_t zcfgFeDalQosShaperDelete(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	s_QosShaperDel QosShaperDel_Info;
	json_object *shaperObj = NULL;
	objIndex_t shaperIid = {0};
	IID_INIT(shaperIid);

	ShaperDelInfoInit(&QosShaperDel_Info);
	getQosShaperDelInfo(Jobj, &QosShaperDel_Info);
	zcfg_offset_t oid;
	oid = RDM_OID_QOS_SHAPER;
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		shaperIid.level = 1;
		shaperIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else
	{
	ret = convertIndextoIid(QosShaperDel_Info.Index, &shaperIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	}
	if(((Jobj != NULL) && json_object_object_get(Jobj, "idx") == NULL) && (QosShaperDel_Info.UpdateOrDel == 0 && QosShaperDel_Info.Index == 0)) {
		// update Shaper
		checkQosShaperInterface();
	}
	else {
		// delete case;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_SHAPER, &shaperIid, &shaperObj) == ZCFG_SUCCESS) {
				zcfgFeObjJsonDel(RDM_OID_QOS_SHAPER, &shaperIid, NULL);
				zcfgFeJsonObjFree(shaperObj);
		}

		else{
			printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Shaper fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_SHAPER);
			return ZCFG_INTERNAL_ERROR;
		}
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalQosShaper(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalQosShaperAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalQosShaperEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalQosShaperDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalQosShaperGet(Jobj, Jarray, NULL);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}
