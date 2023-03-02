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

extern tr98Object_t tr98Obj[];

zcfgRet_t ZyEEObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
    zcfgRet_t           ret             = ZCFG_SUCCESS;
    struct json_object  *ZyEEObj        = NULL;
    struct json_object  *paramValue     = NULL;
    tr98Parameter_t     *paramList      = NULL;
	objIndex_t          objIid;

    printf("%s : Enter\n", __FUNCTION__);
	IID_INIT(objIid);

    if (strcmp(tr98FullPathName, "InternetGatewayDevice.SoftwareModules") == 0)
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
    }
	if (strcmp(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.X_ZYXEL_ExecEnvConfigure") == 0)
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EE_CONF, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecEnv"))
    {
        objIid.level=1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecEnv.%hhu", &objIid.idx[objIid.level - 1]);
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_ENV, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit"))
    {
        if (strstr(tr98FullPathName, "Extensions") != NULL)
        {
            objIid.level=1;
            sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%hhu", &objIid.idx[objIid.level - 1]);

            if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
            {
                printf("%s: fail to get object\n", __FUNCTION__);
                return ret;
            }
        }
       else
       {
            objIid.level=1;
            sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%hhu", &objIid.idx[objIid.level - 1]);
            if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
            {
                printf("%s: fail to get object\n", __FUNCTION__);
                return ret;
            }
       }
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.DeploymentUnit"))
    {
        objIid.level=1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.DeploymentUnit.%hhu", &objIid.idx[objIid.level - 1]);
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_DEPLOYMENT_UNIT, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
    }
    if (strstr(tr98FullPathName, "DUStateChangeComplPolicy"))
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_MGMT_SRV_DU_STATE_CHANGE_COMPL_POL, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
    }

    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;

    while (paramList->name != NULL)
    {
        paramValue = json_object_object_get(ZyEEObj, paramList->name);
        if (paramValue != NULL)
        {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }
        paramList++;
	}

    json_object_put(ZyEEObj);
    return ZCFG_SUCCESS;
}

zcfgRet_t ZyEEObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t           ret                     = ZCFG_SUCCESS;
	struct              json_object *ZyEEObj    = NULL;
	struct              json_object *paramValue = NULL;
	struct              json_object *tmpObj     = NULL;
	tr98Parameter_t     *paramList              = NULL;
	objIndex_t          objIid;
    zcfg_offset_t       oid;
	IID_INIT(objIid);

    printf("%s : Enter\n", __FUNCTION__);

    if (strcmp(tr98FullPathName, "InternetGatewayDevice.SoftwareModules") == 0)
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
        oid = RDM_OID_SW_MODULE;
    }
	if (strcmp(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.X_ZYXEL_ExecEnvConfigure") == 0)
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EE_CONF, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
        oid = RDM_OID_SW_MODULE_EE_CONF;
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecEnv"))
    {
        objIid.level=1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecEnv.%hhu", &objIid.idx[objIid.level - 1]);
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_ENV, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
        oid = RDM_OID_SW_MODULE_EXEC_ENV;
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.DeploymentUnit"))
    {
        objIid.level=1;
        sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.DeploymentUnit.%hhu", &objIid.idx[objIid.level - 1]);
        if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_DEPLOYMENT_UNIT, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
        oid = RDM_OID_SW_MODULE_DEPLOYMENT_UNIT;
    }
    if (strstr(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit"))
    {
        if (strstr(tr98FullPathName, "Extensions") != NULL)
        {
            objIid.level=1;
            sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%hhu", &objIid.idx[objIid.level - 1]);

            if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
            {
                printf("%s: fail to get object\n", __FUNCTION__);
                return ret;
            }
            oid = RDM_OID_SW_MODULE_EXEC_UNIT_EXTENSIONS;
        }
        else
        {
            objIid.level=1;
            sscanf(tr98FullPathName, "InternetGatewayDevice.SoftwareModules.ExecutionUnit.%hhu", &objIid.idx[objIid.level - 1]);

            if ((ret = zcfgFeObjJsonGet(RDM_OID_SW_MODULE_EXEC_UNIT, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
            {
                printf("%s: fail to get object\n", __FUNCTION__);
                return ret;
            }
            oid = RDM_OID_SW_MODULE_EXEC_UNIT;
        }
    }
    if (strstr(tr98FullPathName, "DUStateChangeComplPolicy"))
    {
        if ((ret = zcfgFeObjJsonGet(RDM_OID_MGMT_SRV_DU_STATE_CHANGE_COMPL_POL, &objIid, &ZyEEObj)) != ZCFG_SUCCESS)
        {
            printf("%s: fail to get object\n", __FUNCTION__);
            return ret;
        }
        oid = RDM_OID_MGMT_SRV_DU_STATE_CHANGE_COMPL_POL;
    }

	if (ZyEEObj)
    {
		paramList = tr98Obj[handler].parameter;
		while (paramList->name)
        {
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL)
            {
				json_object_object_add(ZyEEObj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}

        /* if multiple set is required, just appended JSON to multiJobj */
        if (multiJobj)
        {
            tmpObj = ZyEEObj;
            ZyEEObj = NULL;
            ZyEEObj = zcfgFeJsonMultiObjAppend(oid, &objIid, multiJobj, tmpObj);
        }

	}
	return ZCFG_SUCCESS;
}
zcfgRet_t ZyEEObjAdd(char *tr98FullPathName, int *idx)
{
    zcfgRet_t   ret;
    int         index = 0;
    int         emptyIns = 0;
    char        cmd[128] = {0};
    struct      json_object *ZyEEObj = NULL;
    objIndex_t  objIid;
    IID_INIT(objIid);

    /*workaround :Because use to zyee command, but I don?™t know which index to fill, so using this method*/
    while ((ret = zcfgFeObjJsonGetNext(RDM_OID_SW_MODULE_EXEC_ENV, &objIid, &ZyEEObj)) == ZCFG_SUCCESS)
    {
        index++;
        if (objIid.idx[0] == index)
        {
            continue;
        }
        else
        {
            emptyIns = index;
            break;
        }
    }

    if (index == 0)
    {
        snprintf(cmd, sizeof(cmd), "zyeecmd new zyee_tr1 -s 20");
        *idx = index + 1;
    }
#if 0 //only add one zyee object_
    if (emptyIns == 1 && index == 1) //Missing index 1
    {
        snprintf(cmd, sizeof(cmd), "zyeecmd new zyee_tr2");
        *idx = emptyIns;
    }
    if (emptyIns == 0 && index == 1) //Missing index 2
    {
        snprintf(cmd, sizeof(cmd), "zyeecmd new zyee_tr3");
        *idx = index + 1;
    }
#endif
    if (index > 0) //Can only add one objects
    {
        printf("Function = %s There are have one Zyee objects\n",__FUNCTION__);
        zcfgFeJsonObjFree(ZyEEObj);
        return ZCFG_EXCEED_MAX_INS_NUM;
    }

    if (system(cmd) != 0)
    {
        zcfgFeJsonObjFree(ZyEEObj);
        return ret;
    }

    zcfgFeJsonObjFree(ZyEEObj);

    return ZCFG_SUCCESS;
}
