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

#define POLICER_MAX_COMMITED_RATE 1000000
#define POLICER_MAX_BURST_SIZE 20000

dal_param_t QosPolicer_param[] = {
    {"Index",                   dalType_int,       0, 0,                           NULL, NULL,                                                       dal_Edit|dal_Delete},
    {"Enable",                  dalType_boolean,   0, 0,                           NULL, NULL,                                                       dal_Add},
    {"Name",                    dalType_string,    0, 0,                           NULL, NULL,                                                       dal_Add},
    {"MeterType",               dalType_string,    0, 0,                           NULL, "SimpleTokenBucket|SingleRateThreeColor|TwoRateThreeColor", dal_Add},
    {"CommittedRate",           dalType_int,       1, POLICER_MAX_COMMITED_RATE,   NULL, NULL,                                                       dal_Add},
    {"CommittedBurstSize",      dalType_int,       1, POLICER_MAX_BURST_SIZE,      NULL, NULL,                                                       dal_Add},
    {"PartialConformingAction", dalType_string,    0, 0,                           NULL, "Pass|Drop|DSCPMark",                                       0},
    {"Partial_value",           dalType_int,       0, 63,                          NULL, NULL,                                                       0},
    {"ExcessBurstSize",         dalType_int,       1, POLICER_MAX_BURST_SIZE,      NULL, NULL,                                                       0},
    {"PeakRate",                dalType_int,       1, POLICER_MAX_COMMITED_RATE,   NULL, NULL,                                                       0},
    {"PeakBurstSize",           dalType_int,       1, POLICER_MAX_BURST_SIZE,      NULL, NULL,                                                       0},
    {"ConformingAction",        dalType_string,    0, 0,                           NULL, "Pass|DSCPMark",                                            dal_Add},
    {"Conform_value",           dalType_int,       0, 63,                          NULL, NULL,                                                       0},
    {"NonConformingAction",     dalType_string,    0, 0,                           NULL, "Drop|DSCPMark",                                            dal_Add},
    {"NonConform_value",        dalType_int,       0, 63,                          NULL, NULL,                                                       0},
    {"Class",                   dalType_ClassList, 0, 0,                           NULL, NULL,                                                       dal_Add},
    {NULL,                      0,                 0, 0,                           NULL, NULL,                                                       0}
};

typedef struct s_QosPolicer{
	int Index;
	bool Enable;
	char *Name;
	char *MeterType;
	int CommittedRate;
	int CommittedBurstSize;
	char *PartialConformingAction;
	int Partial_value;
	/*for MeterType = "SingleRateThreeColor*/
	int ExcessBurstSize;
	
	/*for MeterType = "TwoRateThreeColor"*/
	int PeakRate;
	int PeakBurstSize;

	char *ConformingAction;
	int Conform_value;
	char *NonConformingAction;
	int NonConform_value;
	char *Class;
}s_QosPolicer;

typedef struct s_QosPolicerDel{
	int Index;
}s_QosPolicerDel;

#define MaxClassNumber 20
int ClassArray[MaxClassNumber] = {0};

extern uint32_t StrToIntConv(char *str);

void PolicerInfoInit(s_QosPolicer *QosPolicer_Info){    
	QosPolicer_Info->Index = 0;   
	QosPolicer_Info->Enable = NULL;    
	QosPolicer_Info->Name = NULL;    
	QosPolicer_Info->MeterType = NULL;	
	QosPolicer_Info->CommittedRate = 0;	
	QosPolicer_Info->CommittedBurstSize = 0;	
	QosPolicer_Info->ExcessBurstSize = 0;	
	QosPolicer_Info->PeakRate = 0; 	
	QosPolicer_Info->PeakBurstSize = 0;
	QosPolicer_Info->PartialConformingAction = NULL;
	QosPolicer_Info->Partial_value = 0;
	QosPolicer_Info->ConformingAction = NULL;
	QosPolicer_Info->Conform_value = 0;
	QosPolicer_Info->NonConformingAction = NULL;
	QosPolicer_Info->NonConform_value = 0;
	QosPolicer_Info->Class = NULL;
	
}
void PolicerDelInit(s_QosPolicerDel *QosPolicerDel_Info){    
	QosPolicerDel_Info->Index = 0;   
}

void ClassArrayInit()
{
    int i = 0;

    for (i = 0; i < MaxClassNumber ; i++)
    {
        ClassArray[i] = 0;
    }
}

void getQosPolicerBasicInfo(struct json_object *Jobj, s_QosPolicer *QosPolicer_Info) {
	char *PartialConformingAction = NULL;
	char *ConformingAction = NULL;
	char *NonConformingAction = NULL;
	int partialvalue = 0, conformvalue = 0, nonconformvalue = 0;
	QosPolicer_Info->Index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	if(json_object_object_get(Jobj, "Enable")!= NULL)
		QosPolicer_Info->Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(json_object_object_get(Jobj, "Name")!= NULL)
		QosPolicer_Info->Name = (char*) json_object_get_string(json_object_object_get(Jobj, "Name"));
	if(json_object_object_get(Jobj, "MeterType")!= NULL)
		QosPolicer_Info->MeterType = (char*) json_object_get_string(json_object_object_get(Jobj, "MeterType"));
	if(json_object_object_get(Jobj, "CommittedRate")!= NULL)
		QosPolicer_Info->CommittedRate = (json_object_get_int(json_object_object_get(Jobj, "CommittedRate")))*1000;
	if(json_object_object_get(Jobj, "CommittedBurstSize")!= NULL)
		QosPolicer_Info->CommittedBurstSize = (json_object_get_int(json_object_object_get(Jobj, "CommittedBurstSize")))*1000;
	if(json_object_object_get(Jobj, "ExcessBurstSize")!= NULL)
		QosPolicer_Info->ExcessBurstSize = (json_object_get_int(json_object_object_get(Jobj, "ExcessBurstSize")))*1000;
	if(json_object_object_get(Jobj, "PeakRate")!= NULL)
		QosPolicer_Info->PeakRate = (json_object_get_int(json_object_object_get(Jobj, "PeakRate")))*1000;
	if(json_object_object_get(Jobj, "PeakBurstSize")!= NULL)
		QosPolicer_Info->PeakBurstSize = (json_object_get_int(json_object_object_get(Jobj, "PeakBurstSize")))*1000;
	if(json_object_object_get(Jobj, "PartialConformingAction")!= NULL){
		PartialConformingAction = (char*)json_object_get_string(json_object_object_get(Jobj, "PartialConformingAction"));
		if(!strcmp(PartialConformingAction,"DSCPMark")){
			partialvalue = json_object_get_int(json_object_object_get(Jobj, "Partial_value"));
			strcpy(PartialConformingAction, "");
			sprintf(PartialConformingAction, "%d", partialvalue);
		}
		QosPolicer_Info->PartialConformingAction = PartialConformingAction;
	}
	if(json_object_object_get(Jobj, "ConformingAction")!= NULL){
		ConformingAction = (char*)json_object_get_string(json_object_object_get(Jobj, "ConformingAction"));
		if(!strcmp(ConformingAction,"DSCPMark")){
			conformvalue = json_object_get_int(json_object_object_get(Jobj, "Conform_value"));
			strcpy(ConformingAction, "");
			sprintf(ConformingAction, "%d", conformvalue);
		}
		QosPolicer_Info->ConformingAction = ConformingAction;
	}
	
	if(json_object_object_get(Jobj, "NonConformingAction")!= NULL){
		NonConformingAction = (char*)json_object_get_string(json_object_object_get(Jobj, "NonConformingAction"));
		if(!strcmp(NonConformingAction,"DSCPMark")){
			nonconformvalue = json_object_get_int(json_object_object_get(Jobj, "NonConform_value"));
			strcpy(NonConformingAction, "");
			sprintf(NonConformingAction, "%d", nonconformvalue);
		}
		QosPolicer_Info->NonConformingAction = NonConformingAction;
	}
	if(json_object_object_get(Jobj, "Class")!= NULL)
		QosPolicer_Info->Class = (char *)json_object_get_string(json_object_object_get(Jobj, "Class"));
	return;
}

void getQosPolicerDelInfo(struct json_object *Jobj, s_QosPolicerDel *QosPolicerDel_Info) {
	QosPolicerDel_Info->Index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	return;
}

void prepareQosPolicerObj(struct json_object *Jobj,s_QosPolicer *QosPolicer_Info, json_object *policerObj) {

	if(json_object_object_get(Jobj, "Enable") != NULL)
		json_object_object_add(policerObj, "Enable", json_object_new_boolean(QosPolicer_Info->Enable));
	if(json_object_object_get(Jobj, "Name") != NULL)
		json_object_object_add(policerObj, "X_ZYXEL_Name", json_object_new_string(QosPolicer_Info->Name));
	if(json_object_object_get(Jobj, "MeterType") != NULL)
		json_object_object_add(policerObj, "MeterType", json_object_new_string(QosPolicer_Info->MeterType));
	if(json_object_object_get(Jobj, "CommittedRate") != NULL)
		json_object_object_add(policerObj, "CommittedRate", json_object_new_int(QosPolicer_Info->CommittedRate));
	if(json_object_object_get(Jobj, "CommittedBurstSize") != NULL)
		json_object_object_add(policerObj, "CommittedBurstSize", json_object_new_int(QosPolicer_Info->CommittedBurstSize));
	if(json_object_object_get(Jobj, "PartialConformingAction") != NULL)
		json_object_object_add(policerObj, "PartialConformingAction", json_object_new_string(QosPolicer_Info->PartialConformingAction));
	if(json_object_object_get(Jobj, "ExcessBurstSize") != NULL)
		json_object_object_add(policerObj, "ExcessBurstSize", json_object_new_int(QosPolicer_Info->ExcessBurstSize));
	if(json_object_object_get(Jobj, "PeakRate") != NULL)
		json_object_object_add(policerObj, "PeakRate", json_object_new_int(QosPolicer_Info->PeakRate));
	if(json_object_object_get(Jobj, "PeakBurstSize") != NULL)
		json_object_object_add(policerObj, "PeakBurstSize", json_object_new_int(QosPolicer_Info->PeakBurstSize));
	if(json_object_object_get(Jobj, "ConformingAction") != NULL)
		json_object_object_add(policerObj, "ConformingAction", json_object_new_string(QosPolicer_Info->ConformingAction));
	if(json_object_object_get(Jobj, "NonConformingAction") != NULL)
		json_object_object_add(policerObj, "NonConformingAction", json_object_new_string(QosPolicer_Info->NonConformingAction));
	return;
}

void checkOriginClass(char *policer_name, char *target){
	char *Class_Policer = NULL;
	char * Class_Name = NULL;
	int i = 1;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};
	IID_INIT(classIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
		Class_Policer = (char*)json_object_get_string(json_object_object_get(classObj, "Policer"));
		Class_Name = (char*)json_object_get_string(json_object_object_get(classObj, "X_ZYXEL_Name"));
		if(!strcmp(Class_Name,target) && !strcmp(Class_Policer, policer_name)){
			ClassArray[i] = 1;	
		}

		if(!strcmp(Class_Name,target) && !strcmp(Class_Policer, "")){
			ClassArray[i] = 2;	
		}
		zcfgFeJsonObjFree(classObj);
		i++;
	}
	return;
}

void updateQosClass(e_ActionType action, char *policer_name, char *target){
	int i = 1;
	char *Class_Policer = NULL;
	char * Class_Name = NULL;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};
	IID_INIT(classIid);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
		Class_Policer = (char*)json_object_get_string(json_object_object_get(classObj, "Policer"));
		Class_Name = (char*)json_object_get_string(json_object_object_get(classObj, "X_ZYXEL_Name"));
		if((action == e_ActionType_ADD) && !strcmp(Class_Name,target) && (strstr(Class_Policer, "Policer")== NULL)) {
			json_object_object_add(classObj, "Policer", json_object_new_string(policer_name));
			zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
			printf("set Policer %s\n", policer_name);
		}
		else if(action == e_ActionType_EDIT) {
			if(ClassArray[i] == 1 && !strcmp(Class_Policer, policer_name)){
				//not to change
				json_object_object_add(classObj, "Policer", json_object_new_string(policer_name));
				zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
			}
			if(ClassArray[i] == 2 && !strcmp(Class_Policer, ""))  {
				//new class
				json_object_object_add(classObj, "Policer", json_object_new_string(policer_name));
				zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
			}
			if(ClassArray[i] == 0 && !strcmp(Class_Policer, policer_name)){
				//old class => remove
				json_object_object_add(classObj, "Policer", json_object_new_string(""));
			zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
		}
		}
		else if(action == e_ActionType_DELETE && !strcmp(Class_Policer, policer_name)) {
			json_object_object_add(classObj, "Policer", json_object_new_string(""));
			zcfgFeObjJsonSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
		}
		zcfgFeJsonObjFree(classObj);
		i++;
	}
	return;
}

void zcfgFeDalShowQosPolicer(struct json_object *Jarray){	
	int i, len = 0;	
	const char *MeterType = NULL;
	const char *conform = NULL;
	const char *nonconform = NULL;
	const char *partial = NULL;
	bool showIndex = false;
	struct json_object *obj, *obj_idx = NULL;		
	if(json_object_get_type(Jarray) != json_type_array){				
		printf("wrong Jobj format!\n");				
	return;
}

	len = json_object_array_length(Jarray);
	obj_idx = json_object_array_get_idx(Jarray, 0);
	showIndex = json_object_get_boolean(json_object_object_get(obj_idx, "ShowIndex"));

	if(len == 1 && showIndex){
		obj = json_object_array_get_idx(Jarray, 0);
		MeterType = json_object_get_string(json_object_object_get(obj, "MeterType"));
		conform = json_object_get_string(json_object_object_get(obj, "ConformingAction"));
		nonconform = json_object_get_string(json_object_object_get(obj, "NonConformingAction"));
		partial = json_object_get_string(json_object_object_get(obj, "PartialConformingAction"));
		printf("Qos Policer Setup\n");
		printf("%-50s %s \n","Index :", json_object_get_string(json_object_object_get(obj, "Index")));
		printf("%-50s %d \n","Enable :", json_object_get_boolean(json_object_object_get(obj, "Enable")));
		printf("%-50s %s \n","Name :", json_object_get_string(json_object_object_get(obj, "Name")));
		printf("%-50s %s \n","Class :", json_object_get_string(json_object_object_get(obj, "Class")));
		printf("%-50s %s \n","Meter Type :", json_object_get_string(json_object_object_get(obj, "MeterType")));
		printf("%-50s %s Kbps \n","Committed Rate :", json_object_get_string(json_object_object_get(obj, "CommittedRate")));
		printf("%-50s %s Kbytes \n","Committed Burst Size :", json_object_get_string(json_object_object_get(obj, "CommittedBurstSize")));
		if(!strcmp(MeterType,"SingleRateThreeColor"))
			printf("%-50s %s Kbytes \n","Excess Burst Size :",json_object_get_string(json_object_object_get(obj, "ExcessBurstSize")));
		if(!strcmp(MeterType,"TwoRateThreeColor")){				
			printf("%-50s %s Kbps \n","Peak Rate :",json_object_get_string(json_object_object_get(obj, "PeakRate")));				
			printf("%-50s %s Kbytes \n","Peak Burst Size :",json_object_get_string(json_object_object_get(obj, "PeakBurstSize")));			
		}
		if(!strcmp(conform,"Pass"))				
			printf("%-50s %s \n","Conforming Action :",json_object_get_string(json_object_object_get(obj, "ConformingAction")));			
		else				
			printf("%-50s DSCP Mark %s \n","Conforming Action :",json_object_get_string(json_object_object_get(obj, "ConformingAction")));
		if(!strcmp(nonconform,"Drop"))				
			printf("%-50s %s \n","Non-Conforming Action :",json_object_get_string(json_object_object_get(obj, "NonConformingAction")));	
		else				
			printf("%-50s DSCP Mark %s \n","Non-Conforming Action :",json_object_get_string(json_object_object_get(obj, "NonConformingAction")));
		if(!strcmp(MeterType,"SingleRateThreeColor")||!strcmp(MeterType,"TwoRateThreeColor")){				
			if(!strcmp(partial,"Pass")||!strcmp(partial,"Drop"))					
				printf("%-50s %s \n","Partial Conforming Action :",json_object_get_string(json_object_object_get(obj, "PartialConformingAction")));				
			else					
				printf("%-50s DSCP Mark %s \n","Partial Conforming Action :",json_object_get_string(json_object_object_get(obj, "PartialConformingAction")));			
		}
	}

	else{
		printf("%-6s %-7s %-15s %-30s %-5s \n","Index", "Enable", "Name", "Classes", "Meter Type");	
		for(i=0;i<len;i++){	
			obj = json_object_array_get_idx(Jarray, i);				
			printf("%-6s %-7s %-15s %-30s %-5s \n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "Name")),
			json_object_get_string(json_object_object_get(obj, "Class")),
			json_object_get_string(json_object_object_get(obj, "MeterType")));
		}
	}

}

zcfgRet_t zcfgFeDalQosPolicerGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){	
	zcfgRet_t ret = ZCFG_SUCCESS;	
	struct json_object *pramJobj = NULL;	
	struct json_object *policerObj = NULL;
	objIndex_t policerIid = {0};
	char number[30] = {0};
	int count = 1,index = 0,commitrate,commitburst,peakrate,peakburst,excess;
	
	IID_INIT(policerIid);
	if(json_object_object_get(Jobj, "Index")){
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &policerIid, RDM_OID_QOS_POLICER, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_POLICER, &policerIid , &policerObj) == ZCFG_SUCCESS){
			sprintf(number, "%d", policerIid.idx[0]);
			pramJobj = json_object_new_object();
			commitrate = (json_object_get_int(json_object_object_get(policerObj, "CommittedRate")))/1000;
			commitburst = (json_object_get_int(json_object_object_get(policerObj, "CommittedBurstSize")))/1000;
			peakrate = (json_object_get_int(json_object_object_get(policerObj, "PeakRate")))/1000;
			peakburst = (json_object_get_int(json_object_object_get(policerObj, "PeakBurstSize")))/1000;
			excess = (json_object_get_int(json_object_object_get(policerObj, "ExcessBurstSize")))/1000;
			json_object_object_add(pramJobj, "ShowIndex", json_object_new_boolean(true));
			json_object_object_add(pramJobj, "Index", json_object_new_string(number));	
			json_object_object_add(pramJobj, "Enable",  JSON_OBJ_COPY(json_object_object_get(policerObj, "Enable")));
			json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(policerObj, "X_ZYXEL_Name")));
			json_object_object_add(pramJobj, "Class", json_object_new_string(number));			
			json_object_object_add(pramJobj, "idx", json_object_new_int(policerIid.idx[0]));
			json_object_object_add(pramJobj, "MeterType",JSON_OBJ_COPY(json_object_object_get(policerObj, "MeterType")));	
			json_object_object_add(pramJobj, "CommittedRate",  json_object_new_int(commitrate));
			json_object_object_add(pramJobj, "CommittedBurstSize",  json_object_new_int(commitburst));	
			json_object_object_add(pramJobj, "ExcessBurstSize",  json_object_new_int(excess));	
			json_object_object_add(pramJobj, "PeakRate",  json_object_new_int(peakrate));	
			json_object_object_add(pramJobj, "PeakBurstSize",  json_object_new_int(peakburst));	
			json_object_object_add(pramJobj, "ConformingAction", JSON_OBJ_COPY(json_object_object_get(policerObj, "ConformingAction")));	
			json_object_object_add(pramJobj, "NonConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "NonConformingAction")));	
			json_object_object_add(pramJobj, "PartialConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "PartialConformingAction")));	
			json_object_array_add(Jarray, pramJobj);						
			zcfgFeJsonObjFree(policerObj);	
		}
	}
	else if(json_object_object_get(Jobj, "idx")){
		policerIid.level = 1;
		policerIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_POLICER, &policerIid , &policerObj) == ZCFG_SUCCESS){
			sprintf(number, "%d", policerIid.idx[0]);
			pramJobj = json_object_new_object();
			commitrate = (json_object_get_int(json_object_object_get(policerObj, "CommittedRate")))/1000;
			commitburst = (json_object_get_int(json_object_object_get(policerObj, "CommittedBurstSize")))/1000;
			peakrate = (json_object_get_int(json_object_object_get(policerObj, "PeakRate")))/1000;
			peakburst = (json_object_get_int(json_object_object_get(policerObj, "PeakBurstSize")))/1000;
			excess = (json_object_get_int(json_object_object_get(policerObj, "ExcessBurstSize")))/1000;
			json_object_object_add(pramJobj, "ShowIndex", json_object_new_boolean(true));
			json_object_object_add(pramJobj, "Index", json_object_new_string(number));	
			json_object_object_add(pramJobj, "Enable",  JSON_OBJ_COPY(json_object_object_get(policerObj, "Enable")));
			json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(policerObj, "X_ZYXEL_Name")));
			json_object_object_add(pramJobj, "Class", json_object_new_string(number));
			json_object_object_add(pramJobj, "idx", json_object_new_int(policerIid.idx[0]));
			json_object_object_add(pramJobj, "MeterType",JSON_OBJ_COPY(json_object_object_get(policerObj, "MeterType")));	
			json_object_object_add(pramJobj, "CommittedRate",  json_object_new_int(commitrate));
			json_object_object_add(pramJobj, "CommittedBurstSize",  json_object_new_int(commitburst));	
			json_object_object_add(pramJobj, "ExcessBurstSize",  json_object_new_int(excess));	
			json_object_object_add(pramJobj, "PeakRate",  json_object_new_int(peakrate));	
			json_object_object_add(pramJobj, "PeakBurstSize",  json_object_new_int(peakburst));	
			json_object_object_add(pramJobj, "ConformingAction", JSON_OBJ_COPY(json_object_object_get(policerObj, "ConformingAction")));	
			json_object_object_add(pramJobj, "NonConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "NonConformingAction")));	
			json_object_object_add(pramJobj, "PartialConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "PartialConformingAction")));	
			json_object_array_add(Jarray, pramJobj);						
			zcfgFeJsonObjFree(policerObj);	
		}
	}
	else{
		while(zcfgFeObjJsonGetNext(RDM_OID_QOS_POLICER, &policerIid , &policerObj) == ZCFG_SUCCESS){
			sprintf(number, "%d", policerIid.idx[0]);
			pramJobj = json_object_new_object();
			commitrate = (json_object_get_int(json_object_object_get(policerObj, "CommittedRate")))/1000;
			commitburst = (json_object_get_int(json_object_object_get(policerObj, "CommittedBurstSize")))/1000;
			peakrate = (json_object_get_int(json_object_object_get(policerObj, "PeakRate")))/1000;
			peakburst = (json_object_get_int(json_object_object_get(policerObj, "PeakBurstSize")))/1000;
			excess = (json_object_get_int(json_object_object_get(policerObj, "ExcessBurstSize")))/1000;
			json_object_object_add(pramJobj, "Index", json_object_new_int(count));	
			json_object_object_add(pramJobj, "Enable",  JSON_OBJ_COPY(json_object_object_get(policerObj, "Enable")));
			json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(policerObj, "X_ZYXEL_Name")));
			json_object_object_add(pramJobj, "Class", json_object_new_string(number));			
			json_object_object_add(pramJobj, "idx", json_object_new_int(policerIid.idx[0]));
			json_object_object_add(pramJobj, "MeterType",JSON_OBJ_COPY(json_object_object_get(policerObj, "MeterType")));	
			json_object_object_add(pramJobj, "CommittedRate",  json_object_new_int(commitrate));
			json_object_object_add(pramJobj, "CommittedBurstSize",  json_object_new_int(commitburst));	
			json_object_object_add(pramJobj, "ExcessBurstSize",  json_object_new_int(excess));	
			json_object_object_add(pramJobj, "PeakRate",  json_object_new_int(peakrate));	
			json_object_object_add(pramJobj, "PeakBurstSize",  json_object_new_int(peakburst));	
			json_object_object_add(pramJobj, "ConformingAction", JSON_OBJ_COPY(json_object_object_get(policerObj, "ConformingAction")));	
			json_object_object_add(pramJobj, "NonConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "NonConformingAction")));	
			json_object_object_add(pramJobj, "PartialConformingAction",  JSON_OBJ_COPY(json_object_object_get(policerObj, "PartialConformingAction")));	
			json_object_array_add(Jarray, pramJobj);		
			count++;				
			zcfgFeJsonObjFree(policerObj);	
		}
	}
	return ret;
}

zcfgRet_t zcfgFeDalQosPolicerAdd(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	s_QosPolicer QosPolicer_Info;
	char *token = NULL;
	char policer_name[30] = {0};
	json_object *policerObj = NULL;
	objIndex_t policerIid = {0};
	IID_INIT(policerIid);
	PolicerInfoInit(&QosPolicer_Info);
	getQosPolicerBasicInfo(Jobj, &QosPolicer_Info);
    ClassArrayInit();
	zcfg_offset_t oid;	
	oid = RDM_OID_QOS_POLICER;	
	if(QosPolicer_Info.Name != NULL){
		ret = dalcmdParamCheck(NULL, QosPolicer_Info.Name , oid, "X_ZYXEL_Name", "Name",  replyMsg);		
		if(ret != ZCFG_SUCCESS)			
			return ret;
	}
	if(!strcmp(QosPolicer_Info.MeterType,"TwoRateThreeColor")){
		if(QosPolicer_Info.PeakRate < QosPolicer_Info.CommittedRate){
			printf("PeakRate need to equal or greater than CommittedRate.\n");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	ret = zcfgFeObjJsonAdd(RDM_OID_QOS_POLICER, &policerIid, NULL);	
	if(ret == ZCFG_SUCCESS) {
		ret = zcfgFeObjJsonGet(RDM_OID_QOS_POLICER, &policerIid, &policerObj);	
	}
	if(ret == ZCFG_SUCCESS) {
		prepareQosPolicerObj(Jobj,&QosPolicer_Info, policerObj);
		zcfgFeObjJsonSet(RDM_OID_QOS_POLICER, &policerIid, policerObj, NULL);

		sprintf(policer_name, "Policer.%d", policerIid.idx[0]);
		token = strtok_r(QosPolicer_Info.Class, ",", &QosPolicer_Info.Class);
		while(token != NULL){
			checkOriginClass(policer_name, token);
			updateQosClass(e_ActionType_ADD, policer_name, token); //PUT QOS CLASS
			token = strtok_r(QosPolicer_Info.Class, ",", &QosPolicer_Info.Class);
		}
		//updatePolicerClassInfo(); // update qosPolicerClassTable
		zcfgFeJsonObjFree(policerObj);	
		return ret;
	}		
	else{		
		printf("%s: Object retrieve fail\n", __FUNCTION__);		
		return ret;
	}
}

zcfgRet_t zcfgFeDalQosPolicerEdit(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	char policer_name[30] = {0};
	s_QosPolicer QosPolicer_Info;
	char *token = NULL,*partial = NULL;
	json_object *policerObj = NULL;
	objIndex_t policerIid = {0};
	IID_INIT(policerIid);
	int excessBurstSize = 0, peakrate = 0, peakburstsize = 0, commitrate = 0;

	PolicerInfoInit(&QosPolicer_Info);
	getQosPolicerBasicInfo(Jobj, &QosPolicer_Info);
    ClassArrayInit();

	zcfg_offset_t oid;		
	oid = RDM_OID_QOS_POLICER;		
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		policerIid.level = 1;
		policerIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else 
	{
		ret = convertIndextoIid(QosPolicer_Info.Index, &policerIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)				
			return ret;	
	}
	if(QosPolicer_Info.Name != NULL){		
		ret = dalcmdParamCheck(&policerIid, QosPolicer_Info.Name , oid, "X_ZYXEL_Name", "Name",  replyMsg);		
		if(ret != ZCFG_SUCCESS)			
			return ret;	
	}

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_POLICER, &policerIid, &policerObj) == ZCFG_SUCCESS) {
		if(json_object_object_get(Jobj, "MeterType") != NULL){
			//Type is SingleRateThreeColor, need to ckeck ExcessBurstSizz and PartialConformingAction 
			if(!strcmp(QosPolicer_Info.MeterType,"SingleRateThreeColor")){
				excessBurstSize = json_object_get_int(json_object_object_get(policerObj, "ExcessBurstSize"));
				partial = (char *)json_object_get_string(json_object_object_get(policerObj, "PartialConformingAction"));
				if(excessBurstSize == 0 && json_object_object_get(Jobj, "ExcessBurstSize") == NULL){
					printf("Need to enter ExcessBurstSize parameter.\n");
					zcfgFeJsonObjFree(policerObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				
				if(!strcmp(partial,"") && json_object_object_get(Jobj, "PartialConformingAction") == NULL){
					printf("Need to enter PartialConformingAction parameter.\n");
					zcfgFeJsonObjFree(policerObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				json_object_object_add(policerObj, "PeakRate", json_object_new_int(0));
				json_object_object_add(policerObj, "PeakBurstSize", json_object_new_int(0));
			}

			//Type is TwoRateThreeColor, need to ckeck PeakRate ,PeakBurstSize ,PartialConformingAction and CommittedRate.  
			if(!strcmp(QosPolicer_Info.MeterType,"TwoRateThreeColor")){
				peakrate = json_object_get_int(json_object_object_get(policerObj, "PeakRate"));
				peakburstsize = json_object_get_int(json_object_object_get(policerObj, "PeakBurstSize"));
				partial = (char *)json_object_get_string(json_object_object_get(policerObj, "PartialConformingAction"));
				commitrate = json_object_get_int(json_object_object_get(policerObj, "CommittedRate"));
				if(peakrate == 0 && json_object_object_get(Jobj, "PeakRate") == NULL){
					printf("Need to enter PeakRate parameter.\n");
					zcfgFeJsonObjFree(policerObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				else{
					if(json_object_object_get(Jobj, "PeakRate") == NULL){
						if(json_object_object_get(policerObj, "CommittedRate") != NULL){
							if( peakrate < QosPolicer_Info.CommittedRate){
								printf("PeakRate need to equal or greater than CommittedRate.\n");
								zcfgFeJsonObjFree(policerObj);
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else{
							if( peakrate < commitrate){
								printf("PeakRate need to equal or greater than CommittedRate.\n");
								zcfgFeJsonObjFree(policerObj);
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
							
					}
					else{
						if(json_object_object_get(policerObj, "CommittedRate") != NULL){
							if(QosPolicer_Info.PeakRate < QosPolicer_Info.CommittedRate){
								printf("PeakRate need to equal or greater than CommittedRate.\n");
								zcfgFeJsonObjFree(policerObj);
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
						else{
							if(QosPolicer_Info.PeakRate < commitrate){
								printf("PeakRate need to equal or greater than CommittedRate.\n");
								zcfgFeJsonObjFree(policerObj);
								return ZCFG_INVALID_PARAM_VALUE;
							}
						}
					}
						
				}
				
				if(peakburstsize == 0 && json_object_object_get(Jobj, "PeakBurstSize") == NULL){
					printf("Need to enter PeakBurstSize parameter.\n");
					zcfgFeJsonObjFree(policerObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				if(!strcmp(partial,"") && json_object_object_get(Jobj, "PartialConformingAction") == NULL){
					printf("Need to enter PartialConformingAction parameter.\n");
					zcfgFeJsonObjFree(policerObj);
					return ZCFG_INVALID_PARAM_VALUE;
				}
				json_object_object_add(policerObj, "ExcessBurstSize", json_object_new_int(0));
			}
			//Type is SimpleTokenBucket, need to ckeck PeakRate ,PeakBurstSize ,PartialConformingAction,ExcessBurstSize. 
			if(!strcmp(QosPolicer_Info.MeterType,"SimpleTokenBucket")){
				json_object_object_add(policerObj, "ExcessBurstSize", json_object_new_int(0));
				json_object_object_add(policerObj, "PeakRate", json_object_new_int(0));
				json_object_object_add(policerObj, "PeakBurstSize", json_object_new_int(0));
				json_object_object_add(policerObj, "PartialConformingAction", json_object_new_string(""));
			}
		}

		prepareQosPolicerObj(Jobj,&QosPolicer_Info, policerObj);
		zcfgFeObjJsonSet(RDM_OID_QOS_POLICER, &policerIid, policerObj, NULL);
		if(QosPolicer_Info.Class != NULL){
			sprintf(policer_name, "Policer.%d", policerIid.idx[0]);
			token = strtok_r(QosPolicer_Info.Class, ",", &QosPolicer_Info.Class);
			while(token != NULL){
				checkOriginClass(policer_name, token);
				updateQosClass(e_ActionType_EDIT, policer_name, token); //PUT QOS CLASS
				token = strtok_r(QosPolicer_Info.Class, ",", &QosPolicer_Info.Class);
			}
		}
		//updatePolicerClassInfo(); // update qosPolicerClassTable
		zcfgFeJsonObjFree(policerObj);
		return ret;
	}

	else{				
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Policer fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_POLICER);
		return ZCFG_INTERNAL_ERROR;		
	}
}

zcfgRet_t zcfgFeDalQosPolicerDelete(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	char policer_name[30] = {0};
	s_QosPolicerDel QosPolicerDel_Info;
	json_object *policerObj = NULL;
	objIndex_t policerIid = {0};
	IID_INIT(policerIid);

#if(QOS_DAL_DEBUG == 1)			
	printf("zcfgFeDalQosPolicerDelete\n");
#endif	
	PolicerDelInit(&QosPolicerDel_Info);
	getQosPolicerDelInfo(Jobj, &QosPolicerDel_Info);
	zcfg_offset_t oid;		
	oid = RDM_OID_QOS_POLICER;		
	if ((Jobj != NULL) && json_object_object_get(Jobj, "idx") != NULL)
	{
		policerIid.level = 1;
		policerIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "idx"));
	}
	else 
	{
		ret = convertIndextoIid(QosPolicerDel_Info.Index, &policerIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)				
			return ret;	
	}
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_POLICER, &policerIid, &policerObj) == ZCFG_SUCCESS) {	
		zcfgFeObjJsonDel(RDM_OID_QOS_POLICER, &policerIid, NULL);
		sprintf(policer_name, "Policer.%d", policerIid.idx[0]);
		updateQosClass(e_ActionType_DELETE, policer_name, NULL); //PUT QOS CLASS
		//updatePolicerClassInfo(); // update qosPolicerClassTable
		zcfgFeJsonObjFree(policerObj);
		return ret;
	}
	else{					
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Policer fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_POLICER);					
		return ZCFG_INTERNAL_ERROR;			
	}
}

zcfgRet_t zcfgFeDalQosPolicer(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalQosPolicerAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalQosPolicerEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalQosPolicerDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {		
		ret = zcfgFeDalQosPolicerGet(Jobj, Jarray, NULL);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}
