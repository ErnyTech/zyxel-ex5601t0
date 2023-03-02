/*!
 *  @file zcfg_fe_dal_zlog_setting.c
 *
 *  @author     ZE-Lin Chen
 *  @date       2018-11-05
 *  @copyright  Copyright 2017 Zyxel Communications Corp. All Rights Reserved.
 */
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
#include "zlog_api.h"
#include <sys/stat.h>

dal_param_t ZLOG_SETTING_param[]=
{
    {"Enable",                      dalType_boolean, 0, 0,  NULL},
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    {"GPON_LOG_Enable",             dalType_boolean, 0, 0,  NULL},
#endif
    {"Severity",                    dalType_string,  0, 0,  NULL ,  "EMERG|ALERT|CRITICAL|ERROR|WARNING|NOTICE|INFO|DEBUG",  0},
    {"Console",                     dalType_boolean, 0, 0,  NULL},
    {"UsbFile",                     dalType_boolean, 0, 0,  NULL},
    {"Filename",                    dalType_string,  0, 0,  NULL},
    {"SyslogServer",                dalType_boolean, 0, 0,  NULL},
    {"SyslogServerIp",              dalType_string,  0, 0,  NULL},
    {"SyslogServerPort",            dalType_int,     0, 0,  NULL},
    {"Userdefinefile",              dalType_boolean, 0, 0,  NULL},
    {"FullPath",                    dalType_string,  0, 0,  NULL},
    {"Index",                       dalType_int,     0, 0,  NULL},
    {NULL,						    0,				 0,	0,  NULL ,   NULL, 				 0},
};

/*!
 *  combine in user config and current config
 *
 *  @param [in]     usr_config      configuration set by/from user
 *  @param [inout]  config          [in]  current configuration in system
 *                                  [out] resulting config of user config + current config
 *
 *  @return 0                       - successful
 *          other                   - failed
 */
static int _edittingSettingCombine(struct json_object *usr_config, struct json_object *config)
{
    bool                enable;
    bool                console;
    bool                usbFile;
    bool                syslogServer;
    bool                userdefinefile;
    const char          *severity;
    const char          *filename;
    const char          *syslogServerIp;
    const char          *fullPath;
    unsigned int        syslogServerPort;
    struct json_object  *tmp = NULL;

    if (usr_config == NULL)
    {
        ZLOG_ERROR("usr_config == NULL");
        return -1;
    }

    if (config == NULL)
    {
        ZLOG_ERROR("config == NULL");
        return -1;
    }

    tmp = json_object_object_get(usr_config, "Enable");

    if (tmp)
    {
        enable = json_object_get_boolean(tmp);
        json_object_object_add(config, "Enable", json_object_new_boolean(enable));
    }

    tmp = json_object_object_get(usr_config, "Severity");

    if (tmp)
    {
        severity = json_object_get_string(tmp);

        if (severity == NULL)
        {
            ZLOG_ERROR("severity == NULL");
            return -1;
        }

        if (!strcmp("EMERG", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(0));
        }
        else if (!strcmp("ALERT", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(1));
        }
        else if (!strcmp("CRITICAL", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(2));
        }
        else if (!strcmp("ERROR", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(3));
        }
        else if (!strcmp("WARNING", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(4));
        }
        else if (!strcmp("NOTICE", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(5));
        }
        else if (!strcmp("INFO", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(6));
        }
        else if (!strcmp("DEBUG", severity))
        {
            json_object_object_add(config, "Severity", json_object_new_int(7));
        }
    }

    tmp = json_object_object_get(usr_config, "Console");

    if (tmp)
    {
        console = json_object_get_boolean(tmp);
        json_object_object_add(config, "Console", json_object_new_boolean(console));
    }

    tmp = json_object_object_get(usr_config, "UsbFile");

    if (tmp)
    {
        usbFile = json_object_get_boolean(tmp);
        json_object_object_add(config, "UsbFile", json_object_new_boolean(usbFile));
    }

    tmp = json_object_object_get(usr_config, "Filename");

    if (tmp)
    {
        filename = json_object_get_string(tmp);
        json_object_object_add(config, "Filename", json_object_new_string(filename));
    }

    tmp = json_object_object_get(usr_config, "SyslogServer");

    if (tmp)
    {
        syslogServer = json_object_get_boolean(tmp);
        json_object_object_add(config, "SyslogServer", json_object_new_boolean(syslogServer));
    }

    tmp = json_object_object_get(usr_config, "SyslogServerIp");

    if (tmp)
    {
        syslogServerIp = json_object_get_string(tmp);
        json_object_object_add(config, "SyslogServerIp", json_object_new_string(syslogServerIp));
    }

    tmp = json_object_object_get(usr_config, "SyslogServerPort");

    if (tmp)
    {
        syslogServerPort = json_object_get_int(tmp);
        json_object_object_add(config, "SyslogServerPort", json_object_new_int(syslogServerPort));
    }

    tmp = json_object_object_get(usr_config, "Userdefinefile");

    if (tmp)
    {
        userdefinefile = json_object_get_boolean(tmp);
        json_object_object_add(config, "Userdefinefile", json_object_new_boolean(userdefinefile));
    }

    tmp = json_object_object_get(usr_config, "FullPath");

    if (tmp)
    {
        fullPath = json_object_get_string(tmp);
        json_object_object_add(config, "FullPath", json_object_new_string(fullPath));
    }

    return 0;
}

/*!
 *  copy all object values into dal-module config
 *
 *  @param [in] categoryJobj        current configuration in system
 *  @param [inout]config            [in]  new config for dal config
 *                                  [out] resulting config of new config + current config  suitable for dal-module config
 *
 *  @return 0                       - successful
 *          other                   - failed
 */
static int _gettingSettingCombine(struct json_object *categoryJobj, struct json_object *config)
{
    int severity;

    if (categoryJobj == NULL)
    {
        ZLOG_ERROR("categoryJobj == NULL");
        return -1;
    }

    if (config == NULL)
    {
        ZLOG_ERROR("programJobj == NULL");
        return -1;
    }

    json_object_object_add(config, "Enable", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "Enable")));
    json_object_object_add(config, "Name", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "Describe")));

    severity = json_object_get_int(json_object_object_get(categoryJobj, "Severity"));

    switch (severity)
    {
        case 0:
            json_object_object_add(config, "Severity", json_object_new_string("EMERG"));
            break;

        case 1:
            json_object_object_add(config, "Severity", json_object_new_string("ALERT"));
            break;

        case 2:
            json_object_object_add(config, "Severity", json_object_new_string("CRITICAL"));
            break;

        case 3:
            json_object_object_add(config, "Severity", json_object_new_string("ERROR"));
            break;

        case 4:
            json_object_object_add(config, "Severity", json_object_new_string("WARNING"));
            break;

        case 5:
            json_object_object_add(config, "Severity", json_object_new_string("NOTICE"));
            break;

        case 6:
            json_object_object_add(config, "Severity", json_object_new_string("INFO"));
            break;

        case 7:
            json_object_object_add(config, "Severity", json_object_new_string("DEBUG"));
            break;

        default:
            ZLOG_ERROR("invalid severity %d", severity);
            return -1;
    } // switch

    json_object_object_add(config, "Console", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "Console")));
    json_object_object_add(config, "UsbFile", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "UsbFile")));
    json_object_object_add(config, "Filename", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "Filename")));
    json_object_object_add(config, "SyslogServer", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "SyslogServer")));
    json_object_object_add(config, "SyslogServerIp", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "SyslogServerIp")));
    json_object_object_add(config, "SyslogServerPort", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "SyslogServerPort")));
    json_object_object_add(config, "Userdefinefile", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "Userdefinefile")));
    json_object_object_add(config, "FullPath", JSON_OBJ_COPY(json_object_object_get(categoryJobj, "FullPath")));

    return 0;
}

zcfgRet_t zcfgFeDalZlogCategory(struct json_object *Jobj);

#ifdef CONFIG_PACKAGE_zconsole_monitor
static zcfgRet_t zcfgFeDalZConsoleLogSet(struct json_object *zconsole_jobj)
{
    struct          json_object *zconsoleLogJobj = NULL, *jobj = NULL;
    objIndex_t      zconsoleIid;
    bool            zconsole_enable = false, common_server = false, overwrite = false;
    char            *zconsole_server_ip = NULL, *pre_run_cmd = NULL;
    uint32_t        zconsole_port = 0, zconsole_buf_size = 0;

    IID_INIT(zconsoleIid);
    if (zcfgFeObjJsonGet(RDM_OID_ZCONSOLE_LOG_REDIRECT, &zconsoleIid, &zconsoleLogJobj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to get RDM_OID_ZCONSOLE_LOG_REDIRECT\n");
        return ZCFG_INTERNAL_ERROR;
    }

    jobj = json_object_object_get(zconsole_jobj, "Enable");
    if (jobj != NULL) 
    {
        zconsole_enable = json_object_get_boolean(jobj);
        json_object_object_add(zconsoleLogJobj, "Enable", json_object_new_boolean(zconsole_enable));
    }

    jobj = json_object_object_get(zconsole_jobj, "LogServerIP");
    if (jobj != NULL)
    {
        zconsole_server_ip = json_object_get_string(jobj);
        json_object_object_add(zconsoleLogJobj, "LogServerIP", json_object_new_string(zconsole_server_ip));
    }

    jobj = json_object_object_get(zconsole_jobj, "LogServerPort");
    if (jobj != NULL)
    {
        zconsole_port = json_object_get_int(jobj);
        json_object_object_add(zconsoleLogJobj, "LogServerPort", json_object_new_int(zconsole_port));
    }

    jobj = json_object_object_get(zconsole_jobj, "BufferSize");
    if (jobj != NULL)
    {
        zconsole_buf_size = json_object_get_int(jobj);
        json_object_object_add(zconsoleLogJobj, "BufferSize", json_object_new_int(zconsole_buf_size));
    }

    jobj = json_object_object_get(zconsole_jobj, "PreRunCommand");
    if (jobj != NULL)
    {
        pre_run_cmd = json_object_get_string(jobj);
        json_object_object_add(zconsoleLogJobj, "PreRunCommand", json_object_new_string(pre_run_cmd));
    }

    jobj = json_object_object_get(zconsole_jobj, "UseCommonServer");
    if (jobj != NULL) 
    {
        common_server = json_object_get_boolean(jobj);
        json_object_object_add(zconsoleLogJobj, "UseCommonServer", json_object_new_boolean(common_server));
    }

    jobj = json_object_object_get(zconsole_jobj, "BufferOverwrite");
    if (jobj != NULL) 
    {
        overwrite = json_object_get_boolean(jobj);
        json_object_object_add(zconsoleLogJobj, "BufferOverwrite", json_object_new_boolean(overwrite));
    }

    if (zcfgFeObjJsonSet(RDM_OID_ZCONSOLE_LOG_REDIRECT, &zconsoleIid, zconsoleLogJobj, NULL) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to set RDM_OID_ZLOG_CONFIG\n");
        zcfgFeJsonObjFree(zconsoleLogJobj);
        return ZCFG_INTERNAL_ERROR;
    }

    return ZCFG_SUCCESS;
}
#endif

/*!
 *  Edit zlog setting fot dalcmd or Edit zlog data model setting
 *
 *  @param [in]  Jobj       is a storage logsetting struct
 *  @param [out] replyMsg   respond message
 *                          could be NULL
 *
 *  @return ZCFG_SUCCESS    - successful
 *          other           - failed
 */
zcfgRet_t zcfgFeDalZlogSettingEdit(struct json_object *Jobj, char *replyMsg)
{
    int             index                       = 0;
    struct          json_object *obj            = NULL;
    struct          json_object *logSettingJobj = NULL;
    struct          json_object *categoryJobj   = NULL;
#ifdef CONFIG_PACKAGE_zconsole_monitor
    struct          json_object *zconsoleJobj   = NULL;
#endif
    objIndex_t      logSettingIid;
    objIndex_t      categoryIid;
    bool            enable;
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    bool            Gpon_enable;
    struct          stat Stat;
#endif

    if (Jobj == NULL)
    {
        ZLOG_ERROR("Jobj == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    /*--for set dal cmd ---*/
    if (json_object_get_type(Jobj) == json_type_object)
    {
        if (!json_object_object_get(Jobj, "Enable") && !json_object_object_get(Jobj, "Index") && !json_object_object_get(Jobj, "GPON_LOG_Enable"))
        {
            if (replyMsg)
            {
                strcpy(replyMsg, "Please enter Enable or Index");
            }
			return ZCFG_INVALID_PARAM_VALUE;
		}

        /*--for zlog-category ---*/
        index = json_object_get_int(json_object_object_get(Jobj, "Index"));

        if (index < 0)
        {
            ZLOG_ERROR("invalid index %d", index);
			return ZCFG_INTERNAL_ERROR;
        }

        if (index != 0)
        {
            IID_INIT(categoryIid);
			categoryIid.idx[0] = index;
			categoryIid.level = 1;

			if (zcfgFeObjJsonGet(RDM_OID_ZLOG_CATEGORY, &categoryIid, &categoryJobj) != ZCFG_SUCCESS)
            {
                ZLOG_ERROR("Fail to get RDM_OID_ZLOG_CATEGORY");
				return ZCFG_INTERNAL_ERROR;
			}

            if (_edittingSettingCombine(Jobj, categoryJobj) == -1)
            {
                ZLOG_ERROR("Fail to _edittingSettingCombine()");
                zcfgFeJsonObjFree(categoryJobj);
                return ZCFG_INTERNAL_ERROR;
            }

            if (zcfgFeObjJsonSet(RDM_OID_ZLOG_CATEGORY, &categoryIid, categoryJobj, NULL) != ZCFG_SUCCESS)
            {
                ZLOG_ERROR("Fail to set RDM_OID_ZLOG_CATEGORY");
                zcfgFeJsonObjFree(categoryJobj);
				return ZCFG_INTERNAL_ERROR;
			}
			zcfgFeJsonObjFree(categoryJobj);
			return ZCFG_SUCCESS;
        }

        /*--for zlog-config ---*/
        IID_INIT(logSettingIid);

        if (zcfgFeObjJsonGet(RDM_OID_ZLOG_CONFIG, &logSettingIid, &logSettingJobj) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to get RDM_OID_ZLOG_CONFIG\n");
            return ZCFG_INTERNAL_ERROR;
        }

        enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));

        if (json_object_object_get(Jobj, "Enable"))
        {
            json_object_object_add(logSettingJobj, "Enable", json_object_new_boolean(enable));
        }

        if (zcfgFeObjJsonSet(RDM_OID_ZLOG_CONFIG, &logSettingIid, logSettingJobj, NULL) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to set RDM_OID_ZLOG_CONFIG\n");
            zcfgFeJsonObjFree(logSettingJobj);
            return ZCFG_INTERNAL_ERROR;
        }
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)   
        Gpon_enable = json_object_get_boolean(json_object_object_get(Jobj, "GPON_LOG_Enable"));

        if (Gpon_enable)
        {
            if (stat("/data/ext/gpon_driver_log", &Stat) != 0)
            {
                system("mkdir /data/ext");
                system("touch /data/gpon_driver_log");
                system("mv /data/gpon_driver_log /data/ext/");
            }
        }
        else
        {
            if (stat("/data/ext/gpon_driver_log", &Stat) == 0)
            {
                system("rm /data/ext/gpon_driver_log");
            }
        }
#endif
        zcfgFeJsonObjFree(logSettingJobj);
        return ZCFG_SUCCESS;
    }//if

    /*--for set zlog data model ---*/
    obj = json_object_array_get_idx(Jobj, 0);
    IID_INIT(logSettingIid);

    if (zcfgFeDalZlogCategory(Jobj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to zcfgFeDalZlogCategory()");
        return ZCFG_INTERNAL_ERROR;
    }

    if (zcfgFeObjJsonGet(RDM_OID_ZLOG_CONFIG, &logSettingIid, &logSettingJobj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to get RDM_OID_ZLOG_CONFIG\n");
        return ZCFG_INTERNAL_ERROR;
    }

    if (_edittingSettingCombine(obj, logSettingJobj) == -1)
    {
        ZLOG_ERROR("Fail to _edittingSettingCombine()");
        zcfgFeJsonObjFree(logSettingJobj);
        return ZCFG_INTERNAL_ERROR;
    }

    if (zcfgFeObjJsonSet(RDM_OID_ZLOG_CONFIG, &logSettingIid, logSettingJobj, NULL) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to set RDM_OID_ZLOG_CONFIG\n");
        zcfgFeJsonObjFree(logSettingJobj);
        return ZCFG_INTERNAL_ERROR;
    }
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    Gpon_enable = json_object_get_boolean(json_object_object_get(obj, "GPON_LOG_Enable"));

    if (Gpon_enable)
    {
        if (stat("/data/ext/gpon_driver_log", &Stat) != 0)
        {
            system("mkdir /data/ext");
            system("touch /data/gpon_driver_log");
            system("mv /data/gpon_driver_log /data/ext/");
        }
    }
    else
    {
        if (stat("/data/ext/gpon_driver_log", &Stat) == 0)
        {
            system("rm -r /data/ext/");
        }
    }
#endif
    zcfgFeJsonObjFree(logSettingJobj);

#ifdef CONFIG_PACKAGE_zconsole_monitor
    zconsoleJobj = json_object_object_get(obj, "zconsole_log");
    if (zconsoleJobj != NULL)
    {
        if(zcfgFeDalZConsoleLogSet(zconsoleJobj) != ZCFG_SUCCESS)
        {
            return ZCFG_INTERNAL_ERROR;
        }
    }
#endif
    return ZCFG_SUCCESS;
}

/*!
 *  zlog data model for category setting
 *
 *  @param [in] Jobj        is a storage categorylogsetting struct object
 *
 *  @return ZCFG_SUCCESS    - successful
 *          other           - failed
 */
zcfgRet_t zcfgFeDalZlogCategory(struct json_object *Jobj)
{
    struct          json_object *reqObject          = NULL;
    struct          json_object *categoryJobj       = NULL;
    int             index                           = 0;
    int             len                             = 0;
    objIndex_t      categoryIid;

    if (Jobj == NULL)
    {
        ZLOG_ERROR("Jobj == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    /*-------------------------------
    Array[0]    : Zlog Setting-Enable
    Array[1]    : All
    Array[2] -  : Program
    ---------------------------------*/
    len = json_object_array_length(Jobj);

    for (int reqObjIdx = 1; reqObjIdx < len; ++reqObjIdx)
    {
        reqObject = json_object_array_get_idx(Jobj, reqObjIdx);

        IID_INIT(categoryIid);
        index = json_object_get_int(json_object_object_get(reqObject, "Index"));
        
        if (index < 0)
        {
            ZLOG_ERROR("invalid index %d", index);
			return ZCFG_INTERNAL_ERROR;
        }

        categoryIid.idx[0] = index;
		categoryIid.level = 1;

        if (zcfgFeObjJsonGet(RDM_OID_ZLOG_CATEGORY, &categoryIid, &categoryJobj) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to Get RDM_OID_ZLOG_CATEGORY");
            return ZCFG_INTERNAL_ERROR;
        }

        if (_edittingSettingCombine(reqObject, categoryJobj) == -1)
        {
            ZLOG_ERROR("Fail to _edittingSettingCombine()");
            zcfgFeJsonObjFree(categoryJobj);
            return ZCFG_INTERNAL_ERROR;
        }

        if (zcfgFeObjJsonSet(RDM_OID_ZLOG_CATEGORY, &categoryIid, categoryJobj, NULL) != ZCFG_SUCCESS)
        {
            ZLOG_ERROR("Fail to Set RDM_OID_ZLOG_CATEGORY");
            zcfgFeJsonObjFree(categoryJobj);
            return ZCFG_INTERNAL_ERROR;
        }
        zcfgFeJsonObjFree(categoryJobj);
    }
    return ZCFG_SUCCESS;
}

/*!
 *  Get zlog data module value put into Jarray
 *
 *  @param [in]  Jobj           is a storage zlogsetting struct
 *  @param [out] Jarray         is a storage zlogsetting all struct array
 *
 *  @return ZCFG_SUCCESS        - successful
 *          other               - failed
 */
zcfgRet_t zcfgFeDalZlogSettingGet(struct json_object *Jobj, struct json_object *Jarray)
{
    struct json_object *LogJobj         = NULL;
    struct json_object *categoryJobj    = NULL;
    struct json_object *config          = NULL;
#ifdef CONFIG_PACKAGE_zconsole_monitor
    struct json_object *zconsoleLogJobj = NULL;
	objIndex_t          zconsoleIid     = {0};
#endif
    objIndex_t          LogIid          = {0};
    objIndex_t          CategoryIid     = {0};
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    struct stat         Stat;
#endif

    if (Jobj == NULL)
    {
        ZLOG_ERROR("Jobj == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    if (Jarray == NULL)
    {
        ZLOG_ERROR("Jarray == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    IID_INIT(LogIid);

    if (zcfgFeObjJsonGet(RDM_OID_ZLOG_CONFIG, &LogIid, &LogJobj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to Get RDM_OID_ZLOG_CONFIG");
        return ZCFG_INTERNAL_ERROR;
    }
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    if (stat("/data/ext/gpon_driver_log", &Stat) == 0)
    {
        json_object_object_add(LogJobj, "GPON_LOG_Enable", json_object_new_boolean(true));
    }
    else
    {
        json_object_object_add(LogJobj, "GPON_LOG_Enable", json_object_new_boolean(false));
    }
#endif

#ifdef CONFIG_PACKAGE_zconsole_monitor
    IID_INIT(zconsoleIid);
    if (zcfgFeObjJsonGet(RDM_OID_ZCONSOLE_LOG_REDIRECT, &zconsoleIid, &zconsoleLogJobj) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Fail to Get RDM_OID_ZCONSOLE_LOG_REDIRECT");
        return ZCFG_INTERNAL_ERROR;
    }
    json_object_object_add(LogJobj, "zconsole_log", zconsoleLogJobj);
#endif

    json_object_array_add(Jarray, LogJobj);

    IID_INIT(CategoryIid);

    while (zcfgFeObjJsonGetNext(RDM_OID_ZLOG_CATEGORY, &CategoryIid, &categoryJobj) == ZCFG_SUCCESS)
    {
        config = json_object_new_object();
		json_object_object_add(config, "Index", json_object_new_int(CategoryIid.idx[0]));

        if (_gettingSettingCombine(categoryJobj, config) == -1)
        {
            ZLOG_ERROR("Fail to _gettingSettingCombine()");
            zcfgFeJsonObjFree(categoryJobj);
            return ZCFG_INTERNAL_ERROR;
        }

        json_object_array_add(Jarray, config);
		zcfgFeJsonObjFree(categoryJobj);
    }

    return ZCFG_SUCCESS;
}

/*!
 *  zlog setting is showing in the console
 *
 *  @param [in] Jarray    is a storage logsetting array
 */
void zcfgFeDalShowZlogSetting(struct json_object *Jarray)
{
    struct json_object    *obj            = NULL;
    int                   len             = 0;
    int                   i;
    int                   index;
    const char            *syslogServerIp;
    const char            *syslogServerPort;
    const char            *name;
    const char            *enable;
    const char            *severity;
    const char            *console;
    const char            *usbFile;
    const char            *filename;
    const char            *syslogServer;
    const char            *userdefinefile;
    const char            *fullPath;

    if (Jarray == NULL)
    {
        ZLOG_ERROR("Jarray == NULL");
        return;
    }

    if (json_object_get_type(Jarray) != json_type_array)
    {
       ZLOG_ERROR("wrong Jobj format!\n");
       return;
    }

    obj = json_object_array_get_idx(Jarray, 0);
    printf("Zlog Setting \n");
    printf("%-2s %-20s %-10s \n", "", "Syslog Logging:", json_object_get_string(json_object_object_get(obj, "Enable")));
    printf("%-2s %-20s %-10s \n", "", "Status:", json_object_get_string(json_object_object_get(obj, "Status")));
#if defined(MOS_SUPPORT) && (BROADCOM_PLATFORM)
    printf("%-2s %-20s %-10s \n", "", "GPON_LOG_Enable:", json_object_get_string(json_object_object_get(obj, "GPON_LOG_Enable")));
#endif
    printf("\n");

    /*-------------------------------
    Array[0]    : Zlog Setting-Enable
    Array[1]    : All
    Array[2] -  : Program
    ---------------------------------*/
    len = json_object_array_length(Jarray);

    for (i = 1; i < len; ++i)
    {
        obj = json_object_array_get_idx(Jarray, i);

        if (obj != NULL)
        {
            index               = json_object_get_int(json_object_object_get(obj, "Index"));
            name                = json_object_get_string(json_object_object_get(obj, "Name"));
            enable              = json_object_get_string(json_object_object_get(obj, "Enable"));
            severity            = json_object_get_string(json_object_object_get(obj, "Severity"));
            console             = json_object_get_string(json_object_object_get(obj, "Console"));
            usbFile             = json_object_get_string(json_object_object_get(obj, "UsbFile"));
            filename            = json_object_get_string(json_object_object_get(obj, "Filename"));
            syslogServer        = json_object_get_string(json_object_object_get(obj, "SyslogServer"));
            syslogServerIp      = json_object_get_string(json_object_object_get(obj, "SyslogServerIp"));
            syslogServerPort    = json_object_get_string(json_object_object_get(obj, "SyslogServerPort"));
            userdefinefile      = json_object_get_string(json_object_object_get(obj, "Userdefinefile"));
            fullPath            = json_object_get_string(json_object_object_get(obj, "FullPath"));

            printf("%-6s %-14s %-7s %-7s %-7s %-7s %-10s %-7s %-8s %-10s %-10s %-20s \n","Index", "Name", "Enable", "Severity", "Console", "UsbFile", "Filename", "Server", "ServerIp", "ServerPort", "Userdefinefile", "FullPath");
            printf("%-6u %-14s %-7s %-8s %-7s %-7s %-10s %-7s %-8s %-10s %-10s %-20s \n", index, name, enable, severity, console, usbFile, filename, syslogServer, syslogServerIp, syslogServerPort, userdefinefile, fullPath);
        }
    }
}

/*!
 *  zlog dal-module put or get
 *
 *  @param [in] method      request method
 *  @param [in] Jobj        is a json_object struct
 *  @param [in] Jarray      is a json_object struct
 *
 *  @return ZCFG_SUCCESS    - successful
 *          other           - failed
 */
zcfgRet_t zcfgFeDalZlogSetting(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (method == NULL)
    {
        ZLOG_ERROR("method == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    if (Jobj == NULL)
    {
        ZLOG_ERROR("Jobj == NULL");
        return ZCFG_INTERNAL_ERROR;
    }

    if (!strcmp(method, "GET"))
    {
        if (Jarray == NULL)
        {
            ZLOG_ERROR("Jarray == NULL");
            return ZCFG_INTERNAL_ERROR;
        }
    }

    if (!strcmp(method, "PUT"))
    {
        ret = zcfgFeDalZlogSettingEdit(Jobj, replyMsg);
    }
    else if (!strcmp(method, "GET"))
    {
        ret = zcfgFeDalZlogSettingGet(Jobj, Jarray);
    }
    else
    {
        ZLOG_ERROR("Unknown method :%s\n", method);
        return ZCFG_INTERNAL_ERROR;
    }

    return ret;
}
