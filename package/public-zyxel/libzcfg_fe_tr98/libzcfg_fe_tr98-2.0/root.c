#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcmd_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_msg.h"
#include "zcfg_debug.h"
#include "zcfg_fe_tr98.h"
#include "root_parameter.h"

extern tr98Object_t tr98Obj[];

/* InternetGatewayDevice */
zcfgRet_t rootObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag)
{
	zcfgRet_t ret;
	objIndex_t rootIid;
	struct json_object *rootObj = NULL;
	struct json_object *paramValue = NULL;
	tr98Parameter_t *paramList = NULL;

	printf("Enter %s\n", __FUNCTION__);

	IID_INIT(rootIid);
	if((ret = feObjJsonGet(RDM_OID_DEVICE, &rootIid, &rootObj, updateFlag)) != ZCFG_SUCCESS)
		return ret;

	/*fill up tr98 root object*/
	*tr98Jobj = json_object_new_object();
	paramList = tr98Obj[handler].parameter;
	while(paramList->name != NULL) {
		
		paramValue = json_object_object_get(rootObj, paramList->name);
		if(paramValue != NULL) {
			json_object_object_add(*tr98Jobj, paramList->name, JSON_OBJ_COPY(paramValue));
			paramList++;
			continue;
		}
		else{ /*fill empty value*/
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
            	if(!strcmp(paramList->name, "LANDeviceNumberOfEntries")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(1));
			    }
				else if(!strcmp(paramList->name, "WANDeviceNumberOfEntries")) {
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_int(2));
			    }
				else
					json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
#else
			json_object_object_add(*tr98Jobj, paramList->name, json_object_new_string(""));
#endif
			paramList++;
			continue;		
		}
		/*Not defined in tr181, give it a default value*/
		printf("Can't find parameter %s in TR181\n", paramList->name);
		paramList++;
	}
	json_object_put(rootObj);

	return ZCFG_SUCCESS;
}
/* InternetGatewayDevice */

