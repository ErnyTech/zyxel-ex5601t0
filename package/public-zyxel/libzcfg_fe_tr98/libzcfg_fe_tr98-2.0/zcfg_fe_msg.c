
#include "zcfg_debug.h"
#include "zcfg_fe_msg.h"
#include "zos_api.h"

zcfgRet_t zcfg98To181ObjMappingAdd(zcfg_name_t seqnum, objIndex_t *tr98ObjIid, zcfg_offset_t oid, objIndex_t *objIid)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	char *sendBuf = NULL, *recvBuf = NULL;
	zcfgMapData_t *mappingData = NULL, *addedMapping = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!tr98ObjIid || !objIid)
		return ZCFG_INTERNAL_ERROR;

	sendBuf = (char *)ZOS_MALLOC(sizeof(zcfgMsg_t) + sizeof(zcfgMapData_t));

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = ZCFG_MSG_ADD_TR98_MAPPING;
	sendMsgHdr->length = sizeof(zcfgMapData_t);
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	mappingData = (zcfgMapData_t *)(sendMsgHdr + 1);
	mappingData->num_1 = seqnum;
	memcpy(&mappingData->objIdxData_1, tr98ObjIid, sizeof(objIndex_t));
	mappingData->num_2 = oid;
	memcpy(&mappingData->objIdxData_2, objIid, sizeof(objIndex_t));

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}
	
	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		ZOS_FREE(recvBuf);
		return ZCFG_INTERNAL_ERROR;
	}

	if( recvMsgHdr->length > 0 ) {
		addedMapping = (zcfgMapData_t *)(recvBuf+sizeof(zcfgMsg_t));
		if(addedMapping->num_1 != seqnum) {

			ZOS_FREE(recvBuf);
			return ZCFG_INTERNAL_ERROR;
		}
		memcpy(tr98ObjIid, &addedMapping->objIdxData_1, sizeof(objIndex_t));
	}

	ZOS_FREE(recvBuf);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgObjMappingGet(uint32_t msgType, uint32_t num_1, objIndex_t *objIdxData_1, uint32_t *num_2, objIndex_t *objIdxData_2)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	char *sendBuf = NULL, *recvBuf = NULL;
	zcfgMapData_t *msgMapData = NULL, *queriedMapping = NULL;

	//zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!objIdxData_1 || !num_2 || !objIdxData_2)
		return ZCFG_INTERNAL_ERROR;

	*num_2 = 0;
	IID_INIT(*objIdxData_2);

	sendBuf = (char *)ZOS_MALLOC(sizeof(zcfgMsg_t) + sizeof(zcfgMapData_t));

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = msgType;
	sendMsgHdr->length = sizeof(zcfgMapData_t);
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	msgMapData = (zcfgMapData_t *)(sendMsgHdr + 1);
	msgMapData->num_1 = num_1;
	memcpy(&msgMapData->objIdxData_1, objIdxData_1, sizeof(objIndex_t));

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret != ZCFG_SUCCESS) {
		return ZCFG_INTERNAL_ERROR;
	}

	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	ret = recvMsgHdr->statCode;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		ZOS_FREE(recvBuf);
		return ret; //ZCFG_INTERNAL_ERROR;
	}

	if( recvMsgHdr->length > 0 ) {
		queriedMapping = (zcfgMapData_t *)(recvBuf+sizeof(zcfgMsg_t));
		if(queriedMapping->num_1 != num_1) {
			zcfgLog(ZCFG_LOG_INFO, "%s: wrong query mapping\n", __FUNCTION__);
			ZOS_FREE(recvBuf);
			return ZCFG_INTERNAL_ERROR;
		}
		*num_2 = queriedMapping->num_2;
		memcpy(objIdxData_2, &queriedMapping->objIdxData_2, sizeof(objIndex_t));
	}

	ZOS_FREE(recvBuf);

	return ret; //ZCFG_SUCCESS
}

zcfgRet_t zcfgFeTr98ObjValueQry(zcfg_name_t seqnum, objIndex_t *tr98ObjIid, const char *parmName, struct json_object **valueObj, int attr)
{
	zcfgParmQryValue_t *queryDataSt;
	zcfgMsg_t *sendMsgHdr = NULL, *recvMsgHdr = NULL;
	char *recvData = NULL;

	int dataLength = sizeof(zcfgMsg_t)+sizeof(zcfgParmQryValue_t)+strlen(parmName)+1;
	sendMsgHdr = (zcfgMsg_t *)malloc(dataLength);
	memset((void *)sendMsgHdr, 0, dataLength);

	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	sendMsgHdr->length = dataLength-sizeof(zcfgMsg_t);
	sendMsgHdr->type = REQUEST_QUERY_OBJ_PARAM_VALUE;

	queryDataSt = (zcfgParmQryValue_t *)(sendMsgHdr+1);
	queryDataSt->objnum = seqnum;
	queryDataSt->attr |= attr;
	memcpy(&queryDataSt->objIid, tr98ObjIid, sizeof(objIndex_t));
	if(parmName){
		char *queryParmName = (char *)(queryDataSt+1);
		strcpy(queryParmName, parmName);
		queryDataSt->parmNameLen = strlen(parmName);
	}

	zcfgRet_t rst = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvMsgHdr, 0);

	if(rst == ZCFG_SUCCESS){
		const char *recvData = (recvMsgHdr->length) ? (const char *)(recvMsgHdr+1) : NULL;
		*valueObj = json_tokener_parse(recvData);
		ZOS_FREE(recvMsgHdr);
	}

	return rst;
}

zcfgRet_t zcfgFeParmAttrQry(zcfg_name_t seqnum, objIndex_t *tr98ObjIid, const char *parmName, zcfg_offset_t *oid, objIndex_t *objIid, char *queriedParmName, uint32_t *attr)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	char *sendBuf = NULL, *recvBuf = NULL, *dataParmName = NULL;
	zcfgParmQryAttr_t *queryData = NULL, *queriedData = NULL;
	int msgDataLen = 0, n = 0, parmNameLen = 0, parmAttr = -1;

	//zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	//if(!tr98ObjIid || !parmName || !oid || !objIid || !queriedParmName)
	if(!tr98ObjIid || !parmName)
		return ZCFG_INTERNAL_ERROR;

	//zcfgLog(ZCFG_LOG_INFO, "%s: %u %hu,%hu,%hu,%hu,%hu,%hu %s\n", __FUNCTION__, seqnum, tr98ObjIid->idx[0],
      //   tr98ObjIid->idx[1], tr98ObjIid->idx[2], tr98ObjIid->idx[3], tr98ObjIid->idx[4], tr98ObjIid->idx[5], parmName);

	parmNameLen = strlen(parmName) + 1;
	parmNameLen += ((n = (parmNameLen % UNITSIZE)) > 0) ? (UNITSIZE - n) : 0;
	msgDataLen = sizeof(zcfgParmQryAttr_t) + parmNameLen;
	sendBuf = (char *)ZOS_MALLOC(sizeof(zcfgMsg_t) + msgDataLen);

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = REQUEST_GET_PARAM_ATTR;
	sendMsgHdr->length = msgDataLen;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	queryData = (zcfgParmQryAttr_t *)(sendMsgHdr + 1);
	queryData->objnum = seqnum;
	memcpy(&queryData->objIid, tr98ObjIid, sizeof(objIndex_t));
	queryData->attr = *attr;
	queryData->parmNameLen = parmNameLen;
	dataParmName = (char *)(queryData + 1);
	strcpy(dataParmName, parmName);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret != ZCFG_SUCCESS) {
		return ret;
	}

	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		ZOS_FREE(recvBuf);
		return ZCFG_INTERNAL_ERROR;
	}

	if(attr) {
		*attr = PARAMETER_ATTR_NOATTR;
	}

	if( recvMsgHdr->length > 0 ) {
		queriedData = (zcfgParmQryAttr_t *)(recvBuf+sizeof(zcfgMsg_t));
		#if 0
		if(queriedData->objnum) {
			// confirm queriedData, if required
			zcfgLog(ZCFG_LOG_INFO, "%s: wrong queried data\n", __FUNCTION__);
			ZOS_FREE(recvBuf);
			return ZCFG_INTERNAL_ERROR;
		}
		#endif


		if(oid)
			*oid = queriedData->objnum;
		if(objIid)
			memcpy(objIid, &queriedData->objIid, sizeof(objIndex_t));
		if(attr) {
			*attr = queriedData->attr;
			parmAttr = *attr;
			//printf("%s: attr 0x%x\n", __FUNCTION__, *attr);
		}
		dataParmName = (char *)(queriedData + 1);
		if(queriedParmName)
			strcpy(queriedParmName, dataParmName);

		//zcfgLog(ZCFG_LOG_INFO, "%s: queried %u %hu,%hu,%hu,%hu,%hu,%hu %s 0x%x\n", __FUNCTION__, *oid, objIid->idx[0],
        // objIid->idx[1], objIid->idx[2], objIid->idx[3], objIid->idx[4], objIid->idx[5], queriedParmName, parmAttr);
	}

	ZOS_FREE(recvBuf);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr181ParmAttrQry(zcfg_offset_t oid, objIndex_t *objIid, char *parmName, zcfg_name_t *seqnum, objIndex_t *tr98ObjIid, const char *queriedParmName, uint32_t *qriedTr181Attr)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	uint32_t attr = 0;

	attr |= PARAMETER_ATTR_TR181;
	if((rst = zcfgFeParmAttrQry(oid, objIid, parmName, seqnum, tr98ObjIid, queriedParmName, &attr)) == ZCFG_SUCCESS)
		*qriedTr181Attr = attr;

	return rst;
}

zcfgRet_t zcfgFeObjParamAttrQry(uint32_t objnum, objIndex_t *objIid, struct json_object **parmAttrObj, uint32_t attr)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	char *sendBuf = NULL, *recvBuf = NULL;
	zcfgParmQryAttr_t *queryData = NULL;
	int msgDataLen = 0;


	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!objIid || !parmAttrObj)
		return ZCFG_INTERNAL_ERROR;

	//zcfgLog(ZCFG_LOG_INFO, "%s: %u %hu,%hu,%hu,%hu,%hu,%hu %s\n", __FUNCTION__, seqnum, tr98ObjIid->idx[0],
	//   tr98ObjIid->idx[1], tr98ObjIid->idx[2], tr98ObjIid->idx[3], tr98ObjIid->idx[4], tr98ObjIid->idx[5], parmName);

	msgDataLen = sizeof(zcfgParmQryAttr_t);
	sendBuf = (char *)ZOS_MALLOC(sizeof(zcfgMsg_t) + msgDataLen);

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = REQUEST_QUERY_OBJ_PARAM_ATTR;
	sendMsgHdr->length = msgDataLen;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;

	queryData = (zcfgParmQryAttr_t *)(sendMsgHdr + 1);
	queryData->objnum = objnum;
	memcpy(&queryData->objIid, objIid, sizeof(objIndex_t));
	queryData->attr = attr;

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret != ZCFG_SUCCESS) {
		return ret;
	}

	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		ZOS_FREE(recvBuf);
		return ZCFG_INTERNAL_ERROR;
	}

	if( recvMsgHdr->length > 0 ) {
		char *queriedData = (char *)malloc(recvMsgHdr->length + 1);
		char *data = (char *)(recvBuf+sizeof(zcfgMsg_t));

		memcpy(queriedData, data, recvMsgHdr->length);
		*(queriedData+recvMsgHdr->length) = '\0';
		struct json_object *obj = json_tokener_parse(queriedData);

		if(obj){
			*parmAttrObj = obj;
		}

		free(queriedData);
	}else{
		//ret = ZCFG_INTERNAL_ERROR;
	}

	ZOS_FREE(recvBuf);

	return ret;
}

zcfgRet_t zcfgFeMsgObjQryInstances(uint32_t type, zcfg_name_t nameSeqNum, objIndex_t *queryObjIid, char **instances)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL;
	zcfgMsg_t *recvMsgHdr = NULL;
	char *sendBuf = NULL, *recvBuf = NULL;

	zcfgLog(ZCFG_LOG_DEBUG, "%s\n", __FUNCTION__);

	if(!queryObjIid || !instances)
		return ZCFG_INTERNAL_ERROR;

	sendBuf = (char *)ZOS_MALLOC(sizeof(zcfgMsg_t));

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	//sendMsgHdr->type = REQUEST_QUERY_TR98_OBJ_INSTANCE_ARRAY;
	sendMsgHdr->type = type;
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	sendMsgHdr->oid = nameSeqNum;
	memcpy(sendMsgHdr->objIid, queryObjIid, sizeof(objIndex_t));

	rst = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(rst != ZCFG_SUCCESS) {
		return rst;
	}

	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		ZOS_FREE(recvBuf);
		return recvMsgHdr->statCode;
	}

	if( recvMsgHdr->length > 0 ) {
		char *queriedData = (char *)malloc(recvMsgHdr->length + 1);
		char *data = (char *)(recvBuf+sizeof(zcfgMsg_t));

		memcpy(queriedData, data, recvMsgHdr->length);
		*(queriedData+recvMsgHdr->length) = '\0';

		*instances = queriedData;
	}else{
		rst = ZCFG_INTERNAL_ERROR;
	}

	ZOS_FREE(recvBuf);

	return rst;
}

zcfgRet_t zcfgFeMultiParmAttrSet(zcfgFeSetParmAttrLst_t *setParmAttrLst)
{
	zcfgFeSetParmAttr_t *setParmAttr = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfgMsg_t *sendMsgHdr = NULL, *recvMsgHdr = NULL;
	bool dataLengthFail = false;
	char *sendBuf = NULL, *recvBuf = NULL, *msgParmAttrData = NULL;
	uint32_t *nSetParmAttr = NULL, msgParmAttrDataLen = 0;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);
		
	if(!setParmAttrLst)
		return ZCFG_INTERNAL_ERROR;

	if(!setParmAttrLst->n || !setParmAttrLst->parmSetAttrDataLen) {
		zcfgLog(ZCFG_LOG_INFO, "%s, nsetParmAttr %d, parmSetAttrDataLen %d\n", __FUNCTION__, setParmAttrLst->n, setParmAttrLst->parmSetAttrDataLen);
		return ZCFG_SUCCESS;
	}

	sendBuf = (char *)malloc(sizeof(zcfgMsg_t) + setParmAttrLst->parmSetAttrDataLen + sizeof(uint32_t));
	memset(sendBuf, 0, (sizeof(zcfgMsg_t) + setParmAttrLst->parmSetAttrDataLen + sizeof(uint32_t)));

	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = REQUEST_SET_PARAM_ATTR;
	sendMsgHdr->length = setParmAttrLst->parmSetAttrDataLen + sizeof(uint32_t);
	sendMsgHdr->dstEid = ZCFG_EID_ZCMD;
	nSetParmAttr = (uint32_t *)(sendMsgHdr + 1);
	*nSetParmAttr = setParmAttrLst->n;
	msgParmAttrData = (char *)(nSetParmAttr + 1);
	
	setParmAttr = setParmAttrLst->start;
	while(setParmAttr) {
		char *parmName = NULL, *setAttrParmName = NULL;
		memcpy(msgParmAttrData, &setParmAttr->parmSetAttr, sizeof(zcfgParmSetAttr_t));
		if(setParmAttr->parmName){
			parmName = msgParmAttrData + sizeof(zcfgParmSetAttr_t);
			strncpy(parmName, setParmAttr->parmName, setParmAttr->parmSetAttr.parmNameLen);
		}
		if(setParmAttr->parmSetAttr.setAttrParmNameLen) {
			setAttrParmName = parmName + setParmAttr->parmSetAttr.parmNameLen;
			strncpy(setAttrParmName, setParmAttr->setAttrParmName, setParmAttr->parmSetAttr.setAttrParmNameLen);
		}

		msgParmAttrData += sizeof(zcfgParmSetAttr_t) + setParmAttr->parmSetAttr.parmNameLen + setParmAttr->parmSetAttr.setAttrParmNameLen;
		msgParmAttrDataLen += sizeof(zcfgParmSetAttr_t) + setParmAttr->parmSetAttr.parmNameLen + setParmAttr->parmSetAttr.setAttrParmNameLen;
		if(msgParmAttrDataLen > setParmAttrLst->parmSetAttrDataLen) {
			dataLengthFail = true;
			break;
		}
		setParmAttr = setParmAttr->next;
	}

	if(dataLengthFail) {

		free(sendBuf);
		return ZCFG_INTERNAL_ERROR;		
	}

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);

	if(ret != ZCFG_SUCCESS) {
		printf("%s: not ZCFG_SUCCESS\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	recvMsgHdr = (zcfgMsg_t*)recvBuf;
	if ( recvMsgHdr->type == RESPONSE_FAIL ) {
		printf("%s: RESPONSE_FAIL\n", __FUNCTION__);
		ret = recvMsgHdr->statCode;
		ZOS_FREE(recvBuf);
		return ret;
	}
	
	ZOS_FREE(recvBuf);

	return ZCFG_SUCCESS;
}

