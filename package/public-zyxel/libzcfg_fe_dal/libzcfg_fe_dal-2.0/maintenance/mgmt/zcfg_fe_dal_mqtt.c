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

dal_param_t DEV_MQTT_NODE_param[] = {
        {"X_ZYXEL_BoundIfList",                 dalType_string, 0,      0,      NULL,   NULL,   NULL},
	{NULL,	0,	0,	0,	NULL,	NULL,	NULL}
};


dal_param_t DEV_MQTT_CLIENT_param[]={
	{"Index", 					dalType_int, 	0, 	0,	 	NULL,	NULL,	dal_Edit | dal_Delete},	
	{"Enable", 					dalType_boolean,0,	0,		NULL,	NULL,	dal_Add},
	{"BrokerAddress", 			dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{"BrokerPort", 				dalType_int,	0,	0,		NULL,	NULL,	NULL},
	{"X_ZYXEL_QoS", 			dalType_int,	0,	0,		NULL,	NULL,	NULL},
	//{"WillEnable", 			dalType_int,	0,	0,		NULL,	NULL,	NULL},
	//{"WillTopic", 			dalType_int,	0,	0,		NULL,	NULL,	NULL},
	//{"WillValue", 			dalType_int,	0,	0,		NULL,	NULL,	NULL},
	{"ClientID", 		        dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{"TransportProtocol", 		dalType_string,	0,	0,		NULL,	NULL,	NULL},
	{"ForceReconnect", 			dalType_boolean,0,	0,		NULL,	NULL,	NULL},
	//{"KeepAliveTime", 		dalType_int,	0,	0,		NULL,	NULL,	NULL},
	{NULL,						0,				0,	0,		NULL,	NULL,	NULL}
};

struct json_object *mqttClientObj;
objIndex_t mqttClientIid = {0};
bool g_enable;
const char* g_brokerAddr;
const char* g_brokerPort;
int g_X_ZYXEL_QoS;
bool g_will_enable;
const char* g_will_topic;
const char* g_will_value;
const char* g_transport_protocol;
const char* g_client_id;
bool g_force_reconnect;
int g_keepalivetime;
const char* Iface;
void initMqttEntryGlobalObjects(){
	mqttClientObj = NULL;
	return;
}

void freeAllmqttClientObjects(){
	if(mqttClientObj) {
		json_object_put(mqttClientObj);
		mqttClientObj = NULL;
	}
	return ;
}

void getMqttBasicInfo(struct json_object *Jobj){
	g_enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	printf("g_enable=%d\n", g_enable);
	
	g_brokerAddr = json_object_get_string(json_object_object_get(Jobj, "BrokerAddress"));
	if(g_brokerAddr)
		printf("g_brokerAddr=%s\n", g_brokerAddr);
	
	g_brokerPort = json_object_get_int(json_object_object_get(Jobj, "BrokerPort"));
		printf("g_brokerPort=%d\n", g_brokerPort);
	
	g_X_ZYXEL_QoS = json_object_get_int(json_object_object_get(Jobj, "X_ZYXEL_QoS"));
	printf("g_X_ZYXEL_QoS=%d\n", g_X_ZYXEL_QoS);

	/*
	g_will_enable = json_object_get_boolean(json_object_object_get(Jobj, "WillEnable"));
	printf("g_will_enable=%d\n", g_will_enable);
	
	g_will_topic = json_object_get_string(json_object_object_get(Jobj, "WillTopic"));
	if(g_will_topic)
		printf("g_will_topic=%s\n", g_will_topic);
	
	g_will_value = json_object_get_string(json_object_object_get(Jobj, "WillValue"));
	if(g_will_value)
		printf("g_will_value=%s\n", g_will_value);
		*/
	g_transport_protocol = json_object_get_string(json_object_object_get(Jobj, "TransportProtocol"));
	if(g_transport_protocol){
		printf("g_transport_protocol=%s\n", g_transport_protocol);
	}
	
	g_client_id = json_object_get_string(json_object_object_get(Jobj, "ClientID"));
	if(g_client_id){
		printf("g_client_id=%s\n", g_client_id);
	}

	g_force_reconnect = json_object_get_boolean(json_object_object_get(Jobj, "ForceReconnect"));
	printf("g_force_reconnect=%d\n", g_force_reconnect);
	/*
	g_keepalivetime = json_object_get_int(json_object_object_get(Jobj, "KeepAliveTime"));
	printf("g_keepalivetime=%d\n", g_keepalivetime);
	*/
	
	return;
}
void zcfgFeDalShowMQTT(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	int force_reconnect = 0;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-5s %-6s %-15s %-10s %-17s %-14s %-10s %-10s\n",
			"Index", "Enable", "BrokerAddress", "BrokerPort", "TransportProtocol", "ForceReconnect", "X_ZYXEL_QoS", "ClientID");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray,i);
		printf("%-5s %-6s %-15s %-10s %-17s %-14s %-10s %-10s\n",
			json_object_get_string(json_object_object_get(obj, "Index")),
			json_object_get_string(json_object_object_get(obj, "Enable")),
			json_object_get_string(json_object_object_get(obj, "BrokerAddress")),
			json_object_get_string(json_object_object_get(obj, "BrokerPort")),
			json_object_get_string(json_object_object_get(obj, "TransportProtocol")),
			force_reconnect?"true":"false",//always show false
			json_object_get_string(json_object_object_get(obj, "X_ZYXEL_QoS")),
			json_object_get_string(json_object_object_get(obj, "ClientID")));
	}
}

zcfgRet_t zcfgFeDalmqttClientObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;

	//printf("%s():--- Edit MQTTEntry Json Object --- \n%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MQTT_CLIENT, &mqttClientIid, &mqttClientObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get MQTT fail oid(%d)\n ",__FUNCTION__,RDM_OID_MQTT_CLIENT);
		return ZCFG_INTERNAL_ERROR;
	}
	
	if(mqttClientObj != NULL){
		if(json_object_object_get(Jobj, "Enable"))
			json_object_object_add(mqttClientObj, "Enable", json_object_new_boolean(g_enable));
		if(json_object_object_get(Jobj, "BrokerAddress"))
			json_object_object_add(mqttClientObj, "BrokerAddress", json_object_new_string(g_brokerAddr));
		if(json_object_object_get(Jobj, "BrokerPort"))
			json_object_object_add(mqttClientObj, "BrokerPort", json_object_new_int(g_brokerPort));
		if(json_object_object_get(Jobj, "X_ZYXEL_QoS"))
			json_object_object_add(mqttClientObj, "X_ZYXEL_QoS", json_object_new_int(g_X_ZYXEL_QoS));
		/*
		if(json_object_object_get(Jobj, "WillEnable"))
			json_object_object_add(mqttClientObj, "WillEnable", json_object_new_boolean(g_will_enable));
		if(json_object_object_get(Jobj, "WillTopic"))
			json_object_object_add(mqttClientObj, "WillTopic", json_object_new_string(g_will_topic));
		if(json_object_object_get(Jobj, "WillValue"))
			json_object_object_add(mqttClientObj, "WillValue", json_object_new_string(g_will_value));
			*/
		if(json_object_object_get(Jobj, "TransportProtocol"))
			json_object_object_add(mqttClientObj, "TransportProtocol", json_object_new_string(g_transport_protocol));
		if(json_object_object_get(Jobj, "ClientID"))
			json_object_object_add(mqttClientObj, "ClientID", json_object_new_string(g_client_id));
		if(json_object_object_get(Jobj, "ForceReconnect"))
			json_object_object_add(mqttClientObj, "ForceReconnect", json_object_new_boolean(g_force_reconnect));
		/*
		if(json_object_object_get(Jobj, "KeepAliveTime"))
			json_object_object_add(mqttClientObj, "KeepAliveTime", json_object_new_int(g_keepalivetime));
			*/
	}
	return ret;
}

zcfgRet_t zcfgFeDalMQTTEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *Jarray = NULL;
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(mqttClientIid);
	oid = RDM_OID_MQTT_CLIENT;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));

	initMqttEntryGlobalObjects();

	getMqttBasicInfo(Jobj);

	Jarray = json_object_new_array();

	zcfgFeDalMQTTGet(NULL,Jarray, NULL);

	ret = convertIndextoIid(index, &mqttClientIid, oid, NULL, NULL, replyMsg);

	if(ret != ZCFG_SUCCESS)
		return ret;	

	if((ret = zcfgFeDalmqttClientObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit MQTT Entry \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_MQTT_CLIENT, &mqttClientIid, mqttClientObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d RDM_OID_MQTT_CLIENT\n", __FUNCTION__, RDM_OID_MQTT_CLIENT);
	}

exit:
	freeAllmqttClientObjects();
	return ret;
}

	
zcfgRet_t zcfgFeDalMQTTGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *MqttJobj = NULL;
	objIndex_t objIid = {0};
	int count = 1;
	int force_reconnect = 0;
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MQTT_CLIENT, &objIid, &MqttJobj) == ZCFG_SUCCESS){
		paramJobj = json_object_new_object();
		json_object_object_add(paramJobj, "Index", json_object_new_int(count));
		json_object_object_add(paramJobj, "Enable", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "Enable")));
		json_object_object_add(paramJobj, "BrokerAddress", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "BrokerAddress")));
		json_object_object_add(paramJobj, "BrokerPort", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "BrokerPort")));
		json_object_object_add(paramJobj, "X_ZYXEL_QoS", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "X_ZYXEL_QoS")));
		/*
		json_object_object_add(paramJobj, "WillEnable", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "WillEnable")));
		json_object_object_add(paramJobj, "WillTopic", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "WillTopic")));
		json_object_object_add(paramJobj, "WillValue", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "WillValue")));
		*/
		json_object_object_add(paramJobj, "TransportProtocol", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "TransportProtocol")));
		json_object_object_add(paramJobj, "ClientID", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "ClientID")));
		json_object_object_add(paramJobj, "ForceReconnect", json_object_new_boolean(force_reconnect));//always show false
		/*
		json_object_object_add(paramJobj, "KeepAliveTime", JSON_OBJ_COPY(json_object_object_get(MqttJobj, "KeepAliveTime")));
		*/
		json_object_array_add(Jarray, paramJobj);
	
		zcfgFeJsonObjFree(MqttJobj);
		count++;
	}
	return ret;
}


zcfgRet_t zcfgFeDalMQTTAdd(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	initMqttEntryGlobalObjects();
	getMqttBasicInfo(Jobj);

	IID_INIT(mqttClientIid);
	
	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_MQTT_CLIENT, &mqttClientIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add MQTT Entry \n ",__FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalmqttClientObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit MQTT Entry \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonSet(RDM_OID_MQTT_CLIENT, &mqttClientIid, mqttClientObj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set object %d MQTT Entry\n", __FUNCTION__, RDM_OID_MQTT_CLIENT);
	}

exit:
	freeAllmqttClientObjects();
	return ret;
}

zcfgRet_t zcfgFeDalMQTTDelete(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(mqttClientIid);

	oid = RDM_OID_MQTT_CLIENT;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(index, &mqttClientIid, oid, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonDel(oid, &mqttClientIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete mqtt obj fail \n", __FUNCTION__);
		goto exit;
	}

exit:
	freeAllmqttClientObjects();
	return ret;
	
}

zcfgRet_t zcfgFeDalMQTT(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	if(!strcmp(method, "PUT")){
		ret = zcfgFeDalMQTTEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalMQTTAdd(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalMQTTDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalMQTTGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

/*-----------------------MQTT Node -------------*/
objIndex_t mqttIid;
struct json_object *MQTTNodeObj;

void zcfgFeDalShowMQTTNode(struct json_object *Jarray){
#if 0
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%s: %s \n","X_ZYXEL_BoundIfName",json_object_get_string(json_object_object_get(Jarray,"X_ZYXEL_BoundIfName")));
#endif
}

zcfgRet_t zcfgFeDalMQTTNodeGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
        zcfgRet_t ret = ZCFG_SUCCESS;
        
	const char* Iface;
	
	IID_INIT(mqttIid);
        if( (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MQTT,&mqttIid,&MQTTNodeObj)) != ZCFG_SUCCESS){
                printf("%s():(ERROR) zcfgFeObjStructGet 1111 get MQTT fail oid(%d), ret = %d\n",__FUNCTION__,RDM_OID_MQTT, ret);
                return ZCFG_INTERNAL_ERROR;
        }
	if(MQTTNodeObj != NULL){
		if(json_object_object_get(MQTTNodeObj,"X_ZYXEL_BoundIfList")){
			Iface = json_object_get_string(json_object_object_get(MQTTNodeObj,"X_ZYXEL_BoundIfList"));
               		printf("X_ZYXEL_BoundIfList = %s\n",Iface);
		}
	        zcfgFeJsonObjFree(MQTTNodeObj);
        }
        return ret;
}

zcfgRet_t zcfgFeDalMQTTNodeEdit(struct json_object *Jobj, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* Iface;
		
	IID_INIT(mqttIid);
	if( (ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MQTT,&mqttIid,&MQTTNodeObj)) != ZCFG_SUCCESS){
		printf(" %s() : (ERROR) zcfgFeObjJsonGetWithoutUpdate get MQTT object fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if(MQTTNodeObj != NULL ){
		if(json_object_object_get(Jobj,"X_ZYXEL_BoundIfList")){
			Iface = json_object_get_string(json_object_object_get(Jobj,"X_ZYXEL_BoundIfList"));
			json_object_object_add(MQTTNodeObj, "X_ZYXEL_BoundIfList", json_object_new_string(Iface));
			zcfgFeObjJsonBlockedSet(RDM_OID_MQTT, &mqttIid, MQTTNodeObj, NULL);
		}
		zcfgFeJsonObjFree(MQTTNodeObj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDalMQTTNode(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
        zcfgRet_t ret = ZCFG_SUCCESS;
        if(!strcmp(method, "PUT")){
                ret = zcfgFeDalMQTTNodeEdit(Jobj, replyMsg);
        }
        else if(!strcmp(method, "GET"))
                ret = zcfgFeDalMQTTNodeGet(Jobj, Jarray, NULL);
        else
                printf("Unknown method:%s\n", method);

        return ret;
}

