#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"



dal_param_t DHCPv6Relay_param[] = {
	{"Index",              dalType_int, 0, 0, NULL, NULL, NULL},
	{"Enable",             dalType_boolean, 0, 0, NULL, NULL, NULL},
	{"Status",             dalType_string, 0, 0, NULL, "Disable|Enable", NULL},
	{"Alias",              dalType_string, 0, 0, NULL, NULL, NULL},
	{"Interface",          dalType_string, 0, 0, NULL, NULL, dal_Edit},
	{"PreviousConfig",     dalType_string, 0, 0, NULL, "Server|Disable", NULL},
	{"DebugEnable",        dalType_int, 0, 0, NULL, NULL, NULL},
	{"WanInterface",       dalType_string, 0, 0, NULL, NULL, dal_Edit},
	{"Dhcpv6ServerIpAddr", dalType_v6Addr, 0, 0, NULL, NULL, dal_Edit},
	{NULL, 0, 0, 0, NULL, NULL, NULL}
};


char *getIpIfaceBySrvName(char *svrName, char *ipIface);
char *getSrvNameByIpIface(char *ipIface, char *svrName);


///////////////////////////////////////////////////////////////////
zcfgRet_t zcfgFeDal_disableDhcpv6Server(char *ipIfPath)
{
	struct json_object *dhcp6SrvObj = NULL;
	struct json_object *ipIfaceObj = NULL;
	objIndex_t dhcp6sPoolIid, ipIfIid;
	int ret = 0;
	
	if (ipIfPath) {
		//printf("[%s][%d] ipIfPath(%s)...\n", __FUNCTION__, __LINE__, ipIfPath);
		// Disable DHCPv6 Server
		IID_INIT(dhcp6sPoolIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, &dhcp6SrvObj) == ZCFG_SUCCESS) {
			char *inface = json_object_get_string(json_object_object_get(dhcp6SrvObj, "Interface"));
			if(!strcmp(inface, ipIfPath)){
				json_object_object_add(dhcp6SrvObj, "Enable", json_object_new_boolean(false));
				zcfgFeObjJsonBlockedSet(RDM_OID_DHCPV6_SRV_POOL, &dhcp6sPoolIid, dhcp6SrvObj, NULL);
				zcfgFeJsonObjFree(dhcp6SrvObj);
				ret = 2;
				break;
			}
			zcfgFeJsonObjFree(dhcp6SrvObj);
		}

		// Enable LAN IPv6
		IID_INIT(ipIfIid);
		ipIfIid.level = 1;
		sscanf(ipIfPath, "IP.Interface.%hhu", &ipIfIid.idx[0]);
		if (zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			json_object_object_add(ipIfaceObj, "IPv6Enable", json_object_new_boolean(true));
			zcfgFeObjJsonBlockedSet(RDM_OID_IP_IFACE, &ipIfIid, ipIfaceObj, NULL);
			zcfgFeJsonObjFree(ipIfaceObj);
			ret++;
		}	
	}

	//printf("[%s][%d] Exit, ret(%d)...\n", __FUNCTION__, __LINE__, ret);
	return ret;
}


zcfgRet_t zcfgFeDal_isDhcpv6RelayEnable(char *ipIfPath)
{
	objIndex_t dhcpv6relayIid;
	struct json_object *dhcpv6RelayObj = NULL;
	int ret = 0;

	if (ipIfPath) {
		//printf("[%s][%d] ipIfPath(%s)...\n", __FUNCTION__, __LINE__, ipIfPath);
		IID_INIT(dhcpv6relayIid);
		while (zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
			char *v6if = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "Interface"));
			
			if (v6if && !strcmp(v6if, ipIfPath)) {
				bool xEnable = json_object_get_boolean(json_object_object_get(dhcpv6RelayObj, "Enable"));
				//printf("[%s][%d] v6if(%s), xEnable(%d)...\n", __FUNCTION__, __LINE__, v6if, xEnable);
				
				if (xEnable == true) {
					ret = 1;
				}
				zcfgFeJsonObjFree(dhcpv6RelayObj);
				break;
			}
			zcfgFeJsonObjFree(dhcpv6RelayObj);
		}
	}

	//printf("[%s][%d] Exit, ret(%d)...\n", __FUNCTION__, __LINE__, ret);
	return ret;
}


char *getIpIfaceBySrvName(char *svrName, char *ipIface)
{
	objIndex_t ipIfIid;
	struct json_object *ipIfaceObj = NULL;
	char tmp[64] = {0};

	if (svrName && ipIface) {
		//printf("[%s][%d] Enter, svrName(%s)...\n", __FUNCTION__, __LINE__, svrName);

		IID_INIT(ipIfIid);
		while (zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipIfIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			char *X_ZYXEL_SrvName = (char *)json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
			if (X_ZYXEL_SrvName && !strcmp(X_ZYXEL_SrvName, svrName)) {
				snprintf(tmp, sizeof(tmp), "IP.Interface.%d", ipIfIid.idx[0]);
				zcfgFeJsonObjFree(ipIfaceObj);
				break;
			}
			zcfgFeJsonObjFree(ipIfaceObj);
		}
	}
	snprintf(ipIface, sizeof(tmp), "%s", tmp);
	//printf("[%s][%d] Exit, ipIface(%s)...\n", __FUNCTION__, __LINE__, ipIface);
	return ipIface;
}

char *getSrvNameByIpIface(char *ipIface, char *svrName)
{
	objIndex_t ipIfIid;
	struct json_object *ipIfaceObj = NULL;
	char tmp[64] = {0};

	if (svrName && ipIface) {
		//printf("[%s][%d] Enter, ipIface(%s)...\n", __FUNCTION__, __LINE__, ipIface);

		IID_INIT(ipIfIid);
		ipIfIid.level = 1;
		sscanf(ipIface, "IP.Interface.%hhu", &ipIfIid.idx[0]);
		if (zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfIid, &ipIfaceObj) == ZCFG_SUCCESS) {
			char *X_ZYXEL_SrvName = (char *)json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
			strcpy(tmp, X_ZYXEL_SrvName);
			zcfgFeJsonObjFree(ipIfaceObj);
		}	
	}
	snprintf(svrName, sizeof(tmp), "%s", tmp);
	//printf("[%s][%d] Exit, svrName(%s)...\n", __FUNCTION__, __LINE__, svrName);
	return svrName;
}

zcfgRet_t zcfgFeDal_Dhcpv6Relay_Add(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpv6relayIid;
	struct json_object *dhcpv6RelayObj = NULL;

	//printf("[%s][%d] Enter, Jobj(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(Jobj));

	IID_INIT(dhcpv6relayIid);
	if (zcfgFeObjJsonAdd(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, NULL) == ZCFG_SUCCESS) {
		if (zcfgFeObjJsonGet(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
			//printf("[%s][%d] dhcpv6RelayObj(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(dhcpv6RelayObj));

			{
				bool xEnable;
				char *xStatus;
				char *xAlias;
				char *xInterface=NULL;
				char *xPreviousConfig;
				uint32_t xDebugEnable;
				char *xWanInterface;
				char *xDhcpv6ServerIpAddr;

				if (json_object_object_get(Jobj, "Enable") != NULL) {
					xEnable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
					//printf("[%s][%d] xEnable(%d)...\n", __FUNCTION__, __LINE__, xEnable);
					json_object_object_add(dhcpv6RelayObj, "Enable", json_object_new_boolean(xEnable));
				}

				if (json_object_object_get(Jobj, "Status") != NULL) {
					xStatus = json_object_get_string(json_object_object_get(Jobj, "Status"));
					//printf("[%s][%d] xStatus(%s)...\n", __FUNCTION__, __LINE__, xStatus);
					json_object_object_add(dhcpv6RelayObj, "Status", json_object_new_string(xStatus));
				}

				if (json_object_object_get(Jobj, "Alias") != NULL) {
					xAlias = json_object_get_string(json_object_object_get(Jobj, "Alias"));
					//printf("[%s][%d] xAlias(%s)...\n", __FUNCTION__, __LINE__, xAlias);
					json_object_object_add(dhcpv6RelayObj, "Alias", json_object_new_string(xAlias));
				}

				if (json_object_object_get(Jobj, "Interface") != NULL) {
					xInterface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
					if (!strstr(xInterface, "IP.Interface.")) {
						sprintf(xInterface, "IP.Interface.%d", atoi(xInterface));
					}
					//printf("[%s][%d] xInterface(%s)...\n", __FUNCTION__, __LINE__, xInterface);
					json_object_object_add(dhcpv6RelayObj, "Interface", json_object_new_string(xInterface));
				}

				if (json_object_object_get(Jobj, "PreviousConfig") != NULL) {
					xPreviousConfig = json_object_get_string(json_object_object_get(Jobj, "PreviousConfig"));
					//printf("[%s][%d] xPreviousConfig(%s)...\n", __FUNCTION__, __LINE__, xPreviousConfig);
					json_object_object_add(dhcpv6RelayObj, "PreviousConfig", json_object_new_string(xPreviousConfig));
				}

				if (json_object_object_get(Jobj, "DebugEnable") != NULL) {
					xDebugEnable = json_object_get_int(json_object_object_get(Jobj, "DebugEnable"));
					//printf("[%s][%d] xDebugEnable(%d)...\n", __FUNCTION__, __LINE__, xDebugEnable);
					json_object_object_add(dhcpv6RelayObj, "DebugEnable", json_object_new_int(xDebugEnable));
				}

				if (json_object_object_get(Jobj, "WanInterface") != NULL) {
					char xxWanInterface[64] = {0};
					xWanInterface = json_object_get_string(json_object_object_get(Jobj, "WanInterface"));
					getIpIfaceBySrvName(xWanInterface, xxWanInterface);
					//printf("[%s][%d] xWanInterface(%s), xxWanInterface(%s)...\n", __FUNCTION__, __LINE__, xWanInterface, xxWanInterface);
					json_object_object_add(dhcpv6RelayObj, "WanInterface", json_object_new_string(xxWanInterface));
				}

				if (json_object_object_get(Jobj, "Dhcpv6ServerIpAddr") != NULL) {
					xDhcpv6ServerIpAddr = json_object_get_string(json_object_object_get(Jobj, "Dhcpv6ServerIpAddr"));
					//printf("[%s][%d] xDhcpv6ServerIpAddr(%s)...\n", __FUNCTION__, __LINE__, xDhcpv6ServerIpAddr);
					json_object_object_add(dhcpv6RelayObj, "Dhcpv6ServerIpAddr", json_object_new_string(xDhcpv6ServerIpAddr));
				}
				
				// Set
				zcfgFeDal_disableDhcpv6Server(xInterface);
				// printf("[%s][%d] SET: dhcpv6RelayObj(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(dhcpv6RelayObj));
				if (zcfgFeObjJsonBlockedSet(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, dhcpv6RelayObj, NULL) == ZCFG_SUCCESS) {
					// printf("[%s][%d] Set Success...\n", __FUNCTION__, __LINE__);
				} else {
					printf("[%s][%d] Set Fail...\n", __FUNCTION__, __LINE__);
				}
				zcfgFeJsonObjFree(dhcpv6RelayObj);

			}
		} // End of get: RDM_OID_DHCPV6_RELAY_INTF_SET
	} // End if add: RDM_OID_DHCPV6_RELAY_INTF_SET

	return ret;
}

zcfgRet_t zcfgFeDal_Dhcpv6Relay_Del(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int idx = 0;
	struct json_object *dhcpv6RelayObj = NULL;
	objIndex_t dhcpv6relayIid = {0};

	//printf("[%s][%d] Enter, Jobj(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(Jobj));
	idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
	//printf("[%s][%d] idx(%d)...\n", __FUNCTION__, __LINE__, idx);

	IID_INIT(dhcpv6relayIid);
	dhcpv6relayIid.level = 1;
	dhcpv6relayIid.idx[0] = idx;
	if (zcfgFeObjJsonGet(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
		char *v6if = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "Interface"));
		char *v6wif = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "WanInterface"));
		char *v6srvip = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "Dhcpv6ServerIpAddr"));
		//printf("[%s][%d] v6if(%s), v6wif(%s), v6srvip(%s)...\n", __FUNCTION__, __LINE__, v6if, v6wif, v6srvip);

		if (zcfgFeObjJsonDel(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, NULL) != ZCFG_SUCCESS) {
			printf("[%s][%d] Del fail: v6if(%s), v6wif(%s), v6srvip(%s)...\n", __FUNCTION__, __LINE__, v6if, v6wif, v6srvip);
		}

		zcfgFeJsonObjFree(dhcpv6RelayObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDal_Dhcpv6Relay_Edit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *Interface = NULL;
	char *WanInterface = NULL;
	char *Dhcpv6ServerIpAddr = NULL;
	struct json_object *dhcpv6RelayObj = NULL;
	objIndex_t dhcpv6relayIid = {0};
	int found = 0, len = 0;
	struct json_object *obj = NULL;
	struct json_object *eachObj = NULL;
	struct json_object *saveObj = NULL;


	//printf("[%s][%d] Enter, Jobj(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(Jobj));

	Interface = (char *)json_object_get_string(json_object_object_get(Jobj, "Interface"));
	if (Interface) {
		if (!strstr(Interface, "IP.Interface.")) {
			sprintf(Interface, "IP.Interface.%d", atoi(Interface));
			//sprintf(ifacePath, "IP.Interface.%hhu", curIpIfaceIid.idx[0]);
		}
	}
	//printf("[%s][%d] Interface(%s)...\n", __FUNCTION__, __LINE__, Interface);

	// To find the instance if it existed.
	IID_INIT(dhcpv6relayIid);
	while (Interface && zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
		char *v6if = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "Interface"));
		if (v6if) {
			//printf("[%s][%d] v6if(%s)...\n", __FUNCTION__, __LINE__, v6if);
			if (!strcmp(Interface, v6if)) {
				found++;
				//zcfgFeJsonObjFree(dhcpv6RelayObj);
				break;
			}
		}
		zcfgFeJsonObjFree(dhcpv6RelayObj);
	}

	if (found == 0) { // Add an instance for this confiruration
		// To find the empty instance if it existed.
		IID_INIT(dhcpv6relayIid);
		while (Interface && zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
			char *v6if = (char *)json_object_get_string(json_object_object_get(dhcpv6RelayObj, "Interface"));
			if (v6if) {
				//printf("[%s][%d] v6if(%s)...\n", __FUNCTION__, __LINE__, v6if);
				if (strlen(v6if) == 0) {
					found++;
					//zcfgFeJsonObjFree(dhcpv6RelayObj);
					break;
				}
			}
			zcfgFeJsonObjFree(dhcpv6RelayObj);
		}
	}

	// Set the instance
	if (found > 0) {
		bool xEnable;
		char *xStatus;
		char *xAlias;
		char *xInterface=NULL;
		char *xPreviousConfig;
		uint32_t xDebugEnable;
		char *xWanInterface;
		char *xDhcpv6ServerIpAddr;

		if (json_object_object_get(Jobj, "Enable") != NULL) {
			xEnable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(dhcpv6RelayObj, "Enable", json_object_new_boolean(xEnable));
		}

		if (json_object_object_get(Jobj, "Status") != NULL) {
			xStatus = json_object_get_string(json_object_object_get(Jobj, "Status"));
			json_object_object_add(dhcpv6RelayObj, "Status", json_object_new_string(xStatus));
		}

		if (json_object_object_get(Jobj, "Alias") != NULL) {
			xAlias = json_object_get_string(json_object_object_get(Jobj, "Alias"));
			json_object_object_add(dhcpv6RelayObj, "Alias", json_object_new_string(xAlias));
		}

		if (json_object_object_get(Jobj, "Interface") != NULL) {
			xInterface = json_object_get_string(json_object_object_get(Jobj, "Interface"));
			json_object_object_add(dhcpv6RelayObj, "Interface", json_object_new_string(xInterface));
		}

		if (json_object_object_get(Jobj, "PreviousConfig") != NULL) {
			xPreviousConfig = json_object_get_string(json_object_object_get(Jobj, "PreviousConfig"));
			json_object_object_add(dhcpv6RelayObj, "PreviousConfig", json_object_new_string(xPreviousConfig));
		}

		if (json_object_object_get(Jobj, "DebugEnable") != NULL) {
			xDebugEnable = json_object_get_int(json_object_object_get(Jobj, "DebugEnable"));
			json_object_object_add(dhcpv6RelayObj, "DebugEnable", json_object_new_int(xDebugEnable));
		}

		if (json_object_object_get(Jobj, "WanInterface") != NULL) {
			char xxWanInterface[64] = {0};
			xWanInterface = json_object_get_string(json_object_object_get(Jobj, "WanInterface"));
			getIpIfaceBySrvName(xWanInterface, xxWanInterface);
			//printf("[%s][%d] xWanInterface(%s), xxWanInterface(%s)...\n", __FUNCTION__, __LINE__, xWanInterface, xxWanInterface);
			json_object_object_add(dhcpv6RelayObj, "WanInterface", json_object_new_string(xxWanInterface));
		}

		if (json_object_object_get(Jobj, "Dhcpv6ServerIpAddr") != NULL) {
			xDhcpv6ServerIpAddr = json_object_get_string(json_object_object_get(Jobj, "Dhcpv6ServerIpAddr"));
			json_object_object_add(dhcpv6RelayObj, "Dhcpv6ServerIpAddr", json_object_new_string(xDhcpv6ServerIpAddr));
		}

		zcfgFeDal_disableDhcpv6Server(xInterface);
		if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, dhcpv6RelayObj, NULL)) == ZCFG_SUCCESS) {
			// printf("[%s][%d] Set Success...\n", __FUNCTION__, __LINE__, len, json_object_get_string(obj));
		} else {
			printf("[%s][%d] Set Fail...\n", __FUNCTION__, __LINE__, len, json_object_get_string(obj));
		}
		zcfgFeJsonObjFree(dhcpv6RelayObj);
	} else { // Add
		ret = zcfgFeDal_Dhcpv6Relay_Add(Jobj, NULL);
	}

	// printf("[%s][%d] Exit, ret(%d)...\n", __FUNCTION__, __LINE__, ret);

	return ret;
}


zcfgRet_t zcfgFeDal_Dhcpv6Relay_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *dhcpv6RelayObj = NULL;
	struct json_object *pramJobj = NULL;
	objIndex_t dhcpv6relayIid = {0};
	int count = 0;

	//printf("[%s][%d] Enter, Jobj(%s), Jarray(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(Jobj), json_object_get_string(Jarray));
	IID_INIT(dhcpv6relayIid);
	while (zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_RELAY_INTF_SET, &dhcpv6relayIid, &dhcpv6RelayObj) == ZCFG_SUCCESS) {
		pramJobj = json_object_new_object();

		json_object_object_add(pramJobj, "Index", json_object_new_int(count));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "Enable")));
		json_object_object_add(pramJobj, "Status", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "Status")));
		json_object_object_add(pramJobj, "Alias", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "Alias")));
		json_object_object_add(pramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "Interface")));
		json_object_object_add(pramJobj, "PreviousConfig", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "PreviousConfig")));
		json_object_object_add(pramJobj, "DebugEnable", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "DebugEnable")));

		{
			char *xWanInterface = json_object_get_string(json_object_object_get(dhcpv6RelayObj, "WanInterface"));
			char xxWanInterface[64] = {0};
			if (xWanInterface) {
				//printf("[%s][%d] xWanInterface(%s)...\n", __FUNCTION__, __LINE__, xWanInterface);
				getSrvNameByIpIface(xWanInterface, xxWanInterface);
				//printf("[%s][%d] xxWanInterface(%s)...\n", __FUNCTION__, __LINE__, xxWanInterface);

				json_object_object_add(pramJobj, "WanInterface", json_object_new_string(xxWanInterface));
			}
		}

		json_object_object_add(pramJobj, "Dhcpv6ServerIpAddr", JSON_OBJ_COPY(json_object_object_get(dhcpv6RelayObj, "Dhcpv6ServerIpAddr")));
		
		json_object_array_add(Jarray, pramJobj);
		count++;

		zcfgFeJsonObjFree(dhcpv6RelayObj);
	}

	return ret;
}


void zcfgFeDalShowDhcpv6Relay(struct json_object *Jarray)
{
	int i, len = 0;
	struct json_object *obj_idx = NULL;

	//printf("[%s][%d] Enter, Jarray(%s)...\n", __FUNCTION__, __LINE__, json_object_get_string(Jarray));
	if (!Jarray) return;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);

	printf("Size(%d)\n%-6s %-10s %-6s %-10s %-16s %-16s %-42s\n",
			    len, "Index", "Name", "Enable", "Status", "Interface", "WanInterface", "Dhcpv6ServerIpAddr");

	printf("%-6s %-10s %-6s %-10s %-16s %-16s %-42s\n",
			    "------", "----------", "------", "----------", "----------", "----------------", "------------------------------------------");

	for(i=0; i<len; i++) {

		obj_idx = json_object_array_get_idx(Jarray, i);

		printf("%-6d %-10s %-6s %-10s %-16s %-16s %-42s\n"
			    , i+1
			    , json_object_get_string(json_object_object_get(obj_idx, "Alias"))
			    , json_object_get_string(json_object_object_get(obj_idx, "Enable"))
			    , json_object_get_string(json_object_object_get(obj_idx, "Status"))
			    , json_object_get_string(json_object_object_get(obj_idx, "Interface"))
			    //, json_object_get_string(json_object_object_get(obj_idx, "PreviousConfig"))
			    //, json_object_get_string(json_object_object_get(obj_idx, "DebugEnable"))
			    , json_object_get_string(json_object_object_get(obj_idx, "WanInterface"))
			    , json_object_get_string(json_object_object_get(obj_idx, "Dhcpv6ServerIpAddr"))
		);
	}

	return;
}


zcfgRet_t zcfgFeDalDhcpv6Relay(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	//printf("[%s][%d] Enter, method(%s), Jobj(%s), Jarray(%s)...\n", __FUNCTION__, __LINE__, method, json_object_get_string(Jobj), json_object_get_string(Jarray));

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_Dhcpv6Relay_Add(Jobj, replyMsg);

	} else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Dhcpv6Relay_Edit(Jobj, replyMsg);

	} else if(!strcmp(method, "GET")) {
		ret = zcfgFeDal_Dhcpv6Relay_Get(Jobj, Jarray, NULL);

	} else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_Dhcpv6Relay_Del(Jobj, replyMsg);

	} else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}


