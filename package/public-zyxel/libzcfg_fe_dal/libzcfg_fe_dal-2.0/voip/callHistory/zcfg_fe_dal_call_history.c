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

dal_param_t VOIP_CALLHISTORY_param[]={
	{"range",	dalType_string,	0,	0,	NULL,	"All",	dal_Delete},
	{NULL,		dalType_int,	0,	0,	NULL,	NULL,	0},
};

//!!!!NOTE: Should sync with 'ZyIMS-0.7.1/core/voiceApp/voice/h/webCallHistory.h'!!!!
typedef enum {
	CALL_HISTORY_CALL_TYPE_INCOMING_BEGIN = 10,
	CALL_HISTORY_CALL_TYPE_INCOMING = CALL_HISTORY_CALL_TYPE_INCOMING_BEGIN,
	CALL_HISTORY_CALL_TYPE_INCOMING_MISSED,
	CALL_HISTORY_CALL_TYPE_INCOMING_BLOCKED,
	CALL_HISTORY_CALL_TYPE_INCOMING_END,

	CALL_HISTORY_CALL_TYPE_OUTGOING_BEGIN = 20,
	CALL_HISTORY_CALL_TYPE_OUTGOING = CALL_HISTORY_CALL_TYPE_OUTGOING_BEGIN,
	CALL_HISTORY_CALL_TYPE_OUTGOING_MISSED, //Reserved, not used now.
	CALL_HISTORY_CALL_TYPE_OUTGOING_BLOCKED,
	CALL_HISTORY_CALL_TYPE_OUTGOING_END
} callHistoryCallType_e;


zcfgRet_t validateCallHistoryJarray(struct json_object *Jarray){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int i, len=0;
	struct json_object *obj = NULL;
	struct json_object *callInfo = NULL;
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
		if(obj!=NULL){
			callInfo = json_object_object_get(obj,"CallInfo");
			if(callInfo != NULL){
				if(!json_object_object_get(callInfo,"CallType"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"Date"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"PeerName"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"PeerNumber"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"LocalDevice"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"OutgoingNumber"))
					ret = ZCFG_INTERNAL_ERROR;
				if(!json_object_object_get(callInfo,"Duration"))
					ret = ZCFG_INTERNAL_ERROR;
			}
			else
				ret = ZCFG_INTERNAL_ERROR;
		} 

		dbg_printf("%s : callInfo = %s\n",__FUNCTION__,json_object_to_json_string(callInfo));
		if(ret != ZCFG_SUCCESS){
			printf("Incomplete call history object!\n");
			printf("%s : callInfo = %s\n",__FUNCTION__,json_object_to_json_string(callInfo));
			return ret;
		}
	}

	return ret;
}

void zcfgFeDalShowVoipCallHistory(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *historyObj;
	struct json_object *contentObj;
	char *callType = NULL;	

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-7s %-20s %-25s %-10s %-10s %-12s %-12s %-10s \n","Index","Type","Date","Name","Number","Phone Dev.","Out Number","Duration(hh:mm:ss)");
	printf("-------------------------------------------------------------------------------------------------------------------\n");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		historyObj = json_object_array_get_idx(Jarray, len-i-1);
		if(historyObj!=NULL){
			contentObj = json_object_object_get(historyObj,"CallInfo");
			printf("%-7d ",i+1);
			callType = (char *)json_object_get_string(json_object_object_get(contentObj,"CallType"));

			/*Backward Compatible*/
			if(!strcmp(callType,"0"))
				printf("%-10s ","Missed");
			else if(!strcmp(callType,"1"))
				printf("%-10s ","Outgoing");
			else if(!strcmp(callType,"2"))
				printf("%-10s ","Incoming");
			else if(!strcmp(callType,"10"))
				printf("%-20s ","Incoming");
			else if(!strcmp(callType,"11"))
				printf("%-20s ","Missed");
			else if(!strcmp(callType,"12"))
				printf("%-20s ","Incoming Blocked");
			else if(!strcmp(callType,"20"))
				printf("%-20s ","Outgoing");
			else if(!strcmp(callType,"22"))
				printf("%-20s ","Outgoing Blocked");
			else
				printf("%-20s ","N/A");

			printf("%-25s ",json_object_get_string(json_object_object_get(contentObj,"Date")));
			printf("%-10s ",json_object_get_string(json_object_object_get(contentObj,"PeerName")));
			printf("%-10s ",json_object_get_string(json_object_object_get(contentObj,"PeerNumber")));
			printf("%-12s ",json_object_get_string(json_object_object_get(contentObj,"LocalDevice")));
			printf("%-12s ",json_object_get_string(json_object_object_get(contentObj,"OutgoingNumber")));
			printf("%-10s ",json_object_get_string(json_object_object_get(contentObj,"Duration")));
			printf("\n");
			//zcfgFeJsonObjFree(historyObj);
		}
	}
	printf("\n");
	
}

zcfgRet_t zcfgFeDalVoipCallHistoryGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	struct stat dectFile;
	struct json_object *callListDatabase = NULL;
	struct json_object *responseObj = NULL;

	if(!stat(CALLLIST_PATH, &dectFile) ){
		callListDatabase = json_object_from_file(CALLLIST_PATH);
		if(is_error(callListDatabase)) {
			printf("get dect book fail\n");
			return ZCFG_INTERNAL_ERROR;
		}
		json_object_object_foreach(callListDatabase,itemCallHistory,val){
			if(!strcmp(itemCallHistory, "CallHistory")){
				json_object_object_foreach(val,key,callInfo){
					responseObj = json_object_new_object();
					json_object_object_add(responseObj, "Index", json_object_new_string(key));
					json_object_object_add(responseObj, "CallInfo", JSON_OBJ_COPY(callInfo));
					json_object_array_add(Jarray, JSON_OBJ_COPY(responseObj));
					zcfgFeJsonObjFree(responseObj);
				}
			}
		}
	}
	else{
		printf("get file error, file not exist\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if(callListDatabase != NULL){
		json_object_put(callListDatabase);
	}

	ret = validateCallHistoryJarray(Jarray);
	return ret;
}

zcfgRet_t zcfgFeDalVoipCallHistoryDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	voiceListAccessReq_t *voiceLAReq = NULL;

	/* allocate message buffer */
	if ((sendBuf = ZOS_MALLOC(sizeof(zcfgMsg_t)+sizeof(voiceListAccessReq_t))) == NULL) {
		printf("%s(): <<Error>> Fail to allocate buffer for message xmit\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	sendMsgHdr = (zcfgMsg_t *)sendBuf;

	sendMsgHdr->type = ZCFG_MSG_VOICE_LIST_ACCESS;
	sendMsgHdr->srcEid = ZCFG_EID_DALCMD;
	sendMsgHdr->dstEid = ZCFG_EID_ZYIMS;
	sendMsgHdr->length = sizeof(voiceListAccessReq_t);

	voiceLAReq = (voiceListAccessReq_t *)(sendMsgHdr + 1);
	voiceLAReq->Type = VOICE_LIST_ACCESS_REQ_CALLHISTORY_CLEAR;
	voiceLAReq->Payload = NULL;

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, ZCFG_MSG_VOICE_REPLY_WAIT_TIMEOUT);
	if (ret != ZCFG_SUCCESS){
		printf("Send Error! ret:%d\n", ret);
	}
	ZOS_FREE(recvBuf);
	
	return ret;
}

zcfgRet_t zcfgFeDalVoipCallHistory(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalVoipCallHistoryDelete(Jobj, NULL);
	}else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalVoipCallHistoryGet(Jobj, Jarray, NULL);
	}

	return ret;
}

