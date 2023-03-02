#ifndef _ESMD_LINK_WAN_H
#define _ESMD_LINK_WAN_H

bool isDslLineLinkUp();
zcfgRet_t initWanLinkInfo();
void esmdAtmLinkStatusChanged(objIndex_t *, bool);
void esmdPtmLinkStatusChanged(objIndex_t *, bool);
void updateWanLinkStatus(const char *, bool skipEthWanAutoHuntProcess);
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
void updateOpticalLinkStatus(char *buf);
#endif
#ifdef ZYXEL_SFP_LINKUP_DELAYTIMES
bool updateSfpWanLinkStatus(const char *);
#endif
void esmdResetWanLinkStatus(int);
void wanConnctionReadyHandle(char *, uint8_t clientEid, uint32_t clientPid);
void wanConnctionLostHandle(char *, uint8_t clientEid, uint32_t clientPid);
//INTERNET_LED_CONTROL
void wanInternetLEDUpdate(void);
int getIPv6DefaultGatwayStatus(char *, int outputDevNameSize);
int getIPv4DefaultGatwayStatus(char *, int outputDevNameSize);
#ifdef CONFIG_ZCFG_BE_MODULE_NAT_PORT_CTRL
#define PCP_MAX_DELAY 120
void pcpRuleUpdate(objIndex_t *, const char *msg);
void PCP_Timer(int , int );
void PCP_sigHandle(int);
void pcpRuleRenew(void);
#endif
bool isVcAutoHuntEnable(void);
bool isVlanAutoHuntEnable(void);
void *periodic_send_log_thread();

//bool isEthLinkUp;
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
bool getMatchedIpIface(int, char *, rdm_IpIface_t **, objIndex_t *);
void Set_VlanTagRule(bool);
void disableIndicateWAN(char *);
#endif

#ifdef HAAAA_CUSTOMIZATION
#define MAX_CHECK_TIMES 10
#define MAX_CHECK_DELAY 2
bool isAtmLinkUp;
bool isPtmLinkUp;
int oldWanServiceID;
int checkInternetLinkUpNum;
bool wanUnderHunting;

bool disableDefaultGatewayWAN(char *linkType);
void activeDefaultWAN(void);
bool checkDefaultWANisUp(void);

#endif

#endif
