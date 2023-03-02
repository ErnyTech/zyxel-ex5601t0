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

dal_param_t TIME_param[] =
{
	{"NTPServer1", 			dalType_serveraddr,  0, 	0, 	NULL,	"None|time.nist.gov|clock.fmt.he.net|clock.nyc.he.net|clock.sjc.he.net|clock.via.net|ntp1.tummy.com|time.cachenetworks.com|pool.ntp.org",	0},
	{"NTPServer2", 			dalType_serveraddr,  0, 	0, 	NULL,	"None|time.nist.gov|clock.fmt.he.net|clock.nyc.he.net|clock.sjc.he.net|clock.via.net|ntp1.tummy.com|time.cachenetworks.com|pool.ntp.org",	0},
	{"NTPServer3", 			dalType_serveraddr,  0, 	0, 	NULL,	"None|time.nist.gov|clock.fmt.he.net|clock.nyc.he.net|clock.sjc.he.net|clock.via.net|ntp1.tummy.com|time.cachenetworks.com|pool.ntp.org",	0},
	{"NTPServer4", 			dalType_serveraddr,  0, 	0, 	NULL,	"None|time.nist.gov|clock.fmt.he.net|clock.nyc.he.net|clock.sjc.he.net|clock.via.net|ntp1.tummy.com|time.cachenetworks.com|pool.ntp.org",	0},
	{"NTPServer5", 			dalType_serveraddr,  0, 	0, 	NULL,	"None|time.nist.gov|clock.fmt.he.net|clock.nyc.he.net|clock.sjc.he.net|clock.via.net|ntp1.tummy.com|time.cachenetworks.com|pool.ntp.org",	0},
	{"DaylightSavings",		dalType_boolean,	0, 	0,	NULL},
	{"X_ZYXEL_TimeZone", 	dalType_string,	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_Location", 	dalType_string,  0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"TimeZone",			dalType_timezone,0,	0,	NULL},
	{"Start_Time",			dalType_HourMin,	0,	0,	NULL},
	{"End_Time",			dalType_HourMin,	0,	0,	NULL},
	/*End*/
	{"X_ZYXEL_EndDay", 		dalType_int,	 0,		0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_EndDayOfWeek", dalType_int,	 0,		0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_EndMin", 		dalType_int, 	 0, 	0,	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_EndHour", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_EndMonth", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_EndWeek", 	dalType_int,  	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	/*Start*/
	{"X_ZYXEL_StartDay", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartDayOfWeek", 	dalType_int, 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartMin", 	dalType_int,  	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartHour", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartHour", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartMonth", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"X_ZYXEL_StartWeek", 	dalType_int, 	 0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
#ifdef ZYXEL_TAAAG_NTP_MULTIWAN//TAAAG, wan interface for NTP
	{"X_ZYXEL_BoundInterfaceList",	dalType_string,  0, 	0,	NULL,	NULL,	dalcmd_Forbid},
#endif
	{NULL, 					0, 				 0, 	0, 	NULL}
};

zcfgRet_t NTPcheck(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	char NTPtmp[5][128];
	char buf[1024] = {0};
	char *newntp1 = NULL, *newntp2 = NULL, *newntp3 = NULL, *newntp4 = NULL, *newntp5 = NULL, *tmp = NULL;
	const char *ntp1 = NULL, *ntp2 = NULL, *ntp3 = NULL, *ntp4 = NULL, *ntp5 = NULL, *curntp1 = NULL, *curntp2 = NULL, *curntp3 = NULL, *curntp4 = NULL, *curntp5 = NULL;
	int i, j, count = 0;
	memset(NTPtmp,0,sizeof(NTPtmp));
	ntp1 = json_object_get_string(json_object_object_get(Jobj, "NTPServer1"));
	ntp2 = json_object_get_string(json_object_object_get(Jobj, "NTPServer2"));
	ntp3 = json_object_get_string(json_object_object_get(Jobj, "NTPServer3"));
	ntp4 = json_object_get_string(json_object_object_get(Jobj, "NTPServer4"));
	ntp5 = json_object_get_string(json_object_object_get(Jobj, "NTPServer5"));
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_TIME, &Iid, &obj) == ZCFG_SUCCESS){
		curntp1 = json_object_get_string(json_object_object_get(obj, "NTPServer1"));
		curntp2 = json_object_get_string(json_object_object_get(obj, "NTPServer2"));
		curntp3 = json_object_get_string(json_object_object_get(obj, "NTPServer3"));
		curntp4 = json_object_get_string(json_object_object_get(obj, "NTPServer4"));
		curntp5 = json_object_get_string(json_object_object_get(obj, "NTPServer5"));
		
		if(json_object_object_get(Jobj, "NTPServer1")){
			strcpy(NTPtmp[0], ntp1);
			json_object_object_del(Jobj, "NTPServer1");
		}
		else
			strcpy(NTPtmp[0], curntp1);
		if(json_object_object_get(Jobj, "NTPServer2")){
			strcpy(NTPtmp[1], ntp2);
			json_object_object_del(Jobj, "NTPServer2");
		}
		else
			strcpy(NTPtmp[1], curntp2);
		if(json_object_object_get(Jobj, "NTPServer3")){
			strcpy(NTPtmp[2], ntp3);
			json_object_object_del(Jobj, "NTPServer3");
		}
		else
			strcpy(NTPtmp[2], curntp3);
		if(json_object_object_get(Jobj, "NTPServer4")){
			strcpy(NTPtmp[3], ntp4);
			json_object_object_del(Jobj, "NTPServer4");
		}
		else
			strcpy(NTPtmp[3], curntp4);
		if(json_object_object_get(Jobj, "NTPServer5")){
			strcpy(NTPtmp[4], ntp5);
			json_object_object_del(Jobj, "NTPServer5");
		}
		else
			strcpy(NTPtmp[4], curntp5);
		for(i=0;i<5;i++){
			for(j=i+1;j<5;j++){
				if(strcmp(NTPtmp[j],"None")){
					if(!strcmp(NTPtmp[i], NTPtmp[j])){
						strcat(replyMsg, "NTP Server is duplicate.");			
						return ZCFG_INVALID_PARAM_VALUE;	
					}
				}				
			}
		}
		for(i=0;i<5;i++){
			if(!strcmp(NTPtmp[i], "None"))
				count++;
			else{
				strcat(buf, NTPtmp[i]);
				strcat(buf, ",");
			}
		}
		if(count>0){
			for(i=0;i<count;i++){
				strcat(buf, "None");
				strcat(buf, ",");
			}
		}
		newntp1 = strtok_r(buf, ",", &tmp);
		newntp2 = strtok_r(NULL, ",", &tmp);
		newntp3 = strtok_r(NULL, ",", &tmp);
		newntp4 = strtok_r(NULL, ",", &tmp);
		newntp5 = strtok_r(NULL, ",", &tmp);
		json_object_object_add(Jobj, "NTPServer1", json_object_new_string(newntp1));
		json_object_object_add(Jobj, "NTPServer2", json_object_new_string(newntp2));
		json_object_object_add(Jobj, "NTPServer3", json_object_new_string(newntp3));
		json_object_object_add(Jobj, "NTPServer4", json_object_new_string(newntp4));
		json_object_object_add(Jobj, "NTPServer5", json_object_new_string(newntp5));
		zcfgFeJsonObjFree(obj);
	}

	return ret;
}


zcfgRet_t zcfgFeDal_Time_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *timeObj = NULL;
	objIndex_t timeIid = {0};
	const char *NTP1 = NULL, *NTP2 = NULL, *NTP3 = NULL, *NTP4 = NULL, *NTP5 = NULL, *timezone = NULL, *location = NULL, *starttime = NULL, *endtime = NULL;
	bool DaylightSaving = false;
	int EndDay = 0, EndDayOfWeek = 0, EndMin = 0, EndHour = 0, EndMonth = 0, EndWeek = 0, StartDay = 0, StartDayOfWeek = 0, StartMin = 0, StartHour = 0, StartMonth = 0, StartWeek = 0;
	
	IID_INIT(timeIid);
	//Start for CLI//
	ret = NTPcheck(Jobj, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;
	if(json_object_object_get(Jobj, "Start_Time")){
		 starttime = json_object_get_string(json_object_object_get(Jobj, "Start_Time"));
		 sscanf(starttime, "%d-%d-%d:%d", &StartMonth, &StartDay, &StartHour, &StartMin);
		 StartWeek = 0;
		 StartDayOfWeek = 0;		 
	}
	if(json_object_object_get(Jobj, "End_Time")){
		 endtime = json_object_get_string(json_object_object_get(Jobj, "End_Time"));
		 sscanf(endtime, "%d-%d-%d:%d", &EndMonth, &EndDay, &EndHour, &EndMin);
		 EndWeek = 0;
		 EndDayOfWeek = 0;
	}
	//End for CLI//
	NTP1 = json_object_get_string(json_object_object_get(Jobj, "NTPServer1"));
	NTP2 = json_object_get_string(json_object_object_get(Jobj, "NTPServer2"));
	NTP3 = json_object_get_string(json_object_object_get(Jobj, "NTPServer3"));
	NTP4 = json_object_get_string(json_object_object_get(Jobj, "NTPServer4"));
	NTP5 = json_object_get_string(json_object_object_get(Jobj, "NTPServer5"));

	DaylightSaving = json_object_get_boolean(json_object_object_get(Jobj, "DaylightSavings"));
	timezone = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_TimeZone"));
	location = json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_Location"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndDay"))
		EndDay = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndDay"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndDayOfWeek"))
		EndDayOfWeek = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndDayOfWeek"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndMin"))
		EndMin = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndMin"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndHour"))
		EndHour = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndHour"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndMonth"))
		EndMonth = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndMonth"));
	if(json_object_object_get(Jobj, "X_ZYXEL_EndWeek"))
		EndWeek = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_EndWeek"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartDay"))
		StartDay = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartDay"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartDayOfWeek"))
		StartDayOfWeek = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartDayOfWeek"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartMin"))
		StartMin = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartMin"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartHour"))
		StartHour = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartHour"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartMonth"))
		StartMonth = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartMonth"));
	if(json_object_object_get(Jobj, "X_ZYXEL_StartWeek"))
		StartWeek = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_StartWeek"));
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_TIME, &timeIid, &timeObj);

	if(ret == ZCFG_SUCCESS){
		json_object_object_add(timeObj, "NTPServer1", json_object_new_string(NTP1));
		json_object_object_add(timeObj, "NTPServer2", json_object_new_string(NTP2));
		json_object_object_add(timeObj, "NTPServer3", json_object_new_string(NTP3));
		json_object_object_add(timeObj, "NTPServer4", json_object_new_string(NTP4));
		json_object_object_add(timeObj, "NTPServer5", json_object_new_string(NTP5));
		json_object_object_add(timeObj, "Enable", json_object_new_boolean(true));
		if(json_object_object_get(Jobj, "DaylightSavings"))
			json_object_object_add(timeObj, "X_ZYXEL_DaylightSavings", json_object_new_boolean(DaylightSaving));
		if(json_object_object_get(Jobj, "X_ZYXEL_TimeZone"))
			json_object_object_add(timeObj, "X_ZYXEL_TimeZone", json_object_new_string(timezone));
		if(json_object_object_get(Jobj, "X_ZYXEL_Location"))
			json_object_object_add(timeObj, "X_ZYXEL_Location", json_object_new_string(location));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndDay") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndDay", json_object_new_int(EndDay));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndDayOfWeek") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndDayOfWeek", json_object_new_int(EndDayOfWeek));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndMin") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndMin", json_object_new_int(EndMin));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndHour") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndHour", json_object_new_int(EndHour));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndMonth") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndMonth", json_object_new_int(EndMonth));
		if(json_object_object_get(Jobj, "X_ZYXEL_EndWeek") || json_object_object_get(Jobj, "End_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_EndWeek", json_object_new_int(EndWeek));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartDay") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartDay", json_object_new_int(StartDay));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartDayOfWeek") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartDayOfWeek", json_object_new_int(StartDayOfWeek));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartMin") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartMin", json_object_new_int(StartMin));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartHour") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartHour", json_object_new_int(StartHour));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartMonth") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartMonth", json_object_new_int(StartMonth));
		if(json_object_object_get(Jobj, "X_ZYXEL_StartWeek") || json_object_object_get(Jobj, "Start_Time"))
			json_object_object_add(timeObj, "X_ZYXEL_StartWeek", json_object_new_int(StartWeek));
#ifdef ZYXEL_TAAAG_NTP_MULTIWAN//TAAAG, wan interface for NTP
        //printf("zcfgFeDal_Time_Edit 222 \n");
		json_object_object_add(timeObj, "X_ZYXEL_BoundInterfaceList", JSON_OBJ_COPY(json_object_object_get(Jobj, "X_ZYXEL_BoundInterfaceList")));
		//printf("edit is %s \n", json_object_get_string(json_object_object_get(Jobj, "X_ZYXEL_BoundInterfaceList")));
		
#endif

		ret = zcfgFeObjJsonBlockedSet(RDM_OID_TIME, &timeIid, timeObj, NULL);
		}
	zcfgFeJsonObjFree(timeObj);
	return ret;
	}

zcfgRet_t zcfgFeDal_Time_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	int startmonthint = 0, endmonthint = 0, startweekint = 0, endweekint = 0, startnumweekint = 0, endnumweekint = 0;
	char *curlocaltime = NULL;
	char *curdate = NULL, *curtime = NULL;
	char startmonth[16] = {0}, endmonth[16] = {0}, startweek[16] = {0}, endweek[16] = {0}, numstartweek[8] = {0}, numendweek[8] = {0};
	if(zcfgFeObjJsonGet(RDM_OID_TIME, &Iid, &obj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		curlocaltime = (char *)json_object_get_string(json_object_object_get(obj, "CurrentLocalTime"));
		curdate = strtok_r(curlocaltime, "T", &curtime);
		
		json_object_object_add(paramJobj, "X_ZYXEL_StartMonth", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartMonth")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndMonth", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndMonth")));
		json_object_object_add(paramJobj, "X_ZYXEL_StartWeek",JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartWeek")));
		json_object_object_add(paramJobj, "X_ZYXEL_StartDayOfWeek", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartDayOfWeek")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndDayOfWeek",JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndDayOfWeek")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndWeek", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndWeek")));
		
		startmonthint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_StartMonth"));
		endmonthint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_EndMonth"));
		startweekint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_StartDayOfWeek"));
		endweekint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_EndDayOfWeek"));
		startnumweekint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_StartWeek"));
		endnumweekint = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_EndWeek"));
		if(startmonthint == 1) strcpy(startmonth, "January");
		else if(startmonthint == 2) strcpy(startmonth, "February");
		else if(startmonthint == 3) strcpy(startmonth, "March");
		else if(startmonthint == 4) strcpy(startmonth, "April");
		else if(startmonthint == 5) strcpy(startmonth, "May ");
		else if(startmonthint == 6) strcpy(startmonth, "June");
		else if(startmonthint == 7) strcpy(startmonth, "July");
		else if(startmonthint == 8) strcpy(startmonth, "August");
		else if(startmonthint == 9) strcpy(startmonth, "September");
		else if(startmonthint == 10) strcpy(startmonth, "October");
		else if(startmonthint == 11) strcpy(startmonth, "November");
		else if(startmonthint == 12) strcpy(startmonth, "December");
		if(endmonthint == 1) strcpy(endmonth, "January");
		else if(endmonthint == 2) strcpy(endmonth, "February");
		else if(endmonthint == 3) strcpy(endmonth, "March");
		else if(endmonthint == 4) strcpy(endmonth, "April");
		else if(endmonthint == 5) strcpy(endmonth, "May ");
		else if(endmonthint == 6) strcpy(endmonth, "June");
		else if(endmonthint == 7) strcpy(endmonth, "July");
		else if(endmonthint == 8) strcpy(endmonth, "August");
		else if(endmonthint == 9) strcpy(endmonth, "September");
		else if(endmonthint == 10) strcpy(endmonth, "October");
		else if(endmonthint == 11) strcpy(endmonth, "November");
		else if(endmonthint == 12) strcpy(endmonth, "December");
		if(startweekint == 0) strcpy(startweek, "Sunday");
		else if(startweekint == 1) strcpy(startweek, "Monday");
		else if(startweekint == 2) strcpy(startweek, "Tuesday");
		else if(startweekint == 3) strcpy(startweek, "Wednesday");
		else if(startweekint == 4) strcpy(startweek, "Thursday");
		else if(startweekint == 5) strcpy(startweek, "Friday");
		else if(startweekint == 6) strcpy(startweek, "Saturday");
		if(endweekint == 0) strcpy(endweek, "Sunday");
		else if(endweekint == 1) strcpy(endweek, "Monday");
		else if(endweekint == 2) strcpy(endweek, "Tuesday");
		else if(endweekint == 3) strcpy(endweek, "Wednesday");
		else if(endweekint == 4) strcpy(endweek, "Thursday");
		else if(endweekint == 5) strcpy(endweek, "Friday");
		else if(endweekint == 6) strcpy(endweek, "Saturday");
		if(startnumweekint == 1) strcpy(numstartweek, "First");
		else if(startnumweekint == 2) strcpy(numstartweek, "Second");
		else if(startnumweekint == 3) strcpy(numstartweek, "Third");
		else if(startnumweekint == 4) strcpy(numstartweek, "Fourth");
		else if(startnumweekint == 5) strcpy(numstartweek, "Last");
		if(endnumweekint == 1) strcpy(numendweek, "First");
		else if(endnumweekint == 2) strcpy(numendweek, "Second");
		else if(endnumweekint == 3) strcpy(numendweek, "Third");
		else if(endnumweekint == 4) strcpy(numendweek, "Fourth");
		else if(endnumweekint == 5) strcpy(numendweek, "Last");
		
		
		json_object_object_add(paramJobj, "Time", json_object_new_string(curtime));
		json_object_object_add(paramJobj, "Date", json_object_new_string(curdate));
		json_object_object_add(paramJobj, "TimeProtocol", json_object_new_string("SNTP (RFC-1769)"));
		json_object_object_add(paramJobj, "NTPServer1", JSON_OBJ_COPY(json_object_object_get(obj, "NTPServer1")));
		json_object_object_add(paramJobj, "NTPServer2", JSON_OBJ_COPY(json_object_object_get(obj, "NTPServer2")));
		json_object_object_add(paramJobj, "NTPServer3", JSON_OBJ_COPY(json_object_object_get(obj, "NTPServer3")));
		json_object_object_add(paramJobj, "NTPServer4", JSON_OBJ_COPY(json_object_object_get(obj, "NTPServer4")));
		json_object_object_add(paramJobj, "NTPServer5", JSON_OBJ_COPY(json_object_object_get(obj, "NTPServer5")));
		json_object_object_add(paramJobj, "TimeZone", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_Location")));
		json_object_object_add(paramJobj, "X_ZYXEL_Location", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_Location")));
		json_object_object_add(paramJobj, "X_ZYXEL_TimeZone", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_TimeZone")));
		json_object_object_add(paramJobj, "DaylightSavings", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_DaylightSavings")));
		json_object_object_add(paramJobj, "TimeSaveEnable", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_DaylightSavings")));
		json_object_object_add(paramJobj, "StartMonth", json_object_new_string(startmonth));
		json_object_object_add(paramJobj, "StartWeek", json_object_new_string(numstartweek));
		json_object_object_add(paramJobj, "StartDayOfWeek", json_object_new_string(startweek));
		json_object_object_add(paramJobj, "StartDay", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartDay")));
		json_object_object_add(paramJobj, "StartHour", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartHour")));
		json_object_object_add(paramJobj, "StartMin", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartMin")));
		json_object_object_add(paramJobj, "X_ZYXEL_StartDay", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartDay")));
		json_object_object_add(paramJobj, "X_ZYXEL_StartHour", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartHour")));
		json_object_object_add(paramJobj, "X_ZYXEL_StartMin", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_StartMin")));
		json_object_object_add(paramJobj, "EndMonth", json_object_new_string(endmonth));
		json_object_object_add(paramJobj, "EndWeek", json_object_new_string(numendweek));
		json_object_object_add(paramJobj, "EndDayOfWeek", json_object_new_string(endweek));
		json_object_object_add(paramJobj, "EndDay", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndDay")));
		json_object_object_add(paramJobj, "EndHour", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndHour")));
		json_object_object_add(paramJobj, "EndMin", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndMin")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndDay", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndDay")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndHour", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndHour")));
		json_object_object_add(paramJobj, "X_ZYXEL_EndMin", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_EndMin")));
#ifdef ZYXEL_TAAAG_NTP_MULTIWAN//TAAAG, wan interface for NTP
				//printf("zcfgFeDal_Time_Get 222 \n");

				json_object_object_add(paramJobj, "X_ZYXEL_BoundInterfaceList", JSON_OBJ_COPY(json_object_object_get(obj, "X_ZYXEL_BoundInterfaceList")));
				//printf("get is %s \n", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_BoundInterfaceList")));
#endif

		json_object_array_add(Jarray, paramJobj);
		zcfgFeJsonObjFree(obj);
	}
	return ret;
}

void zcfgFeDalShowTime(struct json_object *Jarray){
	struct json_object *obj = NULL;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	printf("%-30s %-10s\n","Current Time:",json_object_get_string(json_object_object_get(obj, "Time")));
	printf("%-30s %-10s\n","Current Date:",json_object_get_string(json_object_object_get(obj, "Date")));
	printf("%-30s %-10s\n","Time Protocol:",json_object_get_string(json_object_object_get(obj, "TimeProtocol")));
	printf("%-30s %-10s\n","First Time Server Address:",json_object_get_string(json_object_object_get(obj, "NTPServer1")));
	printf("%-30s %-10s\n","Second Time Server Address:",json_object_get_string(json_object_object_get(obj, "NTPServer2")));
	printf("%-30s %-10s\n","Third Time Server Address:",json_object_get_string(json_object_object_get(obj, "NTPServer3")));
	printf("%-30s %-10s\n","Fourth Time Server Address:",json_object_get_string(json_object_object_get(obj, "NTPServer4")));
	printf("%-30s %-10s\n","Fifth Time Server Address:",json_object_get_string(json_object_object_get(obj, "NTPServer5")));
	printf("%-30s %-10s\n","Time Zone:",json_object_get_string(json_object_object_get(obj, "TimeZone")));
	printf("%-30s %-10s\n","Daylight Savings Enable:",json_object_get_string(json_object_object_get(obj, "TimeSaveEnable")));
	if(json_object_get_boolean(json_object_object_get(obj, "TimeSaveEnable"))){
		printf("Start Rule: \n");
		if(json_object_get_int(json_object_object_get(obj, "StartDay")) != 0){
			printf("%-2s %-10s %-10s\n","","Day: ",json_object_get_string(json_object_object_get(obj, "StartDay")));
		}
		else{
			printf("%-2s %-10s %s %s\n","","Day: ",json_object_get_string(json_object_object_get(obj, "StartWeek")),json_object_get_string(json_object_object_get(obj, "StartDayOfWeek")));
		}
		printf("%-2s %-10s %-10s\n","","Month: ",json_object_get_string(json_object_object_get(obj, "StartMonth")));
		printf("%-2s %-10s %s:%s\n","","Time: ",json_object_get_string(json_object_object_get(obj, "StartHour")),json_object_get_string(json_object_object_get(obj, "StartMin")));
		printf("End Rule: \n");
		if(json_object_get_int(json_object_object_get(obj, "EndDay")) != 0){
			printf("%-2s %-10s %-10s\n","","Day: ",json_object_get_string(json_object_object_get(obj, "EndDay")));
		}
		else{
			printf("%-2s %-10s %s %s\n","","Day: ",json_object_get_string(json_object_object_get(obj, "EndWeek")),json_object_get_string(json_object_object_get(obj, "EndDayOfWeek")));
		}
		printf("%-2s %-10s %-10s\n","","Month: ",json_object_get_string(json_object_object_get(obj, "EndMonth")));
		printf("%-2s %-10s %s:%s\n","","Time: ",json_object_get_string(json_object_object_get(obj, "EndHour")),json_object_get_string(json_object_object_get(obj, "EndMin")));
	}
}



zcfgRet_t zcfgFeDalTime(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_Time_Edit(Jobj, replyMsg);
	}else if(!strcmp(method, "GET")){
		ret = zcfgFeDal_Time_Get(Jobj, Jarray, replyMsg);
	}else
		printf("Unknown method:%s\n", method);

	return ret;
}

