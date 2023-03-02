/******************************************************************************/
/*
*  Copyright (C) 2012 ZyXEL Communications, Corp.
*  All Rights Reserved.
*
* ZyXEL Confidential; Need to Know only.
* Protected as an unpublished work.
*
* The computer program listings, specifications and documentation
* herein are the property of ZyXEL Communications, Corp. and
* shall not be reproduced, copied, disclosed, or used in whole or
* in part for any reason without the prior express written permission of
* ZyXEL Communications, Corp.
*/
/******************************************************************************/

#ifndef _CWMP_CMD_H_
#define _CWMP_CMD_H_

#include <stdio.h>
#include <pthread.h>
#include "cwmpsocket.h"
#include "cwmptypedef.h"

#define TR069CMD_PATH "/tmp/tr069_cmd"
#ifdef ZYXEL_TR69_SUPPLEMENTAL_ACS
#define TR069CMD_SUPPLEMENTAL_PATH "/tmp/tr069_supplemental_cmd"
#endif
#ifdef IAAAA_CUSTOMIZATION
#define TR069CMD_OUTPUT "/tmp/tr069cmdOutputData"
#endif
#define SIZE_IPCVAL				1024 /*ObjectValue*/

extern uint32_t TR069_FEATURE_Flag;
#define TR069_FEATURE_UPLOAD_BY_PUT		(1 << 0) // RPC upload file by PUT method instead of POST.
#define TR069_FEATURE_CLEARTEXT_PSK		(1 << 1) // readable psk instead of empty string.
#define TR069_FEATURE_BOOTSTRAP_WITHOUT_VALUECHANGE		(1 << 2) //According to TR-069_Amendment-5, MUST discard "4 VALUE CHANGE" event on BOOTSTRAP.
#ifdef ZYXEL_HTTP_DIGEST_AUTH_WITH_HEX_CNONCE
#define TR069_FEATURE_HTTP_DIGEST_AUTH_WITH_HEX_CNONCE		(1 << 3)	//Digest Cnonce (BASE64->HEX)
#endif

typedef enum tr069IpcCmd_s{
	eTr069IpcEnable,
	eTr069IpcDisable,
	eTr069IpcClose,
	eTr069IpcDebugOn,
	eTr069IpcDebugOff,
	eTr069IpcFeatureOn,
	eTr069IpcFeatureOff,
	eTr069IpcDisplay,
	eTr069IpcActive,
	eTr069IpcIpProtocol,
	eTr069IpcAcsUrl,
	eTr069IpcAcsUsername,
	eTr069IpcAcsPassword,
	eTr069IpcRequestUsername,
	eTr069IpcRequestPassword,
	eTr069IpcPeriodicEnable,
	eTr069IpcInformInterval,
	eTr069IpcInformTime,
	eTr069IpcActiveNotifyUpdateInterval,
	eTr069IpcApplyNow,
	eTr069IpcOutputObjNumName,
	eTr069IpcDiagTest
}tr069IpcCmd_t;

typedef struct tr069IpcMsgData_s{
	tr069IpcCmd_t ipcCmd;
	char data[SIZE_IPCVAL+1];
}tr069IpcMsgData_t;

extern void TR069_FEATURE_CONFIG( uint32 flag );
void cwmp_cmd_task(void);

#endif /*_CWMP_CMD_H_*/
