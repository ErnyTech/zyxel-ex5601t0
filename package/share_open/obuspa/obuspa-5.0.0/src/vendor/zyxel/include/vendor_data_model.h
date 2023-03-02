/*
 * data_model.h
 *
 *  Created on: Feb 21, 2022
 *
 */

#ifndef _VENDOR_DATA_MODEL_H_
#define _VENDOR_DATA_MODEL_H_

typedef int (*device_data_node_init_cb)(void);
typedef int (*device_data_node_start_cb)(void);
typedef int (*device_data_node_stop_cb)(void);

typedef struct
{
	device_data_node_init_cb device_data_node_init;
	device_data_node_start_cb device_data_node_start;
	device_data_node_stop_cb device_data_node_stop;
}DEVICE_DATA_NODE_CB;

int Device_TR181_Init(void);
int Device_TR181_Start(void);
int Device_TR181_Stop(void);

int Device_TR181_Customize_Init(void);
int Device_TR181_Customize_Start(void);
int Device_TR181_Customize_Stop(void);

#endif /* _VENDOR_DATA_MODEL_H_ */
