#ifndef _ZYWLAN_COMM_H
#define _ZYWLAN_COMM_H

#define MAC_ADDR_LEN 6
typedef struct zyWlanSsid_s {
	bool varEnable;
	bool mainSsid;
	char ssidObjPath[36];
	char apObjPath[36];
#ifdef ZYXEL_OPAL_EXTENDER
	char epObjPath[36];
	bool ulConnected;
#endif
	bool enable;
	bool __enable;
	char ifName[65];
	char ssid[33];
#ifdef ZYXEL_EASYMESH_R2
	uint16_t bss_vlanid;  // TS VLANID
	uint8_t bridge_idx;   // TS Bridge control
#endif
	bool hideSsid;
	bool isolation;
	bool bssIsolation;
	bool wmfEnable;
	bool wmmSupported;
	bool wmmEnable;
	bool wmmNoAck;
	bool wmmApsd;
	uint32_t maxAssoc;
	uint32_t ssid_prio;
	char bssMacAddr[18];
	char brIfName[32];
	/*Security*/
	bool preauth;
	char authMode[16];
	char encryption[16];
	uint8_t wepkeyid;
	char wepkey[4][32];
	char psk[65];
	char keyPass[64];
	uint32_t rekeyInterval;
	uint32_t reauthInterval;
	char radiusIp[46];
	uint32_t radiusPort;
	char radiusKey[256];
#ifdef ZYXEL_SUPPORT_8021X
	char secondRadiusIp[46];
	uint32_t secondRadiusPort;
	char secondRadiusKey[256];
#endif
	char macFltMode[10];
	char macFltList[601];
	/*WSC*/
	bool wpsEnable;
	bool wpsConfigured;
	bool version2;
	char WPS_PairingID[256];
	char WPS_PairingID_Mode[12];
#ifdef ZYXEL_INDEPENDENT_WPS_METHODS
	bool enablePBC;
	bool enableStaPin;
	bool enableAPPin;
#endif
//	char wpsConf[12]; 
	unsigned int wpsConfMethod;
	char wpsDevPin[9];
//	char wpsStaPin[12];
	char wpsUuid[40];
	char wpsDevName[61]; //modelName
	char wpsManufacturer[65];
	char wpsSerialNumber[65];
	uint32_t upRate;
	uint32_t downRate;
	uint32_t vlanId;
	// pmf method
	// 0 = disable
	// 1 = capable
	// 2 = req
	unsigned int pmfMethod;
#ifdef PRESET_BACKHAUL_SETTING
	uint8_t attribute;
#endif
#ifdef ZYXEL_WLAN_PARTIAL_RELOAD_ENHANCE
	void *partial_list;
#endif

}zyWlanSsid_t;

typedef struct zyWlanRadio_s {
	char objPath[36];
	int8_t radioIdx;
	bool enable;
	bool __enable;
	bool autoChSelect;
	bool apCoex;
	bool frameburst;
	bool atf;
	bool acs_fcs_mode;
	bool ssid_prio_en;
	int8_t obss_coex;
	int8_t protection;
	int8_t nProtection;
	int8_t preamble;
	int8_t htExtCha;
	int8_t rifsAdv;
	int8_t wifiMode;
	uint8_t htBw;
	uint8_t txPwrPcnt;
	uint8_t txbf_bfr;
	uint8_t txbf_bfe;
	uint16_t channel;
	uint32_t autoChPeriod;
	uint32_t autoChPolicy;
	uint32_t bcnPeriod;
	uint32_t dtimPeriod;
	uint32_t ctsRtsThrshld;
	uint32_t fragThrshld;
	uint16_t ssidNum;
	int rxChainPwrSaveEnable;
	int rxChainPwrSaveQuietTime;
	int rxChainPwrSavePps;
	int radioPwrSaveEnable;
	int radioPwrSaveQuietTime;
	int radioPwrSavePps;
	int radioPwrSaveLevel;
	int band;
	int rate;
	int mcs;
	int mrate;
	int RegMode;
	int DfsPreIsm;
	int DfsPostIsm;
	int TpcDb;
	int lazyWds;
	int interferenceMode;
	char acs_autoch_skip[129];
	char acs_manual_skip[129];
	char wlMode[12];
	char wdsMacList[80];
	char phyType[5];
	char country[11];
	char ifName[65];
	char status[18];
	unsigned char macaddr[MAC_ADDR_LEN];
	unsigned long GPIOOverlays;
	char __inactive_channels[129];
	char inactive_channels[129];
	bool probresp_rw;
	char singleSKU[9];
#ifdef ZYXEL_WLAN_PARTIAL_RELOAD_ENHANCE
	void *partial_list;
#endif
	bool dfsTxPwrSameBand1;
}zyWlanRadio_t;

#ifdef ZYXEL_EASYMESH
typedef struct zyWlanEasyMesh_s {
	bool enable;
	bool controller; /* set to true if there is controller*/
	bool configured; /*Auto config done or not*/
#ifdef TC_SDK_7_3_251_900
		int role;
#endif
#ifdef ZYXEL_ECONET_DUAL_BAND_BACKHAUL
	int band; /*Backhaul band*/
#endif
#ifdef ZYXEL_EASYMESH_R2
	char ts_policy_list[128];
	uint16_t pvid;
	uint8_t pcp;
	bool TSenable; /* RDM status */
	bool TSactive; /* if no guest bss enable, TS is not active */
#endif
}zyWlanEasyMesh_t;

typedef struct zyWlanExtBssInfo_s {
	uint8_t bandIndex;
	uint8_t attribute;
	uint16_t MDID;
	uint16_t TCI;
}zyWlanExtBssInfo_t;

typedef struct zyWlanSteerLogic_s {
	bool enable;
	bool band_steer;
}zyWlanSteerLogic_t;
#endif

#ifdef ZYXEL_OPAL_EXTENDER
typedef struct zyWlanExtender_s {
	char mode[16];
}zyWlanExtender_t;
#endif

enum {
	CTLSB_NONE=0,
	CTLSB_LOWER,
	CTLSB_UPPER,
	CTLSB_80_LL,
	CTLSB_80_LU,
	CTLSB_80_UL,
	CTLSB_80_UU,
	CTLSB_160_LLL,
	CTLSB_160_LLU,
	CTLSB_160_LUL,
	CTLSB_160_LUU,
	CTLSB_160_ULL,
	CTLSB_160_ULU,
	CTLSB_160_UUL,
	CTLSB_160_UUU
};

// for Guest wifi setting
#define CHAIN_EBT_BR_GVL "GUEST_VLAN_BROUTE"
#define CHAIN_EBT_BR_GWL "GUEST_AP_BROUTE"
#define CHAIN_EBT_FW_GWL "GUEST_AP_FORWARD"
#define CHAIN_EBT_FW_GWL_WAN "GUEST_AP_FORWARD_WAN"
#define CHAIN_EBT_BR_WAN "WAN_BROUTE"

#define WLAN_RADIO_NUM 1
#define WLAN_MAX_BSS ((TOTAL_INTERFACE_24G < TOTAL_INTERFACE_5G)? TOTAL_INTERFACE_5G : TOTAL_INTERFACE_24G) //Ron, def in project.defconfig

#define WL_PHYTYPE_A      0
#define WL_PHYTYPE_B      1
#define WL_PHYTYPE_G      2
#define WL_PHYTYPE_N      4
#define WL_PHYTYPE_LP     5
#define WL_PHYTYPE_SSN    6
#define WL_PHYTYPE_HT     7
#define WL_PHYTYPE_LCN    8
#define WL_PHYTYPE_LCN40  10
#define WL_PHYTYPE_AC     11

#define WL_PHY_TYPE_A    "a"
#define WL_PHY_TYPE_B    "b"
#define WL_PHY_TYPE_G    "g"
#define WL_PHY_TYPE_N    "n"
#define WL_PHY_TYPE_LP   "l"
#define WL_PHY_TYPE_H    "h"
#define WL_PHY_TYPE_AC   "v"

/* authentication mode */
#define WL_AUTH_OPEN                    "open"
#define WL_AUTH_SHARED                  "shared"
#define WL_AUTH_RADIUS                  "radius"
#define WL_AUTH_WPA                     "wpa"
#define WL_AUTH_WPA_PSK                 "psk"
#define WL_AUTH_WPA2                    "wpa2"
#define WL_AUTH_WPA2_PSK                "psk2"
#define WL_AUTH_WPA2_MIX                "wpa wpa2"
#define WL_AUTH_WPA2_PSK_MIX            "psk psk2"

#define WL_AUTHOPEN             0
#define WL_AUTH_WPAPSK          1
#define WL_AUTH_WPA2PSK         2
#define WL_AUTH_WPA2PSK_MIX     3
#define WL_AUTH_WPA3WPA2_MIX    4
#define WL_AUTH_WPA3SAE         5

#define REG_MODE_OFF             0 /* disabled 11h/d mode */
#define REG_MODE_H               1 /* use 11h mode */
#define REG_MODE_D               2 /* use 11d mode */

#define REG_MODE_OFF_STR			"off"
#define REG_MODE_H_STR			"h"
#define REG_MODE_D_STR			"d"

#define BAND_A                  1
#define BAND_B                  2

/* mimo */
#define WL_CTL_SB_LOWER         -1
#define WL_CTL_SB_NONE           0
#define WL_CTL_SB_UPPER          1

/* Config methods */
#define WPS_CONFMET_USBA            0x0001	/* Deprecated in WSC 2.0 */
#define WPS_CONFMET_ETHERNET        0x0002	/* Deprecated in WSC 2.0 */
#define WPS_CONFMET_LABEL           0x0004
#define WPS_CONFMET_DISPLAY         0x0008
#define WPS_CONFMET_EXT_NFC_TOK     0x0010
#define WPS_CONFMET_INT_NFC_TOK     0x0020
#define WPS_CONFMET_NFC_INTF        0x0040
#define WPS_CONFMET_PBC             0x0080
#define WPS_CONFMET_KEYPAD          0x0100
/* WSC 2.0 */
#define WPS_CONFMET_VIRT_PBC        0x0280
#define WPS_CONFMET_PHY_PBC         0x0480
#define WPS_CONFMET_VIRT_DISPLAY    0x2008
#define WPS_CONFMET_PHY_DISPLAY     0x4008

#if defined(ONECONNECT)
#define ZYWLSSID_FLAGS_AUTOCONFIG	(1<<0)
#endif

#define MAX_SUPPORT_RADIO			2

typedef enum {
	ZY_WPS_INIT = 0,
	ZY_WPS_ASSOCIATED,
	ZY_WPS_OK,
	ZY_WPS_MSG_ERR,
	ZY_WPS_TIMEOUT,
	ZY_WPS_SENDM2,
	ZY_WPS_SENDM7,
	ZY_WPS_MSGDONE,
	ZY_WPS_PBCOVERLAP,
	ZY_WPS_FIND_PBC_AP,
	ZY_WPS_ASSOCIATING,
	ZY_WPS_NOT_USED
} ZYWIFID_WPS_SCSTATE;

typedef enum {
	ZY_MODE_80211_G=0,
	ZY_MODE_80211_B,
	ZY_MODE_80211_BG,
	ZY_MODE_80211_N,
	ZY_MODE_80211_BGN,
	ZY_MODE_80211_GN,
	ZY_MODE_80211_BGNAX,
	ZY_MODE_80211_A,
	ZY_MODE_80211_AN,
	ZY_MODE_80211_AC,
	ZY_MODE_80211_NAC,
	ZY_MODE_80211_ANAC,
	ZY_MODE_80211_ANACAX,
	ZY_MODE_80211_NONE,
} ZYWIFID_80211_MODE;

// protection management frame
enum {
	ZY_PMF_DISABLE=0,
	ZY_PMF_CAPABLE,
	ZY_PMF_REQ
};

enum {
	HT_20_1SS = 0,
	HT_20_2SS,
	HT_20_3SS,
	HT_20_4SS,
	HT_40_1SS,
	HT_40_2SS,
	HT_40_3SS,
	HT_40_4SS,
	VHT_20_1SS,
	VHT_20_2SS,
	VHT_20_3SS,
	VHT_20_4SS,
	VHT_40_1SS,
	VHT_40_2SS,
	VHT_40_3SS,
	VHT_40_4SS,
	VHT_80_1SS,
	VHT_80_2SS,
	VHT_80_3SS,
	VHT_80_4SS,
};

/*802.11n, bw = 20~40, antenna_number = 1~4*/
#define HT20_1SS_RATES "7.2,14.4,21.7,28.9,43.3,57.8,65,72.2"     //bw = 20 with 1 antenna
#define HT20_2SS_RATES "7.2,14.4,21.7,28.9,43.3,57.8,65,72.2,86.7,115.6,130,144.4"//+HT20_1SS_RATES
#define HT20_3SS_RATES "7.2,14.4,21.7,28.9,43.3,57.8,65,72.2,86.7,115.6,130,144.4,173.3,195,216.7" //+HT20_1SS_RATES+HT20_2SS_RATES
#define HT20_4SS_RATES "7.2,14.4,21.7,28.9,43.3,57.8,65,72.2,86.7,\
115.6,130,144.4,173.3,195,216.7,231.1,260,288.9"

#define HT40_1SS_RATES "15,30,45,60,90,120,135,150"
#define HT40_2SS_RATES "15,30,45,60,90,120,135,150,180,240,270,300"
#define HT40_3SS_RATES "15,30,45,60,90,120,135,150,180,240,270,300,360,405,450"
#define HT40_4SS_RATES "15,30,45,60,90,120,135,150,180,240,270,300,\
360,405,450,480,540,600"

/*802.11ac, bw = 20~160, antenna_number = 1~8*/
#define VHT20_1SS_RATES "7.2,14.4,21.7,28.9,43.3,57.8,65,72.2,86.7"
#define VHT20_2SS_RATES "115.6,130,144.4,173.3"                                                     //above include 14.4,28.9,43.3,57.8,86.7
#define VHT20_3SS_RATES "195,216.7,260,288.9"                                                       //above include 21.7,43.3,65,86.7,130,173.3
#define VHT20_4SS_RATES "231.1,346.7"                                                               //above include 28.9,57.8,86.7,115.6,173.3,260,288.9
#define VHT20_5SS_RATES "36.1,108.3,325,361.1,433.3"                                                //above include 72.2,144.4,216.7,288.9
#define VHT20_6SS_RATES "390,520,577.8"                                                             //above include 43.3,86.7,130,173.3,260,346.7,433.3
#define VHT20_7SS_RATES "50.6,101.1,151.7,202.2,303.3,404.4,455,505.6,606.7"
#define VHT20_8SS_RATES "462.2,693.3"                                                               //above include 57.8,115.6,173.3,231.1,346.7,520,577.8,

#define VHT40_1SS_RATES "15,30,45,60,90,120,135,150,180,200"
#define VHT40_2SS_RATES "240,270,300,360,400"                                                       //above include 30,60,90,120,180
#define VHT40_3SS_RATES "405,450,540,600"                                                           //above include 45,90,135,180,270,360
#define VHT40_4SS_RATES "480,720,800"                                                               //above include 60,120,180,240,360,540,600
#define VHT40_5SS_RATES "75,225,675,750,900,1000"                                                   //above include 150,300,450,600,
#define VHT40_6SS_RATES "810,1080,1200"                                                             //above include 90,180,270,360,540,720,900
#define VHT40_7SS_RATES "105,210,315,420,630,840,945,1050,1260,1400"
#define VHT40_8SS_RATES "960,1440,1600"                                                             //above include 120,240,360,480,720,1080,1200

#define VHT80_1SS_RATES "32.5,65,97.5,130,195,260,292.5,325,390,433.3"
#define VHT80_2SS_RATES "520,585,650,780,866.7"                                                     //above include 65,130,195,260,390
#define VHT80_3SS_RATES "975,1170,1300"                                                             //above include 97.5,195,292.5,390,585,780
#define VHT80_4SS_RATES "1040,1560,1733.3"                                                          //above include 130,260,390,520,780,1170,1300
#define VHT80_5SS_RATES "162.5,487.5,1462.5,1625,1950,2166.7"                                       //above include 325,650,975,1300
#define VHT80_6SS_RATES "1755,2340"                                                                 //above include 195,390,585,780,1170,1560,1950
#define VHT80_7SS_RATES "227.5,455,682.5,910,1365,1820,2275,2730,3033.3"
#define VHT80_8SS_RATES "2080,2600,3120,3466.7"                                                     //above include 260,520,780,1040,1560,2340,

#define VHT160_1SS_RATES "65,130,195,260,390,520,585,650,780,866.7"
#define VHT160_2SS_RATES "1040,1170,1300,1560,1733.3"                                               //above include 130,260,390,520,780
#define VHT160_3SS_RATES "1755,1950,2340"                                                           //above include 195,390,585,780,1170,1560
#define VHT160_4SS_RATES "2080,2600,3120,3466.7"                                                    //above include 260,520,780,1040,1560,2340
#define VHT160_5SS_RATES "325,975,2925,3250,3900,4333.3"                                            //above include 650,1300,1950,2600
#define VHT160_6SS_RATES "3510,4680,5200"                                                           //above include 390,780,1170,1560,2340,3120,3900
#define VHT160_7SS_RATES "455,910,1365,1820,2730,3640,4095,4550,5460,6066.7"
#define VHT160_8SS_RATES "4160,6240,6933.3"                                                         //above include 520,1040,1560,2080,3120,4680,5200

/*802.11ax, bw = 20~160, antenna_number = 1~8*/

#define HE20_1SS_RATES "8.6,17.2,25.8,34.4,51.6,68.8,77.4,86,103.2,114.7,129,143.4"
#define HE20_2SS_RATES "137.6,154.9,172.1,206.5,229.4,258.1,286.8"                                  //above include 17.2,34.4,51.6,68.8,103.2
#define HE20_3SS_RATES "232.3,309.7,344.1,387.1,430.1"                                              //above include 25.8,51.6,77.4,103.2,154.9,206.5,258.1,
#define HE20_4SS_RATES "275.3,412.9,458.8,516.2,573.5"                                              //above include 34.4,68.8,103.2,137.6,206.5,309.7,344.1,
#define HE20_5SS_RATES "43,645.2,716.9"                                                             //above include 86,129,172.1,258.1,344.1,387.1,430.1,516.2,573.5,
#define HE20_6SS_RATES "464.6,619.4,688.2,774.3,860.3"                                              //above include 51.6,103.2,154.9,206.5,309.7,412.9,516.2
#define HE20_7SS_RATES "60.2,120.4,180.7,240.9,361.3,481.8,542,602.2,722.6,802.9,903.3,1003.7"
#define HE20_8SS_RATES "550.6,825.9,917.6,1032.4,1147.1"                                            //above include 68.8,137.6,206.5,275.3,412.9,619.4,688.2,

#define HE40_1SS_RATES "17.2,34.4,51.6,68.8,103.2,137.6,154.9,172.1,206.5,229.4,258.1,286.8"
#define HE40_2SS_RATES "275.3,309.7,344.1,412.9,458.8,516.2,573.5"                                  //above include 34.4,68.8,103.2,137.6,206.5
#define HE40_3SS_RATES "464.6,619.4,688.2,774.3,860.3"                                              //above include 51.6,103.2,154.9,206.5,309.7,412.9,516.2,
#define HE40_4SS_RATES "550.6,825.9,917.6,1032.4,1147.1"                                            //above include 68.8,137.6,206.5,275.3,412.9,619.4,688.2
#define HE40_5SS_RATES "86,1290.4,1433.8"                                                           //above include 172.1,258.1,344.1,516.2,688.2,774.3,860.3,1032.4,1147.1
#define HE40_6SS_RATES "929.1,1238.8,1376.5,1548.5,1720.6"                                          //above include 103.2,206.5,309.7,412.9,619.4,825.9,1032.4
#define HE40_7SS_RATES "120.4,240.9,361.3,481.8,722.6,963.5,1084,1204.4,1445.3,1605.9,1806.6,2007.4"
#define HE40_8SS_RATES "1101.2,1651.8,1835.3,2064.7,2294.1"                                         //above include 137.6,275.3,412.9,550.6,825.9,1238.8,1376.5

#define HE80_1SS_RATES "36,72.1,108.1,144.1,216.2,288.2,324.3,360.3,432.4,480.4,540.4,600.5"
#define HE80_2SS_RATES "576.5,648.5,720.6,864.7,960.8,1080.9,1201"                                  //above include 72.1,144.1,216.2,288.2,432.4
#define HE80_3SS_RATES "972.8,1297.1,1441.2,1621.3,1801.5"                                          //above include 108.1,216.2,324.3,432.4,648.5,864.7,1080.9
#define HE80_4SS_RATES "1152.9,1729.4,1921.6,2161.8,2402"                                           //above include 144.1,288.2,432.4,576.5,864.7,1297.1,1441.2,
#define HE80_5SS_RATES "180.1,2702.2,3002.5"                                                        //above include 360.3,540.4,720.6,1080.9,1441.2,1621.3,1801.5,2161.8,2402
#define HE80_6SS_RATES "1945.6,2594.1,2882.4,3242.6,3602.9"                                         //above include 216.2,432.4,648.5,864.7,1297.1,1729.4,2161.8
#define HE80_7SS_RATES "252.2,504.4,756.6,1008.8,1513.2,2017.6,2269.9,2522.1,3026.5,3362.7,3783.1,4203.4"
#define HE80_8SS_RATES "2305.9,3458.8,3843.1,4323.5,4803.9"                                         //above include 288.2,576.5,864.7,1152.9,1729.4,2594.1,2882.4

#define HE160_1SS_RATES "72.1,144.1,216.2,288.2,432.4,576.5,648.5,720.6,864.7,960.8,1080.9,1201"
#define HE160_2SS_RATES "1152.9,1297.1,1441.2,1729.4,1921.6,2161.8,2402"                            //above include 144.1,288.2.432.4,576.5,864.7
#define HE160_3SS_RATES "1945.6,2594.1,2882.4,3242.6,3602.9"                                        //above include 216.2,432.4,648.5,864.7,1297.1,1729.4,2161.8
#define HE160_4SS_RATES "2305.9,3458.8,3843.1,4323.5,4803.9"                                        //above include 288.2,576.5,864.7,1152.9,1729.4,2594.1,2882.4
#define HE160_5SS_RATES "360.3,5404.4,6004.9"                                                       //above include 720.6,1080.9,1441.2,2161.8,2882.4,3242.6,3602.9,4323.5,4803.9
#define HE160_6SS_RATES "3891.2,5188.2,5764.7,6485.3,7205.9"                                        //above include 432.4,864.7,1297.1,1729.4,2594.1,3458.8,4323.5
#define HE160_7SS_RATES "504.4,1008.8,1513.2,2017.6,3026.5,4035.3,4539.7,5044.1,6052.9,6725.5,7566.2,8406.9"
#define HE160_8SS_RATES "4611.8,6917.6,7686.3,8647.1,9607.8"                                        //above include 576.5,1152.9,1729.4,2305.9,3458.8,5188.2,5764.7


/*802.11b, 802.11g, 802.11a, BasicDataTxRates for 2.4GHz, 5GHz*/
#define B2G_RATES "1,2,5.5,11"//Basic
#define WL_B_RATES "1,2,5.5,11"
#define WL_G_RATES "1,2,5.5,6,9,11,12,18,24,36,48,54"
#define B5G_RATES "6,12,24"//Basic
#define WL_A_RATES "6,9,12,18,24,36,48,54"

/* MaxPhyRate for 802.11 ac & ax */
#define MAX_RATE_HT_BW20_NSS1 72.2
#define MAX_RATE_HT_BW20_NSS2 144.4
#define MAX_RATE_HT_BW20_NSS3 216.7
#define MAX_RATE_HT_BW20_NSS4 288.9

#define MAX_RATE_HT_BW40_NSS1 150
#define MAX_RATE_HT_BW40_NSS2 300
#define MAX_RATE_HT_BW40_NSS3 450
#define MAX_RATE_HT_BW40_NSS4 600

#define MAX_RATE_VHT_BW20_NSS1 86.7
#define MAX_RATE_VHT_BW20_NSS2 173.3
#define MAX_RATE_VHT_BW20_NSS3 288.9
#define MAX_RATE_VHT_BW20_NSS4 346.7
#define MAX_RATE_VHT_BW20_NSS5 433.3
#define MAX_RATE_VHT_BW20_NSS6 577.8
#define MAX_RATE_VHT_BW20_NSS7 606.7
#define MAX_RATE_VHT_BW20_NSS8 693.3

#define MAX_RATE_VHT_BW40_NSS1 200
#define MAX_RATE_VHT_BW40_NSS2 400
#define MAX_RATE_VHT_BW40_NSS3 600
#define MAX_RATE_VHT_BW40_NSS4 800
#define MAX_RATE_VHT_BW40_NSS5 1000
#define MAX_RATE_VHT_BW40_NSS6 1200
#define MAX_RATE_VHT_BW40_NSS7 1400
#define MAX_RATE_VHT_BW40_NSS8 1600

#define MAX_RATE_VHT_BW80_NSS1 433.3
#define MAX_RATE_VHT_BW80_NSS2 866.7
#define MAX_RATE_VHT_BW80_NSS3 1300
#define MAX_RATE_VHT_BW80_NSS4 1733.3
#define MAX_RATE_VHT_BW80_NSS5 2166.7
#define MAX_RATE_VHT_BW80_NSS6 2340
#define MAX_RATE_VHT_BW80_NSS7 3033.3
#define MAX_RATE_VHT_BW80_NSS8 3466.7

#define MAX_RATE_VHT_BW160_NSS1 866.7
#define MAX_RATE_VHT_BW160_NSS2 1733.3
#define MAX_RATE_VHT_BW160_NSS3 2340
#define MAX_RATE_VHT_BW160_NSS4 3466.7
#define MAX_RATE_VHT_BW160_NSS5 4333.3
#define MAX_RATE_VHT_BW160_NSS6 5200
#define MAX_RATE_VHT_BW160_NSS7 6066.7
#define MAX_RATE_VHT_BW160_NSS8 6933.3

#define MAX_RATE_HE_BW20_NSS1 143.4
#define MAX_RATE_HE_BW20_NSS2 286.8
#define MAX_RATE_HE_BW20_NSS3 430.1
#define MAX_RATE_HE_BW20_NSS4 573.5
#define MAX_RATE_HE_BW20_NSS5 716.9
#define MAX_RATE_HE_BW20_NSS6 860.3
#define MAX_RATE_HE_BW20_NSS7 1003.7
#define MAX_RATE_HE_BW20_NSS8 1147.1

#define MAX_RATE_HE_BW40_NSS1 286.8
#define MAX_RATE_HE_BW40_NSS2 573.5
#define MAX_RATE_HE_BW40_NSS3 860.3
#define MAX_RATE_HE_BW40_NSS4 1147.1
#define MAX_RATE_HE_BW40_NSS5 1433.8
#define MAX_RATE_HE_BW40_NSS6 1720.6
#define MAX_RATE_HE_BW40_NSS7 2007.4
#define MAX_RATE_HE_BW40_NSS8 2294.1

#define MAX_RATE_HE_BW80_NSS1 600.5
#define MAX_RATE_HE_BW80_NSS2 1201
#define MAX_RATE_HE_BW80_NSS3 1801.5
#define MAX_RATE_HE_BW80_NSS4 2402
#define MAX_RATE_HE_BW80_NSS5 3002.5
#define MAX_RATE_HE_BW80_NSS6 3602.9
#define MAX_RATE_HE_BW80_NSS7 4203.4
#define MAX_RATE_HE_BW80_NSS8 4803.9

#define MAX_RATE_HE_BW160_NSS1 1201
#define MAX_RATE_HE_BW160_NSS2 2402
#define MAX_RATE_HE_BW160_NSS3 3602.9
#define MAX_RATE_HE_BW160_NSS4 4803.9
#define MAX_RATE_HE_BW160_NSS5 6004.9
#define MAX_RATE_HE_BW160_NSS6 7205.9
#define MAX_RATE_HE_BW160_NSS7 8406.9
#define MAX_RATE_HE_BW160_NSS8 9607.8

#endif

#ifdef ZYXEL_EASYMESH_R2
extern bool ts_change; /* global use, TSactive is per band status */
extern char tmp_ts_policy_list[128];
#endif
