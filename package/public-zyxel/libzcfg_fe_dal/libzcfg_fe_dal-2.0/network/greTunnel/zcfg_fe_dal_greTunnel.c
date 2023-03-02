
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_msg.h"

dal_param_t GreTunnel_param[] = {
    {"Index",           dalType_int,                0,  0,          NULL,   NULL,   dal_Edit|dal_Delete},
    {"Enable",          dalType_boolean,            0,  0,          NULL,   NULL,   dal_Add},
    {"VlanId",          dalType_int,                0,  4096,       NULL,   NULL,   0},
    {"RemoteEndpoints", dalType_v4Addr,             0,  0,          NULL,   NULL,   dal_Add},
    {"WanIfList",       dalType_IntfGrpWanList,     0,  0,          NULL,   NULL,   dal_Add},
    {"LanIfList",       dalType_IntfGrpLanPortList, 0,  0,          NULL,   NULL,	dal_Add},
    {"UpRate",          dalType_int,                0,  1024000,    NULL,   NULL,   0},
    {"DownRate",        dalType_int,                0,  1024000,    NULL,   NULL,   0},
    {"Dscp",            dalType_int,                0,  63,         NULL,   NULL,   0},
	{"VlanGroup",			dalType_string, 0, 0, NULL,NULL,	dal_Add},
    {NULL,              0,                          0,  0,          NULL,   NULL,   0}
};

void zcfgFeDalShowGreTunnel(struct json_object *Jarray)
{
    struct json_object *obj;
    char bandwidth[64] = {0};
    int i = 0;
    int len = 0;
    int upRate = 0;
    int downRate = 0;

    if(json_object_get_type(Jarray) != json_type_array) {
        printf("wrong Jobj format!\n");
        return;
    }

    printf("%-6s %-12s %-6s %-35s %-55s %-35s %-35s\n",
            "Index", "Enable", "VLAN", "WAN Interface", "LAN Interface", "RemoteEndpoints", "Bandwidth");

    len = json_object_array_length(Jarray);
    for(i=0; i < len; i++) {
        if((obj = json_object_array_get_idx(Jarray, i)) != NULL) {
            upRate = json_object_get_int(json_object_object_get(obj, "UpRate"));
            downRate = json_object_get_int(json_object_object_get(obj, "DownRate"));
            sprintf(bandwidth, "%dkbps/%dkbps", upRate, downRate);

            printf("%-6s %-12s %-6s %-35s %-55s %-35s %-35s\n",
                json_object_get_string(json_object_object_get(obj, "Index")),
                json_object_get_string(json_object_object_get(obj, "Enable")),
                json_object_get_string(json_object_object_get(obj, "VlanId")),
                json_object_get_string(json_object_object_get(obj, "WanIfList")),
                json_object_get_string(json_object_object_get(obj, "LanIfList")),
                json_object_get_string(json_object_object_get(obj, "RemoteEndpoints")),
                bandwidth);
        }
    }
}

zcfgRet_t zcfgFeDalGreTunnelAdd(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *greTunnelObj = NULL;
    struct json_object *greIfaceObj = NULL;
    objIndex_t greTunnelIid = {0};
    objIndex_t greIfaceIid = {0};
    const char *lanIfList = NULL;
    char *lanIface = NULL;
    char *lanIfListfStr = NULL;
    char *tmp = NULL;

    /* Add RDM_OID_GRE_TUNNEL */
    /* ================================================================================ */
    IID_INIT(greTunnelIid);
    if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_GRE_TUNNEL, &greTunnelIid, NULL)) != ZCFG_SUCCESS)
        return ret;

    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL, &greTunnelIid, &greTunnelObj)) != ZCFG_SUCCESS)
        return ret;

    json_object_object_add(greTunnelObj, "Enable", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enable")));
    if(NULL != json_object_object_get(Jobj, "VlanId"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_VLANID", JSON_OBJ_COPY(json_object_object_get(Jobj, "VlanId")));
	json_object_object_add(greTunnelObj, "RemoteEndpoints", JSON_OBJ_COPY(json_object_object_get(Jobj, "RemoteEndpoints")));
    json_object_object_add(greTunnelObj, "X_ZYXEL_BoundIfList", JSON_OBJ_COPY(json_object_object_get(Jobj, "WanIfList")));
    if(NULL != json_object_object_get(Jobj, "UpRate"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_UpRate", JSON_OBJ_COPY(json_object_object_get(Jobj, "UpRate")));
    if(NULL != json_object_object_get(Jobj, "DownRate"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_DownRate", JSON_OBJ_COPY(json_object_object_get(Jobj, "DownRate")));
    if(NULL != json_object_object_get(Jobj, "Dscp"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_DSCP", JSON_OBJ_COPY(json_object_object_get(Jobj, "Dscp")));
	if(NULL != json_object_object_get(Jobj, "VlanGroup"))
		json_object_object_add(greTunnelObj, "X_ZYXEL_VLANGROUP", JSON_OBJ_COPY(json_object_object_get(Jobj, "VlanGroup")));

    if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_GRE_TUNNEL, &greTunnelIid, greTunnelObj, NULL)) != ZCFG_SUCCESS) {
        zcfgFeJsonObjFree(greTunnelObj);
        return ret;
    }
    zcfgFeJsonObjFree(greTunnelObj);
    /* ================================================================================ */

    /* Add RDM_OID_GRE_TUNNEL_IFACE */
    /* ================================================================================ */
    lanIfList = json_object_get_string(json_object_object_get(Jobj, "LanIfList"));
    lanIfListfStr = strdup(lanIfList);
    lanIface = strtok_r(lanIfListfStr, ",", &tmp);
    while(lanIface != NULL) {
        IID_INIT(greIfaceIid);
        greIfaceIid.level = 1;
        greIfaceIid.idx[0] = greTunnelIid.idx[0];
        if((ret = zcfgFeObjJsonAdd(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, NULL)) != ZCFG_SUCCESS)
            break;

        if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, &greIfaceObj)) != ZCFG_SUCCESS)
            break;

        json_object_object_add(greIfaceObj, "Enable", json_object_new_boolean(true));
        json_object_object_add(greIfaceObj, "LowerLayers", json_object_new_string(lanIface));

        if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, greIfaceObj, NULL)) != ZCFG_SUCCESS) {
            zcfgFeJsonObjFree(greIfaceObj);
            break;
        }
        zcfgFeJsonObjFree(greIfaceObj);
        lanIface = strtok_r(NULL, ",", &tmp);
    }
    free(lanIfListfStr);
    /* ================================================================================ */

    return ret;
}


zcfgRet_t zcfgFeDalGreTunnelEdit(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *greTunnelObj = NULL;
    struct json_object *greIfaceObj = NULL;
    objIndex_t greTunnelIid = {0};
    objIndex_t greIfaceIid = {0};
    bool found = false;
    const char *lanIfList = NULL;
    char *lanIface = NULL;
    char *lanIfListfStr = NULL;
    char *tmp = NULL;
    int index = 0;

    index = json_object_get_int(json_object_object_get(Jobj, "Index"));
    if((ret = convertIndextoIid(index, &greTunnelIid, RDM_OID_GRE_TUNNEL, NULL, NULL, replyMsg)) != ZCFG_SUCCESS)
        return ret;

    /* Set RDM_OID_GRE_TUNNEL */
    /* ================================================================================ */
    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL, &greTunnelIid, &greTunnelObj)) != ZCFG_SUCCESS)
        return ret;

    if(NULL != json_object_object_get(Jobj, "Enable"))
        json_object_object_add(greTunnelObj, "Enable", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enable")));
    if(NULL != json_object_object_get(Jobj, "VlanId"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_VLANID", JSON_OBJ_COPY(json_object_object_get(Jobj, "VlanId")));
    if(NULL != json_object_object_get(Jobj, "RemoteEndpoints"))
        json_object_object_add(greTunnelObj, "RemoteEndpoints", JSON_OBJ_COPY(json_object_object_get(Jobj, "RemoteEndpoints")));
    if(NULL != json_object_object_get(Jobj, "WanIfList"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_BoundIfList", JSON_OBJ_COPY(json_object_object_get(Jobj, "WanIfList")));
    if(NULL != json_object_object_get(Jobj, "UpRate"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_UpRate", JSON_OBJ_COPY(json_object_object_get(Jobj, "UpRate")));
    if(NULL != json_object_object_get(Jobj, "DownRate"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_DownRate", JSON_OBJ_COPY(json_object_object_get(Jobj, "DownRate")));
    if(NULL != json_object_object_get(Jobj, "Dscp"))
        json_object_object_add(greTunnelObj, "X_ZYXEL_DSCP", JSON_OBJ_COPY(json_object_object_get(Jobj, "Dscp")));
	if(NULL != json_object_object_get(Jobj, "VlanGroup"))
		json_object_object_add(greTunnelObj, "X_ZYXEL_VLANGROUP", JSON_OBJ_COPY(json_object_object_get(Jobj, "VlanGroup")));

    if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_GRE_TUNNEL, &greTunnelIid, greTunnelObj, NULL)) != ZCFG_SUCCESS) {
        zcfgFeJsonObjFree(greTunnelObj);
        return ret;
    }
    zcfgFeJsonObjFree(greTunnelObj);
    /* ================================================================================ */

    /* Set RDM_OID_GRE_TUNNEL_IFACE */
    /* ================================================================================ */
    if(NULL == json_object_object_get(Jobj, "LanIfList"))
        return ZCFG_SUCCESS;

    lanIfList = json_object_get_string(json_object_object_get(Jobj, "LanIfList"));

    /* 1. Delete the interfaces NOT in new LanIfList */
    IID_INIT(greIfaceIid);
    while((ret = zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj)) == ZCFG_SUCCESS) {
        if(strstr(lanIfList, json_object_get_string(json_object_object_get(greIfaceObj, "LowerLayers"))) == NULL) {
            zcfgFeObjJsonDel(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, NULL);
        }
        zcfgFeJsonObjFree(greIfaceObj);
    }

    /* 2. Add new interfaces */
    lanIfListfStr = strdup(lanIfList);
    lanIface = strtok_r(lanIfListfStr, ",", &tmp);
    while(lanIface != NULL) {
        found = false;
        IID_INIT(greIfaceIid);
        while((ret = zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj)) == ZCFG_SUCCESS) {
            if(!strcmp(lanIface, json_object_get_string(json_object_object_get(greIfaceObj, "LowerLayers")))) {
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
            if((ret = zcfgFeObjJsonAdd(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, NULL)) != ZCFG_SUCCESS)
                break;

            if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, &greIfaceObj)) != ZCFG_SUCCESS)
                break;

            json_object_object_add(greIfaceObj, "Enable", json_object_new_boolean(true));
            json_object_object_add(greIfaceObj, "LowerLayers", json_object_new_string(lanIface));

            if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_GRE_TUNNEL_IFACE, &greIfaceIid, greIfaceObj, NULL)) != ZCFG_SUCCESS) {
                zcfgFeJsonObjFree(greIfaceObj);
                break;
            }
            zcfgFeJsonObjFree(greIfaceObj);
        }
        lanIface = strtok_r(NULL, ",", &tmp);
    }
    free(lanIfListfStr);
    /* ================================================================================ */

    return ret;
}

zcfgRet_t zcfgFeDalGreTunnelDelete(struct json_object *Jobj, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    objIndex_t greTunnelIid = {0};
    int index = 0;

    index = json_object_get_int(json_object_object_get(Jobj, "Index"));
    if((ret = convertIndextoIid(index, &greTunnelIid, RDM_OID_GRE_TUNNEL, NULL, NULL, replyMsg)) == ZCFG_SUCCESS) {
        zcfgFeObjJsonDel(RDM_OID_GRE_TUNNEL, &greTunnelIid, NULL);
    }

    return ret;
}

zcfgRet_t zcfgFeDalGreTunnelGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    struct json_object *greTunnelObj = NULL;
    struct json_object *greIfaceObj = NULL;
    struct json_object *brPortObj = NULL;
    struct json_object *paramJobj = NULL;
    objIndex_t greTunnelIid = {0};
    objIndex_t greIfaceIid = {0};
    objIndex_t brPortIid = {0};
    char brPortPatch[256] = {0};
    char lanIfList[1024] = {0};
    int count = 1;

    IID_INIT(greTunnelIid);
    while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL, &greTunnelIid, &greTunnelObj) == ZCFG_SUCCESS) {
		
		memset(lanIfList, 0, sizeof(lanIfList));
        /* Collect bound LAN interfaces and convert the path to WiFi.SSID.i */
        IID_INIT(greIfaceIid);
        while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_GRE_TUNNEL_IFACE, &greTunnelIid, &greIfaceIid, &greIfaceObj) == ZCFG_SUCCESS) {
            strcpy(brPortPatch, json_object_get_string(json_object_object_get(greIfaceObj, "LowerLayers")));
            IID_INIT(brPortIid);
            brPortIid.level = 2;
            sscanf(brPortPatch, "Bridging.Bridge.%hhu.Port.%hhu", &brPortIid.idx[0], &brPortIid.idx[1]);
            if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &brPortIid, &brPortObj)) == ZCFG_SUCCESS){
                sprintf(lanIfList + strlen(lanIfList), "%s,", json_object_get_string(json_object_object_get(brPortObj, "LowerLayers")));
                zcfgFeJsonObjFree(brPortObj);
            }
            zcfgFeJsonObjFree(greIfaceObj);
        }
        if(strlen(lanIfList) > 0)
            lanIfList[strlen(lanIfList)-1] = '\0';

        paramJobj = json_object_new_object();
        json_object_object_add(paramJobj, "Index", json_object_new_int(count));
        json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "Enable")));
        json_object_object_add(paramJobj, "VlanId", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_VLANID")));
        json_object_object_add(paramJobj, "RemoteEndpoints", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "RemoteEndpoints")));
        json_object_object_add(paramJobj, "ConnectedRemoteEndpoint", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "ConnectedRemoteEndpoint")));
        json_object_object_add(paramJobj, "WanIfList", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_BoundIfList")));
        json_object_object_add(paramJobj, "LanIfList", json_object_new_string(lanIfList));
        json_object_object_add(paramJobj, "UpRate", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_UpRate")));
        json_object_object_add(paramJobj, "DownRate", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_DownRate")));
        json_object_object_add(paramJobj, "Dscp", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_DSCP")));
		json_object_object_add(paramJobj, "VlanGroup", JSON_OBJ_COPY(json_object_object_get(greTunnelObj, "X_ZYXEL_VLANGROUP")));
        json_object_array_add(Jarray, paramJobj);
        zcfgFeJsonObjFree(greTunnelObj);
        count++;
    }

    return ret;
}

zcfgRet_t zcfgFeDalGreTunnel(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if(!method || !Jobj)
        return ZCFG_INTERNAL_ERROR;

    if(!strcmp(method, "POST")) {
        ret = zcfgFeDalGreTunnelAdd(Jobj, replyMsg);
    }
    else if(!strcmp(method, "PUT")) {
        ret = zcfgFeDalGreTunnelEdit(Jobj, replyMsg);
    }
    else if(!strcmp(method, "DELETE")) {
        ret = zcfgFeDalGreTunnelDelete(Jobj, replyMsg);
    }
    else if(!strcmp(method,"GET")) {
        ret = zcfgFeDalGreTunnelGet(Jobj, Jarray, NULL);
    }

    return ret;
}
