#include "zcfg_common.h"
#include "esmd.h"
#ifdef BROADCOM_PLATFORM
#include "esmd_brcm_wrapper.h"
#endif

void handleExternalPhyEvent(void);
void processKernelMonitor(void);
zcfgRet_t esmd_init(void);
void zyEnableWifiLED();
int updateLanLinkStatus(char *msgData);
void updateWANEthIfaceObjStatus(objIndex_t *, void *);
void handleWANAutoDetectETHDown(void);
bool handleWANAutoDetectETH(void);
void updateETHWANStatus(char *msgData);
void handleXtmUpInit(wanLinkType currXtmType, wanLinkType prevXTMType);
void esmdSupportCAT3Cable(void);
bool esmdLanPhyReset();
void flushArpForAPMode(char intfName[]);
//INTERNET_LED_CONTROL
#if !defined(ZYXEL_ZYINETLED)
void esmdWanCtlInternetLED(internet_led_t type, internet_led_state_t state);
#endif
void restartDslConnection(void);
int esmd_getATSHInfo(char *info);
int esmd_setSerialNumber(char *sn);
#ifdef ZYXEL_AT_SET_KEY
int esmd_setKeys(char *key);
int esmd_getKeys(char *key);
#endif
int esmd_atwz(char *mrdInfo);
int esmd_getUbootEnv(char *str);
int esmd_setUbootEnv(char *str);
zcfgRet_t esmd_getRasSize(char *buffer);
int esmd_set81sn(char *sn81);
#ifdef ZYXEL_AT_SET_TUTK_UID
int esmd_setTutkUid(char *t_uid);
int esmd_getTutkUid(char *t_uid);
#endif
void updateExtPhyLinkStatus(void);