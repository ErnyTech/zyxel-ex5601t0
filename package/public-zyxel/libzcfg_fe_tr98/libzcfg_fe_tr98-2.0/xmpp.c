#include <json/json.h>
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
#include "zlog_api.h"
#include "xmpp_parameter.h"

extern tr98Object_t tr98Obj[];


zcfgRet_t xmppObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
    zcfgRet_t           ret = ZCFG_SUCCESS;
    objIndex_t          objIid = {0};
    struct json_object  *xmppObj = NULL;
    struct json_object  *paramValue = NULL;
    tr98Parameter_t     *paramList = NULL;

    ZLOG_DEBUG("Enter Function");

    IID_INIT(objIid);
    if (strcmp(tr98FullPathName, "InternetGatewayDevice.XMPP") == 0)
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_XMPP, &objIid, &xmppObj)) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("fail to get object");
            return ZCFG_INTERNAL_ERROR;
        }
    }

    if (strstr(tr98FullPathName, "InternetGatewayDevice.XMPP.Connection") != NULL)
    {
        objIid.level=1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.XMPP.Connection.%hhu", &objIid.idx[0]);
        if ((ret = zcfgFeObjJsonGet(RDM_OID_XMPP_CONN, &objIid, &xmppObj)) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("fail to get object with tr98FullPathName=%s",tr98FullPathName);
            return ZCFG_INTERNAL_ERROR;
        }
    }

    /* Fill up tr98 object from related tr181 objects */
    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;
    while (paramList->name != NULL)
    {
        /* Write parameter value from tr181 objects to tr98 object */
        paramValue = json_object_object_get(xmppObj, paramList->name);
        if (paramValue != NULL)
        {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }

        /* Not defined in tr181, give it a default value */
        ZLOG_ERROR("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    zcfgFeJsonObjFree(xmppObj);
    return ZCFG_SUCCESS;

}

zcfgRet_t xmppConnSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
    objIndex_t          objIid = {0};
    struct json_object  *xmppConnSrvObj = NULL;
    struct json_object  *paramValue = NULL;
    tr98Parameter_t     *paramList = NULL;
    char                xmppConnSrvPath[128] = "InternetGatewayDevice.XMPP.Connection.%hhu.Server.%hhu";

    ZLOG_DEBUG("Enter Function");

    IID_INIT(objIid);
    objIid.level=2;
    if (sscanf(tr98FullPathName, xmppConnSrvPath, &objIid.idx[0], &objIid.idx[1]) != 2)
    {
        ZLOG_ERROR("invalid object");
        return ZCFG_INVALID_OBJECT;
    }

    if (zcfgFeObjJsonGet(RDM_OID_XMPP_CONN_SRV, &objIid, (void **)&xmppConnSrvObj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("fail to get object with tr98FullPathName=%s",tr98FullPathName);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Fill up tr98 object from related tr181 objects */
    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;
    while (paramList->name != NULL)
    {
        /* Write parameter value from tr181 objects to tr98 object */
        paramValue = json_object_object_get(xmppConnSrvObj, paramList->name);
        if (paramValue != NULL)
        {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }

        /* Not defined in tr181, give it a default value */
        ZLOG_ERROR("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    zcfgFeJsonObjFree(xmppConnSrvObj);
    return ZCFG_SUCCESS;
}


zcfgRet_t xmppObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
    zcfgRet_t                ret = ZCFG_SUCCESS;
    struct json_object      *xmppConnObj = NULL;
    struct json_object      *paramValue = NULL;
    struct json_object      *tmpObj = NULL;
    tr98Parameter_t         *paramList = NULL;
    objIndex_t              objIid;
    zcfg_offset_t           oid;

    ZLOG_DEBUG("Enter Function");
    oid = RDM_OID_XMPP_CONN;
    IID_INIT(objIid);
    objIid.level=1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.XMPP.Connection.%hhu", &objIid.idx[0]);
    if ((ret = zcfgFeObjJsonGet(oid, &objIid, &xmppConnObj)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("fail to get object with tr98FullPathName=%s",tr98FullPathName);
        return ZCFG_INTERNAL_ERROR;
    }

    /* if multiple set is required, just appended JSON to multiJobj */
    if (multiJobj)
    {
        tmpObj = xmppConnObj;
        xmppConnObj = NULL;
        xmppConnObj = zcfgFeJsonMultiObjAppend(oid, &objIid, multiJobj, tmpObj);
    }

    if (xmppConnObj)
    {
        paramList = tr98Obj[handler].parameter;
        while (paramList->name)
        {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);

             if(paramValue != NULL)
             {
                json_object_object_add(xmppConnObj, paramList->name, JSON_OBJ_COPY(paramValue));
            }

            paramList++;
            continue;
        }

        if (!multiJobj)
        {
            if ((ret = zcfgFeObjJsonSet(oid, &objIid, xmppConnObj, NULL)) != ZCFG_SUCCESS )
            {
                ZLOG_ERROR( "Set object(oid %d) Fail with ret=%d", oid, ret);
            }
            zcfgFeJsonObjFree(xmppConnObj);
            return ret;
        }
        zcfgFeJsonObjFree(tmpObj);
    }
    return ZCFG_SUCCESS;
}

zcfgRet_t xmppConnSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{

    zcfgRet_t                ret = ZCFG_SUCCESS;
    struct json_object      *xmppConnSrvObj = NULL;
    struct json_object      *paramValue = NULL;
    struct json_object      *tmpObj = NULL;
    tr98Parameter_t         *paramList = NULL;
    objIndex_t              objIid;
    zcfg_offset_t           oid;
    char                    xmppConnSrvPath[128] = "InternetGatewayDevice.XMPP.Connection.%hhu.Server.%hhu";

    ZLOG_DEBUG("Enter Function");
    oid = RDM_OID_XMPP_CONN_SRV;
    IID_INIT(objIid);
    objIid.level=2;
    if (sscanf(tr98FullPathName, xmppConnSrvPath, &objIid.idx[0], &objIid.idx[1]) != 2)
    {
        return ZCFG_INVALID_OBJECT;
    }

    if ((ret = zcfgFeObjJsonGet(oid, &objIid, &xmppConnSrvObj)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("fail to get object");
        return ZCFG_INTERNAL_ERROR;
    }

    /* if multiple set is required, just appended JSON to multiJobj */
    if (multiJobj)
    {
        tmpObj = xmppConnSrvObj;
        xmppConnSrvObj = NULL;
        xmppConnSrvObj = zcfgFeJsonMultiObjAppend(oid, &objIid, multiJobj, tmpObj);
    }

    if (xmppConnSrvObj)
    {
        paramList = tr98Obj[handler].parameter;
        while (paramList->name)
        {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);

             if(paramValue != NULL)
             {
                json_object_object_add(xmppConnSrvObj, paramList->name, JSON_OBJ_COPY(paramValue));
            }

            paramList++;
            continue;
        }

        if (!multiJobj)
        {
            if ((ret = zcfgFeObjJsonSetWithoutApply(oid, &objIid, xmppConnSrvObj, NULL)) != ZCFG_SUCCESS )
            {
                ZLOG_ERROR( "Set object(oid %d) Fail with ret=%d", oid, ret);
            }
            zcfgFeJsonObjFree(xmppConnSrvObj);
            return ret;
        }
        zcfgFeJsonObjFree(tmpObj);
    }
    return ZCFG_SUCCESS;
}
