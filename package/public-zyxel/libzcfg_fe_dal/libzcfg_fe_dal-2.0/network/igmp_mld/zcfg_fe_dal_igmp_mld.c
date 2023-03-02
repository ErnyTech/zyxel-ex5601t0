#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

dal_param_t IGMPMLD_param[] =
{
	//IGMP
	{"Igmp_Version", 				dalType_int, 	1, 	3, 		NULL,	NULL,	0},
	{"Igmp_QueryInterval", 			dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Igmp_QueryResponseInterval",	dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Igmp_LastMemberQueryInterval", dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Igmp_RobustnessValue", 		dalType_int, 	1, 	7, 		NULL,	NULL,	0},
	{"Igmp_MaxGroups", 				dalType_int, 	1, 	25, 	NULL,	NULL,	0},
	{"Igmp_MaxSources", 			dalType_int, 	1, 	10, 	NULL,	NULL,	0},
	{"Igmp_MaxMembers", 			dalType_int,	1,	25,		NULL,	NULL,	0},
	{"Igmp_FastLeave", 				dalType_boolean,0,	0,	NULL,	NULL,	0},
	{"Igmp_LanToLanEnable", 		dalType_boolean,0, 	0, 	NULL,	NULL,	0},
	{"Igmp_JoinImmediate", 			dalType_boolean,0, 	0, 	NULL,	NULL,	0},
	//MLD
	{"Mld_Version", 				dalType_int, 	1, 	2, 		NULL,	NULL,	0},
	{"Mld_QueryInterval", 			dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Mld_QueryResponseInterval",	dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Mld_LastMemberQueryInterval", dalType_int, 	0, 	30000, 	NULL,	NULL,	0},
	{"Mld_RobustnessValue", 		dalType_int, 	1, 	7, 		NULL,	NULL,	0},
	{"Mld_MaxGroups", 				dalType_int, 	1, 	25, 	NULL,	NULL,	0},
	{"Mld_MaxSources", 				dalType_int, 	1, 	10, 	NULL,	NULL,	0},
	{"Mld_MaxMembers", 				dalType_int,	1,	25,		NULL,	NULL,	0},
	{"Mld_FastLeave", 				dalType_boolean,0,	0,	NULL,	NULL,	0},
	{"Mld_LanToLanEnable", 			dalType_boolean,0, 	0, 	NULL,	NULL,	0},
	{NULL, 							0,				0, 	0, 	NULL,	NULL,	0}
};
void zcfgFeDalShowIGMPMLD(struct json_object *Jarray){
	int i, len = 0;
	struct json_object *obj;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		printf("IGMP Configuration\n");
		printf("%-50s %s \n","Default Version :", json_object_get_string(json_object_object_get(obj, "Igmp_Version")));
		printf("%-50s %s \n","Query Interval :", json_object_get_string(json_object_object_get(obj, "Igmp_QueryInterval")));
		printf("%-50s %s \n","Query Response Interval :", json_object_get_string(json_object_object_get(obj, "Igmp_QueryResponseInterval")));
		printf("%-50s %s \n","Last Member Query Interval :", json_object_get_string(json_object_object_get(obj, "Igmp_LastMemberQueryInterval")));
		printf("%-50s %s \n","Robustness Value :", json_object_get_string(json_object_object_get(obj, "Igmp_RobustnessValue")));
		printf("%-50s %s \n","Maximum Multicast Groups :", json_object_get_string(json_object_object_get(obj, "Igmp_MaxGroups")));
		printf("%-50s %s \n","Maximum Multicast Data Sources (for IGMPv3) :", json_object_get_string(json_object_object_get(obj, "Igmp_MaxSources")));
		printf("%-50s %s \n","Maximum Multicast Group Members :", json_object_get_string(json_object_object_get(obj, "Igmp_MaxMembers")));
		printf("%-50s %s \n","Fast Leave Enable :", json_object_get_string(json_object_object_get(obj, "Igmp_FastLeave")));
		printf("%-50s %s \n","LAN to LAN (Intra LAN) Multicast Enable :", json_object_get_string(json_object_object_get(obj, "Igmp_LanToLanEnable")));
		printf("%-50s %s \n","Membership Join Immediate (IPTV) :", json_object_get_string(json_object_object_get(obj, "Igmp_JoinImmediate")));
		printf("\nMLD Configuration\n");
		printf("%-50s %s \n","Default Version :", json_object_get_string(json_object_object_get(obj, "Mld_Version")));
		printf("%-50s %s \n","Query Interval :", json_object_get_string(json_object_object_get(obj, "Mld_QueryInterval")));
		printf("%-50s %s \n","Query Response Interval :", json_object_get_string(json_object_object_get(obj, "Mld_QueryResponseInterval")));
		printf("%-50s %s \n","Last Member Query Interval :", json_object_get_string(json_object_object_get(obj, "Mld_LastMemberQueryInterval")));
		printf("%-50s %s \n","Robustness Value :", json_object_get_string(json_object_object_get(obj, "Mld_RobustnessValue")));
		printf("%-50s %s \n","Maximum Multicast Groups :", json_object_get_string(json_object_object_get(obj, "Mld_MaxGroups")));
		printf("%-50s %s \n","Maximum Multicast Data Sources (for mldv2) :", json_object_get_string(json_object_object_get(obj, "Mld_MaxSources")));
		printf("%-50s %s \n","Maximum Multicast Group Members :", json_object_get_string(json_object_object_get(obj, "Mld_MaxMembers")));
		printf("%-50s %s \n","Fast Leave Enable :", json_object_get_string(json_object_object_get(obj, "Mld_FastLeave")));
		printf("%-50s %s \n","LAN to LAN (Intra LAN) Multicast Enable :", json_object_get_string(json_object_object_get(obj, "Mld_LanToLanEnable")));
	}
}

zcfgRet_t setIGMP(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *igmpObj = NULL;
	objIndex_t igmpIid = {0};
	int igmpVersion = 0;
	int queryInterval = 0;
	int queryResInterval = 0;
	int lastMemberQueryInterval = 0;
	int robustnessValue = 0;
	int maxGroups = 0;
	int maxSources = 0;
	int maxMembers = 0;
	int fastLeave = 0;
	int lanToLanEnable = 0;
	int joinImmediate = 0;

	igmpVersion = json_object_get_int(json_object_object_get(Jobj, "Igmp_Version"));
	queryInterval = json_object_get_int(json_object_object_get(Jobj, "Igmp_QueryInterval"));
	queryResInterval = json_object_get_int(json_object_object_get(Jobj, "Igmp_QueryResponseInterval"));
	lastMemberQueryInterval = json_object_get_int(json_object_object_get(Jobj, "Igmp_LastMemberQueryInterval"));
	robustnessValue = json_object_get_int(json_object_object_get(Jobj, "Igmp_RobustnessValue"));
	maxGroups = json_object_get_int(json_object_object_get(Jobj, "Igmp_MaxGroups"));
	maxSources = json_object_get_int(json_object_object_get(Jobj, "Igmp_MaxSources"));
	maxMembers = json_object_get_int(json_object_object_get(Jobj, "Igmp_MaxMembers"));
	fastLeave = json_object_get_int(json_object_object_get(Jobj, "Igmp_FastLeave"));
	lanToLanEnable = json_object_get_int(json_object_object_get(Jobj, "Igmp_LanToLanEnable"));
	joinImmediate = json_object_get_int(json_object_object_get(Jobj, "Igmp_JoinImmediate"));

	IID_INIT(igmpIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj);

	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Igmp_Version") != NULL)
		json_object_object_add(igmpObj, "IgmpVersion", json_object_new_int(igmpVersion));
		if(json_object_object_get(Jobj, "Igmp_QueryInterval") != NULL)
		json_object_object_add(igmpObj, "QueryInterval", json_object_new_int(queryInterval));
		if(json_object_object_get(Jobj, "Igmp_QueryResponseInterval") != NULL)
		json_object_object_add(igmpObj, "QueryResponseInterval", json_object_new_int(queryResInterval));
		if(json_object_object_get(Jobj, "Igmp_LastMemberQueryInterval") != NULL)
		json_object_object_add(igmpObj, "LastMemberQueryInterval", json_object_new_int(lastMemberQueryInterval));
		if(json_object_object_get(Jobj, "Igmp_RobustnessValue") != NULL)
		json_object_object_add(igmpObj, "RobustnessValue", json_object_new_int(robustnessValue));
		if(json_object_object_get(Jobj, "Igmp_MaxGroups") != NULL)
		json_object_object_add(igmpObj, "MaxGroups", json_object_new_int(maxGroups));
		if(json_object_object_get(Jobj, "Igmp_MaxSources") != NULL)
		json_object_object_add(igmpObj, "MaxSources", json_object_new_int(maxSources));
		if(json_object_object_get(Jobj, "Igmp_MaxMembers") != NULL)
		json_object_object_add(igmpObj, "MaxMembers", json_object_new_int(maxMembers));
		if(json_object_object_get(Jobj, "Igmp_FastLeave") != NULL)
		json_object_object_add(igmpObj, "FastLeave", json_object_new_int(fastLeave));
		if(json_object_object_get(Jobj, "Igmp_LanToLanEnable") != NULL)
		json_object_object_add(igmpObj, "LanToLanEnable", json_object_new_int(lanToLanEnable));
		if(json_object_object_get(Jobj, "Igmp_JoinImmediate") != NULL)
		json_object_object_add(igmpObj, "JoinImmediate", json_object_new_int(joinImmediate));

		ret = zcfgFeObjJsonSet(RDM_OID_ZY_IGMP, &igmpIid, igmpObj, NULL);
	}

	zcfgFeJsonObjFree(igmpObj);
	return ret;
}

zcfgRet_t setMLD(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *mldObj = NULL;
	objIndex_t mldIid = {0};
	int mldVersion = 0;
	int queryInterval = 0;
	int queryResInterval = 0;
	int lastMemberQueryInterval = 0;
	int robustnessValue = 0;
	int maxGroups = 0;
	int maxSources = 0;
	int maxMembers = 0;
	int fastLeave = 0;
	int lanToLanEnable = 0;

	mldVersion = json_object_get_int(json_object_object_get(Jobj, "Mld_Version"));
	queryInterval = json_object_get_int(json_object_object_get(Jobj, "Mld_QueryInterval"));
	queryResInterval = json_object_get_int(json_object_object_get(Jobj, "Mld_QueryResponseInterval"));
	lastMemberQueryInterval = json_object_get_int(json_object_object_get(Jobj, "Mld_LastMemberQueryInterval"));
	robustnessValue = json_object_get_int(json_object_object_get(Jobj, "Mld_RobustnessValue"));
	maxGroups = json_object_get_int(json_object_object_get(Jobj, "Mld_MaxGroups"));
	maxSources = json_object_get_int(json_object_object_get(Jobj, "Mld_MaxSources"));
	maxMembers = json_object_get_int(json_object_object_get(Jobj, "Mld_MaxMembers"));
	fastLeave = json_object_get_int(json_object_object_get(Jobj, "Mld_FastLeave"));
	lanToLanEnable = json_object_get_int(json_object_object_get(Jobj, "Mld_LanToLanEnable"));
	

	IID_INIT(mldIid);
	ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_MLD, &mldIid, &mldObj);

	if(ret == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "Mld_Version") != NULL)
		json_object_object_add(mldObj, "MldVersion", json_object_new_int(mldVersion));
		if(json_object_object_get(Jobj, "Mld_QueryInterval") != NULL)
		json_object_object_add(mldObj, "QueryInterval", json_object_new_int(queryInterval));
		if(json_object_object_get(Jobj, "Mld_QueryResponseInterval") != NULL)
		json_object_object_add(mldObj, "QueryResponseInterval", json_object_new_int(queryResInterval));
		if(json_object_object_get(Jobj, "Mld_LastMemberQueryInterval") != NULL)
		json_object_object_add(mldObj, "LastMemberQueryInterval", json_object_new_int(lastMemberQueryInterval));
		if(json_object_object_get(Jobj, "Mld_RobustnessValue") != NULL)
		json_object_object_add(mldObj, "RobustnessValue", json_object_new_int(robustnessValue));
		if(json_object_object_get(Jobj, "Mld_MaxGroups") != NULL)
		json_object_object_add(mldObj, "MaxGroups", json_object_new_int(maxGroups));
		if(json_object_object_get(Jobj, "Mld_MaxSources") != NULL)
		json_object_object_add(mldObj, "MaxSources", json_object_new_int(maxSources));
		if(json_object_object_get(Jobj, "Mld_MaxMembers") != NULL)
		json_object_object_add(mldObj, "MaxMembers", json_object_new_int(maxMembers));
		if(json_object_object_get(Jobj, "Mld_FastLeave") != NULL)
		json_object_object_add(mldObj, "FastLeave", json_object_new_int(fastLeave));
		if(json_object_object_get(Jobj, "Mld_LanToLanEnable") != NULL)
		json_object_object_add(mldObj, "LanToLanEnable", json_object_new_int(lanToLanEnable));

		ret = zcfgFeObjJsonSet(RDM_OID_ZY_MLD, &mldIid, mldObj, NULL);
	}

	zcfgFeJsonObjFree(mldObj);
	return ret;
}


zcfgRet_t zcfgFeDal_IGMPMLD_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	setIGMP(Jobj);
	setMLD(Jobj);

	return ret;
}
zcfgRet_t zcfgFeDalIGMPMLDGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *igmpObj = NULL;
	struct json_object *mldObj = NULL;
	struct json_object *paramJobj = NULL;
	objIndex_t igmpIid = {0};
	objIndex_t mldIid = {0};

	paramJobj = json_object_new_object();
	if(zcfgFeObjJsonGet(RDM_OID_ZY_IGMP, &igmpIid, &igmpObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Igmp_Version", JSON_OBJ_COPY(json_object_object_get(igmpObj, "IgmpVersion")));
		json_object_object_add(paramJobj, "Igmp_QueryInterval", JSON_OBJ_COPY(json_object_object_get(igmpObj, "QueryInterval")));
		json_object_object_add(paramJobj, "Igmp_QueryResponseInterval", JSON_OBJ_COPY(json_object_object_get(igmpObj, "QueryResponseInterval")));
		json_object_object_add(paramJobj, "Igmp_LastMemberQueryInterval", JSON_OBJ_COPY(json_object_object_get(igmpObj, "LastMemberQueryInterval")));
		json_object_object_add(paramJobj, "Igmp_RobustnessValue", JSON_OBJ_COPY(json_object_object_get(igmpObj, "RobustnessValue")));
		json_object_object_add(paramJobj, "Igmp_MaxGroups", JSON_OBJ_COPY(json_object_object_get(igmpObj, "MaxGroups")));
		json_object_object_add(paramJobj, "Igmp_MaxSources", JSON_OBJ_COPY(json_object_object_get(igmpObj, "MaxSources")));
		json_object_object_add(paramJobj, "Igmp_MaxMembers", JSON_OBJ_COPY(json_object_object_get(igmpObj, "MaxMembers")));
		json_object_object_add(paramJobj, "Igmp_FastLeave", JSON_OBJ_COPY(json_object_object_get(igmpObj, "FastLeave")));
		json_object_object_add(paramJobj, "Igmp_LanToLanEnable", JSON_OBJ_COPY(json_object_object_get(igmpObj, "LanToLanEnable")));
		json_object_object_add(paramJobj, "Igmp_JoinImmediate", JSON_OBJ_COPY(json_object_object_get(igmpObj, "JoinImmediate")));
		
		zcfgFeJsonObjFree(igmpObj);		
	}
	if(zcfgFeObjJsonGet(RDM_OID_ZY_MLD, &mldIid, &mldObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Mld_Version", JSON_OBJ_COPY(json_object_object_get(mldObj, "MldVersion")));
		json_object_object_add(paramJobj, "Mld_QueryInterval", JSON_OBJ_COPY(json_object_object_get(mldObj, "QueryInterval")));
		json_object_object_add(paramJobj, "Mld_QueryResponseInterval", JSON_OBJ_COPY(json_object_object_get(mldObj, "QueryResponseInterval")));
		json_object_object_add(paramJobj, "Mld_LastMemberQueryInterval", JSON_OBJ_COPY(json_object_object_get(mldObj, "LastMemberQueryInterval")));
		json_object_object_add(paramJobj, "Mld_RobustnessValue", JSON_OBJ_COPY(json_object_object_get(mldObj, "RobustnessValue")));
		json_object_object_add(paramJobj, "Mld_MaxGroups", JSON_OBJ_COPY(json_object_object_get(mldObj, "MaxGroups")));
		json_object_object_add(paramJobj, "Mld_MaxSources", JSON_OBJ_COPY(json_object_object_get(mldObj, "MaxSources")));
		json_object_object_add(paramJobj, "Mld_MaxMembers", JSON_OBJ_COPY(json_object_object_get(mldObj, "MaxMembers")));
		json_object_object_add(paramJobj, "Mld_FastLeave", JSON_OBJ_COPY(json_object_object_get(mldObj, "FastLeave")));
		json_object_object_add(paramJobj, "Mld_LanToLanEnable", JSON_OBJ_COPY(json_object_object_get(mldObj, "LanToLanEnable")));

		zcfgFeJsonObjFree(mldObj);
	}
	json_object_array_add(Jarray, paramJobj);
	
	return ret;
}



zcfgRet_t zcfgFeDalIGMPMLD(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDal_IGMPMLD_Edit(Jobj, NULL);
	}else if(!strcmp(method,"GET")){
		ret = zcfgFeDalIGMPMLDGet(Jobj, Jarray, NULL);
	}else{
		printf("Unknown method:%s\n", method);
	}

	return ret;
}

