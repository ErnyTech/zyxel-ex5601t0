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

dal_param_t ZYEE_DU_param[]={
    {"OPType",                   dalType_string,    0,  0,  NULL},
    {"UUID",                     dalType_string, 0,  0,  NULL},
    {"URL",                      dalType_string, 0,  0,  NULL},
    {"Username",                 dalType_string, 0,  0,  NULL},
    {"Password",                 dalType_int,    0,  0,  NULL},
    {"SelectedEE",                dalType_string, 0,  0,  NULL},
    {"DUName",                dalType_string, 0,  0,  NULL},
    {NULL,                       0,              0,  0,  NULL},
};

#define ZYEE_DU_INSTALL_RST_PATH    "/tmp/gui_zyee_du_install_.log"
#define ZYEE_DU_UNINSTALL_RST_PATH  "/tmp/gui_zyee_du_uninstall.log"

zcfgRet_t zcfgFeDalDUActionGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{    zcfgRet_t       ret = ZCFG_SUCCESS;
     FILE            *fp = NULL;
     char            rcvbuf[256] = {0};
     char            *rcvptr = NULL;
     int             rstIndex = 0;
     char            strStatus[640] = {0};

     const char      *opType=NULL;
     struct json_object *paramJobj = NULL;
     paramJobj = json_object_new_object();

     if ( NULL != json_object_object_get(Jobj, "OPType") )
     {
         opType = json_object_get_string(json_object_object_get(Jobj, "OPType"));
     }

     if (strcmp(opType,"Uninstall") == 0)
     {
         fp = fopen(ZYEE_DU_UNINSTALL_RST_PATH, "r");
     }
     else if (strcmp(opType,"Install") == 0)
     {
         fp = fopen(ZYEE_DU_INSTALL_RST_PATH, "r");
     }
     else
     {
         return ZCFG_INTERNAL_ERROR;
     }

     if (fp == NULL)
     {
         return ZCFG_INTERNAL_ERROR;
     }

     memset(strStatus, 0, sizeof(strStatus));
     /* get the response */
    while ((fgets(rcvbuf, 256, fp)) != NULL)
    {
        ZLOG_INFO("Recive: %s", rcvbuf);
        if (rcvptr = strstr(rcvbuf, "zyeecmd: "))
        {
            rstIndex++;
            ZOS_STRCAT(strStatus, rcvptr, sizeof(strStatus));
            continue;
        }
        if(rstIndex)
        {
            ZOS_STRCAT(strStatus, rcvbuf, sizeof(strStatus));
            break;
        }
        memset(rcvbuf, 0, sizeof(rcvbuf));
    }
	fclose(fp);
    if (strlen(strStatus) > 0)
    {
        json_object_object_add(paramJobj, "OPResult", json_object_new_string(strStatus));
        json_object_object_add(paramJobj, "OPState", json_object_new_string("Complete"));
    }
    else
    {
        if (strcmp(opType,"Uninstall") == 0)
        {
            json_object_object_add(paramJobj, "OPState", json_object_new_string("Uninstalling"));
        }
        else
        {
            json_object_object_add(paramJobj, "OPState", json_object_new_string("Installing"));
        }

        json_object_object_add(paramJobj, "OPResult", json_object_new_string(""));
    }
    json_object_array_add(Jarray, paramJobj);

    return ret;
}

zcfgRet_t zcfgFeDalDUActionSet(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t       ret = ZCFG_SUCCESS;
    char            eidStr[4] = {0};
    char            cmd[1792] = {0};

    const char      *opType=NULL;
    const char      *uuid;
    const char      *url;
    const char      *username;
    const char      *password;
    const char      *eeName;
    const char      *duName;

    ZLOG_INFO("input --- %s \n ",json_object_to_json_string(Jobj));

    if ( NULL != json_object_object_get(Jobj, "OPType") )
    {
        opType = json_object_get_string(json_object_object_get(Jobj, "OPType"));
    }
    if ( NULL != json_object_object_get(Jobj, "UUID") )
    {
        uuid = json_object_get_string(json_object_object_get(Jobj, "UUID"));
    }
    if( NULL != json_object_object_get(Jobj, "URL") )
    {
        url = json_object_get_string(json_object_object_get(Jobj, "URL"));
    }
    if ( NULL != json_object_object_get(Jobj, "Username") )
    {
        username = json_object_get_string(json_object_object_get(Jobj, "Username"));
    }
    if ( NULL != json_object_object_get(Jobj, "Password") )
    {
        password = json_object_get_string(json_object_object_get(Jobj, "Password"));
    }
    if ( NULL != json_object_object_get(Jobj, "NotifyEEName") )
    {
        eeName = json_object_get_string(json_object_object_get(Jobj, "NotifyEEName"));
    }

    if ( NULL != json_object_object_get(Jobj, "DUName") )
    {
        duName = json_object_get_string(json_object_object_get(Jobj, "DUName"));
    }

    if(strcmp(opType,"Uninstall") == 0)
    {
        sprintf(cmd, "zyeecmd notify %s -m ", eeName);
        strcat(cmd, "\"uninstall ");
        strcat(cmd, duName);
        strcat(cmd, "\" -eid ");
        sprintf(eidStr,"%d", ZCFG_EID_ZHTTPD);
        strcat(cmd, eidStr);
        strcat(cmd, " > ");
        strcat(cmd, ZYEE_DU_UNINSTALL_RST_PATH);
        strcat(cmd, " &");
    }
    else if(strcmp(opType,"Install") == 0)
    {
        sprintf(cmd, "zyeecmd notify %s -m ", eeName);
        strcat(cmd, "\"install ");
        strcat(cmd, url);
        strcat(cmd, " -uuid ");
        strcat(cmd, uuid);
        if (strlen(username) != 0)
        {
            strcat(cmd, " -u ");
            strcat(cmd, username);
            strcat(cmd, ":");
            strcat(cmd, password);
        }

        strcat(cmd, "\" -eid ");
        sprintf(eidStr,"%d", ZCFG_EID_ZHTTPD);
        strcat(cmd, eidStr);
        strcat(cmd, " > ");
        strcat(cmd, ZYEE_DU_INSTALL_RST_PATH);
        strcat(cmd, " &");
    }
    ZLOG_INFO("cmd --- %s \n ",cmd);

    // if(ZOS_SYSTEM(cmd) == ZOS_FAIL)
    // {
    //     return ZCFG_INTERNAL_ERROR;
    // }
    ret = dalReqSendWithoutResponse(cmd);

    return ret;
}

zcfgRet_t zcfgFeDalDUAction(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (!method || !Jobj)
    {
        return ZCFG_INTERNAL_ERROR;
    }

    if (!strcmp(method, "PUT"))
    {
        ret = zcfgFeDalDUActionSet(Jobj, NULL);
    }
    else if (!strcmp(method, "GET"))
    {
        ret = zcfgFeDalDUActionGet(Jobj, Jarray, NULL);
    }




    return ret;
}
