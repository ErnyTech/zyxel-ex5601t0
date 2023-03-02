#include <ctype.h>
#include <unistd.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <stdio.h>
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

dal_param_t WIFI_CLIENT_LIST_param[]={
	{"Enable",						dalType_boolean,	0,	0,	NULL,	NULL,	dal_Edit},
	{NULL,		0,	0,	0,	NULL,	NULL,	0},
};

void zcfgFeDalShowWifiClientList(struct json_object *Jarray){						
	char sysCmd[128] = {0};
	char cmd[512] = {0};
	
    //FILE *fp = popen("iwpriv ra0 show stainfo", "r");
    snprintf(sysCmd, sizeof(sysCmd), "iwpriv ra0 show zystainfo");
    ZOS_SYSTEM(sysCmd);
	sleep(2);
	
    FILE *fp = fopen("/tmp/ecn_assoc_list_24g", "r");
    if(fp) {
		while(fgets(cmd, sizeof(cmd)-1, fp) != NULL) { 
			printf("%s", cmd);
			memset(cmd, 0, sizeof(cmd));
		}
		pclose(fp);
    }	
}

zcfgRet_t zcfgFeDalWifiClientListGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalWifiClientList(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	
	if(!strcmp(method, "GET"))
		ret = zcfgFeDalWifiClientListGet(Jobj, Jarray, replyMsg);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}


