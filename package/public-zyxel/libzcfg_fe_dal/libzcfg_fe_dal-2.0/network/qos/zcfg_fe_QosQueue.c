#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <string.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_Qos.h"

typedef struct s_QosQueue {
	int Index;
	bool Enable;
	char *Name;
	char *Interface;
	char *SchedulerAlgorithm;
	char *Setting;
	int Priority;
	int Weight;
	char *DropAlgorithm;
	int ShapingRate;
}s_QosQueue;

typedef struct s_QosQueueDel {
	int Index;
}s_QosQueueDel;

typedef struct s_QosQueueIidMapping{
	int size;
	int iid[20];
}s_QosQueueIidMapping;

s_QosQueueIidMapping QosQueueIidMappingTable = {0};

dal_param_t QosQueue_param[] = {
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	{"Index",				dalType_int, 		0,	0,			NULL,	NULL,					0},
	{"SchedulerAlgorithm",	dalType_string, 	0,	0,			NULL,	NULL,					0},
	{"Setting",				dalType_string, 	0,	0,			NULL,	"SP|SP+WRR|WRR",		dal_Edit},
#else
	{"Index",				dalType_int, 		0,	0,			NULL,	NULL,	dal_Edit|dal_Delete},
#endif
	{"Enable",				dalType_boolean, 	0,	0,			NULL,	NULL,	dal_Add},
	{"Name",				dalType_string, 	0,	0,			NULL,	NULL,	dal_Add},
	{"Interface",			dalType_string, 	0,	0,			NULL,	"WAN",	dal_Add},
	{"Priority",			dalType_int, 		1,	8,			NULL,	NULL,	dal_Add},
	{"Weight",				dalType_int, 		1,	8,			NULL,	NULL,	dal_Add},
	{"DropAlgorithm",		dalType_string, 	0,	0,			NULL,	"DT",	dal_Add},
	{"ShapingRate",			dalType_int, 		0,	QOS_MAX_SHAPING_RATE,	NULL,	NULL,	0},
	{NULL,					0,					0,	0,			NULL,	NULL,	0},	
};

void QueueInfoInit(s_QosQueue *QosQueue_Info)
{
    QosQueue_Info->Index = 0;
    QosQueue_Info->Enable = NULL;
    QosQueue_Info->Name = NULL;
    QosQueue_Info->Interface = NULL;
	QosQueue_Info->SchedulerAlgorithm = NULL;
	QosQueue_Info->Setting = NULL;
	QosQueue_Info->Priority = 0;
	QosQueue_Info->Weight = 0;
	QosQueue_Info->DropAlgorithm = NULL; 
	QosQueue_Info->ShapingRate = 0;
}

void QueueDelInfoInit(s_QosQueueDel *QosQueueDel_Info)
{
    QosQueueDel_Info->Index = 0;
}


void getQosQueueBasicInfo(struct json_object *Jobj, s_QosQueue *QosQueue_Info) {
	
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	if(json_object_object_get(Jobj, "Index")!= NULL)
		QosQueue_Info->Index = (json_object_get_int(json_object_object_get(Jobj, "Index")));
	if(json_object_object_get(Jobj, "Setting") != NULL)
		QosQueue_Info->Setting = (char*)json_object_get_string(json_object_object_get(Jobj, "Setting"));
	if(json_object_object_get(Jobj, "SchedulerAlgorithm") != NULL)		
		QosQueue_Info->SchedulerAlgorithm = (char*)json_object_get_string(json_object_object_get(Jobj, "SchedulerAlgorithm"));
#else
	//decrease default queue column
	QosQueue_Info->Index = (json_object_get_int(json_object_object_get(Jobj, "Index")))-1;
#endif
	if(json_object_object_get(Jobj, "Enable")!= NULL)
		QosQueue_Info->Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(json_object_object_get(Jobj, "Name") != NULL)
		QosQueue_Info->Name = (char*)json_object_get_string(json_object_object_get(Jobj, "Name"));
	if(json_object_object_get(Jobj, "Interface") != NULL)
		QosQueue_Info->Interface = (char*)json_object_get_string(json_object_object_get(Jobj, "Interface"));
	if(json_object_object_get(Jobj, "Priority") != NULL)
		QosQueue_Info->Priority = json_object_get_int(json_object_object_get(Jobj, "Priority"));
	if(json_object_object_get(Jobj, "Weight") != NULL)
		QosQueue_Info->Weight = json_object_get_int(json_object_object_get(Jobj, "Weight"));
	if(json_object_object_get(Jobj, "DropAlgorithm") != NULL)
		QosQueue_Info->DropAlgorithm = (char*)json_object_get_string(json_object_object_get(Jobj, "DropAlgorithm"));
	if(json_object_object_get(Jobj, "ShapingRate")!= NULL)
		QosQueue_Info->ShapingRate = json_object_get_int(json_object_object_get(Jobj, "ShapingRate"));

	return;
}

void prepare_QosQueue_Info(struct json_object *Jobj, json_object *queueObj, s_QosQueue* QosQueue_Info) {
	if(json_object_object_get(Jobj, "Enable") != NULL)
		json_object_object_add(queueObj, "Enable", json_object_new_boolean(QosQueue_Info->Enable));
	if(json_object_object_get(Jobj, "Name") != NULL)
		json_object_object_add(queueObj, "X_ZYXEL_Name", json_object_new_string(QosQueue_Info->Name));
	if(json_object_object_get(Jobj, "Interface") != NULL)
		json_object_object_add(queueObj, "Interface", json_object_new_string(QosQueue_Info->Interface));
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	if(json_object_object_get(Jobj, "SchedulerAlgorithm") != NULL)
		json_object_object_add(queueObj, "SchedulerAlgorithm", json_object_new_string(QosQueue_Info->SchedulerAlgorithm));
#endif
	if(json_object_object_get(Jobj, "Priority") != NULL)
		json_object_object_add(queueObj, "Precedence", json_object_new_int(QosQueue_Info->Priority));
	if(json_object_object_get(Jobj, "Weight") != NULL)
		json_object_object_add(queueObj, "Weight", json_object_new_int(QosQueue_Info->Weight));
	if(json_object_object_get(Jobj, "DropAlgorithm") != NULL)
		json_object_object_add(queueObj, "DropAlgorithm", json_object_new_string(QosQueue_Info->DropAlgorithm));
	json_object_object_add(queueObj, "X_ZYXEL_PtmPriority", json_object_new_int(0));
	json_object_object_add(queueObj, "X_ZYXEL_DslLatency", json_object_new_int(0));
	
	return;
}


void getQosQueueDelKey(struct json_object *Jobj, s_QosQueueDel *QosQueueDel_Info) {
	//decrease default queue column
	QosQueueDel_Info->Index = (json_object_get_int(json_object_object_get(Jobj, "Index")))-1;
	return;
}

void updateQosQueueInfo(e_ActionType action, int target_iid){
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};

	memset(&QosQueueIidMappingTable, 0, sizeof(s_QosQueueIidMapping));
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
		QosQueueIidMappingTable.iid[QosQueueIidMappingTable.size] = queueIid.idx[0];
		QosQueueIidMappingTable.size += 1;
		zcfgFeJsonObjFree(queueObj);
	}

	return;
}
extern uint32_t StrToIntConv(char *str); // inplement in zcfg_fe_dal.c
void remove_related_qosClass(char *TrafficClasses) {
	char *rest_str = NULL;
	char *TrafficClasses_single_str = NULL;
	int TrafficClass = -1;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};
	
	TrafficClasses_single_str = strtok_r(TrafficClasses, ",", &rest_str);
	while(TrafficClasses_single_str != NULL) {
		TrafficClass = StrToIntConv(TrafficClasses_single_str);
		
		IID_INIT(classIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
			if(classIid.idx[0] == TrafficClass) {
				zcfgFeObjJsonDel(RDM_OID_QOS_CLS, &classIid, NULL);
				zcfgFeJsonObjFree(classObj);
				break;
			}
			zcfgFeJsonObjFree(classObj);
		}
		TrafficClasses_single_str = strtok_r(rest_str, ",", &rest_str);
	}
	return;
}

void zcfgFeDalShowQosQueue(struct json_object *Jarray){	
	int i, len = 0;	
	struct json_object *obj;	
	if(json_object_get_type(Jarray) != json_type_array){		
		printf("wrong Jobj format!\n");		
		return;	
	}	

#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
    printf("%-7s %-12s %-20s %-12s %-12s %-12s %-12s %-20s %-12s \n",		    
		"Index", "Enable", "Name", "Interface", "SchedulerAlgorithm", "Priority", "Weight", "Buffer Management", "Rate Limit");
#else
	printf("%-7s %-12s %-20s %-12s %-12s %-12s %-20s %-12s \n",		    
		"Index", "Enable", "Name", "Interface", "Priority", "Weight", "Buffer Management", "Rate Limit");
	//default queue
	printf("%-7s %-12s %-20s %-12s %-12s %-12s %-20s %-12s \n",		    
		"1", "1", "default queue", "WAN", "8", "1", "DT", "0");
#endif
	len = json_object_array_length(Jarray);	
	for(i=0;i<len;i++){		
		obj = json_object_array_get_idx(Jarray, i);		
		if(obj != NULL){			
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
			printf("%-7s %-12d %-20s %-12s %-12s %-12s %-12s %-20s %-12s \n",			
				json_object_get_string(json_object_object_get(obj, "Index")),			
				json_object_get_boolean(json_object_object_get(obj, "Enable")),			
				json_object_get_string(json_object_object_get(obj, "Name")),			
				json_object_get_string(json_object_object_get(obj, "Interface")),
                json_object_get_string(json_object_object_get(obj, "SchedulerAlgorithm")),
				json_object_get_string(json_object_object_get(obj, "Priority")),			
				json_object_get_string(json_object_object_get(obj, "Weight")),		
				json_object_get_string(json_object_object_get(obj, "BufferManagement")),			
				json_object_get_string(json_object_object_get(obj, "RateLimit")));	
#else
			printf("%-7s %-12d %-20s %-12s %-12s %-12s %-20s %-12s \n",			
				json_object_get_string(json_object_object_get(obj, "Index")),			
				json_object_get_boolean(json_object_object_get(obj, "Enable")),			
				json_object_get_string(json_object_object_get(obj, "Name")),			
				json_object_get_string(json_object_object_get(obj, "Interface")),
				json_object_get_string(json_object_object_get(obj, "Priority")),			
				json_object_get_string(json_object_object_get(obj, "Weight")),		
				json_object_get_string(json_object_object_get(obj, "BufferManagement")),			
				json_object_get_string(json_object_object_get(obj, "RateLimit")));		
#endif
		}	
	}
}

zcfgRet_t zcfgFeDalQosQueueGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *pramJobj = NULL;
	struct json_object *queueObj = NULL;
	objIndex_t queueIid = {0};
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	int count = 1;
#else
	int count = 2;
#endif
	int idx = -1;

	if ((Jobj != NULL) && (json_object_object_get(Jobj, "idx") != NULL))
	{
		idx = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}

	IID_INIT(queueIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid , &queueObj) == ZCFG_SUCCESS){
		if (idx == -1 || queueIid.idx[0] == idx)
		{
		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(count));
		json_object_object_add(pramJobj, "idx", json_object_new_int(queueIid.idx[0]));
		json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(queueObj, "X_ZYXEL_Name")));
		json_object_object_add(pramJobj, "RateLimit",JSON_OBJ_COPY(json_object_object_get(queueObj, "ShapingRate")));
		json_object_object_add(pramJobj, "BufferManagement",JSON_OBJ_COPY(json_object_object_get(queueObj, "DropAlgorithm")));
		json_object_object_add(pramJobj, "Interface",JSON_OBJ_COPY(json_object_object_get(queueObj, "Interface")));
		json_object_object_add(pramJobj, "Priority",JSON_OBJ_COPY(json_object_object_get(queueObj, "Precedence")));
		json_object_object_add(pramJobj, "Weight",JSON_OBJ_COPY(json_object_object_get(queueObj, "Weight")));
		json_object_object_add(pramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(queueObj, "Enable")));
		json_object_object_add(pramJobj, "TrafficClasses",  JSON_OBJ_COPY(json_object_object_get(queueObj, "TrafficClasses")));
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
        json_object_object_add(pramJobj, "SchedulerAlgorithm",JSON_OBJ_COPY(json_object_object_get(queueObj, "SchedulerAlgorithm")));
#endif
		json_object_array_add(Jarray, pramJobj);
		}
		count++;		
		zcfgFeJsonObjFree(queueObj);
	}	
	return ret;
}

zcfgRet_t zcfgFeDalQosQueueAdd(struct json_object *Jobj, char *replyMsg) {
	
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	printf("ECONET not support add function.\n");
	return ret;
#endif
	int len = 0;
	s_QosQueue QosQueue_Info;
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};
	IID_INIT(queueIid);
	struct json_object *Jarray = NULL;

	Jarray = json_object_new_array();	
	zcfgFeDalQosQueueGet(NULL, Jarray, NULL);
	len = json_object_array_length(Jarray);	
	if(len == 7){
		printf("Maximum Qos Queue rule entries for WAN port has been reached. Unable to create more Qos Queue entries.\n");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	
	QueueInfoInit(&QosQueue_Info);
	getQosQueueBasicInfo(Jobj, &QosQueue_Info);

	zcfg_offset_t oid;	
	oid = RDM_OID_QOS_QUE;
	if(QosQueue_Info.Name != NULL){
		ret = dalcmdParamCheck(NULL, QosQueue_Info.Name , oid, "X_ZYXEL_Name", "Name",  replyMsg);
		if(ret != ZCFG_SUCCESS)			
		return ret;
	}

	ret = zcfgFeObjJsonAdd(RDM_OID_QOS_QUE, &queueIid, NULL);
	if(ret == ZCFG_SUCCESS) {
		ret = zcfgFeObjJsonGet(RDM_OID_QOS_QUE, &queueIid, &queueObj);
	}

	if(ret == ZCFG_SUCCESS) {
		prepare_QosQueue_Info(Jobj, queueObj, &QosQueue_Info);
		//Shaping rate range is between 0 and QOS_MAX_SHAPING_RATE, 0 imply no limit
		if(json_object_object_get(Jobj, "ShapingRate")!= NULL)
			json_object_object_add(queueObj, "ShapingRate", json_object_new_int(QosQueue_Info.ShapingRate));
		//If shaping rate is not set, set default value to -1
		else{
			json_object_object_add(queueObj, "ShapingRate", json_object_new_int(-1));
		}
		zcfgFeObjJsonBlockedSet(RDM_OID_QOS_QUE, &queueIid, queueObj, NULL);
		updateQosQueueInfo(e_ActionType_ADD, queueIid.idx[0]);
		zcfgFeJsonObjFree(queueObj);
		return ret;
	}

	else{
		printf("%s: Object retrieve fail\n", __FUNCTION__);
		return ret;
	}
	
}

zcfgRet_t zcfgFeDalQosQueueEdit(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	s_QosQueue QosQueue_Info;
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};
	IID_INIT(queueIid);

	QueueInfoInit(&QosQueue_Info);
	getQosQueueBasicInfo(Jobj, &QosQueue_Info);

#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	//check econet edit dalcmd 
	if(QosQueue_Info.Index == 0){
		int count = 1;
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid , &queueObj) == ZCFG_SUCCESS){
			if(!strcmp("SP",QosQueue_Info.Setting))
				json_object_object_add(queueObj, "SchedulerAlgorithm", json_object_new_string("SP"));
			if(!strcmp("WRR",QosQueue_Info.Setting))
				json_object_object_add(queueObj, "SchedulerAlgorithm", json_object_new_string("WRR"));
			if(!strcmp("SP+WRR",QosQueue_Info.Setting)){
				if(count == 1 || count == 2)
					json_object_object_add(queueObj, "SchedulerAlgorithm", json_object_new_string("SP"));
				if(count == 3 || count == 4)
					json_object_object_add(queueObj, "SchedulerAlgorithm", json_object_new_string("WRR"));
			}
			count++;
			zcfgFeObjJsonBlockedSet(RDM_OID_QOS_QUE, &queueIid, queueObj, NULL);
			zcfgFeJsonObjFree(queueObj);
		}
		return ret;
	}
#else
	if(QosQueue_Info.Index == 0){
		printf("Default queue data can not be modified.\n");
		return ZCFG_INVALID_PARAM_VALUE;
	}
#endif
	zcfg_offset_t oid;	
	oid = RDM_OID_QOS_QUE;	
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		queueIid.level = 1;
		queueIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else 
	{
	ret = convertIndextoIid(QosQueue_Info.Index, &queueIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)		
		return ret;
	}
	if(QosQueue_Info.Name != NULL){
		ret = dalcmdParamCheck(&queueIid, QosQueue_Info.Name , oid, "X_ZYXEL_Name", "Name",  replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}

	// PUT RDM_OID_QOS_QUE	
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
		prepare_QosQueue_Info(Jobj, queueObj, &QosQueue_Info);
		//Shaping rate range is between 0 and QOS_MAX_SHAPING_RATE, -1 and 0 imply no limit
		if(json_object_object_get(Jobj, "ShapingRate") != NULL)
			json_object_object_add(queueObj, "ShapingRate", json_object_new_int(QosQueue_Info.ShapingRate));
		zcfgFeObjJsonBlockedSet(RDM_OID_QOS_QUE, &queueIid, queueObj, NULL);
		zcfgFeJsonObjFree(queueObj);		
		return ret;
	}

	else{		
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Queue fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_QUE);		
		return ZCFG_INTERNAL_ERROR;	
	}
}

zcfgRet_t zcfgFeDalQosQueueDelete(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	printf("ECONET not support delete function.\n");
	return ret;
#endif
	s_QosQueueDel QosQueueDel_Info;
	char *TrafficClasses = NULL;
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};
	IID_INIT(queueIid);

	QueueDelInfoInit(&QosQueueDel_Info);
	getQosQueueDelKey(Jobj, &QosQueueDel_Info);

	if(QosQueueDel_Info.Index == 0){
		printf("Default queue data can not be deleted.\n");
		return ZCFG_INVALID_PARAM_VALUE;
	}	
	
	zcfg_offset_t oid;	
	oid = RDM_OID_QOS_QUE;	
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		queueIid.level = 1;
		queueIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else
	{
	ret = convertIndextoIid(QosQueueDel_Info.Index, &queueIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)		
		return ret;
	}
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {			
			TrafficClasses = (char *)json_object_get_string(json_object_object_get(queueObj, "TrafficClasses"));
#if(QOS_DAL_DEBUG == 1)			
			printf("remove que iid %d TrafficClasses %d %s\n", queueIid.idx[0], TrafficClasses, TrafficClasses);
#endif
			zcfgFeObjJsonDel(RDM_OID_QOS_QUE, &queueIid, NULL);
			if(TrafficClasses != NULL) {
				/*remove the qos classed belong to the removed queue*/
				remove_related_qosClass(TrafficClasses);		
			}
			zcfgFeJsonObjFree(queueObj);
		}
	
	else{			
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Queue fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_QUE);			
		return ZCFG_INTERNAL_ERROR;		
	}
	
	updateQosQueueInfo(e_ActionType_DELETE, QosQueueDel_Info.Index);
	
	return ret;
}

zcfgRet_t zcfgFeDalQosQueue(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalQosQueueAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalQosQueueEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalQosQueueDelete(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalQosQueueGet(Jobj, Jarray, NULL);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}
