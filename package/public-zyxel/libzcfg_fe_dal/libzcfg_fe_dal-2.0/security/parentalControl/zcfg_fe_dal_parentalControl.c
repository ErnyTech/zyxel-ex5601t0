#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

//#define PARENT_DEBUG

#ifdef PARENT_DEBUG
#define PARENT_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define PARENT_PRINT(format, ...)
#endif


dal_param_t PAREN_CTL_param[]={
	//Parental Ctl
	{"PrentalCtlEnable",	dalType_boolean,	0,	0,	NULL, NULL, dal_Add_ignore},
	//Parental Ctl Profile
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add_ignore|dal_Delete},
	{"pcpEnable",			dalType_boolean,	0,	0,	NULL},
	{"pcpName",				dalType_string,	1,	64,	NULL, NULL, dal_Add},
	{"WebRedirect",			dalType_boolean,	0,	0,	NULL},
	{"ServicePolicy",		dalType_string,	0,	0,	NULL, "Block|Allow"},
	//{"Type",		dalType_int,	0,	0,	NULL},
	{"UrlPolicy",			dalType_string,	0,	0,	NULL, "Block|Allow"},
	{"MACAddressList",		dalType_MacAddrList,	1,	32,	NULL, "All", dal_Add},
	{"ScheduleRuleList",	dalType_string,	0,	64,	NULL, NULL, dalcmd_Forbid},
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	{"Days",				dalType_string,	0,	0,	NULL, NULL, dal_Add},
	{"TimeRangeList",		dalType_TimeRangeList,	1,	8,	NULL, NULL, dal_Add},
#else
	{"Days",				dalType_DayofWeek,	1,	3,	NULL, NULL, dal_Add},
	{"TimeRangeList",		dalType_TimeRangeList,	1,	3,	NULL, NULL, dal_Add},
#endif	
	{"URLFilter",			dalType_URLFilterList,	0,	580,	NULL, "Empty"},
	{"NetworkServiceList",	dalType_ParenCtlServiceList,	1,	12,	NULL, "Empty"},
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	//TAAAB customized
	{"TTNetworkServiceList",dalType_ParenCtlServiceList,	1,	12,	NULL, "Empty"},
	{"TTServicePolicy",		dalType_string,	0,	0,	NULL, "Block|Allow"},
#endif	
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	{"TimeType",	dalType_string,	0,	0,	NULL, NULL},
	{"TimeEnable",	dalType_string,	0,	0,	NULL, NULL},	
	{"ContentFilterList",	dalType_string,	0,	0,	NULL, NULL},
	{"Id",	dalType_string,	0,	0,	NULL, NULL},
#endif		
	//Schedule
#if 0
	{"Enable",				dalType_boolean,	0,	0,	NULL},
	{"Name",				dalType_string,	0,	0,	NULL},
	{"Description",			dalType_string,	0,	0,	NULL},
	{"Days",				dalType_int,	0,	0,	NULL},
	{"TimeStartHour",		dalType_int,	0,	0,	NULL},
	{"TimeStartMin",		dalType_int,	0,	0,	NULL},
	{"TimeStopHour",		dalType_int,	0,	0,	NULL},
	{"TimeStopMin",			dalType_int,	0,	0,	NULL},
#endif
	{NULL,		0,	0,	0,	NULL},
};

struct json_object *schTimeObj = NULL;
struct json_object *parCtlObj = NULL;
struct json_object *pcpObj = NULL;

objIndex_t schTimeObjIid = {0};
objIndex_t parCtlObjIid = {0};
objIndex_t pcpObjIid = {0};

//Parental Ctl Profile
bool pcpEnable;
const char* pcpName;
bool WebRedirect;
const char* ServicePolicy;
int Type;
const char* MACAddressList;
char ScheduleRuleList[512] = {0};
const char* URLFilter;
const char* NetworkServiceList;
#ifdef ZYXEL_TAAAB_CUSTOMIZATION//TAAAB
const char* TTNetworkServiceList;
const char* TTServicePolicy;
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
const char* ContentFilterList;
const char* Id;
const char* TimeEnable; /*Use the schedule Enable, for TimeEnable*/
const char* TimeType;  /*Add the TimeType*/

#endif
//Parental Ctl
bool PrentalCtlEnable;

//Schedule
bool Enable;
const char* Name;
const char* Description;
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
char *Days;
#else
int Days;
#endif
int TimeStartHour;
int TimeStartMin;
int TimeStopHour;
int TimeStopMin;

char curScheduleRuleList[129] = {0};
time_t t1, t2;

zcfgRet_t freeAllPcpObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;
			
	if(schTimeObj) json_object_put(schTimeObj); 
	if(parCtlObj) json_object_put(parCtlObj);
	if(pcpObj) json_object_put(pcpObj);
			
	return ret;
}

void getBasicPcpInfo(struct json_object *Jobj){
	
	//Parental Ctl Profile
	pcpEnable = json_object_get_boolean(json_object_object_get(Jobj, "pcpEnable"));
	pcpName = json_object_get_string(json_object_object_get(Jobj, "pcpName"));
	WebRedirect = json_object_get_boolean(json_object_object_get(Jobj, "WebRedirect"));
	ServicePolicy = json_object_get_string(json_object_object_get(Jobj, "ServicePolicy"));
	Type = json_object_get_int(json_object_object_get(Jobj, "Type"));
	MACAddressList = json_object_get_string(json_object_object_get(Jobj, "MACAddressList"));
	URLFilter = json_object_get_string(json_object_object_get(Jobj, "URLFilter"));
	NetworkServiceList = json_object_get_string(json_object_object_get(Jobj, "NetworkServiceList"));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	TTServicePolicy = json_object_get_string(json_object_object_get(Jobj, "TTServicePolicy"));
	TTNetworkServiceList = json_object_get_string(json_object_object_get(Jobj, "TTNetworkServiceList"));
#endif	
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	ContentFilterList = json_object_get_string(json_object_object_get(Jobj, "ContentFilterList"));
	Id = json_object_get_string(json_object_object_get(Jobj, "Id"));
	TimeType = json_object_get_string(json_object_object_get(Jobj, "TimeType"));
	TimeEnable = json_object_get_string(json_object_object_get(Jobj, "TimeEnable"));	
#endif	
	//Parental Ctl
	PrentalCtlEnable = json_object_get_boolean(json_object_object_get(Jobj, "PrentalCtlEnable"));
	
	return;
}

void getBasicSchTimeInfo(struct json_object *Jobj){
		
	//Schedule
	Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	Name = json_object_get_string(json_object_object_get(Jobj, "Name"));
	Description = json_object_get_string(json_object_object_get(Jobj, "Description"));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	Days = (char *)json_object_get_string(json_object_object_get(Jobj, "Days"));
#else
	Days = json_object_get_int(json_object_object_get(Jobj, "Days"));
#endif
	TimeStartHour = json_object_get_int(json_object_object_get(Jobj, "TimeStartHour"));
	TimeStartMin = json_object_get_int(json_object_object_get(Jobj, "TimeStartMin"));
	TimeStopHour = json_object_get_int(json_object_object_get(Jobj, "TimeStopHour"));
	TimeStopMin = json_object_get_int(json_object_object_get(Jobj, "TimeStopMin"));
	
	return;
}

void initGlobalPcpObjects(){
		
	schTimeObj = NULL;
	parCtlObj = NULL;
	pcpObj = NULL;
		
	IID_INIT(schTimeObjIid);
	IID_INIT(parCtlObjIid);
	IID_INIT(pcpObjIid);
		
}

extern zcfgRet_t zcfgFeDalScheduleGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg); //implement in zcfg_fe_dal_schedulerRule.c
zcfgRet_t parsePcpObject(struct json_object *pramJobj, struct json_object *tr181Jobj){
	const char *ServicePolicy;
	int Type;
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	char buf[256] = {0};
	
	char timeEnableList[256] = {0};
	char timeDaysList[256] = {0};	
	char timeTypeList[256] = {0};

	char timeEnable[128] = {0};
	boolean timEnableVal = 0;
	char timeType[128] = {0};	
	char timeDays[256] = {0};
	
	//char *ptr2, *tmp2,*ptr3, *tmp3,*ptr4, *tmp4;				
#else
	char buf[128] = {0};
#endif
	char timeRange[16] = {0};
	char timeRangeList[64] = {0};
#if !defined(ZYXEL_HOME_CYBER_SECURITY) || defined(ZYXEL_USA_PRODUCT)
	char *_day = NULL;
#endif
	char *ptr, *tmp;
	struct json_object *schJarray;
	struct json_object *schPramJobj;
	int len, i, schFound;
	//int schIdx, schIndex;
	int startHour = 0, startMin = 0, stopHour = 0, stopMin = 0;
	const char *TR181Path;
	struct json_object *tmpValue = NULL;

	if(NULL == tr181Jobj || ( (NULL != tr181Jobj) && ( false == json_object_object_get_ex(tr181Jobj, "Enable", &tmpValue)) ) )
	{
		printf("%s: empty object\n",__FUNCTION__);
		return ZCFG_SUCCESS;
	}
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	//TAAAB
	const char *TTServicePolicy;
#endif
	ServicePolicy = Jgets(tr181Jobj, "ServicePolicy");
	Type = Jgeti(tr181Jobj, "Type");

	if(!strcmp(ServicePolicy, "Drop"))
		json_object_object_add(pramJobj, "ServicePolicy", json_object_new_string("Block"));
	else
		json_object_object_add(pramJobj, "ServicePolicy", json_object_new_string("Allow"));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	TTServicePolicy = Jgets(tr181Jobj, "TTServicePolicy");
	if(!strcmp(TTServicePolicy, "Drop"))
		json_object_object_add(pramJobj, "TTServicePolicy", json_object_new_string("Block"));
	else
		json_object_object_add(pramJobj, "TTServicePolicy", json_object_new_string("Allow"));
#endif	
	if(Type == 0)
		json_object_object_add(pramJobj, "UrlPolicy", json_object_new_string("Block"));
	else
		json_object_object_add(pramJobj, "UrlPolicy", json_object_new_string("Allow"));

	replaceParam(pramJobj, "pcpName", tr181Jobj, "Name");
	replaceParam(pramJobj, "pcpEnable", tr181Jobj, "Enable");
	replaceParam(pramJobj, "WebRedirect", tr181Jobj, "WebRedirect");
	replaceParam(pramJobj, "UrlPolicy", tr181Jobj, "UrlPolicy");
	replaceParam(pramJobj, "MACAddressList", tr181Jobj, "MACAddressList");
	replaceParam(pramJobj, "ScheduleRuleList", tr181Jobj, "ScheduleRuleList");
	replaceParam(pramJobj, "URLFilter", tr181Jobj, "URLFilter");
	replaceParam(pramJobj, "NetworkServiceList", tr181Jobj, "NetworkServiceList");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	replaceParam(pramJobj, "TTNetworkServiceList", tr181Jobj, "TTNetworkServiceList");
#endif	
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	replaceParam(pramJobj, "ContentFilterList", tr181Jobj, "ContentFilterList");
	replaceParam(pramJobj, "Id", tr181Jobj, "Id");

#endif	

	schJarray = json_object_new_array();
	zcfgFeDalScheduleGet(NULL, schJarray, NULL);
	len = json_object_array_length(schJarray);
	strcpy(buf, Jgets(tr181Jobj, "ScheduleRuleList"));
	ptr = strtok_r(buf, ",", &tmp);
	while(ptr != NULL){
		//sscanf(ptr, "X_ZYXEL_Schedule.%d", &schIndex);
		schFound = 0;
		for(i=0;i<len;i++){
			schPramJobj = json_object_array_get_idx(schJarray, i);
			TR181Path = Jgets(schPramJobj, "TR181Path");
			//if(schIdx == schIndex){
			if(!strcmp(ptr, TR181Path)){
				schFound = 1;
				break;
			}
		}
		if(!schFound)
			continue;
#if !defined(ZYXEL_HOME_CYBER_SECURITY) || defined(ZYXEL_USA_PRODUCT)
		if(_day == NULL)
			_day = (char *)Jgets(schPramJobj, "Days");
#endif		
		startHour = json_object_get_int(json_object_object_get(schPramJobj, "TimeStartHour"));
		startMin = json_object_get_int(json_object_object_get(schPramJobj, "TimeStartMin"));
		stopHour = json_object_get_int(json_object_object_get(schPramJobj, "TimeStopHour"));
		stopMin = json_object_get_int(json_object_object_get(schPramJobj, "TimeStopMin"));
		sprintf(timeRange, "%02d:%02d-%02d:%02d",startHour,startMin,stopHour,stopMin);
		strcat(timeRangeList, timeRange);
		strcat(timeRangeList, ",");
		ptr = strtok_r(NULL, ",", &tmp);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)

		timEnableVal = json_object_get_boolean(json_object_object_get(schPramJobj, "Enable"));
		sprintf(timeEnable,"%d",timEnableVal);	
		strcat(timeEnableList, timeEnable);
		strcat(timeEnableList, ",");
		//ptr2 = strtok_r(NULL, ",", &tmp2);

		if(Jgets(schPramJobj, "Type") != NULL){
			strcpy(timeType, Jgets(schPramJobj, "Type"));	
			strcat(timeTypeList, timeType);
			strcat(timeTypeList, ",");
			//ptr3 = strtok_r(NULL, ",", &tmp3);
		}

		if(Jgets(schPramJobj, "Days") != NULL){
			strcpy(timeDays, Jgets(schPramJobj, "Days"));
			strcat(timeDaysList, timeDays);
			strcat(timeDaysList, ": ");
			//ptr4 = strtok_r(NULL, ":", &tmp4);	
		}
#endif
	}
#if !defined(ZYXEL_HOME_CYBER_SECURITY) || defined(ZYXEL_USA_PRODUCT)
	if(_day == NULL)
		_day = "";
#endif		
	if(strlen(timeRangeList)>0)
	{
		timeRangeList[strlen(timeRangeList)-1]= '\0';
	
	json_object_object_add(pramJobj, "TimeRangeList", json_object_new_string(timeRangeList));
	}
	else
	{
		printf("%s: empty Schedule list\n",__FUNCTION__);
		if(schJarray)
		{
			json_object_put(schJarray);
		}
		return ZCFG_SUCCESS;
	}
#if !defined(ZYXEL_HOME_CYBER_SECURITY) || defined(ZYXEL_USA_PRODUCT)
	json_object_object_add(pramJobj, "Days", json_object_new_string(_day));
#endif
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	if(strlen(timeEnableList)>0)
		timeEnableList[strlen(timeEnableList)-1]= '\0';	
	json_object_object_add(pramJobj, "TimeEnable", json_object_new_string(timeEnableList));
	if(strlen(timeTypeList)>0)
		timeTypeList[strlen(timeTypeList)-1]= '\0';	
	json_object_object_add(pramJobj, "TimeType", json_object_new_string(timeTypeList));
	if(strlen(timeDaysList)>0)
		timeDaysList[strlen(timeDaysList)-1]= '\0';	
	json_object_object_add(pramJobj, "Days", json_object_new_string(timeDaysList));
#endif
	json_object_put(schJarray);
	return ZCFG_SUCCESS;
}

zcfgRet_t editPcObject(struct json_object *Jobj){
	
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL, &parCtlObjIid, &parCtlObj)) != ZCFG_SUCCESS) {
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_pcp_err"));		
		return ret;
	}
	if (PrentalCtlEnable)
		json_object_object_add(parCtlObj, "Enable", json_object_new_boolean(true));
	else
		json_object_object_add(parCtlObj, "Enable", json_object_new_boolean(false));

	if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL, &parCtlObjIid, parCtlObj, NULL)) != ZCFG_SUCCESS){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.set_pcp_err"));		
		return ret;
	}
	return ret;
}

extern zcfgRet_t zcfgFeDalScheduleAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg); //implement in zcfg_fe_dal_schedulerRule.c
zcfgRet_t addPcpObject(struct json_object *Jobj){
		
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *reqSchRuleObjArray = NULL;
	//struct json_object *reqSchRuleObj= NULL;
	uint8_t iidIdx =0;
	char buf[128] = {0};
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	char buf2[128] = {0};
	char buf3[128] = {0};
	char buf4[256] = {0};
	char *ptr2=NULL,*ptr3=NULL,*tmp2=NULL,*tmp3=NULL,*ptr4=NULL,*tmp4=NULL;
	char path[32] = {0};
#endif
	char *ptr, *tmp;
	int count, startHour, startMin, stopHour, stopMin;
	char schRuleName[64] = {0};
	struct json_object *schRuleJobj = NULL;
	struct json_object *schRuleRetJarray = NULL;
	const char *schRulePath;

	IID_INIT(pcpObjIid);
	zcfgFeObjJsonAdd(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, NULL);
#ifdef ZYXEL_HOME_CYBER_SECURITY
	PARENT_PRINT("%s: iid=[%d,%d,%d]\n",__FUNCTION__,pcpObjIid.idx[0],pcpObjIid.idx[1],pcpObjIid.idx[2]);
#endif
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, &pcpObj)) != ZCFG_SUCCESS) {
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.Max_pctl_prof_length_reached"));
		return ret;
	}
	
	ScheduleRuleList[0] = '\0';
	reqSchRuleObjArray = json_object_object_get(Jobj, "reqSchRule");
	if(reqSchRuleObjArray != NULL){//request form GUI
		for(iidIdx = 0; iidIdx < json_object_array_length(reqSchRuleObjArray); iidIdx++){
			Enable = json_object_get_boolean(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Enable"));
			Name = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Name"));
			Description = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Description"));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
//Days = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Days"));
			Days = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Days"));
#else
			Days = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Days"));
#endif
			TimeStartHour = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStartHour"));
			TimeStartMin = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStartMin"));
			TimeStopHour = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStopHour"));
			TimeStopMin = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStopMin"));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			TimeType = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeType"));			
			TimeEnable = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeEnable"));			
#endif
			IID_INIT(schTimeObjIid);
			zcfgFeObjJsonAdd(RDM_OID_SCHEDULE, &schTimeObjIid, NULL);
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SCHEDULE, &schTimeObjIid, &schTimeObj)) != ZCFG_SUCCESS) {
				return ret;
			}	
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			//json_object_object_add(schTimeObj, "Enable", json_object_new_boolean(TimeEnable));
			json_object_object_add(schTimeObj, "Enable", json_object_new_boolean(true));
#else			
			json_object_object_add(schTimeObj, "Enable", json_object_new_boolean(Enable));
#endif
			json_object_object_add(schTimeObj, "Name", json_object_new_string(Name));
			json_object_object_add(schTimeObj, "Description", json_object_new_string(Description));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			//json_object_object_add(schTimeObj, "Days", json_object_new_int(Days));
			json_object_object_add(schTimeObj, "Days", json_object_new_string(Days));
#else
			json_object_object_add(schTimeObj, "Days", json_object_new_int(Days));
#endif			
			json_object_object_add(schTimeObj, "TimeStartHour", json_object_new_int(TimeStartHour));
			json_object_object_add(schTimeObj, "TimeStartMin", json_object_new_int(TimeStartMin));		
			json_object_object_add(schTimeObj, "TimeStopHour", json_object_new_int(TimeStopHour));
			json_object_object_add(schTimeObj, "TimeStopMin", json_object_new_int(TimeStopMin));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			json_object_object_add(schTimeObj, "Type", json_object_new_string(TimeType));
#endif			
				
			if((ret = zcfgFeObjJsonSet(RDM_OID_SCHEDULE, &schTimeObjIid, schTimeObj, NULL)) != ZCFG_SUCCESS)
			{
				return ret;
			}
				
			char temp[20]  = {0};
			
			if(ScheduleRuleList[0] == '\0'){
				sprintf(temp, "X_ZYXEL_Schedule.%d", schTimeObjIid.idx[0]);
			}
			else{
				sprintf(temp, ",X_ZYXEL_Schedule.%d", schTimeObjIid.idx[0]);
			}
			
			strcat(ScheduleRuleList, temp);
		
		}
	}
	else{//CLI, not only for CLI, also for the WebGUI
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)	
		if(Jgets(Jobj, "Days") != NULL){  
			strcpy(buf4, Jgets(Jobj, "Days"));			 			
			ptr4 = strtok_r(buf4, ":", &tmp4);
		}
		if(Jgets(Jobj, "TimeType") != NULL){
			strcpy(buf2, Jgets(Jobj, "TimeType"));			
			ptr2 = strtok_r(buf2, ",", &tmp2);			
		}
		
		if(Jgets(Jobj, "TimeEnable") != NULL){  
			strcpy(buf3, Jgets(Jobj, "TimeEnable"));				
			ptr3 = strtok_r(buf3, ",", &tmp3);
        }
		

#endif

		strcpy(buf, Jgets(Jobj, "TimeRangeList"));
		count = 0;
		ptr = strtok_r(buf, ",", &tmp);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		while((ptr != NULL)&&(ptr2 != NULL)&&(ptr3 != NULL)&&(ptr4 != NULL)){
#else		
		while(ptr != NULL){
#endif		
			parseTimeRange(ptr, &startHour, &startMin, &stopHour, &stopMin);
			count++;
			schRuleJobj = json_object_new_object();
			schRuleRetJarray = json_object_new_array();
			
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)	
			if((ptr3 == NULL)||(!strcmp(ptr3,"false"))||(!strcmp(ptr3,"0"))){
				Jaddb(schRuleJobj, "Enable", false);				
			}else if ((!strcmp(ptr3,"true"))||(!strcmp(ptr3,"1"))){
				Jaddb(schRuleJobj, "Enable", true);				
			}else{
				Jaddb(schRuleJobj, "Enable", true);					
			}
#else
			Jaddb(schRuleJobj, "Enable", true);
#endif
			sprintf(schRuleName, "%s_%d", Jgets(Jobj, "pcpName"), count);
			Jadds(schRuleJobj, "Name", schRuleName);
			Jadds(schRuleJobj, "Description", "ParentalControl");
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			//replaceParam(schRuleJobj, "Days", Jobj, "Days");
			Jadds(schRuleJobj, "Days", ptr4);		
#else			
			replaceParam(schRuleJobj, "Days", Jobj, "Days");
#endif
			Jaddi(schRuleJobj, "TimeStartHour", startHour);
			Jaddi(schRuleJobj, "TimeStartMin", startMin);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			if(ptr2 == NULL)
				Jadds(schRuleJobj, "Type", "NULL");
			else
				Jadds(schRuleJobj, "Type", ptr2);   		
#endif
			if (stopHour >= 24) {
				stopHour = 23;
				stopMin = 59;
			}

			Jaddi(schRuleJobj, "TimeStopHour", stopHour);
			Jaddi(schRuleJobj, "TimeStopMin", stopMin);
			if((ret = zcfgFeDalScheduleAdd(schRuleJobj, schRuleRetJarray, NULL)) != ZCFG_SUCCESS){
				json_object_put(schRuleJobj);
				json_object_put(schRuleRetJarray);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.add_schedule_err"));
				return ret;
			}
			schRulePath = Jgets(json_object_array_get_idx(schRuleRetJarray, 0), "path");
			strcat(ScheduleRuleList, schRulePath);
			strcat(ScheduleRuleList, ",");
			ptr = strtok_r(NULL, ",", &tmp);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)			
			ptr2 = strtok_r(NULL, ",", &tmp2);
			ptr3 = strtok_r(NULL, ",", &tmp3);			
			ptr4 = strtok_r(NULL, ":", &tmp4);						
#endif			
			json_object_put(schRuleJobj);
			json_object_put(schRuleRetJarray);
		}
		if(strlen(ScheduleRuleList)>0){
			ScheduleRuleList[strlen(ScheduleRuleList)-1]= '\0';
		}
	}

#ifdef ZYXEL_HOME_CYBER_SECURITY	
	PARENT_PRINT("%s: id=%s\n",__FUNCTION__,json_object_get_string(json_object_object_get(pcpObj,"Id")))	;
#endif
	//add default value for optional parameter
	if(!ISSET("ServicePolicy"))
		Jadds(Jobj, "ServicePolicy", "Block");
	if(!ISSET("UrlPolicy"))
		Jadds(Jobj, "UrlPolicy", "Block");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	if(!ISSET("TTServicePolicy"))
		Jadds(Jobj, "TTServicePolicy", "Block");
#endif		
	replaceParam(pcpObj, "Enable", Jobj, "pcpEnable");
	replaceParam(pcpObj, "Name", Jobj, "pcpName");
	replaceParam(pcpObj, "WebRedirect", Jobj, "WebRedirect");
	if(!strcmp(Jgets(Jobj, "ServicePolicy"), "Block"))
		Jadds(pcpObj, "ServicePolicy", "Drop");
	else if(!strcmp(Jgets(Jobj, "ServicePolicy"), "Allow"))
		Jadds(pcpObj, "ServicePolicy", "Accept");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	if(!strcmp(Jgets(Jobj, "TTServicePolicy"), "Block"))
		Jadds(pcpObj, "TTServicePolicy", "Drop");
	else if(!strcmp(Jgets(Jobj, "TTServicePolicy"), "Allow"))
		Jadds(pcpObj, "TTServicePolicy", "Accept");
#endif	
	if(!strcmp(Jgets(Jobj, "UrlPolicy"), "Block"))
		Jaddi(pcpObj, "Type", 0);
	else if(!strcmp(Jgets(Jobj, "UrlPolicy"), "Allow"))
		Jaddi(pcpObj, "Type", 1);

	if(!strcmp(Jgets(Jobj, "MACAddressList"), "All"))
		Jadds(pcpObj, "MACAddressList", "00:00:00:00:00:00");
	else
		replaceParam(pcpObj, "MACAddressList", Jobj, "MACAddressList");
	
	//replaceParam(pcpObj, "URLFilter", Jobj, "URLFilter");
	URLFilter = Jgets(Jobj,"URLFilter");
	if(URLFilter && !strcmp(URLFilter, "Empty"))
		Jadds(pcpObj, "URLFilter", "");
	else
		replaceParam(pcpObj, "URLFilter", Jobj, "URLFilter");

	NetworkServiceList = Jgets(Jobj,"NetworkServiceList");
	if(NetworkServiceList && !strcmp(NetworkServiceList, "Empty"))
		Jadds(pcpObj, "NetworkServiceList", "");
	else
		replaceParam(pcpObj, "NetworkServiceList", Jobj, "NetworkServiceList");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	TTNetworkServiceList = Jgets(Jobj,"TTNetworkServiceList");
	if(TTNetworkServiceList && !strcmp(TTNetworkServiceList, "Empty"))
		Jadds(pcpObj, "TTNetworkServiceList", "");
	else
		replaceParam(pcpObj, "TTNetworkServiceList", Jobj, "TTNetworkServiceList");
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	if((ContentFilterList = Jgets(Jobj, "ContentFilterList")) == NULL){
		sprintf(path, "X_ZYXEL_ContentFilter.%d",pcpObjIid.idx[0]);
		Jadds(pcpObj, "ContentFilterList", path);
	}else
		replaceParam(pcpObj, "ContentFilterList", Jobj, "ContentFilterList");
	if((Id = Jgets(Jobj, "Id")) == NULL){
		Jadds(pcpObj, "Id", "");
	}else
		replaceParam(pcpObj, "Id", Jobj, "Id");	
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	json_object_object_add(pcpObj, "TTServicePolicy", json_object_new_string(TTServicePolicy));
#endif		
	if(ScheduleRuleList != NULL)
		json_object_object_add(pcpObj, "ScheduleRuleList", json_object_new_string(ScheduleRuleList));
	else 
		return ZCFG_INVALID_FORMAT;
#ifdef ZYXEL_HOME_CYBER_SECURITY
	if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL)) != ZCFG_SUCCESS){
#else	
	if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL)) != ZCFG_SUCCESS){
#endif	
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.set_pcp_err"));
		return ret;
	}
#if 0
	json_object_object_add(pcpObj, "Enable", json_object_new_boolean(pcpEnable));
	json_object_object_add(pcpObj, "Name", json_object_new_string(pcpName));
	json_object_object_add(pcpObj, "WebRedirect", json_object_new_boolean(WebRedirect));
	json_object_object_add(pcpObj, "ServicePolicy", json_object_new_string(ServicePolicy));
	json_object_object_add(pcpObj, "Type", json_object_new_int(Type));
	json_object_object_add(pcpObj, "MACAddressList", json_object_new_string(MACAddressList));
	json_object_object_add(pcpObj, "URLFilter", json_object_new_string(URLFilter));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	json_object_object_add(pcpObj, "TTServicePolicy", json_object_new_string(TTServicePolicy));
#endif	
	if(ScheduleRuleList != NULL)
		json_object_object_add(pcpObj, "ScheduleRuleList", json_object_new_string(ScheduleRuleList));
	else 
		return ret = ZCFG_INVALID_FORMAT;
	json_object_object_add(pcpObj, "NetworkServiceList", json_object_new_string(NetworkServiceList));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION		
	json_object_object_add(pcpObj, "TTNetworkServiceList", json_object_new_string(TTNetworkServiceList));
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL)) != ZCFG_SUCCESS){
#else
	if((ret = zcfgFeObjJsonSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL)) != ZCFG_SUCCESS){
#endif	
			return ret;
	}
#endif

	return ret;
}
zcfgRet_t curPcpSchRuleDel(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	char* buffer = NULL;
	buffer = strtok (curScheduleRuleList, ",");
	
	if ((buffer == NULL) && strstr(curScheduleRuleList, "X_ZYXEL_Schedule."))
	{
		schTimeObjIid.level = 1;
		sscanf(curScheduleRuleList, "X_ZYXEL_Schedule.%hhu", &schTimeObjIid.idx[0]);
		
		zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &schTimeObjIid, NULL);
		
	}else
	{
		while (buffer) {
			schTimeObjIid.level = 1;
			sscanf(buffer, "X_ZYXEL_Schedule.%hhu", &schTimeObjIid.idx[0]);
			zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &schTimeObjIid, NULL);			
			buffer = strtok (NULL, ",");
		}
	}
	return ret;
	
}

zcfgRet_t zcfgFeDalParentCtlGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
zcfgRet_t editPcpObject(struct json_object *Jobj, char *replyMsg){
		
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *reqSchRuleObjArray = NULL;
	struct json_object *reqSchRuleIidArray = NULL;
	struct json_object *pcpIidArray = NULL;
	ScheduleRuleList[0] = '\0';
	uint8_t iidIdx =0;
	char buf[128] = {0};
#ifdef ZYXEL_HOME_CYBER_SECURITY
	char buf2[128] = {0};
	char buf3[128] = {0};
	char buf4[256] = {0};	
	char *ptr2=NULL,*ptr3=NULL,*tmp2=NULL,*tmp3=NULL,*ptr4=NULL,*tmp4=NULL;
#endif
	char *ptr, *tmp;
	int count, startHour, startMin, stopHour, stopMin;
	char schRuleName[64] = {0};
	struct json_object *schRuleJobj = NULL;
	struct json_object *schRuleRetJarray = NULL;
	const char *schRulePath;
	struct json_object *inputJobj = NULL;
	struct json_object *curJobj = NULL;
	struct json_object *curJarray = NULL;

	pcpIidArray = json_object_object_get(Jobj, "iid");

	if(pcpIidArray != NULL){ //request form GUI
		for(iidIdx = 0; iidIdx < json_object_array_length(pcpIidArray); iidIdx++){
			pcpObjIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(pcpIidArray, iidIdx));
			if(pcpObjIid.idx[iidIdx] != 0)
				pcpObjIid.level = iidIdx+1;
		}
	}
	else{ //request form CLI
		if(!ISSET("Index")){
			if(replyMsg)
				strcpy(replyMsg, "Need specify --Index");
			return ZCFG_INVALID_ARGUMENTS;
		}
		IID_INIT(pcpObjIid);
		pcpObjIid.level = 1;
		pcpObjIid.idx[0] = Jgeti(Jobj, "Index");
		
		inputJobj = json_object_new_object();
		curJarray = json_object_new_array();
		Jaddi(inputJobj, "Index", pcpObjIid.idx[0]);
		if((ret = zcfgFeDalParentCtlGet(inputJobj, curJarray, NULL)) != ZCFG_SUCCESS){
			if(replyMsg)
				sprintf(replyMsg, "Can't find --Index %d", pcpObjIid.idx[0]);
			json_object_put(inputJobj);
			//Horace Add begin
			json_object_put(curJarray);
			//Horace Add end
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_pcp_err"));		
			return ret;
		}
		json_object_put(inputJobj);
		curJobj = json_object_array_get_idx(curJarray, 0);
	}
#ifdef ZYXEL_HOME_CYBER_SECURITY
	PARENT_PRINT("%s: inputobj = %s\n",__FUNCTION__, (char *)json_object_get_string(Jobj));
#endif	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, &pcpObj)) != ZCFG_SUCCESS) {
		json_object_put(curJarray);
		if(replyMsg)
			sprintf(replyMsg, "Can't find --Index %d", pcpObjIid.idx[0]);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_pcp_err"));		
		return ret;
	}
				
	reqSchRuleObjArray = json_object_object_get(Jobj, "reqSchRule");
	reqSchRuleIidArray = json_object_object_get(Jobj, "reqSchRuleIid");
	if(reqSchRuleObjArray != NULL){//request form GUI
		for(iidIdx = 0; iidIdx < json_object_array_length(reqSchRuleIidArray); iidIdx++){
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			TimeEnable = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeEnable"));
#else		
			Enable = json_object_get_boolean(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Enable"));
#endif			
			Name = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Name"));
			Description = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Description"));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			Days = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Days"));
#else
			Days = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "Days"));
#endif
			TimeStartHour = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStartHour"));
			TimeStartMin = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStartMin"));
			TimeStopHour = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStopHour"));
			TimeStopMin = json_object_get_int(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeStopMin"));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			TimeType = json_object_get_string(json_object_object_get(json_object_array_get_idx(reqSchRuleObjArray, iidIdx), "TimeType"));
#endif
			
			schTimeObjIid.idx[0] = json_object_get_int( json_object_array_get_idx(reqSchRuleIidArray, iidIdx));
			schTimeObjIid.level = 1;
			if(schTimeObjIid.idx[0] == 0)
			{
				IID_INIT(schTimeObjIid);
				zcfgFeObjJsonAdd(RDM_OID_SCHEDULE, &schTimeObjIid, NULL);
			}
			
			if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_SCHEDULE, &schTimeObjIid, &schTimeObj)) != ZCFG_SUCCESS) {
				return ret;
			}
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			json_object_object_add(schTimeObj, "Enable", json_object_new_string(TimeEnable));			
			//json_object_object_add(schTimeObj, "Enable", json_object_new_boolean(true));
#else				
			json_object_object_add(schTimeObj, "Enable", json_object_new_boolean(Enable));
#endif
			json_object_object_add(schTimeObj, "Name", json_object_new_string(Name));
			json_object_object_add(schTimeObj, "Description", json_object_new_string(Description));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			json_object_object_add(schTimeObj, "Days", json_object_new_string(Days));
#else
			json_object_object_add(schTimeObj, "Days", json_object_new_int(Days));
#endif
			json_object_object_add(schTimeObj, "TimeStartHour", json_object_new_int(TimeStartHour));
			json_object_object_add(schTimeObj, "TimeStartMin", json_object_new_int(TimeStartMin));		
			json_object_object_add(schTimeObj, "TimeStopHour", json_object_new_int(TimeStopHour));
			json_object_object_add(schTimeObj, "TimeStopMin", json_object_new_int(TimeStopMin));
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			json_object_object_add(schTimeObj, "Type", json_object_new_string(TimeType));
#endif
				
			if((ret = zcfgFeObjJsonSet(RDM_OID_SCHEDULE, &schTimeObjIid, schTimeObj, NULL)) != ZCFG_SUCCESS)
			{
				return ret;
			}
				
			char temp[20]  = {0};
			if(ScheduleRuleList[0] == '\0'){
				snprintf(temp, sizeof(temp), "X_ZYXEL_Schedule.%d", schTimeObjIid.idx[0]);
			}
			else{
				snprintf(temp, sizeof(temp), ",X_ZYXEL_Schedule.%d", schTimeObjIid.idx[0]);
			}
				
	        strcat(ScheduleRuleList, temp);
				
		}
	}
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	else if(ISSET("TimeRangeList") || ISSET("Days") || ISSET("TimeType")|| ISSET("TimeEnable")){//request form CLI
#else	
	else if(ISSET("TimeRangeList") || ISSET("Days")){//request form CLI
#endif
		memset(ScheduleRuleList, 0,sizeof(ScheduleRuleList));
		if(ISSET("TimeRangeList"))
			strcpy(buf, Jgets(Jobj, "TimeRangeList"));
		else{
			strcpy(buf, Jgets(curJobj, "TimeRangeList"));
		}

		count = 0;
		ptr = strtok_r(buf, ",", &tmp);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		if(Jgets(Jobj, "Days") != NULL){  
			strcpy(buf4, Jgets(Jobj, "Days"));						
			ptr4 = strtok_r(buf4, ":", &tmp4);
		}
		if(Jgets(Jobj, "TimeType") != NULL){
			strcpy(buf2, Jgets(Jobj, "TimeType"));			
			ptr2 = strtok_r(buf2, ",", &tmp2);			
		}
		if(Jgets(Jobj, "TimeEnable") != NULL){	
			strcpy(buf3, Jgets(Jobj, "TimeEnable"));				
			ptr3 = strtok_r(buf3, ",", &tmp3);
		}
#endif
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
		while((ptr != NULL)&&(ptr2 != NULL)&&(ptr3 != NULL)&&(ptr4 != NULL)){
#else
		while(ptr != NULL){
#endif
			parseTimeRange(ptr, &startHour, &startMin, &stopHour, &stopMin);
			count++;
			schRuleJobj = json_object_new_object();
			schRuleRetJarray = json_object_new_array();
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)		
			if(ISSET("TimeEnable")){
				if((!strcmp(ptr3,"false"))||(!strcmp(ptr3,"0"))){
					Jaddb(schRuleJobj, "Enable", false);
				}else if ((!strcmp(ptr3,"true"))||(!strcmp(ptr3,"1"))){
					Jaddb(schRuleJobj, "Enable", true); 
				}else{
					Jaddb(schRuleJobj, "Enable", true); 				
				}
			}else
				printf("\n\nTimeEnable is not set\n");
#endif			
			//Jaddb(schRuleJobj, "Enable", true);
			if(ISSET("pcpName"))
				sprintf(schRuleName, "%s_%d", Jgets(Jobj, "pcpName"), count);
			else
				sprintf(schRuleName, "%s_%d", Jgets(curJobj, "pcpName"), count);

			Jadds(schRuleJobj, "Name", schRuleName);
			Jadds(schRuleJobj, "Description", "ParentalControl");
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			if(ISSET("Days"))
				replaceParam(schRuleJobj, "Days", Jobj, "Days");
			else
				replaceParam(schRuleJobj, "Days", curJobj, "Days");
#else			
			if(ISSET("Days"))
				replaceParam(schRuleJobj, "Days", Jobj, "Days");
			else
				replaceParam(schRuleJobj, "Days", curJobj, "Days");
#endif
			Jaddi(schRuleJobj, "TimeStartHour", startHour);
			Jaddi(schRuleJobj, "TimeStartMin", startMin);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)

           if(ISSET("Days")){
			Jadds(schRuleJobj, "Days", ptr4);		
           }
#endif	
			if (stopHour >= 24) {
				stopHour = 23;
				stopMin = 59;
			}

#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
			if(ISSET("TimeType")){
				if(ptr2 == NULL)
					Jadds(schRuleJobj, "Type", "NULL");
				else
					Jadds(schRuleJobj, "Type", ptr2);			
			}
#endif
		
			Jaddi(schRuleJobj, "TimeStopHour", stopHour);
			Jaddi(schRuleJobj, "TimeStopMin", stopMin);
			Jaddb(schRuleJobj, "DontCheckDupName", true);
#ifdef ZYXEL_HOME_CYBER_SECURITY
			PARENT_PRINT("%s: scherule=%s\n",__FUNCTION__,(char *)json_object_get_string(schRuleJobj));
#endif			
			if((ret = zcfgFeDalScheduleAdd(schRuleJobj, schRuleRetJarray, NULL)) != ZCFG_SUCCESS){
				json_object_put(schRuleJobj);
				json_object_put(schRuleRetJarray);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.add_schedule_err"));
				return ret;
			}
			schRulePath = Jgets(json_object_array_get_idx(schRuleRetJarray, 0), "path");
			strcat(ScheduleRuleList, schRulePath);
			strcat(ScheduleRuleList, ",");
			ptr = strtok_r(NULL, ",", &tmp);
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)		
			ptr2 = strtok_r(NULL, ",", &tmp2);
			ptr3 = strtok_r(NULL, ",", &tmp3);			
			ptr4 = strtok_r(NULL, ":", &tmp4);						
#endif
			json_object_put(schRuleJobj);
			json_object_put(schRuleRetJarray);
		}
		if(strlen(ScheduleRuleList)>0){
			ScheduleRuleList[strlen(ScheduleRuleList)-1]= '\0';
		}
	}
	
	strcpy(curScheduleRuleList, json_object_get_string(json_object_object_get(pcpObj, "ScheduleRuleList")));

	replaceParam(pcpObj, "Enable", Jobj, "pcpEnable");
	replaceParam(pcpObj, "Name", Jobj, "pcpName");
	replaceParam(pcpObj, "WebRedirect", Jobj, "WebRedirect");
	if(ISSET("ServicePolicy")){
		if(!strcmp(Jgets(Jobj, "ServicePolicy"), "Block"))
			Jadds(pcpObj, "ServicePolicy", "Drop");
		else if(!strcmp(Jgets(Jobj, "ServicePolicy"), "Allow"))
			Jadds(pcpObj, "ServicePolicy", "Accept");
	}
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	if(ISSET("TTServicePolicy")){
		if(!strcmp(Jgets(Jobj, "TTServicePolicy"), "Block"))
			Jadds(pcpObj, "TTServicePolicy", "Drop");
		else if(!strcmp(Jgets(Jobj, "TTServicePolicy"), "Allow"))
			Jadds(pcpObj, "TTServicePolicy", "Accept");
	}
#endif
	if(ISSET("UrlPolicy")){
		if(!strcmp(Jgets(Jobj, "UrlPolicy"), "Block"))
			Jaddi(pcpObj, "Type", 0);
		else if(!strcmp(Jgets(Jobj, "UrlPolicy"), "Allow"))
			Jaddi(pcpObj, "Type", 1);
	}

	if(ISSET("MACAddressList") && !strcmp(Jgets(Jobj, "MACAddressList"), "All"))
		Jadds(pcpObj, "MACAddressList", "00:00:00:00:00:00");
	else
		replaceParam(pcpObj, "MACAddressList", Jobj, "MACAddressList");
	//replaceParam(pcpObj, "URLFilter", Jobj, "URLFilter");
	//replaceParam(pcpObj, "NetworkServiceList", Jobj, "NetworkServiceList");
	URLFilter = Jgets(Jobj,"URLFilter");
	if(URLFilter && !strcmp(URLFilter, "Empty"))
		Jadds(pcpObj, "URLFilter", "");
	else
		replaceParam(pcpObj, "URLFilter", Jobj, "URLFilter");

	NetworkServiceList = Jgets(Jobj,"NetworkServiceList");
	if(NetworkServiceList && !strcmp(NetworkServiceList, "Empty"))
		Jadds(pcpObj, "NetworkServiceList", "");
	else
		replaceParam(pcpObj, "NetworkServiceList", Jobj, "NetworkServiceList");
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	TTNetworkServiceList = Jgets(Jobj,"TTNetworkServiceList");
	if(TTNetworkServiceList && !strcmp(TTNetworkServiceList, "Empty"))
		Jadds(pcpObj, "TTNetworkServiceList", "");
	else
		replaceParam(pcpObj, "TTNetworkServiceList", Jobj, "TTNetworkServiceList");
#endif
#ifdef ZYXEL_HOME_CYBER_SECURITY
	ContentFilterList = Jgets(Jobj,"ContentFilterList");
	if(ContentFilterList && !strcmp(ContentFilterList, "Empty"))
		Jadds(pcpObj, "ContentFilterList", "");
	else
		replaceParam(pcpObj, "ContentFilterList", Jobj, "ContentFilterList");	
#ifdef ZYXEL_HOME_CYBER_SECURITY
	Id = Jgets(Jobj,"Id");
	if(Id && !strcmp(Id, "Empty"))
		Jadds(pcpObj, "Id", "");
	else
		replaceParam(pcpObj, "Id", Jobj, "Id");	
#endif
#endif		
	
	if(strlen(ScheduleRuleList)>0)
		json_object_object_add(pcpObj, "ScheduleRuleList", json_object_new_string(ScheduleRuleList));
	
	if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL)) != ZCFG_SUCCESS){
		printf("ret=%d\n", ret);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.set_pcp_err"));
		return ret;
	}

#if 0
	json_object_object_add(pcpObj, "Enable", json_object_new_boolean(pcpEnable));
	json_object_object_add(pcpObj, "Name", json_object_new_string(pcpName));
	json_object_object_add(pcpObj, "WebRedirect", json_object_new_boolean(WebRedirect));
	json_object_object_add(pcpObj, "ServicePolicy", json_object_new_string(ServicePolicy));
	json_object_object_add(pcpObj, "Type", json_object_new_int(Type));
	json_object_object_add(pcpObj, "MACAddressList", json_object_new_string(MACAddressList));
	json_object_object_add(pcpObj, "URLFilter", json_object_new_string(URLFilter));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION	
	json_object_object_add(pcpObj, "TTServicePolicy", json_object_new_string(TTServicePolicy));
#endif		
	if(ScheduleRuleList != NULL)
		json_object_object_add(pcpObj, "ScheduleRuleList", json_object_new_string(ScheduleRuleList));
	else 
		return ret = ZCFG_INVALID_FORMAT;
	json_object_object_add(pcpObj, "NetworkServiceList", json_object_new_string(NetworkServiceList));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION		//TAAAB
	json_object_object_add(pcpObj, "TTNetworkServiceList", json_object_new_string(TTNetworkServiceList));
#endif
	zcfgFeObjJsonSet(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, pcpObj, NULL);
	
#endif
	if(reqSchRuleObjArray != NULL){//request form GUI
		curPcpSchRuleDel();
	}
#if defined(ZYXEL_HOME_CYBER_SECURITY) && !defined(ZYXEL_USA_PRODUCT)
	else if(ISSET("TimeRangeList") || ISSET("Days")|| ISSET("TimeType")|| ISSET("TimeEnable")){//request form CLI
#else	
	else if(ISSET("TimeRangeList") || ISSET("Days")){//request form CLI
#endif	
		curPcpSchRuleDel();
	}

	if(curJarray != NULL)
		json_object_put(curJarray);
		
	return ret;
}

void zcfgFeDalShowParentCtl(struct json_object *Jarray){

	int i, j, firstTime, len = 0;
	char buf[1024] = {0};
	char *ptr, *tmp;
	struct json_object *Jobj;
	char serviceDesc[32], protocol[1024], tmpProtocol[1024];

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	if((Jobj = json_object_array_get_idx(Jarray, 0)) == NULL)
		return;
	
	if(Jget(Jobj, "PrentalCtlEnable") != NULL){//get all case
		printf("Parental Control             %s\n", Jgetb(Jobj, "PrentalCtlEnable")?"Enabled":"Disabled");
		printf("\n");
		printf("%-6s %-12s %-30s %-20s\n", "Index", "Status", "PCP Name", "Home Network User MAC");
		printf("------------------------------------------------------------------------\n");

		len = json_object_array_length(Jarray);
		for(i=1;i<len;i++){
			Jobj = json_object_array_get_idx(Jarray, i);
			if(Jobj!=NULL){
				printf("%-6lld ", Jgeti(Jobj, "Index"));
				printf("%-12s ", Jgetb(Jobj, "pcpEnable")?"Enabled":"Disabled");
				printf("%-30s ", Jgets(Jobj, "pcpName"));
				strcpy(buf, Jgets(Jobj, "MACAddressList"));
				ptr = strtok_r(buf, ",", &tmp);
				firstTime = 1;
			  while(ptr != NULL){
			  if(firstTime){
				printf("%-20s\n", ptr);
				firstTime=0;
			  }
			  else{
			  	printf("%-50s %-20s\n", "", ptr);
			  }
				ptr = strtok_r(NULL, ",", &tmp);
			  }
				printf("\n");
			}
		}
	}
	else{//get the specified index
		printf("%-35s %s\n", "Active", Jgetb(Jobj, "pcpEnable")?"Enabled":"Disabled");
		printf("%-35s %s\n", "Parental Control Profile Name", Jgets(Jobj, "pcpName"));
		printf("User MAC Address\n");
		strcpy(buf, Jgets(Jobj, "MACAddressList"));
		ptr = strtok_r(buf, ",", &tmp);
	  while(ptr != NULL){
		printf("%-35s %s\n", "", ptr);
		ptr = strtok_r(NULL, ",", &tmp);
	  }
	  	printf("\n");
	  	printf("%-35s %s\n", "Day", Jgets(Jobj, "Days"));
		printf("Time (Start - End)\n");
		strcpy(buf, Jgets(Jobj, "TimeRangeList"));
		ptr = strtok_r(buf, ",", &tmp);
	  while(ptr != NULL){
		printf("%-35s %s\n", "", ptr);
		ptr = strtok_r(NULL, ",", &tmp);
	  }
		printf("\n");
		printf("%-35s %s\n", "Network Service Setting", Jgets(Jobj, "ServicePolicy"));
#ifdef ZYXEL_HOME_CYBER_SECURITY
		printf("\n");
		printf("%-35s %s\n", "ContentFilterList", Jgets(Jobj, "ContentFilterList"));
		printf("\n");
		printf("%-35s %s\n", "Id", Jgets(Jobj, "Id"));		
		
		printf("\n");
		printf("%-35s %s\n", "TimeType", Jgets(Jobj, "TimeType"));
		printf("\n");
		printf("%-35s %s\n", "TimeEnable", Jgets(Jobj, "TimeEnable"));	
		printf("\n");
		printf("%-35s %s\n", "ScheduleRuleList", Jgets(Jobj, "ScheduleRuleList"));			
	
#endif		
#if 1
		strcpy(buf, Jgets(Jobj, "NetworkServiceList"));
		ptr = strtok_r(buf, "/", &tmp);
		printf("%-35s %-15s %s\n", "", "Service Name", "Protocol:Port");
	  while(ptr != NULL){
	  	if(sscanf(ptr, "%*[^[][%[^@]@%s", serviceDesc, tmpProtocol) !=2)
			continue;
		i=0;
		j=0;
		firstTime =1;
		memset(protocol,0,sizeof(protocol));
		if(!strncmp(&(tmpProtocol[i]), "BOTH", 3)){
			strcpy(protocol, "TCP&UDP");
			i=i+4;
			j=j+7;
		}
		while(tmpProtocol[i] != '\0'){
			if(firstTime && tmpProtocol[i]=='#'){
				protocol[j] = ':';
				i++;
				j++;
			}
			else if(!strncmp(&(tmpProtocol[i]), ":-1", 3)){
				i=i+3;
			}
			else if(!strncmp(&(tmpProtocol[i]), "@TCP#", 5) || !strncmp(&(tmpProtocol[i]), "@UDP#", 5)){
				i=i+5;
				protocol[j] = ',';
				j++;
			}
			else if(!strncmp(&(tmpProtocol[i]), "@BOTH#", 6)){
				i=i+6;
				protocol[j] = ',';
				j++;
			}
			else{
				protocol[j] = tmpProtocol[i];
				i++;
				j++;
			}
		}
		printf("%-35s %-15s %s\n", "", serviceDesc, protocol);
		ptr = strtok_r(NULL, "/", &tmp);
	  }
#endif
		printf("\n");
		
		printf("%-35s %s\n", "Block or Allow the Web Site", Jgets(Jobj, "UrlPolicy"));
		printf("%-35s %s\n", "", "webSite");
		strcpy(buf, Jgets(Jobj, "URLFilter"));
		ptr = strtok_r(buf, ",", &tmp);
	  while(ptr != NULL){
		printf("%-35s %s\n", "", ptr);
		ptr = strtok_r(NULL, ",", &tmp);
	  }
	}


}

zcfgRet_t zcfgFeDalParentCtlGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;

	if((Jobj != NULL) && (Jget(Jobj, "Index")!=NULL)){
		IID_INIT(objIid);
		objIid.level = 1;
		objIid.idx[0] = Jgeti(Jobj, "Index");
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL_PROF, &objIid, &obj) == ZCFG_SUCCESS){
			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(objIid.idx[0]));
			
			parsePcpObject(pramJobj, obj);
			
			json_object_array_add(Jarray, pramJobj);
		
			zcfgFeJsonObjFree(obj);	
		}
	}
	else{
		int maxlen = zcfgFeObjQueryMaxInstance(RDM_OID_PAREN_CTL_PROF);

		IID_INIT(objIid);
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL, &objIid, &obj) == ZCFG_SUCCESS){
			pramJobj = json_object_new_object();
			replaceParam(pramJobj, "PrentalCtlEnable", obj, "Enable");
			json_object_object_add(pramJobj, "MaxLenPrentalCtlPrf", json_object_new_int(maxlen));
			json_object_array_add(Jarray, pramJobj);
			zcfgFeJsonObjFree(obj);
		}
		IID_INIT(objIid);
		while(zcfgFeObjJsonGetNext(RDM_OID_PAREN_CTL_PROF, &objIid, &obj) == ZCFG_SUCCESS){

			pramJobj = json_object_new_object();
			json_object_object_add(pramJobj, "Index", json_object_new_int(objIid.idx[0]));
			
			parsePcpObject(pramJobj, obj);
			
			json_object_array_add(Jarray, pramJobj);
		
			zcfgFeJsonObjFree(obj);	
		}
	}
	return ret;
}

zcfgRet_t zcfgFeDalParentCtlEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
		
	initGlobalPcpObjects();
	getBasicPcpInfo(Jobj);
			
	t2 = time(NULL);
	
	if(!ISSET("PrentalCtlEnable"))
	{
		if((ret = editPcpObject(Jobj, replyMsg)) != ZCFG_SUCCESS)
			goto dalPcp_edit_fail;
	}
	else 
	{
		if((ret = editPcObject(Jobj)) != ZCFG_SUCCESS)
			goto dalPcp_edit_fail;
	}
	
	t2 = time(NULL);
	
	
dalPcp_edit_fail:
	freeAllPcpObjects();
			
	return ret;
}
zcfgRet_t zcfgFeDalParentCtlAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
		
	initGlobalPcpObjects();
	getBasicPcpInfo(Jobj);

	
	if((ret = addPcpObject(Jobj)) != ZCFG_SUCCESS)
		goto dalPcp_add_fail;
	
	
dalPcp_add_fail:
	freeAllPcpObjects();
			
	return ret;
}
zcfgRet_t zcfgFeDalParentCtlDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pctlObjIidArray = NULL;
	uint8_t iidIdx =0;
	const char * scheduleRules = NULL;
	char* buffer = NULL;
	
#ifdef ZYXEL_HOME_CYBER_SECURITY	
	IID_INIT(pcpObjIid);
#endif	
	pctlObjIidArray = json_object_object_get(Jobj, "PctlObjIid");
	if(pctlObjIidArray != NULL){//request form GUI
		for(iidIdx=0; iidIdx < json_object_array_length(pctlObjIidArray); iidIdx++){
			pcpObjIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(pctlObjIidArray, iidIdx));
			if(pcpObjIid.idx[iidIdx] != 0)
				pcpObjIid.level = iidIdx+1;
		}
	}
	else{//request form CLI
		IID_INIT(pcpObjIid);
		pcpObjIid.level = 1;
		pcpObjIid.idx[0] = Jgeti(Jobj, "Index");
	}
	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, &pcpObj)) != ZCFG_SUCCESS){
		if(replyMsg)
			sprintf(replyMsg, "Can't find --Index %d", pcpObjIid.idx[0]);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.get_pcp_err"));		
		return ret;
	}
	
	scheduleRules = json_object_get_string(json_object_object_get(pcpObj, "ScheduleRuleList"));
	
	if((ret = zcfgFeObjJsonDel(RDM_OID_PAREN_CTL_PROF, &pcpObjIid, NULL)) != ZCFG_SUCCESS){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.del_pcp_err"));		
		return ret;
	}
	
	buffer = strtok ((char *)scheduleRules, ",");
	if ((buffer == NULL) && strstr(scheduleRules, "X_ZYXEL_Schedule."))
	{
		schTimeObjIid.level = 1;
		sscanf(ScheduleRuleList, "X_ZYXEL_Schedule.%hhu", &schTimeObjIid.idx[0]);
		if((ret = zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &schTimeObjIid, NULL)) != ZCFG_SUCCESS){
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.del_schedule_err2"));		
			return ret;
		}
	}else
	{
		while (buffer) {
			schTimeObjIid.level = 1;
			sscanf(buffer, "X_ZYXEL_Schedule.%hhu", &schTimeObjIid.idx[0]);
			if((ret = zcfgFeObjJsonDel(RDM_OID_SCHEDULE, &schTimeObjIid, NULL)) != ZCFG_SUCCESS){
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.Parental_Control.error.del_schedule_err2"));		
				return ret;
			}
			buffer = strtok (NULL, ",");
		}
	}
	
	return ret;
}
zcfgRet_t zcfgFeDalParentCtl(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!strcmp(method, "POST"))
		ret = zcfgFeDalParentCtlAdd(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
	   ret = zcfgFeDalParentCtlDelete(Jobj, replyMsg);
	else if(!strcmp(method, "PUT"))
		ret = zcfgFeDalParentCtlEdit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalParentCtlGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);
		
	return ret;
}
