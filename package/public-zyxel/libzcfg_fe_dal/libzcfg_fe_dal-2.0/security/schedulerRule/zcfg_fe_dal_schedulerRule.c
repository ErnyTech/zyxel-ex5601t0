#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <string.h>

#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"



#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t SCHEDULE_param[] =
{
	//Schedule
	{"Index",			dalType_int,		0,	0,	NULL,	NULL,	dal_Edit|dal_Delete},
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	{"Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
#else	
	//{"Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	0},
#endif	
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},	
	{"Description",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Days",			dalType_string,		0,	0,	NULL,	NULL,	0},
	{"TimeStartHour",	dalType_int,		0,	23,	NULL,	NULL,	0},
	{"TimeStartMin",	dalType_int,		0,	59,	NULL,	NULL,	0},
	{"TimeStopHour",	dalType_int,		0,	23,	NULL,	NULL,	0},
	{"TimeStopMin",		dalType_int,		0,	59,	NULL,	NULL,	0},
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	{"Type",		dalType_string,		0,	0,	NULL,	NULL,	0},	
#endif	
	{NULL,				0,					0,	0,	NULL,	NULL,	0}
};

#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
static int hcbSecur = true;
#else
static int hcbSecur = false;
#endif

int schedule_days_string_to_int(char *str){
	int ret = 0;
	if(strcasestr(str,"MON"))
		ret += 2;
	if(strcasestr(str,"TUE"))
		ret += 4;
	if(strcasestr(str,"WED"))
		ret += 8;
	if(strcasestr(str,"THU"))
		ret += 16;
	if(strcasestr(str,"FRI"))
		ret += 32;
	if(strcasestr(str,"SAT"))
		ret += 64;
	if(strcasestr(str,"SUN"))
		ret += 1;
	return ret;
}

void schedule_days_int_to_string(uint32_t days, char *str){
	if(days & DAY_MON){ 
		strcat(str, "Mon,");
	}
	if(days & DAY_TUE){ 
		strcat(str, "Tue,");
	}
	if(days & DAY_WED){ 
		strcat(str, "Wed,");
	}
	if(days & DAY_THU){ 
		strcat(str, "Thu,");
	}
	if(days & DAY_FRI){ 
		strcat(str, "Fri,");
	}
	if(days & DAY_SAT){ 
		strcat(str, "Sat,");
	}
	if(days & DAY_SUN){ 
		strcat(str, "Sun,");
	}
	str[strlen(str)-1]='\0';
}

bool isValidTimePeriod(int startHour, int startMin, int stopHour, int stopMin){
#ifndef ZYXEL_HOME_CYBER_SECURITY
	if((startHour*60+startMin)>=(stopHour*60+stopMin)){
		printf("Time error (start time >= end time)\n");
		return false;
	}
	else
		return true;
#else
		return true;
#endif
}

void zcfgFeDalShowSchedule(struct json_object *Jarray){

	int i, len = 0;
	char *dayString = NULL;
	int startHour = 0, startMin = 0, stopHour = 0, stopMin = 0;
#ifdef ZYXEL_HOME_CYBER_SECURITY
	char timeStr[15] = {0},timetypeStr[33] = {0},timeEnableStr[15] = {0};
#else	
	char timeStr [15] = {0};
#endif	
	struct json_object *obj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	printf("%-6s %-15s %-30s %-15s %-15s %-15s %-30s\n", "Index", "Rule Name", "Days", "Time", "TimeType", "TimeEnable", "Description");
	printf("-----------------------------------------------------------------------------------------------------------------------\n");
#else
	printf("%-6s %-15s %-30s %-15s %-30s\n", "Index", "Rule Name", "Days", "Time", "Description");
	printf("--------------------------------------------------------------------------------\n");
#endif
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){

		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){

			dayString = (char *)json_object_get_string(json_object_object_get(obj, "Days"));			
			startHour = json_object_get_int(json_object_object_get(obj, "TimeStartHour"));
			startMin = json_object_get_int(json_object_object_get(obj, "TimeStartMin"));
			stopHour = json_object_get_int(json_object_object_get(obj, "TimeStopHour"));
			stopMin = json_object_get_int(json_object_object_get(obj, "TimeStopMin"));

#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			if (json_object_get_boolean(json_object_object_get(obj, "Enable")))
				sprintf(timeEnableStr,"Enable");
			else
				sprintf(timeEnableStr,"Disable");

			if (json_object_get_string(json_object_object_get(obj, "Type"))!=NULL)
				strcpy(timetypeStr,json_object_get_string(json_object_object_get(obj, "Type")));
			else 
				sprintf(timetypeStr,"NULL");
#endif
			memset(timeStr, '\0', sizeof(timeStr));
			sprintf(timeStr, "%02d:%02d~%02d:%02d",startHour,startMin,stopHour,stopMin);

			printf("%-6d ",i+1);
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-30s ",dayString);
			printf("%-15s ",timeStr);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			printf("%-15s ",timetypeStr);
			printf("%-15s ",timeEnableStr);	
#endif			
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "Description")));
			printf("\n");

		}
		else{
			printf("ERROR!\n");
		}
		
	}

}

zcfgRet_t zcfgFeDalScheduleGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *ScheduleJobj = NULL;
	struct json_object *pcpJobj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
#ifdef ZYXEL_HOME_CYBER_SECURITY
	struct json_object *enable = NULL;
	struct json_object *type = NULL;	
#else
	//struct json_object *enable = NULL;
#endif	
	struct json_object *name = NULL;
	struct json_object *description = NULL;
	uint32_t days;
	char dayString[30]={0};
	struct json_object *timeStartHour = NULL;
	struct json_object *timeStartMin = NULL;
	struct json_object *timeStopHour = NULL;
	struct json_object *timeStopMin = NULL;
	char TR181Path[64] = {0};
	int obj_MaxLength = 0;
	int listIndex = 0, listSize = 0;
	int allScheduleRuleList[64];
	char tmp[128], *strTmp, *strTmp2;
	uint32_t schIid;
	bool isPcpRule = false;
	
	IID_INIT(objIid);
	while( zcfgFeObjJsonGetNext(RDM_OID_PAREN_CTL_PROF, &objIid, &pcpJobj) == ZCFG_SUCCESS){
		strcpy(tmp, json_object_get_string(json_object_object_get(pcpJobj,"ScheduleRuleList")) );
			strTmp2 = NULL;
			strTmp = strtok_r(tmp, ",", &strTmp2);
			while(strTmp != NULL){
				sscanf(strTmp, "X_ZYXEL_Schedule.%u", &schIid);
				allScheduleRuleList[listIndex] = schIid;
				listIndex++;
				strTmp = strtok_r(NULL, ",", &strTmp2);
			}
	}
	listSize =listIndex;
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_SCHEDULE, &objIid, &ScheduleJobj) == ZCFG_SUCCESS){
		index++;

		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_SCHEDULE);
		sprintf(TR181Path, "X_ZYXEL_Schedule.%u", objIid.idx[0]);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		enable = json_object_object_get(ScheduleJobj, "Enable");
#else		
		//enable = json_object_object_get(ScheduleJobj, "Enable");
#endif		
		name = json_object_object_get(ScheduleJobj, "Name");
		description = json_object_object_get(ScheduleJobj, "Description");

		days = json_object_get_int64(json_object_object_get(ScheduleJobj, "Days"));
		memset(dayString, '\0', sizeof(dayString));
		schedule_days_int_to_string(days,dayString);
		
		timeStartHour = json_object_object_get(ScheduleJobj, "TimeStartHour");
		timeStartMin = json_object_object_get(ScheduleJobj, "TimeStartMin");
		timeStopHour = json_object_object_get(ScheduleJobj, "TimeStopHour");
		timeStopMin = json_object_object_get(ScheduleJobj, "TimeStopMin");
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)		
		type = json_object_object_get(ScheduleJobj, "Type");
#endif
		isPcpRule = false;

		pramJobj = json_object_new_object();

		listIndex = 0;
		for(listIndex=0;listIndex < listSize;listIndex++){
			if(objIid.idx[0] == allScheduleRuleList[listIndex]){
				isPcpRule = true;
				break;
			}
		}
		json_object_object_add(pramJobj, "isPcpRule", json_object_new_boolean(isPcpRule));
		json_object_object_add(pramJobj, "Index", json_object_new_int(index));
		json_object_object_add(pramJobj, "TR181Path", json_object_new_string(TR181Path));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
#else		
		//json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
#endif
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(name));
		json_object_object_add(pramJobj, "Description", JSON_OBJ_COPY(description));
		json_object_object_add(pramJobj, "Days", json_object_new_string(dayString));
		json_object_object_add(pramJobj, "TimeStartHour", JSON_OBJ_COPY(timeStartHour));
		json_object_object_add(pramJobj, "TimeStartMin", JSON_OBJ_COPY(timeStartMin));
		json_object_object_add(pramJobj, "TimeStopHour", JSON_OBJ_COPY(timeStopHour));
		json_object_object_add(pramJobj, "TimeStopMin", JSON_OBJ_COPY(timeStopMin));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		json_object_object_add(pramJobj, "Type", JSON_OBJ_COPY(type));
#endif		
		json_object_object_add(pramJobj, "obj_MaxLength", json_object_new_int(obj_MaxLength));
		if(json_object_object_get(Jobj, "Index")) {
			if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				zcfgFeJsonObjFree(ScheduleJobj);
				break;
			}
		}
		else {
			json_object_array_add(Jarray, JSON_OBJ_COPY(pramJobj));
		}
		zcfgFeJsonObjFree(pramJobj);
		zcfgFeJsonObjFree(ScheduleJobj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalScheduleAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *ScheduleJobj = NULL;
	struct json_object *pramJobj = NULL;
	char *name = NULL;
	char msg[30]={0};
	char path[32] = {0};
	struct json_object *retJobj = NULL;

	// Name validation
	if(!hcbSecur && !ISSET("DontCheckDupName") && json_object_object_get(Jobj, "Name")){
		name = (char *)json_object_get_string(json_object_object_get(Jobj, "Name"));
		ret = dalcmdParamCheck(NULL,name, RDM_OID_SCHEDULE, "Name", "Name", msg);
		if(ret != ZCFG_SUCCESS){
			printf("%s\n",msg);
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SchedulerRule.Input.pop_name_duplicate"));
			return ret; 		
		}
	}


	// Time autofill
	if(!json_object_object_get(Jobj, "TimeStartHour") || !json_object_object_get(Jobj, "TimeStartMin")){
		json_object_object_add(Jobj, "TimeStartHour", json_object_new_int(0));
		json_object_object_add(Jobj, "TimeStartMin", json_object_new_int(0));
		printf("Incomplete start time: set to 00:00\n");
	}
	if(!json_object_object_get(Jobj, "TimeStopHour") || !json_object_object_get(Jobj, "TimeStopMin")){
		json_object_object_add(Jobj, "TimeStopHour", json_object_new_int(23));
		json_object_object_add(Jobj, "TimeStopMin", json_object_new_int(59));
		printf("Incomplete stop time: set to 23:59\n");
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_SCHEDULE, &objIid, NULL)) != ZCFG_SUCCESS){
		if(ret == ZCFG_EXCEED_MAX_INS_NUM){
			printf("Cannot add entry since the maximum number of entries has been reached.\n");
    	}
		return ret;
	}	
	
	if((ret = zcfgFeObjJsonGet(RDM_OID_SCHEDULE, &objIid, &ScheduleJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}	

#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	//json_object_object_add(ScheduleJobj, "Enable", json_object_new_boolean(true));
	if(Jgetb(Jobj, "Enable")){
		json_object_object_add(ScheduleJobj, "Enable", json_object_new_boolean(true));		
		}
	else{
		json_object_object_add(ScheduleJobj, "Enable", json_object_new_boolean(false));		
		}
#else
	json_object_object_add(ScheduleJobj, "Enable", json_object_new_boolean(true));
#endif

	pramJobj = json_object_object_get(Jobj, "Name");
	json_object_object_add(ScheduleJobj, "Name", JSON_OBJ_COPY(pramJobj));
	
	if((pramJobj = json_object_object_get(Jobj, "Description")) != NULL)
		json_object_object_add(ScheduleJobj, "Description", JSON_OBJ_COPY(pramJobj));

	if((pramJobj = json_object_object_get(Jobj, "Days")) != NULL)
		json_object_object_add(ScheduleJobj, "Days", json_object_new_int(schedule_days_string_to_int((char *)json_object_get_string(pramJobj))));
	
	pramJobj = json_object_object_get(Jobj, "TimeStartHour");
	json_object_object_add(ScheduleJobj, "TimeStartHour", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "TimeStartMin");
	json_object_object_add(ScheduleJobj, "TimeStartMin", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "TimeStopHour");
	json_object_object_add(ScheduleJobj, "TimeStopHour", JSON_OBJ_COPY(pramJobj));
	pramJobj = json_object_object_get(Jobj, "TimeStopMin");
	json_object_object_add(ScheduleJobj, "TimeStopMin", JSON_OBJ_COPY(pramJobj));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	if((pramJobj = json_object_object_get(Jobj, "Type")) != NULL)
		json_object_object_add(ScheduleJobj, "Type", JSON_OBJ_COPY(pramJobj));
#endif

	// Time validation
	if(!isValidTimePeriod(
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStartHour")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStartMin")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStopHour")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStopMin"))
		)
	)
	{
		json_object_object_add(ScheduleJobj, "TimeStartHour", json_object_new_int(0));
		json_object_object_add(ScheduleJobj, "TimeStartMin", json_object_new_int(0));
		json_object_object_add(ScheduleJobj, "TimeStopHour", json_object_new_int(23));
		json_object_object_add(ScheduleJobj, "TimeStopMin", json_object_new_int(59));
		printf("Time set to 00:00-23:59..\n");
		/* For need to cease adding
		ret = zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &objIid, NULL);
		json_object_put(ScheduleJobj);
		return ZCFG_INVALID_PARAM_VALUE;
		*/
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_SCHEDULE, &objIid, ScheduleJobj, NULL)) != ZCFG_SUCCESS){
		json_object_put(ScheduleJobj);
		return ret;
	}
	json_object_put(ScheduleJobj);

	if(Jarray != NULL){
		sprintf(path, "X_ZYXEL_Schedule.%d",objIid.idx[0]);
		retJobj = json_object_new_object();
		Jadds(retJobj, "path", path);
		json_object_array_add(Jarray, retJobj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalScheduleEdit(struct json_object *Jobj, char *replyMsg)
{
 	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *ScheduleJobj = NULL;
	struct json_object *pramJobj = NULL;
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	objIndex_t objIid={0};
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));	
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_SCHEDULE, NULL, NULL, replyMsg);
	
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_SCHEDULE, &objIid, &ScheduleJobj)) != ZCFG_SUCCESS) {
		return ret;
	}
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	if((pramJobj = json_object_object_get(Jobj, "Enable")) != NULL)
		json_object_object_add(ScheduleJobj, "Enable", JSON_OBJ_COPY(pramJobj));
#else	
	/*	
	if((pramJobj = json_object_object_get(Jobj, "Enable")) != NULL)
		json_object_object_add(ScheduleJobj, "Enable", JSON_OBJ_COPY(pramJobj));
	*/
#endif	
	if((pramJobj = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobj);
		oldName = (char *)json_object_get_string(json_object_object_get(ScheduleJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_SCHEDULE, "Name", "Name", msg);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(ScheduleJobj, "Name", JSON_OBJ_COPY(pramJobj));
			}
			else{
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SchedulerRule.Input.pop_name_duplicate"));
				json_object_put(ScheduleJobj);
				return ret; 		
			}
		}
	}

	if((pramJobj = json_object_object_get(Jobj, "Description")) != NULL)
		json_object_object_add(ScheduleJobj, "Description", JSON_OBJ_COPY(pramJobj));
	if((pramJobj = json_object_object_get(Jobj, "Days")) != NULL)
		json_object_object_add(ScheduleJobj, "Days", json_object_new_int(schedule_days_string_to_int((char *)json_object_get_string(pramJobj))));
	if((pramJobj = json_object_object_get(Jobj, "TimeStartHour")) != NULL)
		json_object_object_add(ScheduleJobj, "TimeStartHour", JSON_OBJ_COPY(pramJobj));
	if((pramJobj = json_object_object_get(Jobj, "TimeStartMin")) != NULL)
		json_object_object_add(ScheduleJobj, "TimeStartMin", JSON_OBJ_COPY(pramJobj));
	if((pramJobj = json_object_object_get(Jobj, "TimeStopHour")) != NULL)
		json_object_object_add(ScheduleJobj, "TimeStopHour", JSON_OBJ_COPY(pramJobj));
	if((pramJobj = json_object_object_get(Jobj, "TimeStopMin")) != NULL)
		json_object_object_add(ScheduleJobj, "TimeStopMin", JSON_OBJ_COPY(pramJobj));

	if(!isValidTimePeriod(
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStartHour")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStartMin")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStopHour")),
			json_object_get_int(json_object_object_get(ScheduleJobj, "TimeStopMin"))
		)
	)
	{
		json_object_put(ScheduleJobj);
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_SCHEDULE, &objIid, ScheduleJobj, NULL)) != ZCFG_SUCCESS);

	json_object_put(ScheduleJobj);
	return ret;
}

zcfgRet_t zcfgFeDalScheduleDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex;
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_SCHEDULE, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	ret = zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &objIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SchedulerRule.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		return ret; 
	}
	return ret;
}

zcfgRet_t zcfgFeDalSchedule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalScheduleEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalScheduleAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalScheduleDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalScheduleGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

