
#include <json/json.h>
#include <unistd.h>
#include <sys/stat.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"

#define FILE_ACCESS_SUCCESS 0
#define FILE_NAME_SIZE 256
#define FILE_PATH_SIZE 256
#define CMD_SIZE 256
#define LINE_SIZE 2048
#define LOG_MAX_LINES 8000
#define SYSLOG_DIR "/var/log"
#define FLASH_LOG_DIR "/data"
#define LOG_ROTATE 2 


dal_param_t SYS_LOG_param[]={
#ifdef ZYXEL_OPAL_EXTENDER
	{"category",		dalType_string, 0,	0,	NULL,	"syslog",	dal_Get|dal_Delete},
#else
	{"category",		dalType_string, 0,	0,	NULL,	"syslog|securitylog",	dal_Get|dal_Delete},
#endif
	{NULL,				dalType_int,	0,	0,	NULL,	NULL,	0},
};

/*!
 *  Get logs from the log file under /data.
 *
 *  @param[inout]  result          in: a JSON object of json_type_array
 *                                 out: logs, each array entry contains a log entry
 *  @param[in]     logFileSize     number of lines in logFileName
 *  @param[in]     catName         name of category. Now only support "All"
 *  @param[in]     logFileName     name of the regular log file e.g. syslog.log
 *
 *  @return     true  - add log entry success.
 *              false - at least one parameter value is invalid.
 *
 *  @note     catName is the names in LOG_CATEGORY e.g. "dhcpc", "tr69" instead of the names in
 *            LOG_CLASSIFY e.g. "syslog", "securitylog"
 */
zcfgRet_t flashSysLogGet(struct json_object *result, int logFileSize, char *catName, char *logFileName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int i = LOG_ROTATE;
	int len = 0;
	char filePath[FILE_PATH_SIZE] = {0};
	char cmd[CMD_SIZE] = {0};
	char *line = NULL;
	char *tmpLine = NULL;
	FILE *tmpFp;

	if (logFileName == NULL)
	{
		dbg_printf("%s:%d: logFileName cannot be NULL \n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if (catName == NULL)
	{
		dbg_printf("%s:%d: catName cannot be NULL \n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	else if (strcmp(catName,"All"))
	{
		dbg_printf("%s:%d: catName supports \"All\" only \n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if (logFileSize < 0)
	{
		dbg_printf("%s:%d: log file size is less then zero \n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if (result == NULL)
	{
		dbg_printf("%s:%d: Need to init result \n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}

	while (1)
	{
        i--;
        sprintf(filePath, "%s/%s%d", FLASH_LOG_DIR, logFileName, i);        
        tmpFp = fopen(filePath, "r");
        if (tmpFp) {
            // file exists
            fclose(tmpFp);
        } else {
            // file doesn't exist
            if(i >= 0)
                continue;
            else
                break;
        }
		
        if (logFileSize < LOG_MAX_LINES)
		{
			line = (char*)malloc(LINE_SIZE);
            sprintf(cmd, "tail -n %d %s", LOG_MAX_LINES-logFileSize, filePath); 
			tmpFp = popen(cmd, "r");
			if (tmpFp)
			{
				while (fgets(line, LINE_SIZE, tmpFp) != NULL)
				{
					len = strlen(line);
					line[len-1] = '\0';
					tmpLine = line;
					json_object_array_add(result, json_object_new_string(tmpLine));
				}
				pclose(tmpFp);
			}
			else
			{
				dbg_printf("%s:%d: Can't get log (%s), cmd=%s \n",__FUNCTION__,__LINE__,filePath,cmd);
			}
        }
		
		if(line != NULL) free(line);
	}
	
	return ret;
}// flashSysLogGet

/*!
 *  Get log file name and description by category.
 *
 *  @param[out]    fileName        log file name e.g. syslog.log and securitylog.log
 *  @param[out]    description     description of the log e.g. "System Log" and "Security Log"
 *  @param[in]     category        LOG_CLASSIFY name, e.g. syslog and securitylog
 *
 *  @return     true  - get log entry success.
 *              false - parameter value invalid or category match fail.
 *
 *  @note     Please ref. RDM_OID_LOG_CLASSIFY
 */
zcfgRet_t sysLogInfoGet(char *fileName, char *description, char *category)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	char *logFileName = NULL;
	char *logDescr = NULL;
	bool foundLogCat = false;

	if ((fileName == NULL) || (description == NULL) || (category == NULL))
	{
		dbg_printf("%s:%d: fileName or description or category cannot be NULL\n",__FUNCTION__,__LINE__);
		return ZCFG_INVALID_PARAM_VALUE;
	}

	IID_INIT(iid);
	while ((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_LOG_CLASSIFY, &iid, &obj)) == ZCFG_SUCCESS)
	{
		if (!strcmp(json_object_get_string(json_object_object_get(obj, "Name")),category))
		{
			foundLogCat = true;
			logFileName = (char *)json_object_get_string(json_object_object_get(obj, "File"));
			logDescr = (char *)json_object_get_string(json_object_object_get(obj, "Describe"));
			break;
		}
		zcfgFeJsonObjFree(obj);
	}

	if (!foundLogCat)
	{
		printf("\"%s\" does not match any log category\n",category);
		zcfgFeJsonObjFree(obj);
		return ZCFG_NO_SUCH_OBJECT;
	}
	if(logFileName!=NULL){
	strcpy(fileName,logFileName); 
	}
	if(logDescr!=NULL){
	strcpy(description,logDescr);
	}
	zcfgFeJsonObjFree(obj);

	return ret;
}// sysLogInfoGet

void zcfgFeDalShowSysLog(struct json_object *Jarray){
	int i, len = 0;
	int j, logSize = 0;
	struct json_object *logObj;
	struct json_object *contentObj;
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		logObj = json_object_array_get_idx(Jarray, i);
		if(logObj!=NULL){
			printf("%s \n",json_object_get_string(json_object_object_get(logObj,"description")));
			printf("-------------------------\n");
			contentObj = json_object_object_get(logObj,"content");
			logSize = json_object_array_length(contentObj);
			for(j=0;j<logSize;j++){
				obj = json_object_array_get_idx(contentObj,(logSize-j-1));
				if(obj!=NULL){
					printf("%-5d %s\n",j+1,json_object_get_string(obj));
				}
			}			
		}
	}
}

zcfgRet_t zcfgFeDalSysLogGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramObj = NULL;
	struct json_object *contentObj = NULL;

	// regular log file
	FILE *fd;
	char cmd[CMD_SIZE]={0};
	char line[LINE_SIZE]={0};
	char filePath[FILE_PATH_SIZE]={0};
	char fileName[FILE_NAME_SIZE]={0};
	char logDescript[FILE_NAME_SIZE]={0};
	char *cat = NULL;
	int len = 0; 
	int fileLine = 0;

	// rotate / flash log file
	struct stat st_log;
	struct stat st_rotate;
	struct stat st_flash;
	char rotateFilePath[32] = {0};
	char flashFilePath[32] = {0};   
	bool rotateExist = false;
	bool flashExist = false;

	// temp
	char tmpCmd[CMD_SIZE]={0};
	char tmp_str[LINE_SIZE]={0};
	char tmpName[FILE_NAME_SIZE]={0};
	FILE *tmpfp = NULL;
	int i = 0;
	
	contentObj = json_object_new_array();
	paramObj = json_object_new_object();

	// get file name e.g. syslog.log / get description e.g. System Log
	cat = (char *)json_object_get_string(json_object_object_get(Jobj,"category"));
	ret = sysLogInfoGet(fileName,logDescript,cat);
	if (ret != ZCFG_SUCCESS)
	{
		dbg_printf("%s:%d: ret=%d\n",__FUNCTION__,__LINE__,ret);
		json_object_put(contentObj);
		json_object_put(paramObj);
		return ret;
	}
	
	// log file path e.g. /var/log/syslog.log
	sprintf(filePath,"%s/%s",SYSLOG_DIR,fileName);

	// get fileLine: the number of log file lines
	if ((!stat(filePath, &st_log) && (st_log.st_mode & S_IFREG) && st_log.st_size > 0))
	{
		sprintf(tmpCmd, "wc -l %s", filePath);	
		tmpfp = popen(tmpCmd, "r");
		if(fscanf(tmpfp, "%d %s", &fileLine, tmpName) != 2)
		{
			dbg_printf("%s:%d: fscanf error \n",__FUNCTION__,__LINE__);
			printf("Can't read out log!\n");
		}
		pclose(tmpfp);
	}
	else
	{
		fileLine = 0;
	}

	// rotate file path e.g. /var/log/syslog.log.1
	sprintf(rotateFilePath, "%s/%s.%d", SYSLOG_DIR, fileName, 1); 	
	if (!stat(rotateFilePath, &st_rotate) && (st_rotate.st_mode & S_IFREG) && st_rotate.st_size > 0)
	{
		rotateExist = true;		  
	}  

	// flash file path e.g. /data/syslog.log0
	for (i = 0; i < LOG_ROTATE; i++)
	{
		sprintf(flashFilePath, "%s/%s%d", FLASH_LOG_DIR, fileName, i);     
		if(!stat(flashFilePath, &st_flash) && (st_flash.st_mode & S_IFREG) && st_flash.st_size > 0){
			flashExist = true;
			break;
		}
	}	

	// if no rotate file, get log data from flash
	if (!rotateExist && flashExist)
	{
		flashSysLogGet(contentObj, fileLine, "ALL", fileName);
	}

	if(rotateExist)
		sprintf(cmd, "tail -q -n %d %s %s", LOG_MAX_LINES, rotateFilePath, filePath);
	else
		sprintf(cmd, "tail -n %d %s", LOG_MAX_LINES, filePath);

	// dbg_printf("%s:%d: cmd=%s \n",__FUNCTION__,__LINE__,cmd);

	if (access(filePath,F_OK) == FILE_ACCESS_SUCCESS && access(filePath,R_OK) == FILE_ACCESS_SUCCESS)
	{
		fd = popen(cmd, "r");
		if (!fd)
		{
			dbg_printf("%s:%d: popen error \n",__FUNCTION__,__LINE__);
			printf("Can't get log\n");
			return ZCFG_INTERNAL_ERROR;
		}
		else
		{
			while (fgets(line,LINE_SIZE,fd) != NULL)
			{
				len = strlen(line);
				line[len-1] = '\0';
				strcpy(tmp_str,line);
				json_object_array_add(contentObj, json_object_new_string(tmp_str));
			}
			pclose(fd);
		}
	}
	else // file not exist, do nothing
	{
		dbg_printf("%s:%d: no log content in %s\n", __FUNCTION__,__LINE__,filePath);
	}

	json_object_object_add(paramObj, "description", json_object_new_string(logDescript));
	json_object_object_add(paramObj, "content", JSON_OBJ_COPY(contentObj));
	json_object_array_add(Jarray,paramObj); 

	json_object_put(contentObj);
	
	return ret;
}

zcfgRet_t zcfgFeDalSysLogDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char *cat = NULL;
	char fileName[FILE_NAME_SIZE]={0};
	char logDescript[FILE_NAME_SIZE]={0};

	// get file name e.g. syslog.log
	cat = (char *)json_object_get_string(json_object_object_get(Jobj,"category"));
	ret = sysLogInfoGet(fileName,logDescript,cat);
	if (ret != ZCFG_SUCCESS)
	{
		dbg_printf("%s:%d: ret=%d\n", __FUNCTION__,__LINE__,ret);
		return ret;
	}

	ret = zcfgFeSysLogClean(fileName);
	if (ret != ZCFG_SUCCESS)
		dbg_printf("%s:%d: ret=%d\n", __FUNCTION__,__LINE__,ret);
	
	return ret;
}


zcfgRet_t zcfgFeDalSysLog(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalSysLogDelete(Jobj, NULL);
	}else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalSysLogGet(Jobj, Jarray, NULL);
	}

	return ret;
}

