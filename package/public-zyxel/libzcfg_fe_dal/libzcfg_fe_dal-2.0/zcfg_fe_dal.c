#include "zcfg_common.h"
#include "zcfg_msg.h"
#include <json/json.h>
#include <sys/socket.h>	// for inet_addr()
#include <netinet/in.h>
#include <arpa/inet.h>	// for inet_pton(), inet_addr()
#include <ctype.h>
#include <regex.h>
#include "zcfg_rdm_oid.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_dal.h"
#include "zcfg_fe_rdm_string.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <zos_api.h>
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER)
#include "zcfg_debug.h"
#include <cybergarage/http/chttp_authentication.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#endif
#include "zos_api.h"

//#define isdigit(c) ((c >= 0x30) && (c < 0x3A))	// already define in "ctype.h"
#define isEmptyIns(obj) json_object_get_boolean(json_object_object_get(obj, "emptyIns"))

#define DAL_LOGIN_LEVEL "DAL_LOGIN_LEVEL"
#define DAL_LOGIN_USERNAME "DAL_LOGIN_USERNAME"
#define DAL_LOGIN_REMOTEADDR "DAL_LOGIN_REMOTEADDR"
#define DAL_LOGIN_TYPE "cmdtype"

bool printDebugMsg = false;

struct json_object *guiCustomObj = NULL;


extern dal_param_t WAN_param[];
extern dal_param_t MGMT_param[];
extern dal_param_t ETHWANLAN_param[];
extern dal_param_t ADVANCE_param[];
extern dal_param_t WWANBACKUP_param[];
#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
extern dal_param_t Bandwidth_Improvement_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
extern dal_param_t CELLWAN_param[];
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_MULTI_APN
extern dal_param_t CELLWAN_MAPN_param[];
#endif
extern dal_param_t CELLWAN_SIM_param[];
extern dal_param_t CELLWAN_BAND_param[];
extern dal_param_t CELLWAN_PLMN_param[];
extern dal_param_t CELLWAN_PSRU_param[];
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_SMS
extern dal_param_t CELLWAN_SMS_param[];
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_PCI_LOCK
extern dal_param_t CELLWAN_LOCK_param[];
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_GNSS_LOCATION
extern dal_param_t CELLWAN_GNSS_param[];
#endif
#ifdef ZYXEL_WEB_GUI_SUPPORT_SAS_CBSD
extern dal_param_t CBSD_param[];
#endif
extern dal_param_t CELLWAN_MGMT_param[];
extern dal_param_t CELLWAN_STATUS_param[];
extern dal_param_t CELLWAN_WAIT_STATE_param[];
#endif
extern dal_param_t ETH_CTL_param[];
extern dal_param_t DNS_ENTRY_param[];
extern dal_param_t D_DNS_param[];
extern dal_param_t STATICROUTE_param[];
extern dal_param_t DNSROUTE_param[];
extern dal_param_t POLICYROUTE_param[];
extern dal_param_t RIP_param[];
extern dal_param_t NatPortFwd_param[];
extern dal_param_t NAT_PORT_TRIGGERING_param[];
extern dal_param_t NAT_ADDR_MAPPING_param[];
extern dal_param_t NAT_CONF_param[];
extern dal_param_t NAT_Application_param[];
extern dal_param_t IntfGrp_param[];
#ifdef CONFIG_ZCFG_BE_MODULE_GRE_TUNNEL
extern dal_param_t GreTunnel_param[];
#endif
extern dal_param_t LANADV_param[];

extern dal_param_t LANSETUP_param[];

extern dal_param_t SUBNET_param[];
extern dal_param_t STATIC_DHCP_param[];
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE
extern dal_param_t APAS_param[];
extern dal_param_t APASMacRule_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
extern dal_param_t VLANGRP_param[];
#endif
extern dal_param_t IGMPMLD_param[];
extern dal_param_t FIREWALL_param[];
extern dal_param_t FIREWALL_PROTOCOL_param[];
extern dal_param_t FIREWALL_ACL_param[];
extern dal_param_t WIFI_GENERAL_param[];
extern dal_param_t MAC_Filter_param[];
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
extern dal_param_t IP_Filter_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
extern dal_param_t QosGeneral_param[];
extern dal_param_t QosQueue_param[];
extern dal_param_t QosClass_param[];
extern dal_param_t QosShaper_param[];
extern dal_param_t QosPolicer_param[];
#endif
extern dal_param_t WIFI_MACFILTER_param[];
extern dal_param_t WIFI_WPS_param[];
extern dal_param_t WIFI_WMM_param[];
#ifdef ZYXEL_WIFI_SUPPORT_WDS
extern dal_param_t WIFI_WDS_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_END_POINT
extern dal_param_t WIFI_OperatingModes_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_OPERATION_MODE
extern dal_param_t WIFI_Opm_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
extern dal_param_t WIFI_EASYMESH_param[];
#endif
extern dal_param_t WIFI_OTHERS_param[];
extern dal_param_t WLAN_SCHEDULER_param[];
extern dal_param_t WLAN_SCH_ACCESS_param[];
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_WIFI_CHANNEL_HOPPING)
extern dal_param_t WLAN_CHANNELHOPPING_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
extern dal_param_t SNMP_param[];
#endif
extern dal_param_t TIME_param[];
extern dal_param_t TRUST_DOMAIN_param[];
extern dal_param_t SP_TRUST_DOMAIN_param[];
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PPTP_SUPPORT
extern dal_param_t PPTP_param[];
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_VPN_CLIENT
extern dal_param_t VPNClient_param[];
#endif
#if BUILD_PACKAGE_ZyIMS
extern dal_param_t VOIP_LINE_param[];
extern dal_param_t VOIP_PROFILE_param[];
extern dal_param_t VOIP_TLS_param[];
extern dal_param_t VOIP_REGION_param[];
extern dal_param_t VOIP_PHONEDEVICE_param[];
extern dal_param_t VOIP_PHONEDECT_param[];
extern dal_param_t VOIP_CALLRULE_param[];
extern dal_param_t VOIP_CALLBLOCK_param[];
extern dal_param_t VOIP_CALLHISTORY_param[];
extern dal_param_t VOIP_CALLSUMMARY_param[];
#endif
extern dal_param_t MAIL_NOTIFICATION_param[];
extern dal_param_t LOG_SETTING_param[];
#ifdef CONFIG_ZCFG_BE_MODULE_ZLOG_USE_DEBUG
extern dal_param_t ZLOG_SETTING_param[];
#endif
extern dal_param_t PING_TEST_param[];
extern dal_param_t IEEE_8021AG_param[];
extern dal_param_t IEEE_8023AH_param[];
extern dal_param_t OAMPING_param[];
extern dal_param_t USB_FILESHARING_param[];
extern dal_param_t USB_SAMBA_param[];
extern dal_param_t USB_MEDIASERVER_param[];
extern dal_param_t USB_PRINTSERVER_param[];
extern dal_param_t PAREN_CTL_param[];
extern dal_param_t CYBER_SECURE_param[];
extern dal_param_t CONTENT_FILTER_param[];
extern dal_param_t URL_FILTER_param[];
extern dal_param_t URLWhite_FILTER_param[];
extern dal_param_t URLBlack_FILTER_param[];
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LICENSE
extern dal_param_t ZyLicense_param[];
#endif
extern dal_param_t ChangeIconName_param[];
extern dal_param_t SCHEDULE_param[];
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
extern dal_param_t Package_3rdParty_param[];
#endif
extern dal_param_t DEV_SEC_CERT_param[];
extern dal_param_t HOME_CONNECTIVYITY_param[];
extern dal_param_t OUI_param[];
extern dal_param_t MGMT_SRV_param[];
extern dal_param_t SP_MGMT_SRV_param[];
extern dal_param_t LOGIN_PRIVILEGE_param[];
extern dal_param_t QUICK_START_param[];
#ifdef ABUU_CUSTOMIZATION
extern dal_param_t LOG_CFG_param[];
#endif
extern dal_param_t USER_ACCOUNT_param[];
#ifdef CONFIG_ZYXEL_TR140
extern dal_param_t TTSAMBA_ACCOUNT_param[];
#endif
extern dal_param_t NETWORK_MAP_param[];
extern dal_param_t SYS_LOG_param[];
extern dal_param_t Traffic_Status_param[];
extern dal_param_t VOIP_Status_param[];
//#ifdef CAAAB_CUSTOMIZATION // 20170711 Max Add
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
extern dal_param_t WIFI_CAAAB_param[];
extern dal_param_t WIFI_ADV_CAAAB_param[];
#endif
extern dal_param_t PORT_MIRROR_param[];
#ifdef CONFIG_TAAAB_PORTMIRROR
extern dal_param_t TAAAB_PORT_MIRROR_param[];
#endif
#ifdef CONFIG_TAAAB_PACKETCAPTURE
extern dal_param_t PacketCapture_param[];
#endif
extern dal_param_t SENSOR_param[];
#ifdef CONFIG_ZCFG_BE_MODULE_MOS_USER_CONFIG
extern dal_param_t GPON_param[];
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
extern dal_param_t EasyMenuWifi_param[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
extern dal_param_t SFP_MGMT_param[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
#ifdef ZYXEL_WEB_GUI_SHOW_ZYEE /* JoannaSu, 20191115, support ZyEE configuration */
extern dal_param_t ZYEE_DU_param[];
extern dal_param_t ZYEE_EU_param[];
#endif
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_DSL_TROUBLE_SHOOTING
extern DSLDiagd_param[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_REBOOT_SCHEDULE
extern dal_param_t REBOOT_SCHEDULE_param[];
#endif

#ifdef ZYXEL_TAAAG_EDNS
extern dal_param_t EDNS_param[];
#endif

#ifdef ZYXEL_VPNLITE
extern dal_param_t VPNLite_param[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_NAT_PORT_CTRL
extern dal_param_t NAT_PCP_param[];
extern zcfgRet_t zcfgFeDalNatPcp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif

extern zcfgRet_t zcfgFeDalWan(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalDnsEntry(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalDDns(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalEthWanLan(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
extern zcfgRet_t zcfgFeDalWanAdvance(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
extern zcfgRet_t zcfgFeDalWwanBackup(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_BANDWIDTH_IMPROVEMENT
extern zcfgRet_t zcfgFeDalBandwidthImprovement(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
extern zcfgRet_t zcfgFeDalCellWan(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_MULTI_APN
extern zcfgRet_t zcfgFeDalCellWanMApn(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalCellWanSim(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCellWanBand(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCellWanPlmn(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCellWanPsru(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_SMS
extern zcfgRet_t zcfgFeDalCellWanSms(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_PCI_LOCK
extern zcfgRet_t zcfgFeDalCellWanLock(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_GNSS_LOCATION
extern zcfgRet_t zcfgFeDalCellWanGnss(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef ZYXEL_WEB_GUI_SUPPORT_SAS_CBSD
extern zcfgRet_t zcfgFeDalCbsd(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalCellWanMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCellWanStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCellWanWaitState(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalEthCtl(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalStaticRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalDnsRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalPolicyRoute(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalRip(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalNatPortMapping(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalNatPortTriggering(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalNatConf(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalNatAddrMapping(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalNatApplication(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalIntrGrp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_GRE_TUNNEL
extern zcfgRet_t zcfgFeDalGreTunnel(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalLanAdvance(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalLanSetup(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalAddnlSubnet(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalStaticDHCP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE
extern zcfgRet_t zcfgFeDalAPAS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalAPASMacRule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
extern zcfgRet_t zcfgFeDalVlanGroup(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalIGMPMLD(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalFirewall(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalFirewallProtocol(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalFirewallACL(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
extern zcfgRet_t zcfgFeDalWifiGeneral(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalWifiMACFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalWifiWps(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalWifiWmm(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_WIFI_SUPPORT_WDS
extern zcfgRet_t zcfgFeDalWifiWds(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
extern zcfgRet_t zcfgFeDalWifiEasyMesh(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_END_POINT
extern zcfgRet_t zcfgFeDalOperatingModes(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_OPERATION_MODE
extern zcfgRet_t zcfgFeDalWifiOpm(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WLAN_SCHEDULER
extern zcfgRet_t zcfgFeDalWlanScheduler(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalWlanSchAccess(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif

#if defined(ABZQ_CUSTOMIZATION_SUPPORT_WIFI_CHANNEL_HOPPING)
extern zcfgRet_t zcfgFeDalWlanChannelHopping (const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalWifiOthers(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

extern zcfgRet_t zcfgFeDalMACFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
extern zcfgRet_t zcfgFeDalIPFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalQosGeneral(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalQosQueue(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalQosClass(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalQosShaper(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalQosPolicer(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);

#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
extern zcfgRet_t zcfgFeDalSNMP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalTime(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalTrustDomain(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalSpTrustDomain(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PPTP_SUPPORT
extern zcfgRet_t zcfgFeDalPptp(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_VPN_CLIENT
extern zcfgRet_t zcfgFeDalVPNClient(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#if BUILD_PACKAGE_ZyIMS
extern zcfgRet_t zcfgFeDalVoipLine(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipProfile(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipTLS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipPhoneRegion(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipPhoneDevice(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PHONE_DECT
extern zcfgRet_t zcfgFeDalVoipPhoneDECT(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalVoipCallRule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipCallBlock(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipCallHistory(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalVoipCallSummary(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalMailNotification(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalLogSetting(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalSysLog(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_ZLOG_USE_DEBUG
extern zcfgRet_t zcfgFeDalZlogSetting(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalPingTest(const char *method, struct json_object *Jobj,struct json_object *Jarray,  char *replyMsg);
extern zcfgRet_t zcfgFe8021ag(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFe8023ah(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
extern zcfgRet_t zcfgFeOamPing(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalUsbFilesharing(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
extern zcfgRet_t zcfgFeDalUsbSamba(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
extern zcfgRet_t zcfgFeDalUsbMediaserver(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
extern zcfgRet_t zcfgFeDalUsbPrintserver(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalParentCtl(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
extern zcfgRet_t zcfgFeDalCyberSecure(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalContentFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalChangeIconName(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
extern zcfgRet_t zcfgFeDalURLFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalURLWhiteFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalURLBlackFilter(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZYXEL_ZY_LICENSE_CMD
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LICENSE
extern zcfgRet_t zcfgFeDalZyLicense(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#endif
extern zcfgRet_t zcfgFeDalSchedule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
extern zcfgRet_t zcfgFeDalPackage3rdParty(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalDevSecCert(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalHomeConnectivity(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDaloui(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalMgmtSrv(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalSPMgmtSrv(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalLoginPrivilege(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalQuickStart(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef ABUU_CUSTOMIZATION
extern zcfgRet_t zcfgFeDalLogCfg(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalUserAccount(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZYXEL_TR140
extern zcfgRet_t zcfgFeDalSambaAccount(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalNetworkMAP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalCardPageStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef GFIBER_CUSTOMIZATION
extern zcfgRet_t zcfgFeDalGfiberStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalGfiberCardStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalLanPortInfo(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
//#ifdef CAAAB_CUSTOMIZATION // 20170711 Max Add
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
extern zcfgRet_t zcfgFeDalCbtWifi(const char *method, struct json_object *Jobj, char *replyMsg);
extern zcfgRet_t zcfgFeDalCbtWifiAdv(const char *method, struct json_object *Jobj, char *replyMsg);
#endif
#endif
extern zcfgRet_t zcfgFeDalTrafficStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
extern zcfgRet_t zcfgFeDalVoIPStatus(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalPortMirror(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#ifdef CONFIG_TAAAB_PORTMIRROR
extern zcfgRet_t zcfgFeDalTTPortMirror(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
extern zcfgRet_t zcfgFeDalSensor(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_MOS_USER_CONFIG
extern zcfgRet_t zcfgFeDalGpon(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
extern zcfgRet_t zcfgFeDalPacketCapture(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
extern zcfgRet_t zcfgFeDalSfpMgmt(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
#ifdef ZYXEL_WEB_GUI_SHOW_ZYEE /* JoannaSu, 20191115, support ZyEE configuration */
extern zcfgRet_t zcfgFeDalDUAction(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern zcfgRet_t zcfgFeDalEUAction(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_REBOOT_SCHEDULE
extern zcfgRet_t zcfgFeDalRebootSchedule(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
extern zcfgRet_t zcfgFeDalEasyMenuWifi(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
#endif
#ifdef ZYXEL_VPNLITE
extern zcfgRet_t zcfgFeDalVPNLite(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif
extern zcfgRet_t zcfgFeDalIntrGrpGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);

#ifdef CONFIG_ZCFG_BE_MODULE_DSL_TROUBLE_SHOOTING
extern zcfgRet_t zcfgFeDalDSLDiagd(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif

extern void zcfgFeDalShowStaticRoute(struct json_object *Jarray);
extern void zcfgFeDalShowRip(struct json_object *Jarray);
extern void zcfgFeDalShowMgmtSrv(struct json_object *Jarray);
extern void zcfgFeDalShowSPMgmtSrv(struct json_object *Jarray);
extern void zcfgFeDalShowPortFwd(struct json_object *Jarray);
extern void zcfgFeDalShowPortTrigger(struct json_object *Jarray);
extern void zcfgFeDalShowAddrMapping(struct json_object *Jarray);
extern void zcfgFeDalShowNatConf(struct json_object *Jarray);
extern void zcfgFeDalShowNetworkMAP(struct json_object *Jarray);
extern void zcfgFeDalShowEthCtl(struct json_object *Jarray);
extern void zcfgFeDalShowFirewallACL(struct json_object *Jarray);
extern void zcfgFeDalShowWifiGeneral(struct json_object *Jarray);
extern void zcfgFeDalShowWan(struct json_object *Jarray);
extern void zcfgFeDalShowDDns(struct json_object *Jarray);
extern void zcfgFeDalShowIntrGrp(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_GRE_TUNNEL
extern void zcfgFeDalShowGreTunnel(struct json_object *Jarray);
#endif
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE
extern void zcfgFeDalShowAPAS(struct json_object *Jarray);
extern void zcfgFeDalShowAPASMacRule(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowStaticDHCP(struct json_object *Jarray);
extern void zcfgFeDalShowDnsEntry(struct json_object *Jarray);
extern void zcfgFeDalShowIGMPMLD(struct json_object *Jarray);
extern void zcfgFeDalShowLanAdv(struct json_object *Jarray);
extern void zcfgFeDalShowAddnl(struct json_object *Jarray);
extern void zcfgFeDalShowUsbFilesharing(struct json_object *Jarray);
extern void zcfgFeDalShowUsbSamba(struct json_object *Jarray);
extern void zcfgFeDalShowVlanGroup(struct json_object *Jarray);
extern void zcfgFeDalShowMACFilter(struct json_object *Jarray);
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
extern void zcfgFeDalShowIPFilter(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowFirewall(struct json_object *Jarray);
extern void zcfgFeDalShowHomeConnect(struct json_object *Jarray);
extern void zcfgFeDalShowoui(struct json_object *Jarray);
extern void zcfgFeDalShowWanAdvance(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
extern void zcfgFeDalShowSNMP(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowPolicyRoute(struct json_object *Jarray);
extern void zcfgFeDalShowFirewallDos(struct json_object *Jarray);
extern void zcfgFeDalShowDnsRoute(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_VPN_CLIENT
extern void zcfgFeDalShowVPNClient(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowVoipCallRule(struct json_object *Jarray);
extern void zcfgFeDalShowVoipCallBlock(struct json_object *Jarray);
extern void zcfgFeDalShowVoipRegion(struct json_object *Jarray);
extern void zcfgFeDalShowVoipPhoneDevice(struct json_object *Jarray);
extern void zcfgFeDalShowVoipCallHistory(struct json_object *Jarray);
extern void zcfgFeDalShowVoipCallSummary(struct json_object *Jarray);
extern void zcfgFeDalShowFirewallProtocol(struct json_object *Jarray);
extern void zcfgFeDalShowVoipLine(struct json_object *Jarray);
extern void zcfgFeDalShowVoipProfile(struct json_object *Jarray);
extern void zcfgFeDalShowVoipTLS(struct json_object *Jarray);
extern void zcfgFeDalShowSchedule(struct json_object *Jarray);
extern void zcfgFeDalShowMgmt(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
extern void zcfgFeDalShowUsbMediaserver(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowWifiWps(struct json_object *Jarray);
extern void zcfgFeDalShowUserAccount(struct json_object *Jarray);
extern void zcfgFeDalShowTrustDomain(struct json_object *Jarray);
extern void zcfgFeDalShowSpTrustDomain(struct json_object *Jarray);
extern void zcfgFeDalShowQosGeneral(struct json_object *Jarray);
extern void zcfgFeDalShowQosShaper(struct json_object *Jarray);
extern void zcfgFeDalShowQosQueue(struct json_object *Jarray);
extern void zcfgFeDalShowQosPolicer(struct json_object *Jarray);
extern void zcfgFeDalShowQosClass(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
extern void zcfgFeDalShowWwanBackup(struct json_object *Jarray);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
extern void zcfgFeDalShowCellWan(struct json_object *Jarray);
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_MULTI_APN
extern void zcfgFeDalShowCellWanMApn(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowCellWanSim(struct json_object *Jarray);
extern void zcfgFeDalShowCellWanBand(struct json_object *Jarray);
extern void zcfgFeDalShowCellWanPlmn(struct json_object *Jarray);
extern void zcfgFeDalShowCellWanPsru(struct json_object *Jarray);
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_PCI_LOCK
extern void zcfgFeDalShowCellWanLock(struct json_object *Jarray);
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_GNSS_LOCATION
extern void zcfgFeDalShowCellWanGnss(struct json_object *Jarray);
#endif
#ifdef ZYXEL_WEB_GUI_SUPPORT_SAS_CBSD
extern void zcfgFeDalShowCbsd(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowCellWanMgmt(struct json_object *Jarray);
extern void zcfgFeDalShowCellWanStatus(struct json_object *Jarray);
extern void zcfgFeDalShowCellWanWaitState(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowMailNotification(struct json_object *Jarray);
extern void zcfgFeDalShowLogSetting(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_ZLOG_USE_DEBUG
extern void zcfgFeDalShowZlogSetting(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowWifiWmm(struct json_object *Jarray);
extern void zcfgFeDalShowLanSetup(struct json_object *Jarray);
extern void zcfgFeDalShowParentCtl(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
extern void zcfgFeDalShowCyberSecure(struct json_object *Jarray);
extern void zcfgFeDalShowContentFilter(struct json_object *Jarray);
extern void zcfgFeDalShowChangeIconName(struct json_object *Jarray);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
extern void zcfgFeDalShowURLFilter(struct json_object *Jarray);
extern void zcfgFeDalShowURLWhiteFilter(struct json_object *Jarray);
extern void zcfgFeDalShowURLBlackFilter(struct json_object *Jarray);
#ifdef CONFIG_ZYXEL_ZY_LICENSE_CMD
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LICENSE
extern void zcfgFeDalShowZyLicense(struct json_object *Jarray);
#endif
#endif
#endif
extern void zcfgFeDalShowTime(struct json_object *Jarray);
extern void zcfgFeDalShowSysLog(struct json_object *Jarray);
extern void zcfgFeDalShowWifiMACFilter(struct json_object *Jarray);
extern void zcfgFeDalShowWifiOthers(struct json_object *Jarray);
extern void zcfgFeDalShowEthWanLan(struct json_object *Jarray);
extern void zcfgFeDalShowWlanScheduler(struct json_object *Jarray);
extern void zcfgFeDalShowWlanSchAccess(struct json_object *Jarray);
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_BANDWIDTH_IMPROVEMENT
extern void zcfgFeDalShowBandwidthImprovement(struct json_object *Jarray);
#endif
extern void zcfgFeDalShowPORT_MIRROR(struct json_object *Jarray);
#ifdef ZYXEL_WIFI_SUPPORT_WDS
extern void zcfgFeDalShowWifiWds(struct json_object *Jarray);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_MOS_USER_CONFIG
extern void zcfgFeDalShowGpon(struct json_object *Jarray);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
extern void zcfgFeDalShowSfpMgmt(struct json_object *Jarray);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
extern void zcfgFeDalShowWifiEasyMesh(struct json_object *Jarray);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_NAT_PORT_CTRL
extern void zcfgFeDalShowPcp(struct json_object *Jarray);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_IPSEC_VPN //richardfeng add IPSecVPN
extern dal_param_t IPSecVPN_param[];
extern zcfgRet_t zcfgFeDalIPSecVPN(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
extern void zcfgFeDalShowIPSecVPN(struct json_object *Jarray);
#endif

#if defined ZYXEL_OPAL_EXTENDER || defined CONFIG_ZCFG_BE_MODULE_ZYXEL_OPERATION_MODE
#ifdef CONFIG_ZCFG_BE_MODULE_END_POINT
extern void zcfgFeDalShowOperatingModes(struct json_object *Jarray);
#endif
extern zcfgRet_t zcfgFeDalExtenderNetworkMAP(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif


#ifdef ZYXEL_TAAAB_CUSTOMIZATION
extern void zcfgFeDalShowEasyMenuWifi(struct json_object *Jarray);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
extern dal_param_t MultiWan_param[];
extern zcfgRet_t zcfgFeDalMultiWan(const char* method, struct json_object* Jobj, struct json_object* Jarray, char* replyMsg);
extern zcfgRet_t zcfgFeDalShowMultiWan(struct json_object* Jarray);
#endif

#ifdef ZYXEL_TAAAG_EDNS
extern zcfgRet_t zcfgFeDalEDNS(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
#endif

dal_handler_t dalHandler[] = {
{"wan",				WAN_param,			zcfgFeDalWan,			zcfgFeDalShowWan,			"get|add|edit|delete",	"Broadband",											hidden_extender_dalcmd},
{"tr69",			MGMT_param,			zcfgFeDalMgmt,			zcfgFeDalShowMgmt,			"get|edit",			"TR-069_Client",											0},
{"ethwanlan", 		ETHWANLAN_param, 	zcfgFeDalEthWanLan,		zcfgFeDalShowEthWanLan,		"get|edit",				"Broadband",											hidden_extender_dalcmd},
{"dns",				DNS_ENTRY_param,	zcfgFeDalDnsEntry,		zcfgFeDalShowDnsEntry,		"get|add|edit|delete",	"DNS",													hidden_extender_dalcmd}, //DNS
{"ddns",			D_DNS_param,		zcfgFeDalDDns,			zcfgFeDalShowDDns,			"get|edit",			"DNS",														hidden_extender_dalcmd},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{"wan_adv",			ADVANCE_param,		zcfgFeDalWanAdvance,	zcfgFeDalShowWanAdvance,	"get|edit",				"Broadband",											hidden_extender_dalcmd}, //Broadband
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
{"cellular", 		WWANBACKUP_param,	zcfgFeDalWwanBackup,	zcfgFeDalShowWwanBackup,	"get|edit",				"Broadband",											hidden_extender_dalcmd}, //Broadband
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_BANDWIDTH_IMPROVEMENT
{"bandwidth_improvement",Bandwidth_Improvement_param,	zcfgFeDalBandwidthImprovement, zcfgFeDalShowBandwidthImprovement, "edit|get",	"Broadband",						hidden_extender_dalcmd},
#endif
{"ethctl",			ETH_CTL_param,		zcfgFeDalEthCtl,		zcfgFeDalShowEthCtl,		"get|edit",				"ethctl",												0}, //ETHCTL command
{"static_route", 	STATICROUTE_param,	zcfgFeDalStaticRoute,	zcfgFeDalShowStaticRoute,	"get|add|edit|delete",	"Routing",												hidden_extender_dalcmd},
{"dns_route", 		DNSROUTE_param,		zcfgFeDalDnsRoute,		zcfgFeDalShowDnsRoute,		"get|add|edit|delete",	"Routing",												hidden_extender_dalcmd}, //Routing
{"policy_route", 	POLICYROUTE_param,	zcfgFeDalPolicyRoute,	zcfgFeDalShowPolicyRoute,	"get|add|edit|delete",	"Routing",												hidden_extender_dalcmd}, //Routing
{"rip", 			RIP_param,			zcfgFeDalRip,			zcfgFeDalShowRip,			"get|edit",				"Routing",												hidden_extender_dalcmd}, //Routing
{"nat",				NatPortFwd_param,	zcfgFeDalNatPortMapping,zcfgFeDalShowPortFwd,		"get|add|edit|delete", "NAT",													hidden_extender_dalcmd},
{"nat_trigger",NAT_PORT_TRIGGERING_param,zcfgFeDalNatPortTriggering,zcfgFeDalShowPortTrigger,			"get|add|edit|delete", 	"NAT",										hidden_extender_dalcmd}, //NAT
{"nat_addr_map",NAT_ADDR_MAPPING_param,	zcfgFeDalNatAddrMapping,zcfgFeDalShowAddrMapping,	"get|add|edit|delete",	 "NAT",													hidden_extender_dalcmd}, //NAT
{"NAT_APPLICATION",NAT_Application_param,	zcfgFeDalNatApplication,NULL,					"add|delete",		"",															hidden_extender_dalcmd}, //NAT
{"nat_conf",		NAT_CONF_param,		zcfgFeDalNatConf, 		zcfgFeDalShowNatConf,		"edit|get",				"NAT",													hidden_extender_dalcmd}, //NAT
{"intf_group",		IntfGrp_param,		zcfgFeDalIntrGrp,		zcfgFeDalShowIntrGrp,		"add|edit|delete|get",	"Interface_Grouping",									hidden_extender_dalcmd},//
#ifdef CONFIG_ZCFG_BE_MODULE_GRE_TUNNEL
{"gre_tunnel",      GreTunnel_param,    zcfgFeDalGreTunnel,     zcfgFeDalShowGreTunnel,     "add|edit|delete|get",  "Tunnel_Setting",										hidden_extender_dalcmd},
#endif
{"lanadv",			LANADV_param, 		zcfgFeDalLanAdvance,	zcfgFeDalShowLanAdv,		"edit|get",			"Home_Networking",											hidden_extender_dalcmd},
{"lan",				LANSETUP_param, 	zcfgFeDalLanSetup,		zcfgFeDalShowLanSetup,		"get|edit", 			"Home_Networking",0},
{"ipalias",			SUBNET_param,		zcfgFeDalAddnlSubnet,	zcfgFeDalShowAddnl,			"get|edit" ,			"Home_Networking",										hidden_extender_dalcmd},//Home_Networking
{"static_dhcp",		STATIC_DHCP_param,	zcfgFeDalStaticDHCP,	zcfgFeDalShowStaticDHCP,	"get|add|edit|delete",	"Home_Networking",										hidden_extender_dalcmd},//Home_Networking
#ifdef ZYXEL_ANY_PORT_ANY_SERVICE
{"apas",			APAS_param,			zcfgFeDalAPAS,			zcfgFeDalShowAPAS,			"get|add|edit|delete",	"Home_Networking"},//Home_Networking
{"apas_macf",		APASMacRule_param,	zcfgFeDalAPASMacRule,	zcfgFeDalShowAPASMacRule,	"get|add|edit|delete",	"Home_Networking"},//Home_Networking
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
{"vlan_group", 		VLANGRP_param,		zcfgFeDalVlanGroup,		zcfgFeDalShowVlanGroup,		"get|add|edit|delete",	"Vlan Group",											hidden_extender_dalcmd},//Vlan_Group
#endif
{"igmp_mld", 		IGMPMLD_param,		zcfgFeDalIGMPMLD,		zcfgFeDalShowIGMPMLD,		"get|edit",			"IGMP_MLD",													hidden_extender_dalcmd},//IGMP_MLD
{"firewall",		FIREWALL_param, 	zcfgFeDalFirewall,		zcfgFeDalShowFirewall,		"get|edit",			"Firewall",													hidden_extender_dalcmd}, //Firewall
{"firewall_proto",	FIREWALL_PROTOCOL_param, zcfgFeDalFirewallProtocol,zcfgFeDalShowFirewallProtocol,	"get|add|edit|delete", 	"Firewall",									hidden_extender_dalcmd},//Firewall
{"firewall_acl", 	FIREWALL_ACL_param, zcfgFeDalFirewallACL,	zcfgFeDalShowFirewallACL,	"add|edit|delete|get",	"Firewall",												hidden_extender_dalcmd},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{"wlan",			WIFI_GENERAL_param,	zcfgFeDalWifiGeneral,   zcfgFeDalShowWifiGeneral,	"edit|get",			"Wireless",													0},
#endif
{"macfilter",		MAC_Filter_param,	zcfgFeDalMACFilter,		zcfgFeDalShowMACFilter,		"get|add|edit|delete",	"MAC_Filter",											0},//MAC_Filter
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
{"ipfilter",		IP_Filter_param,	zcfgFeDalIPFilter, 		zcfgFeDalShowIPFilter, 	 "get|add|edit|delete",	"MAC_Filter",                                               hidden_extender_dalcmd},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{"EasyMenuWifi",	EasyMenuWifi_param, zcfgFeDalEasyMenuWifi,	zcfgFeDalShowEasyMenuWifi,	"edit|get",         "EasyMenuWifi",                                             hidden_extender_dalcmd},
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
{"qos",				QosGeneral_param,	zcfgFeDalQosGeneral,	zcfgFeDalShowQosGeneral,	"get|edit",	"QoS",																hidden_extender_dalcmd},//QoS
{"qos_queue",		QosQueue_param,		zcfgFeDalQosQueue,		zcfgFeDalShowQosQueue,		"get|add|edit|delete",	"QoS",													hidden_extender_dalcmd},//QoS
{"qos_class",		QosClass_param,		zcfgFeDalQosClass,		zcfgFeDalShowQosClass,	    "get|add|edit|delete",	"QoS",													hidden_extender_dalcmd},//QoS
{"qos_shaper",		QosShaper_param,	zcfgFeDalQosShaper,		zcfgFeDalShowQosShaper,		"get|add|edit|delete",	"QoS",													hidden_extender_dalcmd},//QoS
{"qos_policer",		QosPolicer_param,	zcfgFeDalQosPolicer,	zcfgFeDalShowQosPolicer,	"get|add|edit|delete",	"QoS",													hidden_extender_dalcmd},//QoS
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{"wifi_macfilter",	WIFI_MACFILTER_param,	zcfgFeDalWifiMACFilter,zcfgFeDalShowWifiMACFilter,"get|add|edit|delete",	"Wireless",											0},//
{"wps",				WIFI_WPS_param,		zcfgFeDalWifiWps,		zcfgFeDalShowWifiWps,		"edit|get",			"Wireless",													0},
{"wmm",				WIFI_WMM_param,		zcfgFeDalWifiWmm,		zcfgFeDalShowWifiWmm,		"edit|get",			"Wireless",													0},//
#ifdef ZYXEL_WIFI_SUPPORT_WDS
{"wifi_wds",	       WIFI_WDS_param,		zcfgFeDalWifiWds,		zcfgFeDalShowWifiWds,		"get|add|edit|delete",	"Wireless"},//
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
{"wifi_easy_mesh",		WIFI_EASYMESH_param, 	zcfgFeDalWifiEasyMesh,	zcfgFeDalShowWifiEasyMesh,	"edit|get",		"Wireless",												0},//Wireless
#endif
{"wifi_others",		WIFI_OTHERS_param,	zcfgFeDalWifiOthers,	zcfgFeDalShowWifiOthers,	"edit|get",				"Wireless",												0},//
#ifdef CONFIG_ZCFG_BE_MODULE_WLAN_SCHEDULER
{"wlan_scheduler",	WLAN_SCHEDULER_param, zcfgFeDalWlanScheduler,	zcfgFeDalShowWlanScheduler,	"add|edit|delete|get",	"Wireless",											hidden_extender_dalcmd},
{"wlan_sch_access",	WLAN_SCH_ACCESS_param, zcfgFeDalWlanSchAccess,	zcfgFeDalShowWlanSchAccess, "edit|get",	"Wireless",														hidden_extender_dalcmd},
#endif
#if defined(ABZQ_CUSTOMIZATION_SUPPORT_WIFI_CHANNEL_HOPPING)
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{"wlan_chanhopping",	WLAN_CHANNELHOPPING_param, zcfgFeDalWlanChannelHopping,	NULL, "edit|get",	"Wireless"},
#endif
#endif
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI

#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
{"snmp",			SNMP_param, 		zcfgFeDalSNMP,			zcfgFeDalShowSNMP,			"edit|get",				"SNMP",													hidden_extender_dalcmd},
#endif
{"time",			TIME_param, 		zcfgFeDalTime,			zcfgFeDalShowTime,			"edit|get",				"Time",													0},//Time
{"trust_domain",	TRUST_DOMAIN_param, zcfgFeDalTrustDomain,	zcfgFeDalShowTrustDomain,	"get|add|delete",		"Remote_MGMT",											hidden_extender_dalcmd},//Remote_MGMT
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PPTP_SUPPORT
{"pptp",			PPTP_param,			zcfgFeDalPptp,			NULL,						"get|edit",				"PPTP",													hidden_extender_dalcmd},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_VPN_CLIENT
{"vpnclient",		VPNClient_param,	zcfgFeDalVPNClient,		zcfgFeDalShowVPNClient,		"get|add|edit|delete",	"VPN"},
#endif
{"sp_trust_domain",	SP_TRUST_DOMAIN_param, zcfgFeDalSpTrustDomain, zcfgFeDalShowSpTrustDomain,	"get|add|delete",		"SP_Domain",										hidden_extender_dalcmd},//SP_Domain
#if BUILD_PACKAGE_ZyIMS
{"sip_account",		VOIP_LINE_param,	zcfgFeDalVoipLine,		zcfgFeDalShowVoipLine,		"get|add|edit|delete", "SIP",													hidden_extender_dalcmd},//SIP
{"sip_sp",			VOIP_PROFILE_param,	zcfgFeDalVoipProfile,	zcfgFeDalShowVoipProfile,	"get|add|edit|delete", "SIP",													hidden_extender_dalcmd},//SIP
{"sip_tls",     VOIP_TLS_param,    zcfgFeDalVoipTLS,      zcfgFeDalShowVoipTLS,      "get|edit", "SIP",																		hidden_extender_dalcmd},//SIP
{"region",			VOIP_REGION_param,	zcfgFeDalVoipPhoneRegion,zcfgFeDalShowVoipRegion,	"get|edit",				"Phone",												hidden_extender_dalcmd},//Phone
{"phone",		VOIP_PHONEDEVICE_param,	zcfgFeDalVoipPhoneDevice,zcfgFeDalShowVoipPhoneDevice,					"get|edit",			"Phone",								hidden_extender_dalcmd},//Phone
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_PHONE_DECT
{"PHONEDECT",		VOIP_PHONEDECT_param,zcfgFeDalVoipPhoneDECT,NULL,						"edit",				"",															hidden_extender_dalcmd},//Phone
#endif
{"speeddial",		VOIP_CALLRULE_param,zcfgFeDalVoipCallRule,	zcfgFeDalShowVoipCallRule,	"get|edit|delete",	"Call_Rule",												hidden_extender_dalcmd},//Call_Rule
{"callblock",		VOIP_CALLBLOCK_param,zcfgFeDalVoipCallBlock,zcfgFeDalShowVoipCallBlock, "get|edit|delete",	"Call_Rule",												hidden_extender_dalcmd},//Call_Rule
{"call_history",	VOIP_CALLHISTORY_param,zcfgFeDalVoipCallHistory,	zcfgFeDalShowVoipCallHistory,	"get|delete",	"Call_Rule",										hidden_extender_dalcmd},
{"call_summary",	VOIP_CALLSUMMARY_param,zcfgFeDalVoipCallSummary,	zcfgFeDalShowVoipCallSummary,	"get|delete",	"Call_Rule",										hidden_extender_dalcmd},
#endif
{"email_ntfy",		MAIL_NOTIFICATION_param,zcfgFeDalMailNotification,zcfgFeDalShowMailNotification,	"add|delete|get", 		"Email_Notify",								hidden_extender_dalcmd},//Email_Notify
{"logset",			LOG_SETTING_param,	zcfgFeDalLogSetting,	zcfgFeDalShowLogSetting,	"get|edit", 		"Log",														hidden_extender_dalcmd}, //Log
{"sys_log", 		SYS_LOG_param, 		zcfgFeDalSysLog,		zcfgFeDalShowSysLog,		"get|delete", 		"Log",														0},
#ifdef CONFIG_ZCFG_BE_MODULE_ZLOG_USE_DEBUG
{"zlog",            ZLOG_SETTING_param, zcfgFeDalZlogSetting,   zcfgFeDalShowZlogSetting,   "get|edit",         "Log",														hidden_extender_dalcmd}, //Zlog
#endif
{"PINGTEST",		PING_TEST_param,	zcfgFeDalPingTest,		NULL,						"edit",				"",															0},//
{"8021AG",			IEEE_8021AG_param,	zcfgFe8021ag,			NULL,						"edit",				"",															0},//Diagnostic_Result
{"8023AH",			IEEE_8023AH_param,	zcfgFe8023ah,			NULL,						"edit",				"",															0},//Diagnostic_Result
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
{"OAMPING", 		OAMPING_param,		zcfgFeOamPing,			NULL,						"edit",				"",															0},//Diagnostic_Result
#endif
{"usb_info", 		USB_FILESHARING_param,	zcfgFeDalUsbFilesharing,zcfgFeDalShowUsbFilesharing,	"get|edit", 	"USB_Service",											hidden_extender_dalcmd},//USB_Service
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{"usb_filesharing",	USB_SAMBA_param,	zcfgFeDalUsbSamba,		zcfgFeDalShowUsbSamba,		"get|add|edit|delete",	"USB_Service",											hidden_extender_dalcmd},//USB_Service
#endif
#ifdef CONFIG_ZYXEL_TR140
{"ttsamba_account",	TTSAMBA_ACCOUNT_param, zcfgFeDalSambaAccount,	NULL,	"get|add|edit|delete",	"Samba_Account"},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DLNA
{"media_server", 	USB_MEDIASERVER_param,	zcfgFeDalUsbMediaserver,	zcfgFeDalShowUsbMediaserver,"edit|get", 	"USB_Service",											hidden_extender_dalcmd},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
{"print_server",	USB_PRINTSERVER_param,	zcfgFeDalUsbPrintserver,	NULL,				"edit|get", 			"USB_Service",											hidden_extender_dalcmd},
#endif
{"paren_ctl", 		PAREN_CTL_param,	zcfgFeDalParentCtl,		zcfgFeDalShowParentCtl,						"get|add|edit|delete",	"Parental_Control",						hidden_extender_dalcmd},//Parental_Control
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
{"cyber_secure", 	CYBER_SECURE_param,	zcfgFeDalCyberSecure,	zcfgFeDalShowCyberSecure,						"get|edit",	"Parental_Control",								hidden_extender_dalcmd},//Cyber_security
{"content_filter", 	CONTENT_FILTER_param,	zcfgFeDalContentFilter,	zcfgFeDalShowContentFilter,						"get|add|edit|delete",	"Parental_Control",				hidden_extender_dalcmd},//Cyber_security
{"change_icon_name", 	ChangeIconName_param,	zcfgFeDalChangeIconName,	zcfgFeDalShowChangeIconName,						"get|add|edit|delete",	"Parental_Control",	hidden_extender_dalcmd},//Cyber_security
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_CYBER_SECURITY
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_URL_FILTER
{"URL_filter", 		URL_FILTER_param,	zcfgFeDalURLFilter,	zcfgFeDalShowURLFilter,						"get|add|edit|delete",	"Parental_Control",							hidden_extender_dalcmd},//Cyber_security
{"URL_WhiteList", 	URLWhite_FILTER_param,	zcfgFeDalURLWhiteFilter,	zcfgFeDalShowURLWhiteFilter,	"get|add|edit|delete",	"Parental_Control",							hidden_extender_dalcmd},//Cyber_security
{"URL_BlackList", 	URLBlack_FILTER_param,	zcfgFeDalURLBlackFilter,	zcfgFeDalShowURLBlackFilter,	"get|add|edit|delete",	"Parental_Control",							hidden_extender_dalcmd},//Cyber_security
#endif
#endif
#ifdef CONFIG_ZYXEL_ZY_LICENSE_CMD
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LICENSE
{"zylicense", 		ZyLicense_param,	zcfgFeDalZyLicense,	zcfgFeDalShowZyLicense,						"get|edit",	"Parental_Control",										hidden_extender_dalcmd},//Cyber_security
#endif
#endif
{"scheduler",		SCHEDULE_param, 	zcfgFeDalSchedule,		zcfgFeDalShowSchedule,		"get|add|edit|delete",	"Scheduler_Rule",										hidden_extender_dalcmd},
#ifdef CONFIG_ZCFG_BE_MODULE_IPSEC_VPN //richardfeng add IPSecVPN
{"IPSecVPN",		IPSecVPN_param, 	zcfgFeDalIPSecVPN,		zcfgFeDalShowIPSecVPN,		"get|add|edit|delete",	"IPSecVPN_Rule",									hidden_extender_dalcmd},
#endif
#ifdef ZYXEL_AVAST_AGENT_SUPPORT
{"package_3rdParty",Package_3rdParty_param, 	zcfgFeDalPackage3rdParty,	NULL,		"edit|get", "Package_3rdParty",														hidden_extender_dalcmd},
#endif
{"dev_sec_cert", 	DEV_SEC_CERT_param,	zcfgFeDalDevSecCert, 	NULL,						"get|add|edit|delete", 	"",														0},//Certificates
{"one_connect", 	HOME_CONNECTIVYITY_param, zcfgFeDalHomeConnectivity,zcfgFeDalShowHomeConnect, 	"get|edit", 		"Home_Connectivity",								hidden_extender_dalcmd},//Home_Connectivity
{"oui",				OUI_param,		zcfgFeDaloui,		zcfgFeDalShowoui,		"get|edit",		"Home_Connectivity",														hidden_extender_dalcmd},//Set one connect oui enable/disable, not for GUI
{"mgmt_srv", 		MGMT_SRV_param,	zcfgFeDalMgmtSrv,			zcfgFeDalShowMgmtSrv,		"edit|get",			"Remote_MGMT",												0},
{"sp_mgmt_srv", 	SP_MGMT_SRV_param,	zcfgFeDalSPMgmtSrv,		zcfgFeDalShowSPMgmtSrv,		"edit|get",			"SP_Domain",												hidden_extender_dalcmd},
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
{"cellwan", 		CELLWAN_param,		zcfgFeDalCellWan,		zcfgFeDalShowCellWan,		"get|edit",				"Broadband",											hidden_extender_dalcmd},
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_MULTI_APN
{"cellwan_mapn", 	CELLWAN_MAPN_param,	zcfgFeDalCellWanMApn,	zcfgFeDalShowCellWanMApn,	"get|edit",				"Broadband",											hidden_extender_dalcmd},
#endif
{"cellwan_sim", 	CELLWAN_SIM_param,	zcfgFeDalCellWanSim,	zcfgFeDalShowCellWanSim,	"get|edit",				"Broadband",											hidden_extender_dalcmd},
{"cellwan_band", 	CELLWAN_BAND_param,	zcfgFeDalCellWanBand,	zcfgFeDalShowCellWanBand,	"get|edit",				"Broadband",											hidden_extender_dalcmd},
{"cellwan_plmn", 	CELLWAN_PLMN_param,	zcfgFeDalCellWanPlmn,	zcfgFeDalShowCellWanPlmn,	"get|edit",				"Broadband",											hidden_extender_dalcmd},
{"cellwan_psru", 	CELLWAN_PSRU_param,	zcfgFeDalCellWanPsru,	zcfgFeDalShowCellWanPsru,	"get|edit",				"Broadband",											hidden_extender_dalcmd},
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_SMS
{"cellwan_sms", 	CELLWAN_SMS_param,	zcfgFeDalCellWanSms,	NULL,						"",					"",															hidden_extender_dalcmd}, //temporarily GUI only
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_PCI_LOCK
{"cellwan_lock", 	CELLWAN_LOCK_param,	zcfgFeDalCellWanLock,	zcfgFeDalShowCellWanLock,	"get|add|edit|delete",			"Broadband",									hidden_extender_dalcmd},
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_GNSS_LOCATION
{"cellwan_gnss",        CELLWAN_GNSS_param,     zcfgFeDalCellWanGnss,   zcfgFeDalShowCellWanGnss,       "get|edit",				"Broadband",								hidden_extender_dalcmd},
#endif
#ifdef ZYXEL_WEB_GUI_SUPPORT_SAS_CBSD
{"cbsd", 		CBSD_param,		zcfgFeDalCbsd,		zcfgFeDalShowCbsd,						"get|edit",				"SAS_CBSD",												hidden_extender_dalcmd},
#endif
#ifdef CONFIG_ZYXEL_IPPASS_RM_MULTI_WAN_SUPPORT
{"cellwan_mgmt",	CELLWAN_MGMT_param,	zcfgFeDalCellWanMgmt,	zcfgFeDalShowCellWanMgmt,	"get|edit",				"Remote_MGMT",											hidden_extender_dalcmd},
#endif
{"cellwan_status", 	CELLWAN_STATUS_param,	zcfgFeDalCellWanStatus,	zcfgFeDalShowCellWanStatus,	"get",				"Status",												hidden_extender_dalcmd},
{"cellwan_wait_state", 	CELLWAN_WAIT_STATE_param,	zcfgFeDalCellWanWaitState,	zcfgFeDalShowCellWanWaitState,	"get|edit",		"Status",									hidden_extender_dalcmd},
#endif
{"login_privilege", LOGIN_PRIVILEGE_param,	zcfgFeDalLoginPrivilege,NULL, 					"edit|get",				"",														0},//root_only
{"QUICKSTART", 		QUICK_START_param,	zcfgFeDalQuickStart,	NULL,						"edit",				"",															0},//root_only
#ifdef ABUU_CUSTOMIZATION
{"login_cfg",		LOG_CFG_param, 		zcfgFeDalLogCfg,	NULL,	"get|edit",	"login_cfg"}, 
#endif
{"user_account",	USER_ACCOUNT_param, zcfgFeDalUserAccount,	zcfgFeDalShowUserAccount,	"get|add|edit|delete",	"User_Account",											0}, //temporarily root_only //User_Account
{"lanhosts",		NETWORK_MAP_param, zcfgFeDalNetworkMAP,	zcfgFeDalShowNetworkMAP,		"get|add|edit|delete",				"Status",									hidden_extender_dalcmd},
{"status",			NETWORK_MAP_param, zcfgFeDalStatus, 		NULL,						"",					"",															0}, //temporarily GUI only
{"cardpage_status", NETWORK_MAP_param, zcfgFeDalCardPageStatus, NULL,						"", 				"",															0}, //temporarily cardpage only
#ifdef GFIBER_CUSTOMIZATION
{"gfiberstatus",     NETWORK_MAP_param, zcfgFeDalGfiberStatus,         NULL,                       "",                 ""},
{"gfibercardstatus", NETWORK_MAP_param, zcfgFeDalGfiberCardStatus, NULL,                       "",                 ""},
#endif
{"LanPortInfo", 	NETWORK_MAP_param, zcfgFeDalLanPortInfo, 	NULL,						"", 				"",															0},
//Monitor
{"Traffic_Status", Traffic_Status_param, zcfgFeDalTrafficStatus, 	NULL,						"get", 				"",														0},
#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
{"VOIP_Status", VOIP_Status_param, zcfgFeDalVoIPStatus, 	NULL,						"get", 				"",																0},
#endif
{"port_mirror", PORT_MIRROR_param, zcfgFeDalPortMirror, 	zcfgFeDalShowPORT_MIRROR,						"edit|get",				"Status",								0},
#ifdef CONFIG_TAAAB_PORTMIRROR
{"ttportmirror", TAAAB_PORT_MIRROR_param, zcfgFeDalTTPortMirror, 	NULL,						"edit|get",				""},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
{"packetcapture", PacketCapture_param, zcfgFeDalPacketCapture, 	NULL,						"edit|get",				"Status"},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
{"sensor", 		SENSOR_param, zcfgFeDalSensor, 				NULL,						"edit", 			"",																0},
#endif
//#ifdef CAAAB_CUSTOMIZATION // 20170711 Max Add
#ifdef ZYXEL_WEB_WIRELESS_NEW_VERSION_ONE
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{"WIFI_CBT",		WIFI_CAAAB_param,		zcfgFeDalCbtWifi,		NULL,		"edit",		"",																					0},
{"WIFI_ADVANCED_CBT",		WIFI_ADV_CAAAB_param,		zcfgFeDalCbtWifiAdv,		NULL,		"edit",		"",																	0},
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_WIFI
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_MOS_USER_CONFIG
{"gpon", GPON_param, zcfgFeDalGpon, 	zcfgFeDalShowGpon,						"edit|get",				"gpon",																hidden_extender_dalcmd},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
{"sfp_mgmt", SFP_MGMT_param, zcfgFeDalSfpMgmt, 	zcfgFeDalShowSfpMgmt,		"edit|get",				"SFPManagement",														hidden_extender_dalcmd},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
#ifdef ZYXEL_WEB_GUI_SHOW_ZYEE /* JoannaSu, 20191115, support ZyEE configuration */
{"ZYDUACT", ZYEE_DU_param, zcfgFeDalDUAction,  NULL,       "edit",             "",																							0},
{"ZYEUACT", ZYEE_EU_param, zcfgFeDalEUAction,  NULL,       "edit|get",         "",                                                                                          0},
#endif
#endif
#if defined ZYXEL_OPAL_EXTENDER || defined CONFIG_ZCFG_BE_MODULE_ZYXEL_OPERATION_MODE
#ifdef CONFIG_ZCFG_BE_MODULE_END_POINT
{"OperatingModes", WIFI_OperatingModes_param, zcfgFeDalOperatingModes, 	zcfgFeDalShowOperatingModes,		"edit|get", 			"Wireless",								0},
#endif
{"ExtenderNetMAP",  NETWORK_MAP_param, zcfgFeDalExtenderNetworkMAP,    NULL,                                "get",                  "",                                  0},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_OPERATION_MODE
{"OperationMode", WIFI_Opm_param, zcfgFeDalWifiOpm, 		NULL,						"edit|get", 			"Wireless",													0},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_NAT_PORT_CTRL
{"nat_pcp",NAT_PCP_param,zcfgFeDalNatPcp,zcfgFeDalShowPcp,			"get|add|edit|delete", 	"NAT"}, //NAT
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_REBOOT_SCHEDULE
{"reboot_schedule",		REBOOT_SCHEDULE_param,		zcfgFeDalRebootSchedule,		NULL,		"edit|get",		"",		0},
#endif
#ifdef ZYXEL_TAAAG_EDNS
{"EDNS",		EDNS_param, 	zcfgFeDalEDNS,	NULL,	"edit|get",		"",		0},
#endif
#ifdef ZYXEL_VPNLITE
{"VPNLite",  VPNLite_param, zcfgFeDalVPNLite,    NULL,                                "edit|get",                  "",                                  0},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
{ "MultiWan", MultiWan_param, zcfgFeDalMultiWan, 		zcfgFeDalShowMultiWan,						"edit|get", 			"Broadband",													0 },
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_DSL_TROUBLE_SHOOTING
{"DSLDiagd", DSLDiagd_param, zcfgFeDalDSLDiagd, 		NULL,						"edit|get", 			"dsldiagd",													0},
#endif
{NULL,				NULL,				NULL, 					NULL,						NULL,				 NULL,														0}
};

dal_voip_region_t dalVoipRegion[] = {
{"AE",	"ARE-United_Arab_Emirates"},
{"AU",	"AUS-Austrailia"},
{"BE",	"BEL-Beligium"},
{"BR",	"BRA-Brazil"},
{"CH",	"CHE-Switzerland"},
{"CL",	"CHL-Chile"},
{"CN",	"CHN-China"},
{"CO",	"COL-Colombia"},
{"CZ",	"CZE-Czech"},
{"DE",	"DEU-Germany"},
{"DK",	"DNK-Denmark"},
{"ES",	"ESP-Spain"},
{"FI",	"FIN-Finland"},
{"FR",	"FRA-France"},
{"GB",	"GBR-UK"},
{"HR",	"HRV-Croatia"},
{"HU",	"HUN-Hungary"},
{"IN",	"IND-India"},
{"IT",	"ITA-Italy"},
{"JP",	"JPN-Japan"},
{"NL",	"NLD-Netherlands"},
{"NO",	"NOR-Norway"},
{"NZ",	"NZL-New_Zealand"},
{"PL",	"POL-Poland"},
{"SK",	"SVK-Slovakia"},
{"SI",	"SVN-Slovenia"},
{"SE",	"SWE-Sweden"},
{"TR",	"TUR-Turkey"},
{"TW",	"TWN-Taiwan"},
{"US",	"USA-United_States_of_America"},
{NULL,	NULL}
};

dal_firewall_protocol_t dalFirewallICMP6[] = {
{"1",		"0",		"1/No-Rute"},
{"1", 		"1",		"1/Communication-Prohibited"},
{"1", 		"3",		"1/Address-Unreachable"},
{"1", 		"4",		"1/Port-Unreachable"},
{"2", 		"0",		"2/Packet_too_Big"},
{"3", 		"0",		"3/Time_Exceeded"},
{"3", 		"1",		"3/Time_Exceeded"},
{"4", 		"0",		"4/Parameter_Problem"},
{"4", 		"1",		"4/Parameter_Problem"},
{"4", 		"2",		"4/Parameter_Problem"},
{"128", 	"-1",		"128/Echo_Request"},
{"129", 	"-1",		"129/Echo_Response"},
{"130", 	"-1",		"130/Listener_Query"},
{"131", 	"-1",		"131/Listener_Report"},
{"132", 	"-1",		"132/Listener_Done"},
{"143", 	"-1",		"143/Listener_Report_v2"},
{"133", 	"-1",		"133/Router_Solicitation"},
{"134", 	"-1",		"134/Router_Advertisement"},
{"135", 	"-1",		"135/Neighbor_Solicitation"},
{"136", 	"-1",		"136/Neighbor_Advertisement"},
{"137", 	"-1",		"137/Redirect"},
{NULL,		NULL,		NULL}
};

dal_time_zone_t dalTimeZone[] = {
{"(GMT-12:00) International Date Line West",				"International_Date_Line_West[GMT-12:00]",		"IDLW+12"},
{"(GMT-11:00) Midway Island, Samoa",						"Midway_Island,Samoa[GMT-11:00]",	"SST+11"},
{"(GMT-10:00) Hawaii",										"Hawaii[GMT-10:00]",	"HST+10"},
{"(GMT-09:00) Alaska",										"Alaska[GMT-09:00]",	"AKST+9AKDT"},
{"(GMT-08:00) Pacific Time, Tijuana",						"Pacific_Time,Tijuana[GMT-08:00]",	"PST+8PDT"},
{"(GMT-07:00) Arizona, Mazatlan",							"Arizona,Mazatlan[GMT-07:00]",	"MST+7"},
{"(GMT-07:00) Chihuahua",									"Chihuahua[GMT-07:00]",	"MST+7MDT"},
{"(GMT-07:00) Mountain Time",								"Mountain_Time[GMT-07:00]",	"MST+7MDT2"},
{"(GMT-06:00) Central America",								"Central_America[GMT-06:00]",	"CST+6CDT"},
{"(GMT-06:00) Central Time",								"Central_Time[GMT-06:00]",	"CST+6CDT2"},
{"(GMT-06:00) Guadalajara, Mexico City, Monterrey",			"Guadalajara,Mexico_City,Monterrey[GMT-06:00]",	"CST+6CDT3"},
{"(GMT-06:00) Saskatchewan",								"Saskatchewan[GMT-06:00]",	"CST+6"},
{"(GMT-05:00) Bogota, Lima, Quito",							"Bogota,Lima,Quito[GMT-05:00]",	"COT+5"},
{"(GMT-05:00) Eastern Time",								"Eastern_Time[GMT-05:00]",	"EST+5EDT"},
{"(GMT-05:00) Indiana",										"Indiana[GMT-05:00]",	"EST+5EDT2"},
{"(GMT-04:00) Atlantic Time",								"Atlantic_Time[GMT-04:00]",	"AST+4ADT"},
{"(GMT-04:00) Caracas, La Paz",								"Caracas,La_Paz[GMT-04:00]",	"BOT+4"},
{"(GMT-04:00) Santiago",									"Santiago[GMT-04:00]",	"CLT+4CLST"},
{"(GMT-04:00) Georgetown",									"Georgetown[GMT-04:00]",	"ART+4"},
{"(GMT-03:30) Newfoundland",								"Newfoundland[GMT-03:30]",	"NST+3:30NDT"},
{"(GMT-03:00) Brasilia",									"Brasilia[GMT-03:00]",	"BRT+3BRST"},
{"(GMT-03:00) Buenos Aires",								"Buenos_Aires[GMT-03:00]",	"ART+3"},
{"(GMT-03:00) Greenland",									"Greenland[GMT-03:00]",	"CGT+3"},
{"(GMT-02:00) Mid-Atlantic",								"Mid-Atlantic[GMT-02:00]",	"MAT+2"},
{"(GMT-01:00) Azores",										"Azores[GMT-01:00]",	"AZOT+1AZOST"},
{"(GMT-01:00) Cape Verde Is.",								"Cape_Verde_Is[GMT-01:00]",	"CVT+1"},
{"(GMT-00:00) Casablanca",									"Casablanca[GMT-00:00]",	"WET-0WEST"},
{"(GMT-00:00) Monrovia",									"Monrovia[GMT-00:00]",	"WET-0"},
{"(GMT-00:00) Greenwich Mean Time: Edinburgh, London",		"Greenwich_Mean_Time_Edinburgh,London[GMT-00:00]",	"GMT-0BST"},
{"(GMT-00:00) Greenwich Mean Time: Dublin",					"Greenwich_Mean_Time_Dublin[GMT-00:00]",	"GMT-0IST"},
{"(GMT-00:00) Lisbon",										"Lisbon[GMT-00:00]",	"WET-0WEST2"},
{"(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",	"Amsterdam,Berlin,Bern,Rome,Stockholm,Vienna[GMT+01:00]",	"CET-1CEST"},
{"(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague",	"Belgrade,Bratislava,Budapest,Ljubljana,Prague[GMT+01:00]",	"CET-1CEST2"},
{"(GMT+01:00) Brussels, Copenhagen, Madrid, Paris",			"Brussels,Copenhagen,Madrid,Paris[GMT+01:00]",	"CET-1CEST3"},
{"(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb",			"Sarajevo,Skopje,Warsaw,Zagreb[GMT+01:00]",	"CET-1CEST4"},
{"(GMT+01:00) West Central Africa",							"West_Central_Africa[GMT+01:00]",	"WAT-1"},
{"(GMT+02:00) Athens, Istanbul, Minsk",						"Athens,Istanbul,Minsk[GMT+02:00]",	"EET-2EEST"},
{"(GMT+02:00) Bucharest",									"Bucharest[GMT+02:00]",	"EET-2EEST2"},
{"(GMT+02:00) Cairo",										"Cairo[GMT+02:00]",	"EET-2EEST3"},
{"(GMT+02:00) Harare, Pretoria",							"Harare,Pretoria[GMT+02:00]",	"CAT-2"},
{"(GMT+02:00) Pretoria",									"Pretoria[GMT+02:00]",	"SAST-2"},
{"(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius",		"Helsinki,Kyiv,Riga,Sofia,Tallinn,Vilnius[GMT+02:00]",	"EET-2EEST4"},
{"(GMT+02:00) Jerusalem",									"Jerusalem[GMT+02:00]",	"IST-2IDT"},
{"(GMT+03:00) Baghdad",										"Baghdad[GMT+03:00]",	"AST-3"},
{"(GMT+03:00) Kuwait, Riyadh",								"Kuwait,Riyadh[GMT+03:00]",	"AST-3-2"},
{"(GMT+03:00) Moscow, St. Petersburg, Volgograd",			"Moscow,St.Petersburg,Volgograd[GMT+03:00]",	"MSK-3MSD"},
{"(GMT+03:00) Nairobi",										"Nairobi[GMT+03:00]",	"EAT-3"},
{"(GMT+03:30) Tehran",										"Tehran[GMT+03:30]",	"IRST-3IRDT"},
{"(GMT+04:00) Abu Dhabi, Muscat",							"Abu_Dhabi,Muscat[GMT+04:00]",	"GST-4"},
{"(GMT+04:00) Baku",										"Baku[GMT+04:00]",	"AZT-4AZST"},
{"(GMT+04:00) Yerevan",										"Yerevan[GMT+04:00]",	"AMT-4AMST"},
{"(GMT+04:00) Tbilisi",										"Tbilisi[GMT+04:00]",	"GET-4"},
{"(GMT+04:30) Kabul",										"Kabul[GMT+04:30]",	"AFT-4:30"},
{"(GMT+05:00) Yekaterinburg",								"Yekaterinburg[GMT+05:00]",	"YEKT-5YEKST"},
{"(GMT+05:00) Islamabad, Karachi",							"Islamabad,Karachi[GMT+05:00]",	"PKT-5"},
{"(GMT+05:00) Tashkent",									"Tashkent[GMT+05:00]",	"UZT-5"},
{"(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi",			"Chennai,Kolkata,Mumbai,New_Delhi[GMT+05:30]",	"IST-5:30"},
{"(GMT+05:45) Kathmandu",									"Kathmandu[GMT+05:45]",	"NPT-5:45"},
{"(GMT+06:00) Almaty, Astana",								"Almaty,Astana[GMT+06:00]",	"ALMT-6"},
{"(GMT+06:00) Novosibirsk",									"Novosibirsk[GMT+06:00]",	"NOVT-6NOVST"},
{"(GMT+06:00) Dhaka",										"Dhaka[GMT+06:00]",	"BST-6"},
{"(GMT+06:00) Sri Jayawardenapura",							"Sri_Jayawardenapura[GMT+06:00]",	"LKT-6"},
{"(GMT+06:30) Yangoon",										"Yangoon[GMT+06:30]",	"MMT-6:30"},
{"(GMT+07:00) Bangkok, Hanoi",								"Bangkok,Hanoi[GMT+07:00]",	"ICT-7"},
{"(GMT+07:00) Jakarta",										"Jakarta[GMT+07:00]",	"WIB-7"},
{"(GMT+07:00) Krasnoyarsk",									"Krasnoyarsk[GMT+07:00]",	"KRAT-7KRAST"},
{"(GMT+08:00) Hong Kong",									"Hong_Kong[GMT+08:00]",	"HKT-8"},
{"(GMT+08:00) Beijing, Chongquing, Urumqi",					"Beijing,Chongquing,Urumqi[GMT+08:00]",	"CST-8"},
{"(GMT+08:00) Irkutsk",										"Irkutsk[GMT+08:00]",	"IRKT-8IRST"},
{"(GMT+08:00) Ulaan Bataar",								"Ulaan_Bataar[GMT+08:00]",	"LUAT-8"},
{"(GMT+08:00) Kuala Lumpur",								"Kuala_Lumpur[GMT+08:00]",	"MYT-8"},
{"(GMT+08:00) Singapore",									"Singapore[GMT+08:00]",	"SGT-8"},
{"(GMT+08:00) Perth",										"Perth[GMT+08:00]",	"WST-8"},
{"(GMT+08:00) Taipei",										"Taipei[GMT+08:00]",	"CST-8-2"},
{"(GMT+09:00) Osaka, Sapporo, Tokyo",						"Osaka,Sapporo,Tokyo[GMT+09:00]",	"JST-9"},
{"(GMT+09:00) Seoul",										"Seoul[GMT+09:00]",	"KST-9"},
{"(GMT+09:00) Yakutsk",										"Yakutsk[GMT+09:00]",	"YAKT-9YAKST"},
{"(GMT+09:30) Adelaide",									"Adelaide[GMT+09:30]",	"CST-9:30CDT"},
{"(GMT+09:30) Darwin",										"Darwin[GMT+09:30]",	"CST-9:30"},
{"(GMT+10:00) Brisbane",									"Brisbane[GMT+10:00]",	"EST-10"},
{"(GMT+10:00) Canberra, Melbourne, Sydney",					"Canberra,Melbourne,Sydney[GMT+10:00]",	"EST-10EDT"},
{"(GMT+10:00) Guam",										"Guam[GMT+10:00]",	"ChST-10"},
{"(GMT+10:00) Port Moresby",								"Port_Moresby[GMT+10:00]",	"PGT-10"},
{"(GMT+10:00) Hobart",										"Hobart[GMT+10:00]",	"EST-10EDT2"},
{"(GMT+10:00) Vladivostok",									"Vladivostok[GMT+10:00]",	"VLAT-10VLAST"},
{"(GMT+11:00) Magadan",										"Magadan[GMT+11:00]",	"MAGT-11MAGST-11"},
{"(GMT+11:00) Solomon Is.",									"SolomonIs.[GMT+11:00]",	"SBT-11"},
{"(GMT+11:00) New Caledonia",								"New_Caledonia[GMT+11:00]",	"NCT-11"},
{"(GMT+12:00) Auckland, Wellington",						"Auckland,Wellington[GMT+12:00]",	"NZST-12NZDT"},
{"(GMT+12:00) Kamchatka",									"Kamchatka[GMT+12:00]",	"PETT-12PETST"},
{"(GMT+12:00) Marshall Is.",								"Marshall_Is.[GMT+12:00]",	"MHT-12"},
{"(GMT+12:00) Fiji",										"Fiji[GMT+12:00]",	"FJT-12FJST"},
{NULL,	NULL,	NULL}
};

void hiddenDalcmd(){
    int i;
    for(i = 0; dalHandler[i].name != NULL; i++){
        if(dalHandler[i].previlegebyFlag){
            dalHandler[i].previlege = NULL;
        }
    }
}

bool findInList(char *list, char *target, char *separator){
	char *buf, *ptr, *tmp_ptr = NULL;

	if(list == NULL || target == NULL || separator == NULL)
		return false;

	buf = strdup(list);
	ptr = strtok_r(buf, separator, &tmp_ptr);
	while(ptr != NULL){
		if(!strcmp(ptr, target)){
			ZOS_FREE(buf);
			return true;
		}
		ptr = strtok_r(NULL, separator, &tmp_ptr);
	}
	ZOS_FREE(buf);
	return false;
}

bool getSpecificObj(zcfg_offset_t oid, char *key, json_type type, void *value, objIndex_t *outIid, struct json_object **outJobj)
{
	objIndex_t iid;
	struct json_object *Obj = NULL;
	struct json_object *pramJobj;
	bool found = false;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &Obj) == ZCFG_SUCCESS) {
		pramJobj = json_object_object_get(Obj, key);
		if(pramJobj != NULL){
			if(type == json_type_string){
				if(!strcmp((const char*)value, json_object_get_string(pramJobj))){
					found = true;
				}
			}
			else{
				if(*(int*)value == json_object_get_int(pramJobj))
					found = true;
			}
			
			if(found){
				if(outIid) memcpy(outIid, &iid, sizeof(objIndex_t));
				if(outJobj) *outJobj = Obj;
				else json_object_put(Obj);
				break;
			}
		}
		json_object_put(Obj);
	}

	return found;
}

/*
 * parse time range format: "hh:mm-hh:mm"
*/
bool parseTimeRange(const char *timeRange, int *startHour, int *startMin, int *stopHour, int *stopMin){
	int count = 0;

	count = sscanf(timeRange, "%d:%d-%d:%d", startHour, startMin, stopHour, stopMin);
	if(count!=4){
		return false;
	}

	if(*startHour<0 || *startHour>23 || *stopHour<0 || *stopHour>23 || *startMin<0 || *startMin>59 || *stopMin<0 || *stopMin>59){
		return false;
	}

	if((*startHour*60+*startMin)>=(*stopHour*60+*stopMin)){
		return false;
	}
	
	return true;
}

/*
 * Format: "serviceName@Protocol:startPort:endPort,startPort:endPort,..."
 * e.g. http@TCP:80
 *       xboxlive@UDP:88
 *       vnc@TCP:5500,5800,5900:5910
 *       userdifine1@UDP1510,1511,1550:1580
 *       userdifine2@BOTH3330,3355,3340:3356
*/
bool parseParenService(const char *service, char *parenCtlService, int parenCtlService_len){
	int count = 0;
	int startPort, endPort;
	char *buf, *ptr, *tmp, *serviceName, *protocol, *portList;
	char partRange[16];

	buf = strdup(service);

	if((ptr = strchr(buf, '@'))==NULL)
		goto ParenServiceError;
	*ptr = '\0';
	serviceName = buf;
	protocol = ptr+1;

	if((ptr = strchr(protocol, ':'))==NULL)
		goto ParenServiceError;
	*ptr = '\0';
	portList = ptr+1;

	if(strlen(serviceName)>17)
		goto ParenServiceError;

	if(strcmp(protocol, "TCP") && strcmp(protocol, "UDP") && strcmp(protocol, "BOTH"))
		goto ParenServiceError;

	if(parenCtlService){
		if(strncmp(serviceName, "1[", 2))
			sprintf(parenCtlService, "1[%s", serviceName);
		else
			ZOS_STRCAT(parenCtlService, serviceName, parenCtlService_len);
	} 
	
	count = 0;
	ptr = strtok_r(portList, ",", &tmp);
	while(ptr != NULL){
		count++;
		if(count>6)
			goto ParenServiceError;
		
		if(sscanf(ptr, "%d:%d", &startPort, &endPort)==2){
			if(startPort < 1 || startPort > 65535 || endPort < 1 || endPort > 65535)
				goto ParenServiceError;
			else if(startPort >= endPort)
				goto ParenServiceError;

			if(parenCtlService){
				sprintf(partRange, "@%s#%d:%d", protocol, startPort, endPort);
				ZOS_STRCAT(parenCtlService, partRange, parenCtlService_len);
			}
			
		}
		else if(sscanf(ptr, "%d", &startPort)==1){
			if(startPort < 1 || startPort > 65535 )
				goto ParenServiceError;
			if(parenCtlService){
				sprintf(partRange, "@%s#%d:-1", protocol, startPort);
				ZOS_STRCAT(parenCtlService, partRange, parenCtlService_len);
			}
		}
		else{
			goto ParenServiceError;
		}
		ptr = strtok_r(NULL, ",", &tmp);
	}

	ZOS_FREE(buf);
	return true;

ParenServiceError:
	ZOS_FREE(buf);
	return false;
}

zcfgRet_t getWifiSsidList(struct json_object **Jarray){
	zcfgRet_t ret = ZCFG_SUCCESS;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	objIndex_t iid;
	struct json_object *ssidObj = NULL, *obj = NULL;
	char *ssid = NULL;
#endif

	*Jarray = json_object_new_array();

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNext(RDM_OID_WIFI_SSID, &iid, &ssidObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		ssid = (char *)json_object_get_string(json_object_object_get(ssidObj,"SSID"));
		json_object_object_add(obj, "SSID", json_object_new_string(ssid));
		json_object_array_add(*Jarray, obj);
		json_object_put(ssidObj);
	}
#else
	printf("%s : CONFIG_ZCFG_BE_MODULE_WIFI not support\n", __func__);
#endif

	return ret;
}

zcfgRet_t getCertList(struct json_object **Jarray){
	struct json_object *obj = NULL;
	objIndex_t iid;
	char path[64] = {0};

	*Jarray = json_object_new_array();

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DEV_SEC_CERT, &iid, &obj) == ZCFG_SUCCESS){	
		sprintf(path, "Security.Certificate.%u", iid.idx[0]);
		json_object_object_add(obj, "path", json_object_new_string(path));
		json_object_array_add(*Jarray, obj);
	}

	//printf("*Jarray=%s\n", json_object_to_json_string(*Jarray));
	return ZCFG_SUCCESS;
}

zcfgRet_t getEmailList(struct json_object **Jarray){
	struct json_object *obj = NULL;
	objIndex_t iid;
	*Jarray = json_object_new_array();

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_MAIL_SERVICE, &iid, &obj) == ZCFG_SUCCESS){	
		json_object_object_add(obj, "emailAddress", JSON_OBJ_COPY(json_object_object_get(obj, "EmailAddress")));
		json_object_array_add(*Jarray, obj);
	}
	return ZCFG_SUCCESS;
}


//Suppose parameter need to check is string.
zcfgRet_t dalcmdParamCheck(objIndex_t *Iid, const char *param, zcfg_offset_t oid, char *key, char *key1, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	int i=0;
	bool duplicate = false;
	if(Iid == NULL){		//	for add function use
		while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &obj) == ZCFG_SUCCESS){	
			if(!strcmp(param, json_object_get_string(json_object_object_get(obj, key)))){	
				json_object_put(obj);			
				obj = NULL;			
				strcat(replyMsg, key1);			
				strcat(replyMsg, " is duplicate.");			
				return ZCFG_INVALID_PARAM_VALUE;		
			}	
			json_object_put(obj);		
			obj = NULL;	
		}
	}
	else{					//	for edit function use, need iid
		while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &obj) == ZCFG_SUCCESS){	
		if(!strcmp(param, json_object_get_string(json_object_object_get(obj, key)))){
				for(i=0;i<6;i++){
					if(iid.idx[i] != Iid->idx[i]){
						duplicate = true;
						break;
					}
				}
				if(duplicate){
			json_object_put(obj);
			obj = NULL;
					strcat(replyMsg, key1);			
			strcat(replyMsg, " is duplicate.");
			return ZCFG_INVALID_PARAM_VALUE;
		}
			}		
		json_object_put(obj);
		obj = NULL;
	}	
	}
	return ret;
}

zcfgRet_t paramDuplicateCheck(objIndex_t *Iid, const char *param, const char *param2, zcfg_offset_t oid, char *paramkey, char *paramkey1, char *param2key, char *param2key1, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *obj = NULL;
	objIndex_t iid = {0};
	int i=0;
	bool duplicate = false;

	if (Iid == NULL) {		//	for add function use
		while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &obj) == ZCFG_SUCCESS) {
			if(!strcmp(param, json_object_get_string(json_object_object_get(obj, paramkey)))) {
				if(!strcmp(param2, json_object_get_string(json_object_object_get(obj, param2key)))) {
					json_object_put(obj);
					obj = NULL;
					strcat(replyMsg, paramkey1);
					strcat(replyMsg, " and ");
					strcat(replyMsg, param2key1);
					strcat(replyMsg, " is duplicate.");
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			json_object_put(obj);
			obj = NULL;
		}
	}
	else {					//	for edit function use, need iid
		while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &obj) == ZCFG_SUCCESS) {
		if(!strcmp(param, json_object_get_string(json_object_object_get(obj, paramkey)))) {
				for (i = 0; i < 6; i++) {
					if(iid.idx[i] != Iid->idx[i])	{
						if(!strcmp(param2, json_object_get_string(json_object_object_get(obj, param2key))))	{
							duplicate = true;
							break;
						}
					}
				}
				if (duplicate) {
					json_object_put(obj);
					obj = NULL;
					strcat(replyMsg, paramkey1);
					strcat(replyMsg, " and ");
					strcat(replyMsg, param2key1);
					strcat(replyMsg, " is duplicate.");
					return ZCFG_INVALID_PARAM_VALUE;
				}
			}
			json_object_put(obj);
			obj = NULL;
		}
	}
	return ret;
}

zcfgRet_t convertIndextoIid(int idx, objIndex_t *Iid, zcfg_offset_t oid, char *key, void *ptr, char *replyMsg)
{
	struct json_object *obj = NULL;
	struct json_object *paramObj = NULL;
	enum json_type type = json_type_null;
	int count = 1;
	bool keyMatch = false;
	bool found = false;

	if(idx <= 0) {
		strcat(replyMsg, " Index error.");
		return ZCFG_NO_SUCH_OBJECT;
	}
	while(zcfgFeObjJsonGetNextWithoutUpdate(oid, Iid, &obj) == ZCFG_SUCCESS) {
		if(key != NULL) {
			/*
			 * Conditional converting.
			 * i.g.
			 *   If idx = 5, key = "Enable", *ptr = true
			 *   Iid should be the fifth enabled object of (oid).
			 */
			keyMatch = false;
			paramObj = json_object_object_get(obj, key);
			type = json_object_get_type(paramObj);
			switch(type) {
				case json_type_boolean:
					if(json_object_get_boolean(paramObj) == *(bool *)ptr)
						keyMatch = true;
					break;

				case json_type_int:
				case json_type_uint8:
				case json_type_uint16:
				case json_type_uint32:
				case json_type_ulong:
					if(json_object_get_int(paramObj) == *(int *)ptr)
						keyMatch = true;
					break;

				case json_type_string:
				case json_type_time:
				case json_type_base64:
				case json_type_hex:
					if(!strcmp((char *)ptr, json_object_get_string(paramObj)))
						keyMatch = true;
					break;

				default:
					break;
			}

			if(keyMatch) {
				if(idx == count) {
					json_object_put(obj);
					obj = NULL;
					found = true;
					break;
				}
				count++;
			}
		}
		else {
			/* Without conditional check, only convert idx to corresponding iid. */
			if(idx == count) {
				json_object_put(obj);
				obj = NULL;
				found = true;
				break;
			}
			count++;
		}
		json_object_put(obj);
		obj = NULL;
	}
	if(!found){
		if(replyMsg)
			strcat(replyMsg, " Index error.");

		return ZCFG_NO_SUCH_OBJECT;
	}
	
	return ZCFG_SUCCESS;
}
/*
 * Convert index to iid, and this function only calculate the specified string value of the key.
 * Usage:
 * convert_index_to_iid_for_specified_string(index, &dnsRtIid, oid, "key", "value1,value2,...",replyMsg);
 * Note: The function only support string value now, and please donot calculate the value is "" (empty string).
 */
zcfgRet_t convert_index_to_iid_for_specified_string(int index, objIndex_t *iid, zcfg_offset_t oid, const char *check_key, char *check_value, char *replyMsg){

	if(index <= 0) {
		ZOS_STRCAT(replyMsg, " Index error.",127); // The size of replyMsg defined in zhttpd.c is 128
		return ZCFG_NO_SUCH_OBJECT;
	}

	struct json_object *obj = NULL;
	int count = 1;
	char *check_value_sub = NULL, *save_ptr = NULL, *check_string = NULL;
	bool found = false;

	check_string = ZOS_STRDUP(check_value);

	while((zcfgFeObjJsonGetNextWithoutUpdate(oid, iid, &obj) == ZCFG_SUCCESS)){
		found = false;
		if((NULL != check_key) && (NULL != json_object_object_get(obj,check_key))){
			check_value_sub = strtok_r(check_string, ",", &save_ptr);

			while(check_value_sub != NULL){
				const char *key_value = json_object_get_string(json_object_object_get(obj,check_key));
				if(!strncmp(key_value, check_value_sub, strlen(key_value))){
					found = true;
					break;
				}
				check_value_sub = strtok_r(NULL, ",", &save_ptr);
			}
		}
		zcfgFeJsonObjFree(obj);

		if(found){
			if(index == count)
				break;
			count++;
		}
	}

	ZOS_FREE(check_string);
	if(obj){
		json_object_put(obj);
		obj = NULL;
	}

	return ZCFG_SUCCESS;
}

/*
 * return WAN and LAN list in Jason Array.
 * Format:Jarray.i.Name = Name
 *           Jarray.i.IpIfacePath = IP.Interface.i
 *           Jarray.i.PppIfacePath = PPP.Interface.i  (For PPPoE, PPPoA only)
 *           Jarray.i.Type : LAN|WAN
 *           Jarray.i.LinkType = ATM|PTM|ETH|PON|USB (For WAN only)
 *           Jarray.i.VpiVci = vpi/vci (For ATM only)
 *           Jarray.i.VLANPriority (For WAN only)
 *           Jarray.i.VLANID (For WAN only)
 *           Jarray.i.ConnectionType = IP_Routed|IP_Bridged (For WAN only)
 *           Jarray.i.Encapsulation = IPoE|IPoA|PPPoE|PPPoA (For WAN only, NUL if LinkType is USB))
 *           Jarray.i.BindToIntfGrp = true|false (For WAN only)
 *           Jarray.i.LowerLayerUp = true|false (For  IPoE, PPPoE only, other WAN LowerLayerUp value may not correct)
 *           Jarray.i.WANConnectionReady = true|false (For  WAN only)
 *           Jarray.i.DefaultGateway = true|false (For  WAN only)
 *           Jarray.i.IPv6DefaultGateway = true|false (For  WAN only)
 *           Jarray.i.Group_WAN_IpIface = IP.Interface.i,IP.Interface.i,IP.Interface.i,... (For LAN only)
 *           Jarray.i.BridgingBrPath = Bridging.Bridge.i (For LAN only)


 *           Jarray.i.Enable = true|false (For WAN only)
 *           Jarray.i.IPv4Enable = true|false (For WAN only)
 *           Jarray.i.IPv6Enable = true|false (For WAN only)
 *	       Jarray.i.NatEnable = true|false (For WAN only)
 *           Jarray.i.IGMPEnable = true|false (For WAN only)
 *           Jarray.i.MLDEnable = true|false (For WAN only)

 */

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
int UpWanTypeGet1()
{
     objIndex_t ethIfaceIid;	
	 objIndex_t dslChannelIid;
	 struct json_object *ethIfaceObj = NULL;
	 struct json_object *dslChannelObj = NULL;
	 const char *ethType;
	 const char *ethStatus;
	 const char *dslType;
	 const char *dslStatus;
	 int WanConnectType = 0;
     
	 IID_INIT(ethIfaceIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj) == ZCFG_SUCCESS) {
		 ethType = json_object_get_string(json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		 ethStatus = json_object_get_string(json_object_object_get(ethIfaceObj, "Status"));
		 if(!strcmp(ethType,"ETHWAN") && !strcmp(ethStatus,"Up")){
		 	WanConnectType = 1;
#if 0 // winnie 20191007
			json_object_put(ethIfaceObj);
#endif
			break;
		 }
#if 0 // winnie 20191007
		 else
		 	json_object_put(ethIfaceObj);
#endif
	 }
	 
	 IID_INIT(dslChannelIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &dslChannelIid, &dslChannelObj) == ZCFG_SUCCESS) {
		 dslType = json_object_get_string(json_object_object_get(dslChannelObj, "LinkEncapsulationUsed"));
		 dslStatus = json_object_get_string(json_object_object_get(dslChannelObj, "Status"));
		 if(strstr(dslType,"ATM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 2;
#if 0 // winnie 20191007
			json_object_put(dslChannelObj);
#endif
			break;
		 }
		 else if(strstr(dslType,"PTM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 3;
#if 0 // winnie 20191007
			json_object_put(dslChannelObj);
#endif
			break;
		 }
#if 0 // winnie 20191007
		 else
		 	json_object_put(ethIfaceObj);		 
#endif
	 }	
	 	
	 return WanConnectType;
	 
}
#endif

zcfgRet_t getWanLanList(struct json_object **Jarray){
	struct json_object *ipIfaceJarray, *pppIfaceJarray, *vlanTermJarray, *ethLinkJarray, *ethIfaceJarray, *bridgeBrJarray;
	struct json_object *ipIfaceObj, *pppIfaceObj = NULL, *vlanTermObj = NULL, *ethLinkObj = NULL, *ethIfaceObj = NULL, *bridgeBrObj, *routerObj;
	struct json_object *v4AddrJarray, *v6AddrJarray, *v4AddrObj, *v6AddrObj;
	struct json_object *natIntfJarray, *natIntfObj, *igmpObj, *mldObj;
	struct json_object *atmLinkJarray, *atmLinkObj;
	struct json_object *Jobj = NULL;
	char ifacePath[32] = {0}, BridgingBrPath[32] = {0};
	int len, len2, i, j, X_ZYXEL_ConcurrentWan = 0;
	int X_ZYXEL_HideByPriviledge = 0;
	unsigned char idx;
	#ifdef ZYXEL_TAAAB_CUSTOMIZATION
	const char *X_TAAAB_SrvName = NULL;
	#endif
	const char *X_ZYXEL_BridgeName, *X_ZYXEL_SrvName, *X_ZYXEL_ConnectionType, *X_ZYXEL_IfName, *X_ZYXEL_Group_WAN_IpIface, *LowerLayers;
	const char *Status, *ipIfaceStatus, *IPAddress, *X_ZYXEL_ActiveDefaultGateway, *X_ZYXEL_ActiveV6DefaultGateway;
	//const char *Username, Password;
	char intfGrpWANList[512] = {0};
	char *intf = NULL, *tmp_ptr = NULL;
	const char *IpIfacePath = NULL, *natInterface = NULL, *igmpInterface = NULL, *mldInterface = NULL;
#if defined(ZYXEL_SUPPORT_LAN_AS_ETHWAN)
	int hideEth = 0;
	struct json_object *jarr = json_object_new_array();
	if (ZCFG_SUCCESS == zcfgFeDalHandler("ethwanlan", "GET", NULL, jarr, NULL))
	{
		struct json_object *jobj = json_object_array_get_idx(jarr, 0);
		if (jobj && !json_object_get_boolean(json_object_object_get(jobj, "Enable")))
			hideEth += 1;
	}
	json_object_put(jarr);
#endif
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	int upwantype = 0;
#endif
	bool X_ZYXEL_Default_WAN = false;

	zcfgFeWholeObjJsonGet("Device.IP.Interface.", &ipIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.PPP.Interface.", &pppIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.VLANTermination.", &vlanTermJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.Link.", &ethLinkJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.Interface.", &ethIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.ATM.Link.", &atmLinkJarray);
	zcfgFeWholeObjJsonGet("Device.Bridging.Bridge.", &bridgeBrJarray);
	zcfgFeWholeObjJsonGet("Device.Routing.Router.1.", &routerObj);

	zcfgFeWholeObjJsonGet("Device.NAT.InterfaceSetting.", &natIntfJarray);
	zcfgFeWholeObjJsonGet("Device.X_ZYXEL_IGMP.", &igmpObj);
	zcfgFeWholeObjJsonGet("Device.X_ZYXEL_MLD.", &mldObj);

	if(ipIfaceJarray==NULL||pppIfaceJarray==NULL||vlanTermJarray==NULL||
		ethLinkJarray==NULL||ethIfaceJarray==NULL||atmLinkJarray==NULL||bridgeBrJarray==NULL||
		routerObj==NULL||natIntfJarray==NULL||igmpObj==NULL||mldObj==NULL){

		json_object_put(ipIfaceJarray);
		json_object_put(pppIfaceJarray);
		json_object_put(vlanTermJarray);
		json_object_put(ethLinkJarray);
		json_object_put(ethIfaceJarray);
		json_object_put(atmLinkJarray);
		json_object_put(bridgeBrJarray);
		json_object_put(routerObj);
		json_object_put(natIntfJarray);
		json_object_put(igmpObj);
		json_object_put(mldObj);

		return ZCFG_INTERNAL_ERROR;
	}
	X_ZYXEL_ActiveDefaultGateway = Jgets(routerObj, "X_ZYXEL_ActiveDefaultGateway");
	X_ZYXEL_ActiveV6DefaultGateway = Jgets(routerObj, "X_ZYXEL_ActiveV6DefaultGateway");
	igmpInterface = Jgets(igmpObj, "Interface");
	mldInterface = Jgets(mldObj, "Interface");
	if(X_ZYXEL_ActiveDefaultGateway == NULL) X_ZYXEL_ActiveDefaultGateway = "";
	if(X_ZYXEL_ActiveV6DefaultGateway == NULL) X_ZYXEL_ActiveV6DefaultGateway = "";
	if(igmpInterface == NULL) igmpInterface = "";
	if(mldInterface == NULL) mldInterface = "";

	*Jarray = json_object_new_array();

	len = json_object_array_length(ipIfaceJarray);
	for(i=0;i<len;i++){
		ipIfaceObj = json_object_array_get_idx(ipIfaceJarray, i);
		X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
		X_ZYXEL_IfName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName"));
		X_ZYXEL_ConnectionType = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType"));
		X_ZYXEL_ConcurrentWan = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConcurrentWan"));
		X_ZYXEL_HideByPriviledge = json_object_get_int(json_object_object_get(ipIfaceObj, "X_ZYXEL_HideByPriviledge"));
		LowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
		X_ZYXEL_Group_WAN_IpIface = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
		X_ZYXEL_Default_WAN = json_object_get_boolean(json_object_object_get(ipIfaceObj, "X_ZYXEL_Default_WAN"));
		sprintf(ifacePath, "IP.Interface.%u", i+1);
		if(X_ZYXEL_SrvName==NULL || X_ZYXEL_ConnectionType==NULL || LowerLayers==NULL){
			continue;
		}

#ifndef ZyXEL_WEB_GUI_SHOW_WWAN
        if(strcmp(X_ZYXEL_SrvName,"WWAN") == 0)
        {
            continue;
        }
#endif

		Jobj = json_object_new_object();
		json_object_object_add(Jobj, "IpIfacePath", json_object_new_string(ifacePath));
		json_object_object_add(Jobj, "ipiIndex", json_object_new_int(i+1));
		json_object_object_add(Jobj, "Name", json_object_new_string(X_ZYXEL_SrvName));
		json_object_object_add(Jobj, "IfName", json_object_new_string(X_ZYXEL_IfName));
#ifdef ZYXEL_TAAAB_CUSTOMIZATION
		X_TAAAB_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_TT_SrvName"));
		if(strstr(X_TAAAB_SrvName,"_DSL") != NULL ){
		    json_object_object_add(Jobj, "BindName", json_object_new_string(X_TAAAB_SrvName));
			json_object_object_add(Jobj, "BindType", json_object_new_string("DSL"));
		}
		else if(strstr(X_TAAAB_SrvName,"_WAN") != NULL ){
		    json_object_object_add(Jobj, "BindName", json_object_new_string(X_TAAAB_SrvName));
			json_object_object_add(Jobj, "BindType", json_object_new_string("ETH"));
		}
#ifdef TAAAB_HGW
										char *tmp_string = NULL;
										char newName[128] = {0};
										if(strcmp(X_ZYXEL_SrvName,"Default") == 0 || strcmp(X_ZYXEL_SrvName,"WWAN") == 0)
											json_object_object_add(Jobj, "HGWStyleName", json_object_new_string(X_ZYXEL_SrvName));	
										else{
										tmp_string = strrchr(X_TAAAB_SrvName,'_');
										strncpy(newName, X_TAAAB_SrvName, strlen(X_TAAAB_SrvName)-strlen(tmp_string));
										newName[strlen(X_TAAAB_SrvName)-strlen(tmp_string)] = '\0';
										json_object_object_add(Jobj, "HGWStyleName", json_object_new_string(newName));	
										}
#endif
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
		upwantype = UpWanTypeGet1();
		json_object_object_add(Jobj, "upwantype", json_object_new_int(upwantype));	
#endif

		json_object_object_add(Jobj, "X_ZYXEL_HideByPriviledge", json_object_new_int(X_ZYXEL_HideByPriviledge));
		json_object_object_add(Jobj, "Type", json_object_new_string("WAN"));
		json_object_object_add(Jobj, "VpiVci", json_object_new_string(""));
		Jaddi(Jobj, "VLANPriority", -1);
		Jaddi(Jobj, "VLANID", -1);
		json_object_object_add(Jobj, "ConnectionType", json_object_new_string(X_ZYXEL_ConnectionType));
		json_object_object_add(Jobj, "ConcurrentWan", json_object_new_int(X_ZYXEL_ConcurrentWan));		
		json_object_object_add(Jobj, "BindToIntfGrp", json_object_new_boolean(false));
		json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(false));
		json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(false));
		json_object_object_add(Jobj, "DefaultWAN", json_object_new_boolean(X_ZYXEL_Default_WAN));
		Jadds(Jobj, "PppIfacePath", "");
		//json_object_object_add(Jobj, "pppUsername", json_object_new_string(""));
		//json_object_object_add(Jobj, "pppPassword", json_object_new_string(""));

		if(Jgetb(ipIfaceObj, "Enable"))
			Jaddb(Jobj, "Enable", true);
		else
			Jaddb(Jobj, "Enable", false);
		if(Jgetb(ipIfaceObj, "IPv4Enable"))
			Jaddb(Jobj, "IPv4Enable", true);
		else
			Jaddb(Jobj, "IPv4Enable", false);
		if(Jgetb(ipIfaceObj, "IPv6Enable"))
			Jaddb(Jobj, "IPv6Enable", true);
		else
			Jaddb(Jobj, "IPv6Enable", false);

		//Get DefaultGateway, IPv6DefaultGateway
		if(findInList((char *)X_ZYXEL_ActiveDefaultGateway, ifacePath, ","))
			json_object_object_add(Jobj, "DefaultGateway", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "DefaultGateway", json_object_new_boolean(false));

		if(findInList((char *)X_ZYXEL_ActiveV6DefaultGateway, ifacePath, ","))
			json_object_object_add(Jobj, "IPv6DefaultGateway", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "IPv6DefaultGateway", json_object_new_boolean(false));

		if(findInList((char *)igmpInterface, ifacePath, ","))
			json_object_object_add(Jobj, "IGMPEnable", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "IGMPEnable", json_object_new_boolean(false));

		if(findInList((char *)mldInterface, ifacePath, ","))
			json_object_object_add(Jobj, "MLDEnable", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "MLDEnable", json_object_new_boolean(false));

		//Get NatEnable
		Jaddb(Jobj, "NatEnable", false);
		len2 = json_object_array_length(natIntfJarray);
		for(j=0;j<len2;j++){
			natIntfObj = json_object_array_get_idx(natIntfJarray, j);
			if(isEmptyIns(natIntfObj))
				continue;
			natInterface = Jgets(natIntfObj, "Interface");
			if(natInterface==NULL) continue;
			if(!strcmp(natInterface, ifacePath)){
				if(Jgetb(natIntfObj, "Enable"))
					Jaddb(Jobj, "NatEnable", true);
				break;
			}
		}

		ipIfaceStatus = json_object_get_string(json_object_object_get(ipIfaceObj, "Status"));
		if(ipIfaceStatus != NULL && !strcmp(ipIfaceStatus, "Up")){  //|| !strcmp(ipIfaceStatus, "LowerLayerDown")
			v4AddrJarray = Jget(ipIfaceObj, "IPv4Address");
			if(v4AddrJarray != NULL){
				len2 = json_object_array_length(v4AddrJarray);
				for(j=0;j<len2;j++){
					v4AddrObj = json_object_array_get_idx(v4AddrJarray, j);
					if(isEmptyIns(v4AddrObj))
						continue;
					IPAddress = json_object_get_string(json_object_object_get(v4AddrObj,"IPAddress"));
					Status = json_object_get_string(json_object_object_get(v4AddrObj, "Status"));
					if(IPAddress != NULL && Status != NULL && strcmp(IPAddress, "") && !strcmp(Status, "Enabled")){
						json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(true));
						break;
					}
				}
			}
			if(!Jgetb(Jobj, "WANConnectionReady")){
			v6AddrJarray = Jget(ipIfaceObj, "IPv6Address");
			if(v6AddrJarray != NULL){
				len2 = json_object_array_length(v6AddrJarray);
				for(j=0;j<len2;j++){
					v6AddrObj = json_object_array_get_idx(v6AddrJarray, j);
					if(isEmptyIns(v6AddrObj))
						continue;
					IPAddress = json_object_get_string(json_object_object_get(v6AddrObj,"IPAddress"));
					Status = json_object_get_string(json_object_object_get(v6AddrObj, "Status"));
					if(IPAddress != NULL && Status != NULL && strcmp(IPAddress, "") && !strcmp(Status, "Enabled")){
						json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(true));
						break;
					}
				}
			}
			}
		}


		int count = 0;
		const char *HigherLayer = "IP.Interface.";
		while(count < 4){
			count++;
			if(LowerLayers == NULL){
				json_object_put(Jobj);
				Jobj = NULL;
				break;
			}

			if(!strncmp(HigherLayer, "IP.Interface.", 13)){
				if(!strncmp(LowerLayers, "PPP.Interface.", 14)){
					sscanf(LowerLayers, "PPP.Interface.%hhu", &idx);
					pppIfaceObj = json_object_array_get_idx(pppIfaceJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(pppIfaceObj, "LowerLayers"));
					
					replaceParam(Jobj, "IfName", pppIfaceObj, "X_ZYXEL_IfName");
				}
				else if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));
					replaceParam(Jobj, "VLANPriority", vlanTermObj, "X_ZYXEL_VLANPriority");
					replaceParam(Jobj, "VLANID", vlanTermObj, "VLANID");
				}
				else if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &idx);
					ethLinkObj = json_object_array_get_idx(ethLinkJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9) && !strcmp("IP_Routed", X_ZYXEL_ConnectionType)){
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoA"));
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));

					sscanf(LowerLayers, "ATM.Link.%hhu", &idx);
					atmLinkObj = json_object_array_get_idx(atmLinkJarray, idx-1);
					replaceParam(Jobj, "VpiVci", atmLinkObj, "DestinationAddress");
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp(HigherLayer, "PPP.Interface.", 14)){
				Jadds(Jobj, "PppIfacePath", HigherLayer);
				//replaceParam(Jobj, "pppUsername", pppIfaceObj, "Username");
				//replaceParam(Jobj, "pppPassword", pppIfaceObj, "Password");
						
				if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoE"));
					replaceParam(Jobj, "VLANPriority", vlanTermObj, "X_ZYXEL_VLANPriority");
					replaceParam(Jobj, "VLANID", vlanTermObj, "VLANID");
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					Status = json_object_get_string(json_object_object_get(pppIfaceObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));

					sscanf(LowerLayers, "ATM.Link.%hhu", &idx);
					atmLinkObj = json_object_array_get_idx(atmLinkJarray, idx-1);
					json_object_object_add(Jobj, "Encapsulation",  JSON_OBJ_COPY(json_object_object_get(atmLinkObj, "LinkType")));
					replaceParam(Jobj, "VpiVci", atmLinkObj, "DestinationAddress");
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					Status = json_object_get_string(json_object_object_get(pppIfaceObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25)){
				if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &idx);
					ethLinkObj = json_object_array_get_idx(ethLinkJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
					replaceParam(Jobj, "VLANPriority", vlanTermObj, "X_ZYXEL_VLANPriority");
					replaceParam(Jobj, "VLANID", vlanTermObj, "VLANID");
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.Link.", HigherLayer, 14)){
				if(!strncmp("ATM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));

					sscanf(LowerLayers, "ATM.Link.%hhu", &idx);
					atmLinkObj = json_object_array_get_idx(atmLinkJarray, idx-1);
					replaceParam(Jobj, "VpiVci", atmLinkObj, "DestinationAddress");
					break;
				}
				else if(!strncmp("PTM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PTM"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("Ethernet.Interface.", LowerLayers, 19)){
					sscanf(LowerLayers, "Ethernet.Interface.%hhu", &idx);
					ethIfaceObj = json_object_array_get_idx(ethIfaceJarray, idx-1);
					if(json_object_get_boolean(json_object_object_get(ethIfaceObj, "X_ZYXEL_SFP")) == false)
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					else
						json_object_object_add(Jobj, "LinkType", json_object_new_string("SFP"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("Optical.Interface.", LowerLayers, 18)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PON"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
				else if(!strncmp("Cellular.Interface.", LowerLayers, 19)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("CELL"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
#endif
				else if(!strncmp("Bridging.Bridge.", LowerLayers, 16)){
					sscanf(LowerLayers, "Bridging.Bridge.%hhu.Port.1", &idx);
					bridgeBrObj = json_object_array_get_idx(bridgeBrJarray, idx-1);
					X_ZYXEL_BridgeName = json_object_get_string(json_object_object_get(bridgeBrObj, "X_ZYXEL_BridgeName"));
					if(X_ZYXEL_BridgeName == NULL){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					if(X_ZYXEL_Group_WAN_IpIface == NULL)
						X_ZYXEL_Group_WAN_IpIface = "";
						
					sprintf(BridgingBrPath, "Bridging.Bridge.%u", idx);
					json_object_object_add(Jobj, "Name", json_object_new_string(X_ZYXEL_BridgeName));
					json_object_object_add(Jobj, "Type", json_object_new_string("LAN"));
					json_object_object_add(Jobj, "BridgingBrPath", json_object_new_string(BridgingBrPath));
					json_object_object_add(Jobj, "Group_WAN_IpIface", json_object_new_string(X_ZYXEL_Group_WAN_IpIface));
					strcat(intfGrpWANList, X_ZYXEL_Group_WAN_IpIface);
					strcat(intfGrpWANList, ",");
					break;
				}
				else if(!strcmp("", LowerLayers)){
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
		}
		
#if defined(ZYXEL_SUPPORT_LAN_AS_ETHWAN)
		if (Jobj && hideEth && !strcmp("WAN", json_object_get_string(json_object_object_get(Jobj, "Type")))
				&& !strcmp("ETH", json_object_get_string(json_object_object_get(Jobj, "LinkType"))))
		{
			json_object_put(Jobj);
			Jobj = NULL;
		}
#endif
		
		if(Jobj != NULL)
			json_object_array_add(*Jarray, Jobj);
	}

	if(strcmp(intfGrpWANList, "")){
		intf = strtok_r(intfGrpWANList, ",", &tmp_ptr);
		while(intf != NULL){
			if(*intf != '\0'){
				len = json_object_array_length(*Jarray);
				for(i=0;i<len;i++){
					Jobj = json_object_array_get_idx(*Jarray, i);
					IpIfacePath = json_object_get_string(json_object_object_get(Jobj, "IpIfacePath"));
					if(!strcmp(IpIfacePath, intf)){
						json_object_object_add(Jobj, "BindToIntfGrp", json_object_new_boolean(true));
					}
				}
			}
			intf = strtok_r(NULL, ",", &tmp_ptr);
		}
	}
	//printf("*Jarray=%s\n", json_object_to_json_string(*Jarray));

	json_object_put(ipIfaceJarray);
	json_object_put(pppIfaceJarray);
	json_object_put(vlanTermJarray);
	json_object_put(ethLinkJarray);
	json_object_put(ethIfaceJarray);
	json_object_put(atmLinkJarray);
	json_object_put(bridgeBrJarray);
	json_object_put(routerObj);
	json_object_put(natIntfJarray);
	json_object_put(igmpObj);
	json_object_put(mldObj);
		
	return ZCFG_SUCCESS;
}

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER)
char *base64_decode(unsigned char *input, int *length)
{
	BIO *b64, *bmem;
	BUF_MEM *bptr;
	char *buffer;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf(input, -1);
	bmem = BIO_push(b64, bmem);
	
	BIO_get_mem_ptr(bmem, &bptr); 
	buffer= (char *)malloc(bptr->length+1);
	memset(buffer, 0, bptr->length);
	*length = bptr->length;
	BIO_read(bmem, buffer, bptr->length);
	BIO_free_all(bmem);

	return buffer;
}

char *base64_encode(unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;
	char *buffer;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	buffer= (char *)malloc(bptr->length+1);
	memcpy(buffer, bptr->data, bptr->length);
	buffer[bptr->length] = 0;

	BIO_free_all(b64);

	return buffer;
}

void cleanCurrUser(char *pAuthName, char *pAuthPassword)
{
	char tempbuf[512] = {0}, fsuser[512] = {0}, fspasswd[512] = {0};
	FILE *userfstmp, *userfs;
	char cmd[128];
	char *encodepAuthName = NULL;
	
	if(pAuthName != NULL)
		encodepAuthName = base64_encode((unsigned char*)pAuthName, strlen(pAuthName));
	
	userfs = fopen( "/var/userfile", "r+");
	userfstmp = fopen("/var/userfile.tmp","a+");
	if(userfs == NULL || userfstmp == NULL){
		if(userfs != NULL)
			fclose(userfs);
		if(userfstmp != NULL)
			fclose(userfstmp);
		printf("Open file is failed!!");
	}
	else{
		while(fscanf(userfs, "%[^\n]\n", tempbuf) > 0){
			if(sscanf(tempbuf, "%[^ ] %[^ \n]", fsuser, fspasswd) == 2){				
				if((strcmp(fsuser, encodepAuthName) != 0) || (strcmp(fspasswd, pAuthPassword) != 0)){
					fprintf(userfstmp, "%s\n",tempbuf);
				}
			}
		}
		fclose(userfs);
		fclose(userfstmp);

		snprintf(cmd,sizeof(cmd),"cp /var/userfile.tmp /var/userfile");
		system(cmd);
		unlink("/var/userfile.tmp");
	}
}

zcfgRet_t getmultiuserList(struct json_object **Jarray, const char *userName){
	struct json_object *B_PAGEJarray = NULL;
	struct json_object *Internet_DSLJarray = NULL;
    struct json_object *Internet_WANJarray = NULL;
    struct json_object *IPTV_DSLJarray = NULL;
    struct json_object *IPTV_WANJarray = NULL;
	struct json_object *ipinterfaceJarray = NULL;
	struct json_object *Jobj = NULL;
    struct json_object *AdvancedJarray = NULL;
	struct json_object *GeneralJarray = NULL;
	struct json_object *GuestJarray = NULL;
	struct json_object *WPSJarray = NULL;
	struct json_object *WMMJarray = NULL;	
	struct json_object *OthersJarray = NULL;
	struct json_object *LANSetupJarray = NULL;	
	struct json_object *RoutingJarray = NULL;
	struct json_object *ALGJarray = NULL;	
	struct json_object *DosJarray = NULL;	
	struct json_object *TimeJarray = NULL;	
	struct json_object *LogSettingJarray = NULL;	
	struct json_object *FWUpgradeJarray = NULL;	
	struct json_object *BackupJarray = NULL;	
	struct json_object *MGMTJarray = NULL;
	struct json_object *useraccountobj = NULL;
	struct json_object *RemoteAccessJarray = NULL;
	struct json_object *LocalAccessJarray = NULL;
	struct json_object *ManagementJarray = NULL;
	struct json_object *Acs_DslJarray = NULL;
	struct json_object *Acs_WanJarray = NULL;
	struct json_object *EM_Remote = NULL;
	struct json_object *EM_local = NULL;
	struct json_object *LogJarray = NULL;
	struct json_object *portmirrorJarray = NULL;
	struct json_object *packetcaptureJarray = NULL;
	struct json_object *vlanJarray = NULL;
	struct json_object *Tr069Jarray = NULL;
	struct json_object *EDNSPageJarray = NULL;
	objIndex_t LogIid,portmirrorIid,packetcaptureIid;
	objIndex_t DosIid,TimeIid,LogSettingIid,FWUpgradeIid,BackupIid,MGMTIid;
	objIndex_t WMMIid,OthersIid,LANSetupIid,RoutingIid,ALGIid;
	objIndex_t ipinterfaceIid,AdvancedIid,GeneralIid,GuestIid,WPSIid;
	objIndex_t B_PAGEIid,Internet_DSLIid,Internet_WANIid,IPTV_DSLIid,IPTV_WANIid;
	objIndex_t useraccountIid,vlanIid,Acs_DslIid,Acs_WanIid,ManagementIid;
	objIndex_t RemoteAccessIid,LocalAccessIid,EMIid,Tr069Iid,EDNSPageIid;
    int broadbandv = 1,Internet_DSLv = 1,Internet_WANv = 1;
    int IPTV_DSLv = 1,IPTV_WANv = 1,vlanv = 2;
	int Advancedv = 2,Generalv = 1,Guestv = 1;
	int WPSv = 1,WMMv = 2,Othersv = 1;
	int LANSetupv = 1,Routingv = 2,ALGv = 2;
	int Dosv = 2,Timev = 2,LogSettingv = 2;
	int FWUpgradev = 1,Backupv = 1,MGMTv = 1;
	int Acs_Dslv = 1,Acs_Wanv = 1,Managementv = 1;
	int Logv = 2,portmirrorv = 2,packetcapturev = 2;
	int Tr069v = 2,EDNSv = 2;
	bool LogLimitation = false,portmirrorLimitation = false,packetcaptureLimitation = false;
	bool LogLock = false,portmirrorLock = false,packetcaptureLock = false;
	bool broadbandLimitation = true,broadbandLock = false;
	bool Internet_DSLLimitation = true,Internet_DSLLock = false;
	bool Internet_WANLimitation = true,Internet_WANLock = false;
	bool IPTV_DSLLimitation = true,IPTV_DSLLock = false;
	bool IPTV_WANLimitation = true,IPTV_WANLock = false;	
	bool vlanLimitation = false,vlanLock = false;
	bool AdvancedLimitation = false,GeneralLimitation = false,GuestLimitation = false;
	bool AdvancedLock = false,GeneralLock = false,GuestLock = false;	
	bool WPSLimitation = false,WMMLimitation = false,OthersLimitation = true;
	bool WPSLock = false,WMMLock = false,OthersLock = false;
	bool LANSetupLimitation = true,RoutingLimitation = false,ALGLimitation = false;
	bool LANSetupLock = false,RoutingLock = false,ALGLock = false;
	bool DosLimitation = false,TimeLimitation = false,LogSettingLimitation = false;
	bool DosLock = false,TimeLock = false,LogSettingLock = false;
	bool FWUpgradeLimitation = false,BackupLimitation = false,MGMTLimitation = true;
	bool FWUpgradeLock = false,BackupLock = false,MGMTLock = false;
	bool Acs_DslLimitation = true,Acs_WanLimitation = true,ManagementLimitation = true;
	bool Acs_DslLock = false,Acs_WanLock = false,ManagementLock = false;
	bool Tr069Limitation = false,Tr069Lock = false;
	bool EM_RmoteAccess = false, EM_LocalAccess = false;
	bool EDNSLimitation = false,EDNSLock = false;
	int curloginLevel = 0,dislevel = 0,Internet_DSLpass = 1;
	char wanname[64] = {0},tr69wanname[64] = {0},password[256] = {0},defpassword[256] = {0};
	//char *decopassword = NULL;
	int TAAAGMultiUser = 1,Internet_WANpass = 1,IPTV_DSLpass = 1;
	int IPTV_WANpass = 1,Acs_Dslpass = 1,Acs_Wanpass = 1,ManagementWANpass = 1;
	bool iswanenable= true,issrv_name = true, istypeSelect = true;
	bool ismodeSelect = true,issrvLinkTypeSelect = true,isipModeSelect = true;
	bool ispppUsername = true,ispppPassword = true,isshowPppPassword = true;
	bool isConnectionTrigger = true,ispppIdleTimeoutValue = true,iswbenable = true;
	bool isvpi = true,isvci = true,isencapSelect = true;
	bool issrvCategorySelect = true,isenblVlan = true,isvlanPrio = true;
	bool isvlanId = true,ismtuValue = true,isipaddress = true;
	bool isipAddrValue = true,isipMask = true,isgwValue = true;
	bool isdns = true,isipDns1 = true,isipDns2 = true,isnatEnable = true;
	bool isfullcone_nat = true,isigmpEnable = true,isdefaultGwEnable = true;
	bool isipv6address=true,isipv6AddrValue=true,isipv6PrefixLenVal=true;
	bool isipv6NextHopVal=true,isipv6dns=true,isipv6Dns1=true;
	bool isipv6Dns2=true,ismldEnable=true,isdefaultGw6Enable=true;
	bool isiapdEnable=true,isianaEnable=true,isfirewall=true;
	bool isbb_option42=true,isbb_option43=true,isbb_option120=true;
	bool isbb_option60=true,isbb_vendorIdValue=true,isbb_option61=true;
	bool isbb_option121=true,isbb_iaidValue=true,isbb_duidValue=true;
	bool isbb_option125=true;
	bool dslBonding = true,enblAdslPTM = true,enblAnnexM = true;	
	bool enblPhyrUSDS = true,enblSRA = true,enblcancelsubmit = true;
	bool enblBand = true,enblsideband = true,enblMaxBandwidth = true;
	bool enblEnable = true,enblsecurityLevel = true;
	bool enblChannel = true,enblSsid = true,enblhide = true;
	bool enblwmf = true,enblwepkey = true,enblWEPEncryption = true;
	bool enblSecurity = true,enblwpakey = true,enblGecancelsubmit = true;
	bool isenableWPS = true,isWPSBtn = true,ispinVal = true;
	bool isRegister = true,isenableMethod3 = true,isreleaseCfg = true;
	bool isgenNewPin = true,enblWPScancelsubmit = true;
	bool isenableWMMAPSD = true,enblWMMcancelsubmit = true;
	bool isRtsThrshld = true,isFrgThrshld = true,isPwrPcnt = true;
	bool isBcnIntvl = true,isDtmIntvl = true,isMode = true,isProtect = true;
	bool isPreamble = true,isAutoChannelTimer = true,enblOtherscancelsubmit = true;
	bool isIpAddress = true,isSubnetMask = true,isIgmpSnp = true;
	bool isigmpMode = true,isenblDhcpSrv = true;
	bool isdhcpEthStart = true,isdhcpEthEnd = true,isDhcpAutoReserve = true;
	bool isday = true,ishour = true,isminute = true;
	bool isdhcpRelayServer = true,isdnsType = true,isdns_server_1 = true;
	bool isdns_server_2 = true,isenblIpv6Lan = true,enblLANSetupcancelsubmit = true;
	bool enblDNSRtadd = true,enblRIPcancelsubmit = true;	
	bool enblversion = true,enblmode = true,enblEnbl = true;
	bool enblPolicyRtadd = true,enblStaticRtadd = true,enblNATALG = true;	
	bool enblSIPALG = true,enblRTSPALG = true,enblALGcancelsubmit = true;
	bool enblDosEnable = true,enblDoscancelsubmit = true;
	bool enblshowTime = true,enblshowDate = true,enblntpServer = true;
	bool enblserveraddr = true,enblTimeZone = true,enbldaylightsave = true;
	bool enblsdayrule1 = true,enblstartday = true,enblsdayrule2 = true;
	bool enblstartdayofweek = true,enblWeekDay2 = true,enblstartmonth = true;
	bool enblstarthour = true,enblstartMin = true,enbledayrule1 = true;
	bool enblendday = true,enbledayrule2= true,enblendweek = true;
	bool enblendDayofWeek = true,enblendmonth = true,enblendhour = true;
	bool enblendmin = true,enblTimecancelsubmit = true,enblsyslog = true;
	bool enblmodeSelect = true,enblserver_addr = true,enbludp_port = true;
	bool enbllogMail = true,enbllogMailSubject = true,enblsecurityLogMailSubject = true;
	bool enbllogMailAddress = true,enblalertMailAddress = true,enblalarminterval = true;
	bool enbllogCat0 = true,enbllogCat1 = true,enblLogSettingcancelsubmit = true;
	bool enblfilename = true,enblFWUpgradeupload = true;	
	bool enblBackup = true,enblBackupfilename = true,enblBackupupload = true;
	bool enblresetbtn = true,enbladdNewWanBtn = true,enblbtnwwanupload = true;
	bool enblLanid = true,enblwwanfile = true,enblvlanaddnew = true;
	bool enblwanid = true,enblport = true,enbltelnetLanid = true;
	bool enbltelnetwanid = true,enbltelnetport = true,enblMGMTcancelsubmit = true;
	bool isip6rdEnable = true,is6rdaddressEnable = true,enblwlanid = true;
	bool enbltelnetwlanid = true,enableEdit = true,enabledelete = true;
	bool isgroupname = true,enblExtendMore = true,enblEdit = true;
	bool isAddFromTr69 = false;
	bool enbllevel = true,enblcategory = true,enblclear = true;
	bool enblrefresh = true,enblexport = true,enblemail = true;
	bool RemoteEnable = false,LocalEnable = true;
	bool enbllocallan = true,isMacaddressEnable = true;

	IID_INIT(B_PAGEIid);
	if ( zcfgFeObjJsonGet(RDM_OID_BROADBAND, &B_PAGEIid,&B_PAGEJarray) == ZCFG_SUCCESS)
	{
		broadbandv = Jgeti(B_PAGEJarray, "VisibilityLevel");
		broadbandLimitation = Jgetb(B_PAGEJarray, "Limitation");
		broadbandLock = Jgetb(B_PAGEJarray, "Lock");
	}
	IID_INIT(Internet_DSLIid);
	if ( zcfgFeObjJsonGet(RDM_OID_INTERNET_DSL, &Internet_DSLIid,&Internet_DSLJarray) == ZCFG_SUCCESS)
	{
		Internet_DSLv = Jgeti(Internet_DSLJarray, "VisibilityLevel");
		Internet_DSLLimitation = Jgetb(Internet_DSLJarray, "Limitation");
		Internet_DSLLock = Jgetb(Internet_DSLJarray, "Lock");
	}
	IID_INIT(Internet_WANIid);
	if ( zcfgFeObjJsonGet(RDM_OID_INTERNET_WAN, &Internet_WANIid,&Internet_WANJarray) == ZCFG_SUCCESS)
	{
		Internet_WANv = Jgeti(Internet_WANJarray, "VisibilityLevel");
		Internet_WANLimitation = Jgetb(Internet_WANJarray, "Limitation");
		Internet_WANLock = Jgetb(Internet_WANJarray, "Lock");
	}
	IID_INIT(IPTV_DSLIid);
	if ( zcfgFeObjJsonGet(RDM_OID_IPTV_DSL, &IPTV_DSLIid,&IPTV_DSLJarray) == ZCFG_SUCCESS)
	{
		IPTV_DSLv = Jgeti(IPTV_DSLJarray, "VisibilityLevel");
		IPTV_DSLLimitation = Jgetb(IPTV_DSLJarray, "Limitation");
		IPTV_DSLLock = Jgetb(IPTV_DSLJarray, "Lock");
	}
	IID_INIT(IPTV_WANIid);
	if ( zcfgFeObjJsonGet(RDM_OID_IPTV_WAN, &IPTV_WANIid,&IPTV_WANJarray) == ZCFG_SUCCESS)
	{
		IPTV_WANv = Jgeti(IPTV_WANJarray, "VisibilityLevel");
		IPTV_WANLimitation = Jgetb(IPTV_WANJarray, "Limitation");
		IPTV_WANLock = Jgetb(IPTV_WANJarray, "Lock");
	}
	IID_INIT(Acs_DslIid);
	if ( zcfgFeObjJsonGet(RDM_OID_ACS_DSL, &Acs_DslIid,&Acs_DslJarray) == ZCFG_SUCCESS)
	{
		Acs_Dslv = Jgeti(Acs_DslJarray, "VisibilityLevel");
		Acs_DslLimitation = Jgetb(Acs_DslJarray, "Limitation");
		Acs_DslLock = Jgetb(Acs_DslJarray, "Lock");
	}
	IID_INIT(Acs_WanIid);
	if ( zcfgFeObjJsonGet(RDM_OID_ACS_WAN, &Acs_WanIid,&Acs_WanJarray) == ZCFG_SUCCESS)
	{
		Acs_Wanv = Jgeti(Acs_WanJarray, "VisibilityLevel");
		Acs_WanLimitation = Jgetb(Acs_WanJarray, "Limitation");
		Acs_WanLock = Jgetb(Acs_WanJarray, "Lock");
	}
	IID_INIT(ManagementIid);
	if ( zcfgFeObjJsonGet(RDM_OID_MANAGEMENT_WAN, &ManagementIid,&ManagementJarray) == ZCFG_SUCCESS)
	{
		Managementv = Jgeti(ManagementJarray, "VisibilityLevel");
		ManagementLimitation = Jgetb(ManagementJarray, "Limitation");
		ManagementLock = Jgetb(ManagementJarray, "Lock");
	}

	*Jarray = json_object_new_array();

	Jobj = json_object_new_object();	
		
	json_object_object_add(Jobj, "TAAAGMultiUser", json_object_new_int(1));

	char TAAABuserame[512] = {0};
	struct json_object *TTuserobj = NULL;
	objIndex_t TAAABuserIid;
	IID_INIT(TAAABuserIid);
					
	if (userName != NULL && strlen(userName) > 0){
		while(zcfgFeObjJsonGetNext(RDM_OID_TAAAB_USER, &TTuserIid,&TTuserobj) == ZCFG_SUCCESS){
			ZOS_STRCPY_M(TAAABuserame, Jgets(TAAABuserobj, "Username"));
			if(strcmp(userName, TAAABuserame) == 0){
				curloginLevel = Jgeti(TAAABuserobj, "Level");
				json_object_put(TAAABuserobj);
				break;
				}
			json_object_put(TAAABuserobj);
			}
		}

	IID_INIT(useraccountIid);	
    char useraccountname[512] = {0};
					
	if (userName != NULL && strlen(userName) > 0){
		while(zcfgFeObjJsonGetNext(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &useraccountIid,&useraccountobj) == ZCFG_SUCCESS){
			ZOS_STRCPY_M(useraccountname, Jgets(useraccountobj, "Username"));
			if(strcmp(userName, useraccountname) == 0){
				ZOS_STRCPY_M(password, Jgets(useraccountobj, "Password"));
				ZOS_STRCPY_M(defpassword, Jgets(useraccountobj, "DefaultPassword"));
				json_object_put(useraccountobj);
				break;
			}
			json_object_put(useraccountobj);
		}
	}

	struct json_object *Ttuserobj1 = NULL;
	struct json_object *Ttuserobj2 = NULL;
	struct json_object *userobj = NULL;
	objIndex_t TtuserIid1,TtuserIid2,userIid;
    char Ttuserame1[64] = {0};
	char Ttuserame2[64] = {0};

	IID_INIT(TtuserIid1);
	TtuserIid1.level = 1;
	TtuserIid1.idx[0] = 1; 
	if(zcfgFeObjJsonGet(RDM_OID_TAAAB_USER, &TtuserIid1,&Ttuserobj1) == ZCFG_SUCCESS){
		ZOS_STRCPY_M(Ttuserame1, Jgets(Ttuserobj1, "Username"));
		json_object_put(Ttuserobj1);
	}
	
	IID_INIT(TtuserIid2);
	TtuserIid2.level = 1;
	TtuserIid2.idx[0] = 2; 
	if(zcfgFeObjJsonGet(RDM_OID_TAAAB_USER, &TtuserIid2,&Ttuserobj2) == ZCFG_SUCCESS){
		ZOS_STRCPY_M(Ttuserame2, Jgets(Ttuserobj2, "Username"));
		json_object_put(Ttuserobj2);
	}

	if(curloginLevel == 1)
	{
		if (broadbandv == 2 || broadbandv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): broadband VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if(broadbandLimitation==false)
		{
			if(broadbandLock ==true)
			{
				enbladdNewWanBtn = false;
				enableEdit = false;
				enabledelete =	false;
				//ALL content will be grayed-out for admin
			}
		}
		else if (broadbandLimitation==true)
		{
			if(broadbandLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): broadband Lock is equal to 0\n", __FUNCTION__);
				//Admin will see the limited version of the page
			}
			else if(broadbandLock ==true)
			{
				enbladdNewWanBtn = false;
				enableEdit = false;
				//grayed-out with Limitation parameter
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (broadbandv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): broadband VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(broadbandLock ==true)
		{
			enbladdNewWanBtn = false;
			enableEdit = false;
			enabledelete =  false;
			//see all content grayed-out
		}
	}

	if(TAAAGMultiUser == 1)
	{
		if(curloginLevel == 1)
		{
			if(broadbandLimitation == false)
			{
				dislevel = 0;
			}
			else{

				dislevel = 2;
			}
		}
		else
		{
			if(broadbandLock == false){
				dislevel = 0;
			}
		}
	}
	else{
		dislevel = 0;
	}
	
	if(TAAAGMultiUser == 1){
		if(Internet_DSLv == 2){
			if(curloginLevel == 1)
				Internet_DSLpass = 0;
		}
		else if(Internet_DSLv == 3){
			Internet_DSLpass = 0;
		}
		
		if(Internet_WANv == 2){
			if(curloginLevel == 1)
				Internet_WANpass = 0;
		}
		else if(Internet_WANv == 3){
			Internet_WANpass = 0;
		}
		
		if(IPTV_DSLv == 2){
			if(curloginLevel == 1)
				IPTV_DSLpass = 0;
		}
		else if(IPTV_DSLv == 3){
			IPTV_DSLpass = 0;
		}
		
		if(IPTV_WANv == 2){
			if(curloginLevel == 1)
				IPTV_WANpass = 0;
		}
		else if(IPTV_WANv == 3){
			IPTV_WANpass = 0;
		}
		if(Managementv == 2){
			if(curloginLevel == 1)
				ManagementWANpass = 0;
		}
		else if(Managementv == 3){
			ManagementWANpass = 0;
		}
		if(Acs_Dslv == 2){
			if(curloginLevel == 1)
				Acs_Dslpass = 0;
		}
		else if(Acs_Dslv == 3){
			Acs_Dslpass = 0;
		}

		if(Acs_Wanv == 2){
			if(curloginLevel == 1)
				Acs_Wanpass = 0;
		}
		else if(Acs_Wanv == 3){
			Acs_Wanpass = 0;
		}
	}
	json_object_object_add(Jobj, "curloginLevel", json_object_new_int(curloginLevel));
	json_object_object_add(Jobj, "VisibilityLevel", json_object_new_int(broadbandv));
	json_object_object_add(Jobj, "Limitation", json_object_new_boolean(broadbandLimitation));
	json_object_object_add(Jobj, "Lock", json_object_new_boolean(broadbandLock));
	json_object_object_add(Jobj, "Internet_DSLv", json_object_new_int(Internet_DSLv));
	json_object_object_add(Jobj, "Internet_DSLLimitation", json_object_new_boolean(Internet_DSLLimitation));
	json_object_object_add(Jobj, "Internet_DSLLock", json_object_new_boolean(Internet_DSLLock));
	json_object_object_add(Jobj, "Internet_WANv", json_object_new_int(Internet_WANv));
	json_object_object_add(Jobj, "Internet_WANLimitation", json_object_new_boolean(Internet_WANLimitation));
	json_object_object_add(Jobj, "Internet_WANLock", json_object_new_boolean(Internet_WANLock));
	json_object_object_add(Jobj, "IPTV_DSLv", json_object_new_int(IPTV_DSLv));
	json_object_object_add(Jobj, "IPTV_DSLLimitation", json_object_new_boolean(IPTV_DSLLimitation));
	json_object_object_add(Jobj, "IPTV_DSLLock", json_object_new_boolean(IPTV_DSLLock));
	json_object_object_add(Jobj, "IPTV_WANv", json_object_new_int(IPTV_WANv));
	json_object_object_add(Jobj, "IPTV_WANLimitation", json_object_new_boolean(IPTV_WANLimitation));
	json_object_object_add(Jobj, "IPTV_WANLock", json_object_new_boolean(IPTV_WANLock));
	json_object_object_add(Jobj, "Managementv", json_object_new_int(Managementv));
	json_object_object_add(Jobj, "ManagementLimitation", json_object_new_boolean(ManagementLimitation));
	json_object_object_add(Jobj, "ManagementLock", json_object_new_boolean(ManagementLock));
	json_object_object_add(Jobj, "dislevel", json_object_new_int(dislevel));
	json_object_object_add(Jobj, "Internet_DSLpass", json_object_new_int(Internet_DSLpass));
	json_object_object_add(Jobj, "Internet_WANpass", json_object_new_int(Internet_WANpass));
	json_object_object_add(Jobj, "IPTV_DSLpass", json_object_new_int(IPTV_DSLpass));
	json_object_object_add(Jobj, "IPTV_WANpass", json_object_new_int(IPTV_WANpass));
	json_object_object_add(Jobj, "ManagementWANpass", json_object_new_int(ManagementWANpass));
	json_object_object_add(Jobj, "Acs_Dslpass", json_object_new_int(Acs_Dslpass));
	json_object_object_add(Jobj, "Acs_Wanpass", json_object_new_int(Acs_Wanpass));
	json_object_object_add(Jobj, "enbladdNewWanBtn", json_object_new_boolean(enbladdNewWanBtn));
	json_object_object_add(Jobj, "enableEdit", json_object_new_boolean(enableEdit));
	json_object_object_add(Jobj, "enabledelete", json_object_new_boolean(enabledelete));
	json_object_object_add(Jobj, "password", json_object_new_string(password));
	json_object_object_add(Jobj, "defpassword", json_object_new_string(defpassword));
	json_object_object_add(Jobj, "Ttuserame1", json_object_new_string(Ttuserame1));
	json_object_object_add(Jobj, "Ttuserame2", json_object_new_string(Ttuserame2));	

	FILE *wannamefs;
	char wantempbuf[512];
	wannamefs = fopen( "/var/wannamefile", "r+");
	if(wannamefs != NULL){	
		while(fscanf(wannamefs, "%[^\n]\n", wanname) > 0){			
			if(wanname == NULL){				
				printf("can not get wan name\n");
			}
		}
		fclose(wannamefs);
	}

	IID_INIT(ipinterfaceIid);
	while(zcfgFeObjJsonGetNext(RDM_OID_IP_IFACE, &ipinterfaceIid,&ipinterfaceJarray) == ZCFG_SUCCESS){
		isAddFromTr69 = Jgetb(ipinterfaceJarray, "isAddFromTr69");
		if(isAddFromTr69 == true){
			ZOS_STRCPY_M(tr69wanname,Jgets(ipinterfaceJarray, "X_TT_SrvName"));
			if(!strcmp(wanname, tr69wanname) && strstr(tr69wanname, "_DSL")){
				if(curloginLevel == 1)
				{
					if (Acs_Dslv == 2 || Acs_Dslv == 3)
					{
						zcfgLog(ZCFG_LOG_INFO, "%s(): ACS_DSL VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
					}
					else if(Acs_DslLock==false)
					{
						if(Acs_DslLimitation ==true)
						{
							iswanenable=false;
							issrv_name=false;
							istypeSelect=false;
							ismodeSelect=false;
							issrvLinkTypeSelect=false;
							isipModeSelect=false;

							isConnectionTrigger=false;
							ispppIdleTimeoutValue=false;
							iswbenable=false;

							isvpi=false;
							isvci=false;
							isencapSelect=false;
							issrvCategorySelect=false;

							isipaddress=false;

							isipAddrValue=false;
							isipMask=false;
							isgwValue=false;

							isnatEnable=false;
							isfullcone_nat=false;
							isigmpEnable=false;
							isdefaultGwEnable=false;

							isdns=false;
							isipDns1=false;
							isipDns2=false;

							isip6rdEnable=false;
							is6rdaddressEnable = false;

							isenblVlan=false;
							isvlanPrio=false;
							isvlanId=false;

							ismtuValue=false;
							isMacaddressEnable = false;

							isipv6address=false;

							isipv6AddrValue=false;
							isipv6PrefixLenVal=false;
							isipv6NextHopVal=false;

							isipv6dns=false;
							isipv6Dns1=false;
							isipv6Dns2=false;

							ismldEnable=false;
							isdefaultGw6Enable=false;

							isiapdEnable=false;
							isianaEnable=false;
							
							isfirewall=false;
						}
					}
					else if (Acs_DslLock==true)
					{
						iswanenable=false;
						issrv_name=false;
						istypeSelect=false;
						ismodeSelect=false;
						issrvLinkTypeSelect=false;
						isipModeSelect=false;

						isvpi=false;
						isvci=false;
						isencapSelect=false;
						issrvCategorySelect=false;

						ispppUsername=false;
						ispppPassword=false;
						
						isshowPppPassword=false;
						
						isConnectionTrigger=false;
						ispppIdleTimeoutValue=false;
						iswbenable=false;

						isipaddress=false;

						isipAddrValue=false;
						isipMask=false;
						isgwValue=false;

						isnatEnable=false;
						isfullcone_nat=false;
						isigmpEnable=false;
						isdefaultGwEnable=false;

						isdns=false;
						isipDns1=false;
						isipDns2=false;

						isip6rdEnable=false;
						is6rdaddressEnable = false;

						isenblVlan=false;
						isvlanPrio=false;
						isvlanId=false;

						ismtuValue=false;
						isMacaddressEnable = false;

						isipv6address=false;

						isipv6AddrValue=false;
						isipv6PrefixLenVal=false;
						isipv6NextHopVal=false;

						isipv6dns=false;
						isipv6Dns1=false;
						isipv6Dns2=false;

						ismldEnable=false;
						isdefaultGw6Enable=false;

						isiapdEnable=false;
						isianaEnable=false;
						
						isfirewall=false;
					}
				}
				else if(curloginLevel == 2)
				{
					if (Acs_Dslv == 3)
					{
						zcfgLog(ZCFG_LOG_INFO, "%s(): ACS_DSL VisibilityLevel is equal to 3\n", __FUNCTION__);
					}
					else if(Acs_DslLock ==true)
					{
						iswanenable=false;
						issrv_name=false;
						istypeSelect=false;
						ismodeSelect=false;
						issrvLinkTypeSelect=false;
						isipModeSelect=false;

						isvpi=false;
						isvci=false;
						isencapSelect=false;
						issrvCategorySelect=false;

						ispppUsername=false;
						ispppPassword=false;
						
						isshowPppPassword=false;
						
						isConnectionTrigger=false;
						ispppIdleTimeoutValue=false;
						iswbenable=false;

						isipaddress=false;

						isipAddrValue=false;
						isipMask=false;
						isgwValue=false;

						isnatEnable=false;
						isfullcone_nat=false;
						isigmpEnable=false;
						isdefaultGwEnable=false;

						isdns=false;
						isipDns1=false;
						isipDns2=false;

						isip6rdEnable=false;
						is6rdaddressEnable = false;

						isenblVlan=false;
						isvlanPrio=false;
						isvlanId=false;

						ismtuValue=false;
						isMacaddressEnable = false;

						isipv6address=false;

						isipv6AddrValue=false;
						isipv6PrefixLenVal=false;
						isipv6NextHopVal=false;

						isipv6dns=false;
						isipv6Dns1=false;
						isipv6Dns2=false;

						ismldEnable=false;
						isdefaultGw6Enable=false;

						isiapdEnable=false;
						isianaEnable=false;
						
						isfirewall=false;
					}
				}
			}
			else if(!strcmp(wanname, tr69wanname) && strstr(tr69wanname, "_WAN"))
			{
				if(curloginLevel == 1)
				{
					if (Acs_Wanv == 2 || Acs_Wanv == 3)
					{
						zcfgLog(ZCFG_LOG_INFO, "%s(): ACS_WAN VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
					}
					else if(Acs_WanLock==false)
					{
						if(Acs_WanLimitation ==true)
						{
							iswanenable=false;
							issrv_name=false;
							istypeSelect=false;
							ismodeSelect=false;
							issrvLinkTypeSelect=false;
							isipModeSelect=false;

							isConnectionTrigger=false;
							ispppIdleTimeoutValue=false;
							iswbenable=false;

							isipaddress=false;
							
							isipAddrValue=false;
							isipMask=false;
							isgwValue=false;

							isnatEnable=false;
							isfullcone_nat=false;
							isigmpEnable=false;
							isdefaultGwEnable=false;

							isdns=false;
							isipDns1=false;
							isipDns2=false;

							isip6rdEnable=false;
							is6rdaddressEnable = false;

							isenblVlan=false;
							isvlanPrio=false;
							isvlanId=false;

							ismtuValue=false;
							isMacaddressEnable = false;

							isipv6address=false;

							isipv6AddrValue=false;
							isipv6PrefixLenVal=false;
							isipv6NextHopVal=false;

							isipv6dns=false;
							isipv6Dns1=false;
							isipv6Dns2=false;

							ismldEnable=false;
							isdefaultGw6Enable=false;

							isiapdEnable=false;
							isianaEnable=false;
							
							isfirewall=false;
						}
					}
					else if (Acs_WanLock==true)
					{
						iswanenable=false;
						issrv_name=false;
						istypeSelect=false;
						ismodeSelect=false;
						issrvLinkTypeSelect=false;
						isipModeSelect=false;

						ispppUsername=false;
						ispppPassword=false;
						
						isshowPppPassword=false;
						
						isConnectionTrigger=false;
						ispppIdleTimeoutValue=false;
						iswbenable=false;

						isipaddress=false;

						isipAddrValue=false;
						isipMask=false;
						isgwValue=false;

						isnatEnable=false;
						isfullcone_nat=false;
						isigmpEnable=false;
						isdefaultGwEnable=false;

						isdns=false;
						isipDns1=false;
						isipDns2=false;

						isip6rdEnable=false;
						is6rdaddressEnable = false;

						isenblVlan=false;
						isvlanPrio=false;
						isvlanId=false;

						ismtuValue=false;
						isMacaddressEnable = false;

						isipv6address=false;

						isipv6AddrValue=false;
						isipv6PrefixLenVal=false;
						isipv6NextHopVal=false;

						isipv6dns=false;
						isipv6Dns1=false;
						isipv6Dns2=false;

						ismldEnable=false;
						isdefaultGw6Enable=false;

						isiapdEnable=false;
						isianaEnable=false;
						
						isfirewall=false;
					}
				}
				else if(curloginLevel == 2)
				{
					if (Acs_Wanv == 3)
					{
						zcfgLog(ZCFG_LOG_INFO, "%s(): ACS_WAN VisibilityLevel is equal to 2\n", __FUNCTION__);
					}
					else if(Acs_WanLock ==true)
					{
						iswanenable=false;
						issrv_name=false;
						istypeSelect=false;
						ismodeSelect=false;
						issrvLinkTypeSelect=false;
						isipModeSelect=false;

						ispppUsername=false;
						ispppPassword=false;
						
						isshowPppPassword=false;
						
						isConnectionTrigger=false;
						ispppIdleTimeoutValue=false;
						iswbenable=false;

						isipaddress=false;

						isipAddrValue=false;
						isipMask=false;
						isgwValue=false;

						isnatEnable=false;
						isfullcone_nat=false;
						isigmpEnable=false;
						isdefaultGwEnable=false;

						isdns=false;
						isipDns1=false;
						isipDns2=false;

						isip6rdEnable=false;
						is6rdaddressEnable = false;

						isenblVlan=false;
						isvlanPrio=false;
						isvlanId=false;

						ismtuValue=false;
						isMacaddressEnable = false;

						isipv6address=false;

						isipv6AddrValue=false;
						isipv6PrefixLenVal=false;
						isipv6NextHopVal=false;

						isipv6dns=false;
						isipv6Dns1=false;
						isipv6Dns2=false;

						ismldEnable=false;
						isdefaultGw6Enable=false;

						isiapdEnable=false;
						isianaEnable=false;
						
						isfirewall=false;
					}
				}
			}			
		}
		json_object_put(ipinterfaceJarray);
	}
	
	if(!strcmp(wanname, "Internet_DSL"))
	{
		if(curloginLevel == 1)
		{
			if (Internet_DSLv == 2 || Internet_DSLv == 3)
			{	
				zcfgLog(ZCFG_LOG_INFO, "%s(): Internet_DSL VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
			}
			else if(Internet_DSLLock==false)
			{
				if(Internet_DSLLimitation ==true)
				{
					iswanenable=false;
					issrv_name=false;
					istypeSelect=false;
					issrvLinkTypeSelect=false;
					isipModeSelect=false;

					isvpi=false;
					isvci=false;
					isencapSelect=false;
					issrvCategorySelect=false;

				    isConnectionTrigger=false;
					ispppIdleTimeoutValue=false;
					iswbenable=false;

					isipaddress=false;

					isipAddrValue=false;
					isipMask=false;
					isgwValue=false;

					isnatEnable=false;
					isfullcone_nat=false;
					isigmpEnable=false;
					isdefaultGwEnable=false;

					isdns=false;
					isipDns1=false;
					isipDns2=false;

					isip6rdEnable=false;
					is6rdaddressEnable = false;
					
					isenblVlan=false;
					isvlanPrio=false;
					isvlanId=false;
					isMacaddressEnable = false;

					isipv6address=false;

					isipv6AddrValue=false;
					isipv6PrefixLenVal=false;
					isipv6NextHopVal=false;

					isipv6dns=false;
					isipv6Dns1=false;
					isipv6Dns2=false;

					ismldEnable=false;
					isdefaultGw6Enable=false;

					isiapdEnable=false;
					isianaEnable=false;
					
					isfirewall=false;
				}
				else if(Internet_DSLLimitation ==false)
				{
					iswanenable=false;
					isdns=false;
					isipDns1=false;
					isipDns2=false;
				}
			}
			else if (Internet_DSLLock == true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isvpi=false;
				isvci=false;
				isencapSelect=false;
				issrvCategorySelect=false;

				ispppUsername=false;
				ispppPassword=false;

				isshowPppPassword=false;

				isConnectionTrigger=false;
				ispppIdleTimeoutValue=false;
				iswbenable=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
			}
		}
		else if(curloginLevel == 2)
		{
			if (Internet_DSLv == 3)
			{	
				zcfgLog(ZCFG_LOG_INFO, "%s(): Internet_DSL VisibilityLevel is equal to 3\n", __FUNCTION__);
			}
			else if(Internet_DSLLock ==true)
			{					
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;
					
				isvpi=false;
				isvci=false;
				isencapSelect=false;
				issrvCategorySelect=false;
				
				ispppUsername=false;
				ispppPassword=false;

				isshowPppPassword=false;

				isConnectionTrigger=false;
				ispppIdleTimeoutValue=false;
				iswbenable=false;
					
				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
			}
		}
	}
	else if(!strcmp(wanname, "Internet_WAN"))
	{
		if(curloginLevel == 1)
		{
			if (Internet_WANv == 2 || Internet_WANv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Internet_WAN VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
			}
			else if(Internet_WANLock==false)
			{
				if(Internet_WANLimitation ==true)
				{
					iswanenable=false;
					issrv_name=false;
					istypeSelect=false;
					issrvLinkTypeSelect=false;
					isipModeSelect=false;					

					isConnectionTrigger=false;
					ispppIdleTimeoutValue=false;
					iswbenable=false;

					isipaddress=false;
					
					isipAddrValue=false;
					isipMask=false;
					isgwValue=false;

					isnatEnable=false;
					isfullcone_nat=false;
					isigmpEnable=false;
					isdefaultGwEnable=false;

					isdns=false;
					isipDns1=false;
					isipDns2=false;

					isip6rdEnable=false;
					is6rdaddressEnable = false;

					isenblVlan=false;
					isvlanPrio=false;
					isvlanId=false;
					isMacaddressEnable = false;

					isipv6address=false;

					isipv6AddrValue=false;
					isipv6PrefixLenVal=false;
					isipv6NextHopVal=false;

					isipv6dns=false;
					isipv6Dns1=false;
					isipv6Dns2=false;

					ismldEnable=false;
					isdefaultGw6Enable=false;

					isiapdEnable=false;
					isianaEnable=false;
					
					isfirewall=false;
				}
				else if(Internet_WANLimitation ==false)
				{
					iswanenable=false;
					isdns=false;
					isipDns1=false;
					isipDns2=false;
				}
			}
			else if (Internet_WANLock == true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				ispppUsername=false;
				ispppPassword=false;

				isshowPppPassword=false;

				isConnectionTrigger=false;
				ispppIdleTimeoutValue=false;
				iswbenable=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
			}
		}
		else if(curloginLevel == 2)
		{
			if (Internet_WANv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Internet_WAN VisibilityLevel is equal to 3\n", __FUNCTION__);
			}
			else if(Internet_WANLock ==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;
				
				ispppUsername=false;
				ispppPassword=false;

				isshowPppPassword=false;

				isConnectionTrigger=false;
				ispppIdleTimeoutValue=false;
				iswbenable=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
			}
		}
	}
	else if(!strcmp(wanname, "IPTV_DSL"))
	{
		if(curloginLevel == 1)
		{
			if (IPTV_DSLv == 2 || IPTV_DSLv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): IPTV_DSL VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
			}
			else if(IPTV_DSLLock==false)
			{
				if(IPTV_DSLLimitation ==true)
				{
					iswanenable=false;
					issrv_name=false;
					istypeSelect=false;
					ismodeSelect=false;
					issrvLinkTypeSelect=false;
					isipModeSelect=false;

					isvpi=false;
					isvci=false;
					isencapSelect=false;
					issrvCategorySelect=false;

					isipaddress=false;

					isipAddrValue=false;
					isipMask=false;
					isgwValue=false;

					isnatEnable=false;
					isfullcone_nat=false;
					isigmpEnable=false;
					isdefaultGwEnable=false;

					isdns=false;
					isipDns1=false;
					isipDns2=false;

					isip6rdEnable=false;
					is6rdaddressEnable = false;

					isenblVlan=false;
					isvlanPrio=false;
					isvlanId=false;

					ismtuValue=false;
					isMacaddressEnable = false;

					isipv6address=false;

					isipv6AddrValue=false;
					isipv6PrefixLenVal=false;
					isipv6NextHopVal=false;

					isipv6dns=false;
					isipv6Dns1=false;
					isipv6Dns2=false;

					ismldEnable=false;
					isdefaultGw6Enable=false;

					isiapdEnable=false;
					isianaEnable=false;

					isfirewall=false;
					isbb_option42=false;
					isbb_option43=false;
					isbb_option120=false;
					isbb_option60=false;
					isbb_vendorIdValue=false;
					isbb_option61=false;
					isbb_option121=false;
					isbb_iaidValue=false;
					isbb_duidValue=false;
					isbb_option125=false;
				}
			}
			else if (IPTV_DSLLock==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isvpi=false;
				isvci=false;
				isencapSelect=false;
				issrvCategorySelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
		else if(curloginLevel == 2)
		{
			if (IPTV_DSLv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): IPTV_DSL VisibilityLevel is equal to 3\n", __FUNCTION__);
			}
			else if(IPTV_DSLLock ==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isvpi=false;
				isvci=false;
				isencapSelect=false;
				issrvCategorySelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
	}
	else if(!strcmp(wanname, "IPTV_WAN"))
	{
		if(curloginLevel == 1)
		{
			if (IPTV_WANv == 2 || IPTV_WANv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): IPTV_WAN VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
			}
			else if(IPTV_WANLock==false)
			{
				if(IPTV_WANLimitation ==true)
				{
					iswanenable=false;
					issrv_name=false;
					istypeSelect=false;
					ismodeSelect=false;
					issrvLinkTypeSelect=false;
					isipModeSelect=false;

					isipaddress=false;
					
					isipAddrValue=false;
					isipMask=false;
					isgwValue=false;

					isnatEnable=false;
					isfullcone_nat=false;
					isigmpEnable=false;
					isdefaultGwEnable=false;

					isdns=false;
					isipDns1=false;
					isipDns2=false;

					isip6rdEnable=false;
					is6rdaddressEnable = false;

					isenblVlan=false;
					isvlanPrio=false;
					isvlanId=false;

					ismtuValue=false;
					isMacaddressEnable = false;

					isipv6address=false;

					isipv6AddrValue=false;
					isipv6PrefixLenVal=false;
					isipv6NextHopVal=false;

					isipv6dns=false;
					isipv6Dns1=false;
					isipv6Dns2=false;

					ismldEnable=false;
					isdefaultGw6Enable=false;

					isiapdEnable=false;
					isianaEnable=false;
					
					isfirewall=false;
					isbb_option42=false;
					isbb_option43=false;
					isbb_option120=false;
					isbb_option60=false;
					isbb_vendorIdValue=false;
					isbb_option61=false;
					isbb_option121=false;
					isbb_iaidValue=false;
					isbb_duidValue=false;
					isbb_option125=false;
				}
			}
			else if (IPTV_WANLock==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
		else if(curloginLevel == 2)
		{
		
			if (IPTV_WANv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): IPTV_WAN VisibilityLevel is equal to 3\n", __FUNCTION__);
			}
			else if(IPTV_WANLock ==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;
				
				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
	}
	else if(!strcmp(wanname, "ETH-MANAGEMENT_WAN"))
	{
		if(curloginLevel == 1)
		{
			if (Managementv == 2 || Managementv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): ETH-MANAGEMENT_WAN VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
			}
			else if(ManagementLock==false)
			{
				if(ManagementLimitation ==true)
				{
					iswanenable=false;
					issrv_name=false;
					istypeSelect=false;
					ismodeSelect=false;
					issrvLinkTypeSelect=false;
					isipModeSelect=false;

					isipaddress=false;
					
					isipAddrValue=false;
					isipMask=false;
					isgwValue=false;

					isnatEnable=false;
					isfullcone_nat=false;
					isigmpEnable=false;
					isdefaultGwEnable=false;

					isdns=false;
					isipDns1=false;
					isipDns2=false;

					isip6rdEnable=false;
					is6rdaddressEnable = false;

					isenblVlan=false;
					isvlanPrio=false;
					isvlanId=false;

					ismtuValue=false;
					isMacaddressEnable = false;

					isipv6address=false;

					isipv6AddrValue=false;
					isipv6PrefixLenVal=false;
					isipv6NextHopVal=false;

					isipv6dns=false;
					isipv6Dns1=false;
					isipv6Dns2=false;

					ismldEnable=false;
					isdefaultGw6Enable=false;

					isiapdEnable=false;
					isianaEnable=false;
					
					isfirewall=false;
					isbb_option42=false;
					isbb_option43=false;
					isbb_option120=false;
					isbb_option60=false;
					isbb_vendorIdValue=false;
					isbb_option61=false;
					isbb_option121=false;
					isbb_iaidValue=false;
					isbb_duidValue=false;
					isbb_option125=false;
				}
			}
			else if (ManagementLock==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
		else if(curloginLevel == 2)
		{
		
			if (Managementv == 3)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): ETH-MANAGEMENT_WAN VisibilityLevel is equal to 3\n", __FUNCTION__);
			}
			else if(ManagementLock ==true)
			{
				iswanenable=false;
				issrv_name=false;
				istypeSelect=false;
				ismodeSelect=false;
				issrvLinkTypeSelect=false;
				isipModeSelect=false;

				isipaddress=false;

				isipAddrValue=false;
				isipMask=false;
				isgwValue=false;

				isnatEnable=false;
				isfullcone_nat=false;
				isigmpEnable=false;
				isdefaultGwEnable=false;

				isdns=false;
				isipDns1=false;
				isipDns2=false;

				isip6rdEnable=false;
				is6rdaddressEnable = false;

				isenblVlan=false;
				isvlanPrio=false;
				isvlanId=false;

				ismtuValue=false;
				isMacaddressEnable = false;

				isipv6address=false;

				isipv6AddrValue=false;
				isipv6PrefixLenVal=false;
				isipv6NextHopVal=false;

				isipv6dns=false;
				isipv6Dns1=false;
				isipv6Dns2=false;

				ismldEnable=false;
				isdefaultGw6Enable=false;

				isiapdEnable=false;
				isianaEnable=false;
				
				isfirewall=false;
				isbb_option42=false;
				isbb_option43=false;
				isbb_option120=false;
				isbb_option60=false;
				isbb_vendorIdValue=false;
				isbb_option61=false;
				isbb_option121=false;
				isbb_iaidValue=false;
				isbb_duidValue=false;
				isbb_option125=false;
			}
		}
	}
	json_object_object_add(Jobj, "iswanenable", json_object_new_boolean(iswanenable));
	json_object_object_add(Jobj, "issrv_name", json_object_new_boolean(issrv_name));
	json_object_object_add(Jobj, "istypeSelect", json_object_new_boolean(istypeSelect));
	json_object_object_add(Jobj, "ismodeSelect", json_object_new_boolean(ismodeSelect));
	json_object_object_add(Jobj, "issrvLinkTypeSelect", json_object_new_boolean(issrvLinkTypeSelect));
	json_object_object_add(Jobj, "isipModeSelect", json_object_new_boolean(isipModeSelect));
	json_object_object_add(Jobj, "ispppUsername", json_object_new_boolean(ispppUsername));
	json_object_object_add(Jobj, "ispppPassword", json_object_new_boolean(ispppPassword));
	json_object_object_add(Jobj, "isshowPppPassword", json_object_new_boolean(isshowPppPassword));
	json_object_object_add(Jobj, "isConnectionTrigger", json_object_new_boolean(isConnectionTrigger));
	json_object_object_add(Jobj, "ispppIdleTimeoutValue", json_object_new_boolean(ispppIdleTimeoutValue));
	json_object_object_add(Jobj, "iswbenable", json_object_new_boolean(iswbenable));
	json_object_object_add(Jobj, "isvpi", json_object_new_boolean(isvpi));
	json_object_object_add(Jobj, "isvci", json_object_new_boolean(isvci));
	json_object_object_add(Jobj, "isencapSelect", json_object_new_boolean(isencapSelect));
	json_object_object_add(Jobj, "issrvCategorySelect", json_object_new_boolean(issrvCategorySelect));
	json_object_object_add(Jobj, "isenblVlan", json_object_new_boolean(isenblVlan));
	json_object_object_add(Jobj, "isvlanPrio", json_object_new_boolean(isvlanPrio));
	json_object_object_add(Jobj, "isvlanId", json_object_new_boolean(isvlanId));
	json_object_object_add(Jobj, "ismtuValue", json_object_new_boolean(ismtuValue));
	json_object_object_add(Jobj, "isipaddress", json_object_new_boolean(isipaddress));
	json_object_object_add(Jobj, "isipAddrValue", json_object_new_boolean(isipAddrValue));
	json_object_object_add(Jobj, "isipMask", json_object_new_boolean(isipMask));
	json_object_object_add(Jobj, "isgwValue", json_object_new_boolean(isgwValue));
	json_object_object_add(Jobj, "isdns", json_object_new_boolean(isdns));
	json_object_object_add(Jobj, "isipDns1", json_object_new_boolean(isipDns1));
	json_object_object_add(Jobj, "isipDns2", json_object_new_boolean(isipDns2));
	json_object_object_add(Jobj, "isnatEnable", json_object_new_boolean(isnatEnable));
	json_object_object_add(Jobj, "isfullcone_nat", json_object_new_boolean(isfullcone_nat));
	json_object_object_add(Jobj, "isigmpEnable", json_object_new_boolean(isigmpEnable));
	json_object_object_add(Jobj, "isdefaultGwEnable", json_object_new_boolean(isdefaultGwEnable));
	json_object_object_add(Jobj, "isip6rdEnable", json_object_new_boolean(isip6rdEnable));
	json_object_object_add(Jobj, "is6rdaddressEnable", json_object_new_boolean(is6rdaddressEnable));
	json_object_object_add(Jobj, "isMacaddressEnable", json_object_new_boolean(isMacaddressEnable));
	json_object_object_add(Jobj, "isipv6address", json_object_new_boolean(isipv6address));
	json_object_object_add(Jobj, "isipv6AddrValue", json_object_new_boolean(isipv6AddrValue));
	json_object_object_add(Jobj, "isipv6PrefixLenVal", json_object_new_boolean(isipv6PrefixLenVal));
	json_object_object_add(Jobj, "isipv6NextHopVal", json_object_new_boolean(isipv6NextHopVal));
	json_object_object_add(Jobj, "isipv6dns", json_object_new_boolean(isipv6dns));
	json_object_object_add(Jobj, "isipv6Dns1", json_object_new_boolean(isipv6Dns1));
	json_object_object_add(Jobj, "isipv6Dns2", json_object_new_boolean(isipv6Dns2));
	json_object_object_add(Jobj, "ismldEnable", json_object_new_boolean(ismldEnable));
	json_object_object_add(Jobj, "isdefaultGw6Enable", json_object_new_boolean(isdefaultGw6Enable));
	json_object_object_add(Jobj, "isiapdEnable", json_object_new_boolean(isiapdEnable));
	json_object_object_add(Jobj, "isianaEnable", json_object_new_boolean(isianaEnable));
	json_object_object_add(Jobj, "isfirewall", json_object_new_boolean(isfirewall));
	json_object_object_add(Jobj, "isbb_option42", json_object_new_boolean(isbb_option42));
	json_object_object_add(Jobj, "isbb_option43", json_object_new_boolean(isbb_option43));
	json_object_object_add(Jobj, "isbb_option120", json_object_new_boolean(isbb_option120));
	json_object_object_add(Jobj, "isbb_option60", json_object_new_boolean(isbb_option60));
	json_object_object_add(Jobj, "isbb_vendorIdValue", json_object_new_boolean(isbb_vendorIdValue));
	json_object_object_add(Jobj, "isbb_option61", json_object_new_boolean(isbb_option61));
	json_object_object_add(Jobj, "isbb_option121", json_object_new_boolean(isbb_option121));
	json_object_object_add(Jobj, "isbb_iaidValue", json_object_new_boolean(isbb_iaidValue));
	json_object_object_add(Jobj, "isbb_duidValue", json_object_new_boolean(isbb_duidValue));
	json_object_object_add(Jobj, "isbb_option125", json_object_new_boolean(isbb_option125));

	if(strcmp(tr69wanname, "") == 0){
		json_object_object_add(Jobj, "tr69wanname", json_object_new_string(""));
	}	
	else{
		json_object_object_add(Jobj, "tr69wanname", json_object_new_string(tr69wanname));
	}

	IID_INIT(AdvancedIid);
	if ( zcfgFeObjJsonGet(RDM_OID_ADVANCED, &AdvancedIid,&AdvancedJarray) == ZCFG_SUCCESS)
	{
		Advancedv = Jgeti(AdvancedJarray, "VisibilityLevel");
		AdvancedLimitation = Jgetb(AdvancedJarray, "Limitation");
		AdvancedLock = Jgetb(AdvancedJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Advancedv == 2 || Advancedv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Advanced VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (AdvancedLock ==true)
		{
			dslBonding=false;
			enblAdslPTM=false;
			enblAnnexM=false;
			enblPhyrUSDS=false;
			enblSRA=false;
			enblcancelsubmit=false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Advancedv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Advanced VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(AdvancedLock ==true)
		{
			dslBonding=false;
			enblAdslPTM=false;
			enblAnnexM=false;
			enblPhyrUSDS=false;
			enblSRA=false;
			enblcancelsubmit=false;
		}
	}
	json_object_object_add(Jobj, "AdvancedVisibilityLevel", json_object_new_int(Advancedv));
	json_object_object_add(Jobj, "AdvancedLimitation", json_object_new_boolean(AdvancedLimitation));
	json_object_object_add(Jobj, "AdvancedLock", json_object_new_boolean(AdvancedLock));
	json_object_object_add(Jobj, "dslBonding", json_object_new_boolean(dslBonding));
	json_object_object_add(Jobj, "enblAdslPTM", json_object_new_boolean(enblAdslPTM));
	json_object_object_add(Jobj, "enblAnnexM", json_object_new_boolean(enblAnnexM));
	json_object_object_add(Jobj, "enblPhyrUSDS", json_object_new_boolean(enblPhyrUSDS));
	json_object_object_add(Jobj, "enblSRA", json_object_new_boolean(enblSRA));
	json_object_object_add(Jobj, "enblcancelsubmit", json_object_new_boolean(enblcancelsubmit));

	IID_INIT(GeneralIid);
	if ( zcfgFeObjJsonGet(RDM_OID_GENERAL, &GeneralIid,&GeneralJarray) == ZCFG_SUCCESS)
	{
		Generalv = Jgeti(GeneralJarray, "VisibilityLevel");
		GeneralLimitation = Jgetb(GeneralJarray, "Limitation");
		GeneralLock = Jgetb(GeneralJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Generalv == 2 || Generalv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): General VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if(GeneralLimitation==false)
		{
			if(GeneralLock ==false)
			{
				enblsecurityLevel= true;
			}
			else if(GeneralLock ==true)
			{
				enblsideband= false;
				enblMaxBandwidth= false;
				enblEnable= false;
				
				enblChannel= false;
				enblSsid= false;
				enblhide= false;
				enblwmf= false;
				enblwepkey= false;
				enblWEPEncryption= false;
				enblSecurity= false;
				enblwpakey= false;
				enblExtendMore = false;
				enblGecancelsubmit= false;

				//ALL content will be grayed-out for admin
			}
		}
		else if (GeneralLimitation==true)
		{
			if(GeneralLock ==false)
			{
				enblsecurityLevel= false;

				//Admin will see the limited version of the page
			}
			else if(GeneralLock ==true)
			{
				enblsideband= false;
				enblMaxBandwidth= false;

				enblEnable= false;
				enblsecurityLevel= false;
				enblChannel= false;
				enblSsid= false;
				enblhide= false;
				enblwmf= false;
				enblwepkey= false;
				enblWEPEncryption= false;
				enblSecurity= false;
				enblwpakey= false;
				enblExtendMore = false;
				enblGecancelsubmit= false;
				//grayed-out with Limitation parameter
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (Generalv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): General VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(GeneralLock ==false)
		{
			enblsecurityLevel= true;

			//see/modify all content of the page
		}
		else if(GeneralLock ==true)
		{
			enblsideband= false;
			enblMaxBandwidth= false;
			enblEnable= false;
			enblsecurityLevel= true;
			enblChannel= false;
			enblSsid= false;
			enblhide= false;
			enblwmf= false;
			enblwepkey= false;
			enblWEPEncryption= false;
			enblSecurity= false;
			enblwpakey= false;
			enblExtendMore = false;
			enblGecancelsubmit= false;
			//see all content grayed-out
		}
	}	
	json_object_object_add(Jobj, "GeneralVisibilityLevel", json_object_new_int(Generalv));
	json_object_object_add(Jobj, "GeneralLimitation", json_object_new_boolean(GeneralLimitation));
	json_object_object_add(Jobj, "GeneralLock", json_object_new_boolean(GeneralLock));
	json_object_object_add(Jobj, "enblBand", json_object_new_boolean(enblBand));
	json_object_object_add(Jobj, "enblsideband", json_object_new_boolean(enblsideband));
	json_object_object_add(Jobj, "enblMaxBandwidth", json_object_new_boolean(enblMaxBandwidth));
	json_object_object_add(Jobj, "enblEnable", json_object_new_boolean(enblEnable));
	json_object_object_add(Jobj, "enblsecurityLevel", json_object_new_boolean(enblsecurityLevel));
	json_object_object_add(Jobj, "enblChannel", json_object_new_boolean(enblChannel));
	json_object_object_add(Jobj, "enblSsid", json_object_new_boolean(enblSsid));
	json_object_object_add(Jobj, "enblhide", json_object_new_boolean(enblhide));
	json_object_object_add(Jobj, "enblwmf", json_object_new_boolean(enblwmf));
	json_object_object_add(Jobj, "enblwepkey", json_object_new_boolean(enblwepkey));
	json_object_object_add(Jobj, "enblWEPEncryption", json_object_new_boolean(enblWEPEncryption));
	json_object_object_add(Jobj, "enblSecurity", json_object_new_boolean(enblSecurity));
	json_object_object_add(Jobj, "enblwpakey", json_object_new_boolean(enblwpakey));
	json_object_object_add(Jobj, "enblExtendMore", json_object_new_boolean(enblExtendMore));
	json_object_object_add(Jobj, "enblGecancelsubmit", json_object_new_boolean(enblGecancelsubmit));

	IID_INIT(GuestIid);
	if ( zcfgFeObjJsonGet(RDM_OID_MORE_AP, &GuestIid,&GuestJarray) == ZCFG_SUCCESS)
	{
		Guestv = Jgeti(GuestJarray, "VisibilityLevel");
		GuestLimitation = Jgetb(GuestJarray, "Limitation");
		GuestLock = Jgetb(GuestJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Guestv == 2 || Guestv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Guest VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (GuestLock ==true)
		{
			enblEdit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Guestv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Guest VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(GuestLock ==true)
		{
			enblEdit = false;
		}
	}
	json_object_object_add(Jobj, "GuestVisibilityLevel", json_object_new_int(Guestv));
	json_object_object_add(Jobj, "GuestLimitation", json_object_new_boolean(GuestLimitation));
	json_object_object_add(Jobj, "GuestLock", json_object_new_boolean(GuestLock));
	json_object_object_add(Jobj, "enblEdit", json_object_new_boolean(enblEdit));

	IID_INIT(WPSIid);
	if ( zcfgFeObjJsonGet(RDM_OID_WPS, &WPSIid,&WPSJarray) == ZCFG_SUCCESS)
	{
		WPSv = Jgeti(WPSJarray, "VisibilityLevel");
		WPSLimitation = Jgetb(WPSJarray, "Limitation");
		WPSLock = Jgetb(WPSJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (WPSv == 2 || WPSv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): WPS VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (WPSLock ==true)
		{
			isenableWPS = false;
			isWPSBtn = false;
			ispinVal = false;
			isRegister = false;
			isreleaseCfg = false;
			isgenNewPin = false;
			enblWPScancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (WPSv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): WPS VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(WPSLock ==true)
		{
			isenableWPS = false;
			isWPSBtn = false;
			ispinVal = false;
			isRegister = false;
			isreleaseCfg = false;
			isgenNewPin = false;
			enblWPScancelsubmit = false;
		}
	}
	json_object_object_add(Jobj, "WPSVisibilityLevel", json_object_new_int(WPSv));
	json_object_object_add(Jobj, "WPSLimitation", json_object_new_boolean(WPSLimitation));
	json_object_object_add(Jobj, "WPSLock", json_object_new_boolean(WPSLock));
	json_object_object_add(Jobj, "isenableWPS", json_object_new_boolean(isenableWPS));
	json_object_object_add(Jobj, "isWPSBtn", json_object_new_boolean(isWPSBtn));
	json_object_object_add(Jobj, "ispinVal", json_object_new_boolean(ispinVal));
	json_object_object_add(Jobj, "isRegister", json_object_new_boolean(isRegister));
	json_object_object_add(Jobj, "isreleaseCfg", json_object_new_boolean(isreleaseCfg));
	json_object_object_add(Jobj, "isgenNewPin", json_object_new_boolean(isgenNewPin));
	json_object_object_add(Jobj, "enblWPScancelsubmit", json_object_new_boolean(enblWPScancelsubmit));

	IID_INIT(WMMIid);
	if ( zcfgFeObjJsonGet(RDM_OID_WMM, &WMMIid,&WMMJarray) == ZCFG_SUCCESS)
	{
		WMMv = Jgeti(WMMJarray, "VisibilityLevel");
		WMMLimitation = Jgetb(WMMJarray, "Limitation");
		WMMLock = Jgetb(WMMJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (WMMv == 2 || WMMv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): WMM VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (WMMLock ==true)
		{
			isenableWMMAPSD = false;
			enblWMMcancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (WMMv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): WMM VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(WMMLock ==true)
		{
			isenableWMMAPSD = false;
			enblWMMcancelsubmit = false;
		}
	}	
	json_object_object_add(Jobj, "WMMVisibilityLevel", json_object_new_int(WMMv));
	json_object_object_add(Jobj, "WMMLimitation", json_object_new_boolean(WMMLimitation));
	json_object_object_add(Jobj, "WMMLock", json_object_new_boolean(WMMLock));
	json_object_object_add(Jobj, "isenableWMMAPSD", json_object_new_boolean(isenableWMMAPSD));
	json_object_object_add(Jobj, "enblWMMcancelsubmit", json_object_new_boolean(enblWMMcancelsubmit));

	IID_INIT(OthersIid);
	if ( zcfgFeObjJsonGet(RDM_OID_OTHERS, &OthersIid,&OthersJarray) == ZCFG_SUCCESS)
	{
		Othersv = Jgeti(OthersJarray, "VisibilityLevel");
		OthersLimitation = Jgetb(OthersJarray, "Limitation");
		OthersLock = Jgetb(OthersJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Othersv == 2 || Othersv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Others VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if(OthersLimitation==false)
		{
			if(OthersLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Others Lock is equal to 0\n", __FUNCTION__);
				//all content is displayed 
			}
			else if(OthersLock ==true)
			{
				isRtsThrshld = false;
				isFrgThrshld = false;
				isAutoChannelTimer = false;
				isPwrPcnt = false;
				isBcnIntvl = false;
				isDtmIntvl = false;
				isMode = false;
				isProtect = false;
				isPreamble = false;
				enblOtherscancelsubmit = false;
				//ALL content will be grayed-out for admin
			}
		}
		else if (OthersLimitation==true)
		{
			if(OthersLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Others Lock is equal to 0\n", __FUNCTION__);
				//Admin will see the limited version of the page
			}
			else if(OthersLock ==true)
			{				
				isRtsThrshld = false;
				isFrgThrshld = false;
				isAutoChannelTimer = false;
				isPwrPcnt = false;
				isBcnIntvl = false;
				isDtmIntvl = false;
				isMode = false;
				isProtect = false;
				isPreamble = false;
				enblOtherscancelsubmit = false;
				//grayed-out with Limitation parameter
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (Othersv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Others VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(OthersLock ==false)
		{		
			zcfgLog(ZCFG_LOG_INFO, "%s(): Others Lock is equal to 0\n", __FUNCTION__);
			//see/modify all content of the page
		}
		else if(OthersLock ==true)
		{			
			isRtsThrshld = false;
			isFrgThrshld = false;
			isAutoChannelTimer = false;
			isPwrPcnt = false;
			isBcnIntvl = false;
			isDtmIntvl = false;
			isMode = false;
			isProtect = false;
			isPreamble = false;
			enblOtherscancelsubmit = false;
			//see all content grayed-out
		}
	}
	json_object_object_add(Jobj, "OthersVisibilityLevel", json_object_new_int(Othersv));
	json_object_object_add(Jobj, "OthersLimitation", json_object_new_boolean(OthersLimitation));
	json_object_object_add(Jobj, "OthersLock", json_object_new_boolean(OthersLock));
	json_object_object_add(Jobj, "isRtsThrshld", json_object_new_boolean(isRtsThrshld));
	json_object_object_add(Jobj, "isFrgThrshld", json_object_new_boolean(isFrgThrshld));
	json_object_object_add(Jobj, "isPwrPcnt", json_object_new_boolean(isPwrPcnt));
	json_object_object_add(Jobj, "isBcnIntvl", json_object_new_boolean(isBcnIntvl));
	json_object_object_add(Jobj, "isDtmIntvl", json_object_new_boolean(isDtmIntvl));
	json_object_object_add(Jobj, "isMode", json_object_new_boolean(isMode));
	json_object_object_add(Jobj, "isProtect", json_object_new_boolean(isProtect));
	json_object_object_add(Jobj, "isPreamble", json_object_new_boolean(isPreamble));
	json_object_object_add(Jobj, "isAutoChannelTimer", json_object_new_boolean(isAutoChannelTimer));
	json_object_object_add(Jobj, "enblOtherscancelsubmit", json_object_new_boolean(enblOtherscancelsubmit));

	IID_INIT(LANSetupIid);
	if ( zcfgFeObjJsonGet(RDM_OID_LAN_SETUP, &LANSetupIid,&LANSetupJarray) == ZCFG_SUCCESS)
	{
		LANSetupv = Jgeti(LANSetupJarray, "VisibilityLevel");
		LANSetupLimitation = Jgetb(LANSetupJarray, "Limitation");
		LANSetupLock = Jgetb(LANSetupJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (LANSetupv == 2 || LANSetupv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): LANSetup VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if(LANSetupLimitation==false)
		{
			if(LANSetupLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): LANSetup Lock is equal to 0\n", __FUNCTION__);
				//all content is displayed 
			}
			else if(LANSetupLock ==true)
			{
				isgroupname = false;
				isIpAddress = false;
				isSubnetMask = false;
				isIgmpSnp = false;
				isigmpMode = false;				
				isenblDhcpSrv = false;
				isdhcpEthStart = false;
				isdhcpEthEnd = false;
				isDhcpAutoReserve = false;
				isday = false;
				ishour = false;
				isminute = false;
				isdhcpRelayServer = false;
				isdnsType = false;
				isdns_server_1 = false;
				isdns_server_2 = false;
				isenblIpv6Lan = false;
				enblLANSetupcancelsubmit = false;
				//ALL content will be grayed-out for admin
			}
		}
		else if (LANSetupLimitation==true)
		{
			if(LANSetupLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): LANSetup Lock is equal to 0\n", __FUNCTION__);
				//Admin will see the limited version of the page
			}
			else if(LANSetupLock ==true)
			{
				isgroupname = false;
				isIpAddress = false;
				isSubnetMask = false;
				isIgmpSnp = false;
				isigmpMode = false;				
				isenblDhcpSrv = false;
				isdhcpEthStart = false;
				isdhcpEthEnd = false;
				isDhcpAutoReserve = false;
				isday = false;
				ishour = false;
				isminute = false;
				isdhcpRelayServer = false;
				isdnsType = false;
				isdns_server_1 = false;
				isdns_server_2 = false;
				isenblIpv6Lan = false;
				enblLANSetupcancelsubmit = false;
				//grayed-out with Limitation parameter
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (LANSetupv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): LANSetup VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(LANSetupLock ==false)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): LANSetup Lock is equal to 0\n", __FUNCTION__);
			//see/modify all content of the page
		}
		else if(LANSetupLock ==true)
		{
			isgroupname = false;
			isIpAddress = false;
			isSubnetMask = false;
			isIgmpSnp = false;
			isigmpMode = false;		
			isenblDhcpSrv = false;
			isdhcpEthStart = false;
			isdhcpEthEnd = false;
			isDhcpAutoReserve = false;
			isday = false;
			ishour = false;
			isminute = false;
			isdhcpRelayServer = false;
			isdnsType = false;
			isdns_server_1 = false;
			isdns_server_2 = false;
			isenblIpv6Lan = false;
			enblLANSetupcancelsubmit = false;
			//see all content grayed-out
		}
	}
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
			struct json_object *EDNSJobj = NULL;
			objIndex_t EDNSIid;
	
			IID_INIT(EDNSIid);
			if ( zcfgFeObjJsonGet(RDM_OID_E_D_N_S, &EDNSIid,&EDNSJobj) == ZCFG_SUCCESS)
			{
				if(Jgetb(EDNSJobj, "Enable") == true){
					if(curloginLevel == 1)
						isdnsType = false;
				}
				json_object_put(EDNSJobj);
			}
#endif
	json_object_object_add(Jobj, "LANSetupVisibilityLevel", json_object_new_int(LANSetupv));
	json_object_object_add(Jobj, "LANSetupLimitation", json_object_new_boolean(LANSetupLimitation));
	json_object_object_add(Jobj, "LANSetupLock", json_object_new_boolean(LANSetupLock));
	json_object_object_add(Jobj, "isgroupname", json_object_new_boolean(isgroupname));
	json_object_object_add(Jobj, "isIpAddress", json_object_new_boolean(isIpAddress));
	json_object_object_add(Jobj, "isSubnetMask", json_object_new_boolean(isSubnetMask));
	json_object_object_add(Jobj, "isIgmpSnp", json_object_new_boolean(isIgmpSnp));
	json_object_object_add(Jobj, "isigmpMode", json_object_new_boolean(isigmpMode));
	json_object_object_add(Jobj, "isenblDhcpSrv", json_object_new_boolean(isenblDhcpSrv));
	json_object_object_add(Jobj, "isdhcpEthStart", json_object_new_boolean(isdhcpEthStart));
	json_object_object_add(Jobj, "isdhcpEthEnd", json_object_new_boolean(isdhcpEthEnd));
	json_object_object_add(Jobj, "isDhcpAutoReserve", json_object_new_boolean(isDhcpAutoReserve));
	json_object_object_add(Jobj, "isday", json_object_new_boolean(isday));
	json_object_object_add(Jobj, "ishour", json_object_new_boolean(ishour));
	json_object_object_add(Jobj, "isminute", json_object_new_boolean(isminute));
	json_object_object_add(Jobj, "isdhcpRelayServer", json_object_new_boolean(isdhcpRelayServer));
	json_object_object_add(Jobj, "isdnsType", json_object_new_boolean(isdnsType));
	json_object_object_add(Jobj, "isdns_server_1", json_object_new_boolean(isdns_server_1));
	json_object_object_add(Jobj, "isdns_server_2", json_object_new_boolean(isdns_server_2));
	json_object_object_add(Jobj, "isenblIpv6Lan", json_object_new_boolean(isenblIpv6Lan));
	json_object_object_add(Jobj, "enblLANSetupcancelsubmit", json_object_new_boolean(enblLANSetupcancelsubmit));

	IID_INIT(RoutingIid);
	if ( zcfgFeObjJsonGet(RDM_OID_PAGE_ROUTING, &RoutingIid,&RoutingJarray) == ZCFG_SUCCESS)
	{
		Routingv = Jgeti(RoutingJarray, "VisibilityLevel");
		RoutingLimitation = Jgetb(RoutingJarray, "Limitation");
		RoutingLock = Jgetb(RoutingJarray, "Lock");
	}
	
	if(curloginLevel == 1)
	{
		if (Routingv == 2 || Routingv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Routing VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (RoutingLock ==true)
		{
			enblDNSRtadd = false;
			enblRIPcancelsubmit = false;
			enblPolicyRtadd = false;
			enblStaticRtadd = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Routingv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Routing VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(RoutingLock ==true)
		{
			enblDNSRtadd = false;
			enblRIPcancelsubmit = false;			
			enblPolicyRtadd = false;			
			enblStaticRtadd = false;
		}
	}

	if(TAAAGMultiUser == 1)
	{
		if(curloginLevel == 1 || curloginLevel == 2)
		{
			if(RoutingLock == true)
			{
				enblversion = false;
				enblmode = false;
				enblEnbl = false;
			}
			else if(RoutingLock == false)
			{
				enblversion = true;
				enblmode = true;
				enblEnbl = true;
			}
		}
		else
		{
			enblversion = true;
			enblmode = true;
			enblEnbl = true;
		}
	}
	else
	{
		enblversion = true;
		enblmode = true;
		enblEnbl = true;
	}
	json_object_object_add(Jobj, "RoutingVisibilityLevel", json_object_new_int(Routingv));
	json_object_object_add(Jobj, "RoutingLimitation", json_object_new_boolean(RoutingLimitation));
	json_object_object_add(Jobj, "RoutingLock", json_object_new_boolean(RoutingLock));
	json_object_object_add(Jobj, "enblDNSRtadd", json_object_new_boolean(enblDNSRtadd));
	json_object_object_add(Jobj, "enblversion", json_object_new_boolean(enblversion));
	json_object_object_add(Jobj, "enblmode", json_object_new_boolean(enblmode));
	json_object_object_add(Jobj, "enblEnbl", json_object_new_boolean(enblEnbl));
	json_object_object_add(Jobj, "enblRIPcancelsubmit", json_object_new_boolean(enblRIPcancelsubmit));
	json_object_object_add(Jobj, "enblPolicyRtadd", json_object_new_boolean(enblPolicyRtadd));
	json_object_object_add(Jobj, "enblStaticRtadd", json_object_new_boolean(enblStaticRtadd));

	IID_INIT(ALGIid);
	if ( zcfgFeObjJsonGet(RDM_OID_ALG, &ALGIid,&ALGJarray) == ZCFG_SUCCESS)
	{
		ALGv = Jgeti(ALGJarray, "VisibilityLevel");
		ALGLimitation = Jgetb(ALGJarray, "Limitation");
		ALGLock = Jgetb(ALGJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (ALGv == 2 || ALGv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): ALG VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (ALGLock ==true)
		{
			enblNATALG = false;
			enblSIPALG = false;
			enblRTSPALG = false;
			enblALGcancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (ALGv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): ALG VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(ALGLock ==true)
		{
			enblNATALG = false;
			enblSIPALG = false;
			enblRTSPALG = false;
			enblALGcancelsubmit = false;
		}
	}
	json_object_object_add(Jobj, "ALGVisibilityLevel", json_object_new_int(ALGv));
	json_object_object_add(Jobj, "ALGLimitation", json_object_new_boolean(ALGLimitation));
	json_object_object_add(Jobj, "ALGLock", json_object_new_boolean(ALGLock));
	json_object_object_add(Jobj, "enblNATALG", json_object_new_boolean(enblNATALG));
	json_object_object_add(Jobj, "enblSIPALG", json_object_new_boolean(enblSIPALG));
	json_object_object_add(Jobj, "enblRTSPALG", json_object_new_boolean(enblRTSPALG));
	json_object_object_add(Jobj, "enblALGcancelsubmit", json_object_new_boolean(enblALGcancelsubmit));

	IID_INIT(DosIid);
	if ( zcfgFeObjJsonGet(RDM_OID_DOS, &DosIid,&DosJarray) == ZCFG_SUCCESS)
	{
		Dosv = Jgeti(DosJarray, "VisibilityLevel");
		DosLimitation = Jgetb(DosJarray, "Limitation");
		DosLock = Jgetb(DosJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Dosv == 2 || Dosv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Dos VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (DosLock ==true)
		{
			enblDosEnable = false;
			enblDoscancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Dosv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Dos VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(DosLock ==true)
		{
			enblDosEnable = false;
			enblDoscancelsubmit = false;
		}
	}	
	json_object_object_add(Jobj, "DosVisibilityLevel", json_object_new_int(Dosv));
	json_object_object_add(Jobj, "DosLimitation", json_object_new_boolean(DosLimitation));
	json_object_object_add(Jobj, "DosLock", json_object_new_boolean(DosLock));
	json_object_object_add(Jobj, "enblDosEnable", json_object_new_boolean(enblDosEnable));
	json_object_object_add(Jobj, "enblDoscancelsubmit", json_object_new_boolean(enblDoscancelsubmit));

	IID_INIT(TimeIid);
	if ( zcfgFeObjJsonGet(RDM_OID_PAGE_TIME, &TimeIid,&TimeJarray) == ZCFG_SUCCESS)
	{
		Timev = Jgeti(TimeJarray, "VisibilityLevel");
		TimeLimitation = Jgetb(TimeJarray, "Limitation");
		TimeLock = Jgetb(TimeJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Timev == 2 || Timev == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Time VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (TimeLock ==true)
		{
			enblshowTime = false;
			enblshowDate = false;
			enblntpServer = false;
			enblserveraddr = false;
			enblTimeZone = false;
			enbldaylightsave = false;
			enblsdayrule1 = false;
			enblstartday = false;		
			enblsdayrule2 = false;
			enblstartdayofweek = false;
			enblWeekDay2 = false;
			enblstartmonth = false;
			enblstarthour = false;
			enblstartMin = false;
			enbledayrule1 = false;
			enblendday = false;			
			enbledayrule2 = false;
			enblendweek = false;
			enblendDayofWeek = false;
			enblendmonth = false;
			enblendhour = false;
			enblendmin = false;
			enblTimecancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Timev == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Time VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(TimeLock ==true)
		{
			enblshowTime = false;
			enblshowDate = false;
			enblntpServer = false;
			enblserveraddr = false;
			enblTimeZone = false;
			enbldaylightsave = false;
			enblsdayrule1 = false;
			enblstartday = false;		
			enblsdayrule2 = false;
			enblstartdayofweek = false;
			enblWeekDay2 = false;
			enblstartmonth = false;
			enblstarthour = false;
			enblstartMin = false;
			enbledayrule1 = false;
			enblendday = false;			
			enbledayrule2 = false;
			enblendweek = false;
			enblendDayofWeek = false;
			enblendmonth = false;
			enblendhour = false;
			enblendmin = false;
			enblTimecancelsubmit = false;
		}
	}
	json_object_object_add(Jobj, "TimeVisibilityLevel", json_object_new_int(Timev));
	json_object_object_add(Jobj, "TimeLimitation", json_object_new_boolean(TimeLimitation));
	json_object_object_add(Jobj, "TimeLock", json_object_new_boolean(TimeLock));
	json_object_object_add(Jobj, "enblshowTime", json_object_new_boolean(enblshowTime));
	json_object_object_add(Jobj, "enblshowDate", json_object_new_boolean(enblshowDate));
	json_object_object_add(Jobj, "enblntpServer", json_object_new_boolean(enblntpServer));
	json_object_object_add(Jobj, "enblserveraddr", json_object_new_boolean(enblserveraddr));
	json_object_object_add(Jobj, "enblTimeZone", json_object_new_boolean(enblTimeZone));
	json_object_object_add(Jobj, "enbldaylightsave", json_object_new_boolean(enbldaylightsave));
	json_object_object_add(Jobj, "enblsdayrule1", json_object_new_boolean(enblsdayrule1));
	json_object_object_add(Jobj, "enblstartday", json_object_new_boolean(enblstartday));
	json_object_object_add(Jobj, "enblsdayrule2", json_object_new_boolean(enblsdayrule2));
	json_object_object_add(Jobj, "enblstartdayofweek", json_object_new_boolean(enblstartdayofweek));
	json_object_object_add(Jobj, "enblWeekDay2", json_object_new_boolean(enblWeekDay2));
	json_object_object_add(Jobj, "enblstartmonth", json_object_new_boolean(enblstartmonth));
	json_object_object_add(Jobj, "enblstarthour", json_object_new_boolean(enblstarthour));
	json_object_object_add(Jobj, "enblstartMin", json_object_new_boolean(enblstartMin));
	json_object_object_add(Jobj, "enbledayrule1", json_object_new_boolean(enbledayrule1));
	json_object_object_add(Jobj, "enblendday", json_object_new_boolean(enblendday));
	json_object_object_add(Jobj, "enbledayrule2", json_object_new_boolean(enbledayrule2));
	json_object_object_add(Jobj, "enblendweek", json_object_new_boolean(enblendweek));
	json_object_object_add(Jobj, "enblendDayofWeek", json_object_new_boolean(enblendDayofWeek));
	json_object_object_add(Jobj, "enblendmonth", json_object_new_boolean(enblendmonth));
	json_object_object_add(Jobj, "enblendhour", json_object_new_boolean(enblendhour));
	json_object_object_add(Jobj, "enblendmin", json_object_new_boolean(enblendmin));
	json_object_object_add(Jobj, "enblTimecancelsubmit", json_object_new_boolean(enblTimecancelsubmit));

	IID_INIT(LogSettingIid);
	if ( zcfgFeObjJsonGet(RDM_OID_LOGSETTING, &LogSettingIid,&LogSettingJarray) == ZCFG_SUCCESS)
	{
		LogSettingv = Jgeti(LogSettingJarray, "VisibilityLevel");
		LogSettingLimitation = Jgetb(LogSettingJarray, "Limitation");
		LogSettingLock = Jgetb(LogSettingJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (LogSettingv == 2 || LogSettingv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): LogSetting VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (LogSettingLock ==true)
		{
			enblsyslog = false;
			enblmodeSelect = false;
			enblserver_addr = false;
			enbludp_port = false;
			enbllogMail = false;
			enbllogMailSubject = false;
			enblsecurityLogMailSubject = false;
			enbllogMailAddress = false;
			enblalertMailAddress = false;
			enblalarminterval = false;
			enbllogCat0 = false;
			enbllogCat1 = false;
			enblLogSettingcancelsubmit = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (LogSettingv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): LogSetting VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(LogSettingLock ==true)
		{
			enblsyslog = false;
			enblmodeSelect = false;
			enblserver_addr = false;
			enbludp_port = false;
			enbllogMail = false;
			enbllogMailSubject = false;
			enblsecurityLogMailSubject = false;
			enbllogMailAddress = false;
			enblalertMailAddress = false;
			enblalarminterval = false;
			enbllogCat0 = false;
			enbllogCat1 = false;
			enblLogSettingcancelsubmit = false;
		}
	}

	json_object_object_add(Jobj, "LogSettingVisibilityLevel", json_object_new_int(LogSettingv));
	json_object_object_add(Jobj, "LogSettingLimitation", json_object_new_boolean(LogSettingLimitation));
	json_object_object_add(Jobj, "LogSettingLock", json_object_new_boolean(LogSettingLock));
	json_object_object_add(Jobj, "enblsyslog", json_object_new_boolean(enblsyslog));
	json_object_object_add(Jobj, "enblmodeSelect", json_object_new_boolean(enblmodeSelect));
	json_object_object_add(Jobj, "enblserver_addr", json_object_new_boolean(enblserver_addr));
	json_object_object_add(Jobj, "enbludp_port", json_object_new_boolean(enbludp_port));
	json_object_object_add(Jobj, "enbllogMail", json_object_new_boolean(enbllogMail));
	json_object_object_add(Jobj, "enbllogMailSubject", json_object_new_boolean(enbllogMailSubject));
	json_object_object_add(Jobj, "enblsecurityLogMailSubject", json_object_new_boolean(enblsecurityLogMailSubject));
	json_object_object_add(Jobj, "enbllogMailAddress", json_object_new_boolean(enbllogMailAddress));
	json_object_object_add(Jobj, "enblalertMailAddress", json_object_new_boolean(enblalertMailAddress));
	json_object_object_add(Jobj, "enblalarminterval", json_object_new_boolean(enblalarminterval));
	json_object_object_add(Jobj, "enbllogCat0", json_object_new_boolean(enbllogCat0));
	json_object_object_add(Jobj, "enbllogCat1", json_object_new_boolean(enbllogCat1));
	json_object_object_add(Jobj, "enblLogSettingcancelsubmit", json_object_new_boolean(enblLogSettingcancelsubmit));

	IID_INIT(FWUpgradeIid);
	if ( zcfgFeObjJsonGet(RDM_OID_FIRMWARE_UPGRADE, &FWUpgradeIid,&FWUpgradeJarray) == ZCFG_SUCCESS)
	{
		FWUpgradev = Jgeti(FWUpgradeJarray, "VisibilityLevel");
		FWUpgradeLimitation = Jgetb(FWUpgradeJarray, "Limitation");
		FWUpgradeLock = Jgetb(FWUpgradeJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (FWUpgradev == 2 || FWUpgradev == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): FWUpgrade VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (FWUpgradeLock ==true)
		{
			enblfilename = false;
			enblFWUpgradeupload = false;
			enblwwanfile = false;
			enblbtnwwanupload = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (FWUpgradev == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): FWUpgrade VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(FWUpgradeLock ==true)
		{
			enblfilename = false;
			enblFWUpgradeupload = false;
			enblwwanfile = false;
			enblbtnwwanupload = false;
		}
	}
	json_object_object_add(Jobj, "FWUpgradeVisibilityLevel", json_object_new_int(FWUpgradev));
	json_object_object_add(Jobj, "FWUpgradeLimitation", json_object_new_boolean(FWUpgradeLimitation));
	json_object_object_add(Jobj, "FWUpgradeLock", json_object_new_boolean(FWUpgradeLock));
	json_object_object_add(Jobj, "enblfilename", json_object_new_boolean(enblfilename));
	json_object_object_add(Jobj, "enblFWUpgradeupload", json_object_new_boolean(enblFWUpgradeupload));
	json_object_object_add(Jobj, "enblwwanfile", json_object_new_boolean(enblwwanfile));
	json_object_object_add(Jobj, "enblbtnwwanupload", json_object_new_boolean(enblbtnwwanupload));

	IID_INIT(BackupIid);
	if ( zcfgFeObjJsonGet(RDM_OID_CONFIGURATION, &BackupIid,&BackupJarray) == ZCFG_SUCCESS)
	{
		Backupv = Jgeti(BackupJarray, "VisibilityLevel");
		BackupLimitation = Jgetb(BackupJarray, "Limitation");
		BackupLock = Jgetb(BackupJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Backupv == 2 || Backupv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Backup VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if(BackupLimitation==false)
		{
			if(BackupLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Backup Lock VisibilityLevel is equal to 0\n", __FUNCTION__);
			}
			else if(BackupLock ==true)
			{				
				enblBackup = false;
				enblBackupfilename = false;
				enblBackupupload = false;
				enblresetbtn = false;
				//ALL content will be grayed-out for admin
			}
		}
		else if (BackupLimitation==true)
		{
			if(BackupLock ==false)
			{
				zcfgLog(ZCFG_LOG_INFO, "%s(): Backup Lock VisibilityLevel is equal to 0\n", __FUNCTION__);
			}
			else if(BackupLock ==true)
			{				
				enblBackup = false;
				enblBackupfilename = false;
				enblBackupupload = false;
				enblresetbtn = false;
				//grayed-out with Limitation parameter
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (Backupv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Backup VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(BackupLock ==false)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Backup Lock VisibilityLevel is equal to 0\n", __FUNCTION__);
		}
		else if(BackupLock ==true)
		{
			enblBackup = false;
			enblBackupfilename = false;
			enblBackupupload = false;
			enblresetbtn = false;
			//see all content grayed-out
		}
	}
	json_object_object_add(Jobj, "BackupVisibilityLevel", json_object_new_int(Backupv));
	json_object_object_add(Jobj, "BackupLimitation", json_object_new_boolean(BackupLimitation));
	json_object_object_add(Jobj, "BackupLock", json_object_new_boolean(BackupLock));
	json_object_object_add(Jobj, "enblBackup", json_object_new_boolean(enblBackup));
	json_object_object_add(Jobj, "enblBackupfilename", json_object_new_boolean(enblBackupfilename));
	json_object_object_add(Jobj, "enblBackupupload", json_object_new_boolean(enblBackupupload));
	json_object_object_add(Jobj, "enblresetbtn", json_object_new_boolean(enblresetbtn));

	IID_INIT(MGMTIid);
	if ( zcfgFeObjJsonGet(RDM_OID_REMOTE_MGMT, &MGMTIid,&MGMTJarray) == ZCFG_SUCCESS)
	{
		MGMTv = Jgeti(MGMTJarray, "VisibilityLevel");
		MGMTLimitation = Jgetb(MGMTJarray, "Limitation");
		MGMTLock = Jgetb(MGMTJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (MGMTv == 2 || MGMTv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): MGMT VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		if(MGMTLimitation == false){
			if(MGMTLock == true){
				enblLanid = false;
				enblwlanid = false;
				enblwanid = false;
				enblport = false;
				enblMGMTcancelsubmit = false;
			}
		}
		else if(MGMTLimitation == true){
			if(MGMTLock == false){
				enbltelnetLanid = false;
				enbltelnetwanid = false;
				enbltelnetport = false;
				enbltelnetwlanid = false;
			}
			else if(MGMTLock == true){
				enblLanid = false;
				enblwlanid = false;
				enblwanid = false;
				enblport = false;
				enblMGMTcancelsubmit = false;
			}
		}
	}
	else if(curloginLevel == 2)
	{
		if (MGMTv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): MGMT VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(MGMTLock ==true)
		{
			enblLanid = false;
			enblwlanid = false;
			enblwanid = false;
			enblport = false;
			enblMGMTcancelsubmit = false;
		}
	}

	IID_INIT(RemoteAccessIid);
	if (zcfgFeObjJsonGet(RDM_OID_REMOTE_ACCESS, &RemoteAccessIid, &RemoteAccessJarray) == ZCFG_SUCCESS)  
	{
		RemoteEnable = Jgetb(RemoteAccessJarray, "Enable");
	}
	else
	{
		printf("failed to get <RDM_OID_REMOTE_ACCESS> object.\n");
		RemoteEnable = false;
	}
	
	IID_INIT(LocalAccessIid);
	if (zcfgFeObjJsonGet(RDM_OID_LOCAL_ACCESS, &LocalAccessIid, &LocalAccessJarray) == ZCFG_SUCCESS)  
	{
		LocalEnable = Jgetb(LocalAccessJarray, "Enable");
	}
	else
	{
		printf("failed to get <RDM_OID_LOCAL_ACCESS> object.\n");
		LocalEnable = false;
	}
	
	if((TAAAGMultiUser == 1) && (LocalEnable == false))	/*__MSTC__, Linda, CPE User Definitions and Rights. */
	{
		enbllocallan = false;
	}
	json_object_object_add(Jobj, "MGMTVisibilityLevel", json_object_new_int(MGMTv));
	json_object_object_add(Jobj, "MGMTLimitation", json_object_new_boolean(MGMTLimitation));
	json_object_object_add(Jobj, "MGMTLock", json_object_new_boolean(MGMTLock));
	json_object_object_add(Jobj, "enblLanid", json_object_new_boolean(enblLanid));
	json_object_object_add(Jobj, "enblwlanid", json_object_new_boolean(enblwlanid));
	json_object_object_add(Jobj, "enblwanid", json_object_new_boolean(enblwanid));
	json_object_object_add(Jobj, "enblport", json_object_new_boolean(enblport));
	json_object_object_add(Jobj, "enbltelnetLanid", json_object_new_boolean(enbltelnetLanid));
	json_object_object_add(Jobj, "enbltelnetwanid", json_object_new_boolean(enbltelnetwanid));
	json_object_object_add(Jobj, "enbltelnetport", json_object_new_boolean(enbltelnetport));
	json_object_object_add(Jobj, "enbltelnetwlanid", json_object_new_boolean(enbltelnetwlanid));
	json_object_object_add(Jobj, "enblMGMTcancelsubmit", json_object_new_boolean(enblMGMTcancelsubmit));
	json_object_object_add(Jobj, "enbllocallan", json_object_new_boolean(enbllocallan));
	json_object_object_add(Jobj, "RemoteEnable", json_object_new_boolean(RemoteEnable));

	IID_INIT(LogIid);
	if ( zcfgFeObjJsonGet(RDM_OID_LOG, &LogIid,&LogJarray) == ZCFG_SUCCESS)
	{
		Logv = Jgeti(LogJarray, "VisibilityLevel");
		LogLimitation = Jgetb(LogJarray, "Limitation");
		LogLock = Jgetb(LogJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (Logv == 2 || Logv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Log VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (LogLock ==true)
		{
			enbllevel = false;
			enblcategory = false;
			enblclear = false;
			enblrefresh = false;
			enblexport = false;
			enblemail = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (Logv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Log VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(LogLock ==true)
		{
			enbllevel = false;
			enblcategory = false;
			enblclear = false;
			enblrefresh = false;
			enblexport = false;
			enblemail = false;
		}
	}
	json_object_object_add(Jobj, "LogVisibilityLevel", json_object_new_int(Logv));
	json_object_object_add(Jobj, "LogLimitation", json_object_new_boolean(LogLimitation));
	json_object_object_add(Jobj, "LogLock", json_object_new_boolean(LogLock));
	json_object_object_add(Jobj, "enbllevel", json_object_new_boolean(enbllevel));
	json_object_object_add(Jobj, "enblcategory", json_object_new_boolean(enblcategory));
	json_object_object_add(Jobj, "enblclear", json_object_new_boolean(enblclear));
	json_object_object_add(Jobj, "enblrefresh", json_object_new_boolean(enblrefresh));
	json_object_object_add(Jobj, "enblexport", json_object_new_boolean(enblexport));
	json_object_object_add(Jobj, "enblemail", json_object_new_boolean(enblemail));

	IID_INIT(portmirrorIid);
	if ( zcfgFeObjJsonGet(RDM_OID_ENGDEBUG, &portmirrorIid,&portmirrorJarray) == ZCFG_SUCCESS)
	{
		portmirrorv = Jgeti(portmirrorJarray, "VisibilityLevel");
		portmirrorLimitation = Jgetb(portmirrorJarray, "Limitation");
		portmirrorLock = Jgetb(portmirrorJarray, "Lock");
	}
	json_object_object_add(Jobj, "portmirrorVisibilityLevel", json_object_new_int(portmirrorv));
	json_object_object_add(Jobj, "portmirrorLimitation", json_object_new_boolean(portmirrorLimitation));
	json_object_object_add(Jobj, "portmirrorLock", json_object_new_boolean(portmirrorLock));

	IID_INIT(packetcaptureIid);
	if ( zcfgFeObjJsonGet(RDM_OID_CAPTURE, &packetcaptureIid,&packetcaptureJarray) == ZCFG_SUCCESS)
	{
		packetcapturev = Jgeti(packetcaptureJarray, "VisibilityLevel");
		packetcaptureLimitation = Jgetb(packetcaptureJarray, "Limitation");
		packetcaptureLock = Jgetb(packetcaptureJarray, "Lock");
	}
	json_object_object_add(Jobj, "packetcaptureVisibilityLevel", json_object_new_int(packetcapturev));
	json_object_object_add(Jobj, "packetcaptureLimitation", json_object_new_boolean(packetcaptureLimitation));
	json_object_object_add(Jobj, "packetcaptureLock", json_object_new_boolean(packetcaptureLock));

	IID_INIT(vlanIid);
	if ( zcfgFeObjJsonGet(RDM_OID_LAN_VLAN, &vlanIid,&vlanJarray) == ZCFG_SUCCESS)
	{
		vlanv = Jgeti(vlanJarray, "VisibilityLevel");
		vlanLimitation = Jgetb(vlanJarray, "Limitation");
		vlanLock = Jgetb(vlanJarray, "Lock");
	}

	if(curloginLevel == 1)
	{
		if (vlanv == 2 || vlanv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Vlan VisibilityLevel is equal to 2 or 3\n", __FUNCTION__);
		}
		else if (vlanLock ==true)
		{
			enblvlanaddnew = false;
		}
	}
	else if(curloginLevel == 2)
	{
		if (vlanv == 3)
		{
			zcfgLog(ZCFG_LOG_INFO, "%s(): Vlan VisibilityLevel is equal to 3\n", __FUNCTION__);
		}
		else if(vlanLock ==true)
		{
			enblvlanaddnew = false;
		}
	}
	json_object_object_add(Jobj, "vlanVisibilityLevel", json_object_new_int(vlanv));
	json_object_object_add(Jobj, "vlanLimitation", json_object_new_boolean(vlanLimitation));
	json_object_object_add(Jobj, "vlanLock", json_object_new_boolean(vlanLock));
	json_object_object_add(Jobj, "enblvlanaddnew", json_object_new_boolean(enblvlanaddnew));

	IID_INIT(EMIid);
	if ( zcfgFeObjJsonGet(RDM_OID_E_M__REMOTE_ACCESS, &EMIid,&EM_Remote) == ZCFG_SUCCESS)
	{
		EM_RmoteAccess= Jgetb(EM_Remote, "Lock");
	}
	json_object_object_add(Jobj, "EM_RmoteAccess", json_object_new_boolean(EM_RmoteAccess));
	IID_INIT(EMIid);
	if ( zcfgFeObjJsonGet(RDM_OID_E_M__LOCAL_ACCESS, &EMIid,&EM_local) == ZCFG_SUCCESS)
	{
		EM_LocalAccess= Jgetb(EM_local, "Lock");
	}
	json_object_object_add(Jobj, "EM_LocalAccess", json_object_new_boolean(EM_LocalAccess));

	IID_INIT(Tr069Iid);
	if ( zcfgFeObjJsonGet(RDM_OID_TR069, &Tr069Iid,&Tr069Jarray) == ZCFG_SUCCESS)
	{
		Tr069v = Jgeti(Tr069Jarray, "VisibilityLevel");
		Tr069Limitation = Jgetb(Tr069Jarray, "Limitation");
		Tr069Lock = Jgetb(Tr069Jarray, "Lock");
	}
	json_object_object_add(Jobj, "Tr069VisibilityLevel", json_object_new_int(Tr069v));
	json_object_object_add(Jobj, "Tr069Limitation", json_object_new_boolean(Tr069Limitation));
	json_object_object_add(Jobj, "Tr069Lock", json_object_new_boolean(Tr069Lock));

	IID_INIT(EDNSPageIid);
	if ( zcfgFeObjJsonGet(RDM_OID_EDNS, &EDNSPageIid,&EDNSPageJarray) == ZCFG_SUCCESS)
	{
		EDNSv = Jgeti(EDNSPageJarray, "VisibilityLevel");
		EDNSLimitation = Jgetb(EDNSPageJarray, "Limitation");
		EDNSLock = Jgetb(EDNSPageJarray, "Lock");
	}
	json_object_object_add(Jobj, "EDNSVisibilityLevel", json_object_new_int(EDNSv));
	json_object_object_add(Jobj, "EDNSLimitation", json_object_new_boolean(EDNSLimitation));
	json_object_object_add(Jobj, "EDNSLock", json_object_new_boolean(EDNSLock));

	json_object_array_add(*Jarray, Jobj);
		
	json_object_put(B_PAGEJarray);
	json_object_put(Internet_DSLJarray);
	json_object_put(Internet_WANJarray);
	json_object_put(IPTV_DSLJarray);
	json_object_put(IPTV_WANJarray);
	json_object_put(AdvancedJarray);
	json_object_put(GeneralJarray);
	json_object_put(GuestJarray);
	json_object_put(WPSJarray);
	json_object_put(WMMJarray);
	json_object_put(OthersJarray);	
	json_object_put(LANSetupJarray);	
	json_object_put(RoutingJarray);	
	json_object_put(ALGJarray);	
	json_object_put(DosJarray);	
	json_object_put(TimeJarray);	
	json_object_put(LogSettingJarray);
	json_object_put(FWUpgradeJarray);
	json_object_put(BackupJarray);
	json_object_put(MGMTJarray);
	json_object_put(Acs_DslJarray);
	json_object_put(Acs_WanJarray);
	json_object_put(LogJarray);
	json_object_put(portmirrorJarray);
	json_object_put(packetcaptureJarray);
	json_object_put(vlanJarray);
	json_object_put(RemoteAccessJarray);
	json_object_put(LocalAccessJarray);
	json_object_put(ManagementJarray);
	json_object_put(EM_Remote);
	json_object_put(EM_local);
	json_object_put(Tr069Jarray);
	json_object_put(EDNSPageJarray);
		
	return ZCFG_SUCCESS;
}
#endif

void invalidCharFilter(char *input, int maxlen){
	int i = 0;
	int length = strlen(input);
	
	if(NULL != input){
		for( i = 0; i < length; i++){
			if(input[i] == ';' || input[i] == '&' || input[i] == '|')
				input[i] = '\0';
		}
		if(length > maxlen){
			input[maxlen-1] = '\0';
		}
	}
}

bool HostNameFilter(const char *input){
	bool result = false;
	
	regex_t preg;
	regmatch_t matchptr[1];
	const size_t nmatch = 1;
	const char* pattern = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$";
	int status;
#ifdef WIND_ITALY_CUSTOMIZATION
	char product[64] = {0};
	char host[128] = {0};

	if(zyUtilIGetProductName(product) == ZCFG_SUCCESS)
		zos_snprintf(host, sizeof(host), "Wind3 HUB - %s", product);
	
	if(!strcmp(input, host))
		return true;
#endif
	
	regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);
	status = regexec(&preg, input, nmatch, matchptr, 0);
	if (status == REG_NOMATCH){
	    printf("invalid Hostname\n");
	    result = false;
	}
	else if (status == 0){
	    printf("valid Hostname\n");
	    result = true;
	}
	regfree(&preg);
	return result;
}

bool DomainFilter(const char *input){
	bool result = false;
	
	regex_t preg;
	regmatch_t matchptr[1];
	const size_t nmatch = 1;
	const char* pattern = "^([^-][A-Za-z0-9-]{1,63}\\.)+([A-Za-z0-9-]{1,63}\\.)*+[A-Za-z]{2,6}+([.]?)$";
	int status, maxlen = 254;
	
	if(strlen(input) > maxlen)
		return result;

	regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);
	status = regexec(&preg, input, nmatch, matchptr, 0);
	if (status == REG_NOMATCH){
	    printf("invalid Domain\n");
	    result = false;
	}
	else if (status == 0){
	    printf("valid Domain\n");
	    result = true;
	}
	regfree(&preg);
	return result;
}

bool IPFilter(const char *input){
	bool result = false;
	
	regex_t preg;
	regmatch_t matchptr[1];
	const size_t nmatch = 1;
	const char* pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
	int status;
	
	regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);
	status = regexec(&preg, input, nmatch, matchptr, 0);
	if (status == REG_NOMATCH){
	    printf("invalid IP\n");
	    result = false;
	}
	else if (status == 0){
	    printf("valid IP\n");
	    result = true;
	}
	regfree(&preg);
	return result;
}

bool IPv6Filter(const char *input){
	bool result = false;
	
	regex_t preg;
	regmatch_t matchptr[1];
	const size_t nmatch = 1;
	const char* pattern = "^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|(fe80:(:[0-9a-fA-F]{0,4}){0,6}(((25[0-5]|(2[0-4][0-9]|1{0,1}[0-9]|[0-9]?[0-9]))\\.){3,3}(25[0-5]|(2[0-4][0-9]|1[0-9][0-9]|[0-9]?[0-9])))?(/((1[0-2][0-8]|1{0,1}[0-9])))?)|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$";
	int status;
	
	regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);
		
	status = regexec(&preg, input, nmatch, matchptr, 0);
	if (status == REG_NOMATCH){
	    printf("invalid IPv6 address\n");
	    result = false;
	}
	else if (status == 0){
	    printf("valid IPv6 address\n");
	    result = true;
	}
	regfree(&preg);
	return result;
}

#if 0
/*
 * return WAN and LAN list in Jason Array.
 * Format:Jarray.i.NAME = Name
 *           Jarray.i.IpIfacePath = IP.Interface.i
 *           Jarray.i.PppIfacePath = PPP.Interface.i  (For PPPoE, PPPoA only)
 *           Jarray.i.Type : LAN|WAN
 *           Jarray.i.LinkType = ATM|PTM|ETH|PON|USB (For WAN only)
 *           Jarray.i.ConnectionType = IP_Routed|IP_Bridged (For WAN only)
 *           Jarray.i.Encapsulation = IPoE|IPoA|PPPoE|PPPoA (For WAN only, NUL if LinkType is USB))
 *           Jarray.i.BindToIntfGrp = true|false (For WAN only)
 *           Jarray.i.LowerLayerUp = true|false (For  IPoE, PPPoE only, other WAN LowerLayerUp value may not correct)
 *           Jarray.i.WANConnectionReady = true|false (For  WAN only)
 *           Jarray.i.DefaultGateway = true|false (For  WAN only)
 *           Jarray.i.Group_WAN_IpIface = IP.Interface.i,IP.Interface.i,IP.Interface.i,... (For LAN only)
 *           Jarray.i.BridgingBrPath = Bridging.Bridge.i (For LAN only)
 */
zcfgRet_t getWanLanList(struct json_object **Jarray){
	struct json_object *ipIfaceJarray, *pppIfaceJarray, *vlanTermJarray, *ethLinkJarray, *bridgeBrJarray;
	struct json_object *ipIfaceObj, *pppIfaceObj, *vlanTermObj, *ethLinkObj, *bridgeBrObj, *routerObj;
	struct json_object *v4AddrJarray, *v6AddrJarray, *v4AddrObj, *v6AddrObj;
	struct json_object *Jobj = NULL;
	char ifacePath[32] = {0}, BridgingBrPath[32] = {0};
	int len, len2, i, j;
	unsigned char idx;
	const char *X_ZYXEL_BridgeName, *X_ZYXEL_SrvName, *X_ZYXEL_ConnectionType, *X_ZYXEL_IfName, *X_ZYXEL_Group_WAN_IpIface, *LowerLayers;
	const char *Status, *ipIfaceStatus, *IPAddress, *X_ZYXEL_ActiveDefaultGateway;
	const char *Username, Password;
	char intfGrpWANList[256] = {0};
	char *intf = NULL, *tmp_ptr = NULL;
	const char *IpIfacePath = NULL;

	zcfgFeWholeObjJsonGet("Device.IP.Interface.", &ipIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.PPP.Interface.", &pppIfaceJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.VLANTermination.", &vlanTermJarray);
	zcfgFeWholeObjJsonGet("Device.Ethernet.Link.", &ethLinkJarray);
	zcfgFeWholeObjJsonGet("Device.Bridging.Bridge.", &bridgeBrJarray);
	zcfgFeWholeObjJsonGet("Device.Routing.Router.1.", &routerObj);

	X_ZYXEL_ActiveDefaultGateway = Jgets(routerObj, "X_ZYXEL_ActiveDefaultGateway");

	if(ipIfaceJarray==NULL||pppIfaceJarray==NULL||vlanTermJarray==NULL||ethLinkJarray==NULL||bridgeBrJarray==NULL)
		return ZCFG_INTERNAL_ERROR;

	*Jarray = json_object_new_array();

	len = json_object_array_length(ipIfaceJarray);
	for(i=0;i<len;i++){
		ipIfaceObj = json_object_array_get_idx(ipIfaceJarray, i);
		X_ZYXEL_SrvName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_SrvName"));
		X_ZYXEL_IfName = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_IfName"));
		X_ZYXEL_ConnectionType = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_ConnectionType"));
		LowerLayers = json_object_get_string(json_object_object_get(ipIfaceObj, "LowerLayers"));
		X_ZYXEL_Group_WAN_IpIface = json_object_get_string(json_object_object_get(ipIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
		sprintf(ifacePath, "IP.Interface.%u", i+1);
		if(X_ZYXEL_SrvName==NULL || X_ZYXEL_ConnectionType==NULL || LowerLayers==NULL){
			continue;
		}

		Jobj = json_object_new_object();
		json_object_object_add(Jobj, "IpIfacePath", json_object_new_string(ifacePath));
		json_object_object_add(Jobj, "Name", json_object_new_string(X_ZYXEL_SrvName));

		json_object_object_add(Jobj, "Type", json_object_new_string("WAN"));
		json_object_object_add(Jobj, "ConnectionType", json_object_new_string(X_ZYXEL_ConnectionType));
		json_object_object_add(Jobj, "BindToIntfGrp", json_object_new_boolean(false));
		json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(false));
		json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(false));
		Jadds(Jobj, "PppIfacePath", "");
		//json_object_object_add(Jobj, "pppUsername", json_object_new_string(""));
		//json_object_object_add(Jobj, "pppPassword", json_object_new_string(""));
		
		if(findInList(X_ZYXEL_ActiveDefaultGateway, ifacePath, ","))
			json_object_object_add(Jobj, "DefaultGateway", json_object_new_boolean(true));
		else
			json_object_object_add(Jobj, "DefaultGateway", json_object_new_boolean(false));

		ipIfaceStatus = json_object_get_string(json_object_object_get(ipIfaceObj, "Status"));
		if(ipIfaceStatus != NULL && !strcmp(ipIfaceStatus, "Up")){  //|| !strcmp(ipIfaceStatus, "LowerLayerDown")
			v4AddrJarray = Jget(ipIfaceObj, "IPv4Address");
			if(v4AddrJarray != NULL){
				len2 = json_object_array_length(v4AddrJarray);
				for(j=0;j<len2;j++){
					v4AddrObj = json_object_array_get_idx(v4AddrJarray, j);
					if(isEmptyIns(v4AddrObj))
						continue;
					IPAddress = json_object_get_string(json_object_object_get(v4AddrObj,"IPAddress"));
					Status = json_object_get_string(json_object_object_get(v4AddrObj, "Status"));
					if(IPAddress != NULL && Status != NULL && strcmp(IPAddress, "") && !strcmp(Status, "Enabled")){
						json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(true));
						break;
					}
				}
			}
			if(!Jgetb(Jobj, "WANConnectionReady")){
			v6AddrJarray = Jget(ipIfaceObj, "IPv6Address");
			if(v6AddrJarray != NULL){
				len2 = json_object_array_length(v6AddrJarray);
				for(j=0;j<len2;j++){
					v6AddrObj = json_object_array_get_idx(v6AddrJarray, j);
					if(isEmptyIns(v6AddrObj))
						continue;
					IPAddress = json_object_get_string(json_object_object_get(v6AddrObj,"IPAddress"));
					Status = json_object_get_string(json_object_object_get(v6AddrObj, "Status"));
					if(IPAddress != NULL && Status != NULL && strcmp(IPAddress, "") && !strcmp(Status, "Enabled")){
						json_object_object_add(Jobj, "WANConnectionReady", json_object_new_boolean(true));
						break;
					}
				}
			}
			}
		}

		
		int count = 0;
		const char *HigherLayer = "IP.Interface.";
		while(count < 4){
			count++;
			if(LowerLayers == NULL){
				json_object_put(Jobj);
				Jobj = NULL;
				break;
			}
			if(!strncmp(HigherLayer, "IP.Interface.", 13)){
				if(!strncmp(LowerLayers, "PPP.Interface.", 14)){
					sscanf(LowerLayers, "PPP.Interface.%hhu", &idx);
					pppIfaceObj = json_object_array_get_idx(pppIfaceJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(pppIfaceObj, "LowerLayers"));
					
					replaceParam(Jobj, "IfName", pppIfaceObj, "X_ZYXEL_IfName");
				}
				else if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoE"));
				}
				else if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &idx);
					ethLinkObj = json_object_array_get_idx(ethLinkJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9) && !strcmp("IP_Routed", X_ZYXEL_ConnectionType)){
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("IPoA"));
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp(HigherLayer, "PPP.Interface.", 14)){
				Jadds(Jobj, "PppIfacePath", HigherLayer);
				//replaceParam(Jobj, "pppUsername", pppIfaceObj, "Username");
				//replaceParam(Jobj, "pppPassword", pppIfaceObj, "Password");
						
				if(!strncmp("Ethernet.VLANTermination.", LowerLayers, 25)){
					sscanf(LowerLayers, "Ethernet.VLANTermination.%hhu", &idx);
					vlanTermObj = json_object_array_get_idx(vlanTermJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(vlanTermObj, "LowerLayers"));
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoE"));
				}
				else if(!strncmp("ATM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "Encapsulation", json_object_new_string("PPPoA"));
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					Status = json_object_get_string(json_object_object_get(pppIfaceObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					Status = json_object_get_string(json_object_object_get(pppIfaceObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.VLANTermination.", HigherLayer, 25)){
				if(!strncmp("Ethernet.Link.", LowerLayers, 14)){
					sscanf(LowerLayers, "Ethernet.Link.%hhu", &idx);
					ethLinkObj = json_object_array_get_idx(ethLinkJarray, idx-1);
					HigherLayer = LowerLayers;
					LowerLayers = json_object_get_string(json_object_object_get(ethLinkObj, "LowerLayers"));
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
			else if(!strncmp("Ethernet.Link.", HigherLayer, 14)){
				if(!strncmp("ATM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ATM"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("PTM.Link.", LowerLayers, 9)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PTM"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("Ethernet.Interface.", LowerLayers, 19)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("Optical.Interface.", LowerLayers, 18)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("PON"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("USB.Interface.", LowerLayers, 14)){
					json_object_object_add(Jobj, "LinkType", json_object_new_string("USB"));
					Status = json_object_get_string(json_object_object_get(ethLinkObj, "Status"));
					if(Status != NULL && strstr(Status, "Up"))
						json_object_object_add(Jobj, "LowerLayerUp", json_object_new_boolean(true));
					break;
				}
				else if(!strncmp("Bridging.Bridge.", LowerLayers, 16)){
					sscanf(LowerLayers, "Bridging.Bridge.%hhu.Port.1", &idx);
					bridgeBrObj = json_object_array_get_idx(bridgeBrJarray, idx-1);
					X_ZYXEL_BridgeName = json_object_get_string(json_object_object_get(bridgeBrObj, "X_ZYXEL_BridgeName"));
					if(X_ZYXEL_BridgeName == NULL){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					if(X_ZYXEL_Group_WAN_IpIface == NULL)
						X_ZYXEL_Group_WAN_IpIface = "";
						
					sprintf(BridgingBrPath, "Bridging.Bridge.%u", idx);
					json_object_object_add(Jobj, "Name", json_object_new_string(X_ZYXEL_BridgeName));
					json_object_object_add(Jobj, "Type", json_object_new_string("LAN"));
					json_object_object_add(Jobj, "BridgingBrPath", json_object_new_string(BridgingBrPath));
					json_object_object_add(Jobj, "Group_WAN_IpIface", json_object_new_string(X_ZYXEL_Group_WAN_IpIface));
					strcat(intfGrpWANList, X_ZYXEL_Group_WAN_IpIface);
					strcat(intfGrpWANList, ",");
					break;
				}
				else if(!strcmp("", LowerLayers)){
					if(json_object_get_boolean(json_object_object_get(ethLinkObj, "X_ZYXEL_OrigEthWAN")) == false){
						json_object_put(Jobj);
						Jobj = NULL;
						break;
					}
					json_object_object_add(Jobj, "LinkType", json_object_new_string("ETH"));
					break;
				}
				else{
					json_object_put(Jobj);
					Jobj = NULL;
					break;
				}
			}
		}
		
		if(Jobj != NULL)
			json_object_array_add(*Jarray, Jobj);
	}

	if(strcmp(intfGrpWANList, "")){
		intf = strtok_r(intfGrpWANList, ",", &tmp_ptr);
		while(intf != NULL){
			if(*intf != '\0'){
				len = json_object_array_length(*Jarray);
				for(i=0;i<len;i++){
					Jobj = json_object_array_get_idx(*Jarray, i);
					IpIfacePath = json_object_get_string(json_object_object_get(Jobj, "IpIfacePath"));
					if(!strcmp(IpIfacePath, intf)){
						json_object_object_add(Jobj, "BindToIntfGrp", json_object_new_boolean(true));
					}
				}
			}
			intf = strtok_r(NULL, ",", &tmp_ptr);
		}
	}
	//printf("*Jarray=%s\n", json_object_to_json_string(*Jarray));
	return ZCFG_SUCCESS;
}
#endif
/*
 * return WAN and LAN list in Jason Array.
 * Format:Jarray.i.NAME = Name
 *           Jarray.i.BrPortPath = Bridging.Bridge.i.Port.i
 *           Jarray.i.IntfPath : Ethernet.Interface.i|WiFi.SSID.i
 *           Jarray.i.Type : ETHLAN|WiFiSSID
 */
zcfgRet_t getLanPortList(struct json_object **Jarray){
	objIndex_t iid;	
	struct json_object *Obj = NULL;
	struct json_object *mappingObj = NULL;
	struct json_object *Jobj = NULL;
	char intfPath[64] = {0}, brPortPath[64] = {0};
	const char *LowerLayers = NULL, *X_ZYXEL_LanPort = NULL, *BrPortPath = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	const char *SSID = NULL;
#endif

	*Jarray = json_object_new_array();

	mappingObj = json_object_new_object();
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_BRIDGING_BR_PORT, &iid, &Obj) == ZCFG_SUCCESS) {
		LowerLayers = json_object_get_string(json_object_object_get(Obj, "LowerLayers"));
		if(!strncmp(LowerLayers, "Ethernet.Interface.", 19) || !strncmp(LowerLayers, "WiFi.SSID.", 10)){
			sprintf(brPortPath, "Bridging.Bridge.%u.Port.%u", iid.idx[0], iid.idx[1]);
			json_object_object_add(mappingObj, LowerLayers, json_object_new_string(brPortPath));
		}
		json_object_put(Obj);
	}

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ETH_IFACE, &iid, &Obj) == ZCFG_SUCCESS) {
		if(json_object_get_boolean(json_object_object_get(Obj, "Upstream")) || json_object_get_boolean(json_object_object_get(Obj, "X_ZYXEL_Upstream"))){
			json_object_put(Obj);
			continue;
		}

		sprintf(intfPath, "Ethernet.Interface.%u", iid.idx[0]);
		X_ZYXEL_LanPort = json_object_get_string(json_object_object_get(Obj, "X_ZYXEL_LanPort"));
		BrPortPath = json_object_get_string(json_object_object_get(mappingObj, intfPath));
		if(BrPortPath == NULL)
			BrPortPath = "";
		if(X_ZYXEL_LanPort == NULL)
			X_ZYXEL_LanPort = "";
			
		Jobj = json_object_new_object();
		json_object_object_add(Jobj, "Name", json_object_new_string(X_ZYXEL_LanPort));
		json_object_object_add(Jobj, "BrPortPath", json_object_new_string(BrPortPath));
		json_object_object_add(Jobj, "IntfPath", json_object_new_string(intfPath));
		json_object_object_add(Jobj, "Type", json_object_new_string("ETHLAN"));
		json_object_array_add(*Jarray, Jobj);

		json_object_put(Obj);
	}

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &iid, &Obj) == ZCFG_SUCCESS) {
		if(json_object_get_int(json_object_object_get(Obj, "X_ZYXEL_Attribute")) == 0) {
			sprintf(intfPath, "WiFi.SSID.%u", iid.idx[0]);
			SSID = json_object_get_string(json_object_object_get(Obj, "SSID"));
			BrPortPath = json_object_get_string(json_object_object_get(mappingObj, intfPath));
			if(BrPortPath == NULL)
				BrPortPath = "";
			Jobj = json_object_new_object();
			json_object_object_add(Jobj, "Name", json_object_new_string(SSID));
			json_object_object_add(Jobj, "BrPortPath", json_object_new_string(BrPortPath));
			json_object_object_add(Jobj, "IntfPath", json_object_new_string(intfPath));
			json_object_object_add(Jobj, "Type", json_object_new_string("WiFiSSID"));
			json_object_array_add(*Jarray, Jobj);
		}
		json_object_put(Obj);
	}
#else
	printf("%s : CONFIG_ZCFG_BE_MODULE_WIFI not support\n", __func__);
#endif

	json_object_put(mappingObj);
	return ZCFG_SUCCESS;
}


zcfgRet_t getRouteIntfList(struct json_object **Jarray){
	objIndex_t intfIid;	
	struct json_object *obj = NULL;
	struct json_object *intfObj = NULL;
	const char *connection = NULL;
	const char *name = NULL;
	const char *type = NULL;
	int count = 1;
	*Jarray = json_object_new_array();
	IID_INIT(intfIid);	
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_IP_IFACE, &intfIid, &intfObj) == ZCFG_SUCCESS){		
		obj = json_object_new_object();
		connection = json_object_get_string(json_object_object_get(intfObj, "X_ZYXEL_ConnectionType"));
		type = json_object_get_string(json_object_object_get(intfObj, "Name"));
		name = json_object_get_string(json_object_object_get(intfObj, "X_ZYXEL_SrvName"));
		json_object_object_add(obj, "Name", json_object_new_string(name));
		json_object_object_add(obj, "ConnectionType", json_object_new_string(connection));
		json_object_object_add(obj, "Type", json_object_new_string(type));
		json_object_object_add(obj, "Number", json_object_new_int(count));
		json_object_array_add(*Jarray, obj);
		
		json_object_put(intfObj);
		count++;
	}	
	return ZCFG_SUCCESS;
}

zcfgRet_t getClassList(struct json_object **Jarray){
	objIndex_t classIid;	
	struct json_object *obj = NULL;
	struct json_object *classObj = NULL;
	const char *policer = NULL;
	const char *name = NULL;
	*Jarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
	IID_INIT(classIid);	
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS){		
		obj = json_object_new_object();
		policer = json_object_get_string(json_object_object_get(classObj, "Policer"));
		name = json_object_get_string(json_object_object_get(classObj, "X_ZYXEL_Name"));
		json_object_object_add(obj, "Name", json_object_new_string(name));
		json_object_object_add(obj, "Policer", json_object_new_string(policer));
		json_object_array_add(*Jarray, obj);
		
		json_object_put(classObj);			
	}
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t getQueueList(struct json_object **Jarray){
	objIndex_t queueIid;	
	struct json_object *obj = NULL;
	struct json_object *queueObj = NULL;
	const char *trafficClasses = NULL;
	const char *name = NULL;
	int queueIdx = 0;
	*Jarray = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
	IID_INIT(queueIid);	
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		trafficClasses = json_object_get_string(json_object_object_get(queueObj, "TrafficClasses"));
		name = json_object_get_string(json_object_object_get(queueObj, "X_ZYXEL_Name"));
		queueIdx = queueIid.idx[0];
		json_object_object_add(obj, "Name", json_object_new_string(name));
		json_object_object_add(obj, "TrafficClasses", json_object_new_string(trafficClasses));
		json_object_object_add(obj, "QueueIdx", json_object_new_int(queueIdx));
		json_object_array_add(*Jarray, obj);
		
		json_object_put(queueObj);			
	}
#endif
	return ZCFG_SUCCESS;
}


zcfgRet_t getVoipMultiObj(struct json_object **VoipMultiObj){
#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
	objIndex_t voipIid;
	struct json_object *voipObj = NULL, *obj = NULL;
	char voipline[64] = {0}, sipnumber[64] = {0}, sip[8] = {0}, boundList[64] = {0};
	int count = 1;
	// const char *directoryNumber = NULL;
#endif

	*VoipMultiObj = json_object_new_array();

#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
	IID_INIT(voipIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_LINE, &voipIid, &voipObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		// directoryNumber = json_object_get_string(json_object_object_get(voipObj, "DirectoryNumber"));
		sprintf(sip,"SIP%d",count);
		//sprintf(voipline,"VoiceProfile.%d.Line.%d",voipIid.idx[1],voipIid.idx[2]);
		//sprintf(boundList,"Device.Services.VoiceService.1.VoiceProfile.%d.Line.%d",voipIid.idx[1],voipIid.idx[2]);
		sprintf(voipline,"Device.Services.VoiceService.1.VoiceProfile.%d.Line.%d",voipIid.idx[1],voipIid.idx[2]);
		sprintf(boundList,"VoiceProfile.%d.Line.%d",voipIid.idx[1],voipIid.idx[2]);
		//sprintf(sipnumber,"%d.SIP%d",count,count);
		sprintf(sipnumber,"SIP%d",count);
		json_object_object_add(obj, "SIP", json_object_new_string(sip));
		json_object_object_add(obj, "VoipLine", json_object_new_string(voipline));
		json_object_object_add(obj, "BoundList", json_object_new_string(boundList));
		json_object_object_add(obj, "SIPNumber", json_object_new_string(sipnumber));
		json_object_object_add(obj, "DirectoryNumber", JSON_OBJ_COPY(json_object_object_get(voipObj, "DirectoryNumber")));
		json_object_object_add(obj, "PhyReferenceFXSList", JSON_OBJ_COPY(json_object_object_get(voipObj, "X_ZYXEL_PhyReferenceFXSList")));
		json_object_array_add(*VoipMultiObj, obj);
		json_object_put(voipObj);
		count++;
	}
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t getVoipProviderObj(struct json_object **VoipProviderObj){
#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
	objIndex_t voipProviderIid;
	struct json_object *voipProviderObj = NULL, *obj = NULL;
	const char *name = NULL;
	char voipspname[64] = {0};
	int count = 1;
#endif
	*VoipProviderObj = json_object_new_array();

#ifdef CONFIG_ZCFG_BE_MODULE_VOICE
	IID_INIT(voipProviderIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VOICE_PROF, &voipProviderIid, &voipProviderObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		name = json_object_get_string(json_object_object_get(voipProviderObj, "Name"));
		sprintf(voipspname,"%d.%s",count,name);
		//json_object_object_add(obj, "VoipServiceProvider", json_object_new_string(voipspname));
		json_object_object_add(obj, "VoipServiceProvider", json_object_new_string(name));
		json_object_array_add(*VoipProviderObj, obj);
		json_object_put(voipProviderObj);
		count++;
	}
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t getAccountObj(struct json_object **AccountObj){
	//This function jsut get admin and user level account, for usb file share
	struct json_object *accountObj = NULL, *obj = NULL;
	objIndex_t accountIid = {0};
	accountIid.level = 2;
	accountIid.idx[0] = 2;
	*AccountObj = json_object_new_array();
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &accountIid, &accountObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		if(json_object_object_get(accountObj, "Enabled") && json_object_object_get(accountObj, "Username")){
			json_object_object_add(obj, "Status", JSON_OBJ_COPY(json_object_object_get(accountObj, "Enabled")));
			json_object_object_add(obj, "Username", JSON_OBJ_COPY(json_object_object_get(accountObj, "Username")));
			json_object_array_add(*AccountObj, obj);
		}
		json_object_put(accountObj);
	}
	return ZCFG_SUCCESS;
}

zcfgRet_t getUSBinfoObj(struct json_object **UsbObj){
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	//This function jsut get admin and user level account, for usb file share
	struct json_object *usbObj = NULL, *obj = NULL;
	objIndex_t usbIid = {0};
	usbIid.level = 3;
	usbIid.idx[0] = 1;
	usbIid.idx[1] = 2;
#endif
	*UsbObj = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_USB
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_USB_USBHOSTS_HOST_DEV_PAR, &usbIid, &usbObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		json_object_object_add(obj, "PartitionName", JSON_OBJ_COPY(json_object_object_get(usbObj, "PartitionName")));
		json_object_object_add(obj, "Capacity", JSON_OBJ_COPY(json_object_object_get(usbObj, "Capacity")));
		json_object_object_add(obj, "UsedSpace", JSON_OBJ_COPY(json_object_object_get(usbObj, "UsedSpace")));
		json_object_array_add(*UsbObj, obj);
		json_object_put(usbObj);
	}
	return ZCFG_SUCCESS;
#endif
}

zcfgRet_t getVlanWithoutBrRefKeyinfoObj(struct json_object **VlanObj){
	struct json_object *obj = NULL;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	struct json_object *vlanObj = NULL;
	objIndex_t vlanIid = {0};
	const char *vlantmp = NULL;
#endif
	char vlanName[256] = {0};
	int len = 0;
	
	*VlanObj = json_object_new_array();
	obj = json_object_new_object();
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_VLAN_GROUP, &vlanIid, &vlanObj) == ZCFG_SUCCESS){
		if(!strcmp(json_object_get_string(json_object_object_get(vlanObj, "BrRefKey")),"")){
			vlantmp = json_object_get_string(json_object_object_get(vlanObj, "GroupName"));
			strcat(vlanName,vlantmp);
			strcat(vlanName,",");
		}
		json_object_put(vlanObj);
		vlanObj = NULL;
	}
#endif
	len = strlen(vlanName);
	if(len >0){
		if(vlanName[len-1] == ',')
			vlanName[len-1] = '\0';
	}
	if(!strcmp(vlanName,"")){
		strcat(vlanName, "Vlan type cannot support now, please add a vlan rule.");
	}
	json_object_object_add(obj, "VlanName", json_object_new_string(vlanName));
	json_object_array_add(*VlanObj, obj);

	return ZCFG_SUCCESS;
}

zcfgRet_t getwlanModeObj(struct json_object **modeObj){
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *obj = NULL;
	struct json_object *radioObj = NULL;
	objIndex_t radioIid = {0};
	const char *band = NULL;
	const char *stardard = NULL;
	char buf[32] = {0}, mode[64] = {0};
	char *tmp = NULL, *tmp2 = NULL, *b = NULL, *g = NULL, *n = NULL, *a = NULL, *ac = NULL, *ax = NULL;
	int len = 0;
#endif
	*modeObj = json_object_new_array();

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_RADIO, &radioIid, &radioObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		mode[0] = '\0';
		band = json_object_get_string(json_object_object_get(radioObj, "SupportedFrequencyBands"));
		stardard = json_object_get_string(json_object_object_get(radioObj, "SupportedStandards"));
		if(!strcmp(band, "2.4GHz")){
			ZOS_STRCPY_M(buf, stardard);
			b = strtok_r(buf, ",", &tmp);
			g = strtok_r(tmp, ",", &tmp2);
			n = strtok_r(tmp2, ",", &ax);
			if(!strcmp(b, "b")){
				strcat(mode, "11");
				strcat(mode,b);
				strcat(mode,",");
			}
			if(!strcmp(g, "g")){
				strcat(mode, "11");
				strcat(mode,g);
				strcat(mode,",");
			}
			if(!strcmp(n, "n")){
				strcat(mode, "11");
				strcat(mode,n);
				strcat(mode,",");
			}
			if(!strcmp(b, "b") && !strcmp(g, "g")){
				strcat(mode, "11");
				strcat(mode,b);
				//strcat(mode,"/");
				strcat(mode,g);
				strcat(mode,",");
			}
			if(!strcmp(b, "b") && !strcmp(g, "g") && !strcmp(n, "n")){
				strcat(mode, "11");
				strcat(mode,b);
				//strcat(mode,"/");
				strcat(mode,g);
				//strcat(mode,"/");
				strcat(mode,n);
				strcat(mode,",");
			}
			if(!strcmp(b, "b") && !strcmp(g, "g") && !strcmp(n, "n") && !strcmp(ax, "ax")){
				strcat(mode, "11");
				strcat(mode,b);
				strcat(mode,g);
				strcat(mode,n);
				strcat(mode,ax);
				strcat(mode,",");
			}
			len = strlen(mode);
			if(len > 0){
				if(mode[len-1] == ',')
					mode[len-1] = '\0';
			}
			
		}
		else if(!strcmp(band, "5GHz")){
			ZOS_STRCPY_M(buf, stardard);
			a = strtok_r(buf, ",", &tmp);
			n = strtok_r(tmp, ",", &tmp2);
			ac = strtok_r(tmp2, ",", &ax);
			if(!strcmp(a, "a")){
				strcat(mode, "11");
				strcat(mode,a);
				strcat(mode,",");
			}
			if(!strcmp(n, "n")){
				strcat(mode, "11");
				strcat(mode,n);
				strcat(mode,",");
			}
			if(!strcmp(ac, "ac")){
				strcat(mode, "11");
				strcat(mode,ac);
				strcat(mode,",");
			}
			if(!strcmp(a, "a") && !strcmp(n, "n")){
				strcat(mode, "11");
				strcat(mode,a);
				//strcat(mode,"/");
				strcat(mode,n);
				strcat(mode,",");
			}
			if(!strcmp(a, "a") && !strcmp(n, "n") && !strcmp(ac, "ac")){
				strcat(mode, "11");
				strcat(mode,a);
				//strcat(mode,"/");
				strcat(mode,n);
				//strcat(mode,"/");
				strcat(mode,ac);
				strcat(mode,",");
			}
			if(!strcmp(a, "a") && !strcmp(n, "n") && !strcmp(ac, "ac") && !strcmp(ax, "ax")){
				strcat(mode, "11");
				strcat(mode,a);
				strcat(mode,n);
				strcat(mode,ac);
				strcat(mode,ax);
				strcat(mode,",");
			}
			len = strlen(mode);
			if(len > 0){
				if(mode[len-1] == ',')
					mode[len-1] = '\0';
			}

		}
		json_object_object_add(obj, "band", json_object_new_string(band));
		json_object_object_add(obj, "mode", json_object_new_string(mode));
		json_object_array_add(*modeObj, obj);
		
		json_object_put(radioObj);
		radioObj = NULL;
	}
#else
	printf("%s : CONFIG_ZCFG_BE_MODULE_WIFI not support\n", __func__);
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t getwifiSSIDObj(struct json_object **ssidObj){
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	struct json_object *wifissidObj = NULL;
	struct json_object *obj = NULL;
	objIndex_t wifissidIid = {0};
	const char *ssid = NULL;
#endif

	*ssidObj = json_object_new_array();
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &wifissidIid, &wifissidObj) == ZCFG_SUCCESS){
		obj = json_object_new_object();
		ssid = json_object_get_string(json_object_object_get(wifissidObj, "SSID"));
		json_object_object_add(obj, "SSID", json_object_new_string(ssid));
		json_object_object_add(obj, "idx", json_object_new_int(wifissidIid.idx[0]));
		json_object_array_add(*ssidObj, obj);
		json_object_put(wifissidObj);
		wifissidObj = NULL;
	}
#else
	printf("%s : CONFIG_ZCFG_BE_MODULE_WIFI not support\n", __func__);
#endif

	return ZCFG_SUCCESS;
}

const char *getLoginUserName(char *userName)
{
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (pw)
	{
		ZOS_STRCPY_M(userName, pw->pw_name);
		return pw->pw_name;
	}
	else{
		ZOS_STRCPY_M(userName, "");
		return NULL;
	}
}


/*
 * only used for dalcmd, cant be used by httpd, since the httpd user is root.
*/
zcfgRet_t getPrivilegeData(char *loginUserName, char *level, char *list){
	const char *userName = NULL, *userLevel = NULL, *page = NULL;
	objIndex_t iid;
	struct json_object *obj = NULL;
	bool found = false;
		
	if(getLoginUserName(loginUserName) == NULL)
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP_ACCOUNT, &iid, &obj) == ZCFG_SUCCESS) {
		userName = json_object_get_string(json_object_object_get(obj, "Username"));
		if(!strcmp(userName, loginUserName)){
			found = true;
			json_object_put(obj);
			break;
		}
		json_object_put(obj);
	}

	if(!found)
		return ZCFG_INTERNAL_ERROR;

	iid.level--;
	iid.idx[iid.level] = 0;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_ZY_LOG_CFG_GP, &iid, &obj) == ZCFG_SUCCESS){
		userLevel = json_object_get_string(json_object_object_get(obj, "Level"));
		page = json_object_get_string(json_object_object_get(obj, "Page"));
		ZOS_STRCPY_M(level, userLevel);
		ZOS_STRCPY_M(list, page);
		json_object_put(obj);
		return ZCFG_SUCCESS;
	}
	else
		return ZCFG_INTERNAL_ERROR;	
	
}

zcfgRet_t zyshPrivilege(char *zyshlist, char *allList){
	char userName[17] = {0}, level[16] = {0}, privilegeList[2049] = {0};
	int i, len = 0, len1 = 0;

	if(getPrivilegeData(userName, level, privilegeList) != ZCFG_SUCCESS)
		return ZCFG_INTERNAL_ERROR;

	for(i=0; dalHandler[i].name != NULL; i++){
		if(dalHandler[i].previlege != NULL && strcmp(dalHandler[i].previlege, "") && (!strcmp(level, "high") || findInList(privilegeList, dalHandler[i].previlege, ","))){
			strcat(zyshlist, dalHandler[i].name);
			strcat(zyshlist, ",");
		}
		strcat(allList, dalHandler[i].name);
		strcat(allList, ",");
	}
	len = strlen(zyshlist);
	if(len>0)
		if(zyshlist[len-1] == ',')
	zyshlist[len-1] = '\0';
	len1 = strlen(allList);
	if(len1>0)
		if(allList[len1-1] == ',')
	allList[len1-1] = '\0';

	return ZCFG_SUCCESS;
}




int isDigitString(char *getString)
{
	while(*getString != '\0'){
		if(!isdigit(*getString)){
			return 0;
		}
		getString++;
	}
	
	return 1;
}

//find key2 in Jobj2, if it exist add it to Jobj1 as key1
int replaceParam(struct json_object *Jobj1, char *key1, struct json_object *Jobj2, char *key2){
	struct json_object *pramJobj = NULL;

	if(Jobj1 == NULL || Jobj2 == NULL)
		return 0;

	pramJobj = json_object_object_get(Jobj2, key2);
	if(pramJobj){
		json_object_object_add(Jobj1, key1, JSON_OBJ_COPY(pramJobj));
		return 1;
	}
	return 0;
}

/*!
 *  Get parameter value from RDM giving oid, iid, and parameter name
 *
 *  @param[in]     oid        Target oid
 *  @param[in]     objIid     Target iid
 *  @param[in]     key        Target parameter name
 *  @param[out]    value      Parameter value
 *
 *  @return     true  - Get value success
 *              false - Wrong input / Object not found / parameter not found
 */
zcfgRet_t getObjParamStr (zcfg_offset_t oid, objIndex_t *objIid, char *key, char *value)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid;
	struct json_object *Obj = NULL;

	if (key == NULL)
	{
		dbg_printf("%s:%d: key can't be null, please give a specific parameter name \n",__FUNCTION__,__LINE__);
		return ZCFG_REQUEST_REJECT;
	}

	if (value == NULL)
	{
		dbg_printf("%s:%d: value can't be null, please initiate it first \n",__FUNCTION__,__LINE__);
		return ZCFG_REQUEST_REJECT;
	}
	
	IID_INIT(iid);
	iid.level = 1;
	iid.idx[0] = objIid->idx[0];
	if ((ret = zcfgFeObjJsonGet(oid, &iid, &Obj)) != ZCFG_SUCCESS)
		return ZCFG_NO_SUCH_OBJECT;

	if (!json_object_object_get(Obj, key))
		return ZCFG_NO_SUCH_PARAMETER;

	ZOS_STRCPY_M(value,json_object_get_string(json_object_object_get(Obj, key)));
	json_object_put(Obj);
	return ret;
}

bool validateEmailAddr(const char *emailAddr){
	char buff[128] = {0}, tmp1[128] = {0}, tmp3[128] = {0};
    char *temp1 = NULL, *temp2 = NULL, *temp3 = NULL;
	int i, count = 0;
    ZOS_STRCPY_M(buff, emailAddr);
    temp1 = strtok_r(buff, "@", &temp2);
	if(!temp1)
		return false;
	ZOS_STRCPY_M(tmp1,temp1);
	for(i = 0; tmp1[i] != 0; i++){
		if(tmp1[i] < 45 || tmp1[i] == 47 || (57 < tmp1[i] && tmp1[i] < 65) || (90 < tmp1[i] && tmp1[i] < 95) || tmp1[i] == 96 || tmp1[i] > 122)
        	return false;
    }	
    temp3 = strtok_r(temp2, ".", &temp2);
	if(!strcmp(temp2,""))
		return false;
    while(temp3 != NULL){
		if(strlen(temp3) < 2)
			return false;
		ZOS_STRCPY_M(tmp3,temp3);
		if(count == 0){
			for(i = 0; tmp3[i] != 0; i++){
		        if(tmp3[i] < 45 || tmp3[i] == 46 || tmp3[i] == 47 || (57 < tmp3[i] && tmp3[i] < 65) || (90 < tmp3[i] && tmp3[i] < 95) || tmp3[i] == 96 || tmp3[i] > 122)
					return false;
		    }
		}
		else{
			for(i = 0; tmp3[i] != 0; i++){
		        if(tmp3[i] < 47 || (57 < tmp3[i] && tmp3[i] < 65) || (90 < tmp3[i] && tmp3[i] < 97) || tmp3[i] > 123)
		            return false;
		    }
		}
		count++;
		temp3 = strtok_r(temp2, ".", &temp2);
	}
    return true;
}

#if 1
bool validateMacAddr1(const char *macAddr, const char *delim){
	char buff[32];
    char *p, *tmp;
	int count = 0;

#if 0
	if(strchr(macAddr, '-')){
		delim = "-";
	}
	else if(strchr(macAddr, ':')){
		delim = ":";
	}
	else{
		return false;
	}
#endif
	
	ZOS_STRCPY_M(buff, macAddr);
	p = strtok_r(buff, delim, &tmp);
	while(p != NULL){
		if(strlen(p) == 2){
			count++;
			if(count>6){
				return false;
			}
			if((*p<'0' || *p>'9') && (*p<'a' || *p>'f') && (*p<'A' || *p>'F')){
				return false;
			}
			if((*(p+1)<'0' || *(p+1)>'9') && (*(p+1)<'a' || *(p+1)>'f') && (*(p+1)<'A' || *(p+1)>'F')){
				return false;
			}
		}
		else{
			return false;
		}

		p = strtok_r(NULL, delim, &tmp);
	}
	if(count!=6)
		return false;
	
	return true;
}
#endif

bool validateIPv6(const char *ipStr){

	struct in6_addr ip6addr;
	char *tmp_ptr = NULL, *ip6addr_tmp = NULL;
	char buf[128]={0};

	ZOS_STRCPY_M(buf, ipStr);
	if(strstr(buf, ",") != NULL)
	{
		ip6addr_tmp = strtok_r(buf, ",", &tmp_ptr);
		while(ip6addr_tmp != NULL)
		{
			if (inet_pton(AF_INET6, ip6addr_tmp, &ip6addr) <= 0)
				return false;

			ip6addr_tmp = strtok_r(NULL, ",", &tmp_ptr);
		}
		return true;
	}

	if (inet_pton(AF_INET6, ipStr, &ip6addr) <= 0)
		return false;

	return true;
} 

bool isHexString(char hexStr[]){
	int len = 0, i;

	len = strlen(hexStr);
	if(len > 4)
		return false;
	for(i=0;i<len;i++){
		if((hexStr[i]<'0' || hexStr[i]>'9') && (hexStr[i]<'a' || hexStr[i]>'f') && (hexStr[i]<'A' || hexStr[i]>'F')){
			return false;
		}
	}
	return true;
}

bool IPv6IndentifierType(const char *ipv6indentifiertype){
	char buff[64];
	char p1[32] = {0}, p2[32] = {0}, p3[32] = {0}, p4[32] = {0};

	ZOS_STRNCPY(buff, ipv6indentifiertype, sizeof(buff));
	if(buff[strlen(ipv6indentifiertype) - 1] == ':')
		return false;
	sscanf(buff, "%[^:]:%[^:]:%[^:]:%s", p1, p2, p3, p4);
	if(!strcmp(p4,""))
		return false;
	if(isHexString(p1) == false || isHexString(p2) == false || isHexString(p3) == false || isHexString(p4) == false)
		return false;

	return true;
}

bool validateLocalIPv6(const char *ipStr){
	char tmp[5];
	ZOS_STRNCPY(tmp, ipStr, sizeof(tmp));  //sizeof(tmp) should be 4+1
	if(strcasecmp(tmp, "fe80"))
		return false;
	if(!validateIPv6(ipStr))
		return false;
	return true;
}

bool validateIPv4(const char *ipStr){
	int i, cnt = 0, value;
	const char *cp = ipStr;

	if(ipStr == NULL)
		return false;
	
	for(; *cp != '\0'; cp++) {
		if (*cp == '.')
			cnt++;
		else if((!isdigit(*cp)) && (*cp != '.')) {
			return false;
		}
	}
	if (cnt !=3)
		return false;

	cnt =0;
	cp = ipStr;
	for(i = 24; i >= 0; i -= 8) {
		if((*cp == '\0') || (*cp == '.'))
			return false;
		
		value = atoi(cp);
		if((value > 255) || (value < 0))
			return false;

		cnt++;
		if((cp = strchr(cp, '.')) == NULL)
			break;			
		cp++;
	}
	
	if(cnt != 4)
		return false;

	return true;
}

bool validateMacAddr(const char *macaddr){
	char buff[32] = {0};
	char c, d;
	char *macstr = NULL, *tmp = NULL;
	int count = 0;

	ZOS_STRCPY_M(buff,macaddr);
	macstr = strtok_r(buff, ":", &tmp);
	while(macstr != NULL){
		c = *macstr; 
		d = *(macstr+1); 
		if(!isxdigit(c) || !isxdigit(d))
			return false;
		macstr = strtok_r(tmp, ":", &tmp);
		count++;
	}
	if(count != 6)
		return false;
	return true;
}

int getLeftMostZeroBitPos(int num) {
	int i = 0;   
	int numArr[8] = {128, 64, 32, 16, 8, 4, 2, 1};   
	for ( i = 0; i < 8; i++ ){
		if((num & numArr[i])== 0)
			return i;
	}
	return 8;
}

int getRightMostOneBitPos(int num) {
	int i = 0;   
	int numArr[8] = {1, 2, 4, 8, 16, 32, 64, 128};  
	int c = 0;
	for ( i = 0; i < 8; i++ ){  
		c = (num & numArr[i]);
		if(c>>i)
			return (8 - i - 1); 
	}
	return -1;
}

void zcfgInitDAL()
{
	objIndex_t iid;
	IID_INIT(iid);
	if(guiCustomObj)
		json_object_put(guiCustomObj);
	zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &iid, &guiCustomObj);
}

bool retrieveGUICustomizationFlags(uint32_t *flag1, uint32_t *flag2, uint32_t *flag3, uint32_t *flagVoIP1)
{
	if(!guiCustomObj)
		return false;

	if(flag1)
		*flag1 = json_object_get_int(json_object_object_get(guiCustomObj, "Flag1"));
	if(flag2)
		*flag2 = json_object_get_int(json_object_object_get(guiCustomObj, "Flag2"));
	if(flag3)
		*flag3 = json_object_get_int(json_object_object_get(guiCustomObj, "Flag3"));
	if(flagVoIP1)
		*flagVoIP1 = json_object_get_int(json_object_object_get(guiCustomObj, "FlagVoIP1"));

	return true;
}


bool validMacMask(const char *mask) { 
	char buff[32] = {0};
	int zeros = 0;   
	int zeroBitPos = 0, oneBitPos = 0;   
	bool zeroBitExisted = false;
	char *macstr = NULL, *tmp = NULL;
	int count = 0,num = 0;
	char c, d;
	ZOS_STRCPY_M(buff,mask);
	macstr = strtok_r(buff, ":", &tmp);	
	while(macstr != NULL){
		if(!strcmp(macstr,"00"))
			zeros++;
		c = *macstr; 
		d = *(macstr+1); 
		if(!isxdigit(c) || !isxdigit(d))
			return false;
		sscanf(macstr,"%x",&num);
		if (zeroBitExisted == true && num != 0)         
			return false;
		zeroBitPos = getLeftMostZeroBitPos(num);	  
		oneBitPos = getRightMostOneBitPos(num);	 
		if (zeroBitPos < oneBitPos)		 
			return false;		
		if (zeroBitPos < 8)		  
			zeroBitExisted = true;   
		macstr = strtok_r(tmp, ":", &tmp);
		count++;
	}	
	if(count != 6)
		return false;
	if(zeros == 6)
		return false;
	return true;
}


bool validateIPv4mask(const char *submask){
	unsigned char oneFound = 0;
	uint32_t mask;
	uint32_t netmask = inet_addr(submask);
	
	if(!strcmp(submask, "255.255.255.255"))
		return true;
	else if(netmask == -1)
		return false;
	
	netmask = ntohl(netmask);
	for (mask = 1; mask; mask <<= 1){
		if (netmask & mask)
			oneFound++;
		else if(oneFound)
			return false;
	}
	return true;
}

bool validateServerName(const char *servername){
	char buf[128] = {0};
	char *tmp = NULL, *prefix = NULL;
	ZOS_STRCPY_M(buf,servername);
	prefix = strtok_r(buf, ".", &tmp);
	if( prefix == NULL )printf("%s: delimiters was found!\n", __FUNCTION__);
	if(!strcmp(tmp,""))
		return false;
	return true;	
}

bool validatetime(const char *time){
	int month = 0, date = 0, hour = 0, minute = 0; 

	if(sscanf(time, "%d-%d-%d:%d", &month, &date, &hour, &minute) != 4)
		return false;
	
	if(month >12 || month < 0)
		return false;
	if(date > 31 || date <1)
		return false;
	if(hour > 23 || hour < 0)
		return false;
	if(minute > 60 || minute <0)
		return false;
	return true;	
}

#ifdef CONFIG_ABZQ_CUSTOMIZATION
zcfgRet_t insertLoginUserInfo(struct json_object *Jobj, const char *userName, const char *level, const char *remoteAddress, const char *accessMode, const char *srvName){
#else
zcfgRet_t insertLoginUserInfo(struct json_object *Jobj, const char *userName, const char *level, const char *remoteAddress){
#endif	
	struct json_object *subJobj;
#ifdef CONFIG_ABZQ_CUSTOMIZATION
	if(!Jobj || !userName || !level || !remoteAddress || !accessMode || !srvName){
		printf("%s: value is NULL\n",__FUNCTION__ );
#else
	if(!Jobj || !userName || !level || !remoteAddress){
#endif
		return ZCFG_INTERNAL_ERROR;
}
	
	if(json_object_get_type(Jobj) == json_type_object){
		json_object_object_add(Jobj, DAL_LOGIN_LEVEL, json_object_new_string(level));
		json_object_object_add(Jobj, DAL_LOGIN_USERNAME, json_object_new_string(userName));
		json_object_object_add(Jobj, DAL_LOGIN_REMOTEADDR, json_object_new_string(remoteAddress));
		json_object_object_add(Jobj, DAL_LOGIN_TYPE, json_object_new_string("GUI"));
#ifdef CONFIG_ABZQ_CUSTOMIZATION
		json_object_object_add(Jobj, "AccessMode", json_object_new_string(accessMode));
		json_object_object_add(Jobj, "SrvName", json_object_new_string(srvName));
#endif
	}
	else if(json_object_get_type(Jobj) == json_type_array){
		subJobj = json_object_array_get_idx(Jobj, 0);
		if(!subJobj)
			return ZCFG_INTERNAL_ERROR;
		
		json_object_object_add(subJobj, DAL_LOGIN_LEVEL, json_object_new_string(level));
		json_object_object_add(subJobj, DAL_LOGIN_USERNAME, json_object_new_string(userName));
		json_object_object_add(subJobj, DAL_LOGIN_REMOTEADDR, json_object_new_string(remoteAddress));
		json_object_object_add(subJobj, DAL_LOGIN_TYPE, json_object_new_string("GUI"));
	}
	else
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}

zcfgRet_t getLoginUserInfo(struct json_object *Jobj, char *userName, char *level, char *remoteAddress){
	struct json_object *subJobj;

	if(!Jobj)
		return ZCFG_INTERNAL_ERROR;
	
	if(json_object_get_type(Jobj) == json_type_object){
		if(userName) ZOS_STRCPY_M(userName, json_object_get_string(json_object_object_get(Jobj, DAL_LOGIN_USERNAME)));
		if(level) ZOS_STRCPY_M(level, json_object_get_string(json_object_object_get(Jobj, DAL_LOGIN_LEVEL)));
		if(remoteAddress) ZOS_STRCPY_M(remoteAddress, json_object_get_string(json_object_object_get(Jobj, DAL_LOGIN_REMOTEADDR)));
	}
	else if(json_object_get_type(Jobj) == json_type_array){
		subJobj = json_object_array_get_idx(Jobj, 0);
		if(!subJobj)
			return ZCFG_INTERNAL_ERROR;
		
		if(userName) ZOS_STRCPY_M(userName, json_object_get_string(json_object_object_get(subJobj, DAL_LOGIN_USERNAME)));
		if(level) ZOS_STRCPY_M(level, json_object_get_string(json_object_object_get(subJobj, DAL_LOGIN_LEVEL)));
		if(remoteAddress) ZOS_STRCPY_M(remoteAddress, json_object_get_string(json_object_object_get(subJobj, DAL_LOGIN_REMOTEADDR)));
	}
	else
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}

bool findDuplicate(char *list, char *duplicate, char *separator){
	char *buf, *ptr, *tmp_ptr = NULL;

	if(list == NULL || separator == NULL)
		return false;

	buf = strdup(list);
	ptr = strtok_r(buf, separator, &tmp_ptr);
	while(ptr != NULL){
		if(findInList(tmp_ptr, ptr, separator)){
			if(duplicate != NULL)
				ZOS_STRCPY_M(duplicate, ptr);
			ZOS_FREE(buf);
			return true;
		}
			
		ptr = strtok_r(NULL, separator, &tmp_ptr);
	}
	ZOS_FREE(buf);
	return false;
}

bool isSameSubNet( char *lanIp, char *lansubnetMask, char *staticIP) {
	unsigned int ip_n[4] = {0}, subnetMask_n[4] = {0}, staticIP_n[4] = {0};
	int count = 0;

	sscanf(lanIp, "%u.%u.%u.%u", &ip_n[3], &ip_n[2], &ip_n[1], &ip_n[0]);
	sscanf(lansubnetMask, "%u.%u.%u.%u", &subnetMask_n[3], &subnetMask_n[2], &subnetMask_n[1], &subnetMask_n[0]);
	sscanf(staticIP, "%u.%u.%u.%u", &staticIP_n[3], &staticIP_n[2], &staticIP_n[1], &staticIP_n[0]);
	for (int i = 0; i < 4; i++) {
		if ((ip_n[i] & subnetMask_n[i]) == (staticIP_n[i] & subnetMask_n[i]))
		  count++;
	}

	if (count == 4)
		return true;
	else
		return false;
}

//support only for char
bool checkValidateParameter (char *ptr, struct json_object **obj, char *key) {
	int len = 0, i;
	struct json_object *tmpObj = NULL;
	char *param = NULL;
	bool found = false;
	
	len = json_object_array_length(*obj);
	for (i=0; i<len; i++){
		tmpObj = json_object_array_get_idx(*obj, i);
		param = (char *)json_object_get_string(json_object_object_get(tmpObj, key));
		if(!strcmp(ptr, param)){
			found = true;
			break;
		}
	}
	if(found)
		return true;
	else
		return false;
}

bool parseValidateParameter(struct json_object *Jobj, char *method, char *handler_name, dal_param_t *paramList, char *invalidParm){
	int i, j, k=0;
	struct json_object *pramJobj = NULL;
	struct json_object *WanLanListJarray = NULL, *WanLanObj = NULL, *RouteIntfJarray = NULL, *intfObj = NULL, *LanPortListJarray = NULL, *QueueListJarray = NULL, *VoipList = NULL, *certListJarray = NULL, *LanPortObj = NULL;
	struct json_object *obj = NULL, *obj_tmp = NULL, *newObj = NULL;
	const char *stringPtr = NULL, *IpIfacePath = NULL, *Name = NULL, *Type = NULL, *LinkType = NULL, *ConnectionType = NULL, *Group_WAN_IpIface, *Interface = NULL, *IntfPath = NULL, *BrPortPath = NULL, *VoipName = NULL, *timezone = NULL, *timezonevalue = NULL, *Policer = NULL;
	bool BindToIntfGrp;
	char *ptr = NULL, *ptr_buf = NULL, *tmp_ptr = NULL, *nameList = NULL, *intfName = NULL, *tmp = NULL, *LANName = NULL, *account = NULL, *mode = NULL;
	char buf[64] = {0}, parenCtlServiceList[1024] = {0}, parenCtlService[128], tr181PathList[256] = {0}, route[32] = {0}, duplicateName[128] = {0}, brPath[32] = {0}, intfList[256] = {0}, queue[32] = {0}, voipList[1024] = {0}, usnAccount[256] = {0}, wlmode[16] = {0}, buff[128] = {0};
	int intValue = 0, len = 0, valid = 0, count = 0, Num = 0, CLIidx = 0, icmptye = 0, icmptypecode = 0, def = 0;
	bool ret = true, changetype = false, foundIntf = false;
	bool AtmUsed = false, PtmUsed = false, EthUsed = false, PonUsed = false, UsbUsed = false, ReUsed = false, wanIntf = false, found = false;
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
	bool CellUsed = false;
#endif
	char *macAddrDelim = NULL; // MAC address delimiter
	char allDays[] = "Mon,Tue,Wed,Thu,Fri,Sat,Sun";
	int startHour, startMin, stopHour, stopMin;
	char Days[32] = {0};
	unsigned char brIndex;
	char *GPON_tmp = "SLID";

	if(Jobj == NULL || paramList == NULL)
		return true;

	if(json_object_object_get(Jobj, "cmdtype")){	// GUI show Jobj, but CLI cannot show Jobj. Rex add 20170918
		if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI"))
	dbg_printf("Jobj=%s\n", json_object_to_json_string(Jobj));
	}

	for(i=0;paramList[i].paraName;i++){

        if(invalidParm != NULL)
        {   ZOS_STRCPY_M(invalidParm, paramList[i].paraName);   }

        if((pramJobj = json_object_object_get(Jobj, paramList[i].paraName)) != NULL){
			if(paramList[i].validate != NULL){
				ret = paramList[i].validate(paramList[i].type);
				return ret;
			}
			else{
				switch(paramList[i].type){
					case dalType_string:
						stringPtr = json_object_get_string(pramJobj);
						len = strlen(stringPtr);
						if(paramList[i].enumeration != NULL){
							sprintf(buf, "%s", stringPtr);
							if(findInList(paramList[i].enumeration, buf, "|") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						if(strcmp(paramList[i].paraName, GPON_tmp) == 0){
							paramList[i].min = 0;
							paramList[i].max = 0;
						}
						if(paramList[i].min || paramList[i].max){ // if min ==0 and max ==0, skip
							if(len<paramList[i].min || len>paramList[i].max){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						break;
					case dalType_v4AddrList:
						stringPtr = json_object_get_string(pramJobj);
						len = strlen(stringPtr);
						
						if(paramList[i].enumeration != NULL){
							sprintf(buf, "%s", stringPtr);
							if(findInList(paramList[i].enumeration, buf, "|") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}

						if(paramList[i].min || paramList[i].max){ // if min ==0 and max ==0, skip
							if(len<paramList[i].min || len>paramList[i].max){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}

						ptr_buf = strdup(stringPtr);
						ptr = strtok_r(ptr_buf, ",", &tmp_ptr);
						while ((ptr != NULL) && (strlen(ptr)>3))   
						{
							if(validateIPv4(ptr) == false){
								printf("Wrong parameter value <%s> of %s\n", ptr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}						
							ptr = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(ptr_buf);
						break;
					case dalType_int:
						if(json_object_get_type(pramJobj) == 9){
							stringPtr = json_object_get_string(pramJobj);
							intValue = atoi(stringPtr);
							changetype = true;
						}
						else
						intValue = json_object_get_int(pramJobj);

						if(paramList[i].enumeration != NULL){
							sprintf(buf, "%d", intValue);
							if(findInList(paramList[i].enumeration, buf, "|") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						else if(paramList[i].min || paramList[i].max){
							if(intValue<paramList[i].min || intValue>paramList[i].max){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						if(changetype){
							json_object_object_del(Jobj, paramList[i].paraName);
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_int(intValue));
							changetype = false;
						}
						break;
					case dalType_boolean:
						if(json_object_get_type(pramJobj) == 9){
							stringPtr = json_object_get_string(pramJobj);
							intValue = atoi(stringPtr);
							if(strcmp(stringPtr, "0") && strcmp(stringPtr, "1")){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								printf("Available value is <0|1>\n");
								return false;
							}
							changetype = true;
						}
						else
						intValue = json_object_get_boolean(pramJobj);
						
						if(changetype){
							json_object_object_del(Jobj, paramList[i].paraName);
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_boolean(intValue));
							changetype = false;
						}
						break;
					case dalType_v4Addr:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateIPv4(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_v4Mask:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateIPv4mask(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_v6Addr:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;

						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = ZOS_STRDUP(stringPtr);
						ptr = strchr(ptr_buf, '/');

						if (ptr != NULL)
						{
							*ptr = '\0';
							intValue = atoi(ptr+1);

							if (intValue < 0 || intValue > 128)
							{
								printf("Wrong intValue range 0 to 128 <%s> of %u   %s\n", stringPtr,paramList[i].type, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
						}

						if (validateIPv6(ptr_buf) == false)
						{
							printf("Wrong IPv6 address format <%s> of %u   %s\n", stringPtr, paramList[i].type, paramList[i].paraName);
							ZOS_FREE(ptr_buf);
							return false;
						}
						ZOS_FREE(ptr_buf);
						break;
					case dalType_v6AddrPrefix: //ex. 2001:123::11:22:33/64
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;

						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = ZOS_STRDUP(stringPtr);
						if((ptr = strchr(ptr_buf, '/')) == NULL)
						{
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							ZOS_FREE(ptr_buf);
							return false;
						}
						
						*ptr = '\0';
						intValue = atoi(ptr+1);
						if(intValue<0 || intValue>128){
							printf("Wrong intValue range 0 to 128 <%s> of %u   %s\n", stringPtr,paramList[i].type, paramList[i].paraName);
							ZOS_FREE(ptr_buf);
							return false;
						}
						if(validateIPv6(ptr_buf) == false){
							printf("Wrong IPv6 address format <%s> of %u   %s\n", stringPtr, paramList[i].type, paramList[i].paraName);
							ZOS_FREE(ptr_buf);
							return false;
						}
						ZOS_FREE(ptr_buf);
						break;
					case dalType_v4v6Addr:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateIPv4(stringPtr) == false && validateIPv6(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_v4v6AddrDomainName:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateIPv4(stringPtr) == false && validateIPv6(stringPtr) == false && validateServerName(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_PortRange:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						else{
							int startPort, endPort;
							ptr_buf = strdup(stringPtr);
							ptr = strchr(ptr_buf, ':');
							if(ptr != NULL){
								*ptr = '\0';
								if(!strcmp(ptr_buf, "") || (strcmp(ptr_buf, "-1") && !isDigitString(ptr_buf))){
									printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
									ZOS_FREE(ptr_buf);
									return false;
								}
								if(!strcmp(ptr+1, "") || (strcmp(ptr+1, "-1") && !isDigitString(ptr+1))){
									printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
									ZOS_FREE(ptr_buf);
									return false;
								}
								sscanf(stringPtr, "%d:%d", &startPort, &endPort);
								if(startPort == -1){
									endPort = -1;
								}
							}
							else{
								if(!strcmp(ptr_buf, "") || (strcmp(ptr_buf, "-1") && !isDigitString(ptr_buf))){
									printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
									ZOS_FREE(ptr_buf);
									return false;
								}
								startPort = atoi(stringPtr);
								endPort = -1;
							}
							ZOS_FREE(ptr_buf);
							if(startPort == -1){
								//pass case, do nothing
							}
							else if(startPort < -1 || startPort > 65535){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								printf("Start port must between -1 and 65535\n");
								return false;
							}
							else if(endPort < -1 || endPort > 65535){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								printf("End port must between -1 and 65535\n");
								return false;
							}
							else if(endPort != -1 && (startPort >= endPort)){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								printf("End port must larger than start port\n");
								return false;
							}
						}
						break;
					case dalType_vpivci:
						valid = 1;
						stringPtr = json_object_get_string(pramJobj);
						ptr_buf = strdup(stringPtr);
						ptr = strchr(ptr_buf, '/');
						if(ptr == NULL){
							valid = 0;
						}

						if(ptr){
							int vpi, vci;
							*ptr = '\0';
							if(!strcmp(ptr_buf, "") || !isDigitString(ptr_buf)){
								valid = 0;
							}
							if(!strcmp(ptr+1, "") || !isDigitString(ptr+1)){
								valid = 0;
							}
							sscanf(stringPtr, "%d/%d", &vpi, &vci);
							if(vpi<0 || vpi>255 || vci<32 || vci>65535){
								valid = 0;
							}
						}
						ZOS_FREE(ptr_buf);
						if(!valid){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
						case dalType_IPPrefix:
							stringPtr = json_object_get_string(pramJobj);
							if(paramList[i].enumeration != NULL){
								if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
									break;
								}
							ptr_buf = strdup(stringPtr);
							ptr = strchr(ptr_buf, '/');
							if(ptr == NULL || !strcmp(ptr+1, "") || !isDigitString(ptr+1)){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
							*ptr = '\0';
							intValue = atoi(ptr+1);
							if(strchr(ptr_buf, ':') != NULL){ //IPv6 Prefix
								if(intValue<0 || intValue>128 || validateIPv6(ptr_buf) == false){
									printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
									ZOS_FREE(ptr_buf);
									return false;
								}
							}
							else{ //IPv4 Prefix
								if(intValue<=0 || intValue>32 || validateIPv4(ptr_buf) == false){
									printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
									ZOS_FREE(ptr_buf);
									return false;
								}
							}
							ZOS_FREE(ptr_buf);
							break;
					case dalType_emailAddr:
					case dalType_emailList:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateEmailAddr(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_MacAddr:
						macAddrDelim = ":";
					case dalType_MacAddrHyphen:
						macAddrDelim = "-";
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						if(validateMacAddr1(stringPtr, macAddrDelim) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;						
						}
						break;
					case dalType_MacAddrList:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = strdup(stringPtr);
						count = 0;
						ptr = strtok_r(ptr_buf, ",", &tmp_ptr);
						while(ptr != NULL){
							if(validateMacAddr1(ptr, ":") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
							count++;
							ptr = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(ptr_buf);
						if(paramList[i].min || paramList[i].max){
							if((count > paramList[i].max) || (count < paramList[i].min)){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}							
						}
						break;
					case dalType_DayofWeek:
						Days[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = strdup(stringPtr);
						count = 0;
						ptr = strtok_r(ptr_buf, ",", &tmp_ptr);
						while(ptr != NULL){
							if(findInList(allDays, ptr, ",") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
							if(findInList(Days, ptr, ",") == false){ //prevent duplicate case
								count++;
								strcat(Days, ptr);
								strcat(Days, ",");
							}
							ptr = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(ptr_buf);
						if(count == 0){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						Days[strlen(Days)-1]='\0';
						break;
					case dalType_TimeRangeList:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = strdup(stringPtr);
						count = 0;
						ptr = strtok_r(ptr_buf, ",", &tmp_ptr);
						while(ptr != NULL){
							if(parseTimeRange(ptr, &startHour, &startMin, &stopHour, &stopMin) == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
							count++;
							ptr = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(ptr_buf);
						if(paramList[i].min || paramList[i].max){
							if((count > paramList[i].max) || (count < paramList[i].min)){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								printf("The number of time range must between %d and %d\n", paramList[i].min, paramList[i].max);
								return false;
							}
						}
						else if(count == 0){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
						}
						break;
					case dalType_URLFilterList:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						len = strlen(stringPtr);
						if(paramList[i].min || paramList[i].max){ // if min ==0 and max ==0, skip
							if(len<paramList[i].min || len>paramList[i].max){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}						
						}						
						break;
					case dalType_ParenCtlServiceList:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true)
								break;
						}
						ptr_buf = strdup(stringPtr);
						count = 0;
						memset(parenCtlServiceList, 0, sizeof(parenCtlServiceList));
						ptr = strtok_r(ptr_buf, "/", &tmp_ptr);
						while(ptr != NULL){
							if(parseParenService(ptr, parenCtlService, sizeof(parenCtlService)) == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
							else{
								json_object_object_del(Jobj, "NetworkServiceList");
								json_object_object_add(Jobj, "NetworkServiceList", json_object_new_string(parenCtlService));
							}
							
							count++;
							if(count>paramList[i].max)
								break;
							ptr = strtok_r(NULL, "/", &tmp_ptr);
						}
						ZOS_FREE(ptr_buf);
						if(paramList[i].max || paramList[i].min){
							if(count > paramList[i].max || count < paramList[i].min){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						else if(count == 0){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_LanRtWanWWan:
					case dalType_LanRtWan:
					case dalType_RtWanWWan:
					case dalType_MultiRtWanWWan:
					case dalType_RtWan:
					case dalType_BrWan:
					case dalType_Lan:
					case dalType_LanName:
						tr181PathList[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(!strncmp("IP.Interface.", stringPtr, 13)){
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
							break;
						}
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							break;
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true){
								if(paramList[i].type == dalType_RtWanWWan && !strcmp(stringPtr, "Default")){
									json_object_object_del(Jobj, paramList[i].paraName);
									json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(""));
								}
								break;
							}
						}
						if(WanLanListJarray == NULL)
							getWanLanList(&WanLanListJarray);
						if(paramList[i].type == dalType_MultiRtWanWWan){
							len = json_object_array_length(WanLanListJarray);
							nameList = strdup(stringPtr);
							intfName = strtok_r(nameList, ",", &tmp_ptr);

							while(intfName != NULL){
								for(j=0; j<len; j++){
									WanLanObj = json_object_array_get_idx(WanLanListJarray, j);
									IpIfacePath = json_object_get_string(json_object_object_get(WanLanObj, "IpIfacePath"));
									Name = json_object_get_string(json_object_object_get(WanLanObj, "Name"));
									if(!strcmp(intfName,Name)){
										strcat(tr181PathList,IpIfacePath);
										strcat(tr181PathList,",");
									}
								}	
								intfName = strtok_r(tmp_ptr, ",", &tmp_ptr);
							}
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(tr181PathList));
							ZOS_FREE(nameList);
							break;
						}
						
						foundIntf = false;
						len = json_object_array_length(WanLanListJarray);
						for(j=0; j<len; j++){
							WanLanObj = json_object_array_get_idx(WanLanListJarray, j);
							IpIfacePath = json_object_get_string(json_object_object_get(WanLanObj, "IpIfacePath"));
							Name = json_object_get_string(json_object_object_get(WanLanObj, "Name"));
							Type = json_object_get_string(json_object_object_get(WanLanObj, "Type"));
							LinkType = json_object_get_string(json_object_object_get(WanLanObj, "LinkType"));
							ConnectionType = json_object_get_string(json_object_object_get(WanLanObj, "ConnectionType"));
							if(paramList[j].type == dalType_LanRtWanWWan){
								if(!strcmp(Type, "WAN") && !strcmp(ConnectionType, "IP_Bridged"))
									continue;
							}
							else if(paramList[j].type == dalType_LanRtWan){
								if(!strcmp(Type, "WAN") && !strcmp(ConnectionType, "IP_Bridged"))
									continue;
								if(!strcmp(Type, "WAN") && !strcmp(LinkType, "USB"))
									continue;
							}
							else if(paramList[j].type == dalType_RtWanWWan){								
								if(!strcmp(Type, "LAN"))
									continue;
								if(!strcmp(Type, "WAN") && !strcmp(ConnectionType, "IP_Bridged"))
									continue;
							}
							else if(paramList[j].type == dalType_RtWan){
								if(!strcmp(Type, "LAN"))
									continue;
								if(!strcmp(Type, "WAN") && !strcmp(ConnectionType, "IP_Bridged"))
									continue;
								if(!strcmp(Type, "WAN") && !strcmp(LinkType, "USB"))
									continue;
							}
							else if(paramList[j].type == dalType_BrWan){
								if(!strcmp(Type, "LAN"))
									continue;
								if(!strcmp(Type, "WAN") && !strcmp(ConnectionType, "IP_Routed"))
									continue;
							}
							else if(paramList[j].type == dalType_Lan || paramList[j].type == dalType_LanName){
								if(!strcmp(Type, "WAN"))
									continue;
							}
							if(!strcmp(Name, stringPtr)){
								foundIntf = true;
								if(paramList[j].type == dalType_LanName){
									json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								}
								else{
									if(strstr(handler_name,"nat")!= NULL){
										if(!strcmp(stringPtr, "Default")){
											json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(""));
										}
										else{
											json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(IpIfacePath));
										}
									}
									else{
										json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(IpIfacePath));
									}
								}
							}
						}
						json_object_put(WanLanListJarray);
						WanLanListJarray = NULL;
						if(!foundIntf){
							printf("Can't find inerface: parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_RouteIntf:	// froe here do not check
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						intfName = strdup(stringPtr);
						if(RouteIntfJarray == NULL)
							getRouteIntfList(&RouteIntfJarray);
						foundIntf = false;
						len = json_object_array_length(RouteIntfJarray);
						for(j=0; j<len; j++){
							intfObj= json_object_array_get_idx(RouteIntfJarray, j);
							Name = json_object_get_string(json_object_object_get(intfObj, "Name"));
							Num = json_object_get_int(json_object_object_get( intfObj, "Number"));
							if(!strcmp(Name, intfName)){
								foundIntf = true;
								sprintf(route,"%d",Num);
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(route));
								break;
							}
						}
						if(!foundIntf){
							if(!strcmp(intfName, "Unchange")){
								Num = 0;
								sprintf(route,"%d",Num);
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(route));
							}
							else{
								printf("Can't find inerface: parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ZOS_FREE(intfName);
								return false;
							}
						}
						ZOS_FREE(intfName);
						json_object_put(RouteIntfJarray);
						RouteIntfJarray = NULL;
						break;
					case dalType_IntfGrpWanList:
						tr181PathList[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(!strncmp("IP.Interface.", stringPtr, 13)){
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
						}
						nameList = strdup(stringPtr);
						if(findDuplicate(nameList, duplicateName, ",")){
							printf("Duplicate WAN interface name %s\n", duplicateName);
							ZOS_FREE(nameList);
							return false;
						}
						Group_WAN_IpIface = NULL;
						getWanLanList(&WanLanListJarray);
						obj = json_object_new_array();
						zcfgFeDalIntrGrpGet(NULL, obj, NULL);
						if(!strcmp(method,"edit")){
							len = json_object_array_length(WanLanListJarray);
							CLIidx = json_object_get_int(json_object_object_get(Jobj, "Index"));
							for(k = 0; k<json_object_array_length(obj); k++){
								obj_tmp = json_object_array_get_idx(obj, k);
								if(CLIidx == json_object_get_int(json_object_object_get(obj_tmp, "Index")))
									Interface = json_object_get_string(json_object_object_get(obj_tmp, "wanIntf"));
							}
							for(j=0;j<len;j++){
								WanLanObj = json_object_array_get_idx(WanLanListJarray, j);
								if(!strcmp(nameList,json_object_get_string(json_object_object_get(WanLanObj, "Name")))){
									if(json_object_get_boolean(json_object_object_get(WanLanObj, "BindToIntfGrp"))){
										if(findInList((char *)Interface, (char *)json_object_get_string(json_object_object_get(WanLanObj, "IpIfacePath")),",")){
											wanIntf = true;
											continue;
										}
										printf("wanIntf error.\n");
										ZOS_FREE(nameList);
										json_object_put(obj);
										obj = NULL;
										return false;
									}
								}			
							}
					 	}
						json_object_put(obj);
						obj = NULL;
						
						intfName = strtok_r(nameList, ",", &tmp_ptr);
						while(intfName != NULL){
							if(*intfName != '\0'){
								foundIntf = false;
								len = json_object_array_length(WanLanListJarray);
								for(j=0; j<len; j++){
									WanLanObj = json_object_array_get_idx(WanLanListJarray, j);
									IpIfacePath = json_object_get_string(json_object_object_get(WanLanObj, "IpIfacePath"));
									Name = json_object_get_string(json_object_object_get(WanLanObj, "Name"));
									Type = json_object_get_string(json_object_object_get(WanLanObj, "Type"));
									LinkType = json_object_get_string(json_object_object_get(WanLanObj, "LinkType"));
									BindToIntfGrp = json_object_get_boolean(json_object_object_get( WanLanObj, "BindToIntfGrp"));

									if(!findInList((char *)Group_WAN_IpIface, (char *)IpIfacePath, ",")){
										if(!strcmp(Type, "LAN"))
											continue;
										if(!strcmp(method,"edit")){
											if(!strcmp(Type, "WAN") && BindToIntfGrp){
												if(!findInList((char *)Interface,(char *)IpIfacePath,","))
													continue;
											}
										}
										else if(!strcmp(method,"add")){
										if(!strcmp(Type, "WAN") && BindToIntfGrp)
											continue;
										}
									}
									if(!strcmp(Name, intfName)){
										if(!strcmp(LinkType, "ATM")){
											if(!AtmUsed) AtmUsed = 1;
											else ReUsed = true;
										}
										else if(!strcmp(LinkType, "PTM")){
											if(!PtmUsed) PtmUsed = 1;
											else ReUsed = true;
										}
										else if(!strcmp(LinkType, "ETH")){
											if(!EthUsed) EthUsed = 1;
											else ReUsed = true;
										}
										else if(!strcmp(LinkType, "PON")){
											if(!PonUsed) PonUsed = 1;
											else ReUsed = true;
										}
										else if(!strcmp(LinkType, "USB")){
											if(!UsbUsed) UsbUsed = 1;
											else ReUsed = true;
										}
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
										else if(!strcmp(LinkType, "CELL")){
											if(!CellUsed) CellUsed = 1;
											else ReUsed = true;
										}
#endif
										if(ReUsed){
											printf("Only allow one WAN interface for each link type.");
											ret = false;
										}

										foundIntf = true;
										strcat(tr181PathList, IpIfacePath);
										strcat(tr181PathList, ",");
									}
								}
								if(wanIntf)
									foundIntf = true;
								if(!foundIntf){
									printf("Can't find inerface: %s\n", intfName);
									ret = false;
								}
							}
							intfName = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(nameList);
						json_object_put(WanLanListJarray);
						WanLanListJarray = NULL;
						len = strlen(tr181PathList);
						if(len>0){
							if(tr181PathList[len-1] == ',')
								tr181PathList[len-1] = '\0';
						}
						if(!ret)
							return ret;
					
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(tr181PathList));
						break;
					case dalType_IntfGrpLanPortList:
						tr181PathList[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(!strncmp("Bridging.Bridge.", stringPtr, 16)){
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
							break;
						}
						nameList = strdup(stringPtr);
						if(findDuplicate(nameList, duplicateName, ",")){
							printf("Duplicate LAN port name %s\n", duplicateName);
							ZOS_FREE(nameList);
							return false;
						}
						getLanPortList(&LanPortListJarray);
						if(!strcmp(method, "edit")){
							brIndex = json_object_get_int(json_object_object_get(Jobj, "Index"));
							sprintf(brPath, "Bridging.Bridge.%u.", brIndex);
						}
						intfName = strtok_r(nameList, ",", &tmp_ptr);
						while(intfName != NULL){
							if(*intfName != '\0'){
								foundIntf = false;
								len = json_object_array_length(LanPortListJarray);
								for(k=0;k<len;k++){
									LanPortObj = json_object_array_get_idx(LanPortListJarray, k);
									IntfPath = json_object_get_string(json_object_object_get(LanPortObj, "IntfPath"));
									BrPortPath = json_object_get_string(json_object_object_get(LanPortObj, "BrPortPath"));
									Name = json_object_get_string(json_object_object_get(LanPortObj, "Name"));
					
									if(!strcmp(method, "edit") && !strncmp(brPath, BrPortPath, strlen(brPath)) && !strcmp(Name, intfName)){
										foundIntf = true;
										strcat(tr181PathList, BrPortPath);
										strcat(tr181PathList, ",");
									}
									else if(!strcmp(Name, intfName) && !strncmp(BrPortPath, "Bridging.Bridge.1.", 18)){
										foundIntf = true;
										strcat(tr181PathList, BrPortPath);
										strcat(tr181PathList, ",");
									}			
								}
					
								if(!foundIntf){
									printf("Can't find inerface: %s\n", intfName);
									ret = false;
								}

							}
							intfName = strtok_r(NULL, ",", &tmp_ptr);
						}
						ZOS_FREE(nameList);
						json_object_put(LanPortListJarray);
						LanPortListJarray = NULL;
						len = strlen(tr181PathList);
						if(len>0){
							if(tr181PathList[len-1] == ',')
								tr181PathList[len-1] = '\0';
						}
						if(!ret){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return ret;
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(tr181PathList));
						break;
					case dalType_LanPortListBr0NoWiFi:
						stringPtr = json_object_get_string(pramJobj);					
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
					case dalType_IntfGrpLanPortListNoWiFi:
						intfList[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						ptr_buf = strdup(stringPtr);
						nameList = strtok_r(ptr_buf, ",", &tmp_ptr);
						while(nameList != NULL){
							tmp = strtok_r(nameList, ".", &LANName);
							strcat(intfList, nameList);
							strcat(intfList, ",");
							nameList = strtok_r(tmp_ptr, ",", &tmp_ptr);
						}
						if(strlen(intfList) > 0){
							if(intfList[strlen(intfList)-1] == ',')
								intfList[strlen(intfList)-1] = '\0';
						}
						ZOS_FREE(ptr_buf);
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(intfList));
						break;
					case dalType_ClassIntf:
						tr181PathList[0] = '\0';
						ret = true;
						stringPtr = json_object_get_string(pramJobj);
						if(!strncmp("Bridging.Bridge.", stringPtr, 16)){
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
							break;
						}
						intfName = strdup(stringPtr);
						getLanPortList(&LanPortListJarray);
						if(intfName != NULL){
							foundIntf = false;
							len = json_object_array_length(LanPortListJarray);
							for(k=0;k<len;k++){
								LanPortObj = json_object_array_get_idx(LanPortListJarray, k);
								IntfPath = json_object_get_string(json_object_object_get(LanPortObj, "IntfPath"));
								BrPortPath = json_object_get_string(json_object_object_get(LanPortObj, "BrPortPath"));
								Name = json_object_get_string(json_object_object_get(LanPortObj, "Name"));
								if(!strcmp(Name, intfName)){
									foundIntf = true;
									strcat(tr181PathList, BrPortPath);	
									break;
								}
							}
							if(!foundIntf){
								if(!strcmp("LAN",intfName))
									strcat(tr181PathList, intfName);
								else if(!strcmp("Local",intfName) || !strcmp("lo",intfName))
									strcat(tr181PathList, "lo");
								else{
									printf("Can't find inerface: %s\n", intfName);
									ret = false;
								}
							}
						}

						ZOS_FREE(intfName);
						json_object_put(LanPortListJarray);
						LanPortListJarray = NULL;
						if(!ret){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return ret;
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(tr181PathList));
						break;
					case dalType_ClassList:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
							}
						}
						getClassList(&obj);
						newObj = json_object_new_array();
						len = json_object_array_length(obj);
						for(k=0;k<len;k++){
							obj_tmp = json_object_array_get_idx(obj, k);
							Policer= json_object_get_string(json_object_object_get(obj_tmp, "Policer"));
							if(!strcmp(Policer, "")){
								json_object_array_add(newObj, JSON_OBJ_COPY(obj_tmp));
							}
						}
						if(checkValidateParameter((char *)stringPtr, &newObj, "Name") == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								ret = false;
							}
						json_object_put(obj);
						json_object_put(newObj);
						newObj = NULL;
						obj = NULL;
						break;
					case dalType_QueueList:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						intfName = strdup(stringPtr);
						getQueueList(&QueueListJarray);
						if(intfName != NULL){
							foundIntf = false;
							len = json_object_array_length(QueueListJarray);
							for(k=0;k<len;k++){
								obj = json_object_array_get_idx(QueueListJarray, k);
								Name = json_object_get_string(json_object_object_get(obj, "Name"));
								if(!strcmp(Name, intfName)){
									foundIntf = true;
									Num = json_object_get_int(json_object_object_get(obj, "QueueIdx"));
									sprintf(queue,"%d",Num);
									break;
								}
							}
							if(!foundIntf){
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
								printf("Can't find inerface: %s\n", intfName);
								ret = false;
#else
								if(!strcmp("default_queue",intfName)){
									def = 1;
									Num = 0;
									sprintf(queue,"%d",Num);
								}
								else{
									printf("Can't find inerface: %s\n", intfName);
									ret = false;
								}
#endif
							}
						}
						
						ZOS_FREE(intfName);
						json_object_put(QueueListJarray);
						QueueListJarray = NULL;
						if(!ret && def == 0)
							return ret;
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(queue));
						break;
					case dalType_VoipRegion:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						for(j = 0; dalVoipRegion[j].Region != NULL; j++){
							if(!strcmp(dalVoipRegion[j].Region,stringPtr)){
								Name = dalVoipRegion[j].simplifyRegion;
								found = true;
								break;
							}
						}
						if(found)
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(Name));
						else{
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						break;
					case dalType_VoipLine:
					case dalType_MultiVoipLine:
						voipList[0] = '\0';
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						ptr_buf = strdup(stringPtr);
						getVoipMultiObj(&VoipList);
						len = json_object_array_length(VoipList);
						nameList = strtok_r(ptr_buf, ",", &tmp_ptr);
						if(paramList[i].type == dalType_VoipLine){
							if(strcmp(tmp_ptr,"")){
								printf("Too many input parameter of %s\n", paramList[i].paraName);
								ZOS_FREE(ptr_buf);
								return false;
							}
						}
						while(nameList != NULL){
							for(j=0;j<len;j++){
								obj = json_object_array_get_idx(VoipList, j);
								if(!strcmp(nameList,json_object_get_string(json_object_object_get(obj, "SIPNumber")))){
									if(paramList[i].type == dalType_VoipLine){
										VoipName = json_object_get_string(json_object_object_get(obj, "VoipLine"));
									}
									else if(paramList[i].type == dalType_MultiVoipLine){
										VoipName = json_object_get_string(json_object_object_get(obj, "BoundList"));
									}
									strcat(voipList, VoipName);
									strcat(voipList, ",");
									break;
								}	
							}
							nameList = strtok_r(tmp_ptr, ",", &tmp_ptr);
						}
						json_object_put(VoipList);
						VoipList = NULL;

						if(voipList[strlen(voipList) - 1] == ',')
							voipList[strlen(voipList) - 1] = '\0';
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(voipList));
						break;
					case dalType_VoipProvider:
						stringPtr = json_object_get_string(pramJobj);
						getVoipProviderObj(&VoipList);
						if(checkValidateParameter((char *)stringPtr, &VoipList, "VoipServiceProvider") == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							json_object_put(VoipList);
							VoipList = NULL;
							return NULL;
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						json_object_put(VoipList);
						VoipList = NULL;
						break;
					case dalType_ICMPv6:
						stringPtr = json_object_get_string(pramJobj);
						for(k = 0; dalFirewallICMP6[k].type != NULL; k++){
							if(!strcmp(dalFirewallICMP6[k].type,stringPtr)){
								icmptye= atoi(dalFirewallICMP6[k].icmptype);
								icmptypecode = atoi(dalFirewallICMP6[k].icmptypecode);
							}
						}
						json_object_object_add(Jobj, "ICMPTypeCode", json_object_new_int(icmptypecode));
						json_object_object_add(Jobj, "ICMPType", json_object_new_int(icmptye));
						break;
					case dalType_LocalCert:
						stringPtr = json_object_get_string(pramJobj);
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						foundIntf = false;
						if(!strcmp(stringPtr, "")){
							foundIntf = true;
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						}
						else{
							getCertList(&certListJarray);
							len = json_object_array_length(certListJarray);
							for(j=0; j<len; j++){
								obj = json_object_array_get_idx(certListJarray, k);
								Name = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Name"));
								Type = json_object_get_string(json_object_object_get(obj, "X_ZYXEL_Type"));
								if(!strcmp(Name, stringPtr) && !strcmp(Type, "signed")){
									foundIntf = true;
									json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
									break;
								}
							}
							json_object_put(certListJarray);
							certListJarray = NULL;
						}
						if(!foundIntf){
							printf("Can't find local certificate: %s\n", stringPtr);
							return false;
						}
						break;
					case dalType_timezone:
						stringPtr = json_object_get_string(pramJobj);
						for(k = 0; dalTimeZone[k].timezoneCLI != NULL; k++){
							if(!strcmp(dalTimeZone[k].timezoneCLI,stringPtr)){
								timezone= dalTimeZone[k].timezone;
								timezonevalue = dalTimeZone[k].timezonevalue;
								found = true;
								break;
							}
						}
						if(found){
						json_object_object_add(Jobj, "X_ZYXEL_TimeZone", json_object_new_string(timezonevalue));
						json_object_object_add(Jobj, "X_ZYXEL_Location", json_object_new_string(timezone));
						}
						else{
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						
						break;
					case dalType_HourMin:
						stringPtr = json_object_get_string(pramJobj);
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(validatetime(stringPtr) == false){
							printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
					case dalType_serveraddr:
						stringPtr = json_object_get_string(pramJobj);
						if(paramList[i].enumeration != NULL){
							if(findInList(paramList[i].enumeration, (char *)stringPtr, "|") == true){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						if(strcmp(stringPtr, "None")){
							if(validateServerName(stringPtr) == false){
								printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
								return false;
							}
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
					case dalType_sambafolder:
						stringPtr = json_object_get_string(pramJobj);
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
					case dalType_usrAccount:
						stringPtr = json_object_get_string(pramJobj);
						ptr_buf = strdup(stringPtr);
						account = strtok_r(ptr_buf, ",", &tmp);
						while(account != NULL){
							strcat(usnAccount, account);
							strcat(usnAccount, " ");
							account = strtok_r(NULL, ",", &tmp);
						}
						ZOS_FREE(ptr_buf);
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(usnAccount));
						break;
					case dalType_usbInfo:
						stringPtr = json_object_get_string(pramJobj);
						if(strstr(stringPtr, "..")){
							printf("Wrong usb path value <%s> of %s\n", stringPtr, paramList[i].paraName);
							return false;
						}
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
					case dalType_intfVlan:
						stringPtr = json_object_get_string(pramJobj);
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;	
					case dalType_wlanMode:
						stringPtr = json_object_get_string(pramJobj);
						wlmode[0] = '\0';
						if(json_object_object_get(Jobj, "cmdtype")){
							if(!strcmp(json_object_get_string(json_object_object_get(Jobj, "cmdtype")),"GUI")){
								json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
								break;
							}
						}
						ptr_buf = strdup(stringPtr);
						ZOS_STRCPY_M(buff, ptr_buf);
						if(strstr(&buff[6],"/")){
							strcat(wlmode, "11");
							mode = strtok_r(&buff[6], "/",&tmp);
							while(mode != NULL){
								strcat(wlmode, mode);
								mode = strtok_r(NULL, "/", &tmp);
							}
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(wlmode));
						}
						else
							json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(&buff[4]));
						ZOS_FREE(ptr_buf);
						break;	
					case dalType_wifiSSID:
						stringPtr = json_object_get_string(pramJobj);
						json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
						break;
                    case dalType_ShaperIntf:
                        stringPtr = json_object_get_string(pramJobj);
                        if ((obj_tmp = json_object_object_get(Jobj, "cmdtype")) != NULL)
                        {
                            if (!strcmp(json_object_get_string(obj_tmp), "GUI"))
                            {
                                json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(stringPtr));
                                break;
                            }
                        }

                        if (strstr(stringPtr, "LAN"))
                        {
                            getLanPortList(&LanPortListJarray);

                            len = json_object_array_length(LanPortListJarray);
                            for (j = 0; j < len; j++)
                            {
                                LanPortObj = json_object_array_get_idx(LanPortListJarray, j);
                                if ((obj_tmp = json_object_object_get(LanPortObj, "Name")) != NULL)
                                {
                                    Name = json_object_get_string(obj_tmp);
                                    if (!strcmp(stringPtr, Name))
                                    {
                                        if ((obj_tmp = json_object_object_get(LanPortObj, "IntfPath")) != NULL)
                                        {
                                            IntfPath = json_object_get_string(obj_tmp);
                                            json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(IntfPath));
                                            found = true;
                                        }
                                    }
                                }
                            }
							json_object_put(LanPortListJarray);
							LanPortListJarray = NULL;
                        }
                        else
                        {
                            getWanLanList(&WanLanListJarray);

                            len = json_object_array_length(WanLanListJarray);
                            for (j = 0; j < len; j++)
                            {
                                WanLanObj = json_object_array_get_idx(WanLanListJarray, j);
                                if ((obj_tmp = json_object_object_get(WanLanObj, "Name")) != NULL)
                                {
                                    Name = json_object_get_string(obj_tmp);
                                    if (!strcmp(stringPtr, Name))
                                    {
                                        if ((obj_tmp = json_object_object_get(WanLanObj, "IpIfacePath")) != NULL)
                                        {
                                            IpIfacePath = json_object_get_string(obj_tmp);
                                            json_object_object_add(Jobj, paramList[i].paraName, json_object_new_string(IpIfacePath));
                                            found = true;
                                        }
                                    }
                                }
                            }
							json_object_put(WanLanListJarray);
							WanLanListJarray = NULL;
                        }

                        if (!found)
                        {
                            printf("Wrong parameter value <%s> of %s\n", stringPtr, paramList[i].paraName);
                            return false;
                        }
                        break;
					case dalType_mediaserverPath:
						stringPtr = json_object_get_string(pramJobj);
						if(stringPtr == NULL || !strcmp(stringPtr, ""))
							return false;
						if(strstr(stringPtr, "/mnt/") != NULL && strcmp(strstr(stringPtr, "/mnt/"),stringPtr) == 0 && strstr(stringPtr, "../") == NULL) 
						{							
						}
		                else 
							return false; 				
						break;						
					default:
						break;
				}
			}
		}
	}

	return true;
}

/*
 * get value in _retJobj of Jobj
 * input: Jobj, key
 * output: value
*/
int zcfgFeDalGetReturnValue(struct json_object *Jobj, const char *key, void* value){
	json_type type;
	
	struct json_object *retParamJobj = json_object_object_get(json_object_object_get(Jobj, JSON_RET_OBJ_NAME), key);

	if(retParamJobj == NULL)
		return -1;

	type = json_object_get_type(retParamJobj);

	switch(type) {
		case json_type_string:
			ZOS_STRCPY_M((char*)value, (char*)json_object_get_string(retParamJobj));
			break;
		case json_type_boolean:
			*(json_bool*)value = json_object_get_boolean(retParamJobj);
			break;
		case json_type_int:
			*(int*)value = json_object_get_int(retParamJobj);
			break;
		default:
			return -1;
	}

	return 0;
}

/*
 * add {key:value} into _retJobj  (_retJobj is a JSON object in Jobj)
 * input: Jobj, type(only support string, int, boolean), key, value
 *
*/

int zcfgFeDalSetReturnValue(struct json_object *Jobj, json_type type, const char *key, void* value){

	struct json_object *retParamJobj = json_object_object_get(Jobj, JSON_RET_OBJ_NAME);

	if(retParamJobj == NULL){
		retParamJobj = json_object_new_object();
		json_object_object_add(Jobj, JSON_RET_OBJ_NAME, retParamJobj);
	}

	switch(type) {
		case json_type_string:
			json_object_object_add(retParamJobj, key, json_object_new_string((const char *)value));
			break;
		case json_type_boolean:
			json_object_object_add(retParamJobj, key, json_object_new_boolean(*(json_bool*)value));
			break;
		case json_type_int:
			json_object_object_add(retParamJobj, key, json_object_new_int(*(int*)value));
			break;
		default:
			return -1;
	}

	return 0;
}

/*
 * input:handlerName,  method, Jobj
 * output: Jarray, replyMsg
*/
zcfgRet_t zcfgFeDalHandler(const char *handlerName, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int i = 0, found = 0;

	if(handlerName == NULL)
		return ret;
	
	for(i=0; dalHandler[i].name != NULL; i++){
		if(!strcmp(handlerName, dalHandler[i].name)){
			found =1;
			break;
		}
	}

	//printf("[zcfgFeDal]line(%d) : handlerName = %s, method = %s, i = %d\n", __LINE__, handlerName, method, i);

	if(!found){
		printf("%s:%d : Unknown handlerName: %s\n", __FUNCTION__, __LINE__, handlerName);
		return ret;
	}
	if(parseValidateParameter(Jobj, (char *)method, dalHandler[i].name, dalHandler[i].parameter,replyMsg) == false){
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if(dalHandler[i].handler != NULL)
		ret = dalHandler[i].handler(method, Jobj, Jarray, replyMsg);
	return ret;
}


uint32_t CharToIntConv(char chr) {
	uint32_t res = 0;

	if(('0' <= chr) && (chr <= '9')) {
		res = (chr)-0x30;
	}
	else {
		printf("CharToIntConv Error: inupt is not from 0~9, input = %c\n", chr);
	}

	return res;
}

uint32_t StrToIntConv(char *str) {
	uint32_t res = 0;
	short len = strlen(str);
	short i = 0;
	uint32_t unit = 1;

	//printf("StrToIntConv: %s, len = %d\n", str, len);
	for(i=len-1; i>=0; i--) {
		res += ((CharToIntConv(str[i]))*unit);
		unit*=10;
	}

	return res;
}

/*
Function Name: ip4Compare
Description  : Compare 2 IP strings
Returns      : 0 (ip1 = ip2), 1 (ip1 < ip2), 2 (ip1 > ip2), -1 error*/
int ip4Compare(char *ip1, char *ip2)
{
	char ipaddr1[20] = {0}, ipaddr2[20] = {0};
	int ip11=0,ip12=0,ip13=0,ip14=0;
	int ip21=0,ip22=0,ip23=0,ip24=0;
	
	if (ip1==NULL || ip2==NULL)
		return -1;

	ZOS_STRCPY_M(ipaddr1, ip1);
	ZOS_STRCPY_M(ipaddr2, ip2);
	sscanf(ipaddr1, "%d.%d.%d.%d", &ip11, &ip12, &ip13, &ip14);
	sscanf(ipaddr2, "%d.%d.%d.%d", &ip21, &ip22, &ip23, &ip24);

	if (ip11<ip21)
		return 1;
	else if (ip11>ip21)
		return 2;

	if (ip12<ip22)
		return 1;
	else if (ip12>ip22)
		return 2;

	if (ip13<ip23)
		return 1;
	else if (ip13>ip23)
		return 2;

	if (ip14<ip24)
		return 1;
	else if (ip14>ip24)
		return 2;
	return 0;
}

#ifdef CONFIG_ABZQ_CUSTOMIZATION
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_FILE_PATH "/var/log/"
#define FLASH_LOG_FILE_PATH "/data/"
#define FLASH_LOG_FILE_SYSLOG "syslog.log"
#define FLASH_LOG_FILE_SECURITY "security.log"
#define LOG_ROTATE 2

int device_open(const char *device, int mode)
{
	int m, f, fd;

	m = mode | O_NONBLOCK;

	/* Retry up to 5 times */
	/* TODO: explain why it can't be considered insane */
	for (f = 0; f < 5; f++) {
		fd = open(device, m, 0600);
		if (fd >= 0)
			break;
	}
	if (fd < 0)
		return fd;
	/* Reset original flags. */
	if (m != mode)
		fcntl(fd, F_SETFL, mode);
	return fd;
}

void rotateLog(char *flashLogFilePath, int logFileRotate)
{
	int i;
	char oldFile[(strlen(flashLogFilePath)+3)], newFile[(strlen(flashLogFilePath)+3)];
	FILE *fp;

	for(i=logFileRotate-1;i>0;i--) {
		snprintf(oldFile, (strlen(flashLogFilePath)+2), "%s%d", flashLogFilePath, i-1);
		snprintf(newFile, (strlen(flashLogFilePath)+2), "%s%d", flashLogFilePath, i);
		rename(oldFile, newFile);
	}
}

void copyLogToFlash(char *logFilePath, char *flashLogFilePath, unsigned int logFileSize, int logFileRotate)
{
	char cmd[128];
	char *tempFile = "/var/log/temp.log";
	int fd;
	FILE *fp;
	char buffer[1024];
	int index=0;
	int logRemainSize=0;

	//If there is no any log stored in flash, copy /log/system.log to /data/log/system0.log
	FILE *dummyFP, *rotateFP;

	char log_0_file[128];
	char log_1_file[128];

	snprintf(log_1_file, sizeof(log_1_file), "%s.%d", logFilePath, 1);

	if((rotateFP = fopen(log_1_file,"r")) != NULL)
	{
		fclose(rotateFP);
		printf("move %s to flash\n", log_1_file);
		snprintf(cmd, sizeof(cmd),"cp %s %s%d", log_1_file, flashLogFilePath, 1);
		system(cmd);
		printf("move %s to flash\n", logFilePath);
		snprintf(cmd, sizeof(cmd),"cp %s %s%d", logFilePath, flashLogFilePath, 0);
		system(cmd);

		return;
	}

	snprintf(log_0_file, sizeof(log_0_file), "%s%d", flashLogFilePath, 0);

	struct stat st;	
	struct stat stflash;	

	stat(logFilePath, &st); 
	stat(flashLogFilePath, &stflash); 

	if((dummyFP = fopen(logFilePath,"r")) == NULL)
	{
		printf("Error: dummyFP == NULL \n");			
		return;
	}
	else if((dummyFP = fopen(log_0_file,"r")) == NULL)
	{
		printf("move %s to flash\n", logFilePath);
		snprintf(cmd, sizeof(cmd),"cp %s %s%d", logFilePath, flashLogFilePath, 0);
		system(cmd);
		// Clean /var/log/syslog.log. Do not delete it.
		snprintf(cmd, sizeof(cmd), "> %s", logFilePath);
		system(cmd);
	}
	else
	{
		fclose(dummyFP);
		printf("save %s to flash\n", logFilePath);

		//Copy /data/log/system0.log to /log/temp.log
		snprintf(cmd, sizeof(cmd),"cp %s%d %s", flashLogFilePath, 0, tempFile);
		system(cmd);

		//Calculate the remained size of /log/temp.log (/data/log/system0.log)
		fd = device_open(tempFile, O_RDONLY);
		logRemainSize = logFileSize - 1024 - lseek(fd,0,SEEK_END);
		close(fd);

		//Write logs in /log/system.log to /log/temp.log
		if((fp = fopen(logFilePath,"r")) != NULL)
		{
			fseek(fp,0L,SEEK_END);
			int logFilesize = ftell(fp)+1;
			int moveAll = FALSE;            

			if(logFilesize >= 150000 )
			{ /* 150k */
				moveAll = TRUE;
				rotateLog(flashLogFilePath, logFileRotate);

				//Copy /log/system.log to /data/log/system0.log
				printf("move %s to flash\n", logFilePath);			    
				snprintf(cmd, sizeof(cmd),"cp %s %s%d", logFilePath, flashLogFilePath, 0);
				system(cmd);

			}
			else if((logFilesize < 150000) && (logFilesize > 20000))
			{ /* 150k -30k */
				if(logRemainSize < logFilesize)
				{
					moveAll = TRUE;
					rotateLog(flashLogFilePath, logFileRotate);

					//Copy /log/system.log to /data/log/system0.log
					printf("move %s to flash\n", logFilePath);				        
					snprintf(cmd, sizeof(cmd),"cp %s %s%d", logFilePath, flashLogFilePath, 0);
					system(cmd);
				}
				else
				{
				moveAll = TRUE;
				printf("save %s to flash\n", logFilePath);	                    
				snprintf(cmd, sizeof(cmd),"cat %s %s >> %s%d", tempFile, logFilePath, flashLogFilePath, 0);
				system(cmd);
				}             
			}

			if(moveAll)
			{
				fclose(fp);

				//Remove /log/temp.log
				unlink(tempFile);
				// Clean /var/log/syslog.log. Do not delete it.
				snprintf(cmd, sizeof(cmd), "> %s", logFilePath);
				system(cmd);
				return;
			}

			fseek(fp, 0, SEEK_SET);
			index = 0;
			while (index < logRemainSize)
			{
				if(!fgets(buffer, 1024, fp))
					break;
				buffer[strlen(buffer)-1] = '\0';
				snprintf(cmd, sizeof(cmd), "echo \"%s\" >> %s", buffer, tempFile);
				system(cmd);
				index += strlen(buffer);
			}
		}
		else
		{
			printf("syslogd can not open %s\n", logFilePath);
			return;
		}

		//Copy back /log/temp.log to /data/log/system0.log
		snprintf(cmd, sizeof(cmd),"cp %s %s%d", tempFile, flashLogFilePath, 0);
		system(cmd);

		if(index >= logRemainSize)
		{
			rotateLog(flashLogFilePath, logFileRotate);

			//Write remains logs in /log/system.log to /log/temp.log
			//Clear /log/temp.log
			fd = device_open(tempFile, O_WRONLY);
			ftruncate( fd, 0 );
			close(fd);
			//Write logs into /log/temp.log
			while(fgets(buffer, 1024, fp))
			{
				buffer[strlen(buffer)-1] = '\0';
				snprintf(cmd, sizeof(cmd), "echo \"%s\" >> %s", buffer, tempFile);
				system(cmd);
			}
			fclose(fp);

			//Copy /log/temp.log to /data/log/system0.log
			snprintf(cmd, sizeof(cmd),"cp %s %s%d", tempFile, flashLogFilePath, 0);
			system(cmd);
		}
		//Remove /log/temp.log
		unlink(tempFile);

		// Clean /var/log/syslog.log. Do not delete it.
		snprintf(cmd, sizeof(cmd), "> %s", logFilePath);
		system(cmd);
	}
}

void copySystemLogToFlash (bool uSaveSecurity)
{
	char tmpfilename[64] = {0};
	char tmpflashfilename[64] = {0};

	snprintf(tmpfilename, sizeof(tmpfilename),"%s%s",LOG_FILE_PATH, FLASH_LOG_FILE_SYSLOG);
	snprintf(tmpflashfilename, sizeof(tmpflashfilename),"%s%s",FLASH_LOG_FILE_PATH, FLASH_LOG_FILE_SYSLOG);

	copyLogToFlash(tmpfilename, tmpflashfilename, 200*1024, LOG_ROTATE);

	if ( uSaveSecurity == true )
	{
		snprintf(tmpfilename, sizeof(tmpfilename),"%s%s",LOG_FILE_PATH, FLASH_LOG_FILE_SECURITY);
		snprintf(tmpflashfilename, sizeof(tmpflashfilename),"%s%s",FLASH_LOG_FILE_PATH, FLASH_LOG_FILE_SECURITY);

		copyLogToFlash(tmpfilename, tmpflashfilename, 200*1024, LOG_ROTATE);
	}

	return;
}
#endif

zcfgRet_t dalReqSendWithoutResponse(char *payload)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int payloadLen = 0;
	void *sendBuf = NULL;
	zcfgMsg_t *sendMsgHdr = NULL;

	payloadLen = sizeof(zcfgMsg_t) + strlen(payload) + 1;
	sendBuf = (void *)ZOS_MALLOC(sizeof(zcfgMsg_t) + payloadLen);
	sendMsgHdr = (zcfgMsg_t *)sendBuf;
	sendMsgHdr->type = ZCFG_MSG_FAKEROOT_COMMAND;
	sendMsgHdr->length = payloadLen;
	sendMsgHdr->dstEid = ZCFG_EID_ESMD;

	if(payloadLen > 0)
		ZOS_STRCPY_M((char *)(sendMsgHdr + 1), payload);

	ret = zcfgMsgSendAndGetReply(sendMsgHdr, NULL, 0);
	if(ret == ZCFG_SUCCESS) {
		//didn`t expect return response
		return ZCFG_INTERNAL_ERROR;
	}
	else if(ret == ZCFG_SUCCESS_AND_NO_REPLY) {
		ret = ZCFG_SUCCESS;
	}
	else {
		return ZCFG_INTERNAL_ERROR;
	}

	return ret;
}

zcfgRet_t zcfgFeLanCheckWanDhcpServer(struct json_object *Jobj){
	zcfgRet_t ret;
	json_object *lanIpIfaceObj = NULL;
	json_object *wanIpIfaceObj = NULL;
	json_object *dhcp4SrvPoolObj = NULL;
	objIndex_t dhcp4SrvPoolIid = {0};
	objIndex_t lanIpIfaceIid = {0};
	objIndex_t wanIpIfaceIid = {0};
	char ifaceList[1024] = {0};
	char *itfsaveptr = NULL;
	char *bindIf = NULL;
	const char *langrpwanIpiface = NULL;
	
	while((ret = zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_DHCPV4_SRV_POOL, &dhcp4SrvPoolIid, &dhcp4SrvPoolObj)) == ZCFG_SUCCESS){
		/* find lan ip interface */
		lanIpIfaceIid.level = 1;
		sscanf(json_object_get_string(json_object_object_get(dhcp4SrvPoolObj, "Interface")), "IP.Interface.%hhu", &lanIpIfaceIid.idx[0]);
		if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &lanIpIfaceIid, &lanIpIfaceObj)) != ZCFG_SUCCESS){
			printf("%s: fail to get lan RDM_OID_IP_IFACE with ret=%d\n", __FUNCTION__, ret);
			zcfgFeJsonObjFree(dhcp4SrvPoolObj);
			return ret;
		}
		
		/* find wan ip interface */
		/* default bridge group(br0), br0 is not binded to any wan interface (all wans are available to forward) */
		langrpwanIpiface = json_object_get_string(json_object_object_get(lanIpIfaceObj, "X_ZYXEL_Group_WAN_IpIface"));
		if(langrpwanIpiface != NULL && strcmp(langrpwanIpiface,"")){ 
			wanIpIfaceIid.level = 1;
			snprintf(ifaceList, sizeof(ifaceList), "%s", json_object_get_string(json_object_object_get(lanIpIfaceObj, "X_ZYXEL_Group_WAN_IpIface")));
			bindIf = strtok_r(ifaceList, ",", &itfsaveptr);
			while (bindIf != NULL){
				sscanf(bindIf, "IP.Interface.%hhu", &wanIpIfaceIid.idx[0]);
				if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_IP_IFACE, &wanIpIfaceIid, &wanIpIfaceObj)) == ZCFG_SUCCESS){
					/* if route mode, enable dhcp server */
					if(!strcmp(json_object_get_string(json_object_object_get(wanIpIfaceObj, "X_ZYXEL_ConnectionType")),"IP_Routed")){
						json_object_object_add(dhcp4SrvPoolObj, "Enable", json_object_new_boolean(true));
						zcfgFeJsonObjFree(wanIpIfaceObj);
						break;
					}
					/* if bridge mode, disable dhcp server */
					else if(!strcmp(json_object_get_string(json_object_object_get(wanIpIfaceObj, "X_ZYXEL_ConnectionType")),"IP_Bridged")){
						json_object_object_add(dhcp4SrvPoolObj, "Enable", json_object_new_boolean(false));
						zcfgFeJsonObjFree(wanIpIfaceObj);
					}
				}
				bindIf = strtok_r(NULL, ",", &itfsaveptr);
			}
			if((ret = zcfgFeObjJsonSet(RDM_OID_DHCPV4_SRV_POOL, &dhcp4SrvPoolIid, dhcp4SrvPoolObj, NULL)) != ZCFG_SUCCESS){
				printf("%s: fail to set RDM_OID_DHCPV4_SRV_POOL with ret=%d\n", __FUNCTION__, ret);
			}
		}
		zcfgFeJsonObjFree(lanIpIfaceObj);
		zcfgFeJsonObjFree(dhcp4SrvPoolObj);
	}
	return ret;
}

