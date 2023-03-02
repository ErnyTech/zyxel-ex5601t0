#include <json/json.h>
#include <unistd.h>

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
#include "devInfo_parameter.h"

extern tr98Object_t tr98Obj[];

#define FILE_SIZE 32768
#define SYSLOG_FILE_1 "/var/log/syslog.log.1"
#define SYSLOG_FILE "/var/log/syslog.log"

#define DATA_SIZE 256

#define retrieveData(fd, log, retrieve_size)	{ \
		char data[DATA_SIZE] = {0}; \
		int len = 0; \
		bool complete = ((fgets(data, DATA_SIZE, fd)) != NULL && (retrieve_size > 0) && (len < retrieve_size-1)) ? false : true; \
		while(!complete) { \
			strncpy(log+len, data, retrieve_size-len); \
			len = strlen(log); \
			complete = ((fgets(data, DATA_SIZE, fd)) != NULL && (len < retrieve_size-1)) ? false : true; \
		} \
		if(retrieve_size > 0) log[retrieve_size-1] = '\0'; \
	}

/* InternetGatewayDevice.DeviceInfo */
zcfgRet_t devInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
#if 0
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
#endif
	objIndex_t devInfoIid;
	struct json_object *devInfoObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	//char log[FILE_SIZE] = {0};
	//char log_old[FILE_SIZE] = {0};
	char *log = NULL, *log2 = NULL;
	int filelen1 = 0, filelen2 = 0;
	FILE *fd1 = NULL;
	FILE *fd2 = NULL;
	//char *lst = calloc(256, sizeof(char));
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
    FILE *frfp=NULL;
	char frbuf[20] = {0};
	int ReasonCode=0;
	char *frReasonCode = NULL;
	char *frDate = NULL;  
	char frCount[8] = {0};
	int FRCount=0;
#endif
#ifdef CONFIG_ZYXEL_CUSTOMIZATION_DEVICELOG
    FILE *mfd = NULL;
    FILE *hfd = NULL;
    char cmd[128] = {0};
#endif

#if 0
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	
	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);

	if((ret = feObjJsonGet(devInfoOid, &devInfoIid, &devInfoObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#else
	/* InternetGatewayDevice.DeviceInfo <-> Device.DeviceInfo */
	IID_INIT(devInfoIid);
	if((ret = feObjJsonGet(RDM_OID_DEV_INFO, &devInfoIid, &devInfoObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;
#endif	

	/*fill up tr98 devInfo object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(paramList->name, "SpecVersion") == 0){
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("1.0"));
			paramList++;
			continue;		
		}
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
		if(strcmp(paramList->name, "ModelName") == 0){
			const char *swVer = json_object_get_string(json_object_object_get(devInfoObj, "SoftwareVersion"));
			if (strstr(swVer, "ABLZ") != NULL) {
				json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string("Home&Life HUB"));
				paramList++;
				continue;
			}
		}
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
                /* cover no reboot case -Peter.wang 2020/07/30*/
				if(strcmp(paramList->name, "X_TTNET_FR_ReasonCode") == 0){
					frfp = fopen("/data/FactoryResetforTT", "r+");
					if (frfp != NULL)
					{
						fgets(frbuf, 20, frfp);
						frReasonCode = strtok(frbuf, "@");
						frDate = strtok(NULL, "@");
						ReasonCode = atoi(frReasonCode);
						fclose(frfp);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(ReasonCode));
					}
					else{
						paramValue = json_object_object_get(devInfoObj, paramList->name);
						if(paramValue != NULL) {
							json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						}
					}
					paramList++;
					continue;
				}
				if(strcmp(paramList->name, "X_TTNET_FR_Date") == 0){
					frfp = fopen("/data/FactoryResetforTT", "r+");
					if (frfp != NULL)
					{
						fgets(frbuf, 20, frfp);
						frReasonCode = strtok(frbuf, "@");
						frDate = strtok(NULL, "@");
						ReasonCode = atoi(frReasonCode);
						fclose(frfp);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(frDate));
					}
					else{
						paramValue = json_object_object_get(devInfoObj, paramList->name);
						if(paramValue != NULL) {
							json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						}
					}
					paramList++;
					continue;
				}
				if(strcmp(paramList->name, "X_TTNET_FR_Count") == 0){
					frfp = fopen("/data/FRCountforTT", "r+");
					if (frfp != NULL)
					{
						fgets(frCount, sizeof(frCount), frfp);
						FRCount = atoi(frCount);
						fclose(frfp);
						json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(FRCount));
					}
					else{
						paramValue = json_object_object_get(devInfoObj, paramList->name);
						if(paramValue != NULL) {
							json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
						}
					}
					paramList++;
					continue;
				}
#endif
		if(strcmp(paramList->name, "DeviceLog") == 0){
#ifdef CONFIG_ZYXEL_CUSTOMIZATION_DEVICELOG
			log = calloc(FILE_SIZE, sizeof(char));
			strcpy(log, "");

            if(access("/var/log/ttdevicelog", F_OK) == 0){
				mfd = fopen("/var/log/ttdevicelog", "r");
			} 
			if(mfd != NULL){
					fseek(mfd, 0, SEEK_END);
					filelen1 = ftell(mfd);
					fseek(mfd, 0, SEEK_SET);
			}
			if(filelen1 >= FILE_SIZE){
					fseek(mfd, ((filelen1%FILE_SIZE) + (filelen1/FILE_SIZE - 1)*FILE_SIZE), SEEK_SET);
					retrieveData(mfd, log, FILE_SIZE);
			}
			else
			{
			   		if(mfd != NULL)
					   retrieveData(mfd, log, filelen1);
			   }

            	if(mfd != NULL){
					fclose(mfd);
				}
#else
			if(access(SYSLOG_FILE_1, F_OK) == 0){
				fd1 = fopen(SYSLOG_FILE_1, "r");
			}
			if(access(SYSLOG_FILE, F_OK) == 0){
				if((fd2 = fopen(SYSLOG_FILE, "r")) == NULL ){
					printf("TR69 open log file fail!!\n");
				}
			}
			log = calloc(FILE_SIZE, sizeof(char));
			strcpy(log, "");
			if((fd1 != NULL) || (fd2 != NULL)){
				if(fd1 != NULL){
					fseek(fd1, 0, SEEK_END);
					filelen1 = ftell(fd1);
					fseek(fd1, 0, SEEK_SET);
				}
				if(fd2 != NULL){
					fseek(fd2, 0, SEEK_END);
					filelen2 = ftell(fd2);
					fseek(fd2, 0, SEEK_SET);
				}
				if(filelen2 > FILE_SIZE){
					fseek(fd2, ((filelen2%FILE_SIZE) + (filelen2/FILE_SIZE - 1)*FILE_SIZE), SEEK_SET);
					retrieveData(fd2, log, FILE_SIZE);
#if 0
					fgets(lst, 256, fd2);
					while((fgets(lst, 256, fd2)) != NULL) {
//						strcat(log, lst);
						strcpy(log_old, log);
						strcpy(log, lst);
						strcat(log, log_old);
					}
#endif
				}else{
					if(fd1 != NULL && (filelen2 != FILE_SIZE)){
						int logspace = filelen2 ? (FILE_SIZE - (filelen2 + 1)) : FILE_SIZE;
						int pos = (logspace && (filelen1 > logspace)) ? ((filelen1%logspace) + (filelen1/logspace - 1)*logspace) : 0;
						logspace = (logspace == FILE_SIZE) ? logspace : (logspace+1);
						fseek(fd1, pos, SEEK_SET);
						retrieveData(fd1, log, logspace);
#if 0
						fgets(lst, 256, fd1);
						while((fgets(lst, 256, fd1)) != NULL) {
//							strcat(log, lst);
							strcpy(log_old, log);
							strcpy(log, lst);
							strcat(log, log_old);
						}
#endif
					}
					if(fd2 != NULL && filelen2) {
						int logsz, datasz;
						logsz = strlen(log);
						log2  = log + logsz;
						datasz = (FILE_SIZE > (logsz+1)) ? (FILE_SIZE - logsz) : 0;
						retrieveData(fd2, log2, datasz);
					}
#if 0
					while((fgets(lst, 256, fd2)) != NULL) {
//						strcat(log, lst);
						strcpy(log_old, log);
						strcpy(log, lst);
						strcat(log, log_old);
					}
#endif
				}
				if(fd1 != NULL){
					fclose(fd1);
				}
				if(fd2 != NULL){
					fclose(fd2);
				}
			}
#endif
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(log));
			paramList++;
			free(log);
			//free(lst);
			continue;	
		}
		
		paramValue = json_object_object_get(devInfoObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(devInfoObj);

	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice.DeviceInfo */
zcfgRet_t devInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	struct json_object *devInfoObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
	if((ret = zcfgFeObjJsonGet(devInfoOid, &devInfoIid, &devInfoObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = devInfoObj;
		devInfoObj = NULL;
		devInfoObj = zcfgFeJsonMultiObjAppend(RDM_OID_DEV_INFO, &devInfoIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(devInfoObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(devInfoObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;	
	} /*Edn while*/
	
	/*Set Device.DeviceInfo*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DEV_INFO, &devInfoIid, devInfoObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(devInfoObj);
			return ret;
		}
		json_object_put(devInfoObj);
	}


	return ZCFG_SUCCESS;
}
/*
 *   TR98 Object Name : InternetGatewayDevice.DeviceInfo.MemoryStatus
 *
 *   Related object in TR181:
 *   Device.DeviceInfo.MemoryStatus
 */
 zcfgRet_t memoryStatusObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *memorystatusJobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DEV_INFO_MEM_STAT, &objIid, &memorystatusJobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(memorystatusJobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(memorystatusJobj);

	return ZCFG_SUCCESS;
}

#if 0
zcfgRet_t devInfoObjNotify(char *tr98ObjName, struct json_object *tr181ParamObj, int handler, struct json_object **tr98NotifyInfo)
{
	bool found = false;
	char *tr181ParamList = NULL;
	char *token = NULL;
	char tr98NotifyNameList[2048] = "";
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	tr181ParamList = (char *)json_object_get_string(tr181ParamObj);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();	
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		token = strtok(tr181ParamList, ",");
		while(token != NULL) {
			if(strcmp(token, paramList->name) == 0) {
				found = true;
				break;
			}
			token = strtok(NULL, ",");
		}

		if(found) {
			if(strlen(tr98NotifyNameList) == 0) {
				strcpy(tr98NotifyNameList, paramList->name);
			}
			else {
				strcat(tr98NotifyNameList, ",");
				strcpy(tr98NotifyNameList, paramList->name);
			}
		}
		
		paramList++;
		found = false;
	}

	if(strlen(tr98NotifyNameList) != 0) {
		json_object_object_add(*tr98NotifyInfo, tr98ObjName, json_object_new_string(tr98NotifyNameList));
	}

	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t devInfoObjNotify(char *tr98ObjName, char *tr181ParamName, struct json_object *tr181ParamVal, int handler, struct json_object **tr98NotifyInfo)
{
	char tr98Notify[256] = {0};
	tr98Parameter_t *paramList = NULL;
	char paramName[100] = {0};
	char *paramPtr = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(*tr98NotifyInfo == NULL) {
		*tr98NotifyInfo = json_object_new_object();
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		if(strcmp(tr181ParamName, paramList->name) == 0) {
			paramPtr = tr181ParamName;
			break;
		}

		paramList++;
	}

	if(!paramPtr) {
		if(!strcmp(tr181ParamName, "FirmwareVersion")) {
			strcpy(paramName, "ModemFirmwareVersion");
			paramPtr = paramName;
		}
	}

	if(paramPtr) {
		sprintf(tr98Notify, "%s.%s", tr98ObjName, paramPtr);
		json_object_object_add(*tr98NotifyInfo, tr98Notify, JSON_OBJ_COPY(tr181ParamVal));
	}

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.DeviceInfo */
int devInfoObjAttrGet(char *tr98FullPathName, int handler, char *paramName)
{
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = ZCFG_NO_SUCH_PARAMETER;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	if(zcfgFe98To181MappingNameGet(tr98FullPathName, mapObj) != ZCFG_SUCCESS) {
		return ZCFG_INVALID_OBJECT;
	}
	

	IID_INIT(devInfoIid);
	snprintf(tr181Obj, sizeof(tr181Obj),"Device.%s", mapObj);
	devInfoOid = zcfgFeObjNameToObjId(tr181Obj, &devInfoIid);
		

	/*fill up tr98 devInfo object*/
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*find the matched parameter*/
		if(strcmp(paramList->name, paramName)){
			paramList++;
			continue;
		}

		/*special case*/
		if(strcmp(paramList->name, "SpecVersion") == 0){
			attrValue = PARAMETER_ATTR_PASSIVENOTIFY;
		}
		else if(!strcmp(paramList->name, "EnabledOptions")) {
			attrValue = 0;
		}
		else if(!strcmp(paramList->name, "DeviceLog")) {
			attrValue = 0;
		}
		else if(!strcmp(paramList->name, "ModemFirmwareVersion")) {
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DSL_LINE, "FirmwareVersion");
#endif
		}
		else {
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramList->name);
		}
		
		if(attrValue < 0 ) {
			attrValue = 0;
		}

		break;
	}

	return attrValue;
}

/* InternetGatewayDevice.DeviceInfo *//* all use the batch set, only tr069 use this action*/
zcfgRet_t devInfoObjAttrSet(char *tr98FullPathName, int handler, char *paramName, int newNotify, struct json_object *multiAttrJobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr181Obj[128] = {0};
	char mapObj[128] = {0};
	uint32_t  devInfoOid = 0;
	objIndex_t devInfoIid;
	int attrValue = 0;
	tr98Parameter_t *paramList = NULL;
	char attrParamName[100] = {0};
	char *paramNamePtr = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);
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
		if((strcmp(paramName, "SpecVersion") == 0) || (!strcmp(paramName, "SoftwareVersion")) || (!strcmp(paramName, "EnabledOptions"))
			|| (!strcmp(paramName, "ProvisioningCode")) || (!strcmp(paramName, "DeviceLog"))) {
			return ZCFG_SUCCESS;
		}
		else if(!strcmp(paramName, "ModemFirmwareVersion")) {
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
			strcpy(attrParamName, "FirmwareVersion");
			attrValue = zcfgFeParamAttrGetByName(RDM_OID_DSL_LINE, attrParamName);
			devInfoOid = RDM_OID_DSL_LINE;
			paramNamePtr = attrParamName;
#else
			zcfgLog(ZCFG_LOG_NOTICE, "%s : DSL not support\n", __FUNCTION__);
			return ZCFG_SUCCESS;
#endif
		}
		else {
			attrValue = zcfgFeParamAttrGetByName(devInfoOid, paramName);
			paramNamePtr = paramName;
		}
		if(attrValue < 0) {
			ret = ZCFG_INVALID_ARGUMENTS;
			break;
		}

		/*Write new parameter attribute from tr98 object to tr181 objects*/
		attrValue = zcfgFeNotifyAttrValSet(attrValue, newNotify);
		if( (ret = zcfgFeMultiParamAttrAppend(devInfoOid, multiAttrJobj, paramNamePtr, attrValue)) != ZCFG_SUCCESS){
			
			zcfgLog(ZCFG_LOG_ERR, "%s(): set %d %s attribute fail\n", __FUNCTION__, devInfoOid, paramNamePtr);
		}

		break;

	} /*Edn while*/
	
	return ret;
}

/* InternetGatewayDevice.DeviceInfo.VendorConfigFile.{i}. */
zcfgRet_t devInfoVendorCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t devInfoIid;
	struct json_object *devInfoObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	/* InternetGatewayDevice.DeviceInfo.VendorConfigFile.{i} <-> Device.DeviceInfo.VendorConfigFile.{i}. */
	IID_INIT(devInfoIid);
	devInfoIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.DeviceInfo.VendorConfigFile.%hhu", &devInfoIid.idx[0]);
	if((ret = feObjJsonGet(RDM_OID_DEV_INFO_VEND_CONF_FILE, &devInfoIid, &devInfoObj, updateFlag)) != ZCFG_SUCCESS) {
		return ret;
	}

	/*fill up tr98 devInfo object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {

		paramValue = json_object_object_get(devInfoObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		/*Not defined in tr181, give it a default value*/
		zcfgLog(ZCFG_LOG_DEBUG, "Can't find parameter %s in TR181\n", paramList->name);
		json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
		paramList++;
	}
	json_object_put(devInfoObj);

	return ZCFG_SUCCESS;
}

/* InternetGatewayDevice.DeviceInfo.VendorConfigFile.{i}. */
zcfgRet_t devInfoVendorCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	objIndex_t devInfoIid;
	struct json_object *devInfoObj = NULL;
	struct json_object *paramValue = NULL;
	struct json_object *tr181ParamValue = NULL;
	struct json_object *tmpObj = NULL;
	tr98Parameter_t *paramList = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "Enter %s\n", __FUNCTION__);

	IID_INIT(devInfoIid);
	devInfoIid.level = 1;
	sscanf(tr98FullPathName, "InternetGatewayDevice.DeviceInfo.VendorConfigFile.%hhu", &devInfoIid.idx[0]);

	if((ret = zcfgFeObjJsonGet(RDM_OID_DEV_INFO_VEND_CONF_FILE, &devInfoIid, &devInfoObj)) != ZCFG_SUCCESS)
		return ret;
	if(multiJobj){
		tmpObj = devInfoObj;
		devInfoObj = NULL;
		devInfoObj = zcfgFeJsonMultiObjAppend(RDM_OID_DEV_INFO_VEND_CONF_FILE, &devInfoIid, multiJobj, tmpObj);
	}

	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write new parameter value from tr98 object to tr181 objects*/
		paramValue = json_object_object_get(tr98Jobj, paramList->name);
		if(paramValue != NULL) {
			tr181ParamValue = json_object_object_get(devInfoObj, paramList->name);
			if(tr181ParamValue != NULL) {
				json_object_object_add(devInfoObj, paramList->name, JSON_OBJ_COPY(paramValue));
				paramList++;
				continue;
			}
		}
		paramList++;
	} /*Edn while*/

	/*Set Device.DeviceInfo*/
	if(multiJobj){
		json_object_put(tmpObj);
	}
	else{
		if((ret = zcfgFeObjJsonSet(RDM_OID_DEV_INFO_VEND_CONF_FILE, &devInfoIid, devInfoObj, NULL)) != ZCFG_SUCCESS ) {
			json_object_put(devInfoObj);
			return ret;
		}
		json_object_put(devInfoObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t devProcStGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *Jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	if((ret = feObjJsonGet(RDM_OID_DEV_INFO_PS_STAT, &objIid, &Jobj, updateFlag)) != ZCFG_SUCCESS)
		return ret;	

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(Jobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(Jobj);

	return ZCFG_SUCCESS;
}


zcfgRet_t devProcStPsGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t objIid;
	struct json_object *Jobj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;
	char psStatPsFormate[128] = "InternetGatewayDevice.DeviceInfo.ProcessStatus.Process.%hhu";

	printf("%s : Enter\n", __FUNCTION__);

	/*Get related tr181 objects*/
	IID_INIT(objIid);
	objIid.level = 1;
	if(sscanf(tr98FullPathName, psStatPsFormate, &(objIid.idx[0])) != 1)
		return ZCFG_INVALID_OBJECT;
	if((ret = feObjJsonGet(RDM_OID_DEV_INFO_PS_STAT_PS, &objIid, &Jobj, updateFlag)) != ZCFG_SUCCESS){
		printf("ret=%d\n", ret);
		return ret;	
	}

	/*fill up tr98 object from related tr181 objects*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		/*Write parameter value from tr181 objects to tr98 object*/		
		paramValue = json_object_object_get(Jobj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}

		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;	
	}

	json_object_put(Jobj);

	return ZCFG_SUCCESS;
}

