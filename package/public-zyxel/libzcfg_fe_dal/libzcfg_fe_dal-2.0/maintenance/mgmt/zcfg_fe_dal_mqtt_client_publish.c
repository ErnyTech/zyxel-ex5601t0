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


dal_param_t DEV_MQTT_CLIENT_PUBLISH_param[]={
	{"Index", 					dalType_int, 	0, 	0,	 	NULL,	NULL,	dal_Edit | dal_Delete},	
	{"Parent_Index", 			dalType_int, 	0, 	0,	 	NULL,	NULL,	dal_Add | dal_Edit | dal_Delete},	
	{"Topic", 			dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{"Interval", 		dalType_int,	0,	0,		NULL,	NULL,	NULL},
	{"Script", 			dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{"Output", 			dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{NULL,						0,				0,	0,		NULL,	NULL,	NULL}
};

struct json_object *mqttClientPublishObj;

objIndex_t mqttClientPublishIid = {0};

int g_parent_index;
const char* g_Topic;
int g_Interval;
const char* g_Script;
const char* g_Output;

void initMqttClientPublishEntryGlobalObjects(){
	mqttClientPublishObj = NULL;
	return;
}

void freeAllmqttClientPublishObjects(){
	if(mqttClientPublishObj) {
		json_object_put(mqttClientPublishObj);
		mqttClientPublishObj = NULL;
	}

	return;
}

void getMqttClientPublishBasicInfo(struct json_object *Jobj){
	g_parent_index = json_object_get_int(json_object_object_get(Jobj, "Parent_Index"));
	printf("g_parent_index=%d\n", g_parent_index);
	
	g_Topic = json_object_get_string(json_object_object_get(Jobj, "Topic"));
	if(g_Topic)
		printf("g_Topic=%s\n", g_Topic);

	g_Interval = json_object_get_int(json_object_object_get(Jobj, "Interval"));
	printf("g_Interval=%d\n", g_Interval);
	
	g_Script = json_object_get_string(json_object_object_get(Jobj, "Script"));
	if(g_Script)
		printf("g_Script=%s\n", g_Script);

	g_Output = json_object_get_string(json_object_object_get(Jobj, "Output"));
	if(g_Output)
		printf("g_Output=%s\n", g_Output);
	
	return;
}

void zcfgFeDalShowMQTTClientPublish(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-12s %-6s %-12s %-15s %-13s\n",
			"Parent_Index", "Index", "Topic", "Interval", "Output");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
		printf("%-12s %-6s %-12s %-15s %-13s\n",
			json_object_get_string(json_object_object_get(obj, "Parent_Index")),
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Topic")),
			json_object_get_string(json_object_object_get(obj, "Interval")),
			json_object_get_string(json_object_object_get(obj, "Output")));
	}
}

zcfgRet_t zcfgFeDalmqttClientPublishObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MQTT_CLIENT_PUB_CONF, &mqttClientPublishIid, &mqttClientPublishObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get MQTT Client Publish fail oid(%d)\n ",__FUNCTION__,RDM_OID_MQTT_CLIENT_PUB_CONF);
		return ZCFG_INTERNAL_ERROR;
	}

	if(mqttClientPublishObj != NULL){
		if(json_object_object_get(Jobj, "Topic"))
			json_object_object_add(mqttClientPublishObj, "Topic", json_object_new_string(g_Topic));
		if(json_object_object_get(Jobj, "Interval"))
			json_object_object_add(mqttClientPublishObj, "Interval", json_object_new_int(g_Interval));
		if(json_object_object_get(Jobj, "Script"))
			json_object_object_add(mqttClientPublishObj, "Script", json_object_new_string(g_Script));
		if(json_object_object_get(Jobj, "Output"))
			json_object_object_add(mqttClientPublishObj, "Output", json_object_new_string(g_Output));
	}
	
	return ret;
}

zcfgRet_t zcfgFeDalMQTTClientPublishEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0, parent_index = 0;
	IID_INIT(mqttClientPublishIid);


	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	parent_index = json_object_get_int(json_object_object_get(Jobj, "Parent_Index"));

	initMqttClientPublishEntryGlobalObjects();

	getMqttClientPublishBasicInfo(Jobj);

	//Get the actual index in level 1
	mqttClientPublishIid.level = 1;
	ret = convertIndextoIid(parent_index, &mqttClientPublishIid, RDM_OID_MQTT_CLIENT, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	//Get the actual index in level 2
	mqttClientPublishIid.level = 2;
	ret = convertIndextoIid(index, &mqttClientPublishIid, RDM_OID_MQTT_CLIENT_PUB_CONF, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeDalmqttClientPublishObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit MQTT Entry \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_MQTT_CLIENT_PUB_CONF, &mqttClientPublishIid, mqttClientPublishObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d RDM_OID_MQTT_CLIENT_PUBLISH\n", __FUNCTION__, RDM_OID_MQTT_CLIENT_PUB_CONF);
	}

exit:
	freeAllmqttClientPublishObjects();
	return ret;
}

zcfgRet_t zcfgFeDalMQTTClientPublishGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *MqttClientPublishJobj = NULL;
	struct json_object *MqttClientJobj = NULL;
	objIndex_t child_objIid = {0}, parent_objIid = {0};
	int parent_index = 0, index = 0, current_parent_index = 0, i = 0;
	int parent_exist[256] = {0};

	IID_INIT(parent_objIid);
	//check if the parent node exists
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MQTT_CLIENT, &parent_objIid, &MqttClientJobj) == ZCFG_SUCCESS){
		parent_exist[parent_objIid.idx[0] - 1] = 1;
		zcfgFeJsonObjFree(MqttClientJobj);
	}

	IID_INIT(child_objIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MQTT_CLIENT_PUB_CONF, &child_objIid, &MqttClientPublishJobj) == ZCFG_SUCCESS){

		//The index displayed does not contain deleted nodes.
		//Get the correct parent_index to be displayed.
		if(current_parent_index < child_objIid.idx[0]){
			for(i = current_parent_index; i < child_objIid.idx[0];i++){
				if(parent_exist[i] != 0){
					parent_index++;
				}
			}
			index = 1;
			current_parent_index = child_objIid.idx[0];
		}
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Parent_Index", json_object_new_int(parent_index));
		json_object_object_add(paramJobj, "Index", json_object_new_int(index++));
		json_object_object_add(paramJobj, "Topic", JSON_OBJ_COPY(json_object_object_get(MqttClientPublishJobj, "Topic")));
		json_object_object_add(paramJobj, "Interval", JSON_OBJ_COPY(json_object_object_get(MqttClientPublishJobj, "Interval")));
		json_object_object_add(paramJobj, "Script", JSON_OBJ_COPY(json_object_object_get(MqttClientPublishJobj, "Script")));
		json_object_object_add(paramJobj, "Output", JSON_OBJ_COPY(json_object_object_get(MqttClientPublishJobj, "Output")));
		json_object_array_add(Jarray, paramJobj);
	
		zcfgFeJsonObjFree(MqttClientPublishJobj);
	}
	
	return ret;
}


zcfgRet_t zcfgFeDalMQTTClientPublishAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	initMqttClientPublishEntryGlobalObjects();
	getMqttClientPublishBasicInfo(Jobj);

	IID_INIT(mqttClientPublishIid);

	//MQTT.Client.i.Publish_Object.i
	mqttClientPublishIid.level = 1;
	mqttClientPublishIid.idx[0] = g_parent_index;
	
	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_MQTT_CLIENT_PUB_CONF, &mqttClientPublishIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add MQTT Entry \n ",__FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalmqttClientPublishObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit MQTT Entry \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_MQTT_CLIENT_PUB_CONF, &mqttClientPublishIid, mqttClientPublishObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d MQTT Entry\n", __FUNCTION__, RDM_OID_MQTT_CLIENT_PUB_CONF);
	}

exit:
	freeAllmqttClientPublishObjects();
	return ret;
}

zcfgRet_t zcfgFeDalMQTTClientPublishDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0, parent_index = 0;
	IID_INIT(mqttClientPublishIid);

	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	parent_index = json_object_get_int(json_object_object_get(Jobj, "Parent_Index"));

	//Get the actual index in level 1
	mqttClientPublishIid.level = 1;
	ret = convertIndextoIid(parent_index, &mqttClientPublishIid, RDM_OID_MQTT_CLIENT, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	//Get the actual index in level 2
	mqttClientPublishIid.level = 2;
	ret = convertIndextoIid(index, &mqttClientPublishIid, RDM_OID_MQTT_CLIENT_PUB_CONF, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonDel(RDM_OID_MQTT_CLIENT_PUB_CONF, &mqttClientPublishIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete mqtt obj fail \n", __FUNCTION__);
		goto exit;
	}

exit:
	freeAllmqttClientPublishObjects();
	return ret;
	
}

zcfgRet_t zcfgFeDalMQTTClientPublish(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDalMQTTClientPublishEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalMQTTClientPublishAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalMQTTClientPublishDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalMQTTClientPublishGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
