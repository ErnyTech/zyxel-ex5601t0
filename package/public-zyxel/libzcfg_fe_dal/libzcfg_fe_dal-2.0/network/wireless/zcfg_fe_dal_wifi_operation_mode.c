#ifdef ZYXEL_ECONET_OPERATION_MODE
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

int var_OPM = 0;
int var_Band = 0;
int var_Connect = 0;
int flag_scan = 0;

dal_param_t WIFI_Opm_param[]=
{
	{"operationMode",				dalType_int,		0,	0,	NULL,	NULL,	0},
	{"Band",						dalType_int,		0,	0,	NULL,	NULL,	0},
	{"SSID",						dalType_string,		0,	0,	NULL,	NULL,	0},
	{"FrequencyBand",				dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Encryption",					dalType_string,		0,	0,	NULL,	NULL,	0},
	{"wifiStrength",				dalType_string,		0,	0,	NULL,	NULL,	0},
	{"Connect",						dalType_boolean,	0,	0,	NULL,	NULL,	NULL}

};

/* Remove header and trailing space  */
static char* ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static char* rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char* trim(char *s)
{
    return rtrim(ltrim(s));
}

int zcfgFeDalWifiOpmUpdateApClient(char* ssid){
	int connect = 0;
	FILE *fp = NULL;
	char buff[64];
	char SSID[32]={0};
	int SSIDStatus = 0;

	/* load /tmp/apclistatus get wifi ssid */
	fp = fopen("/tmp/apclistatus","r");
	if(fp != NULL){
		while(fgets(buff, 64, fp) != NULL){
			if (sscanf( buff, "apcli0 status %d %s", &SSIDStatus, SSID))
			{
				connect = SSIDStatus;
				if(SSIDStatus)
					strcpy(ssid, SSID);
			}
		}
		fclose(fp);
	}

	return connect;
}

zcfgRet_t zcfgFeDalWifiOpmGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *operationMode = NULL;
	struct json_object *band = NULL;
	struct json_object *connect = NULL;
	struct json_object *pramJobj = NULL;
	struct json_object *pramJobj1 = NULL;
	struct json_object *pramJobj2 = NULL;
	struct json_object *pramJobj3 = NULL;
	struct json_object *allobj = NULL;
	struct json_object *wifiListtmpObj = NULL;
	allobj = json_object_new_object();
	wifiListtmpObj = json_object_new_array();
	char connectSSID[32] = {0};

	IID_INIT(objIid);
	if(zcfgFeObjJsonGet(RDM_OID_OPERATION_MODE, &objIid, &operationMode) == ZCFG_SUCCESS) {
		var_OPM = json_object_get_int(json_object_object_get(operationMode, "Mode"));
		var_Band = json_object_get_int(json_object_object_get(operationMode, "Band"));
		flag_scan = json_object_get_int(json_object_object_get(operationMode, "Scan"));
		zcfgFeJsonObjFree(operationMode);
	}

	if(var_OPM==2)
	{
		var_Connect = zcfgFeDalWifiOpmUpdateApClient(connectSSID);
		//printf("zcfgFeDalWifiOpmGet: var_Connect=%d\n",var_Connect);
		/*if(var_Connect)
			printf("zcfgFeDalWifiOpmGet: connectSSID=%s\n",connectSSID);*/
	}

	operationMode = json_object_new_int(var_OPM);
	band = json_object_new_int(var_Band);
	connect = json_object_new_int(var_Connect);

	pramJobj = json_object_new_object();
	json_object_object_add(pramJobj, "operationMode", operationMode);
	if(var_OPM == 2){
		json_object_object_add(pramJobj, "Band", band);
	}
	json_object_object_add(allobj, "OP_Mode", pramJobj);

	/* load /tmp/apcliWireless get wifi ssid */
	{
		FILE *fp = NULL;
		char buff[256];
		int No=0, Ch=0;
		char SSID[33]={0}, Security[32]={0}, Siganl[16]={0};
		char SSIDStr[33]={0}, SecurityStr[32]={0}, SiganlStr[16]={0};
		char BandStr[32]={0}, BandBuf[32]={0};
		char *SSIDPtr = NULL;
		char *SecurityPtr = NULL;
		char *SiganlPtr = NULL;

		if(var_Band==0)
		{
			strcpy(BandStr,"2.4GHz");
			strcpy(BandBuf,"11b/g/n");
		}
		else
		{
			strcpy(BandStr,"5GHz");
			strcpy(BandBuf,"11a/n");
		}
		fp = fopen("/tmp/apcliWireless","r");
		if(fp != NULL){
			while(fgets(buff, 256, fp) != NULL)
			{
				if(strstr(buff, BandBuf))
				{
					strncpy(SSID,buff+8,32);
					SSIDPtr = trim(SSID);
					strcpy(SSIDStr,SSIDPtr);
					strncpy(Security,buff+61,22);
					SecurityPtr = trim(Security);
					strcpy(SecurityStr,SecurityPtr);
					strncpy(Siganl,buff+91,8);
					SiganlPtr = trim(Siganl);
					strcpy(SiganlStr,SiganlPtr);
					if (strcmp(SSIDStr,""))
					{
						pramJobj1 = json_object_new_object();
						json_object_object_add(pramJobj1, "SSID", json_object_new_string(SSIDStr));
						json_object_object_add(pramJobj1, "FrequencyBand", json_object_new_string(BandStr));
						json_object_object_add(pramJobj1, "Encryption", json_object_new_string(SecurityStr));
						json_object_object_add(pramJobj1, "wifiStrength", json_object_new_string(SiganlStr));
						json_object_array_add(wifiListtmpObj, pramJobj1);
			        }

				}
			}
			fclose(fp);
		}
		json_object_object_add(allobj, "wifiList", wifiListtmpObj);
	}

	pramJobj3 = json_object_new_object();
	json_object_object_add(pramJobj3, "Connect", connect);
	json_object_object_add(pramJobj3, "Connect_SSID", json_object_new_string(connectSSID));
	json_object_object_add(allobj, "currentconnect", pramJobj3);

	json_object_array_add(Jarray, allobj);

	return ret;
}

zcfgRet_t zcfgFeDalWifiOpmEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t objIid;
	struct json_object *operationMode = NULL;
	struct json_object *band = NULL;
	struct json_object *currentconnect = NULL;
	struct json_object *scan = NULL;
	struct json_object *WifiApOpmJobj = NULL;
	char *var_SSID = NULL;
	char *var_Encryption = NULL;
	char *password = NULL;
	char authMode[32]={0};
	char encrypType[32]={0};

	if(operationMode = json_object_object_get(Jobj, "operationMode")){
		var_OPM = json_object_get_int(operationMode);
	}
	if(band = json_object_object_get(Jobj, "Band")){
		var_Band = json_object_get_int(band);
	}
	/*if(currentconnect = json_object_object_get(json_object_object_get(Jobj, "Connect"),"Index")){
		var_Connect = json_object_get_int(currentconnect);
	}
	else{
		var_Connect = 0;
	}*/
	if(scan = json_object_object_get(Jobj, "scan")){
		flag_scan = json_object_get_int(scan);
	}
	else {
		flag_scan = 0;
	}

	password = json_object_get_string(json_object_object_get(Jobj, "password"));
	var_SSID = json_object_get_string(json_object_object_get(json_object_object_get(Jobj, "Connect"),"SSID"));
	var_Encryption = json_object_get_string(json_object_object_get(json_object_object_get(Jobj, "Connect"),"Encryption"));

	if(var_Encryption!=NULL)
	{
		char *tocken = NULL;
		char tmp[32]={0};
		memcpy(tmp, var_Encryption, sizeof(tmp));
		tocken = strtok(tmp, "/");
		strcpy(authMode,tocken);
		if(tocken){
			tocken = strtok(NULL, "/");
			strcpy(encrypType,tocken);
		}
	}

	IID_INIT(objIid);
	if(zcfgFeObjJsonGet(RDM_OID_OPERATION_MODE, &objIid, &operationMode) == ZCFG_SUCCESS) {

		json_object_object_add(operationMode, "Mode", json_object_new_int(var_OPM));
		json_object_object_add(operationMode, "Band", json_object_new_int(var_Band));
		json_object_object_add(operationMode, "Scan", json_object_new_int(flag_scan));
		if(var_SSID!=NULL && var_Encryption!=NULL)
		{
			json_object_object_add(operationMode, "SSID", json_object_new_string(var_SSID));
			json_object_object_add(operationMode, "AuthMode", json_object_new_string(authMode));
			json_object_object_add(operationMode, "EncrypType", json_object_new_string(encrypType));
			json_object_object_add(operationMode, "WpaPskKey", json_object_new_string(password));
		}
		ret = zcfgFeObjJsonBlockedSet(RDM_OID_OPERATION_MODE, &objIid, operationMode, NULL);
		if(operationMode) zcfgFeJsonObjFree(operationMode);
	}

	return ret;
}

zcfgRet_t zcfgFeDalWifiOpm(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalWifiOpmEdit(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalWifiOpmGet(Jobj, Jarray, NULL);
	}
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
