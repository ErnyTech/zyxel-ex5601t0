#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#ifdef ZYXEL_EASYMESH
dal_param_t WIFI_EASYMESH_param[] =
{
	{"EasyMeshEnable",		dalType_boolean,	0,	0,	NULL, 	NULL, 		0},
	{"Controller_mode",		dalType_int,		0,	0,	NULL, 	"0|1|2", 	0},
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
	{"BH_band",				dalType_int,		0,	0,	NULL, 	"0|1", 		0},
#endif
	{"CurRole",				dalType_int,		0,	0,	NULL, 	"1|2", 	dalcmd_Forbid},
	{NULL,		0,	0,	0,	NULL,	NULL, 0}
};

void zcfgFeDalShowWifiEasyMesh(struct json_object *Jarray){
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("%-20s %-10s \n", "EasyMesh Setup:", json_object_get_string(json_object_object_get(obj, "EasyMeshEnable")));
}

zcfgRet_t zcfgFeDalWifiEasyMeshGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t easymeshIid={0};
	struct json_object *easymeshObj = NULL;
	struct json_object *pramJobj = NULL;

	IID_INIT(easymeshIid);
	if(zcfgFeObjJsonGet(RDM_OID_EASY_MESH, &easymeshIid, &easymeshObj) != ZCFG_SUCCESS){
		return ret;
	}

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "EasyMeshEnable", JSON_OBJ_COPY(json_object_object_get(easymeshObj, "Enable")));
	json_object_object_add(pramJobj, "Controller_mode", JSON_OBJ_COPY(json_object_object_get(easymeshObj, "Controller")));
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
	json_object_object_add(pramJobj, "BH_band", JSON_OBJ_COPY(json_object_object_get(easymeshObj, "Band")));
#endif
	json_object_object_add(pramJobj, "CurRole", JSON_OBJ_COPY(json_object_object_get(easymeshObj, "Role")));
	json_object_array_add(Jarray, pramJobj);

	zcfgFeJsonObjFree(easymeshObj);

	return ret;
}

zcfgRet_t zcfgFeDalWifiEasyMeshEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	int EasyMeshEnable = false, EasyMeshEnable_ori = false;
	int Controller = 0, Controller_ori = 0;
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
	int Band = 0, Band_ori = 0;
#endif

	EasyMeshEnable = json_object_get_int(json_object_object_get(Jobj, "EasyMeshEnable"));
	Controller = json_object_get_int(json_object_object_get(Jobj, "Controller_mode"));
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
	Band = json_object_get_int(json_object_object_get(Jobj, "BH_band"));
#endif
	struct json_object *EasyMeshObj = NULL;
	objIndex_t EasyMeshIid;

	IID_INIT(EasyMeshIid);
	if(zcfgFeObjJsonGet(RDM_OID_EASY_MESH, &EasyMeshIid, &EasyMeshObj) == ZCFG_SUCCESS){
		EasyMeshEnable_ori = json_object_get_int(json_object_object_get(EasyMeshObj, "Enable"));
		Controller_ori = json_object_get_int(json_object_object_get(EasyMeshObj, "Controller"));
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
		Band_ori = json_object_get_int(json_object_object_get(EasyMeshObj, "Band"));
#endif
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")), "GUI")){
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
			if(EasyMeshEnable != EasyMeshEnable_ori || Controller != Controller_ori || Band != Band_ori)
#else
			if(EasyMeshEnable != EasyMeshEnable_ori || Controller != Controller_ori)
#endif
			{
				json_object_object_add(EasyMeshObj, "Enable", json_object_new_boolean(EasyMeshEnable));
				json_object_object_add(EasyMeshObj, "Controller", json_object_new_int(Controller));
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
				json_object_object_add(EasyMeshObj, "Band", json_object_new_int(Band));
#endif
				zcfgFeObjJsonBlockedSet(RDM_OID_EASY_MESH, &EasyMeshIid, EasyMeshObj, NULL);
			}
		}
		zcfgFeJsonObjFree(EasyMeshObj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalWifiEasyMesh(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj){
		return ZCFG_INTERNAL_ERROR;
	}

	if(!strcmp(method, "GET"))
		ret = zcfgFeDalWifiEasyMeshGet(Jobj, Jarray, replyMsg);
	else if(!strcmp(method, "PUT"))
		ret = zcfgFeDalWifiEasyMeshEdit(Jobj, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
