/*
 * register_data_node.h
 *
 *  Created on: Mar 13, 2022
 *
 */

#ifndef _DEVICE_DATA_NODE_API_H_
#define _DEVICE_DATA_NODE_API_H_
// Include standard
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

//Include Obuspa's API
#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "dm_access.h"

typedef struct
{
	dm_get_value_cb_t get_value_cb;
	dm_set_value_cb_t set_value_cb;

	dm_add_cb_t add_cb;
    dm_del_cb_t del_cb;

	dm_validate_value_cb_t validate_value_cb;
	dm_notify_set_cb_t notify_set_cb;

	dm_validate_add_cb_t validate_add_cb;
	dm_notify_add_cb_t notify_add_cb;

	dm_validate_del_cb_t validate_del_cb;
	dm_notify_del_cb_t notify_del_cb;

	dm_sync_oper_cb_t sync_oper_cb;
	dm_async_oper_cb_t async_oper_cb;

}DM_REGISTER_DATA_NODE_CB;

int Register_Object_Paramter_Node(char *obj_node_path,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb);
int Register_Object_Node(char *data_node_path,DM_REGISTER_DATA_NODE_CB *pdm_register_data_node_cb);
int Sync_Object_NumberOfEntries_Node(char *data_node_path);
int Get_Object_Parameter_Value(dm_req_t *req, char *buf, int len);
int Set_Object_Parameter_Value(dm_req_t *req, char *buf);
int Add_Object_Node(dm_req_t *req);
int Del_Object_Node(dm_req_t *req);


#endif /* _DEVICE_DATA_NODE_API_H_ */
