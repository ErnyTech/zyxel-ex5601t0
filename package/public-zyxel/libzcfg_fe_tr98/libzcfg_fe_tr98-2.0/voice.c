#include <json/json.h>

#ifdef CONFIG_PACKAGE_ZyIMS

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"

extern tr98Object_t tr98Obj[];


zcfgRet_t getVoiceConfig(zcfg_offset_t oid, objIndex_t *objIid, int handler, struct json_object **tr98Jobj){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *getObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef USE_OLD_VOIP_PREFIX
	char tempName[64] = {0};
#endif

	if((ret = zcfgFeObjJsonGet(oid, objIid, &getObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get obj(oid %d) codec object ret=%d\n", __FUNCTION__, oid, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
#ifdef USE_OLD_VOIP_PREFIX
		if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			paramValue = json_object_object_get(getObj, tempName);
		}else if(!strncmp(paramList->name, "X_ZYXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			paramValue = json_object_object_get(getObj, tempName);
		}else if(!strncmp(paramList->name, "X_ZyXEL_", 8)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 8);
			paramValue = json_object_object_get(getObj, tempName);
		}
		else
#endif
		paramValue = json_object_object_get(getObj, paramList->name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}

		paramList++;
		continue;
	}

	zcfgFeJsonObjFree(getObj);
	return ZCFG_SUCCESS;

}

zcfgRet_t setVoiceConfig(zcfg_offset_t oid, objIndex_t *objIid, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
#ifdef USE_OLD_VOIP_PREFIX
	char tempName[64] = {0};
#endif

	if((ret = zcfgFeObjJsonGet(oid, objIid, &obj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get object(oid %d) with ret=%d\n", __FUNCTION__, oid, ret);
		return ret;
	}
	
	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = obj;
		obj = NULL;
		obj = zcfgFeJsonMultiObjAppend(oid, objIid, multiJobj, tmpObj);
	}

	/* set(update) value to target object */
	if(obj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
#ifdef USE_OLD_VOIP_PREFIX
				if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12)){
					sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
					json_object_object_add(obj, tempName, JSON_OBJ_COPY(paramValue));
				}else if(!strncmp(paramList->name, "X_ZYXEL_COM_", 12)){
					sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
					json_object_object_add(obj, tempName, JSON_OBJ_COPY(paramValue));
				}else if(!strncmp(paramList->name, "X_ZyXEL_", 8)){
					sprintf(tempName, "X_ZYXEL_%s", paramList->name + 8);
					json_object_object_add(obj, tempName, JSON_OBJ_COPY(paramValue));
				}
				else
#endif				
				json_object_object_add(obj, paramList->name, JSON_OBJ_COPY(paramValue));
			}

			paramList++;
			continue;
		}

		/* is not multiple set, jsut set object immediately after update parameter value */
		if(!multiJobj){
			if((ret = zcfgFeObjJsonSet(oid, objIid, obj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set object(oid %d) Fail with ret=%d\n", __FUNCTION__, oid, ret);
			}
			zcfgFeJsonObjFree(obj);
			return ret;
		}
		zcfgFeJsonObjFree(tmpObj);
	
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t voiceNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;
#ifdef USE_OLD_VOIP_PREFIX
	char tempName[64] = {0};
#endif

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();	
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef USE_OLD_VOIP_PREFIX
		if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			if(!strcmp(tr181ParamName, tempName)){
				found = true;
				sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
				break;
			}
		}else if(!strncmp(paramList->name, "X_ZYXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			if(!strcmp(tr181ParamName, tempName)){
				found = true;
				sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
				break;
			}
		}else if(!strncmp(paramList->name, "X_ZyXEL_", 8)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 8);
			if(!strcmp(tr181ParamName, tempName)){
				found = true;
				sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
				break;
			}
		}
		else
#endif
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}

		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr181FindingIpIfaceByLowerLayer(char *lowerLayer, char *result)
{
	objIndex_t objIid;
	rdm_IpIface_t *iface = NULL;

	if(lowerLayer == NULL || result == NULL){
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(objIid);
	while(zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &objIid, (void **)&iface) == ZCFG_SUCCESS) {
		if(strcmp(iface->LowerLayers, lowerLayer) == 0) {
			sprintf(result,"IP.Interface.%d",objIid.idx[0]);// TR181 path is one base.
			printf("result: %s\n",result);
			zcfgFeObjStructFree(iface);
			return ZCFG_SUCCESS;
		}

		zcfgFeObjStructFree(iface);
	}

	return ZCFG_NO_SUCH_OBJECT;

}

int getVoiceAttrGet(zcfg_offset_t oid, int handler, char *paramName){
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;
#ifdef USE_OLD_VOIP_PREFIX
	char tempName[64] = {0};
#endif

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

#ifdef USE_OLD_VOIP_PREFIX
		if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}else if(!strncmp(paramList->name, "X_ZYXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}else if(!strncmp(paramList->name, "X_ZyXEL_", 8)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 8);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}
		else
#endif
		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);

		if(attrValue < 0 ) {
			attrValue = 0;
			break;
		}

		break;
	}

	return attrValue;
}

zcfgRet_t getVoiceAttrSet(zcfg_offset_t oid, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;
#ifdef USE_OLD_VOIP_PREFIX
	char tempName[64] = {0};
#endif

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

#ifdef USE_OLD_VOIP_PREFIX
		if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}else if(!strncmp(paramList->name, "X_ZYXEL_COM_", 12)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}else if(!strncmp(paramList->name, "X_ZyXEL_", 8)){
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 8);
			attrValue = zcfgFeParamAttrGetByName(oid, tempName);
		}
		else
#endif
		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);

#ifdef USE_OLD_VOIP_PREFIX
		if(strncmp(paramList->name, "X_ZyXEL_COM_", 12) && strncmp(paramList->name, "X_ZYXEL_COM_", 12 ) && strncmp(paramList->name, "X_ZyXEL_", 8))
			strcpy(tempName, paramList->name);

		if( (ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, tempName, attrValue)) != ZCFG_SUCCESS){
			
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
#else
		if( (ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue)) != ZCFG_SUCCESS){
			
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
#endif

		break;

	} /*Edn while*/
	
	return ret;
}

zcfgRet_t voiceSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t srvIid;
	

	IID_INIT(srvIid);
	srvIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu", &srvIid.idx[srvIid.level - 1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_SRV, &srvIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_SRV with ret=%d", __FUNCTION__, ret);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}


int voiceSrvAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_SRV, handler, paramName);
}

zcfgRet_t voiceSrvAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_SRV, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceCapbObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t capbIid;
	

	IID_INIT(capbIid);
	capbIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.Capabilities", &capbIid.idx[capbIid.level - 1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_CAPB, &capbIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_CAPB with ret=%d", __FUNCTION__, ret);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}

int voiceCapbAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_CAPB, handler, paramName);
}

zcfgRet_t voiceCapbAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_CAPB, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceCapbSipObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t capbSipIid;

	IID_INIT(capbSipIid);
	capbSipIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.Capabilities.SIP", &capbSipIid.idx[capbSipIid.level - 1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_CAPB_SIP, &capbSipIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_CAPB_SIP with ret=%d", __FUNCTION__, ret);
		return ret;
	}
	return ZCFG_SUCCESS;
}

int voiceCapbSipAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_CAPB_SIP, handler, paramName);
}

zcfgRet_t voiceCapbSipAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_CAPB_SIP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceCapbCodecObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t capbCodecIid;

	IID_INIT(capbCodecIid);
	capbCodecIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.Capabilities.Codec.%hhu", 
		&capbCodecIid.idx[0], &capbCodecIid.idx[1]);
	
	if((ret = getVoiceConfig(RDM_OID_VOICE_CAPB_CODEC, &capbCodecIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_CAPB_CODEC with ret=%d", __FUNCTION__, ret);
		return ret;
	}
	return ZCFG_SUCCESS;
}

int voiceCapbCodecAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_CAPB_CODEC, handler, paramName);
}

zcfgRet_t voiceCapbCodecAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_CAPB_CODEC, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid, objIid;
	char *intf = NULL, *tmp_ptr = NULL, *boundList = NULL;
	char tempName[128] = {0};
	char tr98ObjName[128] = {0};
	char boundIfList[512] = {0};
	rdm_IpIface_t *ipIfaceObj = NULL;

	IID_INIT(profIid);
	profIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu", 
		&profIid.idx[0], &profIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF, &profIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	/*Brady 20161017, the format of BoundList should follow TR-98 path for GET(WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}/WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.{i}). */
	boundList = json_object_get_string(json_object_object_get(*tr98Jobj,"X_ZYXEL_BoundIfList"));
	if(boundList != NULL){
		intf = strtok_r(boundList, ", ", &tmp_ptr);
		boundIfList[0] = '\0';
		while(intf != NULL){
			if(*intf != '\0' ){
				IID_INIT(objIid);
				objIid.level = 1;
				sscanf(intf, "IP.Interface.%hhu", &objIid.idx[0]);
				memset(tr98ObjName, 0, sizeof(tr98ObjName));
				memset(tempName, 0, sizeof(tempName));
				if(feObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIfaceObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS ) {
					if(strstr(ipIfaceObj->LowerLayers, "PPP.Interface") != NULL){
						zcfgFe181To98MappingNameGet(ipIfaceObj->LowerLayers, tr98ObjName);
					}else{
						zcfgFe181To98MappingNameGet(intf, tr98ObjName);
					}
					
					if(tr98ObjName[0] != '\0'){
						sscanf(tr98ObjName,"InternetGatewayDevice.%s",tempName);
						if(boundIfList[0] != '\0'){
							strcat(boundIfList, ",");
						}
						strcat(boundIfList, tempName);
					}

					zcfgFeObjStructFree(ipIfaceObj);

				}else{
					printf("%s() Can't find corresponding ipIface Obj ,interface(%s)\n", __FUNCTION__, intf);
				}
				

			}
			intf = strtok_r(NULL, ", ", &tmp_ptr);
		}
		json_object_object_add(*tr98Jobj, "X_ZYXEL_BoundIfList", json_object_new_string(boundIfList));
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid;
	struct json_object *profObj = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *paramValue = NULL;
	char *intf = NULL, *tmp_ptr = NULL;
	char *value = NULL;
	char *enalbe = NULL;
	char new_string[128]={0};
	char tr181ObjPath[128] = {0};
	char tr98ObjPath[128] = {0};
	char tr181Name[32] = {0};
	char boundIfList[512] = {0};
	
	IID_INIT(profIid);
	profIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu", 
		&profIid.idx[0], &profIid.idx[1]);
	
	enalbe = json_object_get_string(json_object_object_get(tr98Jobj,"Enable"));
	
	if(!strcmp(enalbe, "Enabled") || !strcmp(enalbe, "Disabled") )
	{
		zcfgLog(ZCFG_LOG_INFO,"%s: VoiceProfObj->Enable : (%s) \n",__FUNCTION__,enalbe);
	}else{
		zcfgLog(ZCFG_LOG_ERR, "%s:VoiceProfObj->Enable only support Disabled and Enabled\n", __FUNCTION__);
		strcpy(paramfault,"Enable");

		return ZCFG_NO_SUCH_PARAMETER;	
	}
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_PROF, &profIid, &profObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get voice prof object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* set(update) value to target object */
	if(profObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			value = json_object_get_string(paramValue);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
				/* special case */
				if(!strcmp(paramList->name, "X_ZYXEL_FaxMode")){
					if(!strcmp(value, "Disable")){
/*20150904 Braby. Fax support PassThrough & T38Relay."Fax disable" only stop to send re-Invite,dont support close function realy,remove "disable" option*/
#if 0
						strcpy(new_string,"Disable");
						json_object_object_add(profObj, "ModemPassThrough", json_object_new_string("Disable"));
						json_object_object_add(profObj, "FAXPassThrough", json_object_new_string("Disable"));
#else
						strcpy(paramfault,"X_ZYXEL_FaxMode");
						zcfgFeJsonObjFree(profObj);
						return ZCFG_NO_SUCH_PARAMETER;
#endif
/*20150904 Braby end.*/
					}else if(!strcmp(value, "G711PassThrough")){
						strcpy(new_string,"G711PassThrough");
						json_object_object_add(profObj, "ModemPassThrough", json_object_new_string("Auto"));
						json_object_object_add(profObj, "FAXPassThrough", json_object_new_string("Auto"));
					}else if(!strcmp(value, "T38Relay")){
						strcpy(new_string,"T38Relay");
						json_object_object_add(profObj, "ModemPassThrough", json_object_new_string("Auto"));
						json_object_object_add(profObj, "FAXPassThrough", json_object_new_string("Auto"));
					}else{
						strcpy(paramfault,"X_ZYXEL_FaxMode");
						zcfgFeJsonObjFree(profObj);
				    	return ZCFG_NO_SUCH_PARAMETER;  
					}
					json_object_object_add(profObj, paramList->name, json_object_new_string(new_string));
				}else if(!strcmp(paramList->name, "X_ZYXEL_BoundIfList")){
					char paramValue[512+1] = {0};
					strncpy(paramValue, value, sizeof(paramValue)-1);
					/*Brady 20161017, the format of BoundList followed TR-181 path for SET(IP.Interface.{i}). */
					intf = strtok_r(paramValue, ", ", &tmp_ptr);
					while(intf != NULL){
						if(*intf != '\0' ){
							memset(tr181Name, 0, sizeof(tr181Name));
							memset(tr181ObjPath, 0, sizeof(tr181ObjPath));
							snprintf(tr98ObjPath, sizeof(tr98ObjPath), "InternetGatewayDevice.%s",intf);
							zcfgFe98To181MappingNameGet(tr98ObjPath, tr181ObjPath);
							if(strstr(tr181ObjPath, "PPP.Interface") != NULL){
								zcfgFeTr181FindingIpIfaceByLowerLayer(tr181ObjPath, tr181Name);
							}else if(strstr(tr181ObjPath, "IP.Interface") != NULL){
								strcpy(tr181Name, tr181ObjPath);
							}else{
								printf("%s() Bonded Interface Not accept. Interface:%s\n", __FUNCTION__, tr181ObjPath);
							}

							if(tr181Name[0] != '\0'){
								if(boundIfList[0] != '\0'){
									strcat(boundIfList, ",");
								}	
								strcat(boundIfList, tr181Name);
							}
							
						}
						intf = strtok_r(NULL, ", ", &tmp_ptr);
					}
					json_object_object_add(profObj, "X_ZYXEL_BoundIfList", json_object_new_string(boundIfList));
				}else{
					json_object_object_add(profObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}

		if(multiJobj) {
			tmpObj = profObj;
			profObj = NULL;
			profObj = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF, &profIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}else {
			ret = zcfgFeObjJsonSet(RDM_OID_VOICE_PROF, &profIid, profObj, NULL);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set VOICE_PROFs Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set VOICE_PROF Success\n", __FUNCTION__);
			}
			zcfgFeJsonObjFree(profObj);
		}
	
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid;
	zcfgMsg_t *msgHdr = NULL;

	IID_INIT(profIid);
	profIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.", &profIid.idx[0]);
	
	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_VOICE_PROF, &profIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add voice profile instance with ret=%d\n", __FUNCTION__, ret);
		return ZCFG_INTERNAL_ERROR;//ZCFG_INS_CREATE_ERR??
	}

	*idx = profIid.idx[profIid.level - 1];

	if((msgHdr = (zcfgMsg_t *)calloc(1, sizeof(zcfgMsg_t))) == NULL){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to allocate message xmit buffer\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	msgHdr->type = ZCFG_MSG_VOICE_CONFIG_SAVED;
	msgHdr->dstEid = ZCFG_EID_ZYIMS;
	msgHdr->srcEid = ZCFG_EID_TR69;

	if((ret = zcfgMsgSendAndGetReply(msgHdr, NULL, 0)) != ZCFG_SUCCESS_AND_NO_REPLY){
		zcfgLog(ZCFG_LOG_ERR, "%s : fail to send msg to ESMD with ret=%d\n", __FUNCTION__, ret);
		return ZCFG_INTERNAL_ERROR;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profIid;

	IID_INIT(profIid);
	profIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu", 
		&profIid.idx[0], &profIid.idx[1]);

	//if((ret = zcfgFeObjJsonDel(RDM_OID_VOICE_PROF, &profIid, NULL)) != ZCFG_SUCCESS){
	ret = zcfgFeObjJsonDelayRdmDel(RDM_OID_VOICE_PROF, &profIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS)) {
		zcfgLog(ZTR_DL_ERR, "%s: fail to delete profile instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}

int voiceProfAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF, handler, paramName);
}

zcfgRet_t voiceProfAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfSipObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profSipIid;

	IID_INIT(profSipIid);
	profSipIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.SIP", 
		&profSipIid.idx[0], &profSipIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_SIP, &profSipIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_SIP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfSipSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profSipIid;

	IID_INIT(profSipIid);
	profSipIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.SIP", 
		&profSipIid.idx[0], &profSipIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_SIP, &profSipIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_SIP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfSipAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_SIP, handler, paramName);
}

zcfgRet_t voiceProfSipAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_SIP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfRtpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpIid;

	IID_INIT(profRtpIid);
	profRtpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP",
		&profRtpIid.idx[0], &profRtpIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_RTP, &profRtpIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_RTP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfRtpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpIid;

	IID_INIT(profRtpIid);
	profRtpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP",
		&profRtpIid.idx[0], &profRtpIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_RTP, &profRtpIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_RTP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfRtpAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_RTP, handler, paramName);
}

zcfgRet_t voiceProfRtpAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_RTP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfRtpRtcpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpRtcpIid;

	IID_INIT(profRtpRtcpIid);
	profRtpRtcpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP.RTCP",
		&profRtpRtcpIid.idx[0], &profRtpRtcpIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_RTP_RTCP, &profRtpRtcpIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_RTP_RTCP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfRtpRtcpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpRtcpIid;

	IID_INIT(profRtpRtcpIid);
	profRtpRtcpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP.RTCP",
		&profRtpRtcpIid.idx[0], &profRtpRtcpIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_RTP_RTCP, &profRtpRtcpIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_RTP_RTCP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfRtpRtcpAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_RTP_RTCP, handler, paramName);
}

zcfgRet_t voiceProfRtpRtcpAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_RTP_RTCP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfRtpSrtpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpSrtpIid;

	IID_INIT(profRtpSrtpIid);
	profRtpSrtpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP.SRTP",
		&profRtpSrtpIid.idx[0], &profRtpSrtpIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_RTP_SRTP, &profRtpSrtpIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_RTP_SRTP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfRtpSrtpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profRtpSrtpIid;

	IID_INIT(profRtpSrtpIid);
	profRtpSrtpIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.RTP.SRTP",
		&profRtpSrtpIid.idx[0], &profRtpSrtpIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_RTP_SRTP, &profRtpSrtpIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_RTP_SRTP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfRtpSrtpAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_RTP_SRTP, handler, paramName);
}

zcfgRet_t voiceProfRtpSrtpAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_RTP_SRTP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfNumPlanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profNumPlanIid;

	IID_INIT(profNumPlanIid);
	profNumPlanIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.NumberingPlan", 
		&profNumPlanIid.idx[0], &profNumPlanIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_NUM_PLAN, &profNumPlanIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_NUM_PLAN with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfNumPlanObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profNumPlanIid;

	IID_INIT(profNumPlanIid);
	profNumPlanIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.NumberingPlan", 
		&profNumPlanIid.idx[0], &profNumPlanIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_NUM_PLAN, &profNumPlanIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_NUM_PLAN with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfNumPlanAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_NUM_PLAN, handler, paramName);
}

zcfgRet_t voiceProfNumPlanAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_NUM_PLAN, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceProfNumPlanPrefixInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profNumPlanPrefixInfoIid;

	IID_INIT(profNumPlanPrefixInfoIid);
	profNumPlanPrefixInfoIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.NumberingPlan.PrefixInfo.%hhu",
		&profNumPlanPrefixInfoIid.idx[0], &profNumPlanPrefixInfoIid.idx[1], &profNumPlanPrefixInfoIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profNumPlanPrefixInfoIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceProfNumPlanPrefixInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profNumPlanPrefixInfoIid;
	const char *prefixRange = NULL;
	const char *facilityAction = NULL;
	const char *prefixMinNumberOfDigits = NULL;
	const char *prefixMaxNumberOfDigits	= NULL;

	struct json_object * prefixInfoObj = NULL;
	tr98Parameter_t *paramList = NULL;

	IID_INIT(profNumPlanPrefixInfoIid);
	profNumPlanPrefixInfoIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.NumberingPlan.PrefixInfo.%hhu",
		&profNumPlanPrefixInfoIid.idx[0], &profNumPlanPrefixInfoIid.idx[1], &profNumPlanPrefixInfoIid.idx[2]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profNumPlanPrefixInfoIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceProfNumPlanPrefixInfoAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, handler, paramName);
}

zcfgRet_t voiceProfNumPlanPrefixInfoAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, handler, paramName, newNotify, multiAttrJobj);
}
zcfgRet_t voiceProfFaxT38ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{


	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profFaxT38Iid;

	IID_INIT(profFaxT38Iid);
	profFaxT38Iid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.FaxT38", 
		&profFaxT38Iid.idx[0], &profFaxT38Iid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_PROF_FAX_T38, &profFaxT38Iid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_PROF_FAX_T38 with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}


zcfgRet_t voiceProfFaxT38ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t profFaxT38Iid;
#ifdef ZYXEL_VOICE_CONFIG_TR069_FAXT38_ENABLE_MAPPING_TO_FAXMODE
	/*yushiuan add for WIND , Because WIND need control Oid: RDM_OID_VOICE_PROF_FAX_T38 , parameter : Enable via TR069 ,
	   If Enable = true , it represent CPE will use T38 to do fax transmission , If Enable = false , CPE will use G711 to do fax transmission.
	   But for ZyIMS , we already use X_ZYXEL_FaxMode to indicate CPE will use which codec to do fax , in order to avoid modifying
	   ZyIMS architecture , I implement this mapping in front-end(TR69) , according to Enable value to determine X_ZYXEL_FaxMode
	   will use T38 or G711 . Others parameters under RDM_OID_VOICE_PROF_FAX_T38 dont need mapping. 2015/11/27*/
	struct json_object *profFaxT38Obj = NULL;
	struct json_object *profObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tmpObj_2 = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	bool t38Enable = true;
	char faxMode[32];


	IID_INIT(profFaxT38Iid);
	profFaxT38Iid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.FaxT38", 
		&profFaxT38Iid.idx[0], &profFaxT38Iid.idx[1]);

	t38Enable = json_object_get_boolean(json_object_object_get(tr98Jobj,"Enable"));
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_PROF_FAX_T38, &profFaxT38Iid, &profFaxT38Obj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get voice Profile FaxT38 object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_PROF, &profFaxT38Iid, &profObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get voice profile object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	//json_object_get_string(&profObj);
	if(profFaxT38Obj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL) {
				if(!strcmp(paramList->name, "Enable")){
					if(profObj){
						if(t38Enable){
							json_object_object_add(profObj, "X_ZYXEL_FaxMode", json_object_new_string("T38Relay"));
						}
						else{
							json_object_object_add(profObj, "X_ZYXEL_FaxMode", json_object_new_string("G711PassThrough"));
						}
					}				
				}else{
					json_object_object_add(profFaxT38Obj, paramList->name, JSON_OBJ_COPY(paramValue));					
				}
				
			}
			paramList++;
			continue;
		}
		
		if(multiJobj) {
			tmpObj = profFaxT38Obj;
			profFaxT38Obj = NULL;
			profFaxT38Obj = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF_FAX_T38, &profFaxT38Iid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			tmpObj_2 = profObj;
			profFaxT38Obj = NULL;
			profFaxT38Obj = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF, &profFaxT38Iid, multiJobj, tmpObj_2);
			json_object_put(tmpObj_2);
		}else {
			ret = zcfgFeObjJsonSet(RDM_OID_VOICE_PROF_FAX_T38, &profFaxT38Iid, profFaxT38Obj, NULL);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set VOICE_PROF_Fax_T38 is Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set VOICE_PROF_Fax_T38 Success\n", __FUNCTION__);
			}
			zcfgFeJsonObjFree(profFaxT38Obj);
			ret = zcfgFeObjJsonSet(RDM_OID_VOICE_PROF, &profFaxT38Iid, profObj, NULL);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set VOICE_PROF is Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set VOICE_PROF Success\n", __FUNCTION__);
			}
			zcfgFeJsonObjFree(profObj);
		}

	}
#else
	
	IID_INIT(profFaxT38Iid);
	profFaxT38Iid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.FaxT38", 
		&profFaxT38Iid.idx[0], &profFaxT38Iid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_PROF_FAX_T38, &profFaxT38Iid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_PROF_FAX_T38 with ret=%d", __FUNCTION__, ret);
		return ret;
	}

#endif

	return ZCFG_SUCCESS;
}



int voiceProfFaxT38AttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	
	return getVoiceAttrGet(RDM_OID_VOICE_PROF_FAX_T38, handler, paramName);
}

zcfgRet_t voiceProfFaxT38AttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PROF_FAX_T38, handler, paramName, newNotify, multiAttrJobj);
}


zcfgRet_t voiceLineObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineIid;

	IID_INIT(lineIid);
	lineIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu", 
		&lineIid.idx[0], &lineIid.idx[1], &lineIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE, &lineIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

#if ZYXEL_VOICE_CONFIG_TR069_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST

#define ERROR_PHONE_SELECT_BITMAP 0xFFFF
int getSipLineCnt()
{
	int cnt = 0;
	objIndex_t objIid;
	struct json_object *lineObj = NULL;

	IID_INIT(objIid);
	while((zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &objIid, &lineObj)) == ZCFG_SUCCESS){
		if(lineObj != NULL){
			cnt++;
		}
		zcfgFeJsonObjFree(lineObj);
	}
	return cnt;
}

int getFxsPhyInterfaceCnt()
{
	int cnt = 0;
	objIndex_t objIid;
	struct json_object *PhyIntfObj = NULL;

	IID_INIT(objIid);
	while((zcfgFeObjJsonGetNext(RDM_OID_VOICE_PHY_INTF, &objIid, &PhyIntfObj)) == ZCFG_SUCCESS){
		if(PhyIntfObj != NULL){
			cnt++;
		}
		zcfgFeJsonObjFree(PhyIntfObj);
	}
	return cnt;
}

uint32_t parsePhonePortList(char *phoneList){//ex. input(PhyReferenceList):0,1(1,2)  output(bitmap):0x3
	char List[256] = {0};
	char *tmpStr = NULL;
	int phyPort;
	uint32_t listBitMap = 0;

	if(phoneList == NULL){
		printf("%s()-PhonePortList is NULL...fail return!\n  ",__FUNCTION__);
		return ERROR_PHONE_SELECT_BITMAP; //0xFFFF to indicate Error/Fail return.
	}

	strcpy(List, phoneList);
	tmpStr = strtok(List,",");
	while (tmpStr != NULL){
		if(true != sscanf(tmpStr,"%d",&phyPort)){
		#if SKIP_NON_NUMBER_PHONE_PORT_VALUE_WHILE_PARSE_PHY_REFERENCE_LIST
			zcfgLog(ZCFG_LOG_ERR, "%s : parse phone port num fail ==> Skip this one and goto next!!\n", __FUNCTION__);
			tmpStr = strtok (NULL, ",");
			continue;
		#else
			zcfgLog(ZCFG_LOG_ERR, "%s : parse phone port num fail ==> Fail retuen!!\n", __FUNCTION__);
			return ERROR_PHONE_SELECT_BITMAP;
		#endif //SKIP_NON_NUMBER_PHONE_PORT_VALUE_WHILE_PARSE_PHY_REFERENCE_LIST.
		}
#if defined (ZYPRJ_CUSTOMER_WIND) || defined (WIND_CUSTOMIZATION)//0-base
		phyPort = phyPort+1;
#endif
		if((phyPort > sizeof(uint32_t)*8) || (phyPort <= 0)){
			zcfgLog(ZCFG_LOG_ERR, "%s : Invalid value (%d), the accepatble range: [1, %d]!!\n", __FUNCTION__, phyPort, sizeof(uint32_t)*8);
			return ERROR_PHONE_SELECT_BITMAP;
		}
		listBitMap |= 1 << (phyPort-1);
		tmpStr = strtok (NULL, ",");
	}
	return listBitMap;
}

bool voiceLinePhyReferenceListCheckObj(struct json_object *multiJobj, objIndex_t lineIid, uint32_t referenceList)
{
	struct json_object *oidJobj = NULL;
	struct json_object *paramJobj = NULL;
	struct json_object *iidJobj = NULL;
	struct json_object *lineObj = NULL;
	uint32_t *multiObjList = NULL;
	uint32_t *readyList = NULL;
	uint32_t checkMultiList = 0;
	uint32_t checkCfgList = 0;
	uint32_t finalCheckList = 0;
	uint32_t tmpList = 0;
	char *value = NULL;
	char *LineSelect = NULL;
	char objIndexID[32] = {0};
	char objID[8] = {0};
	objIndex_t Iid;
	int lineNumber = 0;
	int lineIdx = 0;
	bool ret = false;

	lineNumber = getSipLineCnt();
	if(lineNumber == 0){
		printf("<<Error>>No any SipLine exist, so phone port can't be binded!!\n");
		return ret;//false
	}
	//printf("lineNumber : %d\n",lineNumber);

	readyList = (uint32_t *)malloc(lineNumber*sizeof(uint32_t));
	if(readyList == NULL){
		printf("%s(): <<Error>> 'readyList' init: NO enough memory!!\n", __FUNCTION__);
		return ret;//false
	}
	memset(readyList, 0, lineNumber*sizeof(uint32_t));

	multiObjList = (uint32_t *)malloc(lineNumber*sizeof(uint32_t));
	if(multiObjList == NULL){
		printf("%s(): <<Error>> 'multiObjList' init: NO enough memory!!\n", __FUNCTION__);
		free(readyList);
		return ret;//false
	}
	memset(multiObjList, 0, lineNumber*sizeof(uint32_t));

	//Step 1.To parse data model of phone binding.
	IID_INIT(Iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &Iid, &lineObj) == ZCFG_SUCCESS) {
		LineSelect = json_object_get_string(json_object_object_get(lineObj, "PhyReferenceList"));
		//1.1 Don't check the same index of sip account.
		lineIdx = Iid.idx[2];
		if(lineIdx == lineIid.idx[2]){
			continue;
		}

		//1.2 List of data model
		tmpList = parsePhonePortList(LineSelect);
		//printf("tmpList (lineIdx=%d) : 0x%08X (from the System Config).\n", lineIdx, tmpList);
		if(tmpList != ERROR_PHONE_SELECT_BITMAP){
			readyList[lineIdx-1] = tmpList;
		}else{
			printf("%s: <<Error>>Parse sip line object parameter 'PhyReferenceList' from the Syetem Config fail!!\n", __FUNCTION__);
			zcfgFeJsonObjFree(lineObj);
			free(readyList);
			free(multiObjList);
			return ret;//false
		}
		zcfgFeJsonObjFree(lineObj);
	}

	//Step2.To parse multiObj of phone binding
	sprintf(objID, "%d", RDM_OID_VOICE_LINE);
	oidJobj = json_object_object_get(multiJobj, objID);
	if(oidJobj != NULL) {
		for(lineIdx=1; lineIdx <= lineNumber; lineIdx++){
			sprintf(objIndexID, "%d.%d.%d.%d.%d.%d.%d", 3, lineIid.idx[0], lineIid.idx[1], lineIdx, 0, 0, 0);
			//printf("objIndexID : %s\n",objIndexID);
			iidJobj = json_object_object_get(oidJobj, objIndexID);

			if(iidJobj != NULL){// Multi Object is not empty.
				//printf("[Multi]Object :\n [%s]\n",json_object_to_json_string(iidJobj));
				paramJobj = json_object_object_get(iidJobj, "PhyReferenceList");
				if(paramJobj != NULL){
					value = json_object_get_string(paramJobj);
					//printf("[Multi]PhyRefernceList(%d) : %s\n", lineIdx, value?value:"(NULL)");

					tmpList = parsePhonePortList(value);
					//printf("tmpList (lineIdx=%d) : 0x%08X (from the MultiObject).\n", lineIdx, tmpList);
					if(tmpList != ERROR_PHONE_SELECT_BITMAP){
						multiObjList[lineIdx-1] = tmpList;
						readyList[lineIdx-1] = 0;// Don't check data model.
					}else{
						printf("%s: <<Error>>Parse sip line object parameter 'PhyReferenceList' from the MultiObject fail!!\n", __FUNCTION__);
						free(readyList);
						free(multiObjList);
						return ret;//false
					}
				}
			}
		}
	}

	//Step3.To check result of phone binding is not duplicated.
	for(lineIdx=0 ; lineIdx < lineNumber; lineIdx++){
		checkCfgList |= readyList[lineIdx];
		finalCheckList |= readyList[lineIdx];
		checkMultiList |= multiObjList[lineIdx];
		finalCheckList |= multiObjList[lineIdx];
	}
	//printf("out checkCfgList   : 0x%08X.\n",checkCfgList);
	//printf("out checkMultiList : 0x%08X.\n",checkMultiList);
	//printf("out finalCheckList : 0x%08X.\n",finalCheckList);

	free(readyList);
	free(multiObjList);

	if((referenceList&finalCheckList) == 0){
		ret = true;
	}else{//phone port is duplicated.
		ret = false;
	}

	return ret; //true: No Conflict, false: Confict.
}
#endif

zcfgRet_t voiceLineSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineIid;
	char *enable = NULL;
	uint32_t tmpList = 0;
	uint32_t maxList = 0;

	IID_INIT(lineIid);
	lineIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu", 
		&lineIid.idx[0], &lineIid.idx[1], &lineIid.idx[2]);

	enable = json_object_get_string(json_object_object_get(tr98Jobj,"Enable"));

	if(!strcmp(enable, "Enabled") || !strcmp(enable, "Disabled") )
    {
		zcfgLog(ZCFG_LOG_INFO,"%s: VoiceLineObj->Enable : (%s) \n",__FUNCTION__,enable);
    }else{
        zcfgLog(ZCFG_LOG_ERR, "%s:VoiceLineObj->Enable only support Disabled and Enabled\n", __FUNCTION__);
		strcpy(paramfault,"Enable");
    	return ZCFG_NO_SUCH_PARAMETER;  
    }

#if ZYXEL_VOICE_CONFIG_TR069_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST
	/*Brady 20151127.To support phone port binding by standard branches of TR104.To avoid bind the same phone port*/
	printf("%s(): Check PHY_REFERENCE_LIST..\n",__FUNCTION__);
	char *PhyReferenceList = NULL;
	uint32_t referenceList = 0;
	int FxsPhyInterfaceCnt = 0;

	PhyReferenceList = json_object_get_string(json_object_object_get(tr98Jobj,"PhyReferenceList"));
	tmpList = parsePhonePortList(PhyReferenceList);
	if(tmpList != ERROR_PHONE_SELECT_BITMAP){
		referenceList = tmpList;
	}else{
		printf("%s: <<Error>>Parse sip line object parameter 'PhyReferenceList' from the TR069 Input fail!!\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("input list : %d (0x%08X)\n", referenceList, referenceList);
	FxsPhyInterfaceCnt = getFxsPhyInterfaceCnt();
	//printf("FxsPhyInterfaceCnt : %d\n",FxsPhyInterfaceCnt);
	if(FxsPhyInterfaceCnt <= 0){
		printf("<<Error>>No any FxsPhyInterface exist,so sip account can't bind phone port!!\n");
		return ZCFG_INTERNAL_ERROR;
	}

	for(int i=0;i<FxsPhyInterfaceCnt;i++){//maxlist indicate how many phone port exist.
		maxList |= 1 <<(i);
	}
	//printf("maxList : (0x%08X)\n", maxList);

	if(referenceList > maxList){// Input list can't exeed  maxList.
		printf("<<Error>>'PhyReferenceList'(0x%08X) exceed max value(0x%08X)!!\n", referenceList, maxList);
		return ZCFG_INTERNAL_ERROR;
	}
#if 0
	/*To check input list is duplicated or not*/
	if(voiceLinePhyReferenceListCheckObj(multiJobj, lineIid, referenceList) != true){
		zcfgLog(ZCFG_LOG_ERR, "%s : phone port binding is failed!!\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
#endif
	/*Brady end*/
#endif

	if((ret = setVoiceConfig(RDM_OID_VOICE_LINE, &lineIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_LINE with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineAdd(char *tr98FullPathName, int *idx){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineIid;
	zcfgMsg_t *msgHdr = NULL;

	IID_INIT(lineIid);
	lineIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.", 
		&lineIid.idx[0], &lineIid.idx[1]);
	
	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_VOICE_LINE, &lineIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add voice line instance with ret=%d\n", __FUNCTION__, ret);
		return ZCFG_INTERNAL_ERROR;//ZCFG_INS_CREATE_ERR??
	}

	*idx = lineIid.idx[lineIid.level - 1];

	if((msgHdr = (zcfgMsg_t *)calloc(1, sizeof(zcfgMsg_t))) == NULL){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to allocate message xmit buffer\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	msgHdr->type = ZCFG_MSG_VOICE_CONFIG_SAVED;
	msgHdr->dstEid = ZCFG_EID_ZYIMS;
	msgHdr->srcEid = ZCFG_EID_TR69;

	if((ret = zcfgMsgSendAndGetReply(msgHdr, NULL, 0)) != ZCFG_SUCCESS_AND_NO_REPLY){
		zcfgLog(ZCFG_LOG_ERR, "%s : fail to send msg to ESMD with ret=%d\n", __FUNCTION__, ret);
		return ZCFG_INTERNAL_ERROR;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineDel(char *tr98FullPathName){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineIid;

	IID_INIT(lineIid);
	lineIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu", 
		&lineIid.idx[0], &lineIid.idx[1], &lineIid.idx[2]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_VOICE_LINE, &lineIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete line instance with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}
	
	return ZCFG_SUCCESS;
}

int voiceLineAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE, handler, paramName);
}

zcfgRet_t voiceLineAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceLineCallingFeatureObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t linecfIid;
	char *RDM_name = NULL;
	
	IID_INIT(linecfIid);
	linecfIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.CallingFeatures", 
		&linecfIid.idx[0], &linecfIid.idx[1], &linecfIid.idx[2]);

	struct json_object *getObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_LINE_CALLING_FEATURE, &linecfIid, &getObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get obj(oid %d) codec object ret=%d\n", __FUNCTION__, RDM_OID_VOICE_LINE_CALLING_FEATURE, ret);
		return ret;
	}
	

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;

	while(paramList->name){
		/*Brady 20150709, follow TR104 to use AnonymousCalEnable*/
		if(!strcmp(paramList->name, "AnonymousCalEnable")){
				char *MapName = "AnonymousCallEnable";
				RDM_name = MapName;
		}else{
				RDM_name = paramList->name;
		}

		/* get parameter value from tr181 */
		paramValue = json_object_object_get(getObj, RDM_name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {
			struct json_object *cfObj = NULL;
			/*Brady 20150709, follow TR104 to use AnonymousCalEnable*/
			bool Enable;
			if(!strcmp(RDM_name, "AnonymousCallEnable"))
			{
				Enable = json_object_get_boolean(paramValue);
				json_object_object_add(*tr98Jobj,"AnonymousCalEnable",json_object_new_boolean(Enable));
			}
			else
			{
				json_object_object_add(*tr98Jobj, RDM_name, JSON_OBJ_COPY(paramValue));
			}
		}

		paramList++;
		continue;
	}
	zcfgFeJsonObjFree(getObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineCallingFeatureObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t linecfIid;
	char *RDM_name = NULL;

	IID_INIT(linecfIid);
	linecfIid.level = 3;

	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.CallingFeatures", 
		&linecfIid.idx[0], &linecfIid.idx[1], &linecfIid.idx[2]);

	struct json_object *obj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	bool Enable;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_LINE_CALLING_FEATURE, &linecfIid, &obj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get object(oid %d) with ret=%d\n", __FUNCTION__, RDM_OID_VOICE_LINE_CALLING_FEATURE, ret);
		return ret;
	}
	
	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = obj;
		obj = NULL;
		obj = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE_CALLING_FEATURE, &linecfIid, multiJobj, tmpObj);
	}

	if(obj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			paramValue = json_object_object_get(tr98Jobj, paramList->name);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
				/*Brady 20150709, follow TR104 to use AnonymousCalEnable*/
				if(!strcmp(paramList->name, "AnonymousCalEnable")){
					Enable = json_object_get_boolean(paramValue);
					json_object_object_del(obj, "AnonymousCallEnable");
					json_object_object_add(obj, "AnonymousCallEnable", json_object_new_boolean(Enable));
				}else{					
					json_object_object_add(obj, paramList->name, JSON_OBJ_COPY(paramValue));
				}

			}

			paramList++;
			continue;
		}

		if(!multiJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_VOICE_LINE_CALLING_FEATURE, &linecfIid, obj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set object(oid %d) Fail with ret=%d\n", __FUNCTION__, RDM_OID_VOICE_LINE_CALLING_FEATURE, ret);
			}
			zcfgFeJsonObjFree(obj);
			return ret;
		}
		zcfgFeJsonObjFree(tmpObj);
	
	}

	return ZCFG_SUCCESS;
}

int voiceLineCallingFeatureAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	/*Brady 20150709 ,follow TR104 to use AnonymousCalEnable*/
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "AnonymousCalEnable")){
			char *newname ="AnonymousCallEnable";
			paramList->name = newname;
		}

		attrValue = zcfgFeParamAttrGetByName(RDM_OID_VOICE_LINE_CALLING_FEATURE, paramList->name);
		if(attrValue < 0 ) {
			attrValue = 0;
			break;
		}

		if(!strcmp(paramList->name, "AnonymousCallEnable")){
			char *newname ="AnonymousCalEnable";
			paramList->name = newname;
		}

		break;
	}

	return attrValue;
	/*Brady end, 20150709 */
	//return getVoiceAttrGet(RDM_OID_VOICE_LINE_CALLING_FEATURE, handler, paramName);
}

int voiceLineCallingFeatureAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_CALLING_FEATURE, handler, paramName, newNotify, multiAttrJobj);
}


zcfgRet_t voiceLineSipObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineSipIid;

	IID_INIT(lineSipIid);
	lineSipIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.SIP", 
		&lineSipIid.idx[0], &lineSipIid.idx[1], &lineSipIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE_SIP, &lineSipIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE_SIP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineSipSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineSipIid;

	IID_INIT(lineSipIid);
	lineSipIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.SIP", 
		&lineSipIid.idx[0], &lineSipIid.idx[1], &lineSipIid.idx[2]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_LINE_SIP, &lineSipIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_LINE_SIP with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceLineSipAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE_SIP, handler, paramName);
}

zcfgRet_t voiceLineSipAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_SIP, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceLineProcObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineProcIid;

	IID_INIT(lineProcIid);
	lineProcIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.VoiceProcessing", 
		&lineProcIid.idx[0], &lineProcIid.idx[1], &lineProcIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE_PROCESSING, &lineProcIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE_PROCESSING with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineProcSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineProcIid;

	IID_INIT(lineProcIid);
	lineProcIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.VoiceProcessing", 
		&lineProcIid.idx[0], &lineProcIid.idx[1], &lineProcIid.idx[2]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_LINE_PROCESSING, &lineProcIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_LINE_PROCESSING with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceLineProcAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE_PROCESSING, handler, paramName);
}

zcfgRet_t voiceLineProcAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_PROCESSING, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceLineCodecObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineCodecIid;

	IID_INIT(lineCodecIid);
	lineCodecIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.Codec", 
		&lineCodecIid.idx[0], &lineCodecIid.idx[1], &lineCodecIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE_CODEC, &lineCodecIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE_CODEC with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceLineCodecAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE_CODEC, handler, paramName);
}

zcfgRet_t voiceLineCodecAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_CODEC, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceLineCodecListObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineCodecListIid;

	IID_INIT(lineCodecListIid);
	lineCodecListIid.level = 4;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.Codec.List.%hhu", 
		&lineCodecListIid.idx[0], &lineCodecListIid.idx[1], &lineCodecListIid.idx[2], &lineCodecListIid.idx[3]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE_CODEC_LIST, &lineCodecListIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE_CODEC_LIST with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineCodecListObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)

{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t lineCodecListIid;

	IID_INIT(lineCodecListIid);
	lineCodecListIid.level = 4;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.Codec.List.%hhu", 
	&lineCodecListIid.idx[0], &lineCodecListIid.idx[1], &lineCodecListIid.idx[2], &lineCodecListIid.idx[3]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_LINE_CODEC_LIST, &lineCodecListIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_LINE_CODEC_LIST with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}


int voiceLineCodecListAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE_CODEC_LIST, handler, paramName);
}

zcfgRet_t voiceLineCodecListAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_CODEC_LIST, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceLineStatsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t voiceLineStatsIid;

	IID_INIT(voiceLineStatsIid);
	voiceLineStatsIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.Stats", 
		&voiceLineStatsIid.idx[0], &voiceLineStatsIid.idx[1], &voiceLineStatsIid.idx[2]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_LINE_STATS, &voiceLineStatsIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_LINE_STATS with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceLineStatsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t voiceLineStatsIid;

	IID_INIT(voiceLineStatsIid);
	voiceLineStatsIid.level = 3;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.VoiceProfile.%hhu.Line.%hhu.Stats",
		&voiceLineStatsIid.idx[0], &voiceLineStatsIid.idx[1], &voiceLineStatsIid.idx[2]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_LINE_STATS, &voiceLineStatsIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_LINE_STATS with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceLineStatsAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_LINE_STATS, handler, paramName);
}

zcfgRet_t voiceLineStatsAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_LINE_STATS, handler, paramName, newNotify, multiAttrJobj);
}


zcfgRet_t voicePhyIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t phyIntfIid;
	struct json_object *phyIntfObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char lineSelect[128] = {0};

	IID_INIT(phyIntfIid);
	phyIntfIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.PhyInterface.%hhu", 
		&phyIntfIid.idx[0], &phyIntfIid.idx[1]);
	
	if((ret = feObjJsonGet(RDM_OID_VOICE_PHY_INTF, &phyIntfIid, &phyIntfObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get voice Phy Intf object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name){
		/* get parameter value from tr181 */
#ifdef USE_OLD_VOIP_PREFIX
		if(!strncmp(paramList->name, "X_ZyXEL_COM_", 12) || !strncmp(paramList->name, "X_ZYXEL_COM_", 12) || !strncmp(paramList->name, "X_ZyXEL_", 8)){
			char tempName[64] = {0};
			sprintf(tempName, "X_ZYXEL_%s", paramList->name + 12);
			paramValue = json_object_object_get(phyIntfObj, tempName);
		}
		else
#endif
		paramValue = json_object_object_get(phyIntfObj, paramList->name);

		/* write it to tr098 json object */
		if(paramValue != NULL) {

			/* special case */
#ifdef USE_OLD_VOIP_PREFIX
			if(!strcmp(paramList->name, "X_ZyXEL_COM_LineSelect"))
#else
			if(!strcmp(paramList->name, "X_ZYXEL_LineSelect"))
#endif
			{
				lineSelect[0] = '\0';

				if(strcmp(json_object_get_string(paramValue), "") != 0){
    				sprintf(lineSelect, "InternetGatewayDevice.%s", json_object_get_string(paramValue) + strlen("Device."));
				}
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(lineSelect));
				paramList++;
				continue;
			}
			
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}

		paramList++;
		continue;
	}

	zcfgFeJsonObjFree(phyIntfObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t voicePhyIntfSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t phyIntfIid;
	struct json_object *phyIntfObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char lineSelect[128];

	IID_INIT(phyIntfIid);
	phyIntfIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.PhyInterface.%hhu", 
		&phyIntfIid.idx[0], &phyIntfIid.idx[1]);
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_PHY_INTF, &phyIntfIid, &phyIntfObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get voice Phy Intf object ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	/* if multiple set is required, just appended JSON to multiJobj */
	if(multiJobj){
		tmpObj = phyIntfObj;
		phyIntfObj = NULL;
		phyIntfObj = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PHY_INTF, &phyIntfIid, multiJobj, tmpObj);
	}

	/* set(update) value to target object */
	if(phyIntfObj){
		paramList = tr98Obj[handler].parameter;
		while(paramList->name){
			/* get parameter value from tr098 */
			paramValue = json_object_object_get(tr98Jobj, paramList->name);

			/* write it to tr181 json object */
			if(paramValue != NULL) {
				/* special case */
#ifdef USE_OLD_VOIP_PREFIX
				if(!strcmp(paramList->name, "X_ZyXEL_COM_LineSelect"))
#else
				if(!strcmp(paramList->name, "X_ZYXEL_LineSelect"))
#endif
				{
					lineSelect[0] = '\0';
					sprintf(lineSelect, "Device.%s", json_object_get_string(paramValue) + strlen("InternetGatewayDevice."));
					json_object_object_add(phyIntfObj, paramList->name, json_object_new_string(lineSelect));
				}
				else{
					json_object_object_add(phyIntfObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}

			paramList++;
			continue;
		}

		/* is not multiple set, jsut set object immediately after update parameter value */
		if(!multiJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_VOICE_PHY_INTF, &phyIntfIid, phyIntfObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set object RDM_OID_VOICE_PHY_INTF Fail with ret=%d\n", __FUNCTION__, ret);
			}
			zcfgFeJsonObjFree(phyIntfObj);
			return ret;
		}
		zcfgFeJsonObjFree(tmpObj);
	
	}

	return ZCFG_SUCCESS;
}

int voicePhyIntfAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_PHY_INTF, handler, paramName);
}

zcfgRet_t voicePhyIntfAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_PHY_INTF, handler, paramName, newNotify, multiAttrJobj);
}

zcfgRet_t voiceFxsCIDObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t fxsCIDIid;

	IID_INIT(fxsCIDIid);
	fxsCIDIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.PhyInterface.%hhu.X_ZYXEL_CID", &fxsCIDIid.idx[0], &fxsCIDIid.idx[1]);

	if((ret = getVoiceConfig(RDM_OID_VOICE_FXS_CID, &fxsCIDIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_FXS_CID with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceFxsCIDObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t fxsCIDIid;

	IID_INIT(fxsCIDIid);
	fxsCIDIid.level = 2;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.PhyInterface.%hhu.X_ZYXEL_CID", &fxsCIDIid.idx[0], &fxsCIDIid.idx[1]);

	if((ret = setVoiceConfig(RDM_OID_VOICE_FXS_CID, &fxsCIDIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_FXS_CID with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

int voiceFxsCIDAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_FXS_CID, handler, paramName);
}

zcfgRet_t voiceFxsCIDAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_FXS_CID, handler, paramName, newNotify, multiAttrJobj);
}


zcfgRet_t voiceCommonObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t commIid;

	IID_INIT(commIid);
	commIid.level = 1;
#ifdef USE_OLD_VOIP_PREFIX
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZyXEL_COM_Common", &commIid.idx[commIid.level - 1]);
#else
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZYXEL_Common", &commIid.idx[commIid.level - 1]);
#endif

	if((ret = getVoiceConfig(RDM_OID_VOICE_COMMON, &commIid, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_VOICE_COMMON with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceCommonSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t commIid;

	IID_INIT(commIid);
	commIid.level = 1;
#ifdef USE_OLD_VOIP_PREFIX
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZyXEL_COM_Common", &commIid.idx[commIid.level - 1]);
#else
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZYXEL_Common", &commIid.idx[commIid.level - 1]);
#endif

	if((ret = setVoiceConfig(RDM_OID_VOICE_COMMON, &commIid, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_VOICE_COMMON with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

#if 0
int voiceCommonAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	return getVoiceAttrGet(RDM_OID_VOICE_COMMON, handler, paramName);
}

zcfgRet_t voiceCommonAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	return getVoiceAttrSet(RDM_OID_VOICE_COMMON, handler, paramName, newNotify, multiAttrJobj);
}
#endif // 0.


zcfgRet_t voiceCallPolicyBookObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t callPolicyBook;

	IID_INIT(callPolicyBook);
	callPolicyBook.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZYXEL_VoicePhoneBook.CallPolicyBook", &callPolicyBook.idx[callPolicyBook.level - 1]);

	if((ret = getVoiceConfig(RDM_OID_CALL_POLICY_BOOK, &callPolicyBook, handler, tr98Jobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get config from tr181 for object RDM_OID_CALL_POLICY_BOOK with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t voiceCallPolicyBookObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t callPolicyBook;

	IID_INIT(callPolicyBook);
	callPolicyBook.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.Services.VoiceService.%hhu.X_ZYXEL_VoicePhoneBook.CallPolicyBook", &callPolicyBook.idx[callPolicyBook.level - 1]);

	if((ret = setVoiceConfig(RDM_OID_CALL_POLICY_BOOK, &callPolicyBook, handler, tr98Jobj, multiJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to set config to tr181 for object RDM_OID_CALL_POLICY_BOOK with ret=%d", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}


#endif
