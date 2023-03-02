
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

dal_param_t VOIP_REGION_param[]={
	{"Service_Mode",							dalType_string,	0,	0,	NULL,	"Europe_Type|USA_Type",	0},
	{"Region",									dalType_VoipRegion,	0,	0,	NULL,	NULL,	0},
	{"Region_Cap",                              dalType_string,	0,	0,	NULL,	NULL,	0},
    {"FxsCIDSigProtocol",                       dalType_string,  0,  0,  NULL,  NULL,  0},
    {"FxsCIDMode",                              dalType_string,  0,  0,  NULL,  NULL,  0},
	{NULL,										0,				0,	0,	NULL},
};

unsigned int flag1;

const char* X_ZYXEL_FlashUsageStyle;
const char* Region;

struct json_object *profObj;
struct json_object *multiJobj;

objIndex_t Iid = {0};

void zcfgFeDalShowVoipRegion(struct json_object *Jarray){
	struct json_object *obj = NULL;
	const char *serviceMode = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);

		if(!strcmp(json_object_get_string(json_object_object_get(obj, "Service_Mode")),"EUROP_Flash_Style"))
			serviceMode = "Europe Type";
		else if(!strcmp(json_object_get_string(json_object_object_get(obj, "Service_Mode")),"USA_Flash_Style"))
			serviceMode = "USA Type";
	printf("Region : %s \n",json_object_get_string(json_object_object_get(obj, "Region")));
#ifdef ZYXEL_PHONE_CUSTOMIZED_FXS_CLID_PARAMS_SUPPORT
    printf("Caller ID Signal Type : %s \n", json_object_get_string(json_object_object_get(obj, "FxsCIDSigProtocol")));
    printf("Caller ID Mode : %s \n", json_object_get_string(json_object_object_get(obj, "FxsCIDMode")));
#endif
	printf("Call Service Mode : %s \n",serviceMode);

}


void initRegionGlobalObjects(){
	multiJobj = NULL;
	profObj = NULL;

	return;
}

void getRegionBasicInfo(struct json_object *Jobj)
{

	X_ZYXEL_FlashUsageStyle = json_object_get_string(json_object_object_get(Jobj, "Service_Mode"));

	// transfer param values from DAL format to RDM format
	if (X_ZYXEL_FlashUsageStyle != NULL)
	{
		if (!strcmp(X_ZYXEL_FlashUsageStyle, "Europe_Type"))
		{
			json_object_object_del(Jobj, "Service_Mode");
			json_object_object_add(Jobj, "Service_Mode", json_object_new_string("EUROP_Flash_Style"));
			X_ZYXEL_FlashUsageStyle = json_object_get_string(json_object_object_get(Jobj, "Service_Mode"));
		}
		else if (!strcmp(X_ZYXEL_FlashUsageStyle, "USA_Type"))
		{
			json_object_object_del(Jobj, "Service_Mode");
			json_object_object_add(Jobj, "Service_Mode", json_object_new_string("USA_Flash_Style"));
			X_ZYXEL_FlashUsageStyle = json_object_get_string(json_object_object_get(Jobj, "Service_Mode"));
		}
		else
		{
			dbg_printf("%s:%d: Unknown X_ZYXEL_FlashUsageStyle \"%s\" \n",__FUNCTION__,__LINE__,X_ZYXEL_FlashUsageStyle);
		}
	}

	Region = json_object_get_string(json_object_object_get(Jobj, "Region"));

	return;
}

void freeAllRegionObjects(){
	if(multiJobj) json_object_put(multiJobj);
	if(profObj) json_object_put(profObj);

	return;
}


zcfgRet_t zcfgFeDalVoipFlashAndCountryEdit(struct json_object *Jobj)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *setJson = NULL;

    IID_INIT(Iid);
    while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &Iid, &profObj) == ZCFG_SUCCESS)
    {
        if (json_object_object_get(Jobj, "Service_Mode"))
        {
            json_object_object_add(profObj, "X_ZYXEL_FlashUsageStyle", json_object_new_string(X_ZYXEL_FlashUsageStyle));
        }
        if (json_object_object_get(Jobj, "Region"))
        {
            json_object_object_add(profObj, "Region", json_object_new_string(Region));
        }

        if ((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_PROF, &Iid, multiJobj, profObj)) == NULL)
        {
            printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_PROF);
            zcfgFeJsonObjFree(profObj);
            return ZCFG_INTERNAL_ERROR;
        }

        zcfgFeJsonObjFree(profObj);

        break;
        /* Backend will sync these two configs, so we only need to set the first profile in DAL. */
    }

    //printf("%s(): Append Pro obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
    //printf("%s() exit \n ",__FUNCTION__);
    return ret;
}

zcfgRet_t zcfgFeDalVoipRegionEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	initRegionGlobalObjects();
	getRegionBasicInfo(Jobj);
	multiJobj = zcfgFeJsonObjNew();
    struct json_object *voiceCommonObj = NULL;
    char *callerIDSignalType;
    char *callerIDMode;

	if((ret = zcfgFeDalVoipFlashAndCountryEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s: Edit prof  obj fail \n", __FUNCTION__);
		goto exit;
	}

    IID_INIT(Iid);
    while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_COMMON, &Iid, &voiceCommonObj) == ZCFG_SUCCESS)
    {
        if ((callerIDSignalType = (char *)json_object_get_string(json_object_object_get(Jobj, "FxsCIDSigProtocol"))) != NULL)
        {
            if (!strcmp(callerIDSignalType, "Default"))
            {
                json_object_object_add(voiceCommonObj, "FxsCIDSigProtocol", json_object_new_string(""));
            }
            else
            {
                json_object_object_add(voiceCommonObj, "FxsCIDSigProtocol", json_object_new_string(callerIDSignalType));
            }
        }

        if ((callerIDMode = (char *)json_object_get_string(json_object_object_get(Jobj, "FxsCIDMode"))) != NULL)
        {
            if (!strcmp(callerIDMode, "Default"))
            {
                json_object_object_add(voiceCommonObj, "FxsCIDMode", json_object_new_string(""));
            }
            else
            {
                json_object_object_add(voiceCommonObj, "FxsCIDMode", json_object_new_string(callerIDMode));
            }
        }

        if (zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_COMMON, &Iid, multiJobj, voiceCommonObj) == NULL)
        {
            printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_COMMON);
            zcfgFeJsonObjFree(voiceCommonObj);
            goto exit;
        }

        zcfgFeJsonObjFree(voiceCommonObj);
    }

	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d\n", __FUNCTION__, RDM_OID_VOICE_PROF);
		goto exit;
	}

exit:
	freeAllRegionObjects();
	return ret;
}

zcfgRet_t zcfgFeDalVoipRegionCapGet(struct json_object *paramJobj)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *voicecapobj = NULL;
    objIndex_t voicecapobjIid;

    IID_INIT(voicecapobjIid);
	voicecapobjIid.level = 1;
	voicecapobjIid.idx[0] = 1;

    if ((ret = zcfgFeObjJsonGet(RDM_OID_VOICE_CAPB, &voicecapobjIid, &voicecapobj)) == ZCFG_SUCCESS)
    {
        if (paramJobj != NULL)
        {
            json_object_object_add(paramJobj, "Region_Cap", JSON_OBJ_COPY(json_object_object_get(voicecapobj, "Regions")));
        }
        zcfgFeJsonObjFree(voicecapobj);
    }

    return ret;
}

zcfgRet_t zcfgFeDalVoipRegionGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *voiceregionJobj = NULL;
    struct json_object *voiceCommonObj = NULL;
    char *callerIDSignalType;
    char *callerIDMode;

    paramJobj = json_object_new_object();

    IID_INIT(Iid);
    while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &Iid, &voiceregionJobj) == ZCFG_SUCCESS)
    {
        json_object_object_add(paramJobj, "Service_Mode", JSON_OBJ_COPY(json_object_object_get(voiceregionJobj, "X_ZYXEL_FlashUsageStyle")));
        json_object_object_add(paramJobj, "Region", JSON_OBJ_COPY(json_object_object_get(voiceregionJobj, "Region")));
        zcfgFeJsonObjFree(voiceregionJobj);
        break;
    }

    IID_INIT(Iid);
    while (zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_COMMON, &Iid, &voiceCommonObj) == ZCFG_SUCCESS)
    {
        if ((callerIDSignalType = (char *)json_object_get_string(json_object_object_get(voiceCommonObj, "FxsCIDSigProtocol"))) != NULL)
        {
            if (callerIDSignalType[0] == '\0')
            {
                json_object_object_add(paramJobj, "FxsCIDSigProtocol", json_object_new_string("Default"));
            }
            else
            {
                json_object_object_add(paramJobj, "FxsCIDSigProtocol", json_object_new_string(callerIDSignalType));
            }
        }

        if ((callerIDMode = (char *)json_object_get_string(json_object_object_get(voiceCommonObj, "FxsCIDMode"))) != NULL)
        {
            if (callerIDMode[0] == '\0')
            {
                json_object_object_add(paramJobj, "FxsCIDMode", json_object_new_string("Default"));
            }
            else
            {
                json_object_object_add(paramJobj, "FxsCIDMode", json_object_new_string(callerIDMode));
            }
        }

        zcfgFeJsonObjFree(voiceCommonObj);
    }

    ret = zcfgFeDalVoipRegionCapGet(paramJobj);
    json_object_array_add(Jarray, paramJobj);

	return ret;
}


zcfgRet_t zcfgFeDalVoipPhoneRegion(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVoipRegionEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDalVoipRegionGet(Jobj, Jarray, NULL);
	}

	return ret;
}

