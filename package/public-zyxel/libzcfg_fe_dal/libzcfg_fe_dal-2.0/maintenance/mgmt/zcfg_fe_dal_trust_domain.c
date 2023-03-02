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
#include "zcfg_fe_dal.h"
#ifdef ABUU_CUSTOMIZATION
#include "zlog_api.h"
#include <arpa/inet.h>
#endif

dal_param_t TRUST_DOMAIN_param[] =
{
	{"Enable", 				dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"IPAddress", 			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"SubnetMask", 			dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"IPAddressPrefix", 	dalType_IPPrefix,   0, 	0, 	NULL,	NULL, 	dal_Add},
	{"Index", 	 			dalType_int, 	    0, 	0, 	NULL,	NULL,	dal_Delete},
	{NULL, 					0, 				    0, 	0, 	NULL,	NULL, 	0}
};

#ifdef ABUU_CUSTOMIZATION
/*! checkIPinSubnet
 *  Check if the address is in the subnet
 *  Note: IPv6 is not done. Please refer to checkCidrBlock() in zhttpd.c.
 *  
 *  @param[in]     findAddress        Address needs to be checked
 *  @param[in]     targetAddress      Target subnet address
 *  @param[in]     targetMask         Target subnet mask
 *
 *  @return     true  - address in the subnet
 *              false - address not in the subnet
 */
bool checkIPinSubnet(const char *findAddress, const char *targetAddress, const char *targetMask) {
	bool result = false;
	struct in_addr ip, netip;
	uint32_t mask = (0xFFFFFFFF << (32 - (uint32_t)atoi(targetMask))) & 0xFFFFFFFF;

	//printf("%s: %s %s %s\n", __FUNCTION__, findAddress, targetAddress, targetMask);
	inet_aton(findAddress, &ip);
	inet_aton(targetAddress, &netip);
	mask = htonl(mask);
	//printf("%s: 0x%08x 0x%08x 0x%08x\n", __FUNCTION__, ip.s_addr, netip.s_addr, mask);

	if ((netip.s_addr & mask) == (ip.s_addr & mask))
		result = true;

	//printf("%s: 0x%08x 0x%08x 0x%08x [%d]\n", __FUNCTION__, ip.s_addr, netip.s_addr, mask, result);
	return result;
}

/*! getTrustDomainInfo
 *  get the address list when address found or login with lan host(findAddress is 0.0.0.0)
 *  Note: 
 *  1. The address list is in TrustDomainInfo, which is InterfaceTrustDomain sub-layer
 *  2. The lan host that _getLoginUserInfo_ get 0.0.0.0 as its address.
 *  
 *  @param[in]     findAddress        Find this address (without mask)
 *  @param[in]     intfIid            Target InterfaceTrustDomain
 *  @param[out]    infoJarray         Return the info list
 *
 *  @return     true  - address found or login with lan host
 *              false - address not found and not login with lan host
 */
bool getTrustDomainInfo(const char *findAddress, objIndex_t *intfIid, struct json_object *infoJarray)
{
	bool found = false;
	int index = 0;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
	char bufStr[64] = {0};
	objIndex_t infoIid = {0};
	struct json_object *infoObj = NULL;
	struct json_object *tmpObj = NULL;

	IID_INIT(infoIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, intfIid, &infoIid, &infoObj) == ZCFG_SUCCESS)
	{
		tmpObj = json_object_new_object();

		index ++;
		IPAddress = json_object_get_string(json_object_object_get(infoObj, "IPAddress"));
		SubnetMask = json_object_get_string(json_object_object_get(infoObj, "SubnetMask"));

		if(IPAddress && checkIPinSubnet(findAddress, IPAddress, SubnetMask))
			found = true;

		if(!strcmp(IPAddress, "AnyIP"))
			sprintf(bufStr, "%s", IPAddress);
		else
			sprintf(bufStr, "%s/%s", IPAddress, SubnetMask);

		json_object_object_add(tmpObj, "IntfIndex", json_object_new_int(intfIid->idx[0]));
		json_object_object_add(tmpObj, "Index", json_object_new_int(index));
		json_object_object_add(tmpObj, "IPAddressPrefix", json_object_new_string(bufStr));
		json_object_array_add(infoJarray, tmpObj);
		ZLOG_DEBUG("%s: [%d] %d - IPAddressPrefix=%s\n", __FUNCTION__, intfIid->idx[0], index, bufStr);
		zcfgFeJsonObjFree(infoObj);
	}

	/* Address found or lan host account should return the address list */
	if(!found && strcmp(findAddress, "0.0.0.0")){
		return false;
	}

	return true;
}

/*! setTrustDomainInfo
 *  set the address list when address found or login with lan host(findAddress is 0.0.0.0)
 *  Note: 
 *  1. The address list is in TrustDomainInfo, which is InterfaceTrustDomain sub-layer
 *  2. The lan host that _getLoginUserInfo_ get 0.0.0.0 as its address.
 *  
 *  @param[in]     findAddress        Find this address (without mask)
 *  @param[in]     setAddress         Set this address (with mask)
 *  @param[in]     intfIid            Target InterfaceTrustDomain
 *
 *  @return     true  - set sucess
 *              false - set fail
 */
bool setTrustDomainInfo(const char *findAddress, const char *setAddress, objIndex_t *intfIid)
{
	bool lanhost = false;
	bool found = false;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
	char ip[64] = {0};
	char mask[8] = {0};
	objIndex_t infoIid = {0};
	struct json_object *infoObj = NULL;

	/* Check setAddress */
	if(!setAddress){
		ZLOG_ERROR("%s: setAddress is NULL\n", __FUNCTION__);
		return false;
	}

	/* Check is lan host */
	if(!strcmp(findAddress, "0.0.0.0"))
		lanhost = true;
	else{
		/* Find address */
		IID_INIT(infoIid);
		while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, intfIid, &infoIid, &infoObj) == ZCFG_SUCCESS)
		{
			IPAddress = json_object_get_string(json_object_object_get(infoObj, "IPAddress"));
			SubnetMask = json_object_get_string(json_object_object_get(infoObj, "SubnetMask"));
			if(IPAddress && checkIPinSubnet(findAddress, IPAddress, SubnetMask)){
				found = true;
				zcfgFeJsonObjFree(infoObj);
				break;
			}
			zcfgFeJsonObjFree(infoObj);
		}
	}

	/* Address found or lan host account should add the address to list */
	if(lanhost || found)
	{
		IID_INIT(infoIid);
		infoIid.level = 1;
		infoIid.idx[0] = intfIid->idx[0];
		if(zcfgFeObjJsonAdd(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &infoIid, NULL) != ZCFG_SUCCESS)
			ZLOG_ERROR("%s: infoObj zcfgFeObjJsonAdd fail\n", __FUNCTION__);

		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &infoIid, &infoObj) == ZCFG_SUCCESS){
			sscanf(setAddress, "%[^/]/%s", ip, mask);
			ZLOG_DEBUG("%s: infoIid level=%d - %d %d setAddress=%s ip=%s mask=%s\n",__FUNCTION__, infoIid.level, infoIid.idx[0], infoIid.idx[1], setAddress, ip, mask);
			json_object_object_add(infoObj, "IPAddress", json_object_new_string(ip));
			json_object_object_add(infoObj, "SubnetMask", json_object_new_string(mask));
			json_object_object_add(infoObj, "Enable", json_object_new_boolean(true));

			if(zcfgFeObjJsonBlockedSet(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &infoIid, infoObj, NULL) != ZCFG_SUCCESS){
				ZLOG_ERROR("%s: zcfgFeObjJsonBlockedSet fail\n", __FUNCTION__);
				zcfgFeJsonObjFree(infoObj);
				return false;
			}
			zcfgFeJsonObjFree(infoObj);
		}
		return true;
	}

	return false;
}
#endif

void zcfgFeDalShowTrustDomain(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-10s %-45s \n",
		     "Index", "IP Address");		
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("%-10s %-45s \n",
				json_object_get_string(json_object_object_get(obj, "Index")),
				json_object_get_string(json_object_object_get(obj, "IPAddressPrefix")));
	}
}
zcfgRet_t zcfgFeDal_TrustDomain_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
#ifndef ABUU_CUSTOMIZATION
	struct json_object *paramJobj = NULL;
	int Index = 0;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
	char buf[64] = {0};
#endif
#ifdef ABUU_CUSTOMIZATION
	char loginLevel[16] = {0};
	char remoteAddress[64] = {0};
	const char* accountLevel = NULL;
	struct json_object *infoJarray = NULL;
	struct json_object *infoObj = NULL;
	int j = 0, len2 = 0;
	bool isLoginLevelEmpty = false;

	/* get account login level and source address to find the trust domain group */
	if(getLoginUserInfo(Jobj, NULL, loginLevel, remoteAddress) == ZCFG_SUCCESS){
		if(!strcmp(loginLevel, ""))
			isLoginLevelEmpty = true;

		IID_INIT(iid);
		while(!isLoginLevelEmpty && zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN, &iid, &obj) == ZCFG_SUCCESS){
			accountLevel = json_object_get_string(json_object_object_get(obj, "AccountLevel"));
			if(!accountLevel){
				zcfgFeJsonObjFree(obj);
				continue;
			}

			infoJarray = json_object_new_array();
			ZLOG_DEBUG("%s: remoteAddress=%s loginLevel=%s accountLevel=%s\n", __FUNCTION__, remoteAddress, loginLevel, accountLevel);
			if(strstr(accountLevel, loginLevel) && getTrustDomainInfo(remoteAddress, &iid, infoJarray)){
				if(infoJarray != NULL){
					len2 = json_object_array_length(infoJarray);
					for(j=0;j<len2;j++){
						infoObj = json_object_array_get_idx(infoJarray, j);
						json_object_array_add(Jarray, JSON_OBJ_COPY(infoObj));
					}
				}
			}
			json_object_put(infoJarray);
			zcfgFeJsonObjFree(obj);
		}
	}
#else
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &iid, &obj) == ZCFG_SUCCESS) {
		Index++;
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(Index));
		IPAddress = json_object_get_string(json_object_object_get(obj, "IPAddress"));
		SubnetMask = json_object_get_string(json_object_object_get(obj, "SubnetMask"));
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "IPAddress")), "AnyIP"))
			sprintf(buf, "%s", IPAddress);
		else
			sprintf(buf, "%s/%s", IPAddress, SubnetMask);
		json_object_object_add(paramJobj, "IPAddressPrefix", json_object_new_string(buf));
		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(obj);
	}
#endif
	return ret;
}
zcfgRet_t zcfgFeDal_TrustDomain_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *trustDomainObj = NULL;
	objIndex_t trustDomainIid = {0};
#ifdef ABUU_CUSTOMIZATION
	char loginLevel[16] = {0};
	char remoteAddress[64] = {0};
	const char *accountLevel = NULL;
	const char *addAddress = NULL;
	bool isLoginLevelEmpty = false;

	addAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddressPrefix"));

	/* get account login level and source address to find the trust domain group */
	if(getLoginUserInfo(Jobj, NULL, loginLevel, remoteAddress) == ZCFG_SUCCESS){
		if(!strcmp(loginLevel, ""))
			isLoginLevelEmpty = true;

		IID_INIT(trustDomainIid);
		while(!isLoginLevelEmpty && zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN, &trustDomainIid, &trustDomainObj) == ZCFG_SUCCESS){
			accountLevel = json_object_get_string(json_object_object_get(trustDomainObj, "AccountLevel"));
			if(!accountLevel){
				zcfgFeJsonObjFree(trustDomainObj);
				continue;
			}

			if(strstr(accountLevel, loginLevel) && setTrustDomainInfo(remoteAddress, addAddress, &trustDomainIid))
				ZLOG_INFO("%s: setTrustDomainInfo success\n", __FUNCTION__);
			zcfgFeJsonObjFree(trustDomainObj);
		}
	}
#else
	struct json_object *paramObj = NULL;
	const char *charVal = NULL;
	char ip[64] = {0};
	char mask[8] = {0};
	bool enable = true;
	const char *IPAddress = NULL;
	const char *SubnetMask = NULL;
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	char *userName = NULL;
	char *accessMode = NULL;
	char *srvName = NULL;
#endif	

	IID_INIT(trustDomainIid);
	ret = zcfgFeObjJsonAdd(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &trustDomainIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &trustDomainIid, &trustDomainObj);
	}
	if(ret == ZCFG_SUCCESS){	
		if((paramObj = json_object_object_get(Jobj, "IPAddressPrefix")) != NULL){
			charVal = json_object_get_string(paramObj);
			sscanf(charVal, "%[^/]/%s", ip, mask);
			//printf("SourceIPPrefix ip=%s mask=%s\n", ip, mask);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
			userName = json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_USERNAME"));
			accessMode = json_object_get_string(json_object_object_get(Jobj, "AccessMode"));
			srvName = json_object_get_string(json_object_object_get(Jobj, "SrvName"));
#endif
			json_object_object_add(trustDomainObj, "IPAddress", json_object_new_string(ip));
			json_object_object_add(trustDomainObj, "SubnetMask", json_object_new_string(mask));
			json_object_object_add(trustDomainObj, "Enable", json_object_new_boolean(enable));
		}else
		{
			IPAddress = json_object_get_string(json_object_object_get(Jobj, "IPAddress"));
			SubnetMask = json_object_get_string(json_object_object_get(Jobj, "SubnetMask"));
			//printf("SourceIPPrefix ip=%s mask=%s\n", IPAddress, SubnetMask);
			json_object_object_add(trustDomainObj, "IPAddress", json_object_new_string(IPAddress));
			json_object_object_add(trustDomainObj, "SubnetMask", json_object_new_string(SubnetMask));
			enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
			json_object_object_add(trustDomainObj, "Enable", json_object_new_boolean(enable));
		}
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &trustDomainIid, trustDomainObj, NULL);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		if (ret == ZCFG_SUCCESS){
			zcfgLogPrefix(ZCFG_LOG_INFO, ZCFG_LOGPRE_ACL, "%s User '%s' via %s add ACL %s/%s\n", accessMode, userName, srvName, ip, mask);
			//copySystemLogToFlash (false);
		}
#endif	

		zcfgFeJsonObjFree(trustDomainObj);
	}
#endif
	return ret;
}

zcfgRet_t zcfgFeDal_TrustDomain_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0;
#ifdef ABUU_CUSTOMIZATION
	int intfIndex = 0;
	int count = 0;
	objIndex_t infoIid = {0};
	struct json_object *infoObj = NULL;

	intfIndex = json_object_get_int(json_object_object_get(Jobj, "IntfIndex"));
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));


	IID_INIT(infoIid);
	infoIid.level = 2;
	infoIid.idx[0] = intfIndex;
	/* Find the index in object */
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &infoIid, &infoObj) == ZCFG_SUCCESS){
		count ++;
		if(count == index){
			ZLOG_DEBUG("%s:intfIndex=%d index=%d count=%d - %s/%s \n", __FUNCTION__, intfIndex, index, count, json_object_get_string(json_object_object_get(infoObj, "IPAddress")), json_object_get_string(json_object_object_get(infoObj, "SubnetMask")));
			if((ret = zcfgFeObjJsonDel(RDM_OID_INTERFACE_TRUST_DOMAIN_INFO, &infoIid, NULL)) != ZCFG_SUCCESS)
				ZLOG_ERROR("%s: Delete Trust Domain Info object fail! idx[0]=%d idx[1]=%d\n", __FUNCTION__, intfIndex, index);
			zcfgFeJsonObjFree(infoObj);
			break;
		}
		zcfgFeJsonObjFree(infoObj);
	}
#else
	objIndex_t trustDomainIid = {0};
	zcfg_offset_t oid;
	oid = RDM_OID_REMO_MGMT_TRUST_DOMAIN;
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	struct json_object *trustDomainObj = NULL;
	char *userName = NULL;
	char *accessMode = NULL;
	char *srvName = NULL;
	char *ip = NULL;
	char *mask = NULL;
	char cmdStr[256] = {0};
#endif	

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));	
	ret = convertIndextoIid(index, &trustDomainIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		userName = json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_USERNAME"));
		accessMode = json_object_get_string(json_object_object_get(Jobj, "AccessMode"));
		srvName = json_object_get_string(json_object_object_get(Jobj, "SrvName"));

		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &trustDomainIid, &trustDomainObj) == ZCFG_SUCCESS){
			ip = json_object_get_string(json_object_object_get(trustDomainObj, "IPAddress"));
			mask = json_object_get_string(json_object_object_get(trustDomainObj, "SubnetMask"));
			snprintf(cmdStr, sizeof(cmdStr), "%s User '%s' via %s delete ACL %s/%s\n", accessMode, userName, srvName, ip, mask);
			zcfgFeJsonObjFree(trustDomainObj);
		}
#endif	

	ret = zcfgFeObjJsonDel(RDM_OID_REMO_MGMT_TRUST_DOMAIN, &trustDomainIid, NULL);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	if (ret == ZCFG_SUCCESS){
		zcfgLogPrefix(ZCFG_LOG_INFO, ZCFG_LOGPRE_ACL, "%s", cmdStr);
		//copySystemLogToFlash (false);
	}
#endif	

#endif
	return ret;

}

zcfgRet_t zcfgFeDalTrustDomain(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDal_TrustDomain_Add(Jobj, NULL);
	}else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDal_TrustDomain_Delete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_TrustDomain_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


