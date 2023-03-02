#include <stdlib.h>

#include <json/json.h>
#include <time.h>

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
#include "zcfg_net.h"
#include "wanDev_parameter.h"

#include "wanDev_api.h"
#include "zos_api.h"

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
#define Jgeti(obj, key) json_object_get_int(json_object_object_get(obj, key))
#endif

#define WAN_ADD_INDIVIDUAL

extern bool isBondingWanProcess;

extern tr98Object_t tr98Obj[];

extern zcfgRet_t zcfgFeTr181IfaceStackHigherLayerGet(char *, char *);

zcfgRet_t WANIpConnObjDel(char *);
zcfgRet_t WANPppConnObjDel(char *);


#ifdef CONFIG_TAAAB_DSL_BINDING
/*GPV InternetGatewayDevice.WANDevice.i.WANCommonInterfaceConfig.NumberOfActiveConnections*/
int ActConnectionGet(char *wanType){
	struct json_object *WanLanListJarray = NULL,*WanLanObj = NULL;
	const char *Name, *ConnectionType,*Type, *LinkType;
	int len=0,i;
	bool WANConnectionReady;
	getWanLanList(&WanLanListJarray);
	len = json_object_array_length(WanLanListJarray);
	int ActWanCounter=0;
	for(i=0; i<len; i++){
					   
		WanLanObj = json_object_array_get_idx(WanLanListJarray, i);
		Name = json_object_get_string(json_object_object_get(WanLanObj, "Name"));
		Type = json_object_get_string(json_object_object_get(WanLanObj, "Type"));
		LinkType = json_object_get_string(json_object_object_get(WanLanObj, "LinkType"));
		WANConnectionReady = json_object_get_boolean(json_object_object_get(WanLanObj, "WANConnectionReady"));
	 			
	if(LinkType && !strcmp(wanType,LinkType) && WANConnectionReady)
	   {
		ActWanCounter=ActWanCounter+1;
		}
	}
	json_object_put(WanLanListJarray);
	return ActWanCounter;
} 
#endif



/* InternetGatewayDevice.WANDevice.i */
zcfgRet_t WANDeviceObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char wanConnDevName[128] = {0};
	int count = 0;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret=zcfgFe98To181MappingNameGet(tr98FullPathName, tr181Obj)) != ZCFG_SUCCESS) {
		uint8_t idx;
		sscanf(tr98FullPathName, "InternetGatewayDevice.WANDevice.%hhu", &idx);

		/*  The instance number of WANDevice.i will be continuous because of the tr98 to tr181 mapping table. 
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANDevice object.
		 */
#ifdef ZYXEL_SUPPORT_TR098_BRCM_BONDING_LINE
		if (idx < 13)
			return ZCFG_NO_SUCH_OBJECT;
#else
		if (idx < 5)
			return ZCFG_NO_SUCH_OBJECT;
#endif
		return ret;
	}

	sprintf(wanConnDevName, "%s.WANConnectionDevice.%d", tr98FullPathName, (count+1));

	while((ret = zcfgFe98To181MappingNameGet(wanConnDevName, tr181Obj)) == ZCFG_SUCCESS) {
		count++;
		sprintf(wanConnDevName, "%s.WANConnectionDevice.%d", tr98FullPathName, (count+1));
	}

	*tr98Jobj = json_object_new_object();
	json_object_object_add(*tr98Jobj, "WANConnectionNumberOfEntries", json_object_new_int(count));

#if CONFIG_XPON_SUPPORT   // 1:pon, 2:3g, 3:eth
	if(strstr(tr98FullPathName, "WANDevice.2") ){
#else
	if(strstr(tr98FullPathName, "WANDevice.4") ){
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
		objIndex_t wwanBackupIid;
		rdm_ZyWWANBackup_t *wwanBackupobj = NULL;
		
		IID_INIT(wwanBackupIid);
		if(feObjStructGet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanBackupIid, (void **)&wwanBackupobj, updateFlag) == ZCFG_SUCCESS) {
			json_object_object_add(*tr98Jobj, "X_ZYXEL_3GEnable", json_object_new_boolean(wwanBackupobj->Enable));
			json_object_object_add(*tr98Jobj, "X_ZYXEL_PinNumber", json_object_new_string(wwanBackupobj->PIN));
			zcfgFeObjStructFree(wwanBackupobj);
		}
#else
		zcfgLog(ZCFG_LOG_NOTICE, "%s : WWAN Backup not support\n", __FUNCTION__);
#endif
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.WANDevice.i */
zcfgRet_t WANDeviceObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
#if CONFIG_XPON_SUPPORT   // 1:pon, 2:3g, 3:eth
	if(strstr(tr98FullPathName, "WANDevice.2") ){
#else
	if(strstr(tr98FullPathName, "WANDevice.4") ){
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
		zcfgRet_t ret;
		objIndex_t wwanBackupIid;
		struct json_object *wwanBackupJobj = NULL;
		struct json_object *paramValue = NULL;
		struct json_object *tmpObj = NULL;
	
		zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	
		IID_INIT(wwanBackupIid);
		if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanBackupIid, &wwanBackupJobj)) != ZCFG_SUCCESS)
			return ret;
		if(multiJobj){
			tmpObj = wwanBackupJobj;
			wwanBackupJobj = NULL;
			wwanBackupJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanBackupIid, multiJobj, tmpObj);
		}
	
		paramValue = json_object_object_get(tr98Jobj, "X_ZYXEL_3GEnable");
		if(paramValue != NULL) {
			json_object_object_add(wwanBackupJobj, "Enable", JSON_OBJ_COPY(paramValue));
		}
		
		paramValue = json_object_object_get(tr98Jobj, "X_ZYXEL_PinNumber");
		if(paramValue != NULL) {
			json_object_object_add(wwanBackupJobj, "PIN", JSON_OBJ_COPY(paramValue));
		}
		
		/*Set */
		if(multiJobj){
			json_object_put(tmpObj);
		}
		else{
			if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanBackupIid, wwanBackupJobj, NULL)) != ZCFG_SUCCESS ) {
				json_object_put(wwanBackupJobj);
				return ret;
			}
			json_object_put(wwanBackupJobj);
		}
	
		return ZCFG_SUCCESS;
#else
		return ZCFG_INVALID_OBJECT;
#endif
	}else{
		int nparam = 0;
		struct json_object *faultobj = json_object_new_object();
		if(json_object_object_get(tr98Jobj, "X_ZYXEL_3GEnable")){
			nparam++;
			json_object_object_add(faultobj, "X_ZYXEL_3GEnable", json_object_new_int(TR069_INVALID_PARAMETER_NAME));
		}

		if(json_object_object_get(tr98Jobj, "X_ZYXEL_PinNumber")){
			nparam++;
			json_object_object_add(faultobj, "X_ZYXEL_PinNumber", json_object_new_int(TR069_INVALID_PARAMETER_NAME));
		}


		if(nparam){
			zcfgFeJsonMultiObjSetAddFaultObj(multiJobj, faultobj);
		}else
			json_object_put(faultobj);

		return ZCFG_NO_SUCH_PARAMETER;
	}
}

#define LT_PARM_EMPTY  "None"
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
/* InternetGatewayDevice.WANDevice.1.WANDSLDiagnostics */
zcfgRet_t WANDslDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	rdm_DslDiagAdslLineTest_t *DslDiagObj = NULL;
	struct json_object *dslDiagJObj = NULL, *tr181ParamObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char *tr181ParamStr = NULL, *tr181ParamStateStr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	if(!strstr(tr98FullPathName, ".WANDevice")) return ZCFG_NO_SUCH_OBJECT;

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLDiagnostics");
	*ptr = '\0';
	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, tr181Obj)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}
	if(strstr(tr181Obj, "DSL") == NULL) {
		zcfgLog(ZCFG_LOG_WARNING, "%s : Not a DSL Interface\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}
	*tr98Jobj = json_object_new_object();
		
	IID_INIT(objIid);

	objIid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_DSL_DIAG_ADSL_LINE_TEST, &objIid, &dslDiagJObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s : retrieve ADSLLineTest object Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	tr181ParamStr = json_object_get_string(json_object_object_get(dslDiagJObj, "Interface"));

	printf("%s: tr181Obj %s\n", __FUNCTION__, tr181Obj);

	if(!tr181ParamStr) {
		printf("%s: tr181ParamStr 0\n", __FUNCTION__);
		usleep(10000);
	}
	
	printf("%s: tr181ParamStr %s\n", __FUNCTION__, tr181ParamStr);
	usleep(10000);

	if(tr181ParamStr && strstr(tr181ParamStr, tr181Obj)) {
		tr181ParamObj = json_object_object_get(dslDiagJObj, "DiagnosticsState");
		tr181ParamStateStr = json_object_get_string(tr181ParamObj);
	}

	if(tr181ParamStateStr && strstr(tr181ParamStateStr, "Complete")) {
		paramList = tr98Obj[handler].parameter;
		json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(tr181ParamObj));
		paramList ++;
		while(paramList->name != NULL) {
			struct json_object *paramValue = json_object_object_get(dslDiagJObj, paramList->name);
			if(paramValue) json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));

			paramList++;
		}
		json_object_object_add(*tr98Jobj, "GAINSpsds", json_object_new_string(LT_PARM_EMPTY)); // In tr98, GAINSpsds parameter is DEPRECATED.
	}else {
		if(!tr181ParamStateStr) {
			json_object_object_add(*tr98Jobj, "LoopDiagnosticsState", json_object_new_string("None"));
		}else{
			if(tr181ParamObj) json_object_object_add(*tr98Jobj, "LoopDiagnosticsState", JSON_OBJ_COPY(tr181ParamObj));
			else json_object_object_add(*tr98Jobj, "LoopDiagnosticsState", json_object_new_string("None"));
		}

		{
			DslDiagObj = (rdm_DslDiagAdslLineTest_t *)malloc(sizeof(rdm_DslDiagAdslLineTest_t));
			memset(DslDiagObj, 0, sizeof(rdm_DslDiagAdslLineTest_t));
			json_object_object_add(*tr98Jobj, "ACTPSDds", json_object_new_int(DslDiagObj->ACTPSDds));
			json_object_object_add(*tr98Jobj, "ACTPSDus", json_object_new_int(DslDiagObj->ACTPSDus));
			json_object_object_add(*tr98Jobj, "ACTATPds", json_object_new_int(DslDiagObj->ACTATPds));
			json_object_object_add(*tr98Jobj, "ACTATPus", json_object_new_int(DslDiagObj->ACTATPus));
			json_object_object_add(*tr98Jobj, "HLINSCds", json_object_new_int(DslDiagObj->HLINSCds));
			json_object_object_add(*tr98Jobj, "HLINSCus", json_object_new_int(DslDiagObj->HLINSCus));
			json_object_object_add(*tr98Jobj, "HLINGds", json_object_new_int(DslDiagObj->HLINGds));
			json_object_object_add(*tr98Jobj, "HLINGus", json_object_new_int(DslDiagObj->HLINGus));
			json_object_object_add(*tr98Jobj, "HLOGGds", json_object_new_int(DslDiagObj->HLOGGds));
			json_object_object_add(*tr98Jobj, "HLOGGus", json_object_new_int(DslDiagObj->HLOGGus));
			json_object_object_add(*tr98Jobj, "HLOGpsds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "HLOGpsus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "HLOGMTds", json_object_new_int(DslDiagObj->HLOGMTds));
			json_object_object_add(*tr98Jobj, "HLOGMTus", json_object_new_int(DslDiagObj->HLOGMTus));
			json_object_object_add(*tr98Jobj, "LATNpbds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "LATNpbus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "SATNds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "SATNus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "HLINpsds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "HLINpsus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "QLNGds", json_object_new_int(DslDiagObj->QLNGds));
			json_object_object_add(*tr98Jobj, "QLNGus", json_object_new_int(DslDiagObj->QLNGus));
			json_object_object_add(*tr98Jobj, "QLNpsds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "QLNpsus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "QLNMTds", json_object_new_int(DslDiagObj->QLNMTds));
			json_object_object_add(*tr98Jobj, "QLNMTus", json_object_new_int(DslDiagObj->QLNMTus));
			json_object_object_add(*tr98Jobj, "SNRGds", json_object_new_int(DslDiagObj->SNRGds));
			json_object_object_add(*tr98Jobj, "SNRGus", json_object_new_int(DslDiagObj->SNRGus));
			json_object_object_add(*tr98Jobj, "SNRpsds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "SNRpsus", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "SNRMTds", json_object_new_int(DslDiagObj->SNRMTds));
			json_object_object_add(*tr98Jobj, "SNRMTus", json_object_new_int(DslDiagObj->SNRMTus));
			json_object_object_add(*tr98Jobj, "BITSpsds", json_object_new_string(LT_PARM_EMPTY));
			json_object_object_add(*tr98Jobj, "BITSpsus", json_object_new_string(LT_PARM_EMPTY));
			// GAINSpsds DEPRECATED, but reply
			json_object_object_add(*tr98Jobj, "GAINSpsds", json_object_new_string(LT_PARM_EMPTY));
			free(DslDiagObj);
		}
	}
	zcfgFeJsonObjFree(dslDiagJObj);

	return ZCFG_SUCCESS;
}

#define ADSL_DSL_CHANNEL_INST_NUM  1
zcfgRet_t WANDslDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	struct json_object *DslDiagObj = NULL, *dslChannelObj = NULL;
	struct json_object *tr98ParamObj = NULL;
	struct json_object *tmpObj = NULL;
	const char *tr98ParamStr, *tr181ParamStr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	if(!strstr(tr98FullPathName, ".WANDevice")) return ZCFG_NO_SUCH_OBJECT;

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLDiagnostics");
	*ptr = '\0';
		
	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, tr181Obj)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}
	printf("%s: tr181 %s\n", __FUNCTION__, tr181Obj);
	if(!strstr(tr181Obj, "DSL.Channel"))
		return ZCFG_NO_SUCH_OBJECT;
	
	if(!(tr98ParamObj = json_object_object_get(tr98Jobj, "LoopDiagnosticsState"))) {
		zcfgLog(ZCFG_LOG_ERR, "%s : retrieve tr98 LoopDiagnosticsState Fail\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}

	tr98ParamStr = json_object_get_string(tr98ParamObj);
	if(!strstr(tr98ParamStr, "Requested"))
		return ZCFG_INVALID_PARAM_VALUE;

	IID_INIT(objIid);
	sscanf(tr181Obj, "DSL.Channel.%hhu", &objIid.idx[0]);
	if(!objIid.idx[0] || objIid.idx[0] > 2)
		return ZCFG_NO_SUCH_OBJECT;

	objIid.level = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_DSL_CHANNEL, &objIid, &dslChannelObj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s : retrieve DSL Channel object Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	tr181ParamStr = json_object_get_string(json_object_object_get(dslChannelObj, "Status"));
	if(!tr181ParamStr || (tr181ParamStr && strcmp(tr181ParamStr, "Up"))) {
		zcfgLog(ZCFG_LOG_INFO, "%s: DSL Channel status not active!\n", __FUNCTION__);
		json_object_put(dslChannelObj);
		return ZCFG_REQUEST_REJECT;
	}
	json_object_put(dslChannelObj);

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DSL_DIAG_ADSL_LINE_TEST, &objIid, &DslDiagObj)) != ZCFG_SUCCESS){
		//zcfgLog(ZCFG_LOG_ERR, "%s : retrieve ADSLLineTest object Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	tr181ParamStr = json_object_get_string(json_object_object_get(DslDiagObj, "DiagnosticsState"));
	if((!tr181ParamStr) || strstr(tr181ParamStr, "Requested")) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : DSL.Diagnostics is on processing\n", __FUNCTION__);
		json_object_put(DslDiagObj);
		return ZCFG_REQUEST_REJECT;
	}
	json_object_object_add(DslDiagObj, "DiagnosticsState", json_object_new_string("Requested"));
	json_object_object_add(DslDiagObj, "Interface", json_object_new_string(tr181Obj));

	if(multiJobj) {
		tmpObj = DslDiagObj;
		DslDiagObj = NULL;
		DslDiagObj = zcfgFeJsonMultiObjAppend(RDM_OID_DSL_DIAG_ADSL_LINE_TEST, &objIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}else {
		ret = zcfgFeObjJsonSet(RDM_OID_DSL_DIAG_ADSL_LINE_TEST, &objIid, DslDiagObj, NULL);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set DSL.Diagnostics Fail\n", __FUNCTION__);
		}
		else {
			zcfgLog(ZCFG_LOG_DEBUG, "%s : Set DSL.Diagnostics Success\n", __FUNCTION__);
		}

		json_object_put(DslDiagObj);
	}

	return ret;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_DSL

/* InternetGatewayDevice.WANDevice.i.WANCommonInterfaceConfig */
zcfgRet_t WANCommIfaceCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
#if 0
	char tr181ObjPath[32] = {0};
	char higherLayerPath[128] = {0};
#endif	
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	objIndex_t ipAddrIid;
	bool enableForInternet = false;
	uint64_t BytesSent = 0;
	uint64_t BytesReceived = 0;
	uint64_t PacketsSent = 0;
	uint64_t PacketsReceived = 0;
	char *wanType=NULL;



	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANCommonInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, tr181Obj)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}

	*tr98Jobj = json_object_new_object();

#if 0
	ZOS_STRCPY_M(tr181ObjPath, tr181Obj);
	/*specical case to get parameter "EnabledForInternet"*/
	while((ret = zcfgFeTr181IfaceStackHigherLayerGet(tr181ObjPath, higherLayerPath)) == ZCFG_SUCCESS) {
		if(strstr(higherLayerPath, "IP.Interface") != NULL) {
			rdm_IpIface_t *ipIntfObj = NULL;
			rdm_IpIfaceV4Addr_t *v4AddrObj = NULL;
			rdm_IpIfaceV6Addr_t *v6AddrObj = NULL;
			IID_INIT(objIid);
			IID_INIT(ipAddrIid);
			objIid.level = 1;
			sscanf(higherLayerPath, "IP.Interface.%hhu", &objIid.idx[0]);
			if((ret = feObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIntfObj, updateFlag)) == ZCFG_SUCCESS){
	
				if(ipIntfObj->IPv4Enable){				
					if((ret = feObjStructGetSubNext(RDM_OID_IP_IFACE_V4_ADDR, &objIid,&ipAddrIid, (void **)&v4AddrObj, updateFlag)) == ZCFG_SUCCESS){
						if(!strcmp(v4AddrObj->Status, TR181_ENABLED))
							enableForInternet = true;
						else
							enableForInternet = false;
						
						zcfgFeObjStructFree(v4AddrObj);					
					}
				}
				else if(ipIntfObj->IPv6Enable){			
					if((ret = feObjStructGetSubNext(RDM_OID_IP_IFACE_V6_ADDR, &objIid,&ipAddrIid, (void **)&v6AddrObj, updateFlag)) == ZCFG_SUCCESS){
						if(strcmp(v6AddrObj->Status, TR181_ENABLED))
							enableForInternet = true;
						else
							enableForInternet = false;

						zcfgFeObjStructFree(v6AddrObj);
					}
				}
				
				json_object_object_add(*tr98Jobj, "EnabledForInternet" ,json_object_new_boolean(enableForInternet));
			}

			zcfgFeObjStructFree(ipIntfObj);
			break;
		}

		ZOS_STRCPY_M(tr181ObjPath, higherLayerPath);
	}
#endif

/*From DefaultGateway get interface name*/
	rdm_RoutingRouter_t *routeObj = NULL;
	objIndex_t routeIid;
	IID_INIT(routeIid);
	routeIid.level = 1;
	routeIid.idx[0] = 1;
	char * pch;
	char *comma = ",";
	char defaultGateway[128]={0};
	char tr98ObjPath[128] = {0};
	char interfaceName[128] = {0};
	char tr181TmpName[1025]={0};

	if((ret=zcfgFeObjStructGet(RDM_OID_ROUTING_ROUTER, &routeIid, (void **)&routeObj)) == ZCFG_SUCCESS){
		ZOS_STRCPY_M(defaultGateway,routeObj->X_ZYXEL_ActiveDefaultGateway);
		zcfgFeObjStructFree(routeObj);

		pch = strtok(defaultGateway,comma);
		while (pch != NULL){
			ZOS_STRCPY_M(interfaceName,pch);
			if(strstr(interfaceName, "IP.Interface") != NULL){
				rdm_IpIface_t *ipIntfObj = NULL;
				rdm_IpIfaceV4Addr_t *v4AddrObj = NULL;
				rdm_IpIfaceV6Addr_t *v6AddrObj = NULL;
				IID_INIT(objIid);
				IID_INIT(ipAddrIid);
				objIid.level = 1;
				sscanf(interfaceName, "IP.Interface.%hhu", &objIid.idx[0]);
				if((ret = feObjStructGet(RDM_OID_IP_IFACE, &objIid, (void **)&ipIntfObj, updateFlag)) == ZCFG_SUCCESS){
					if(ipIntfObj->IPv4Enable){				
						if((ret = feObjStructGetSubNext(RDM_OID_IP_IFACE_V4_ADDR, &objIid,&ipAddrIid, (void **)&v4AddrObj, updateFlag)) == ZCFG_SUCCESS){
							if(!strcmp(v4AddrObj->Status, TR181_ENABLED)){	
								if(strstr(ipIntfObj->LowerLayers, "PPP")){
									ZOS_STRCPY_M(tr181TmpName, ipIntfObj->LowerLayers);
								}
								else{
									sprintf(tr181TmpName, "IP.Interface.%hhu", objIid.idx[0]);
								}
								
								 if((ret = zcfgFe181To98MappingNameGet(tr181TmpName, tr98ObjPath)) == ZCFG_SUCCESS) {
									if(!strncmp(tr98ObjPath,tr98FullPathName,strlen("InternetGatewayDevice.WANDevice.i")))
										enableForInternet = true;
								}
							}
							zcfgFeObjStructFree(v4AddrObj);
						}					
					}
					else if(ipIntfObj->IPv6Enable){			
						if((ret = feObjStructGetSubNext(RDM_OID_IP_IFACE_V6_ADDR, &objIid,&ipAddrIid, (void **)&v6AddrObj, updateFlag)) == ZCFG_SUCCESS){
							if(strcmp(v6AddrObj->Status, TR181_ENABLED)){
								
								if(strstr(ipIntfObj->LowerLayers, "PPP")){
									ZOS_STRCPY_M(tr181TmpName, ipIntfObj->LowerLayers);
								}
								else{
									sprintf(tr181TmpName, "IP.Iterface.%hhu", objIid.idx[0]);
								}
								
								if((ret = zcfgFe181To98MappingNameGet(tr181TmpName, tr98ObjPath)) == ZCFG_SUCCESS) {
									if(!strncmp(tr98ObjPath,tr98FullPathName,strlen("InternetGatewayDevice.WANDevice.i")))
										enableForInternet = true;
								}
							}
							zcfgFeObjStructFree(v6AddrObj);
						}
					}
				
					json_object_object_add(*tr98Jobj, "EnabledForInternet" ,json_object_new_boolean(enableForInternet));
				}

				zcfgFeObjStructFree(ipIntfObj);
			}	
			pch = strtok (NULL, comma);
		}
	}

	if(!json_object_object_get(*tr98Jobj,"EnabledForInternet")){
		json_object_object_add(*tr98Jobj, "EnabledForInternet" ,json_object_new_boolean(enableForInternet));
	}
	/*Hard coding*/
	json_object_object_add(*tr98Jobj, "WANAccessProvider", json_object_new_string(""));
	json_object_object_add(*tr98Jobj, "MaximumActiveConnections", json_object_new_int(0));
	json_object_object_add(*tr98Jobj, "NumberOfActiveConnections", json_object_new_int(0));
#ifdef TAAAB_CUSTOMIZATION
	json_object_object_add(*tr98Jobj, "MaximumActiveConnections", json_object_new_int(8));
#endif
	/*Init value*/
	json_object_object_add(*tr98Jobj, "PhysicalLinkStatus", json_object_new_string("Unavailable"));
	json_object_object_add(*tr98Jobj, "Layer1UpstreamMaxBitRate", json_object_new_int(0));
	json_object_object_add(*tr98Jobj, "Layer1DownstreamMaxBitRate", json_object_new_int(0));
	IID_INIT(objIid);
	objIid.level = 1;

	if(strstr(tr181Obj, "DSL") != NULL) {
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
		rdm_DslChannel_t *dslChannel = NULL;
		char highLayer[257] = {0};

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		char wanDslType[] = " DSL";
		char *type = wanDslType;
		int sn = sscanf(tr181Obj, "DSL.Channel.%hhu", &objIid.idx[0]);
		type = (sn == 1) ? type : type+1;
		*type = (sn == 1) ? ((objIid.idx[0] == 1) ? 0x41 : 0x56) : *type;
		json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string(type));
#else
		json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string("DSL"));
		sscanf(tr181Obj, "DSL.Channel.%hhu", &objIid.idx[0]);
#endif

		if(feObjStructGet(RDM_OID_DSL_CHANNEL, &objIid, (void **)&dslChannel, updateFlag) == ZCFG_SUCCESS) {
			json_object_object_add(*tr98Jobj, "Layer1UpstreamMaxBitRate", json_object_new_int(dslChannel->UpstreamCurrRate * 1000));
			json_object_object_add(*tr98Jobj, "Layer1DownstreamMaxBitRate", json_object_new_int(dslChannel->DownstreamCurrRate * 1000));

			if(!strcmp(dslChannel->Status, "Up") || !strcmp(dslChannel->Status, "Down")) {
				json_object_object_add(*tr98Jobj, "PhysicalLinkStatus", json_object_new_string(dslChannel->Status));
			}

			zcfgFeObjStructFree(dslChannel);
		}

		if (zcfgFeTr181IfaceStackHigherLayerGet(tr181Obj, highLayer) == ZCFG_SUCCESS) {
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			objIndex_t atmObjIid = {0};
#endif
			objIndex_t ptmObjIid = {0};
			if (strstr(highLayer, "ATM")) {
				wanType="ATM";
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
				rdm_AtmLinkSt_t *atmLinkSt = NULL;
				while(feObjStructGetNext(RDM_OID_ATM_LINK_ST, &atmObjIid, (void **)&atmLinkSt, TR98_GET_UPDATE) == ZCFG_SUCCESS) {
					BytesSent += atmLinkSt->BytesSent;
					BytesReceived += atmLinkSt->BytesReceived;
					PacketsSent += atmLinkSt->PacketsSent;
					PacketsReceived += atmLinkSt->PacketsReceived;
					zcfgFeObjStructFree(atmLinkSt);
				}
#else
				zcfgLog(ZCFG_LOG_NOTICE, "%s : ATM Link not support\n", __FUNCTION__);
#endif
			}
			else if (strstr(highLayer, "PTM")) {
				wanType="PTM";
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
				rdm_PtmLinkSt_t *ptmLinkSt = NULL;
				while(feObjStructGetNext(RDM_OID_PTM_LINK_ST, &ptmObjIid, (void **)&ptmLinkSt, TR98_GET_UPDATE) == ZCFG_SUCCESS) {
					BytesSent += ptmLinkSt->BytesSent;
					BytesReceived += ptmLinkSt->BytesReceived;
					PacketsSent += ptmLinkSt->PacketsSent;
					PacketsReceived += ptmLinkSt->PacketsReceived;
					zcfgFeObjStructFree(ptmLinkSt);
				}
#else
				zcfgLog(ZCFG_LOG_NOTICE, "%s : PTM Link not support\n", __FUNCTION__);
#endif
			}
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
		json_object_object_add(*tr98Jobj, "NumberOfActiveConnections", json_object_new_int(ActConnectionGet(wanType)));
#endif
		json_object_object_add(*tr98Jobj, "TotalBytesSent", json_object_new_int(BytesSent));
		json_object_object_add(*tr98Jobj, "TotalBytesReceived", json_object_new_int(BytesReceived));
		json_object_object_add(*tr98Jobj, "TotalPacketsSent", json_object_new_int(PacketsSent));
		json_object_object_add(*tr98Jobj, "TotalPacketsReceived", json_object_new_int(PacketsReceived));
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_DSL
	}
	else if(strstr(tr181Obj, "Ethernet") != NULL) {
		wanType="ETH";
		rdm_EthIface_t *ethIface = NULL;
		rdm_EthIntfSt_t *ethIfaceSt = NULL;
		
		json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string("Ethernet"));
		sscanf(tr181Obj, "Ethernet.Interface.%hhu", &objIid.idx[0]);

		if(feObjStructGet(RDM_OID_ETH_IFACE, &objIid, (void **)&ethIface, updateFlag) == ZCFG_SUCCESS) {
			json_object_object_add(*tr98Jobj, "Layer1UpstreamMaxBitRate", json_object_new_int(ethIface->MaxBitRate * 1000000));
			json_object_object_add(*tr98Jobj, "Layer1DownstreamMaxBitRate", json_object_new_int(ethIface->MaxBitRate * 1000000));

			if(!strcmp(ethIface->Status, "Up") || !strcmp(ethIface->Status, "Down")) {
				json_object_object_add(*tr98Jobj, "PhysicalLinkStatus", json_object_new_string(ethIface->Status));
			}

			zcfgFeObjStructFree(ethIface);
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
		json_object_object_add(*tr98Jobj, "NumberOfActiveConnections", json_object_new_int(ActConnectionGet(wanType)));
#endif

		if(feObjStructGet(RDM_OID_ETH_INTF_ST, &objIid, (void **)&ethIfaceSt, updateFlag) == ZCFG_SUCCESS) {
			json_object_object_add(*tr98Jobj, "TotalBytesSent", json_object_new_int(ethIfaceSt->BytesSent));
			json_object_object_add(*tr98Jobj, "TotalBytesReceived", json_object_new_int(ethIfaceSt->BytesReceived));
			json_object_object_add(*tr98Jobj, "TotalPacketsSent", json_object_new_int(ethIfaceSt->PacketsSent));
			json_object_object_add(*tr98Jobj, "TotalPacketsReceived", json_object_new_int(ethIfaceSt->PacketsReceived));
			zcfgFeObjStructFree(ethIfaceSt);
		}
	}
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	else if(strstr(tr181Obj, "Optical") != NULL) {
		wanType="Optical";
		rdm_OptIntf_t *optIface = NULL;
		rdm_OptIntfSt_t *optIfaceSt = NULL;
		
		json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string("Optical"));
		sscanf(tr181Obj, "Optical.Interface.%hhu", &objIid.idx[0]);

		if(feObjStructGet(RDM_OID_OPT_INTF, &objIid, (void **)&optIface, updateFlag) == ZCFG_SUCCESS) {
			if(!strcmp(optIface->Status, "Up") || !strcmp(optIface->Status, "Down")) {
				json_object_object_add(*tr98Jobj, "PhysicalLinkStatus", json_object_new_string(optIface->Status));
			}

			zcfgFeObjStructFree(optIface);
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
		json_object_object_add(*tr98Jobj, "NumberOfActiveConnections", json_object_new_int(ActConnectionGet(wanType)));
#endif
		if(feObjStructGet(RDM_OID_OPT_INTF_ST, &objIid, (void **)&optIfaceSt, updateFlag) == ZCFG_SUCCESS) {
			json_object_object_add(*tr98Jobj, "TotalBytesSent", json_object_new_int(optIfaceSt->BytesSent));
			json_object_object_add(*tr98Jobj, "TotalBytesReceived", json_object_new_int(optIfaceSt->BytesReceived));
			json_object_object_add(*tr98Jobj, "TotalPacketsSent", json_object_new_int(optIfaceSt->PacketsSent));
			json_object_object_add(*tr98Jobj, "TotalPacketsReceived", json_object_new_int(optIfaceSt->PacketsReceived));
			zcfgFeObjStructFree(optIfaceSt);
		}
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	else if(strstr(tr181Obj, "USB") != NULL) {
		wanType="USB";
		rdm_UsbIntf_t *usbIface = NULL;
		rdm_UsbIntfStat_t *usbIfaceSt = NULL;
		char usbIfaceStatus[20] = {0};
		
		json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string("3G"));
		sscanf(tr181Obj, "USB.Interface.%hhu", &objIid.idx[0]);

		if(feObjStructGet(RDM_OID_USB_INTF, &objIid, (void **)&usbIface, updateFlag) == ZCFG_SUCCESS) {
			ZOS_STRCPY_M(usbIfaceStatus, usbIface->Status);
			if(!strcmp(usbIface->Status, "Up") || !strcmp(usbIface->Status, "Down")) {
				json_object_object_add(*tr98Jobj, "PhysicalLinkStatus", json_object_new_string(usbIface->Status));
			}

			zcfgFeObjStructFree(usbIface);
		}

		if(!strcmp(usbIfaceStatus, "Up")){
			if(feObjStructGet(RDM_OID_USB_INTF_STAT, &objIid, (void **)&usbIfaceSt, updateFlag) == ZCFG_SUCCESS) {
				BytesSent = usbIfaceSt->BytesSent;
				BytesReceived = usbIfaceSt->BytesReceived;
				PacketsSent = usbIfaceSt->PacketsSent;
				PacketsReceived = usbIfaceSt->PacketsReceived;
				zcfgFeObjStructFree(usbIfaceSt);
			}
		}
#ifdef CONFIG_TAAAB_DSL_BINDING
		json_object_object_add(*tr98Jobj, "NumberOfActiveConnections", json_object_new_int(ActConnectionGet(wanType)));
#endif
		json_object_object_add(*tr98Jobj, "TotalBytesSent", json_object_new_int(BytesSent));
		json_object_object_add(*tr98Jobj, "TotalBytesReceived", json_object_new_int(BytesReceived));
		json_object_object_add(*tr98Jobj, "TotalPacketsSent", json_object_new_int(PacketsSent));
		json_object_object_add(*tr98Jobj, "TotalPacketsReceived", json_object_new_int(PacketsReceived));

	}
#endif // CONFIG_ZCFG_BE_MODULE_USB
	else {
		zcfgLog(ZCFG_LOG_ERR, "Unknown Object %s\n", tr181Obj);
		return ZCFG_INTERNAL_ERROR;
	}

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_ZCFG_BE_MODULE_DSL
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig
 *
 *   Related object in TR181:
 *   Device.DSL.Line.i
 *   Device.DSL.Line.i.Stats
 *   Device.DSL.Channel.i
 */
zcfgRet_t WANDslIfaceCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char dslChannelObj[32] = {0};
	char dslLineObj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char paramName[64] = {0};
	char *ptr = NULL;
	uint32_t  dslChannelOid = 0, dslLineOid = 0;
	objIndex_t dslChannelIid, dslLineIid;
	struct json_object *dslChannel = NULL, *dslLine = NULL, *dslLineSt = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, dslChannelObj)) == ZCFG_SUCCESS) {
		if(strstr(dslChannelObj, "DSL") == NULL) {
			zcfgLog(ZCFG_LOG_WARNING, "%s : Not a DSL Interface\n", __FUNCTION__);
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
			/*Not a DSL interface, all parameters set to 0 or empty*/
			*tr98Jobj = json_object_new_object();
			paramList = tr98Obj[handler].parameter;
			while(paramList->name != NULL) {
				if(!strstr(paramList->name, "DataPath") && !strstr(paramList->name, "InterleaveDepth")){
					zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
				}
				paramList++;
			}
			return ZCFG_SUCCESS;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(dslChannelIid);
		sprintf(tr181Obj, "Device.%s", dslChannelObj);
		dslChannelOid = zcfgFeObjNameToObjId(tr181Obj, &dslChannelIid);
		
		if((ret = feObjJsonGet(dslChannelOid, &dslChannelIid, &dslChannel, updateFlag)) != ZCFG_SUCCESS)
			return ret;

		ZOS_STRCPY_M(dslLineObj, json_object_get_string(json_object_object_get(dslChannel, "LowerLayers")));

		sprintf(tr181Obj, "Device.%s", dslLineObj);
#ifdef ZYXEL_SUPPORT_TR098_BRCM_BONDING_LINE
		/*
			WANDevice.12 --> DSL.Line.2 (ADSL,VDSL) Bonding line
			WANDevice.13 --> DSL.Line.2 (ADSL,VDSL) Bonding line
		*/
		if(isBondingWanProcess){
			sprintf(tr181Obj, "Device.%s", "DSL.Line.2");
		}
		printf("%s(%d):  Device:%s  isBondingWanProcess:%d \n",__FUNCTION__,__LINE__,tr181Obj,isBondingWanProcess);
#endif // ZYXEL_SUPPORT_TR098_BRCM_BONDING_LINE
		
		IID_INIT(dslLineIid);
		dslLineOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);

		if((ret = feObjJsonGet(dslLineOid, &dslLineIid, &dslLine, updateFlag)) != ZCFG_SUCCESS){
			json_object_put(dslChannel);
			return ret;
		}

		if((ret = feObjJsonGet(RDM_OID_DSL_CHANNEL_ST, &dslLineIid, &dslLineSt, updateFlag)) != ZCFG_SUCCESS){
			json_object_put(dslChannel);
			json_object_put(dslLine);
			return ret;
		}

		/*fill up tr98 WANDSLInterfaceConfig object*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
			if(!strcmp(paramList->name, "DataPath")){
				//struct json_object *ModulationTypeValue = NULL;
				//ModulationTypeValue = json_object_object_get(dslLine, "X_ZYXEL_ModulationType");
				//if((ModulationTypeValue != NULL) && !strncmp(json_object_get_string(ModulationTypeValue),"ADSL_G.dmt",10)) //G.992.1
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Interleaved"));
				//else
					//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("None"));
				 paramList++;
				 continue;
			}
			if(!strcmp(paramList->name, "InterleaveDepth")){
				 json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
				 paramList++;
				 continue;
			}
#endif
			if(!strcmp(paramList->name, "Status"))
				paramValue = json_object_object_get(dslLine, "LinkStatus");
			else
				paramValue = json_object_object_get(dslLine, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			paramValue = json_object_object_get(dslChannel, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			paramValue = json_object_object_get(dslLineSt, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
			if(strstr(paramList->name, "ATU") != NULL) {
				ZOS_STRCPY_M(paramName, paramList->name);
				paramName[0] = 'X';
				paramValue = json_object_object_get(dslLine, paramName);
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;	
				}
			}
			else if(!strcmp(paramList->name, "ModulationType")) {
				paramValue = json_object_object_get(dslLine, "X_ZYXEL_ModulationType");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "LinkEncapsulationRequested")) {
				if((paramValue = json_object_object_get(dslChannel, "LinkEncapsulationUsed"))) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "UpstreamMaxRate")) {
				if((paramValue = json_object_object_get(dslLine, "UpstreamMaxBitRate"))) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "DownstreamMaxRate")) {
				if((paramValue = json_object_object_get(dslLine, "DownstreamMaxBitRate"))) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			
			/*Not defined in tr181, give it a default value*/

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;
		}

		json_object_put(dslChannel);
		json_object_put(dslLine);
		json_object_put(dslLineSt);
	}
	else {
		return ret;	
	}

	return ZCFG_SUCCESS;
}
/*   
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig
 *
 *   Related object in TR181:
 *   Device.DSL.Channel.i
 *   Device.DSL.Line.i
 */
zcfgRet_t WANDslIfaceCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char dslChannelObj[32] = {0};
	char dslLineObj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	tr98Parameter_t *paramList = NULL;
	objIndex_t objIid, dslLineIid;
	uint32_t dslLineOid = 0;
	struct json_object *dslChannel = NULL, *dslLine = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, dslChannelObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(strstr(dslChannelObj, "DSL") == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Not a DSL Interface\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	const char *linkEncapUsed = json_object_get_string(json_object_object_get(tr98Jobj, "LinkEncapsulationUsed"));
	const char *linkEncapReq = json_object_get_string(json_object_object_get(tr98Jobj, "LinkEncapsulationRequested"));
	if(linkEncapUsed && linkEncapReq && strcmp(linkEncapReq, linkEncapUsed))
		return ZCFG_INVALID_PARAM_VALUE;

	IID_INIT(objIid);
	sscanf(dslChannelObj, "DSL.Channel.%hhu", &objIid.idx[0]);
	objIid.level = 1;

	if((ret = zcfgFeObjJsonGet(RDM_OID_DSL_CHANNEL, &objIid, &dslChannel)) == ZCFG_SUCCESS) {
		ZOS_STRCPY_M(dslLineObj, json_object_get_string(json_object_object_get(dslChannel, "LowerLayers")));
		if(multiJobj){
			tmpObj = dslChannel;
			dslChannel = NULL;
			dslChannel = zcfgFeJsonMultiObjAppend(RDM_OID_DSL_CHANNEL, &objIid, multiJobj, tmpObj);
		}
		json_object_object_add(dslChannel, "Enable", JSON_OBJ_COPY(json_object_object_get(tr98Jobj, "Enable")));

		if(multiJobj){
			json_object_put(tmpObj);
		}
		else {
			if((ret = zcfgFeObjJsonSet(RDM_OID_DSL_CHANNEL, &objIid, dslChannel, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set DSL.Channel Fail\n", __FUNCTION__);
				json_object_put(dslChannel);
				return ret;
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set DSL.Channel Success\n", __FUNCTION__);
			}
			json_object_put(dslChannel);
		}

	}

	sprintf(tr181Obj, "Device.%s", dslLineObj);
	IID_INIT(dslLineIid);
	dslLineOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);

	if((ret =ret = zcfgFeObjJsonGet(dslLineOid, &dslLineIid, &dslLine)) == ZCFG_SUCCESS){
		if(multiJobj){
			tmpObj = dslLine;
			dslLine = NULL;
			dslLine = zcfgFeJsonMultiObjAppend(dslLineOid, &dslLineIid, multiJobj, tmpObj);
		}

		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			/*Write new parameter value from tr98 object to tr181 objects*/
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(dslLine, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(dslLine, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#if TR98_MAPPING_DEBUG
			printf("Can't find parameter %s in TR181\n", paramList->name);
#endif
			paramList++;
		}

		if(multiJobj){
			json_object_put(tmpObj);
		}
		else {
			if((ret = zcfgFeObjJsonSet(dslLineOid, &dslLineIid, dslLine, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set DSL.Line Fail\n", __FUNCTION__);
				json_object_put(dslLine);
				return ret;
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set DSL.Line Success\n", __FUNCTION__);
			}
			json_object_put(dslLine);
		}
	}

	return ZCFG_SUCCESS;
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.TestParams 
 *
 *   Related object in TR181:
 *   Device.DSL.Line.i.TestParams
 */
zcfgRet_t WANDslIfTestParamObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr181ObjName[32] = {0};
	tr98Parameter_t *paramList = NULL;
	objIndex_t dslChIid, dslLineIid;
	rdm_DslChannel_t *dslChObj = NULL;
	struct json_object *dslTestParamJobj = NULL;
	struct json_object *paramValue = NULL;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	/*find tr181 mapping object*/
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLInterfaceConfig.TestParams");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, tr181ObjName)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}
		
	if(strstr(tr181ObjName, "DSL") == NULL) {
		zcfgLog(ZCFG_LOG_WARNING, "%s : Not a DSL Interface\n", __FUNCTION__);
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
		/*Not a DSL interface, all parameters set to 0 or empty*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			paramList++;
		}
		return ZCFG_SUCCESS;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(dslChIid);
	sscanf(tr181ObjName, "DSL.Channel.%hhu", &dslChIid.idx[0]);
	dslChIid.level = 1;

	if((ret = feObjStructGet(RDM_OID_DSL_CHANNEL, &dslChIid, (void **)&dslChObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(dslLineIid);
	sscanf(dslChObj->LowerLayers, "DSL.Line.%hhu", &dslLineIid.idx[0]);
	dslLineIid.level = 1;

	zcfgFeObjStructFree(dslChObj);

	if((ret = feObjJsonGet(RDM_OID_DSL_LINE_TEST_PARAMS, &dslLineIid, &dslTestParamJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(dslTestParamJobj, paramList->name);

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;	
	}

	json_object_put(dslTestParamJobj);

	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.Total

    Related object in TR181:
    Device.DSL.Line.i.Stats.Total.
*/
extern zcfgRet_t WANDslIfaceCfgTtlObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181PathName[MAX_TR181_PATHNAME] = {0};
	char tr181Obj[128] = {0};
	tr98Parameter_t *paramList = NULL;
	objIndex_t dslLineIid;
	uint32_t  dslLineOid = 0;
	struct json_object *dslStasTotal = NULL;
	struct json_object *paramValue = NULL;
	char vendorParamName[32] = {0};
	struct json_object *vendorParamValue = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181PathName)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s : Get TR181 mapping name fail.\n", __FUNCTION__);
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
		/*Not a DSL interface, all parameters set to 0 or empty*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			paramList++;
		}
		return ZCFG_SUCCESS;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
		return ret;
	}

	IID_INIT(dslLineIid);
	sprintf(tr181Obj, "Device.%s", tr181PathName);
	dslLineOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);
	if((ret = feObjJsonGet(dslLineOid, &dslLineIid, &dslStasTotal, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get dslStasTotal obj fail.\n", __FUNCTION__);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		sprintf(vendorParamName, "X_ZYXEL_%s", paramList->name);
		paramValue = json_object_object_get(dslStasTotal, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if( (vendorParamValue = json_object_object_get(dslStasTotal, vendorParamName)) != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(vendorParamValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}

	json_object_put(dslStasTotal);	
	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.Showtime

    Related object in TR181:
    Device.DSL.Line.i.Stats.Total.
*/
extern zcfgRet_t WANDslIfaceCfgStObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181PathName[MAX_TR181_PATHNAME] = {0};
	char tr181Obj[128] = {0};
	tr98Parameter_t *paramList = NULL;
	objIndex_t dslLineIid;
	uint32_t  dslLineOid = 0;
	struct json_object *dslStasShowTime = NULL;
	struct json_object *paramValue = NULL;
	char vendorParamName[32] = {0};
	struct json_object *vendorParamValue = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181PathName)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s : Get TR181 mapping name fail.\n", __FUNCTION__);
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
		/*Not a DSL interface, all parameters set to 0 or empty*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			paramList++;
		}
		return ZCFG_SUCCESS;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
		return ret;
	}

	IID_INIT(dslLineIid);
	sprintf(tr181Obj, "Device.%s", tr181PathName);
	dslLineOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);
	if((ret = feObjJsonGet(dslLineOid, &dslLineIid, &dslStasShowTime, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get dslStasShowTime obj fail.\n", __FUNCTION__);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		sprintf(vendorParamName, "X_ZYXEL_%s", paramList->name);
		paramValue = json_object_object_get(dslStasShowTime, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if( (vendorParamValue = json_object_object_get(dslStasShowTime, vendorParamName)) != NULL){
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(vendorParamValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}

	json_object_put(dslStasShowTime);	
	return ZCFG_SUCCESS;
}

#define MAX_32BITSIGNINT  2147483647
#define MAX_32BITUNSIGNINT 4294967295
#define DSLSTATS_PARAM_NOTSUPPORT MAX_32BITUNSIGNINT
/* It was #define DSLSTATS_PARAM_NOTSUPPORT (MAX_32BITUNSIGNINT-1).
    Follow the Spec of TR-098 and TR-181:
    "If the parameter is implemented but no value is available, its value MUST be 4294967295 (the maximum for its data type)"
*/
//#define TR98_DSLINTFCFG_STATS_REQ_SUBOBJPARAM_NOTIFY

#ifdef TR98_DSLINTFCFG_STATS_REQ_SUBOBJPARAM_NOTIFY
// require 'WANDSLInterfaceConfig.Stats' sub object param notify
zcfgRet_t WANDslIfaceCfgStatsSubObj_RetrieveTr181DslLineStatsSubObj(char *tr98FullPathName, struct json_object **tr98Jobj, struct json_object **dslStatsObj, bool updateFlag, int handler)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	tr98Parameter_t *paramList = NULL;
	char tr181PathName[MAX_TR181_PATHNAME] = {0};
	char tr181Obj[128] = {0};
	objIndex_t dslLineIid;
	uint32_t dslLineStatsOid = 0;
	int n = 0;
	bool subPathNameMatch = false;
	const char *subPathNames[] = { "Stats.Total", "Stats.Showtime", "Stats.CurrentDay", "Stats.QuarterHour", "Stats.LastShowtime", 0 };

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181PathName)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s : Get TR181 mapping name fail.\n", __FUNCTION__);
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
		/*Not a DSL interface, all parameters set to 0 or empty*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			if(!strstr(paramList->name, "ATUCFECErrors") && !strstr(paramList->name, "ATUCCRCErrors")){
				zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			}
			paramList++;
		}
		return ZCFG_OBJECT_UPDATED;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
		return ret;
	}
	printf("%s: Tr181 mapping name: %s\n", __FUNCTION__, tr181PathName);

	while(subPathNames[n] && !subPathNameMatch) {
		char subPathName[30] = {0};
		ZOS_STRCPY_M(subPathName, subPathNames[n]);
		if(strstr(tr98FullPathName, subPathName)) {
			subPathNameMatch = true;
			if(!strstr(tr181PathName, subPathName)) {
				zcfgLog(ZCFG_LOG_INFO, "%s : Tr98/Tr181 mapping not match\n", __FUNCTION__);
				return ZCFG_INTERNAL_ERROR;
			}
		}
		n++;
	}

	if(!subPathNameMatch) {
		zcfgLog(ZCFG_LOG_INFO, "%s : Tr98/Tr181 mapping not match\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(dslLineIid);
	sprintf(tr181Obj, "Device.%s", tr181PathName);
	dslLineStatsOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);
	if((ret = feObjJsonGet(dslLineStatsOid, &dslLineIid, dslStatsObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get dslStatsObj obj fail.\n", __FUNCTION__);
		return ret;
	}

	return ZCFG_SUCCESS;
}
#else
zcfgRet_t WANDslIfaceCfgStatsSubObj_RetrieveTr181DslLineStatsSubObj(char *tr98FullPathName, struct json_object **tr98Jobj, struct json_object **dslStatsObj, bool updateFlag, int handler)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	tr98Parameter_t *paramList = NULL;
	int n = 0, subPathNameMatch = 0;
	char tr98WanIfaceName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181DslPathName[MAX_TR181_PATHNAME] = {0};;
	const char *dslLineName = NULL;
	objIndex_t dslChannelIid, dslLineIid;
	struct json_object *dslChannelObj = NULL;
	const char *subPathNames[] = { "Stats.Total", "Stats.Showtime", "Stats.CurrentDay", "Stats.QuarterHour", "Stats.LastShowtime", 0 };
	int subObjOids[] = {RDM_OID_DSL_LINE_ST_TOTAL, RDM_OID_DSL_LINE_ST_SHOWTIME, RDM_OID_DSL_LINE_ST_LAST_CUR_DAY, RDM_OID_DSL_LINE_ST_LAST_QUAR_HR, RDM_OID_DSL_LINE_ST_LAST_SHOWTIME};
	uint32_t dslLineStatsOid = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98WanIfaceName, tr98FullPathName);
	char *p = strstr(tr98WanIfaceName, ".WANDSLInterfaceConfig");
	*p = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98WanIfaceName, tr181DslPathName)) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	while(subPathNames[n] && !subPathNameMatch) {
		char subPathName[30] = {0};
		ZOS_STRCPY_M(subPathName, subPathNames[n]);
		if(strstr(tr98FullPathName, subPathName)) {
			subPathNameMatch = n+1;
		}
		n++;
	}

	if(!subPathNameMatch) {
		zcfgLog(ZCFG_LOG_INFO, "%s : Tr98/Tr181 mapping not match\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	printf("%s: Tr181 mapping name: %s\n", __FUNCTION__, tr181DslPathName);

	IID_INIT(dslChannelIid);
	if(sscanf(tr181DslPathName, "DSL.Channel.%hhu", &dslChannelIid.idx[0]) != 1){
#ifdef EAAAA_TR69_WANDSL_CUSTOMIZATION
		/*Not a DSL interface, all parameters set to 0 or empty*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			if(!strstr(paramList->name, "ATUCFECErrors") && !strstr(paramList->name, "ATUCCRCErrors")){
				zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			}
			paramList++;
		}
		return ZCFG_SUCCESS;
#endif //EAAAA_TR69_WANDSL_CUSTOMIZATION
		return ZCFG_NO_SUCH_OBJECT;
	}

	dslChannelIid.level = 1;
	if((ret = feObjJsonGet(RDM_OID_DSL_CHANNEL, &dslChannelIid, &dslChannelObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get dslChannelObj obj fail.\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(dslLineIid);
	dslLineIid.level = 1;
	dslLineName = json_object_get_string(json_object_object_get(dslChannelObj, "LowerLayers"));
	if(sscanf(dslLineName, "DSL.Line.%hhu", &dslLineIid.idx[0]) != 1){
		json_object_put(dslChannelObj);
		return ZCFG_NO_SUCH_OBJECT;
	}

	dslLineStatsOid = subObjOids[subPathNameMatch-1];
	if((ret = feObjJsonGet(dslLineStatsOid, &dslLineIid, dslStatsObj, updateFlag)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_INFO, "%s : Get dslStatsObj obj fail.\n", __FUNCTION__);
		json_object_put(dslChannelObj);
		return ret;
	}

	if(strcmp(json_object_get_string(json_object_object_get(dslChannelObj, "Status")), "Up")){
		// The requested DSL intf isnt running, set all parameter values to empty or 0 and then return
		json_object_object_foreach(*dslStatsObj,stKey,stValue){
			if((strstr(stKey,"ATUCFECErrors") || strstr(stKey,"ATUCCRCErrors")) && (json_object_get_int(json_object_object_get(*dslStatsObj,stKey)) == DSLSTATS_PARAM_NOTSUPPORT)){
				continue;
			}
			json_type type = json_object_get_type(json_object_object_get(*dslStatsObj,stKey));
			zcfgFeTr98DefaultValueSet(*dslStatsObj, stKey, type);
		}
		json_object_put(dslChannelObj);
		return ZCFG_SUCCESS;
	}

	json_object_put(dslChannelObj);
	return ret;
}
#endif

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.SubPathNames

    Related object in TR181:
    Device.DSL.Line.i.Stats.SubPathNames.
*/
extern zcfgRet_t WANDslIfaceCfgStatsSubObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	struct json_object *dslStatsObj = NULL;
	struct json_object *paramValue = NULL;
	char vendorParamName[32] = {0};
	struct json_object *vendorParamValue = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	*tr98Jobj = NULL;
	ret = WANDslIfaceCfgStatsSubObj_RetrieveTr181DslLineStatsSubObj(tr98FullPathName, tr98Jobj, &dslStatsObj, updateFlag, handler);
	if(*tr98Jobj)
		return ret;
	else if(!dslStatsObj)
		return ret;

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				if(!strcmp(paramList->name,"LInit")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					paramList++;
					continue;
				}
		
				if(!strcmp(paramList->name,"ATUCSeverelyErroredSecs")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(MAX_32BITUNSIGNINT));
					paramList++;
					continue;
				}
		
				if(!strcmp(paramList->name,"ATUCErroredSecs")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(MAX_32BITUNSIGNINT));
					paramList++;
					continue;
				}
#endif

		sprintf(vendorParamName, "X_ZYXEL_%s", paramList->name);
		paramValue = json_object_object_get(dslStatsObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if( (vendorParamValue = json_object_object_get(dslStatsObj, vendorParamName)) != NULL){
			if(strstr(vendorParamName, "ATUCFECErrors") || strstr(vendorParamName, "ATUCCRCErrors")) {
				uint32_t paramInt = (uint32_t)json_object_get_int(vendorParamValue);
				printf("%s: %s, %u\n", __FUNCTION__, paramList->name, paramInt);
				printf("%s: %s, %d\n", __FUNCTION__, paramList->name, paramInt);
				if(paramInt == DSLSTATS_PARAM_NOTSUPPORT) {
					paramList++;
					continue;
				}
			}
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(vendorParamValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}

	json_object_put(dslStatsObj);	
	return ZCFG_SUCCESS;
}
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_DSL

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANEthernetInterfaceConfig
 *
 *  Related object in TR181:
 *  Device.Ethernet.Interface.i
 */
zcfgRet_t WANEthIfaceCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ethIfaceObj[32] = {0};
	uint32_t  ethIfaceOid = 0;
	objIndex_t ethIfaceIid;
	struct json_object *ethIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANEthernetInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethIfaceObj)) == ZCFG_SUCCESS) {
		if(strstr(ethIfaceObj, "Ethernet") == NULL) {
			printf("%s : Not an Ethernet Interface\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(ethIfaceIid);
		sprintf(tr181Obj, "Device.%s", ethIfaceObj);
		ethIfaceOid = zcfgFeObjNameToObjId(tr181Obj, &ethIfaceIid);

		if((ret = feObjJsonGet(ethIfaceOid, &ethIfaceIid, &ethIfaceJobj, updateFlag)) != ZCFG_SUCCESS)
			return ret;
		
		//printf("Ethernet Interface %s\n", json_object_to_json_string(ethIfaceJobj));

		/*fill up tr98 WANEthernetInterfaceConfig object*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			paramValue = json_object_object_get(ethIfaceJobj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			
			/*Not defined in tr181, give it a default value*/

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);

			paramList++;
		}

		json_object_put(ethIfaceJobj);
	}

	return ZCFG_SUCCESS;	
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANEthernetInterfaceConfig

    Related object in TR181:
    Device.Ethernet.Interface.i
 */
zcfgRet_t WANEthIfaceCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ethIfaceName[32] = {0};
	char *ptr = NULL;
	objIndex_t ethIfaceIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *ethIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*find tr181 mapping object*/
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANEthernetInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethIfaceName)) == ZCFG_SUCCESS) {
		if(strstr(ethIfaceName, "Ethernet") == NULL) {
			printf("%s : Not an Ethernet Interface\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(ethIfaceIid);
		sscanf(ethIfaceName, "Ethernet.Interface.%hhu", &ethIfaceIid.idx[0]);
		ethIfaceIid.level = 1;

		if((ret = zcfgFeObjJsonGet(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceJobj)) != ZCFG_SUCCESS)
			return ret;		

		if(multiJobj){
			tmpObj = ethIfaceJobj;
			ethIfaceJobj = NULL;
			ethIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_IFACE, &ethIfaceIid, multiJobj, tmpObj);
		}

		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			/*Write new parameter value from tr98 object to tr181 objects*/	
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(ethIfaceJobj, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(ethIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;	
		}

		if(multiJobj){
			json_object_put(tmpObj);
		}
		else{
			if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_IFACE, &ethIfaceIid, ethIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
				json_object_put(ethIfaceJobj);
				return ret;
			}
			json_object_put(ethIfaceJobj);
		}
	}

	return ZCFG_SUCCESS;
}

/*  InternetGatewayDevice.WANDevice.i.WANEthernetInterfaceConfig.Stats

    Related object in TR181:
    Device.Ethernet.Interface.i.Stats.
 */
zcfgRet_t WANEthIfaceCfgStObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ethIfaceName[32] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	rdm_EthIntfSt_t *ethIfaceSt = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANEthernetInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethIfaceName)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}

	if(strstr(ethIfaceName, "Ethernet") == NULL) {
		printf("%s : Not an Ethernet Interface\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	*tr98Jobj = json_object_new_object();

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(ethIfaceName, "Ethernet.Interface.%hhu", &objIid.idx[0]);

	if(feObjStructGet(RDM_OID_ETH_INTF_ST, &objIid, (void **)&ethIfaceSt, updateFlag) == ZCFG_SUCCESS) {
		json_object_object_add(*tr98Jobj, "BytesSent", json_object_new_int(ethIfaceSt->BytesSent));
		json_object_object_add(*tr98Jobj, "BytesReceived", json_object_new_int(ethIfaceSt->BytesReceived));
		json_object_object_add(*tr98Jobj, "PacketsSent", json_object_new_int(ethIfaceSt->PacketsSent));
		json_object_object_add(*tr98Jobj, "PacketsReceived", json_object_new_int(ethIfaceSt->PacketsReceived));
		zcfgFeObjStructFree(ethIfaceSt);
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
zcfgRet_t WANConnDevObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{	
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char wanIpConnName[128] = {0};
	char wanPppConnName[128] = {0};
	int count = 0;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*Check if WANConnectionDevice.i exists or not*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, tr181Obj)) != ZCFG_SUCCESS) {
		/*  The instance number of WANConnectionDevice.i will be continuous because of the tr98 to tr181 mapping table. 
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANConnectionDevice object.
		 */	
		return ret;//ZCFG_NO_MORE_INSTANCE;
	}

#if 0
	sprintf(wanIpConnName, "%s.WANIPConnection.%d", tr98FullPathName, (count+1));
	while((ret = zcfgFe98To181MappingNameGet(wanIpConnName, tr181Obj)) == ZCFG_SUCCESS) {
		count++;
		sprintf(wanIpConnName, "%s.WANIPConnection.%d", tr98FullPathName, (count+1));
	}

	*tr98Jobj = json_object_new_object();

	json_object_object_add(*tr98Jobj, "WANIPConnectionNumberOfEntries", json_object_new_int(count));


	count = 0;
	sprintf(wanPppConnName, "%s.WANPPPConnection.%d", tr98FullPathName, (count+1));
	while((ret = zcfgFe98To181MappingNameGet(wanPppConnName, tr181Obj)) == ZCFG_SUCCESS) {
		count++;
		sprintf(wanPppConnName, "%s.WANPPPConnection.%d", tr98FullPathName, (count+1));
	}

	json_object_object_add(*tr98Jobj, "WANPPPConnectionNumberOfEntries", json_object_new_int(count));
#endif

	zcfg_name_t seqnum;
	objIndex_t tr98ObjIid;
	*tr98Jobj = json_object_new_object();

	/*add tr181 Device.PPP.SupportedNCPs*/
	json_object_object_add(*tr98Jobj, "X_ZYXEL_WANPPPConnectionSupportedNCPs", json_object_new_string("IPCP,IPv6CP"));
	json_object_object_add(*tr98Jobj, "WANPPPConnectionNumberOfEntries", json_object_new_int(0));
	json_object_object_add(*tr98Jobj, "WANIPConnectionNumberOfEntries", json_object_new_int(0));

	if((ret = zcfgFe98NameToSeqnum((const char *)tr98FullPathName, &seqnum, &tr98ObjIid)) == ZCFG_SUCCESS){
		struct json_object *valueObj = NULL;
		const char wanPppNumStr[] = "WANPPPConnectionNumberOfEntries";

		if(zcfgFeTr98ObjValueQry(seqnum, &tr98ObjIid, wanPppNumStr, &valueObj, PARAMETER_ATTR_TR98) == ZCFG_SUCCESS){
			if(valueObj != NULL){
				json_object_object_add(*tr98Jobj, wanPppNumStr, valueObj);
			}else{
				json_object_object_add(*tr98Jobj, wanPppNumStr, json_object_new_int(0));
			}
		}else{
			json_object_object_add(*tr98Jobj, wanPppNumStr, json_object_new_int(0));
		}

		valueObj = NULL;

		const char wanIpNumStr[] = "WANIPConnectionNumberOfEntries";
		if(zcfgFeTr98ObjValueQry(seqnum, &tr98ObjIid, wanIpNumStr, &valueObj, PARAMETER_ATTR_TR98) == ZCFG_SUCCESS){
			if(valueObj != NULL){
				json_object_object_add(*tr98Jobj, wanIpNumStr, valueObj);
			}
			else
			{
				json_object_object_add(*tr98Jobj, wanIpNumStr, json_object_new_int(0));
			}
		}else{
			json_object_object_add(*tr98Jobj, wanIpNumStr, json_object_new_int(0));
		}

	}else{
		return ret;
	}

	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
zcfgRet_t WANConnDevObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr181Obj[48] = {0};
	char wanIntfObj[48] = {0};
	char wanObjName[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	uint32_t  wanIntfOid = 0, tr181Objnum = 0;
	objIndex_t wanIntfIid;
	objIndex_t objIid;
	char *ptr = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	rdm_DslChannel_t *dslChannelObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	rdm_AtmLink_t *atmLinkObj = NULL;
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	rdm_PtmLink_t *ptmLinkObj = NULL;
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION	
	rdm_EthLink_t *ethLinkObj = NULL;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANConnectionDevice");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanIntfObj)) != ZCFG_SUCCESS) {
		return ret;
	}
        	
	/* wanIntfObj will be DSL.Channel.i or Ethernet.Interface.i */
	IID_INIT(wanIntfIid);
	IID_INIT(objIid);
	sprintf(tr181Obj, "Device.%s", wanIntfObj);
	wanIntfOid = zcfgFeObjNameToObjId(tr181Obj, &wanIntfIid);
		
	if(strstr(wanIntfObj, "DSL") != NULL) { // DSL WAN
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
		if((ret = zcfgFeObjStructGet(wanIntfOid, &wanIntfIid, (void **)&dslChannelObj)) != ZCFG_SUCCESS)
			return ret;

		if(strstr(dslChannelObj->LinkEncapsulationUsed, "ATM") != NULL) {
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
			printf("%s : Add ATM Link\n", __FUNCTION__);
			if((ret = zcfgFeObjStructAdd(RDM_OID_ATM_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
				printf("Add Instance Fail!!\n");
				return ret;
			}
			tr181Objnum = RDM_OID_ATM_LINK;
			/*Set LowerLayer for ATM.Link*/
			if((ret = zcfgFeObjStructGet(RDM_OID_ATM_LINK, &objIid, (void **)&atmLinkObj)) == ZCFG_SUCCESS) {
				//atmLinkObj->Enable = true;
				ZOS_STRCPY_M(atmLinkObj->LowerLayers, wanIntfObj);
#ifdef CONFIG_TAAAB_CUSTOMIZATION
				ZOS_STRCPY_M(atmLinkObj->LinkType, "EoA");
				ZOS_STRCPY_M(atmLinkObj->Encapsulation, "LLC");
#endif
				if((ret = zcfgFeObjStructSet(RDM_OID_ATM_LINK, &objIid, (void *)atmLinkObj, NULL)) != ZCFG_SUCCESS) {
					printf("Set ATM.Link LowerLayers Fail\n");
				}

				zcfgFeObjStructFree(atmLinkObj);
			}
			else {
				return ret;
			}

			/*We consider only one ATM phy interface*/
			*idx = objIid.idx[0];
#else
			printf("%s : ATM Link not support\n", __FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
#endif
		}
		else if(strstr(dslChannelObj->LinkEncapsulationUsed, "PTM") != NULL) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION	
			printf("%s : Add ETH Link\n", __FUNCTION__);
			if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
				printf("Add Instance Fail!!\n");
				return ret;
			}
			tr181Objnum = RDM_OID_ETH_LINK;
			/*Set LowerLayer for Ethernet.Link*/
			if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS) {
				ethLinkObj->Enable = true;
				ZOS_STRCPY_M(ethLinkObj->LowerLayers, "PTM.Link.1");
				if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS) {
					printf("Set ATM.Link LowerLayers Fail\n");
				}

				zcfgFeObjStructFree(ethLinkObj);
			}
			else {
				return ret;
			}

			/*We consider only one ATM phy interface*/
			*idx = objIid.idx[0];
#else
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
			/*Check if PTM WAN Connection Device exists or not*/
			strcat(tr98TmpName, ".WANConnectionDevice.1");
			if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanObjName)) == ZCFG_SUCCESS) {
				printf("%s : Can't add more than one WANConnectionDevice under WANDevice with PTM type\n", __FUNCTION__);
				return ZCFG_INTERNAL_ERROR;
			}

			printf("%s : Add PTM Link\n", __FUNCTION__);
			if((ret = zcfgFeObjStructAdd(RDM_OID_PTM_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
				printf("Add Instance Fail!!\n");
				return ret;
			}
			tr181Objnum = RDM_OID_PTM_LINK;
			/*Set LowerLayer for PTM.Link*/
			if((ret = zcfgFeObjStructGet(RDM_OID_PTM_LINK, &objIid, (void **)&ptmLinkObj)) == ZCFG_SUCCESS) {
				//ptmLinkObj->Enable = true;
				ZOS_STRCPY_M(ptmLinkObj->LowerLayers, wanIntfObj);
				if((ret = zcfgFeObjStructSet(RDM_OID_PTM_LINK, &objIid, (void *)ptmLinkObj, NULL)) != ZCFG_SUCCESS) {
					printf("%s : Set PTM.Link LowerLayers Fail\n", __FUNCTION__);
				}

				zcfgFeObjStructFree(ptmLinkObj);
			}
			else {
				return ret;
			}

			/*Hard coding here, only one WANConnectionDevice.i will exist under InternetGatewayDevice.WANDevice.i if type is PTM*/
			*idx = 1;
#else
			printf("%s : PTM Link not support\n", __FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
#endif // CONFIG_ZCFG_BE_MODULE_PTM_LINK
#endif
		}
		else {
			printf("%s : Unknown Link Encapsulation Type\n", __FUNCTION__);
		}
	
		zcfgFeObjStructFree(dslChannelObj);
#else
		printf("%s : DSL not support\n", __FUNCTION__);
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_DSL
	}
	else if(strstr(wanIntfObj, "Ethernet") != NULL) { // Ethernet WAN
#ifdef CONFIG_TAAAB_CUSTOMIZATION
		printf("%s : Add ETH Link\n", __FUNCTION__);
		if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
			printf("Add Instance Fail!!\n");
			return ret;
		}
		tr181Objnum = RDM_OID_ETH_LINK;
		/*Set LowerLayer for Ethernet.Link*/
		if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS) {
			ethLinkObj->Enable = true;
			ZOS_STRCPY_M(ethLinkObj->LowerLayers, wanIntfObj);
			if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS) {
				printf("Set ATM.Link LowerLayers Fail\n");
			}

			zcfgFeObjStructFree(ethLinkObj);
		}
		else {
			return ret;
		}

		/*We consider only one ATM phy interface*/
		*idx = objIid.idx[0];
#else
#if 1 //WANDevice.i and WANDevice.i.WANConnectionDevice.i are both mapping to Ethernet.Interface
		return TR069_RESOURCES_EXCEEDED;
#else
		/*Check if Ethernet WAN Connection Device exists or not*/
		strcat(tr98TmpName, ".WANConnectionDevice.1");
		if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanObjName)) == ZCFG_SUCCESS) {
			printf("%s : Can't add more than one WANConnectionDevice under WANDevice with Ethernet type\n", __FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}

		printf("%s : Add Ethernet Link\n", __FUNCTION__);
		if((ret = zcfgFeObjStructAdd(RDM_OID_ETH_LINK, &objIid, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Add Ethernet Link Fail!!\n", __FUNCTION__);
			return ret;
		}
		tr181Objnum = RDM_OID_ETH_LINK;
		/*Set LowerLayer for Ethernet.Link*/
		if((ret = zcfgFeObjStructGet(RDM_OID_ETH_LINK, &objIid, (void **)&ethLinkObj)) == ZCFG_SUCCESS) {
			ethLinkObj->Enable = true;
			ZOS_STRCPY_M(ethLinkObj->LowerLayers, wanIntfObj);
			if((ret = zcfgFeObjStructSet(RDM_OID_ETH_LINK, &objIid, (void *)ethLinkObj, NULL)) != ZCFG_SUCCESS) {
				printf("%s : Set Ethernet.Link LowerLayers Fail\n", __FUNCTION__);
			}

			zcfgFeObjStructFree(ethLinkObj);
		}

		/*Hard coding here, only one WANConnectionDevice.i will exist under InternetGatewayDevice.WANDevice.i if type is Ethernet*/
		*idx = 1;
#endif
#endif
	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	*idx = objIid.idx[0];
#else
	printf("%s: return %d\n", __FUNCTION__, ret);

	if((ret == ZCFG_SUCCESS) || (ret == ZCFG_DELAY_APPLY)) {
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		sscanf(tr98FullPathName, "InternetGatewayDevice.WANDevice.%hhu", &tr98ObjIid.idx[0]);
		tr98ObjIid.level = 1;
		if(zcfg98To181ObjMappingAdd(e_TR98_WAN_CONN_DEV, &tr98ObjIid, tr181Objnum, &objIid) == ZCFG_SUCCESS) {
			printf("%s: Added WANConnectionDevice %hhu\n", __FUNCTION__, tr98ObjIid.idx[1]);
			*idx = tr98ObjIid.idx[1];
		}
	}
#endif

	return ret;
}
/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
zcfgRet_t WANConnDevObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[48] = {0};
	char wanLinkPathName[32] = {0};
	char wanIpConnName[128] = {0};
	char wanPppConnName[128] = {0};
	uint32_t  wanLinkOid = 0, ipcount = 0, pppcount = 0, c = 0;
	objIndex_t wanLinkIid;
	struct json_object *tr98Jobj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*Check if WANConnectionDevice.i exists or not*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, wanLinkPathName)) != ZCFG_SUCCESS) {
		return ret;
	}

	if((ret = WANConnDevObjGet(tr98FullPathName, 0, &tr98Jobj, TR98_GET_UPDATE)) != ZCFG_SUCCESS) {
		return ret;
	}

	ipcount = json_object_get_int(json_object_object_get(tr98Jobj, "WANIPConnectionNumberOfEntries"));
	pppcount = json_object_get_int(json_object_object_get(tr98Jobj, "WANPPPConnectionNumberOfEntries"));

	json_object_put(tr98Jobj);

	/*Delete WANIPConnection*/
	for(c = 1; c <= ipcount; c++) {
		sprintf(wanIpConnName, "%s.WANIPConnection.1", tr98FullPathName);
		ret = WANIpConnObjDel(wanIpConnName);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			printf("%s : Delete %s fail\n", __FUNCTION__, wanIpConnName);
			return ret;
		}
	}

	/*Delete WANPPPConnection*/
	for(c = 1; c <= pppcount; c++) {
		sprintf(wanPppConnName, "%s.WANPPPConnection.1", tr98FullPathName);
		ret = WANPppConnObjDel(wanPppConnName);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			printf("%s : Delete %s fail\n", __FUNCTION__, wanPppConnName);
			return ret;
		}
		else {
			printf("%s : Delete %s Success\n", __FUNCTION__, wanPppConnName);
		}
	}

	printf("%s : WAN Link %s\n", __FUNCTION__, wanLinkPathName);
	sprintf(tr181Obj, "Device.%s", wanLinkPathName);
	wanLinkOid = zcfgFeObjNameToObjId(tr181Obj, &wanLinkIid);

	ret = zcfgFeObjJsonDel(wanLinkOid, &wanLinkIid, NULL);
	if(ret != ZCFG_SUCCESS) {
		printf("%s : Delete Object Fail\n", __FUNCTION__);	
	}

	return ret;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069	
void  getDSLLineModulationType(char *tr98FullPathName,bool updateFlag, char *ModulationType){
		char tr98TmpLineName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		char *ptr = NULL;
		char tr181Obj[32] = {0};
		char dslChannelObj[32] = {0};
		char dslLineObj[32] = {0};
		uint32_t  dslChannelOid = 0, dslLineOid = 0;
		objIndex_t dslChannelIid, dslLineIid;
		struct json_object *dslChannel = NULL, *dslLine = NULL;
		zcfgRet_t ret;

		ZOS_STRCPY_M(tr98TmpLineName, tr98FullPathName);
		ptr = strstr(tr98TmpLineName, ".WANConnectionDevice");
		*ptr = '\0';
		//printf("WANDslLinkConfObjGet11	--%s--\r\n",tr98TmpLineName);
		/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
		if((ret = zcfgFe98To181MappingNameGet(tr98TmpLineName, dslChannelObj)) == ZCFG_SUCCESS) {
			if(strstr(dslChannelObj, "DSL") == NULL) {
				zcfgLog(ZCFG_LOG_WARNING, "%s : Not a DSL Interface\n", __FUNCTION__);
				return;
			}
	
			IID_INIT(dslChannelIid);
			sprintf(tr181Obj, "Device.%s", dslChannelObj);
			//printf("WANDslLinkConfObjGet22	--%s--\r\n",tr181Obj);
			dslChannelOid = zcfgFeObjNameToObjId(tr181Obj, &dslChannelIid);
			
			if((ret = feObjJsonGet(dslChannelOid, &dslChannelIid, &dslChannel, updateFlag)) != ZCFG_SUCCESS)
				return;
	
			ZOS_STRCPY_M(dslLineObj, json_object_get_string(json_object_object_get(dslChannel, "LowerLayers")));
	
			sprintf(tr181Obj, "Device.%s", dslLineObj);
			//printf("WANDslLinkConfObjGet33	--%s--\r\n",tr181Obj);

			IID_INIT(dslLineIid);
			dslLineOid = zcfgFeObjNameToObjId(tr181Obj, &dslLineIid);

			if((ret = feObjJsonGet(dslLineOid, &dslLineIid, &dslLine, updateFlag)) == ZCFG_SUCCESS){
				//printf("WANDslLinkConfObjGet333	--%s--\r\n",json_object_get_string(json_object_object_get(dslLine, "X_ZYXEL_ModulationType")));
				ZOS_STRCPY_M(ModulationType,json_object_get_string(json_object_object_get(dslLine, "X_ZYXEL_ModulationType")));
				json_object_put(dslLine);
			}
			json_object_put(dslChannel);
			
		}
}

void  getPTMLinkStatus(char *tr98FullPathName,bool updateFlag, char *LinkStatus){
		char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		char *ptr = NULL;
		char tr181Obj[32] = {0};
		char ptmLink[32] = {0};
		char dslLineObj[32] = {0};
		uint32_t  dslChannelOid = 0, dslLineOid = 0;
		objIndex_t ptmLinkIid, dslLineIid;
		struct json_object *ptmLinkJobj = NULL, *dslLine = NULL;
		zcfgRet_t ret;

		ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
		ptr = strstr(tr98TmpName, ".WANDSLLinkConfig");
		*ptr = '\0';
		//printf("WANDslLinkConfObjGet11	--%s--\r\n",tr98TmpLineName);
		/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
		if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ptmLink)) == ZCFG_SUCCESS) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 //InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i mapping to eth link
				struct json_object *ethLinkJobj = NULL;
				objIndex_t ethLinkIid;
				IID_INIT(ethLinkIid);
				sscanf(ptmLink, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
				ethLinkIid.level = 1;
				if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, false)) != ZCFG_SUCCESS){
					return ;
				}
				
				ZOS_STRCPY_M(ptmLink, json_object_get_string(json_object_object_get(ethLinkJobj, "LowerLayers")));
				json_object_put(ethLinkJobj);
#endif
			if(strstr(ptmLink, "PTM") == NULL) {
				zcfgLog(ZCFG_LOG_WARNING, "%s : Not a PTM Interface\n", __FUNCTION__);
				return;
			}
		
			/*Get related tr181 objects*/
			IID_INIT(ptmLinkIid);
			sscanf(ptmLink, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
			ptmLinkIid.level = 1;
			
			/*Get Device.PTM.Link.i*/
			if((ret = feObjJsonGet(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkJobj, updateFlag)) != ZCFG_SUCCESS)
				return ;

			ZOS_STRCPY_M(LinkStatus, json_object_get_string(json_object_object_get(ptmLinkJobj, "Status")));
			
			json_object_put(ptmLinkJobj);
			
		}
}

#endif


#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANDSLLinkConfig

    Related object in TR181:
    Device.ATM.Link.i
    Device.ATM.Link.i.Stats.
    Device.ATM.Link.i.QoS
 */
zcfgRet_t WANDslLinkConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char test[16] = {0};
	char atmLink[24] = {0};
	objIndex_t atmLinkIid;
	struct json_object *atmLinkJobj = NULL, *atmLinkStJobj = NULL, *atmLinkQosJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;	
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	char ModulationType[32]={0};
	char LinkStatus[16]={0};
#endif
	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLLinkConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	getDSLLineModulationType(tr98FullPathName,updateFlag,ModulationType);
	getPTMLinkStatus(tr98FullPathName,updateFlag,LinkStatus);
#endif
	if(strstr(atmLink, "ATM") == NULL) {
		printf("%s : Not a ATM Link\n", __FUNCTION__);
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		//printf("%s : LinkStatus--%s--%s--\n", __FUNCTION__,LinkStatus,ModulationType);
		if(strlen(LinkStatus) && !strcmp(LinkStatus,"Up")){
			*tr98Jobj = json_object_new_object();
			if(strlen(ModulationType))
				json_object_object_add(*tr98Jobj,"ModulationType", json_object_new_string(ModulationType));
			json_object_object_add(*tr98Jobj,"LinkStatus", json_object_new_string(LinkStatus));
			json_object_object_add(*tr98Jobj,"LinkType", json_object_new_string("EoA"));
			return ZCFG_SUCCESS;
		}
#endif
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(atmLinkIid);
	sscanf(atmLink, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
	atmLinkIid.level = 1;

	if((ret = feObjJsonGet(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	if((ret = feObjJsonGet(RDM_OID_ATM_LINK_ST, &atmLinkIid, &atmLinkStJobj, updateFlag)) != ZCFG_SUCCESS) {
		json_object_put(atmLinkJobj);	
		return ret;
	}

	if((ret = feObjJsonGet(RDM_OID_ATM_LINK_QOS, &atmLinkIid, &atmLinkQosJobj, updateFlag)) != ZCFG_SUCCESS) {
		json_object_put(atmLinkStJobj);
		json_object_put(atmLinkJobj);
		return ret;
	}

	/*fill up tr98 WANDSLLinkConfig object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		if(strlen(ModulationType) && strstr(paramList->name, "ModulationType")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(ModulationType));
			paramList++;
			continue;
		}
#endif
		paramValue = json_object_object_get(atmLinkJobj, paramList->name);
		if(strstr(paramList->name, "DestinationAddress") != NULL){
			if(paramValue != NULL) {
				sprintf(test, "PVC:%s", (char *)json_object_get_string(paramValue));
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(test));
				paramList++;
				continue;
			}
		}
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
			
		paramValue = json_object_object_get(atmLinkQosJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(strstr(paramList->name, "ATM") != NULL) {
			ptr = paramList->name + 3;

			paramValue = json_object_object_get(atmLinkStJobj, ptr);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}				

			paramValue = json_object_object_get(atmLinkJobj, ptr);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			paramValue = json_object_object_get(atmLinkQosJobj, ptr);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		if(!strcmp(paramList->name, "AAL5CRCErrors")) {
			paramValue = json_object_object_get(atmLinkStJobj, "CRCErrors");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(strcmp(paramList->name, "ATMQoS") == 0) {
			paramValue = json_object_object_get(atmLinkQosJobj, "QoSClass");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		/*End of special case*/

		if(!strcmp(paramList->name, "LinkStatus")) {
			paramValue = json_object_object_get(atmLinkJobj, "Status");
			const char *tr181Status = json_object_get_string(paramValue);
			if(strstr(tr181Status, "Up") || strstr(tr181Status, "Down")) json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			else json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unavailable"));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}
		
	json_object_put(atmLinkJobj);
	json_object_put(atmLinkStJobj);
	json_object_put(atmLinkQosJobj);

	return ret;
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANDSLLinkConfig

    Related object in TR181:
    Device.ATM.Link.i
    Device.ATM.Link.i.QoS
 */
zcfgRet_t WANDslLinkConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char *test = NULL;
	char atmLink[32] = {0};
	objIndex_t atmLinkIid;
	struct json_object *atmLinkJobj = NULL, *atmLinkQosJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	int dstAdrModified = 0, atmStatus = 0;
	boolean atmSetDisable = false;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANDSLLinkConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(strstr(atmLink, "ATM") == NULL) {
		printf("%s : Not a ATM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(atmLinkIid);
	sscanf(atmLink, "ATM.Link.%hhu", &atmLinkIid.idx[0]);
	atmLinkIid.level = 1;

	/*Get Device.ATM.Link.i*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkJobj)) != ZCFG_SUCCESS)
		return ret;	

	/*Get Device.ATM.Link.i.QoS*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_ATM_LINK_QOS, &atmLinkIid, &atmLinkQosJobj)) != ZCFG_SUCCESS) {
		json_object_put(atmLinkJobj);
		return ret;
	}

	if(multiJobj){
		tmpObj = atmLinkQosJobj;
		atmLinkQosJobj = NULL;
		atmLinkQosJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ATM_LINK_QOS, &atmLinkIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
		
	/*write tr181 Device.ATM.Link.i and Device.ATM.Link.i.QoS object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANDSLLinkConfig to Device.ATM.Link.i*/
			tr181ParamValue = json_object_object_get(atmLinkJobj, paramList->name);
			if(strstr(paramList->name, "DestinationAddress") != NULL)
			{
				struct json_object *tr181ParamStatus = json_object_object_get(atmLinkJobj, "Status");
				if(tr181ParamValue != NULL) {
					const char *tr181adr = json_object_get_string(tr181ParamValue);
					const char *atmLinkStatus = json_object_get_string(tr181ParamStatus);
					test = (char *)json_object_get_string(paramValue);
					if(strstr(test, "PVC:") != NULL){
						test = test + 4;
						if(!strstr(tr181adr, test)) dstAdrModified = 1;
						json_object_object_add(atmLinkJobj, paramList->name, json_object_new_string(test));
					}else{
						if(!strstr(tr181adr, test)) dstAdrModified = 1;
						json_object_object_add(atmLinkJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					}
					if(strstr(atmLinkStatus, "Up")) atmStatus = 1;
					if((paramValue = json_object_object_get(tr98Jobj, "Enable")))
					{
						if(!json_object_get_boolean(paramValue)) atmSetDisable = true;
					}
					paramList++;
					continue;
				}
			}
			
			if(tr181ParamValue != NULL) {
				json_object_object_add(atmLinkJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*Write value from WANDSLLinkConfig to Device.ATM.Link.i.QoS*/
			tr181ParamValue = json_object_object_get(atmLinkQosJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(atmLinkQosJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
			if(strstr(paramList->name, "ATM") != NULL) {
				ptr = paramList->name + 3;
			
				tr181ParamValue = json_object_object_get(atmLinkJobj, ptr);
				if(tr181ParamValue != NULL) {
					json_object_object_add(atmLinkJobj, ptr, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				tr181ParamValue = json_object_object_get(atmLinkQosJobj, ptr);
				if(tr181ParamValue != NULL) {
					json_object_object_add(atmLinkQosJobj, ptr, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
				
			if(strcmp(paramList->name, "ATMQoS") == 0) {
				tr181ParamValue = json_object_object_get(atmLinkQosJobj, "QoSClass");
				if(tr181ParamValue != NULL) {
					json_object_object_add(atmLinkQosJobj, "QoSClass", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			/*End of special case*/
			/*Not defined in tr181, give it a default value*/

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;
		}
		else {
			paramList++;
		}
	} /*Edn while*/

    if(multiJobj) {
        if(dstAdrModified && atmStatus && (atmSetDisable == false)) {
			json_object_object_add(atmLinkJobj, "Enable", json_object_new_boolean(false));
			ret = zcfgFeObjJsonSet(RDM_OID_ATM_LINK, &atmLinkIid, atmLinkJobj, NULL);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Link.i fail, %d\n", __FUNCTION__, ret);
				json_object_put(atmLinkJobj);
				json_object_put(atmLinkQosJobj);
				return ret;	
			}

			json_object_object_add(atmLinkJobj, "Enable", json_object_new_boolean(true));
		}
		tmpObj = atmLinkJobj;
		atmLinkJobj = NULL;
		atmLinkJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ATM_LINK, &atmLinkIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
	}
		
	if(multiJobj == NULL){
		/*Set Device.ATM.Link.i.QoS first*/
		if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_LINK_QOS, &atmLinkIid, atmLinkQosJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Link.i.QoS fail\n", __FUNCTION__);
			json_object_put(atmLinkJobj);
			json_object_put(atmLinkQosJobj);
			return ret;
		}
		if(dstAdrModified && atmStatus && (atmSetDisable == false))
		{
			json_object_object_add(atmLinkJobj, "Enable", json_object_new_boolean(false));
			if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_LINK, &atmLinkIid, atmLinkJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Link.i fail\n", __FUNCTION__);
				json_object_put(atmLinkJobj);
				json_object_put(atmLinkQosJobj);
				return ret;	
			}
			json_object_object_add(atmLinkJobj, "Enable", json_object_new_boolean(true));
		}

		/*Set Device.ATM.Link.i*/
		if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_LINK, &atmLinkIid, atmLinkJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Link.i fail\n", __FUNCTION__);
			json_object_put(atmLinkJobj);
			json_object_put(atmLinkQosJobj);
			return ret;
		}
		json_object_put(atmLinkJobj);
		json_object_put(atmLinkQosJobj);
	}

	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_ATM_LINK

#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPTMLinkConfig

    Related object in TR181:
    Device.PTM.Link.i
 */
zcfgRet_t WANPtmLinkConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ptmLink[24] = {0};
	objIndex_t ptmLinkIid;
	struct json_object *ptmLinkJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	const char *ptmLinkParm = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANPTMLinkConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ptmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION //InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i mapping to eth link
	struct json_object *ethLinkJobj = NULL;
	objIndex_t ethLinkIid;
	IID_INIT(ethLinkIid);
	sscanf(ptmLink, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
	ethLinkIid.level = 1;
	if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, false)) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	
	ZOS_STRCPY_M(ptmLink, json_object_get_string(json_object_object_get(ethLinkJobj, "LowerLayers")));
	json_object_put(ethLinkJobj);
#endif
	if(strstr(ptmLink, "PTM") == NULL) {
		printf("%s : Not a PTM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	/*Get related tr181 objects*/
	IID_INIT(ptmLinkIid);
	sscanf(ptmLink, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
	ptmLinkIid.level = 1;

	/*Get Device.PTM.Link.i*/
	if((ret = feObjJsonGet(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ptmLinkJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Distinct name in tr181*/
		if(!strcmp(paramList->name, "LinkStatus")) {
			paramValue = json_object_object_get(ptmLinkJobj, "Status");
			if(!paramValue) {
				printf("%s: Can't get %s Status\n", __FUNCTION__, ptmLink);
				paramList++;
				continue;
			}
			
			ptmLinkParm = json_object_get_string(paramValue);
			if(!strcmp(ptmLinkParm, "Up") || !strcmp(ptmLinkParm, "Down")) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}else if(!strcmp(ptmLinkParm, "Dormant")) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Initializing"));
			}else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unavailable"));
			}
			paramList++;
			continue;
    		}

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}

	json_object_put(ptmLinkJobj);

	return ZCFG_SUCCESS;
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPTMLinkConfig

    Related object in TR181:
    Device.PTM.Link.i
 */
zcfgRet_t WANPtmLinkConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ptmLink[32] = {0};
	tr98Parameter_t *paramList = NULL;
	objIndex_t ptmLinkIid;
	struct json_object *ptmLinkJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANPTMLinkConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ptmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION //InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i mapping to eth link
	struct json_object *ethLinkJobj = NULL;
	objIndex_t ethLinkIid;
	IID_INIT(ethLinkIid);
	sscanf(ptmLink, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
	ethLinkIid.level = 1;
	if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, false)) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	
	ZOS_STRCPY_M(ptmLink, json_object_get_string(json_object_object_get(ethLinkJobj, "LowerLayers")));
	json_object_put(ethLinkJobj);
#endif

	if(strstr(ptmLink, "PTM") == NULL) {
		printf("%s : Not a PTM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(ptmLinkIid);
	sscanf(ptmLink, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
	ptmLinkIid.level = 1;

	/*Get Device.PTM.Link.i*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_PTM_LINK, &ptmLinkIid, &ptmLinkJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = ptmLinkJobj;
		ptmLinkJobj = NULL;
		ptmLinkJobj = zcfgFeJsonMultiObjAppend(RDM_OID_PTM_LINK, &ptmLinkIid, multiJobj, tmpObj);
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANPTMLinkConfig to Device.PTM.Link.i*/
			tr181ParamValue = json_object_object_get(ptmLinkJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ptmLinkJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/

	/*Set Device.PTM.Link.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_PTM_LINK, &ptmLinkIid, ptmLinkJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(ptmLinkJobj);
			return ret;
		}
		json_object_put(ptmLinkJobj);
	}

	return ZCFG_SUCCESS;
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPTMLinkConfig.Stats

    Related object in TR181:
    Device.PTM.Link.i.Stats
 */
zcfgRet_t WANPtmLinkConfStObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ptmLink[24] = {0};
	objIndex_t ptmLinkIid;
	struct json_object *ptmLinkStJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANPTMLinkConfig.Stats");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ptmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION //InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i mapping to eth link
	struct json_object *ethLinkJobj = NULL;
	objIndex_t ethLinkIid;
	IID_INIT(ethLinkIid);
	sscanf(ptmLink, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
	ethLinkIid.level = 1;
	if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, false)) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	
	ZOS_STRCPY_M(ptmLink, json_object_get_string(json_object_object_get(ethLinkJobj, "LowerLayers")));
	json_object_put(ethLinkJobj);
#endif


	if(strstr(ptmLink, "PTM") == NULL) {
		printf("%s : Not a PTM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	/*Get related tr181 objects*/
	IID_INIT(ptmLinkIid);
	sscanf(ptmLink, "PTM.Link.%hhu", &ptmLinkIid.idx[0]);
	ptmLinkIid.level = 1;

	if((ret = feObjJsonGet(RDM_OID_PTM_LINK_ST, &ptmLinkIid, &ptmLinkStJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(ptmLinkStJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "FramesSent")) { 
			paramValue = json_object_object_get(ptmLinkStJobj, "PacketsSent");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}	
		else if(!strcmp(paramList->name, "FramesReceived")) {
			paramValue = json_object_object_get(ptmLinkStJobj, "PacketsReceived");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}	
		else if(!strcmp(paramList->name, "OOSNearEnd") || !strcmp(paramList->name, "OOSFarEnd")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

	json_object_put(ptmLinkStJobj);

	return ZCFG_SUCCESS;
}
#endif

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANEthernetLinkConfig

    Related object in TR181:
    Device.Ethernet.Link
 */
zcfgRet_t WANEthLinkConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ethIntf[32] = {0};
	objIndex_t objIid;
	rdm_EthLink_t *ethIntfObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANEthernetLinkConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ethIntf)) == ZCFG_SUCCESS) {
		if(strstr(ethIntf, "Ethernet") == NULL) {
			printf("%s : Not an Ethernet Link\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(objIid);
		sscanf(ethIntf, "Ethernet.Interface.%hhu", &objIid.idx[0]);
		objIid.level = 1;

		/*Get Ethernet.Link.i*/
		if(feObjStructGet(RDM_OID_ETH_IFACE, &objIid, (void **)&ethIntfObj, updateFlag) == ZCFG_SUCCESS) {
			*tr98Jobj = json_object_new_object();

			if(!strcmp(ethIntfObj->Status, "Up") || !strcmp(ethIntfObj->Status, "Down")) {
				json_object_object_add(*tr98Jobj, "EthernetLinkStatus", json_object_new_string(ethIntfObj->Status));
			}
			else {
				json_object_object_add(*tr98Jobj, "EthernetLinkStatus", json_object_new_string("Unavailable"));
			}
			zcfgFeObjStructFree(ethIntfObj);
		}
		else {
			printf("%s : Can't get Ethernet.Link object\n", __FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	return ZCFG_SUCCESS;
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv4Address.i
 */
zcfgRet_t WANIpConnObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char ipIface[32] = {0};
	objIndex_t ipIfaceIid, vlanTermIid, ipv4AddrIid;
	objIndex_t dnsSrvIid, natIid, natSessionIid, ethLinkIid;
#ifdef IAAAA_CUSTOMIZATION
	objIndex_t ripIid;
	struct json_object *ripObj = NULL;
#endif
	struct json_object *ipIfaceJobj = NULL, *ipAddrJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *dnsSrvJobj = NULL;
	struct json_object *vlanTermJobj = NULL;
	struct json_object *ethLinkJobj = NULL;
	struct json_object *natJobj = NULL;
	struct json_object *natSessionJobj = NULL;
#ifdef IPV6INTERFACE_PROFILE
	objIndex_t dnsV6SrvIid;
	objIndex_t rtIid;
	struct json_object *dnsV6SrvJobj = NULL;
	struct json_object *rtJobj = NULL;
	const char *v6DefaultGw = NULL;
#endif
	tr98Parameter_t *paramList = NULL;
	int numOfEntries = 0;
	const char *connStatus = NULL;
	const char *lowerLayer = NULL;
#if 0
	int i;
	char pmPath[256] = {0};
	char tmp[256] = {0};
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char typeBuf[20] = {0};
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	const char *dnstype = NULL;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ret;
	}

	if(!strlen(ipIface) || !strstr(ipIface, "IP.Interface")){
		ztrdbg(ZTR_DL_ERR, "%s: querying ipiface returns %s, not IP.Interface\n", __FUNCTION__, ipIface);
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(ipIfaceIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
	ipIfaceIid.level = 1;

	if((ret = feObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface Fail\n", __FUNCTION__);
		goto ipConnFree;
	}
#ifdef IAAAA_CUSTOMIZATION
	getSpecificObj(RDM_OID_ROUTING_RIP_INTF_SET, "Interface", json_type_string, ipIface, &ripIid, &ripObj);
#endif
	
	
	/* Get Device.Ethernet.VLANTermination.i */
	lowerLayer = json_object_get_string(json_object_object_get(ipIfaceJobj, "LowerLayers"));
	if (lowerLayer && strstr(lowerLayer, "Ethernet.VLANTermination") != NULL){
		IID_INIT(vlanTermIid);
		sscanf(lowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
		vlanTermIid.level = 1;
		if((ret = feObjJsonGet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermJobj, updateFlag)) != ZCFG_SUCCESS)
			goto ipConnFree;
	}

	/* Get Device.Ethernet.Link.i */
	lowerLayer = json_object_get_string(json_object_object_get(vlanTermJobj, "LowerLayers"));
	if (lowerLayer && strstr(lowerLayer, "Ethernet.Link.") != NULL){
		IID_INIT(ethLinkIid);
		sscanf(lowerLayer, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
		ethLinkIid.level = 1;
		if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, updateFlag)) != ZCFG_SUCCESS)
			goto ipConnFree;
	}

	IID_INIT(natIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &natIid, &natJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get NAT Fail\n", __FUNCTION__);
		goto ipConnFree;
	}
	
	IID_INIT(natSessionIid);
	if((ret = feObjJsonGet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, &natSessionJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get RDM_OID_NAT_SESSION_CTL Fail\n", __FUNCTION__);
		goto ipConnFree;
	}
	
#ifdef IPV6INTERFACE_PROFILE
	IID_INIT(rtIid);
	rtIid.level = 1;
	rtIid.idx[0] = 1;
	if((ret = feObjJsonGet(RDM_OID_ROUTING_ROUTER, &rtIid, &rtJobj, updateFlag)) != ZCFG_SUCCESS) {
		goto ipConnFree;
	}
#endif

	/* Get Device.IP.Interface.i.IPv4Address.i */
	memcpy(&ipv4AddrIid, &ipIfaceIid, sizeof(objIndex_t));
	ipv4AddrIid.level++;
	ipv4AddrIid.idx[1] = 1;

	if(feObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &ipAddrJobj, updateFlag) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : Can't get IPv4Address.i Object\n", __FUNCTION__);
		ipAddrJobj = NULL;
	}
	else {
		zcfgLog(ZCFG_LOG_DEBUG, "%s : IPv4Address.i %s\n", __FUNCTION__, json_object_to_json_string(ipAddrJobj));
	}

	/*DNSServer*/
	dnsInfoGetJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false, updateFlag); //dns v4
#ifdef IPV6INTERFACE_PROFILE
	dnsInfoGetJobj(ipIface, &dnsV6SrvIid, (struct json_object **)&dnsV6SrvJobj, true, updateFlag); //dns v6
#endif

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(!strcmp(paramList->name, "Name")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_SrvName");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		if(ipAddrJobj != NULL) {
			paramValue = json_object_object_get(ipAddrJobj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		paramValue = json_object_object_get(ethLinkJobj, paramList->name);
		if(paramValue != NULL && strcmp(paramList->name, "MACAddress") == 0) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "ExternalIPAddress")) {
			if(ipAddrJobj != NULL) {
				paramValue = json_object_object_get(ipAddrJobj, "IPAddress");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}

				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
				paramList++;
				continue;
			}
		}
#if 0	/* Should be found in ipAddrJobj */
		else if(!strcmp(paramList->name, "SubnetMask")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}
#endif
		else if(!strcmp(paramList->name, "NATEnabled")) {
			natInfoSet(ipIface, *tr98Jobj);
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "DefaultGateway")) {
			defaultGwInfoSet(ipIface, *tr98Jobj);
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "DNSOverrideAllowed")){
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
            if(dnsSrvJobj != NULL) {
				dnstype = json_object_get_string(json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type"));
				if(!strcmp(dnstype,"Static"))
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
				else
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			else{
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
#else
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
#endif
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "DNSEnabled")) {
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "Enable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			paramList++;
			continue;
		}else if(!strcmp(paramList->name, "DNSServers")){
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "DNSServer");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				// Default DNS type is DHCPv4
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("DHCPv4"));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSV6Type")){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "X_ZYXEL_Type");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("DHCPv6"));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(isIGMPProxyEnable(ipIface)));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(isMLDProxyEnable(ipIface)));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "PossibleConnectionTypes")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_PossibleConnectionTypes");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "Uptime")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_UpTime");
			if(paramValue) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "ConnectionType")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_ConnectionType");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv4Enable")){
			paramValue = json_object_object_get(ipIfaceJobj, "IPv4Enable");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_IPv4Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "PortMappingNumberOfEntries")) {
			numOfEntries = portMappingNumGet(ipIface);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(numOfEntries));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "ConnectionStatus")) {
			paramValue = json_object_object_get(ipIfaceJobj, "Status");
			if(paramValue != NULL) {
				connStatus = json_object_get_string(paramValue);
				if(connStatus && !strcmp(connStatus, "Up")){
					const char *ipv4AddrStatus = NULL;
					if(ipAddrJobj != NULL){
						ipv4AddrStatus =json_object_get_string(json_object_object_get(ipAddrJobj, "Status"));
						if(!strcmp(ipv4AddrStatus, "Enabled"))
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Connected"));
						else
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Connecting"));
					}
					else
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Connecting"));
				}
				else if (connStatus && (!strcmp(connStatus, "Down") ||!strcmp(connStatus, "LowerLayerDown")))
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Disconnected"));
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unconfigured"));
			}
			else
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unconfigured"));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")) {
			if(vlanTermJobj != NULL) {
				paramValue = json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANEnable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")) {
			if (vlanTermJobj){
				paramValue = json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANPriority");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")) {
			if (vlanTermJobj){
				paramValue = json_object_object_get(vlanTermJobj, "VLANID");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
#ifdef IAAAA_CUSTOMIZATION
		else if(!strcmp(paramList->name, "X_ZYXEL_RIPEnable"))
		{
			if(ripObj != NULL)
			{
				paramValue = json_object_object_get(ripObj, "Enable");
				if (paramValue != NULL)
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			paramList++;
			continue;
		}
#endif
#ifndef MSTC_TAAAG_MULTI_USER
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") || !strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#else
		else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif			
				!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
				!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
			if(natJobj){
				paramValue = json_object_object_get(natJobj, paramList->name);
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
			if(natSessionJobj){
				paramValue = json_object_object_get(natSessionJobj, "MaxSessionPerHost");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
#ifdef IPV6INTERFACE_PROFILE		
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6Enable");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")) {
			//paramValue = json_object_object_get(ipIfaceJobj, "IPv6Status");
			//json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			if(	json_object_get_boolean(json_object_object_get(ipIfaceJobj, "IPv6Enable")))
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_ENABLED));
			else
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_DISABLED));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6AddressNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6PrefixNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}		
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "Enable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			paramList++;
			continue;
		}
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "DNSServer");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6DefaultGatewayIface")) {
			if(rtJobj != NULL) {
				paramValue = json_object_object_get(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway");
				if(paramValue != NULL) {
					v6DefaultGw = json_object_get_string(paramValue);
					if(v6DefaultGw && strstr(v6DefaultGw, ipIface)) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
					}
					else {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					}
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_Enable_DSLite");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")) {
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLite_Type");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif

		/*Not defined in tr181, give it a default value*/
		if(!strcmp(paramList->name, "LastConnectionError")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("ERROR_NONE"));
		}
		else if(!strcmp(paramList->name, "ConnectionTrigger")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("On-Demand"));
		}
		else if(!strcmp(paramList->name, "RouteProtocolRx")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Off"));
		}
		else if(!strcmp(paramList->name, "ShapingRate")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(-1));
		}
		else {

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
		}

		paramList++;
	}
ipConnFree:
	if (ipIfaceJobj)
		json_object_put(ipIfaceJobj);
	if(ipAddrJobj)
		json_object_put(ipAddrJobj);
	if(dnsSrvJobj)
		json_object_put(dnsSrvJobj);
	if(vlanTermJobj)
		json_object_put(vlanTermJobj);
	if (ethLinkJobj)
		json_object_put(ethLinkJobj);
	if(natJobj){
		json_object_put(natJobj);
	}
	if(natSessionJobj){
		json_object_put(natSessionJobj);
	}
#ifdef IAAAA_CUSTOMIZATION
	if(ripObj)
		json_object_put(ripObj);
#endif
#ifdef IPV6INTERFACE_PROFILE
	if(dnsV6SrvJobj){
		json_object_put(dnsV6SrvJobj);
	}
	if(rtJobj)
		json_object_put(rtJobj);
#endif	
	return ret;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv4Address.i
 */
zcfgRet_t WANIpConnObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char devIpIface[32] = {0};
	objIndex_t ipIfaceIid;
#ifdef IAAAA_CUSTOMIZATION
	objIndex_t ripIid;
	struct json_object *ripObj = NULL;
#endif
	objIndex_t dnsSrvIid, dnsSrvV6Iid, dnsIid, igmpIid, mldIid, vlanTermIid, natIid, natSessionIid;
	objIndex_t ipv4AddrIid, dhcpv4CltIid;
	struct json_object *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *paramValue2 = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *dnsSrvJobj = NULL, *dnsSrvV6Jobj = NULL, *dnsJobj = NULL, *igmpJobj = NULL, *mldJobj = NULL,
						*vlanTermJobj = NULL, *natJobj = NULL, *natSessionJobj = NULL;
	struct json_object *ipAddrJobj = NULL, *dhcpv4CltObj = NULL;
	struct json_object *ipAddrMultiJobj = NULL;
	const char *addrType = NULL, *connType = NULL;
	char tr181AddrType[20] = {0};
	tr98Parameter_t *paramList = NULL;
#if 0
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char typeBuf[20] = {0};
#endif
	char tmpbuf[256] = {0};
	char *igmpInterface = NULL, *mldInterface = NULL, *intf = NULL, *tmp_ptr = NULL;
	bool isDnsObjExist = false;
	bool isDnsV6ObjExist = false;
	objIndex_t natIntfIid;
	struct json_object *natIntfJobj = NULL;
	objIndex_t rtIpIid;
	struct json_object *rtIpJobj = NULL;
	char *new_activeDefaultGt = NULL;
	objIndex_t rtIid;
	struct json_object *rtJobj = NULL;
	bool defaultRtModify = false;
	const char *lowerLayer = NULL;
	bool staticDnsType = false;
	bool staticDnsV6Type = false;
	char tr181DnsType[45+1] = {0};
	char tr181DnsV6Type[45+1] = {0};
	bool runSpvValid;
	bool AutoSetStatic = false;
	const char Static[] = "Static";
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
    bool Igmpsetflag = false;
	bool ipv4setflag = false;
	char X_ZYXEL_IfName[32] = {0};
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	struct json_object *tr98SpvObj = NULL;
	tr98SpvObj = zcfgFeRetrieveSpv(tr98FullPathName);

	if(tr98SpvObj && (json_object_object_get(tr98SpvObj, "AddressingType") == NULL) &&
		json_object_object_get(tr98SpvObj, "ExternalIPAddress") &&
		json_object_object_get(tr98SpvObj, "DefaultGateway") &&
		json_object_object_get(tr98SpvObj, "SubnetMask") &&
		strcmp(json_object_get_string(json_object_object_get(tr98Jobj, "AddressingType")), "Static"))
		AutoSetStatic = true;

#ifdef MTK_PLATFORM
	bool ipIfaceReqReset = false;

	if(tr98SpvObj && json_object_object_get(tr98SpvObj, "X_ZYXEL_VlanEnable") &&
			json_object_object_get(tr98SpvObj, "X_ZYXEL_VlanMux8021p") &&
			json_object_object_get(tr98SpvObj, "X_ZYXEL_VlanMuxID")
			// && possible other params
			)
	{
		ipIfaceReqReset = true;
	}
#endif


	//sprintf(devIpIface, "Device.%s", ipIface);
	sprintf(devIpIface, "%s", ipIface);
	IID_INIT(ipIfaceIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
	ipIfaceIid.level = 1;

	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj)) != ZCFG_SUCCESS)
		return ret;

#ifndef ABUU_CUSTOMIZATION
	json_object_object_add(ipIfaceJobj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid.idx[0] - 1));	
	json_object_object_add(ipIfaceJobj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid.idx[0] - 1));
#endif
	
	/* GetDevice.Ethernet.VLANTermination.i */
	lowerLayer = json_object_get_string(json_object_object_get(ipIfaceJobj, "LowerLayers"));
	if (strstr(lowerLayer, "Ethernet.VLANTermination") != NULL){
		IID_INIT(vlanTermIid);
		sscanf(lowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
		vlanTermIid.level = 1;
		if((ret = zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermJobj)) != ZCFG_SUCCESS) {
			json_object_put(ipIfaceJobj);
			return ret;
		}
	}
#ifdef IAAAA_CUSTOMIZATION
	getSpecificObj(RDM_OID_ROUTING_RIP_INTF_SET, "Interface", json_type_string, ipIface, &ripIid, &ripObj);
#endif
	
	IID_INIT(natIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &natIid, &natJobj)) != ZCFG_SUCCESS){
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		return ret;
	}
	
	IID_INIT(natSessionIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, &natSessionJobj)) != ZCFG_SUCCESS){
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		return ret;
	}


	/*DNSServer*/
	const char *tr98DnsType = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_DNSType"));
	if(json_object_object_get(tr98SpvObj, "DNSServers") || (tr98DnsType && !strcmp(tr98DnsType, TR181_DNS_TYPE_STATIC)))
		staticDnsType = true;

	const char *tr98DnsV6Type = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_DNSV6Type"));
	if(json_object_object_get(tr98SpvObj, "X_ZYXEL_IPv6DNSServers") || (tr98DnsV6Type && !strcmp(tr98DnsV6Type, TR181_DNS_TYPE_STATIC)))
	{
		staticDnsV6Type = true;
	}

	if(dnsInfoGetJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false, false) == ZCFG_SUCCESS){ //dns v4
		isDnsObjExist = true;
		const char *paramDnsType = json_object_get_string(json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type"));
		if(paramDnsType && strlen(paramDnsType))
			ZOS_STRCPY_M(tr181DnsType, paramDnsType);
	}

#ifdef IPV6INTERFACE_PROFILE
	if(dnsInfoGetJobj(ipIface, &dnsSrvV6Iid, (struct json_object **)&dnsSrvV6Jobj, true, false) == ZCFG_SUCCESS) //dns v6
	{
		isDnsV6ObjExist = true;
		const char *paramDnsV6Type = json_object_get_string(json_object_object_get(dnsSrvV6Jobj, "X_ZYXEL_Type"));
		if(paramDnsV6Type && strlen(paramDnsV6Type))
		{
			strncpy(tr181DnsV6Type, paramDnsV6Type, sizeof(tr181DnsV6Type)-1);
		}
	}
#endif
	if(!isDnsObjExist && (staticDnsType == true)){
		if(dnsAddJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj,false) != ZCFG_SUCCESS) 
		{
			json_object_put(ipIfaceJobj);
			json_object_put(vlanTermJobj);
			json_object_put(natJobj);
			json_object_put(natSessionJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		isDnsObjExist = true;
	}

#ifdef IPV6INTERFACE_PROFILE
	if(!isDnsV6ObjExist && (staticDnsV6Type == true))
	{
		if(dnsAddJobj(ipIface, &dnsSrvV6Iid, (struct json_object **)&dnsSrvV6Jobj,true) != ZCFG_SUCCESS)
		{
			json_object_put(ipIfaceJobj);
			json_object_put(vlanTermJobj);
			json_object_put(natJobj);
			json_object_put(natSessionJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		isDnsV6ObjExist = true;
	}
#endif

	IID_INIT(dnsIid);
	zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsJobj);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsJobj)) != ZCFG_SUCCESS) {
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(dnsSrvJobj);
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ret;
	}

	IID_INIT(igmpIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpIid, &igmpJobj)) != ZCFG_SUCCESS) {
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ret;
	}

	IID_INIT(mldIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &mldIid, &mldJobj)) != ZCFG_SUCCESS) {
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(igmpJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ret;
	}

	IID_INIT(natIntfIid);
	if(natIntfGetJobj((const char *)ipIface, &natIntfIid, (struct json_object **)&natIntfJobj) != ZCFG_SUCCESS) {
		printf("%s : Retrieve NAT interface setting fail\n", __FUNCTION__);
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(igmpJobj);
		json_object_put(mldJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ZCFG_INTERNAL_ERROR;
	}

	paramValue = json_object_object_get(tr98Jobj, "AddressingType");
	if(AutoSetStatic)
		addrType = &Static;
	else
		addrType = json_object_get_string(paramValue);

	paramValue = json_object_object_get(tr98Jobj, "ConnectionType");
	connType = json_object_get_string(paramValue);

	runSpvValid = zcfgFeTr98StatusRunSpvValid(multiJobj);

	IID_INIT(rtIid);
	rtIid.level = 1;
	rtIid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &rtIid, &rtJobj)) != ZCFG_SUCCESS) {
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		json_object_put(igmpJobj);
		json_object_put(mldJobj);
		json_object_put(natIntfJobj);

		return ret;
	}

	IID_INIT(rtIpIid);
	if(!strcmp(connType, "IP_Routed") &&
		!runSpvValid && defaultRtGetObj((const char *)ipIface, &rtIpIid, (struct json_object **)&rtIpJobj, addrType) != ZCFG_SUCCESS) {
		printf("%s : Retrieve routing ip fail\n", __FUNCTION__);
		json_object_put(ipIfaceJobj);
		json_object_put(vlanTermJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(igmpJobj);
		json_object_put(mldJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		json_object_put(natIntfJobj);
		json_object_put(rtJobj);
		return ZCFG_INTERNAL_ERROR;
	}
#if 0
	//tr181AddrTypeQuery(ipIfaceIid, tr181AddrType)
	{
		const char *addrTypeStr = NULL;

		memcpy(&ipv4AddrIid, &ipIfaceIid, sizeof(objIndex_t));
		ipv4AddrIid.level++;
		ipv4AddrIid.idx[1] = 1;

		if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &ipAddrJobj) == ZCFG_SUCCESS) {
			addrTypeStr = json_object_get_string(json_object_object_get(ipAddrJobj, "AddressingType"));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
         const char *dnsaddr = NULL, *ipaddr=NULL, *pvcstatus=NULL, *ipstatus=NULL;
         bool ipenable = false;
         dnsaddr = json_object_get_string(json_object_object_get(tr98Jobj, "DNSServers"));
		 pvcstatus = json_object_get_string(json_object_object_get(ipIfaceJobj, "Status"));
		 ipaddr = json_object_get_string(json_object_object_get(ipAddrJobj, "IPAddress"));
		 ipstatus = json_object_get_string(json_object_object_get(ipAddrJobj, "Status"));
		 ipenable = json_object_get_boolean(json_object_object_get(ipAddrJobj, "Enable"));
		 if(!strcmp(pvcstatus, "Up") && !strcmp(ipstatus,"Enabled") && ipenable && dnsaddr && ipaddr && strlen(dnsaddr) && strlen(ipaddr) && strstr(dnsaddr,ipaddr)){
		 	printf("\r\n%s,go to error set,ip is %s,dns is %s\r\n", "WanIpSet",ipaddr,dnsaddr);
			ret = ZCFG_REQUEST_REJECT;
			json_object_put(ipAddrJobj);
			json_object_put(ipIfaceJobj);
			json_object_put(vlanTermJobj);
			json_object_put(natJobj);
			json_object_put(natSessionJobj);
			json_object_put(igmpJobj);
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
			json_object_put(natIntfJobj);
			json_object_put(rtIpJobj);
			json_object_put(rtJobj);		
			return ret;
		 }
#endif
		}

		if(addrTypeStr && strlen(addrTypeStr))
			strncpy(tr181AddrType, addrTypeStr, sizeof(tr181AddrType)-1);

		json_object_put(ipAddrJobj);
		ipAddrJobj = NULL;
	}
#endif
	if(!strcmp(connType, "IP_Routed")) {
		/*Set IP Address*/
		if(multiJobj)
			ret = ipaddrSet(ipIface, &ipIfaceIid, &ipIfaceJobj, addrType, tr98Jobj, ipAddrMultiJobj);
		else
			ret = ipaddrSet(ipIface, &ipIfaceIid, &ipIfaceJobj, addrType, tr98Jobj, NULL);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			printf("%s: ipaddrSet return %d\n", __FUNCTION__, ret);
			json_object_put(ipIfaceJobj);
			json_object_put(vlanTermJobj);
			json_object_put(natJobj);
			json_object_put(natSessionJobj);
			json_object_put(igmpJobj);
			json_object_put(mldJobj);
			json_object_put(dnsSrvJobj);
#ifdef IPV6INTERFACE_PROFILE
			json_object_put(dnsSrvV6Jobj);
#endif
			json_object_put(dnsJobj);
			json_object_put(natIntfJobj);
			json_object_put(rtIpJobj);
			json_object_put(rtJobj);
			return ret;
		}
		else {
			ret = ZCFG_SUCCESS;
		}
	}


	if(multiJobj){
		multiJobj = zcfgFeSessMultiObjSetUtilize(multiJobj);

		if(vlanTermJobj){
			tmpObj = vlanTermJobj;
			vlanTermJobj = NULL;
			vlanTermJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ETH_VLAN_TERM, &vlanTermIid, "apply", json_object_new_boolean(false));
		}
#ifdef IAAAA_CUSTOMIZATION
		if(natIntfJobj)
#endif
		{
			tmpObj = natIntfJobj;
			natIntfJobj = NULL;
			natIntfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_INTF_SETTING, &natIntfIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT_INTF_SETTING, &natIntfIid, "apply", json_object_new_boolean(false));
		}

		if(dnsSrvJobj){
			tmpObj = dnsSrvJobj;
			dnsSrvJobj = NULL;
			dnsSrvJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, multiJobj, tmpObj);			
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, "apply", json_object_new_boolean(false));
		}

#ifdef IPV6INTERFACE_PROFILE
		if(dnsSrvV6Jobj){
			tmpObj = dnsSrvV6Jobj;
			dnsSrvV6Jobj = NULL;
			dnsSrvV6Jobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, multiJobj, tmpObj);			
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, "apply", json_object_new_boolean(true));
		}
#endif
#ifdef IAAAA_CUSTOMIZATION
	if(ipIfaceJobj)
#endif
		{
			tmpObj = ipIfaceJobj;
			ipIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipIfaceIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_IFACE, &ipIfaceIid, "apply", json_object_new_boolean(true));
		}

//#ifdef IAAAA_CUSTOMIZATION
	if(rtIpJobj)
//#endif
		{
			tmpObj = rtIpJobj;
			rtIpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "apply", json_object_new_boolean(false));
		}

		IID_INIT(ipv4AddrIid);
		memcpy(&ipv4AddrIid, &ipIfaceIid, sizeof(objIndex_t));
		ipv4AddrIid.level++;
		ipv4AddrIid.idx[1] = 1;
		zcfgFeJsonMultiObjRetrieve(ipAddrMultiJobj, RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, NULL, &ipAddrJobj);
		if(ipAddrJobj){
			zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, multiJobj, ipAddrJobj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, "apply", json_object_new_boolean(false));
		}

		IID_INIT(dhcpv4CltIid);
		zcfgFeJsonMultiObjRetrieve(ipAddrMultiJobj, RDM_OID_DHCPV4_CLIENT, NULL, &dhcpv4CltIid, &dhcpv4CltObj);
		if(dhcpv4CltObj){
			zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_CLIENT, &dhcpv4CltIid, multiJobj, dhcpv4CltObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DHCPV4_CLIENT, &dhcpv4CltIid, "apply", json_object_new_boolean(true));
		}

		if(igmpJobj){
			tmpObj = igmpJobj;
			igmpJobj = NULL;
			igmpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IGMP, &igmpIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ZY_IGMP, &igmpIid, "apply", json_object_new_boolean(true));
		}

		if(mldJobj){
			tmpObj = mldJobj;
			mldJobj = NULL;
			mldJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_MLD, &mldIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ZY_MLD, &mldIid, "apply", json_object_new_boolean(true));
		}
		
		if(natSessionJobj){
			tmpObj = natSessionJobj;
			natSessionJobj = NULL;
			natSessionJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_SESSION_CTL, &natSessionIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT_SESSION_CTL, &natSessionIid, "apply", json_object_new_boolean(false));
		}
#ifdef IAAAA_CUSTOMIZATION
	if(rtJobj)
#endif
		{
			tmpObj = rtJobj;
			rtJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER, &rtIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			
		}

		if(dnsJobj){
			tmpObj = dnsJobj;
			dnsJobj = NULL;
			dnsJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS, &dnsIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS, &dnsIid, "apply", json_object_new_boolean(true));
		}

		if(natJobj){
			tmpObj = natJobj;
			natJobj = NULL;
			natJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &natIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT, &natIid, "apply", json_object_new_boolean(true));
		}
		
#ifdef IAAAA_CUSTOMIZATION
		if(ripObj)
		{
			tmpObj = ripObj;
			ripObj = NULL;
			ripObj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_RIP_INTF_SET, &ripIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_RIP_INTF_SET, &ripIid, "apply", json_object_new_boolean(true));
		}
#endif

		zcfgFeJsonMultiObjSetBlock(multiJobj);
		zcfgFeJsonMultiObjSetDelayApply(multiJobj);
	}


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
			if(!strcmp(paramList->name, "Enable")) {
				json_object_object_add(ipIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
                if(json_object_get_boolean(paramValue)){
                          ipv4setflag = true;
				}
				if(json_object_get_boolean(paramValue) && strstr(json_object_get_string(json_object_object_get(ipIfaceJobj, "X_TT_SrvName")), "IPTV_")){
                          Igmpsetflag = true;
				}
				paramList++;
				continue;
			}
#endif
			if(!strcmp(paramList->name, "Name")){
				tr181ParamValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_SrvName");
			}
			else {
				tr181ParamValue = json_object_object_get(ipIfaceJobj, paramList->name);
			}
			if(tr181ParamValue != NULL) {
				if(!strcmp(paramList->name, "X_ZYXEL_DefaultGatewayIface")){
					bool tr98SetDefaultIface = json_object_get_boolean(paramValue);
					bool tr181SetDefaultIface = json_object_get_boolean(tr181ParamValue);
					const char *activeDefaultGt = json_object_get_string(json_object_object_get(rtJobj, "X_ZYXEL_ActiveDefaultGateway"));

					if(tr98SetDefaultIface && !tr181SetDefaultIface)
						new_activeDefaultGt = activeDefaultGt_Add(activeDefaultGt, ipIface);

					if(!tr98SetDefaultIface && tr181SetDefaultIface)
						new_activeDefaultGt = activeDefaultGt_Strip(activeDefaultGt, ipIface);

					if(new_activeDefaultGt) {
						json_object_object_add(rtJobj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(new_activeDefaultGt));
					}
				}

				if(!strcmp(paramList->name, "Name")){
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
					char *setvalue = NULL;
					setvalue = json_object_get_string(paramValue);
					if(!strcmp(setvalue, "")){
						ret = ZCFG_INVALID_PARAM_VALUE;
						ZOS_STRCPY_M(paramfault,"Name");
						return ret;
					}
#endif	
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_SrvName", JSON_OBJ_COPY(paramValue));
				}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				else if(!strcmp(paramList->name, "X_TTNET_Bootp_Flag")){
					unsigned int X_TAAAG_Bootp_Flag = 0;
					X_TAAAG_Bootp_Flag = json_object_get_int(paramValue);
					if(X_TAAAG_Bootp_Flag < 0 || X_TAAAG_Bootp_Flag > 1){
							sprintf(paramfault, "X_TTNET_Bootp_Flag value is wrong");
							return ZCFG_INVALID_PARAM_VALUE;
					}
					json_object_object_add(ipIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
#endif
				else if(!strcmp(paramList->name, "Reset")){
					json_object_object_add(ipIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
				else
					json_object_object_add(ipIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));

				paramList++;
				continue;
			}

			/*special case*/
			if(!strcmp(paramList->name, "ConnectionType")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_ConnectionType", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv4Enable")) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
                if(ipv4setflag == true){
					json_object_object_add(ipIfaceJobj, "IPv4Enable", json_object_new_boolean(true));
                }
				else{
				json_object_object_add(ipIfaceJobj, "IPv4Enable", JSON_OBJ_COPY(paramValue));
				}
#else
				json_object_object_add(ipIfaceJobj, "IPv4Enable", JSON_OBJ_COPY(paramValue));
#endif
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "NATEnabled")){
				json_object_object_add(natIntfJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")){
				// const char *cmd = json_object_get_string(paramValue);
				json_object_object_add(natIntfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "DefaultGateway") && rtIpJobj){
				const char *defaultRt = json_object_get_string(json_object_object_get(tr98SpvObj, "DefaultGateway"));
				const char *spvAddrType = NULL;
				if(AutoSetStatic)
					spvAddrType = &Static;
				else
					if(json_object_object_get(tr98SpvObj, "AddressingType") != NULL)
						spvAddrType = json_object_get_string(json_object_object_get(tr98SpvObj, "AddressingType"));
					else
						spvAddrType = json_object_get_string(json_object_object_get(tr98Jobj, "AddressingType"));

				if(defaultRt && strcmp(spvAddrType, "Static") && strcmp(addrType, "Static")){
					sprintf(paramfault, "Can't modify DefaultGateway");
					return ZCFG_INVALID_PARAM_VALUE;
				}

				if(defaultRt && !strcmp(defaultRt, "")) {
					json_object_object_add(rtIpJobj, "GatewayIPAddress", json_object_new_string(""));
					json_object_object_add(rtIpJobj, "Enable", json_object_new_boolean(false));
					json_object_object_add(rtIpJobj, "StaticRoute", json_object_new_boolean(true));
					json_object_object_add(rtIpJobj, "Origin", json_object_new_string("Static"));
					zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "skipReadOnlyCheck", json_object_new_boolean(true));
					defaultRtModify = true;
				}
				else if(defaultRt && strcmp(defaultRt, "")) {
					json_object_object_add(rtIpJobj, "GatewayIPAddress", json_object_new_string(defaultRt));
					json_object_object_add(rtIpJobj, "Enable", json_object_new_boolean(true));
					json_object_object_add(rtIpJobj, "StaticRoute", json_object_new_boolean(true));
					json_object_object_add(rtIpJobj, "Origin", json_object_new_string("Static"));
					zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "skipReadOnlyCheck", json_object_new_boolean(true));
					defaultRtModify = true;
				}else if(spvAddrType && !strcmp(spvAddrType, "Static")){
					json_object_object_add(rtIpJobj, "StaticRoute", json_object_new_boolean(true));
					json_object_object_add(rtIpJobj, "Origin", json_object_new_string("Static"));
					zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "skipReadOnlyCheck", json_object_new_boolean(true));
					defaultRtModify = true;
				}else if(spvAddrType && !strcmp(spvAddrType, "DHCP")){
					json_object_object_add(rtIpJobj, "StaticRoute", json_object_new_boolean(false));
					json_object_object_add(rtIpJobj, "Origin", json_object_new_string("DHCPv4"));
					zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "skipReadOnlyCheck", json_object_new_boolean(true));
					defaultRtModify = true;
				}
#if 0
				const char *defaultRt = json_object_get_string(paramValue);
				const char *defaultRtAddr = json_object_get_string(json_object_object_get(rtIpJobj, "GatewayIPAddress"));
				if(!strcmp(defaultRt, "") && strcmp(defaultRtAddr, "")) {
					json_object_object_add(rtIpJobj, "GatewayIPAddress", json_object_new_string(""));
					json_object_object_add(rtIpJobj, "Enable", json_object_new_boolean(false));
					defaultRtModify = true;
				}
				else if(strcmp(defaultRt, defaultRtAddr)) {
					json_object_object_add(rtIpJobj, "DestIPAddress", json_object_new_string(""));
					json_object_object_add(rtIpJobj, "DestSubnetMask", json_object_new_string(""));
					json_object_object_add(rtIpJobj, "GatewayIPAddress", json_object_new_string(defaultRt));
					json_object_object_add(rtIpJobj, "Enable", json_object_new_boolean(true));
					defaultRtModify = true;
				}

				if(strcmp(tr181AddrType, "DHCP") && !strcmp(addrType, "DHCP")){
					json_object_object_add(rtIpJobj, "Interface", json_object_new_string(""));
					defaultRtModify = true;
				}
				else if(!strcmp(addrType, "Static")){
					json_object_object_add(rtIpJobj, "StaticRoute", json_object_new_boolean(true));
					json_object_object_add(rtIpJobj, "Origin", json_object_new_string("Static"));
					zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, "skipReadOnlyCheck", json_object_new_boolean(true));
					defaultRtModify = true;
				}
#endif
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "DNSEnabled")){
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "DNSServers")){
				if(json_object_object_get(tr98SpvObj, "DNSServers") && dnsSrvJobj){
					printf("%s: spv DNSServers\n", __FUNCTION__);
					json_object_object_add(dnsSrvJobj, "DNSServer", JSON_OBJ_COPY(paramValue));
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("Static"));
				}
				else if(dnsSrvJobj){
					paramValue2 = json_object_object_get(dnsSrvJobj, "DNSServer");
#if 0
					if(paramValue2){
						if(strcmp(json_object_get_string(paramValue), json_object_get_string(paramValue2))){
							json_object_object_add(tr98Jobj, "X_ZYXEL_DNSType", json_object_new_string(TR181_DNS_TYPE_STATIC)); // need to set tr98Jobj, workaround
							json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string(TR181_DNS_TYPE_STATIC));
							if(!strcmp(json_object_get_string(paramValue), "")){
								json_object_object_add(dnsSrvJobj, "Interface", json_object_new_string(""));
							}
						}
					}
#endif
					if(!strcmp(tr181DnsType, TR181_DNS_TYPE_STATIC) &&  strcmp(tr98DnsType, TR181_DNS_TYPE_STATIC))
						json_object_object_add(dnsSrvJobj, "DNSServer", json_object_new_string(""));
					else
						json_object_object_add(dnsSrvJobj, "DNSServer", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
				struct json_object *spvParamValue;
#if 0 //check null point IChia 20190923
				if((spvParamValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_DNSType"))){
#else
				if(dnsSrvJobj &&  (spvParamValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_DNSType"))){
#endif
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", JSON_OBJ_COPY(spvParamValue));
				}
#if 0
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
#endif
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DNSV6Type"))
			{
				struct json_object *spvV6ParamValue;
				if(dnsSrvV6Jobj &&  (spvV6ParamValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_DNSV6Type")))
				{
					json_object_object_add(dnsSrvV6Jobj, "X_ZYXEL_Type", JSON_OBJ_COPY(spvV6ParamValue));
				}
			}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
            else if(!strcmp(paramList->name, "DNSOverrideAllowed")){
			bool dnstype = 0;
			dnstype = json_object_get_boolean(paramValue);
			if(dnstype == 0)
			{
			    if(dnsSrvJobj){
				    json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("Static"));
			    }	
			}
			else
			{	
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("DHCPv4"));
				}	
			}
            }
#endif
			else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")) {
				igmpInterface = strdup(json_object_get_string(json_object_object_get(igmpJobj, "Interface")));
				memset(tmpbuf, 0, sizeof(tmpbuf));
				if(json_object_get_boolean(paramValue)){
					if(!strcmp(igmpInterface, "")){
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", ipIface);
					}
					else{
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", igmpInterface);
						if(!strstr(igmpInterface, ipIface)){
							strcat(tmpbuf, ",");
							strcat(tmpbuf, ipIface);
						}
					}
				}
				else{
					if(strstr(igmpInterface, ipIface) != NULL){
						tmp_ptr = igmpInterface;
						intf = strtok_r(igmpInterface, ",", &tmp_ptr);
						while(intf != NULL){
							if(*intf != '\0' && !strstr(intf, ipIface)){
								if(tmpbuf[0] != '\0'){
									strcat(tmpbuf, ",");
								}
								strcat(tmpbuf, intf);
							}
							intf = strtok_r(NULL, ",", &tmp_ptr);
						}
					}
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", igmpInterface);
				}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
                if(Igmpsetflag == true){
 					if(!strcmp(tmpbuf, "")){
						ZOS_STRCPY_M(tmpbuf, ipIface);
					}
					else{
						if(!strstr(tmpbuf, ipIface)){
							strcat(tmpbuf, ",");
							strcat(tmpbuf, ipIface);
						}
					}
                }
#endif
				json_object_object_add(igmpJobj, "Interface", json_object_new_string(tmpbuf));

				if(igmpInterface)
					free(igmpInterface);

				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")) {
				mldInterface = strdup(json_object_get_string(json_object_object_get(mldJobj, "Interface")));
				memset(tmpbuf, 0, sizeof(tmpbuf));
				if(json_object_get_boolean(paramValue)){
					if(!strcmp(mldInterface, "")){
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", ipIface);
					}
					else{
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", mldInterface);
						if(!strstr(mldInterface, ipIface)){
							strncat(tmpbuf, ",", sizeof(tmpbuf)-strlen(tmpbuf)-1);
							strncat(tmpbuf, ipIface, sizeof(tmpbuf)-strlen(tmpbuf)-1);
						}
					}
				}
				else{
					if(strstr(mldInterface, ipIface) != NULL){
						tmp_ptr = mldInterface;
						intf = strtok_r(mldInterface, ",", &tmp_ptr);
						while(intf != NULL){
							if(*intf != '\0' && !strstr(intf, ipIface)){
								if(tmpbuf[0] != '\0'){
									strncat(tmpbuf, ",", sizeof(tmpbuf)-strlen(tmpbuf)-1);
								}
								strncat(tmpbuf, intf, sizeof(tmpbuf)-strlen(tmpbuf)-1);
							}
							intf = strtok_r(NULL, ",", &tmp_ptr);
						}
					}
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", mldInterface);
				}

				json_object_object_add(mldJobj, "Interface", json_object_new_string(tmpbuf));

				if(mldInterface)
					free(mldInterface);

				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")){
				if(vlanTermJobj){
					json_object_object_add(vlanTermJobj, "X_ZYXEL_VLANEnable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")){
				if(vlanTermJobj){
					printf("set X_ZYXEL_VlanMux8021p\n");
					if(json_object_get_boolean(json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANEnable"))
						&& !strcmp(json_object_get_string(paramValue),"-1")){
						json_object_object_add(vlanTermJobj, "X_ZYXEL_VLANPriority", json_object_new_int(0));
					}
					else{
						json_object_object_add(vlanTermJobj, "X_ZYXEL_VLANPriority", JSON_OBJ_COPY(paramValue));
					}
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")){
				if(vlanTermJobj){
					printf("set X_ZYXEL_VlanMuxID\n");
					json_object_object_add(vlanTermJobj, "VLANID", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#ifndef MSTC_TAAAG_MULTI_USER			
			else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") || !strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#else
			else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif
					!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
					!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
				if(natJobj){
					json_object_object_add(natJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
				if(natSessionJobj){
					json_object_object_add(natSessionJobj, "MaxSessionPerHost", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#ifdef IPV6INTERFACE_PROFILE		
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv6DefaultGatewayIface")){
				bool tr98SetV6DefaultIface = json_object_get_boolean(paramValue);
				const char *activeV6DefaultGt = json_object_get_string(json_object_object_get(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway"));
				const char *new_activeV6DefaultGt = NULL;

				if(tr98SetV6DefaultIface && !strstr(activeV6DefaultGt, ipIface))
					new_activeV6DefaultGt = activeDefaultGt_Add(activeV6DefaultGt, ipIface);

				if(!tr98SetV6DefaultIface && strstr(activeV6DefaultGt, ipIface))
					new_activeV6DefaultGt = activeDefaultGt_Strip(activeV6DefaultGt, ipIface);

				if(new_activeV6DefaultGt) {
					json_object_object_add(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway", json_object_new_string(new_activeV6DefaultGt));
					free(new_activeV6DefaultGt);
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")) {
				json_object_object_add(ipIfaceJobj, "IPv6Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
				if(dnsSrvV6Jobj){
					json_object_object_add(dnsSrvV6Jobj, "Enable", JSON_OBJ_COPY(paramValue));
					paramList++;	
					continue;
				}
			}
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
				if(dnsSrvV6Jobj){	
					paramValue2 = json_object_object_get(dnsSrvV6Jobj, "DNSServer");		
					if(paramValue2){
						if(strcmp(json_object_get_string(paramValue), json_object_get_string(paramValue2))){
							json_object_object_add(dnsSrvV6Jobj, "X_ZYXEL_Type", json_object_new_string(TR181_DNS_TYPE_STATIC));
							if(!strcmp(json_object_get_string(paramValue), "")){
								json_object_object_add(dnsSrvV6Jobj, "Interface", json_object_new_string(""));
							}
						}
					}
					json_object_object_add(dnsSrvV6Jobj, "DNSServer", JSON_OBJ_COPY(paramValue));

					paramList++;	
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_Enable_DSLite", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLite_Type", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#endif
#ifdef IAAAA_CUSTOMIZATION
		else if(!strcmp(paramList->name, "X_ZYXEL_RIPEnable"))
		{
			if(ripObj != NULL)
				if(paramValue != NULL )
					json_object_object_add(ripObj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;		
		}
#endif				
		}
		paramList++;	
	}

#ifdef MTK_PLATFORM
	if(ipIfaceReqReset)
	{
		json_object_object_add(ipIfaceJobj, "X_ZYXEL_AccessDescription", json_object_new_string("IfaceReset"));
	}
#endif

	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		if(defaultRtModify == true){
			printf("%s: set default route is required\n", __FUNCTION__);
		}

		if(new_activeDefaultGt || defaultRtModify) {
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER, &rtIid, "apply", json_object_new_boolean(true));
			if(new_activeDefaultGt)
				free(new_activeDefaultGt);
		}else
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER, &rtIid, "apply", json_object_new_boolean(false));

		json_object_put(ipAddrMultiJobj);
	}
	else{
		if(dnsSrvJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, dnsSrvJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS.Client.Server.%d Fail\n", __FUNCTION__, dnsSrvIid.idx[0]);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS.Client.Server.%d Success\n", __FUNCTION__, dnsSrvIid.idx[0]);
			}
			json_object_put(dnsSrvJobj);
		}
#ifdef IPV6INTERFACE_PROFILE	
		if(dnsSrvV6Jobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, dnsSrvV6Jobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS.Client.Server.%d Fail\n", __FUNCTION__, dnsSrvV6Iid.idx[0]);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS.Client.Server.%d Success\n", __FUNCTION__, dnsSrvV6Iid.idx[0]);
			}
			json_object_put(dnsSrvV6Jobj);
		}
#endif
		if(dnsJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS Success\n", __FUNCTION__);
			}
			json_object_put(dnsJobj);
		}

		if(igmpJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.X_ZYXEL_IGMP Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.X_ZYXEL_IGMP Success\n", __FUNCTION__);
			}
			json_object_put(igmpJobj);
		}

		if(mldJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.X_ZYXEL_MLD Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.X_ZYXEL_MLD Success\n", __FUNCTION__);
			}
			json_object_put(mldJobj);
		}
		
		if(vlanTermJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Ethernet.VLANTermination.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Ethernet.VLANTermination.i Success\n", __FUNCTION__);
			}
			json_object_put(vlanTermJobj);
		}
		
		if(ipIfaceJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.IP.Interface.i Success\n", __FUNCTION__);
			}
			json_object_put(ipIfaceJobj);
		}
		
		if(natIntfJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_INTF_SETTING, &natIntfIid, natIntfJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.NAT.InterfaceSetting.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.NAT.InterfaceSetting.i Success\n", __FUNCTION__);
			}
			json_object_put(natIntfJobj);
		}
		
		if(natJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &natIid, natJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.NAT Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.NAT Success\n", __FUNCTION__);
			}
			json_object_put(natJobj);
		}
		
		if(natSessionJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, natSessionJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.NAT.X_ZYXEL_SessionControl Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.NAT.X_ZYXEL_SessionControl Success\n", __FUNCTION__);
			}
			json_object_put(natSessionJobj);
		}

		if(defaultRtModify == true)	{
			if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V4_FWD, &rtIpIid, rtIpJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Router.Routing.i.IP.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Router.Routing.i.IP.i Success\n", __FUNCTION__);
			}
		}
		json_object_put(rtIpJobj);

		if(new_activeDefaultGt) {
			if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER, &rtIid, rtJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Router.Routing. Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Router.Routing Success\n", __FUNCTION__);
			}
			free(new_activeDefaultGt);
		}
		json_object_put(rtJobj);
		
	}

	return ret;
}

#ifdef WAN_ADD_INDIVIDUAL
enum {
	WAN_IPCONN_EINTELVLIP,
	WAN_IPCONN_ATMIP
};
#endif
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
 *
 *   Related object in TR181:
 */
zcfgRet_t WANIpConnObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char wanIntfObj[128] = {0};
	char wanLinkObj[128] = {0};
	char wanIpConnObj[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ethLinkPathName[32] = {0};
	char vlanTermPathName[32] = {0};
	char ipIfacePathName[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	uint32_t  wanLinkOid = 0;
	objIndex_t wanLinkIid;
	char *ptr = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	rdm_AtmLink_t *atmLinkObj = NULL;
#endif
#ifdef WAN_ADD_INDIVIDUAL
	int wanIpConnType = WAN_IPCONN_EINTELVLIP;
	char *wanLinkName = NULL;
#else
	char higherLayer[64] = {0};
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	rdm_PtmLink_t *ptmLinkObj = NULL;
#endif
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION
    int WanType = 3;   //1:eth,2:ATM,3:PTM
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANConnectionDevice");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanIntfObj)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef ZCFG_DYNAMIC_NAMEMAPPING
	{
		char tr181MappingName[128] = {0};
		zcfgFe98To181ObjMappingNameGet(tr98TmpName, tr181MappingName);
		zcfgLog(ZCFG_LOG_INFO, "%s : tr181 mapping, %s\n", __FUNCTION__, tr181MappingName);
	}
#endif
	/* wanIntfObj will be DSL.Channel.i or Ethernet.Interface.i */
	/* Check it's DSL or Ethernet first */
	if(strstr(wanIntfObj, "DSL") != NULL) { // DSL WAN
		ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
		ptr = strstr(tr98TmpName, ".WANIPConnection");
		*ptr = '\0';

		/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
		if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanLinkObj)) != ZCFG_SUCCESS) {
			return ret;
		}
			
		/* wanLinkObj will be ATM.Link or PTM.Link */
		IID_INIT(wanLinkIid);
		sprintf(tr181Obj, "Device.%s", wanLinkObj);
		wanLinkOid = zcfgFeObjNameToObjId(tr181Obj, &wanLinkIid);
		if(wanLinkOid == ZCFG_NO_SUCH_OBJECT) {
			printf("%s : can not find OID for %s\n", __func__, tr181Obj);
			return ZCFG_NO_SUCH_OBJECT;
		}
#ifdef CONFIG_TAAAB_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			WanType = 2;
		}
		else
#endif
		if(wanLinkOid == RDM_OID_PTM_LINK) {
			WanType = 3;
		}
#endif

#ifdef WAN_ADD_INDIVIDUAL
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			printf("ATM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_ATM_LINK, &wanLinkIid, (void **)&atmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			if(strcmp(atmLinkObj->LinkType, "EoA") == 0) {
				wanIpConnType = WAN_IPCONN_EINTELVLIP;
			}
			else if(strcmp(atmLinkObj->LinkType, "IPoA") == 0) {
				wanIpConnType = WAN_IPCONN_ATMIP;
			}
			else {
				printf("ATM Link type %s not supported\n", atmLinkObj->LinkType);
				zcfgFeObjStructFree(atmLinkObj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeObjStructFree(atmLinkObj);
		}
#endif // CONFIG_ZCFG_BE_MODULE_ATM_LINK
		wanLinkName = wanLinkObj;
	}
	else if(strstr(wanIntfObj, "Ethernet") != NULL) { // Ethernet WAN
#ifdef CONFIG_TAAAB_CUSTOMIZATION
		WanType = 1;
#endif
		wanIpConnType = WAN_IPCONN_EINTELVLIP;
		wanLinkName = wanIntfObj;
	}
	else if(strstr(wanIntfObj, "Optical") != NULL) { // Optical WAN
		wanIpConnType = WAN_IPCONN_EINTELVLIP;
		wanLinkName = wanIntfObj;
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	if(wanIpConnType == WAN_IPCONN_EINTELVLIP) {
		struct json_object *multiObj = json_object_new_object();

#ifdef CONFIG_TAAAB_CUSTOMIZATION
		wanLinkName = wanLinkObj;
		
		if(strstr(wanIntfObj, "Ethernet") != NULL || strstr(wanIntfObj, "Optical") != NULL){//ETH or PON
			ZOS_STRCPY_M(ethLinkPathName, wanLinkName);
		}
		else if(strstr(wanIntfObj, "DSL") != NULL && wanLinkOid == RDM_OID_ETH_LINK){//PTM
			ZOS_STRCPY_M(ethLinkPathName, wanLinkName);
		}
		else{//ATM
			if((ret = multiObjEthLinkAdd(ethLinkPathName, wanLinkName, multiObj)) != ZCFG_SUCCESS){
				json_object_put(multiObj);
				return ret;
			}
		}
		
#else
		/*Add Device.Ethernet.Link.i*/
		if((ret = multiObjEthLinkAdd(ethLinkPathName, wanLinkName, multiObj)) != ZCFG_SUCCESS){
			json_object_put(multiObj);
			return ret;
		}
#endif

		/*add Device.Ethernet.VLANTermination.i*/
		if((ret = multiObjVlanTermAdd(vlanTermPathName, ethLinkPathName, multiObj)) != ZCFG_SUCCESS){
			json_object_put(multiObj);
			return ret;
		}

		/*Add Device.IP.Interface.i*/
#ifdef CONFIG_TAAAB_CUSTOMIZATION
		if((ret = multiObjIpIfaceAdd(ipIfacePathName, vlanTermPathName, multiObj, WanType)) != ZCFG_SUCCESS){
#else
		if((ret = multiObjIpIfaceAdd(ipIfacePathName, vlanTermPathName, multiObj)) != ZCFG_SUCCESS){
#endif
			json_object_put(multiObj);
			return ret;
		}

		zcfgFeJsonMultiObjSetBlock(multiObj);
		zcfgFeJsonMultiObjSetDelayApply(multiObj);

		ret = zcfgFeMultiObjJsonSet(multiObj, NULL);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			json_object_put(multiObj);
			return ret;
		}
		json_object_put(multiObj);

	}
	else if(wanIpConnType == WAN_IPCONN_ATMIP) {
		printf("ATM Link type is IPoA\n");
		/*Add Device.IP.Interface.i ?*/
		ret = ipIfaceAdd(ipIfacePathName, wanLinkName);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			return ret;
		}

	}

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	if(ret == ZCFG_SUCCESS) {
		if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
			return ret;
		}
	}

	sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
#else
	{
		zcfg_name_t nameSeqNum = 0;
		objIndex_t tr98ObjIid;

		IID_INIT(tr98ObjIid);
		zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)ipIfacePathName, &nameSeqNum, &tr98ObjIid);

		if(IS_SEQNUM_RIGHT(nameSeqNum)) {
			printf("Added WANIPConnection %d\n", tr98ObjIid.idx[2]);
			*idx = tr98ObjIid.idx[2];
		}
	}
#endif

#else

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			printf("ATM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_ATM_LINK, &wanLinkIid, (void **)&atmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			/*Try to get the higher layer of ATM.Link*/
			ret = zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, higherLayer);
			if(ret == ZCFG_NO_SUCH_OBJECT) {
				if(strcmp(atmLinkObj->LinkType, "EoA") == 0) {
					/*Add Device.Ethernet.Link.i*/
					ret = ethLinkAdd(ethLinkPathName, wanLinkObj);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					/*add Device.Ethernet.VLANTermination.i*/
					ret = vlanTermAdd(vlanTermPathName, ethLinkPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					/*Add Device.IP.Interface.i*/
					ret = ipIfaceAdd(ipIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) { 
						if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
				}
				else if(strcmp(atmLinkObj->LinkType, "IPoA") == 0) {
					printf("ATM Link type is IPoA\n");
					/*Add Device.IP.Interface.i ?*/
					ret = ipIfaceAdd(ipIfacePathName, wanLinkObj);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) { 
						if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
				}
				else if(strcmp(atmLinkObj->LinkType, "PPPoA") == 0) {
					printf("ATM Link type is PPPoA\n");
					/*Add Device.PPP.Interface.i and Device.IP.Interface.i ?*/
				}
				else {
					printf("ATM Link type %s not supported\n", atmLinkObj->LinkType);
				}
			}
			else {
				/*Ethernet.Link already exists*/
				if(strstr(higherLayer, "Ethernet.Link") != NULL) {

					printf("Ethernet.Link already exists\n");

					if(zcfgFeTr181IfaceStackHigherLayerGet(higherLayer, vlanTermPathName) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(atmLinkObj);
						return ZCFG_INTERNAL_ERROR;
					}

					if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
						ret = vlanTermAdd(vlanTermPathName, higherLayer);
						if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}
					
					/*Add Device.IP.Interface.i*/
					ret = ipIfaceAdd(ipIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
				}
				else{
					printf("%s : Unknown HigherLayers of ATM.Link\n", __FUNCTION__);
				}
			}
			zcfgFeObjStructFree(atmLinkObj);
		}
		else
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
		if(wanLinkOid == RDM_OID_PTM_LINK) {
			printf("PTM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_PTM_LINK, &wanLinkIid, (void **)&ptmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			/*Try to get the higher layer of PTM.Link*/
			if(zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, higherLayer) != ZCFG_SUCCESS) {
				zcfgFeObjStructFree(ptmLinkObj);
				return ZCFG_INTERNAL_ERROR;
			}

			if(ret == ZCFG_NO_SUCH_OBJECT) {
				/*Add Device.Ethernet.Link.i*/
				ret = ethLinkAdd(ethLinkPathName, wanLinkObj);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					return ret;
				}

				/*add Device.Ethernet.VLANTermination.i*/
				ret = vlanTermAdd(vlanTermPathName, ethLinkPathName);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					return ret;
				}

				/*Add Device.IP.Interface.i*/
				ret = ipIfaceAdd(ipIfacePathName, vlanTermPathName);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					printf("%s : Add Device.IP.Interface.i fail\n", __FUNCTION__);
					return ret;
				}

				if(ret == ZCFG_SUCCESS) {
					if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ret;
					}
				}

				sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
			}
			else {
				/*Ethernet.Link already exists*/
				if(strstr(higherLayer, "Ethernet.Link") != NULL) {

					printf("Ethernet.Link already exists\n");

					if(zcfgFeTr181IfaceStackHigherLayerGet(higherLayer, vlanTermPathName) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ZCFG_INTERNAL_ERROR;
					}

					if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
						ret = vlanTermAdd(vlanTermPathName, higherLayer);
						if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
							zcfgFeObjStructFree(ptmLinkObj);
							return ret;
						}
					}
					printf("Add IP.Interface: %s\n", vlanTermPathName);
					/*Add Device.IP.Interface.i*/
					ret = ipIfaceAdd(ipIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(ptmLinkObj);
							return ret;
						}
					}

					sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
				}
				else{
					printf("%s : Unknown HigherLayers of PTM.Link\n", __FUNCTION__);
				}	
			}

			zcfgFeObjStructFree(ptmLinkObj);
		}
		else
#endif // CONFIG_ZCFG_BE_MODULE_PTM_LINK 
		{
			printf("%s : Unknown Link Type\n", __FUNCTION__);
		}
	}
	else if(strstr(wanIntfObj, "Ethernet") != NULL) { // Ethernet WAN
		printf("Ethernet WAN\n");
		/*Try to get the higher layer of Ethernet.Interface*/
		ret = zcfgFeTr181IfaceStackHigherLayerGet(wanIntfObj, wanLinkObj);
		if(ret == ZCFG_NO_SUCH_OBJECT) {
			/*It should not happen here*/
			return ret;
		}

		if(zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, vlanTermPathName) != ZCFG_SUCCESS) {
			return ZCFG_INTERNAL_ERROR;
		}

		if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
			ret = vlanTermAdd(vlanTermPathName, wanLinkObj);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				return ret;
			}
		}
					
		/*Add Device.IP.Interface.i*/
		ret = ipIfaceAdd(ipIfacePathName, vlanTermPathName);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			return ret;
		}

		if(ret == ZCFG_SUCCESS) {
			if((ret = zcfgFe181To98MappingNameGet(ipIfacePathName, wanIpConnObj)) != ZCFG_SUCCESS) {
				return ret;
			}
		}

		sscanf(wanIpConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%d", idx);
	}
#endif

	return ret;
}

#ifdef WAN_ADD_INDIVIDUAL
static zcfgRet_t WANIpPppConnObjDelEther(const char *layerName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vlanTermIid, etherLinkIid;
	struct json_object *vlanTermObj = NULL;
	const char *lowerLayer = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	if(strstr(layerName, "VLANTerm")) {
		//it is Ethernet.VLANTermination
		IID_INIT(vlanTermIid);
		sscanf(layerName, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
		vlanTermIid.level = 1;
	}
	else if(strstr(layerName, "ATM")) {
		printf("%s : lowerlayer is ATM link\n", __FUNCTION__);
		return ZCFG_SUCCESS;
	}
	else {
		printf("%s : lowerlayer is unknown\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermObj) != ZCFG_SUCCESS) {
		printf("%s : retrieve vlanTerm fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	lowerLayer = NULL;
	lowerLayer = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
	if(strstr(lowerLayer, "Ethernet.Link")) {
		//it is Ethernet.Link

		printf("%s : vlanTerm lowerlayer %s\n", __FUNCTION__, lowerLayer);

		IID_INIT(etherLinkIid);
		sscanf(lowerLayer, "Ethernet.Link.%hhu", &etherLinkIid.idx[0]);
		etherLinkIid.level = 1;

		if((ret = zcfgFeObjJsonDel(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, NULL)) != ZCFG_SUCCESS) {
			printf("%s : Delete vlanTerm Fail\n", __FUNCTION__);
		}

		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			json_object_put(vlanTermObj);
			return ret;
		}
	}
	else {
		printf("%s : vlanTerm lowerlayer is unknown\n", __FUNCTION__);
		json_object_put(vlanTermObj);
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_put(vlanTermObj);

#ifdef CONFIG_TAAAB_CUSTOMIZATION
	struct json_object *ethLinkObj = NULL;
	if(zcfgFeObjJsonGet(RDM_OID_ETH_LINK, &etherLinkIid, &ethLinkObj) != ZCFG_SUCCESS) {
		printf("%s : retrieve ethLinkObj fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	lowerLayer = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
	if(lowerLayer != NULL && strstr(lowerLayer, "ATM.Link.") != NULL){
	if((ret = zcfgFeObjJsonDel(RDM_OID_ETH_LINK, &etherLinkIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s : Delete ether link Fail\n", __FUNCTION__);
	}
	}
	json_object_put(ethLinkObj);
#else
	if((ret = zcfgFeObjJsonDel(RDM_OID_ETH_LINK, &etherLinkIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s : Delete ether link Fail\n", __FUNCTION__);
	}
#endif

	return ret;
}
#endif

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
 *
 *   Related object in TR181:
 */
zcfgRet_t WANIpConnObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	objIndex_t ipIfaceIid;
#ifdef WAN_ADD_INDIVIDUAL
	struct json_object *ipIfaceObj = NULL;
	const char *lowerLayer = NULL;
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipIfaceIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
	ipIfaceIid.level = 1;

#ifdef WAN_ADD_INDIVIDUAL
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceObj)) != ZCFG_SUCCESS) {
		printf("%s : Retreieve IP Interface Fail\n", __FUNCTION__);
		return ret;
	}
#endif

	ret = zcfgFeObjJsonDel(RDM_OID_IP_IFACE, &ipIfaceIid, NULL);
	if(ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete Object Fail\n", __FUNCTION__);	
	}

#ifdef WAN_ADD_INDIVIDUAL
	lowerLayer = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
	if(!lowerLayer) {
		printf("%s : ipIface lowerlayer is none\n", __FUNCTION__);
		json_object_put(ipIfaceObj);
		return ZCFG_INTERNAL_ERROR;
	}
	printf("%s : ipIface lowerlayer %s\n", __FUNCTION__, lowerLayer);

	ret = WANIpPppConnObjDelEther(lowerLayer);

	json_object_put(ipIfaceObj);

#endif

	return ret;
}

zcfgRet_t WANIpConnObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		if(strcmp(tr181ParamName, "IPAddress") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ExternalIPAddress");
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANIpConnObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
/*
	char tr181Obj[128] = {0};
	uint32_t  devInfoOid = 0, ipv4Oid = 0;
	objIndex_t devInfoIid, ipv4Iid;
*/
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
/*
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	
	IID_INIT(ipv4Iid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s.IPv4Address.1", mapObj);
	ipv4Oid = zcfgFeObjNameToObjId(tr181Obj, &ipv4Iid);
*/
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/
		if(!strcmp(paramList->name, "ExternalIPAddress")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE_V4_ADDR, "IPAddress");
		}
		else if(!strcmp(paramList->name, "SubnetMask") || !strcmp(paramList->name, "AddressingType")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE_V4_ADDR, paramList->name);
		}
		else if(!strcmp(paramList->name, "DNSEnabled") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "X_ZYXEL_Type");
		}
		else if(!strcmp(paramList->name, "DNSServers") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "DNSServer");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ZY_IGMP, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ZY_MLD, "Enable");
		}
		else if(!strcmp(paramList->name, "NATEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_INTF_SETTING, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_INTF_SETTING, "X_ZYXEL_FullConeEnabled");
		}
		else if(!strcmp(paramList->name, "DefaultGateway")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ROUTING_ROUTER_V4_FWD, "GatewayIPAddress");
		}
		else if(!strcmp(paramList->name, "Name")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "X_ZYXEL_SrvName");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6Enable");
		}
		else if(!strcmp(paramList->name, "ConnectionType")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "X_ZYXEL_ConnectionType");
		}
		else if(!strcmp(paramList->name, "Uptime")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "LastChange");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6PrefixNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6AddressNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")){
			//attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6Status");
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "Status");
		}
		else if(!strcmp(paramList->name, "ConnectionStatus")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "Status");
		}
		else if(!strcmp(paramList->name, "PossibleConnectionTypes")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "X_ZYXEL_PossibleConnectionTypes");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANEnable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANPriority");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "VLANID");
		}
#ifndef MSTC_TAAAG_MULTI_USER
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") || !strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") || 
#else
		else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif
				!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
				!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_SESSION_CTL, "MaxSessionPerHost");
		}
		else{
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANIpConnObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;
	uint32_t rdmOid = 0;
	char attrParamName[100] = {0};
	char *attrParamNamePtr = NULL;
	char *paramNamePtr = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		paramNamePtr = paramName;

		break;

	} /*Edn while*/

	if(paramNamePtr) {
		/*special case*/
		if(!strcmp(paramList->name, "ExternalIPAddress")){
			rdmOid = RDM_OID_IP_IFACE_V4_ADDR;
			ZOS_STRCPY_M(attrParamName, "IPAddress");
		}
		else if(!strcmp(paramList->name, "SubnetMask") || !strcmp(paramList->name, "AddressingType")){
			rdmOid = RDM_OID_IP_IFACE_V4_ADDR;
		}
		else if(!strcmp(paramList->name, "DNSEnabled") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled")){
			rdmOid = RDM_OID_DNS_CLIENT_SRV;
			ZOS_STRCPY_M(attrParamName, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
			rdmOid = RDM_OID_DNS_CLIENT_SRV;
			ZOS_STRCPY_M(attrParamName, "X_ZYXEL_Type");
		}
		else if(!strcmp(paramList->name, "DNSServers") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers")){
			rdmOid = RDM_OID_DNS_CLIENT_SRV;
			ZOS_STRCPY_M(attrParamName, "DNSServer");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")){
			rdmOid = RDM_OID_ZY_IGMP;
			ZOS_STRCPY_M(attrParamName, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")){
			rdmOid = RDM_OID_ZY_MLD;
			strncpy(attrParamName, "Enable", sizeof(attrParamName)-1);
		}
		else if(!strcmp(paramList->name, "NATEnabled")){
			rdmOid = RDM_OID_NAT_INTF_SETTING;
			ZOS_STRCPY_M(attrParamName, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")){
			rdmOid = RDM_OID_NAT_INTF_SETTING;
			ZOS_STRCPY_M(attrParamName, "X_ZYXEL_FullConeEnabled");
		}
		else if(!strcmp(paramList->name, "DefaultGateway")){
			rdmOid = RDM_OID_ROUTING_ROUTER_V4_FWD;
			ZOS_STRCPY_M(attrParamName, "GatewayIPAddress");
		}
		else if(!strcmp(paramList->name, "Name")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "X_ZYXEL_SrvName");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "IPv6Enable");
		}
		else if(!strcmp(paramList->name, "ConnectionType")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "X_ZYXEL_ConnectionType");
		}
		else if(!strcmp(paramList->name, "Uptime")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "LastChange");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "IPv6PrefixNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "IPv6AddressNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")){
			//attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6Status");
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "Status");
		}
		else if(!strcmp(paramList->name, "ConnectionStatus")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "Status");
		}
		else if(!strcmp(paramList->name, "PossibleConnectionTypes")){
			rdmOid = RDM_OID_IP_IFACE;
			ZOS_STRCPY_M(attrParamName, "X_ZYXEL_PossibleConnectionTypes");
		}
		else if(!strcmp(paramList->name, "LastConnectionError") || !strcmp(paramList->name, "AutoDisconnectTime") || !strcmp(paramList->name, "IdleDisconnectTime")
				|| !strcmp(paramList->name, "WarnDisconnectDelay") || !strcmp(paramList->name, "RSIPAvailable") || !strcmp(paramList->name, "DNSOverrideAllowed")
					|| !strcmp(paramList->name, "MACAddress") || !strcmp(paramList->name, "MACAddressOverride") || !strcmp(paramList->name, "ConnectionTrigger")
				|| !strcmp(paramList->name, "RouteProtocolRx") || !strcmp(paramList->name, "ShapingRate") || !strcmp(paramList->name, "ShapingBurstSize")
					|| !strcmp(paramList->name, "PortMappingNumberOfEntries")){
			return ZCFG_SUCCESS;
		}
#ifndef MSTC_TAAAG_MULTI_USER					
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") || !strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") || 
#else
		else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif
				!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
				!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
			rdmOid = RDM_OID_NAT;
			ZOS_STRCPY_M(attrParamName, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
			rdmOid = RDM_OID_NAT_SESSION_CTL;
			ZOS_STRCPY_M(attrParamName, "MaxSessionPerHost");
		}
		else {
			rdmOid = RDM_OID_IP_IFACE;
		}
	}
	else {
		return ZCFG_INVALID_ARGUMENTS;
	}

	if(rdmOid) {
		if(strlen(attrParamName) > 0)
			attrParamNamePtr = attrParamName;
		else
			attrParamNamePtr = paramName;

		attrValue = zcfgFeParamAttrGetByName(rdmOid, attrParamNamePtr);
		if(attrValue < 0) {
			return ZCFG_INVALID_ARGUMENTS;
		}
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(rdmOid, multiAttrJobj, attrParamNamePtr, attrValue);
		if(ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, rdmOid, attrParamNamePtr);
		}
	}
	else {
		return ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient
 *
 *   Related object in TR181:
 *   Device.DHCPv4.Client.i
 */
zcfgRet_t DhcpClientObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	rdm_Dhcpv4Client_t *dhcpcObj = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".DHCPClient");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(objIid);
	while((ret = feObjStructGetNext(RDM_OID_DHCPV4_CLIENT, &objIid, (void **)&dhcpcObj, updateFlag)) == ZCFG_SUCCESS) {
		if(!strcmp(dhcpcObj->Interface, ipIface)) {
			*tr98Jobj = json_object_new_object();
			json_object_object_add(*tr98Jobj, "SentDHCPOptionNumberOfEntries", json_object_new_int(dhcpcObj->SentOptionNumberOfEntries));
			json_object_object_add(*tr98Jobj, "ReqDHCPOptionNumberOfEntries", json_object_new_int(dhcpcObj->ReqOptionNumberOfEntries));
			json_object_object_add(*tr98Jobj, "X_ZYXEL_RetryCount", json_object_new_int(dhcpcObj->X_ZYXEL_RetryCount));
			zcfgFeObjStructFree(dhcpcObj);
			break;
		}
		
		zcfgFeObjStructFree(dhcpcObj);
	}

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient
 *
 *   Related object in TR181:
 *   Device.DHCPv4.Client.i
 */
zcfgRet_t DhcpClientObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj )
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	objIndex_t dhcpcIid = {0};
	struct json_object *dhcpcObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	bool found = false;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	strncpy(tr98TmpName, tr98FullPathName, sizeof(tr98TmpName)-1);
	ptr = strstr(tr98TmpName, ".DHCPClient");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(dhcpcIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV4_CLIENT, &dhcpcIid, &dhcpcObj)) == ZCFG_SUCCESS) {
		if(strcmp(ipIface, json_object_get_string(json_object_object_get(dhcpcObj, "Interface")))==0) {
			found = true;
			break;
		}
		else
			json_object_put(dhcpcObj);
	}

	if(!found)
		return ret;

	if(multiJobj) {
		tmpObj = dhcpcObj;
		dhcpcObj = NULL;
		dhcpcObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_CLIENT, &dhcpcIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.DHCPv4.Client.i*/
			tr181ParamValue = json_object_object_get(dhcpcObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(dhcpcObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;
	}

	/*Set Device.DHCPv4.Client.i*/
	if(multiJobj) {
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_CLIENT, &dhcpcIid, dhcpcObj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Client.%d Fail\n", __FUNCTION__, dhcpcIid.idx[0]);
			json_object_put(dhcpcObj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv4.Client.%d Fail\n", __FUNCTION__, dhcpcIid.idx[0]);
		}
		json_object_put(dhcpcObj);
	}

	return ret;
}

zcfgRet_t DhcpClientObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	char *attrParamNamePtr = NULL;
	char attrParamName[100] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	if(!attrParamNamePtr) {
		attrParamNamePtr = attrParamName;
		if(!strcmp(tr181ParamName, "SentOptionNumberOfEntries")) {
			ZOS_STRCPY_M(attrParamNamePtr, "SentDHCPOptionNumberOfEntries");
		}
		else if(!strcmp(tr181ParamName, "ReqOptionNumberOfEntries")) {
			ZOS_STRCPY_M(attrParamNamePtr, "ReqDHCPOptionNumberOfEntries");
		}
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);

		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int DhcpClientObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if(!strcmp(paramList->name, "SentDHCPOptionNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_CLIENT, "SentOptionNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "ReqDHCPOptionNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_CLIENT, "ReqOptionNumberOfEntries");
		}

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t DhcpClientObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;
	char attrParamName[100] = {0};

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if(!strcmp(paramName, "SentDHCPOptionNumberOfEntries")){
			ZOS_STRCPY_M(attrParamName, "SentOptionNumberOfEntries");
		}
		else if(!strcmp(paramName, "ReqDHCPOptionNumberOfEntries")){
			ZOS_STRCPY_M(attrParamName, "ReqOptionNumberOfEntries");
		}
		attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_CLIENT, attrParamName);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RDM_OID_DHCPV4_CLIENT, multiAttrJobj, attrParamName, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RDM_OID_DHCPV4_CLIENT, attrParamName);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*
 *   InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient.SentDHCPOption.i.
 *
 *   Related object in TR181:
 *   Device.DHCPv4.Client.i.SentOption.i
 */
zcfgRet_t DhcpClientSentOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	struct json_object *optObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/* get parent DHCP client */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	/* obtain index of dhcp option */
	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.SentDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if(optIid.idx[optIid.level - 1] == 0){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to obtain index of DHCPClient.SentDHCPOption.i\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}

	/* get child DHCP client sent options */
	if((ret = feObjJsonGet(RDM_OID_DHCPV4_SENT_OPT, &optIid, &optObj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		uint32_t paramTag=0;
		/* get parameter value from tr181 */
		paramValue = json_object_object_get(optObj, paramList->name);
        paramTag = json_object_get_int(json_object_object_get(optObj, "Tag"));
		/* write it to tr098 json object */
		if(paramValue != NULL) {
			if((paramTag == 60) && (!strcmp(paramList->name, "Value"))) {
				const char *paramValueStr = NULL;
				char tmpbuf[254] = {0};
				char *b = tmpbuf;
				const char *p;
				char *encodeStr = NULL;
				int c;

				paramValueStr = json_object_get_string(paramValue);

				for ( paramValueStr += 4 ; paramValueStr[0] && paramValueStr[1] && sscanf(paramValueStr, "%2x", &c) ; paramValueStr += 2){
					b += sprintf(b, "%c", c);
				} 
                *b = 0;
				
				//encodeStr = base64_encode(tmpbuf, strlen(tmpbuf));
				//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(encodeStr));
				//free(encodeStr);
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tmpbuf));
			}
			else if(!strcmp(paramList->name, "Value")){
#if 0
				const char *paramValueStr = NULL;
				char *encodeStr = NULL;
				
				paramValueStr = json_object_get_string(paramValue);
				encodeStr = base64_encode(paramValueStr, strlen(paramValueStr));
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(encodeStr));
				free(encodeStr);
#endif
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
		}else if(!strcmp(paramList->name, "Enable")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
		}else if(!strcmp(paramList->name, "Tag")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(0));
		}else if(!strcmp(paramList->name, "Value")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(NULL));
		}
		paramList++;
	}
	zcfgFeJsonObjFree(optObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t DhcpClientSentOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	struct json_object *optObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/* get parent DHCP client */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	/* obtain index of dhcp option */
	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.SentDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if(optIid.idx[optIid.level - 1] == 0){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to obtain index of DHCPClient.SentDHCPOption.i\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}

	/* get child DHCP client sent options */
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_SENT_OPT, &optIid, &optObj)) == ZCFG_SUCCESS){
		/* if multiple set is required, just appended JSON to multiJobj */
		if(multiJobj){

			tmpObj = optObj;
			optObj = NULL;
			optObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_SENT_OPT, &optIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}

		/* set(update) value to target object */

		if(optObj){
			paramList = tr98Obj[handler].parameter;
			while(paramList->name != NULL) {
				/* get parameter from tr098 */
				paramValue = json_object_object_get(tr98Jobj, paramList->name);
				const char *paramValueStr = NULL;
				uint32_t paramTag=0;
					
				paramTag = json_object_get_int(json_object_object_get(tr98Jobj, "Tag"));						
				
				if ((paramTag == 60) && (!strcmp(paramList->name, "Value")))
				{
					char *decodeStr = NULL;
					int decodeStr_len = 0;
					const char *p;
					char tmpbuf[254] = {0};
					char *b = tmpbuf;
					char dhcpOptStr[256]={0};
					int dhcpOptStrlen = 0;
					
					sprintf(dhcpOptStr, "%02x", paramTag);
					
					paramValueStr = json_object_get_string(paramValue);
					
					if((paramValueStr != NULL) && (paramValueStr[0] != '\0')
						&& (strlen(paramValueStr) != 0) && (paramValueStr[0] != ' '))
					{
						if(json_object_get_type(paramValue) == json_type_base64){
							decodeStr = base64_decode( paramValueStr, &decodeStr_len);
						}else{
							int paramValueStrLen = strlen(paramValueStr);
							decodeStr = malloc(paramValueStrLen+1);
							strncpy(decodeStr, paramValueStr, paramValueStrLen);
							*(decodeStr+paramValueStrLen) = '\0';
							decodeStr_len = paramValueStrLen;
						}

						if(decodeStr_len > 125){
							free(decodeStr);
							return ZCFG_INVALID_PARAM_VALUE;
						}

						if ((decodeStr != NULL) && (decodeStr != '\0') && 
							 (decodeStr_len != 0))
						{
							//convert to hex String
							for(p=decodeStr; *p; ++p)
								b += sprintf(b, "%02x", (const int)*p);
							*b = 0;
								
							dhcpOptStrlen = strlen(tmpbuf);
								
							if (dhcpOptStrlen > 1)
								dhcpOptStrlen = dhcpOptStrlen/2;
						}
					}
					char hexStrlen[4]={0} ;
					//prefix length of 2 digits hex numbers
					sprintf(hexStrlen, "%02d",dhcpOptStrlen);
					
					//append prefix as option60 hex string+length of 2 digits hex numbers
					strcat(dhcpOptStr, hexStrlen);
					
					//append prefix as option60 hexdec+length of 2 digits hex numbers+hex string
					strcat(dhcpOptStr, tmpbuf);
					/* write it to tr181 json object */
					if((dhcpOptStr != NULL) && (!strcmp(paramList->name, "Value"))) {	
					  json_object_object_add(optObj, paramList->name, json_object_new_string(dhcpOptStr));
					}
					if(decodeStr){
						free(decodeStr);
					}
				}
				else if (!strcmp(paramList->name, "Value"))
				{
					char *decodeStr = NULL;
					int decodeStr_len = 0;
					
					paramValueStr = json_object_get_string(paramValue);
					
					if((json_object_get_type(paramValue) == json_type_base64) && (paramValueStr != NULL) && (paramValueStr[0] != '\0')
						&& (strlen(paramValueStr) != 0) && (paramValueStr[0] != ' '))
					{
						decodeStr = base64_decode( paramValueStr, &decodeStr_len);
						json_object_object_add(optObj, paramList->name, json_object_new_string(decodeStr));
						free(decodeStr);
					}else{
						json_object_object_add(optObj, paramList->name, JSON_OBJ_COPY(paramValue));
					}
				}
				else if(paramValue != NULL) {
					json_object_object_add(optObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
				
#if 0
				/* write it to tr181 json object */
				if(paramValue != NULL) {
					json_object_object_add(optObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
#endif
				paramList++;
			}
		}
#if 0
		if(multiJobj){

			char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
			char wanIntfName[100] = {0}, *ptr = NULL;
			struct json_object *wanLinkObj = NULL;
			objIndex_t wanLinkObjIndex;
			zcfg_offset_t linkOid = 0;
			int applied = 0;


			//Retrieve tr181 link with InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i
			ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
			ptr = strstr(tr98TmpName, ".WANConnectionDevice");
			*ptr = '\0';

			/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */

			//Retrieve tr181 link with InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
			ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
			ptr = strstr(tr98TmpName, ".DHCPClient");
			*ptr = '\0';

			/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANIPConnection.i */
			if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanIntfName)) != ZCFG_SUCCESS) {
				zcfgFeJsonObjFree(optObj);
				return ret;
			}
			printf("%s: %s relink\n", __FUNCTION__, wanIntfName);

			paramValue = NULL;
			//if !linkOid, wanLinkObj is NULL
			linkOid = zcfgFeTr181WanSetLink(wanIntfName, false, (void **)&wanLinkObj, &wanLinkObjIndex, &applied);
			if(linkOid && applied) {
				if((paramValue = json_object_object_get(wanLinkObj, "Enable"))) {
					json_object_object_add(wanLinkObj, "Enable", json_object_new_boolean(true));
					tmpObj = wanLinkObj;
					wanLinkObj = zcfgFeJsonMultiObjAppend(linkOid, &wanLinkObjIndex, multiJobj, tmpObj);
					json_object_put(tmpObj);
				}else {
					zcfgFeJsonObjFree(wanLinkObj);
					zcfgFeJsonObjFree(optObj);
					return ZCFG_INTERNAL_ERROR;
				}
			}else {
				if(wanLinkObj) zcfgFeJsonObjFree(wanLinkObj);

				if(!linkOid) {
					zcfgFeJsonObjFree(optObj);
					return ZCFG_INTERNAL_ERROR;
				}
			}
		}
#endif
		/* is not multiple set, jsut set object immediately after update parameter value */
		if(!multiJobj){
			ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SENT_OPT, &optIid, optObj, NULL);
 			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_DHCPV4_SENT_OPT object Fail with ret=%d\n", __FUNCTION__, ret);
				zcfgFeJsonObjFree(optObj);
				return ret;
			}
			zcfgFeJsonObjFree(optObj);
		}
	}
	else{
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t DhcpClientSentOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;

	/* get parent Iid */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	memcpy(&optIid, &dhcpcIid, sizeof(dhcpcIid));
	
	/* add child under get parent */
	if((ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_SENT_OPT, &optIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add dhcp client set option with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*idx = optIid.idx[1];

	return ZCFG_SUCCESS;
	
}


zcfgRet_t DhcpClientSentOptObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	
	/* get parent Iid */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.SentDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_SENT_OPT, &optIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete dhcpc sent option with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t DhcpClientOptObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);

		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int DhcpClientSentOptObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_SENT_OPT, paramName);

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t DhcpClientSentOptObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_SENT_OPT, paramName);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RDM_OID_DHCPV4_SENT_OPT, multiAttrJobj, paramName, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RDM_OID_DHCPV4_SENT_OPT, paramName);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*
 *   InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient.ReqDHCPOption.i.
 *
 *   Related object in TR181:
 *   Device.DHCPv4.Client.i.ReqOption.i
 */
zcfgRet_t DhcpClientReqOptObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	struct json_object *optObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/* get parent DHCP client */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	/* obtain index of dhcp option */
	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.ReqDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if(optIid.idx[optIid.level - 1] == 0){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to obtain index of DHCPClient.ReqDHCPOption.i\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}
	
	/* get child DHCP client sent options */
	if((ret = feObjJsonGet(RDM_OID_DHCPV4_REQ_OPT, &optIid, &optObj, updateFlag)) == ZCFG_SUCCESS){
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			/* get parameter value from tr181 */
			paramValue = json_object_object_get(optObj, paramList->name);

			/* write it to tr098 json object */
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
			}
		}
		zcfgFeJsonObjFree(optObj);
	}
	else{
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t DhcpClientReqOptObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	struct json_object *optObj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);

	/* get parent DHCP client */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	/* obtain index of dhcp option */
	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.ReqDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if(optIid.idx[optIid.level - 1] == 0){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to obtain index of DHCPClient.ReqDHCPOption.i\n", __FUNCTION__);
		return ZCFG_INVALID_ARGUMENTS;
	}

	/* get child DHCP client sent options */
	if((ret = zcfgFeObjJsonGet(RDM_OID_DHCPV4_REQ_OPT, &optIid, &optObj)) == ZCFG_SUCCESS){
		/* if multiple set is required, just appended JSON to multiJobj */
		if(multiJobj){
			tmpObj = optObj;
			optObj = NULL;
			optObj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV4_REQ_OPT, &optIid, multiJobj, tmpObj);
			zcfgFeJsonObjFree(tmpObj);
		}

		/* set(update) value to target object */
		if(optObj){
			paramList = tr98Obj[handler].parameter;
			while(paramList->name != NULL) {
				/* get parameter from tr098 */
				paramValue = json_object_object_get(tr98Jobj, paramList->name);

				/* write it to tr181 json object */
				if(paramValue != NULL) {
					json_object_object_add(optObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}

				paramList++;
			}
		}

		/* is not multiple set, jsut set object immediately after update parameter value */
		if(!multiJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_REQ_OPT, &optIid, optObj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set RDM_OID_DHCPV4_SENT_OPT object Fail with ret=%d\n", __FUNCTION__, ret);
				zcfgFeJsonObjFree(optObj);
				return ret;
			}
			zcfgFeJsonObjFree(optObj);
		}
	}
	else{
		return ret;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t DhcpClientReqOptObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;

	/* get parent Iid */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	memcpy(&optIid, &dhcpcIid, sizeof(dhcpcIid));
	
	/* add child under get parent */
	if((ret = zcfgFeObjJsonAdd(RDM_OID_DHCPV4_REQ_OPT, &optIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to add dhcp client req option with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	*idx = optIid.idx[1];

	return ZCFG_SUCCESS;
	
}


zcfgRet_t DhcpClientReqOptObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t dhcpcIid, optIid;
	
	/* get parent Iid */
	if((ret = DhcpcIidGet(tr98FullPathName, &dhcpcIid)) == ZCFG_NOT_FOUND){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to get dhcpc Iid\n", __FUNCTION__);
		return ZCFG_NOT_FOUND;
	}

	memcpy(&optIid, &dhcpcIid, sizeof(objIndex_t));
	optIid.level++;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANIPConnection.%*d.DHCPClient.ReqDHCPOption.%hhu", &optIid.idx[optIid.level - 1]);

	if((ret = zcfgFeObjJsonDel(RDM_OID_DHCPV4_REQ_OPT, &optIid, NULL)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s: fail to delete dhcpc req option with ret=%d\n", __FUNCTION__, ret);
		return ret;
	}

	return ZCFG_SUCCESS;
}

#if 0
zcfgRet_t DhcpClientReqOptObjAttrNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);

		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}
#endif

int DhcpClientReqOptObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_REQ_OPT, paramName);

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t DhcpClientReqOptObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);


	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		attrValue = zcfgFeParamAttrGetByName(RDM_OID_DHCPV4_REQ_OPT, paramName);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(RDM_OID_DHCPV4_REQ_OPT, multiAttrJobj, paramName, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, RDM_OID_DHCPV4_REQ_OPT, paramName);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i
 *
 *   Related object in TR181:
 *   Device.PPP.Interface.i
 *   Device.PPP.Interface.i.PPPoA
 *   Device.PPP.Interface.i.PPPoE
 *   Device.PPP.Interface.i.IPCP
 *   Device.PPP.Interface.i.IPV6CP
 *   Device.IP.Interface.i
 */
zcfgRet_t WANPppConnObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char pppIface[32] = {0}, ipIface[32] = {0};
	tr98Parameter_t *paramList = NULL;
	uint32_t  pppConnOid = 0;
	objIndex_t pppConnIid, ipIfaceIid, vlanTermIid, ethLinkIid;
	objIndex_t dnsSrvIid, natIid, natSessionIid;
	objIndex_t natIntfSetIid;
#ifdef IAAAA_CUSTOMIZATION
	objIndex_t ripIid;
	struct json_object *ripObj = NULL;
#endif
	struct json_object *pppConnJobj = NULL, *pppoeJobj = NULL, *ipcpJobj = NULL, *ipIfaceJobj = NULL;
	struct json_object *ipv6cpJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *dnsSrvJobj = NULL;
	struct json_object *vlanTermJobj = NULL;
	struct json_object *ethLinkJobj = NULL;
	struct json_object *natJobj = NULL;
	struct json_object *natSessionJobj = NULL;
	struct json_object *natIntfSetJobj = NULL;
#ifdef IPV6INTERFACE_PROFILE
	objIndex_t dnsV6SrvIid;
	struct json_object *dnsV6SrvJobj = NULL;
	objIndex_t rtIid;
	struct json_object *rtJobj = NULL;
	const char *v6DefaultGw = NULL;
#endif
	int numOfEntries = 0;
	const char *lowerLayer = NULL;

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
	char currType[10] = {0};
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
    const char *dnstype = NULL;
#endif

	ztrdbg(ZTR_DL_DEBUG, "Enter %s\n", __FUNCTION__);

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, pppIface)) != ZCFG_SUCCESS) {
		/*  The instance number of WANPPPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANPPPConnection object.
		 */
		return ret;
	}

	zcfgLog(ZCFG_LOG_INFO, "PPP Interface object %s\n", pppIface);
	IID_INIT(pppConnIid);
	sprintf(tr181Obj, "Device.%s", pppIface);
	pppConnOid = zcfgFeObjNameToObjId(tr181Obj, &pppConnIid);

	/*Get related tr181 objects*/
	/*Get PPP.Interface.i*/
	if((ret = feObjJsonGet(pppConnOid, &pppConnIid, &pppConnJobj, updateFlag)) != ZCFG_SUCCESS)
		goto pppConnFree;

	/* Get Device.Ethernet.VLANTermination.i */
	lowerLayer = json_object_get_string(json_object_object_get(pppConnJobj, "LowerLayers"));
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
	if(strstr(lowerLayer, "USB.Interface.") != NULL ){
			strncpy(currType, "3G/4G", sizeof(currType));
	}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
	if (lowerLayer && strstr(lowerLayer, "Ethernet.VLANTermination") != NULL){
		IID_INIT(vlanTermIid);
		sscanf(lowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
		vlanTermIid.level = 1;
		if((ret = feObjJsonGet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermJobj, updateFlag)) != ZCFG_SUCCESS)
			goto pppConnFree;
	}

	/* Get Device.Ethernet.Link.i */
	if(vlanTermJobj) {
		lowerLayer = json_object_get_string(json_object_object_get(vlanTermJobj, "LowerLayers"));
		if (lowerLayer && strstr(lowerLayer, "Ethernet.Link.") != NULL){
			IID_INIT(ethLinkIid);
			sscanf(lowerLayer, "Ethernet.Link.%hhu", &ethLinkIid.idx[0]);
			ethLinkIid.level = 1;
			if((ret = feObjJsonGet(RDM_OID_ETH_LINK, &ethLinkIid, &ethLinkJobj, updateFlag)) != ZCFG_SUCCESS)
				goto pppConnFree;
		}
	}

	IID_INIT(natIid);
	if((ret = feObjJsonGet(RDM_OID_NAT, &natIid, &natJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get NAT Fail\n", __FUNCTION__);
		goto pppConnFree;
	}
	
	IID_INIT(natSessionIid);
	if((ret = feObjJsonGet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, &natSessionJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get RDM_OID_NAT_SESSION_CTL Fail\n", __FUNCTION__);
		goto pppConnFree;
	}

	/*Get PPP.Interface.i.PPPoA(gnore now)*/

	/*Get PPP.Interface.i.PPPoE*/
	if((ret = feObjJsonGet(RDM_OID_PPP_IFACE_PPPOE, &pppConnIid, &pppoeJobj, updateFlag)) != ZCFG_SUCCESS)
		goto pppConnFree;

	/*Get PPP.Interface.i.IPCP*/
	if((ret = feObjJsonGet(RDM_OID_PPP_IFACE_IPCP, &pppConnIid, &ipcpJobj, updateFlag)) != ZCFG_SUCCESS)
		goto pppConnFree;

	/*Get PPP.Interface.i.IPV6CP*/
	if((ret = feObjJsonGet(RDM_OID_PPP_INTF_IPV6CP, &pppConnIid, &ipv6cpJobj, updateFlag)) != ZCFG_SUCCESS)
		goto pppConnFree;

	/*Get IP.Interface.i above PPP.Interface.i*/
	IID_INIT(ipIfaceIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj) == ZCFG_SUCCESS){
		if(!strcmp(json_object_get_string(json_object_object_get(ipIfaceJobj, "LowerLayers")), pppIface)){
			sprintf(ipIface, "IP.Interface.%u", ipIfaceIid.idx[0]);
			zcfgFeJsonObjFree(ipIfaceJobj);
			break;
		}
		zcfgFeJsonObjFree(ipIfaceJobj);
	}

	if(!strcmp(ipIface, ""))
		goto pppConnFree;
	
	/*Get IP.Interface.i*/
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj, updateFlag)) != ZCFG_SUCCESS)
		goto pppConnFree;

	IID_INIT(natIntfSetIid);
	if(retrieveJobjWithIpIface((const char *)ipIface, RDM_OID_NAT_INTF_SETTING, &natIntfSetIid, &natIntfSetJobj) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_DEBUG, "%s: retrieve NAT_INTF_SETTING fail\n", __FUNCTION__);
	}
#ifdef IAAAA_CUSTOMIZATION
	getSpecificObj(RDM_OID_ROUTING_RIP_INTF_SET, "Interface", json_type_string, ipIface, &ripIid, &ripObj);
#endif
	/*DNSServer*/
	dnsInfoGetJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false, updateFlag); //dns v4
#ifdef IPV6INTERFACE_PROFILE
	dnsInfoGetJobj(ipIface, &dnsV6SrvIid, (struct json_object **)&dnsV6SrvJobj, true, updateFlag); //dns v6

	/*Get Device.Routing.Router.i*/
	IID_INIT(rtIid);
	rtIid.level = 1;
	rtIid.idx[0] = 1;
	if((ret = feObjJsonGet(RDM_OID_ROUTING_ROUTER, &rtIid, &rtJobj, updateFlag)) != ZCFG_SUCCESS){
		printf("%s_%d: Get RDM_OID_ROUTING_ROUTER Fail\n", __FUNCTION__, __LINE__);
		goto pppConnFree;
	}
#endif

	/*Fill up tr98 WANPPPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/
		paramValue = json_object_object_get(pppConnJobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "Name")) {
				paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_SrvName");
				const char *ipIfaceSrvName = json_object_get_string(paramValue);
				if(ipIfaceSrvName && strlen(ipIfaceSrvName))
					printf("X_ZYXEL_SrvName, %s\n", ipIfaceSrvName);
				else
					printf("X_ZYXEL_SrvName null\n");
			}
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
			else if(!strcmp(paramList->name, "CurrentMRUSize")) {
				paramValue = json_object_object_get(pppConnJobj, "MaxMRUSize");
			}
#endif
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(pppoeJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		paramValue = json_object_object_get(ipcpJobj, paramList->name);
		if(paramValue != NULL) {
			if( !strcmp(paramList->name, "DNSServers")){// have been get before by dnsInfoSet()
				//paramList++;
				//continue;				
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Is IP.Interface.i necessary here?*/

		/*special case*/

		if(!strcmp(paramList->name, "MACAddress")) {
			paramValue = ethLinkJobj ? json_object_object_get(ethLinkJobj, paramList->name) : ethLinkJobj;
			if(paramValue != NULL)
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			else
				zcfgFeTr98DefaultValueSet(*tr98Jobj, "MACAddress", json_type_string);
			paramList++;
			continue;
		}
		else if(strstr(paramList->name, "PPPoESessionID") != NULL) {
			paramValue = json_object_object_get(pppoeJobj, "SessionID");
		}
		else if(!strcmp(paramList->name, "PPPEncryptionProtocol")) {
			paramValue = json_object_object_get(pppConnJobj, "EncryptionProtocol");
		}
		else if(!strcmp(paramList->name, "PPPCompressionProtocol")) {
			paramValue = json_object_object_get(pppConnJobj, "CompressionProtocol");
		}
		else if(!strcmp(paramList->name, "PPPoEACName")) {
			paramValue = json_object_object_get(pppoeJobj, "ACName");
		}
		else if(!strcmp(paramList->name, "PPPoEServiceName")) {
#ifdef ZYXEL_PPPoE_SERVICE_OPTION
            paramValue = json_object_object_get(pppConnJobj, "X_ZYXEL_ServiceName");
#else
			paramValue = json_object_object_get(pppoeJobj, "ServiceName");
#endif
		}
		else if(!strcmp(paramList->name, "PPPLCPEcho")) {
			paramValue = json_object_object_get(pppConnJobj, "LCPEcho");
		}
		else if(!strcmp(paramList->name, "PPPLCPEchoRetry")) {
			paramValue = json_object_object_get(pppConnJobj, "LCPEchoRetry");
		}
		else if(!strcmp(paramList->name, "ExternalIPAddress")) {
			paramValue = json_object_object_get(ipcpJobj, "LocalIPAddress");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_LocalInterfaceIdentifier")) {
			paramValue = json_object_object_get(ipv6cpJobj, "LocalInterfaceIdentifier");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_RemoteInterfaceIdentifier")) {
			paramValue = json_object_object_get(ipv6cpJobj, "RemoteInterfaceIdentifier");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPCPEnable")) {
			paramValue = json_object_object_get(pppConnJobj, "IPCPEnable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6CPEnable")) {
			paramValue = json_object_object_get(pppConnJobj, "IPv6CPEnable");
		}
		else if(!strcmp(paramList->name, "PPPAuthenticationProtocol")) {
			paramValue = json_object_object_get(pppConnJobj, "AuthenticationProtocol");
		}
		else if(!strcmp(paramList->name, "PossibleConnectionTypes")) {
			paramValue = json_object_object_get(pppConnJobj, "X_ZYXEL_PossibleConnectionTypes");
		}
		else if(!strcmp(paramList->name, "ConnectionType")) {
			paramValue = json_object_object_get(pppConnJobj, "X_ZYXEL_ConnectionType");
		}
		else if(!strcmp(paramList->name, "NATEnabled")) {
			natInfoSet(ipIface, *tr98Jobj);
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "Uptime")) {
			paramValue = json_object_object_get(pppConnJobj, "X_ZYXEL_LastConnectionUpTime");
			if(paramValue) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "DefaultGateway")) {
			defaultGwInfoSet(ipIface, *tr98Jobj);
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DefaultGatewayIface")){
			paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6DefaultGatewayIface")) {
			if(rtJobj != NULL) {
				paramValue = json_object_object_get(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway");
				if(paramValue != NULL) {
					v6DefaultGw = json_object_get_string(paramValue);
					if(v6DefaultGw && strstr(v6DefaultGw, ipIface)) {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
					}
					else {
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
					}
				}
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv4Enable")){
			paramValue = json_object_object_get(ipIfaceJobj, "IPv4Enable");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_IPv4Enable", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_ConcurrentWan")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_ConcurrentWan");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_ConcurrentWan", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_Enable_DSLite");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_Enable_DSLite", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_DSLiteRelayIPv6Addresses", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLite_Type");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_DSLite_Type", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE
		else if(!strcmp(paramList->name, "X_ZYXEL_Enable_Firewall")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_Enable_Firewall");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_Enable_Firewall", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
#endif
    	else if(!strcmp(paramList->name, "DNSEnabled")) {
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "Enable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "DNSOverrideAllowed")){
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
						if(dnsSrvJobj != NULL) {
							dnstype = json_object_get_string(json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type"));
							if(!strcmp(dnstype,"Static"))
								json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
							else
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
						}
						else{
							json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
						}
#else
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
#endif
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "DNSServers")){
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "DNSServer");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else{
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
			if(dnsSrvJobj != NULL) {
				paramValue = json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
#ifdef IPV6INTERFACE_PROFILE
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSV6Type")){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "X_ZYXEL_Type");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
#endif
		else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(isIGMPProxyEnable(ipIface)));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(isMLDProxyEnable(ipIface)));

			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "PortMappingNumberOfEntries")){
			numOfEntries = portMappingNumGet(ipIface);
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(numOfEntries));
			paramList++;
			continue;			
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")) {
			if(vlanTermJobj != NULL) {
				paramValue = json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANEnable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}else{
				zcfgFeTr98DefaultValueSet(*tr98Jobj, "X_ZYXEL_VlanEnable", json_type_boolean);
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")) {
			if (vlanTermJobj){
				paramValue = json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANPriority");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}else{
				json_object_object_add(*tr98Jobj, "X_ZYXEL_VlanMux8021p", json_object_new_int(-1));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")) {
			if (vlanTermJobj){
				paramValue = json_object_object_get(vlanTermJobj, "VLANID");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				}
			}else{
				json_object_object_add(*tr98Jobj, "X_ZYXEL_VlanMuxID", json_object_new_int(-1));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")) {
			paramValue = natIntfSetJobj ? json_object_object_get(natIntfSetJobj, paramList->name) : NULL;
		}
#ifndef MSTC_TAAAG_MULTI_USER			
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") ||!strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") || 
#else		
		else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif	
				!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
				!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
			if (natJobj){
				paramValue = json_object_object_get(natJobj, paramList->name);
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")) {
			if (natSessionJobj){
				paramValue = json_object_object_get(natSessionJobj, "MaxSessionPerHost");
				if(paramValue != NULL) {
					json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
#ifdef IPV6RD_PROFILE
#ifndef SAAAD_TR98_CUSTOMIZATION // SAAAD
		else if(!strcmp(paramList->name, "X_ZYXEL_Enable_6RD")) {
			paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_BorderRelayIPv4Addresses")) {
			paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_SPIPv6Prefix")) {
			paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv4MaskLength")) {
			paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		}
#endif
#endif
#ifdef IPV6INTERFACE_PROFILE		
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6Enable");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")) {
			//paramValue = json_object_object_get(ipIfaceJobj, "IPv6Status");
			//json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			if(	json_object_get_boolean(json_object_object_get(ipIfaceJobj, "IPv6Enable")))
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_ENABLED));
			else
				json_object_object_add(*tr98Jobj, paramList->name, 	json_object_new_string(TR181_DISABLED));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6AddressNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")) {
			paramValue = json_object_object_get(ipIfaceJobj, "IPv6PrefixNumberOfEntries");
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "Enable");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(true));
			}
			paramList++;
			continue;
		}
		else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
			if(dnsV6SrvJobj != NULL) {
				paramValue = json_object_object_get(dnsV6SrvJobj, "DNSServer");
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			}
			else {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			}
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_Enable_DSLite");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_Enable_DSLite", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_DSLiteRelayIPv6Addresses", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")){
			paramValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_DSLite_Type");
			json_object_object_add(*tr98Jobj, "X_ZYXEL_DSLite_Type", JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}	
#endif
		else {

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			zcfgFeTr98DefaultValueSet(*tr98Jobj, paramList->name, paramList->type);
			paramList++;
			continue;
		}

		if(paramValue != NULL) {		
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
		}

		paramValue = NULL;
		/*End of special case*/
		
		paramList++;	
	}

pppConnFree:
	if (pppConnJobj) zcfgFeJsonObjFree(pppConnJobj);
	if (pppoeJobj) zcfgFeJsonObjFree(pppoeJobj);
	if (ipcpJobj) zcfgFeJsonObjFree(ipcpJobj);
	if (ipIfaceJobj) zcfgFeJsonObjFree(ipIfaceJobj);
	if (vlanTermJobj) zcfgFeJsonObjFree(vlanTermJobj);
	if (ethLinkJobj) zcfgFeJsonObjFree(ethLinkJobj);

	if (natJobj) zcfgFeJsonObjFree(natJobj);
	if (natSessionJobj) zcfgFeJsonObjFree(natSessionJobj);
	if (natIntfSetJobj) zcfgFeJsonObjFree(natIntfSetJobj);

	if (dnsSrvJobj) zcfgFeJsonObjFree(dnsSrvJobj);
	if (ipv6cpJobj) zcfgFeJsonObjFree(ipv6cpJobj);
#ifdef IAAAA_CUSTOMIZATION
	if(ripObj){
		zcfgFeJsonObjFree(ripObj);
	}
#endif
#ifdef IPV6INTERFACE_PROFILE
	if (dnsV6SrvJobj) zcfgFeJsonObjFree(dnsV6SrvJobj);
	if(rtJobj)
		zcfgFeJsonObjFree(rtJobj);
#endif	

	return ret;
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i
 *
 *   Related object in TR181:
 *   Device.PPP.Interface.i
 *   Device.PPP.Interface.i.PPPoA
 *   Device.PPP.Interface.i.PPPoE
 *   Device.PPP.Interface.i.IPCP
 */
zcfgRet_t WANPppConnObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	objIndex_t pppIfaceIid, ipIfaceIid, vlanTermIid;
	objIndex_t dnsSrvIid, dnsSrvV6Iid, dnsIid, igmpIid, mldIid;
#ifdef IAAAA_CUSTOMIZATION
	objIndex_t ripIid;
	struct json_object *ripObj = NULL;
#endif
	struct json_object *pppIfaceJobj = NULL, *ipIfaceJobj = NULL, *vlanTermJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *paramValue2 = NULL;
	struct json_object *paramValue3 = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *dnsSrvJobj = NULL, *dnsSrvV6Jobj = NULL, *dnsJobj = NULL, *igmpJobj = NULL, *mldJobj;
	tr98Parameter_t *paramList = NULL;
	char ifBuf[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char typeBuf[20] = {0};
	char tmpbuf[256] = {0};
	char *igmpInterface = NULL, *mldInterface = NULL, *intf = NULL, *tmp_ptr = NULL;
	bool isDnsObjExist = false;
	bool isDnsV6ObjExist = false;
	const char *connType = NULL, *ipAddr = NULL, *snetMask = NULL;
	objIndex_t natIntfIid, natIid, natSessionIid;
	struct json_object *natIntfJobj = NULL;
	struct json_object *natJobj = NULL;
	struct json_object *natSessionJobj = NULL;
	const char *lowerLayer = NULL;
	struct json_object *rtJobj = NULL;
	objIndex_t rtIid;
	bool staticDnsType = false;
	bool staticDnsV6Type = false;
	char tr181DnsType[45+1] = {0};
	char tr181DnsV6Type[45+1] = {0};
	char *new_activeDefaultGt = NULL;
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	bool ipv4setflag = false;
	char X_ZYXEL_IfName[32] = {0};
#endif

	zcfgLog(ZCFG_LOG_INFO,"%s : Enter\n", __FUNCTION__);

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	printf("TR181 object %s\n", pppIface);
	IID_INIT(pppIfaceIid);
	pppIfaceIid.level = 1;
	sscanf(pppIface, "PPP.Interface.%hhu", &pppIfaceIid.idx[0]);

	/*Get related tr181 objects*/
	/*Get PPP.Interface.i*/
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIfaceJobj)) != ZCFG_SUCCESS) {
		printf("%s : Get PPP.Interface fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	
	/* Get Device.Ethernet.VLANTermination.i */
	lowerLayer = json_object_get_string(json_object_object_get(pppIfaceJobj, "LowerLayers"));
	if (strstr(lowerLayer, "Ethernet.VLANTermination") != NULL){
		IID_INIT(vlanTermIid);
		sscanf(lowerLayer, "Ethernet.VLANTermination.%hhu", &vlanTermIid.idx[0]);
		vlanTermIid.level = 1;
		if((ret = zcfgFeObjJsonGet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, &vlanTermJobj)) != ZCFG_SUCCESS) {
			json_object_put(pppIfaceJobj);
			return ret;
		}
	}

	/*Get IP.Interface.i above PPP.Interface.i*/
	if(zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(pppIfaceJobj);
		return ret;
	}

	IID_INIT(ipIfaceIid);
	ipIfaceIid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipIfaceIid, &ipIfaceJobj)) != ZCFG_SUCCESS) {
		printf("%s : Get IP.Interface fail\n", __FUNCTION__);
		json_object_put(vlanTermJobj);
		json_object_put(pppIfaceJobj);
		return ZCFG_INTERNAL_ERROR;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		{
			if(json_object_object_get(ipIfaceJobj, "X_ZYXEL_IfName") != NULL){
				ZOS_STRCPY_M(X_ZYXEL_IfName, json_object_get_string(json_object_object_get(ipIfaceJobj, "X_ZYXEL_IfName")));
			}
			
			objIndex_t ipv4AddrIid;
			struct json_object *ipAddrJobj = NULL;
			const char *dnsaddr = NULL, *ipaddr=NULL, *pvcstatus=NULL, *ipstatus=NULL;
			bool ipenable = false;
	
			memcpy(&ipv4AddrIid, &ipIfaceIid, sizeof(objIndex_t));
			ipv4AddrIid.level++;
			ipv4AddrIid.idx[1] = 1;
	
			if(zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V4_ADDR, &ipv4AddrIid, &ipAddrJobj) == ZCFG_SUCCESS) {
			 dnsaddr = json_object_get_string(json_object_object_get(tr98Jobj, "DNSServers"));
			 pvcstatus = json_object_get_string(json_object_object_get(ipIfaceJobj, "Status"));
			 ipaddr = json_object_get_string(json_object_object_get(ipAddrJobj, "IPAddress"));
			 ipstatus = json_object_get_string(json_object_object_get(ipAddrJobj, "Status"));
			 ipenable = json_object_get_boolean(json_object_object_get(ipAddrJobj, "Enable"));
			 if(!strcmp(pvcstatus, "Up") && !strcmp(ipstatus,"Enabled") && ipenable && dnsaddr && ipaddr && strlen(dnsaddr) && strlen(ipaddr) && strstr(dnsaddr,ipaddr)){
				printf("\r\n%s,go to error set,ip is %s,dns is %s\r\n", "WanPPpSet",ipaddr,dnsaddr);
				ret = ZCFG_REQUEST_REJECT;
				json_object_put(ipAddrJobj);
		        json_object_put(vlanTermJobj);
		        json_object_put(pppIfaceJobj);	
				json_object_put(ipIfaceJobj);
				return ret;
			 }
			}
	
			json_object_put(ipAddrJobj);
			ipAddrJobj = NULL;
		}
#endif

	json_object_object_add(ipIfaceJobj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid.idx[0] - 1));
	json_object_object_add(ipIfaceJobj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid.idx[0] - 1));
	json_object_object_add(pppIfaceJobj, "X_ZYXEL_ConnectionId", json_object_new_int(ipIfaceIid.idx[0] - 1));
	json_object_object_add(pppIfaceJobj, "X_ZYXEL_IPR2_MARKING", json_object_new_int(ipIfaceIid.idx[0] - 1));
	
	struct json_object *tr98SpvObj = NULL;
	tr98SpvObj = zcfgFeRetrieveSpv(tr98FullPathName);

	bool runSpvValid = zcfgFeTr98StatusRunSpvValid(multiJobj);

	if(multiJobj)
	{
		tr98SpvObj = json_object_object_get(multiJobj, "spv");
	}

	IID_INIT(natIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT, &natIid, &natJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		return ret;
	}
	
	IID_INIT(natSessionIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, &natSessionJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		return ret;
	}

	IID_INIT(rtIid);
	rtIid.level = 1;
	rtIid.idx[0] = 1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER, &rtIid, &rtJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(natIntfIid);
	if(natIntfGetJobj((const char *)ipIface, &natIntfIid, (struct json_object **)&natIntfJobj) != ZCFG_SUCCESS) {
		printf("%s : Retrieve NAT interface setting fail\n", __FUNCTION__);
		json_object_put(vlanTermJobj);
		json_object_put(rtJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		return ZCFG_INTERNAL_ERROR;
	}
	
#ifdef IAAAA_CUSTOMIZATION
	getSpecificObj(RDM_OID_ROUTING_RIP_INTF_SET, "Interface", json_type_string, ipIface, &ripIid, &ripObj);
#endif
	
	/*DNSServer*/


	const char *tr98DnsType = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_DNSType"));
	if(json_object_object_get(tr98SpvObj, "DNSServers") || (tr98DnsType && !strcmp(tr98DnsType, TR181_DNS_TYPE_STATIC)))
		staticDnsType = true;

	const char *tr98DnsV6Type = json_object_get_string(json_object_object_get(tr98Jobj, "X_ZYXEL_DNSV6Type"));
	if(json_object_object_get(tr98SpvObj, "X_ZYXEL_IPv6DNSServers") || (tr98DnsV6Type && !strcmp(tr98DnsV6Type, TR181_DNS_TYPE_STATIC)))
	{
		staticDnsV6Type = true;
	}

	if(dnsInfoGetJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false, false) == ZCFG_SUCCESS) { //dns v4
		isDnsObjExist = true;
		const char *paramDnsType = json_object_get_string(json_object_object_get(dnsSrvJobj, "X_ZYXEL_Type"));
		if(paramDnsType && strlen(paramDnsType))
			ZOS_STRCPY_M(tr181DnsType, paramDnsType);
	}

#ifdef IPV6INTERFACE_PROFILE
	if(dnsInfoGetJobj(ipIface, &dnsSrvV6Iid, (struct json_object **)&dnsSrvV6Jobj, true, false) == ZCFG_SUCCESS) //dns v6
	{
		isDnsV6ObjExist = true;
		const char *paramDnsV6Type = json_object_get_string(json_object_object_get(dnsSrvV6Jobj, "X_ZYXEL_Type"));
		if(paramDnsV6Type && strlen(paramDnsV6Type))
		{
			strncpy(tr181DnsV6Type, paramDnsV6Type, sizeof(tr181DnsV6Type)-1);
		}
	}
#endif	

	if(!isDnsObjExist && (staticDnsType == true)){
		if(dnsAddJobj(ipIface, &dnsSrvIid, (struct json_object **)&dnsSrvJobj, false) != ZCFG_SUCCESS) {
			json_object_put(vlanTermJobj);
			json_object_put(rtJobj);
			json_object_put(pppIfaceJobj);
			json_object_put(ipIfaceJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		isDnsObjExist = true;
	}
#ifdef IPV6INTERFACE_PROFILE
	if(!isDnsV6ObjExist && (staticDnsV6Type == true))
	{
		if(dnsAddJobj(ipIface, &dnsSrvV6Iid, (struct json_object **)&dnsSrvV6Jobj, true) != ZCFG_SUCCESS)
		{
			json_object_put(ipIfaceJobj);
			json_object_put(vlanTermJobj);
			json_object_put(natJobj);
			json_object_put(natSessionJobj);
			return ZCFG_INTERNAL_ERROR;
		}
		isDnsV6ObjExist = true;
	}
#endif

	IID_INIT(dnsIid);
	zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsJobj);
	if((ret = zcfgFeObjJsonGet(RDM_OID_DNS, &dnsIid, &dnsJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(rtJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(natIntfJobj);
		json_object_put(dnsSrvJobj);
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ret;
	}

	IID_INIT(igmpIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpIid, &igmpJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(rtJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(natIntfJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		return ret;

	}

	IID_INIT(mldIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &mldIid, &mldJobj)) != ZCFG_SUCCESS) {
		json_object_put(vlanTermJobj);
		json_object_put(rtJobj);
		json_object_put(pppIfaceJobj);
		json_object_put(ipIfaceJobj);
		json_object_put(natJobj);
		json_object_put(natSessionJobj);
		json_object_put(natIntfJobj);
		if(isDnsObjExist == true) {
			json_object_put(dnsSrvJobj);
			json_object_put(dnsJobj);
		}
#ifdef IPV6INTERFACE_PROFILE
		json_object_put(dnsSrvV6Jobj);
#endif
		json_object_put(igmpJobj);
		return ret;
	}


	if(multiJobj){
		multiJobj = zcfgFeSessMultiObjSetUtilize(multiJobj);

		if(vlanTermJobj){
			tmpObj = vlanTermJobj;
			vlanTermJobj = NULL;
			vlanTermJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ETH_VLAN_TERM, &vlanTermIid, "apply", json_object_new_boolean(false));
		}
#ifdef IAAAA_CUSTOMIZATION
	if(natIntfJobj)
#endif
		{
			tmpObj = natIntfJobj;
			natIntfJobj = NULL;
			natIntfJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_INTF_SETTING, &natIntfIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT_INTF_SETTING, &natIntfIid, "apply", json_object_new_boolean(false));
		}
		if(dnsSrvJobj){
			tmpObj = dnsSrvJobj;
			dnsSrvJobj = NULL;
			dnsSrvJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, multiJobj, tmpObj);			
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, "apply", json_object_new_boolean(true));
		}
#ifdef IPV6INTERFACE_PROFILE
		if(dnsSrvV6Jobj){
			tmpObj = dnsSrvV6Jobj;
			dnsSrvV6Jobj = NULL;
			dnsSrvV6Jobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, multiJobj, tmpObj);			
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, "apply", json_object_new_boolean(true));
		}
#endif
		if(ipIfaceJobj){
			tmpObj = ipIfaceJobj;
			ipIfaceJobj = NULL;
			ipIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipIfaceIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_IP_IFACE, &ipIfaceIid, "apply", json_object_new_boolean(true));
		}
		if(pppIfaceJobj){
			tmpObj = pppIfaceJobj;
			pppIfaceJobj = NULL;
			pppIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_PPP_IFACE, &pppIfaceIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_PPP_IFACE, &pppIfaceIid, "apply", json_object_new_boolean(true));
		}
		if(igmpJobj){
			tmpObj = igmpJobj;
			igmpJobj = NULL;
			igmpJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_IGMP, &igmpIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ZY_IGMP, &igmpIid, "apply", json_object_new_boolean(true));
		}
		if(mldJobj){
			tmpObj = mldJobj;
			mldJobj = NULL;
			mldJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_MLD, &mldIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ZY_MLD, &mldIid, "apply", json_object_new_boolean(true));
		}
		if(natSessionJobj){
			tmpObj = natSessionJobj;
			natSessionJobj = NULL;
			natSessionJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_SESSION_CTL, &natSessionIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT_SESSION_CTL, &natSessionIid, "apply", json_object_new_boolean(false));
		}
#ifdef IAAAA_CUSTOMIZATION
	if(rtJobj)
#endif
		{
			tmpObj = rtJobj;
			rtJobj = NULL;
			rtJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER, &rtIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}
		if(dnsJobj){
			tmpObj = dnsJobj;
			dnsJobj = NULL;
			dnsJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DNS, &dnsIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_DNS, &dnsIid, "apply", json_object_new_boolean(true));
		}
		if(natJobj){
			tmpObj = natJobj;
			natJobj = NULL;
			natJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT, &natIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_NAT, &natIid, "apply", json_object_new_boolean(true));
		}
#ifdef IAAAA_CUSTOMIZATION
		if(ripObj){
			tmpObj = ripObj;
			ripObj = NULL;
			ripObj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_RIP_INTF_SET, &ripIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}
#endif
		zcfgFeJsonMultiObjSetBlock(multiJobj);
		zcfgFeJsonMultiObjSetDelayApply(multiJobj);
	}

	paramValue = json_object_object_get(tr98Jobj, "ConnectionType");
	connType = json_object_get_string(paramValue);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(!strcmp(paramList->name, "Enable")) {
				json_object_object_add(ipIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				json_object_object_add(pppIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
                if(json_object_get_boolean(paramValue)){
                          ipv4setflag = true;
				}
#endif
				paramList++;
				continue;
			}
			if(!strcmp(paramList->name, "Name")) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
				char *setvalue = NULL;
				setvalue = json_object_get_string(paramValue);
				if(!strcmp(setvalue, "")){
					ret = ZCFG_INVALID_PARAM_VALUE;
					ZOS_STRCPY_M(paramfault,"Name");
					return ret;
				}
#endif
				if((tr181ParamValue = json_object_object_get(ipIfaceJobj, "X_ZYXEL_SrvName"))) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_SrvName", JSON_OBJ_COPY(paramValue));
				}
				paramList++;
				continue;
			}
			if(!strcmp(paramList->name, "X_ZYXEL_DefaultGatewayIface")){
				if(!(tr181ParamValue = json_object_object_get(ipIfaceJobj, paramList->name))){
					paramList++;
					continue;
				}
				bool tr98SetDefaultIface = json_object_get_boolean(paramValue);
				bool tr181SetDefaultIface = json_object_get_boolean(tr181ParamValue);
				const char *activeDefaultGt = json_object_get_string(json_object_object_get(rtJobj, "X_ZYXEL_ActiveDefaultGateway"));

				if(tr98SetDefaultIface && !tr181SetDefaultIface)
					new_activeDefaultGt = activeDefaultGt_Add(activeDefaultGt, ipIface);

				if(!tr98SetDefaultIface && tr181SetDefaultIface)
					new_activeDefaultGt = activeDefaultGt_Strip(activeDefaultGt, ipIface);

				if(new_activeDefaultGt) {
					json_object_object_add(rtJobj, "X_ZYXEL_ActiveDefaultGateway", json_object_new_string(new_activeDefaultGt));
					free(new_activeDefaultGt);
				}

				json_object_object_add(ipIfaceJobj, paramList->name, json_object_new_boolean(tr98SetDefaultIface));
				paramList++;
				continue;
			}

			/*Write value from WANPPPConnection to Device.PPP.Interface.i*/
			tr181ParamValue = json_object_object_get(pppIfaceJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(pppIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
			if(!strcmp(paramList->name, "ConnectionType")) {
				json_object_object_add(pppIfaceJobj, "X_ZYXEL_ConnectionType", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv4Enable") && (paramValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_IPv4Enable"))) {
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
								if(ipv4setflag == true){
									json_object_object_add(ipIfaceJobj, "IPv4Enable", json_object_new_boolean(true));
								}
								else{
				json_object_object_add(ipIfaceJobj, "IPv4Enable", JSON_OBJ_COPY(paramValue));
								}
#else
				json_object_object_add(ipIfaceJobj, "IPv4Enable", JSON_OBJ_COPY(paramValue));
#endif
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPCPEnable") && (paramValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_IPCPEnable"))) {
				json_object_object_add(pppIfaceJobj, "IPCPEnable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_Enable_DSLite", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}	
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLite_Type", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_ConcurrentWan")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_ConcurrentWan", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#ifdef ZYXEL_TAAAG_FIREWALL_WANINTERFACE
			else if(!strcmp(paramList->name, "X_ZYXEL_Enable_Firewall")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_Enable_Firewall", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#endif
			else if(!strcmp(paramList->name, "NATEnabled")){
				json_object_object_add(natIntfJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")){
				json_object_object_add(natIntfJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "ExternalIPAddress")){
				ipAddr = json_object_get_string(paramValue);
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "SubnetMask")){
				snetMask = json_object_get_string(paramValue);
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "DNSEnabled")){
				if(dnsSrvJobj){
					json_object_object_add(dnsSrvJobj, "Enable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "DNSServers")){
				if(json_object_object_get(tr98SpvObj, "DNSServers") && dnsSrvJobj){
					printf("%s: spv DNSServers \n", __FUNCTION__);
					json_object_object_add(dnsSrvJobj, "DNSServer", JSON_OBJ_COPY(paramValue));
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("Static"));
				}
				else if(dnsSrvJobj){
					paramValue2 = json_object_object_get(dnsSrvJobj, "DNSServer");
#if 0
					if(paramValue2){
						if(strcmp(json_object_get_string(paramValue), json_object_get_string(paramValue2))){
							json_object_object_add(tr98Jobj, "X_ZYXEL_DNSType", json_object_new_string(TR181_DNS_TYPE_STATIC)); // need to set tr98Jobj, workaround
							json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string(TR181_DNS_TYPE_STATIC));
							if(!strcmp(json_object_get_string(paramValue), "")){
								json_object_object_add(dnsSrvJobj, "Interface", json_object_new_string(""));
							}
						}
					}
#endif
					if(!strcmp(tr181DnsType, TR181_DNS_TYPE_STATIC) &&  strcmp(tr98DnsType, TR181_DNS_TYPE_STATIC))
						json_object_object_add(dnsSrvJobj, "DNSServer", json_object_new_string(""));
					else
 						json_object_object_add(dnsSrvJobj, "DNSServer", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
				struct json_object *spvParamValue;

				if(dnsSrvJobj && (spvParamValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_DNSType"))){
					json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", JSON_OBJ_COPY(spvParamValue));
				}
			}
#ifdef IPV6INTERFACE_PROFILE
			else if(!strcmp(paramList->name, "X_ZYXEL_DNSV6Type"))
			{
				struct json_object *spvV6ParamValue;
				if(dnsSrvV6Jobj &&  (spvV6ParamValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_DNSV6Type")))
				{
					json_object_object_add(dnsSrvV6Jobj, "X_ZYXEL_Type", JSON_OBJ_COPY(spvV6ParamValue));
				}
			}
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
						else if(!strcmp(paramList->name, "DNSOverrideAllowed")){
						bool dnstype = 0;
						dnstype = json_object_get_boolean(paramValue);
						if(dnstype == 0)
						{
							if(dnsSrvJobj){
								json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("Static"));
							}	
						}
						else
						{	
							if(dnsSrvJobj){
								json_object_object_add(dnsSrvJobj, "X_ZYXEL_Type", json_object_new_string("IPCP"));
							}	
						}
						}
#endif
			else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")) {
				igmpInterface = strdup(json_object_get_string(json_object_object_get(igmpJobj, "Interface")));
				memset(tmpbuf, 0, sizeof(tmpbuf));
				if(json_object_get_boolean(paramValue)){
					if(!strcmp(igmpInterface, "")){
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", ipIface);
					}
					else{
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", igmpInterface);
						if(!strstr(igmpInterface, ipIface)){
							strcat(tmpbuf, ",");
							strcat(tmpbuf, ipIface);
						}
					}
				}
				else{
					if(strstr(igmpInterface, ipIface) != NULL){
						tmp_ptr = igmpInterface;
						intf = strtok_r(igmpInterface, ",", &tmp_ptr);
						while(intf != NULL){
							if(*intf != '\0' && !strstr(intf, ipIface)){
								if(tmpbuf[0] != '\0'){
									strcat(tmpbuf, ",");
								}
								strcat(tmpbuf, intf);
							}
							intf = strtok_r(NULL, ",", &tmp_ptr);
						}
					}
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", igmpInterface);
				}
				json_object_object_add(igmpJobj, "Interface", json_object_new_string(tmpbuf));

				if(igmpInterface)
					free(igmpInterface);

				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")) {
				mldInterface = strdup(json_object_get_string(json_object_object_get(mldJobj, "Interface")));
				memset(tmpbuf, 0, sizeof(tmpbuf));
				if(json_object_get_boolean(paramValue)){
					if(!strcmp(mldInterface, "")){
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", ipIface);
					}
					else{
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", mldInterface);
						if(!strstr(mldInterface, ipIface)){
							strncat(tmpbuf, ",", sizeof(tmpbuf)-strlen(tmpbuf)-1);
							strncat(tmpbuf, ipIface, sizeof(tmpbuf)-strlen(tmpbuf)-1);
						}
					}
				}
				else{
					if(strstr(mldInterface, ipIface) != NULL){
						tmp_ptr = mldInterface;
						intf = strtok_r(mldInterface, ",", &tmp_ptr);
						while(intf != NULL){
							if(*intf != '\0' && !strstr(intf, ipIface)){
								if(tmpbuf[0] != '\0'){
									strncat(tmpbuf, ",", sizeof(tmpbuf)-strlen(tmpbuf)-1);
								}
								strncat(tmpbuf, intf, sizeof(tmpbuf)-strlen(tmpbuf)-1);
							}
							intf = strtok_r(NULL, ",", &tmp_ptr);
						}
					}
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "%s", mldInterface);
				}
				json_object_object_add(mldJobj, "Interface", json_object_new_string(tmpbuf));

				if(mldInterface)
					free(mldInterface);

				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")){
				if(vlanTermJobj){
					if(!runSpvValid && (json_object_get_boolean(paramValue) != json_object_get_boolean(json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANEnable")))){
						json_object_object_add(pppIfaceJobj, "Reset", json_object_new_boolean(true));
					}

					json_object_object_add(vlanTermJobj, "X_ZYXEL_VLANEnable", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")){
				if(vlanTermJobj){
					if(!runSpvValid && (json_object_get_int(paramValue) != json_object_get_int(json_object_object_get(vlanTermJobj, "X_ZYXEL_VLANPriority")))){
						json_object_object_add(pppIfaceJobj, "Reset", json_object_new_boolean(true));
					}

					json_object_object_add(vlanTermJobj, "X_ZYXEL_VLANPriority", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")){
				if(vlanTermJobj){
					if(!runSpvValid && (json_object_get_int(paramValue) != json_object_get_int(json_object_object_get(vlanTermJobj, "VLANID")))){
						json_object_object_add(pppIfaceJobj, "Reset", json_object_new_boolean(true));
					}

					json_object_object_add(vlanTermJobj, "VLANID", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#ifndef MSTC_TAAAG_MULTI_USER						
			else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") ||!strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") || 
#else
			else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif
					!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
					!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
				if (natJobj){
					json_object_object_add(natJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
				if(natSessionJobj){
					json_object_object_add(natSessionJobj, "MaxSessionPerHost", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#ifdef IPV6INTERFACE_PROFILE		
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv6DefaultGatewayIface")){

				bool tr98SetV6DefaultIface = json_object_get_boolean(paramValue);
				const char *activeV6DefaultGt = json_object_get_string(json_object_object_get(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway"));
				const char *new_activeV6DefaultGt = NULL;

				if(tr98SetV6DefaultIface && !strstr(activeV6DefaultGt, ipIface))
					new_activeV6DefaultGt = activeDefaultGt_Add(activeV6DefaultGt, ipIface);

				if(!tr98SetV6DefaultIface && strstr(activeV6DefaultGt, ipIface))
					new_activeV6DefaultGt = activeDefaultGt_Strip(activeV6DefaultGt, ipIface);

				if(new_activeV6DefaultGt) {
					json_object_object_add(rtJobj, "X_ZYXEL_ActiveV6DefaultGateway", json_object_new_string(new_activeV6DefaultGt));
					free(new_activeV6DefaultGt);
				}
				json_object_object_add(ipIfaceJobj, paramList->name, json_object_new_boolean(tr98SetV6DefaultIface));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv6CPEnable") && (paramValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_IPv6CPEnable"))){
				json_object_object_add(pppIfaceJobj, "IPv6CPEnable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable") && (paramValue = json_object_object_get(tr98SpvObj, "X_ZYXEL_IPv6Enable"))){
				json_object_object_add(ipIfaceJobj, "IPv6Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled") == 0){
				if(dnsSrvV6Jobj){
					json_object_object_add(dnsSrvV6Jobj, "Enable", JSON_OBJ_COPY(paramValue));
					paramList++;	
					continue;
				}
			}
			else if(strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers") == 0){
				if(dnsSrvV6Jobj){
					paramValue2 = json_object_object_get(dnsSrvV6Jobj, "DNSServer");		
					if(paramValue2){
						if(strcmp(json_object_get_string(paramValue), json_object_get_string(paramValue2))){
							json_object_object_add(dnsSrvV6Jobj, "X_ZYXEL_Type", json_object_new_string(TR181_DNS_TYPE_STATIC));
							if(!strcmp(json_object_get_string(paramValue), "")){
								json_object_object_add(dnsSrvV6Jobj, "Interface", json_object_new_string(""));
							}
						}
					}
					json_object_object_add(dnsSrvV6Jobj, "DNSServer", JSON_OBJ_COPY(paramValue));

					paramList++;	
					continue;
				}
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_Enable_DSLite")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_Enable_DSLite", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLiteRelayIPv6Addresses")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLiteRelayIPv6Addresses", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_DSLite_Type")) {
				json_object_object_add(ipIfaceJobj, "X_ZYXEL_DSLite_Type", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#endif
#ifdef IAAAA_CUSTOMIZATION
		else if(!strcmp(paramList->name, "X_ZYXEL_RIPEnable"))
		{
			if(ripObj != NULL)
				if(paramValue != NULL )
					json_object_object_add(ripObj, "Enable", JSON_OBJ_COPY(paramValue));
			paramList++;	
			continue;		
		}
#endif				
		}
		paramList++;
	}

	
	if(multiJobj){
		if(new_activeDefaultGt){
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ROUTING_ROUTER, &rtIid, "apply", json_object_new_boolean(true));
		}
	}
	else {
		if(dnsSrvJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsSrvIid, dnsSrvJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS.Client.Server.%d Fail\n", __FUNCTION__, dnsSrvIid.idx[0]);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS.Client.Server.%d Success\n", __FUNCTION__, dnsSrvIid.idx[0]);
			}
			json_object_put(dnsSrvJobj);
		}
#ifdef IPV6INTERFACE_PROFILE	
		if(dnsSrvV6Jobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS_CLIENT_SRV, &dnsSrvV6Iid, dnsSrvV6Jobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS.Client.Server.%d Fail\n", __FUNCTION__, dnsSrvV6Iid.idx[0]);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS.Client.Server.%d Success\n", __FUNCTION__, dnsSrvV6Iid.idx[0]);
			}
			json_object_put(dnsSrvV6Jobj);
		}
#endif
		if(dnsJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_DNS, &dnsIid, dnsJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DNS Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.DNS Success\n", __FUNCTION__);
			}
			json_object_put(dnsJobj);
		}
		
		if(vlanTermJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ETH_VLAN_TERM, &vlanTermIid, vlanTermJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Ethernet.VLANTermination.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Ethernet.VLANTermination.i Success\n", __FUNCTION__);
			}
			json_object_put(vlanTermJobj);
		}
		
		if(natJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT, &natIid, natJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.NAT Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.NAT Success\n", __FUNCTION__);
			}
			json_object_put(natJobj);
		}
		
		if(natSessionJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_SESSION_CTL, &natSessionIid, natSessionJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.NAT.X_ZYXEL_SessionControl Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.NAT.X_ZYXEL_SessionControl Success\n", __FUNCTION__);
			}
			json_object_put(natSessionJobj);
		}
		
		if(pppIfaceJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_PPP_IFACE, &pppIfaceIid, pppIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
				printf("%s : Set PPP.Interface Fail\n", __FUNCTION__);
				json_object_put(pppIfaceJobj);
				return ret;
			}
			else {
				printf("%s : Set PPP.Interface Success\n", __FUNCTION__);
			}
			json_object_put(pppIfaceJobj);
		}
		
		if(ipIfaceJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipIfaceIid, ipIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.IP.Interface.i Success\n", __FUNCTION__);
			}
			json_object_put(ipIfaceJobj);
		}

		if(rtJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER, &rtIid, rtJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Routing.Router. Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Routing.Router. Success\n", __FUNCTION__);
			}
			json_object_put(rtJobj);
		}
#ifdef IAAAA_CUSTOMIZATION
		if (ripObj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_RIP_INTF_SET, &ripIid, ripObj, NULL)) != ZCFG_SUCCESS ) {
				printf("%s : Set RDM_OID_ROUTING_RIP_INTF_SET object Fail with ret=%d\n", __FUNCTION__, ret);
			}
			json_object_put(ripObj);
		}
#endif
	}

	return ZCFG_SUCCESS;
}

#ifdef WAN_ADD_INDIVIDUAL
enum {
	WAN_PPPCONN_EINTELVLPPP,
	WAN_PPPCONN_ATMPPP
};
#endif
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i
 *
 *   Related object in TR181:
 */
zcfgRet_t WANPppConnObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char wanIntfObj[128] = {0};
	char wanLinkObj[128] = {0};
	char wanPppConnObj[128] = {0};
	char ethLinkPathName[32] = {0};
	char vlanTermPathName[32] = {0};
	char pppIfacePathName[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};  /*tr98TmpName is used to look up mapping table*/
	char *ptr = NULL;
	uint32_t  wanLinkOid = 0;
	objIndex_t wanLinkIid;
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	rdm_AtmLink_t *atmLinkObj = NULL;
#endif
#ifdef WAN_ADD_INDIVIDUAL
	int wanPppConnType = WAN_PPPCONN_EINTELVLPPP;
	char *wanLinkName = NULL;
#else
	char higherLayer[64] = {0};
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	rdm_PtmLink_t *ptmLinkObj = NULL;
#endif
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION
	int WanType = 3;   //1:eth,2:ATM,3:PTM
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANConnectionDevice");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanIntfObj)) != ZCFG_SUCCESS) {
		return ret;
	}

        /* wanIntfObj will be DSL.Channel.i or Ethernet.Interface.i */
	printf("%s : TR181 object %s\n", __FUNCTION__, wanIntfObj);

	/*Check it's DSL or Ethernet first*/
	if(strstr(wanIntfObj, "DSL") != NULL) { // DSL WAN
		printf("DSL WAN\n");
		ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
		ptr = strstr(tr98TmpName, ".WANPPPConnection");
		*ptr = '\0';

		/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
		if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, wanLinkObj)) != ZCFG_SUCCESS) {
			return ret;
		}
			
		/* wanLinkObj will be ATM.Link or PTM.Link */
		printf("TR181 object %s\n", wanLinkObj);
		IID_INIT(wanLinkIid);
		sprintf(tr181Obj, "Device.%s", wanLinkObj);
		wanLinkOid = zcfgFeObjNameToObjId(tr181Obj, &wanLinkIid);
		if(wanLinkOid == ZCFG_NO_SUCH_OBJECT) {
			printf("%s : can not find OID for %s\n", __func__, tr181Obj);
			return ZCFG_NO_SUCH_OBJECT;
		}
#ifdef CONFIG_TAAAB_CUSTOMIZATION

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			WanType = 2;
		}
		else
#endif 
		if (wanLinkOid == RDM_OID_PTM_LINK) {
			WanType = 3;
		}
#endif		

#ifdef WAN_ADD_INDIVIDUAL

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			printf("ATM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_ATM_LINK, &wanLinkIid, (void **)&atmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			if(strcmp(atmLinkObj->LinkType, "EoA") == 0) {
				wanPppConnType = WAN_PPPCONN_EINTELVLPPP;
			}
			else if(strcmp(atmLinkObj->LinkType, "PPPoA") == 0) {
				wanPppConnType = WAN_PPPCONN_ATMPPP;
			}
			else {
				printf("ATM Link type %s not supported\n", atmLinkObj->LinkType);
				zcfgFeObjStructFree(atmLinkObj);
				return ZCFG_INVALID_PARAM_VALUE;
			}
			zcfgFeObjStructFree(atmLinkObj);
		}
#endif //CONFIG_ZCFG_BE_MODULE_ATM_LINK
		wanLinkName = wanLinkObj;
	}
	else if(strstr(wanIntfObj, "Ethernet") != NULL) { // Ethernet WAN
#ifdef CONFIG_TAAAB_CUSTOMIZATION
		WanType = 1;
#endif
		wanPppConnType = WAN_PPPCONN_EINTELVLPPP;
		wanLinkName = wanIntfObj;
	}
	else if(strstr(wanIntfObj, "Optical") != NULL) { // Optical WAN
		wanPppConnType = WAN_PPPCONN_EINTELVLPPP;
		wanLinkName = wanIntfObj;
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	struct json_object *multiObj = json_object_new_object();
	if(wanPppConnType == WAN_PPPCONN_EINTELVLPPP) {

#ifdef CONFIG_TAAAB_CUSTOMIZATION
		wanLinkName = wanLinkObj;
		if(strstr(wanIntfObj, "Ethernet") != NULL || strstr(wanIntfObj, "Optical") != NULL){//ETH or PON
			ZOS_STRCPY_M(ethLinkPathName, wanLinkName);
		}
		else if(strstr(wanIntfObj, "DSL") != NULL && wanLinkOid == RDM_OID_ETH_LINK){//PTM
			ZOS_STRCPY_M(ethLinkPathName, wanLinkName);
		}
		else{//ATM
			if((ret = multiObjEthLinkAdd(ethLinkPathName, wanLinkName, multiObj)) != ZCFG_SUCCESS){
				json_object_put(multiObj);
				return ret;
			}
		}
				
#else
		/*Add Device.Ethernet.Link.i*/
		if((ret = multiObjEthLinkAdd(ethLinkPathName, wanLinkName, multiObj)) != ZCFG_SUCCESS){
			json_object_put(multiObj);
			return ret;
		}
#endif

		/*add Device.Ethernet.VLANTermination.i*/
		if((ret = multiObjVlanTermAdd(vlanTermPathName, ethLinkPathName, multiObj)) != ZCFG_SUCCESS){
			json_object_put(multiObj);
			return ret;
		}

		/*Add Device.PPP.Interface.i*/
		#ifdef CONFIG_TAAAB_CUSTOMIZATION
		if((ret = multiObjPppIfaceAdd(pppIfacePathName, vlanTermPathName, multiObj, WanType)) != ZCFG_SUCCESS){
		#else
		if((ret = multiObjPppIfaceAdd(pppIfacePathName, vlanTermPathName, multiObj)) != ZCFG_SUCCESS){
		#endif
			json_object_put(multiObj);
			return ret;
		}
	}
	else if(wanPppConnType == WAN_PPPCONN_ATMPPP) {
		printf("ATM Link type is PPPoA\n");
		/*Add Device.PPP.Interface.i and Device.IP.Interface.i ?*/
		#ifdef CONFIG_TAAAB_CUSTOMIZATION
		if((ret = multiObjPppIfaceAdd(pppIfacePathName, wanLinkName, multiObj, WanType)) != ZCFG_SUCCESS){
		#else
		if((ret = multiObjPppIfaceAdd(pppIfacePathName, wanLinkName, multiObj)) != ZCFG_SUCCESS){
		#endif
			json_object_put(multiObj);
			return ret;
		}
	}

	zcfgFeJsonMultiObjSetBlock(multiObj);
	zcfgFeJsonMultiObjSetDelayApply(multiObj);

	ret = zcfgFeMultiObjJsonSet(multiObj, NULL);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
		json_object_put(multiObj);
			return ret;
		}
	json_object_put(multiObj);

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	if(ret == ZCFG_SUCCESS) {
		if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
			return ret;
		}
	}

	sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
#else
	zcfg_name_t nameSeqNum = 0;
	objIndex_t tr98ObjIid;

	IID_INIT(tr98ObjIid);
	zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)pppIfacePathName, &nameSeqNum, &tr98ObjIid);

	if(IS_SEQNUM_RIGHT(nameSeqNum)) {
		printf("Added WANIPConnection %d\n", tr98ObjIid.idx[2]);
		*idx = tr98ObjIid.idx[2];
	}
#endif

#else

#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
		if(wanLinkOid == RDM_OID_ATM_LINK) {
			printf("ATM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_ATM_LINK, &wanLinkIid, (void **)&atmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			/*Try to get the higher layer of ATM.Link*/
			ret = zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, higherLayer);
			if(ret == ZCFG_NO_SUCH_OBJECT) {
				if(strcmp(atmLinkObj->LinkType, "EoA") == 0) {
					/*Add Device.Ethernet.Link.i*/
					ret = ethLinkAdd(ethLinkPathName, wanLinkObj);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					/*Add Device.Ethernet.VLANTermination.i*/
					ret = vlanTermAdd(vlanTermPathName, ethLinkPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					/*Add Device.PPP.Interface.i*/
					ret = pppIfaceAdd(pppIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
				}
				else if(strcmp(atmLinkObj->LinkType, "IPoA") == 0) {
					printf("ATM Link type is IPoA\n");
					/*Add Device.IP.Interface.i ?*/
				}
				else if(strcmp(atmLinkObj->LinkType, "PPPoA") == 0) {
					printf("ATM Link type is PPPoA\n");
					/*Add Device.PPP.Interface.i and Device.IP.Interface.i ?*/
					ret = pppIfaceAdd(pppIfacePathName, wanLinkObj);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
				}
				else {
					printf("ATM Link type %s not supported\n", atmLinkObj->LinkType);
				}
			}
			else {
				/*Ethernet.Link already exists*/
				if(strstr(higherLayer, "Ethernet.Link") != NULL) {

					printf("%s : Ethernet.Link already exists\n", __FUNCTION__);

					if(zcfgFeTr181IfaceStackHigherLayerGet(higherLayer, vlanTermPathName) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(atmLinkObj);
						return ZCFG_INTERNAL_ERROR;
					}

					if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
						ret = vlanTermAdd(vlanTermPathName, higherLayer);
						if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}
					
					/*Add Device.PPP.Interface.i*/
					ret = pppIfaceAdd(pppIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(atmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(atmLinkObj);
							return ret;
						}
					}

					sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
				}
				else{
					printf("%s : Unknown HigherLayers %s\n", __FUNCTION__, higherLayer);
				}
			}
			zcfgFeObjStructFree(atmLinkObj);
		}
		else
#endif // CONFIG_ZCFG_BE_MODULE_ATM_LINK
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
		if(wanLinkOid == RDM_OID_PTM_LINK) {
			printf("PTM Link\n");
			if((ret = zcfgFeObjStructGet(RDM_OID_PTM_LINK, &wanLinkIid, (void **)&ptmLinkObj)) != ZCFG_SUCCESS) {
				return ret;
			}

			/*Try to get the higher layer of PTM.Link*/
			ret = zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, higherLayer);
			if(ret == ZCFG_NO_SUCH_OBJECT) {
				/*Add Device.Ethernet.Link.i*/
				ret = ethLinkAdd(ethLinkPathName, wanLinkObj);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					return ret;
				}

				/*Add Device.Ethernet.VLANTermination.i*/
				ret = vlanTermAdd(vlanTermPathName, ethLinkPathName);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					return ret;
				}

				/*Add Device.PPP.Interface.i*/
				ret = pppIfaceAdd(pppIfacePathName, vlanTermPathName);
				if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
					zcfgFeObjStructFree(ptmLinkObj);
					return ret;
				}

				if(ret == ZCFG_SUCCESS) {
					if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ret;
					}
				}

				sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
			}
			else {
				/*Ethernet.Link already exists*/
				if(strstr(higherLayer, "Ethernet.Link") != NULL) {
					printf("%s : Ethernet.Link already exists\n", __FUNCTION__);

					if(zcfgFeTr181IfaceStackHigherLayerGet(higherLayer, vlanTermPathName) != ZCFG_SUCCESS) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ZCFG_INTERNAL_ERROR;
					}
					if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
						ret = vlanTermAdd(vlanTermPathName, higherLayer);
						if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
							zcfgFeObjStructFree(ptmLinkObj);
							return ret;
						}
					}
					
					/*Add Device.PPP.Interface.i*/
					ret = pppIfaceAdd(pppIfacePathName, vlanTermPathName);
					if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
						zcfgFeObjStructFree(ptmLinkObj);
						return ret;
					}

					if(ret == ZCFG_SUCCESS) {
						if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
							zcfgFeObjStructFree(ptmLinkObj);
							return ret;
						}
					}

					sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
				}
				else{
					printf("%s : Unknown HigherLayers %s\n", __FUNCTION__, higherLayer);
				}	
			}

			zcfgFeObjStructFree(ptmLinkObj);
		}
		else 
#endif // CONFIG_ZCFG_BE_MODULE_PTM_LINK
		{
			printf("Unknown Link Type\n");
		}
	}
	else if(strstr(wanIntfObj, "Ethernet") != NULL) { // Ethernet WAN
		printf("Ethernet WAN\n");
		/*Try to get the higher layer of Ethernet.Interface*/
		ret = zcfgFeTr181IfaceStackHigherLayerGet(wanIntfObj, wanLinkObj);
		if(ret == ZCFG_NO_SUCH_OBJECT) {
			/*It should not happen here*/
			return ret;
		}

		printf("Ethernet Link %s\n", wanLinkObj);

		if(zcfgFeTr181IfaceStackHigherLayerGet(wanLinkObj, vlanTermPathName) != ZCFG_SUCCESS) {
			return ZCFG_INTERNAL_ERROR;
		}

		if (strstr(vlanTermPathName, "Ethernet.VLANTermination.") == NULL){
			ret = vlanTermAdd(vlanTermPathName, wanLinkObj);
			if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
				return ret;
			}
		}

		/*Add Device.PPP.Interface.i*/
		ret = pppIfaceAdd(pppIfacePathName, vlanTermPathName);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			return ret;
		}

		if(ret == ZCFG_SUCCESS) {
			if((ret = zcfgFe181To98MappingNameGet(pppIfacePathName, wanPppConnObj)) != ZCFG_SUCCESS) {
				return ret;
			}
		}

		sscanf(wanPppConnObj, "%*[^.].%*[^.].%*d.%*[^.].%*d.WANPPPConnection.%d", idx);
	}
#endif

	return ret;
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i

    Related object in TR181:
 */
zcfgRet_t WANPppConnObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char pppIfacePathName[32] = {0};
	objIndex_t pppIfaceIid, ipIfaceIid;
	rdm_IpIface_t *ipIface = NULL;
#ifdef WAN_ADD_INDIVIDUAL
	struct json_object *pppIface = NULL;
	const char *lowerLayer = NULL;
#endif
	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, pppIfacePathName)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	printf("%s : pppIface %s\n", __FUNCTION__, pppIfacePathName);
	sprintf(tr181Obj, "Device.%s", pppIfacePathName);
	zcfgFeObjNameToObjId(tr181Obj, &pppIfaceIid);

	IID_INIT(ipIfaceIid);
	while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
		if(strcmp(ipIface->LowerLayers, pppIfacePathName) == 0) {
			if(zcfgFeObjStructDel(RDM_OID_IP_IFACE, &ipIfaceIid, NULL) == ZCFG_SUCCESS) {
				printf("%s : Delete IP Interface Success\n", __FUNCTION__);
			}
			else {
				printf("%s : Delete IP Interface Fail\n", __FUNCTION__);
			}
			zcfgFeObjStructFree(ipIface);
			break;
		}
		zcfgFeObjStructFree(ipIface);
	}

#ifdef WAN_ADD_INDIVIDUAL
	if((ret = zcfgFeObjJsonGet(RDM_OID_PPP_IFACE, &pppIfaceIid, &pppIface)) != ZCFG_SUCCESS) {
		printf("%s : Retreieve PPP Interface Fail\n", __FUNCTION__);
		return ret;
	}
#endif
	ret = zcfgFeObjStructDel(RDM_OID_PPP_IFACE, &pppIfaceIid, NULL);
	if (ret == ZCFG_SUCCESS) {
		printf("%s : Delete PPP Interface Success\n", __FUNCTION__);
	}
	else {
		printf("%s : Delete PPP Interface Fail\n", __FUNCTION__);
	}
#ifdef WAN_ADD_INDIVIDUAL
	lowerLayer = json_object_get_string(json_object_object_get(pppIface, "LowerLayers"));
	if(!lowerLayer) {
		printf("%s : pppIface lowerlayer is none\n", __FUNCTION__);
		json_object_put(pppIface);
		return ZCFG_INTERNAL_ERROR;
	}
	printf("%s : pppIface lowerlayer %s\n", __FUNCTION__, lowerLayer);

	ret = WANIpPppConnObjDelEther(lowerLayer);

	json_object_put(pppIface);

#endif

	return ret;
}

zcfgRet_t WANPppConnObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		if(strcmp(tr181ParamName, "LocalIPAddress") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "ExternalIPAddress");
			break;
		}

		if(strcmp(tr181ParamName, "LocalInterfaceIdentifier") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "X_ZYXEL_LocalInterfaceIdentifier");
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANPppConnObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
/*
	char tr181Obj[128] = {0};
	uint32_t  devInfoOid = 0, ipcpOid = 0;
	objIndex_t devInfoIid, ipcpIid;
*/
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
/*
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	
	IID_INIT(ipcpIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s.IPCP", mapObj);
	ipcpOid = zcfgFeObjNameToObjId(tr181Obj, &ipcpIid);
*/
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/
		if(strcmp(paramList->name, "ExternalIPAddress") == 0){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE_IPCP, "LocalIPAddress");
		}
		else if(strcmp(paramList->name, "RemoteIPAddress") == 0){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE_IPCP, paramList->name);
		}
		else if(strcmp(paramList->name, "X_ZYXEL_LocalInterfaceIdentifier") == 0){ //++
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_INTF_IPV6CP, "LocalInterfaceIdentifier");
		}
		else if(strcmp(paramList->name, "X_ZYXEL_RemoteInterfaceIdentifier") == 0){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_INTF_IPV6CP, "RemoteInterfaceIdentifier");
		}		
		else if(!strcmp(paramList->name, "Name")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "X_ZYXEL_SrvName");
		}
		else if(!strcmp(paramList->name, "DNSEnabled") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DNSType")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "X_ZYXEL_Type");
		}
		else if(!strcmp(paramList->name, "DNSServers") || !strcmp( paramList->name, "X_ZYXEL_IPv6DNSServers")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DNS_CLIENT_SRV, "DNSServer");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IGMPProxyEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ZY_IGMP, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MLDProxyEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ZY_MLD, "Enable");
		}
		else if(!strcmp(paramList->name, "DefaultGateway")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ROUTING_ROUTER_V4_FWD, "GatewayIPAddress");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_DefaultGatewayIface")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, paramList->name);
		}
		else if(!strcmp(paramList->name, "NATEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_INTF_SETTING, "Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_FullConeEnabled")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_INTF_SETTING, "X_ZYXEL_FullConeEnabled");
		}
		else if(!strcmp(paramList->name, "PPPoESessionID")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE_PPPOE, "SessionID");
		}
		else if(!strcmp(paramList->name, "PPPoEACName")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE_PPPOE, "ACName");
		}
		else if(!strcmp(paramList->name, "PPPoEServiceName")){
#ifdef ZYXEL_PPPoE_SERVICE_OPTION
            attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "X_ZYXEL_ServiceName");
#else
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE_PPPOE, "ServiceName");
#endif
		}	
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Enable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6Enable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6Status")){
			//attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6Status");
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "Status");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6AddressNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6AddressNumberOfEntries");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_IPv6PrefixNumberOfEntries")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_IP_IFACE, "IPv6PrefixNumberOfEntries");
		}	
		else if(!strcmp(paramList->name, "PPPEncryptionProtocol")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "EncryptionProtocol");
		}
		else if(!strcmp(paramList->name, "PPPCompressionProtocol")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "CompressionProtocol");
		}
		else if(!strcmp(paramList->name, "PPPLCPEcho")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "LCPEcho");
		}
		else if(!strcmp(paramList->name, "PPPLCPEchoRetry")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "LCPEchoRetry");
		}
		else if(!strcmp(paramList->name, "PPPAuthenticationProtocol")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "AuthenticationProtocol");
		}
		else if(!strcmp(paramList->name, "PossibleConnectionTypes")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "X_ZYXEL_PossibleConnectionTypes");
		}
		else if(!strcmp(paramList->name, "ConnectionType")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, "X_ZYXEL_ConnectionType");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANEnable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANPriority");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "VLANID");
		}
#ifndef MSTC_TAAAG_MULTI_USER					
		else if(!strcmp(paramList->name, "X_ZYXEL_DMZHost_Enable") || !strcmp(paramList->name, "X_ZYXEL_DMZHost") || !strcmp(paramList->name, "X_ZYXEL_NAT_ALG") || 
#else
		else if(!strcmp(paramList->name, "X_ZYXEL_NAT_ALG") ||
#endif
				!strcmp(paramList->name, "X_ZYXEL_SIP_ALG") || !strcmp(paramList->name, "X_ZYXEL_RTSP_ALG") ||
				!strcmp(paramList->name, "X_ZYXEL_PPTP_ALG") || !strcmp(paramList->name, "X_ZYXEL_IPSEC_ALG")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT, paramList->name);
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_MaxSessionPerHost")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_NAT_SESSION_CTL, "MaxSessionPerHost");
		}
		else{
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_PPP_IFACE, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANPppConnObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0, ipcpOid = 0;
	objIndex_t devInfoIid, ipcpIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	
	IID_INIT(ipcpIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s.IPCP", mapObj);
	ipcpOid = zcfgFeObjNameToObjId(tr181Obj, &ipcpIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;


		/*special case*/
		if(strcmp(paramList->name, "ExternalIPAddress") == 0){
			attrValue = zcfgFeParamAttrGetByName(ipcpOid, "LocalIPAddress");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanEnable")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANEnable");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMux8021p")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "X_ZYXEL_VLANPriority");
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_VlanMuxID")){
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_ETH_VLAN_TERM, "VLANID");
		}
		else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "ExternalIPAddress") == 0){
			ret = zcfgFeMultiParamAttrAppend(ipcpOid, multiAttrJobj, "LocalIPAddress", attrValue);
		}
		else if(strcmp(paramList->name, "X_ZYXEL_VlanEnable") == 0){
			ret = zcfgFeMultiParamAttrAppend(RDM_OID_ETH_VLAN_TERM, multiAttrJobj, "X_ZYXEL_VLANEnable", attrValue);
		}
		else if(strcmp(paramList->name, "X_ZYXEL_VlanMux8021p") == 0){
			ret = zcfgFeMultiParamAttrAppend(RDM_OID_ETH_VLAN_TERM, multiAttrJobj, "X_ZYXEL_VLANPriority", attrValue);
		}
		else if(strcmp(paramList->name, "X_ZYXEL_VlanMuxID") == 0){
			ret = zcfgFeMultiParamAttrAppend(RDM_OID_ETH_VLAN_TERM, multiAttrJobj, "VLANID", attrValue);
		}
		else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}

/*  TR98 Object Name : 
   InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i
   InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i
*/
zcfgRet_t WANIpPppConnObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgFeSetParmAttr_t *setParmAttr = NULL;
	uint32_t seqnum = 0;
	objIndex_t tr98ObjIid, tr181ObjIid;
	uint32_t attr = 0;
	struct json_object *jobj = NULL;
	char ipIfaceName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	printf("%s: parm %s\n", __FUNCTION__, paramName);

	if(zcfgFe98NameToSeqnum((const char *)tr98FullPathName, &seqnum, &tr98ObjIid) != ZCFG_SUCCESS)
		return ZCFG_NO_SUCH_OBJECT;

	IID_INIT(tr181ObjIid);
	if(seqnum == e_TR98_WAN_PPP_CONN) {
		zcfg_offset_t oid = 0;
		char pppIfaceName[MAX_TR181_PATHNAME] = {0};
		// PPP.Interface mapping
		if(zcfgFe98To181ObjMappingGet(seqnum, &tr98ObjIid, &oid, &tr181ObjIid) != ZCFG_SUCCESS)	{
			printf("%s: no WANPPPConnection mapping\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}
		if(oid != RDM_OID_PPP_IFACE)
			return ZCFG_NO_SUCH_OBJECT;

		sprintf(pppIfaceName, "PPP.Interface.%hhu", tr181ObjIid.idx[0]);
		if(zcfgFeTr181IfaceStackHigherLayerGet(pppIfaceName, ipIfaceName) != ZCFG_SUCCESS) {
			printf("%s: retrieve PPP.Interface higher layer fail\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}
		printf("%s: WANPPPConn ipIface: %s\n", __FUNCTION__, ipIfaceName);
	}else if(seqnum == e_TR98_WAN_IP_CONN) {
		if(zcfgFe98To181MappingNameGet(tr98FullPathName, ipIfaceName) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;

		printf("%s: WANIPConn ipIface %s\n", __FUNCTION__, ipIfaceName);			
	}
	else
		return ZCFG_NO_SUCH_OBJECT;
	
	// DNSEnabled, DNSServers, X_ZYXEL_IPv6DNSEnabled, X_ZYXEL_IPv6DNSServers, DefaultGateway
	if(!strcmp(paramName, "DNSEnabled") || !strcmp(paramName, "X_ZYXEL_IPv6DNSEnabled")) {
		if(retrieveJobjWithIpIface(ipIfaceName, RDM_OID_DNS_CLIENT_SRV, &tr181ObjIid, &jobj) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;

		json_object_put(jobj);
		attr = PARAMETER_ATTR_SET_TR98(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, paramName, 0, NULL, NULL, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
		attr = PARAMETER_ATTR_SET_TR181(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(RDM_OID_DNS_CLIENT_SRV, &tr181ObjIid, "Enable", seqnum, &tr98ObjIid, paramName, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
	}
	else if(!strcmp(paramName, "DNSServers") || !strcmp(paramName, "X_ZYXEL_IPv6DNSServers")) {
		if(retrieveJobjWithIpIface(ipIfaceName, RDM_OID_DNS_CLIENT_SRV, &tr181ObjIid, &jobj) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;

		json_object_put(jobj);
		attr = PARAMETER_ATTR_SET_TR98(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, paramName, 0, NULL, NULL, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
		attr = PARAMETER_ATTR_SET_TR181(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(RDM_OID_DNS_CLIENT_SRV, &tr181ObjIid, "DNSServer", seqnum, &tr98ObjIid, paramName, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
	}
	else if(!strcmp(paramName, "DefaultGateway")) {
		if(retrieveJobjWithIpIface(ipIfaceName, RDM_OID_ROUTING_ROUTER_V4_FWD, &tr181ObjIid, &jobj) != ZCFG_SUCCESS)
			return ZCFG_NO_SUCH_OBJECT;

		json_object_put(jobj);
		attr = PARAMETER_ATTR_SET_TR98(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, paramName, 0, NULL, NULL, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
		attr = PARAMETER_ATTR_SET_TR181(newNotify);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(RDM_OID_ROUTING_ROUTER_V4_FWD, &tr181ObjIid, "GatewayIPAddress", seqnum, &tr98ObjIid, paramName, attr);
		json_object_array_add(multiAttrJobj, json_object_new_int((int)setParmAttr));
	}

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD
 * 						InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t WANConnMldObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	objIndex_t wanMldIid;
	struct json_object *wanMldJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_MLD");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	
	IID_INIT(wanMldIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_MLD, &wanMldIid, &wanMldJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
	
	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(wanMldJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		paramList++;
	}

	json_object_put(wanMldJobj);
	
	return ZCFG_SUCCESS;	
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD
 * 						InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t WANConnMldObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *wanMldJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	tr98Parameter_t *paramList = NULL;
	
	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &objIid, &wanMldJobj)) != ZCFG_SUCCESS)
		return ret;

	if(multiJobj){
		tmpObj = wanMldJobj;
		wanMldJobj = NULL;
		wanMldJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_MLD, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(wanMldJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(wanMldJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		printf("Can't find parameter %s in TR181\n", paramList->name);

		paramList++;
	}

	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &objIid, wanMldJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(wanMldJobj);
			return ret;
		}
		json_object_put(wanMldJobj);
	}

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD
 * 						InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t WANConnMldObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, attrParamNamePtr);
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD
 * 						InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
int WANConnMldObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char mapObj[128] = {0};
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		attrValue = zcfgFeParamAttrGetByName(RDM_OID_ZY_MLD, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD
 * 						InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD
 *
 *   Related object in TR181:
 *   Device.X_ZYXEL_MLD
 */
zcfgRet_t WANConnMldObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	uint32_t oid = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		attrValue = zcfgFeParamAttrGetByName(oid, paramList->name);
		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramList->name, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramList->name);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_AddrMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_AddrMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.X_ZYXEL_AddrMapping.i
 */
zcfgRet_t WANAddrMappingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char natAddrMap[32] = {0};
	objIndex_t objIid;
	struct json_object *addrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, natAddrMap)) != ZCFG_SUCCESS) {
		/*  The instance number of X_ZYXEL_AddrMapping.i will be continuous because of the tr98 to tr181 mapping table. 
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this PortMapping object.
		 */	
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(natAddrMap, "NAT.X_ZYXEL_AddrMapping.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_NAT_ADDR_MAPPING, &objIid, &addrJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(addrJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

	json_object_put(addrJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_AddrMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_AddrMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.X_ZYXEL_AddrMapping.i
 */
zcfgRet_t WANAddrMappingSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *addrJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	char addrMapping[32] = {0};
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, addrMapping)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_DEBUG, "%s : addrMapping %s\n", __FUNCTION__, addrMapping);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(addrMapping, "NAT.X_ZYXEL_AddrMapping.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_ADDR_MAPPING, &objIid, &addrJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(multiJobj){
		tmpObj = addrJobj;
		addrJobj = NULL;
		addrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_ADDR_MAPPING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(addrJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(addrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;	
	}

	/*Set Device.NAT.X_ZYXEL_AddrMapping.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_ADDR_MAPPING, &objIid, addrJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(addrJobj);
			return ret;
		}
		json_object_put(addrJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t WANAddrMappingAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	char tr98ConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char iface[32] = {0};
	char *ptr = NULL;
	rdm_IpIface_t *ipIface = NULL;
	rdm_NatAddrMapping_t *natAddrMappingObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98ConnName, tr98FullPathName);
	ptr = strstr(tr98ConnName, ".X_ZYXEL_AddrMapping");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnName, iface)) != ZCFG_SUCCESS) {
		return ret;
	}
	ztrdbg(ZTR_DL_DEBUG, "iface = %s\n", iface);
	// PPP.Interface
	if(strstr(iface, "PPP")){
		//printf("PPP case\n");
		IID_INIT(objIid);
		while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &objIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
			if(strcmp(ipIface->LowerLayers, iface) == 0) {
				memset(iface, '\0', sizeof(iface));
				snprintf(iface, sizeof(iface), "IP.Interface.%d",  objIid.idx[0]);
				printf("match! new iface = %s\n", iface);
				zcfgFeObjStructFree(ipIface);
				break;
			}
			zcfgFeObjStructFree(ipIface);
		}
	}else{
		//IP.Interface
		IID_INIT(objIid);
		sscanf(iface, "IP.Interface.%hhu", &objIid.idx[0]);
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_NAT_ADDR_MAPPING, &objIid, NULL)) != ZCFG_SUCCESS) {
		ztrdbg(ZTR_DL_DEBUG, "%s : Add NAT.X_ZYXEL_AddrMapping Fail.\n", __FUNCTION__);
		return ret;
	}

	if(zcfgFeObjStructGet(RDM_OID_NAT_ADDR_MAPPING, &objIid, (void **)&natAddrMappingObj) == ZCFG_SUCCESS) {
		ZOS_STRCPY_M(natAddrMappingObj->Interface, iface);
		ret = zcfgFeObjStructSet(RDM_OID_NAT_ADDR_MAPPING, &objIid, (void *)natAddrMappingObj, NULL);
		if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
		{
			ztrdbg(ZTR_DL_DEBUG, "%s : Set NAT.X_ZYXEL_AddrMapping Fail.\n", __FUNCTION__);
		}
		ztrdbg(ZTR_DL_DEBUG, "%s: Set X_ZYXEL_AddrMapping complete\n", __FUNCTION__);
		zcfgFeObjStructFree(natAddrMappingObj);

#ifdef ZCFG_DYNAMIC_NAMEMAPPING
		if((ret == ZCFG_DELAY_APPLY) || (ret == ZCFG_SUCCESS)){
			zcfg_name_t nameSeqNum = 0;
			objIndex_t tr98ObjIid;
			char tr181PathName[64] = {0};

			sprintf(tr181PathName, "NAT.X_ZYXEL_AddrMapping.%d", objIid.idx[0] );
		
			IID_INIT(tr98ObjIid);
			zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)tr181PathName, &nameSeqNum, &tr98ObjIid);
		
			if(IS_SEQNUM_RIGHT(nameSeqNum)) {
				ztrdbg(ZTR_DL_DEBUG, "%s: Added X_ZYXEL_AddrMapping %d\n", __FUNCTION__, tr98ObjIid.idx[3]);
				*idx = tr98ObjIid.idx[3];
			}
		}
#endif
		return ret;
	}
	else {
		ztrdbg(ZTR_DL_DEBUG, "%s : Get NAT.X_ZYXEL_AddrMapping Fail.\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;	
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t WANAddrMappingDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	char addrMapping[32] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, addrMapping)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(addrMapping, "NAT.X_ZYXEL_AddrMapping.%hhu", &objIid.idx[0]);

	ret = zcfgFeObjStructDel(RDM_OID_NAT_ADDR_MAPPING, &objIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
	{
		printf("%s : Delete NAT.X_ZYXEL_AddrMapping Fail\n", __FUNCTION__);
	}

	return ret;
}

zcfgRet_t WANAddrMappingNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANAddrMappingAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANAddrMappingAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;
	}
	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_PortTriggering.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_PortTriggering.i
 *
 *   Related object in TR181:
 *   Device.NAT.X_ZYXEL_PortTriggering.i
 */
zcfgRet_t WANPortTriggerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char natPortTrigger[MAX_TR181_PATHNAME] = {0};
	objIndex_t objIid;
	struct json_object *triggerJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, natPortTrigger)) != ZCFG_SUCCESS) {
		/*  The instance number of X_ZYXEL_PortTriggering.i will be continuous because of the tr98 to tr181 mapping table. 
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this PortMapping object.
		 */	
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(natPortTrigger, "NAT.X_ZYXEL_PortTriggering.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &triggerJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(triggerJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

	json_object_put(triggerJobj);

	return ZCFG_SUCCESS;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_PortTriggering.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_PortTriggering.i
 *
 *   Related object in TR181:
 *   Device.NAT.X_ZYXEL_PortTriggering.i
 */
zcfgRet_t WANPortTriggerSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *triggerJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	char portTrigger[MAX_TR181_PATHNAME] = {0};
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, portTrigger)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_DEBUG, "%s : portTrigger %s\n", __FUNCTION__, portTrigger);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(portTrigger, "NAT.X_ZYXEL_PortTriggering.%hhu", &objIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, &triggerJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(multiJobj){
		tmpObj = triggerJobj;
		triggerJobj = NULL;
		triggerJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_PORT_TRIGGERING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(triggerJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(triggerJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;	
	}

	/*Set Device.NAT.X_ZYXEL_PortTriggering.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, triggerJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(triggerJobj);
			return ret;
		}
		json_object_put(triggerJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t WANPortTriggerAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	char tr98ConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char iface[MAX_TR181_PATHNAME] = {0};
	char *ptr = NULL;
	rdm_IpIface_t *ipIface = NULL;
	rdm_NatPortTriggering_t *natPortTriggerObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98ConnName, tr98FullPathName);
	ptr = strstr(tr98ConnName, ".X_ZYXEL_PortTriggering");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnName, iface)) != ZCFG_SUCCESS) {
		return ret;
	}
	ztrdbg(ZTR_DL_DEBUG, "%s: iface = %s\n",  __FUNCTION__, iface);
	// PPP.Interface
	if(strstr(iface, "PPP")){
		//printf("PPP case\n");
		IID_INIT(objIid);
		while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &objIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
			if(strcmp(ipIface->LowerLayers, iface) == 0) {
				memset(iface, '\0', sizeof(iface));
				snprintf(iface, sizeof(iface), "IP.Interface.%d",  objIid.idx[0]);
				printf("match! new iface = %s\n", iface);
				zcfgFeObjStructFree(ipIface);
				break;
			}
			zcfgFeObjStructFree(ipIface);
		}
	}else{
		//IP.Interface
		IID_INIT(objIid);
		sscanf(iface, "IP.Interface.%hhu", &objIid.idx[0]);
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NULL)) != ZCFG_SUCCESS) {
		ztrdbg(ZTR_DL_DEBUG, "%s : Add NAT.X_ZYXEL_PortTriggering Fail.\n", __FUNCTION__);
		return ret;
	}

	if(zcfgFeObjStructGet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, (void **)&natPortTriggerObj) == ZCFG_SUCCESS) {
		ZOS_STRCPY_M(natPortTriggerObj->Interface, iface);
		ret = zcfgFeObjStructSet(RDM_OID_NAT_PORT_TRIGGERING, &objIid, (void *)natPortTriggerObj, NULL);
		if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
		{
			ztrdbg(ZTR_DL_DEBUG, "%s : Set NAT.X_ZYXEL_PortTriggering Fail.\n", __FUNCTION__);
		}
		ztrdbg(ZTR_DL_DEBUG, "%s: Set PortTriggering complete\n", __FUNCTION__);
		zcfgFeObjStructFree(natPortTriggerObj);

#ifdef ZCFG_DYNAMIC_NAMEMAPPING
		if((ret == ZCFG_DELAY_APPLY) || (ret == ZCFG_SUCCESS)){
			zcfg_name_t nameSeqNum = 0;
			objIndex_t tr98ObjIid;
			char tr181PathName[64] = {0};

			sprintf(tr181PathName, "NAT.X_ZYXEL_PortTriggering.%d", objIid.idx[0] );
		
			IID_INIT(tr98ObjIid);
			zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)tr181PathName, &nameSeqNum, &tr98ObjIid);
		
			if(IS_SEQNUM_RIGHT(nameSeqNum)) {
				ztrdbg(ZTR_DL_DEBUG, "%s: Added PortTriggering %d\n", __FUNCTION__, tr98ObjIid.idx[3]);
				*idx = tr98ObjIid.idx[3];
			}
		}
#endif
		return ret;
	}
	else {
		ztrdbg(ZTR_DL_DEBUG, "%s : Get NAT.X_ZYXEL_PortTriggering Fail.\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;	
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t WANPortTriggerDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	char portTrigger[MAX_TR181_PATHNAME] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, portTrigger)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(portTrigger, "NAT.X_ZYXEL_PortTriggering.%hhu", &objIid.idx[0]);

	ret = zcfgFeObjStructDel(RDM_OID_NAT_PORT_TRIGGERING, &objIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
	{
		printf("%s : Delete NAT.X_ZYXEL_PortTriggering Fail\n", __FUNCTION__);
	}

	return ret;
}

zcfgRet_t WANPortTriggerNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANPortTriggerAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[MAX_TR181_PATHNAME] = {0};
	char mapObj[MAX_TR181_PATHNAME] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	
	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANPortTriggerAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[MAX_TR181_PATHNAME] = {0};
	char mapObj[MAX_TR181_PATHNAME] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;
		
		attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;
	}
	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.PortMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.PortMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.PortMapping.i
 */
zcfgRet_t WANPortMappingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char natPortMap[MAX_TR181_PATHNAME] = {0};
	objIndex_t objIid;
	struct json_object *portMapJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, natPortMap)) != ZCFG_SUCCESS) {
		/*  The instance number of PortMapping.i will be continuous because of the tr98 to tr181 mapping table. 
		 *  Therefore, just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this PortMapping object.
		 */	
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(natPortMap, "NAT.PortMapping.%hhu", &objIid.idx[0]);

	if((ret = feObjJsonGet(RDM_OID_NAT_PORT_MAPPING, &objIid, &portMapJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(portMapJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "PortMappingEnabled")) { 
			paramValue = json_object_object_get(portMapJobj, "Enable");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}	
		else if(!strcmp(paramList->name, "PortMappingLeaseDuration")) {
			paramValue = json_object_object_get(portMapJobj, "LeaseDuration");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}	
		else if(!strcmp(paramList->name, "PortMappingProtocol")) {
			paramValue = json_object_object_get(portMapJobj, "Protocol");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "PortMappingDescription")) {
			paramValue = json_object_object_get(portMapJobj, "Description");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		else if(!strcmp(paramList->name, "X_ZYXEL_OriginatingIpAddress")) {
			paramValue = json_object_object_get(portMapJobj, "OriginatingIpAddress");
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

	json_object_put(portMapJobj);

	return ZCFG_SUCCESS;
}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
zcfgRet_t CheckSetPortValid(objIndex_t *objIid, struct json_object *newtr98portMapJobj)
{
	rdm_RemoSrv_t *remoSrvObj = NULL;
	objIndex_t remoSrvIid;
	rdm_MgmtSrv_t *MgmtSrvObj = NULL;
	objIndex_t mgmtSrvIid;
	bool portisvalid = true;
    int new_eEnd = 0,new_eStart = 0,new_iEnd = 0,new_iStart = 0;
    int cur_eEnd = 0,cur_eStart = 0,cur_iEnd = 0,cur_iStart = 0;
    int tempeEnd = 0,tempiEnd = 0;
	rdm_NatPortMapping_t *portMapObj = NULL;

            new_eEnd = Jgeti(newtr98portMapJobj,"ExternalPortEndRange");
			new_eStart = Jgeti(newtr98portMapJobj,"ExternalPort");
			new_iEnd = Jgeti(newtr98portMapJobj,"X_ZYXEL_InternalPortEndRange");
			new_iStart = Jgeti(newtr98portMapJobj,"InternalPort");

			if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, objIid, (void **)&portMapObj) == ZCFG_SUCCESS) {
				            cur_eEnd = portMapObj->ExternalPortEndRange;
							cur_eStart = portMapObj->ExternalPort;
							cur_iEnd = portMapObj->X_ZYXEL_InternalPortEndRange;
							cur_iStart = portMapObj->InternalPort;  
							zcfgFeObjStructFree(portMapObj);
			}

			if(new_eEnd == cur_eEnd){
				if(new_iEnd == cur_iEnd){
					new_iEnd = new_iStart;
					new_eEnd = new_eStart;
				}else{ 
					if(new_iEnd < new_iStart){
						new_iEnd = new_iStart;
						new_eEnd = new_eStart;
					}else{
					    tempeEnd = new_eStart + (new_iEnd - new_iStart);
					    new_eEnd = tempeEnd;
					}
				}
			}else if(new_iEnd == cur_iEnd){
				if(new_eEnd < new_eStart){
					new_iEnd = new_iStart;
					new_eEnd = new_eStart;
				}else{
					tempiEnd = new_iStart + (new_eEnd - new_eStart);
					new_iEnd = tempiEnd;
				}
			}else if((new_eEnd != cur_eEnd) && (new_iEnd != cur_iEnd)){
				if((new_eEnd - new_eStart) != (new_iEnd - new_iStart)){
					tempeEnd = new_eStart + (new_iEnd - new_iStart);
					new_eEnd = tempeEnd;
				}
			}

		if(new_eEnd != 0){
			if(new_iEnd == 0){
				new_iEnd = new_iStart + (new_eEnd - new_eStart);
			}
		}else if(new_iEnd != 0){
			if(new_eEnd == 0){
				new_eEnd = new_eStart + (new_iEnd - new_iStart);
			}
		}else if((new_iStart != 0) && (new_eStart != 0)){
			new_iEnd = new_iStart;
			new_eEnd = new_eStart;
		}

        IID_INIT(remoSrvIid);
	    while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_REMO_SRV, &remoSrvIid, (void **)&remoSrvObj) == ZCFG_SUCCESS){
				if(remoSrvObj->Port != 0)
				{
                    if(remoSrvObj->Port >= new_eStart && remoSrvObj->Port <= new_eEnd){
				           portisvalid = false;
						   zcfgFeObjStructFree(remoSrvObj);
						   break;
			        }

					if(remoSrvObj->Port >= new_iStart && remoSrvObj->Port <= new_iEnd){
				           portisvalid = false;
						   zcfgFeObjStructFree(remoSrvObj);
						   break;
			        }
				}
				zcfgFeObjStructFree(remoSrvObj);
	    }

        IID_INIT(mgmtSrvIid);
        if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_MGMT_SRV, &mgmtSrvIid, (void **)&MgmtSrvObj) == ZCFG_SUCCESS) {
			if(MgmtSrvObj->X_ZYXEL_ConnectionRequestPort != 0)
			{
				if(MgmtSrvObj->X_ZYXEL_ConnectionRequestPort >= new_eStart && MgmtSrvObj->X_ZYXEL_ConnectionRequestPort <= new_eEnd){
					   portisvalid = false;
				}
			
				if(MgmtSrvObj->X_ZYXEL_ConnectionRequestPort >= new_iStart && MgmtSrvObj->X_ZYXEL_ConnectionRequestPort <= new_iEnd){
					   portisvalid = false;
				}
			}
			zcfgFeObjStructFree(MgmtSrvObj);
        }

        if(!portisvalid){
		   return ZCFG_INVALID_PARAM_VALUE;
        }

        return ZCFG_SUCCESS;
}
#endif

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.PortMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.PortMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.PortMapping.i
 */
zcfgRet_t WANPortMappingSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *portMapJobj = NULL;
	struct json_object *tmpObj = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *paramValue = NULL;
	char portMap[MAX_TR181_PATHNAME] = {0};
	tr98Parameter_t *paramList = NULL;
#ifdef CONFIG_TAAAB_DSL_BINDING
		objIndex_t portMapobjIid;
		rdm_NatPortMapping_t *portMapObj = NULL;
		rdm_NatPortMapping_t *natPortMapObj = NULL;
		objIndex_t ipIfaceIid;
		rdm_IpIface_t *ipIfaceObj = NULL;
		objIndex_t referobjIid = {0};
		struct json_object *referportMapJobj = NULL;
		char New_SrvName[64] = {0};
		char New_TAAAB_SrvName[64] = {0};
		int newreferindex = -1;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, portMap)) != ZCFG_SUCCESS) {
		return ret;
	}

	printf("%s : portMap %s\n", __FUNCTION__, portMap);

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(portMap, "NAT.PortMapping.%hhu", &objIid.idx[0]);

#ifdef CONFIG_TAAAB_DSL_BINDING	   
								IID_INIT(portMapobjIid);
								portMapobjIid.level = 1;
								portMapobjIid.idx[0] = objIid.idx[0];
								if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &portMapobjIid, (void **)&portMapObj)) != ZCFG_SUCCESS) {
									return ret;
								}
	
								IID_INIT(ipIfaceIid);
								ipIfaceIid.level = 1;
								sscanf(portMapObj->Interface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
								if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceObj) == ZCFG_SUCCESS)
								{
									ZOS_STRCPY_M(New_SrvName,ipIfaceObj->X_ZYXEL_SrvName);
									ZOS_STRCPY_M(New_TAAAB_SrvName,ipIfaceObj->X_TAAAB_SrvName);
									zcfgFeObjStructFree(ipIfaceObj);
								}
								
						if(strstr(New_SrvName,"_ETHWAN") != NULL)
						{		
						   newreferindex = -1;
						}
						else if(strstr(New_SrvName,"_VDSL") != NULL)
						{
						   newreferindex = portMapObj->X_ZYXEL_REFERENCE;		
						}
						else if(strstr(New_SrvName,"_ADSL") != NULL)
						{
								IID_INIT(referobjIid);
								while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void **)&natPortMapObj) == ZCFG_SUCCESS) {
								if(natPortMapObj->X_ZYXEL_REFERENCE == portMapobjIid.idx[0] )
								{
										newreferindex = referobjIid.idx[0];
										zcfgFeObjStructFree(natPortMapObj);
										break;
								}
								zcfgFeObjStructFree(natPortMapObj);
								}
						}
	
						if(portMapObj)
							zcfgFeObjStructFree(portMapObj);
#endif

	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_MAPPING, &objIid, &portMapJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		ret = CheckSetPortValid(&objIid,tr98Jobj);

		if(ret != ZCFG_SUCCESS){
			json_object_put(portMapJobj);
			return ret;	
		}
#endif

	if(multiJobj){
		tmpObj = portMapJobj;
		portMapJobj = NULL;
		portMapJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_PORT_MAPPING, &objIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(portMapJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(portMapJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
			if(!strcmp(paramList->name, "PortMappingEnabled")) { 
				json_object_object_add(portMapJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingLeaseDuration")) {
				json_object_object_add(portMapJobj, "LeaseDuration", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingProtocol")) {
				json_object_object_add(portMapJobj, "Protocol", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingDescription")) {
				json_object_object_add(portMapJobj, "Description", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_OriginatingIpAddress")) {
				json_object_object_add(portMapJobj, "OriginatingIpAddress", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;	
	}

	/*Set Device.NAT.PortMapping.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &objIid, portMapJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(portMapJobj);
			return ret;
		}
		json_object_put(portMapJobj);
	}

//TAAAB sync entry
#ifdef CONFIG_TAAAB_DSL_BINDING
    if(newreferindex != -1)
    {
    IID_INIT(referobjIid);
	referobjIid.level = 1;
	referobjIid.idx[0] = newreferindex;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, &referportMapJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(multiJobj){
		tmpObj = referportMapJobj;
		referportMapJobj = NULL;
		referportMapJobj = zcfgFeJsonMultiObjAppend(RDM_OID_NAT_PORT_MAPPING, &referobjIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(referportMapJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(referportMapJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
			if(!strcmp(paramList->name, "PortMappingEnabled")) { 
				json_object_object_add(referportMapJobj, "Enable", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingLeaseDuration")) {
				json_object_object_add(referportMapJobj, "LeaseDuration", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingProtocol")) {
				json_object_object_add(referportMapJobj, "Protocol", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "PortMappingDescription")) {
				json_object_object_add(referportMapJobj, "Description", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
			else if(!strcmp(paramList->name, "X_ZYXEL_OriginatingIpAddress")) {
				json_object_object_add(referportMapJobj, "OriginatingIpAddress", JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;	
	}

	/*Set Device.NAT.PortMapping.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, referportMapJobj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(referportMapJobj);
			return ret;
		}
		json_object_put(referportMapJobj);
	}
    }
#endif

	return ZCFG_SUCCESS;
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.PortMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.PortMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.PortMapping.i
 */
zcfgRet_t WANPortMappingAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	rdm_NatPortMapping_t *natPortMapObj = NULL;
	char tr98ConnName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//char tr98PortMapName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char iface[MAX_TR181_PATHNAME] = {0};
	//char portMap[32] = {0};
	char *ptr = NULL;
	rdm_IpIface_t *ipIface = NULL;
	FILE *fp = NULL;
#ifdef CONFIG_TAAAB_DSL_BINDING
			objIndex_t ipIfaceIid;
			rdm_IpIface_t *ipIfaceObj = NULL;
			objIndex_t referobjIid = {0};
			char New_SrvName[64] = {0};
			char New_TAAAB_SrvName[64] = {0};
			char newreferinterface[64] = {0};
			int newreferindex = -1;
			int vdindex = -1;
#endif

	fp = fopen("/data/nat_wol.txt", "r");
	if(fp == NULL) {
		fp = fopen("/data/nat_wol.txt", "w");
	}
	if (fp != NULL)
	fclose(fp);

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98ConnName, tr98FullPathName);
	ptr = strstr(tr98ConnName, ".PortMapping");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnName, iface)) != ZCFG_SUCCESS) {
		return ret;
	}
	printf("iface = %s\n", iface);
	//if PPP.Interface
	if(strstr(iface, "PPP")){
		printf("PPP case\n");
		IID_INIT(objIid);
		while((ret = zcfgFeObjStructGetNext(RDM_OID_IP_IFACE, &objIid, (void **)&ipIface)) == ZCFG_SUCCESS) {
			if(strcmp(ipIface->LowerLayers, iface) == 0) {
				memset(iface, '\0', sizeof(iface));
				snprintf(iface, sizeof(iface), "IP.Interface.%d",  objIid.idx[0]);
				printf("match! new iface = %s\n", iface);
				zcfgFeObjStructFree(ipIface);
				break;
			}
			zcfgFeObjStructFree(ipIface);
		}
	}else{
		//IP.Interface
		IID_INIT(objIid);
		sscanf(iface, "IP.Interface.%hhu", &objIid.idx[0]);
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjStructAdd(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL)) != ZCFG_SUCCESS) {
		printf("%s : Add NAT.PortMapping Fail.\n", __FUNCTION__);
		return ret;
	}

#ifdef CONFIG_TAAAB_DSL_BINDING	   
						IID_INIT(ipIfaceIid);
						ipIfaceIid.level = 1;
						sscanf(iface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
						if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceObj) == ZCFG_SUCCESS)
						{
							ZOS_STRCPY_M(New_SrvName,ipIfaceObj->X_ZYXEL_SrvName);
							ZOS_STRCPY_M(New_TAAAB_SrvName,ipIfaceObj->X_TAAAB_SrvName);
							zcfgFeObjStructFree(ipIfaceObj);
						}
						
				if(strstr(New_SrvName,"_ETHWAN") != NULL)
				{		
				   newreferindex = -1;
				}
				else if(strstr(New_SrvName,"_VDSL") != NULL)
				{
				   IID_INIT(referobjIid);
				   ret = zcfgFeObjStructAdd(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);
				   if(ret == ZCFG_SUCCESS){
					   newreferindex = referobjIid.idx[0];
				   }			
				}
				else if(strstr(New_SrvName,"_ADSL") != NULL)
				{
				   IID_INIT(referobjIid);
				   ret = zcfgFeObjStructAdd(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);
				   if(ret == ZCFG_SUCCESS){
				   	   newreferindex = objIid.idx[0];
					   vdindex = referobjIid.idx[0];
				   }			
				}
#endif

	if(zcfgFeObjStructGet(RDM_OID_NAT_PORT_MAPPING, &objIid, (void **)&natPortMapObj) == ZCFG_SUCCESS) {
		ZOS_STRCPY_M(natPortMapObj->Interface, iface);
#ifdef CONFIG_TAAAB_DSL_BINDING
        if(strstr(New_SrvName,"_ETHWAN") != NULL){
		    natPortMapObj->X_ZYXEL_REFERENCE = -1;
            sprintf(natPortMapObj->Description, "acsaddeth_%d",objIid.idx[0]);
        }
		else if(strstr(New_SrvName,"_VDSL") != NULL){
			natPortMapObj->X_ZYXEL_REFERENCE = newreferindex;
			sprintf(natPortMapObj->Description, "acsaddptm_%d",objIid.idx[0]);
		}
		else if(strstr(New_SrvName,"_ADSL") != NULL){
			natPortMapObj->X_ZYXEL_REFERENCE = -1;
			sprintf(natPortMapObj->Description, "acsaddatm_%d",objIid.idx[0]);
		}
#endif
		ret = zcfgFeObjStructSet(RDM_OID_NAT_PORT_MAPPING, &objIid, (void *)natPortMapObj, NULL);
		if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
		{
			printf("%s : Set NAT.PortMapping Fail.\n", __FUNCTION__);
		}
        printf("%s: Set portmapping complete\n", __FUNCTION__);
		zcfgFeObjStructFree(natPortMapObj);
#if 0
		if(ret == ZCFG_SUCCESS)
		{
			sprintf(portMap, "NAT.PortMapping.%hhu", objIid.idx[0]);
		
			if((ret = zcfgFe181To98MappingNameGet(portMap, tr98PortMapName)) != ZCFG_SUCCESS) {
				return ret;
			}
		
			sscanf(tr98PortMapName, "%*[^.].%*[^.].%*d.%*[^.].%*d.%*[^.].%*d.PortMapping.%d", idx);
		}else
#endif
#ifdef ZCFG_DYNAMIC_NAMEMAPPING
		if((ret == ZCFG_DELAY_APPLY) || (ret == ZCFG_SUCCESS)){
			zcfg_name_t nameSeqNum = 0;
			objIndex_t tr98ObjIid;
			char tr181PathName[64] = {0};

			sprintf(tr181PathName, "NAT.PortMapping.%d", objIid.idx[0] );
		
			IID_INIT(tr98ObjIid);
			zcfgFe98To181ObjMappingNameAdd((const char *)tr98FullPathName, (const char *)tr181PathName, &nameSeqNum, &tr98ObjIid);
		
			if(IS_SEQNUM_RIGHT(nameSeqNum)) {
				printf("Added PortMapping %d\n", tr98ObjIid.idx[3]);
				*idx = tr98ObjIid.idx[3];
			}
		}
#endif
//TAAAB symc entry
#ifdef CONFIG_TAAAB_DSL_BINDING
						if(strstr(New_SrvName,"_VDSL") != NULL){
								IID_INIT(referobjIid);
								referobjIid.level = 1;
								referobjIid.idx[0] = newreferindex;
								zcfgFeObjStructGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void **)&natPortMapObj);
			
									IID_INIT(ipIfaceIid);
									while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceObj) == ZCFG_SUCCESS) {
										if(!strcmp(ipIfaceObj->X_TAAAB_SrvName,New_TAAAB_SrvName) && strcmp(ipIfaceObj->X_ZYXEL_SrvName,New_SrvName) && strstr(ipIfaceObj->X_ZYXEL_SrvName,"_ADSL"))
										{
												sprintf(newreferinterface, "IP.Interface.%u", ipIfaceIid.idx[0]);
												ZOS_STRCPY_M(natPortMapObj->Interface, newreferinterface);
												natPortMapObj->X_ZYXEL_REFERENCE = -1;
												zcfgFeObjStructFree(ipIfaceObj);
												break;
										}
										zcfgFeObjStructFree(ipIfaceObj);
									}
			
									sprintf(natPortMapObj->Description, "acsaddatm_%d",referobjIid.idx[0]);

									ret = zcfgFeObjStructSet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void *)natPortMapObj, NULL);
									if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
									{
										printf("%s : Set ATM NAT.PortMapping Fail.\n", __FUNCTION__);
									}
									printf("%s: Set ATM portmapping complete\n", __FUNCTION__);
									zcfgFeObjStructFree(natPortMapObj);
						}	
						else if(strstr(New_SrvName,"_ADSL") != NULL){
								IID_INIT(referobjIid);
								referobjIid.level = 1;
								referobjIid.idx[0] = vdindex;
								zcfgFeObjStructGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void **)&natPortMapObj);
			
									IID_INIT(ipIfaceIid);
									while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceObj) == ZCFG_SUCCESS) {
										if(!strcmp(ipIfaceObj->X_TAAAB_SrvName,New_TAAAB_SrvName) && strcmp(ipIfaceObj->X_ZYXEL_SrvName,New_SrvName) && strstr(ipIfaceObj->X_ZYXEL_SrvName,"_VDSL"))
										{
												sprintf(newreferinterface, "IP.Interface.%u", ipIfaceIid.idx[0]);
												ZOS_STRCPY_M(natPortMapObj->Interface, newreferinterface);
												natPortMapObj->X_ZYXEL_REFERENCE = newreferindex;
												zcfgFeObjStructFree(ipIfaceObj);
												break;
										}
										zcfgFeObjStructFree(ipIfaceObj);
									}
			
									sprintf(natPortMapObj->Description, "acsaddptm_%d",referobjIid.idx[0]);

									ret = zcfgFeObjStructSet(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void *)natPortMapObj, NULL);
									if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
									{
										printf("%s : Set PTM NAT.PortMapping Fail.\n", __FUNCTION__);
									}
									printf("%s: Set PTM portmapping complete\n", __FUNCTION__);
									zcfgFeObjStructFree(natPortMapObj);
						}
#endif

		return ret;
	}
	else {
		printf("%s : Get NAT.PortMapping Fail.\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;	
	}

	return ZCFG_SUCCESS;
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.PortMapping.i
 *                      InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.PortMapping.i
 *
 *   Related object in TR181:
 *   Device.NAT.PortMapping.i
 */
zcfgRet_t WANPortMappingDel(char *tr98FullPathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	char portMap[MAX_TR181_PATHNAME] = {0};
#ifdef CONFIG_TAAAB_DSL_BINDING
    objIndex_t portMapobjIid;
	rdm_NatPortMapping_t *portMapObj = NULL;
    rdm_NatPortMapping_t *natPortMapObj = NULL;
	objIndex_t ipIfaceIid;
	rdm_IpIface_t *ipIfaceObj = NULL;
	objIndex_t referobjIid = {0};
	char New_SrvName[64] = {0};
	char New_TAAAB_SrvName[64] = {0};
	int newreferindex = -1;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, portMap)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(portMap, "NAT.PortMapping.%hhu", &objIid.idx[0]);

#ifdef CONFIG_TAAAB_DSL_BINDING	   
                            IID_INIT(portMapobjIid);
							portMapobjIid.level = 1;
                            portMapobjIid.idx[0] = objIid.idx[0];
							if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &portMapobjIid, (void **)&portMapObj)) != ZCFG_SUCCESS) {
								return ret;
							}

							IID_INIT(ipIfaceIid);
							ipIfaceIid.level = 1;
							sscanf(portMapObj->Interface, "IP.Interface.%hhu", &ipIfaceIid.idx[0]);
							if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceObj) == ZCFG_SUCCESS)
							{
								ZOS_STRCPY_M(New_SrvName,ipIfaceObj->X_ZYXEL_SrvName);
								ZOS_STRCPY_M(New_TAAAB_SrvName,ipIfaceObj->X_TAAAB_SrvName);
								zcfgFeObjStructFree(ipIfaceObj);
							}
							
					if(strstr(New_SrvName,"_ETHWAN") != NULL)
					{		
					   newreferindex = -1;
					}
					else if(strstr(New_SrvName,"_VDSL") != NULL)
					{
					   newreferindex = portMapObj->X_ZYXEL_REFERENCE;		
					}
					else if(strstr(New_SrvName,"_ADSL") != NULL)
					{
							IID_INIT(referobjIid);
							while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_NAT_PORT_MAPPING, &referobjIid, (void **)&natPortMapObj) == ZCFG_SUCCESS) {
							if(natPortMapObj->X_ZYXEL_REFERENCE == portMapobjIid.idx[0] )
							{
									newreferindex = referobjIid.idx[0];
									zcfgFeObjStructFree(natPortMapObj);
									break;
							}
							zcfgFeObjStructFree(natPortMapObj);
						    }
					}

					if(portMapObj)
						zcfgFeObjStructFree(portMapObj);
#endif

	ret = zcfgFeObjStructDel(RDM_OID_NAT_PORT_MAPPING, &objIid, NULL);
	if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
	{
		printf("%s : Delete NAT.PortMapping Fail\n", __FUNCTION__);
	}

#ifdef CONFIG_TAAAB_DSL_BINDING	
    if(newreferindex != -1){
		IID_INIT(referobjIid);
		referobjIid.level = 1;
        referobjIid.idx[0] = newreferindex;
		ret = zcfgFeObjStructDel(RDM_OID_NAT_PORT_MAPPING, &referobjIid, NULL);
		if((ret != ZCFG_DELAY_APPLY) && (ret != ZCFG_SUCCESS))
		{
			printf("%s : Delete NAT.PortMapping Fail\n", __FUNCTION__);
		}
    }	
#endif

	return ret;
}

zcfgRet_t WANPortMappingNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char tr98Notify[256] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, paramList->name);
			break;
		}
		
		if(strcmp(tr181ParamName, "Enable") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "PortMappingEnabled");
			break;
		}

		if(strcmp(tr181ParamName, "LeaseDuration") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "PortMappingLeaseDuration");
			break;
		}
		
		if(strcmp(tr181ParamName, "Protocol") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "PortMappingProtocol");
			break;
		}
		
		if(strcmp(tr181ParamName, "Description") == 0) {
			found = true;
			sprintf(tr98Notify, "%s.%s", tr98ObjName, "PortMappingDescription");
			break;
		}
		
		paramList++;
	}

	if(found) {
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANPortMappingAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[MAX_TR181_PATHNAME] = {0};
	char mapObj[MAX_TR181_PATHNAME] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}
		
		/*special case*/
		if(strcmp(paramList->name, "PortMappingEnabled") == 0){
            attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "PortMappingLeaseDuration") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "LeaseDuration");
		}else if(strcmp(paramList->name, "PortMappingProtocol") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Protocol");
		}else if(strcmp(paramList->name, "PortMappingDescription") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Description");
		}else if(strcmp(paramList->name, "X_ZYXEL_OriginatingIpAddress") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "OriginatingIpAddress");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANPortMappingAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[MAX_TR181_PATHNAME] = {0};
	char mapObj[MAX_TR181_PATHNAME] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		/*special case*/
		if(strcmp(paramList->name, "PortMappingEnabled") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Enable");
		}else if(strcmp(paramList->name, "PortMappingLeaseDuration") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "LeaseDuration");
		}else if(strcmp(paramList->name, "PortMappingProtocol") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Protocol");
		}else if(strcmp(paramList->name, "PortMappingDescription") == 0){
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, "Description");
		}else{
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if(strcmp(paramList->name, "PortMappingEnabled") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Enable", attrValue);
		}else if(strcmp(paramList->name, "PortMappingLeaseDuration") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "LeaseDuration", attrValue);
		}else if(strcmp(paramList->name, "PortMappingProtocol") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Protocol", attrValue);
		}else if(strcmp(paramList->name, "PortMappingDescription") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "Description", attrValue);
		}else if(strcmp(paramList->name, "X_ZYXEL_OriginatingIpAddress") == 0){
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, "OriginatingIpAddress", attrValue);
		}else{
			ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramList->name, attrValue);
		}
		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramList->name);
		}
		break;

	} /*Edn while*/
	
	return ret;
}


/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.Stats
 *			or InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.Stats
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.Stats
 *   or Device.PPP.Interface.i.Stats
 */
zcfgRet_t WANConnStObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char iface[32] = {0};
	objIndex_t objIid;
	struct json_object *ifaceStJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	uint32_t oid;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".Stats");
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, iface)) == ZCFG_SUCCESS) {
		IID_INIT(objIid);
		objIid.level = 1;

		if(strstr(iface, "IP.Interface") != NULL) {
			sscanf(iface, "IP.Interface.%hhu", &objIid.idx[0]);
			oid = RDM_OID_IP_IFACE_STAT;
		}
		else {
			sscanf(iface, "PPP.Interface.%hhu", &objIid.idx[0]);	
			oid = RDM_OID_PPP_IFACE_STAT;
		}

		if((ret = feObjJsonGet(oid, &objIid, &ifaceStJobj, updateFlag)) != ZCFG_SUCCESS)
			return ret;

		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			ptr = paramList->name + strlen("Ethernet");
			paramValue = json_object_object_get(ifaceStJobj, ptr);	
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*Not defined in tr181, give it a default value*/

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;		
		}
		
		json_object_put(ifaceStJobj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t WANConnStObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char *attrParamNamePtr = NULL;
	
	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, tr181ParamName)){
			paramList++;
			continue;
		}
		attrParamNamePtr = tr181ParamName;
		break;
	}

	if(attrParamNamePtr && strlen(attrParamNamePtr) > 0) {
		sprintf(tr98Notify, "%s.Ethernet%s", tr98ObjName, attrParamNamePtr);

		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

int WANConnStObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;
	uint32_t oid = 0;
	int tr181ParamPos = 0;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(strstr(tr98FullPathName, "WANIP") != NULL) {
		oid = RDM_OID_IP_IFACE_STAT;
	}
	else if(strstr(tr98FullPathName, "WANPPP") != NULL) {
		oid = RDM_OID_PPP_IFACE_STAT;
	}
	else
		return ZCFG_INVALID_OBJECT;

	tr181ParamPos = strlen("Ethernet");
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		char *paramPtr = NULL;
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		paramPtr = paramName;
		paramPtr += tr181ParamPos;

		attrValue = zcfgFeParamAttrGetByName(oid, paramPtr);

		if(attrValue < 0 ) {
			attrValue = 0;
		}
		break;
	}
	
	return attrValue;
}

zcfgRet_t WANConnStObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	uint32_t oid = 0;
	tr98Parameter_t *paramList = NULL;
	char *paramPtr = NULL;
	int tr181ParamPos = 0;

	zcfgLog(ZCFG_LOG_INFO, "Enter %s\n", __FUNCTION__);

	if(strstr(tr98FullPathName, "WANIP") != NULL) {
		oid = RDM_OID_IP_IFACE_STAT;
	}
	else if(strstr(tr98FullPathName, "WANPPP") != NULL) {
		oid = RDM_OID_PPP_IFACE_STAT;
	}
	else
		return ZCFG_INVALID_OBJECT;

	tr181ParamPos = strlen("Ethernet");
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, newNotify)) != ZCFG_SUCCESS)
			return ret;

		paramPtr = paramName;
		paramPtr += tr181ParamPos;

		attrValue = zcfgFeParamAttrGetByName(oid, paramPtr);

		break;
	}

	if(attrValue >= 0) {
		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		ret = zcfgFeMultiParamAttrAppend(oid, multiAttrJobj, paramPtr, attrValue);

		if( ret != ZCFG_SUCCESS){
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, oid, paramPtr);
		}
	}
	else {
		ret = ZCFG_INVALID_ARGUMENTS;
	}

	return ret;
}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANATMF5LoopbackDiagnostics

    Related object in TR181:
    Device.ATM.Diagnostics.F5Loopback.
 */
zcfgRet_t WANAtmF5LoConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char atmLink[24] = {0};
	objIndex_t atmF5LoIid;
	struct json_object *atmF5LoJobj = NULL;
	struct json_object *paramValue = NULL;
	// struct json_object *ifValue = NULL;
	tr98Parameter_t *paramList = NULL;	
	
	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANATMF5LoopbackDiagnostics");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(strstr(atmLink, "ATM") == NULL) {
		printf("%s : Not a ATM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}


	IID_INIT(atmF5LoIid);
	if((ret = feObjJsonGet(RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, &atmF5LoJobj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}


	/*fill up tr98 WANATMF5LoopbackDiagnostics object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	// ifValue = json_object_object_get(atmF5LoJobj, "Interface");

	while(paramList->name != NULL) {
		paramValue = json_object_object_get(atmF5LoJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
			
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

		
	json_object_put(atmF5LoJobj);

	return ret;
}

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANATMF5LoopbackDiagnostics

    Related object in TR181:
    Device.ATM.Diagnostics.F5Loopback.
 */
zcfgRet_t WANAtmF5LoConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char atmLink[32] = {0};
	objIndex_t atmF5LoIid;
	struct json_object *atmF5LoJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *paramDiagStatValueObj = NULL;
	char runningDiagStat[30] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".WANATMF5LoopbackDiagnostics");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}
#if 0
	if(strstr(atmLink, "ATM") == NULL) {
		printf("%s : Not a ATM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}
#endif
	struct json_object *atmLinkObj;
	objIndex_t atmLinkIid;
	IID_INIT(atmLinkIid);
	atmLinkIid.level = 1;
	if(sscanf(atmLink, "ATM.Link.%hhu", &atmLinkIid.idx[0]) != 1){
		return ZCFG_NO_SUCH_OBJECT;
	}
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, (void **)&atmLinkObj) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	bool atmLinkStatus = 0;
	const char *atmLinkStatusStr = json_object_get_string(json_object_object_get(atmLinkObj, "Status"));
	if(!strcmp(atmLinkStatusStr, TR181_UP)){
		atmLinkStatus = 1;
	}
	json_object_put(atmLinkObj);

	/*Get Device.ATM.Diagnostics.F5Loopback*/
	IID_INIT(atmF5LoIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, &atmF5LoJobj)) != ZCFG_SUCCESS) {
		return ret;
	}
	const char *tr181DiagStatStr = json_object_get_string(json_object_object_get(atmF5LoJobj, "DiagnosticsState"));
	if(tr181DiagStatStr && strlen(tr181DiagStatStr))
		ZOS_STRCPY_M(runningDiagStat, tr181DiagStatStr);
	json_object_put(atmF5LoJobj);

	atmF5LoJobj = json_object_new_object();

	if(multiJobj){
		multiJobj = zcfgFeSessMultiObjSetUtilize(multiJobj);

		tmpObj = atmF5LoJobj;
		atmF5LoJobj = NULL;
		atmF5LoJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, "partialset", json_object_new_boolean(true));
	}
		
	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	/*write tr181 Device.ATM.Diagnostics.F5Loopback  object*/
	paramList = tr98Obj[handler].parameter;	
	json_object_object_add(atmF5LoJobj, "Interface", json_object_new_string(atmLink));
 	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANATMF5LoopbackDiagnostics to Device.ATM.Diagnostics.F5Loopback*/
			if(!strcmp(paramList->name, "DiagnosticsState")){
				paramDiagStatValueObj = paramValue;
			}
			json_object_object_add(atmF5LoJobj, paramList->name, JSON_OBJ_COPY(paramValue));

			/*Not defined in tr181, give it a default value*/
			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;
		}
		else {
			paramList++;
		}
	}

 	const char *diagnosticsState = json_object_get_string(paramDiagStatValueObj);
	if(diagnosticsState){
		if(!strcmp(diagnosticsState, "Requested") && !atmLinkStatus){
			sprintf(paramfault, "ATM Link not ready");
			if(!multiJobj) json_object_put(atmF5LoJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}else{
		if(strcmp(runningDiagStat, "Requested"))
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, "apply", json_object_new_boolean(false));
	}
	json_object_object_add(atmF5LoJobj, "X_ZYXEL_Creator", json_object_new_string("ACS"));
		
	if(multiJobj == NULL){
		//if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, atmF5LoJobj, NULL)) != ZCFG_SUCCESS ) {
		const char *atmdiagobjstr;
		atmdiagobjstr = json_object_to_json_string(atmF5LoJobj);
		if((ret = zcfgFeObjWrite(RDM_OID_ATM_DIAG_F5_LO, &atmF5LoIid, atmdiagobjstr, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Diagnostics.F5Loopback fail\n", __FUNCTION__);
			json_object_put(atmF5LoJobj);
			return ret;
		}

		json_object_put(atmF5LoJobj);
	}

	return ZCFG_SUCCESS;
}


/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WANATMF4LoopbackDiagnostics

    Related object in TR181:
    Device.ATM.Diagnostics.X_ZYXEL_F4Loopback.
 */
zcfgRet_t WANAtmF4LoConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char atmLink[24] = {0};
	objIndex_t atmF4LoIid;
	struct json_object *atmF4LoJobj = NULL;
	struct json_object *paramValue = NULL;
	// struct json_object *ifValue = NULL;
	tr98Parameter_t *paramList = NULL;	
	
	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".X_ZYXEL_WANATMF4LoopbackDiagnostics");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

	if(strstr(atmLink, "ATM") == NULL) {
		printf("%s : Not a ATM Link\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}


	IID_INIT(atmF4LoIid);
	if((ret = feObjJsonGet(RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, &atmF4LoJobj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}


	/*fill up tr98 WANATMF4LoopbackDiagnostics object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	// ifValue = json_object_object_get(atmF4LoJobj, "Interface");

	while(paramList->name != NULL) {
		paramValue = json_object_object_get(atmF4LoJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
			
		/*Not defined in tr181, give it a default value*/

		ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

		paramList++;
	}	

		
	json_object_put(atmF4LoJobj);

	return ret;
}

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WANATMF4LoopbackDiagnostics

    Related object in TR181:
    Device.ATM.Diagnostics.X_ZYXEL_F4Loopback.
 */
zcfgRet_t WANAtmF4LoConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char atmLink[32] = {0};
	objIndex_t atmF4LoIid;
	struct json_object *atmF4LoJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;
	struct json_object *paramDiagStatValueObj = NULL;
	char runningDiagStat[30] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".X_ZYXEL_WANATMF4LoopbackDiagnostics");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, atmLink)) != ZCFG_SUCCESS) {
		return ret;
	}

	struct json_object *atmLinkObj;
	objIndex_t atmLinkIid;
	IID_INIT(atmLinkIid);
	atmLinkIid.level = 1;
	if(sscanf(atmLink, "ATM.Link.%hhu", &atmLinkIid.idx[0]) != 1){
		return ZCFG_NO_SUCH_OBJECT;
	}
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_LINK, &atmLinkIid, &atmLinkObj) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}
	bool atmLinkStatus = 0;
	const char *atmLinkStatusStr = json_object_get_string(json_object_object_get(atmLinkObj, "Status"));
	if(!strcmp(atmLinkStatusStr, TR181_UP)){
		atmLinkStatus = 1;
	}
	json_object_put(atmLinkObj);

	/*Get Device.ATM.Diagnostics.F4Loopback*/
	IID_INIT(atmF4LoIid);
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, &atmF4LoJobj)) != ZCFG_SUCCESS) {
		return ret;
	}
	const char *tr181DiagStatStr = json_object_get_string(json_object_object_get(atmF4LoJobj, "DiagnosticsState"));
	if(tr181DiagStatStr && strlen(tr181DiagStatStr))
		ZOS_STRCPY_M(runningDiagStat, tr181DiagStatStr);
	json_object_put(atmF4LoJobj);

	atmF4LoJobj = json_object_new_object();

	if(multiJobj){
		multiJobj = zcfgFeSessMultiObjSetUtilize(multiJobj);

		tmpObj = atmF4LoJobj;
		atmF4LoJobj = NULL;
		atmF4LoJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, multiJobj, tmpObj);
		json_object_put(tmpObj);
		zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, "partialset", json_object_new_boolean(true));
	}
		
	tr98Jobj = zcfgFeRetrieveSpv(tr98FullPathName);

	/*write tr181 Device.ATM.Diagnostics.F4Loopback  object*/
	paramList = tr98Obj[handler].parameter;	
	json_object_object_add(atmF4LoJobj, "Interface", json_object_new_string(atmLink));
 	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANATMF4LoopbackDiagnostics to Device.ATM.Diagnostics.F4Loopback*/
			if(!strcmp(paramList->name, "DiagnosticsState")){
				paramDiagStatValueObj = paramValue;
			}
			json_object_object_add(atmF4LoJobj, paramList->name, JSON_OBJ_COPY(paramValue));
#if 0
			tr181ParamValue = json_object_object_get(atmF4LoJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(atmF4LoJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
#endif
  
			/*Not defined in tr181, give it a default value*/

			ztrdbg(ZTR_DL_INFO, "%s: Can't find parameter %s in TR181\n", __FUNCTION__, paramList->name);

			paramList++;
		}
		else {
			paramList++;
		}
	}   
 
 	const char *diagnosticsState = json_object_get_string(paramDiagStatValueObj);
	if(diagnosticsState){
		if(!strcmp(diagnosticsState, "Requested") && !atmLinkStatus){
			sprintf(paramfault, "ATM Link not ready");
			if(!multiJobj) json_object_put(atmF4LoJobj);
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}else{
		if(strcmp(runningDiagStat, "Requested"))
			zcfgFeJsonMultiObjAddParam(multiJobj, RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, "apply", json_object_new_boolean(false));
	}

		
	if(multiJobj == NULL){
		//if((ret = zcfgFeObjJsonSet(RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, atmF4LoJobj, NULL)) != ZCFG_SUCCESS ) {
		const char *atmdiagobjstr;
		atmdiagobjstr = json_object_to_json_string(atmF4LoJobj);
		if((ret = zcfgFeObjWrite(RDM_OID_ATM_DIAG_F4_LO, &atmF4LoIid, atmdiagobjstr, REQUEST_SET_OBJ | ZCFG_PARTIAL_OBJECT_SET, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.ATM.Diagnostics.X_ZYXEL_F4Loopback fail\n", __FUNCTION__);
			json_object_put(atmF4LoJobj);
			return ret;
		}

		json_object_put(atmF4LoJobj);
	}

	return ZCFG_SUCCESS;
}
#endif

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 * 	 Device.Routing.Router.i.IPv6Forwarding.i
 */
zcfgRet_t WANIpv6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t prefixInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);

	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;
	
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6AddrJobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "Prefix") == 0){//transfer
				ZOS_STRCPY_M(prefixObjName, json_object_get_string(paramValue));
				ptr = strstr(prefixObjName, ".IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe181To98MappingNameGet(prefixObjName, tr98ConnObjName) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix),"%s.X_ZYXEL_IPv6Prefix.%d", tr98ConnObjName, prefixInstance);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		else if(!strcmp(paramList->name, "NextHop")) {
			nextHopInfoSet(ipIface, *tr98Jobj);
			paramList++;
			continue;
		}
		paramList++;
	}

	json_object_put(ipv6AddrJobj);

	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Address.i.
 * 	 Device.Routing.Router.i.IPv6Forwarding.i
 */

zcfgRet_t WANIpv6AddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	objIndex_t ipv6FwdIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *ipv6FwdJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t prefixInstance = 0;
	rdm_RoutingRouterV6Fwd_t *v6FwdObj = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}
	
	IID_INIT(ipv6FwdIid);
	while(zcfgFeObjStructGetNextWithoutUpdate(RDM_OID_ROUTING_ROUTER_V6_FWD, &ipv6FwdIid, (void **)&v6FwdObj) == ZCFG_SUCCESS) {
		if(strstr(ipIface, v6FwdObj->Interface)) {
			zcfgFeObjStructFree(v6FwdObj);
			break;
		}
		zcfgFeObjStructFree(v6FwdObj);
	}
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_ROUTING_ROUTER_V6_FWD, &ipv6FwdIid, &ipv6FwdJobj)) != ZCFG_SUCCESS)
		return ret;
	
	if(multiJobj){
		if(ipv6FwdJobj){
			tmpObj = ipv6FwdJobj;
			ipv6FwdJobj = NULL;
			ipv6FwdJobj = zcfgFeJsonMultiObjAppend(RDM_OID_ROUTING_ROUTER_V6_FWD, &ipv6FwdIid, multiJobj, tmpObj);
			json_object_put(tmpObj);
		}
		
		tmpObj = ipv6AddrJobj;
		ipv6AddrJobj = NULL;
		ipv6AddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "Prefix") == 0){//transfer
				ZOS_STRCPY_M(prefixObjName, json_object_get_string(paramValue));
				ptr = strstr(prefixObjName, ".X_ZYXEL_IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe98To181MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(ipv6AddrJobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}
			else{
				/*Write value from WANIPConnection to Device.IP.Interface.i*/
				tr181ParamValue = json_object_object_get(ipv6AddrJobj, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(ipv6AddrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
			/*special case*/
			if(!strcmp(paramList->name, "NextHop")){
				if(ipv6FwdJobj){
					printf("set NextHop\n");
					json_object_object_add(ipv6FwdJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, ipv6AddrJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
			json_object_put(ipv6AddrJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		}
		json_object_put(ipv6AddrJobj);
		
		if(ipv6FwdJobj){
			if((ret = zcfgFeObjJsonSet(RDM_OID_ROUTING_ROUTER_V6_FWD, &ipv6FwdIid, ipv6FwdJobj, NULL)) != ZCFG_SUCCESS ) {
				zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.Routing.Router.i.IPv6Forwarding.i Fail\n", __FUNCTION__);
			}
			else {
				zcfgLog(ZCFG_LOG_DEBUG, "%s : Set Device.Routing.Router.i.IPv6Forwarding.i Success\n", __FUNCTION__);
			}
			json_object_put(ipv6FwdJobj);
		}
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t WANIpv6AddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}


	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_ADDR, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Address Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t WANIpv6AddrObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char ipIface[32] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}


	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_ADDR, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANIpv6PrefixObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);

	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;
	
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6PrefixJobj);

	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */

zcfgRet_t WANIpv6PrefixObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}


	if(multiJobj){
		tmpObj = ipv6PrefixJobj;
		ipv6PrefixJobj = NULL;
		ipv6PrefixJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6PrefixJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, ipv6PrefixJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
			json_object_put(ipv6PrefixJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		}
		json_object_put(ipv6PrefixJobj);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANIpv6PrefixObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}


	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_PREFIX, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Prefix Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];
	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANIpv6PrefixObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char ipIface[32] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98FullPathName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_DHCPv6Client.
 *
 *   Related object in TR181:
 *   Device.DHCPv6.Client.i.
 */
zcfgRet_t WANIpv6DhcpV6ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6ClientIid;
	struct json_object *ipv6ClientJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Client");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}

	IID_INIT(ipv6ClientIid);
	while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, &ipv6ClientJobj, updateFlag)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6ClientJobj, "Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6ClientJobj);
	}

	if(!found)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	
	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6ClientJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6ClientJobj);
	
	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_DHCPv6Client.
 *
 *   Related object in TR181:
 *   Device.IPv6rd.
 *   Device.IPv6rd.InterfaceSetting.i.
 */

zcfgRet_t WANIpv6DhcpV6ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6ClientIid;
	struct json_object *ipv6ClientJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Client");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6ClientIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, &ipv6ClientJobj)) == ZCFG_SUCCESS) {
		if(strcmp(ipIface, json_object_get_string(json_object_object_get(ipv6ClientJobj, "Interface")))==0){
			found = 1;
			break;
		}
		else
			json_object_put(ipv6ClientJobj);
	}

	if(!found )
		return ret;

	if(multiJobj){
		tmpObj = ipv6ClientJobj;
		ipv6ClientJobj = NULL;
		ipv6ClientJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6ClientJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6ClientJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, ipv6ClientJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Client.%d Fail\n", __FUNCTION__, ipv6ClientIid.idx[0]);
			json_object_put(ipv6ClientJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Client.%d Fail\n", __FUNCTION__, ipv6ClientIid.idx[0]);
		}
		json_object_put(ipv6ClientJobj);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6rd.
 *
 *   Related object in TR181:
 *   Device.IPv6rd.
 *   Device.IPv6rd.InterfaceSetting.i.
 */
#ifdef SAAAD_TR98_CUSTOMIZATION // SAAAD
zcfgRet_t WANIpv6RdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
//	struct json_object *ipv6RdJobj = NULL;
	struct json_object *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;
	
        ztrdbg(ZTR_SDL_DEBUG, "%s : Enter\n", __FUNCTION__);
	printf("98 full path = %s \n", tr98FullPathName);

	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		return ZCFG_NO_MORE_INSTANCE;
	}

	IID_INIT(ipv6RdIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6RdIid.idx[0]);
	ipv6RdIid.level = 1;

	//get the 6rd related parameter in the IpIface, because the 6rd backend is triggered by these parameters
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE, &ipv6RdIid, &ipIfaceJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface Fail\n", __FUNCTION__);
		return ret;
	}

	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipIfaceJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
	}
	json_object_put(ipIfaceJobj);
	return ZCFG_SUCCESS;	
}
#else
zcfgRet_t WANIpv6RdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
	struct json_object *ipv6RdJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	IID_INIT(ipv6RdIid);
	while((ret = feObjJsonGetNext(RDM_OID_IPV6RD_INTF, &ipv6RdIid, &ipv6RdJobj, updateFlag)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6RdJobj, "X_ZYXEL_Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6RdJobj);
	}
	if(!found)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	
	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6RdJobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "AddressSource") == 0){//transfer
				ZOS_STRCPY_M(addrsrcObjName, json_object_get_string(paramValue));
				ptr = strstr(addrsrcObjName, ".IPv4Address");
				if(ptr){
					sscanf(ptr, ".IPv4Address.%hhu", &addrsrcInstance);
					*ptr = '\0';
					if(zcfgFe181To98MappingNameGet(addrsrcObjName, tr98ConnObjName) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv4Address.%d", tr98ConnObjName, addrsrcInstance);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}			
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6RdJobj);
	
	return ZCFG_SUCCESS;	
}
#endif
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6rd.
 *
 *   Related object in TR181:
 *   Device.DHCPv6.Client.i.
 */

#ifdef SAAAD_TR98_CUSTOMIZATION // SAAAD
zcfgRet_t WANIpv6RdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
	struct json_object *ipv6RdJobj = NULL;
	struct json_object *ipIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	printf("!!!ipiface = %s\n", ipIface);

	IID_INIT(ipv6RdIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6RdIid.idx[0]);
	ipv6RdIid.level = 1;

	//get the 6rd related parameter in the IpIface, because the 6rd backend is triggered by these parameters
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE, &ipv6RdIid, &ipIfaceJobj)) != ZCFG_SUCCESS){
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface Fail\n", __FUNCTION__);
		return ret;
	}

	if(multiJobj){
		tmpObj = ipIfaceJobj;
		ipIfaceJobj = NULL;
		ipIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE, &ipv6RdIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipIfaceJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(!strcmp(paramList->name, "X_ZYXEL_Enable_6RD")) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_Enable_6RD", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
				if(!strcmp(paramList->name, "X_ZYXEL_6RD_Type")) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_6RD_Type", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
				if(!strcmp(paramList->name, "X_ZYXEL_BorderRelayIPv4Addresses")) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_BorderRelayIPv4Addresses", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
				if(!strcmp(paramList->name, "X_ZYXEL_SPIPv6Prefix")) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_SPIPv6Prefix", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
				if(!strcmp(paramList->name, "X_ZYXEL_IPv4MaskLength")) {
					json_object_object_add(ipIfaceJobj, "X_ZYXEL_IPv4MaskLength", JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
		}
		paramList++;	
	}

	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE, &ipv6RdIid, ipIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
		}
		json_object_put(ipIfaceJobj);
	}

	return ret;
}
#else
zcfgRet_t WANIpv6RdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
	struct json_object *ipv6RdJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(ipv6RdIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_IPV6RD_INTF, &ipv6RdIid, &ipv6RdJobj)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6RdJobj, "X_ZYXEL_Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6RdJobj);
	}


	if(!found )
		return ret;

	if(multiJobj){
		tmpObj = ipv6RdJobj;
		ipv6RdJobj = NULL;
		ipv6RdJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IPV6RD_INTF, &ipv6RdIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6RdJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(strcmp( paramList->name, "AddressSource") == 0){//transfer
					ZOS_STRCPY_M(addrsrcObjName, json_object_get_string(paramValue));
					ptr = strstr(addrsrcObjName, ".IPv4Address");
					if(ptr){
						sscanf(ptr, ".IPv4Address.%hhu", &addrsrcInstance);
						*ptr = '\0';
						if(zcfgFe98To181MappingNameGet(addrsrcObjName, ipIface) == ZCFG_SUCCESS) {
							snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv4Address.%d", ipIface, addrsrcInstance);
							json_object_object_add(ipv6RdJobj, paramList->name, json_object_new_string(tr98Prefix));
							paramList++;
							continue;
						}
					}
				}	

				json_object_object_add(ipv6RdJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IPV6RD_INTF, &ipv6RdIid, ipv6RdJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
			json_object_put(ipv6RdJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
		}
		json_object_put(ipv6RdJobj);
	}

	return ret;
}
#endif

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t WANPppv6AddrObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	//char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	//char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	//uint8_t prefixInstance = 0;
	char ipv6AddrName[MAX_TR181_PATHNAME] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
#if 0
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);

	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;
	
	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;
#endif

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipv6AddrName) != ZCFG_SUCCESS)
		return ZCFG_NO_MORE_INSTANCE;

	IID_INIT(ipv6AddrIid);
	ipv6AddrIid.level = 2;
	if(sscanf(ipv6AddrName, "IP.Interface.%hhu.IPv6Address.%hhu", &ipv6AddrIid.idx[0], &ipv6AddrIid.idx[1]) != ipv6AddrIid.level)
		return ZCFG_NO_MORE_INSTANCE;

	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6AddrJobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "Prefix") == 0 ){//transfer
				if(json_object_get_string(paramValue))
					ZOS_STRCPY_M(prefixObjName, json_object_get_string(paramValue));
				if(strlen(prefixObjName) && zcfgFe181To98MappingNameGet(prefixObjName, tr98Prefix) == ZCFG_SUCCESS){
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
				}
				paramList++;
				continue;
#if 0
				ptr = strstr(prefixObjName, ".IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe181To98MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix),"%s.X_ZYXEL_IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
#endif
			}
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6AddrJobj);

	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */

zcfgRet_t WANPppv6AddrObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char prefixObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6AddrIid;
	struct json_object *ipv6AddrJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t prefixInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(ipv6AddrIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6AddrIid.idx[0]);
	ipv6AddrIid.level = 2;
	ipv6AddrIid.idx[1] = ipv6Instance;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, &ipv6AddrJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		return ret;
	}


	if(multiJobj){
		tmpObj = ipv6AddrJobj;
		ipv6AddrJobj = NULL;
		ipv6AddrJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "Prefix") == 0){//transfer
				ZOS_STRCPY_M(prefixObjName, json_object_get_string(paramValue));
				ptr = strstr(prefixObjName, ".X_ZYXEL_IPv6Prefix");
				if(ptr){
					sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &prefixInstance);
					*ptr = '\0';
					if(zcfgFe98To181MappingNameGet(prefixObjName, ipIface) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv6Prefix.%d", ipIface, prefixInstance);
						json_object_object_add(ipv6AddrJobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}			
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6AddrJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6AddrJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_ADDR, &ipv6AddrIid, ipv6AddrJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
			json_object_put(ipv6AddrJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, ipv6AddrIid.idx[0], ipv6AddrIid.idx[1]);
		}
		json_object_put(ipv6AddrJobj);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t WANPppv6AddrObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char pppIface[32] = {0}, ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;


	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_ADDR, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Address Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];

	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Address.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Address.i.
 */
zcfgRet_t WANPppv6AddrObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char pppIface[32] = {0}, ipIface[32] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Address");
	sscanf(ptr, ".X_ZYXEL_IPv6Address.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;


	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_ADDR, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Address.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANPppv6PrefixObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);

	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;
	
	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;
	
	if((ret = feObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj, updateFlag)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}

	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6PrefixJobj);

	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */

zcfgRet_t WANPppv6PrefixObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6PrefixIid;
	struct json_object *ipv6PrefixJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	uint8_t ipv6Instance = 0;
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(ipv6PrefixIid);
	sscanf(ipIface, "IP.Interface.%hhu", &ipv6PrefixIid.idx[0]);
	ipv6PrefixIid.level = 2;
	ipv6PrefixIid.idx[1] = ipv6Instance;
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, &ipv6PrefixJobj)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		return ret;
	}


	if(multiJobj){
		tmpObj = ipv6PrefixJobj;
		ipv6PrefixJobj = NULL;
		ipv6PrefixJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6PrefixJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6PrefixJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IP_IFACE_V6_PREFIX, &ipv6PrefixIid, ipv6PrefixJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
			json_object_put(ipv6PrefixJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, ipv6PrefixIid.idx[0], ipv6PrefixIid.idx[1]);
		}
		json_object_put(ipv6PrefixJobj);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANPppv6PrefixObjAdd(char *tr98FullPathName, int *idx)
{
	zcfgRet_t ret;
	objIndex_t objV6Iid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char pppIface[32] = {0}, ipIface[32] = {0};
	char *ptr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;


	IID_INIT(objV6Iid);
	objV6Iid.level = 1;
	sscanf(ipIface, "IP.Interface.%hhu", &objV6Iid.idx[0]);
	if((ret = zcfgFeObjStructAdd(RDM_OID_IP_IFACE_V6_PREFIX, &objV6Iid, NULL)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Add IP.Interface.%d.IPv6Prefix Fail.\n", __FUNCTION__, objV6Iid.idx[0]);
		return ret;
	}

	*idx = objV6Iid.idx[1];
	return ZCFG_SUCCESS;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Prefix.i.
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv6Prefix.i.
 */
zcfgRet_t WANPppv6PrefixObjDel(char *tr98FullPathName)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	uint8_t ipv6Instance = 0;
	char pppIface[32] = {0}, ipIface[32] = {0};
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6Prefix");
	sscanf(ptr, ".X_ZYXEL_IPv6Prefix.%hhu", &ipv6Instance);
	*ptr = '\0';

	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(objIid);
	objIid.level = 2;
	sscanf(ipIface, "IP.Interface.%hhu", &objIid.idx[0]);
	objIid.idx[1] = ipv6Instance;

	ret = zcfgFeObjStructDel(RDM_OID_IP_IFACE_V6_PREFIX, &objIid, NULL);
	if (ret != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Delete IP.Interface.%d.IPv6Prefix.%d Fail\n", __FUNCTION__, objIid.idx[0], objIid.idx[1]);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_DHCPv6Client.
 *
 *   Related object in TR181:
 *   Device.DHCPv6.Client.i.
 */
zcfgRet_t WANPppv6DhcpV6ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6ClientIid;
	struct json_object *ipv6ClientJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Client");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(ipv6ClientIid);
	while((ret = feObjJsonGetNext(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, &ipv6ClientJobj, updateFlag)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6ClientJobj, "Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6ClientJobj);
	}

	if(!found)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	
	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6ClientJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6ClientJobj);
	
	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_DHCPv6Client.
 *
 *   Related object in TR181:
 *   Device.IPv6rd.
 *   Device.IPv6rd.InterfaceSetting.i.
 */

zcfgRet_t WANPppv6DhcpV6ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6ClientIid;
	struct json_object *ipv6ClientJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_DHCPv6Client");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(ipv6ClientIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, &ipv6ClientJobj)) == ZCFG_SUCCESS) {
		if(strcmp(ipIface, json_object_get_string(json_object_object_get(ipv6ClientJobj, "Interface")))==0){
			found = 1;
			break;
		}
		else
			json_object_put(ipv6ClientJobj);
	}

	if(!found )
		return ret;

	if(multiJobj){
		tmpObj = ipv6ClientJobj;
		ipv6ClientJobj = NULL;
		ipv6ClientJobj = zcfgFeJsonMultiObjAppend(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6ClientJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(ipv6ClientJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV6_CLIENT, &ipv6ClientIid, ipv6ClientJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Client.%d Fail\n", __FUNCTION__, ipv6ClientIid.idx[0]);
			json_object_put(ipv6ClientJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.DHCPv6.Client.%d Fail\n", __FUNCTION__, ipv6ClientIid.idx[0]);
		}
		json_object_put(ipv6ClientJobj);
	}

	return ret;
}

/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6rd.
 *
 *   Related object in TR181:
 *   Device.IPv6rd.
 *   Device.IPv6rd.InterfaceSetting.i.
 */
zcfgRet_t WANPppv6RdObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
	struct json_object *ipv6RdJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;
	
	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;
	
	IID_INIT(ipv6RdIid);
	while((ret = feObjJsonGetNext(RDM_OID_IPV6RD_INTF, &ipv6RdIid, &ipv6RdJobj, updateFlag)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6RdJobj, "X_ZYXEL_Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6RdJobj);
	}
	if(!found)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	
	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ipv6RdJobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp( paramList->name, "AddressSource") == 0){//transfer
				ZOS_STRCPY_M(addrsrcObjName, json_object_get_string(paramValue));
				ptr = strstr(addrsrcObjName, ".IPv4Address");
				if(ptr){
					sscanf(ptr, ".IPv4Address.%hhu", &addrsrcInstance);
					*ptr = '\0';
					if(zcfgFe181To98MappingNameGet(addrsrcObjName, tr98ConnObjName) == ZCFG_SUCCESS) {
						snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv4Address.%d", tr98ConnObjName, addrsrcInstance);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(tr98Prefix));
						paramList++;
						continue;
					}
				}
			}			
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/

		paramList++;
	}

	json_object_put(ipv6RdJobj);
	
	return ZCFG_SUCCESS;	
}
/*  
 *   TR98 Object Name : InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6rd.
 *
 *   Related object in TR181:
 *   Device.DHCPv6.Client.i.
 */

zcfgRet_t WANPppv6RdObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char pppIface[32] = {0}, ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char addrsrcObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ipv6RdIid;
	struct json_object *ipv6RdJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;	
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;
	char tr98Prefix[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	uint8_t addrsrcInstance = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_IPv6rd");
	*ptr = '\0';

	/*find tr181 mapping object*/
	if((ret = zcfgFe98To181MappingNameGet(tr98ConnObjName, pppIface)) != ZCFG_SUCCESS) {
		return ret;
	}
	/*Get IP.Interface.i above PPP.Interface.i*/
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(pppIface, ipIface)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(ipv6RdIid);
	while((ret = zcfgFeObjJsonGetNext(RDM_OID_IPV6RD_INTF, &ipv6RdIid, &ipv6RdJobj)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ipv6RdJobj, "X_ZYXEL_Interface");
		if(paramValue != NULL){
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ipv6RdJobj);
	}


	if(!found )
		return ret;

	if(multiJobj){
		tmpObj = ipv6RdJobj;
		ipv6RdJobj = NULL;
		ipv6RdJobj = zcfgFeJsonMultiObjAppend(RDM_OID_IPV6RD_INTF, &ipv6RdIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/		
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			/*Write value from WANIPConnection to Device.IP.Interface.i*/
			tr181ParamValue = json_object_object_get(ipv6RdJobj, paramList->name);
			if(tr181ParamValue != NULL) {
				if(strcmp( paramList->name, "AddressSource") == 0){//transfer
					ZOS_STRCPY_M(addrsrcObjName, json_object_get_string(paramValue));
					ptr = strstr(addrsrcObjName, ".IPv4Address");
					if(ptr){
						sscanf(ptr, ".IPv4Address.%hhu", &addrsrcInstance);
						*ptr = '\0';
						if(zcfgFe98To181MappingNameGet(addrsrcObjName, ipIface) == ZCFG_SUCCESS) {
							snprintf(tr98Prefix, sizeof(tr98Prefix), "%s.IPv4Address.%d", ipIface, addrsrcInstance);
							json_object_object_add(ipv6RdJobj, paramList->name, json_object_new_string(tr98Prefix));
							paramList++;
							continue;
						}
					}
				}	

				json_object_object_add(ipv6RdJobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*special case*/
		}
		paramList++;	
	}


	/*Set Device.IP.Interface.i*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_IPV6RD_INTF, &ipv6RdIid, ipv6RdJobj, NULL)) != ZCFG_SUCCESS ) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
			json_object_put(ipv6RdJobj);
			return ret;
		}
		else {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set Device.IPv6rd.InterfaceSetting.%d Fail\n", __FUNCTION__, ipv6RdIid.idx[0]);
		}
		json_object_put(ipv6RdJobj);
	}

	return ret;
}

#ifdef CONFIG_XPON_SUPPORT
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.X_ZYZEL_WANPonInterfaceConfig
 *
 *  Related object in TR181:
 *  Device.Optical.Interface.i
 */
zcfgRet_t WANPonIfaceCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[32] = {0};
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr = NULL;
	char ponIfaceObj[32] = {0};
	uint32_t  ponIfaceOid = 0;
	objIndex_t ponIfaceIid;
	struct json_object *ponIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);
	//printf("\033[0;32m Debug: [%s]:tr98FullPathName=%s \033[0m \n",__FUNCTION__,tr98FullPathName);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".X_ZYZEL_WANPonInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ponIfaceObj)) == ZCFG_SUCCESS) {
		if(strstr(ponIfaceObj, "Optical") == NULL) {
			printf("%s : Not an Pon(Optical) Interface:%s\n", __FUNCTION__,ponIfaceObj);
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(ponIfaceIid);
		sprintf(tr181Obj, "Device.%s", ponIfaceObj );
		ponIfaceOid = zcfgFeObjNameToObjId(tr181Obj, &ponIfaceIid);

		if((ret = feObjJsonGet(ponIfaceOid, &ponIfaceIid, &ponIfaceJobj, updateFlag)) != ZCFG_SUCCESS)
			return ret;

		printf("Optical Interface %s\n", json_object_to_json_string(ponIfaceJobj));

		/*fill up tr98 WANEthernetInterfaceConfig object*/
		*tr98Jobj = json_object_new_object();
		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			paramValue = json_object_object_get(ponIfaceJobj, paramList->name);
			if(paramValue != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}

			/*Not defined in tr181, give it a default value*/
#if TR98_MAPPING_DEBUG
			printf("Can't find parameter %s in TR181\n", paramList->name);
#endif
			paramList++;
		}

		json_object_put(ponIfaceJobj);
	}

	return ZCFG_SUCCESS;
}
/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.X_ZYZEL_WANPonInterfaceConfig

    Related object in TR181:
    Device.Optical.Interface.i
 */
zcfgRet_t WANPonIfaceCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ponIfaceName[32] = {0};
	char *ptr = NULL;
	objIndex_t ponIfaceIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *ponIfaceJobj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	/*find tr181 mapping object*/
	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".X_ZYZEL_WANPonInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ponIfaceName)) == ZCFG_SUCCESS) {
		if(strstr(ponIfaceName, "Optical") == NULL) {
			printf("%s : Not an Pon(Optical) Interface\n", __FUNCTION__);
			return ZCFG_NO_SUCH_OBJECT;
		}

		IID_INIT(ponIfaceIid);
		sscanf(ponIfaceName, "Optical.Interface.%hhu", &ponIfaceIid.idx[0]);
		ponIfaceIid.level = 1;

		if((ret = zcfgFeObjJsonGet(RDM_OID_OPT_INTF, &ponIfaceIid, &ponIfaceJobj)) != ZCFG_SUCCESS)
			return ret;

		if(multiJobj){
			tmpObj = ponIfaceJobj;
			ponIfaceJobj = NULL;
			ponIfaceJobj = zcfgFeJsonMultiObjAppend(RDM_OID_OPT_INTF, &ponIfaceIid, multiJobj, tmpObj);
		}

		paramList = tr98Obj[handler].parameter;
		while(paramList->name != NULL) {
			/*Write new parameter value from tr98 object to tr181 objects*/
			paramValue = json_object_object_get(tr98Jobj, paramList->name);
			if(paramValue != NULL) {
				tr181ParamValue = json_object_object_get(ponIfaceJobj, paramList->name);
				if(tr181ParamValue != NULL) {
					json_object_object_add(ponIfaceJobj, paramList->name, JSON_OBJ_COPY(paramValue));
					paramList++;
					continue;
				}
			}
#if TR98_MAPPING_DEBUG
			printf("Can't find parameter %s in TR181\n", paramList->name);
#endif
			paramList++;
		}

		if(multiJobj){
			json_object_put(tmpObj);
		}
		else{
			if((ret = zcfgFeObjJsonSet(RDM_OID_OPT_INTF, &ponIfaceIid, ponIfaceJobj, NULL)) != ZCFG_SUCCESS ) {
				json_object_put(ponIfaceJobj);
				return ret;
			}
			json_object_put(ponIfaceJobj);
		}
	}

	return ZCFG_SUCCESS;
}

/*  TR98 Object Name : InternetGatewayDevice.WANDevice.i.X_ZYZEL_WANPonInterfaceConfig.Stats

    Related object in TR181:
    Device.Optical.Interface.i.Stats.
 */
zcfgRet_t WANPonIfaceCfgStObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr98TmpName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char ponIfaceName[32] = {0};
	char *ptr = NULL;
	objIndex_t objIid;
	rdm_OptIntfSt_t *ponIfaceSt = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);

	ZOS_STRCPY_M(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".X_ZYZEL_WANPonInterfaceConfig");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.WANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, ponIfaceName)) != ZCFG_SUCCESS) {
		/*No such object*/
		return ret;
	}

	if(strstr(ponIfaceName, "Optical") == NULL) {
		printf("%s : Not an Pon(Optical) Interface\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	*tr98Jobj = json_object_new_object();

	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(ponIfaceName, "Optical.Interface.%hhu", &objIid.idx[0]);

	if(feObjStructGet(RDM_OID_OPT_INTF_ST, &objIid, (void **)&ponIfaceSt, updateFlag) == ZCFG_SUCCESS) {
		json_object_object_add(*tr98Jobj, "BytesSent", json_object_new_int(ponIfaceSt->BytesSent));
		json_object_object_add(*tr98Jobj, "BytesReceived", json_object_new_int(ponIfaceSt->BytesReceived));
		json_object_object_add(*tr98Jobj, "PacketsSent", json_object_new_int(ponIfaceSt->PacketsSent));
		json_object_object_add(*tr98Jobj, "PacketsReceived", json_object_new_int(ponIfaceSt->PacketsReceived));
		zcfgFeObjStructFree(ponIfaceSt);
	}
	return ZCFG_SUCCESS;
}
#endif/*CONFIG_XPON_SUPPORT*/

#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig  |  Device.X_ZYXEL_WWAN_BACKUP*/
zcfgRet_t wwanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	char pppIf[128] = {0};
	uint32_t  wwanOid = 0;
	objIndex_t wwanIid, wwanPingIid;
	struct json_object *wwanObj = NULL;
	struct json_object *wwanPingObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(wwanIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wwanOid = zcfgFeObjNameToObjId(tr181Obj, &wwanIid);
	if((ret = feObjJsonGet(wwanOid, &wwanIid, &wwanObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(wwanPingIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_PING_CHECK, &wwanPingIid, &wwanPingObj)) != ZCFG_SUCCESS) {
		json_object_put(wwanObj);
		return ret;
	}

	/*fill up tr98 mgmtSrv object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if (strcmp(paramList->name, "AccessPointName") == 0)
			paramValue = json_object_object_get(wwanObj, "APN");
		else if (strcmp(paramList->name, "PhoneNumber") == 0)
			paramValue = json_object_object_get(wwanObj, "DialNumber");
		else if (strcmp(paramList->name, "PINCode") == 0)
			paramValue = json_object_object_get(wwanObj, "PIN");
		else if (strcmp(paramList->name, "PUKCode") == 0)
			paramValue = json_object_object_get(wwanObj, "PUK");
		else if (strcmp(paramList->name, "PINVerification") == 0)
			paramValue = json_object_object_get(wwanObj, "PIN_Verification");
		else if (strcmp(paramList->name, "BackupInterval") == 0)
			paramValue = json_object_object_get(wwanPingObj, "Backup_Interval");
		else if (strcmp(paramList->name, "RecoveryInterval") == 0)
			paramValue = json_object_object_get(wwanPingObj, "Recovery_Interval");
		else if (strcmp(paramList->name, "CheckedDefaultGateway") == 0)
			paramValue = json_object_object_get(wwanPingObj, "DefaultGateway");
		else if (strcmp(paramList->name, "CheckedIPAddr") == 0)
			paramValue = json_object_object_get(wwanPingObj, "Destination");
		else paramValue = json_object_object_get(wwanObj, paramList->name);

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(wwanObj);
	json_object_put(wwanPingObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig  |  Device.X_ZYXEL_WWAN_BACKUP*/
zcfgRet_t wwanObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  wwanOid = 0, wwanPingOid = 0;
	objIndex_t wwanIid, wwanPingIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *wwanObj = NULL;
	struct json_object *wwanPingObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpwwanObj = NULL;
	struct json_object *tmpwwanPingObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(wwanIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wwanOid = zcfgFeObjNameToObjId(tr181Obj, &wwanIid);
	if((ret = zcfgFeObjJsonGet(wwanOid, &wwanIid, &wwanObj)) != ZCFG_SUCCESS) {
		return ret;
	}

	IID_INIT(wwanPingIid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_ZY_W_W_A_N_PING_CHECK, &wwanPingIid, &wwanPingObj)) != ZCFG_SUCCESS) {
		json_object_put(wwanObj);
		return ret;
	}

	if(multiJobj){
		tmpwwanObj = wwanObj;
		tmpwwanPingObj = wwanPingObj;
		wwanObj = NULL;
		wwanPingObj = NULL;
		wwanObj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanIid, multiJobj, tmpwwanObj);
		wwanPingObj = zcfgFeJsonMultiObjAppend(RDM_OID_ZY_W_W_A_N_PING_CHECK, &wwanPingIid, multiJobj, tmpwwanPingObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if (strcmp(paramList->name, "AccessPointName") == 0)
				tr181ParamValue = json_object_object_get(wwanObj, "APN");
			else if (strcmp(paramList->name, "PhoneNumber") == 0)
				tr181ParamValue = json_object_object_get(wwanObj, "DialNumber");
			else if (strcmp(paramList->name, "PINCode") == 0)
				tr181ParamValue = json_object_object_get(wwanObj, "PIN");
			else if (strcmp(paramList->name, "PUKCode") == 0)
				tr181ParamValue = json_object_object_get(wwanObj, "PUK");
			else if (strcmp(paramList->name, "BackupInterval") == 0)
				tr181ParamValue = json_object_object_get(wwanPingObj, "Backup_Interval");
			else if (strcmp(paramList->name, "RecoveryInterval") == 0)
				tr181ParamValue = json_object_object_get(wwanPingObj, "Recovery_Interval");
			else if (strcmp(paramList->name, "CheckedDefaultGateway") == 0)
				tr181ParamValue = json_object_object_get(wwanPingObj, "DefaultGateway");
			else if (strcmp(paramList->name, "CheckedIPAddr") == 0)
				tr181ParamValue = json_object_object_get(wwanPingObj, "Destination");
			else tr181ParamValue = json_object_object_get(wwanObj, paramList->name);

			if(tr181ParamValue != NULL) {
				if (strcmp(paramList->name, "AccessPointName") == 0)
					json_object_object_add(wwanObj, "APN", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "PhoneNumber") == 0)
					json_object_object_add(wwanObj, "DialNumber", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "PINCode") == 0)
					json_object_object_add(wwanObj, "PIN", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "PUKCode") == 0)
					json_object_object_add(wwanObj, "PUK", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "BackupInterval") == 0)
					json_object_object_add(wwanPingObj, "Backup_Interval", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "RecoveryInterval") == 0)
					json_object_object_add(wwanPingObj, "Recovery_Interval", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "CheckedDefaultGateway") == 0)
					json_object_object_add(wwanPingObj, "DefaultGateway", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "CheckedIPAddr") == 0)
					json_object_object_add(wwanPingObj, "Destination", JSON_OBJ_COPY(paramValue));
				else json_object_object_add(wwanObj, paramList->name, JSON_OBJ_COPY(paramValue));

				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	/*Set Device.X_ZYXEL_WWAN_BACKUP*/
	if(multiJobj){
		json_object_put(tmpwwanObj);
		json_object_put(tmpwwanPingObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanIid, wwanObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(wwanObj);
			return ret;
		}
		json_object_put(wwanObj);
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig.Stats  |  Device.X_ZYXEL_WWAN_BACKUP.Stats*/
zcfgRet_t wwanObjStatsGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	char pppIf[128] = {0};
	uint32_t  wwanCfgOid = 0, wwanStatsOid = 0, wwanPPPOid = 0, wwanPPPStatOid = 0;
	objIndex_t wwanCfgIid, wwanStatsIid, wwanPPPIid, wwanPPPStatsIid;
	struct json_object *wwanStatsObj = NULL;
	struct json_object *wwanPPPStatsObj = NULL;
	struct json_object *wwanPPPObj = NULL;
	struct json_object *wwanCfgObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(wwanCfgIid);
	IID_INIT(wwanPPPIid);
	IID_INIT(wwanPPPStatsIid);
	if((ret = feObjJsonGet(RDM_OID_ZY_W_W_A_N_BACKUP, &wwanCfgIid, &wwanCfgObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}
	paramValue = json_object_object_get(wwanCfgObj, "PPPInterface");
	if (paramValue == NULL) {
		json_object_put(wwanCfgObj);
		return ZCFG_INTERNAL_ERROR;
	}
	ZOS_STRCPY_M(pppIf, json_object_get_string(paramValue));

	if (strlen(pppIf) == 0) 
		return ZCFG_INTERNAL_ERROR;
	wwanPPPIid.level = 1;
	wwanPPPStatsIid.level = 1;
	sscanf(pppIf, "PPP.Interface.%hhu", &wwanPPPIid.idx[0]);
	wwanPPPStatsIid.idx[0] = wwanPPPIid.idx[0];
	if((ret = feObjJsonGet(RDM_OID_PPP_IFACE, &wwanPPPIid, &wwanPPPObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}
	if((ret = feObjJsonGet(RDM_OID_PPP_IFACE_STAT, &wwanPPPStatsIid, &wwanPPPStatsObj, updateFlag)) != ZCFG_SUCCESS) {
		json_object_put(wwanPPPObj);
		return ret;
	}


	IID_INIT(wwanStatsIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wwanStatsOid = zcfgFeObjNameToObjId(tr181Obj, &wwanStatsIid);
	if((ret = feObjJsonGet(wwanStatsOid, &wwanStatsIid, &wwanStatsObj, updateFlag)) != ZCFG_SUCCESS) {
		json_object_put(wwanPPPObj);
		json_object_put(wwanPPPStatsObj);
		return ret;
	}

	/*fill up tr98 mgmtSrv object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if (strcmp(paramList->name, "BackupInterface") == 0)
			paramValue = json_object_object_get(wwanCfgObj, "Interface");
		else if (strcmp(paramList->name, "DeviceManufacturer") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "Manufacturer");
		else if (strcmp(paramList->name, "DeviceModelName") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "Model");
		else if (strcmp(paramList->name, "DeviceFirmwareVersion") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "FWVersion");
		else if (strcmp(paramList->name, "SIMCardIMSI") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "SIMIMSI");
		else if (strcmp(paramList->name, "DeviceIMEI") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "DEVIMEI");
		else if (strcmp(paramList->name, "ConnectionUpTime") == 0)
			paramValue = json_object_object_get(wwanStatsObj, "ConnectionUptime");
		else if (strcmp(paramList->name, "CellsSent") == 0)
			paramValue = json_object_object_get(wwanPPPStatsObj, "PacketsSent");
		else if (strcmp(paramList->name, "CellsReceived") == 0)
			paramValue = json_object_object_get(wwanPPPStatsObj, "PacketsReceived");
		else paramValue = json_object_object_get(wwanStatsObj, paramList->name);

		if (paramValue == NULL)
			paramValue = json_object_object_get(wwanPPPObj, paramList->name);
		if (paramValue == NULL)
			paramValue = json_object_object_get(wwanPPPStatsObj, paramList->name);

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(wwanCfgObj);
	json_object_put(wwanStatsObj);
	json_object_put(wwanPPPObj);
	json_object_put(wwanPPPStatsObj);
	return ZCFG_SUCCESS;
}
#endif // CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP

/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig.Email.i    |    Device.X_ZYXEL_EmailNotification.EmailEventConfig.i    */
zcfgRet_t wwanEmailObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  wwanEmailOid = 0;
	objIndex_t wwanEmailIid;
	struct json_object *wwanEmailObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_DL_DEBUG, "%s : Enter\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(wwanEmailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wwanEmailOid = zcfgFeObjNameToObjId(tr181Obj, &wwanEmailIid);
	if((ret = feObjJsonGet(wwanEmailOid, &wwanEmailIid, &wwanEmailObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*fill up tr98 mgmtSrv object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if (strcmp(paramList->name, "EmailEnable") == 0)
			paramValue = json_object_object_get(wwanEmailObj, "Enable");
		else if (strcmp(paramList->name, "EmailTitle") == 0)
			paramValue = json_object_object_get(wwanEmailObj, "EmailSubject");
		else paramValue = json_object_object_get(wwanEmailObj, paramList->name);

		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		ztrdbg(ZTR_DL_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(wwanEmailObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig.Email.i    |    Device.X_ZYXEL_EmailNotification.EmailEventConfig.i    */
zcfgRet_t wwanEmailObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  wwanEmailOid = 0;
	objIndex_t wwanEmailIid;
	tr98Parameter_t *paramList = NULL;
	struct json_object *wwanEmailObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;

	ztrdbg(ZTR_DL_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}

	IID_INIT(wwanEmailIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	wwanEmailOid = zcfgFeObjNameToObjId(tr181Obj, &wwanEmailIid);
	if((ret = zcfgFeObjJsonGet(wwanEmailOid, &wwanEmailIid, &wwanEmailObj)) != ZCFG_SUCCESS) {
		return ret;
	}
	if(multiJobj){
		tmpObj = wwanEmailObj;
		wwanEmailObj = NULL;
		wwanEmailObj = zcfgFeJsonMultiObjAppend(RDM_OID_MAIL_EVENT_CFG, &wwanEmailIid, multiJobj, tmpObj);	
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if (strcmp(paramList->name, "EmailEnable") == 0)
				tr181ParamValue = json_object_object_get(wwanEmailObj, "Enable");
			else if (strcmp(paramList->name, "EmailTitle") == 0)
				tr181ParamValue = json_object_object_get(wwanEmailObj, "EmailSubject");
			else tr181ParamValue = json_object_object_get(wwanEmailObj, paramList->name);

			if(tr181ParamValue != NULL) {
				if (strcmp(paramList->name, "EmailEnable") == 0)
					json_object_object_add(wwanEmailObj, "Enable", JSON_OBJ_COPY(paramValue));
				else if (strcmp(paramList->name, "EmailTitle") == 0) {
					json_object_object_add(wwanEmailObj, "EmailSubject", JSON_OBJ_COPY(paramValue));
				}
				else { 
					json_object_object_add(wwanEmailObj, paramList->name, JSON_OBJ_COPY(paramValue));
				}

				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	/*Set Device.X_ZYXEL_WWAN_BACKUP*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &wwanEmailIid, wwanEmailObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(wwanEmailObj);
			return ret;
		}
		json_object_put(wwanEmailObj);
	}

	return ZCFG_SUCCESS;
}

#ifdef ROUTEINFO_INTERFACE
/*  
 *   TR98 Object Name :InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_RouteInformation.
 *
 *   Related object in TR181:
 *   Device.Routing.RouteInformation.
 *   Device.Routing.RouteInformation.InterfaceSetting.i.
 */
zcfgRet_t WANIpIntfRouteInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	char ipIface[32] = {0};
	char tr98ConnObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t ifsetIid;
	struct json_object *ifsetJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char *ptr = NULL;
	int found = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s : Enter\n", __FUNCTION__);
	ZOS_STRCPY_M(tr98ConnObjName, tr98FullPathName);
	ptr = strstr(tr98ConnObjName, ".X_ZYXEL_RouteInformation");
	*ptr = '\0';

	//printf("WANIpIntfRouteInfoObjGet: tr98ConnObjName=%s\n",tr98ConnObjName);
	if(zcfgFe98To181MappingNameGet(tr98ConnObjName, ipIface) != ZCFG_SUCCESS) {
		/*  The instance number of WANIPConnection.i will be continuous. Therefore, 
		 *  just return "ZCFG_NO_MORE_INSTANCE" when there is no related mapping of tr181 for
		 *  this WANIPConnection object.
		 */
		return ZCFG_NO_MORE_INSTANCE;
	}
	//printf("WANIpIntfRouteInfoObjGet: ipIface=%s\n",ipIface);
	IID_INIT(ifsetIid);
	while((ret = feObjJsonGetNext(RDM_OID_ROUTING_RT_INFO_INTF_SET, &ifsetIid, &ifsetJobj, updateFlag)) == ZCFG_SUCCESS) {
		paramValue = json_object_object_get(ifsetJobj, "Interface");
		if(paramValue != NULL){
			printf("lanIpIntfRouterAdverObjGet: paramValue=%s\n",json_object_get_string(paramValue));
			if(strcmp(ipIface, json_object_get_string(paramValue))==0){
				found = 1;
				break;
			}
		}
		json_object_put(ifsetJobj);
	}
	if(!found)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	
	/*Fill up tr98 WANIPConnection.i object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(ifsetJobj, paramList->name);
		//printf("paramList->name=%s\n",paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*special case*/
		if(!strcmp(paramList->name, "RouteLifetime"))
		{
			time_t cur_time = time(NULL);
			time_t life_time;
			struct tm *nPtr=NULL;
			char RouteLifeTimeStr[24];

			paramValue = json_object_object_get(ifsetJobj, "X_ZYXEL_RouterLifetime");
			//printf("X_ZYXEL_RouterLifetime=%d\n",json_object_get_int(paramValue));
			life_time=cur_time+json_object_get_int(paramValue);
			nPtr = localtime(&life_time);
			sprintf(RouteLifeTimeStr,"%u-%02u%02uT%02u:%02u:%02u",nPtr->tm_year + 1900,
														nPtr->tm_mon + 1,
														nPtr->tm_mday,
														nPtr->tm_hour,
														nPtr->tm_min,
														nPtr->tm_sec);
			//printf("RouteLifeTimeStr=%s\n",RouteLifeTimeStr);
			/*change format*/
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(RouteLifeTimeStr));
			paramList++;
			continue;
		}

		paramList++;
	}

	json_object_put(ifsetJobj);

	return ZCFG_SUCCESS;
}
#endif

#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
/*  
 *   TR98 Object Name : InternetGatewayDevice.X_ZYXEL_ActiveWANDevice.i
 *
 *   Related object in TR181:
 *   Device.IP.Interface.i
 *   Device.IP.Interface.i.IPv4Address.i
 *   or
 *   Related object in TR181:
 *   Device.PPP.Interface.i
 *   Device.PPP.Interface.i.PPPoA
 *   Device.PPP.Interface.i.PPPoE
 *   Device.PPP.Interface.i.IPCP
 *   Device.PPP.Interface.i.IPV6CP
 *   Device.IP.Interface.i
 */
zcfgRet_t RUNNINGWANConnObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid = {0};
	objIndex_t ipIfaceIid= {0};
	rdm_MgmtSrv_t *MgmtSrvObj = NULL;
	rdm_IpIface_t *ipIfaceJobj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	rdm_AtmLink_t *atmObj = NULL;
#endif
	int wanipInstance = 0;
	int wanDeviceInstance = 0;
	int runningwanInstance = 0;
	char currType[10] = {0};
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
	int origWanDeviceInstance = 0; //WWAN=2;Ethernet =3;ADSL=4;VDSL=5
	char origTr98FullPathName[128] = {0};
	char origLowerLayers[32] = {0};
#endif

	IID_INIT(objIid);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, (void **)&MgmtSrvObj) == ZCFG_SUCCESS) {
		sscanf(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface, "IP.Interface.%d", &wanipInstance);
		zcfgFeObjStructFree(MgmtSrvObj);
	}else{
		zcfgLog(ZCFG_LOG_ERR, "%s : Get RDM_OID_MGMT_SRV Fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	sscanf(tr98FullPathName, "InternetGatewayDevice.X_ZYXEL_ActiveWANDevice.%d", &runningwanInstance);
	IID_INIT(ipIfaceIid);
	ipIfaceIid.idx[0] = wanipInstance;
	ipIfaceIid.level = 1;
	wanDeviceInstance = 1; //always get WANDevice.1 
	if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &ipIfaceIid, (void **)&ipIfaceJobj)) == ZCFG_SUCCESS) {
		if(!strcmp(ipIfaceJobj->Status, "Up")){
			if(strstr(ipIfaceJobj->LowerLayers, "Ethernet") != NULL){	
					snprintf(tr98FullPathName, TR98_MAX_OBJ_NAME_LENGTH, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.1.WANIPConnection.%d", wanDeviceInstance, runningwanInstance);
					#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
					ZOS_STRNCPY(origLowerLayers, "WANIPConnection", sizeof(origLowerLayers));
					#endif
					zcfgFeTr98ObjSpvGet(tr98FullPathName, tr98Jobj, updateFlag);	
			}else if(strstr(ipIfaceJobj->LowerLayers, "PPP") != NULL){
					snprintf(tr98FullPathName, TR98_MAX_OBJ_NAME_LENGTH, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.1.WANPPPConnection.%d", wanDeviceInstance, runningwanInstance);
					#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
					ZOS_STRNCPY(origLowerLayers, "WANPPPConnection", sizeof(origLowerLayers));
					#endif					
					zcfgFeTr98ObjSpvGet(tr98FullPathName, tr98Jobj, updateFlag);
			}

			if(*tr98Jobj != NULL){
				json_object_object_add(*tr98Jobj, "ATMEncapsulation", json_object_new_string(""));
				json_object_object_add(*tr98Jobj, "LinkType", json_object_new_string(""));
				json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string(""));
				IID_INIT(objIid);
				checkWANType(ipIfaceIid, ipIfaceJobj, currType, &objIid.idx[0]);
				if(currType[0] != '\0'){
					json_object_object_add(*tr98Jobj, "WANAccessType", json_object_new_string(currType));
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
					if(!strcmp(currType, "ADSL")){
						#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
						origWanDeviceInstance = 4;
						#endif
						objIid.level = 1;
						if((ret = zcfgFeObjStructGetWithoutUpdate(RDM_OID_ATM_LINK, &objIid, (void **)&atmObj)) == ZCFG_SUCCESS){
							if(atmObj->Encapsulation)
								json_object_object_add(*tr98Jobj, "ATMEncapsulation", json_object_new_string(atmObj->Encapsulation));
							if(atmObj->LinkType){
								json_object_object_add(*tr98Jobj, "LinkType", json_object_new_string(atmObj->LinkType));
								fflush(stdout);
							}
							zcfgFeObjStructFree(atmObj);
						}
					}
#endif
					#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
					if(!strcmp(currType, "VDSL")){
						origWanDeviceInstance = 5;
					}
					else if(!strcmp(currType, "WWAN")){
						origWanDeviceInstance = 2;
					}
					else if(!strcmp(currType, "Ethernet")){
						origWanDeviceInstance = 3;
					}
					#endif
				}
				#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION //hardcode for WIND
				snprintf(origTr98FullPathName, sizeof(origTr98FullPathName), "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.1.%s.%d", origWanDeviceInstance, origLowerLayers, runningwanInstance);
				json_object_object_add(*tr98Jobj, "DataConnectionInterface", json_object_new_string(origTr98FullPathName));
				#endif
				snprintf(tr98FullPathName, TR98_MAX_OBJ_NAME_LENGTH, "InternetGatewayDevice.X_ZYXEL_ActiveWANDevice.%d", runningwanInstance);
			}
		}	
		zcfgFeObjStructFree(ipIfaceJobj);
	}else
		zcfgLog(ZCFG_LOG_ERR, "%s : Get RDM_OID_IP_IFACE Fail\n", __FUNCTION__);
	return ret;	
}
#endif

