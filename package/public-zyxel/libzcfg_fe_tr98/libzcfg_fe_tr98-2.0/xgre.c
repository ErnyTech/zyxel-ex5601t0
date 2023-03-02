#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_net.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "xgre_parameter.h"

extern tr98Object_t tr98Obj[];

zcfgRet_t zyXGREObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    objIndex_t objIid = {0};
    struct json_object *greObj = NULL;
    struct json_object *paramValue = NULL;
    tr98Parameter_t *paramList = NULL;

    printf("%s : Enter\n", __FUNCTION__);

    IID_INIT(objIid);
    if((ret = feObjJsonGet(RDM_OID_GRE, &objIid, &greObj, updateFlag)) != ZCFG_SUCCESS) {
        return ret;
    }

    /* Fill up tr98 object from related tr181 objects */
    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;
    while(paramList->name != NULL) {
        /* Write parameter value from tr181 objects to tr98 object */
        paramValue = json_object_object_get(greObj, paramList->name);
        if(paramValue != NULL) {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }

        /* Not defined in tr181, give it a default value */
        printf("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    zcfgFeJsonObjFree(greObj);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyXGRETunnelObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    objIndex_t greTunnelIid = {0};
    objIndex_t greIfaceIid = {0};
    objIndex_t brPortIid = {0};
    struct json_object *greTunnelObj = NULL;
    struct json_object *greIfaceObj = NULL;
    struct json_object *brPortObj = NULL;
    struct json_object *paramValue = NULL;
    tr98Parameter_t *paramList = NULL;
    char mapping98ObjName[128] = {0};
    char result[1024] = {0};
    const char *strPtr = NULL;
    char *strBuf = NULL;
    char *strToken = NULL;
    char *tmp = NULL;

    printf("%s : Enter\n", __FUNCTION__);

    IID_INIT(greTunnelIid);
    greTunnelIid.level = 1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.X_GRE.Tunnel.%hhu", &greTunnelIid.idx[0]);
    if((ret = zcfgFeObjJsonGet(RDM_OID_GRE_TUNNEL, &greTunnelIid, &greTunnelObj)) != ZCFG_SUCCESS) {
        printf("%s : Get RDM_OID_GRE_TUNNEL fail, ret = %d\n", __FUNCTION__, ret);
        return ret;
    }

    /* Fill up tr98 object from related tr181 objects */
    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;
    while(paramList->name != NULL) {
        if(!strcmp(paramList->name, "BoundWanInterfaces")) {
            /* Collect bound WAN interfaces and convert to TR-098 parameter name */
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_BoundIfList");
            if(paramValue != NULL) {
                memset(result, 0, sizeof(result));
                strPtr = json_object_get_string(paramValue);
                if(strlen(strPtr) > 0) {
                    strBuf = strdup(strPtr);
                    strToken = strtok_r(strBuf, ",", &tmp);
                    while(strToken != NULL) {
                        memset(mapping98ObjName, 0, sizeof(mapping98ObjName));
                        if(zcfgFe181To98MappingNameGet(strToken, mapping98ObjName) == ZCFG_SUCCESS)
                            sprintf(result + strlen(result), "%s,", mapping98ObjName);
                        else
                            printf("%s : zcfgFe181To98MappingNameGet fail\n", __FUNCTION__);
                        strToken = strtok_r(NULL, ",", &tmp);
                    }
                    if(strlen(result) > 0)
                        result[strlen(result)-1] = '\0';
                    free(strBuf);
                }

                json_object_object_add(*tr98Jobj, "BoundWanInterfaces", json_object_new_string(result));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "BoundLanInterfaces")) {
            /* Collect bound LAN interfaces and convert to TR-098 parameter name */
            memset(result, 0, sizeof(result));
            IID_INIT(greIfaceIid);
            while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj) == ZCFG_SUCCESS) {
                paramValue = json_object_object_get(greIfaceObj, "LowerLayers");
                if(paramValue != NULL) {
                    strPtr = json_object_get_string(paramValue);
                    if(strlen(strPtr) > 0) {
                        IID_INIT(brPortIid);
                        brPortIid.level = 2;
                        sscanf(strPtr, "Bridging.Bridge.%hhu.Port.%hhu", &brPortIid.idx[0], &brPortIid.idx[1]);
                        if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &brPortIid, &brPortObj) == ZCFG_SUCCESS) {
                            strPtr = json_object_get_string(json_object_object_get(brPortObj, "LowerLayers"));
                            memset(mapping98ObjName, 0, sizeof(mapping98ObjName));
                            if(zcfgFe181To98MappingNameGet(strPtr, mapping98ObjName) == ZCFG_SUCCESS)
                                sprintf(result + strlen(result), "%s,", mapping98ObjName);
                            else
                                printf("%s : zcfgFe181To98MappingNameGet fail\n", __FUNCTION__);
                            zcfgFeJsonObjFree(brPortObj);
                        }
                    }
                }
                zcfgFeJsonObjFree(greIfaceObj);
            }
            if(strlen(result) > 0)
                result[strlen(result)-1] = '\0';

            json_object_object_add(*tr98Jobj, "BoundLanInterfaces", json_object_new_string(result));
            paramList++;
            continue;
        }

        if(!strcmp(paramList->name, "VlanId")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_VLANID");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "VlanId", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "UpRate")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_UpRate");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "UpRate", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "DownRate")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DownRate");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "DownRate", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "DSCP")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DSCP");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "DSCP", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        /*Write parameter value from tr181 objects to tr98 object*/
        paramValue = json_object_object_get(greTunnelObj, paramList->name);
        if(paramValue != NULL) {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }

        /*Not defined in tr181, give it a default value*/
        printf("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    zcfgFeJsonObjFree(greTunnelObj);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyXGRETunnelObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    objIndex_t greTunnelIid = {0};
    objIndex_t greIfaceIid = {0};
    objIndex_t brPortIid = {0};
    struct json_object *greTunnelObj = NULL;
    struct json_object *greIfaceObj = NULL;
    struct json_object *brPortObj = NULL;
    struct json_object *tmpObj = NULL;
    struct json_object *paramValue = NULL;
    struct json_object *tr181ParamValue = NULL;
    tr98Parameter_t *paramList = NULL;
    char mapping181ObjName[128] = {0};
    char tmpResult[1024] = {0};
    char result[1024] = {0};
    const char *strPtr = NULL;
    char *strBuf = NULL;
    char *strToken = NULL;
    char *tmp = NULL;
    bool found = false;

    printf("%s : Enter\n", __FUNCTION__);

    IID_INIT(greTunnelIid);
    greTunnelIid.level = 1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.X_GRE.Tunnel.%hhu", &greTunnelIid.idx[0]);

    /*Get related tr181 objects*/
    if((ret = zcfgFeObjJsonGet(RDM_OID_GRE_TUNNEL, &greTunnelIid, &greTunnelObj)) != ZCFG_SUCCESS) {
        printf("%s: fail to get GRE tunnel obj codec object ret=%d\n", __FUNCTION__, ret);
        return ret;
    }

    if(multiJobj) {
        tmpObj = greTunnelObj;
        greTunnelObj = NULL;
        greTunnelObj = zcfgFeJsonMultiObjAppend(RDM_OID_GRE_TUNNEL, &greTunnelIid, multiJobj, tmpObj);
    }

    paramList = tr98Obj[handler].parameter;
    while(paramList->name != NULL) {
        if(!strcmp(paramList->name, "BoundWanInterfaces")) {
            /* Convert to TR-181 parameter name and validate it */
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            tr181ParamValue = json_object_object_get(greTunnelObj, "X_ZYXEL_BoundIfList");
            if(paramValue != NULL && tr181ParamValue != NULL) {
                strPtr = json_object_get_string(paramValue);
                memset(result, 0, sizeof(result));
                if(strlen(strPtr) > 0) {
                    strBuf = strdup(strPtr);
                    strToken = strtok_r(strBuf, ",", &tmp);
                    while(strToken != NULL) {
                        memset(mapping181ObjName, 0, sizeof(mapping181ObjName));
                        if(zcfgFe98To181MappingNameGet(strToken, mapping181ObjName) == ZCFG_SUCCESS
                            && (strstr(mapping181ObjName, "IP.Interface.") != NULL
                            || (strstr(mapping181ObjName, "PPP.Interface.") != NULL
                            && zcfgFeTr181IfaceStackHigherLayerGet(mapping181ObjName, mapping181ObjName) == ZCFG_SUCCESS))) {
                            sprintf(result + strlen(result), "%s,", mapping181ObjName);
                        }
                        else {
                            printf("%s : zcfgFe98To181MappingNameGet fail or invalid parameter value for %s\n", __FUNCTION__, paramList->name);
                            zcfgFeJsonObjFree(greTunnelObj);
                            free(strBuf);
                            return ZCFG_INVALID_PARAM_VALUE;
                        }
                        strToken = strtok_r(NULL, ",", &tmp);
                    }
                    if(strlen(result) > 0)
                        result[strlen(result)-1] = '\0';
                    free(strBuf);
                }

                json_object_object_add(greTunnelObj, "X_ZYXEL_BoundIfList", json_object_new_string(result));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "BoundLanInterfaces")) {
            /* Convert to TR-181 parameter name and validate it */
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL) {
                strPtr = json_object_get_string(paramValue);
                memset(tmpResult, 0, sizeof(tmpResult));
                memset(result, 0, sizeof(result));
                if(strlen(strPtr) > 0) {
                    strBuf = strdup(strPtr);
                    strToken = strtok_r(strBuf, ",", &tmp);
                    while(strToken != NULL) {
                        memset(mapping181ObjName, 0, sizeof(mapping181ObjName));
                        if(zcfgFe98To181MappingNameGet(strToken, mapping181ObjName) == ZCFG_SUCCESS
                            && strstr(mapping181ObjName, "WiFi.SSID.") != NULL) {
                            sprintf(tmpResult + strlen(tmpResult), "%s,", mapping181ObjName);
                        }
                        else {
                            printf("%s : zcfgFe98To181MappingNameGet fail or invalid parameter value for %s\n", __FUNCTION__, paramList->name);
                            zcfgFeJsonObjFree(greTunnelObj);
                            free(strBuf);
                            return ZCFG_INVALID_PARAM_VALUE;
                        }
                        strToken = strtok_r(NULL, ",", &tmp);
                    }
                    if(strlen(tmpResult) > 0)
                        tmpResult[strlen(tmpResult)-1] = '\0';
                    free(strBuf);

                    /* Convert WiFi.SSID.i to Bridging.Bridge.i.Port.i */
                    IID_INIT(brPortIid);
                    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &brPortIid, &brPortObj) == ZCFG_SUCCESS) {
                        strPtr = json_object_get_string(json_object_object_get(brPortObj, "LowerLayers"));
                        if(!json_object_get_boolean(json_object_object_get(brPortObj, "ManagementPort"))
                            && strPtr != NULL && strlen(strPtr) > 0 && strstr(tmpResult, strPtr) != NULL) {
                            sprintf(result + strlen(result), "Bridging.Bridge.%hhu.Port.%hhu,", brPortIid.idx[0], brPortIid.idx[1]);
                        }
                        zcfgFeJsonObjFree(brPortObj);
                    }
                    if(strlen(result) > 0)
                        result[strlen(result)-1] = '\0';

                    /* Delete the interfaces NOT in new BoundLanInterfaces */
                    IID_INIT(greIfaceIid);
                    while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj) == ZCFG_SUCCESS) {
                        strPtr = json_object_get_string(json_object_object_get(greIfaceObj, "LowerLayers"));
                        if(strPtr == NULL || strlen(strPtr) == 0 || strstr(result, strPtr) == NULL) {
                            zcfgFeObjJsonDel(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, NULL);
                        }
                        zcfgFeJsonObjFree(greIfaceObj);
                    }

                    /* Add new interfaces */
                    strToken = strtok_r(result, ",", &tmp);
                    while(strToken != NULL) {
                        found = false;
                        IID_INIT(greIfaceIid);
                        while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj) == ZCFG_SUCCESS) {
                            strPtr = json_object_get_string(json_object_object_get(greIfaceObj, "LowerLayers"));
                            if(strPtr != NULL && !strcmp(strToken, strPtr)) {
                                found = true;
                                zcfgFeJsonObjFree(greIfaceObj);
                                break;
                            }
                            zcfgFeJsonObjFree(greIfaceObj);
                        }

                        if(!found) {
                            IID_INIT(greIfaceIid);
                            greIfaceIid.level = 1;
                            greIfaceIid.idx[0] = greTunnelIid.idx[0];
                            if(zcfgFeObjJsonAdd(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, NULL) != ZCFG_SUCCESS)
                                break;

                            if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, &greIfaceObj) != ZCFG_SUCCESS)
                                break;

                            json_object_object_add(greIfaceObj, "Enable", json_object_new_boolean(true));
                            json_object_object_add(greIfaceObj, "LowerLayers", json_object_new_string(strToken));

                            zcfgFeObjJsonSet(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, greIfaceObj, NULL);
                            zcfgFeJsonObjFree(greIfaceObj);
                        }
                        strToken = strtok_r(NULL, ",", &tmp);
                    }
                }

                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "VlanId")) {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL) {
                tr181ParamValue = json_object_object_get(greTunnelObj, "X_ZYXEL_VLANID");
                if(tr181ParamValue != NULL) {
                    json_object_object_add(greTunnelObj, "X_ZYXEL_VLANID", JSON_OBJ_COPY(paramValue));
                    paramList++;
                    continue;
                }
            }
        }

        if(!strcmp(paramList->name, "UpRate")) {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL) {
                tr181ParamValue = json_object_object_get(greTunnelObj, "X_ZYXEL_UpRate");
                if(tr181ParamValue != NULL) {
                    json_object_object_add(greTunnelObj, "X_ZYXEL_UpRate", JSON_OBJ_COPY(paramValue));
                    paramList++;
                    continue;
                }
            }
        }

        if(!strcmp(paramList->name, "DownRate")) {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL) {
                tr181ParamValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DownRate");
                if(tr181ParamValue != NULL) {
                    json_object_object_add(greTunnelObj, "X_ZYXEL_DownRate", JSON_OBJ_COPY(paramValue));
                    paramList++;
                    continue;
                }
            }
        }

        if(!strcmp(paramList->name, "DSCP")) {
            paramValue = json_object_object_get(tr98Jobj, paramList->name);
            if(paramValue != NULL) {
                tr181ParamValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DSCP");
                if(tr181ParamValue != NULL) {
                    json_object_object_add(greTunnelObj, "X_ZYXEL_DSCP", JSON_OBJ_COPY(paramValue));
                    paramList++;
                    continue;
                }
            }
        }

        /*Write new parameter value from tr98 object to tr181 objects*/
        paramValue = json_object_object_get(tr98Jobj, paramList->name);
        if(paramValue != NULL) {
            tr181ParamValue = json_object_object_get(greTunnelObj, paramList->name);
            if(tr181ParamValue != NULL) {
                json_object_object_add(greTunnelObj, paramList->name, JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }
        printf("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    if(multiJobj) {
        zcfgFeJsonObjFree(tmpObj);
    }
    else {
        if((ret = zcfgFeObjJsonSet(RDM_OID_GRE_TUNNEL, &greTunnelIid, greTunnelObj, NULL)) != ZCFG_SUCCESS ) {
            zcfgFeJsonObjFree(greTunnelObj);
            return ret;
        }
        zcfgFeJsonObjFree(greTunnelObj);
    }

    return ZCFG_SUCCESS;
}

zcfgRet_t zyXGRETunnelObjAdd(char *tr98FullPathName, int *idx)
{
    zcfgRet_t ret;
    objIndex_t objIid;

    printf("%s : Enter\n", __FUNCTION__);

    IID_INIT(objIid);
    ret = zcfgFeObjStructAdd(RDM_OID_GRE_TUNNEL, &objIid, NULL);
    *idx = objIid.idx[0];

    return ret;
}

zcfgRet_t zyXGRETunnelObjDel(char *tr98FullPathName)
{
    objIndex_t objIid;

    printf("%s : Enter\n", __FUNCTION__);

    IID_INIT(objIid);
    objIid.level = 1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.X_GRE.Tunnel.%hhu", &objIid.idx[0]);

    return zcfgFeObjStructDel(RDM_OID_GRE_TUNNEL, &objIid, NULL);
}

zcfgRet_t zyXGRETunnelStatsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
    zcfgRet_t ret;
    objIndex_t objIid;
    struct json_object *greTunnelObj = NULL;
    struct json_object *paramValue = NULL;
    tr98Parameter_t *paramList = NULL;
    //printf("%s enter\n",__FUNCTION__);

    IID_INIT(objIid);
    objIid.level = 1;
    sscanf(tr98FullPathName, "InternetGatewayDevice.X_GRE.Tunnel.%hhu.Stats", &objIid.idx[0]);

    if((ret = zcfgFeObjJsonGet(RDM_OID_GRE_TUNNEL_STAT, &objIid, &greTunnelObj)) != ZCFG_SUCCESS) {
        printf("%s: fail to get GRE tunnel stat obj codec object ret=%d\n", __FUNCTION__, ret);
        return ret;
    }

    /*fill up tr98 object from related tr181 objects*/
    *tr98Jobj = json_object_new_object();
    paramList = tr98Obj[handler].parameter;
    while(paramList->name != NULL) {
        if(!strcmp(paramList->name, "DiscardPacketsSent")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DiscardPacketsSent");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "DiscardPacketsSent", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        if(!strcmp(paramList->name, "DiscardPacketsReceived")) {
            paramValue = json_object_object_get(greTunnelObj, "X_ZYXEL_DiscardPacketsReceived");
            if(paramValue != NULL) {
                json_object_object_add(*tr98Jobj, "DiscardPacketsReceived", JSON_OBJ_COPY(paramValue));
                paramList++;
                continue;
            }
        }

        /*Write parameter value from tr181 objects to tr98 object*/
        paramValue = json_object_object_get(greTunnelObj, paramList->name);
        if(paramValue != NULL) {
            json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
            paramList++;
            continue;
        }

        /*Not defined in tr181, give it a default value*/
        printf("Can't find parameter %s in TR181\n", paramList->name);
        paramList++;
    }

    zcfgFeJsonObjFree(greTunnelObj);

    return ZCFG_SUCCESS;
}

