
/**
 *
 * Implements the Device.XXX data model object
 *
 */
#include "device_data_node_api.h"
//------------------------------------------------------------------------------
// Data model path strings
#define DEVICE_DUMMY "Device.Dummy"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Ccached values of ip parameter


//------------------------------------------------------------------------------
// Data node callback functions

int DEVICE_DUMMY_Get_Value_CB(dm_req_t *req, char *buf, int len){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Set_Value_CB(dm_req_t *req, char *buf){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s] \n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Validate_Value_CB(dm_req_t *req, char *value){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Notify_Set_CB(dm_req_t *req, char *value){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Validate_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Notify_Add_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Validate_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Notify_Del_CB(dm_req_t *req){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Sync_Oper_CB(dm_req_t *req, char *command_key,kv_vector_t *input_args, kv_vector_t *output_args){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}
int DEVICE_DUMMY_Async_Oper_CB(dm_req_t *req, kv_vector_t *input_args,int instance){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return USP_ERR_OK;
}

DM_REGISTER_DATA_NODE_CB Register_DEVICE_DUMMY = {
	DEVICE_DUMMY_Get_Value_CB,
	DEVICE_DUMMY_Set_Value_CB,
	DEVICE_DUMMY_Add_CB,
	DEVICE_DUMMY_Del_CB,
	DEVICE_DUMMY_Validate_Value_CB,
	DEVICE_DUMMY_Notify_Set_CB,
	DEVICE_DUMMY_Validate_Add_CB,
	DEVICE_DUMMY_Notify_Add_CB,
	DEVICE_DUMMY_Validate_Del_CB,
	DEVICE_DUMMY_Notify_Del_CB,
	DEVICE_DUMMY_Sync_Oper_CB,
	DEVICE_DUMMY_Async_Oper_CB
};

//------------------------------------------------------------------------------
//Entry point base functions : DEVICE_DUMMY_Init ,DEVICE_DUMMY_Start ,DEVICE_DUMMY_Stop

int DEVICE_DUMMY_Init(void) {
	int err = USP_ERR_OK;
	// Register parameters implemented by this component
	//err |= Register_Object_Node(DEVICE_DUMMY,&Register_DEVICE_DUMMY);

	return err;
}
int DEVICE_DUMMY_Start(void) {
	int err = USP_ERR_OK;
	//err |= Sync_Object_NumberOfEntries_Node(DEVICE_DUMMY);
	return err;
}
int DEVICE_DUMMY_Stop(void) {

	return USP_ERR_OK;
}


