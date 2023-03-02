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
#include "zos_api.h"

#define SCHEDULE_FQCODEL_TIME_LEN 20
dal_param_t SCH_FQCODEL_param[]={		//RDM_OID_REMO_SRV
	//periodic reboot time format: 0001-01-01P00:00:00Z
	//one-shot reboot time format: 0001-01-01T00:00:00Z
	{"scheduleFQCODEL", 					dalType_string,	SCHEDULE_FQCODEL_TIME_LEN,	SCHEDULE_FQCODEL_TIME_LEN,		NULL, NULL, 				dal_Edit},
	{NULL,						0,				0,	0,		NULL, NULL, 				0},
};

//structure to store information of scheduleReboot
typedef struct {
	int type; //0; dateTime, 1: weekly-periodic
	int year; // YYYY
	int month; // 1 - 12
	int date; // 1 - 31
	int hour; // 0 - 23
	int minute; // 0 - 59
	int second; // 0 - 59
	int weekday; // 0 - 7
}dateTime_t;

//return 0 if it is correct format, or other return values for each fail case
//pass the dateTime_t struct pointer if you need to keep these information, or leave it as NULL
int verifyScheduleFQCODELTimeFormat(char *ScheduleFQCODEL, dateTime_t* dtReturn, char *replyMsg)
{
	int i=0, j=0, next=0;
	char buf[32] = {0};
	int ret = 0;
	dateTime_t *dt = NULL;

	dt = (dateTime_t*)malloc(sizeof(dateTime_t));
	memset(dt, 0x0, sizeof(dateTime_t));

	if (ScheduleFQCODEL) {

		if (strstr(ScheduleFQCODEL, "P")) {
			dt->type = 1;
			// Do periodic, 0001-01-01P00:00:00Z
			for (i=0; i<strlen(ScheduleFQCODEL); i++) {
				if (ScheduleFQCODEL[i] == 'P') {
					next++;
					continue;
				}
				if (next > 0) {
					if (ScheduleFQCODEL[i] == ':') {
						j=0;
						if (next == 1) {
							dt->hour = atoi(buf);
							if ((dt->hour > 23) || (dt->hour < 0)) {
								if(replyMsg){
								   	sprintf(replyMsg, \
											"Request reject:\nIn %s, the hour[%d] is invalid.",\
											ScheduleFQCODEL, dt->hour);
								}
								ret = 1;
								break;
							}
						} else if (next == 2) {
							dt->minute = atoi(buf);
							if ((dt->minute > 59) || (dt->minute < 0)) {
								if(replyMsg){
								   	sprintf(replyMsg, \
											"Request reject:\nIn %s, the minute[%d] is invalid.",\
											ScheduleFQCODEL, dt->minute);
								}
								ret = 2;
								break;
							}
						}
						memset(buf, 0x0, sizeof(buf));
						next++;
						continue;
					}
					if (ScheduleFQCODEL[i] == 'Z') {
						dt->weekday = atoi(buf);
						if ((dt->weekday > 7) || (dt->minute < 0)) {
							if(replyMsg){
							   	sprintf(replyMsg,\
									   	"Request reject:\nIn %s, the weekday[%d] is invalid.",\
										ScheduleFQCODEL, dt->weekday);
							}
							ret = 3;
							break;
						}
						j=0;
						memset(buf, 0x0, sizeof(buf));
						next++;
						continue;
					}
					buf[j++] = ScheduleFQCODEL[i];
				}
			}
		} else if (strstr(ScheduleFQCODEL, "T")) {
			dt->type = 0;
			// Do one-shot, 0001-01-01T00:00:00Z
			for (i=0; i<strlen(ScheduleFQCODEL); i++) {
				if (ScheduleFQCODEL[i] == '-') {
					j=0;
					if (next == 0) {
						dt->year = atoi(buf);
						if ((dt->year > 9999) || (dt->year < 0)) {
							if(replyMsg){
							   	sprintf(replyMsg,\
									   	"Request reject:\nIn %s, the year[%d] is invalid.",\
										ScheduleFQCODEL, dt->year);
							}
							ret = 4;
							break;
						}
					} else if (next == 1) {
						dt->month = atoi(buf);
						if ((dt->month > 12) || (dt->month < 1)) {
							if(replyMsg){
							   	sprintf(replyMsg,\
									   	"Request reject:\nIn %s, the month[%d] is invalid.",\
										ScheduleFQCODEL, dt->month);
							}
							ret = 5;
							break;
						}
					}
					memset(buf, 0x0, sizeof(buf));
					next++;
					continue;
				}

				if (ScheduleFQCODEL[i] == 'T') {
					dt->date = atoi(buf);
					if (((dt->month==2) && (dt->date > 29)) || (dt->date > 31) || (dt->date < 1)) {
						if(replyMsg){
						   	sprintf(replyMsg, \
									"Request reject:\nIn %s, the month[%d]/date[%d] is invalid.",\
									ScheduleFQCODEL, dt->month, dt->date);
						}
						ret = 6;
						break;
					}
					j=0;
					memset(buf, 0x0, sizeof(buf));
					next++;
					continue;
				}
				if (ScheduleFQCODEL[i] == ':') {
					j=0;
					if (next == 3) {
						dt->hour = atoi(buf);
						if ((dt->hour > 23) || (dt->hour < 0)) {
							if(replyMsg){
							   	sprintf(replyMsg, \
										"Request reject:\nIn %s, the hour[%d] is invalid.",\
										ScheduleFQCODEL, dt->hour);
							}
							ret = 7;
							break;
						}
					} else if (next == 4) {
						dt->minute = atoi(buf);
						if ((dt->minute > 59) || (dt->minute < 0)) {
							if(replyMsg){
							   	sprintf(replyMsg,\
									   	"Request reject:\nIn %s, the minute[%d] is invalid.",\
										ScheduleFQCODEL, dt->minute);
							}
							ret = 8;
							break;
						}
					}
					memset(buf, 0x0, sizeof(buf));
					next++;
					continue;
				}
				if (ScheduleFQCODEL[i] == 'Z') {
					dt->second = atoi(buf);
					if ((dt->second > 59) || (dt->second < 0)) {
						if(replyMsg){
						   	sprintf(replyMsg,\
								   	"Request reject:\nIn %s, the second[%d] is invalid.",\
									ScheduleFQCODEL, dt->second);
						}
						ret = 9;
						break;
					}
					j=0;
					memset(buf, 0x0, sizeof(buf));
					next++;
					continue;
				}

				buf[j++] = ScheduleFQCODEL[i];
			}
		}
		else{
			if(replyMsg){
			   	strcpy(replyMsg, "Request reject:\nIt is not One-shot mode neither Periodic mode.");
			}
			ret = 10;
		}
	}

	//printf("[%s][%d] Exit, type(%d), Y(%d), M(%d), D(%d), h(%d), m(%d), s(%d), w(%d)...\n", __FUNCTION__, __LINE__, dt->type, dt->year, dt->month, dt->date, dt->hour, dt->minute, dt->second, dt->weekday);
	if(dtReturn != NULL){
		memcpy(dtReturn, dt, sizeof(dateTime_t));
	}
	free(dt);

	return ret;
}




zcfgRet_t zcfgFeDal_schFQCODEL_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	rdm_MgmtSrv_t *mgmtSrvObj = NULL;
	enum json_type jsonType;
	struct json_object *paramJobj = NULL;
	objIndex_t objIid;
	dateTime_t *dt = NULL;
	int result = 0;
	const char* cmdType = NULL;

	IID_INIT(objIid);
	if(zcfgFeObjStructGet(RDM_OID_MGMT_SRV, &objIid, (void **)&mgmtSrvObj) != ZCFG_SUCCESS){
		if(replyMsg) strcpy(replyMsg, "Request reject");
		return ZCFG_REQUEST_REJECT;
	}

	jsonType = json_object_get_type(Jobj);
	if(jsonType == json_type_object){
		cmdType = json_object_get_string(json_object_object_get(Jobj, "cmdtype"));

		if(!strcmp(cmdType, "GUI")){
			dt = (dateTime_t*)malloc(sizeof(dateTime_t));
			result = verifyScheduleFQCODELTimeFormat(mgmtSrvObj->ScheduleFQCODEL, dt, replyMsg);
			if(result != 0){
				if(replyMsg) strcpy(replyMsg, "Request reject");
				free(dt);
				return ZCFG_REQUEST_REJECT;
			}

			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "isPeriodic", json_object_new_boolean(dt->type));
			json_object_object_add(paramJobj, "Year", json_object_new_int(dt->year));
			json_object_object_add(paramJobj, "Month", json_object_new_int(dt->month));
			json_object_object_add(paramJobj, "Date", json_object_new_int(dt->date));
			json_object_object_add(paramJobj, "Day", json_object_new_int(dt->weekday));
			json_object_object_add(paramJobj, "Hour", json_object_new_int(dt->hour));
			json_object_object_add(paramJobj, "Minute", json_object_new_int(dt->minute));
			json_object_array_add(Jarray, paramJobj);

			free(dt);
		}else if(!strcmp(cmdType, "CLI")){
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "ScheduleFQCODEL", json_object_new_string(mgmtSrvObj->ScheduleFQCODEL));
			json_object_array_add(Jarray, paramJobj);
		}

		if(mgmtSrvObj != NULL){
			zcfgFeObjStructFree(mgmtSrvObj);
		}
	}
	return ret;
}

void zcfgFeDalShowSchFqCodel(struct json_object *Jarray){
	struct json_object *obj = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	printf("%s%s \n","ScheduleFQCODEL:", json_object_get_string(json_object_object_get(obj, "ScheduleFQCODEL")));
		
}


zcfgRet_t zcfgFeDal_schFQCODEL_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	enum json_type jsonType;
	rdm_MgmtSrv_t *mgmtSrvObj = NULL;
	objIndex_t objIid;
	char *scheduleFQCODEL = NULL;
	const char* cmdType = NULL;
	int result = 0;

	jsonType = json_object_get_type(Jobj);

	if(jsonType == json_type_object){
		cmdType = json_object_get_string(json_object_object_get(Jobj, "cmdtype"));

		if(!strcmp(cmdType, "GUI")){
			scheduleFQCODEL = (char*)malloc(sizeof(char)*(SCHEDULE_FQCODEL_TIME_LEN + 1));
			snprintf(scheduleFQCODEL, (SCHEDULE_FQCODEL_TIME_LEN + 1), "%4d-%02d-%02d%s%02d:%02d:%02dZ",
					atoi(Jgets(Jobj, "Year")),
					atoi(Jgets(Jobj, "Month")),
					atoi(Jgets(Jobj, "Date")),
					(!strcmp(Jgets(Jobj, "isPeriodic"), "true")?"P":"T"),
					atoi(Jgets(Jobj, "Hour")),
					atoi(Jgets(Jobj, "Minute")),
					//no seconds input, set it to zero in T type
					(!strcmp(Jgets(Jobj, "isPeriodic"), "true")?atoi(Jgets(Jobj, "Day")):0)
					);
		}else if(!strcmp(cmdType, "CLI")){
			scheduleFQCODEL = Jgets(Jobj, "scheduleFQCODEL");
		}
		result = verifyScheduleFQCODELTimeFormat(scheduleFQCODEL, NULL, replyMsg);

		//the input format was wrong
		if(result != 0){
			if(replyMsg) strcpy(replyMsg, "Request reject");
			return ZCFG_REQUEST_REJECT;
		}

		//set to data model
		IID_INIT(objIid);
		if(zcfgFeObjStructGet(RDM_OID_MGMT_SRV, &objIid, (void **)&mgmtSrvObj) != ZCFG_SUCCESS){
			if(replyMsg) strcpy(replyMsg, "Request reject");
			return ZCFG_REQUEST_REJECT;
		}
		ZOS_STRNCPY(mgmtSrvObj->ScheduleFQCODEL,scheduleFQCODEL, sizeof(mgmtSrvObj->ScheduleFQCODEL));
		if(zcfgFeObjStructSet(RDM_OID_MGMT_SRV, &objIid, (void *)mgmtSrvObj, NULL) != ZCFG_SUCCESS){
			if(replyMsg) strcpy(replyMsg, "Request reject");
			if(mgmtSrvObj!=NULL){
				zcfgFeObjStructFree(mgmtSrvObj);
			}
			return ZCFG_REQUEST_REJECT;
		}

		//release allocated memories
		if(mgmtSrvObj!=NULL){
			zcfgFeObjStructFree(mgmtSrvObj);
		}
		if(!strcmp(cmdType, "GUI")){
			free(scheduleFQCODEL);
		}
	}//End of if(jsonType == json_type_object)


	return ret;
}


zcfgRet_t zcfgFeDalSchFqCodel(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_schFQCODEL_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_schFQCODEL_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

