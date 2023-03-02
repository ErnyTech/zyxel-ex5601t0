#include <json/json.h>
#include <sys/stat.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zcfg_msg_voice.h"
#include "zos_api.h"

#define CALLLIST_PATH 			"/data/calllist.dat"

dal_param_t VOIP_CALLSUMMARY_param[]={
	{"range",			dalType_string,	0,	0,	NULL,	"All",	dal_Delete},
	{NULL,				dalType_int,	0,	0,	NULL,	NULL,	0},
};

void zcfgFeDalShowVoipCallSummary(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *historyObj;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-20s %-10s %-10s %-10s %-10s %-20s\n","Date","Total","Incoming","Outgoing","Missed","Duration");
	printf("------------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		historyObj = json_object_array_get_idx(Jarray, i);
		if(historyObj!=NULL){
			printf("%-20s ",json_object_get_string(json_object_object_get(historyObj,"Date")));
			printf("%-10s ",json_object_get_string(json_object_object_get(historyObj,"Total")));
			printf("%-10s ",json_object_get_string(json_object_object_get(historyObj,"Incoming")));
			printf("%-10s ",json_object_get_string(json_object_object_get(historyObj,"Outgoing")));
			printf("%-10s ",json_object_get_string(json_object_object_get(historyObj,"Missed")));
			printf("%-20s ",json_object_get_string(json_object_object_get(historyObj,"Duration")));
			printf("\n");
		}
	}
	printf("\n");
	
}

// e.g. addDur="12:13:14" curDur="22:23:24" then newDur="34:36:38"
void accumDuration(char *addDur, char *curDur, char *newDur){
	char str1[256]={0};
	char str2[256]={0};
	char str3[256]={0};
	char str4[256]={0};
	strcpy(str1,addDur);
	strcpy(str2,curDur);
	int hh1=0, hh2=0, hh=0;
	int mm1=0, mm2=0, mm=0;
	int ss1=0, ss2=0, ss=0;
	int t1=0, t2=0, t3=0;

	strncpy(str3,str1,2);
	*(str3+2)='\0';
	hh1 = (int)strtol(str3,NULL,10);		
	strncpy(str3,str1+3,2);
	*(str3+2)='\0';
	mm1 = (int)strtol(str3,NULL,10);		
	strncpy(str3,str1+6,2);
	*(str3+2)='\0';
	ss1 = (int)strtol(str3,NULL,10);

	t1 = hh1*3600 + mm1*60 + ss1;

	strncpy(str4,str2,2);
	*(str4+2)='\0';
	hh2 = (int)strtol(str4,NULL,10);		
	strncpy(str4,str2+3,2);
	*(str4+2)='\0';
	mm2 = (int)strtol(str4,NULL,10);		
	strncpy(str4,str2+6,2);
	*(str4+2)='\0';
	ss2 = (int)strtol(str4,NULL,10);
	
	t2 = hh2*3600 + mm2*60 + ss2;

	t3 = t1 + t2;
	ss = t3%60;
	mm = ((t3-ss)/60)%60;
	hh = (int) t3/3600;
	sprintf(str3,"%02d:%02d:%02d",hh,mm,ss);
	strcpy(newDur,str3);

}

extern zcfgRet_t zcfgFeDalVoipCallHistoryGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg); // implement in zcfg_fe_dal_call_history.c
zcfgRet_t zcfgFeDalVoipCallSummaryGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *histJarray = NULL;
	struct json_object *histObj = NULL;
	struct json_object *callInfoObj = NULL;
	int i, histArrLen = 0;
	int j, JarrayLen = 0;
	struct json_object *sumObj = NULL;
	char date[32];
	char *callType=NULL;
	int totalCount = -1;
	int inCount = -1;
	int outCount = -1;
	int missedCount = -1;
	char *tmpDate=NULL;
	char *ptr=NULL;
	char *tmpDuration=NULL;
	bool foundEntry = false;
	
	histJarray = json_object_new_array();
	if((ret=zcfgFeDalVoipCallHistoryGet(Jobj,histJarray,NULL))!=ZCFG_SUCCESS){
		return ret;
	}
	
	histArrLen = json_object_array_length(histJarray);
	for(i=0;i<histArrLen;i++){
		histObj = json_object_array_get_idx(histJarray, i);
		if(histObj!=NULL){
			callInfoObj = json_object_object_get(histObj,"CallInfo");

			// date
			tmpDate = (char *)json_object_get_string(json_object_object_get(callInfoObj,"Date"));
			ptr = strchr(tmpDate,' ');
			ZOS_STRNCPY(date,tmpDate,(ptr-tmpDate+1));

			// tmpDuration
			tmpDuration = (char *)json_object_get_string(json_object_object_get(callInfoObj,"Duration"));

			// callType
			callType = (char *)json_object_get_string(json_object_object_get(callInfoObj,"CallType"));
			
			JarrayLen = json_object_array_length(Jarray);
			foundEntry = false;

			if(JarrayLen > 0){ // update existed entry
				for(j=0;j<JarrayLen;j++){
					sumObj = json_object_array_get_idx(Jarray, j);
				
					if(!strcmp(date,json_object_get_string(json_object_object_get(sumObj,"Date")))){
						foundEntry = true;
						totalCount = json_object_get_int(json_object_object_get(sumObj,"Total"));
						json_object_object_add(sumObj,"Total",json_object_new_int((totalCount+1)));
						if(!strcmp(callType,"0")){
							missedCount = json_object_get_int(json_object_object_get(sumObj,"Missed"));
							json_object_object_add(sumObj,"Missed",json_object_new_int((missedCount+1)));
						}
						else if(!strcmp(callType,"1")){
							outCount = json_object_get_int(json_object_object_get(sumObj,"Outgoing"));
							json_object_object_add(sumObj,"Outgoing",json_object_new_int((outCount+1)));
						}
						else{
							inCount = json_object_get_int(json_object_object_get(sumObj,"Incoming"));
							json_object_object_add(sumObj,"Incoming",json_object_new_int((inCount+1)));
						}
						
						char *curDuration = (char *)json_object_get_string(json_object_object_get(sumObj,"Duration"));
						char newDuration[32]={0};
						accumDuration(tmpDuration,curDuration,newDuration);
						json_object_object_add(sumObj,"Duration",json_object_new_string(newDuration));
						json_object_array_put_idx(Jarray,j,JSON_OBJ_COPY(sumObj));
						break;
					}
				}
			}

			if(!foundEntry){ // Add new entry
				sumObj = json_object_new_object();
				json_object_object_add(sumObj, "Date", json_object_new_string(date));
				json_object_object_add(sumObj, "Total", json_object_new_int(1));
				if(!strcmp(callType,"0")){
					json_object_object_add(sumObj, "Incoming", json_object_new_int(0));
					json_object_object_add(sumObj, "Outgoing", json_object_new_int(0));
					json_object_object_add(sumObj, "Missed", json_object_new_int(1));
				}
				else if(!strcmp(callType,"1")){
					json_object_object_add(sumObj, "Incoming", json_object_new_int(0));
					json_object_object_add(sumObj, "Outgoing", json_object_new_int(1));
					json_object_object_add(sumObj, "Missed", json_object_new_int(0));
				}
				else{
					json_object_object_add(sumObj, "Incoming", json_object_new_int(1));
					json_object_object_add(sumObj, "Outgoing", json_object_new_int(0));
					json_object_object_add(sumObj, "Missed", json_object_new_int(0));
				}
				json_object_object_add(sumObj, "Duration", json_object_new_string(tmpDuration));
				json_object_array_add(Jarray,JSON_OBJ_COPY(sumObj));
			}
		}
	}

	if(histJarray)
		zcfgFeJsonObjFree(histJarray);
	return ret;
}

extern zcfgRet_t zcfgFeDalVoipCallHistoryDelete(struct json_object *Jobj, char *replyMsg);
zcfgRet_t zcfgFeDalVoipCallSummaryDelete(struct json_object *Jobj, char *replyMsg){
	return zcfgFeDalVoipCallHistoryDelete(NULL,NULL);
}

zcfgRet_t zcfgFeDalVoipCallSummary(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalVoipCallSummaryDelete(Jobj, NULL);
	}else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalVoipCallSummaryGet(Jobj, Jarray, NULL);
	}

	return ret;
}


