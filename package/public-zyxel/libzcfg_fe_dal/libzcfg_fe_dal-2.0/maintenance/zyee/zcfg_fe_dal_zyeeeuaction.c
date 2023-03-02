#include <json/json.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zlog_api.h"
#include "zcfg_system.h"
#include "zos_api.h"

dal_param_t ZYEE_EU_param[]={
    {"EUName",                              dalType_string, 0,  0,    NULL},
    {"RequestedState",                      dalType_string, 0,  0,    NULL},
    {"CmdLineStart",                        dalType_string, 0,  0,    NULL},
    {"CmdLineStop",                         dalType_string, 0,  0,    NULL},
    {NULL,                                  0,              0,  0,    NULL},
};

zcfgRet_t zcfgFeDalEUActionSet(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t                   ret = ZCFG_SUCCESS;
    struct                      json_object *swModuleEUObj = NULL;
    struct                      json_object *swModuleEUcmdObj = NULL;
    struct                      json_object *reqObject = NULL;
    const char                  *eeName;
    const char                  *CmdLineStart;
    const char                  *CmdLineStop;
    const char                  *RequestedState;
    int                         index  = 0;
    int                         len = 0;
    objIndex_t                  euobjIid;

    len = json_object_array_length(Jobj);

    for (int reqObjIdx = 0; reqObjIdx < len; ++reqObjIdx)
    {
        reqObject = json_object_array_get_idx(Jobj, reqObjIdx);
        IID_INIT(euobjIid);
        index = json_object_get_int(json_object_object_get(reqObject, "Index"));
        RequestedState = json_object_get_string(json_object_object_get(reqObject, "RequestedState"));

        euobjIid.idx[0] = index;
		euobjIid.level = 1;

        if(zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT, &euobjIid, &swModuleEUObj) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to Get RDM_OID_SW_MODULE_EXEC_UNIT\n");
            zcfgFeJsonObjFree(swModuleEUObj);
            return ZCFG_INTERNAL_ERROR;
        }
        if(zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS, &euobjIid, &swModuleEUcmdObj) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to Get RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS\n");
            zcfgFeJsonObjFree(swModuleEUcmdObj);
            return ZCFG_INTERNAL_ERROR;
        }

        if (RequestedState != NULL)
        {
            if (strcmp(RequestedState, "Active") == 0)
            {
                json_object_object_add(swModuleEUObj, "RequestedState", json_object_new_string(RequestedState));
                if (NULL != json_object_object_get(reqObject, "CmdLineStart"))
                {
                    CmdLineStart = json_object_get_string(json_object_object_get(reqObject, "CmdLineStart"));
                    json_object_object_add(swModuleEUcmdObj, "CmdLineStart", json_object_new_string(CmdLineStart));
                }
            }
            if (strcmp(RequestedState, "Idle") == 0)
            {
                json_object_object_add(swModuleEUObj, "RequestedState", json_object_new_string(RequestedState));
                if (NULL != json_object_object_get(reqObject, "CmdLineStop"))
                {
                    CmdLineStop = json_object_get_string(json_object_object_get(reqObject, "CmdLineStop"));
                    json_object_object_add(swModuleEUcmdObj, "CmdLineStop", json_object_new_string(CmdLineStop));
                }
            }
        }

        if(zcfgFeObjJsonSet(RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS, &euobjIid, swModuleEUcmdObj, NULL) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to set RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS\n");
            zcfgFeJsonObjFree(swModuleEUcmdObj);
            return ZCFG_INTERNAL_ERROR;
        }
        if(zcfgFeObjJsonSet(RDM_OID_SW_MODULE_EXEC_UNIT, &euobjIid, swModuleEUObj, NULL) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to set RDM_OID_SW_MODULE_EXEC_UNIT\n");
            zcfgFeJsonObjFree(swModuleEUObj);
            return ZCFG_INTERNAL_ERROR;
        }
    }
    zcfgFeJsonObjFree(swModuleEUObj);
    zcfgFeJsonObjFree(swModuleEUcmdObj);
    return ZCFG_SUCCESS;
}
zcfgRet_t zcfgFeDalEUActionGet(struct json_object *Jobj, struct json_object *Jarray)
{
    zcfgRet_t                               ret = ZCFG_SUCCESS;
    rdm_SwModuleExecUnit_t                  *swModuleEUObj = NULL;
    rdm_SwModuleExecUnitExtensions_t        *swModuleEUcmdObj = NULL;
    objIndex_t                              euobjIid;
    struct json_object                      *tmpObj = NULL;
    struct json_object                      *eeInfoObj = NULL;
    struct json_object                      *duInfoObj = NULL;
    struct json_object                      *euInfoObj = NULL;

    euInfoObj = json_object_new_array();

    /*get EU object*/
    IID_INIT(euobjIid);
    while(zcfgFeObjJsonGetNext(RDM_OID_SW_MODULE_EXEC_UNIT, &euobjIid, (void **)&swModuleEUObj) == ZCFG_SUCCESS)
    {
        tmpObj = json_object_new_object();
        json_object_object_add(tmpObj, "Index", json_object_new_int(euobjIid.idx[0]));
        json_object_object_add(tmpObj, "EUName", JSON_OBJ_COPY(json_object_object_get(swModuleEUObj, "Name")));
        json_object_object_add(tmpObj, "ExecEnvLabel", JSON_OBJ_COPY(json_object_object_get(swModuleEUObj, "ExecEnvLabel")));
        json_object_object_add(tmpObj, "RequestedState", JSON_OBJ_COPY(json_object_object_get(swModuleEUObj, "RequestedState")));

        /*get EU option cmdLine object*/
        if(zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS, &euobjIid, (void **)&swModuleEUcmdObj) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to set RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS\n");
            zcfgFeJsonObjFree(swModuleEUcmdObj);
            return ZCFG_INTERNAL_ERROR;
        }

        json_object_object_add(tmpObj, "CmdLineStart", JSON_OBJ_COPY(json_object_object_get(swModuleEUcmdObj, "CmdLineStart")));
        json_object_object_add(tmpObj, "CmdLineStop", JSON_OBJ_COPY(json_object_object_get(swModuleEUcmdObj, "CmdLineStop")));
        zcfgFeJsonObjFree(swModuleEUObj);
        zcfgFeJsonObjFree(swModuleEUcmdObj);
        json_object_array_add(Jarray, tmpObj);
    }

    return ret;
}

zcfgRet_t zcfgFeDalEUAction(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (!method || !Jobj)
    {
        return ZCFG_INTERNAL_ERROR;
    }
    if (!strcmp(method, "PUT"))
    {
        ret = zcfgFeDalEUActionSet(Jobj, NULL);
    }
    else if (!strcmp(method, "GET"))
    {
        ret = zcfgFeDalEUActionGet(Jobj, Jarray);
    }
    return ret;
}