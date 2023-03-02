
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t DEV_SEC_CERT_param[] =
{
	//DevSecCert
	{"Index", 		dalType_int, 		0, 	0, 	NULL},
	{"Enable",		dalType_boolean,	0,	0,	NULL},
	{"X_ZYXEL_Name",		dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_Type",		dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_Certificate",		dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_PrivateKey",		dalType_string,	0,	0,	NULL},	
	{"X_ZYXEL_SigningRequest",		dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_CommonName",		dalType_string,	0,	0,	NULL},	
	{"X_ZYXEL_OrgName",		dalType_string,	0,	0,	NULL},
	{"X_ZYXEL_StateName",		dalType_string,	0,	0,	NULL},	
	{"X_ZYXEL_CountryName",		dalType_string,	0,	0,	NULL},
	{"LastModif",		dalType_string,	0,	0,	NULL},	
	{"SerialNumber",		dalType_string,	0,	0,	NULL},
	{"Issuer",		dalType_string,	0,	0,	NULL},
	{"NotBefore",		dalType_string,	0,	0,	NULL},
	{"NotAfter",		dalType_string,	0,	0,	NULL},	
	{"Subject",		dalType_string,	0,	0,	NULL},
	{"SubjectAlt",		dalType_string,	0,	0,	NULL},
	{"SignatureAlgorithm",		dalType_string,	0,	0,	NULL},		
	{NULL,		0,	0,	0,	NULL}
};

bool isCertNameDuplicate(const char *name, objIndex_t *objIid)
{
	objIndex_t certIid = {0};
	struct json_object *certJobj = NULL;

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DEV_SEC_CERT, &certIid, &certJobj) == ZCFG_SUCCESS) {
		if((objIid == NULL || objIid->idx[0] != certIid.idx[0])
			&& strcmp(name, json_object_get_string(json_object_object_get(certJobj, "X_ZYXEL_Name"))) == 0) {
			zcfgFeJsonObjFree(certJobj);
			return true;
		}

		zcfgFeJsonObjFree(certJobj);
	}

	return false;
}

zcfgRet_t zcfgFeDalDevSecCertAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *DevSecCertJobj = NULL;
	int n=0;

	paraName = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_Name"));
	if(paraName == NULL) {
		if(replyMsg != NULL)
			strcpy(replyMsg, "missing parameters");
		return ZCFG_REQUEST_REJECT;
	}
	else if(isCertNameDuplicate(paraName, NULL)) {
		if(replyMsg != NULL)
			strcpy(replyMsg, "duplicate name");
		Jadds(Jobj, "__multi_lang_replyMsg", "zylang.Certificates.Local.Warning.same_name");
		return ZCFG_REQUEST_REJECT;
	}

	IID_INIT(objIid);
	zcfgFeObjJsonAdd(RDM_OID_DEV_SEC_CERT, &objIid, NULL);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_SEC_CERT, &objIid, &DevSecCertJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}

	paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
	while(paraName){
		struct json_object *paramValue = json_object_object_get(Jobj, paraName);
		if(!paramValue) {
			printf("function:%s, Line =%d\n", __FUNCTION__,__LINE__);
			paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
			continue;
		}
		json_object_object_add(DevSecCertJobj, paraName, JSON_OBJ_COPY(paramValue));
		paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
	}

	if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_DEV_SEC_CERT, &objIid, DevSecCertJobj, NULL)) != ZCFG_SUCCESS)
	{
		return ret;
	}
	
	json_object_put(DevSecCertJobj);
	return ret;
}
zcfgRet_t zcfgFeDalDevSecCertEdit(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *DevSecCertJobj = NULL;
	int paramModified = 0, n = 0;
	struct json_object *iidArray = NULL;
	uint8_t iidIdx =0;
	
	
	//IID_INIT(objIid);
	iidArray = json_object_object_get(Jobj, "Iid");
	for(iidIdx=0;iidIdx<6;iidIdx++){
		objIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(iidArray, iidIdx));
		if(objIid.idx[iidIdx] != 0)
			objIid.level = iidIdx+1;
	}

	paraName = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_Name"));
	if(paraName == NULL) {
		if(replyMsg != NULL)
			strcpy(replyMsg, "missing parameters");
		return ZCFG_REQUEST_REJECT;
	}
	else if(isCertNameDuplicate(paraName, &objIid)) {
		if(replyMsg != NULL)
			strcpy(replyMsg, "duplicate name");
		Jadds(Jobj, "__multi_lang_replyMsg", "zylang.Certificates.Local.Warning.same_name");
		return ZCFG_REQUEST_REJECT;
	}

	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_SEC_CERT, &objIid, &DevSecCertJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
	while(paraName){
		struct json_object *paramValue = json_object_object_get(Jobj, paraName);
		if(!paramValue) {
			paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
			continue;
		}
		json_object_object_add(DevSecCertJobj, paraName, JSON_OBJ_COPY(paramValue));
		paramModified = 1;				
		paraName = (const char *)DEV_SEC_CERT_param[n++].paraName;
	}

	if(paramModified) {
		if((ret = zcfgFeObjJsonSet(RDM_OID_DEV_SEC_CERT, &objIid, DevSecCertJobj, NULL)) != ZCFG_SUCCESS);
	}
	json_object_put(DevSecCertJobj);
	return ret;
}
zcfgRet_t zcfgFeDalDevSecCertDelete(struct json_object *Jobj, char *replyMsg)
{
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *iidArray = NULL;

	IID_INIT(objIid);
	if( (iidArray = json_object_object_get(Jobj, "Index")) != NULL ) {
		objIid.level = 1;
		objIid.idx[0] = json_object_get_int(iidArray);
		json_object_object_del(Jobj, "Index");
	}
	else {
		if(replyMsg != NULL){
			strcat(replyMsg, "internal error");
		}
		return ZCFG_INTERNAL_ERROR;
	}
	
	zcfgFeObjJsonDel(RDM_OID_DEV_SEC_CERT, &objIid, NULL);
	
	return ret;
	
}
zcfgRet_t zcfgFeDalDevSecCertGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *DevSecCertJobj = NULL;
	objIndex_t objIid = {0};
	int index = 0;
	if(json_object_object_get(Jobj, "Index")){
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));	
		objIid.level = 1;
		objIid.idx[0] = index;

		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_DEV_SEC_CERT, &objIid , &DevSecCertJobj) == ZCFG_SUCCESS){
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Index", json_object_new_int(objIid.idx[0]));
			json_object_object_add(paramJobj, "X_ZYXEL_Name", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Name")));
			json_object_object_add(paramJobj, "X_ZYXEL_Type", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Type")));
			json_object_object_add(paramJobj, "X_ZYXEL_Certificate", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Certificate")));
			json_object_object_add(paramJobj, "X_ZYXEL_PrivateKey", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_PrivateKey")));
			json_object_object_add(paramJobj, "X_ZYXEL_SigningRequest", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_SigningRequest")));
			json_object_object_add(paramJobj, "Issuer", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "Issuer")));
			json_object_object_add(paramJobj, "Subject", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "Subject")));
			json_object_object_add(paramJobj, "NotBefore", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "NotBefore")));
			json_object_object_add(paramJobj, "NotAfter", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "NotAfter")));
			json_object_array_add(Jarray, paramJobj);
		
			zcfgFeJsonObjFree(DevSecCertJobj);
		}
		
	}
	else{
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DEV_SEC_CERT, &objIid, &DevSecCertJobj) == ZCFG_SUCCESS){
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Index", json_object_new_int(objIid.idx[0]));
			json_object_object_add(paramJobj, "X_ZYXEL_Name", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Name")));
			json_object_object_add(paramJobj, "X_ZYXEL_Type", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Type")));
			json_object_object_add(paramJobj, "X_ZYXEL_Certificate", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_Certificate")));
			json_object_object_add(paramJobj, "X_ZYXEL_PrivateKey", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_PrivateKey")));
			json_object_object_add(paramJobj, "X_ZYXEL_SigningRequest", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "X_ZYXEL_SigningRequest")));
			json_object_object_add(paramJobj, "Issuer", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "Issuer")));
			json_object_object_add(paramJobj, "Subject", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "Subject")));
			json_object_object_add(paramJobj, "NotBefore", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "NotBefore")));
			json_object_object_add(paramJobj, "NotAfter", JSON_OBJ_COPY(json_object_object_get(DevSecCertJobj, "NotAfter")));
			json_object_array_add(Jarray, paramJobj);
		
			zcfgFeJsonObjFree(DevSecCertJobj);
		}	
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalDevSecCert(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalDevSecCertEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalDevSecCertAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalDevSecCertDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalDevSecCertGet(Jobj, Jarray, replyMsg);
	}

	return ret;
}
