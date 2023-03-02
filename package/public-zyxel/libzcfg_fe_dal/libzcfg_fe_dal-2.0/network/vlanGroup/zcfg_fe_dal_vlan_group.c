#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t VLANGRP_param[] =
{
	{"Index", 		dalType_int, 		0, 	0, 	NULL,	NULL,	dal_Delete|dal_Edit},
	{"Enable", 		dalType_boolean, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"type", 		dalType_string, 	0, 	0, 	NULL,	NULL,	dalcmd_Forbid},
	{"GroupName", 	dalType_string, 	0, 	0, 	NULL,	NULL,	dal_Add},
	{"VlanId", 		dalType_int, 		1, 	4094, 	NULL,	NULL,	dal_Add},
	{"IntfList", 	dalType_IntfGrpLanPortListNoWiFi,		0,	0,	NULL,	NULL,	dal_Add},
	{"TagList", 	dalType_IntfGrpLanPortListNoWiFi,		0,	0,	NULL,	NULL,	0},
	{NULL, 			0, 					0, 	0, 	NULL}
};
void zcfgFeDalShowVlanGroup(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj = NULL;
	const char *taglist = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	printf("%-10s %-20s %-20s %-20s %-10s\n",
		"Index", "Group Name", "VLAN ID", "Interface", "TAG");
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "TagList")),""))
			taglist = "N/A";
		else
			taglist = json_object_get_string(json_object_object_get(obj, "TagList"));
		printf("%-10s %-20s %-20s %-20s %-10s\n",
		json_object_get_string(json_object_object_get(obj, "Index")),
		json_object_get_string(json_object_object_get(obj, "GroupName")),
		json_object_get_string(json_object_object_get(obj, "VlanId")),
		json_object_get_string(json_object_object_get(obj, "IntfList")),
		taglist);
	}
}

char* dalcmdLanListParse(const char *parseparam, char vlanarray[]){		// for dalcmd use
	const char *LanPort = NULL;
	char buff[32] = {0};
	char *tmp = NULL;
	strcpy(buff, parseparam);
	if(strstr(buff, ",")){	
		if(strstr(buff, " ")){
			strcpy(vlanarray, parseparam);
		}	
		else{
			LanPort = strtok_r(buff, ",", &tmp);
			while(LanPort != NULL){
				strcat(vlanarray, LanPort);
				strcat(vlanarray, ", ");
				LanPort = strtok_r(tmp, ",", &tmp);
			}
			if(vlanarray[strlen(vlanarray)-2] == ',')
				vlanarray[strlen(vlanarray)-2] = '\0';
		}
	}
	else
		strcpy(vlanarray, parseparam);
	return vlanarray;
	
}

zcfgRet_t dalcmdTagCheck(char unTagLan[], char TagLan[], char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char buff[32] = {0};
	char *lan = NULL, *tmp = NULL;
	
	strcpy(buff, TagLan);
	lan = strtok_r(buff, ", ",&tmp);
	while(lan != NULL){
		if(strstr(unTagLan, lan) == NULL){
			strcpy(replyMsg, "Parameter error!!!");
			return ZCFG_INVALID_PARAM_VALUE;
		}
		lan = strtok_r(tmp, ", ", &tmp);
	}
	return ret;
}

zcfgRet_t VlanIdDuplicateIntf(int VlanId, const char *IntfLanPort, int idx, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *vlanGrpObj = NULL;
	objIndex_t vlanGrpIid = {0};
	int OrigVlanID = 0;
	const char *OrigIntfLan = NULL;
	char buff[32] = {0};
	char *lan = NULL, *tmp = NULL;
	
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanGrpIid, &vlanGrpObj) == ZCFG_SUCCESS){
		if(idx == vlanGrpIid.idx[0]){
			zcfgFeJsonObjFree(vlanGrpObj);
			continue;
		}
		OrigVlanID = json_object_get_int(json_object_object_get(vlanGrpObj, "VlanId"));
		if(VlanId == OrigVlanID){
			OrigIntfLan = json_object_get_string(json_object_object_get(vlanGrpObj, "IntfList"));
			strcpy(buff, IntfLanPort);
			lan = strtok_r(buff, ", ",&tmp);
			while(lan != NULL){
				if(strcmp(strstr(OrigIntfLan, lan),"")){
					zcfgFeJsonObjFree(vlanGrpObj);
					strcpy(replyMsg, "LAN Interface cannot duplicate if Vlan ID is the same.");
					return ZCFG_INVALID_PARAM_VALUE;
				}
				lan = strtok_r(tmp, ", ", &tmp);
			}
		}
		zcfgFeJsonObjFree(vlanGrpObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDal_VlanGroup_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *vlanGrpObj = NULL;
	objIndex_t vlanGrpIid = {0};
	const char *groupName = NULL;
	int vlanID, idx = 0;
	const char *tagList = NULL;
	const char *intfList = NULL;
	char untag[32] = {0}, tag[32] = {0};
	zcfg_offset_t oid;

	IID_INIT(vlanGrpIid);

	groupName = json_object_get_string(json_object_object_get(Jobj, "GroupName"));
	vlanID = json_object_get_int(json_object_object_get(Jobj, "VlanId"));
	tagList = json_object_get_string(json_object_object_get(Jobj, "TagList"));
	intfList = json_object_get_string(json_object_object_get(Jobj, "IntfList"));
	dalcmdLanListParse(intfList, untag);
	if(json_object_object_get(Jobj, "TagList"))
	dalcmdLanListParse(tagList, tag);
	if(!json_object_object_get(Jobj, "type")){
		if(json_object_object_get(Jobj, "TagList"))
		ret = dalcmdTagCheck(untag, tag, replyMsg);
		if (ret != ZCFG_SUCCESS)	
			return ret;
		ret = VlanIdDuplicateIntf(vlanID, intfList, idx, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
		oid = RDM_OID_VLAN_GROUP;
		ret = dalcmdParamCheck(NULL, groupName, oid, "GroupName", "GroupName", replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}
	ret = zcfgFeObjJsonAdd(RDM_OID_VLAN_GROUP, &vlanGrpIid, NULL);
	if(ret == ZCFG_SUCCESS){
		ret = zcfgFeObjJsonGet(RDM_OID_VLAN_GROUP, &vlanGrpIid, &vlanGrpObj);
	}else
		printf("Add VLAN GROUP fail = %d\n",ret);

	if(ret == ZCFG_SUCCESS){
		json_object_object_add(vlanGrpObj, "Enable", json_object_new_boolean(true));
		json_object_object_add(vlanGrpObj, "VlanId", json_object_new_int(vlanID));
		json_object_object_add(vlanGrpObj, "GroupName", json_object_new_string(groupName));
		json_object_object_add(vlanGrpObj, "TagList", json_object_new_string(tag));
		json_object_object_add(vlanGrpObj, "IntfList", json_object_new_string(untag));
		
		ret = zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &vlanGrpIid, vlanGrpObj, NULL);
	}

	zcfgFeJsonObjFree(vlanGrpObj);
	return ret;
}

zcfgRet_t zcfgFeDal_VlanGroup_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *vlanGrpObj = NULL;
	objIndex_t vlanGrpIid = {0};
	int vlanID = 0;
	const char *tagList = NULL;
	const char *intfList = NULL;
	char untag[32] = {0}, tag[32] = {0};

	IID_INIT(vlanGrpIid);
	vlanGrpIid.level = 1;
	vlanGrpIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));

		vlanID = json_object_get_int(json_object_object_get(Jobj, "VlanId"));
		tagList = json_object_get_string(json_object_object_get(Jobj, "TagList"));
		intfList = json_object_get_string(json_object_object_get(Jobj, "IntfList"));
	if(!json_object_object_get(Jobj, "type")){
		if((json_object_object_get(Jobj, "TagList") && !json_object_object_get(Jobj, "IntfList"))){
			strcpy(replyMsg, "IntfLise and TagList are necessary parameter if modify TagList or IntfList.");
				return ZCFG_INVALID_PARAM_VALUE;
		}
		else if((!json_object_object_get(Jobj, "TagList") && json_object_object_get(Jobj, "IntfList"))){
			dalcmdLanListParse(intfList, untag);
		}
		else if(json_object_object_get(Jobj, "TagList") && json_object_object_get(Jobj, "IntfList")){
			dalcmdLanListParse(intfList, untag);
			dalcmdLanListParse(tagList, tag);
			ret = dalcmdTagCheck(untag, tag, replyMsg);
			if (ret != ZCFG_SUCCESS)	
				return ret;
		}
		if(json_object_object_get(Jobj, "VlanId") && json_object_object_get(Jobj, "IntfList")){
			ret = VlanIdDuplicateIntf(vlanID, intfList, vlanGrpIid.idx[0], replyMsg);
			if(ret != ZCFG_SUCCESS)
				return ret;
		}
	}
	else{
		dalcmdLanListParse(intfList, untag);
		dalcmdLanListParse(tagList, tag);
	}
	if(zcfgFeObjJsonGet(RDM_OID_VLAN_GROUP, &vlanGrpIid, &vlanGrpObj) == ZCFG_SUCCESS){
		json_object_object_add(vlanGrpObj, "Enable", json_object_new_boolean(true));
		if(json_object_object_get(Jobj, "VlanId"))
		json_object_object_add(vlanGrpObj, "VlanId", json_object_new_int(vlanID));
		if(json_object_object_get(Jobj, "TagList"))
			json_object_object_add(vlanGrpObj, "TagList", json_object_new_string(tag));
		if(json_object_object_get(Jobj, "IntfList"))
			json_object_object_add(vlanGrpObj, "IntfList", json_object_new_string(untag));
		ret = zcfgFeObjJsonSet(RDM_OID_VLAN_GROUP, &vlanGrpIid, vlanGrpObj, NULL);
	}

	zcfgFeJsonObjFree(vlanGrpObj);
	return ret;
}
zcfgRet_t zcfgFeDal_VlanGroup_Delete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t vlanGrpIid = {0};
	bool found = false;
	struct json_object *VlanGroupJobj = NULL;

	IID_INIT(vlanGrpIid);
	vlanGrpIid.level = 1;
	vlanGrpIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanGrpIid, &VlanGroupJobj) == ZCFG_SUCCESS) {
		if(json_object_get_boolean(json_object_object_get(VlanGroupJobj, "Enable")))
			found = true;
		zcfgFeJsonObjFree(VlanGroupJobj);
	}

	if (found) {
		ret = zcfgFeObjJsonDel(RDM_OID_VLAN_GROUP, &vlanGrpIid, NULL);
	} else {
		if (replyMsg != NULL)
			strcpy(replyMsg, "Can't find the group");
		return ZCFG_NOT_FOUND;
	}

	return ret;
}
zcfgRet_t zcfgFeDal_VlanGroup_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL;
	struct json_object *VlanJobj = NULL;
	struct json_object *indexObj = NULL;
	objIndex_t VlanIid = {0};
	int indexGet = -1;
	int obj_MaxLength = 0;

	if ((indexObj = json_object_object_get(Jobj, "Index")) != NULL){
		indexGet = json_object_get_int(indexObj);
	}

	IID_INIT(VlanIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &VlanIid, &VlanJobj) == ZCFG_SUCCESS){
		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_VLAN_GROUP);
		if ((indexGet == -1) || (VlanIid.idx[0] == indexGet))
		{
			paramJobj = json_object_new_object();
			json_object_object_add(paramJobj, "Index", json_object_new_int(VlanIid.idx[0]));
			json_object_object_add(paramJobj, "GroupName", JSON_OBJ_COPY(json_object_object_get(VlanJobj, "GroupName")));
			json_object_object_add(paramJobj, "VlanId", JSON_OBJ_COPY(json_object_object_get(VlanJobj, "VlanId")));
			json_object_object_add(paramJobj, "IntfList", JSON_OBJ_COPY(json_object_object_get(VlanJobj, "IntfList")));
			json_object_object_add(paramJobj, "TagList", JSON_OBJ_COPY(json_object_object_get(VlanJobj, "TagList")));
			json_object_object_add(paramJobj, "obj_MaxLength", json_object_new_int(obj_MaxLength));
			json_object_array_add(Jarray, paramJobj);
		}
		zcfgFeJsonObjFree(VlanJobj);
	}
	return ret;
	}
	

zcfgRet_t zcfgFeDalVlanGroup(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_VlanGroup_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "POST"))
		ret = zcfgFeDal_VlanGroup_Add(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDal_VlanGroup_Delete(Jobj, NULL);
	else if(!strcmp(method,"GET"))
		ret = zcfgFeDal_VlanGroup_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


