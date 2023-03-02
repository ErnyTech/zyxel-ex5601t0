
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zcfg_fe_dal_voip.h"

#include "zlog_api.h"

dal_param_t VOIP_TLS_param[]={
    {"TLSPort",                                 dalType_int   , 1,  65535,  NULL, NULL},
    {"TLSCertificate",                            dalType_string,0,   0,  NULL, NULL},
    {"TLSCipherSuite",                            dalType_string,0,   0,  NULL, NULL},
    {"TLSCertVerifyMode",                            dalType_string,0,   0,  NULL, NULL},
    {NULL,                                      0,              0,  0,  NULL},
};


void zcfgFeDalShowVoipTLS(struct json_object *Jarray){

}


zcfgRet_t zcfgFeDalVoipTLSGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *voice_common_obj = NULL;
    struct json_object *voice_tls_obj = NULL;
    struct json_object *tmp_obj = NULL;
    objIndex_t objIid;


    voice_tls_obj = json_object_new_object();

    if(voice_tls_obj == NULL)
    {
        ZLOG_ERROR("%s():%d <<Error>> Create new Obj fail\n", __FUNCTION__, __LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    IID_INIT(objIid);
    if((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_COMMON, &objIid, &voice_common_obj)) != ZCFG_SUCCESS) {
        ZLOG_ERROR("%s():%d <<Error>> Get Voice Common Obj fail\n", __FUNCTION__, __LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    tmp_obj = json_object_object_get(voice_common_obj, "TLS_Port");
    if(tmp_obj != NULL)
    {
        json_object_object_add(voice_tls_obj, "TLS_Port", JSON_OBJ_COPY(tmp_obj));;
    }

    tmp_obj = json_object_object_get(voice_common_obj, "TLS_Certificate");
    if(tmp_obj != NULL)
    {
        json_object_object_add(voice_tls_obj, "TLS_Certificate", JSON_OBJ_COPY(tmp_obj));;
    }

    tmp_obj = json_object_object_get(voice_common_obj, "TLS_CipherSuite");
    if(tmp_obj != NULL)
    {
        json_object_object_add(voice_tls_obj, "TLS_CipherSuite", JSON_OBJ_COPY(tmp_obj));;
    }

    tmp_obj = json_object_object_get(voice_common_obj, "TLS_CertVerifyMode");
    if(tmp_obj != NULL)
    {
        json_object_object_add(voice_tls_obj, "TLS_CertVerifyMode", JSON_OBJ_COPY(tmp_obj));;
    }


    json_object_array_add(Jarray, voice_tls_obj);

    if(voice_common_obj)
    {
        json_object_put(voice_common_obj);
    }

    return ret;
}

zcfgRet_t zcfgFeDalVoipTLSEdit(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    objIndex_t objIid;
    struct json_object *voice_common_obj = NULL;
    struct json_object *multiJobj = zcfgFeJsonObjNew();

    if(multiJobj == NULL)
    {
        ZLOG_ERROR("%s():%d <<Error>> Multi Obj create fail\n", __FUNCTION__, __LINE__);
        goto Exit;
    }

    IID_INIT(objIid);
    if((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_COMMON, &objIid, &voice_common_obj)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("%s():%d <<Error>> Get Voice Common Obj fail\n", __FUNCTION__, __LINE__);
        goto Exit;
    }

    //Apply config object to set object
    json_object_object_foreach(Jobj, key, val)
    {
        json_object_object_add(voice_common_obj, key, JSON_OBJ_COPY(val));
    }

    if((zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_COMMON, &objIid, multiJobj, voice_common_obj)) == NULL){
        ZLOG_ERROR("%s:<<Error>> fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE);
        goto Exit;
    }


    if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
        ZLOG_ERROR("%s:(ERROR) fail to set multiJobj \n", __FUNCTION__);
        goto Exit;
    }

Exit:
    zcfgFeJsonObjFree(multiJobj);
    zcfgFeJsonObjFree(voice_common_obj);


    return ret;
}

zcfgRet_t zcfgFeDalVoipTLS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if(!method || !Jobj)
        return ZCFG_INTERNAL_ERROR;

    if(!strcmp(method, "PUT")) {
        ret = zcfgFeDalVoipTLSEdit(Jobj, NULL);
    }
    else if(!strcmp(method, "GET")) {
        ret = zcfgFeDalVoipTLSGet(Jobj, Jarray, NULL);
    }

    return ret;
}

