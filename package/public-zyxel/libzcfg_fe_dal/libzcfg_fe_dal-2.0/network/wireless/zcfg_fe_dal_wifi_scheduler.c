
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t WLAN_SCHEDULER_param[] =
{
	//Schedule
	{"Index",			dalType_int,		0,	0,	NULL,	NULL,	dal_Edit|dal_Delete},
	{"Enable",			dalType_boolean,	0,	0,	NULL,	NULL,	dal_Add},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"Description",		dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"SSID",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"Interface",		dalType_int,		0,	0,	NULL,	NULL,	dalcmd_Forbid},
	{"Days",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"TimeStartHour",	dalType_int,		0,	23,	NULL,	NULL,	dal_Add},
	{"TimeStartMin",	dalType_int,		0,	59,	NULL,	NULL,	dal_Add},
	{"TimeStopHour",	dalType_int,		0,	24,	NULL,	NULL,	dal_Add},
	{"TimeStopMin",		dalType_int,		0,	59,	NULL,	NULL,	dal_Add},
	{NULL,		0,	0,	0,	NULL,	NULL,	0}
};

/* if found ret Iid.idx[0] else return 0 */
static int getWifiSSIDIndexFromSSID(char *ssid){
	objIndex_t iid;
	struct json_object *ssidObj = NULL;
	char inputSSID[256]={0};
	char *tmpSSID = NULL;
	int foundIndex = 0;

	strcpy(inputSSID,ssid);
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &iid, &ssidObj) == ZCFG_SUCCESS){

		tmpSSID = (char *)json_object_get_string(json_object_object_get(ssidObj,"SSID"));
		if(!strcmp(inputSSID,tmpSSID)){
			foundIndex = iid.idx[0];
			zcfgFeJsonObjFree(ssidObj);
			break;
		}
		zcfgFeJsonObjFree(ssidObj);
	}

	return foundIndex;
}

void zcfgFeDalShowWlanScheduler(struct json_object *Jarray){

	int i, len = 0;
	char *dayString = NULL;
	int startHour = 0, startMin = 0, stopHour = 0, stopMin = 0;
	char timeStr [15] = {0};
	struct json_object *obj;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-6s %-10s %-15s %-20s %-30s %-15s %-30s\n", "Index", "Status", "Rule Name", "SSID", "Days", "Time", "Description");
	printf("---------------------------------------------------------------------------------------------------------\n");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){

		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){

			dayString = (char *)json_object_get_string(json_object_object_get(obj, "Days"));			
			startHour = json_object_get_int(json_object_object_get(obj, "TimeStartHour"));
			startMin = json_object_get_int(json_object_object_get(obj, "TimeStartMin"));
			stopHour = json_object_get_int(json_object_object_get(obj, "TimeStopHour"));
			stopMin = json_object_get_int(json_object_object_get(obj, "TimeStopMin"));

			memset(timeStr, '\0', sizeof(timeStr));
			sprintf(timeStr, "%02d:%02d~%02d:%02d",startHour,startMin,stopHour,stopMin);

			printf("%-6d ",i+1);
			if(json_object_get_boolean(json_object_object_get(obj, "Enable")))
				printf("%-10s ","Enabled");
			else
				printf("%-10s ","Disabled");
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-20s ",json_object_get_string(json_object_object_get(obj, "SSID")));
			printf("%-30s ",dayString);
			printf("%-15s ",timeStr);
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "Description")));
			printf("\n");

		}
		else{
			printf("ERROR!\n");
		}
		
	}

}

extern void schedule_days_int_to_string(uint32_t days, char *str); // implement in zcfg_fe_dal_schedulerRule.c
zcfgRet_t zcfgFeDalWlanSchedulerGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *wlanSchObj = NULL;
	struct json_object *pramJobj = NULL;

	int count = 1;
	struct json_object *enable = NULL;
	struct json_object *name = NULL;
	struct json_object *description = NULL;
#ifndef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
	struct json_object *ssid = NULL;
#endif
	uint32_t days;
	char dayString[30]={0};
	struct json_object *timeStartHour = NULL;
	struct json_object *timeStartMin = NULL;
	struct json_object *timeStopHour = NULL;
	struct json_object *timeStopMin = NULL;
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
	int group = 0;
#endif
	
	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WLAN_SCHEDULER, &objIid, &wlanSchObj) == ZCFG_SUCCESS){
		enable = json_object_object_get(wlanSchObj, "Enable");
		name = json_object_object_get(wlanSchObj, "Name");
		description = json_object_object_get(wlanSchObj, "Description");
#ifndef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
		ssid = json_object_object_get(wlanSchObj, "SSID");
#endif

		days = json_object_get_int64(json_object_object_get(wlanSchObj, "Days"));
		memset(dayString, '\0', sizeof(dayString));
		schedule_days_int_to_string(days,dayString);
		
		timeStartHour = json_object_object_get(wlanSchObj, "TimeStartHour");
		timeStartMin = json_object_object_get(wlanSchObj, "TimeStartMin");
		timeStopHour = json_object_object_get(wlanSchObj, "TimeStopHour");
		timeStopMin = json_object_object_get(wlanSchObj, "TimeStopMin");

		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(count));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(enable));
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(name));
		json_object_object_add(pramJobj, "Description", JSON_OBJ_COPY(description));

#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
		group = json_object_get_int(json_object_object_get(wlanSchObj, "Interface"));
		if(!group){
			json_object_object_add(pramJobj, "SSID", json_object_new_string("Main WLAN"));
		}else{
			json_object_object_add(pramJobj, "SSID", json_object_new_string("Guest WLAN"));
		}
#else
		json_object_object_add(pramJobj, "SSID", JSON_OBJ_COPY(ssid));
#endif
		json_object_object_add(pramJobj, "Days", json_object_new_string(dayString));
		json_object_object_add(pramJobj, "TimeStartHour", JSON_OBJ_COPY(timeStartHour));
		json_object_object_add(pramJobj, "TimeStartMin", JSON_OBJ_COPY(timeStartMin));
		json_object_object_add(pramJobj, "TimeStopHour", JSON_OBJ_COPY(timeStopHour));
		json_object_object_add(pramJobj, "TimeStopMin", JSON_OBJ_COPY(timeStopMin));
		json_object_array_add(Jarray, pramJobj);
		
		zcfgFeJsonObjFree(wlanSchObj);
		count++;
	}
	dbg_printf("%s : Jarray=%s\n",__FUNCTION__,json_object_to_json_string(Jarray));
	
	return ret;
}

extern int schedule_days_string_to_int(char *str); // implement in zcfg_fe_dal_schedulerRule.c
extern bool isValidTimePeriod(int startHour, int startMin, int stopHour, int stopMin);
zcfgRet_t zcfgFeDalWlanSchedulerAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *WlanSchedulerJobj = NULL;
	char *name = NULL;
	char msg[30]={0};
	int startHr = 0, startMin = 0, stopHr = 0, stopMin = 0;
	int ssidIndex = 0;
	char *ssidName = NULL;

	// Name validation
	if(!ISSET("DontCheckDupName") && json_object_object_get(Jobj, "Name")){
		name = (char *)json_object_get_string(json_object_object_get(Jobj, "Name"));
		ret = dalcmdParamCheck(NULL,name, RDM_OID_WLAN_SCHEDULER, "Name", "Name", msg);
		if(ret != ZCFG_SUCCESS){
			printf("%s\n",msg);
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
		json_object_object_add(Jobj, "TimeStopHour", json_object_new_int(24));
		json_object_object_add(Jobj, "TimeStopMin", json_object_new_int(0));
		printf("Incomplete stop time: set to 24:00\n");
	}

	IID_INIT(objIid);
	if((ret = zcfgFeObjJsonAdd(RDM_OID_WLAN_SCHEDULER, &objIid, NULL)) != ZCFG_SUCCESS){
		if(ret == ZCFG_EXCEED_MAX_INS_NUM)
			printf("Cannot add entry since the maximum number of entries has been reached.\n");
		return ret;
	}	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WLAN_SCHEDULER, &objIid, &WlanSchedulerJobj)) != ZCFG_SUCCESS) {
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}	

	if(json_object_object_get(Jobj, "Enable")){
		json_object_object_add(WlanSchedulerJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enable")));
	}
	if(json_object_object_get(Jobj, "Name")){
		ssidName = (char *)json_object_get_string(json_object_object_get(Jobj, "Name"));
		if(!strcmp(ssidName,"ALL_WLAN"))
			json_object_object_add(WlanSchedulerJobj, "Name", json_object_new_string("ALL WLAN"));
		else
			json_object_object_add(WlanSchedulerJobj, "Name", json_object_new_string(ssidName));
	}
	if(json_object_object_get(Jobj, "Description")){
		json_object_object_add(WlanSchedulerJobj, "Description", JSON_OBJ_COPY(json_object_object_get(Jobj, "Description")));
	}
	if(json_object_object_get(Jobj, "SSID")){
		json_object_object_add(WlanSchedulerJobj, "SSID", JSON_OBJ_COPY(json_object_object_get(Jobj, "SSID")));
	}

	// For CLI, use SSID to prepare Interface
	if(json_object_object_get(Jobj, "SSID") && !json_object_object_get(Jobj, "Interface")){
		ssidIndex = getWifiSSIDIndexFromSSID((char *)json_object_get_string(json_object_object_get(Jobj, "SSID")));
		json_object_object_add(WlanSchedulerJobj, "Interface", json_object_new_int(ssidIndex));
	}

	if(json_object_object_get(Jobj, "Interface")){
		json_object_object_add(WlanSchedulerJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
	}
	if(json_object_object_get(Jobj, "Days")){
		json_object_object_add(WlanSchedulerJobj, "Days", json_object_new_int(schedule_days_string_to_int((char *)json_object_get_string(json_object_object_get(Jobj, "Days")))));
	}
	if(json_object_object_get(Jobj, "TimeStartHour")||
		json_object_object_get(Jobj, "TimeStartMin")||
		json_object_object_get(Jobj, "TimeStopHour")||
		json_object_object_get(Jobj, "TimeStopMin")){
		json_object_object_add(WlanSchedulerJobj, "TimeStartHour", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStartHour")));
		json_object_object_add(WlanSchedulerJobj, "TimeStartMin", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStartMin")));
		json_object_object_add(WlanSchedulerJobj, "TimeStopHour", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStopHour")));
		json_object_object_add(WlanSchedulerJobj, "TimeStopMin", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStopMin")));
	}

	// Time validation
	startHr = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStartHour"));
	startMin = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStartMin"));
	stopHr = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStopHour"));
	stopMin = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStopMin"));
	if(!isValidTimePeriod(startHr,startMin,stopHr,stopMin)){
		json_object_object_add(WlanSchedulerJobj, "TimeStartHour", json_object_new_int(0));
		json_object_object_add(WlanSchedulerJobj, "TimeStartMin", json_object_new_int(0));
		json_object_object_add(WlanSchedulerJobj, "TimeStopHour", json_object_new_int(24));
		json_object_object_add(WlanSchedulerJobj, "TimeStopMin", json_object_new_int(0));
		printf("Time set to 00:00-24:00..\n");
	}	

	if((ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER, &objIid, WlanSchedulerJobj, NULL)) != ZCFG_SUCCESS)
	{
		return ret;
	}
	
	json_object_put(WlanSchedulerJobj);
	return ret;
}
zcfgRet_t zcfgFeDalWlanSchedulerEdit(struct json_object *Jobj, char *replyMsg)
{

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	struct json_object *WlanSchedulerJobj = NULL;
	bool paramModified = false;
	int objIndex = 0;
	char msg[30]={0};
	int startHr = 0, startMin = 0, stopHr = 0, stopMin = 0;
	int ssidIndex = 0;
	char *ssidName = NULL;
	// name check
	struct json_object *pramJobj = NULL;
	char *newName= NULL;
	char *oldName=NULL;
	
	//IID_INIT(objIid);
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_WLAN_SCHEDULER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WLAN_SCHEDULER, &objIid, &WlanSchedulerJobj)) != ZCFG_SUCCESS) {
		return ret;
	}

	// Name validation
	if((pramJobj = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobj);
		oldName = (char *)json_object_get_string(json_object_object_get(WlanSchedulerJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_WLAN_SCHEDULER, "Name", "Name", msg);
			if(ret != ZCFG_SUCCESS){
				printf("%s\n",msg);
				json_object_put(WlanSchedulerJobj);
				return ret; 		
			}
		}
	}

	if(json_object_object_get(Jobj, "Enable")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(Jobj, "Enable")));
	}
	if(json_object_object_get(Jobj, "Name")){
		paramModified = true;
		ssidName = (char *)json_object_get_string(json_object_object_get(Jobj, "Name"));
		if(!strcmp(ssidName,"ALL_WLAN"))
			json_object_object_add(WlanSchedulerJobj, "Name", json_object_new_string("ALL WLAN"));
		else
			json_object_object_add(WlanSchedulerJobj, "Name", json_object_new_string(ssidName));
	}
	if(json_object_object_get(Jobj, "Description")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "Description", JSON_OBJ_COPY(json_object_object_get(Jobj, "Description")));
	}
	if(json_object_object_get(Jobj, "SSID")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "SSID", JSON_OBJ_COPY(json_object_object_get(Jobj, "SSID")));
	}

	// For CLI, use SSID to prepare Interface
	if(json_object_object_get(Jobj, "SSID") && !json_object_object_get(Jobj, "Interface")){
		ssidIndex = getWifiSSIDIndexFromSSID((char *)json_object_get_string(json_object_object_get(Jobj, "SSID")));
		json_object_object_add(WlanSchedulerJobj, "Interface", json_object_new_int(ssidIndex));
	}

	if(json_object_object_get(Jobj, "Interface")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(Jobj, "Interface")));
	}
	if(json_object_object_get(Jobj, "Days")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "Days", json_object_new_int(schedule_days_string_to_int((char *)json_object_get_string(json_object_object_get(Jobj, "Days")))));
	}
	if(json_object_object_get(Jobj, "TimeStartHour")||
		json_object_object_get(Jobj, "TimeStartMin")||
		json_object_object_get(Jobj, "TimeStopHour")||
		json_object_object_get(Jobj, "TimeStopMin")){
		paramModified = true;
		json_object_object_add(WlanSchedulerJobj, "TimeStartHour", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStartHour")));
		json_object_object_add(WlanSchedulerJobj, "TimeStartMin", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStartMin")));
		json_object_object_add(WlanSchedulerJobj, "TimeStopHour", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStopHour")));
		json_object_object_add(WlanSchedulerJobj, "TimeStopMin", JSON_OBJ_COPY(json_object_object_get(Jobj, "TimeStopMin")));
	}

	// Time validation
	startHr = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStartHour"));
	startMin = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStartMin"));
	stopHr = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStopHour"));
	stopMin = json_object_get_int(json_object_object_get(WlanSchedulerJobj, "TimeStopMin"));
	if(!isValidTimePeriod(startHr,startMin,stopHr,stopMin)){
		json_object_object_add(WlanSchedulerJobj, "TimeStartHour", json_object_new_int(0));
		json_object_object_add(WlanSchedulerJobj, "TimeStartMin", json_object_new_int(0));
		json_object_object_add(WlanSchedulerJobj, "TimeStopHour", json_object_new_int(24));
		json_object_object_add(WlanSchedulerJobj, "TimeStopMin", json_object_new_int(0));
		printf("Time set to 00:00-24:00..\n");
	}	

	if(paramModified) {
		if((ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER, &objIid, WlanSchedulerJobj, NULL)) != ZCFG_SUCCESS);
	}
	json_object_put(WlanSchedulerJobj);
	return ret;
}

zcfgRet_t zcfgFeDalWlanSchedulerActiveEdit(struct json_object *Jobj, char *replyMsg){ 
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *wlanSchedulerActiveObj = NULL;
	struct json_object *reqObj = NULL;
	int reqObjIdx = 1;
	objIndex_t schedulerIid = {0};
	
	bool enable = false;
	int Index = 0;

	int len = 0;
	
	len = json_object_array_length(Jobj) - 1;
	while((reqObj = json_object_array_get_idx(Jobj, reqObjIdx)) !=NULL){

		Index = json_object_get_int(json_object_object_get(reqObj, "Index"));
		enable = json_object_get_boolean(json_object_object_get(reqObj, "Enable"));
		reqObjIdx++;
	
		IID_INIT(schedulerIid);
		schedulerIid.level = 1;
		schedulerIid.idx[0] = Index;
	
		ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WLAN_SCHEDULER, &schedulerIid, &wlanSchedulerActiveObj);
		if(ret == ZCFG_SUCCESS) {
			json_object_object_add(wlanSchedulerActiveObj, "Enable", json_object_new_boolean(enable));
			if(Index == len) {
				ret = zcfgFeObjJsonSet(RDM_OID_WLAN_SCHEDULER, &schedulerIid, wlanSchedulerActiveObj, NULL);
			}
			else {
				ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_WLAN_SCHEDULER, &schedulerIid, wlanSchedulerActiveObj, NULL);
			}
		}
		zcfgFeJsonObjFree(wlanSchedulerActiveObj);
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalWlanSchedulerDelete(struct json_object *Jobj, char *replyMsg)
{	
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid={0};
	int objIndex = 0;
	
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_WLAN_SCHEDULER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	
	ret = zcfgFeObjJsonDel(RDM_OID_WLAN_SCHEDULER, &objIid, NULL);
	return ret;
	
}
zcfgRet_t zcfgFeDalWlanScheduler(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;


	if(!strcmp(method, "PUT")) {
		if(json_object_get_type(Jobj) == json_type_array){ // for GUI, tmp solution
			ret = zcfgFeDalWlanSchedulerActiveEdit(Jobj, NULL); 
		}
	 	else{
			ret = zcfgFeDalWlanSchedulerEdit(Jobj, NULL); 	
		}
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalWlanSchedulerAdd(Jobj, NULL);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalWlanSchedulerDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWlanSchedulerGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method.\n");
	}

	return ret;
}

