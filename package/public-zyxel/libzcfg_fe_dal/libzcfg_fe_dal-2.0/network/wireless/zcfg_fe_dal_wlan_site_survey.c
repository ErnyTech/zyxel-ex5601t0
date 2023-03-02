#include <ctype.h>
#include <unistd.h>
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
#include "zos_api.h"

//#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

dal_param_t WLAN_SITE_SURVEY_param[]={
	{"Enable",						dalType_boolean,	0,	0,	NULL,	NULL,	dal_Edit},
	{NULL,		0,	0,	0,	NULL,	NULL,	0},
};

void zcfgFeDalShowWlanSiteSurvey(struct json_object *Jarray){						
	char sysCmd[64] = {0};
	char cmd[512] = {0};
	
	snprintf(sysCmd, sizeof(sysCmd), "dmesg -n 1; iwpriv ra0 set SiteSurvey=1 > /dev/null");
    ZOS_SYSTEM(sysCmd);
	
	sleep(5);
	
	snprintf(sysCmd, sizeof(sysCmd), "dmesg -n 7; iwpriv ra0 get_site_survey > /tmp/site_survey_ra0");
    ZOS_SYSTEM(sysCmd);
	
    FILE *fp = fopen("/tmp/site_survey_ra0", "r");
    if(fp) {
		while(fgets(cmd, sizeof(cmd)-1, fp) != NULL) { 
			printf("%s", cmd);
			memset(cmd, 0, sizeof(cmd));
		}
		fclose(fp);
    }	
}

zcfgRet_t zcfgFeDalWlanSiteSurveyGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalWlanSiteSurveyEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char sysCmd[32] = {0};
	
	snprintf(sysCmd, sizeof(sysCmd), "iwpriv ra0 set SiteSurvey=1");
    ZOS_SYSTEM(sysCmd);
	return ret;
}

zcfgRet_t zcfgFeDalWlanSiteSurvey(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	//if(!strcmp(method, "POST"))
		//ret = zcfgFeDalWifiOthersAdd(Jobj, NULL);
	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalWlanSiteSurveyEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalWlanSiteSurveyGet(Jobj, Jarray, replyMsg);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


