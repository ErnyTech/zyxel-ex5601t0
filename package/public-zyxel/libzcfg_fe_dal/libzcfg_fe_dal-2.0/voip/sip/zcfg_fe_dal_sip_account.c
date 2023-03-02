
#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_fe_schema.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_webgui_flag.h"
#include "zcfg_fe_dal_voip.h"

#define IVR_PLAY_INDEX_ONLY_SHOW_DEFAULT	0

dal_param_t VOIP_LINE_param[]={
	{"Index",								dalType_int,	0,	0,	NULL,	NULL,		dal_Edit|dal_Delete},
	{"ServiceProviderName",					dalType_VoipProvider,	0,	0,	NULL,	NULL,		dal_Add},	// add new type for service provider
	{"EditPath",							dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},	//	need to deal
	{"AddPath",								dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},	//	need to deal
	{"Enable",								dalType_boolean,0,	0,	NULL,	NULL,		0},
	{"SIP_Account_Number",					dalType_string,	0,	0,	NULL,	NULL,		0},
	//{"DirectoryNumber",						dalType_string,	0,	0,	NULL},
	{"PhyReferenceList",					dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},	// no support 20170524 Rex
	{"AuthUserName",						dalType_string,	0,	0,	NULL,	NULL,		0},
	{"AuthPassword",						dalType_string,	0,	0,	NULL,	NULL,		0},
	{"Url_Type",							dalType_string,	0,	0,	NULL,	"SIP|TEL",	0},
	{"primaryCodec",						dalType_string,	0,	0,	NULL,	"G711MuLaw|G711ALaw|G729|G726_24|G726_32|G722",		0},
	{"secondarydCodec",						dalType_string,	0,	0,	NULL,	"G711MuLaw|G711ALaw|G729|G726_24|G726_32|G722|None",	0},
	{"thirdCodec",							dalType_string,	0,	0,	NULL,	"G711MuLaw|G711ALaw|G729|G726_24|G726_32|G722|None",	0},
	{"TransmitGain",						dalType_string,	0,	0,	NULL,	"Minimum|Middle|Maximum",	0},
	{"ReceiveGain",							dalType_string,	0,	0,	NULL,	"Minimum|Middle|Maximum",	0},
	{"EchoCancellationEnable",				dalType_boolean,0,	0,	NULL,	NULL,		0},
	{"SilenceSuppression",					dalType_boolean,0,	0,	NULL,	NULL,		0},
	
	{"AnonymousCallEnable",					dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallTransferEnable",					dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallWaitingEnable",					dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallWaitingRejectTime",				dalType_int,	10,	60,	NULL,	NULL,		dalcmd_Forbid},
	//{"X_ZYXEL_CallWaitingRejectTime",		dalType_int,	0,	0,	NULL},
	{"CallForwardUnconditionalEnable",		dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallForwardUnconditionalNumber",		dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},		//need to add replyMsg
	{"CallForwardOnBusyEnable",				dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallForwardOnBusyNumber",				dalType_string,0,	0,	NULL,	NULL,		dalcmd_Forbid},		//need to add replyMsg
	{"CallForwardOnNoAnswerEnable",			dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"CallForwardOnNoAnswerNumber",			dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},		//need to add replyMsg
	{"CallForwardOnNoAnswerRingCount",		dalType_int,	10,	119,NULL,	NULL,		dalcmd_Forbid},		//for add case if CallForwardOnNoAnswerEnable == false the need to give a default value 20
	{"AnonymousCallBlockEnable",			dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"DoNotDisturbEnable",					dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"MWIEnable",							dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"MwiExpireTime",						dalType_int,	0,	0,	NULL,	NULL,		dalcmd_Forbid},		//for add case if MWIEnable == false the need to give a default value 3600
//	Hot Line / Warm Line Number:
	{"HotLine_WarmLine_Enable",				dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"HotLineEnable",						dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"WarmLineNumber",						dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"HotLineNumber",						dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"TimerFirstDigitWarmLine",				dalType_int,	5,	300,	NULL,	NULL,		dalcmd_Forbid},	//for add case if TimerFirstDigitWarmLine == false the need to give a default value 20
	
	
	{"MissedCallEmailEnable",				dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"MissedCallEmailEvent",				dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"RemoteRingbackTone",					dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"RemoteRingbackToneIndex",				dalType_string,	0,	0,	NULL,	"Default|No1|No2|No3|No4|No5",		dalcmd_Forbid},
	{"MusicOnHoldTone",						dalType_boolean,0,	0,	NULL,	NULL,		dalcmd_Forbid},
	{"MusicOnHoldToneIndex",				dalType_string,	0,	0,	NULL,	"Default|No1|No2|No3|No4|No5",		dalcmd_Forbid},
	{"MailAccount",							dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},	// add a type for email show to choose
	//{"SericeRef",							dalType_string,	0,	0,	NULL,	NULL,		NULL},
	{"Missed_Call_Email_Title",				dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},
	//{"EmailSubject",						dalType_string,	0,	0,	NULL,	NULL,		NULL},
	{"Send_Notification_to_Email",			dalType_string,	0,	0,	NULL,	NULL,		dalcmd_Forbid},
	//{"EmailTo",								dalType_string,	0,	0,	NULL,	NULL,		NULL},
	{"VoiceJBMode",							dalType_string,	0,	0,	NULL,	NULL,	0},
	{"VoiceJBMFixedValue",					dalType_int,	0,	0,	NULL,	NULL,	0},
	{"VoiceJBMin",							dalType_int,	0,	0,	NULL,	NULL,	0},
	{"VoiceJBMax",							dalType_int,	0,	0,	NULL,	NULL,	0},
	{"DataJBMode",							dalType_string,	0,	0,	NULL,	NULL,	0},
	{"DataJBTarget",						dalType_int,	0,	0,	NULL,	NULL,	0},
	{NULL,									0,				0,	0,	NULL},
};


int	profileId;
bool voicelineEnable;
const char* enable;
const char* DirectoryNumber;
const char* PhyReferenceList;
const char* AuthUserName;
const char* AuthPassword;
const char hiddenAuthPassword[] = "********" ;
const char* X_ZYXEL_Url_Type;
bool CallTransferEnable;
bool CallWaitingEnable;
unsigned int X_ZYXEL_CallWaitingRejectTime;
bool CallForwardUnconditionalEnable;
const char* CallForwardUnconditionalNumber;
const char* CallForwardOnBusyNumber;
bool CallForwardOnBusyEnable;
bool CallForwardOnNoAnswerEnable;
const char* CallForwardOnNoAnswerNumber;
unsigned int CallForwardOnNoAnswerRingCount;
bool AnonymousCallEnable;
bool AnonymousCallBlockEnable;
bool DoNotDisturbEnable;
bool MWIEnable;
bool X_ZYXEL_WarmLineEnable;
bool X_ZYXEL_HotLineEnable;
const char* X_ZYXEL_HotLineNum;
const char* X_ZYXEL_WarmLineNumber;
unsigned int X_ZYXEL_TimerFirstDigitWarmLine;
unsigned int X_ZYXEL_MwiExpireTime;
bool X_ZYXEL_MissedCallEmailEnable;
char* X_ZYXEL_MissedCallEmailEvent;
bool X_ZYXEL_RemoteRingbackTone;
unsigned int X_ZYXEL_RemoteRingbackToneIndex;
bool X_ZYXEL_MusicOnHoldTone;
unsigned int X_ZYXEL_MusicOnHoldToneIndex;
bool X_ZYXEL_CCBSEnable;
bool EchoCancellationEnable;
int TransmitGain;
int ReceiveGain;
char* X_ZYXEL_VoiceJBMode;
int X_ZYXEL_VoiceJBMin;
int X_ZYXEL_VoiceJBMax;
int X_ZYXEL_VoiceJBTarget;
char* X_ZYXEL_DataJBMode;
int X_ZYXEL_DataJBTarget;
const char* primaryCodec;
const char* secondarydCodec;
const char* thirdCodec;
const char* SericeRef;
const char* EmailAddress;
const char* EmailSubject;
const char* EmailTo;
const char* Event;
bool SilenceSuppression;
bool X_ZYXEL_ComfortNoise;
const char* profileName;
unsigned int sipAccountIdx;
char* Codec;
unsigned int priority;
bool CodecEnable;

unsigned int flag1;

struct json_object *phyIntfObj;
struct json_object *lineObj;
struct json_object *profileObj;
struct json_object *profNumPlanpInfoObj;
struct json_object *lineSipObj;
struct json_object *lineCallingFeatureObj;
struct json_object *lineProcessingObj;
struct json_object *lineCodecListObj;
struct json_object *lineMailCfgObj;
struct json_object *lineMailserviceObj;
struct json_object *lineSchemaObj;
struct json_object *multiJobj;
struct json_object *guiCustObj;
struct json_object *singleLineobj = NULL;

objIndex_t phyIntfIid = {0};
objIndex_t lineIid = {0};
objIndex_t lineSipIid = {0};
objIndex_t lineCallingFeatureIid = {0};
objIndex_t lineProcessingIid = {0};
objIndex_t lineCodecListIid = {0};
objIndex_t lineMailCfgIid = {0};
objIndex_t lineMailServiceIid = {0};
objIndex_t flagIid = {0};

void zcfgFeDalShowVoipLine(struct json_object *Jarray){
	int len = 0, i;
	struct json_object *obj = NULL, *obj_idx = NULL;
	int remoteRingback = 0, musicTone = 0;
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	len = json_object_array_length(Jarray);
	obj_idx = json_object_array_get_idx(Jarray, 0);
	if(len == 1 && json_object_get_boolean(json_object_object_get(obj_idx, "ShowDetail"))){
		obj = json_object_array_get_idx(Jarray, 0);
		printf("%-35s %s \n","SIP Account Selection:", json_object_get_string(json_object_object_get(obj, "SipAccount")));
		printf("%-35s %s \n","SIP Account Associated with:", json_object_get_string(json_object_object_get(obj, "Name")));
		printf("%-35s %s \n","SIP Account:", json_object_get_string(json_object_object_get(obj, "Enable")));
		printf("%-35s %s \n","SIP Account Number:", json_object_get_string(json_object_object_get(obj, "DirectoryNumber")));
		printf("%-20s %s \n","Username:", json_object_get_string(json_object_object_get(obj, "AuthUserName")));
#ifdef IAAAA_CUSTOMIZATION
		printf("%-20s %s \n","Password:", json_object_get_string(json_object_object_get(obj, "AuthPassword")));
#else
		printf("%-20s %s \n","Password:", "********");
#endif
		if(!strcmp("URL_Tel",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Url_Type"))))
			printf("%-20s %s \n","URL Type:", "TEL");
		else if(!strcmp("URL_Sip",json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Url_Type"))))
			printf("%-20s %s \n","URL Type:", "SIP");
		printf("%-35s %s \n","Primary Compression Type:", json_object_get_string(json_object_object_get(obj, "primaryCodec")));
		printf("%-35s %s \n","Secondary Compression Type:", json_object_get_string(json_object_object_get(obj, "secondarydCodec")));
		printf("%-35s %s \n","Third Compression Type:", json_object_get_string(json_object_object_get(obj, "thirdCodec")));
		if(json_object_get_int(json_object_object_get(obj, "TransmitGain")) == -3)
			printf("%-35s %s \n","Speaking Volume Control:", "Minimum");
		else if(json_object_get_int(json_object_object_get(obj, "TransmitGain")) == 0)
			printf("%-35s %s \n","Speaking Volume Control:", "Middle");
		else if(json_object_get_int(json_object_object_get(obj, "TransmitGain")) == 6)
			printf("%-35s %s \n","Speaking Volume Control:", "Maximum");
		if(json_object_get_int(json_object_object_get(obj, "ReceiveGain")) == -3)
			printf("%-35s %s \n","Speaking Volume Control:", "Minimum");
		else if(json_object_get_int(json_object_object_get(obj, "ReceiveGain")) == 0)
			printf("%-35s %s \n","Speaking Volume Control:", "Middle");
		else if(json_object_get_int(json_object_object_get(obj, "ReceiveGain")) == 6)
			printf("%-35s %s \n","Speaking Volume Control:", "Maximum");
		printf("%-40s %s \n","Enable G.168 (Echo Cancellation)", json_object_get_string(json_object_object_get(obj, "EchoCancellationEnable")));
		printf("%-40s %s \n","Enable VAD (Voice Active Detector)", json_object_get_string(json_object_object_get(obj, "SilenceSuppression")));
		printf("%-40s %s \n","Send Caller ID", json_object_get_string(json_object_object_get(obj, "AnonymousCallEnable")));
		printf("%-40s %s \n","Enable Call Transfer", json_object_get_string(json_object_object_get(obj, "CallTransferEnable")));
		printf("%-40s %s \n","Enable Call Waiting", json_object_get_string(json_object_object_get(obj, "CallWaitingEnable")));
		printf("%-40s %lld \n","Call Waiting Reject Timer(sec):", json_object_get_int(json_object_object_get(obj, "X_ZYXEL_CallWaitingRejectTime")));
		printf("%-40s %s \n","Enable Do Not Disturb (DND)", json_object_get_string(json_object_object_get(obj, "DoNotDisturbEnable")));
		printf("%-40s %s \n","Active Incoming Anonymous Call Block", json_object_get_string(json_object_object_get(obj, "AnonymousCallBlockEnable")));
		if(json_object_get_boolean(json_object_object_get(obj, "MWIEnable"))){
			printf("%-40s %s \n","Enable MWI", json_object_get_string(json_object_object_get(obj, "MWIEnable")));
			printf("%-40s %s \n","MWI Subscribe Expiration Time(sec) :", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_MwiExpireTime")));
		}
		else
			printf("%-40s %s \n","Enable MWI", json_object_get_string(json_object_object_get(obj, "MWIEnable")));
		if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_WarmLineEnable"))){
			printf("%-40s %s \n","Warm Line", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_WarmLineEnable")));
			printf("%-40s %s \n","Warm Line Number:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_WarmLineNumber")));
			printf("%-40s %s \n","Warm Line Timer(sec) :", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_TimerFirstDigitWarmLine")));
		}
		else if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_HotLineEnable"))){
			printf("%-40s %s \n","Hot Line", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_HotLineEnable")));
			printf("%-40s %s \n","Hot Line Number:", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_HotLineNum")));
		}
		else{
			printf("%-40s %s \n","Warm Line", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_WarmLineEnable")));
			printf("%-40s %s \n","Hot Line", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_HotLineEnable")));
		}
		if(json_object_get_boolean(json_object_object_get(obj, "X_ZYXEL_MissedCallEmailEnable"))){
			printf("%-40s %s \n","Enable Missed Call Email Notification", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_MissedCallEmailEnable")));
			printf("%-40s %s \n","Mail Account :", json_object_get_string(json_object_object_get(obj, "SericeRef")));
			printf("%-40s %s \n","Send Notification to E-mail :", json_object_get_string(json_object_object_get(obj, "EmailTo")));
			printf("%-40s %s \n","Missed Call E-mail Title :", json_object_get_string(json_object_object_get(obj, "EmailSubject")));
		}
		else
			printf("%-40s %s \n","Enable Missed Call Email Notification", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_MissedCallEmailEnable")));
		printf("%-40s %s \n","Early Media", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_RemoteRingbackTone")));
		remoteRingback = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_RemoteRingbackToneIndex"));
		if(remoteRingback == 66)
			printf("%-40s %s \n","IVR Play Index :", "Default");
		else if(remoteRingback == 0)
			printf("%-40s %s \n","IVR Play Index :", "No1");
		else if(remoteRingback == 1)
			printf("%-40s %s \n","IVR Play Index :", "No2");
		else if(remoteRingback == 2)
			printf("%-40s %s \n","IVR Play Index :", "No3");
		else if(remoteRingback == 3)
			printf("%-40s %s \n","IVR Play Index :", "No4");
		else if(remoteRingback == 4)
			printf("%-40s %s \n","IVR Play Index :", "No5");
		printf("%-40s %s \n","Music On Hold (MOH)", json_object_get_string(json_object_object_get(obj, "X_ZYXEL_MusicOnHoldTone")));
		musicTone = json_object_get_int(json_object_object_get(obj, "X_ZYXEL_MusicOnHoldToneIndex"));
		if(musicTone == 66)
			printf("%-40s %s \n","IVR Play Index :", "Default");
		else if(musicTone == 0)
			printf("%-40s %s \n","IVR Play Index :", "No1");
		else if(musicTone == 1)
			printf("%-40s %s \n","IVR Play Index :", "No2");
		else if(musicTone == 2)
			printf("%-40s %s \n","IVR Play Index :", "No3");
		else if(musicTone == 3)
			printf("%-40s %s \n","IVR Play Index :", "No4");
		else if(musicTone == 4)
			printf("%-40s %s \n","IVR Play Index :", "No5");

	}
	else{
		printf("%-10s %-10s %-15s %-20s %-10s \n","Index", "Status", "SIP Account", "Service Provider", "Account Number");
		for(i=0;i<len;i++){
			obj = json_object_array_get_idx(Jarray, i);
			printf("%-10s %-10s %-15s %-20s %-10s \n",
				json_object_get_string(json_object_object_get(obj, "Index")),
				json_object_get_string(json_object_object_get(obj, "Enable")),
				json_object_get_string(json_object_object_get(obj, "SipAccount")),
				json_object_get_string(json_object_object_get(obj, "Name")),
				json_object_get_string(json_object_object_get(obj, "DirectoryNumber")));
		}
	}
}

zcfgRet_t zcfgFeDalLineParaSet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t initLineSchemaGlobalObjects(){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t getLineSchemaBasicInfo(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}


bool checkMaxLineNum(int bindProfIndex){
	unsigned int lineNum = 0;
	objIndex_t getLineIid;
	struct json_object* targetobj = NULL;

	IID_INIT(getLineIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE, &getLineIid, &targetobj) == ZCFG_SUCCESS) {
		if(getLineIid.idx[1] == bindProfIndex){
			lineNum++;
		}
		zcfgFeJsonObjFree(targetobj);
	}

	if(lineNum >= ZYXEL_SIP_LINE_PER_PROFILE_NUM_MAX){
		printf("%s():<WARRNING>Max line number(%d) Currect Line number(%d) - User can't add more line.\n", __FUNCTION__, ZYXEL_SIP_LINE_PER_PROFILE_NUM_MAX, lineNum);
		return false;
	}

	return true;
}

zcfgRet_t zcfgFeDalLineSchemaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//initLineSchemaGlobalObjects();
	//getLineSchemaBasicInfo(Jobj);

	//zcfgFeDalLineSchemaParaGet(Jobj);
	//zcfgFeDalLineSipSchemaParaGet(Jobj);
	//zcfgFeDalLineCallfeatureSchemaParaGet(Jobj);
	//zcfgFeDalLineMailCfgSchemaParaGet(Jobj);
	//zcfgFeDalLineCodecListSchemaParaGet(Jobj);
	//zcfgFeDalLineProcessingSchemaParaGet(Jobj);

	return ret;
}

zcfgRet_t zcfgFeDalparseMailserviceObj(struct json_object *paserobj);
zcfgRet_t zcfgFeDalVoipLineGetDefault(struct json_object* defaultJobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_LINE, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n", __FUNCTION__, RDM_OID_VOICE_LINE);
		return ret;
	}
	enable = json_object_get_string(json_object_object_get(defaultJobj, "Enable"));
	json_object_object_add(defaultJobj, "LineEnable", json_object_new_string(enable));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "Enable");

	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_LINE_SIP, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_LINE_SIP);
		return ret;
	}
	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_LINE_CALLING_FEATURE, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_LINE_CALLING_FEATURE);
		return ret;
	}
	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_LINE_PROCESSING, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_LINE_PROCESSING);
		return ret;
	}
	if((ret = zcfgFeGetSchemaParameterByOid(RDM_OID_VOICE_LINE_CODEC_LIST, defaultJobj)) != ZCFG_SUCCESS){
		printf("%s(): Get Schma Default value fail, oid(=%d) \n ", __FUNCTION__, RDM_OID_VOICE_LINE_CODEC_LIST);
		return ret;
	}

	if((ret = zcfgFeDalparseMailserviceObj(defaultJobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
		return ret;
	}
	
	CodecEnable = json_object_get_boolean(json_object_object_get(defaultJobj, "Enable"));
	json_object_object_add(defaultJobj, "CodecEnable", json_object_new_boolean(CodecEnable));//To avoid other voice obj use the same parameter name.
	json_object_object_del(defaultJobj, "Enable");

	return ret;
}

zcfgRet_t zcfgFeDalLineSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalLineSipSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalLineCallfeatureSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalLineMailCfgSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalLineCodecListSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

zcfgRet_t zcfgFeDalLineProcessingSchemaParaGet(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	return ret;
}

void initLineGlobalObjects(){
	phyIntfObj = NULL;
	lineObj = NULL;
	profileObj = NULL;
	profNumPlanpInfoObj = NULL;
	lineSipObj = NULL;
	lineCallingFeatureObj = NULL;
	lineProcessingObj = NULL;
	lineCodecListObj = NULL;
	lineMailCfgObj = NULL;
	lineMailserviceObj = NULL;
	lineSchemaObj = NULL;
	multiJobj = NULL;
	guiCustObj = NULL;
	singleLineobj = NULL;

	return;
}

void zcfgFeDalCustomizationflagGet(){

	IID_INIT(flagIid);
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_GUI_CUSTOMIZATION, &flagIid, &guiCustObj) == ZCFG_SUCCESS){
		flag1 = json_object_get_int(json_object_object_get(guiCustObj, "Flag1"));
		zcfgFeJsonObjFree(guiCustObj);
		return;
	}

	return;
}

void zcfgFeDalLineObjEditIidGet(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* path = NULL;
	char tmpStr[64] = {0};
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(lineIid);
	oid = RDM_OID_VOICE_LINE;

	/* Current convertIndextoIid will convert error after 52c781e33,
	 * because it use index to match rdm idx, but the index GUI using hase same schema with CLI
	 * The voice GUI will send full path, so only do index convert when config not have full path. 2018-03-15 Steve
	 */
	path = json_object_get_string(json_object_object_get(Jobj, "EditPath"));

	if(path != NULL){
		//Init lineIid
		lineIid.level = 3;
		lineIid.idx[0] = 1;

		//Parse full path
		sscanf(path,"VoiceProfile.%hhu.Line.%hhu",&lineIid.idx[1], &lineIid.idx[2]);
		sprintf(tmpStr,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d",lineIid.idx[1],lineIid.idx[2]);
		json_object_object_add(Jobj, "MissedCallEmailEvent", json_object_new_string(tmpStr));
		X_ZYXEL_MissedCallEmailEvent = (char *)json_object_get_string(json_object_object_get(Jobj, "MissedCallEmailEvent"));

	}else{
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));

		ret = convertIndextoIid(index, &lineIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS){
			return;
		}

		if(json_object_get_string(json_object_object_get(Jobj, "Index"))){				// for CLI edit
			sprintf(tmpStr,"VoiceProfile.%d.Line.%d",lineIid.idx[1],lineIid.idx[2]);
			json_object_object_add(Jobj, "EditPath", json_object_new_string(tmpStr));
			sprintf(tmpStr,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d",lineIid.idx[1],lineIid.idx[2]);
			json_object_object_add(Jobj, "MissedCallEmailEvent", json_object_new_string(tmpStr));
			X_ZYXEL_MissedCallEmailEvent = (char *)json_object_get_string(json_object_object_get(Jobj, "MissedCallEmailEvent"));
		}

	}


	return;
}

void freeAllLineObjects(){
	if(phyIntfObj) json_object_put(phyIntfObj);
	if(lineObj) json_object_put(lineObj);
	if(profileObj) json_object_put(profileObj);
	if(profNumPlanpInfoObj) json_object_put(profNumPlanpInfoObj);
	if(lineSipObj) json_object_put(lineSipObj);
	if(lineCallingFeatureObj) json_object_put(lineCallingFeatureObj);
	if(lineProcessingObj) json_object_put(lineProcessingObj);
	if(lineCodecListObj) json_object_put(lineCodecListObj);
	if(lineMailCfgObj) json_object_put(lineMailCfgObj);
	if(lineMailserviceObj) json_object_put(lineMailserviceObj);
	if(lineSchemaObj) json_object_put(lineSchemaObj);
	if(multiJobj) json_object_put(multiJobj);
	if(guiCustObj) json_object_put(guiCustObj);
	if(singleLineobj) json_object_put(singleLineobj);

	return;
}

zcfgRet_t getLineBasicInfo_Add(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t Iid = {0};
	char tmppath[32] = {0};

	if(json_object_get_string(json_object_object_get(Jobj, "ServiceProviderName"))){	// for CLI add
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &Iid, &obj) == ZCFG_SUCCESS){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ServiceProviderName")),json_object_get_string(json_object_object_get(obj, "Name")))){
				sprintf(tmppath,"VoiceProfile.%d",Iid.idx[1]);
				json_object_object_add(Jobj, "AddPath", json_object_new_string(tmppath));
				zcfgFeJsonObjFree(obj);
				break;
			}
			zcfgFeJsonObjFree(obj);
		}
	}
	voicelineEnable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));

	if(json_object_object_get(Jobj, "Enable")){
	if(voicelineEnable)
		enable = "Enabled";
	else 
		enable = "Disabled";
	}
	else{
		enable = "Enabled";
		json_object_object_add(Jobj, "Enable", json_object_new_string(enable));
		}
	if(json_object_object_get(Jobj, "SIP_Account_Number"))
		DirectoryNumber = json_object_get_string(json_object_object_get(Jobj, "SIP_Account_Number"));
	else
		DirectoryNumber = "ChangeMe";
	
	if(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST&flag1){
		PhyReferenceList = json_object_get_string(json_object_object_get(Jobj, "PhyReferenceList"));
	}
	if(json_object_object_get(Jobj, "AuthUserName"))
	AuthUserName = json_object_get_string(json_object_object_get(Jobj, "AuthUserName"));
	else
		AuthUserName = "ChangeMe";
	if(json_object_object_get(Jobj, "AuthPassword"))
	AuthPassword = json_object_get_string(json_object_object_get(Jobj, "AuthPassword"));
	else
		AuthPassword = "ChangeMe";
	if(json_object_object_get(Jobj, "Url_Type")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "Url_Type")),"SIP"))
			X_ZYXEL_Url_Type = "URL_Sip";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "Url_Type")),"TEL"))
			X_ZYXEL_Url_Type = "URL_Tel";
	}
	else
		X_ZYXEL_Url_Type = "URL_Sip";
#ifndef	WIND_ITALY_CUSTOMIZATION
	if(json_object_object_get(Jobj, "CallTransferEnable"))
		CallTransferEnable= json_object_get_boolean(json_object_object_get(Jobj, "CallTransferEnable"));
	else{
		CallTransferEnable = true;
		json_object_object_add(Jobj, "CallTransferEnable", json_object_new_boolean(true));	
	}
#endif
	if(json_object_object_get(Jobj, "CallWaitingEnable"))
		CallWaitingEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallWaitingEnable"));
	else{
		CallWaitingEnable = true;
		json_object_object_add(Jobj, "CallWaitingEnable", json_object_new_boolean(true));	
	}
	
	if(json_object_object_get(Jobj, "CallWaitingRejectTime"))
		X_ZYXEL_CallWaitingRejectTime = json_object_get_int(json_object_object_get(Jobj, "CallWaitingRejectTime"));
	else{
		X_ZYXEL_CallWaitingRejectTime = 15;
		json_object_object_add(Jobj, "CallWaitingRejectTime", json_object_new_int(15));	
	}
	
#ifndef	WIND_ITALY_CUSTOMIZATION
	if(json_object_object_get(Jobj, "CallForwardUnconditionalEnable"))
		CallForwardUnconditionalEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardUnconditionalEnable"));
	else{
		 CallForwardUnconditionalEnable = false;
		json_object_object_add(Jobj, "CallForwardUnconditionalEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "CallForwardUnconditionalNumber"))
		CallForwardUnconditionalNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardUnconditionalNumber"));
	else{
		CallForwardUnconditionalNumber = "";
		json_object_object_add(Jobj, "CallForwardUnconditionalNumber", json_object_new_string(""));	
	}

	if(json_object_object_get(Jobj, "CallForwardOnBusyEnable"))
		CallForwardOnBusyEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardOnBusyEnable"));
	else{
		CallForwardOnBusyEnable = false;
		json_object_object_add(Jobj, "CallForwardOnBusyEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "CallForwardOnBusyNumber"))
		CallForwardOnBusyNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardOnBusyNumber"));
	else{
		CallForwardOnBusyNumber = "";
		json_object_object_add(Jobj, "CallForwardOnBusyNumber", json_object_new_string(""));	
	}

	if(json_object_object_get(Jobj, "CallForwardOnNoAnswerEnable"))	
		CallForwardOnNoAnswerEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardOnNoAnswerEnable"));
	else{
		CallForwardOnNoAnswerEnable = false;
		json_object_object_add(Jobj, "CallForwardOnNoAnswerEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "CallForwardOnNoAnswerNumber"))
		CallForwardOnNoAnswerNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardOnNoAnswerNumber"));
	else{
		CallForwardOnNoAnswerNumber = "";
		json_object_object_add(Jobj, "CallForwardOnNoAnswerNumber", json_object_new_string(""));	
	}
	if(json_object_object_get(Jobj, "CallForwardOnNoAnswerRingCount"))
		CallForwardOnNoAnswerRingCount = json_object_get_int(json_object_object_get(Jobj, "CallForwardOnNoAnswerRingCount"));
	else{
		CallForwardOnNoAnswerRingCount = 20;
		json_object_object_add(Jobj, "CallForwardOnNoAnswerRingCount", json_object_new_int(20));	
	}
	if(json_object_object_get(Jobj, "AnonymousCallEnable"))
		AnonymousCallEnable = json_object_get_boolean(json_object_object_get(Jobj, "AnonymousCallEnable"));
	else{
		AnonymousCallEnable = false;
		json_object_object_add(Jobj, "AnonymousCallEnable", json_object_new_boolean(false));	
	}
#endif
	if(json_object_object_get(Jobj, "AnonymousCallBlockEnable"))
		AnonymousCallBlockEnable = json_object_get_boolean(json_object_object_get(Jobj, "AnonymousCallBlockEnable"));
	else{
		AnonymousCallBlockEnable = false;
		json_object_object_add(Jobj, "AnonymousCallBlockEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "DoNotDisturbEnable"))
		DoNotDisturbEnable = json_object_get_boolean(json_object_object_get(Jobj, "DoNotDisturbEnable"));
	else{
		DoNotDisturbEnable = false;
		json_object_object_add(Jobj, "DoNotDisturbEnable", json_object_new_boolean(false));	
	}
#ifndef	WIND_ITALY_CUSTOMIZATION	
	if(json_object_object_get(Jobj, "MWIEnable"))
		MWIEnable = json_object_get_boolean(json_object_object_get(Jobj, "MWIEnable"));
	else{
		MWIEnable = false;
		json_object_object_add(Jobj, "MWIEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "MwiExpireTime"))
		X_ZYXEL_MwiExpireTime = json_object_get_int(json_object_object_get(Jobj, "MwiExpireTime"));
	else{
		X_ZYXEL_MwiExpireTime = 3600;
		json_object_object_add(Jobj, "MwiExpireTime", json_object_new_int(3600));	
	}
	if(json_object_object_get(Jobj, "HotLine_WarmLine_Enable"))
		X_ZYXEL_WarmLineEnable = json_object_get_boolean(json_object_object_get(Jobj, "HotLine_WarmLine_Enable"));
	else{
		json_object_object_add(Jobj, "HotLine_WarmLine_Enable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "HotLineEnable"))
		X_ZYXEL_HotLineEnable = json_object_get_boolean(json_object_object_get(Jobj, "HotLineEnable"));
	else{
		X_ZYXEL_HotLineEnable = false;
		json_object_object_add(Jobj, "HotLineEnable", json_object_new_boolean(false));	
	}
	if(json_object_object_get(Jobj, "HotLineNumber"))
		X_ZYXEL_HotLineNum = json_object_get_string(json_object_object_get(Jobj, "HotLineNumber"));
	else{
		X_ZYXEL_HotLineNum = "";
		json_object_object_add(Jobj, "HotLineNumber", json_object_new_string(""));	
	}
	if(json_object_object_get(Jobj, "WarmLineNumber"))
		X_ZYXEL_WarmLineNumber = json_object_get_string(json_object_object_get(Jobj, "WarmLineNumber"));
	else{
		X_ZYXEL_WarmLineNumber = "";
		json_object_object_add(Jobj, "WarmLineNumber", json_object_new_string(""));	
	}
	if(json_object_object_get(Jobj, "TimerFirstDigitWarmLine"))
		X_ZYXEL_TimerFirstDigitWarmLine = json_object_get_int(json_object_object_get(Jobj, "TimerFirstDigitWarmLine"));
	else{
		X_ZYXEL_TimerFirstDigitWarmLine = 5;
		json_object_object_add(Jobj, "TimerFirstDigitWarmLine", json_object_new_int(5));	
	}
#endif	
	if(json_object_object_get(Jobj, "MissedCallEmailEnable"))
		X_ZYXEL_MissedCallEmailEnable = json_object_get_boolean(json_object_object_get(Jobj, "MissedCallEmailEnable"));
	else{
		X_ZYXEL_MissedCallEmailEnable = false;
		json_object_object_add(Jobj, "MissedCallEmailEnable", json_object_new_boolean(false));	
	}

	//X_ZYXEL_MissedCallEmailEvent = json_object_get_string(json_object_object_get(Jobj, "MissedCallEmailEvent"));
	if(!(HIDE_VOICE_IVR&flag1)){
		if(json_object_object_get(Jobj, "RemoteRingbackTone"))
			X_ZYXEL_RemoteRingbackTone = json_object_get_boolean(json_object_object_get(Jobj, "RemoteRingbackTone"));
		else{
			X_ZYXEL_RemoteRingbackTone = false;
			json_object_object_add(Jobj, "RemoteRingbackTone", json_object_new_boolean(false));	
		}

		if(json_object_object_get(Jobj, "RemoteRingbackToneIndex")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"Default"))
				X_ZYXEL_RemoteRingbackToneIndex = 66;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No1"))
				X_ZYXEL_RemoteRingbackToneIndex = 0;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No2"))
				X_ZYXEL_RemoteRingbackToneIndex = 1;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No3"))
				X_ZYXEL_RemoteRingbackToneIndex = 2;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No4"))
				X_ZYXEL_RemoteRingbackToneIndex = 3;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No5"))
				X_ZYXEL_RemoteRingbackToneIndex = 4;
		}
		else {
			X_ZYXEL_RemoteRingbackToneIndex = 66;
			json_object_object_add(Jobj, "RemoteRingbackToneIndex", json_object_new_string("Default"));
		}
			

		if(json_object_object_get(Jobj, "MusicOnHoldTone"))
			X_ZYXEL_MusicOnHoldTone = json_object_get_boolean(json_object_object_get(Jobj, "MusicOnHoldTone"));
		else{ 
			X_ZYXEL_MusicOnHoldTone = false;
			json_object_object_add(Jobj, "MusicOnHoldTone", json_object_new_boolean(false));	
		}
		if(json_object_object_get(Jobj, "MusicOnHoldToneIndex")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"Default"))
				X_ZYXEL_MusicOnHoldToneIndex = 66;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No1"))
				X_ZYXEL_MusicOnHoldToneIndex = 0;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No2"))
				X_ZYXEL_MusicOnHoldToneIndex = 1;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No3"))
				X_ZYXEL_MusicOnHoldToneIndex = 2;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No4"))
				X_ZYXEL_MusicOnHoldToneIndex = 3;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No5"))
				X_ZYXEL_MusicOnHoldToneIndex = 4;
		}
		else{
			X_ZYXEL_MusicOnHoldToneIndex = 66;
			json_object_object_add(Jobj, "MusicOnHoldToneIndex", json_object_new_string("Default"));
		}
	}
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		X_ZYXEL_CCBSEnable = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_CCBSEnable"));
	}
	if(json_object_object_get(Jobj, "EchoCancellationEnable"))
		EchoCancellationEnable = json_object_get_boolean(json_object_object_get(Jobj, "EchoCancellationEnable"));
	else{
		EchoCancellationEnable = true;
		json_object_object_add(Jobj, "EchoCancellationEnable", json_object_new_boolean(true));
	}

	if(json_object_object_get(Jobj, "TransmitGain")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Minimum"))
			TransmitGain = -3;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Middle"))
			TransmitGain = 0;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Maximum"))
			TransmitGain = 6;
	}
	else{
		TransmitGain = 0;
		json_object_object_add(Jobj, "TransmitGain", json_object_new_string("Middle"));
	}

	if(json_object_object_get(Jobj, "ReceiveGain")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Minimum"))
			ReceiveGain = -3;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Middle"))
			ReceiveGain = 0;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Maximum"))
			ReceiveGain = 6;
	}
	else{
		ReceiveGain = 0;
		json_object_object_add(Jobj, "ReceiveGain", json_object_new_string("Middle"));
	}

	if (json_object_object_get(Jobj, "VoiceJBMode"))
	{
		X_ZYXEL_VoiceJBMode = (char *)json_object_get_string(json_object_object_get(Jobj, "VoiceJBMode"));
	}
	if (json_object_object_get(Jobj, "VoiceJBMin"))
	{
		X_ZYXEL_VoiceJBMin = json_object_get_int(json_object_object_get(Jobj, "VoiceJBMin"));
	}
	if (json_object_object_get(Jobj, "VoiceJBMax"))
	{
		X_ZYXEL_VoiceJBMax = json_object_get_int(json_object_object_get(Jobj, "VoiceJBMax"));
	}
	if (json_object_object_get(Jobj, "VoiceJBTarget"))
	{
		X_ZYXEL_VoiceJBTarget = json_object_get_int(json_object_object_get(Jobj, "VoiceJBTarget"));
	}
	if (json_object_object_get(Jobj, "DataJBMode"))
	{
		X_ZYXEL_DataJBMode = (char *)json_object_get_string(json_object_object_get(Jobj, "DataJBMode"));
	}
	if (json_object_object_get(Jobj, "DataJBTarget"))
	{
		X_ZYXEL_DataJBTarget = json_object_get_int(json_object_object_get(Jobj, "DataJBTarget"));
	}

	if(json_object_object_get(Jobj, "primaryCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G711MuLaw"))
			primaryCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G711ALaw"))
			primaryCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G729"))
			primaryCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G726_24"))
			primaryCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G726_32"))
			primaryCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G722"))
			primaryCodec = "G.722";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"None"))
			primaryCodec = "None";
		else
			primaryCodec = json_object_get_string(json_object_object_get(Jobj, "primaryCodec"));
	}
	else{ 
		primaryCodec = "G.711MuLaw";
		json_object_object_add(Jobj, "primaryCodec", json_object_new_string("G.711MuLaw"));
	}

	if(json_object_object_get(Jobj, "secondarydCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G711MuLaw"))
			secondarydCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G711ALaw"))
			secondarydCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G729"))
			secondarydCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G726_24"))
			secondarydCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G726_32"))
			secondarydCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G722"))
			secondarydCodec = "G.722";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"None"))
			secondarydCodec = "None";
		else
			secondarydCodec = json_object_get_string(json_object_object_get(Jobj, "secondarydCodec"));
	}
	else{
		secondarydCodec = "G.711ALaw";
		json_object_object_add(Jobj, "secondarydCodec", json_object_new_string("G.711ALaw"));
	}

	if(json_object_object_get(Jobj, "thirdCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G711MuLaw"))
			thirdCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G711ALaw"))
			thirdCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G729"))
			thirdCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G726_24"))
			thirdCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G726_32"))
			thirdCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G722"))
			thirdCodec = "G.722";
		else
			thirdCodec = json_object_get_string(json_object_object_get(Jobj, "thirdCodec"));
	}
	else{
		thirdCodec = "G.722";
		json_object_object_add(Jobj, "thirdCodec", json_object_new_string("G.722"));
	}
	
	if(json_object_object_get(Jobj, "MailAccount"))
		SericeRef = json_object_get_string(json_object_object_get(Jobj, "MailAccount"));
	else{ 
		SericeRef = "";
		json_object_object_add(Jobj, "MailAccount", json_object_new_string(""));
	}
	if(json_object_object_get(Jobj, "Missed_Call_Email_Title"))
		EmailSubject = json_object_get_string(json_object_object_get(Jobj, "Missed_Call_Email_Title"));
	else{
		EmailSubject = MISSED_CALL_MAIL_TITLE_DEFAULT; //Amber.20171228.Modify to Marco
		json_object_object_add(Jobj, "Missed_Call_Email_Title", json_object_new_string(MISSED_CALL_MAIL_TITLE_DEFAULT));
	}
	if(json_object_object_get(Jobj, "Send_Notification_to_Email"))
		EmailTo = json_object_get_string(json_object_object_get(Jobj, "Send_Notification_to_Email"));
	else{
		EmailTo = "";
		json_object_object_add(Jobj, "Send_Notification_to_Email", json_object_new_string(""));
	}
	if(json_object_object_get(Jobj, "SilenceSuppression"))
		SilenceSuppression = json_object_get_boolean(json_object_object_get(Jobj, "SilenceSuppression"));
	else{
		SilenceSuppression = true;
		json_object_object_add(Jobj, "SilenceSuppression", json_object_new_boolean(true));
	}
	if(SHOW_CONFIG_VOIP_COMFORT_NOISE&flag1){
		X_ZYXEL_ComfortNoise = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_ComfortNoise"));
	}

	return ret;
}


zcfgRet_t getLineBasicInfo(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	voicelineEnable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(voicelineEnable)
		enable = "Enabled";
	else 
		enable = "Disabled";
	//enable = json_object_get_string(json_object_object_get(Jobj, "Enable"));
	DirectoryNumber = json_object_get_string(json_object_object_get(Jobj, "SIP_Account_Number"));
	if(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST&flag1){

		PhyReferenceList = json_object_get_string(json_object_object_get(Jobj, "PhyReferenceList"));
	}
	AuthUserName = json_object_get_string(json_object_object_get(Jobj, "AuthUserName"));
	AuthPassword = json_object_get_string(json_object_object_get(Jobj, "AuthPassword"));
	if(json_object_object_get(Jobj, "Url_Type")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "Url_Type")),"SIP"))
			X_ZYXEL_Url_Type = "URL_Sip";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "Url_Type")),"TEL"))
			X_ZYXEL_Url_Type = "URL_Tel";
	}
#ifndef	WIND_ITALY_CUSTOMIZATION
	CallTransferEnable= json_object_get_boolean(json_object_object_get(Jobj, "CallTransferEnable"));
#endif
	CallWaitingEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallWaitingEnable"));
	X_ZYXEL_CallWaitingRejectTime = json_object_get_int(json_object_object_get(Jobj, "CallWaitingRejectTime"));
	
#ifndef	WIND_ITALY_CUSTOMIZATION		
	CallForwardUnconditionalEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardUnconditionalEnable"));
	CallForwardUnconditionalNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardUnconditionalNumber"));

	if(CallForwardUnconditionalEnable){
		if(!CallForwardUnconditionalNumber || !strcmp(CallForwardUnconditionalNumber, "")){
			strcat(replyMsg, "If you active unconditional call forward, you have to set the forward number.");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SIP.Account.Input.warn_unconditional_call_forward_Num_empty"));
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}

	CallForwardOnBusyEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardOnBusyEnable"));
	CallForwardOnBusyNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardOnBusyNumber"));
	if(CallForwardOnBusyEnable){
		if(!CallForwardOnBusyNumber || !strcmp(CallForwardOnBusyNumber, "")){
			strcat(replyMsg, "If you active Busy call forward , you have to set the forward number.");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SIP.Account.Input.warn_busy_call_forward_Num_empty"));
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	
	CallForwardOnNoAnswerEnable = json_object_get_boolean(json_object_object_get(Jobj, "CallForwardOnNoAnswerEnable"));
	CallForwardOnNoAnswerNumber = json_object_get_string(json_object_object_get(Jobj, "CallForwardOnNoAnswerNumber"));
	if(CallForwardOnNoAnswerEnable){
		if(!CallForwardOnNoAnswerNumber || !strcmp(CallForwardOnNoAnswerNumber, "")){
			strcat(replyMsg, "If you active no answer call forward , you have to set the forward number.");
			json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SIP.Account.Input.warn_no_answer_call_forward_Num_empty"));
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}

	CallForwardOnNoAnswerRingCount = json_object_get_int(json_object_object_get(Jobj, "CallForwardOnNoAnswerRingCount"));
	AnonymousCallEnable = json_object_get_boolean(json_object_object_get(Jobj, "AnonymousCallEnable"));
#endif
	AnonymousCallBlockEnable = json_object_get_boolean(json_object_object_get(Jobj, "AnonymousCallBlockEnable"));
	DoNotDisturbEnable = json_object_get_boolean(json_object_object_get(Jobj, "DoNotDisturbEnable"));
#ifndef	WIND_ITALY_CUSTOMIZATION	
	MWIEnable = json_object_get_boolean(json_object_object_get(Jobj, "MWIEnable"));
	X_ZYXEL_WarmLineEnable = json_object_get_boolean(json_object_object_get(Jobj, "HotLine_WarmLine_Enable"));
	X_ZYXEL_HotLineEnable = json_object_get_boolean(json_object_object_get(Jobj, "HotLineEnable"));
	X_ZYXEL_HotLineNum = json_object_get_string(json_object_object_get(Jobj, "HotLineNumber"));
	X_ZYXEL_WarmLineNumber = json_object_get_string(json_object_object_get(Jobj, "WarmLineNumber"));
	X_ZYXEL_TimerFirstDigitWarmLine = json_object_get_int(json_object_object_get(Jobj, "TimerFirstDigitWarmLine"));
	X_ZYXEL_MwiExpireTime = json_object_get_int(json_object_object_get(Jobj, "MwiExpireTime"));
#endif	
	X_ZYXEL_MissedCallEmailEnable = json_object_get_boolean(json_object_object_get(Jobj, "MissedCallEmailEnable"));
	X_ZYXEL_MissedCallEmailEvent = (char *)json_object_get_string(json_object_object_get(Jobj, "MissedCallEmailEvent"));
	if(!(HIDE_VOICE_IVR&flag1)){
		X_ZYXEL_RemoteRingbackTone = json_object_get_boolean(json_object_object_get(Jobj, "RemoteRingbackTone"));
		if(json_object_object_get(Jobj, "RemoteRingbackToneIndex")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"Default"))
				X_ZYXEL_RemoteRingbackToneIndex = 66;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No1"))
				X_ZYXEL_RemoteRingbackToneIndex = 0;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No2"))
				X_ZYXEL_RemoteRingbackToneIndex = 1;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No3"))
				X_ZYXEL_RemoteRingbackToneIndex = 2;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No4"))
				X_ZYXEL_RemoteRingbackToneIndex = 3;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "RemoteRingbackToneIndex")),"No5"))
				X_ZYXEL_RemoteRingbackToneIndex = 4;
		}

		X_ZYXEL_MusicOnHoldTone = json_object_get_boolean(json_object_object_get(Jobj, "MusicOnHoldTone"));

		if(json_object_object_get(Jobj, "MusicOnHoldToneIndex")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"Default"))
				X_ZYXEL_MusicOnHoldToneIndex = 66;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No1"))
				X_ZYXEL_MusicOnHoldToneIndex = 0;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No2"))
				X_ZYXEL_MusicOnHoldToneIndex = 1;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No3"))
				X_ZYXEL_MusicOnHoldToneIndex = 2;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No4"))
				X_ZYXEL_MusicOnHoldToneIndex = 3;
			else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "MusicOnHoldToneIndex")),"No5"))
				X_ZYXEL_MusicOnHoldToneIndex = 4;
		}

	}
	if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
		X_ZYXEL_CCBSEnable = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_CCBSEnable"));
	}
	if(json_object_object_get(Jobj, "EchoCancellationEnable"))
	EchoCancellationEnable = json_object_get_boolean(json_object_object_get(Jobj, "EchoCancellationEnable"));
	else
		EchoCancellationEnable = true;
	if(json_object_object_get(Jobj, "TransmitGain")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Minimum"))
			TransmitGain = -3;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Middle"))
			TransmitGain = 0;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "TransmitGain")),"Maximum"))
			TransmitGain = 6;
	}
	else
		TransmitGain = 0;
	if(json_object_object_get(Jobj, "ReceiveGain")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Minimum"))
			ReceiveGain = -3;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Middle"))
			ReceiveGain = 0;
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ReceiveGain")),"Maximum"))
			ReceiveGain = 6;
	}
	else
		ReceiveGain = 0;
	
	if(json_object_object_get(Jobj, "primaryCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G711MuLaw"))
			primaryCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G711ALaw"))
			primaryCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G729"))
			primaryCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G726_24"))
			primaryCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G726_32"))
			primaryCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "primaryCodec")),"G722"))
			primaryCodec = "G.722";
		else
		{
			dbg_printf("%s:%d: Unknown \"primaryCodec\" value..\n",__FUNCTION__,__LINE__);
			primaryCodec = json_object_get_string(json_object_object_get(Jobj, "primaryCodec"));
		}
	}

	if(json_object_object_get(Jobj, "secondarydCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G711MuLaw"))
			secondarydCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G711ALaw"))
			secondarydCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G729"))
			secondarydCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G726_24"))
			secondarydCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G726_32"))
			secondarydCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"G722"))
			secondarydCodec = "G.722";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "secondarydCodec")),"None"))
			secondarydCodec = "None";
		else
		{
			dbg_printf("%s:%d: Unknown \"secondarydCodec\" value..\n",__FUNCTION__,__LINE__);
			secondarydCodec = json_object_get_string(json_object_object_get(Jobj, "secondarydCodec"));
		}
	}

	if(json_object_object_get(Jobj, "thirdCodec")){
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G711MuLaw"))
			thirdCodec = "G.711MuLaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G711ALaw"))
			thirdCodec = "G.711ALaw";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G729"))
			thirdCodec = "G.729";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G726_24"))
			thirdCodec = "G.726-24";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G726_32"))
			thirdCodec = "G.726-32";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"G722"))
			thirdCodec = "G.722";
		else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "thirdCodec")),"None"))
			thirdCodec = "None";
		else
		{
			dbg_printf("%s:%d: Unknown \"thirdCodec\" value..\n",__FUNCTION__,__LINE__);
			thirdCodec = json_object_get_string(json_object_object_get(Jobj, "thirdCodec"));
		}
	}

	SericeRef = json_object_get_string(json_object_object_get(Jobj, "MailAccount"));
	EmailSubject = json_object_get_string(json_object_object_get(Jobj, "Missed_Call_Email_Title"));
	EmailTo = json_object_get_string(json_object_object_get(Jobj, "Send_Notification_to_Email"));
	SilenceSuppression = json_object_get_boolean(json_object_object_get(Jobj, "SilenceSuppression"));
	if(SHOW_CONFIG_VOIP_COMFORT_NOISE&flag1){
		X_ZYXEL_ComfortNoise = json_object_get_boolean(json_object_object_get(Jobj, "X_ZYXEL_ComfortNoise"));
	}

	if (json_object_object_get(Jobj, "VoiceJBMode"))
	{
		X_ZYXEL_VoiceJBMode = (char *)json_object_get_string(json_object_object_get(Jobj, "VoiceJBMode"));
	}
	if (json_object_object_get(Jobj, "VoiceJBMin"))
	{
		X_ZYXEL_VoiceJBMin = json_object_get_int(json_object_object_get(Jobj, "VoiceJBMin"));
	}
	if (json_object_object_get(Jobj, "VoiceJBMax"))
	{
		X_ZYXEL_VoiceJBMax = json_object_get_int(json_object_object_get(Jobj, "VoiceJBMax"));
	}
	if (json_object_object_get(Jobj, "VoiceJBTarget"))
	{
		X_ZYXEL_VoiceJBTarget = json_object_get_int(json_object_object_get(Jobj, "VoiceJBTarget"));
	}
	if (json_object_object_get(Jobj, "DataJBMode"))
	{
		X_ZYXEL_DataJBMode = (char *)json_object_get_string(json_object_object_get(Jobj, "DataJBMode"));
	}
	if (json_object_object_get(Jobj, "DataJBTarget"))
	{
		X_ZYXEL_DataJBTarget = json_object_get_int(json_object_object_get(Jobj, "DataJBTarget"));
	}

	return ret;
}

zcfgRet_t zcfgFeDalLineObjAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	IID_INIT(lineIid);
	int profileNum;
	const char* path = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	path = json_object_get_string(json_object_object_get(Jobj, "AddPath"));
	//printf("%s(): Edit Line path : %s \n ",__FUNCTION__,path);
	sscanf(path,"VoiceProfile.%d",&profileNum);
	//printf("Edit Line - profile: %d \n ",profileNum);

	if(checkMaxLineNum(profileNum) == false){
		json_object_object_add(Jobj, "__multi_lang_replyMsg", json_object_new_string("zylang.SIP.Account.Input.warn_Max_line_of_profile"));
		return ZCFG_INVALID_PARAM_VALUE;
	}

	lineIid.level = 2;
	lineIid.idx[0] = 1;
	lineIid.idx[1] = profileNum;

	if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_VOICE_LINE, &lineIid, NULL)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonAdd add line fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	return ret;
}


zcfgRet_t zcfgFeDalLineObjEdit(struct json_object *Jobj){
	struct json_object *setJson = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE, &lineIid, &lineObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get line fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	//printf("%s():  get data moedl =>lineObj :%s \n ",__FUNCTION__,json_object_to_json_string(lineObj));

	if(lineObj != NULL){
		if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(lineObj, "Enable", json_object_new_string(enable));
		if(json_object_object_get(Jobj, "SIP_Account_Number"))
		json_object_object_add(lineObj, "DirectoryNumber", json_object_new_string(DirectoryNumber));
		if(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST&flag1){

			if(PhyReferenceList){ //PhyReferenceList configured
				json_object_object_add(lineObj, "PhyReferenceList", json_object_new_string(PhyReferenceList));
			}

		}


		//printf("%s(): lineObj :%s \n ",__FUNCTION__,json_object_to_json_string(lineObj));
		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE, &lineIid, multiJobj, lineObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE);
			zcfgFeJsonObjFree(lineObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(lineObj);
	}
	//printf("%s(): Append Line obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalLineSipObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_SIP, &lineIid, &lineSipObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get line sip fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineSipObj != NULL){
		if(json_object_object_get(Jobj, "AuthUserName"))
		json_object_object_add(lineSipObj, "AuthUserName", json_object_new_string(AuthUserName));
		if(json_object_object_get(Jobj, "AuthPassword"))
		if(json_object_object_get(Jobj, "AuthPassword")){
			if (strcmp(AuthPassword, hiddenAuthPassword)){ //it means user changes password
				json_object_object_add(lineSipObj, "AuthPassword", json_object_new_string(AuthPassword));
			}
		}
		if(json_object_object_get(Jobj, "Url_Type"))
		json_object_object_add(lineSipObj, "X_ZYXEL_Url_Type", json_object_new_string(X_ZYXEL_Url_Type));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE_SIP, &lineIid, multiJobj, lineSipObj)) == NULL){
			printf("%s: fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE_SIP);
			zcfgFeJsonObjFree(lineSipObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(lineSipObj);
	}
	//printf("%s():Append Line Sip obj to multiJobj (%s) \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}
zcfgRet_t zcfgFeDalLineCallFeatureObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	char EmailEvent[65] = {0};
	//printf("%s() Enter \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_CALLING_FEATURE, &lineIid, &lineCallingFeatureObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get line calling feature fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineCallingFeatureObj != NULL){
#ifndef	WIND_ITALY_CUSTOMIZATION		
		if(json_object_object_get(Jobj, "CallTransferEnable"))
		json_object_object_add(lineCallingFeatureObj, "CallTransferEnable", json_object_new_boolean(CallTransferEnable));
#endif
		if(json_object_object_get(Jobj, "CallWaitingEnable"))
		json_object_object_add(lineCallingFeatureObj, "CallWaitingEnable", json_object_new_boolean(CallWaitingEnable));
		if(json_object_object_get(Jobj, "CallWaitingRejectTime"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_CallWaitingRejectTime", json_object_new_int(X_ZYXEL_CallWaitingRejectTime));
#ifndef	WIND_ITALY_CUSTOMIZATION			
		if(json_object_object_get(Jobj, "CallForwardUnconditionalEnable"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardUnconditionalEnable", json_object_new_boolean(CallForwardUnconditionalEnable));
		if(json_object_object_get(Jobj, "CallForwardUnconditionalNumber"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardUnconditionalNumber", json_object_new_string(CallForwardUnconditionalNumber));
		if(json_object_object_get(Jobj, "CallForwardOnBusyNumber"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardOnBusyNumber", json_object_new_string(CallForwardOnBusyNumber));
		if(json_object_object_get(Jobj, "CallForwardOnBusyEnable"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardOnBusyEnable", json_object_new_boolean(CallForwardOnBusyEnable));
		if(json_object_object_get(Jobj, "CallForwardOnNoAnswerEnable"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardOnNoAnswerEnable", json_object_new_boolean(CallForwardOnNoAnswerEnable));
		if(json_object_object_get(Jobj, "CallForwardOnNoAnswerNumber"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardOnNoAnswerNumber", json_object_new_string(CallForwardOnNoAnswerNumber));
		if(json_object_object_get(Jobj, "CallForwardOnNoAnswerRingCount"))
		json_object_object_add(lineCallingFeatureObj, "CallForwardOnNoAnswerRingCount", json_object_new_int(CallForwardOnNoAnswerRingCount));	
		if(json_object_object_get(Jobj, "AnonymousCallEnable"))
		json_object_object_add(lineCallingFeatureObj, "AnonymousCallEnable", json_object_new_boolean(AnonymousCallEnable));
#endif
		if(json_object_object_get(Jobj, "AnonymousCallBlockEnable"))
		json_object_object_add(lineCallingFeatureObj, "AnonymousCallBlockEnable", json_object_new_boolean(AnonymousCallBlockEnable));
		if(json_object_object_get(Jobj, "DoNotDisturbEnable"))
		json_object_object_add(lineCallingFeatureObj, "DoNotDisturbEnable", json_object_new_boolean(DoNotDisturbEnable));
#ifndef	WIND_ITALY_CUSTOMIZATION			
		if(json_object_object_get(Jobj, "MWIEnable"))
		json_object_object_add(lineCallingFeatureObj, "MWIEnable", json_object_new_boolean(MWIEnable));
		if(json_object_object_get(Jobj, "MwiExpireTime"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MwiExpireTime", json_object_new_int(X_ZYXEL_MwiExpireTime));		
		if(json_object_object_get(Jobj, "HotLine_WarmLine_Enable"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_WarmLineEnable", json_object_new_boolean(X_ZYXEL_WarmLineEnable));
		if(json_object_object_get(Jobj, "HotLineEnable")){
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_HotLineEnable", json_object_new_boolean(X_ZYXEL_HotLineEnable));
			if(X_ZYXEL_HotLineEnable)
				json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_WarmLineEnable", json_object_new_boolean(false));
		}
		if(json_object_object_get(Jobj, "HotLineNumber"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_HotLineNum", json_object_new_string(X_ZYXEL_HotLineNum));
		if(json_object_object_get(Jobj, "WarmLineNumber"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_WarmLineNumber", json_object_new_string(X_ZYXEL_WarmLineNumber));
		if(json_object_object_get(Jobj, "HotLine_WarmLine_Enable") || json_object_object_get(Jobj, "TimerFirstDigitWarmLine"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_TimerFirstDigitWarmLine", json_object_new_int(X_ZYXEL_TimerFirstDigitWarmLine));
#endif		
		if(json_object_object_get(Jobj, "MissedCallEmailEnable"))
		json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MissedCallEmailEnable", json_object_new_boolean(X_ZYXEL_MissedCallEmailEnable));

		if(!(HIDE_VOICE_IVR&flag1)){
			if(json_object_object_get(Jobj, "RemoteRingbackTone"))
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_RemoteRingbackTone", json_object_new_boolean(X_ZYXEL_RemoteRingbackTone));
			if(json_object_object_get(Jobj, "RemoteRingbackToneIndex"))
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_RemoteRingbackToneIndex", json_object_new_int(X_ZYXEL_RemoteRingbackToneIndex));
			if(json_object_object_get(Jobj, "MusicOnHoldTone"))
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MusicOnHoldTone", json_object_new_boolean(X_ZYXEL_MusicOnHoldTone));
			if(json_object_object_get(Jobj, "MusicOnHoldToneIndex"))
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MusicOnHoldToneIndex", json_object_new_int(X_ZYXEL_MusicOnHoldToneIndex));
		}
		if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
			if(json_object_object_get(Jobj, "X_ZYXEL_CCBSEnable"))
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_CCBSEnable", json_object_new_boolean(X_ZYXEL_CCBSEnable));
		}

		if(json_object_get_string(json_object_object_get(Jobj, "AddPath")) != NULL){//For Add.
			sprintf(EmailEvent,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d",lineIid.idx[1],lineIid.idx[2]);
			//if(json_object_object_get(Jobj, "MissedCallEmailEvent")) //Amber.20180117: Value of MissedCallEmailEvent should always generate by DAL.
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MissedCallEmailEvent", json_object_new_string(EmailEvent));
		}else if(json_object_get_string(json_object_object_get(Jobj, "EditPath")) != NULL){//For Edit.
			json_object_object_add(lineCallingFeatureObj, "X_ZYXEL_MissedCallEmailEvent", json_object_new_string(X_ZYXEL_MissedCallEmailEvent));
		}

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE_CALLING_FEATURE, &lineIid, multiJobj, lineCallingFeatureObj)) == NULL){
			printf("%s: fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE_CALLING_FEATURE);
			zcfgFeJsonObjFree(lineCallingFeatureObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(lineCallingFeatureObj);
	}

	//printf("%s(): Append Line Call Feature obj to multiJobj :%s \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}
zcfgRet_t zcfgFeDalLineCodecListObjEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	const char* codec = NULL;
	int codecpriority = 0;
	//printf("%s() Enter \n ",__FUNCTION__);

	IID_INIT(lineCodecListIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE_CODEC_LIST, &lineIid, &lineCodecListIid, &lineCodecListObj) == ZCFG_SUCCESS){
		codec = json_object_get_string(json_object_object_get(lineCodecListObj, "Codec"));
		codecpriority = json_object_get_int(json_object_object_get(lineCodecListObj, "Priority"));
		if(primaryCodec != NULL){
			if(strcmp(codec, primaryCodec) && codecpriority == 1)
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(100));
		}
		if(secondarydCodec != NULL){
			if(strcmp(codec, secondarydCodec) && codecpriority == 2)
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(100));
		}
		if(thirdCodec != NULL){
			if(strcmp(codec, thirdCodec) && codecpriority == 3)
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(100));
		}
		//printf("codec : %s   => primaryCodec : %s  ,  secondarydCodec : %s  , thirdCodec : %s \n",codec,primaryCodec, secondarydCodec, thirdCodec);
		if(json_object_object_get(Jobj, "primaryCodec") && json_object_object_get(Jobj, "secondarydCodec") && json_object_object_get(Jobj, "thirdCodec")){
			if(strcmp(primaryCodec,codec) && strcmp(secondarydCodec,codec) && strcmp(thirdCodec,codec)){
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(100));
			}
		}
		if(json_object_object_get(Jobj, "primaryCodec")){
			if(!strcmp(primaryCodec,codec)){
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(1));
			}
		}
		if(json_object_object_get(Jobj, "secondarydCodec")){
			if(!strcmp(secondarydCodec,codec)){
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(2));
			}
		}
		if(json_object_object_get(Jobj, "thirdCodec")){
			if(!strcmp(thirdCodec,codec)){
				json_object_object_add(lineCodecListObj, "Priority", json_object_new_int(3));
			}
		}
		if(json_object_object_get(Jobj, "SilenceSuppression"))
			json_object_object_add(lineCodecListObj, "SilenceSuppression", json_object_new_boolean(SilenceSuppression));
		if(SHOW_CONFIG_VOIP_COMFORT_NOISE&flag1){
			json_object_object_add(lineCodecListObj, "X_ZYXEL_ComfortNoise", json_object_new_int(X_ZYXEL_ComfortNoise));

		}
		//printf("%s(): Append codec list boj  :%s \n ",__FUNCTION__,json_object_to_json_string(lineCodecListObj));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE_CODEC_LIST, &lineCodecListIid, multiJobj, lineCodecListObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE_CODEC_LIST);
			zcfgFeJsonObjFree(lineCodecListObj);
			return ZCFG_INTERNAL_ERROR;
		}

		zcfgFeJsonObjFree(lineCodecListObj);
	}

	//printf("%s(): Append Line Codec List obj to multiJobj :%s \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;

}
zcfgRet_t zcfgFeDalLineProcessingEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *setJson = NULL;
	//printf("\033[0;32m%s : Enter \033[0m \n", __FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_PROCESSING, &lineIid, &lineProcessingObj)) != ZCFG_SUCCESS){
		printf("%s(): zcfgFeObjJsonGetWithoutUpdate get line processing fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineProcessingObj != NULL){
		if(json_object_object_get(Jobj, "EchoCancellationEnable"))
		json_object_object_add(lineProcessingObj, "EchoCancellationEnable", json_object_new_boolean(EchoCancellationEnable));
		if(json_object_object_get(Jobj, "TransmitGain"))
		json_object_object_add(lineProcessingObj, "TransmitGain", json_object_new_int(TransmitGain));
		if(json_object_object_get(Jobj, "ReceiveGain"))
		json_object_object_add(lineProcessingObj, "ReceiveGain", json_object_new_int(ReceiveGain));

		if (json_object_object_get(Jobj, "VoiceJBMode"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_VoiceJBMode", json_object_new_string(X_ZYXEL_VoiceJBMode));
		}
		if (json_object_object_get(Jobj, "VoiceJBMin"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_VoiceJBMin", json_object_new_int(X_ZYXEL_VoiceJBMin));
		}
		if (json_object_object_get(Jobj, "VoiceJBMax"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_VoiceJBMax", json_object_new_int(X_ZYXEL_VoiceJBMax));
		}
		if (json_object_object_get(Jobj, "VoiceJBTarget"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_VoiceJBTarget", json_object_new_int(X_ZYXEL_VoiceJBTarget));
		}
		if (json_object_object_get(Jobj, "DataJBMode"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_DataJBMode", json_object_new_string(X_ZYXEL_DataJBMode));
		}
		if (json_object_object_get(Jobj, "DataJBTarget"))
		{
			json_object_object_add(lineProcessingObj, "X_ZYXEL_DataJBTarget", json_object_new_int(X_ZYXEL_DataJBTarget));
		}

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_VOICE_LINE_PROCESSING, &lineIid, multiJobj, lineProcessingObj)) == NULL){
			printf("%s:(ERROR) fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_VOICE_LINE_PROCESSING);
			zcfgFeJsonObjFree(lineProcessingObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(lineProcessingObj);
	}

	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalLineMailCfgAdd(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* tmpEvent = NULL;
	char eventTarget[65] = {0};
	char EmailBody[128] = {0};
	int profileId,lineId;
	bool mailEventConfigIsExist = false;

	//printf("%s() Enter \n ",__FUNCTION__);

	profileId = lineIid.idx[1];
	lineId = lineIid.idx[2];
	sprintf(eventTarget,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d",profileId,lineId);
	sprintf(EmailBody,"%s","You have one missed call on SIP-#{lineid}:#{localnum} from number #{peernum} on #{time}");

	//printf("%s():eventTarget : %s\n",__FUNCTION__,eventTarget);
	//printf("%s():eventTarget : %s\n",__FUNCTION__,EmailBody);

	IID_INIT(lineMailCfgIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj) == ZCFG_SUCCESS) {
		if(lineMailCfgObj != NULL){
			tmpEvent = json_object_get_string(json_object_object_get(lineMailCfgObj, "Event"));
			if(strcmp(tmpEvent,eventTarget) == 0){
				mailEventConfigIsExist = true;
				zcfgFeJsonObjFree(lineMailCfgObj);
				break;
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}
	}

	if(mailEventConfigIsExist != true){
		IID_INIT(lineMailCfgIid);
		if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, NULL)) != ZCFG_SUCCESS){
			printf("%s():(ERROR) zcfgFeObjJsonAdd add mail config fail~ \n ",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}
	}

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get mail config fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_add(lineMailCfgObj, "Event", json_object_new_string(eventTarget));
	json_object_object_add(lineMailCfgObj, "Enable", json_object_new_boolean(true));
	json_object_object_add(lineMailCfgObj, "SericeRef", json_object_new_string(SericeRef));
	json_object_object_add(lineMailCfgObj, "EmailBody", json_object_new_string(EmailBody));
	json_object_object_add(lineMailCfgObj, "EmailSubject", json_object_new_string(EmailSubject));
	json_object_object_add(lineMailCfgObj, "EmailTo", json_object_new_string(EmailTo));

	if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, lineMailCfgObj, NULL)) != ZCFG_SUCCESS ){
		zcfgLog(ZCFG_LOG_ERR, "%s : config mail config\n", __FUNCTION__);
		zcfgFeJsonObjFree(lineMailCfgObj);
		return ZCFG_INTERNAL_ERROR;
	}
	zcfgFeJsonObjFree(lineMailCfgObj);

	//printf("%s(): Append mailCfg obj to multiJobj :%s \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalLineMailCfgEdit(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* tmpEvent;
	struct json_object *setJson = NULL;
	lineMailCfgObj = NULL;

	//printf("%s() Enter \n ",__FUNCTION__);
	IID_INIT(lineMailCfgIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj) == ZCFG_SUCCESS) {
		if(lineMailCfgObj != NULL){
			tmpEvent = json_object_get_string(json_object_object_get(lineMailCfgObj, "Event"));
			if(strcmp(tmpEvent,X_ZYXEL_MissedCallEmailEvent) == 0){
				break;
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}
	}

	if(lineMailCfgObj != NULL){
		if(json_object_object_get(Jobj, "MailAccount"))
		json_object_object_add(lineMailCfgObj, "SericeRef", json_object_new_string(SericeRef));
		if(json_object_object_get(Jobj, "Missed_Call_Email_Title"))
		json_object_object_add(lineMailCfgObj, "EmailSubject", json_object_new_string(EmailSubject));
		if(json_object_object_get(Jobj, "Send_Notification_to_Email"))
		json_object_object_add(lineMailCfgObj, "EmailTo", json_object_new_string(EmailTo));

		if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, multiJobj, lineMailCfgObj)) == NULL){
			printf("%s: fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_MAIL_EVENT_CFG);
			zcfgFeJsonObjFree(lineMailCfgObj);
			return ZCFG_INTERNAL_ERROR;
		}
		zcfgFeJsonObjFree(lineMailCfgObj);
	}else{
		if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, NULL)) != ZCFG_SUCCESS){
			printf("%s():(ERROR) zcfgFeObjJsonAdd add mail config fail~ \n ",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}

		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj)) != ZCFG_SUCCESS){
			printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get mail config fail~ \n ",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}

		if(lineMailCfgObj != NULL){
			if(json_object_object_get(Jobj, "MailAccount"))
			json_object_object_add(lineMailCfgObj, "SericeRef", json_object_new_string(SericeRef));
			if(json_object_object_get(Jobj, "Missed_Call_Email_Title"))
			json_object_object_add(lineMailCfgObj, "EmailSubject", json_object_new_string(EmailSubject));
			if(json_object_object_get(Jobj, "Send_Notification_to_Email"))
			json_object_object_add(lineMailCfgObj, "EmailTo", json_object_new_string(EmailTo));
			if(json_object_object_get(Jobj, "MissedCallEmailEvent"))
			json_object_object_add(lineMailCfgObj, "Event", json_object_new_string(X_ZYXEL_MissedCallEmailEvent));
			json_object_object_add(lineMailCfgObj, "Enable", json_object_new_boolean(true));

			if((setJson = zcfgFeJsonMultiObjAppend(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, multiJobj, lineMailCfgObj)) == NULL){
				printf("%s: fail to append object oid(%d)\n", __FUNCTION__, RDM_OID_MAIL_EVENT_CFG);
				zcfgFeJsonObjFree(lineMailCfgObj);
				return ZCFG_INTERNAL_ERROR;
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}
	}

	//printf("%s(): Append mailCfg obj to multiJobj : %s \n ",__FUNCTION__,json_object_to_json_string(multiJobj));
	//printf("%s() exit \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipLineGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
zcfgRet_t zcfgFeDalVoipLineAdd(struct json_object *Jobj,struct json_object * prepareMultiJobj,char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL, *objtmp = NULL;
	int len = 0, i, count = 0;
	obj = json_object_new_array();
	if(json_object_get_string(json_object_object_get(Jobj, "ServiceProviderName"))){
		ret = zcfgFeDalVoipLineGet(NULL, obj, NULL);
		len = json_object_array_length(obj);
		for(i=0;i<len;i++){
			objtmp = json_object_array_get_idx(obj, i);
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "ServiceProviderName")),json_object_get_string(json_object_object_get(objtmp, "Name")))){
				count++;
			}
		}
		if(count >= 4){
			strcat(replyMsg, "Maximum account number of one provider is 4.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	zcfgFeDalCustomizationflagGet();
	initLineGlobalObjects();
	ret = getLineBasicInfo_Add(Jobj, replyMsg);
	if(json_object_get_int(json_object_object_get(Jobj, "Index"))){
		if(replyMsg)
		strcat(replyMsg, "Add function doesn't need Index.");
	}

	if(prepareMultiJobj != NULL){
		multiJobj = prepareMultiJobj;
	}
	else
	{
		multiJobj = zcfgFeJsonObjNew();
	}

	if((ret = zcfgFeDalLineObjAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to add line obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineSipObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line sip obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCallFeatureObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit call feature obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCodecListObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit codec list obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineProcessingEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line processing obj \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeMultiObjJsonBlockedSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) zcfgFeMultiObjJsonSet fail!!\n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeDalLineMailCfgAdd(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to add mail config obj \n", __FUNCTION__);
		goto exit;
	}

	//printf("%s(): Exit.. \n ",__FUNCTION__);

exit:
	freeAllLineObjects();
	return ret;
}

zcfgRet_t zcfgFeDalVoipLineEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgFeDalCustomizationflagGet();
	initLineGlobalObjects();
	ret = getLineBasicInfo(Jobj, replyMsg);

    //printf("\033[0;32m%s : Enter \033[0m \n", __FUNCTION__);

	if(ret != ZCFG_SUCCESS){
		goto exit;
	}
	
	multiJobj = zcfgFeJsonObjNew();

	zcfgFeDalLineObjEditIidGet(Jobj, replyMsg);

	if((ret = zcfgFeDalLineObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineSipObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line sip obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCallFeatureObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit call feature obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineCodecListObjEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit codec list obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineProcessingEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit line processing obj \n", __FUNCTION__);
		goto exit;
	}
	if((ret = zcfgFeDalLineMailCfgEdit(Jobj)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to edit mail config obj \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeMultiObjJsonSet(multiJobj, NULL)) != ZCFG_SUCCESS){
		printf("%s:(ERROR) fail to set multiJobj \n", __FUNCTION__);
		goto exit;
	}

	//printf("%s(): Exit.. \n ",__FUNCTION__);

exit:
	freeAllLineObjects();
	return ret;

}

zcfgRet_t zcfgFeDalMailConfigDel(struct json_object *Jobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	char eventTarget[65] = {0};
	const char* tmpEvent;
	int proId, lineId;
	lineMailCfgObj = NULL;

	proId = lineIid.idx[1];
	lineId = lineIid.idx[2];
	sprintf(eventTarget,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d", proId, lineId);

	IID_INIT(lineMailCfgIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj) == ZCFG_SUCCESS) {
		if(lineMailCfgObj != NULL){
			tmpEvent = json_object_get_string(json_object_object_get(lineMailCfgObj, "Event"));
			if(strcmp(tmpEvent,eventTarget) == 0){
				printf("%s():Delete Mail Event Config - (%s)\n", __FUNCTION__, tmpEvent);
				zcfgFeJsonObjFree(lineMailCfgObj);
				break;
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}
	}

	if((ret = zcfgFeObjJsonDel(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete mail event obj fail \n", __FUNCTION__);
		return ret;
	}

	//printf("%s(): Exit.. \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalVoipLineDel(struct json_object *Jobj, char *replyMsg){
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	//printf("%s(): string :%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));
	zcfgRet_t ret = ZCFG_SUCCESS;
	const char* path;
	//int profileNum, lineNum;
	int index = 0;
	zcfg_offset_t oid;
	IID_INIT(lineIid);
	oid = RDM_OID_VOICE_LINE;
	index = json_object_get_int(json_object_object_get(Jobj, "Index"));
	path = json_object_get_string(json_object_object_get(Jobj, "DelPath"));


	if(path != NULL){
		//Init lineIid
		lineIid.level = 3;
		lineIid.idx[0] = 1;

		//Parse full path
		sscanf(path,"VoiceProfile.%hhu.Line.%hhu",&lineIid.idx[1], &lineIid.idx[2]);

	}else{
		ret = convertIndextoIid(index, &lineIid, oid, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS){
			return ret;
		}
	}
	//printf("%s(): Delete Line path : %s \n ",__FUNCTION__, path);
	//sscanf(path,"VoiceProfile.%d.Line.%d",&profileNum,&lineNum);
	//printf("Delete Line- profile: %d, line: %d \n ", profileNum, lineNum);

	//IID_INIT(lineIid);
	//lineIid.level = 3;
	//lineIid.idx[0] = 1;
	//lineIid.idx[1] = profileNum;
	//lineIid.idx[2] = lineNum;

	if((ret = zcfgFeDalMailConfigDel(Jobj)) != ZCFG_SUCCESS){
		printf("%s: delete mail config obj fail \n", __FUNCTION__);
		goto exit;
	}

	if((ret = zcfgFeObjJsonDel(RDM_OID_VOICE_LINE, &lineIid, NULL)) != ZCFG_SUCCESS){
		printf("%s: delete line obj fail \n", __FUNCTION__);
		goto exit;
	}

exit:
	return ret;
}

zcfgRet_t zcfgFeDalparsePhyIntfObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s() Enter..\n", __FUNCTION__);
	int phyIntfCounter = 0;

	IID_INIT(phyIntfIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PHY_INTF, &phyIntfIid, &phyIntfObj) == ZCFG_SUCCESS){
		if(phyIntfObj != NULL){
			phyIntfCounter++;
			zcfgFeJsonObjFree(phyIntfObj);
		}
	}
	json_object_object_add(paserobj, "phyIntfCounter", json_object_new_int(phyIntfCounter));

	//printf("%s(): Exit.. \n ",__FUNCTION__);
	return ret;
}

zcfgRet_t zcfgFeDalparseSingleLineObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	char sipAccount[32] = {0};
	const char *status = NULL;	
	
	enable = json_object_get_string(json_object_object_get(lineObj, "Enable"));
	DirectoryNumber = json_object_get_string(json_object_object_get(lineObj, "DirectoryNumber"));
	if(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST&flag1){
		PhyReferenceList = json_object_get_string(json_object_object_get(lineObj, "PhyReferenceList"));
	}
	status = json_object_get_string(json_object_object_get(lineObj, "Status"));

	json_object_object_add(paserobj, "Enable", json_object_new_string(enable));
	json_object_object_add(paserobj, "DirectoryNumber", json_object_new_string(DirectoryNumber));
	if(VOIP_SIP_PHONE_MAPPING_BY_STD_PHY_REFERENCE_LIST&flag1){
		json_object_object_add(paserobj, "PhyReferenceList", json_object_new_string(PhyReferenceList));
	}
	json_object_object_add(paserobj, "Status", json_object_new_string(status));
	json_object_object_add(paserobj, "lineIdx", json_object_new_int(lineIid.idx[2]));
	snprintf(sipAccount, sizeof(sipAccount), "SIP%d", sipAccountIdx);
	json_object_object_add(paserobj, "SipAccount", json_object_new_string(sipAccount));


	return ret;
}

zcfgRet_t zcfgFeDalparseSingleLineSIPObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_SIP, &lineIid, &lineSipObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get line sip fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineSipObj != NULL){
		AuthUserName = json_object_get_string(json_object_object_get(lineSipObj, "AuthUserName"));
		AuthPassword = json_object_get_string(json_object_object_get(lineSipObj, "AuthPassword"));
		X_ZYXEL_Url_Type = json_object_get_string(json_object_object_get(lineSipObj, "X_ZYXEL_Url_Type"));

		json_object_object_add(paserobj, "AuthUserName", json_object_new_string(AuthUserName));
		//Change hidden password to show in GUI >> 8 digit, if not emty than show '********'
		if (strlen(AuthPassword) > 0){
			json_object_object_add(paserobj, "AuthPassword", json_object_new_string(hiddenAuthPassword));
		}else{
			json_object_object_add(paserobj, "AuthPassword", json_object_new_string(AuthPassword));
		}
		json_object_object_add(paserobj, "X_ZYXEL_Url_Type", json_object_new_string(X_ZYXEL_Url_Type));

		zcfgFeJsonObjFree(lineSipObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleLineCallFeatureObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_CALLING_FEATURE, &lineIid, &lineCallingFeatureObj)) != ZCFG_SUCCESS){
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get line call feature fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineCallingFeatureObj != NULL){
#ifndef	WIND_ITALY_CUSTOMIZATION		
		CallTransferEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "CallTransferEnable"));
#endif
		CallWaitingEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "CallWaitingEnable"));
		X_ZYXEL_CallWaitingRejectTime = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_CallWaitingRejectTime"));
#ifndef	WIND_ITALY_CUSTOMIZATION		
		CallForwardUnconditionalEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "CallForwardUnconditionalEnable"));
		CallForwardUnconditionalNumber = json_object_get_string(json_object_object_get(lineCallingFeatureObj, "CallForwardUnconditionalNumber"));
		CallForwardOnBusyNumber = json_object_get_string(json_object_object_get(lineCallingFeatureObj, "CallForwardOnBusyNumber"));
		CallForwardOnBusyEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "CallForwardOnBusyEnable"));
		CallForwardOnNoAnswerEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "CallForwardOnNoAnswerEnable"));
		CallForwardOnNoAnswerNumber = json_object_get_string(json_object_object_get(lineCallingFeatureObj, "CallForwardOnNoAnswerNumber"));
		CallForwardOnNoAnswerRingCount = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "CallForwardOnNoAnswerRingCount"));
		AnonymousCallEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "AnonymousCallEnable"));
#endif
		AnonymousCallBlockEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "AnonymousCallBlockEnable"));
		DoNotDisturbEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "DoNotDisturbEnable"));
#ifndef	WIND_ITALY_CUSTOMIZATION		
		MWIEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "MWIEnable"));
		X_ZYXEL_MwiExpireTime = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_MwiExpireTime"));
		X_ZYXEL_WarmLineEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_WarmLineEnable"));
		X_ZYXEL_HotLineEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_HotLineEnable"));
		X_ZYXEL_HotLineNum = json_object_get_string(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_HotLineNum"));
		X_ZYXEL_WarmLineNumber = json_object_get_string(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_WarmLineNumber"));
		X_ZYXEL_TimerFirstDigitWarmLine = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_TimerFirstDigitWarmLine"));
#endif		
		X_ZYXEL_MissedCallEmailEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_MissedCallEmailEnable"));
		if(!(HIDE_VOICE_IVR&flag1)){
			X_ZYXEL_RemoteRingbackTone = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_RemoteRingbackTone"));
			X_ZYXEL_RemoteRingbackToneIndex = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_RemoteRingbackToneIndex"));
			X_ZYXEL_MusicOnHoldTone = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_MusicOnHoldTone"));
			X_ZYXEL_MusicOnHoldToneIndex = json_object_get_int(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_MusicOnHoldToneIndex"));
		}
		if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
			X_ZYXEL_CCBSEnable = json_object_get_boolean(json_object_object_get(lineCallingFeatureObj, "X_ZYXEL_CCBSEnable"));
		}
#ifndef	WIND_ITALY_CUSTOMIZATION
		json_object_object_add(paserobj, "CallTransferEnable", json_object_new_boolean(CallTransferEnable));
#endif
		json_object_object_add(paserobj, "CallWaitingEnable", json_object_new_boolean(CallWaitingEnable));
		json_object_object_add(paserobj, "X_ZYXEL_CallWaitingRejectTime", json_object_new_int(X_ZYXEL_CallWaitingRejectTime));
#ifndef	WIND_ITALY_CUSTOMIZATION		
		json_object_object_add(paserobj, "CallForwardUnconditionalEnable", json_object_new_boolean(CallForwardUnconditionalEnable));
		json_object_object_add(paserobj, "CallForwardUnconditionalNumber", json_object_new_string(CallForwardUnconditionalNumber));
		json_object_object_add(paserobj, "CallForwardOnBusyNumber", json_object_new_string(CallForwardOnBusyNumber));
		json_object_object_add(paserobj, "CallForwardOnBusyEnable", json_object_new_boolean(CallForwardOnBusyEnable));
		json_object_object_add(paserobj, "CallForwardOnNoAnswerEnable", json_object_new_boolean(CallForwardOnNoAnswerEnable));
		json_object_object_add(paserobj, "CallForwardOnNoAnswerNumber", json_object_new_string(CallForwardOnNoAnswerNumber));
		json_object_object_add(paserobj, "CallForwardOnNoAnswerRingCount", json_object_new_int(CallForwardOnNoAnswerRingCount));
		json_object_object_add(paserobj, "AnonymousCallEnable", json_object_new_boolean(AnonymousCallEnable));
#endif
		json_object_object_add(paserobj, "AnonymousCallBlockEnable", json_object_new_boolean(AnonymousCallBlockEnable));
		json_object_object_add(paserobj, "DoNotDisturbEnable", json_object_new_boolean(DoNotDisturbEnable));
#ifndef	WIND_ITALY_CUSTOMIZATION		
		json_object_object_add(paserobj, "MWIEnable", json_object_new_boolean(MWIEnable));
		json_object_object_add(paserobj, "X_ZYXEL_MwiExpireTime", json_object_new_int(X_ZYXEL_MwiExpireTime));
		json_object_object_add(paserobj, "X_ZYXEL_WarmLineEnable", json_object_new_boolean(X_ZYXEL_WarmLineEnable));
		json_object_object_add(paserobj, "X_ZYXEL_HotLineEnable", json_object_new_boolean(X_ZYXEL_HotLineEnable));
		json_object_object_add(paserobj, "X_ZYXEL_HotLineNum", json_object_new_string(X_ZYXEL_HotLineNum));
		json_object_object_add(paserobj, "X_ZYXEL_WarmLineNumber", json_object_new_string(X_ZYXEL_WarmLineNumber));
		json_object_object_add(paserobj, "X_ZYXEL_TimerFirstDigitWarmLine", json_object_new_int(X_ZYXEL_TimerFirstDigitWarmLine));
#endif
		json_object_object_add(paserobj, "X_ZYXEL_MissedCallEmailEnable", json_object_new_boolean(X_ZYXEL_MissedCallEmailEnable));
		if(!(HIDE_VOICE_IVR&flag1)){
			json_object_object_add(paserobj, "X_ZYXEL_RemoteRingbackTone", json_object_new_boolean(X_ZYXEL_RemoteRingbackTone));
			json_object_object_add(paserobj, "X_ZYXEL_RemoteRingbackToneIndex", json_object_new_int(X_ZYXEL_RemoteRingbackToneIndex));
			json_object_object_add(paserobj, "X_ZYXEL_MusicOnHoldTone", json_object_new_boolean(X_ZYXEL_MusicOnHoldTone));
			json_object_object_add(paserobj, "X_ZYXEL_MusicOnHoldToneIndex", json_object_new_int(X_ZYXEL_MusicOnHoldToneIndex));
		}
		if(SHOW_NOT_USE_OFTEN_VOIP_SETTING&flag1){
			json_object_object_add(paserobj, "X_ZYXEL_CCBSEnable", json_object_new_boolean(X_ZYXEL_CCBSEnable));
		}

		zcfgFeJsonObjFree(lineCallingFeatureObj);
	}

	return ret;
}

zcfgRet_t zcfgFeDalparseSingleLineCodecListObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	int codecCounter = 0;
	char *outputCodecVal = NULL;

	IID_INIT(lineCodecListIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE_CODEC_LIST, &lineIid, &lineCodecListIid, &lineCodecListObj) == ZCFG_SUCCESS){
		priority = json_object_get_int(json_object_object_get(lineCodecListObj, "Priority"));
		Codec = (char *)json_object_get_string(json_object_object_get(lineCodecListObj, "Codec"));
		SilenceSuppression = json_object_get_boolean(json_object_object_get(lineCodecListObj, "SilenceSuppression"));
		X_ZYXEL_ComfortNoise = json_object_get_boolean(json_object_object_get(lineCodecListObj, "X_ZYXEL_ComfortNoise"));

		// output param value to follow DAL format
		if (!strcmp(Codec,"G.711MuLaw"))
			outputCodecVal = "G711MuLaw";
		else if (!strcmp(Codec,"G.711ALaw"))
			outputCodecVal = "G711ALaw";
		else if (!strcmp(Codec,"G.729"))
			outputCodecVal = "G729";
		else if (!strcmp(Codec,"G.726-32"))
			outputCodecVal = "G726_32";
		else if (!strcmp(Codec,"G.726-24"))
			outputCodecVal = "G726_24";
		else if (!strcmp(Codec,"G.722"))
			outputCodecVal = "G722";
		else if (!strcmp(Codec,"None"))
			outputCodecVal = "None";
		else
		{
			dbg_printf("%s:%d Unknown Codec value \"%s\"\n",__FUNCTION__,__LINE__,Codec);
			outputCodecVal = Codec;
		}

		if (priority == 1)
		{
			json_object_object_add(paserobj, "primaryCodec", json_object_new_string(outputCodecVal));
		}else if (priority == 2)
		{
			json_object_object_add(paserobj, "secondarydCodec", json_object_new_string(outputCodecVal));
		}else if (priority == 3)
		{
			json_object_object_add(paserobj, "thirdCodec", json_object_new_string(outputCodecVal));
		}

/*
		if(priority == 1){
			json_object_object_add(paserobj, "primaryCodecIndex", json_object_new_int(lineCodecListIid.idx[3]-1));
		}else if(priority == 2){
			json_object_object_add(paserobj, "secondarydCodecIndex", json_object_new_int(lineCodecListIid.idx[3]-1));
		}else if(priority == 3){
			json_object_object_add(paserobj, "thirdCodecIndex", json_object_new_int(lineCodecListIid.idx[3]-1));
		}
*/
		json_object_object_add(paserobj, "SilenceSuppression", json_object_new_boolean(SilenceSuppression));
		json_object_object_add(paserobj, "X_ZYXEL_ComfortNoise", json_object_new_boolean(X_ZYXEL_ComfortNoise));

		codecCounter++;
		zcfgFeJsonObjFree(lineCodecListObj);
	}

    if (json_object_object_get(paserobj, "secondarydCodec") == NULL)
    {
        json_object_object_add(paserobj, "secondarydCodec", json_object_new_string("None"));
    }
    if (json_object_object_get(paserobj, "thirdCodec") == NULL)
    {
        json_object_object_add(paserobj, "thirdCodec", json_object_new_string("None"));
    }

	json_object_object_add(paserobj, "CodecCounter", json_object_new_int(codecCounter));
	return ret;
}


zcfgRet_t zcfgFeDalparseSingleLineProcessingObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_LINE_PROCESSING, &lineIid, &lineProcessingObj)) != ZCFG_SUCCESS){
		printf("%s(): zcfgFeObjJsonGetWithoutUpdate get line processing fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(lineProcessingObj != NULL){
		EchoCancellationEnable = json_object_get_boolean(json_object_object_get(lineProcessingObj, "EchoCancellationEnable"));
		TransmitGain = json_object_get_int(json_object_object_get(lineProcessingObj, "TransmitGain"));
		ReceiveGain = json_object_get_int(json_object_object_get(lineProcessingObj, "ReceiveGain"));
		X_ZYXEL_VoiceJBMode = (char *)json_object_get_string(json_object_object_get(lineProcessingObj, "X_ZYXEL_VoiceJBMode"));
		X_ZYXEL_VoiceJBMin = json_object_get_int(json_object_object_get(lineProcessingObj, "X_ZYXEL_VoiceJBMin"));
		X_ZYXEL_VoiceJBMax = json_object_get_int(json_object_object_get(lineProcessingObj, "X_ZYXEL_VoiceJBMax"));
		X_ZYXEL_VoiceJBTarget = json_object_get_int(json_object_object_get(lineProcessingObj, "X_ZYXEL_VoiceJBTarget"));
		X_ZYXEL_DataJBMode = (char *)json_object_get_string(json_object_object_get(lineProcessingObj, "X_ZYXEL_DataJBMode"));
		X_ZYXEL_DataJBTarget = json_object_get_int(json_object_object_get(lineProcessingObj, "X_ZYXEL_DataJBTarget"));

		json_object_object_add(paserobj, "EchoCancellationEnable", json_object_new_boolean(EchoCancellationEnable));
		json_object_object_add(paserobj, "TransmitGain", json_object_new_int(TransmitGain));
		json_object_object_add(paserobj, "ReceiveGain", json_object_new_int(ReceiveGain));
		json_object_object_add(paserobj, "VoiceJBMode", json_object_new_string(X_ZYXEL_VoiceJBMode));
		json_object_object_add(paserobj, "VoiceJBMin", json_object_new_int(X_ZYXEL_VoiceJBMin));
		json_object_object_add(paserobj, "VoiceJBMax", json_object_new_int(X_ZYXEL_VoiceJBMax));
		json_object_object_add(paserobj, "VoiceJBTarget", json_object_new_int(X_ZYXEL_VoiceJBTarget));
		json_object_object_add(paserobj, "DataJBMode", json_object_new_string(X_ZYXEL_DataJBMode));
		json_object_object_add(paserobj, "DataJBTarget", json_object_new_int(X_ZYXEL_DataJBTarget));

		zcfgFeJsonObjFree(lineProcessingObj);
	}

	return ret;
}


zcfgRet_t zcfgFeDalparseMailcfgObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int profidx, lineidx;
	bool exist = false;
	char eventTarget[128] = {0};

	//printf("%s(): Enter.. \n ",__FUNCTION__);

	IID_INIT(lineMailCfgIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj) == ZCFG_SUCCESS){
		if(lineMailCfgObj != NULL){
			Event = json_object_get_string(json_object_object_get(lineMailCfgObj, "Event"));
			if(strstr(Event, "MAILSEND_EVENT_VOICE_MISSED_CALL") != 0){
				sscanf(Event, "MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d", &profidx, &lineidx);
				if((profidx == lineIid.idx[1])&&(lineidx == lineIid.idx[2])){
					EmailSubject = json_object_get_string(json_object_object_get(lineMailCfgObj, "EmailSubject"));
					SericeRef = json_object_get_string(json_object_object_get(lineMailCfgObj, "SericeRef"));
					EmailTo = json_object_get_string(json_object_object_get(lineMailCfgObj, "EmailTo"));

					json_object_object_add(paserobj, "EmailSubject", json_object_new_string(EmailSubject));
					json_object_object_add(paserobj, "SericeRef", json_object_new_string(SericeRef));
					json_object_object_add(paserobj, "EmailTo", json_object_new_string(EmailTo));
					json_object_object_add(paserobj, "Event", json_object_new_string(Event));
					exist = true;

					zcfgFeJsonObjFree(lineMailCfgObj);
					break;
				}
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}
	}

	if(exist == false){
		IID_INIT(lineMailCfgIid);
		if((ret = zcfgFeObjJsonBlockedAdd(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, NULL)) != ZCFG_SUCCESS){
			printf("%s():(ERROR) zcfgFeObjJsonAdd add mail cfg fail~ \n ",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}

		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, &lineMailCfgObj)) != ZCFG_SUCCESS){
			printf("%s(): zcfgFeObjJsonGetWithoutUpdate get mail cfg fail~ \n ",__FUNCTION__);
			return ZCFG_INTERNAL_ERROR;
		}

		if(lineMailCfgObj != NULL){
			sprintf(eventTarget,"MAILSEND_EVENT_VOICE_MISSED_CALL_%d_%d", lineIid.idx[1], lineIid.idx[2]);

			json_object_object_add(lineMailCfgObj, "EmailSubject", json_object_new_string(MISSED_CALL_MAIL_TITLE_DEFAULT)); //Amber.20171228.Modify to Marco
			json_object_object_add(lineMailCfgObj, "EmailBody", json_object_new_string("You have one missed call on SIP-#{lineid}:#{localnum} from number #{peernum} on #{time}"));
			json_object_object_add(lineMailCfgObj, "Event", json_object_new_string(eventTarget));
			json_object_object_add(lineMailCfgObj, "Enable", json_object_new_boolean(true));


			if((ret = zcfgFeObjJsonSet(RDM_OID_MAIL_EVENT_CFG, &lineMailCfgIid, lineMailCfgObj, NULL)) != ZCFG_SUCCESS ){
				zcfgLog(ZCFG_LOG_ERR, "%s : config mail config\n", __FUNCTION__);
				zcfgFeJsonObjFree(lineMailCfgObj);
				return ZCFG_INTERNAL_ERROR;
			}
			zcfgFeJsonObjFree(lineMailCfgObj);
		}

	}

	return ret;
}


zcfgRet_t zcfgFeDalparseMailserviceObj(struct json_object *paserobj)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char emailList[256] = {0};
	//printf("%s(): Enter...\n", __FUNCTION__);

	IID_INIT(lineMailServiceIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_SERVICE, &lineMailServiceIid, &lineMailserviceObj) == ZCFG_SUCCESS)
	{
		if(lineMailserviceObj != NULL)
		{
			EmailAddress = json_object_get_string(json_object_object_get(lineMailserviceObj, "EmailAddress"));
			//printf("EmailAddress (%d) %s\n", __LINE__, EmailAddress);

			if(strcmp(emailList,"") == 0)
			{
				snprintf(emailList, sizeof(emailList), "%s", EmailAddress);
			}
			else
			{
				//Steven.20191225.Modify: Change to use strcat to structure EmailAddress since the destination and source are the same paramter in sprintf will not work correctly on glibc.
				if((sizeof(emailList)-strlen(emailList)-1) > strlen(EmailAddress))
				{
					strcat(emailList, ",");
					strcat(emailList, EmailAddress);
				}
				//printf("emailList %d (%s) %s\n", __LINE__, emailList, EmailAddress);
			}
			zcfgFeJsonObjFree(lineMailserviceObj);
		}
	}
	json_object_object_add(paserobj, "EmailAddress", json_object_new_string(emailList));

	return ret;
}

zcfgRet_t zcfgFeDalparseProfileObj(struct json_object *paserobj){
	zcfgRet_t ret = ZCFG_SUCCESS;
	//printf("%s(): Enter.. \n ",__FUNCTION__);
	objIndex_t profileIid;
#if IVR_PLAY_INDEX_ONLY_SHOW_DEFAULT
	objIndex_t profPrefixInfoIid;
	char* FacilityAction = NULL;
	char* PrefixRange = NULL;
	bool showDefaultOnly;
#endif

	IID_INIT(profileIid);
	profileIid.level = 2;
	profileIid.idx[0] = lineIid.idx[0];
	profileIid.idx[1] = lineIid.idx[1];
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_VOICE_PROF, &profileIid, &profileObj) != ZCFG_SUCCESS){
		printf("%s(): zcfgFeObjJsonGetWithoutUpdate get profile fail~ \n ",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if(profileObj != NULL){
		profileName = json_object_get_string(json_object_object_get(profileObj, "Name"));
		json_object_object_add(paserobj, "Name", json_object_new_string(profileName));
		json_object_object_add(paserobj, "profileIdx", json_object_new_int(profileIid.idx[1]));
		zcfgFeJsonObjFree(profileObj);
	}

#if IVR_PLAY_INDEX_ONLY_SHOW_DEFAULT //Disable controler to decide only show "Default" option of IVR index on the webGUI or not.
	IID_INIT(profPrefixInfoIid);
	while(zcfgFeSubInObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF_NUM_PLAN_PREFIX_INFO, &profileIid, &profPrefixInfoIid, &profNumPlanpInfoObj) == ZCFG_SUCCESS) {
		if(profNumPlanpInfoObj != NULL){
			FacilityAction = json_object_get_string(json_object_object_get(profNumPlanpInfoObj, "FacilityAction"));
			if(strcmp(FacilityAction,"X_ZYXEL_IVR") == 0){
				PrefixRange = json_object_get_string(json_object_object_get(profNumPlanpInfoObj, "PrefixRange"));
				if(strcmp(PrefixRange,"") == 0){
					showDefaultOnly = true;
				}else{
					showDefaultOnly = false;
				}
				zcfgFeJsonObjFree(profNumPlanpInfoObj);
				break;
			}
			zcfgFeJsonObjFree(profNumPlanpInfoObj);
		}
	}

	if(showDefaultOnly == true){
		json_object_object_add(paserobj, "IVRonlyDefault", json_object_new_boolean(true));
	}else{
		json_object_object_add(paserobj, "IVRonlyDefault", json_object_new_boolean(false));
	}
#else
	json_object_object_add(paserobj, "IVRonlyDefault", json_object_new_boolean(false));
#endif
	return ret;
}

zcfgRet_t zcfgFeDalVoipLineGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int idx = 1, index = 0;

	initLineGlobalObjects();
	zcfgFeDalCustomizationflagGet();
	sipAccountIdx = 1;
	IID_INIT(lineIid);
	if(json_object_object_get(Jobj, "Index")){
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));
		ret = convertIndextoIid(index, &lineIid, RDM_OID_VOICE_LINE, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)
			return ret;
		if(zcfgFeObjJsonGet(RDM_OID_VOICE_LINE, &lineIid, &lineObj) == ZCFG_SUCCESS) {
			if(lineObj != NULL){
				singleLineobj = json_object_new_object();
				json_object_object_add(singleLineobj, "ShowDetail", json_object_new_boolean(true));
				if(singleLineobj == NULL){
					printf("%s:Create new json Obj fail \n", __FUNCTION__);
					goto exit;
				}

				/*if((ret = zcfgFeDalparsePhyIntfObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Obj \n", __FUNCTION__);
					goto exit;
				}*/
				sipAccountIdx = index;
				if((ret = zcfgFeDalparseSingleLineObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleLineSIPObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line SIP Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleLineCallFeatureObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Call Feature Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleLineCodecListObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Codec List Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseSingleLineProcessingObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseMailcfgObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseMailserviceObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
					goto exit;
				}
				if((ret = zcfgFeDalparseProfileObj(singleLineobj)) != ZCFG_SUCCESS){
					printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
					goto exit;
				}
				json_object_object_add(singleLineobj, "Index", json_object_new_int(idx));

				if( ret == ZCFG_SUCCESS ){
					json_object_array_add(Jarray, JSON_OBJ_COPY(singleLineobj));
				}

				zcfgFeJsonObjFree(singleLineobj);
				zcfgFeJsonObjFree(lineObj);
			}
		}
	}
	else {
	while(zcfgFeObjJsonGetNext(RDM_OID_VOICE_LINE, &lineIid, &lineObj) == ZCFG_SUCCESS) {
		if(lineObj != NULL){
			singleLineobj = json_object_new_object();
			if(singleLineobj == NULL){
				printf("%s:Create new json Obj fail \n", __FUNCTION__);
				goto exit;
			}

			/*if((ret = zcfgFeDalparsePhyIntfObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Obj \n", __FUNCTION__);
				goto exit;
			}*/

			if((ret = zcfgFeDalparseSingleLineObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleLineSIPObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line SIP Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleLineCallFeatureObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Call Feature Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleLineCodecListObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Codec List Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseSingleLineProcessingObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseMailcfgObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseMailserviceObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
				goto exit;
			}
			if((ret = zcfgFeDalparseProfileObj(singleLineobj)) != ZCFG_SUCCESS){
				printf("%s:(ERROR) fail to parse single Line Processing Obj \n", __FUNCTION__);
				goto exit;
			}
			json_object_object_add(singleLineobj, "Index", json_object_new_int(idx));

			if( ret == ZCFG_SUCCESS ){
				json_object_array_add(Jarray, JSON_OBJ_COPY(singleLineobj));
			}
			idx++;

			zcfgFeJsonObjFree(singleLineobj);
			sipAccountIdx++;
			zcfgFeJsonObjFree(lineObj);
			}
		}
	}
	//printf("%s(): Jobj :%s \n ",__FUNCTION__,json_object_to_json_string(Jobj));

exit:
	freeAllLineObjects();
	return ret;

}

zcfgRet_t zcfgFeDalVoipLine(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *defaultJobj = NULL;

	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalVoipLineAdd(Jobj, NULL, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalVoipLineEdit(Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalVoipLineDel(Jobj, NULL);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalVoipLineGet(Jobj, Jarray, NULL);
	}
	else if(!strcmp(method, "DEFAULT")) {
		defaultJobj = json_object_new_object();
		ret = zcfgFeDalVoipLineGetDefault(defaultJobj, NULL);
		if((ret == ZCFG_SUCCESS) && (defaultJobj!=NULL)){
			json_object_array_add(Jarray, JSON_OBJ_COPY(defaultJobj));
		}
		json_object_put(defaultJobj);
	}
	return ret;
}


