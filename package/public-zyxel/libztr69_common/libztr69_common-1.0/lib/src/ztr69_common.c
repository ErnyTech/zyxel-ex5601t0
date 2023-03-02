#include "zcfg_fe_tr98.h"
#include "zcfg_msg.h"
#include "zcfg_eid.h"
#include "ztr69_common.h"
#include "zcfg_fe_rdm_string.h"
typedef uint16_t uint16;		/* 16-bit unsigned integer	*/

/*
    RPC data type definitions.
*/
typedef enum{
	XSTRING = 0,
	XINT,
	XUINT,
	XBOOLEAN,
	XDATETIME,
	XBASE64,
	XDOUBLE,
	XOBJ,
	XARRAY,
	XHEX,
	XLONG,
	XULONG
}xsdType_t;

typedef struct spvHdlStruct_s {
	void *tr98SpvObj;
}spvHdlStruct_t;

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

extern tr069ERR_e cwmp_cm_get_values(char *objname, aParameterList_t *pList, int *maxNumOfBuf );
extern tr069ERR_e cwmp_cm_parse_fullpath( char *fullpath, char *objpath, char *name, bool *isPartial);
extern void cwmp_rpc_add_value( aParameterList_t *parameterList, char *name, xsdType_t xsiType, char *value); 
extern xsdType_t cwmp_cm_type_cm2rpc( json_type cmType );
extern tr069ERR_e cwmp_cm_set_batch_values(aParameterList_t * pList,aFault_t * pFault,int * maxNumOfBuf);
extern tr069ERR_e cwmp_cm_add(char * objname,uint16 * instanceNum);
extern tr069ERR_e cwmp_cm_delete( char *objname );
extern void cwmp_rpc_convert_TR181_Interface_value(bool addPrefix, aParameterList_t *pList);
extern char* cwmp_rpc_buf_new( int bufSize, int *maxNum);
extern void cwmp_rpc_add_value( aParameterList_t *parameterList, char *name, xsdType_t xsiType, char *value);
extern void cwmp_rpc_clear_parametervaluestruct( aParameterList_t *parameterList);
extern bool gTr181mode;

int _gloglevel = ZTR69_COMMON_LOG_LEVEL_OUTPUT;
int RD_debug = 0;
#define WRAPPER_LOG(logLev,format, ...) { \
    if(_gloglevel >= logLev) { \
	if(RD_debug){ fprintf(stdout,"%s(%s:%d) ", __func__,__FILE__,__LINE__); } \
        fprintf(stdout, format, ##__VA_ARGS__);} \
	}

ZTR69_COMMON_ERR_e ztr69_common_init(int log_level)
{
	_gloglevel = log_level;
	gTr181mode = TRUE;
	return 0;
}

ZTR69_COMMON_ERR_e ztr69_common_para_alloc(ztr69ParamterList_t *pCwmpPList)
{
	/*allocate parameter list*/
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter \n");
	memset(&pCwmpPList->List,0,sizeof(aParameterList_t));

	pCwmpPList->List.parameterValueStruct = (aParameterValueStruct_t*)cwmp_rpc_buf_new(sizeof(aParameterValueStruct_t), &pCwmpPList->maxNumOfListBuf);
	if(!pCwmpPList->List.parameterValueStruct)
		return ZTR69_COMMON_RESOURCES_EXCEEDED;

	pCwmpPList->List.rpcHdlDesc = (void *)malloc(sizeof(spvHdlStruct_t));
	if(!pCwmpPList->List.rpcHdlDesc)
		return ZTR69_COMMON_RESOURCES_EXCEEDED;

	memset(pCwmpPList->List.rpcHdlDesc, 0, sizeof(spvHdlStruct_t));
	/*allocate fault buffer*/
	memset(&pCwmpPList->faultBuf,0,sizeof(aFault_t));
	pCwmpPList->faultBuf.setParameterValuesFault = (aSetParameterValuesFault_t*)cwmp_rpc_buf_new(sizeof(aSetParameterValuesFault_t), &pCwmpPList->maxNumOfListfaultBuf);
	if(!pCwmpPList->faultBuf.setParameterValuesFault)
		return ZTR69_COMMON_RESOURCES_EXCEEDED;
		
	return ZTR69_COMMON_OK;
}

ZTR69_COMMON_ERR_e ztr69_common_para_clear(ztr69ParamterList_t *pCwmpPList)
{
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter \n");
	/*clear parameter list context*/
	cwmp_rpc_clear_parametervaluestruct(&pCwmpPList->List);
	/*clear fault parameter context*/
	if(pCwmpPList->faultBuf.arrayNum)
	{
		memset(pCwmpPList->faultBuf.setParameterValuesFault,0,sizeof(aSetParameterValuesFault_t)*pCwmpPList->faultBuf.arrayNum);
		pCwmpPList->faultBuf.arrayNum = 0;
	}
	pCwmpPList->faultBuf.faultString = NULL; //this value is not used for setparameter case, we only use the value in setParameterValuesFault for set value case. 
	pCwmpPList->faultBuf.faultCode = 0;
	return ZTR69_COMMON_OK;
}


ZTR69_COMMON_ERR_e ztr69_common_para_free(ztr69ParamterList_t *pCwmpPList)
{
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter \n");
	/*free parameter list*/
	ztr69_common_para_clear(pCwmpPList);
	if(pCwmpPList->List.rpcHdlDesc!=NULL)
	{
		free(pCwmpPList->List.rpcHdlDesc);
		pCwmpPList->List.rpcHdlDesc = NULL;
	}
	pCwmpPList->maxNumOfListBuf = 0;

	/*free fault buffer*/
	if(pCwmpPList->faultBuf.setParameterValuesFault!=NULL)
	{
		free(pCwmpPList->faultBuf.setParameterValuesFault);
		pCwmpPList->faultBuf.setParameterValuesFault = NULL;
	}
	pCwmpPList->maxNumOfListfaultBuf = 0;
	return ZTR69_COMMON_OK;
}

ZTR69_COMMON_ERR_e ztr69_common_para_add(ztr69ParamterList_t *pCwmpPList, char * objname, char * value)
{
	bool isPartial = FALSE;
	int oid = 0;
	int paramType;
	char objpath[STRSIZE(SIZE_NAME)] = {0}, name[STRSIZE(SIZE_NAME)] = {0};
	objIndex_t objIid;
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter ,objname = %s, value = %s\n",objname,value);
	if( cwmp_cm_parse_fullpath( objname, objpath, name, &isPartial)!= TR069_OK){
		return ZTR69_COMMON_INVALID_PARAMETER_NAME;
	}
	IID_INIT(objIid);
	oid = zcfgFeObjNameToObjId( objpath, &objIid);
	if(oid < 0)
	{
		return ZTR69_COMMON_INVALID_ARGUMENTS;
	}
	paramType = zcfgFeParamTypeGetByName(oid, name);
	if(paramType < 0)
	{
		return ZTR69_COMMON_INVALID_ARGUMENTS;
	}
	cwmp_rpc_add_value(&pCwmpPList->List, objname, cwmp_cm_type_cm2rpc(paramType), value);
	return ZTR69_COMMON_OK;
}

ZTR69_COMMON_ERR_e ztr69_common_get(char *objname, ztr69ParamterList_t *pCwmpPList)
{
	//just do wrapper
	ZTR69_COMMON_ERR_e ret;
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter,objname = %s \n",objname);
	ret = cwmp_cm_get_values(objname, &pCwmpPList->List, &pCwmpPList->maxNumOfListBuf );
	if(ret == ZTR69_COMMON_OK)
	{
		//convert result from get tr181. Add "Device."
		cwmp_rpc_convert_TR181_Interface_value(1,&pCwmpPList->List);
	}
	return ret;
}

ZTR69_COMMON_ERR_e ztr69_common_set(ztr69ParamterList_t *pCwmpPList)
{
	tr069ERR_e ret;
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter \n");
	//convert to set tr181. remove "Device."
	cwmp_rpc_convert_TR181_Interface_value(0,&pCwmpPList->List);
	ret = cwmp_cm_set_batch_values(&pCwmpPList->List, &pCwmpPList->faultBuf, &pCwmpPList->maxNumOfListBuf);
	return ret;
}

ZTR69_COMMON_ERR_e ztr69_common_add(char* objname,uint16* instanceNum)
{
	tr069ERR_e ret;
	bool isPartial = FALSE;
	char objpath[STRSIZE(SIZE_NAME)] = {0}, name[STRSIZE(SIZE_NAME)] = {0};
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter, objname = %s\n",objname);
	if( cwmp_cm_parse_fullpath( objname, objpath, name, &isPartial)!= TR069_OK){
		return TR069_INVALID_PARAMETER_NAME;
	}
	ret = cwmp_cm_add(objname,instanceNum);
	return ret;
}

ZTR69_COMMON_ERR_e ztr69_common_delete(char* objname)
{
	tr069ERR_e ret;
	WRAPPER_LOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"enter, objname = %s\n",objname);
	ret = cwmp_cm_delete(objname);
	return ret;
}
