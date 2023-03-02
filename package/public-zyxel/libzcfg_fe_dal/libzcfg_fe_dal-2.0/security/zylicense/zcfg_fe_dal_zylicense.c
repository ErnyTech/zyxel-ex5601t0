#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t ZyLicense_param[]={
	{"license_enable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{"license_apply",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{"FSC_enable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	{NULL,		0,	0,	0,	NULL}
};

struct json_object *zyLicenseObj = NULL;
objIndex_t zyLicenseIid;
bool zyLicenseEnable;
bool zyLicenseApply;
bool FSCEnable;

zcfgRet_t freeAllzyLicenseObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(zyLicenseObj) json_object_put(zyLicenseObj);
	return ret;
}

void getBasicZyLicenseInfo(struct json_object *Jobj){

	if((json_object_object_get(Jobj, "license_enable")) != NULL)
	{
		zyLicenseEnable = json_object_get_boolean(json_object_object_get(Jobj, "license_enable"));
		printf("%s: zyLicenseEnable = %d\n",__FUNCTION__,zyLicenseEnable );
	}
	else
	{ 
		if((zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LICNSE, &zyLicenseIid, &zyLicenseObj)) == ZCFG_SUCCESS)
		{
			zyLicenseEnable = Jgetb(zyLicenseObj, "Cyber_Security_FSC");
		}
	}
	
	if((json_object_object_get(Jobj, "license_apply")) != NULL)
	{
		zyLicenseApply = json_object_get_boolean(json_object_object_get(Jobj, "license_apply"));
		printf("%s: zyLicenseApply = %d\n",__FUNCTION__,zyLicenseApply );
	}
	else
	{
		if((zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LICNSE, &zyLicenseIid, &zyLicenseObj)) == ZCFG_SUCCESS)
		{
			zyLicenseApply = Jgetb(zyLicenseObj, "FSC_License_Apply");
		}
	}

	if((json_object_object_get(Jobj, "FSC_enable")) != NULL)
	{
		FSCEnable = json_object_get_boolean(json_object_object_get(Jobj, "FSC_enable"));
		printf("%s: FSCEnable = %d\n",__FUNCTION__,FSCEnable );
	}
	else
	{
		if((zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LICNSE, &zyLicenseIid, &zyLicenseObj)) == ZCFG_SUCCESS)
		{
			FSCEnable = Jgetb(zyLicenseObj, "FSC_Enable");
		}
	}
	printf("%s: %s\n",__FUNCTION__,json_object_get_string(Jobj) );
	return;
}

void initGlobalzyLicenseObjects(){
	zyLicenseObj = NULL;
	IID_INIT(zyLicenseIid);

}

zcfgRet_t editzyLicenseObject(struct json_object *Jobj){

	zcfgRet_t ret = ZCFG_SUCCESS;
	bool value;
	bool apply;
	bool enable;
	bool changed = 0;

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LICNSE, &zyLicenseIid, &zyLicenseObj)) != ZCFG_SUCCESS) {
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_zylicense_err"));
		return ret;
	}
	value = Jgetb(zyLicenseObj, "Cyber_Security_FSC");
	apply = Jgetb(zyLicenseObj, "FSC_License_Apply");
	enable = Jgetb(zyLicenseObj, "FSC_Enable");
	printf("%s: value = %d, %d\n",__FUNCTION__,value,zyLicenseEnable);
	printf("%s: apply = %d, %d\n",__FUNCTION__,apply,zyLicenseApply);
	printf("%s: enable = %d, %d\n",__FUNCTION__,enable,FSCEnable);
	if ( value != zyLicenseEnable ) {
		printf("%s: License Change from %d to %d\n",__FUNCTION__,value,zyLicenseEnable);
		json_object_object_add(zyLicenseObj, "Cyber_Security_FSC", json_object_new_boolean(zyLicenseEnable));
		changed = 1;
	}
	if ( apply != zyLicenseApply ) {
		printf("%s: Apply Change from %d to %d\n",__FUNCTION__,apply,zyLicenseApply);
		json_object_object_add(zyLicenseObj, "FSC_License_Apply", json_object_new_boolean(zyLicenseApply));	
		changed = 1;
	}
	if ( enable != FSCEnable ) {
		printf("%s: enable Change from %d to %d\n",__FUNCTION__,enable,FSCEnable);
		json_object_object_add(zyLicenseObj, "FSC_Enable", json_object_new_boolean(FSCEnable));
		changed = 1;
	}
	if(changed)
	{ 
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_LICNSE, &zyLicenseIid, zyLicenseObj, NULL)) != ZCFG_SUCCESS){
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.set_zylicense_err"));
			return ret;
		}
	}
	return ret;
}

void zcfgFeDalShowZyLicense(struct json_object *Jarray){
	struct json_object *Jobj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	if((Jobj = json_object_array_get_idx(Jarray, 0)) == NULL)
		return;

	printf("Secure License %s\n", Jgetb(Jobj, "license_enable")?"Enabled":"Disabled");
	printf("FSC apply %s\n", Jgetb(Jobj, "license_apply")?"Enabled":"Disabled");
	printf("FSC Enable %s\n", Jgetb(Jobj, "FSC_enable")?"Enabled":"Disabled");
}

zcfgRet_t zcfgFeDalZyLicenseGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;

	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;

	IID_INIT(objIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LICNSE, &objIid, &obj) == ZCFG_SUCCESS){
		pramJobj = json_object_new_object();
		replaceParam(pramJobj, "license_enable", obj, "Cyber_Security_FSC");
		replaceParam(pramJobj, "license_apply", obj, "FSC_License_Apply");
		replaceParam(pramJobj, "FSC_enable", obj, "FSC_Enable");
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}

zcfgRet_t zcfgFeDalZyLicenseEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	initGlobalzyLicenseObjects();
	getBasicZyLicenseInfo(Jobj);
	ret = editzyLicenseObject(Jobj);
	freeAllzyLicenseObjects();
	return ret;
}

zcfgRet_t zcfgFeDalZyLicense(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalZyLicenseEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalZyLicenseGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);
	return ret;
}
