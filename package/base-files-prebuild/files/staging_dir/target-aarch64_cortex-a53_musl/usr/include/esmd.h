#ifndef _ESMD_H
#define _ESMD_H
#ifdef IAAAA_CUSTOMIZATION_Motorola_VIP1003
#include <time.h>
#endif

int esmdApplyAddObjectTmTask(const char *, int , int , const char *);
int esmdApplyDeleteObjectTmTask(const char *);

void updateLinkStatus(const char*);
//void updateLanLinkStatus(void);
//void updateWanLinkStatus(void);
void updateBridgeVlanCounter(const char*);
bool checkFcTableExist (const char*);
bool checkFcTableExistSrcMac (const char*);
void split(char**, char*, const char*);
void emailEventHandle(char *buf, uint8_t clientEid, uint32_t clientPid);

#ifdef ZYXEL_SFP_LINKUP_DELAYTIMES
enum {
	sfpLinkUpPrepare = 0,
	sfpLinkupDelay,
	sfpLinkUpContinue
};
#endif

#ifdef ZYXEL_SIP_DELAYTIMES 
enum {
	SipPrepare = 0,
	SipDelayStatus,
	SipContinue
};
#endif

typedef enum {
		isATM,
		isPTM,
        isXTM,
        isEth,
        isMoca,
        isGpon,
        isEpon,
		isWwan,
		isMulti,
		isLteWan,
		none
} wanLinkType;

typedef enum {
        Eth_Ethwan,
        Eth_SFP
} EthwanLinkType;


//INTERNET_LED_CONTROL
typedef enum{
	INTERNET_LED_FAIL = 0,
	INTERNET_LED_DATA
}internet_led_t;

typedef enum{
	INTERNET_LED_STATE_OFF = 0,
	INTERNET_LED_STATE_ON,
#ifdef ZYXEL_LED_ERROR_INDICATION_FOR_100M
	INTERNET_LED_STATE_100M,          
#endif	
	INTERNET_LED_STATE_FAIL,
	INTERNET_LED_STATE_DONGLE,
	INTERNET_LED_STATE_NONE
}internet_led_state_t;

#ifdef IAAAA_CUSTOMIZATION_Motorola_VIP1003
#define NUM_LAN_PORT 4
typedef struct {
	char portName[16];
	time_t time;
} sPortName;

extern sPortName lanPortName[NUM_LAN_PORT];
#endif
#endif

#define JSON_OBJ_COPY(json_object) json_tokener_parse(json_object_to_json_string(json_object))

#ifdef CONFIG_ZYXEL_HOME_CYBER_SECURITY_FSECURE
/* NTP update and notify F-secure */
extern void fsc_ntp_update ( void );
#endif

extern bool wanLinkInfoInit;

#if defined (ZYXEL_ECONET_OPERATION_MODE) && defined (YAAAB_CUSTOMIZATION)
extern int var_OPM; //ZYXEL_ECONET_OPERATION_MODE Value
#define OPERTION_MODE_ROUTER 0
#define OPERTION_MODE_AP 1
//LED PING Thread
extern void ping_thread_create();
extern void * LED_PING_TEST_FUN(void *ptr);
#endif

#ifdef CONFIG_ZYXEL_MULTI_WAN
/* New Multiple WAN operation */

#else //C0NFIG_ZYXEL_MULTI_WAN
/* Original Multiple WAN operation */
extern wanLinkType currWanLinkType;
extern wanLinkType currWanLinkType;
extern wanLinkType WANPriority;
extern EthwanLinkType currEthWanLinkType;
extern EthwanLinkType EthWANPriority;
extern wanLinkType prevXTMLinkType;
extern bool supportSFP;
extern bool isVcAutoHuntRunnig;
extern bool isVlanAutoHuntRunnig;
extern bool isAutoHuntFinished;

extern void esmd_mwan_init( void );
#endif // C0NFIG_ZYXEL_MULTI_WAN
