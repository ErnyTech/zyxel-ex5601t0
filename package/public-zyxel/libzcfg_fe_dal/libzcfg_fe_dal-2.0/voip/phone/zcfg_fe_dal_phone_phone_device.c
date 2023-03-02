
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
#include "zos_api.h"

#define ISFXSBINDING	0
#define ISDECTBINDING	1

dal_param_t VOIP_PHONEDEVICE_param[]={
	{"Index",									dalType_int,	0,	0,	NULL},
	{"LineSelect",								dalType_VoipLine,	0,	0,	NULL},
	{"DectLineSelect",							dalType_string,	0,	0,	NULL,	NULL,	dalcmd_Forbid},	
	{"phoneBoundingList",						dalType_MultiVoipLine,	0,	0,	NULL},
	{"DialEnable",								dalType_boolean,0,	0,	NULL},
	{NULL,										0,				0,	0,	NULL},
};

unsigned int flag1;
bool phonePortBinding;
int phyIntfNum;
int dectntfNum;

const char* X_ZYXEL_LineSelect;
unsigned int lineSelectBitMap;
bool X_ZYXEL_Pound;
unsigned int X_ZYXEL_PhyReferenceFXSList;
const char* LineSelect;
char* phoneBoundingList;


struct json_object *phyIntfObj;
struct json_object *multiJobj;
struct json_object *guiCustObj;
struct json_object *accountObj;
struct json_object *dectObj;
struct json_object *singlePhyobj;

objIndex_t phyIid = {0};
objIndex_t dectIid = {0};

void zcfgFeDalShowVoipPhoneDevice(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-5s %-10s %-20s %-35s %-10s \n","Index","PHONE ID","Internal Number","Incoming SIP Number","Outgoing SIP Number");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-5s %-10s %-20s %-35s %-10s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "PhoneID")),
			json_object_get_string(json_object_object_get(obj, "InterNum")),
			json_object_get_string(json_object_object_get(obj, "IncomingSIP")),
			json_object_get_string(json_object_object_get(obj, "LineSelect")));
	}
}

void initPhyInterfaceGlobalObjects(){
	phyIntfObj = NULL;
	multiJobj = NULL;
	guiCustObj = NULL;
	accountObj = NULL;
	dectObj = NULL;
	singlePhyobj = NULL;

	return;
}

void freeAllPhyIftObjects(){
	if(phyIntfObj) json_object_put(phyIntfObj);
	if(multiJobj) json_object_put(multiJobj);
	if(guiCustObj) json_object_put(guiCustObj);
	if(accountObj) json_object_put(accountObj);
	if(dectObj) json_object_put(dectObj);
	if(singlePhyobj) json_object_put(singlePhyobj);

	return;
}

void getPhyIntfBasicInfo(struct json_object *Jobj){
		if(!(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST & flag1)){
			X_ZYXEL_LineSelect = json_object_get_string(json_object_object_get(Jobj, "LineSelect"));
			phoneBoundingList = (char *)json_object_get_string(json_object_object_get(Jobj, "phoneBoundingList"));
			X_ZYXEL_Pound = json_object_get_boolean(json_object_object_get(Jobj, "DialEnable"));
		}
		if(!(HIDE_DECT & flag1)){
			LineSelect = json_object_get_string(json_object_object_get(Jobj, "DectLineSelect"));
		}

		return;
}

void zcfgFeDalPhyCustomizationflagGet(){

	IID_INIT(phyIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &phyIid, &guiCustObj) == ZCFG_SUCCESS){
		flag1 = json_object_get_int(json_object_object_get(guiCustObj, "Flag1"));
		zcfgFeJsonObjFree(guiCustObj);
		return;
	}
	zcfgFeJsonObjFree(guiCustObj);
	return;
}

void zcfgFeDalPhyIntfObjEditIidGet(struct json_object *Jobj){
	//const char* path = NULL;
	IID_INIT(phyIid);

	phyIntfNum = json_object_get_int(json_object_object_get(Jobj, "Index"));
	//if(path == NULL){
	//	return;
	//}
	//printf("%s(): Edit Phyitf path : %s \n ",__FUNCTION__,path);

	phonePortBinding = ISFXSBINDING;

	//sscanf(path,"Device.Services.VoiceService.1.PhyInterface.%d",&phyIntfNum);
	//printf(" phyIntfNum: %d \n ",phyIntfNum);

	phyIid.level = 2;
	phyIid.idx[0] = 1;
	phyIid.idx[1] = phyIntfNum;

	return;
}

void zcfgFeDalDectObjEditIidGet(struct json_object *Jobj){
	const char* path = NULL;
	IID_INIT(dectIid);

	path = json_object_get_string(json_object_object_get(Jobj, "DectItfpath"));
	if(path == NULL){
		return;
	}
	//printf("%s(): Edit Dect path : %s \n ",__FUNCTION__,path);

	phonePortBinding = ISDECTBINDING;

	sscanf(path,"Device.Services.VoiceService.1.X_ZYXEL_DECT.Handset.%d",&dectntfNum);
	//printf(" dectntfNum: %d \n ",dectntfNum);

	dectIid.level = 2;
	dectIid.idx[0] = 1;
	dectIid.idx[1] = dectntfNum;

	return;
}

zcfgRet_t zcfgFeDalPhyInterfaceObjEdit(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PHY_INTF, &phyIid, &phyIntfObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get phyitf obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(phyIntfObj != NULL){
		if(json_object_object_get(Jobj, "LineSelect"))
		json_object_object_add(phyIntfObj, "X_ZYXEL_LineSelect", json_object_new_string(X_ZYXEL_LineSelect));
		if(json_object_object_get(Jobj, "DialEnable"))
		json_object_object_add(phyIntfObj, "X_ZYXEL_Pound", json_object_new_boolean(X_ZYXEL_Pound));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PHY_INTF, &phyIid, multiJobj, phyIntfObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PHY_INTF);
			zcfgFeJsonObjFree(phyIntfObj);
			return ZCFG_INTERNAL_ERROR;
		}

	}

	//printf("%s(): Append phyitf obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalDectObjEdit(struct json_object *Jobj){
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PHONE_DECT
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_DECT_H_S, &dectIid, &dectObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get dect_H_S Obj fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(dectObj != NULL){
		json_object_object_add(dectObj, "LineSelect", json_object_new_string(LineSelect));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_DECT_H_S, &dectIid, multiJobj, dectObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_DECT_H_S);
			zcfgFeJsonObjFree(dectObj);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	//printf("%s(): Append dect_H_S obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
#else
	printf("%s: PHONE_DECT not support\n", __FUNCTION__);
	return ZCFG_INTERNAL_ERROR;
#endif
}

zcfgRet_t zcfgFeDalPortBindingEdit(struct json_object *Jobj, bool BindingType){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);
	objIndex_t lineIid;
	char *linePath = NULL;
	char boundList[256];
	int profileNum, lineNum;
	int phoneBindingBase;
	int portNum;

	if(BindingType == ISFXSBINDING){
		phoneBindingBase = 0;
		portNum = phyIntfNum;
	}else if(BindingType == ISDECTBINDING){
		phoneBindingBase = 16;
		portNum = dectntfNum;
	}

	IID_INIT(lineIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE, &lineIid, &accountObj) == ZCFG_SUCCESS) {
		X_ZYXEL_PhyReferenceFXSList = json_object_get_int(json_object_object_get(accountObj, "X_ZYXEL_PhyReferenceFXSList"));
		X_ZYXEL_PhyReferenceFXSList &= ~(1 << (portNum+phoneBindingBase-1));
		if(json_object_object_get(Jobj, "phoneBoundingList")){
		if(phoneBoundingList != NULL){
			ZOS_STRNCPY(boundList, phoneBoundingList, sizeof(boundList));
			linePath = strtok(boundList,",");
			while (linePath != NULL){
				sscanf(linePath,"VoiceProfile.%d.Line.%d", &profileNum, &lineNum);

				if((profileNum == lineIid.idx[1])&&(lineNum == lineIid.idx[2])){
					X_ZYXEL_PhyReferenceFXSList |= 1 << (portNum+phoneBindingBase-1);
					break;
				}

				linePath = strtok (NULL, ",");
			}
		}
		json_object_object_add(accountObj, "X_ZYXEL_PhyReferenceFXSList", json_object_new_int(X_ZYXEL_PhyReferenceFXSList));
		}
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE, &lineIid, multiJobj, accountObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE);
			zcfgFeJsonObjFree(accountObj);
			return ZCFG_INTERNAL_ERROR;
		}

		zcfgFeJsonObjFree(accountObj);
	}

	//printf("%s(): Append line obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipPhoneDeviceEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): input --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

	initPhyInterfaceGlobalObjects();
	getPhyIntfBasicInfo(Jobj);
	zcfgFeDalPhyIntfObjEditIidGet(Jobj);
	zcfgFeDalDectObjEditIidGet(Jobj);
	multiJobj = zcfgFeJsonObjNew();

	if(phonePortBinding == ISFXSBINDING){
		//printf("phonePortBinding == ISFXSBINDING\n");
		if((ret = zcfgFeDalPhyInterfaceObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit phyinf obj \n", __FUNCTION__);
			goto exit;
		}

		if((ret = zcfgFeDalPortBindingEdit(Jobj, phonePortBinding)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit port binding \n", __FUNCTION__);
			goto exit;
		}
	}

	if((phonePortBinding == ISDECTBINDING) && !(HIDE_DECT & flag1)){
		printf("phonePortBinding == ISDECTBINDING\n");
		if((ret = zcfgFeDalDectObjEdit(Jobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit line obj \n", __FUNCTION__);
			goto exit;
		}
		if((ret = zcfgFeDalPortBindingEdit(Jobj, phonePortBinding)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to edit port binding \n", __FUNCTION__);
			goto exit;
		}
	}

	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_VOICE_PROF);
		goto exit;
	}

exit:
	freeAllPhyIftObjects();
	return ret;
}

zcfgRet_t zcfgFeDalPhyItfObjGet(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	X_ZYXEL_LineSelect = json_object_get_string(json_object_object_get(phyIntfObj, "X_ZYXEL_LineSelect"));
	json_object_object_add(paserobj, "X_ZYXEL_LineSelect", json_object_new_string(X_ZYXEL_LineSelect));

	return ret;

}

zcfgRet_t zcfgFeDalPhyItfGet(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	IID_INIT(phyIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PHY_INTF, &phyIid, &phyIntfObj) == ZCFG_SUCCESS){
		singlePhyobj = json_object_new_object();

		if((ret = zcfgFeDalPhyItfObjGet(singlePhyobj)) != ZCFG_SUCCESS){
			printf("%s:(ERROR) fail to parse Phy ift Obj \n", __FUNCTION__);
			goto exit;
		}

		if( ret == ZCFG_SUCCESS ){
			json_object_array_add(Jobj, JSON_OBJ_COPY(singlePhyobj));
		}

		zcfgFeJsonObjFree(singlePhyobj);
		zcfgFeJsonObjFree(phyIntfObj);
	}

	//printf("%s(): output --- %s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	exit:
	freeAllPhyIftObjects();
	return ret;
}

zcfgRet_t zcfgFeDalVoipPhoneDeviceGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *InSIPJobj = NULL;
	struct json_object *voipdeviceObj = NULL;
	struct json_object *voipvoicelineObj = NULL;
	objIndex_t voipdeviceIid = {0};
	objIndex_t voipvoicelineIid = {0};
	InSIPJobj = json_object_new_object();
	int count = 1, PhyReferenceFXSList = 0;
	char internum[8] = {0}, phoneId[8] = {0};
    char incomingSIP1[512] = {0}, incomingSIP2[512] = {0}, incomingSIP[16] = {0};
    char incomingSIP1List[512] = {0}, incomingSIP2List[512] = {0}, incomingSIPList[20] = {0};
    char LinePath[64] = {0}, LineList[512] = {0};
    char NumberList[128] = {0};
    const char *directoryNumber = NULL;

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE, &voipvoicelineIid, &voipvoicelineObj) == ZCFG_SUCCESS){
		PhyReferenceFXSList = json_object_get_int(json_object_object_get(voipvoicelineObj, "X_ZYXEL_PhyReferenceFXSList"));
		directoryNumber = json_object_get_string(json_object_object_get(voipvoicelineObj, "DirectoryNumber"));

        if (NumberList[0] != '\0')
        {
            strcat(NumberList, ",");
        }
        strcat(NumberList, directoryNumber);

        snprintf(LinePath, sizeof(LinePath), "Device.Services.VoiceService.1.VoiceProfile.%d.Line.%d", voipvoicelineIid.idx[1], voipvoicelineIid.idx[2]);

        if (LineList[0] != '\0')
        {
            strcat(LineList, ",");
        }
        strcat(LineList, LinePath);

        if (PhyReferenceFXSList == 1)
        {
            if (incomingSIP1[0] != '\0')
            {
                strcat(incomingSIP1,",");
            }
            strcat(incomingSIP1,directoryNumber);

            if (incomingSIP1List[0] != '\0')
            {
                strcat(incomingSIP1List,",");
            }
            strcat(incomingSIP1List, LinePath);
        }
        else if (PhyReferenceFXSList == 2)
        {
            if (incomingSIP2[0] != '\0')
            {
                strcat(incomingSIP2,",");
            }
            strcat(incomingSIP2,directoryNumber);

            if (incomingSIP2List[0] != '\0')
            {
                strcat(incomingSIP2List,",");
            }
            strcat(incomingSIP2List, LinePath);
        }
        else if (PhyReferenceFXSList == 3)
        {
            if (incomingSIP1[0] != '\0')
            {
                strcat(incomingSIP1,",");
            }
            strcat(incomingSIP1,directoryNumber);

            if (incomingSIP1List[0] != '\0')
            {
                strcat(incomingSIP1List,",");
            }
            strcat(incomingSIP1List,LinePath);

            if (incomingSIP2[0] != '\0')
            {
                strcat(incomingSIP2,",");
            }
            strcat(incomingSIP2,directoryNumber);

            if (incomingSIP2List[0] != '\0')
            {
                strcat(incomingSIP2List,",");
            }
            strcat(incomingSIP2List, LinePath);
        }
        zcfgFeJsonObjFree(voipvoicelineObj);
    }

	json_object_object_add(InSIPJobj, "incomingSIP1", json_object_new_string(incomingSIP1));
	json_object_object_add(InSIPJobj, "incomingSIP2", json_object_new_string(incomingSIP2));
    json_object_object_add(InSIPJobj, "incomingSIP1List", json_object_new_string(incomingSIP1List));
    json_object_object_add(InSIPJobj, "incomingSIP2List", json_object_new_string(incomingSIP2List));
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PHY_INTF, &voipdeviceIid, &voipdeviceObj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		sprintf(phoneId,"PHONE%d", count);
		json_object_object_add(paramJobj, "PhoneID", json_object_new_string(phoneId));
		sprintf(internum,"**%d%d", voipdeviceIid.idx[0], voipdeviceIid.idx[1]);
		json_object_object_add(paramJobj, "InterNum", json_object_new_string(internum));
        sprintf(incomingSIP, "incomingSIP%d", count);
		json_object_object_add(paramJobj, "IncomingSIP", JSON_OBJ_COPY(json_object_object_get(InSIPJobj, incomingSIP)));
        sprintf(incomingSIPList, "incomingSIP%dList", count);
		json_object_object_add(paramJobj, "phoneBoundingList", JSON_OBJ_COPY(json_object_object_get(InSIPJobj, incomingSIPList)));
		json_object_object_add(paramJobj, "LineSelect", JSON_OBJ_COPY(json_object_object_get(voipdeviceObj, "X_ZYXEL_LineSelect")));
        json_object_object_add(paramJobj, "DialEnable", JSON_OBJ_COPY(json_object_object_get(voipdeviceObj, "X_ZYXEL_Pound")));
        json_object_object_add(paramJobj, "NumberList", json_object_new_string(NumberList));
        json_object_object_add(paramJobj, "LineList", json_object_new_string(LineList));

        count++;
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(voipdeviceObj);
	}
	zcfgFeJsonObjFree(InSIPJobj);
	return ret;
}


zcfgRet_t zcfgFeDalVoipPhoneDevice(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	zcfgFeDalPhyCustomizationflagGet();

	if(!strcmp(method, "PUT")) {
		if(!(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST & flag1)){
			ret = zcfgFeDalVoipPhoneDeviceEdit(Jobj, NULL);
		}
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalVoipPhoneDeviceGet(Jobj, Jarray, NULL);
	}
	/*else if(!strcmp(method, "GET")){
		ret = zcfgFeDalPhyItfGet(Jobj, NULL);
	}*/

	return ret;
}

