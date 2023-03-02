#ifndef _ZCFG_FE_RDM_ACCESS_H
#define _ZCFG_FE_RDM_ACCESS_H
#include "zcfg_msg.h"
/*
 *  Provide the API for front end process to communicate with
 *  zcmd.
 */
zcfgRet_t zcfgFeSharedMemInit();
zcfgRet_t zcfgFeObjRead(zcfg_offset_t, objIndex_t *, uint32_t, char **);
zcfgRet_t zcfgFeObjWrite(zcfg_offset_t, objIndex_t *, char *, uint32_t, char *);
zcfgRet_t zcfgFeWholeObjReadByName(char *tr181path, uint32_t type, char **result);
zcfgRet_t zcfgFeWholeObjWriteByName(char *tr181path, char *setValue, uint32_t type, char *feedbackMsg);
zcfgRet_t zcfgFeIfStackObjReadByLayer(char *layer, int hilo, char **result);

zcfgRet_t zcfgFeReqSend(uint32_t, char *);

#define zcfgFeDelayApply(payload)                       zcfgFeReqSend(ZCFG_MSG_DELAY_APPLY, payload)
#define zcfgFeReApply()                                 zcfgFeReqSend(ZCFG_MSG_RE_APPLY, NULL)
#define zcfgFeFwUpgrade(payload)                        zcfgFeReqSend(REQUEST_FIRMWARE_UPGRADE, payload)
#define zcfgFeFwUpgradeFinish(payload)                  zcfgFeReqSend(ZCFG_MSG_FIRMWARE_UPGRADE, payload)
#define zcfgFeFwUpgradeForFWUR(payload)                 zcfgFeReqSend(REQUEST_FIRMWARE_UPGRADE_FWUR, payload)
#define zcfgFeFwUpgradeWrite(payload)                   zcfgFeReqSend(REQUEST_FIRMWARE_WRITE_ONLY, payload)
#define zcfgFeFwUpgradeStore(payload)					zcfgFeReqSend(REQUEST_FIRMWARE_WRITE_AND_STORE, payload)
#define zcfgFeFwUpgradeCheck(payload)                   zcfgFeReqSend(REQUEST_FIRMWARE_UPGRADE_CHK, payload)
#define zcfgFeRestoreDefault(payload)                   zcfgFeReqSend(REQUEST_RESTORE_DEFAULT, payload)
#define zcfgFeRestoreDeCheck(payload)                   zcfgFeReqSend(REQUEST_RESTORE_DEFAULT_CHK, payload)
#define zcfgFeRestorePartial(payload)                   zcfgFeReqSend(REQUEST_PARTIAL_RESTORE_DEFAULT, payload)
#define zcfgFeRomdUpgrade(payload)                      zcfgFeReqSend(REQUEST_ROMD_UPGRADE, payload)
#define zcfgFeRomdSave(payload)                         zcfgFeReqSend(ZCFG_MSG_SAVE_ROMD, payload)
#define zcfgFeRomdClean(payload)                        zcfgFeReqSend(ZCFG_MSG_CLEAN_ROMD, payload)
#define zcfgFeRomdGet()                                 zcfgFeReqSend(ZCFG_MSG_GET_ROMD, NULL)
#define zcfgFeCheckFwIdWhenUpgradingStatus(payload)     zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_FWID_STATUS, payload)
#define zcfgFeCheckModelWhenUpgradingStatus(payload)    zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_STATUS, payload)
#define zcfgFeConfBackup(payload)                       zcfgFeReqSend(REQUEST_CONFIG_BACKUP, payload)
#define zcfgFeAutoProvision(payload)                    zcfgFeReqSend(ZCFG_MSG_AUTO_PROVISION, payload)
#define zcfgFeResetWan(payload)                         zcfgFeReqSend(REQUEST_REINIT_MACLESS_PROVISIONING, payload)
#define zcfgFeReqReboot(payload)                        zcfgFeReqSend(ZCFG_MSG_REQ_REBOOT, payload)
#define zcfgFeReqSyncFlash(payload)                     zcfgFeReqSend(ZCFG_MSG_REQ_SYNCFLASH, payload)
#define zcfgFeCheckFwIdWhenUpgradingOff(payload)        zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_FWID_OFF, payload)
#define zcfgFeCheckFwIdWhenUpgradingOn(payload)         zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_FWID_ON, payload)
#define zcfgFeCheckModelWhenUpgradingOff(payload)       zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_OFF, payload)
#define zcfgFeCheckModelWhenUpgradingOn(payload)        zcfgFeReqSend(ZCFG_MSG_UPGRADING_FW_CHECK_MODEL_ON, payload)
#define zcfgFeReqTr98ParmAttrList(payload)              zcfgFeReqSend(ZCFG_MSG_REQUEST_GET_PARAM_ATTR_LIST, payload)
#define zcfgFeReqSwitchBootPartition(payload)           zcfgFeReqSend(ZCFG_MSG_REQ_SWITCH_BOOT_PARTITION, payload)
#define zcfgFeSysLogClean(payload)                      zcfgFeReqSend(ZCFG_MSG_CLEAN_SYSLOG, payload)
#define zcfgFeQoSGet(payload)                           zcfgFeReqSend(ZCFG_MSG_QOS_GET_STATE, payload)
#define zcfgFeSetParamK(payload)                        zcfgFeReqSend(REQUEST_SET_PARAMETERKEY, payload)
#define zcfgFeFwUpgradeOnlineCheck(payload)             zcfgFeReqSend(REQUEST_FIRMWARE_UPGRADE_ONLINE_CHK, payload)
#define zcfgFeFwIDCheck(payload)                        zcfgFeReqSend(REQUEST_FIRMWARE_ID_CHK, payload)
#define zcfgFeImportCertificate(payload)                zcfgFeReqSend(REQUEST_Import_Certificate, payload)
#define zcfgFeDoSystemCall(payload)                	zcfgFeReqSend(REQUEST_LOW_AUTHORITY_DO_SYSTEM_CALL, payload)
#define zcfgFeDoSystemCall_S(payload)                	zcfgFeReqSend(REQUEST_LOW_AUTHORITY_DO_SYSTEM_CALL_S, payload)
#define zcfgFeReqCalibration24g(payload)                zcfgFeReqSend(ZCFG_MSG_REQ_CALIBRATION_24G, payload)
#define zcfgFeReqCalibration5g(payload)                 zcfgFeReqSend(ZCFG_MSG_REQ_CALIBRATION_5G, payload)
#ifdef ZYXEL_WEB_GUI_SHOW_ZYEE
#define zcfgFeZyEEMonitorGet(payload)                   zcfgFeReqSend(ZCFG_MSG_GET_ZYEE_MONITOR, payload)
#endif

#ifdef ZYXEL_WIFI_KEY_REMINDER
#define zcfgFeSetWifiRedirect(payload)                  zcfgFeReqSend(ZCFG_MSG_SET_WIFIREDIRECT, payload)
#endif

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
#define zcfgFeRestoreConfig(payload)                    zcfgFeReqSend(REQUEST_RESTORE_CONFIG, payload)
#endif

/*TR98 support*/
zcfgRet_t zcfgFeMappingNameGet(uint32_t msgType, char *fullPathName, char *result);

zcfgRet_t zcfgFe98To181ObjMappingNameGet(const char *tr98PathName, char *tr181PathName);
zcfgRet_t zcfgFe181To98ObjMappingNameGet(const char *tr181PathName, char *tr98PathName);
#define zcfgFeAllIGDNameGet()           zcfgFeReqSend(ZCFG_MSG_GET_TR98_ALL_IGD_NAMES, NULL)

#if 1 //change  to new TR98 mapping table
#define zcfgFe98To181MappingNameGet(fullPathName, result) zcfgFe98To181ObjMappingNameGet(fullPathName, result)
#define zcfgFe181To98MappingNameGet(fullPathName, result) zcfgFe181To98ObjMappingNameGet(fullPathName, result)
#else
#define zcfgFe98To181MappingNameGet(fullPathName, result) zcfgFeMappingNameGet(ZCFG_MSG_REQUEST_TR98_MAPPING, fullPathName, result)
#define zcfgFe181To98MappingNameGet(fullPathName, result) zcfgFeMappingNameGet(ZCFG_MSG_REQUEST_TR181_MAPPING, fullPathName, result)
#endif

/*End of TR98 support*/

int zcfgFeObjMaxLenGet(zcfg_offset_t rdmObjId);
int zcfgFeArrayLengthGetByName(char* tr181path);
void SetFactoryResetCode(const char *frCode);
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
void SetFactoryResetCode_TAAAB(const char *frCode);
void SetRebootCode_TT(const char *rbCode);
void SetInterfaceDownCode(const char *IntfDownCode);
#endif
extern zcfgRet_t zcfgFeZyshCmdSend(int eid, uint32_t cmd, int value, char *param,int len, char *zysh_ret);

#endif
