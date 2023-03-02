/*
 * Device_Customize_Main_Entry.c
 *
 *  Created on: Feb 21, 2022
 *
 */
#include "device_data_node_api.h"
#include "vendor_data_model.h"

// Add entry point functions of data node for TRXXX data model
//Device.Dummy
extern int DEVICE_DUMMY_Init(void);
extern int DEVICE_DUMMY_Start(void);
extern int DEVICE_DUMMY_Stop(void);


DEVICE_DATA_NODE_CB TR181_Customize_Data_Node[] = {
	{ DEVICE_DUMMY_Init, DEVICE_DUMMY_Start, DEVICE_DUMMY_Stop }, // Device.Dummy
};


//------------------------------------------------------------------------------
// Data node callback functions


/*********************************************************************//**
 **
 ** Device_TR181_Customize_Init
 **
 ** Initialises this component, and registers all parameters and vendor hooks, which it implements
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/

int Device_TR181_Customize_Init(void) {
	int err = USP_ERR_OK;
	int i = 0;

	if(err == USP_ERR_OK){
		DEVICE_DATA_NODE_CB *pDataNode = NULL;
		for(i= 0;i < sizeof(TR181_Customize_Data_Node)/sizeof(TR181_Customize_Data_Node[0]);i++){
				pDataNode =  &TR181_Customize_Data_Node[i];
				if(pDataNode->device_data_node_init != NULL){
					err |= pDataNode->device_data_node_init();
				}

		}
	}
	return err;
}

/*********************************************************************//**
 **
 ** Device_TR181_Customize_Start
 **
 ** Called after data model has been registered and after instance numbers have been read from the USP database
 ** Typically this function is used to seed the data model with instance numbers or
 ** initialise internal data structures which require the data model to be running to access parameters
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/
int Device_TR181_Customize_Start(void) {
	int err = USP_ERR_OK;
	int i = 0;
	DEVICE_DATA_NODE_CB *pDataNode = NULL;
	for(i= 0;i < sizeof(TR181_Customize_Data_Node)/sizeof(TR181_Customize_Data_Node[0]);i++){
			pDataNode =  &TR181_Customize_Data_Node[i];
			if(pDataNode->device_data_node_start != NULL){
				err |= pDataNode->device_data_node_start();
			}
	}
	return err;
}
/*********************************************************************//**
 **
 ** Device_TR181_Customize_Stop
 **
 ** Called when stopping USP agent gracefully, to free up memory and shutdown
 ** any vendor processes etc
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/
int Device_TR181_Customize_Stop(void) {
	int err = USP_ERR_OK;
	int i = 0;
	DEVICE_DATA_NODE_CB *pDataNode = NULL;
	for(i= 0;i < sizeof(TR181_Customize_Data_Node)/sizeof(TR181_Customize_Data_Node[0]);i++){
			pDataNode =  &TR181_Customize_Data_Node[i];
			if(pDataNode->device_data_node_stop != NULL){
				err |= pDataNode->device_data_node_stop();
			}
	}
	return err;
}

