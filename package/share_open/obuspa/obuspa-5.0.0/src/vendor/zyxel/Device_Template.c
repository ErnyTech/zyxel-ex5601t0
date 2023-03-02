
/**
 *
 * Implements the Device.XXX data model object
 *
 */
#include "device_data_node_api.h"
//------------------------------------------------------------------------------
// Data model path strings
#define DEVICE_TEMPLATE "Device.Template"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Ccached values of ip parameter


//------------------------------------------------------------------------------
// Data node callback functions

int DEVICE_TEMPLATE_Get_Value_CB(dm_req_t *req, char *buf, int len){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return Get_Object_Parameter_Value(req,buf,len);
}
int DEVICE_TEMPLATE_Set_Value_CB(dm_req_t *req, char *buf){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return Set_Object_Parameter_Value(req,buf);
}
int DEVICE_TEMPLATE_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s] \n",req->path);
	return Add_Object_Node(req);
}
int DEVICE_TEMPLATE_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return Del_Object_Node(req);
}
int DEVICE_TEMPLATE_Validate_Value_CB(dm_req_t *req, char *value){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Notify_Set_CB(dm_req_t *req, char *value){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Validate_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Notify_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Validate_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Notify_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Sync_Oper_CB(dm_req_t *req, char *command_key,kv_vector_t *input_args, kv_vector_t *output_args){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_TEMPLATE_Async_Oper_CB(dm_req_t *req, kv_vector_t *input_args,int instance){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}

DM_REGISTER_DATA_NODE_CB Register_DEVICE_TEMPLATE = {
	DEVICE_TEMPLATE_Get_Value_CB,
	DEVICE_TEMPLATE_Set_Value_CB,
	DEVICE_TEMPLATE_Add_CB,
	DEVICE_TEMPLATE_Del_CB,
	DEVICE_TEMPLATE_Validate_Value_CB,
	DEVICE_TEMPLATE_Notify_Set_CB,
	DEVICE_TEMPLATE_Validate_Add_CB,
	DEVICE_TEMPLATE_Notify_Add_CB,
	DEVICE_TEMPLATE_Validate_Del_CB,
	DEVICE_TEMPLATE_Notify_Del_CB,
	DEVICE_TEMPLATE_Sync_Oper_CB,
	DEVICE_TEMPLATE_Async_Oper_CB
};

//------------------------------------------------------------------------------
//Entry point base functions : DEVICE_TEMPLATE_Init ,DEVICE_TEMPLATE_Start ,DEVICE_TEMPLATE_Stop

int DEVICE_TEMPLATE_Init(void) {
	int err = USP_ERR_OK;
	// Register parameters implemented by this component
	err |= Register_Object_Node(DEVICE_TEMPLATE,&Register_DEVICE_TEMPLATE);

	return err;
}
int DEVICE_TEMPLATE_Start(void) {
	int err = USP_ERR_OK;
	err |= Sync_Object_NumberOfEntries_Node(DEVICE_TEMPLATE);
	return err;
}
int DEVICE_TEMPLATE_Stop(void) {

	return USP_ERR_OK;
}


