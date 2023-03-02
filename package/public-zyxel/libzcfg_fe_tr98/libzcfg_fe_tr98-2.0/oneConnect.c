#include <json/json.h>
#include <sqlite3.h>
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
#include "oneConnect_parameter.h"
#include "zos_api.h"

extern tr98Object_t tr98Obj[];

sqlite3 *db = NULL;
#define DATABASE_PATH    "/tmp/netprobe.db"
#define RET_CHECK_TIME	5
#define RET_FILE		"/tmp/1905ReturnValue"
#define RET_SUCCESS		"00"
#define POWERON			0
#define POWEROFF		2
#define HOSTPOLLING_PS_TMP "/tmp/hpps_"

#define FWCHECK			(1 << 0)
#define FW_UPGRADE		(1 << 1)
#define FW_DOWNLOAD		(1 << 2)


int tr_db_init()
{
    int rc = 0;

    /* Open database */
	rc = sqlite3_open_v2(DATABASE_PATH, &db, SQLITE_OPEN_READONLY, NULL);
	//rc = sqlite3_open_v2(DATABASE_PATH, &db, SQLITE_OPEN_READWRITE, NULL);
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		db = NULL;
        return -1;
    }

    return 0;
}

static int tr_db_result_get(char *sql, int *rows, int *cols, char ***result)
{
	char *error_report = NULL;

	if( sqlite3_get_table(db, sql, result, rows, cols, &error_report) != SQLITE_OK ) {
        fprintf( stderr , "\t> CMD: %s , Error: %s\n" , sql , error_report );
		sqlite3_free(error_report);
		*rows = -1;
		*cols = -1;
		return -1;
	}

	return 0;
}

int tr_db_is_subIntf(char *mac, char *almac)
{
	char sql[128] = {0};
	int rows = 0, cols = 0, ret = 0;
	char **result = NULL;
	//bool subIntf = false;

	sprintf(sql, "SELECT ALMac FROM SubIntf WHERE [IntfMAC]='%s' OR [ALMac]='%s';", mac, mac);

	ret = tr_db_result_get(sql, &rows, &cols, &result);
	/*
		result :
		 0 ALMac
		   -----
		 1 MAC1
		 2 MAC2
		    .
		    .
	 */
	if(rows > 0) {
		//subIntf = true;

		/*Use first result*/
		if(strcmp(result[1], "") != 0)
			strcpy(almac, result[1]);
	}

	if(ret == 0)
		sqlite3_free_table(result);

	return ret;
}

void tr_db_close(){
	
	sqlite3_close(db);
}


zcfgRet_t oneConnectCommand(const char *cmd, const char * alMac){
	FILE *fp = NULL;
	char result[16] = {0}, sql[256] = {0};
	int index = 0;
	time_t startTime, finTime;
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	system(cmd);
	
	startTime = time((time_t *)0);
	
	while(1) {
		if((fp = fopen(RET_FILE, "r")) != NULL) {
			if(fgets(result, sizeof(result), fp) != NULL ){
				fclose(fp);
				break;
			}
			fclose(fp);
		}
		sleep(1);
		finTime = time((time_t *)0);
		// check result in five seconds
		if(difftime(finTime, startTime) >= RET_CHECK_TIME){
			printf("Can't get any response from network extender which ALMAC is %s\n", alMac);
			return ZCFG_INTERNAL_ERROR;
		}		
	}
	
	// the content in RET_FILE is xx\n\0  but we don't need \n
	for(; index < strlen(result); index++){
		if(result[index] == '\n'){
			result[index] = '\0';
			break;
		}
	}
	
	if(strcmp(result, RET_SUCCESS) != 0){
		printf("oneConnect command failed, result = %s\n", result);
		ret = ZCFG_INTERNAL_ERROR;
	}

	return ret;
}

int checkPollingWorking(char *mac)
{
	int working = 0;
	char tmp[128] = {0}, buf[128] = {0};
	FILE *fp = NULL;
	
	sprintf(tmp, "ps | grep hostpolling > %s%s", HOSTPOLLING_PS_TMP, mac);
	system(tmp);
	
	sprintf(tmp, "%s%s", HOSTPOLLING_PS_TMP, mac);
	if((fp = fopen(tmp, "r")) != NULL){
		while(fgets(buf, sizeof(buf) - 1, fp)){
			if(!strstr(buf, "grep") && strstr(buf, mac)){
				working = 1;
				break;
			}
		}
		fclose(fp);
	}
	unlink(tmp);
	
	return working;
}

/*
 *   TR98 Object Name : InternetGatewayDevice.LANDevice.i.Hosts.Host.i.X_ZYXEL_EXT"
 *
 *   Related object in TR181:
 *   Device.Hosts.Host.i.Ext
 */
zcfgRet_t oneConnectExtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	tr98Parameter_t *paramList = NULL;
	int idx = 0;
	struct json_object *paramValue = NULL;
	rdm_HostsHost_t *stHost = NULL;
	char macAddress[65] = {0}, alMac[65] = {0}, cmd[256] = {0}, fwUrl[512] = {0}, media[9] = {0}, media_temp[9] = {0};
	char fwUrlTmp[256] = {0}, *fwUrlPtr, *fwUrlPtrTmp;
	bool changePwr = false;
	unsigned int actOption = 0;
	int urlOption = 0, pwrStatus = 0, index = 0;
#if 1 // modify, 20150810
#else
	objIndex_t hostIid;
	char tr181Name[32] = {0};
	char tr98TmpName[128] = {0};
	char higherLayerPath[32] = {0};
	char mappingPathName[32] = {0};
	char *ptr = NULL;
	uint32_t hostNum = 0;
#endif
	objIndex_t oneConnectIid;
	rdm_ZyOneConnect_t *oneConnectObj = NULL;
	
	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);
	
	IID_INIT(oneConnectIid);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_ZY_ONE_CONNECT, &oneConnectIid, (void **)&oneConnectObj) == ZCFG_SUCCESS){
		if(!oneConnectObj->Enable){
			zcfgFeObjStructFree(oneConnectObj);
			return ZCFG_INVALID_ARGUMENTS;
		}
		zcfgFeObjStructFree(oneConnectObj);
	}
	
#if 1 // modify, 20150810
	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%hhu.X_ZYXEL_EXT", &objIid.idx[0]);
	
	if((ret = zcfgFeObjStructGet(RDM_OID_HOSTS_HOST, &objIid, (void **)&stHost)) == ZCFG_SUCCESS){
		if(stHost->Active != true){
			zcfgFeObjStructFree(stHost);
			return ZCFG_INVALID_ARGUMENTS;
		}
		ZOS_STRNCPY(macAddress, stHost->PhysAddress, sizeof(macAddress));
		zcfgFeObjStructFree(stHost);
	}
#else
	strcpy(tr98TmpName, tr98FullPathName);
	ptr = strstr(tr98TmpName, ".Hosts");
	*ptr = '\0';

	/* tr98TmpName will be InternetGatewayDevice.LANDevice.i */
	if((ret = zcfgFe98To181MappingNameGet(tr98TmpName, mappingPathName)) != ZCFG_SUCCESS){
		return ret;
	}

	strcpy(tr181Name, mappingPathName);
	if((ret = zcfgFeTr181IfaceStackHigherLayerGet(tr181Name, higherLayerPath)) != ZCFG_SUCCESS)
		return ret;

	if(strstr(higherLayerPath, "IP.Interface") != NULL) {
		IID_INIT(objIid);
		IID_INIT(hostIid);
		objIid.level = 1;
		sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%d", &idx);

		while(zcfgFeObjStructGetNext(RDM_OID_HOSTS_HOST, &objIid, (void **)&stHost) == ZCFG_SUCCESS){
			if(strcmp(stHost->Layer3Interface, higherLayerPath) == 0){
				hostNum++;
				if((idx == hostNum) && (stHost->Active != true)) {
					zcfgFeObjStructFree(stHost);
					return ZCFG_INVALID_ARGUMENTS;
				}
				else if(idx == hostNum){
					hostIid = objIid;
					strncpy(macAddress, stHost->PhysAddress, sizeof(macAddress)); 
					zcfgFeObjStructFree(stHost);
					break;
				}
			}
			zcfgFeObjStructFree(stHost);
		}
	}
#endif
	else
		return ZCFG_NOT_FOUND;
	
	if(strcmp(macAddress, "") == 0){ 
		printf("Can't find information in config\n");
		return ZCFG_INVALID_ARGUMENTS;
	}
	
	// open database
	if(tr_db_init() != 0){
		fprintf( stderr , "can't init database\n");
		return ZCFG_INTERNAL_ERROR;
	}
	
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			if(strcmp(paramList->name, "L2DevCtrl_Reboot") == 0){
				if(json_object_get_boolean(paramValue) == true) {
					if(tr_db_is_subIntf(macAddress, alMac) != 0){ 
						printf("can't find ALMac\n");
						return ZCFG_INTERNAL_ERROR;
					}
					
					if(strchr(alMac, ':') != 0){
#ifdef HOSTPOLLING
						if(checkPollingWorking(macAddress) == 0){
#endif
							sprintf(cmd, "/usr/sbin/zy1905App 2 %s", alMac);
							ret = oneConnectCommand(cmd, alMac);
							printf("%d:zy1905 cmd = %s\n", __LINE__, cmd);
#ifdef HOSTPOLLING
							if(ret == ZCFG_SUCCESS){
								sprintf(cmd, "/sbin/hostpolling %s reboot --debug %s &", macAddress, alMac);
								printf("[%s]\n", cmd);
								system(cmd);
							}
						}
						else{
							printf("%s host polling already working, so do nothing!!!\n", macAddress);
							return ZCFG_INTERNAL_ERROR;
						}
#endif
					}
					else{
						printf("%d: Wrong ALMac %s\n", __LINE__, alMac);
						return ZCFG_INTERNAL_ERROR;
					}
				}
			}
			else if(strcmp(paramList->name, "L2DevCtrl_Reset") == 0){
				if(json_object_get_boolean(paramValue) == true) {
					if(tr_db_is_subIntf(macAddress, alMac) != 0){
						printf("can't find ALMac\n");
						return ZCFG_INTERNAL_ERROR;
					}
					
					if(strchr(alMac, ':') != 0){
#ifdef HOSTPOLLING
						if(checkPollingWorking(macAddress) == 0){
#endif
							sprintf(cmd, "/usr/sbin/zy1905App 10 %s", alMac);
							ret = oneConnectCommand(cmd, alMac);
							printf("%d:zy1905 cmd = %s\n", __LINE__, cmd);
#ifdef HOSTPOLLING
							if(ret == ZCFG_SUCCESS){
								sprintf(cmd, "/sbin/hostpolling %s reset --debug %s &", macAddress, alMac);
								printf("[%s]\n", cmd);
								system(cmd);
							}
						}
						else{
							printf("%s host polling already working, so do nothing!!!\n", macAddress);
							return ZCFG_INTERNAL_ERROR;
						}
#endif
					}
					else{
						printf("%d: Wrong ALMac %s\n", __LINE__, alMac);
						return ZCFG_INTERNAL_ERROR;
					}
				}
			}
			else if(strcmp(paramList->name, "L2DevCtrl_FWCheck") == 0){ 
				if(json_object_get_boolean(paramValue) == true)
					actOption |= FWCHECK;
			}
			else if(strcmp(paramList->name, "L2DevCtrl_FWDownload") == 0){ 
				if(json_object_get_boolean(paramValue) == true)
					actOption |= FW_DOWNLOAD;
			}
			else if(strcmp(paramList->name, "L2DevCtrl_FWUpgrade") == 0){
				if(json_object_get_boolean(paramValue) == true)
					actOption |= FW_UPGRADE;
			}
			else if(strcmp(paramList->name, "L2DevCtrl_FWURL") == 0){ 
				strcpy(fwUrl, json_object_get_string(paramValue));
				if(strcmp(fwUrl, "") == 0){
					strcpy(fwUrl, "NULL");
					urlOption = 0;
				}
				else{
					if(strncmp(fwUrl, "ftp", strlen("ftp")) == 0)
						urlOption = 1;
					else if(strncmp(fwUrl, "https", strlen("https")) == 0)
						//urlOption = 3;
						urlOption = 1;
					else if(strncmp(fwUrl, "http", strlen("http")) == 0)
						//urlOption = 2;
						urlOption = 1;
					else{ // if protocol is not supported, set url to NULL
						strcpy(fwUrl, "NULL");
						urlOption = 0;
					}
					
					// add '\' before '(' and ')'
					if(urlOption){
						fwUrlPtr = fwUrl;
						while((fwUrlPtr = (fwUrlPtrTmp = strchr(fwUrlPtr, '(')) ? fwUrlPtrTmp : strchr(fwUrlPtr, ')')) != NULL){
							strcpy(fwUrlTmp, fwUrlPtr);
							strcpy(fwUrlPtr + 1, fwUrlTmp);
							fwUrlPtr[0] = '\\';
							fwUrlPtr += 2;
						}
					}
				}
			}
			else if(strcmp(paramList->name, "L2DevCtrl_WiFiPwr") == 0){
				if(json_object_get_boolean(paramValue))
					pwrStatus = POWERON;
				else
					pwrStatus = POWEROFF;
				changePwr = true;
			}
			else if(strcmp(paramList->name, "L2DevCtrl_MediaType") == 0){
				strcpy(media, json_object_get_string(paramValue));
				
				if(strcmp(media, "") != 0){
					for(; index < strlen(media); index++){
						media_temp[index] = media[index];
					}
					memset(media, 0, sizeof(media));
				}
			}
		}
		paramList++;
	}

// Set wlan power Start-------------------------
	if(changePwr && (strcmp(media_temp, "") != 0)){
		if(strcmp(media_temp, "ALL") == 0)
			ZOS_STRNCPY(media, "0x01ff", sizeof(media));
		else if(strcmp(media_temp, "2.4G") == 0)
			ZOS_STRNCPY(media, "0x0103", sizeof(media));
		else if(strcmp(media_temp, "5G") == 0)
			ZOS_STRNCPY(media, "0x0105", sizeof(media));
		else{
			printf("%s: Unknown mediatype\n", __FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}
		
		if(tr_db_is_subIntf(macAddress, alMac) != 0){
			printf("can't find ALMac\n");
			return ZCFG_INTERNAL_ERROR;
		}
		
		if(strchr(alMac, ':') != 0){
			sprintf(cmd, "/usr/sbin/zy1905App 3 %s 00:00:00:00:00:00 %s %d", alMac, media, pwrStatus);
			ret = oneConnectCommand(cmd, alMac);
			printf("%d:zy1905 cmd = %s\n", __LINE__, cmd);
		}
		else{
			printf("%d: Wrong ALMac %s\n", __LINE__, alMac);
			return ZCFG_INTERNAL_ERROR;
		}
	}

// Set wlan power end-------------------------	
	
// Firmware Control Start-------------------------	
	if(actOption != 0) {	
		if(tr_db_is_subIntf(macAddress, alMac) != 0){
			printf("can't find ALMac\n");
			return ZCFG_INTERNAL_ERROR;
		}
		
		if(strchr(alMac, ':') != 0){
#ifdef HOSTPOLLING
			if(checkPollingWorking(macAddress) == 0){
#endif
				sprintf(cmd, "/usr/sbin/zy1905App 9 %s %d %d %s", alMac, actOption, urlOption, fwUrl);
				ret = oneConnectCommand(cmd, alMac);
				printf("%d:zy1905 cmd = %s\n", __LINE__, cmd);
#ifdef HOSTPOLLING
				if((ret == ZCFG_SUCCESS) && actOption && FW_UPGRADE){
					sprintf(cmd, "/sbin/hostpolling %s FWupgrade --debug %s &", macAddress, alMac);
					printf("[%s]\n", cmd);
					system(cmd);
				}
			}
			else{
				printf("%s host polling already working, so do nothing!!!\n", macAddress);
				return ZCFG_INTERNAL_ERROR;
			}
#endif
		}
		else{
			printf("%d: Wrong ALMac %s\n", __LINE__, alMac);
			return ZCFG_INTERNAL_ERROR;
		}
	
	}	
// Firmware Control End-------------------------	
	
	if(db != NULL)
		tr_db_close();
	
	return ret;
}


/*  TR98 Object Name : InternetGatewayDevice.LANDevice.i.Hosts.Host.i.X_ZYXEL_EXT

    Related object in TR181:
    Device.Hosts.Host.i
*/
zcfgRet_t oneConnectExtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	tr98Parameter_t *paramList = NULL;
	objIndex_t objIid;
	struct json_object *hostExtJobj = NULL;
	struct json_object *paramValue = NULL;
	objIndex_t oneConnectIid;
	rdm_ZyOneConnect_t *oneConnectObj = NULL;
	
	zcfgLog(ZCFG_LOG_DEBUG, "%s : Enter\n", __FUNCTION__);
	printf("%s : Enter\n", __FUNCTION__);

	IID_INIT(oneConnectIid);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_ZY_ONE_CONNECT, &oneConnectIid, (void **)&oneConnectObj) == ZCFG_SUCCESS){
		if(!oneConnectObj->Enable){
			zcfgFeObjStructFree(oneConnectObj);
			return ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(oneConnectObj);
	}
	
	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	sscanf(tr98FullPathName, "%*[^.].%*[^.].%*d.Hosts.Host.%hhu.X_ZYXEL_EXT", &objIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_ZY_HOSTS_HOST_EXT, &objIid, &hostExtJobj, updateFlag)) != ZCFG_SUCCESS){
		return ret;
	}
		
	//fill up tr98 Host object
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		paramValue = json_object_object_get(hostExtJobj, paramList->name);
		
		if(!strcmp(paramList->name, "L2DevCtrl_Reboot")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_RebootResult") && (paramValue != NULL)) {
			//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unknown"));
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_Reset")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_ResetResult") && (paramValue != NULL)) {
			//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unknown"));
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_FWCheck")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_FWDownload")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_FWUpgrade")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_FWURL")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_FWUpgradeResult") && (paramValue != NULL)) {
			//json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Unknown"));
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_WiFiPwr")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_boolean(false));
			paramList++;
			continue;
		}
		else if(!strcmp(paramList->name, "L2DevCtrl_MediaType")) {
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
			paramList++;
			continue;
		}
		
	//Not defined in tr181, give it a default value
#if TR98_MAPPING_DEBUG
		printf("Can't find parameter %s in TR181\n", paramList->name);
#endif
		paramList++;
	}
	
	json_object_put(hostExtJobj);
	
	return ZCFG_SUCCESS;
}
