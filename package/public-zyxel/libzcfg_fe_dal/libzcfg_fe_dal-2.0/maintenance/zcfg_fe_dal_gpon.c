#include <ctype.h>
#include <json/json.h>
#include <time.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t GPON_param[] =
{
	{"SLID", 		dalType_string,	 0, 	74, 	NULL,	NULL,	0},
	{"OnuOperationMode", 	dalType_string,	 0, 	0, 		NULL,	"AutoDetect|SFU|HGU|Hybrid",	0},
	{NULL, 					0, 				 0, 	0, 	NULL}
};

//copy from mos common.h
typedef enum
{
	MOS_ONU_OPERATION_MODE_MIN,
	MOS_ONU_OPERATION_MODE_AUTO_DETECT=MOS_ONU_OPERATION_MODE_MIN,
	MOS_ONU_OPERATION_MODE_BRIDGE,
	MOS_ONU_OPERATION_MODE_RG,
	MOS_ONU_OPERATION_MODE_HYBRID,
	MOS_ONU_OPERATION_MODE_DEV_MODE,
	MOS_ONU_OPERATION_MODE_MAX=MOS_ONU_OPERATION_MODE_DEV_MODE
} mosOnuOperationMode;

zcfgRet_t zcfgFeDal_Gpon_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	const char *SLID = NULL;
	const char *OnuOperationMode_str = NULL;
	uint32_t OnuOperationMode_int = 0;
	bool needReboot = false;

	SLID = Jgets(Jobj, "SLID");
	OnuOperationMode_str = Jgets(Jobj, "OnuOperationMode");
	
	IID_INIT(iid);
#ifdef ECONET_PLATFORM
	if( SLID!= NULL && (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ONU, &iid, &obj)) == ZCFG_SUCCESS){
		Jadds(obj, "Password", SLID);

		ret = zcfgFeObjJsonSet(RDM_OID_ONU, &iid, obj, NULL);
		zcfgFeJsonObjFree(obj);
	}
#else
	if( SLID!= NULL && (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_INFO, &iid, &obj)) == ZCFG_SUCCESS){
		Jadds(obj, "X_ZYXEL_Ploam_Password", SLID);

		ret = zcfgFeObjJsonSet(RDM_OID_DEV_INFO, &iid, obj, NULL);
		zcfgFeJsonObjFree(obj);
	}
#endif

	IID_INIT(iid);
	if( OnuOperationMode_str != NULL && (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MOS_USR_CFG, &iid, &obj)) == ZCFG_SUCCESS){
		if(!strcmp(OnuOperationMode_str, "AutoDetect"))
			OnuOperationMode_int = MOS_ONU_OPERATION_MODE_AUTO_DETECT;
		else if(!strcmp(OnuOperationMode_str, "SFU"))
			OnuOperationMode_int = MOS_ONU_OPERATION_MODE_BRIDGE;
		else if(!strcmp(OnuOperationMode_str, "HGU"))
			OnuOperationMode_int = MOS_ONU_OPERATION_MODE_RG;
		else if(!strcmp(OnuOperationMode_str, "Hybrid"))
			OnuOperationMode_int = MOS_ONU_OPERATION_MODE_HYBRID;

		if(OnuOperationMode_int != Jgeti(obj, "OnuOperationMode")){
			Jaddi(obj, "OnuOperationMode", OnuOperationMode_int);
			ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_MOS_USR_CFG, &iid, obj, NULL);//neet reboot to change ONU mode
			needReboot = true;
		}
		zcfgFeJsonObjFree(obj);
	}

	if(needReboot)
		zcfgFeReqReboot(NULL);
	
	return ret;
}

zcfgRet_t zcfgFeDal_Gpon_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	uint32_t OnuOperationMode = 0;
	
	IID_INIT(iid);
		paramJobj = json_object_new_object();
	if(paramJobj == NULL)
	{
		printf("%s: paramJobj is null\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
#ifdef ECONET_PLATFORM
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ONU, &iid, &obj) == ZCFG_SUCCESS){
		replaceParam(paramJobj, "SLID", obj, "Password");
		zcfgFeJsonObjFree(obj);
	}
#else
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_INFO, &iid, &obj) == ZCFG_SUCCESS){
		replaceParam(paramJobj, "SLID", obj, "X_ZYXEL_Ploam_Password");
		zcfgFeJsonObjFree(obj);
	}
#endif

	IID_INIT(iid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MOS_USR_CFG, &iid, &obj) == ZCFG_SUCCESS){
		OnuOperationMode = Jgeti(obj, "OnuOperationMode");
		
		switch(OnuOperationMode){
			case MOS_ONU_OPERATION_MODE_AUTO_DETECT:
				Jadds(paramJobj, "OnuOperationMode", "AutoDetect");
				break;
			case MOS_ONU_OPERATION_MODE_BRIDGE:
				Jadds(paramJobj, "OnuOperationMode", "SFU");
				break;
			case MOS_ONU_OPERATION_MODE_RG:
				Jadds(paramJobj, "OnuOperationMode", "HGU");
				break;
			case MOS_ONU_OPERATION_MODE_HYBRID:
				Jadds(paramJobj, "OnuOperationMode", "Hybrid");
				break;
			default:
				Jadds(paramJobj, "OnuOperationMode", "AutoDetect");
				break;
		}
		zcfgFeJsonObjFree(obj);
	}
	json_object_array_add(Jarray, paramJobj);
	
	return ret;
}

void zcfgFeDalShowGpon(struct json_object *Jarray){
	struct json_object *obj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("%-30s 0x%-10s\n","SLID:",Jgets(obj, "SLID"));
	printf("%-30s %-10s\n","OnuOperationMode:",Jgets(obj, "OnuOperationMode"));
}



zcfgRet_t zcfgFeDalGpon(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Gpon_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Gpon_Get(Jobj, Jarray, replyMsg);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}

