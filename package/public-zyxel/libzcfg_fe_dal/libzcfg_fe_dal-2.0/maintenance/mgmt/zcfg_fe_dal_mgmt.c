
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"
#include "zlog_api.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)
const char hiddenPassword[] = "********" ;

dal_param_t MGMT_param[] =
{
	{"EnableCWMP", dalType_boolean, 0, 0, NULL},
	{"URL", dalType_string, 0, 256, NULL},
	{"Username", dalType_string, 0, 256, NULL},
	{"Password", dalType_string, 0, 256, NULL},	
	{"ConnectionRequestUsername", dalType_string, 0, 256, NULL},	
	{"ConnectionRequestPassword", dalType_string, 0, 256, NULL},	
	{"PeriodicInformEnable", dalType_boolean, 0, 0, NULL},	
	{"PeriodicInformInterval", dalType_int, 30, 2147483647, NULL},
	{"PeriodicInformTime", dalType_string, 0, 256, NULL, NULL, dalcmd_Forbid},
	{"X_ZYXEL_ActiveNotifyUpdateInterval", dalType_int, 0, 0, NULL, NULL, dalcmd_Forbid},
	{"DebugLevelEnable", dalType_int, 0, 0, NULL},
	{"DebugLevelDisable", dalType_int, 0, 0, NULL},
	{"FetureOptionEnable", dalType_int, 0, 0, NULL},
	{"FetureOptionDisable", dalType_int, 0, 0, NULL},
	{"X_ZYXEL_ConnectionRequestPort", dalType_int, 0, 0, NULL},
	{"IPv6_Enable", dalType_boolean, 0, 0, NULL},
	{"IPv4_Enable", dalType_boolean, 0, 0, NULL},
	{"DisplaySOAP", dalType_boolean, 0, 0, NULL},
	{"EnableAuthentication", dalType_boolean, 0, 0, NULL, NULL},	
	{"BoundInterfaceMode", dalType_string, 0, 256, NULL, "Any_WAN|Multi_WAN", hidden_extender_dalcmd_param},
	{"BoundInterfaceList", dalType_MultiRtWanWWan, 0, 256, NULL, NULL, hidden_extender_dalcmd_param},
	{"ConnectionRequestURL", dalType_string, 0, 0, NULL, NULL, dalcmd_Forbid},
	{"CheckCert", dalType_boolean, 0, 0,NULL},
	{"DataModelSpec", dalType_string, 0, 256, NULL, "TR-098|TR-181"},
	{"Certificate", dalType_LocalCert, 0, 256, NULL},
	{"X_ZYXEL_SerialNumber_Type", dalType_int, 0, 1, NULL},
#ifdef ZYXEL_TR69_SUPPLEMENTAL_ACS
	{"X_ZYXEL_Supplemental_EnableCWMP", dalType_boolean, 0, 0, NULL},
	{"X_ZYXEL_Supplemental_URL", dalType_string, 0, 256, NULL},
	{"X_ZYXEL_Supplemental_Username", dalType_string, 0, 256, NULL},
	{"X_ZYXEL_Supplemental_Password", dalType_string, 0, 256, NULL},
	{"X_ZYXEL_Supplemental_ConnReqURL", dalType_string, 0, 0, NULL, NULL, dalcmd_Forbid},
	{"X_ZYXEL_Supplemental_ConnReqUsername", dalType_string, 0, 256, NULL},
	{"X_ZYXEL_Supplemental_ConnReqPassword", dalType_string, 0, 256, NULL},
	{"X_ZYXEL_Supplemental_ConnReqPort", dalType_int, 0, 0, NULL},
#endif
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
    {"EnableXmppConnReq", dalType_boolean, 0, 0, NULL},
    {"XmppConnUsername", dalType_string, 0, 256, NULL},
    {"XmppConnPassword", dalType_string, 0, 256, NULL},
    {"XmppConnDomain", dalType_string, 0, 64, NULL},
    {"XmppConnResource", dalType_string, 0, 64, NULL},
    {"XmppServerAddress", dalType_string, 0, 256, NULL},
    {"XmppServerPort", dalType_int, 0, 0, NULL},
#endif
	{NULL, 0, 0, 0, NULL}
};

void zcfgFeDalShowMgmt(struct json_object *Jarray){
	struct json_object *Jobj;
	const char *mode = NULL;
	//const char *Spec = NULL;
	char proto[32] = {0};
	bool v4Enable, v6Enable;
	const char *conUser, *conPass, *cert;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	Jobj = json_object_array_get_idx(Jarray, 0);
	
	v4Enable = json_object_get_boolean(json_object_object_get(Jobj, "IPv4_Enable"));
	v6Enable = json_object_get_boolean(json_object_object_get(Jobj, "IPv6_Enable"));
	if(v4Enable && v6Enable)
		strcpy(proto, "Auto Select");
	else if(v6Enable)
		strcpy(proto, "TR069 on IPv6 Only");
	else
		strcpy(proto, "TR069 on IPv4 Only");

	mode = json_object_get_string(json_object_object_get(Jobj, "BoundInterfaceMode"));
	conUser = json_object_get_string(json_object_object_get(Jobj, "ConnectionRequestUsername"));
	conPass = json_object_get_string(json_object_object_get(Jobj, "ConnectionRequestPassword"));
	cert = json_object_get_string(json_object_object_get(Jobj, "Certificate"));
	//Spec = json_object_get_string(json_object_object_get(Jobj, "DataModelSpec"));
	
	printf("TR-069 Client Information:\n");
	printf("%-45s %s\n", "CWMP Active", json_object_get_string(json_object_object_get(Jobj, "EnableCWMP")));
	printf("%-45s %s\n", "Inform", json_object_get_string(json_object_object_get(Jobj, "PeriodicInformEnable")));
	printf("%-45s %s\n", "Inform Interval", json_object_get_string(json_object_object_get(Jobj, "PeriodicInformInterval")));
	printf("%-45s %s\n", "IP Protocol", proto);
	printf("%-45s %s\n", "ACS URL", json_object_get_string(json_object_object_get(Jobj, "URL")));
	printf("%-45s %s\n", "ACS User Name", json_object_get_string(json_object_object_get(Jobj, "Username")));
#ifdef IAAAA_CUSTOMIZATION
	printf("%-45s %s\n", "ACS Password", json_object_get_string(json_object_object_get(Jobj, "Password")));
#else
	printf("%-45s %s\n", "ACS Password", "********");
#endif
#ifndef ZYXEL_OPAL_EXTENDER
	printf("%-45s %s\n", "WAN Interface Used by TR-069 Client:", json_object_get_string(json_object_object_get(Jobj, "BoundInterfaceMode")));
	printf("%-45s %s\n", "", !strcmp(mode, "Any_WAN")?"":json_object_get_string(json_object_object_get(Jobj, "BoundInterfaceList")));
#endif
	printf("%-45s %s\n", "Display SOAP Messages on Serial Console", json_object_get_string(json_object_object_get(Jobj, "DisplaySOAP")));
	printf("%-45s %s\n", "Connection Request Authentication", (*conUser | *conPass)?"true":"false");
	printf("%-45s %s\n", "Connection Request User Name", (*conUser | *conPass)?json_object_get_string(json_object_object_get(Jobj, "ConnectionRequestUsername")):"");
	printf("%-45s %s\n", "Connection Request Password", (*conUser | *conPass)?json_object_get_string(json_object_object_get(Jobj, "ConnectionRequestPassword")):"");
	printf("%-45s %s\n", "Connection Request URL", json_object_get_string(json_object_object_get(Jobj, "ConnectionRequestURL")));
	printf("%-45s %s\n", "Local Certificate Used by TR-069 Client", (*cert == '0')?"":cert);
	printf("%-45s %s\n", "Data Model Tyep", !strcmp(mode, "TR-098")?"":json_object_get_string(json_object_object_get(Jobj, "DataModelSpec")));
	
	
}
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
static zcfgRet_t zcfgFeDalXmppInfoGet(struct json_object **xmppConnJobj, struct json_object **xmppConnSrvJobj, objIndex_t *xmppConnObjIid, objIndex_t *xmppConnSrvObjIid, const char * connPath)
{
    zcfgRet_t               ret = ZCFG_SUCCESS;
    bool                    found = false;
    struct json_object      *tmpObj = NULL;

    IID_INIT(*xmppConnObjIid);
    IID_INIT(*xmppConnSrvObjIid);

    if (connPath)
    {
        if(strstr(connPath, "Device.XMPP.Connection") != NULL)
        {
            sscanf(connPath,"Device.XMPP.Connection.%hhu", &xmppConnObjIid->idx[0]);
            sscanf(connPath,"Device.XMPP.Connection.%hhu", &xmppConnSrvObjIid->idx[0]);
            found = true;
        }
        else if(strstr(connPath, "XMPP.Connection") != NULL)
        {
            sscanf(connPath,"XMPP.Connection.%hhu", &xmppConnObjIid->idx[0]);
            sscanf(connPath,"XMPP.Connection.%hhu", &xmppConnSrvObjIid->idx[0]);
            found = true;
        }
    }

    if (!found)
    {
        xmppConnObjIid->idx[0] = 1;
        xmppConnSrvObjIid->idx[0] = 1;
    }

    /* Get XMPP connection object*/
    xmppConnObjIid->level = 1;
    //xmppConnObjIid->idx[0] = 1;
    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_XMPP_CONN, xmppConnObjIid, &tmpObj)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("RDM_OID_XMPP_CONN retrieve fail");
        return ret;
    }
    *xmppConnJobj = tmpObj;

    /* Get XMPP server object*/
    xmppConnSrvObjIid->level = 2;
    //xmppConnSrvObjIid->idx[0] = 1;
    xmppConnSrvObjIid->idx[1] = 1;
    if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_XMPP_CONN_SRV, xmppConnSrvObjIid, &tmpObj)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("RDM_OID_XMPP_CONN_SRV retrieve fail");
        return ret;
    }
    *xmppConnSrvJobj = tmpObj;

    return ret;
}
#endif
static zcfgRet_t zcfgFeDalMgmtGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *mgmtJobj = NULL;
	struct json_object *pramJobj = NULL;
	bool enable;
	int intVal;
	const char *str;
	int n = 0;
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
    struct json_object  *xmppConnJobj = NULL;
    struct json_object  *xmppConnSrvJobj = NULL;
    objIndex_t      xmppConnObjIid = {0};
    objIndex_t      xmppConnSrvObjIid = {0};
#endif

	paraName = (const char *)MGMT_param[n++].paraName;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, &mgmtJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}
	
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
	str = json_object_get_string(json_object_object_get(mgmtJobj, "ConnReqXMPPConnection"));
	if ((ret = zcfgFeDalXmppInfoGet(&xmppConnJobj, &xmppConnSrvJobj, &xmppConnObjIid, &xmppConnSrvObjIid, str)) != ZCFG_SUCCESS)
	{
	    return ret;
	}
#endif

	pramJobj = json_object_new_object();	
	while(paraName) {
		if(!strcmp(paraName, "DebugLevelEnable")) {
			intVal = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_DebugLevel"));
			json_object_object_add(pramJobj, paraName, json_object_new_int(intVal));
		}
		else if(!strcmp(paraName, "DebugLevelDisable")) {
			//do nothing
		}
		else if(!strcmp(paraName, "FetureOptionEnable")) {	
			intVal = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_Feature"));
			json_object_object_add(pramJobj, paraName, json_object_new_int(intVal));
		}
		else if(!strcmp(paraName, "X_ZYXEL_ConnectionRequestPort")) {	
			intVal = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_ConnectionRequestPort"));
			json_object_object_add(pramJobj, paraName, json_object_new_int(intVal));
		}
		else if(!strcmp(paraName, "FetureOptionDisable")) {
			//do nothing			
		}
		else if(!strcmp(paraName, "IPv6_Enable")) {
			enable = json_object_get_boolean(json_object_object_get(mgmtJobj, "X_ZYXEL_IPv6_Protocol"));
			json_object_object_add(pramJobj, paraName, json_object_new_boolean(enable));
		}
		else if(!strcmp(paraName, "IPv4_Enable")) {
			enable = json_object_get_boolean(json_object_object_get(mgmtJobj, "X_ZYXEL_IPv4_Protocol"));
			json_object_object_add(pramJobj, paraName, json_object_new_boolean(enable));		
		}
		else if(!strcmp(paraName, "DisplaySOAP")) {			
			enable = json_object_get_boolean(json_object_object_get(mgmtJobj, "X_ZYXEL_DisplaySOAP"));
			json_object_object_add(pramJobj, paraName, json_object_new_boolean(enable));
		}
		else if(!strcmp(paraName, "BoundInterfaceMode")) {
			str = json_object_get_string(json_object_object_get(mgmtJobj, "X_ZYXEL_BoundInterface"));
			json_object_object_add(pramJobj, paraName, json_object_new_string(str));
		}
		else if(!strcmp(paraName, "BoundInterfaceList")) { //note that BoundInterfaceMode must be processed before BoundInterfaceList
			str = json_object_get_string(json_object_object_get(mgmtJobj, "X_ZYXEL_BoundInterfaceList"));
			json_object_object_add(pramJobj, paraName, json_object_new_string(str));
		}
		else if(!strcmp(paraName, "CheckCert")) {			
			enable = json_object_get_boolean(json_object_object_get(mgmtJobj, "X_ZYXEL_CheckCertificate"));
			json_object_object_add(pramJobj, paraName, json_object_new_boolean(enable));
		}
		else if(!strcmp(paraName, "Certificate")) {
			str = json_object_get_string(json_object_object_get(mgmtJobj, "X_ZYXEL_Certificate"));
			json_object_object_add(pramJobj, paraName, json_object_new_string(str));
		}
		else if(!strcmp(paraName, "DataModelSpec")) {
			str = json_object_get_string(json_object_object_get(mgmtJobj, "X_ZYXEL_DataModelSpec"));
			json_object_object_add(pramJobj, paraName, json_object_new_string(str));
		}
		else if(!strcmp(paraName, "Password"))
		{
			str = json_object_get_string(json_object_object_get(mgmtJobj, "Password"));
			if (strlen(str) > 0){
				json_object_object_add(pramJobj, paraName, json_object_new_string(hiddenPassword));
			}else{
				json_object_object_add(pramJobj, paraName, json_object_new_string(""));
			}
		}
		else if(!strcmp(paraName, "ConnectionRequestPassword"))
		{
			str = json_object_get_string(json_object_object_get(mgmtJobj, "ConnectionRequestPassword"));
			if (strlen(str) > 0){
				json_object_object_add(pramJobj, paraName, json_object_new_string(hiddenPassword));
			}else{
				json_object_object_add(pramJobj, paraName, json_object_new_string(""));
			}
		}
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
        else if (!strcmp(paraName, "EnableXmppConnReq"))
        {
            enable = json_object_get_boolean(json_object_object_get(xmppConnJobj, "Enable"));
            json_object_object_add(pramJobj, paraName, json_object_new_boolean(enable));
        }
        else if (!strcmp(paraName, "XmppConnUsername"))
        {
            str = json_object_get_string(json_object_object_get(xmppConnJobj, "Username"));
            json_object_object_add(pramJobj, paraName, json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnPassword"))
        {
            str = json_object_get_string(json_object_object_get(xmppConnJobj, "Password"));
            json_object_object_add(pramJobj, paraName, json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnDomain"))
        {
            str = json_object_get_string(json_object_object_get(xmppConnJobj, "Domain"));
            json_object_object_add(pramJobj, paraName, json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnResource"))
        {
            str = json_object_get_string(json_object_object_get(xmppConnJobj, "Resource"));
            json_object_object_add(pramJobj, paraName, json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppServerAddress"))
        {
            str = json_object_get_string(json_object_object_get(xmppConnSrvJobj, "ServerAddress"));
            json_object_object_add(pramJobj, paraName, json_object_new_string(str));
        }
        else if(!strcmp(paraName, "XmppServerPort"))
        {
            intVal = json_object_get_int(json_object_object_get(xmppConnSrvJobj, "Port"));
            json_object_object_add(pramJobj, paraName, json_object_new_int(intVal));
        }
#endif /* #ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST */
		else {
			json_object_object_add(pramJobj, paraName, JSON_OBJ_COPY(json_object_object_get(mgmtJobj, paraName)));
		}

		paraName = (const char *)MGMT_param[n++].paraName;
	}

	json_object_put(mgmtJobj);
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
	json_object_put(xmppConnJobj);
	json_object_put(xmppConnSrvJobj);
#endif
	json_object_array_add(Jarray, pramJobj);
	
	return ret;
}


static zcfgRet_t zcfgFeDalMgmtSet(struct json_object *paramObj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char *paraName;
	objIndex_t objIid;
	struct json_object *mgmtJobj = NULL;
	int paramModified = 0, n = 0;

#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
    struct json_object  *xmppConnJobj = NULL;
    struct json_object  *xmppConnSrvJobj = NULL;
    objIndex_t          xmppConnObjIid = {0};
    objIndex_t          xmppConnSrvObjIid = {0};
    const char          *connPath = NULL;
#endif


	paraName = (const char *)MGMT_param[n++].paraName;

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, &mgmtJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}
	if(json_object_object_get(paramObj, "IPv4_Enable") &&json_object_object_get(paramObj, "IPv6_Enable")){
		bool v4Enable = json_object_get_boolean(json_object_object_get(paramObj, "IPv4_Enable"));
		bool v6Enable = json_object_get_boolean(json_object_object_get(paramObj, "IPv6_Enable"));
		if(!v4Enable && !v6Enable){
			if(replyMsg)
				strcat(replyMsg, "IPv4 and IPv6 protocol cannot disable together.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}

#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
    connPath = json_object_get_string(json_object_object_get(mgmtJobj, "ConnReqXMPPConnection"));
	if ((ret = zcfgFeDalXmppInfoGet(&xmppConnJobj, &xmppConnSrvJobj, &xmppConnObjIid, &xmppConnSrvObjIid, connPath)) != ZCFG_SUCCESS)
    {
        return ret;
    }
#endif
	while(paraName) {
		struct json_object *paramValue = json_object_object_get(paramObj, paraName);
		if(!paramValue) {
			paraName = (const char *)MGMT_param[n++].paraName;
			continue;
		}
		
		if(!strcmp(paraName, "DebugLevelEnable")) {
			int debugLevels = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_DebugLevel"));
			int debugLevel = json_object_get_int(paramValue);
			debugLevels |= debugLevel;
			json_object_object_add(mgmtJobj, "X_ZYXEL_DebugLevel", json_object_new_int(debugLevels));
			if(debugLevels & CWMP_SOAP_MSG_DEBUG)
				json_object_object_add(mgmtJobj, "X_ZYXEL_DisplaySOAP", json_object_new_boolean(true));
			paramModified = 1;
		}
		else if(!strcmp(paraName, "DebugLevelDisable")) {
			int debugLevels = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_DebugLevel"));
			int debugLevel = json_object_get_int(paramValue);
			debugLevels &= ~debugLevel;
			json_object_object_add(mgmtJobj, "X_ZYXEL_DebugLevel", json_object_new_int(debugLevels));
			if(!(debugLevels & CWMP_SOAP_MSG_DEBUG))
				json_object_object_add(mgmtJobj, "X_ZYXEL_DisplaySOAP", json_object_new_boolean(false));
			paramModified = 1;
		}
		else if(!strcmp(paraName, "FetureOptionEnable")) {
			int features = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_Feature"));
			int featureEnable = json_object_get_int(paramValue);
			features |= featureEnable;
			json_object_object_add(mgmtJobj, "X_ZYXEL_Feature", json_object_new_int(features));	
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "FetureOptionDisable")) {
			int features = json_object_get_int(json_object_object_get(mgmtJobj, "X_ZYXEL_Feature"));
			int featureEnable = json_object_get_int(paramValue);
			features &= ~featureEnable;
			json_object_object_add(mgmtJobj, "X_ZYXEL_Feature", json_object_new_int(features));		
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "IPv6_Enable")) {
			bool enable = json_object_get_boolean(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_IPv6_Protocol", json_object_new_boolean(enable));
			paramModified = 1;	
		}
		else if(!strcmp(paraName, "IPv4_Enable")) {
			bool enable = json_object_get_boolean(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_IPv4_Protocol", json_object_new_boolean(enable));	
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "DisplaySOAP")) {
			bool enable = json_object_get_boolean(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_DisplaySOAP", json_object_new_boolean(enable));	
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "BoundInterfaceMode")) {
			const char *mode = json_object_get_string(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_BoundInterface", json_object_new_string(mode));
			if(!strcmp(mode, "Any_WAN")){
				//json_object_object_add(mgmtJobj, "X_ZYXEL_BoundInterfaceList", json_object_new_string("SUPPORT_MULTI_WAN"));
			}
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "BoundInterfaceList")) { //note that BoundInterfaceMode must be processed before BoundInterfaceList
			const char *curMode = json_object_get_string(json_object_object_get(mgmtJobj, "X_ZYXEL_BoundInterface"));
			if(!strcmp(curMode, "Multi_WAN")){
				const char *list = json_object_get_string(paramValue);
				json_object_object_add(mgmtJobj, "X_ZYXEL_BoundInterfaceList", json_object_new_string(list));		
				paramModified = 1;
			}
		}
		else if(!strcmp(paraName, "CheckCert")) {
			bool enable = json_object_get_boolean(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_CheckCertificate", json_object_new_boolean(enable));	
			paramModified = 1;
		}
		else if(!strcmp(paraName, "Certificate")) {
			const char *certName = json_object_get_string(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_Certificate", json_object_new_string(certName));		
			paramModified = 1;			
		}
		else if(!strcmp(paraName, "ConnectionRequestURL")) {
			//readonly parameter		
		}
		else if(!strcmp(paraName, "EnableAuthentication")){
			bool enableAuth = json_object_get_boolean(paramValue);
			if(!enableAuth){
				json_object_object_add(mgmtJobj, "ConnectionRequestUsername", json_object_new_string(""));	
				json_object_object_add(mgmtJobj, "ConnectionRequestPassword", json_object_new_string(""));
				paramModified = 1;	
			}
		}
		else if(!strcmp(paraName, "DataModelSpec")) {
			const char *certName = json_object_get_string(paramValue);
			json_object_object_add(mgmtJobj, "X_ZYXEL_DataModelSpec", json_object_new_string(certName));
			paramModified = 1;
		}
		else if(!strcmp(paraName, "Password"))
		{
			const char *password = json_object_get_string(paramValue);
			if(strcmp(password, hiddenPassword)){
				json_object_object_add(mgmtJobj, "Password", json_object_new_string(password));
				paramModified = 1;
			}
		}
		else if(!strcmp(paraName, "ConnectionRequestPassword"))
		{
			const char *connectionpassword = json_object_get_string(paramValue);
			if(strcmp(connectionpassword, hiddenPassword)){
				json_object_object_add(mgmtJobj, "ConnectionRequestPassword", json_object_new_string(connectionpassword));
				paramModified = 1;
			}
		}
#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
        else if (!strcmp(paraName, "EnableXmppConnReq"))
        {
            bool enable = json_object_get_boolean(paramValue);
            json_object_object_add(xmppConnJobj, "Enable", json_object_new_boolean(enable));
            json_object_object_add(xmppConnSrvJobj, "Enable", json_object_new_boolean(enable));
        }
        else if (!strcmp(paraName, "XmppConnUsername"))
        {
            const char *str = json_object_get_string(paramValue);
            json_object_object_add(xmppConnJobj, "Username", json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnPassword"))
        {
            const char *str = json_object_get_string(paramValue);
            json_object_object_add(xmppConnJobj, "Password", json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnDomain"))
        {
            const char *str = json_object_get_string(paramValue);
            json_object_object_add(xmppConnJobj, "Domain", json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppConnResource"))
        {
            const char *str = json_object_get_string(paramValue);
            json_object_object_add(xmppConnJobj, "Resource", json_object_new_string(str));
        }
        else if (!strcmp(paraName, "XmppServerAddress"))
        {
            const char *str = json_object_get_string(paramValue);
            json_object_object_add(xmppConnSrvJobj, "ServerAddress", json_object_new_string(str));
        }
        else if(!strcmp(paraName, "XmppServerPort"))
        {
            int intVal = json_object_get_int(paramValue);
            json_object_object_add(xmppConnSrvJobj, "Port", json_object_new_int(intVal));
        }
#endif /* #ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST */
		else {
#ifdef TAAAB_HGW
					if(!strcmp(paraName, "URL")) {
						if(0 < strlen(json_object_get_string(paramValue))){
							json_object_object_add(mgmtJobj, "X_ZYXEL_AllowOption43ChangeURL", json_object_new_boolean(false));
						}
						else{
							json_object_object_add(mgmtJobj, "X_ZYXEL_AllowOption43ChangeURL", json_object_new_boolean(true));
						}
					}
#endif					
			json_object_object_add(mgmtJobj, paraName, JSON_OBJ_COPY(paramValue));
			paramModified = 1;			
		}

		paraName = (const char *)MGMT_param[n++].paraName;
	}

	if(paramModified) {
		if((ret = zcfgFeObjJsonSet(RDM_OID_MGMT_SRV, &objIid, mgmtJobj, NULL)) != ZCFG_SUCCESS)
			printf("%s: Write object fail\n", __FUNCTION__);
	}

	json_object_put(mgmtJobj);

#ifdef ZYXEL_TR69_XMPP_CONNECTION_REQUEST
	if((ret = zcfgFeObjJsonSet(RDM_OID_XMPP_CONN, &xmppConnObjIid, xmppConnJobj, NULL)) != ZCFG_SUCCESS)
	{
	    ZLOG_ERROR("%s: Write RDM_OID_XMPP_CONN object fail");
	}
	if((ret = zcfgFeObjJsonSet(RDM_OID_XMPP_CONN_SRV, &xmppConnSrvObjIid, xmppConnSrvJobj, NULL)) != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("%s: Write RDM_OID_XMPP_CONN_SRV object fail");
    }
    json_object_put(xmppConnJobj);
    json_object_put(xmppConnSrvJobj);
#endif

	return ret;
}

zcfgRet_t zcfgFeDalMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalMgmtSet(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalMgmtSet(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalMgmtGet(Jobj, Jarray, NULL);
	}
	else if(!strcmp(method, "DELETE")) {
	
	}

	return ret;
}
