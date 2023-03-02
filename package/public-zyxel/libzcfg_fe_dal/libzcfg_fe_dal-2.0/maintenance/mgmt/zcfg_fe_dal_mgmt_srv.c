#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#ifdef MSTC_TAAAG_MULTI_USER
#include "zcfg_net.h"
#endif
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"

#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
static int highGpIdx = 1;
static int FTPIdx = 3;
static int SSHIdx = 5;
static int SNMPIdx = 6;
#endif

dal_param_t MGMT_SRV_param[]={		//RDM_OID_REMO_SRV
	{"Name", 					dalType_string,	0,	0,		NULL, NULL, 				dalcmd_Forbid},
	{"WAN_Interface", 			dalType_string,	0,	0,		NULL, "Any_WAN|Multi_WAN", 				0},
	{"BoundInterfaceList", 		dalType_MultiRtWanWWan,		0,	0,		NULL, NULL, 				0},
#ifdef ZYXEL_REMOMGMT_SEPARATE_LAN_WLAN_PRIVILEGE	
	{"Mode",					dalType_string, 0,	0,		NULL, "LAN_withoutWL|LAN_withoutWL_WAN|LAN_withoutWL_TstDm|WL_WAN|WL_Trust_Dm|WL_ONLY|LAN_ONLY|WAN_ONLY|LAN_WAN|Trust_Dm|LAN_TstDm|None",	0},
#else	
	{"Mode",					dalType_string, 0,	0,		NULL, "LAN_ONLY|WAN_ONLY|LAN_WAN|Trust_Dm|LAN_TstDm|None",	0},
#endif
	{"Port", 	 				dalType_int,	1,	65535,	NULL, NULL, 				0},
	{"Index", 	 				dalType_int,	0,	0,		NULL, NULL, 				0},
	{"Protocol", 				dalType_string,	0,	0,		NULL, NULL, 				dalcmd_Forbid},
	{"RestartDeamon", 		 	dalType_boolean,0,	0,		NULL, NULL, 				dalcmd_Forbid},
#ifdef	CAAAB_CUSTOMIZATION
	{"LifeTime", 	 			dalType_int,	0,	0,		NULL, NULL, 				0},		//if(CustomerName.search("CBT") != -1)
#endif
	{NULL,						0,				0,	0,		NULL, NULL, 				0},
};
#ifdef CONFIG_ABZQ_CUSTOMIZATION
const char *userName = NULL;
const char *accessMode = NULL;
const char *srvName = NULL;

void getSysLogInfo(struct json_object *Jobj, struct json_object *reqObject){
		//the user info is set in index 0
		reqObject = json_object_array_get_idx(Jobj, 0);
		
		if(json_object_get_string(json_object_object_get(reqObject, "DAL_LOGIN_USERNAME")))
			userName = json_object_get_string(json_object_object_get(reqObject, "DAL_LOGIN_USERNAME"));
		if(json_object_get_string(json_object_object_get(reqObject, "AccessMode")))
			accessMode = json_object_get_string(json_object_object_get(reqObject, "AccessMode"));
		if(json_object_get_string(json_object_object_get(reqObject, "SrvName")))
			srvName = json_object_get_string(json_object_object_get(reqObject, "SrvName"));
}
#endif

#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169
inline char* dalSec_getStatus(const bool lanfound, const bool wanfound)
{
   if (wanfound && !lanfound)
   {
      return MODE_WAN_ONLY;
   }
   else if (lanfound && !wanfound)
   {
      return MODE_LAN_ONLY;
   }
   else if (!lanfound && !wanfound)
   {
      return NULL;
   }
   else
   {
      return MODE_LAN_WAN;
   }
}
#endif


zcfgRet_t zcfgFeDal_mgmtSrv_Get(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *obj = NULL;
	struct json_object *pramJobj = NULL;
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};
#endif
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
    int SPport;
#endif
#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169
	struct json_object *UsersUserObj=NULL;
	objIndex_t UsersUser_iidStack;
	char pawd[64] = {0};
	char *decopassword = NULL;
	char proLA[64] = {0};
	char proRA[64] = {0};
	bool httpLanFound=false, httpsLanFound=false, ftpLanFound=false, telnetLanFound=false, sshLanFound=false;
	bool httpWanFound=false, httpsWanFound=false, ftpWanFound=false, telnetWanFound=false, sshWanFound=false;
	bool found = false;
	char *copy_protocol = NULL;
	char *token = NULL;
	char username[128] = {0};
	char httpmode[64] = {0},httpsmode[64] = {0},telnetmode[64] = {0},ftpmode[64] = {0}, sshmode[64] = {0};

	strcpy(username, json_object_get_string(json_object_object_get(Jobj, "DAL_LOGIN_USERNAME")));

#endif

#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
#endif

#ifdef MSTC_TAAAG_MULTI_USER //__NSTC__, Bruce for item 169
		while (!found && (zcfgFeObjJsonGetNext(RDM_OID_TAAAB_USER, &UsersUser_iidStack, &UsersUserObj) == ZCFG_SUCCESS))
		{
			if(strcmp(username, json_object_get_string(json_object_object_get(UsersUserObj, "Username"))) == 0){
				if(json_object_get_boolean(json_object_object_get(UsersUserObj, "LocalAccessCapable"))){
					strncpy(proLA, json_object_get_string(json_object_object_get(UsersUserObj, "Allowed_LA_Protocols")), sizeof(proLA));
				}
				if(json_object_get_boolean(json_object_object_get(UsersUserObj, "RemoteAccessCapable"))){
					strncpy(proRA, json_object_get_string(json_object_object_get(UsersUserObj, "Allowed_RA_Protocols")), sizeof(proRA));
				}
				strncpy(pawd, json_object_get_string(json_object_object_get(UsersUserObj, "Password")), 64);
				found = true;
			}
			zcfgFeJsonObjFree(UsersUserObj);
		}
				
		if(found){
			//Local
			if(proLA != NULL){
				copy_protocol = strdup(proLA);	
				if(copy_protocol != NULL){
					token = strtok(copy_protocol, ",");
					while (token != NULL) {						
						if(strcmp(token, "HTTP") == 0){
							httpLanFound = true;
						}
						else if(strcmp(token, "HTTPS") == 0){
							httpsLanFound = true;
						}
						else if(strcmp(token, "FTP") == 0){
							ftpLanFound = true;
						}
						else if(strcmp(token, "TELNET") == 0){
							telnetLanFound = true;
						}
						else if(strcmp(token, "SSH") == 0){
							sshLanFound = true;
						}
						else{
							free(copy_protocol);
							//zcfgFeJsonObjFree(obj);
							return ZCFG_INVALID_PARAM_VALUE;
						}
						token = strtok(NULL, ",");
					}
					free(copy_protocol);
				}	
			}
	
			//Remote
			if(proLA != NULL){
				copy_protocol = strdup(proRA);
				if(copy_protocol != NULL){
					token = strtok(copy_protocol, ",");					
					while (token != NULL) {						
						if(strcmp(token, "HTTP") == 0){
							httpWanFound = true;
						}
						else if(strcmp(token, "HTTPS") == 0){
							httpsWanFound = true;
						}
						else if(strcmp(token, "FTP") == 0){
							ftpWanFound = true;
						}
						else if(strcmp(token, "TELNET") == 0){
							telnetWanFound = true;
						}
						else if(strcmp(token, "SSH") == 0){
							sshWanFound = true;
						}
						else{
							free(copy_protocol);
							//zcfgFeJsonObjFree(obj);
							return ZCFG_INVALID_PARAM_VALUE;
						}
						token = strtok(NULL, ",");
					}
					free(copy_protocol);
				}	
			}
			if(dalSec_getStatus(httpLanFound,httpWanFound) != NULL)
				strcpy(httpmode, dalSec_getStatus(httpLanFound,httpWanFound));
			if(dalSec_getStatus(httpsLanFound,httpsWanFound) != NULL)
				strcpy(httpsmode, dalSec_getStatus(httpsLanFound,httpsWanFound));
			if(dalSec_getStatus(telnetLanFound,telnetWanFound) != NULL)
				strcpy(telnetmode, dalSec_getStatus(telnetLanFound,telnetWanFound));
			if(dalSec_getStatus(ftpLanFound,ftpWanFound) != NULL)
				strcpy(ftpmode, dalSec_getStatus(ftpLanFound,ftpWanFound));
			if(dalSec_getStatus(sshLanFound,sshWanFound) != NULL)
				strcpy(sshmode, dalSec_getStatus(sshLanFound,sshWanFound));
		}
#endif

#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_SP_REMO_SRV, &iid, &obj) == ZCFG_SUCCESS) {
		if(!strcmp(json_object_get_string(json_object_object_get(obj, "Name")), "HTTP")){
			SPport = json_object_get_int(json_object_object_get(obj, "Port"));
			zcfgFeJsonObjFree(obj);
            break;
		}
		zcfgFeJsonObjFree(obj);
	}
#endif

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV, &iid, &obj) == ZCFG_SUCCESS) {
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
		if(!strcmp(loginLevel, "high") || !strcmp(loginLevel, "medium")){
			if( highGpIdx == iid.idx[0]){
				zcfgFeJsonObjFree(obj);
				continue;
			}
		}else{
			if( (FTPIdx == iid.idx[0]) || (SSHIdx == iid.idx[0]) || (SNMPIdx == iid.idx[0])){
				zcfgFeJsonObjFree(obj);
				continue;
			}
		}
#endif

		pramJobj = json_object_new_object();
		json_object_object_add(pramJobj, "Index", json_object_new_int(iid.idx[0]));
#ifdef ACAU_CUSTOMIZATION
		if ( strcmp("FTP", json_object_get_string(json_object_object_get(obj, "Name"))) == 0 )
			json_object_object_add(pramJobj, "Name", json_object_new_string("FTPS"));
		else
			json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(obj, "Name")));
#else
		json_object_object_add(pramJobj, "Name", JSON_OBJ_COPY(json_object_object_get(obj, "Name")));
#endif
		json_object_object_add(pramJobj, "Port", JSON_OBJ_COPY(json_object_object_get(obj, "Port")));
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
        json_object_object_add(pramJobj, "WANPort", JSON_OBJ_COPY(json_object_object_get(obj, "WANPort")));
        json_object_object_add(pramJobj, "SPport", json_object_new_int(SPport));
#endif
		//json_object_object_add(pramJobj, "Mode", JSON_OBJ_COPY(json_object_object_get(obj, "Mode")));
#ifdef MSTC_TAAAG_MULTI_USER //__NSTC__, Bruce for item 169		
		if(strcmp("HTTP", json_object_get_string(json_object_object_get(obj, "Name"))) == 0)
			json_object_object_add(pramJobj, "Mode", json_object_new_string(httpmode));
		else if(strcmp("HTTPS", json_object_get_string(json_object_object_get(obj, "Name"))) == 0)
			json_object_object_add(pramJobj, "Mode", json_object_new_string(httpsmode));
		else if(strcmp("TELNET", json_object_get_string(json_object_object_get(obj, "Name"))) == 0)
			json_object_object_add(pramJobj, "Mode", json_object_new_string(telnetmode));
		else if(strcmp("FTP", json_object_get_string(json_object_object_get(obj, "Name"))) == 0)
			json_object_object_add(pramJobj, "Mode", json_object_new_string(ftpmode));
		else if(strcmp("SSH", json_object_get_string(json_object_object_get(obj, "Name"))) == 0)
			json_object_object_add(pramJobj, "Mode", json_object_new_string(sshmode));
		else	
		json_object_object_add(pramJobj, "Mode", JSON_OBJ_COPY(json_object_object_get(obj, "Mode")));
#else
		json_object_object_add(pramJobj, "Mode", JSON_OBJ_COPY(json_object_object_get(obj, "Mode")));
#endif		
		json_object_object_add(pramJobj, "BoundInterfaceList", JSON_OBJ_COPY(json_object_object_get(obj, "BoundInterfaceList")));
		json_object_array_add(Jarray, pramJobj);
		zcfgFeJsonObjFree(obj);
	}
	
	return ret;
}

void zcfgFeDalShowMgmtSrv(struct json_object *Jarray){

	int i, len = 0;
	struct json_object *obj;
	int Index, Port;
	const char *Name, *Mode, *BoundIntflist = NULL;
	char mode[32] = {0};
	char port[32] = {0};

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}
	obj = json_object_array_get_idx(Jarray, 0);
	
	BoundIntflist = json_object_get_string(json_object_object_get(obj, "BoundInterfaceList"));
	if(!strcmp(BoundIntflist,""))
		printf("WAN Interface used for services: Any WAN \n");
	else
		printf("WAN Interface used for services: Multi WAN, %s \n",BoundIntflist);

	printf("%-7s %-10s %-12s %-10s\n",
		    "Index", "Name", "Mode", "Port");
	
	len = json_object_array_length(Jarray);
	for(i=0;i<len;i++){
		obj = json_object_array_get_idx(Jarray, i);
		if(obj != NULL){
			Index = json_object_get_int(json_object_object_get(obj, "Index"));
			Port = json_object_get_int(json_object_object_get(obj, "Port"));
			Name = json_object_get_string(json_object_object_get(obj, "Name"));
			Mode = json_object_get_string(json_object_object_get(obj, "Mode"));

			if(strcmp(Mode, ""))
				strcpy(mode, Mode);
			else
				strcpy(mode, "None");

			if(strcmp(Name, "PING"))
				sprintf(port, "%d", Port);
			else
				strcpy(port, "N/A");

			printf("%-7u %-10s %-12s %-10s\n",Index,Name,mode,port);
		}

	}
	
}

#ifdef MSTC_TAAAG_MULTI_USER //__NSTC__, Bruce for item 169
zcfgRet_t dalSec_updateUsersUserObject(struct json_object *Jobj)
{
	struct json_object *UsersUserObj=NULL;
	objIndex_t UsersUser_iidStack;
	struct json_object *ManagementJarray = NULL;
	objIndex_t ManagementIid;
	zcfgRet_t ret = ZCFG_SUCCESS;
	char httpStatus[16] = {0};
	char httpsStatus[16] = {0};
	char ftpStatus[16] = {0};
	char telnetStatus[16] = {0};
	char sshStatus[16] = {0};
	char lanProtocolStr[64] = {0};
	char wanProtocolStr[64] = {0};
	bool loAccessCapable = false;
	bool reAccessCapable = false;
	bool found = false;
	struct json_object *reqObject = NULL;
	int index = 0;
	int len = 0;
	char username[128] = {0};
	char loginlevel[128] = {0};
	bool ManagementLimitation = true;
	len = json_object_array_length(Jobj);
	
	IID_INIT(ManagementIid);
	if ( zcfgFeObjJsonGet(RDM_OID_REMOTE_MGMT, &ManagementIid,&ManagementJarray) == ZCFG_SUCCESS)
	{
		ManagementLimitation = Jgetb(ManagementJarray, "Limitation");			
		zcfgFeJsonObjFree(ManagementJarray);
	}
	
	for(int reqObjIdx = 0;reqObjIdx<len;reqObjIdx++){
		reqObject = json_object_array_get_idx(Jobj, reqObjIdx);
		index = json_object_get_int(json_object_object_get(reqObject, "Index"));	
		if(index == 1){
			strcpy(username, json_object_get_string(json_object_object_get(reqObject, "DAL_LOGIN_USERNAME")));
			strcpy(loginlevel,json_object_get_string(json_object_object_get(reqObject, "DAL_LOGIN_LEVEL")));
		}	

		if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"HTTP")){
			if(ManagementLimitation == true){
			if(!strcmp(loginlevel,"medium")){
				struct json_object *mgmtsrvobj=NULL;
				objIndex_t MgmtsrvIid;
				IID_INIT(MgmtsrvIid);
				char *currMode = NULL,*Mode = NULL;
				Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
				MgmtsrvIid.level = 1;
				MgmtsrvIid.idx[0] = 1;
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
					currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
					//printf("can not get RDM_OID_REMO_SRV obj");
						zcfgFeJsonObjFree(mgmtsrvobj);
				}
				if(Mode != NULL && currMode != NULL ){
					if(strcmp(Mode,currMode))
						Mode = currMode;
				}
				else if(Mode == NULL && currMode != NULL){
					Mode = currMode;
				}
				else if(Mode != NULL && currMode == NULL){
					Mode = currMode;
				}

				if(Mode != NULL)
					json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
				else
					json_object_object_add(reqObject, "Mode", json_object_new_string(""));
			}			
			}
			strcpy(httpStatus, json_object_get_string(json_object_object_get(reqObject, "Mode")));
		}
		else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"HTTPS"))		
			strcpy(httpsStatus, json_object_get_string(json_object_object_get(reqObject, "Mode")));
		else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"FTP"))
			strcpy(ftpStatus, json_object_get_string(json_object_object_get(reqObject, "Mode")));
		else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"TELNET")){
			if(ManagementLimitation == true){
			if(!strcmp(loginlevel,"medium")){
				struct json_object *mgmtsrvobj=NULL;
				objIndex_t MgmtsrvIid;
				IID_INIT(MgmtsrvIid);
				char *currMode = NULL,*Mode = NULL;
				Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
				MgmtsrvIid.level = 1;
				MgmtsrvIid.idx[0] = 4;
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
					currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
					//printf("can not get RDM_OID_REMO_SRV obj");
						zcfgFeJsonObjFree(mgmtsrvobj);
				}
				if(Mode != NULL && currMode != NULL ){
					if(strcmp(Mode,currMode))
						Mode = currMode;
				}
				else if(Mode == NULL && currMode != NULL){
					Mode = currMode;
				}
				else if(Mode != NULL && currMode == NULL){
					Mode = currMode;
				}

				if(Mode != NULL)
					json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
				else
					json_object_object_add(reqObject, "Mode", json_object_new_string(""));
			}			
			}
			strcpy(telnetStatus, json_object_get_string(json_object_object_get(reqObject, "Mode")));
		}	
		else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"SSH")){
			if(ManagementLimitation == true){
			if(!strcmp(loginlevel,"medium")){
				struct json_object *mgmtsrvobj=NULL;
				objIndex_t MgmtsrvIid;
				IID_INIT(MgmtsrvIid);
				char *currMode = NULL,*Mode = NULL;
				Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
				MgmtsrvIid.level = 1;
				MgmtsrvIid.idx[0] = 5;
				if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
					currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
					//printf("can not get RDM_OID_REMO_SRV obj");
						zcfgFeJsonObjFree(mgmtsrvobj);
				}
				if(Mode != NULL && currMode != NULL ){
					if(strcmp(Mode,currMode))
						Mode = currMode;
				}
				else if(Mode == NULL && currMode != NULL){
					Mode = currMode;
				}
				else if(Mode != NULL && currMode == NULL){
					Mode = currMode;
				}

				if(Mode != NULL)
					json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
				else
					json_object_object_add(reqObject, "Mode", json_object_new_string(""));
			}			
			}
			strcpy(sshStatus, json_object_get_string(json_object_object_get(reqObject, "Mode")));
		}
		else
			zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}	

	/*http*/
	if(strcmp(httpStatus, MODE_LAN_WAN) == 0){
		strncat(lanProtocolStr, "HTTP,", sizeof(lanProtocolStr));
		strncat(wanProtocolStr, "HTTP,", sizeof(wanProtocolStr));
	}
	else if(strcmp(httpStatus, MODE_WAN_ONLY) == 0){
		strncat(wanProtocolStr, "HTTP,", sizeof(wanProtocolStr));
	}
	else if(strcmp(httpStatus, MODE_LAN_ONLY) == 0){
		strncat(lanProtocolStr, "HTTP,", sizeof(lanProtocolStr));
	}
	else{
		zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}

	/*https*/
	if(strcmp(httpsStatus, MODE_LAN_WAN) == 0){
		strncat(lanProtocolStr, "HTTPS,", sizeof(lanProtocolStr));
		strncat(wanProtocolStr, "HTTPS,", sizeof(wanProtocolStr));
	}
	else if(strcmp(httpsStatus, MODE_WAN_ONLY) == 0){
		strncat(wanProtocolStr, "HTTPS,", sizeof(wanProtocolStr));
	}
	else if(strcmp(httpsStatus, MODE_LAN_ONLY) == 0){
		strncat(lanProtocolStr, "HTTPS,", sizeof(lanProtocolStr));
	}
	else{
		zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}

	/*ftp*/
	if(strcmp(ftpStatus, MODE_LAN_WAN) == 0){
		strncat(lanProtocolStr, "FTP,", sizeof(lanProtocolStr));
		strncat(wanProtocolStr, "FTP,", sizeof(wanProtocolStr));
	}
	else if(strcmp(ftpStatus, MODE_WAN_ONLY) == 0){
		strncat(wanProtocolStr, "FTP,", sizeof(wanProtocolStr));
	}
	else if(strcmp(ftpStatus, MODE_LAN_ONLY) == 0){
		strncat(lanProtocolStr, "FTP,", sizeof(lanProtocolStr));
	}
	else{
		zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}

	/*telnet*/
	if(strcmp(telnetStatus, MODE_LAN_WAN) == 0){
		strncat(lanProtocolStr, "TELNET,", sizeof(lanProtocolStr));
		strncat(wanProtocolStr, "TELNET,", sizeof(wanProtocolStr));
	}
	else if(strcmp(telnetStatus, MODE_WAN_ONLY) == 0){
		strncat(wanProtocolStr, "TELNET,", sizeof(wanProtocolStr));
	}
	else if(strcmp(telnetStatus, MODE_LAN_ONLY) == 0){
		strncat(lanProtocolStr, "TELNET,", sizeof(lanProtocolStr));
	}
	else{
		zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}

	/*ssh*/
	if(strcmp(sshStatus, MODE_LAN_WAN) == 0){
		strncat(lanProtocolStr, "SSH,", sizeof(lanProtocolStr));
		strncat(wanProtocolStr, "SSH,", sizeof(wanProtocolStr));
	}
	else if(strcmp(sshStatus, MODE_WAN_ONLY) == 0){
		strncat(wanProtocolStr, "SSH,", sizeof(wanProtocolStr));
	}
	else if(strcmp(sshStatus, MODE_LAN_ONLY) == 0){
		strncat(lanProtocolStr, "SSH,", sizeof(lanProtocolStr));
	}
	else{
		zcfgLog(ZCFG_LOG_INFO, "Do nothing!");
	}

	if(strlen(lanProtocolStr)>0){
		lanProtocolStr[strlen(lanProtocolStr)-1] = '\0';
		loAccessCapable = true;
	}
	if(strlen(wanProtocolStr)>0){
		wanProtocolStr[strlen(wanProtocolStr)-1] = '\0';
		reAccessCapable = true;
	}

	IID_INIT(UsersUser_iidStack);
	while (!found && zcfgFeObjJsonGetNext(RDM_OID_TAAAB_USER, &UsersUser_iidStack, &UsersUserObj) == ZCFG_SUCCESS)
	{
		if(strcmp(json_object_get_string(json_object_object_get(UsersUserObj, "Username")), username) == 0)
		{	
			json_object_object_add(UsersUserObj, "RemoteAccessCapable", json_object_new_boolean(reAccessCapable));
			json_object_object_add(UsersUserObj, "LocalAccessCapable", json_object_new_boolean(loAccessCapable));
			json_object_object_add(UsersUserObj, "Allowed_RA_Protocols", json_object_new_string(wanProtocolStr));
			json_object_object_add(UsersUserObj, "Allowed_LA_Protocols", json_object_new_string(lanProtocolStr));
			if ((ret = zcfgFeObjJsonSet(RDM_OID_TAAAB_USER, &UsersUser_iidStack, UsersUserObj, NULL)) != ZCFG_SUCCESS)
			{
				printf("failed to set <RDM_OID_TAAAB_USER> object. ret:%d", ret);
			}
			found = true;
		}
		zcfgFeJsonObjFree(UsersUserObj);
	}

	return ret;
}

void dalSec_updateRemoteObject(const bool isEnable)
{
	objIndex_t iidStack;
	struct json_object *RemoteAccessobj = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	bool remoenable = false;
	char TempStr[64] = {0}, TempStr2[64] = {0};
	char httpport[32] = {0};
	char httpsport[32] = {0};
	char telnetport[32] = {0};
	char ftpport[32] = {0};
	
	IID_INIT(iidStack);
	if (zcfgFeObjJsonGet(RDM_OID_REMOTE_ACCESS, &iidStack, &RemoteAccessobj) == ZCFG_SUCCESS)	
	{
		remoenable = json_object_get_boolean(json_object_object_get(RemoteAccessobj, "Enable"));
		if(remoenable != isEnable){
			remoenable = isEnable;
			
			json_object_object_add(RemoteAccessobj, "Enable", json_object_new_boolean(remoenable));
			if ((ret = zcfgFeObjJsonSet(RDM_OID_REMOTE_ACCESS, &iidStack, RemoteAccessobj, NULL)) != ZCFG_SUCCESS)
			{
				printf("failed to set <RDM_OID_REMOTE_ACCESS> object. ret:%d", ret);
			}
		}

	  	zcfgFeJsonObjFree(RemoteAccessobj);
	}
}

void dalSec_getOtherUserPro(struct json_object *Jobj, char *proLA, char *proRA)
{
	struct json_object *UsersUserObj=NULL;
	objIndex_t UsersUser_iidStack;
	bool found = false;
	char username[128] = {0};
	struct json_object *reqObject = NULL;
	int index = 0;
	
	reqObject = json_object_array_get_idx(Jobj, 0);
	index = json_object_get_int(json_object_object_get(reqObject, "Index"));	
	if(index == 1)
		strcpy(username, json_object_get_string(json_object_object_get(reqObject, "DAL_LOGIN_USERNAME")));

	IID_INIT(UsersUser_iidStack);
	while (!found && (zcfgFeObjJsonGetNext(RDM_OID_TAAAB_USER, &UsersUser_iidStack, &UsersUserObj) == ZCFG_SUCCESS))
	{
	
		if(strcmp(json_object_get_string(json_object_object_get(UsersUserObj, "Username")), username)){
			strncpy(proLA, json_object_get_string(json_object_object_get(UsersUserObj, "Allowed_LA_Protocols")), 64);
			strncpy(proRA, json_object_get_string(json_object_object_get(UsersUserObj, "Allowed_RA_Protocols")), 64);
			found = true;
		}
		zcfgFeJsonObjFree(UsersUserObj);
	}
	return;
}
#endif

#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169
inline char* dalSec_getStatus_zyxel(const char *mode, const bool lanCheck, const bool wanCheck)
{ 
   if ((!strcmp(mode,MODE_WAN_ONLY) && !lanCheck) && (!strcmp(mode,MODE_WAN_ONLY) || wanCheck))
   {
      return MODE_WAN_ONLY;
   }
   else if ((!strcmp(mode,MODE_LAN_ONLY) || lanCheck) && (!strcmp(mode,MODE_LAN_ONLY) && !wanCheck))
   {
      return MODE_LAN_ONLY;
   }
   else if ((!strcmp(mode,"") && !lanCheck) && (!strcmp(mode,"") && !wanCheck))
   {
      return NULL;
   }
   else
   {
      return MODE_LAN_WAN;
   }   
}
#endif

zcfgRet_t zcfgFeDal_mgmtSrv_Edit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *mgmtSrvObj = NULL;
	struct json_object *reqObject = NULL;
	struct json_object *obj = NULL;
	objIndex_t mgmtSrvIid = {0};
	objIndex_t Iid = {0};
#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169
	char proLA[64] = {0};
	char proRA[64] = {0};
	bool httpLanCheck = false;
	bool httpWanCheck = false;
	bool httpsLanCheck = false;
	bool httpsWanCheck = false;
	bool ftpLanCheck = false;
	bool ftpWanCheck = false;
	bool telnetLanCheck = false;
	bool telnetWanCheck = false;
	bool sshLanCheck = false;
	bool sshWanCheck = false;
	char *copy_protocol = NULL;
	char *token = NULL;
	struct json_object *userObject = NULL;
	struct json_object *UsersUserObj=NULL;
	objIndex_t UsersUser_iidStack;
	char username[128] = {0};
	char pawd[128] = {0};
	char *decopassword = NULL;
	bool found = false;
	char loginlevel[21] = {0};
	struct json_object *loginlevelObj=NULL;
	int index1 = 0;
	struct json_object *ManagementJarray = NULL;
	objIndex_t ManagementIid;	
	bool ManagementLimitation = true;
#endif

	const char *boundinterfacelist = NULL;
	const char *mode = NULL;
	const char *protocol = NULL;
	const char *wnaIntf = NULL;
	int port = 0;
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
    int wanport = 0;
    int origwanport = 0;
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	struct json_object *httpsObject = NULL;
	struct json_object *httpsSrvObj = NULL;
	objIndex_t httpsSrvIid = {0};
	int httpswanport = 0;
	int orighttpswanport = 0;
#endif
	bool restartdeamon = false;
	int index = 0;
	int origport = 0;
	int otherorigport = 0;
	int httpport = 0;
	int httpsport = 0;
#ifdef	CAAAB_CUSTOMIZATION
	int lifetime = 0;
#endif
	int len = 0;
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	const char *oldMode = NULL;
	const char *name = NULL;
	bool changed = false;
	char cmdStr[256] = {0};
#endif	

	if(json_object_get_type(Jobj) == json_type_object)
		len = json_object_object_length(Jobj);
	else if(json_object_get_type(Jobj) == json_type_array)
		len = json_object_array_length(Jobj);
	else
	{
		printf("wrong Jobj format!\n");
		return ZCFG_INVALID_FORMAT;
	}

	int tmp_idx[len];
	for(int i=0;i<len;i++){
		 if(i == len-2)		
		 	tmp_idx[i] = 1;
		 else if(i == len-1)
		 	tmp_idx[i] = 0;
		 else
		 	tmp_idx[i] = i+2;
	}		

	if(json_object_get_type(Jobj) == json_type_object){// set by dalcmd
		const char *Name = NULL;
		if(json_object_object_get(Jobj, "WAN_Interface")){
			if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "WAN_Interface")),"Multi_WAN")){
				wnaIntf = json_object_get_string(json_object_object_get(Jobj, "BoundInterfaceList"));
			}else if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "WAN_Interface")),"Any_WAN")){
				wnaIntf = "";

			}
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_REMO_SRV, &Iid, &obj) == ZCFG_SUCCESS){
				json_object_object_add(obj, "BoundInterfaceList", json_object_new_string(wnaIntf));
				zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV, &Iid, obj, NULL);
				zcfgFeJsonObjFree(obj);
			}
		}
		if(!json_object_object_get(Jobj, "WAN_Interface") && !json_object_object_get(Jobj, "Index")){
			if(replyMsg)
				strcpy(replyMsg, "Please enter WAN_Interface or Intex.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
		if((index = json_object_get_int(json_object_object_get(Jobj, "Index"))) != 0){
			mode = json_object_get_string(json_object_object_get(Jobj, "Mode"));
			port = json_object_get_int(json_object_object_get(Jobj, "Port"));
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
            wanport = json_object_get_int(json_object_object_get(Jobj, "WANPort"));

            if(!mode && !port && !wanport)//nothing changed
	            return ZCFG_SUCCESS;

#else
			if(!mode && !port)//nothing changed
				return ZCFG_SUCCESS;
#endif
			
			
			IID_INIT(mgmtSrvIid);
			mgmtSrvIid.idx[0] = index;
			mgmtSrvIid.level = 1;
			if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &mgmtSrvIid, &mgmtSrvObj) != ZCFG_SUCCESS){
				if(replyMsg)
					sprintf(replyMsg, "Can't find remote Mgmt rule of index %d\n", index);
				return ZCFG_NOT_FOUND;
			}

			json_object_object_add(mgmtSrvObj, "RestartDeamon", json_object_new_boolean(false));
			origport = json_object_get_int(json_object_object_get(mgmtSrvObj, "Port"));
			if((port != 0) && (origport != port)){
				json_object_object_add(mgmtSrvObj, "Port", json_object_new_int(port));
				Name = json_object_get_string(json_object_object_get(mgmtSrvObj, "Name"));
				if(!strcmp(Name, "HTTP") || !strcmp(Name, "HTTPS")){
					json_object_object_add(mgmtSrvObj, "RestartDeamon", json_object_new_boolean(true));
				}
			}
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
			origwanport = json_object_get_int(json_object_object_get(mgmtSrvObj, "WANPort"));
			if((wanport != 0) && (origwanport != wanport)){
				json_object_object_add(mgmtSrvObj, "WANPort", json_object_new_int(wanport));
				Name = json_object_get_string(json_object_object_get(mgmtSrvObj, "Name"));
				if(!strcmp(Name, "HTTP") || !strcmp(Name, "HTTPS")){
					json_object_object_add(mgmtSrvObj, "RestartDeamon", json_object_new_boolean(true));
				}
			}       
#endif
			if(mode != NULL){
				if(!strcmp(mode, "None"))
					json_object_object_add(mgmtSrvObj, "Mode", json_object_new_string(""));
				else
					json_object_object_add(mgmtSrvObj, "Mode", json_object_new_string(mode));
			}

			if((ret = zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV, &mgmtSrvIid, mgmtSrvObj, NULL)) != ZCFG_SUCCESS){
				if(replyMsg)
					sprintf(replyMsg, "Set remote Mgmt rule fail!");
                zcfgFeJsonObjFree(mgmtSrvObj);
				return ret;
			}
			zcfgFeJsonObjFree(mgmtSrvObj);				
			return ret;
		}
		return ZCFG_SUCCESS;
	}

	reqObject = json_object_array_get_idx(Jobj, len-1);
	protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));
	origport = json_object_get_int(json_object_object_get(reqObject, "origport"));
	otherorigport = json_object_get_int(json_object_object_get(reqObject, "otherorigport"));
	httpport = json_object_get_int(json_object_object_get(reqObject, "httpport"));
	httpsport = json_object_get_int(json_object_object_get(reqObject, "httpsport"));

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	httpsObject = json_object_array_get_idx(Jobj, len-7);
	httpswanport = json_object_get_int(json_object_object_get(httpsObject, "WANPort"));
	IID_INIT(httpsSrvIid);
	httpsSrvIid.idx[0] = 2;
	httpsSrvIid.level = 1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &httpsSrvIid, &httpsSrvObj) == ZCFG_SUCCESS){
		orighttpswanport = json_object_get_int(json_object_object_get(httpsSrvObj, "WANPort"));
		zcfgFeJsonObjFree(httpsSrvObj);				
	}	
#endif


	if(!strcmp(protocol,"http")){
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		if((origport-httpport == 0) && ((otherorigport-httpsport != 0) || (orighttpswanport-httpswanport != 0))){
#else
		if((origport-httpport == 0) && (otherorigport-httpsport != 0)){
#endif
			tmp_idx[len-2] = 0;
			tmp_idx[len-1] = 1;
		}
	}
	else if(!strcmp(protocol,"https")){
		tmp_idx[len-2] = 0;
		tmp_idx[len-1] = 1;
		if((origport-httpsport==0) && (otherorigport-httpport != 0)){
			tmp_idx[len-2] = 1;
			tmp_idx[len-1] = 0;
		}
		}
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	getSysLogInfo(Jobj, reqObject);
#endif
			
#ifdef MSTC_TAAAG_MULTI_USER //__NSTC__, Bruce for item 169
			/*need to update current user object at the same time*/
			if((ret = dalSec_updateUsersUserObject(Jobj)) != ZCFG_SUCCESS){
				printf("failed to update <RDM_OID_TAAAB_USER> object. ret=%d", ret);
			}
#endif
			
#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169			
		loginlevelObj = json_object_array_get_idx(Jobj, 0);
		index1 = json_object_get_int(json_object_object_get(loginlevelObj, "Index"));	
		if(index1 == 1)
			strcpy(loginlevel,json_object_get_string(json_object_object_get(loginlevelObj, "DAL_LOGIN_LEVEL")));
#endif	
			
	for(int reqObjIdx = 0;reqObjIdx<len;reqObjIdx++){
		reqObject = json_object_array_get_idx(Jobj, tmp_idx[reqObjIdx]);

			IID_INIT(mgmtSrvIid);
			index = json_object_get_int(json_object_object_get(reqObject, "Index"));
			// protocol = json_object_get_string(json_object_object_get(reqObject, "Protocol"));
			mgmtSrvIid.idx[0] = index;
			mgmtSrvIid.level = 1;
			ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &mgmtSrvIid, &mgmtSrvObj);
			if(ret == ZCFG_SUCCESS){
#ifdef MSTC_TAAAG_MULTI_USER //__MSTC__, Bruce for item 169
				IID_INIT(ManagementIid);
				if ( zcfgFeObjJsonGet(RDM_OID_REMOTE_MGMT, &ManagementIid,&ManagementJarray) == ZCFG_SUCCESS)
				{
					ManagementLimitation = Jgetb(ManagementJarray, "Limitation");
					zcfgFeJsonObjFree(ManagementJarray);
				}	
				if(ManagementLimitation == true){
				if(!strcmp(loginlevel,"medium")){
					if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"TELNET")){
						struct json_object *mgmtsrvobj=NULL;
						objIndex_t MgmtsrvIid;
						IID_INIT(MgmtsrvIid);
						char *currMode = NULL,*Mode = NULL;
						Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
						MgmtsrvIid.level = 1;
						MgmtsrvIid.idx[0] = 4;
						if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
							currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
							//printf("can not get RDM_OID_REMO_SRV obj");
						}
						if(Mode != NULL && currMode != NULL ){
							if(strcmp(Mode,currMode))
								Mode = currMode;							
						}
						else if(Mode == NULL && currMode != NULL){
							Mode = currMode;
						}
						else if(Mode != NULL && currMode == NULL){
							Mode = currMode;
						}

						if(Mode != NULL)
							json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
						else
							json_object_object_add(reqObject, "Mode", json_object_new_string(""));
						
						int currport = 0;
						currport = json_object_get_int(json_object_object_get(mgmtsrvobj, "Port"));
						if(port != currport){
							port = currport;
							json_object_object_add(reqObject, "Port", json_object_new_int(port));
						}	
				        #ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
						int currwanport = 0;
						currwanport = json_object_get_int(json_object_object_get(mgmtsrvobj, "WANPort"));
						if(wanport != currwanport){
							wanport = currwanport;
							json_object_object_add(reqObject, "WANPort", json_object_new_int(wanport));
						}	
						#endif
						zcfgFeJsonObjFree(mgmtsrvobj);
					}
					else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"HTTP")){
						struct json_object *mgmtsrvobj=NULL;
						objIndex_t MgmtsrvIid;
						IID_INIT(MgmtsrvIid);
						char *currMode = NULL,*Mode = NULL;
						Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
						MgmtsrvIid.level = 1;
						MgmtsrvIid.idx[0] = 1;
						if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
							currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
							//printf("can not get RDM_OID_REMO_SRV obj");
						}
						if(Mode != NULL && currMode != NULL ){
							if(strcmp(Mode,currMode))
								Mode = currMode;							
						}
						else if(Mode == NULL && currMode != NULL){
							Mode = currMode;
						}
						else if(Mode != NULL && currMode == NULL){
							Mode = currMode;
						}

						if(Mode != NULL)
							json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
						else
							json_object_object_add(reqObject, "Mode", json_object_new_string(""));
						
						int currport = 0;
						currport = json_object_get_int(json_object_object_get(mgmtsrvobj, "Port"));
						if(port != currport){
							port = currport;
							json_object_object_add(reqObject, "Port", json_object_new_int(port));
						}
				        #ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
						int currwanport = 0;
						currwanport = json_object_get_int(json_object_object_get(mgmtsrvobj, "WANPort"));
						if(wanport != currwanport){
							wanport = currwanport;
							json_object_object_add(reqObject, "WANPort", json_object_new_int(wanport));
						}	
						#endif
						zcfgFeJsonObjFree(mgmtsrvobj);
					}
					else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"SSH")){
						struct json_object *mgmtsrvobj=NULL;
						objIndex_t MgmtsrvIid;
						IID_INIT(MgmtsrvIid);
						char *currMode = NULL,*Mode = NULL;
						Mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
						MgmtsrvIid.level = 1;
						MgmtsrvIid.idx[0] = 5;
						if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_REMO_SRV, &MgmtsrvIid, &mgmtsrvobj) == ZCFG_SUCCESS){
							currMode = json_object_get_string(json_object_object_get(mgmtsrvobj, "Mode"));
							//printf("can not get RDM_OID_REMO_SRV obj");
						}
						if(Mode != NULL && currMode != NULL ){
							if(strcmp(Mode,currMode))
								Mode = currMode;							
						}
						else if(Mode == NULL && currMode != NULL){
							Mode = currMode;
						}
						else if(Mode != NULL && currMode == NULL){
							Mode = currMode;
						}

						if(Mode != NULL)
							json_object_object_add(reqObject, "Mode", json_object_new_string(Mode));
						else
							json_object_object_add(reqObject, "Mode", json_object_new_string(""));
						
						int currport = 0;
						currport = json_object_get_int(json_object_object_get(mgmtsrvobj, "Port"));
						if(port != currport){
							port = currport;
							json_object_object_add(reqObject, "Port", json_object_new_int(port));
						}
				        #ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
						int currwanport = 0;
						currwanport = json_object_get_int(json_object_object_get(mgmtsrvobj, "WANPort"));
						if(wanport != currwanport){
							wanport = currwanport;
							json_object_object_add(reqObject, "WANPort", json_object_new_int(wanport));
						}	
						#endif	
						zcfgFeJsonObjFree(mgmtsrvobj);
					}
				}
				}

				/*Get another user's allowed_LA_Protocols and allowed_RA_Protocols*/
				dalSec_getOtherUserPro(Jobj, proLA, proRA);

				//Local
				if(proLA != NULL){
					copy_protocol = strdup(proLA);
					if(copy_protocol != NULL){
						token = strtok(copy_protocol, ","); 
						while (token != NULL) {
							if(strcmp(token, "HTTP") == 0){
								httpLanCheck = true;
							}
							else if(strcmp(token, "HTTPS") == 0){
								httpsLanCheck = true;
							}
							else if(strcmp(token, "FTP") == 0){
								ftpLanCheck = true;
							}
							else if(strcmp(token, "TELNET") == 0){
								telnetLanCheck = true;
							}
							else if(strcmp(token, "SSH") == 0){
								sshLanCheck = true;
							}
							else{
								free(copy_protocol);
								return ZCFG_INVALID_PARAM_VALUE;
							}
							token = strtok(NULL, ",");
						}
						free(copy_protocol);
					}
				}
				//Remote				
				if(proRA != NULL){
					copy_protocol = strdup(proRA);
					if(copy_protocol != NULL){
						token = strtok(copy_protocol, ","); 
						while (token != NULL) {
							if(strcmp(token, "HTTP") == 0){
								httpWanCheck = true;
							}
							else if(strcmp(token, "HTTPS") == 0){
								httpsWanCheck = true;
							}
							else if(strcmp(token, "FTP") == 0){
								ftpWanCheck = true;
							}
							else if(strcmp(token, "TELNET") == 0){
								telnetWanCheck = true;
							}
							else if(strcmp(token, "SSH") == 0){
								sshWanCheck = true;
							}
							else{
								free(copy_protocol);
								return ZCFG_INVALID_PARAM_VALUE;
							}
							token = strtok(NULL, ",");
						}
						free(copy_protocol);
					}	
				}			
				if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"HTTP"))
					mode = dalSec_getStatus_zyxel(json_object_get_string(json_object_object_get(reqObject, "Mode")),httpLanCheck,httpWanCheck);
				else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"HTTPS"))		
					mode = dalSec_getStatus_zyxel(json_object_get_string(json_object_object_get(reqObject, "Mode")),httpsLanCheck,httpsWanCheck);
				else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"FTP"))
					mode = dalSec_getStatus_zyxel(json_object_get_string(json_object_object_get(reqObject, "Mode")),ftpLanCheck,ftpWanCheck);
				else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"TELNET"))
					mode = dalSec_getStatus_zyxel(json_object_get_string(json_object_object_get(reqObject, "Mode")),telnetLanCheck,telnetWanCheck);
				else if(!strcmp(json_object_get_string(json_object_object_get(reqObject, "Name")),"SSH"))
					mode = dalSec_getStatus_zyxel(json_object_get_string(json_object_object_get(reqObject, "Mode")),sshLanCheck,sshWanCheck);
				else
					mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
				
#else
				mode = json_object_get_string(json_object_object_get(reqObject, "Mode"));
#endif

				boundinterfacelist = json_object_get_string(json_object_object_get(reqObject, "BoundInterfaceList"));
				port = json_object_get_int(json_object_object_get(reqObject, "Port"));
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
				wanport = json_object_get_int(json_object_object_get(reqObject, "WANPort"));
#endif
#ifdef MSTC_TAAAG_MULTI_USER				
				if(mode != NULL){
					if(!strcmp(mode,MODE_WAN_ONLY) || !strcmp(mode,MODE_LAN_WAN)){
						dalSec_updateRemoteObject(true);
					}
				}
#endif

				restartdeamon = json_object_get_boolean(json_object_object_get(reqObject, "RestartDeamon"));
				if(reqObjIdx-(len-1) == 0){
					restartdeamon = true;	//restart
				}
#ifdef CONFIG_ABZQ_CUSTOMIZATION
				name = json_object_get_string(json_object_object_get(mgmtSrvObj, "Name"));
				oldMode = json_object_get_string(json_object_object_get(mgmtSrvObj, "Mode"));
				if(strcmp(mode, oldMode)) changed = true;
				snprintf(cmdStr, sizeof(cmdStr), "%s User '%s' modified SCL %s from [%s] to [%s] via %s\n", accessMode, userName, name, oldMode, mode, srvName);
#endif	

#ifdef	CAAAB_CUSTOMIZATION
				lifetime = json_object_get_int(json_object_object_get(reqObject, "LifeTime"));
				json_object_object_add(mgmtSrvObj, "LifeTime", json_object_new_int(lifetime));
#endif

				json_object_object_add(mgmtSrvObj, "BoundInterfaceList", json_object_new_string(boundinterfacelist));
#ifdef MSTC_TAAAG_MULTI_USER
				if(mode != NULL)
				json_object_object_add(mgmtSrvObj, "Mode", json_object_new_string(mode));
				else
					json_object_object_add(mgmtSrvObj, "Mode", json_object_new_string(""));
#else				
				json_object_object_add(mgmtSrvObj, "Mode", json_object_new_string(mode));
#endif			
				json_object_object_add(mgmtSrvObj, "Port", json_object_new_int(port));
#ifdef CONFIG_ZYXEL_TAAAB_MGMTPORT_SEPARETE
                json_object_object_add(mgmtSrvObj, "WANPort", json_object_new_int(wanport));
#endif
				json_object_object_add(mgmtSrvObj, "RestartDeamon", json_object_new_int(restartdeamon));
				
#ifdef CONFIG_ABZQ_CUSTOMIZATION
				ret = zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV, &mgmtSrvIid, mgmtSrvObj, NULL);
				if ((ret == ZCFG_SUCCESS) && changed){
					changed = false;
					zcfgLogPrefix(ZCFG_LOG_INFO, ZCFG_LOGPRE_SCL, "%s", cmdStr);
				}
#else

				zcfgFeObjJsonBlockedSet(RDM_OID_REMO_SRV, &mgmtSrvIid, mgmtSrvObj, NULL);
#endif	
                zcfgFeJsonObjFree(mgmtSrvObj);
			}
	}

	zcfgFeJsonObjFree(mgmtSrvObj);

	return ret;
}


zcfgRet_t zcfgFeDalMgmtSrv(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	char loginLevel[16] = {0};
	char loginUserName[32] = {0};
#endif

#ifdef ZYXEL_ADMIN_HIDE_FTP_SNMP_SSH_PORT_NUMBER
	if(!method || !Jobj)
		return ZCFG_INTERNAL_ERROR;

	if(getLoginUserInfo(Jobj, loginUserName, loginLevel, NULL) != ZCFG_SUCCESS){
		printf("getLoginUserInfo Fail");
		return ZCFG_INTERNAL_ERROR;
	}
#endif

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDal_mgmtSrv_Edit(Jobj, replyMsg);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDal_mgmtSrv_Get(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

