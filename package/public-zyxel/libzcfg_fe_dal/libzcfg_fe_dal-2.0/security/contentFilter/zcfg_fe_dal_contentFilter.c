#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"


dal_param_t CONTENT_FILTER_param[]={
	{"Index",				dalType_int,	0,	0,	NULL, NULL, dal_Add|dal_Delete},
	{"Name",			dalType_string,		0,	0,	NULL,	NULL,	dal_Add},
	{"Description",		dalType_string,		0,	0,	NULL,	NULL,	0},
	{"ContentFilterEnable",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"adultContent",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"drugs",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"alcoholandTobacco",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"disturbing",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"gambling",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"illegal",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"illegalDownloads",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"violence",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"hate",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"weapons",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"dating",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"timeShoppingandAuctions",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"videoStreamingServices",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"socialNetworks",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"anonymizers",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{"unknown",	dalType_boolean,	0,	0,	NULL, NULL, 0},
	{NULL,		0,	0,	0,	NULL}
};


void zcfgFeDalShowContentFilter(struct json_object *Jarray){

	int i, len = 0;
	char settingStr [64] = {0};
	struct json_object *obj;
	bool ContentFilterEnable;
	bool adultContentEnable;
	bool drugsEnable;
	bool alcoholandTobaccoEnable;
	bool disturbingEnable;
	bool gamblingEnable;
	bool illegalEnable;
	bool illegalDownloadsEnable;
	bool violenceEnable;
	bool hateEnable;
	bool weaponsEnable;
	bool datingEnable;
	bool timeShoppingandAuctionsEnable;
	bool videoStreamingServicesEnable;
	bool socialNetworksEnable;
	bool anonymizersEnable;
	bool unknownEnable;


	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	printf("%-6s %-6s %-15s %-64s %-30s\n", "Index", "Enable","Filter Name", "Setting(bolean)", "Description");
	printf("%-6s %-6s %-15s %-64s %-30s\n", "------", "------","---------------", "ad:dr:al:ds:ga:il:il:vi:ha:we:da:ti:vi:so:an:un", "--");	
	printf("--------------------------------------------------------------------------------\n");

	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){

		obj = json_object_array_get_idx(Jarray, i);
		if(obj!=NULL){
			ContentFilterEnable = json_object_get_boolean(json_object_object_get(obj, "ContentFilterEnable"));
			adultContentEnable = json_object_get_boolean(json_object_object_get(obj, "adultContent"));
			drugsEnable = json_object_get_boolean(json_object_object_get(obj, "drugs"));
			alcoholandTobaccoEnable = json_object_get_boolean(json_object_object_get(obj, "alcoholandTobacco"));
			disturbingEnable = json_object_get_boolean(json_object_object_get(obj, "disturbing"));
			gamblingEnable = json_object_get_boolean(json_object_object_get(obj, "gambling"));
			illegalEnable = json_object_get_boolean(json_object_object_get(obj, "illegal"));
			illegalDownloadsEnable = json_object_get_boolean(json_object_object_get(obj, "illegalDownloads"));
			violenceEnable = json_object_get_boolean(json_object_object_get(obj, "violence"));
			hateEnable = json_object_get_boolean(json_object_object_get(obj, "hate"));
			weaponsEnable = json_object_get_boolean(json_object_object_get(obj, "weapons"));
			datingEnable = json_object_get_boolean(json_object_object_get(obj, "dating"));
			timeShoppingandAuctionsEnable = json_object_get_boolean(json_object_object_get(obj, "timeShoppingandAuctions"));
			videoStreamingServicesEnable = json_object_get_boolean(json_object_object_get(obj, "videoStreamingServices"));
			socialNetworksEnable = json_object_get_boolean(json_object_object_get(obj, "socialNetworks"));
			anonymizersEnable = json_object_get_boolean(json_object_object_get(obj, "anonymizers"));
			unknownEnable = json_object_get_boolean(json_object_object_get(obj, "unknown"));
			memset(settingStr, '\0', sizeof(settingStr));
			sprintf(settingStr, "%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d:%-2d",adultContentEnable,drugsEnable,alcoholandTobaccoEnable,disturbingEnable,
				gamblingEnable,illegalEnable,illegalDownloadsEnable,violenceEnable,hateEnable,weaponsEnable,datingEnable,
				timeShoppingandAuctionsEnable,videoStreamingServicesEnable,socialNetworksEnable,anonymizersEnable,unknownEnable);

			printf("%-6d ",i+1);
			printf("%-6d ",ContentFilterEnable);
			printf("%-15s ",json_object_get_string(json_object_object_get(obj, "Name")));
			printf("%-64s ",settingStr);
			printf("%-30s ",json_object_get_string(json_object_object_get(obj, "Description")));
			printf("\n");

		}
		else{
			printf("ERROR!\n");
		}

	}


}

zcfgRet_t zcfgFeDalContentFilterGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *ContentFilterJobj = NULL;
	struct json_object *pramJobj = NULL;

	int index = 0;
	//char TR181Path[64] = {0};
	int obj_MaxLength = 0;


	IID_INIT(objIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_CONTENT_FILTER, &objIid, &ContentFilterJobj) == ZCFG_SUCCESS){
		index++;

		obj_MaxLength = zcfgFeObjMaxLenGet(RDM_OID_CONTENT_FILTER);
		if(obj_MaxLength == 0)printf("RDM_OID_CONTENT_FILTER have on data!\n");
		//sprintf(TR181Path, "X_ZYXEL_ContentFilter.%u", objIid.idx[0]);

		pramJobj = json_object_new_object();
		replaceParam(pramJobj,"Name",ContentFilterJobj,"Name");
		replaceParam(pramJobj,"Description",ContentFilterJobj,"Description");

		replaceParam(pramJobj,"ContentFilterEnable",ContentFilterJobj,"Enable");
		replaceParam(pramJobj,"adultContent",ContentFilterJobj,"AdultContent");
		replaceParam(pramJobj,"drugs",ContentFilterJobj,"Drugs");
		replaceParam(pramJobj,"alcoholandTobacco",ContentFilterJobj,"AlcoholandTobacco");
		replaceParam(pramJobj,"disturbing",ContentFilterJobj,"Disturbing");
		replaceParam(pramJobj,"gambling",ContentFilterJobj,"Gambling");
		replaceParam(pramJobj,"illegal",ContentFilterJobj,"Illegal");
		replaceParam(pramJobj,"illegalDownloads",ContentFilterJobj,"IllegalDownloads");
		replaceParam(pramJobj,"violence",ContentFilterJobj,"Violence");
		replaceParam(pramJobj,"hate",ContentFilterJobj,"Hate");
		replaceParam(pramJobj,"weapons",ContentFilterJobj,"Weapons");
		replaceParam(pramJobj,"dating",ContentFilterJobj,"Dating");
		replaceParam(pramJobj,"timeShoppingandAuctions",ContentFilterJobj,"TimeShoppingandAuctions");
		replaceParam(pramJobj,"videoStreamingServices",ContentFilterJobj,"VideoStreamingServices");
		replaceParam(pramJobj,"socialNetworks",ContentFilterJobj,"SocialNetworks");
		replaceParam(pramJobj,"anonymizers",ContentFilterJobj,"Anonymizers");
		replaceParam(pramJobj,"unknown",ContentFilterJobj,"Unknown");

		if(json_object_object_get(Jobj, "Index")) {
			if(index == json_object_get_int(json_object_object_get(Jobj, "Index"))) {
				json_object_array_add(Jarray, pramJobj);
				if(ContentFilterJobj) zcfgFeJsonObjFree(ContentFilterJobj);
				break;
			}
			if(pramJobj) zcfgFeJsonObjFree(pramJobj);
		}
		else {
			json_object_array_add(Jarray, pramJobj);
		}
		if(ContentFilterJobj) zcfgFeJsonObjFree(ContentFilterJobj);
	}

	return ret;



}

zcfgRet_t zcfgFeDalContentFilterEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	struct json_object *ContentFilterJobj = NULL;
	struct json_object *pramJobjcopy = NULL;
	int objIndex;
	char *newName= NULL;
	char *oldName=NULL;
	char msg[30]={0};

	bool ContentFilterEnable;
	bool adultContentEnable;
	bool drugsEnable;
	bool alcoholandTobaccoEnable;
	bool disturbingEnable;
	bool gamblingEnable;
	bool illegalEnable;
	bool illegalDownloadsEnable;
	bool violenceEnable;
	bool hateEnable;
	bool weaponsEnable;
	bool datingEnable;
	bool timeShoppingandAuctionsEnable;
	bool videoStreamingServicesEnable;
	bool socialNetworksEnable;
	bool anonymizersEnable;
	bool unknownEnable;

	objIndex_t objIid={0};
	IID_INIT(objIid);
	objIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_CONTENT_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_CONTENT_FILTER, &objIid, &ContentFilterJobj)) != ZCFG_SUCCESS) {
		printf("Get json fail..\n");
		return ret;
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Name")) != NULL){
		newName = (char *)json_object_get_string(pramJobjcopy);
		oldName = (char *)json_object_get_string(json_object_object_get(ContentFilterJobj, "Name"));
		if(strcmp(newName,oldName)!=0){ // if newName != oldName
			ret = dalcmdParamCheck(&objIid, newName, RDM_OID_CONTENT_FILTER, "Name", "Name", msg);
			if(ret == ZCFG_SUCCESS){
				json_object_object_add(ContentFilterJobj, "Name", JSON_OBJ_COPY(pramJobjcopy));
				//if(pramJobjcopy) json_object_put(pramJobjcopy);	
			}
			else{
				printf("%s\n",msg);
				json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.ContentFilter.Input.pop_name_duplicate"));
				if(ContentFilterJobj) json_object_put(ContentFilterJobj);
				//if(pramJobjcopy) json_object_put(pramJobjcopy);
				return ret;
			}
		}
	}
	if((pramJobjcopy = json_object_object_get(Jobj, "Description")) != NULL){
		json_object_object_add(ContentFilterJobj, "Description", JSON_OBJ_COPY(pramJobjcopy));
		//if(pramJobjcopy) json_object_put(pramJobjcopy);
	}

	if((pramJobjcopy = json_object_object_get(Jobj, "ContentFilterEnable")) != NULL){	
		ContentFilterEnable = json_object_get_boolean(json_object_object_get(Jobj,"ContentFilterEnable"));
		json_object_object_add(ContentFilterJobj, "Enable",json_object_new_boolean(ContentFilterEnable));
	}
	
	adultContentEnable = json_object_get_boolean(json_object_object_get(Jobj,"adultContent"));
	json_object_object_add(ContentFilterJobj, "AdultContent",json_object_new_boolean(adultContentEnable));

	drugsEnable = json_object_get_boolean(json_object_object_get(Jobj,"drugs"));
	json_object_object_add(ContentFilterJobj, "Drugs",json_object_new_boolean(drugsEnable));

	alcoholandTobaccoEnable = json_object_get_boolean(json_object_object_get(Jobj,"alcoholandTobacco"));
	json_object_object_add(ContentFilterJobj, "AlcoholandTobacco",json_object_new_boolean(alcoholandTobaccoEnable));

	disturbingEnable = json_object_get_boolean(json_object_object_get(Jobj,"disturbing"));
	json_object_object_add(ContentFilterJobj, "Disturbing",json_object_new_boolean(disturbingEnable));

	gamblingEnable = json_object_get_boolean(json_object_object_get(Jobj,"gambling"));
	json_object_object_add(ContentFilterJobj, "Gambling",json_object_new_boolean(gamblingEnable));

	illegalEnable = json_object_get_boolean(json_object_object_get(Jobj,"illegal"));
	json_object_object_add(ContentFilterJobj, "Illegal",json_object_new_boolean(illegalEnable));

	illegalDownloadsEnable = json_object_get_boolean(json_object_object_get(Jobj,"illegalDownloads"));
	json_object_object_add(ContentFilterJobj, "IllegalDownloads",json_object_new_boolean(illegalDownloadsEnable));

	violenceEnable = json_object_get_boolean(json_object_object_get(Jobj,"violence"));
	json_object_object_add(ContentFilterJobj, "Violence",json_object_new_boolean(violenceEnable));

	hateEnable = json_object_get_boolean(json_object_object_get(Jobj,"hate"));
	json_object_object_add(ContentFilterJobj, "Hate",json_object_new_boolean(hateEnable));


	weaponsEnable = json_object_get_boolean(json_object_object_get(Jobj,"weapons"));	
	json_object_object_add(ContentFilterJobj, "Weapons",json_object_new_boolean(weaponsEnable));

	datingEnable = json_object_get_boolean(json_object_object_get(Jobj,"dating"));
	json_object_object_add(ContentFilterJobj, "Dating",json_object_new_boolean(datingEnable));

	timeShoppingandAuctionsEnable = json_object_get_boolean(json_object_object_get(Jobj,"timeShoppingandAuctions"));
	json_object_object_add(ContentFilterJobj, "TimeShoppingandAuctions",json_object_new_boolean(timeShoppingandAuctionsEnable));

	videoStreamingServicesEnable = json_object_get_boolean(json_object_object_get(Jobj,"videoStreamingServices"));	
	json_object_object_add(ContentFilterJobj, "VideoStreamingServices",json_object_new_boolean(videoStreamingServicesEnable));

	socialNetworksEnable = json_object_get_boolean(json_object_object_get(Jobj,"socialNetworks"));
	json_object_object_add(ContentFilterJobj, "SocialNetworks",json_object_new_boolean(socialNetworksEnable));

	anonymizersEnable = json_object_get_boolean(json_object_object_get(Jobj,"anonymizers"));
	json_object_object_add(ContentFilterJobj, "Anonymizers",json_object_new_boolean(anonymizersEnable));

	unknownEnable = json_object_get_boolean(json_object_object_get(Jobj,"unknown"));	
	json_object_object_add(ContentFilterJobj, "Unknown",json_object_new_boolean(unknownEnable));

	if((ret = zcfgFeObjJsonSet(RDM_OID_CONTENT_FILTER, &objIid, ContentFilterJobj, NULL)) != ZCFG_SUCCESS);{
	 	if(ContentFilterJobj) json_object_put(ContentFilterJobj);
		printf("Wrong jason set..\n");
		return ret;
	}

	if(ContentFilterJobj) json_object_put(ContentFilterJobj);
}


zcfgRet_t zcfgFeDalContentFilterAdd(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{

		zcfgRet_t ret = ZCFG_SUCCESS;
		objIndex_t objIid;
		struct json_object *ContentFilterJobj = NULL;
		struct json_object *pramJobj = NULL;
		struct json_object *pramJobjcopy = NULL;		
		char path[64] = {0};
		struct json_object *retJobj = NULL;

		bool ContentFilterEnable;
		bool adultContentEnable;
		bool drugsEnable;
		bool alcoholandTobaccoEnable;
		bool disturbingEnable;
		bool gamblingEnable;
		bool illegalEnable;
		bool illegalDownloadsEnable;
		bool violenceEnable;
		bool hateEnable;
		bool weaponsEnable;
		bool datingEnable;
		bool timeShoppingandAuctionsEnable;
		bool videoStreamingServicesEnable;
		bool socialNetworksEnable;
		bool anonymizersEnable;
		bool unknownEnable;		

		IID_INIT(objIid);
		pramJobj = json_object_new_object();
		if((ret = zcfgFeObjJsonAdd(RDM_OID_CONTENT_FILTER, &objIid, NULL)) != ZCFG_SUCCESS){
			if(ret == ZCFG_EXCEED_MAX_INS_NUM){
				printf("Cannot add entry since the maximum number of entries has been reached.\n");
			}
			if(pramJobj) json_object_put(pramJobj);				
			return ret;
		}

		if((ret = zcfgFeObjJsonGet(RDM_OID_CONTENT_FILTER, &objIid, &ContentFilterJobj)) != ZCFG_SUCCESS) {
			if(pramJobj) json_object_put(pramJobj);	
			printf("%s: Object retrieve fail\n", __FUNCTION__);
			return ret;
		}

		json_object_object_add(ContentFilterJobj, "Enable", json_object_new_boolean(true));
		json_object_object_add(ContentFilterJobj, "Name", JSON_OBJ_COPY(json_object_object_get(Jobj, "Name")));		
		if((pramJobjcopy = json_object_object_get(Jobj, "Description")) != NULL){
			json_object_object_add(ContentFilterJobj, "Description", JSON_OBJ_COPY(pramJobjcopy));
			//if(pramJobjcopy) json_object_put(pramJobjcopy);				
		}else{
			sprintf(path, "X_ZYXEL_ContentFilter.%d",objIid.idx[0]);
			json_object_object_add(ContentFilterJobj, "Description", json_object_new_string(path));	
			//if(pramJobjcopy) json_object_put(pramJobjcopy);
		}

		if((pramJobjcopy = json_object_object_get(Jobj, "ContentFilterEnable")) != NULL){
			ContentFilterEnable = json_object_get_boolean(json_object_object_get(Jobj,"ContentFilterEnable"));
			json_object_object_add(ContentFilterJobj, "Enable",json_object_new_boolean(ContentFilterEnable));
		}else
			json_object_object_add(ContentFilterJobj, "Enable",json_object_new_boolean(false));
			
		
		adultContentEnable = json_object_get_boolean(json_object_object_get(Jobj,"adultContent"));
		json_object_object_add(ContentFilterJobj, "AdultContent",json_object_new_boolean(adultContentEnable));

		drugsEnable = json_object_get_boolean(json_object_object_get(Jobj,"drugs"));
		json_object_object_add(ContentFilterJobj, "Drugs",json_object_new_boolean(drugsEnable));

		alcoholandTobaccoEnable = json_object_get_boolean(json_object_object_get(Jobj,"alcoholandTobacco"));
		json_object_object_add(ContentFilterJobj, "AlcoholandTobacco",json_object_new_boolean(alcoholandTobaccoEnable));
		
		disturbingEnable = json_object_get_boolean(json_object_object_get(Jobj,"disturbing"));
		json_object_object_add(ContentFilterJobj, "Disturbing",json_object_new_boolean(disturbingEnable));
		
		gamblingEnable = json_object_get_boolean(json_object_object_get(Jobj,"gambling"));
		json_object_object_add(ContentFilterJobj, "Gambling",json_object_new_boolean(gamblingEnable));
		
		illegalEnable = json_object_get_boolean(json_object_object_get(Jobj,"illegal"));
		json_object_object_add(ContentFilterJobj, "Illegal",json_object_new_boolean(illegalEnable));
		
		illegalDownloadsEnable = json_object_get_boolean(json_object_object_get(Jobj,"illegalDownloads"));
		json_object_object_add(ContentFilterJobj, "IllegalDownloads",json_object_new_boolean(illegalDownloadsEnable));
		
		violenceEnable = json_object_get_boolean(json_object_object_get(Jobj,"violence"));
		json_object_object_add(ContentFilterJobj, "Violence",json_object_new_boolean(violenceEnable));
		
		hateEnable = json_object_get_boolean(json_object_object_get(Jobj,"hate"));
		json_object_object_add(ContentFilterJobj, "Hate",json_object_new_boolean(hateEnable));		
		
		weaponsEnable = json_object_get_boolean(json_object_object_get(Jobj,"weapons"));	
		json_object_object_add(ContentFilterJobj, "Weapons",json_object_new_boolean(weaponsEnable));
		
		datingEnable = json_object_get_boolean(json_object_object_get(Jobj,"dating"));
		json_object_object_add(ContentFilterJobj, "Dating",json_object_new_boolean(datingEnable));
		
		timeShoppingandAuctionsEnable = json_object_get_boolean(json_object_object_get(Jobj,"timeShoppingandAuctions"));
		json_object_object_add(ContentFilterJobj, "TimeShoppingandAuctions",json_object_new_boolean(timeShoppingandAuctionsEnable));
		
		videoStreamingServicesEnable = json_object_get_boolean(json_object_object_get(Jobj,"videoStreamingServices"));	
		json_object_object_add(ContentFilterJobj, "VideoStreamingServices",json_object_new_boolean(videoStreamingServicesEnable));
		
		socialNetworksEnable = json_object_get_boolean(json_object_object_get(Jobj,"socialNetworks"));
		json_object_object_add(ContentFilterJobj, "SocialNetworks",json_object_new_boolean(socialNetworksEnable));
		
		anonymizersEnable = json_object_get_boolean(json_object_object_get(Jobj,"anonymizers"));
		json_object_object_add(ContentFilterJobj, "Anonymizers",json_object_new_boolean(anonymizersEnable));
		
		unknownEnable = json_object_get_boolean(json_object_object_get(Jobj,"unknown"));	
		json_object_object_add(ContentFilterJobj, "Unknown",json_object_new_boolean(unknownEnable));
		
		if((ret = zcfgFeObjJsonSet(RDM_OID_CONTENT_FILTER, &objIid, ContentFilterJobj, NULL)) != ZCFG_SUCCESS){
			if(ContentFilterJobj) json_object_put(ContentFilterJobj);
			if(pramJobj) json_object_put(pramJobj);	
			printf("%s: Object retrieve fail\n", __FUNCTION__);			
			return ret;
		}
		if(Jarray != NULL){
			sprintf(path, "X_ZYXEL_ContentFilter.%d",objIid.idx[0]);
			retJobj = json_object_new_object();
			Jadds(retJobj, "path", path);
			json_object_array_add(Jarray, retJobj);
		}

	if(ContentFilterJobj) json_object_put(ContentFilterJobj);
	if(pramJobj) json_object_put(pramJobj);	
//	if(retJobj) json_object_put(retJobj);
	return ret;
}
zcfgRet_t zcfgFeDalContentFilterDelete(struct json_object *Jobj, char *replyMsg)
{


	zcfgRet_t ret = ZCFG_SUCCESS;
	//objIndex_t objIid={0};
	struct json_object *cfObjIidArray = NULL;
	uint8_t iidIdx =0;
#if 0
	int objIndex;
#endif
	objIndex_t cfObjIid = {0};
	struct json_object *cfObj = NULL;

	IID_INIT(cfObjIid);
	cfObjIidArray = json_object_object_get(Jobj, "ConteFilterObjIid");
	if(cfObjIidArray != NULL){//request form GUI
		for(iidIdx=0; iidIdx < json_object_array_length(cfObjIidArray); iidIdx++){
			cfObjIid.idx[iidIdx] = json_object_get_int( json_object_array_get_idx(cfObjIidArray, iidIdx));
			if(cfObjIid.idx[iidIdx] != 0)
				cfObjIid.level = iidIdx+1;
		}
	}else{
		cfObjIid.idx[0] = json_object_get_int(json_object_object_get(Jobj, "Index"));
		cfObjIid.level = 1;
	}	
	

#if 0
	ret = convertIndextoIid(objIndex, &objIid, RDM_OID_CONTENT_FILTER, NULL, NULL, replyMsg);
	if(ret != ZCFG_SUCCESS){
		printf("Wrong index..\n");
		return ret;
	}
#endif	
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CONTENT_FILTER, &cfObjIid, &cfObj)) != ZCFG_SUCCESS){
		if(replyMsg)
			sprintf(replyMsg, "Can't find --Index %d", cfObjIid.idx[0]);
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.ContentFilter.delete_pcp_sch_err_msg"));
		if(cfObj) json_object_put(cfObj);
		return ret;
	}

	ret = zcfgFeObjJsonDel(RDM_OID_CONTENT_FILTER, &cfObjIid, NULL);
	if(ret == ZCFG_DELETE_REJECT){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.ContentFilter.delete_pcp_sch_err_msg"));
		printf("%s %s %d \n",json_object_to_json_string(Jobj),__func__,__LINE__);
		if(cfObj) json_object_put(cfObj);		
		return ret;
	}
	if(cfObj) json_object_put(cfObj);
	ret = ZCFG_SUCCESS;
	return ret;

}

zcfgRet_t zcfgFeDalContentFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){

	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalContentFilterEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "POST")) {
		ret = zcfgFeDalContentFilterAdd(Jobj, Jarray, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalContentFilterDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalContentFilterGet(Jobj, Jarray, NULL);
	}
	else{
		printf("Unknown method:%s\n", method);
	}
	return ret;
}

