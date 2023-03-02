/*
 * vendor_device_main.c
 *
 *  Created on: Mar 20, 2022
 *
 */

#include "zcfg_msg.h"

#include "common_defs.h"

#include "vendor_data_model.h"
#include "vendor_device_main.h"

int Vendor_Device_Init(void) {
	int err = USP_ERR_OK;
	zcfgEidInit(ZCFG_EID_TR69, "obuspa");
	err |= Device_TR181_Init();
	err |= Device_TR181_Customize_Init();
	return err;
}

int Vendor_Device_Start(void) {
	int err = USP_ERR_OK;
	err |= Device_TR181_Start();
	err |= Device_TR181_Customize_Start();
	return err;
}

int Vendor_Device_Stop(void) {
	int err = USP_ERR_OK;
	err |= Device_TR181_Stop();
	err |= Device_TR181_Customize_Stop();
	return err;
}


