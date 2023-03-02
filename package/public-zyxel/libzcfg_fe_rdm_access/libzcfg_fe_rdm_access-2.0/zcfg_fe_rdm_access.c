#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <json/json.h>
#include <time.h>
#include "zcfg_common.h"
#include "zcfg_fe_rdm_access.h"
#include "zcmd_schema.h"
#include "zcmd_tool.h"
#include "zcfg_msg.h"
#include "zcfg_eid.h"
#include "zcfg_debug.h"
#include "zcfg_msg_zysh.h"
#include "zos_api.h"

void *schemaShmAddr = NULL;
void *objNameShmAddr = NULL;
void *paramNameShmAddr = NULL;
void *objDefaultValPtr = NULL;
int shmid;
int schemaShmSize = 0;

/*
 *  Function Name: zcfgFeSharedMemInit
 *  Description: Used by front-end process to attach the shared memory.
 *               By doing so, the front-end process just can get object
 *               and parameter information stored in shared memory.
 *
 */
zcfgRet_t zcfgFeSharedMemInit()
{
	key_t key = ftok(ZCFG_SHM_KEY, 0);
	int size;
	FILE *fptr;
	char *objFileName = "/etc/zcfg_objName.bin";
	char *paramFileName = "/etc/zcfg_paramName.bin";
	char *defaultStrFileName = "/etc/zcfg_defaultStr.bin";

	//zcfgLog(ZCFG_LOG_INFO, "key %d\n", (int)key);

	if((fptr = fopen("/var/shm_size", "r")) == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Can't open \"/var/shm_size\" to read shared memory size\n");
		return ZCFG_INTERNAL_ERROR;
	}
	else {
		if( fscanf(fptr, "%d", &size) != 1) {
			zcfgLog(ZCFG_LOG_ERR, "\ncannot read out the data!\n");
			fclose(fptr);
			return ZCFG_INTERNAL_ERROR;
		}
		schemaShmSize = size;
	}

	fclose(fptr);

	if ((shmid = shmget(key, size, SHM_R)) < 0) {
		perror("shmget:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot create shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if ((schemaShmAddr = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot attach shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	zcfgLog(ZCFG_LOG_NOTICE, "Attached to schema shared memory\n");

	/*attach to object name shared memory buffer*/
	key = ftok(objFileName, 0);

	if((fptr = fopen(objFileName, "rb")) == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Can't fopen %s\n", objFileName);
		return ZCFG_INTERNAL_ERROR;
	}
	else {
		fseek(fptr, 0L, SEEK_END);
		size = ftell(fptr);
		fseek(fptr, 0L, SEEK_SET);
	}

	fclose(fptr);

	if ((shmid = shmget(key, size, SHM_R)) < 0) {
		perror("shmget:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot create shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if (( objNameShmAddr = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot attach shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	zcfgLog(ZCFG_LOG_NOTICE, "Attached to object name shared memory\n");

	/*attach to parameter name shared memory buffer*/
	key = ftok(paramFileName, 0);

	if((fptr = fopen(paramFileName, "rb")) == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Can't fopen %s\n", paramFileName);
		return ZCFG_INTERNAL_ERROR;
	}
	else {
		fseek(fptr, 0L, SEEK_END);
		size = ftell(fptr);
		fseek(fptr, 0L, SEEK_SET);
	}

	fclose(fptr);

	if ((shmid = shmget(key, size, SHM_R)) < 0) {
		perror("shmget:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot create shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if (( paramNameShmAddr = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot attach shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	zcfgLog(ZCFG_LOG_NOTICE, "Attached to parameter name shared memory\n");

	/*attach to all of default string shared memory buffer*/
	key = ftok(defaultStrFileName, 0);
	if((fptr = fopen(defaultStrFileName, "rb")) == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Can't fopen %s\n", defaultStrFileName);
		return ZCFG_INTERNAL_ERROR;
	}
	else {
		fseek(fptr, 0L, SEEK_END);
		size = ftell(fptr);
		fseek(fptr, 0L, SEEK_SET);
	}

	fclose(fptr);

	if ((shmid = shmget(key, size, SHM_R )) < 0) {
		perror("shmget:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot create shared memory\n");
		printf("cannot create shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	if (( objDefaultValPtr = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat:");
		zcfgLog(ZCFG_LOG_ERR, "\ncannot attach shared memory\n");
		printf("cannot attach shared memory\n");
		return ZCFG_INTERNAL_ERROR;
	}

	zcfgLog(ZCFG_LOG_NOTICE, "\n");

	return ZCFG_SUCCESS;
}
/*
 *  Function Name: zcfgFeObjRead
 *  Description: Used by front-end process to send request message
 *               to ZCMD to get object.
 *
 */
zcfgRet_t zcfgFeObjRead(zcfg_offset_t oid, objIndex_t *objIid, uint32_t type, char **result)
{
	zcfgRet_t ret;
	char *recvStr = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

    zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
    if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	sendMsgHdr = (zcfgMsg_t *)malloc(sizeof(zcfgMsg_t));
	sendMsgHdr->type = type;
	/*No payload*/
	sendMsgHdr->length = 0;
	memcpy(sendMsgHdr->objIid, objIid, sizeof(objIndex_t));
	sendMsgHdr->oid = oid;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;

		if(recvMsgHdr->type == RESPONSE_GET_FAIL) {
			ret = recvMsgHdr->statCode;
			if(ret == ZCFG_NO_SUCH_OBJECT)
				memcpy(objIid, recvMsgHdr->objIid, sizeof(objIndex_t));
		}
		else if(recvMsgHdr->type == RESPONSE_NO_MORE_INSTANCE) {
			ret = ZCFG_NO_MORE_INSTANCE;
		}
		else {
			memcpy(objIid, recvMsgHdr->objIid, sizeof(objIndex_t));
			/*Get json string from message*/
			recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));

			*result = (char *)malloc(strlen(recvStr)+1);
			strcpy(*result, recvStr);

			if(recvMsgHdr->type == RESPONSE_GET_SUCCESS)
				ret = ZCFG_SUCCESS;
			else{
				free(*result);
				ret = ZCFG_EMPTY_OBJECT;
			}
		}
	}

	ZOS_FREE(recvBuf);
	return ret;
}
/*
 *  Function Name: zcfgFeObjWrite
 *  Description: Used by front-end process to send request message
 *               to ZCMD to write object in the data model.
 *
 */
zcfgRet_t zcfgFeObjWrite(zcfg_offset_t oid, objIndex_t *objIid, char *setValue, uint32_t type, char *feedbackMsg)
{
	zcfgRet_t ret;
	int len = 0;
	char *recvStr = NULL;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

    zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
    if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(setValue != NULL)
		len = strlen(setValue) + 1;

	sendBuf = (void *)malloc(sizeof(zcfgMsg_t) + len);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = type;
	sendMsgHdr->length = len;
	memcpy(sendMsgHdr->objIid, objIid, sizeof(objIndex_t));
	sendMsgHdr->oid = oid;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	if(setValue != NULL)
		strcpy((char *)(sendMsgHdr + 1), setValue);

	ret = zcfgMsgSendAndGetReply(sendBuf, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;


#ifdef ZCFG_RPC_REQUEST_ADDITIONS
		if(type & ZCFG_MSG_RPC_ADDITIONS && recvMsgHdr->length)
		{
			char **replyStr = {0};

			replyStr = (char **)feedbackMsg;
			replyStr[0] = (char *)malloc(recvMsgHdr->length);
			recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));
			strcpy(replyStr[0], recvStr);
		}else if(recvMsgHdr->type == RESPONSE_WRITE_OBJ_SUCCESS) {
#else
		if(recvMsgHdr->type == RESPONSE_WRITE_OBJ_SUCCESS) {
#endif
			memcpy(objIid, recvMsgHdr->objIid, sizeof(objIndex_t));
			ret = recvMsgHdr->statCode;
			//ret = ZCFG_SUCCESS;
		}
		else {
			/*Check if there is a feedback error message*/
			if( recvMsgHdr->length > 0 ) {
				recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));
				if(feedbackMsg != NULL) {
					strcpy(feedbackMsg, recvStr);
				}
			}
			ret = recvMsgHdr->statCode;
		}
	}

	ZOS_FREE(recvBuf);
	return ret;
}


zcfgRet_t zcfgFeIfStackObjReadByLayer(char *layer, int hilo, char **result)
{
	zcfgRet_t ret;
	char *recvStr = NULL;
	void *recvBuf = NULL;
	void *sendBuf = NULL;
	int len = 0;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

	zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
	if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(layer == NULL || strlen(layer)<=0)
		return ZCFG_REQUEST_REJECT;

	len = strlen(layer) + 1;
	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + len);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->length = len;
	sendMsgHdr->type = REQUEST_GET_IFACE_STACK_OBJ_BY_LAYER;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	sendMsgHdr->statCode = hilo;
	strcpy((char *)(sendMsgHdr + 1), layer);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;

		if(recvMsgHdr->type == RESPONSE_GET_FAIL) {
			ret = recvMsgHdr->statCode;
		}
		else {
			/*Get json string from message*/
			recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));
			if(recvMsgHdr->type == RESPONSE_GET_SUCCESS) {
				*result = (char *)json_tokener_parse(recvStr);
				ret = ZCFG_SUCCESS;
			}
			else{
				*result = NULL;
				ret = ZCFG_EMPTY_OBJECT;
			}
		}
	}

	ZOS_FREE(recvBuf);
	return ret;
} /* zcfgFeIfStackObjReadByLayer */


zcfgRet_t zcfgFeWholeObjReadByName(char *tr181path, uint32_t type, char **result)
{
	zcfgRet_t ret;
	char *recvStr = NULL;
	void *recvBuf = NULL;
	void *sendBuf = NULL;
	int len = 0;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

    zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
    if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(tr181path == NULL || strlen(tr181path)<=0)
		return ZCFG_REQUEST_REJECT;

	len = strlen(tr181path) + 1;
	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + len);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->length = len;
	sendMsgHdr->type = type;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	strcpy((char *)(sendMsgHdr + 1), tr181path);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;

		if(recvMsgHdr->type == RESPONSE_GET_FAIL) {
			ret = recvMsgHdr->statCode;
		}
		else {
			/*Get json string from message*/
			recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));

			*result = (char *)malloc(strlen(recvStr)+1);
			strcpy(*result, recvStr);

			if(recvMsgHdr->type == RESPONSE_GET_SUCCESS)
				ret = ZCFG_SUCCESS;
			else{
				free(*result);
				*result = NULL;
				ret = ZCFG_EMPTY_OBJECT;
			}
		}
	}

	ZOS_FREE(recvBuf);
	return ret;
}

zcfgRet_t zcfgFeWholeObjWriteByName(char *tr181path, char *setValue, uint32_t type, char *feedbackMsg)
{
	zcfgRet_t ret;
	int len = 0;
	char *recvStr = NULL;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

    zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
    if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	if(schemaShmAddr == NULL)
		zcfgFeSharedMemInit();

	if(setValue == NULL || tr181path == NULL || strlen(tr181path)<=0 || strlen(setValue)<=0)
		return ZCFG_REQUEST_REJECT;

	len = strlen(tr181path) + strlen(setValue) + 2;

	sendBuf = (void *)malloc(sizeof(zcfgMsg_t) + len);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = type;
	sendMsgHdr->length = len;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	sprintf((char *)(sendMsgHdr + 1), "%s,%s", tr181path, setValue);

	ret = zcfgMsgSendAndGetReply(sendBuf, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;

		if(recvMsgHdr->type == RESPONSE_WRITE_OBJ_SUCCESS) {
			ret = recvMsgHdr->statCode;
			//ret = ZCFG_SUCCESS;
		}
		else {
			/*Check if there is a feedback error message*/
			if( recvMsgHdr->length > 0 ) {
				recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));
				if(feedbackMsg != NULL) {
					strcpy(feedbackMsg, recvStr);
				}
			}
			ret = recvMsgHdr->statCode;
		}
	}

	ZOS_FREE(recvBuf);
	return ret;
}


zcfgRet_t zcfgFeReqSend(uint32_t msgType, char *payload)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int payloadLen = 0;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

	switch(msgType) {
		case ZCFG_MSG_DELAY_APPLY:
		case REQUEST_FIRMWARE_UPGRADE:
		case REQUEST_FIRMWARE_UPGRADE_CHK:
		case REQUEST_FIRMWARE_UPGRADE_ONLINE_CHK:
		case REQUEST_FIRMWARE_ID_CHK:
		case REQUEST_FIRMWARE_WRITE_ONLY:
		case REQUEST_FIRMWARE_WRITE_AND_STORE:
		case REQUEST_ROMD_UPGRADE:
		case REQUEST_FIRMWARE_UPGRADE_FWUR:
			if(payload == NULL || strlen(payload) == 0)
				return ZCFG_INVALID_ARGUMENTS;

			payloadLen = strlen(payload)+1;
			break;
		case REQUEST_RESTORE_DEFAULT:
		case ZCFG_MSG_UPGRADING_FW_CHECK_FWID_ON:
		case ZCFG_MSG_UPGRADING_FW_CHECK_FWID_OFF:
		case ZCFG_MSG_UPGRADING_FW_CHECK_FWID_STATUS:
		case ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_ON:
		case ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_OFF:
		case ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_STATUS:
		case REQUEST_RESTORE_DEFAULT_CHK:
		case REQUEST_CONFIG_BACKUP:
		case ZCFG_MSG_RE_APPLY:
		case ZCFG_MSG_AUTO_PROVISION:
		case REQUEST_REINIT_MACLESS_PROVISIONING:
		case ZCFG_MSG_GET_TR98_ALL_IGD_NAMES:
		case ZCFG_MSG_SAVE_ROMD:
		case ZCFG_MSG_CLEAN_ROMD:
		case ZCFG_MSG_CLEAN_SYSLOG:
		case ZCFG_MSG_QOS_GET_STATE:
		case ZCFG_MSG_REQ_REBOOT:
		case ZCFG_MSG_REQ_SYNCFLASH:
#ifdef ZYXEL_PARTIAL_RESET
		case REQUEST_PARTIAL_RESTORE_DEFAULT:
#endif
#ifdef ZYXEL_WEB_GUI_SHOW_ZYEE
		case ZCFG_MSG_GET_ZYEE_MONITOR:
#endif
#ifdef ZYXEL_WIFI_KEY_REMINDER
		case ZCFG_MSG_SET_WIFIREDIRECT:
#endif
		case ZCFG_MSG_REQUEST_GET_PARAM_ATTR_LIST:
		case ZCFG_MSG_REQ_SWITCH_BOOT_PARTITION:
		case REQUEST_SET_PARAMETERKEY:
		case ZCFG_MSG_REQ_CALIBRATION_24G:
		case ZCFG_MSG_REQ_CALIBRATION_5G:
		case ZCFG_MSG_GET_ROMD:
		case REQUEST_LOW_AUTHORITY_DO_SYSTEM_CALL:
		case REQUEST_LOW_AUTHORITY_DO_SYSTEM_CALL_S:
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
		case REQUEST_RESTORE_CONFIG:
#endif
		case REQUEST_Import_Certificate:
		case ZCFG_MSG_FIRMWARE_UPGRADE:
			if(payload == NULL || strlen(payload) == 0)
				payloadLen = 0;
			else
				payloadLen = strlen(payload)+1;
			break;
		case ZCFG_MSG_UPDATE_PARAMETER_NOTIFY:
		case ZCFG_MSG_HOSTS:
			break;
		default:
			return ZCFG_INVALID_PARAM_VALUE;
	}
		/*if(msgType==REQUEST_RESTORE_DEFAULT){
		//printf("%d %s\n", __LINE__, __func__);
		//printf("SetFactoryResetCode!\n");
		SetFactoryResetCode("3");	
	}*/

	sendBuf = (void *)malloc(sizeof(zcfgMsg_t) + payloadLen);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = msgType;
	sendMsgHdr->length = payloadLen;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	if(payloadLen > 0)
		strcpy((char *)(sendMsgHdr + 1), payload);
	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);
	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;
		if ( recvMsgHdr->type == RESPONSE_FAIL ) {
			ret = recvMsgHdr->statCode;
		}
	}
	else if(ret == ZCFG_SUCCESS_AND_NO_REPLY) {
		ret = ZCFG_SUCCESS;
	}
	else {

		return ZCFG_INTERNAL_ERROR;
	}

	ZOS_FREE(recvBuf);
	return ret;
}

#ifdef ZCFG_TR98_SUPPORT
zcfgRet_t zcfgFeMappingNameGet(uint32_t msgType, char *fullPathName, char *result)
{
	zcfgRet_t ret;
	int payloadLen = 0;
	char *recvStr = NULL;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;

	if(fullPathName != NULL) {
		payloadLen = strlen(fullPathName) + 1;
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	if(payloadLen == 0)
		return ZCFG_INVALID_ARGUMENTS;

	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + payloadLen);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = msgType;
	sendMsgHdr->length = payloadLen;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	strcpy((char *)(sendMsgHdr + 1), fullPathName);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;
		if ( recvMsgHdr->type == RESPONSE_FAIL ) {
			ZOS_FREE(recvBuf);
			return ZCFG_INTERNAL_ERROR;
		}
		else {
			if( recvMsgHdr->length > 0 ) {
				recvStr = (char *)(recvBuf+sizeof(zcfgMsg_t));
				strcpy(result, recvStr);
			}
		}

		ZOS_FREE(recvBuf);
	}
	else
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}
#endif

int zcfgFeObjMaxLenGet(zcfg_offset_t rdmObjId)
{
	objIndex_t Iid;
	objInfo_t *objInfo = (objInfo_t *)(schemaShmAddr + rdmObjId);
	int maxLen = 0;

	IID_INIT(Iid);
	maxLen = objInfo->maxEntry[Iid.level];
	return maxLen;
}

int zcfgFeArrayLengthGetByName(char* tr181path)
{
	struct json_object *objArray=NULL;
	int len = 0;
	zcfgRet_t ret;
	char *result = NULL;
		
	ret = zcfgFeWholeObjReadByName(tr181path, REQUEST_GET_WHOLE_OBJ_BY_NAME, &result);
	if(ret == ZCFG_SUCCESS) {
		if(result){
			objArray = json_tokener_parse(result);
			free(result);
			result = NULL;			
		}
	}

	if(objArray != NULL){
		len = json_object_array_length(objArray);
	}
	return len;
}

zcfgRet_t zcfgFeZyshCmdSend(int eid, uint32_t cmd, int value, char *param,int len, char *zysh_ret)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	zcfgZysh_t *zysh;

	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + sizeof(zcfgZysh_t));
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = ZCFG_MSG_ZYSH_EXE_ROOT_CMD;
	sendMsgHdr->length = sizeof(zcfgZysh_t);
	sendMsgHdr->dstEid = eid;

	zysh = (zcfgZysh_t *) (sendMsgHdr + 1);
	zysh->value = value;
	zysh->cmd = cmd;
	if(param != NULL )
		memcpy(zysh->buf,param,len);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);
	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;
		ret = recvMsgHdr->statCode;
		if ( recvMsgHdr->type == RESPONSE_FAIL ) {
			ret = recvMsgHdr->statCode;
		}
		else {
			if (recvMsgHdr->length && ( zysh_ret != NULL ) ) {
				memcpy(zysh_ret, ( recvMsgHdr + 1 ), sizeof( zcfgZysh_t ) );
			}

		}
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	ZOS_FREE(recvBuf);
	return ret;
}

zcfgRet_t zcfgFeFakeRootSend(char *param, int len, char **zysh_ret, bool reply)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	void *sendBuf = NULL;
	void *recvBuf = NULL;
	char *recvStr = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	if(reply & zysh_ret==NULL)
	{
        zcfgLog(ZCFG_LOG_ERR, "NULL zysh_ret\n");
		return ZCFG_INTERNAL_ERROR;
	}

	zcfg_msg_eid_t myEid = zcfg_msg_eidGet();
	if (myEid == 0) {
		zcfgLog(ZCFG_LOG_ERR, "Invalid eid\n");
		return ZCFG_INVALID_EID;
	}

	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + len);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	if(reply)
		sendMsgHdr->type = ZCFG_MSG_ZYSHCMD_WRAPPER_EXE_ROOT_CMD & ~ZCFG_NO_WAIT_REPLY;
	else
		sendMsgHdr->type = ZCFG_MSG_ZYSHCMD_WRAPPER_EXE_ROOT_CMD;
	sendMsgHdr->length = len;
	sendMsgHdr->srcEid = myEid;
	sendMsgHdr->dstEid = ZCFG_EID_ZYSHCMD_WRAPPER;

	sendBuf = sendMsgHdr+1;
	if(param != NULL )
		memcpy(sendBuf, param, len);
	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, reply?2000:0);
	if(ret == ZCFG_SUCCESS) {
		recvMsgHdr = (zcfgMsg_t*)recvBuf;
		ret = recvMsgHdr->statCode;
		if ( recvMsgHdr->type == RESPONSE_FAIL ) {
			ret = recvMsgHdr->statCode;
		}
		else {
			if (recvMsgHdr->length) {
				recvStr = (char*)(recvMsgHdr+1);
				*zysh_ret = (char *)malloc(strlen(recvStr)+1);
				if(*zysh_ret != NULL)
				{
					ZOS_STRNCPY(*zysh_ret,recvStr,strlen(recvStr)+1);
				}
			}
		}
		ZOS_FREE(recvBuf);
	}else if(ret == ZCFG_SUCCESS_AND_NO_REPLY) {
		ret = ZCFG_SUCCESS;
	}
	else{
		ret = ZCFG_INTERNAL_ERROR;
	}
	return ret;
}
#if 1  //TAAAG Feature Request: Interface Down and Factory Reset Reason Code to record the reason of CPE reset to default
void SetFactoryResetCode(const char *frCode)
{
	FILE *frfp=NULL;
	char frbuf[16] = {0};
	time_t timep;
	struct tm *tm;
	char file_value[8];
	unsigned int fr_count=0;
	
	printf("=== FR Entered ===\n");

	time(&timep);
	tm = gmtime(&timep);
	strftime(frbuf, sizeof(frbuf), "%d%m%Y%H%M%S", tm);	//format DDMMYYYYhhmmss, hh = 24 hour
	/*timekeep*/
	printf("store tmp time\n");
	frfp = fopen("/data/timekeep","r+");
	if(frfp != NULL)
	{	
		//if exist : delete
		fclose(frfp);
		//system("rm /data/timekeep");
		unlink("/data/timekeep");
		//creat new one and write
		frfp= fopen("/data/timekeep","w+");
		if(frfp != NULL)
		{
			fprintf(frfp,"%s\n", frbuf);
			fclose(frfp);
			sync();
		}
		else
		{
			return;
		}
		
	}else
	{
		frfp= fopen("/data/timekeep","w+");
		if(frfp != NULL)
		{
			fprintf(frfp,"%s\n", frbuf);
			fclose(frfp);
			sync();
		}
		else
		{
			return;
		}
	}
	
	/*FactoryReset*/
	frfp = fopen("/data/FactoryReset","r+");
	if(frfp != NULL)
	{
		fclose(frfp);
		frfp= fopen("/data/FactoryReset","a+");
		fprintf(frfp,"%s@ %s\n", frCode, frbuf);	//for reset to default, FR_ReasonCode: 1=GUI, 2=Telnet, 3=Button, 4=TR-069
		fclose(frfp);
		sync();
	}else
	{
		//printf("%d %s\n", __LINE__, __func__);
		frfp= fopen("/data/FactoryReset","w+");
		if(frfp != NULL)
		{
			//printf("%d %s\n", __LINE__, __func__);
			fprintf(frfp,"%s","for reset to default,\nFR_ReasonCode: 1=GUI, 2=Telnet, 3=Button, 4=TR-069\nformat DDMMYYYYhhmmss hh = 24 hour\n");
			fprintf(frfp,"%s@ %s\n", frCode, frbuf);
			fclose(frfp);
			sync();
			
		}
		else
		{
			return;
		}
	}
	frfp= fopen("/misc/FRCount","r+");
	if(frfp != NULL)
	{
		if( fgets(file_value, sizeof(file_value), frfp) != NULL)
		{
			fr_count = atoi(file_value) + 1;
			printf("@@ r+ file value add:%d",fr_count);
		}
		else
		{
			printf("cannot read out /misc/FRCount!\n");
		}
		fclose(frfp);
		
		frfp= fopen("/misc/FRCount","w+");
		if(frfp != NULL)
		{
			fprintf(frfp,"%d",fr_count);
			fclose(frfp);
			sync();
		}
	}
	else
	{
		frfp= fopen("/misc/FRCount","w+");
		if(frfp != NULL)
		{
			printf("@@ w+ file value init: 1");
			fprintf(frfp,"%s","1");
			fclose(frfp);
			sync();
		}
		else
		{
			return;
		}
	}
}
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069  //TAAAG Feature Request: Interface Down and Factory Reset Reason Code to record the reason of CPE reset to default
void SetFactoryResetCode_TAAAB(const char *frCode)
{
	FILE *frfp=NULL;
	char frbuf[16] = {0};
	time_t timep;
	struct tm *tm;
	char file_value[8];
	int fr_count=0;
	
	printf("=== FR Entered ===\n");

	time(&timep);
	tm = localtime(&timep);
	strftime(frbuf, sizeof(frbuf), "%d%m%Y%H%M%S", tm);	//format DDMMYYYYhhmmss, hh = 24 hour

	frfp = fopen("/data/FactoryResetforTT","w+");
	if(frfp != NULL)
	{
		fprintf(frfp,"%s@%s", frCode, frbuf);	//for reset to default, X_TAAAB_FR_ReasonCode: 1=GUI, 2=Telnet, 3=Button, 4=TR-069
		fclose(frfp);
		sync();
	}else
		return;
	
	frfp= fopen("/data/FRCountforTT","r+");
	if(frfp != NULL)
	{
		if( fgets(file_value, sizeof(file_value), frfp) != NULL)
		{
			fr_count = atoi(file_value) + 1;
			printf("@@ r+ file value add:%d",fr_count);
		}
		else
		{
			printf("cannot read out /misc/FRCount!\n");
		}
		fclose(frfp);
		
		frfp= fopen("/data/FRCountforTT","w+");
		if(frfp != NULL)
		{
			fprintf(frfp,"%d",fr_count);
			fclose(frfp);
			sync();
		}
	}
	else
	{
		frfp= fopen("/data/FRCountforTT","w+");
		if(frfp != NULL)
		{
			printf("@@ w+ file value init: 1");
			fprintf(frfp,"%s","1");
			fclose(frfp);
			sync();
		}
		else
		{
			return;
		}
	}
}

void SetRebootCode_TT(const char *rbCode)
{
	FILE *rbfp=NULL;
	char rbbuf[16] = {0};
	time_t timep;
	struct tm *tm;
	
	printf("=== Reboot Entered ===\n");

	time(&timep);
	tm = localtime(&timep);
	strftime(rbbuf, sizeof(rbbuf), "%d%m%Y%H%M%S", tm);	//format DDMMYYYYhhmmss, hh = 24 hour

	rbfp = fopen("/data/RebootforTT","w+");
	if(rbfp != NULL)
	{
		fprintf(rbfp,"%s@%s", rbCode, rbbuf);
		fclose(rbfp);
		sync();
	}else
		return;
}

void SetInterfaceDownCode(const char *IntfDownCode)
{
	FILE *Intfdownfp=NULL;
	//char Intfdownbuf[16] = {0};
	time_t timep;
	struct tm *tm;
	char file_value[8];
	//int fr_count=0;
	
	printf("=== Interface Down Entered ===\n");

	//time(&timep);
	//tm = localtime(&timep);
	//strftime(Intfdownbuf, sizeof(Intfdownbuf), "%d%m%Y%H%M%S", tm);	//format DDMMYYYYhhmmss, hh = 24 hour

	Intfdownfp = fopen("/data/InterfaceDown","w+");
	if(Intfdownfp != NULL)
	{
		fprintf(Intfdownfp,"%s", IntfDownCode);
		fclose(Intfdownfp);
		sync();
	}else
		return;

}
#endif

