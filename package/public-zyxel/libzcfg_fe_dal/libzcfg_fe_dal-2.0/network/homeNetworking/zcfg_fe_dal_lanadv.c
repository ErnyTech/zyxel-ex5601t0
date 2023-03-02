#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include <zos_api.h>

#ifdef MINIUPNPD
#define MINIUPNP_RULE_PATH        "/tmp/miniupnpd_rule"
#endif

dal_param_t LANADV_param[]={
	{"STBVendorID1", dalType_string,	0,	0,	NULL},
	{"STBVendorID2", dalType_string,	0,	0,	NULL},
	{"STBVendorID3", dalType_string,	0,	0,	NULL},
	{"STBVendorID4", dalType_string,	0,	0,	NULL},
	{"STBVendorID5", dalType_string,	0,	0,	NULL},
	{"TFTPServerName", dalType_string,	0,	0,	NULL},
	{"UPnPEnable", dalType_boolean,	0,	0,	NULL},
	{"NATTEnable", dalType_boolean,	0,	0,	NULL},
	{NULL,					0,	0,	0,	NULL},

};

void zcfgFeDalShowLanAdv(struct json_object *Jarray){
	int i, len = 0, count = 1;
	struct json_object *obj = NULL, *upnpArray = NULL, *upnpObj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("STB Vendor ID: \n");
	printf("Vendor ID 1: %s \n",json_object_get_string(json_object_object_get(obj, "STBVendorID1")));
	printf("Vendor ID 2: %s \n",json_object_get_string(json_object_object_get(obj, "STBVendorID2")));
	printf("Vendor ID 3: %s \n",json_object_get_string(json_object_object_get(obj, "STBVendorID3")));
	printf("Vendor ID 4: %s \n",json_object_get_string(json_object_object_get(obj, "STBVendorID4")));
	printf("Vendor ID 5: %s \n",json_object_get_string(json_object_object_get(obj, "STBVendorID5")));
	printf("\nTFTP Server: \n");
	printf("TFTP Server Name: %s \n",json_object_get_string(json_object_object_get(obj, "TFTPServerName")));
	printf("\nUPnP State: \n");
	printf("UPnP: %s \n",json_object_get_string(json_object_object_get(obj, "UPnPEnable")));
	printf("UPnP NAT-T: %s \n",json_object_get_string(json_object_object_get(obj, "NATTEnable")));
	if(json_object_get_boolean(json_object_object_get(obj, "UPnPEnable")) && json_object_get_boolean(json_object_object_get(obj, "NATTEnable"))){
#ifdef CONFIG_PACKAGE_zupnp
		upnpArray = json_object_array_get_idx(Jarray, 1);
		if(upnpArray){
			printf("%-6s %-60s %-25s %15s %-15s %-10s \n", "Index", "Description", "Destination IP Address", "External Port", "Internal Port", "Protocol");	
			len = json_object_array_length(upnpArray);
			for(i=0;i<len;i++){
				upnpObj = json_object_array_get_idx(upnpArray, i);
				printf("%-6d %-60s %-25s %15s %-15s %-10s \n", 
					count, 
					json_object_get_string(json_object_object_get(upnpObj, "Description")), 
					json_object_get_string(json_object_object_get(upnpObj, "Destination")), 
					json_object_get_string(json_object_object_get(upnpObj, "ExternalPort")), 
					json_object_get_string(json_object_object_get(upnpObj, "InternalPort")), 
					json_object_get_string(json_object_object_get(upnpObj, "Protocol")));
				count++;
			}
		}
#endif
#ifdef MINIUPNPD
		char tmpStr[256] = {'\0'};
		char proto[16] = {'\0'}, destIP[16] = {'\0'};
		int ExternalPort = 0, InternalPort = 0;
		FILE *fp = NULL;

		printf("%-6s %-60s %-25s %15s %-15s %-10s \n", "Index", "Description", "Destination IP Address", "External Port", "Internal Port", "Protocol");
		sprintf(tmpStr ,"iptables -t nat -n -L MINIUPNPD > %s", MINIUPNP_RULE_PATH);
		// system(tmpStr);

		if(ZCFG_SUCCESS != lanadvReqSendWithoutResponse(tmpStr))
			printf("Write to %s failed\n", MINIUPNP_RULE_PATH);

		fp = fopen(MINIUPNP_RULE_PATH, "r");
		if (fp != NULL) {
			memset(tmpStr, '\0', sizeof(tmpStr));
			while (fgets(tmpStr, 255, fp) != NULL) {
				// skip first and second line
				if (!strstr(tmpStr, "DNAT"))
					continue;

				sscanf(tmpStr, "DNAT       %s  --  %*s      %*s            %*s dpt:%d to:%[^:]:%d", proto, &ExternalPort, destIP, &InternalPort);
				printf("%-6d %-60s %-25s %15d %-15d %-10s \n", count, "", destIP, ExternalPort, InternalPort, proto);
				count ++;
			}
			fclose(fp);
			unlink(MINIUPNP_RULE_PATH);
		}
#endif
	}

}
#ifdef MINIUPNPD
/*!
 *  Get NAT-T port mapping list from miniupnpd daemon.
 *
 *  @param [out] retObj         NAT-T port mapping list
 *  @param [out] replyMsg       A preserved variable
 *
 *  @return ZCFG_SUCCESS                successfully send and get reply
 *          others                      failed
 *
 *  @note
 *          - retObj and replyMsg should both be initiated and freed by caller
 *          - retObj format: {"list":[{"Enable":,"Protocol":,"Destination":,"ExternalPort":,"InternalPort":,"Description":}]}
 */
zcfgRet_t zcfgFeDal_miniupnpd_NATTPortMap_GET(struct json_object *retObj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pramJobj = NULL;
	struct json_object *Jarray = NULL;
	char tmpStr[256] = {'\0'};
	char proto[16] = {'\0'}, destIP[16] = {'\0'};
	int ExternalPort = 0, InternalPort = 0, count = 1;
	FILE *fp = NULL;
	
	FILE *dalfp = NULL;
	char buf[256]={'\0'};
	char tmpRule[4096]={'\0'};
	char	miniDesr[128] = {'\0'};
	int miniExternalPort = 0,miniproto=0;

	Jarray = json_object_new_array();

	sprintf(tmpStr ,"iptables -t nat -n -L MINIUPNPD > %s", MINIUPNP_RULE_PATH);
	// system(tmpStr);

	if(ZCFG_SUCCESS != lanadvReqSendWithoutResponse(tmpStr))
		printf("Write to %s failed\n", MINIUPNP_RULE_PATH);

	fp = fopen(MINIUPNP_RULE_PATH, "r");
	if (fp != NULL) {
		memset(tmpStr, '\0', sizeof(tmpStr));
		while (fgets(tmpStr, 255, fp) != NULL) {
			// skip first and second line
			if (!strstr(tmpStr, "DNAT"))
				continue;

			pramJobj = json_object_new_object();
			sscanf(tmpStr, "DNAT       %s  --  %*s      %*s            %*s dpt:%d to:%[^:]:%d", proto, &ExternalPort, destIP, &InternalPort);
			json_object_object_add(pramJobj, "Enable", json_object_new_boolean(true));
			json_object_object_add(pramJobj, "Protocol", json_object_new_string(proto));
			json_object_object_add(pramJobj, "Destination", json_object_new_string(destIP));
			json_object_object_add(pramJobj, "ExternalPort", json_object_new_int(ExternalPort));
			json_object_object_add(pramJobj, "InternalPort", json_object_new_int(InternalPort));
			json_object_object_add(pramJobj, "Description", json_object_new_string(""));
			
			//printf("153curObj: proto:%s ExternalPort:%d\n",proto,ExternalPort);
			
			dalfp = fopen("/tmp/miniupnpDescr", "r");
			if(dalfp!=NULL){
				while (fgets(buf, sizeof(buf), dalfp) != NULL){
				
					sscanf(buf,"%d(zyxel)%d(zyxel)%[^\n]",&miniproto,&miniExternalPort,miniDesr);
				
					//printf("found:%d %s des%s  ex%d  proto%d\n", __LINE__, __func__,miniDesr,miniExternalPort,miniproto);
				
					if(miniproto==6 &&!strcmp("tcp",proto) && miniExternalPort==ExternalPort){
						//printf("%d %s\n", __LINE__, __func__);
						json_object_object_add(pramJobj, "Description", json_object_new_string(miniDesr));
					}
					if(miniproto==17 && !strcmp("udp",proto) && miniExternalPort==ExternalPort){
						//printf("%d %s\n", __LINE__, __func__);
						json_object_object_add(pramJobj, "Description", json_object_new_string(miniDesr));
					}
					
				}
			}
			else{
				//printf("%d %s\n", __LINE__, __func__);
				json_object_object_add(pramJobj, "Description", json_object_new_string("miniupnprule"));
			}
			/*else{
						printf("%d %s\n", __LINE__, __func__);
						json_object_object_add(pramJobj, "Description", json_object_new_string("miniupnprule"));
					}	*/
			fclose(dalfp);
			
			count ++;
			json_object_array_add(Jarray, pramJobj);
		}
		fclose(fp);
		unlink(MINIUPNP_RULE_PATH);
	}
	json_object_object_add(retObj, "list", Jarray);

	return ret;
}
#endif

zcfgRet_t lanadvReqSendWithoutResponse(char *payload)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int payloadLen = 0;
	void *sendBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;

	payloadLen = sizeof(zcfgMsg_t) + strlen(payload) + 1;
	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + payloadLen);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = ZCFG_MSG_FAKEROOT_COMMAND;
	sendMsgHdr->length = payloadLen;
	sendMsgHdr->dstEid = ZCFG_EID_ESMD;

	if(payloadLen > 0)
		strcpy((char *)(sendMsgHdr + 1), payload);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, NULL, 0);
	if(ret == ZCFG_SUCCESS) {
		//didn`t expect return response
		return ZCFG_INTERNAL_ERROR;
	}
	else if(ret == ZCFG_SUCCESS_AND_NO_REPLY) {
		ret = ZCFG_SUCCESS;
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	return ret;
}

#ifdef CONFIG_PACKAGE_zupnp
/*!
 *  Get NAT-T port mapping list from zupnp daemon.
 *
 *  @param [out] retObj         NAT-T port mapping list
 *  @param [out] replyMsg       A preserved variable
 *
 *  @return ZCFG_SUCCESS                successfully send and get reply
 *          others                      failed
 *
 *  @note
 *          - retObj and replyMsg should both be initiated and freed by caller
 *          - retObj format: {"list":[{"Enable":,"Protocol":,"Destination":,"ExternalPort":,"InternalPort":,"Description":}]}
 */
zcfgRet_t zcfgFeDal_NATTPortMap_GET(struct json_object *retObj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	//zcfgMsg_t *recvMsgHdr = NULL;
	char *recv_str = NULL;
	//int payloadLen = 0;

	sendMsgHdr = (zcfgMsg_t *)malloc(sizeof(zcfgMsg_t));
	sendMsgHdr->type = ZCFG_MSG_UPNP_PORTMAP_GET;
	sendMsgHdr->length = 0;
	sendMsgHdr->srcEid = ZCFG_EID_ZHTTPD;
	sendMsgHdr->dstEid = ZCFG_EID_ZUPNP;

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 2000);

	if(ret == ZCFG_SUCCESS){
		//recvMsgHdr = (zcfgMsg_t*)recvBuf;
		//payloadLen = recvMsgHdr->length;
		//recv_str = (char *)malloc(payloadLen);
		recv_str = (char *)recvBuf+sizeof(zcfgMsg_t);
		obj = json_tokener_parse(recv_str);
		json_object_object_add(retObj, "list", obj);
		ZOS_FREE(recvBuf);
	}

	return ret;
}
#endif

zcfgRet_t zcfgFeDal_LanAdv_GET(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgRet_t ret_portmap = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *obj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	struct json_object *stbObj = NULL;
#endif
	struct json_object *tftpObj = NULL;
	struct json_object *portMapObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
	struct json_object *upnpObj = NULL;
#endif

	IID_INIT(iid);
	obj = json_object_new_object();
	portMapObj = json_object_new_object();
#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_S_T_B_VENDOR_I_D, &iid, &stbObj);
	if(ret == ZCFG_SUCCESS){
		json_object_object_add(obj, "STBVendorID1", JSON_OBJ_COPY(json_object_object_get(stbObj, "STBVendorID1")));
		json_object_object_add(obj, "STBVendorID2", JSON_OBJ_COPY(json_object_object_get(stbObj, "STBVendorID2")));
		json_object_object_add(obj, "STBVendorID3", JSON_OBJ_COPY(json_object_object_get(stbObj, "STBVendorID3")));
		json_object_object_add(obj, "STBVendorID4", JSON_OBJ_COPY(json_object_object_get(stbObj, "STBVendorID4")));
		json_object_object_add(obj, "STBVendorID5", JSON_OBJ_COPY(json_object_object_get(stbObj, "STBVendorID5")));
		zcfgFeJsonObjFree(stbObj);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_TFTP_SERVER_NAME
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_T_F_T_P_SRV_NAME, &iid, &tftpObj);
	if(ret == ZCFG_SUCCESS){
		json_object_object_add(obj, "TFTPServerName", JSON_OBJ_COPY(json_object_object_get(tftpObj, "TFTPServerName")));
		zcfgFeJsonObjFree(tftpObj);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_UPNP_DEV, &iid, &upnpObj);
	if(ret == ZCFG_SUCCESS){
		json_object_object_add(obj, "UPnPEnable", JSON_OBJ_COPY(json_object_object_get(upnpObj, "Enable")));
		json_object_object_add(obj, "NATTEnable", JSON_OBJ_COPY(json_object_object_get(upnpObj, "X_ZYXEL_NATTEnable")));
		if(json_object_get_boolean(json_object_object_get(upnpObj, "Enable")) && json_object_get_boolean(json_object_object_get(upnpObj, "X_ZYXEL_NATTEnable"))){
#ifdef MINIUPNPD
			ret_portmap = zcfgFeDal_miniupnpd_NATTPortMap_GET(portMapObj,replyMsg);
#endif
#ifdef CONFIG_PACKAGE_zupnp
			ret_portmap = zcfgFeDal_NATTPortMap_GET(portMapObj,replyMsg);
#endif
			/* Ignore error of acquiring NAT-T port mapping list. */
			if (ret_portmap != ZCFG_SUCCESS){
				dbg_printf("%s: Get NAT-T Port Mapping List fail. Please try again later.\n",__FUNCTION__);
			}

		}
		zcfgFeJsonObjFree(upnpObj);
	}
#endif
	json_object_array_add(Jarray, obj);
	json_object_array_add(Jarray, JSON_OBJ_COPY(json_object_object_get(portMapObj, "list")));
	zcfgFeJsonObjFree(portMapObj);
	return ret;
}

zcfgRet_t zcfgFeDal_LanAdv_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *tftpObj = NULL, *origArry = NULL, *origObj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
	struct json_object *upnpObj = NULL;
#endif
	objIndex_t Iid = {0};
#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	struct json_object *stbObj = NULL;
	const char *stb1 = NULL, *stb2 = NULL, *stb3 = NULL, *stb4 = NULL, *stb5 = NULL;
#endif
	const char *thtpname = NULL;
	bool upnpEnable = false, natEnable = false, origupnpEnable = false, orignatEnable = false;

	origArry = json_object_new_array();
	IID_INIT(Iid);

#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	stb1 = json_object_get_string(json_object_object_get(Jobj, "STBVendorID1"));
	stb2 = json_object_get_string(json_object_object_get(Jobj, "STBVendorID2"));
	stb3 = json_object_get_string(json_object_object_get(Jobj, "STBVendorID3"));
	stb4 = json_object_get_string(json_object_object_get(Jobj, "STBVendorID4"));
	stb5 = json_object_get_string(json_object_object_get(Jobj, "STBVendorID5"));
#endif

	thtpname = json_object_get_string(json_object_object_get(Jobj, "TFTPServerName"));
 	zcfgFeDal_LanAdv_GET(NULL, origArry,NULL);
	origObj = json_object_array_get_idx(origArry, 0);

#ifdef CONFIG_ZCFG_BE_MODULE_STB_VENDOR_ID
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_S_T_B_VENDOR_I_D, &Iid, &stbObj);
	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "STBVendorID1"))
			json_object_object_add(stbObj, "STBVendorID1", json_object_new_string(stb1));
		if(json_object_object_get(Jobj, "STBVendorID2"))
			json_object_object_add(stbObj, "STBVendorID2", json_object_new_string(stb2));
		if(json_object_object_get(Jobj, "STBVendorID3"))
			json_object_object_add(stbObj, "STBVendorID3", json_object_new_string(stb3));
		if(json_object_object_get(Jobj, "STBVendorID4"))
			json_object_object_add(stbObj, "STBVendorID4", json_object_new_string(stb4));
		if(json_object_object_get(Jobj, "STBVendorID5"))
			json_object_object_add(stbObj, "STBVendorID5", json_object_new_string(stb5));

		ret = zcfgFeObjJsonSet(RDM_OID_S_T_B_VENDOR_I_D, &Iid, stbObj, NULL);
		zcfgFeJsonObjFree(stbObj);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_TFTP_SERVER_NAME
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_T_F_T_P_SRV_NAME, &Iid, &tftpObj);
	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "TFTPServerName"))
			json_object_object_add(tftpObj, "TFTPServerName", json_object_new_string(thtpname));
		
		ret = zcfgFeObjJsonSet(RDM_OID_T_F_T_P_SRV_NAME, &Iid, tftpObj, NULL);
		zcfgFeJsonObjFree(tftpObj);
	}
#endif
	/* upnp */
	if (json_object_object_get(Jobj, "UPnPEnable") || json_object_object_get(Jobj, "NATTEnable")) {
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
		ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_UPNP_DEV, &Iid, &upnpObj);
		if (ret != ZCFG_SUCCESS) {
			if(replyMsg) {
				strcpy(replyMsg, "Get UPnP object fail.");
			}
			return ret;
		}

		// prepare input
		origupnpEnable = json_object_get_boolean(json_object_object_get(origObj, "UPnPEnable"));
		orignatEnable = json_object_get_boolean(json_object_object_get(origObj, "NATTEnable"));

		if(json_object_object_get(Jobj, "UPnPEnable"))
			upnpEnable = json_object_get_boolean(json_object_object_get(Jobj, "UPnPEnable"));
		else
			upnpEnable = json_object_get_boolean(json_object_object_get(origObj, "UPnPEnable"));

		if(json_object_object_get(Jobj, "NATTEnable"))
			natEnable = json_object_get_boolean(json_object_object_get(Jobj, "NATTEnable"));
		else
			natEnable = json_object_get_boolean(json_object_object_get(origObj, "NATTEnable"));

		// validate input
		if (!upnpEnable && natEnable) {
			if(replyMsg) {
				strcpy(replyMsg, "UPnP must be enabled before enabling UPnP NAT-T.");
			}
			zcfgFeJsonObjFree(upnpObj);
			return ZCFG_REQUEST_REJECT;
		}

		/* If new setting is the same with old, it should not be set to data model 
		   because upnp daemon will restart and NAT-T port mapping rules will disapear */
		if ( (upnpEnable == origupnpEnable) && (natEnable == orignatEnable) ) {
			dbg_printf("%s: The current setting and new setting are the same. Do not set data model.\n",__FUNCTION__);
		}
		else {
			json_object_object_add(upnpObj, "Enable", json_object_new_boolean(upnpEnable));
			json_object_object_add(upnpObj, "X_ZYXEL_NATTEnable", json_object_new_boolean(natEnable));
			ret = zcfgFeObjJsonSet(RDM_OID_UPNP_DEV, &Iid, upnpObj, NULL);
		}
		zcfgFeJsonObjFree(upnpObj);
#else
		printf("%s : UPnP not support\n", __FUNCTION__);
		ret = ZCFG_REQUEST_REJECT;
#endif
	}

	return ret;
}


zcfgRet_t zcfgFeDalLanAdvance(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT")){
		ret = zcfgFeDal_LanAdv_Edit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_LanAdv_GET(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}


