#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "zcfg_fe_tr98.h"
#include "wanDev.h"
#include "devInfo.h"
#include "devTime.h"
#include "root.h"
#include "mgmtSrv.h"
#include "lanDev.h"
#include "l2bridge.h"
#include "l3fwd.h"
#include "lanConfSec.h"
#include "zyExt.h"
#include "diag.h"
#include "firewall.h"
#include "schedule.h"
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
#include "qos.h"
#endif
#include "voice.h"
#ifdef ONECONNECT
#include "oneConnect.h"
#endif
#include "ispService.h"
#include "ipv6Spec.h"
#ifdef ZYXEL_GRE_SUPPORT
#include "xgre.h"
#endif

#ifdef ZYXEL_XMPP
#include "xmpp.h"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
#include "zcfg_fe_tr98_ZyEE.h"
#endif

#include "zcfg_fe_rdm_access.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_webgui_flag.h"
#include "zcfg_fe_tr98_handle.h"
#include "zcfg_tr98_common.h"
#include "zos_api.h"

#define ReqOptLen 1536 //define MAXLEN for dhcpv4 option 121 value string parsing
#ifdef ZCFG_PARM_NAMEMAPPING
#define TR98MAPPING_PREFIX "Tr98_"
#define IS_HANDLE_ROUTINE_REQUIRED(attr) (attr & PARAMETER_ATTR_ROUTINE_REQ)
#define ZCFG_PARM_NAMEMAPPING_NOT_APPLIED
#endif

static struct json_object *tr98StaticNameTable = NULL;

#ifdef ZCFG_APPLY_MULTIOBJSET_ON_SESSIONCOMPLETE
static struct json_object *sessMultiObjSet = NULL;
static struct json_object *sessTr98SpvObj = NULL;
#endif
int volatile wanDeviceInstanceNum = 0;
bool StaticWanDevOption = false;
bool isBondingWanProcess = false;

#ifdef CONFIG_TAAAB_DSL_BINDING
int WanConnectType = 3;   //1:eth,2:ATM,3:PTM
#endif


tr98NameObj_t tr98NameObjList[] =
{
{e_TR98_IGD, &tr98Obj[e_TR98_IGD-e_TR98_START]}, 
#ifdef CONFIG_PACKAGE_ZyIMS
{e_TR98_SERVICE                             , &tr98Obj[e_TR98_SERVICE                             -e_TR98_START]},
{e_TR98_VOICE_SRV                           , &tr98Obj[e_TR98_VOICE_SRV                           -e_TR98_START]},
{e_TR98_VOICE_CAPB                          , &tr98Obj[e_TR98_VOICE_CAPB                          -e_TR98_START]},
{e_TR98_VOICE_CAPB_SIP                      , &tr98Obj[e_TR98_VOICE_CAPB_SIP                      -e_TR98_START]},
//{e_TR98_VOICE_CAPB_MGCP                     , &tr98Obj[e_TR98_VOICE_CAPB_MGCP                     -e_TR98_START]},
//{e_TR98_VOICE_CAPB_H323                     , &tr98Obj[e_TR98_VOICE_CAPB_H323                     -e_TR98_START]},
{e_TR98_VOICE_CAPB_CODEC                    , &tr98Obj[e_TR98_VOICE_CAPB_CODEC                    -e_TR98_START]},
{e_TR98_VOICE_PROF                          , &tr98Obj[e_TR98_VOICE_PROF                          -e_TR98_START]},
//{e_TR98_VOICE_PROF_SRV_PRO_INFO             , &tr98Obj[e_TR98_VOICE_PROF_SRV_PRO_INFO             -e_TR98_START]},
{e_TR98_VOICE_PROF_SIP                      , &tr98Obj[e_TR98_VOICE_PROF_SIP                      -e_TR98_START]},
//{e_TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ        , &tr98Obj[e_TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ        -e_TR98_START]},
//{e_TR98_VOICE_PROF_SIP_RESP_MAP_OBJ         , &tr98Obj[e_TR98_VOICE_PROF_SIP_RESP_MAP_OBJ         -e_TR98_START]},
{e_TR98_VOICE_PROF_RTP                      , &tr98Obj[e_TR98_VOICE_PROF_RTP                      -e_TR98_START]},
{e_TR98_VOICE_PROF_RTP_RTCP                 , &tr98Obj[e_TR98_VOICE_PROF_RTP_RTCP                 -e_TR98_START]},
{e_TR98_VOICE_PROF_RTP_SRTP                 , &tr98Obj[e_TR98_VOICE_PROF_RTP_SRTP                 -e_TR98_START]},
//{e_TR98_VOICE_PROF_RTP_REDUNDANCY           , &tr98Obj[e_TR98_VOICE_PROF_RTP_REDUNDANCY           -e_TR98_START]},
{e_TR98_VOICE_PROF_NUM_PLAN                 , &tr98Obj[e_TR98_VOICE_PROF_NUM_PLAN                 -e_TR98_START]},
{e_TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO     , &tr98Obj[e_TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO     -e_TR98_START]},
//{e_TR98_VOICE_PROF_TONE                     , &tr98Obj[e_TR98_VOICE_PROF_TONE                     -e_TR98_START]},
//{e_TR98_VOICE_PROF_TONE_EVENT               , &tr98Obj[e_TR98_VOICE_PROF_TONE_EVENT               -e_TR98_START]},
//{e_TR98_VOICE_PROF_TONE_DESCRIPTION         , &tr98Obj[e_TR98_VOICE_PROF_TONE_DESCRIPTION         -e_TR98_START]},
//{e_TR98_VOICE_PROF_TONE_PATTERN             , &tr98Obj[e_TR98_VOICE_PROF_TONE_PATTERN             -e_TR98_START]},
{e_TR98_VOICE_PROF_FAX_T38                  , &tr98Obj[e_TR98_VOICE_PROF_FAX_T38                  -e_TR98_START]},
{e_TR98_VOICE_LINE                          , &tr98Obj[e_TR98_VOICE_LINE                          -e_TR98_START]},
{e_TR98_VOICE_LINE_SIP                      , &tr98Obj[e_TR98_VOICE_LINE_SIP                      -e_TR98_START]},
//{e_TR98_VOICE_LINE_SIP_EVENT_SUBS           , &tr98Obj[e_TR98_VOICE_LINE_SIP_EVENT_SUBS           -e_TR98_START]},
//{e_TR98_VOICE_LINE_RINGER                   , &tr98Obj[e_TR98_VOICE_LINE_RINGER                   -e_TR98_START]},
//{e_TR98_VOICE_LINE_RINGER_EVENT             , &tr98Obj[e_TR98_VOICE_LINE_RINGER_EVENT             -e_TR98_START]},
//{e_TR98_VOICE_LINE_RINGER_DESCRIPTION       , &tr98Obj[e_TR98_VOICE_LINE_RINGER_DESCRIPTION       -e_TR98_START]},
//{e_TR98_VOICE_LINE_RINGER_PATTERN           , &tr98Obj[e_TR98_VOICE_LINE_RINGER_PATTERN           -e_TR98_START]},
{e_TR98_VOICE_LINE_CALLING_FEATURE          , &tr98Obj[e_TR98_VOICE_LINE_CALLING_FEATURE          -e_TR98_START]},
{e_TR98_VOICE_LINE_PROCESSING               , &tr98Obj[e_TR98_VOICE_LINE_PROCESSING               -e_TR98_START]},
{e_TR98_VOICE_LINE_CODEC                    , &tr98Obj[e_TR98_VOICE_LINE_CODEC                    -e_TR98_START]},
{e_TR98_VOICE_LINE_CODEC_LIST               , &tr98Obj[e_TR98_VOICE_LINE_CODEC_LIST               -e_TR98_START]},
//{e_TR98_VOICE_LINE_SESSION                  , &tr98Obj[e_TR98_VOICE_LINE_SESSION                  -e_TR98_START]},
{e_TR98_VOICE_LINE_STATS                    , &tr98Obj[e_TR98_VOICE_LINE_STATS                    -e_TR98_START]},
{e_TR98_VOICE_PHY_INTF                      , &tr98Obj[e_TR98_VOICE_PHY_INTF                      -e_TR98_START]},
{e_TR98_VOICE_FXS_CID                       , &tr98Obj[e_TR98_VOICE_FXS_CID                       -e_TR98_START]},
//{e_TR98_VOICE_PSTN                          , &tr98Obj[e_TR98_VOICE_PSTN                          -e_TR98_START]},
{e_TR98_VOICE_COMMON                        , &tr98Obj[e_TR98_VOICE_COMMON                        -e_TR98_START]},
//{e_TR98_VOICE_PHONE_BOOK                    , &tr98Obj[e_TR98_VOICE_PHONE_BOOK                    -e_TR98_START]},
{e_TR98_VOICE_CALL_POLICY_BOOK              , &tr98Obj[e_TR98_VOICE_CALL_POLICY_BOOK              -e_TR98_START]},
//{e_TR98_VOICE_FXO                           , &tr98Obj[e_TR98_VOICE_FXO                           -e_TR98_START]},
//{e_TR98_VOICE_FXO_PORT                      , &tr98Obj[e_TR98_VOICE_FXO_PORT                      -e_TR98_START]},
#endif
#ifdef AVAST_AGENT_PACKAGE
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{e_TR98_ISP_SERVICE             , &tr98Obj[e_TR98_ISP_SERVICE             -e_TR98_START]},
#endif
#endif
{e_TR98_SVR_SELECT_DIAG         , &tr98Obj[e_TR98_SVR_SELECT_DIAG         -e_TR98_START]},
{e_TR98_CAPB                    , &tr98Obj[e_TR98_CAPB                    -e_TR98_START]},
{e_TR98_PERF_DIAG               , &tr98Obj[e_TR98_PERF_DIAG               -e_TR98_START]},
{e_TR98_DEV_INFO                , &tr98Obj[e_TR98_DEV_INFO                -e_TR98_START]},
{e_TR98_MemoryStatus			, &tr98Obj[e_TR98_MemoryStatus			  -e_TR98_START]},
{e_TR98_VEND_CONF_FILE          , &tr98Obj[e_TR98_VEND_CONF_FILE          -e_TR98_START]},
{e_TR98_DEV_INFO_PS_STAT		, &tr98Obj[e_TR98_DEV_INFO_PS_STAT		  -e_TR98_START]},
{e_TR98_DEV_INFO_PS_STAT_PS		, &tr98Obj[e_TR98_DEV_INFO_PS_STAT_PS	  -e_TR98_START]},
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
{e_TR98_DEV_CONF                , &tr98Obj[e_TR98_DEV_CONF                -e_TR98_START]},
#endif
//{e_TR98_DEV_CONF                , &tr98Obj[e_TR98_DEV_CONF                -e_TR98_START]},
{e_TR98_MGMT_SRV                , &tr98Obj[e_TR98_MGMT_SRV                -e_TR98_START]},
{e_TR98_MGAB_DEV                , &tr98Obj[e_TR98_MGAB_DEV                -e_TR98_START]},
{e_TR98_TIME                    , &tr98Obj[e_TR98_TIME                    -e_TR98_START]},
//{e_TR98_USR_INTF                , &tr98Obj[e_TR98_USR_INTF                -e_TR98_START]},
//{e_TR98_CAPT_PORTAL             , &tr98Obj[e_TR98_CAPT_PORTAL             -e_TR98_START]},
{e_TR98_L3_FWD                  , &tr98Obj[e_TR98_L3_FWD                  -e_TR98_START]},
{e_TR98_FWD                     , &tr98Obj[e_TR98_FWD                     -e_TR98_START]},
#if defined(L3IPV6FWD) && !defined(CONFIG_TAAAB_CUSTOMIZATION_TR069)
{e_TR98_IPV6FWD 				, &tr98Obj[e_TR98_IPV6FWD				  -e_TR98_START]},
#endif
{e_TR98_L2_BR                   , &tr98Obj[e_TR98_L2_BR                   -e_TR98_START]},
{e_TR98_BR                      , &tr98Obj[e_TR98_BR                      -e_TR98_START]},
{e_TR98_PORT                    , &tr98Obj[e_TR98_PORT                    -e_TR98_START]},
{e_TR98_VLAN                    , &tr98Obj[e_TR98_VLAN                    -e_TR98_START]},
{e_TR98_FILTER                  , &tr98Obj[e_TR98_FILTER                  -e_TR98_START]},
{e_TR98_MARK                    , &tr98Obj[e_TR98_MARK                    -e_TR98_START]},
{e_TR98_AVAI_INTF               , &tr98Obj[e_TR98_AVAI_INTF               -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
{e_TR98_QUE_MGMT                , &tr98Obj[e_TR98_QUE_MGMT                -e_TR98_START]},
{e_TR98_CLS                     , &tr98Obj[e_TR98_CLS                     -e_TR98_START]},
//{e_TR98_APP                     , &tr98Obj[e_TR98_APP                     -e_TR98_START]},
//{e_TR98_FLOW                    , &tr98Obj[e_TR98_FLOW                    -e_TR98_START]},
{e_TR98_POLICER                 , &tr98Obj[e_TR98_POLICER                 -e_TR98_START]},
{e_TR98_QUE                     , &tr98Obj[e_TR98_QUE                     -e_TR98_START]},
{e_TR98_SHAPER                  , &tr98Obj[e_TR98_SHAPER                  -e_TR98_START]},
//{e_TR98_QUE_STAT                , &tr98Obj[e_TR98_QUE_STAT                -e_TR98_START]},
#endif
{e_TR98_LAN_CONF_SEC            , &tr98Obj[e_TR98_LAN_CONF_SEC            -e_TR98_START]},
{e_TR98_IP_PING_DIAG            , &tr98Obj[e_TR98_IP_PING_DIAG            -e_TR98_START]},
{e_TR98_TRA_RT_DIAG             , &tr98Obj[e_TR98_TRA_RT_DIAG             -e_TR98_START]},
{e_TR98_RT_HOP                  , &tr98Obj[e_TR98_RT_HOP                  -e_TR98_START]},
{e_TR98_NSLOOKUP_DIAG           , &tr98Obj[e_TR98_NSLOOKUP_DIAG           -e_TR98_START]},
{e_TR98_NSLOOKUP_RESULT         , &tr98Obj[e_TR98_NSLOOKUP_RESULT         -e_TR98_START]},
{e_TR98_DL_DIAG                 , &tr98Obj[e_TR98_DL_DIAG                 -e_TR98_START]},
{e_TR98_DL_DIAG_PER_CONN_RST 	, &tr98Obj[e_TR98_DL_DIAG_PER_CONN_RST	  -e_TR98_START]},
{e_TR98_DL_DIAG_INCREM_RST      , &tr98Obj[e_TR98_DL_DIAG_INCREM_RST      -e_TR98_START]},
{e_TR98_UL_DIAG                 , &tr98Obj[e_TR98_UL_DIAG                 -e_TR98_START]},
{e_TR98_UL_DIAG_PER_CONN_RST	, &tr98Obj[e_TR98_UL_DIAG_PER_CONN_RST	  -e_TR98_START]},
{e_TR98_UL_DIAG_INCREM_RST		, &tr98Obj[e_TR98_UL_DIAG_INCREM_RST	  -e_TR98_START]},
{e_TR98_UDP_ECHO_CONF           , &tr98Obj[e_TR98_UDP_ECHO_CONF           -e_TR98_START]},
{e_TR98_UDP_ECHO_DIAG			, &tr98Obj[e_TR98_UDP_ECHO_DIAG 		  -e_TR98_START]},
{e_TR98_UDP_ECHO_DIAG_PAK_RST	, &tr98Obj[e_TR98_UDP_ECHO_DIAG_PAK_RST   -e_TR98_START]},
{e_TR98_LAN_DEV                 , &tr98Obj[e_TR98_LAN_DEV                 -e_TR98_START]},
{e_TR98_LAN_HOST_CONF_MGMT      , &tr98Obj[e_TR98_LAN_HOST_CONF_MGMT      -e_TR98_START]},
{e_TR98_IP_INTF                 , &tr98Obj[e_TR98_IP_INTF                 -e_TR98_START]},
#ifdef LANIPALIAS
{e_TR98_IP_ALIAS            ,&tr98Obj[e_TR98_IP_ALIAS                 -e_TR98_START]},
#endif
#ifdef IPV6INTERFACE_PROFILE
{e_TR98_IP_INTF_IPV6ADDR        , &tr98Obj[e_TR98_IP_INTF_IPV6ADDR        -e_TR98_START]},
{e_TR98_IP_INTF_IPV6PREFIX      , &tr98Obj[e_TR98_IP_INTF_IPV6PREFIX      -e_TR98_START]},
#endif
#ifdef ROUTERADVERTISEMENT_PROFILE
{e_TR98_IP_INTF_ROUTERADVER     , &tr98Obj[e_TR98_IP_INTF_ROUTERADVER     -e_TR98_START]},
#endif
#ifdef IPV6SERVER_PROFILE
{e_TR98_IP_INTF_DHCPV6SRV       , &tr98Obj[e_TR98_IP_INTF_DHCPV6SRV       -e_TR98_START]},
#endif
{e_TR98_DHCP_STATIC_ADDR        , &tr98Obj[e_TR98_DHCP_STATIC_ADDR        -e_TR98_START]},
{e_TR98_DHCP_OPT                , &tr98Obj[e_TR98_DHCP_OPT                -e_TR98_START]},
#ifdef ZyXEL_DHCP_OPTION125
{e_TR98_VENDOR_SPECIFIC             , &tr98Obj[e_TR98_VENDOR_SPECIFIC             -e_TR98_START]},
{e_TR98_VENDOR_SPECIFIC_PROF            , &tr98Obj[e_TR98_VENDOR_SPECIFIC_PROF            -e_TR98_START]},
#endif
{e_TR98_DHCP_COND_SERVPOOL      , &tr98Obj[e_TR98_DHCP_COND_SERVPOOL      -e_TR98_START]},
{e_TR98_SERVPOOL_DHCP_STATICADDR, &tr98Obj[e_TR98_SERVPOOL_DHCP_STATICADDR-e_TR98_START]},
{e_TR98_SERVPOOL_DHCP_OPT       , &tr98Obj[e_TR98_SERVPOOL_DHCP_OPT       -e_TR98_START]},
{e_TR98_LAN_ETH_INTF_CONF       , &tr98Obj[e_TR98_LAN_ETH_INTF_CONF       -e_TR98_START]},
{e_TR98_LAN_ETH_INTF_CONF_STAT  , &tr98Obj[e_TR98_LAN_ETH_INTF_CONF_STAT  -e_TR98_START]},
//{e_TR98_LAN_USB_INTF_CONF       , &tr98Obj[e_TR98_LAN_USB_INTF_CONF       -e_TR98_START]},
//{e_TR98_LAN_USB_INTF_CONF_STAT  , &tr98Obj[e_TR98_LAN_USB_INTF_CONF_STAT  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_LAN_DEV_WLAN_CFG        , &tr98Obj[e_TR98_LAN_DEV_WLAN_CFG        -e_TR98_START]},
{e_TR98_WLAN_CFG_STAT           , &tr98Obj[e_TR98_WLAN_CFG_STAT           -e_TR98_START]},
{e_TR98_WPS                     , &tr98Obj[e_TR98_WPS                     -e_TR98_START]},
#endif
//{e_TR98_REG                     , &tr98Obj[e_TR98_REG                     -e_TR98_START]},
{e_TR98_ASSOC_DEV               , &tr98Obj[e_TR98_ASSOC_DEV               -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_ASSOC_DEV_STAT			, &tr98Obj[e_TR98_ASSOC_DEV_STAT 		  -e_TR98_START]},
{e_TR98_WEP_KEY                 , &tr98Obj[e_TR98_WEP_KEY                 -e_TR98_START]},
{e_TR98_PSK                     , &tr98Obj[e_TR98_PSK                     -e_TR98_START]},
#endif
//{e_TR98_AP_WMM_PARAM            , &tr98Obj[e_TR98_AP_WMM_PARAM            -e_TR98_START]},
//{e_TR98_STA_WMM_PARAM           , &tr98Obj[e_TR98_STA_WMM_PARAM           -e_TR98_START]},
{e_TR98_HOSTS                   , &tr98Obj[e_TR98_HOSTS                   -e_TR98_START]},
{e_TR98_HOST                    , &tr98Obj[e_TR98_HOST                    -e_TR98_START]},
#ifdef ONECONNECT
{e_TR98_ONECONNECT_EXT		   , &tr98Obj[e_TR98_ONECONNECT_EXT 	 -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_NEIBOR_WIFI_DIAG        , &tr98Obj[e_TR98_NEIBOR_WIFI_DIAG        -e_TR98_START]},
{e_TR98_NEIBOR_WIFI_DIAG_RESULT , &tr98Obj[e_TR98_NEIBOR_WIFI_DIAG_RESULT -e_TR98_START]},
#endif
{e_TR98_LAN_INTF                , &tr98Obj[e_TR98_LAN_INTF                -e_TR98_START]},
//{e_TR98_LAN_INTF_ETH_INTF_CONF  , &tr98Obj[e_TR98_LAN_INTF_ETH_INTF_CONF  -e_TR98_START]},
//{e_TR98_USB_INTF_CONF           , &tr98Obj[e_TR98_USB_INTF_CONF           -e_TR98_START]},
//{e_TR98_LAN_INTF_WLAN_CFG       , &tr98Obj[e_TR98_LAN_INTF_WLAN_CFG       -e_TR98_START]},
{e_TR98_WAN_DEV                 , &tr98Obj[e_TR98_WAN_DEV                 -e_TR98_START]},
{e_TR98_WAN_COMM_INTF_CONF      , &tr98Obj[e_TR98_WAN_COMM_INTF_CONF      -e_TR98_START]},
//{e_TR98_WAN_COMM_INTF_CONNECT   , &tr98Obj[e_TR98_WAN_COMM_INTF_CONNECT   -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{e_TR98_WAN_DSL_INTF_CONF       , &tr98Obj[e_TR98_WAN_DSL_INTF_CONF       -e_TR98_START]},
{e_TR98_WAN_DSL_INTF_TEST_PARAM , &tr98Obj[e_TR98_WAN_DSL_INTF_TEST_PARAM -e_TR98_START]},
{e_TR98_WAN_DSL_INTF_CONF_STAT  , &tr98Obj[e_TR98_WAN_DSL_INTF_CONF_STAT  -e_TR98_START]},
{e_TR98_TTL                     , &tr98Obj[e_TR98_TTL                     -e_TR98_START]},
{e_TR98_ST                      , &tr98Obj[e_TR98_ST                      -e_TR98_START]},
//{e_TR98_LST_SHOWTIME            , &tr98Obj[e_TR98_LST_SHOWTIME            -e_TR98_START]},
{e_TR98_CURRENT_DAY             , &tr98Obj[e_TR98_CURRENT_DAY             -e_TR98_START]},
{e_TR98_QTR_HR                  , &tr98Obj[e_TR98_QTR_HR                  -e_TR98_START]},
#endif // CONFIG_ZCFG_BE_MODULE_DSL
{e_TR98_WAN_ETH_INTF_CONF       , &tr98Obj[e_TR98_WAN_ETH_INTF_CONF       -e_TR98_START]},
{e_TR98_WAN_ETH_INTF_CONF_STAT  , &tr98Obj[e_TR98_WAN_ETH_INTF_CONF_STAT  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{e_TR98_WAN_DSL_DIAG            , &tr98Obj[e_TR98_WAN_DSL_DIAG            -e_TR98_START]},
#endif
{e_TR98_WAN_CONN_DEV            , &tr98Obj[e_TR98_WAN_CONN_DEV            -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
{e_TR98_WAN_DSL_LINK_CONF       , &tr98Obj[e_TR98_WAN_DSL_LINK_CONF       -e_TR98_START]},
{e_TR98_WAN_ATM_F5_LO_DIAG      , &tr98Obj[e_TR98_WAN_ATM_F5_LO_DIAG      -e_TR98_START]},
{e_TR98_WAN_ATM_F4_LO_DIAG      , &tr98Obj[e_TR98_WAN_ATM_F4_LO_DIAG      -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
{e_TR98_WAN_PTM_LINK_CONF       , &tr98Obj[e_TR98_WAN_PTM_LINK_CONF       -e_TR98_START]},
{e_TR98_WAN_PTM_LINK_CONF_STAT  , &tr98Obj[e_TR98_WAN_PTM_LINK_CONF_STAT  -e_TR98_START]},
#endif
{e_TR98_WAN_ETH_LINK_CONF       , &tr98Obj[e_TR98_WAN_ETH_LINK_CONF       -e_TR98_START]},
//{e_TR98_WAN_POTS_LINK_CONF      , &tr98Obj[e_TR98_WAN_POTS_LINK_CONF      -e_TR98_START]},
{e_TR98_WAN_IP_CONN             , &tr98Obj[e_TR98_WAN_IP_CONN             -e_TR98_START]},
{e_TR98_WAN_IP_CONN_MLD         , &tr98Obj[e_TR98_WAN_IP_CONN_MLD         -e_TR98_START]},
{e_TR98_DHCP_CLIENT             , &tr98Obj[e_TR98_DHCP_CLIENT             -e_TR98_START]},
{e_TR98_SENT_DHCP_OPT           , &tr98Obj[e_TR98_SENT_DHCP_OPT           -e_TR98_START]},
{e_TR98_REQ_DHCP_OPT            , &tr98Obj[e_TR98_REQ_DHCP_OPT            -e_TR98_START]},
{e_TR98_WAN_IP_PORT_MAP         , &tr98Obj[e_TR98_WAN_IP_PORT_MAP         -e_TR98_START]},
{e_TR98_WAN_IP_PORT_TRIGGER     , &tr98Obj[e_TR98_WAN_IP_PORT_TRIGGER     -e_TR98_START]},
{e_TR98_WAN_IP_ADDR_MAPPING     , &tr98Obj[e_TR98_WAN_IP_ADDR_MAPPING     -e_TR98_START]},
{e_TR98_WAN_IP_CONN_STAT        , &tr98Obj[e_TR98_WAN_IP_CONN_STAT        -e_TR98_START]},
#ifdef IPV6INTERFACE_PROFILE
{e_TR98_WAN_IP_CONN_IPV6ADDR    , &tr98Obj[e_TR98_WAN_IP_CONN_IPV6ADDR    -e_TR98_START]},
{e_TR98_WAN_IP_CONN_IPV6PREFIX	 , &tr98Obj[e_TR98_WAN_IP_CONN_IPV6PREFIX	 -e_TR98_START]},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{e_TR98_WAN_IP_CONN_DHCPV6CLIENT, &tr98Obj[e_TR98_WAN_IP_CONN_DHCPV6CLIENT-e_TR98_START]},
#endif
#ifdef IPV6RD_PROFILE
{e_TR98_WAN_IP_CONN_IPV6RD	     , &tr98Obj[e_TR98_WAN_IP_CONN_IPV6RD	     -e_TR98_START]},
#endif
#ifdef ROUTEINFO_INTERFACE
{e_TR98_WAN_IP_CONN_ROUTEINFO    , &tr98Obj[e_TR98_WAN_IP_CONN_ROUTEINFO     -e_TR98_START]},
#endif
{e_TR98_WAN_PPP_CONN            , &tr98Obj[e_TR98_WAN_PPP_CONN            -e_TR98_START]},
{e_TR98_WAN_PPP_PORT_MAP        , &tr98Obj[e_TR98_WAN_PPP_PORT_MAP        -e_TR98_START]},
{e_TR98_WAN_PPP_PORT_TRIGGER    , &tr98Obj[e_TR98_WAN_PPP_PORT_TRIGGER    -e_TR98_START]},
{e_TR98_WAN_PPP_ADDR_MAPPING    , &tr98Obj[e_TR98_WAN_PPP_ADDR_MAPPING    -e_TR98_START]},
{e_TR98_WAN_PPP_CONN_STAT       , &tr98Obj[e_TR98_WAN_PPP_CONN_STAT       -e_TR98_START]},
{e_TR98_WAN_PPP_CONN_MLD        , &tr98Obj[e_TR98_WAN_PPP_CONN_MLD        -e_TR98_START]},
#ifdef IPV6INTERFACE_PROFILE
{e_TR98_WAN_PPP_CONN_IPV6ADDR    , &tr98Obj[e_TR98_WAN_PPP_CONN_IPV6ADDR    -e_TR98_START]},
{e_TR98_WAN_PPP_CONN_IPV6PREFIX  , &tr98Obj[e_TR98_WAN_PPP_CONN_IPV6PREFIX  -e_TR98_START]},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{e_TR98_WAN_PPP_CONN_DHCPV6CLIENT, &tr98Obj[e_TR98_WAN_PPP_CONN_DHCPV6CLIENT-e_TR98_START]},
#endif
#ifdef IPV6RD_PROFILE
{e_TR98_WAN_PPP_CONN_IPV6RD      , &tr98Obj[e_TR98_WAN_PPP_CONN_IPV6RD      -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
{e_TR98_WWAN_INTERFACE_CONFIG   ,&tr98Obj[e_TR98_WWAN_INTERFACE_CONFIG        -e_TR98_START]},
{e_TR98_WWAN_INTERFACE_CONFIG_STATUS	,&tr98Obj[e_TR98_WWAN_INTERFACE_CONFIG_STATUS		-e_TR98_START]},
#endif
{e_TR98_WWAN_EMAIL_CONFIG   ,&tr98Obj[e_TR98_WWAN_EMAIL_CONFIG        -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
{e_TR98_WAN_PON_INTF_CONF      , &tr98Obj[e_TR98_WAN_PON_INTF_CONF            -e_TR98_START]},
{e_TR98_WAN_PON_INTF_CONF_STAT      , &tr98Obj[e_TR98_WAN_PON_INTF_CONF_STAT  -e_TR98_START]},
#endif
{e_TR98_FIREWALL                , &tr98Obj[e_TR98_FIREWALL                -e_TR98_START]},
{e_TR98_FIREWALL_LEVEL          , &tr98Obj[e_TR98_FIREWALL_LEVEL          -e_TR98_START]},
{e_TR98_FIREWALL_CHAIN          , &tr98Obj[e_TR98_FIREWALL_CHAIN          -e_TR98_START]},
{e_TR98_FIREWALL_CHAIN_RULE     , &tr98Obj[e_TR98_FIREWALL_CHAIN_RULE     -e_TR98_START]},
{e_TR98_STD_UPNP				, &tr98Obj[e_TR98_STD_UPNP				  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{e_TR98_STD_UPNP_DEV			, &tr98Obj[e_TR98_STD_UPNP_DEV			  -e_TR98_START]},
#endif
#ifdef ZYXEL_IPV6SPEC
{e_TR98_IPV6SPEC				, &tr98Obj[e_TR98_IPV6SPEC				  -e_TR98_START]},
{e_TR98_IPV6_PPP 				, &tr98Obj[e_TR98_IPV6_PPP				  -e_TR98_START]},
{e_TR98_IPV6_PPP_INTF			, &tr98Obj[e_TR98_IPV6_PPP_INTF			  -e_TR98_START]},
{e_TR98_IPV6_PPP_INTF_IPV6CP	, &tr98Obj[e_TR98_IPV6_PPP_INTF_IPV6CP	  -e_TR98_START]},
{e_TR98_IPV6_IP					, &tr98Obj[e_TR98_IPV6_IP				  -e_TR98_START]},
{e_TR98_IPV6_IP_INTF			, &tr98Obj[e_TR98_IPV6_IP_INTF			  -e_TR98_START]},
{e_TR98_IPV6_IP_INTF_IPV6ADDR	, &tr98Obj[e_TR98_IPV6_IP_INTF_IPV6ADDR   -e_TR98_START]},
{e_TR98_IPV6_IP_INTF_IPV6PREFIX	, &tr98Obj[e_TR98_IPV6_IP_INTF_IPV6PREFIX -e_TR98_START]},
{e_TR98_ROUTING					, &tr98Obj[e_TR98_ROUTING 				  -e_TR98_START]},
{e_TR98_ROUTER					, &tr98Obj[e_TR98_ROUTER 				  -e_TR98_START]},
{e_TR98_ROUTER_IPV6FWD			, &tr98Obj[e_TR98_ROUTER_IPV6FWD 		  -e_TR98_START]},
{e_TR98_ROUTEINFO				, &tr98Obj[e_TR98_ROUTEINFO 			  -e_TR98_START]},
{e_TR98_ROUTEINFO_INTF			, &tr98Obj[e_TR98_ROUTEINFO_INTF 		  -e_TR98_START]},
{e_TR98_ND						, &tr98Obj[e_TR98_ND		  			  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
{e_TR98_ND_INTF					, &tr98Obj[e_TR98_ND_INTF		  		  -e_TR98_START]},
#endif
{e_TR98_RTADR					, &tr98Obj[e_TR98_RTADR 				  -e_TR98_START]},
{e_TR98_RTADR_INTF				, &tr98Obj[e_TR98_RTADR_INTF			  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
{e_TR98_RTADR_INTF_OPT			, &tr98Obj[e_TR98_RTADR_INTF_OPT		  -e_TR98_START]},
#endif
{e_TR98_IPV6_HOSTS				, &tr98Obj[e_TR98_IPV6_HOSTS			  -e_TR98_START]},
{e_TR98_IPV6_HOSTS_HOST			, &tr98Obj[e_TR98_IPV6_HOSTS_HOST		  -e_TR98_START]},
{e_TR98_IPV6_HOSTS_IPV6ADR		, &tr98Obj[e_TR98_IPV6_HOSTS_IPV6ADR	  -e_TR98_START]},
{e_TR98_IPV6_DNS				, &tr98Obj[e_TR98_IPV6_DNS	  			  -e_TR98_START]},
{e_TR98_IPV6_DNS_CLIENT			, &tr98Obj[e_TR98_IPV6_DNS_CLIENT	  	  -e_TR98_START]},
{e_TR98_IPV6_DNS_SERVER			, &tr98Obj[e_TR98_IPV6_DNS_SERVER	      -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_DNS_RELAY
{e_TR98_IPV6_DNS_RELAY			, &tr98Obj[e_TR98_IPV6_DNS_RELAY	      -e_TR98_START]},
{e_TR98_IPV6_DNS_RELAYFWD		, &tr98Obj[e_TR98_IPV6_DNS_RELAYFWD	      -e_TR98_START]},
#endif
{e_TR98_IPV6_DHCPV6				, &tr98Obj[e_TR98_IPV6_DHCPV6 	 		  -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_CLIENT		, &tr98Obj[e_TR98_IPV6_DHCPV6_CLIENT 	  -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_CLIENT_SRV	, &tr98Obj[e_TR98_IPV6_DHCPV6_CLIENT_SRV  -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_CLIENT_SENT	, &tr98Obj[e_TR98_IPV6_DHCPV6_CLIENT_SENT -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_CLIENT_RCV	, &tr98Obj[e_TR98_IPV6_DHCPV6_CLIENT_RCV  -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER		, &tr98Obj[e_TR98_IPV6_DHCPV6_SERVER 	  -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_POOL	, &tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_POOL -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_CNT  , &tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_CNT -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR	,&tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX	,&tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_CNT_OPT	,&tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_CNT_OPT -e_TR98_START]},
{e_TR98_IPV6_DHCPV6_SERVER_OPT	, &tr98Obj[e_TR98_IPV6_DHCPV6_SERVER_OPT -e_TR98_START]},
#endif
#ifdef ZYXEL_IPV6_MAP
{e_TR98_IPV6MAP                 , &tr98Obj[e_TR98_IPV6MAP                 -e_TR98_START]},
{e_TR98_IPV6MAP_DOMAIN          , &tr98Obj[e_TR98_IPV6MAP_DOMAIN          -e_TR98_START]},
{e_TR98_IPV6MAP_DOMAIN_RULE     , &tr98Obj[e_TR98_IPV6MAP_DOMAIN_RULE     -e_TR98_START]},
#endif
#ifdef ZYXEL_GRE_SUPPORT
{e_TR98_GRE						,&tr98Obj[e_TR98_GRE 					-e_TR98_START]},
{e_TR98_GRE_TUNNEL				,&tr98Obj[e_TR98_GRE_TUNNEL 			-e_TR98_START]},
{e_TR98_GRE_TUNNEL_STATS		,&tr98Obj[e_TR98_GRE_TUNNEL_STATS 		-e_TR98_START]},
#endif
#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng
{e_TR98_TAAAB_H323Alg				, &tr98Obj[e_TR98_TAAAB_H323Alg 			  -e_TR98_START]},
{e_TR98_TAAAB_SIPAlg				, &tr98Obj[e_TR98_TAAAB_SIPAlg			  -e_TR98_START]},
{e_TR98_TAAAB_RTSPAlg				, &tr98Obj[e_TR98_TAAAB_RTSPAlg			  -e_TR98_START]},
{e_TR98_TAAAB_PPTPAlg				, &tr98Obj[e_TR98_TAAAB_PPTPAlg			  -e_TR98_START]},
{e_TR98_TAAAB_IPSecAlg				, &tr98Obj[e_TR98_TAAAB_IPSecAlg			  -e_TR98_START]},
{e_TR98_TAAAB_L2TPAlg				, &tr98Obj[e_TR98_TAAAB_L2TPAlg			  -e_TR98_START]},
{e_TR98_TAAAB_IGMPProxy			, &tr98Obj[e_TR98_TAAAB_IGMPProxy			  -e_TR98_START]},
{e_TR98_TAAAB_SMTP					, &tr98Obj[e_TR98_TAAAB_SMTP			  -e_TR98_START]},
{e_TR98_TAAAB_ICMP 				, &tr98Obj[e_TR98_TAAAB_ICMP			  -e_TR98_START]},
{e_TR98_TAAAB_NTP 				, &tr98Obj[e_TR98_TAAAB_NTP			  -e_TR98_START]},
#endif
#ifdef ZYXEL_TAAAG_EDNS// "Add ppp username to edns option for TAAAB"
{e_TR98_TAAAB_EDNS					, &tr98Obj[e_TR98_TAAAB_EDNS 			  -e_TR98_START]},
#endif
#ifdef CONFIG_TAAAB_PORTMIRROR
{e_TR98_TAAAB_PORTMIRROR		, &tr98Obj[e_TR98_TAAAB_PORTMIRROR			  -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
{e_TR98_TAAAB_PACKETCAPTURE		, &tr98Obj[e_TR98_TAAAB_PACKETCAPTURE			  -e_TR98_START]},
{e_TR98_TAAAB_PACKETCAPTUREUPLOAD		, &tr98Obj[e_TR98_TAAAB_PACKETCAPTUREUPLOAD			  -e_TR98_START]},
#endif
#if ZYXEL_EXT
{e_TR98_ZYXEL_EXT               , &tr98Obj[e_TR98_ZYXEL_EXT               -e_TR98_START]},
#ifdef ZYXEL_LANDING_PAGE_FEATURE
{e_TR98_LANDING_PAGE			, &tr98Obj[e_TR98_LANDING_PAGE			  -e_TR98_START]},
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
{e_TR98_ONLINE_FWUPGRADE			, &tr98Obj[e_TR98_ONLINE_FWUPGRADE			  -e_TR98_START]},
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
{e_TR98_WLAN_SCHEDULER			, &tr98Obj[e_TR98_WLAN_SCHEDULER			  -e_TR98_START]},
{e_TR98_WLAN_SCHEDULER_Rule          , &tr98Obj[e_TR98_WLAN_SCHEDULER_Rule     	  -e_TR98_START]},
#endif
{e_TR98_FEATURE_FLAG			, &tr98Obj[e_TR98_FEATURE_FLAG			  -e_TR98_START]},
{e_TR98_DNS			            , &tr98Obj[e_TR98_DNS         			  -e_TR98_START]},
{e_TR98_DNS_RT_ENTRY            , &tr98Obj[e_TR98_DNS_RT_ENTRY            -e_TR98_START]},
{e_TR98_DNS_ENTRY    	        , &tr98Obj[e_TR98_DNS_ENTRY     	      -e_TR98_START]},
{e_TR98_D_DNS                   , &tr98Obj[e_TR98_D_DNS                   -e_TR98_START]},
#ifdef ZCFG_TR64
{e_TR98_TR064	                , &tr98Obj[e_TR98_TR064	                  -e_TR98_START]},
#else
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
{e_TR98_VLAN_GROUP	            , &tr98Obj[e_TR98_VLAN_GROUP	          -e_TR98_START]},
#endif
{e_TR98_EMAIL_NOTIFY            , &tr98Obj[e_TR98_EMAIL_NOTIFY            -e_TR98_START]},
{e_TR98_EMAIL_SERVICE           , &tr98Obj[e_TR98_EMAIL_SERVICE	          -e_TR98_START]},
{e_TR98_EMAIL_EVENT_CFG         , &tr98Obj[e_TR98_EMAIL_EVENT_CFG         -e_TR98_START]},
{e_TR98_SYS_INFO                , &tr98Obj[e_TR98_SYS_INFO                -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
{e_TR98_GPON_INFO               , &tr98Obj[e_TR98_GPON_INFO                -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
{e_TR98_SNMP	                , &tr98Obj[e_TR98_SNMP					  -e_TR98_START]},
{e_TR98_SNMP_TRAP               , &tr98Obj[e_TR98_SNMP_TRAP				  -e_TR98_START]},
#endif
{e_TR98_SCHEDULE                , &tr98Obj[e_TR98_SCHEDULE                -e_TR98_START]},
{e_TR98_REMO_MGMT               , &tr98Obj[e_TR98_REMO_MGMT               -e_TR98_START]},
{e_TR98_REMO_SRV                , &tr98Obj[e_TR98_REMO_SRV                -e_TR98_START]},
{e_TR98_SP_REMO_SRV                , &tr98Obj[e_TR98_SP_REMO_SRV                -e_TR98_START]},
{e_TR98_REMO_TRUSTDOMAIN        , &tr98Obj[e_TR98_REMO_TRUSTDOMAIN        -e_TR98_START]},
{e_TR98_SP_TRUSTDOMAIN        , &tr98Obj[e_TR98_SP_TRUSTDOMAIN        -e_TR98_START]},
#ifdef ABUU_CUSTOMIZATION
{e_TR98_IFACETRUSTDOMAIN        , &tr98Obj[e_TR98_IFACETRUSTDOMAIN        -e_TR98_START]},
{e_TR98_IFACETRUSTDOMAININFO    , &tr98Obj[e_TR98_IFACETRUSTDOMAININFO    -e_TR98_START]},
#endif
{e_TR98_IEEE8021AG              , &tr98Obj[e_TR98_IEEE8021AG              -e_TR98_START]},
#ifdef ZYXEL_ETHOAM_TMSCTL
{e_TR98_IEEE8023AH				, &tr98Obj[e_TR98_IEEE8023AH			  -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{e_TR98_SAMBA		            , &tr98Obj[e_TR98_SAMBA		              -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{e_TR98_UPNP	           		, &tr98Obj[e_TR98_UPNP		              -e_TR98_START]},
#endif
{e_TR98_IGMP                    , &tr98Obj[e_TR98_IGMP                    -e_TR98_START]},
{e_TR98_MLD                    , &tr98Obj[e_TR98_MLD                    -e_TR98_START]},
#ifdef ZYXEL_TR69_DATA_USAGE
{e_TR98_DATA_USAGE	           		, &tr98Obj[e_TR98_DATA_USAGE		              -e_TR98_START]},
{e_TR98_DATA_USAGE_LAN                              , &tr98Obj[e_TR98_DATA_USAGE_LAN                          -e_TR98_START]},
{e_TR98_DATA_USAGE_WAN                              , &tr98Obj[e_TR98_DATA_USAGE_WAN                          -e_TR98_START]},
#endif
#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
{e_TR98_LOG_CFG           		 , &tr98Obj[e_TR98_LOG_CFG 				  -e_TR98_START]},
{e_TR98_LOG_CFG_GP           	 , &tr98Obj[e_TR98_LOG_CFG_GP             -e_TR98_START]},
{e_TR98_LOG_CFG_GP_ACCOUNT       , &tr98Obj[e_TR98_LOG_CFG_GP_ACCOUNT     -e_TR98_START]},
#endif
{e_TR98_LOG_SETTING             , &tr98Obj[e_TR98_LOG_SETTING             -e_TR98_START]},
{e_TR98_LOG_CLASSIFY            , &tr98Obj[e_TR98_LOG_CLASSIFY            -e_TR98_START]},
{e_TR98_LOG_CATEGORY	     	, &tr98Obj[e_TR98_LOG_CATEGORY	    	  -e_TR98_START]},
{e_TR98_MACFILTER	     , &tr98Obj[e_TR98_MACFILTER	     			  -e_TR98_START]},
{e_TR98_MACFILTER_WHITELIST	    , &tr98Obj[e_TR98_MACFILTER_WHITELIST	  -e_TR98_START]},
{e_TR98_PAREN_CTL           	, &tr98Obj[e_TR98_PAREN_CTL            	  -e_TR98_START]},
{e_TR98_PAREN_CTL_PROF          , &tr98Obj[e_TR98_PAREN_CTL_PROF     	  -e_TR98_START]},
{e_TR98_SEC		          		, &tr98Obj[e_TR98_SEC		              -e_TR98_START]},
{e_TR98_SEC_CERT		        , &tr98Obj[e_TR98_SEC_CERT		   		  -e_TR98_START]},
{e_TR98_GUI_CUSTOMIZATION		, &tr98Obj[e_TR98_GUI_CUSTOMIZATION		  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{e_TR98_DSL						, &tr98Obj[e_TR98_DSL					  -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
{e_TR98_ONECONNECT		       , &tr98Obj[e_TR98_ONECONNECT	     -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
{e_TR98_ONECONNECT_SPEEDTESTINFO	, &tr98Obj[e_TR98_ONECONNECT_SPEEDTESTINFO            -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
{e_TR98_AP_STEERING		       , &tr98Obj[e_TR98_AP_STEERING	     -e_TR98_START]},
#endif // #ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
{e_TR98_PROXIMITY_SENSOR		       , &tr98Obj[e_TR98_PROXIMITY_SENSOR	     -e_TR98_START]},
#endif
#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
{e_TR98_BANDWIDTH_IMPROVEMENT , &tr98Obj[e_TR98_BANDWIDTH_IMPROVEMENT	     -e_TR98_START]},
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT
#if ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_WLANCONFIG,                       &tr98Obj[e_TR98_WLANCONFIG	      -e_TR98_START]},
{e_TR98_AIRTIMEFAIRNESS,                  &tr98Obj[e_TR98_AIRTIMEFAIRNESS     -e_TR98_START]},
#endif
{e_TR98_ACCOUNT_RESETPASSWD,              &tr98Obj[e_TR98_ACCOUNT_RESETPASSWD -e_TR98_START]},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#if OBSOLETED
#else
//{e_TR98_WAN_DSL_CONN_MGMT       , &tr98Obj[e_TR98_WAN_DSL_CONN_MGMT       -e_TR98_START]},
//{e_TR98_WAN_DSL_CONN_SRVC       , &tr98Obj[e_TR98_WAN_DSL_CONN_SRVC       -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
{e_TR98_PRINT_SERVER             , &tr98Obj[e_TR98_PRINT_SERVER             -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{e_TR98_IPK_PKG					, &tr98Obj[e_TR98_IPK_PKG					-e_TR98_START]},
{e_TR98_IPK_PKG_INSTALL			, &tr98Obj[e_TR98_IPK_PKG_INSTALL			-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_WiFiCommunity			, &tr98Obj[e_TR98_WiFiCommunity			-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
{e_TR98_EasyMesh		       , &tr98Obj[e_TR98_EasyMesh	     -e_TR98_START]},
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
{e_TR98_VendorEasyMesh		   , &tr98Obj[e_TR98_VendorEasyMesh		 -e_TR98_START]},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
{e_TR98_ONESSID					, &tr98Obj[e_TR98_ONESSID					-e_TR98_START]},
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
{e_TR98_SFP_MGMT					, &tr98Obj[e_TR98_SFP_MGMT					-e_TR98_START]},
#endif
#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
{e_TR98_TAAAB_Configuration_Button				, &tr98Obj[e_TR98_TAAAB_Configuration_Button					-e_TR98_START]},
{e_TR98_TAAAB_Configuration_Button_WiFi		, &tr98Obj[e_TR98_TAAAB_Configuration_Button_WiFi					-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
{e_TR98_TAAAB_Configuration_Quantwifi			, &tr98Obj[e_TR98_TAAAB_Configuration_Quantwifi					-e_TR98_START]},
#endif
#ifdef ZYXEL_XMPP
{e_TR98_XMPP                        , &tr98Obj[e_TR98_XMPP                      -e_TR98_START]},
{e_TR98_XMPP_CONNECTION             , &tr98Obj[e_TR98_XMPP_CONNECTION           -e_TR98_START]},
{e_TR98_XMPP_CONNECTION_SERVER      , &tr98Obj[e_TR98_XMPP_CONNECTION_SERVER    -e_TR98_START]},
#endif //#ifdef ZYXEL_XMPP
#ifdef MSTC_TAAAG_MULTI_USER
{e_TR98_Nat_DMZ               , &tr98Obj[e_TR98_Nat_DMZ               -e_TR98_START]},
#endif
#if defined(ZYXEL_IGMPDIAG) || defined(ZYXEL_CUSTOMIZATION_SYSLOG)
{e_TR98_TAAAB_Function				, &tr98Obj[e_TR98_TAAAB_Function			  -e_TR98_START]},
#endif
#ifdef ZYXEL_IGMPDIAG
{e_TR98_TTIGMPDiagnostics			, &tr98Obj[e_TR98_TTIGMPDiagnostics			  -e_TR98_START]},
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
{e_TR98_TAAAB_WanConnDiagnostics			, &tr98Obj[e_TR98_TAAAB_WanConnDiagnostics			  -e_TR98_START]},
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
{e_TR98_TAAAB_Syslog				, &tr98Obj[e_TR98_TAAAB_Syslog 			  -e_TR98_START]},
{e_TR98_TAAAB_SyslogUpload				, &tr98Obj[e_TR98_TAAAB_SyslogUpload			  -e_TR98_START]},
#endif
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
{e_TR98_TAAAB               , &tr98Obj[e_TR98_TAAAB               -e_TR98_START]},
{e_TR98_TAAAB_Users				, &tr98Obj[e_TR98_TAAAB_Users 			  -e_TR98_START]},
{e_TR98_TAAAB_Users_User				, &tr98Obj[e_TR98_TAAAB_Users_User 			  -e_TR98_START]},
{e_TR98_TAAAB_UserInterface				 , &tr98Obj[e_TR98_TAAAB_UserInterface				-e_TR98_START]},
{e_TR98_TAAAB_UserInterface_RemoteAccess				 , &tr98Obj[e_TR98_TAAAB_UserInterface_RemoteAccess 			-e_TR98_START]},
{e_TR98_TAAAB_UserInterface_LocalAccess				 , &tr98Obj[e_TR98_TAAAB_UserInterface_LocalAccess 			-e_TR98_START]},
{e_TR98_TAAAB_Firewall				 , &tr98Obj[e_TR98_TAAAB_Firewall 			-e_TR98_START]},
{e_TR98_TAAAB_UPnP				 , &tr98Obj[e_TR98_TAAAB_UPnP 			-e_TR98_START]},
{e_TR98_TAAAB_UPnP_Device				 , &tr98Obj[e_TR98_TAAAB_UPnP_Device 			-e_TR98_START]},
{e_TR98_TAAAB_Configuration				 , &tr98Obj[e_TR98_TAAAB_Configuration				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL 			 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN 			 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance 			-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS 			-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture 			-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu 			 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu			-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess 			 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess				 , &tr98Obj[e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess				-e_TR98_START]},
{e_TR98_TAAAB_Configuration_Shell				 , &tr98Obj[e_TR98_TAAAB_Configuration_Shell			-e_TR98_START]},
#endif
#ifdef CONFIG_ZYXEL_TR140
#ifndef CONFIG_PACKAGE_ZyIMS
{e_TR98_SERVICE                             , &tr98Obj[e_TR98_SERVICE                             -e_TR98_START]},
#endif
{e_TR98_StorageService          , &tr98Obj[e_TR98_StorageService          -e_TR98_START]},
{e_TR98_StorageService_UserAccount			, &tr98Obj[e_TR98_StorageService_UserAccount		  -e_TR98_START]},
{e_TR98_StorageService_Capabilites			, &tr98Obj[e_TR98_StorageService_Capabilites		  -e_TR98_START]},
{e_TR98_StorageService_NetInfo			, &tr98Obj[e_TR98_StorageService_NetInfo		  -e_TR98_START]},
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{e_TR98_StorageService_NetworkServer			, &tr98Obj[e_TR98_StorageService_NetworkServer		  -e_TR98_START]},
#endif
{e_TR98_StorageService_FTPServer			, &tr98Obj[e_TR98_StorageService_FTPServer		  -e_TR98_START]},
{e_TR98_StorageService_PhysicalMedium			, &tr98Obj[e_TR98_StorageService_PhysicalMedium		  -e_TR98_START]},
{e_TR98_StorageService_LogicalVolume			, &tr98Obj[e_TR98_StorageService_LogicalVolume		  -e_TR98_START]},
{e_TR98_StorageService_LogicalVolume_Folder			, &tr98Obj[e_TR98_StorageService_LogicalVolume_Folder		  -e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
{e_TR98_Configuration_Lifemote					, &tr98Obj[e_TR98_Configuration_Lifemote					-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
{e_TR98_ZyEE_SoftwareMoudles 			                                , &tr98Obj[e_TR98_ZyEE_SoftwareMoudles		                            -e_TR98_START]},
{e_TR98_ZyEE_SoftwareMoudles_EeConf										, &tr98Obj[e_TR98_ZyEE_SoftwareMoudles_EeConf							-e_TR98_START]},
{e_TR98_ZyEE_SoftwareMoudles_ExecEnv 		                            , &tr98Obj[e_TR98_ZyEE_SoftwareMoudles_ExecEnv		                    -e_TR98_START]},
{e_TR98_ZyEE_SoftwareMoudles_DeploymentUnit			                    , &tr98Obj[e_TR98_ZyEE_SoftwareMoudles_DeploymentUnit		            -e_TR98_START]},
{e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit			                    , &tr98Obj[e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit		            -e_TR98_START]},
{e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions			        , &tr98Obj[e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions		    -e_TR98_START]},
{e_TR98_ZyEE_DUStateChangeComplPolicy			                        , &tr98Obj[e_TR98_ZyEE_DUStateChangeComplPolicy		                    -e_TR98_START]},
#endif
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
{e_TR98_MultipleConfig					, &tr98Obj[e_TR98_MultipleConfig					-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
{e_TR98_CELL,			&tr98Obj[e_TR98_CELL        		-e_TR98_START]},
{e_TR98_CELL_INTF,		&tr98Obj[e_TR98_CELL_INTF   		-e_TR98_START]},
{e_TR98_CELL_INTF_USIM,	&tr98Obj[e_TR98_CELL_INTF_USIM  	-e_TR98_START]},
{e_TR98_CELL_AP,		&tr98Obj[e_TR98_CELL_AP   			-e_TR98_START]},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
{e_TR98_WAN_MULTI_WAN,		&tr98Obj[e_TR98_WAN_MULTI_WAN        -e_TR98_START]},
{e_TR98_WAN_MULTI_WAN_INTF,	&tr98Obj[e_TR98_WAN_MULTI_WAN_INTF   -e_TR98_START]},
#endif
#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
{e_TR98_RUNNING_WAN_CONN		, &tr98Obj[e_TR98_RUNNING_WAN_CONN		  -e_TR98_START]},
#endif
{e_TR98_WAN_IP_V4ROUTINGTABLE   , &tr98Obj[e_TR98_WAN_IP_V4ROUTINGTABLE   -e_TR98_START]},
{e_TR98_WAN_IP_V6ROUTINGTABLE   , &tr98Obj[e_TR98_WAN_IP_V6ROUTINGTABLE   -e_TR98_START]}
};

static zcfgRet_t zcfgFeTr98ClassNameGet(char *objName, char *className);


#ifdef ZCFG_TR181_RPC_OBJECT_HANDLE_ROUT

static zcfgRet_t spvDhcpv4SrvOptRout(uint32_t , objIndex_t *, struct json_object *, struct json_object *, char *);

static tr181SpvHandleRout_t spvHandleRoutList[] = {
{RDM_OID_DHCPV4_SRV_POOL_OPT, spvDhcpv4SrvOptRout},
{0, NULL}
};

static struct json_object *tr181SpvHandleRoutObj = NULL;

//-------------------------------------------------------------------------------------------------------------------
static struct json_object *zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(void *);

static zcfgRet_t queryDhcpv4SrvOptRout(uint32_t , objIndex_t *, struct json_object *, void *);
static zcfgRet_t queryDhcpv4ReqOptRout(uint32_t , objIndex_t *, struct json_object *, void *);

static zcfgRet_t queryUPnPRout(uint32_t , objIndex_t *, struct json_object *, void *);

static zcfgRet_t queryFeatureFlagRout(uint32_t , objIndex_t *, struct json_object *, void *);
static zcfgRet_t queryGuiCustomRout(uint32_t , objIndex_t *, struct json_object *, void *);
static zcfgRet_t queryOiWzardRout(uint32_t , objIndex_t *, struct json_object *, void *);
static zcfgRet_t queryExtRout(uint32_t , objIndex_t *, struct json_object *, void *);


static tr181QueryHandleRout_t queryHandleRoutList[] = {
{RDM_OID_DHCPV4_SRV_POOL_OPT, queryDhcpv4SrvOptRout},
{RDM_OID_DHCPV4_REQ_OPT, queryDhcpv4ReqOptRout},
#ifdef CAAAC
{RDM_OID_UPNP_DEV, queryUPnPRout},
{RDM_OID_UPNP_DEV_CAPB, queryUPnPRout},
{RDM_OID_FEATURE_FLAG, queryFeatureFlagRout},
{RDM_OID_GUI_CUSTOMIZATION, queryGuiCustomRout},
#ifdef CONFIG_ZCFG_BE_MODULE_OAAAA_CUSTOMIZATION
{RDM_OID_OAAAA_WIZARD, queryOiWzardRout},
#endif
{RDM_OID_ZY_EXT, queryExtRout},
#endif
{0, NULL}
};




#define tr98objhead "InternetGatewayDevice"
#define tr181objhead "Device"

enum{
	DATA_MODEL_NOTDEFINED,
	DATA_MODEL_TR98,
	DATA_MODEL_TR181
};

static bool enableRootQuryBlackObjPath = 0;
static objPathIdx_t *rootQuryBlackObjPathIdx = NULL;
static objPathIdx_t *quryBlackObjPathIdx = NULL;
static int tr98QryRootPath;
static int runningDataModel = DATA_MODEL_TR98;

static bool zcfgQueryObjectPathIdxNotRepliedPathName(const char *objectPathName, objPathIdx_t *objectPathIdx);

static struct json_object *tr181QueryHandleRoutObj = NULL;

#endif

//#ifdef ZCFG_TR98_RPC_OBJECT_HANDLE_ROUT

static struct json_object *tr98QueryHandleRoutObj = NULL;

static zcfgRet_t tr98GpnObjWanDevRout(const char *, int, objIndex_t *, struct json_object **, void *data);
static zcfgRet_t tr98GpnObjHostRout(const char *, int, objIndex_t *, struct json_object **, void *data);
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
static zcfgRet_t tr98GpnObjWanDslIfCfgStsCuDayRout(const char *, int, objIndex_t *, struct json_object **, void *data);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
static zcfgRet_t tr98GpnObjWlanCfgRout(const char *, int , objIndex_t *, struct json_object **, void *);
#endif

tr98QueryHandleRout_t tr98GpnObjHandleRoutList[] = {
{e_TR98_HOSTS, tr98GpnObjHostRout},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{e_TR98_LAN_DEV_WLAN_CFG, tr98GpnObjWlanCfgRout},
#endif
{e_TR98_WAN_DEV, tr98GpnObjWanDevRout},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{e_TR98_CURRENT_DAY, tr98GpnObjWanDslIfCfgStsCuDayRout},
#endif
{0, NULL}
};

static struct json_object *tr98GpnQueryHandleRoutObj = NULL;
#define zcfgTr98GpnHandleRoutPick(seqStr) (tr98GpnQueryHandleRoutObj ? (tr98QryHandleRout *)json_object_get_int64(json_object_object_get(tr98GpnQueryHandleRoutObj, seqStr)) : NULL)

//#endif

static json_object *zcfgTr98GpnSpeedObjs = NULL;
static json_object *zcfgTr98GpvSpeedObjs = NULL;

static struct json_object *rpcObjPathNameList = NULL;
static json_object *zcfgTr98AttrList = NULL;

static zcfgRet_t zcfgObjIidToMappingName(uint32_t objnum, objIndex_t *objIid, char *mappingName);
static zcfgRet_t parmListSetObjParamNameAttr(struct json_object **tr98parm, tr98Parameter_t *parmList);

static struct json_object *zcfgTr98SpeedObjsRetrieve(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid);
static struct json_object *zcfgTr98SpeedObjsRetObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid);
static zcfgRet_t zcfgTr98SpeedObjsAddObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid, struct json_object *obj);
static zcfgRet_t zcfgTr98SpeedObjsDeleteObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid);
static zcfgRet_t zcfgTr98SpeedObjsAddParam(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid, const char *parmname, struct json_object *paramObj);

static uint32_t guiFlag1 = 0;

#ifdef ZCFG_APPLY_RPC_METHOD_AND_SUBSEQUENT_ROUT
static zcfgRet_t spvSntDhcpOptSubRout(const char *, objIndex_t *, struct json_object *, struct json_object *);

// it might execute subsequent rout on spv these obj
tr98RpcSubRout_t spvSubseqRoutList[] = {
{e_TR98_SENT_DHCP_OPT, spvSntDhcpOptSubRout},
{0, NULL}
};

#endif

#ifdef ZCFG_TR98_PARAM_GUARD
tr98ParmRout_t parmGurdRoutList[] = {
{e_TR98_QUE, qMgmtQueParmGurd},
{e_TR98_CLS, qMgmtClsParmGurd},
{0, NULL}
};

static struct json_object *zcfgTr98ParmGurdObjs;

zcfgRet_t zcfgTr98ParmGurdInit()
{
	struct json_object *parmGurdObj = NULL;
	zcfg98sn_t seq;
	int igdstrsz, n = 0;
	const char *name, *reqObjName;

	ztrdbg(ZTR_SL_INFO, "%s\n", __FUNCTION__);

	zcfgTr98ParmGurdObjs = json_object_from_file(ZCFG_TR98_PARMGURD);
	if(is_error(zcfgTr98ParmGurdObjs)) {
		ztrdbg(ZTR_SL_INFO, "%s: Fail to Load %s\n", __FUNCTION__, ZCFG_TR98_PARMGURD);
		zcfgTr98ParmGurdObjs = NULL;
		return ZCFG_NOT_FOUND;
	}

	igdstrsz = strlen(TR98_IGD) + 1;

	while(parmGurdRoutList[n].nameSeqNum){
		seq = parmGurdRoutList[n].nameSeqNum;
		seq -= e_TR98_START;
		name = (tr98NameObjList[seq].tr98Obj)->name;
		reqObjName = (strcmp(name, TR98_IGD)) ? (name + igdstrsz) : name;
		ztrdbg(ZTR_SL_INFO, "%s: object name %s\n", __FUNCTION__, reqObjName);
		parmGurdObj = json_object_object_get(zcfgTr98ParmGurdObjs, reqObjName);
		if(!parmGurdObj){
			n++;
			continue;
		}

		json_object_object_add(parmGurdObj, "ParmGurdRout", json_object_new_int64((intptr_t)(parmGurdRoutList[n].rout)));
		n++;
	}

	return ZCFG_SUCCESS;
}

struct json_object *zcfgTr98RetrieveParmGurdObj(const char *objpath)
{
	struct json_object *parmGurdObj = NULL;
	int igdstrsz;
	const char *reqObjName;
	char tr98ClassName[256+1] = {0};

	if(!zcfgTr98ParmGurdObjs || !objpath)
		return NULL;

	if(zcfgFeTr98ClassNameGet((char *)objpath, tr98ClassName) != ZCFG_SUCCESS)
		return NULL;

	igdstrsz = strlen(TR98_IGD) + 1;
	reqObjName = (strcmp((const char *)tr98ClassName, TR98_IGD)) ? (const char *)(tr98ClassName + igdstrsz) : (const char *)tr98ClassName;
	parmGurdObj = json_object_object_get(zcfgTr98ParmGurdObjs, reqObjName);
	if(parmGurdObj)
		ztrdbg(ZTR_SL_INFO, "%s\n", __FUNCTION__);

	return parmGurdObj;
}

static zcfgRet_t zcfgTr98ParmGurdMinMax(struct json_object *spvparamobj, struct json_object *minmax)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	json_type datatype;

	if(!spvparamobj || !minmax)
		return ZCFG_SUCCESS;

	const char *minmaxvalue = json_object_get_string(minmax);
	if(!minmaxvalue || strlen(minmaxvalue) == 0)
		return ZCFG_SUCCESS;

	datatype = json_object_get_type(spvparamobj);
	ztrdbg(ZTR_SL_INFO, "%s: datatype %d\n", __FUNCTION__, (int)datatype);

	switch(datatype){
	case json_type_int:
	{
		int value = json_object_get_int(spvparamobj);
		int min = 0, max = 0;
		sscanf(minmaxvalue, "(%d-%d)", &min, &max);
		ztrdbg(ZTR_SL_INFO, "%s: minmax %d %d\n", __FUNCTION__, min, max);
		ret = (value >= min && value <=max) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
	}
	break;
	case json_type_uint32:
	{
		uint32_t value = (uint32_t)json_object_get_int(spvparamobj);
		uint32_t min = 0, max = 0;
		sscanf(minmaxvalue, "(%u-%u)", &min, &max);
		ztrdbg(ZTR_SL_INFO, "%s: minmax %u %u\n", __FUNCTION__, min, max);
		ret = (value >= min && value <=max) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
	}
	break;
	default: break;
	}

	return ret;
}

static zcfgRet_t zcfgTr98ParmGurdCmp(struct json_object *spvparamobj, const char *value)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	json_type datatype = json_object_get_type(spvparamobj);

	//!spvobj, !value

	switch(datatype){
	case json_type_int:
	{
		int spv = json_object_get_int(spvparamobj);
		int intvalue = atoi(value);
		ret = (spv == intvalue) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
	}
	break;
	case json_type_string:
	{
		const char *spv = json_object_get_string(spvparamobj);
		if(value[0] == '\'' && value[1] == '\''){
			ztrdbg(ZTR_SL_INFO, "%s: empty on acl\n", __FUNCTION__);
			ret =  (!spv || strlen(spv) == 0) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
		}else{
			ret = (!strcmp(spv, value)) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
		}
	}
	break;
	default: break;
	}

	return ret;
}

static zcfgRet_t zcfgTr98ParmGurdACL(struct json_object *spvparamobj, struct json_object *yesacl, struct json_object *noacl)
{
	zcfgRet_t ret, retcmp;
	const char *yes, *no;
	char *buf = NULL;
	char *paramPtr, *paramPos;
	int yeslen, nolen;

	if(!spvparamobj || (!yesacl && !noacl))
		return ZCFG_SUCCESS;

	ztrdbg(ZTR_SL_INFO, "%s\n", __FUNCTION__);

	yes = json_object_get_string(yesacl);
	no = json_object_get_string(noacl);

	if(yes && (yeslen=strlen(yes))>0){
		buf = malloc(yeslen+1);
		strncpy(buf, yes, yeslen);
		*(buf+yeslen) = '\0';
		paramPtr = buf;
		paramPos = strchr(buf, ',');
		while(paramPos){
			*paramPos = '\0';
			retcmp = zcfgTr98ParmGurdCmp(spvparamobj, paramPtr);
			if(retcmp == ZCFG_SUCCESS){
				ret = retcmp;
				goto complete;
			}

			paramPtr = paramPos+1;
			paramPos = strchr(paramPtr, ',');
		}

		if(paramPtr){
			retcmp = zcfgTr98ParmGurdCmp(spvparamobj, paramPtr);
			if(retcmp == ZCFG_SUCCESS){
				ret = retcmp;
				goto complete;
			}
		}
	}
	ZOS_FREE(buf);


	if(no && (nolen=strlen(no))>0){
		buf = malloc(nolen+1);
		strncpy(buf, no, nolen);
		*(buf+nolen) = '\0';
		paramPtr = buf;
		paramPos = strchr(buf, ',');
		while(paramPos){
			*paramPos = '\0';
			retcmp = zcfgTr98ParmGurdCmp(spvparamobj, paramPtr);
			if(retcmp == ZCFG_SUCCESS){
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto complete;
			}

			paramPtr = paramPos+1;
			paramPos = strchr(paramPtr, ',');
		}

		if(paramPtr){
			retcmp = zcfgTr98ParmGurdCmp(spvparamobj, paramPtr);
			if(retcmp == ZCFG_SUCCESS){
				ret = ZCFG_INVALID_PARAM_VALUE;
				goto complete;
			}
		}
	}

	if(yes && !no)
		ret = ZCFG_INVALID_PARAM_VALUE;
	else if(!yes && no)
		ret = ZCFG_SUCCESS;
	else{
		//not yes,no
		ret = ZCFG_NOT_FOUND;
	}

complete:
	ZOS_FREE(buf);

	return ret;
}

zcfgRet_t zcfgTr98ParmGurdScan(const char *objpath, const char *parmname, struct json_object *objParmGurd, struct json_object *spvobj, char **faultString)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramObj;
	struct json_object *yesacl, *noacl;
	const char *attr;
	struct json_object *spvparamobj;

	//!objpath || !parmname || !spvobj

	if(!objParmGurd)
		objParmGurd = zcfgTr98RetrieveParmGurdObj(objpath);

	if(!objParmGurd)
		return ZCFG_SUCCESS;

	ztrdbg(ZTR_SL_INFO, "%s\n", __FUNCTION__);

	paramObj = json_object_object_get(objParmGurd, parmname);
	spvparamobj = json_object_object_get(spvobj, parmname);
	if(!paramObj || !spvparamobj)
		return ZCFG_SUCCESS;

	attr = json_object_get_string(json_object_object_get(paramObj, "attr"));
	if(!attr || strlen(attr) == 0)
		return ZCFG_SUCCESS;

	// it's possible minmax and acl
	if(strstr(attr, "minmax")){
		struct json_object *minmax = json_object_object_get(paramObj, "minmax");
		ret = zcfgTr98ParmGurdMinMax(spvparamobj, minmax);
		if(ret == ZCFG_SUCCESS)
			return ret;

		//add faultString
	}

	if(strstr(attr, "acl")){
		yesacl = json_object_object_get(paramObj, "yes");
		if(yesacl)
			ztrdbg(ZTR_SL_INFO, "%s: %s %s has yes acl\n", __FUNCTION__, objpath, parmname);
		noacl = json_object_object_get(paramObj, "no");
		if(noacl)
			ztrdbg(ZTR_SL_INFO, "%s: %s %s has no acl\n", __FUNCTION__, objpath, parmname);

		ret = zcfgTr98ParmGurdACL(spvparamobj, yesacl, noacl);
		if(ret == ZCFG_SUCCESS)
			return ret;

 		if(ret == ZCFG_INVALID_PARAM_VALUE){
			//add faultString
			return ret;
		}
	}

	if(strstr(attr, "rout")){
		parmGurdRout rout;
		rout = json_object_get_int64(json_object_object_get(objParmGurd, "ParmGurdRout"));
		if(rout != 0){
			ztrdbg(ZTR_SL_INFO, "%s: %s %s has param guard routine\n", __FUNCTION__, objpath, parmname);
			ret = rout(objpath, parmname, spvparamobj, paramObj, faultString);
		}
	}else if(ret != ZCFG_SUCCESS && ret != ZCFG_INVALID_PARAM_VALUE)
		ret = ZCFG_INVALID_PARAM_VALUE;

	return ret;
}
#endif

#ifdef ZCFG_APPLY_MULTIOBJSET_ON_SESSIONCOMPLETE

void zcfgFeSessMultiObjSet()
{

	if(!sessMultiObjSet){
		sessMultiObjSet = json_object_new_object();
	}
}

void zcfgFeSessMultiObjSetClear()
{
	if(sessMultiObjSet){
		json_object_put(sessMultiObjSet);
	}
	sessMultiObjSet = json_object_new_object();
}

struct json_object *zcfgFeSessMultiObjSetRetrieve()
{
	return (sessMultiObjSet) ? json_object_object_get(sessMultiObjSet, "multiobj") : NULL;
}

void zcfgFeSessMultiObjSetSave(struct json_object *multiobj)
{
	if(sessMultiObjSet){
		struct json_object *oldMultiobj = json_object_object_get(sessMultiObjSet, "multiobj");
		struct json_object *newMultiobj = (oldMultiobj == multiobj) ? JSON_OBJ_COPY(multiobj) : multiobj;
		json_object_object_add(sessMultiObjSet, "multiobj", newMultiobj);
	}

}

// utilize session MultiObjSet, and it ought to apply multiobjset on session complete
struct json_object *zcfgFeSessMultiObjSetUtilize(struct json_object *multiobj)
{
#if 0
	struct json_object *multiObjSet = zcfgFeSessMultiObjSetRetrieve();
	if(!multiObjSet){
		multiObjSet = json_object_new_object();
		sessMultiObjSet = multiObjSet;
	}
	json_object_object_add(sessMultiObjSet, "applyonsesscomp", json_object_new_boolean(true));

	json_object_put(multiobj);
	return sessMultiObjSet;
#endif
	json_object_object_add(multiobj, "applyonsesscomp", json_object_new_boolean(true));

	return multiobj;
}

void zcfgFeJsonMultiObjSetParamk(const char *paramK)
{
	if(sessMultiObjSet && paramK && strlen(paramK))
	{
		json_object_object_add(sessMultiObjSet, "notobjpathbutparameterkey", json_object_new_string(paramK));
	}
}

struct json_object *zcfgFeJsonMultiObjParamkRetrieve()
{
	if(sessMultiObjSet)
	{
		return json_object_object_get(sessMultiObjSet, "notobjpathbutparameterkey");
	}
	return NULL;
}
#endif //ZCFG_APPLY_MULTIOBJSET_ON_SESSIONCOMPLETE

#ifdef ZCFG_RUNNING_WANDEV_INDEX_CUSTOMIZATION
#if 0
zcfgRet_t ReplaceWANDeviceIndex(char* pathname)
{	
	objIndex_t objIid;
	rdm_MgmtSrv_t *MgmtSrvObj = NULL;
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;
	char *ptr1 = NULL, *ptr2 = NULL;
	char tr69WANpath[128] = {0};

	
	printf("%s : Enter\n", __FUNCTION__);
	printf("pathname=%s\n", pathname);

	IID_INIT(objIid);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, (void **)&MgmtSrvObj) == ZCFG_SUCCESS) {
		printf("tr69WanIPName=%s\n", MgmtSrvObj->X_ZYXEL_ActiveBoundInterface);
		if (strlen(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface) > 0){
			if(zcfgFe181To98MappingNameGet(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface, tr69WANpath) == ZCFG_SUCCESS)
				ret = ZCFG_SUCCESS;
		}
		zcfgFeObjStructFree(MgmtSrvObj);
	}

	if (ret == ZCFG_SUCCESS){
		ret = ZCFG_INTERNAL_ERROR;
		if (ptr1 = strstr(tr69WANpath, "WANDevice.")){
			if (ptr2 = strstr(pathname, "WANDevice.")){
				*(ptr2+10) = *(ptr1+10);
				ret = ZCFG_SUCCESS;
			}
		}
	}

	printf("ret= %d, new pathname=%s\n", ret, pathname);
	
	return ret;	
}
#endif

zcfgRet_t zcfgFeTr98StaticWANDevSync(void *obj)
{
#if 0
	objIndex_t objIid;
	rdm_MgmtSrv_t *MgmtSrvObj = NULL;

	IID_INIT(objIid);
	if(zcfgFeObjStructGetWithoutUpdate(RDM_OID_MGMT_SRV, &objIid, (void **)&MgmtSrvObj) == ZCFG_SUCCESS) {
		ztrdbg(ZTR_DL_DEBUG, "%s: tr69WanIPName=%s\n", __FUNCTION__, MgmtSrvObj->X_ZYXEL_ActiveBoundInterface);
		if (strlen(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface) > 0){
			ret = zcfgFe181To98MappingNameGet(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface, tr98WANInterface);
		}
		zcfgFeObjStructFree(MgmtSrvObj);
	}
#endif
	const char *wanDeviceName = NULL;
	int wanDeviceInstance = 0;
	rdm_MgmtSrv_t *MgmtSrvObj = (rdm_MgmtSrv_t *)obj;
	char tr98WANInterface[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;

	if (MgmtSrvObj && strlen(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface) > 0){
		ret = zcfgFe181To98MappingNameGet(MgmtSrvObj->X_ZYXEL_ActiveBoundInterface, tr98WANInterface);
	}

	if ((ret ==  ZCFG_SUCCESS) &&  (wanDeviceName = strstr(tr98WANInterface, "WANDevice") ) && strlen(wanDeviceName)) {
		sscanf(wanDeviceName, "WANDevice.%d", &wanDeviceInstance);
		//ztrdbg(ZTR_DL_DEBUG, "%s: tr98WANInterface=%s\n", __FUNCTION__, tr98WANInterface);
	}

	wanDeviceInstanceNum = wanDeviceInstance;

	return ZCFG_SUCCESS;
}
#endif

#ifdef CONFIG_TAAAB_DSL_BINDING
void WANName( void )
{
     objIndex_t ethIfaceIid;	
	 objIndex_t dslChannelIid;
	 struct json_object *ethIfaceObj = NULL;
	 struct json_object *dslChannelObj = NULL;
	 const char *ethType;
	 const char *ethStatus;
	 const char *dslType;
	 const char *dslStatus;
     
	 IID_INIT(ethIfaceIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_ETH_IFACE, &ethIfaceIid, &ethIfaceObj) == ZCFG_SUCCESS) {
		 ethType = json_object_get_string(json_object_object_get(ethIfaceObj, "X_ZYXEL_LanPort"));
		 ethStatus = json_object_get_string(json_object_object_get(ethIfaceObj, "Status"));
		 if(!strcmp(ethType,"ETHWAN") && !strcmp(ethStatus,"Up")){
		 	WanConnectType = 1;
			zcfgFeJsonObjFree(ethIfaceObj);
			break;
		 }
		 else
		 	zcfgFeJsonObjFree(ethIfaceObj);
	 }
	 
	 IID_INIT(dslChannelIid);
	 while(zcfgFeObjJsonGetNext(RDM_OID_DSL_CHANNEL, &dslChannelIid, &dslChannelObj) == ZCFG_SUCCESS) {
		 dslType = json_object_get_string(json_object_object_get(dslChannelObj, "LinkEncapsulationUsed"));
		 dslStatus = json_object_get_string(json_object_object_get(dslChannelObj, "Status"));
		 if(strstr(dslType,"ATM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 2;
			zcfgFeJsonObjFree(dslChannelObj);
			break;
		 }
		 else if(strstr(dslType,"PTM") && !strcmp(dslStatus,"Up")){
		 	WanConnectType = 3;
			zcfgFeJsonObjFree(dslChannelObj);
			break;
		 }
		 else
		 	zcfgFeJsonObjFree(ethIfaceObj);		 
	 }

}
#endif

zcfgRet_t ReplaceWANDeviceIndex(const char *tr98ObjName, char* pathname)
{
#ifndef ZCFG_RUNNING_WANDEV_INDEX_CUSTOMIZATION
#ifdef CONFIG_TAAAB_DSL_BINDING
   char *search = NULL;
   char *oriindex = NULL;
   zcfgRet_t ret = ZCFG_SUCCESS;	  

   strcpy(pathname, tr98ObjName);
   if(!strncmp(pathname,"InternetGatewayDevice.WANDevice.",32) && strlen(pathname)>=33)
   {
     search = strstr(pathname, "InternetGatewayDevice.WANDevice.");
     oriindex = search + 32;

     if(*oriindex == '1'){   //DSL
	    if(WanConnectType == 1 || WanConnectType == 3 )
		    *(search + 32) = '2';
	    else if(WanConnectType == 2)
		    *(search + 32) = '1';
     }
     else if(*oriindex == '2'){
	     *(search + 32) = '3';
     }
     else
	     ret = ZCFG_NO_SUCH_OBJECT;
   }

   return ret;

#else
	return ZCFG_SUCCESS;
#endif
#else
	int wanDeviceInstance =  0;
	int wanDevPathSize = 0;
	const char *wanDeviceName = NULL;
	bool staticWanDev = zcfgFe98StaticWanDevStatus();

	if(staticWanDev != true)
		return ZCFG_SUCCESS;

	wanDeviceName = strstr(tr98ObjName, "WANDevice.1");
	if(!wanDeviceName || strstr(tr98ObjName, "X_ZYXEL_ActiveWANDevice.1"))
		return ZCFG_SUCCESS;

	wanDevPathSize = strlen(TR98_WAN_DEV);
	if(_isdigit(tr98ObjName[wanDevPathSize]))
		return ZCFG_NO_SUCH_OBJECT;

	wanDeviceInstance = wanDeviceInstanceNum;
	if(!wanDeviceInstance || wanDeviceInstance >= 10)
		return ZCFG_NO_SUCH_OBJECT;

	strcpy(pathname, tr98ObjName);
	pathname[wanDevPathSize-1] = (char)(0x30+wanDeviceInstance);
	//printf("%s: modified=%s\n", __FUNCTION__, pathname);

	return ZCFG_SUCCESS;
#endif
}

zcfgRet_t ReplaceStaticWANDeviceIndex(const char *tr98ObjName, char *pathname)
{
#ifndef ZCFG_RUNNING_WANDEV_INDEX_CUSTOMIZATION
#ifdef CONFIG_TAAAB_DSL_BINDING 
	char *search = NULL;
    char *oriindex = NULL;
    zcfgRet_t ret = ZCFG_SUCCESS;    

    strcpy(pathname, tr98ObjName);
	if(!strncmp(pathname,"InternetGatewayDevice.WANDevice.",32) && strlen(pathname)>=33)
	{
	  search = strstr(pathname, "InternetGatewayDevice.WANDevice.");
	  oriindex = search + 32;

	  if(*oriindex == '1' || *oriindex == '2' ){   //DSL
		 	 *(search + 32) = '1';
	  }
	  else if(*oriindex == '3'){       //ETH
		 *(search + 32) = '2';
	  }
	}

Done:
	return ret;

#else
	return ZCFG_SUCCESS;
#endif
#else
	int wanDeviceInstance =  0;
	int wanDevPathSize = 0;

	//if(!tr98ObjName || !pathname)

	//printf("%s: tr98ObjName=%s\n", __FUNCTION__, tr98ObjName);

	//memset(pathname, 0, TR98_MAX_OBJ_NAME_LENGTH);
	bool staticWanDev = zcfgFe98StaticWanDevStatus();

	if(staticWanDev != true)
		return ZCFG_SUCCESS;

	wanDevPathSize = strlen(TR98_WAN_DEV);
	if (!strstr(tr98ObjName, "e.WANDevice.") || (strlen(tr98ObjName) < wanDevPathSize))
		return ZCFG_SUCCESS;

	if ( !_isdigit(tr98ObjName[wanDevPathSize-1]) || tr98ObjName[wanDevPathSize] != '.')
		return ZCFG_NOT_INSTANCE;

	wanDeviceInstance = wanDeviceInstanceNum;
	if(!wanDeviceInstance)
		return ZCFG_NO_SUCH_OBJECT;

	strcpy(pathname, tr98ObjName);
	if((int)pathname[wanDevPathSize-1] == (0x30 + wanDeviceInstance))
		pathname[wanDevPathSize-1] = '1';
	//printf("%s: modified=%s\n", __FUNCTION__, pathname);

	return ZCFG_SUCCESS;
#endif
}

zcfgRet_t zcfgFeTr98AllIGDNameGet()
{
#if 0
	zcfgRet_t ret;

	if(tr98_names_array != NULL)
		json_object_put(tr98_names_array);

	tr98_names_array = NULL;

	if((ret = zcfgFeAllIGDNameGet()) != ZCFG_SUCCESS) {
		printf("%s : Error. ret %d\n", __FUNCTION__, ret);
		return ret;
	}

	tr98_names_array = json_object_from_file("/tmp/ALL_IGD");

	return ret;
#else
	return zcfgFeAllIGDNameGet();
#endif
}

void zcfgSetRunningDataModelSpec(const char *dataModelSpec)
{
	runningDataModel = dataModelSpec ? ( !strcmp(dataModelSpec, "TR-098") ? DATA_MODEL_TR98 :
			(!strcmp(dataModelSpec, "TR-181") ? DATA_MODEL_TR181 : DATA_MODEL_NOTDEFINED) )
			: DATA_MODEL_NOTDEFINED;
}

static zcfgRet_t zcfgFeTr98ClassNameGet(char *objName, char *className)
{
	char *token = NULL;
	char *tmpName = NULL;
	uint16_t len = 0, c = 0;

	if(objName == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "Object Name is NULL\n");
		return ZCFG_INVALID_OBJECT;
	}

	tmpName = (char *)malloc(strlen(objName)+1);
	strcpy(tmpName, objName);

	if(*(tmpName + strlen(tmpName) - 1) == '.') {
		ztrdbg(ZTR_DL_ERR,  "%s : Invalid object Name %s ended with '.'\n", __FUNCTION__, objName);
		ZOS_FREE(tmpName);
		return ZCFG_INVALID_OBJECT;
	}

	token = strtok(tmpName, ".");
	/*InternetGatewayDevice*/
	strcpy(className, token);
	token = strtok(NULL, ".");

	/* Replace number with "i" */
	while(token != NULL) {
		strcat(className, ".");

		len = strlen(token);
		for(c = 0; c < len; c++) {
			if(!isdigit(*(token + c)))
				break;
		}

		if(len == c)
			strcat(className, "i");
		else
			strcat(className, token);

		token = strtok(NULL, ".");
	}

	ZOS_FREE(tmpName);
	
	return ZCFG_SUCCESS;
}

#if 0
static int zcfgFeTr98HandlerGet(char *tr98FullPathName)
{
	static int last = 0;
	int c = 0;

	c = last;

	do {
		if(tr98Obj[c].name == NULL) {
			c = 0;

			if(c == last)
				break;
		}

		if(!strcmp(tr98Obj[c].name, tr98FullPathName)) {
			last = c;
			return c;
		}

		c++;
	} while(c != last);

	return -1;
}
#endif

static int zcfgFeTr98HandlerGet(char *tr98FullPathName)
{
	tr98NameObj_t *entry = NULL;
	tr98Object_t *tr98Obj = NULL;
	int seqnum = 0;
	char pathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	if(!tr98FullPathName)
		return -1;

	entry = (tr98NameObj_t *)json_object_object_get(tr98StaticNameTable, (const char *)tr98FullPathName);
	if(!entry)
		return -1;

	seqnum = (int)entry->nameSeqNum;
	if(!IS_SEQNUM_RIGHT(seqnum)) {
		zcfgLog(ZCFG_LOG_ERR, "%s: wrong sequence num\n", __FUNCTION__);
		return -1;
	}

	tr98Obj = entry->tr98Obj;

#ifdef ZYXEL_OPAL_EXTENDER
	if (tr98Obj->attr & OBJECT_ATTR_EXTENDER_HIDDEN)
	{
	    return -1;
	}
#endif
#ifdef ZYXEL_OPAL_BRIDGE_ACSPARAMETERS_HIDDEN
	if (tr98Obj->attr & OBJECT_ATTR_EXTENDER_HIDDEN)
	{
	    return -1;
	}
	if (tr98Obj->name == TR98_LAN_DEV_WLAN_CFG)
	{
		return -1;
	}
#endif
	strcpy(pathName, tr98Obj->name);
	if(strcmp(tr98FullPathName, pathName)) {
		zcfgLog(ZCFG_LOG_ERR, "%s: wrong tr98Obj entry\n", __FUNCTION__);
		return -1;
	}

	return ARRAY_SEQNUM(seqnum);
}

zcfgRet_t zcfgFeTr181IfaceStackLowerLayerGet(char *higherLayer, char *result)
{
	struct json_object *resultObj, *jobj, *jobj_lo;
	int i,length;

	/* replace RDM_OID_IFACE_STACK loop search */
	if ( zcfgFeIfStackObjJsonGet(higherLayer, 1, &resultObj) == ZCFG_SUCCESS ) {
		length = json_object_array_length(resultObj);
		for ( i = 0; i<length; i ++ ) {
			jobj = json_object_array_get_idx(resultObj, i);
			jobj_lo = json_object_object_get(jobj,"LowerLayer");
			if ( jobj_lo ) {
				strcpy(result, json_object_get_string(jobj_lo));
				/* TODO: Just find the first one?. It will depends on the order of
						 interface configuration */
				json_object_put(resultObj);
				return ZCFG_SUCCESS;
			}
		}
		json_object_put(resultObj);
	}
	return ZCFG_NO_SUCH_OBJECT;
}

zcfgRet_t zcfgFeTr181IfaceStackHigherLayerGet(char *lowerLayer, char *result)
{
	struct json_object *resultObj, *jobj, *jobj_hi;
	int i,length;

	/* replace RDM_OID_IFACE_STACK loop search */
	if ( zcfgFeIfStackObjJsonGet(lowerLayer, 0, &resultObj) == ZCFG_SUCCESS ) {
		length = json_object_array_length(resultObj);
		for ( i = 0; i<length; i ++ ) {
			jobj = json_object_array_get_idx(resultObj, i);
			jobj_hi = json_object_object_get(jobj,"HigherLayer");
			if ( jobj_hi ) {
				strcpy(result, json_object_get_string(jobj_hi));
				/* TODO: Just find the first one?. It will depends on the order of
						 interface configuration */
				json_object_put(resultObj);
				return ZCFG_SUCCESS;
			}
		}
		json_object_put(resultObj);
	}
	return ZCFG_NO_SUCH_OBJECT;
}

zcfgRet_t zcfgFe98To181IpIface(const char *tr98IfaceName, char *ipIface)
{
	char tr98ObjName[300] = {0}, tr181ObjName[300] = {0};
	char higherLayer[300] = {0};

	if(!tr98IfaceName || !ipIface)
		return ZCFG_INTERNAL_ERROR;

	ipIface[0] = '\0';

	if(!strcmp(tr98IfaceName, "")) {
		return ZCFG_SUCCESS;
	}

	ZOS_STRNCPY(tr98ObjName, tr98IfaceName, sizeof(tr98ObjName));
	if(strcmp(tr98ObjName, "") && (zcfgFe98To181MappingNameGet(tr98ObjName, tr181ObjName) != ZCFG_SUCCESS)) {
		return ZCFG_NO_SUCH_OBJECT;
	}

	if(strstr(tr181ObjName, "PPP.Interface") != NULL) {
		if((zcfgFeTr181IfaceStackHigherLayerGet(tr181ObjName, higherLayer)) != ZCFG_SUCCESS) {
			return ZCFG_NO_SUCH_OBJECT;
		}
		ZOS_STRNCPY(tr181ObjName, higherLayer, sizeof(tr98ObjName));
					
		if(!strstr(tr181ObjName, "IP.Interface")) {
			return ZCFG_NO_SUCH_OBJECT;
		}
	}

	if(!strstr(tr181ObjName, "IP.Interface")) {
		return ZCFG_NO_SUCH_OBJECT;
	}

	//strncpy(ipIface, tr181ObjName, sizeof(tr181ObjName)-1);  //the size of ipIface is unknown, don't use strncpy
	strcpy(ipIface, tr181ObjName);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr181IpIfaceRunning(const char *iface, char *paramfault)
{
	objIndex_t ipIfIid, addrIid;
	struct json_object *ipv4AddrObj;
	const char *status;

	if(!iface || strlen(iface) == 0)
		return ZCFG_EMPTY_OBJECT;

	IID_INIT(ipIfIid);
	if(sscanf(iface, "IP.Interface.%hhu", &ipIfIid.idx[0]) != 1){
		if(paramfault) sprintf(paramfault, "%s not correct!", iface ? iface : "Interface");
		return ZCFG_NO_SUCH_OBJECT;
	}

	ipIfIid.level = 1;
	IID_INIT(addrIid);
	if(zcfgFeSubInObjJsonGetNext(RDM_OID_IP_IFACE_V4_ADDR, &ipIfIid, &addrIid, &ipv4AddrObj) != ZCFG_SUCCESS) {
		if(paramfault) sprintf(paramfault, "Interface not running!");
		return ZCFG_INVALID_PARAM_VALUE;
	}

	if( (json_object_get_boolean(json_object_object_get(ipv4AddrObj, "Enable")) != true) ||
	!(status = json_object_get_string(json_object_object_get(ipv4AddrObj, "Status"))) ||
	 strcmp(status, "Enabled") ) {
		json_object_put(ipv4AddrObj);
		if(paramfault) sprintf(paramfault, "Interface not running!");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	json_object_put(ipv4AddrObj);

	return ZCFG_SUCCESS;
}

#define MAX_TR181_WANIPIFACE_PARAM_LEN 1024
#define MAX_TR98_WANIPIFACE_PARAM_LEN 1500

char *translateWanIpIface(int dataTransType, const char *wanIpIfaces)
{
	char *wanIfacesTok = NULL, *transWanIface = NULL;
	char mappingPathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *token, *tmp = NULL;
	int wanIfacesTok_Len = 0, transWanIface_Len = 0;
	int hasMapping, len = 0;
#ifdef CONFIG_TAAAB_DSL_BINDING
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tmpTr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[MAX_TR181_PATHNAME] = {0};
#endif

	if(!wanIpIfaces || !strlen(wanIpIfaces) ||
		 ((dataTransType != TRANS_WANIPIFACE_98TO181) && (dataTransType != TRANS_WANIPIFACE_181TO98)))
		return NULL;

	wanIfacesTok_Len = (dataTransType == TRANS_WANIPIFACE_181TO98) ? MAX_TR181_WANIPIFACE_PARAM_LEN : MAX_TR98_WANIPIFACE_PARAM_LEN;
	wanIfacesTok = (char *)malloc(wanIfacesTok_Len);
	ZOS_STRNCPY(wanIfacesTok, wanIpIfaces, wanIfacesTok_Len);

	transWanIface_Len = (dataTransType == TRANS_WANIPIFACE_181TO98) ? MAX_TR98_WANIPIFACE_PARAM_LEN : MAX_TR181_WANIPIFACE_PARAM_LEN;
	transWanIface = (char *)malloc(transWanIface_Len);
	memset(transWanIface, 0, transWanIface_Len);

	token = strtok_r(wanIfacesTok, ",", &tmp);
	while(token) {
		len = strlen(token);
		if(token[len-1] == '.')
			token[len-1] = '\0';
		memset(mappingPathName, 0, sizeof(mappingPathName));
		hasMapping = 0;
#ifdef CONFIG_TAAAB_DSL_BINDING
        if((dataTransType == TRANS_WANIPIFACE_181TO98) && strstr(token, "IP.Interface")){
		zcfgFe181To98MappingNameGet(token, tmpTr98ObjName);
		strcpy(tr98ObjName, tmpTr98ObjName);
		ReplaceStaticWANDeviceIndex(tmpTr98ObjName, tr98ObjName);
		strcpy(mappingPathName,tr98ObjName);
		hasMapping = 1;
        }

        if((dataTransType == TRANS_WANIPIFACE_98TO181) && strstr(token, "IPConnection")){
        strcpy(tmpTr98ObjName, token);
		ReplaceWANDeviceIndex(token, tmpTr98ObjName);
		zcfgFe98To181MappingNameGet(tmpTr98ObjName, mappingPathName);
		hasMapping = 1;
        }

        if((dataTransType == TRANS_WANIPIFACE_98TO181) && strstr(token, "PPPConnection")){
        strcpy(tmpTr98ObjName, token);
		ReplaceWANDeviceIndex(token, tmpTr98ObjName);
		zcfgFe98To181MappingNameGet(tmpTr98ObjName, mappingPathName);
		char higherLayer[MAX_TR181_PATHNAME] = {0};
			if(strstr(mappingPathName, "PPP.Interface") && (zcfgFeTr181IfaceStackHigherLayerGet(mappingPathName, higherLayer) == ZCFG_SUCCESS)
				&& strstr(higherLayer, "IP.Interface")) {
				strncpy(mappingPathName, higherLayer, sizeof(mappingPathName)-1);
				hasMapping = 1;
			}
        }	

#else
		if( ((dataTransType == TRANS_WANIPIFACE_181TO98) && strstr(token, "IP.Interface") &&
                                  (zcfgFe181To98MappingNameGet(token, mappingPathName) == ZCFG_SUCCESS)) ||
			((dataTransType == TRANS_WANIPIFACE_98TO181) && strstr(token, "IPConnection") &&
                                  (zcfgFe98To181MappingNameGet(token, mappingPathName) == ZCFG_SUCCESS)) ) {
			hasMapping = 1;
		}
		else if( (dataTransType == TRANS_WANIPIFACE_98TO181) && strstr(token, "PPPConnection") &&
                                  (zcfgFe98To181MappingNameGet(token, mappingPathName) == ZCFG_SUCCESS) ) {
			char higherLayer[MAX_TR181_PATHNAME] = {0};
			if(strstr(mappingPathName, "PPP.Interface") && (zcfgFeTr181IfaceStackHigherLayerGet(mappingPathName, higherLayer) == ZCFG_SUCCESS)
				&& strstr(higherLayer, "IP.Interface")) {
				ZOS_STRNCPY(mappingPathName, higherLayer, sizeof(mappingPathName));
				hasMapping = 1;
			}
		}
#endif
		if(hasMapping){
			if(strlen(transWanIface)) {
				strcat(transWanIface, ",");
				strcat(transWanIface, mappingPathName);
			}
			else
				strcpy(transWanIface, mappingPathName);
		}
		token = strtok_r(NULL, ",", &tmp);
	}

	if(!strlen(transWanIface)) {
		ZOS_FREE(wanIfacesTok);
		ZOS_FREE(transWanIface);
		return NULL;
	}

	ZOS_FREE(wanIfacesTok);

	return transWanIface;
}

#define setLinkObjParm(linkObj, value) { \
	struct json_object *obj = (struct json_object *)(linkObj); \
	bool paramValue = value; \
	json_object_object_add(obj, "Enable", json_object_new_boolean(paramValue)); }

#define relObj(obj) {json_object_put((struct json_object *)(obj));}
#define TR181_STATUS_UP "Up"

/*
 * zcfgFeTr181WanSetLink is generic interface object enable/disable routine
 *
 * wanLinkName: interface stack to enable/disable, 
 *            as DSL.Channel.i, IP.Interface.i, Ethernet.Link.i, ATM.Link.i, PTM.Link.i
 * set: true is enable, false is disable
 * retObj: means it could return retrieved object, once !=0
 * retObjIndex: means it could return object index, once !=0
 * applied: return whether demand is applied
 *
 * Return: 'Object Identity' it is applied while not zero
 */
zcfg_offset_t zcfgFeTr181WanSetLink(char *wanLinkName, bool set, void **retObj, objIndex_t *retObjIndex, int *applied)
{
	objIndex_t *linkIid = NULL;
	void *linkObj = NULL;
	zcfg_offset_t linkOid = 0;
	const char *paramStr = NULL;
	bool linkParmStatus;
	char hLIntfName[100];

	if(!applied) {
		zcfgLog(ZCFG_LOG_INFO, "%s : no 'applied' parameter\n", __FUNCTION__);
		return 0;
	}

	if(retObj) *retObj = NULL;

	*applied = 0;

	if(retObjIndex) {
		linkIid = retObjIndex;
	}
	else linkIid = (objIndex_t *)malloc(sizeof(objIndex_t));

	memset(linkIid, 0, sizeof(objIndex_t));

	if(strstr(wanLinkName, "DSL.Channel") || strstr(wanLinkName, "Ethernet.Link")) {
		memset(hLIntfName, 0, sizeof(hLIntfName));
		if(zcfgFeTr181IfaceStackHigherLayerGet(wanLinkName, hLIntfName) ==  ZCFG_SUCCESS) {
			wanLinkName = hLIntfName;
		}
		else{
			ZOS_FREE(linkIid);
			return 0;
		} 			
	}
	else {
		//might recognize another
	}

	if(strstr(wanLinkName, "IP.Interface")) {
		sscanf(wanLinkName, "IP.Interface.%hhu", &linkIid->idx[0]);
		linkOid = RDM_OID_IP_IFACE;
		zcfgLog(ZCFG_LOG_INFO, "%s : set IP.Interface.%d\n", __FUNCTION__, linkIid->idx[0]);
	}
	else if(strstr(wanLinkName, "Ethernet.Link")) {
		sscanf(wanLinkName, "Ethernet.Link.%hhu", &linkIid->idx[0]);
		linkOid = RDM_OID_ETH_LINK;
		zcfgLog(ZCFG_LOG_INFO, "%s : set Ethernet.Link.%d\n", __FUNCTION__, linkIid->idx[0]);
	}
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	else if(strstr(wanLinkName, "ATM.Link")) {
		sscanf(wanLinkName, "ATM.Link.%hhu", &linkIid->idx[0]);
		linkOid = RDM_OID_ATM_LINK;
		zcfgLog(ZCFG_LOG_INFO, "%s : set ATM.Link.%d\n", __FUNCTION__, linkIid->idx[0]);
	}
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	else if(strstr(wanLinkName, "PTM.Link")) {
		sscanf(wanLinkName, "PTM.Link.%hhu", &linkIid->idx[0]);
		linkOid = RDM_OID_PTM_LINK;
		zcfgLog(ZCFG_LOG_INFO, "%s : set PTM.Link.%d\n", __FUNCTION__, linkIid->idx[0]);
	}
#endif
	else {
		zcfgLog(ZCFG_LOG_NOTICE, "%s : unknown %s\n", __func__, wanLinkName); 
		//might recognize another
		ZOS_FREE(linkIid);
		return 0;
	}
	linkIid->level = 1;

	//if(zcfgFeObjStructGet(linkOid, linkIid, (void **)&linkObj) != ZCFG_SUCCESS) {
	if(zcfgFeObjJsonGet(linkOid, linkIid, (struct json_object **)&linkObj) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_ERR, "%s : Get Link Fail\n", __FUNCTION__);
		if(!retObjIndex){
			ZOS_FREE(linkIid);
		}

		return 0;
	}

	paramStr = json_object_get_string(json_object_object_get((struct json_object *)linkObj, "Status"));
	linkParmStatus = (strstr(paramStr, TR181_STATUS_UP)) ? true : false;
	ztrdbg(ZTR_DL_DEBUG, "%s: set %s %d, %d\n", __FUNCTION__, wanLinkName, linkParmStatus, set);
	
	if(linkParmStatus == set) {
		if(retObj) *retObj = linkObj;
		else relObj(linkObj);

		if(!retObjIndex) {
			ZOS_FREE(linkIid);
		}

	}
	else {
		zcfgRet_t ret = ZCFG_SUCCESS;
		setLinkObjParm(linkObj, set);
		//ret = zcfgFeObjStructSet(linkOid, linkIid, (void *)linkObj, NULL);
		ret = zcfgFeObjJsonSet(linkOid, linkIid, (struct json_object *)linkObj, NULL);
		if((ret != ZCFG_SUCCESS) && (ret != ZCFG_DELAY_APPLY)) {
			zcfgLog(ZCFG_LOG_ERR, "%s : Set %s fail\n", __FUNCTION__, wanLinkName);
			relObj(linkObj);
			linkOid = 0;
		}
		else {
			if(retObj) *retObj = linkObj;
			else relObj(linkObj);
			
			*applied = 1;
			zcfgLog(ZCFG_LOG_INFO, "%s : Set %s complete\n", __FUNCTION__, wanLinkName);
		}
		if(!retObjIndex) {
			ZOS_FREE(linkIid);
		}
	}

	return linkOid;
}

void zcfgRpcSpeedObjsRpcObjectListInit()
{
	if(rpcObjPathNameList){
		json_object_put(rpcObjPathNameList);
	}
	rpcObjPathNameList = json_object_new_object();
}

void zcfgRpcSpeedObjsAddRpcObject(const char *rpcObjPathName)
{
	json_object_object_add(rpcObjPathNameList, rpcObjPathName, json_object_new_int(1));

}

static int zcfgRpcSpeedObjsAskRpcObject(const char *objPathName)
{
	int rpcObj = 0;

	if(rpcObjPathNameList){

		rpcObj = !json_object_object_get(rpcObjPathNameList, objPathName) ? 0 : 1;
	}

	return rpcObj;
}


static zcfgRet_t zcfgFeTr98ObjQueryAndSpeed(char *tr98ObjName, struct json_object **value, int enableSaveSpeedObj)
{
	zcfgRet_t rst;
	char tr98ClassName[256+1] = {0};
	int handler;

	if((rst = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS)
		return rst;

	handler = zcfgFeTr98HandlerGet(tr98ClassName);

	if(handler == -1)
		return ZCFG_INVALID_OBJECT;

	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;

	const char *tr98ObjNamePtr = (newtr98ObjName[0] != '\0') ? (const char *)newtr98ObjName : (const char *)tr98ObjName;

	zcfg_name_t seqnum;
	objIndex_t tr98ObjIid;
	IID_INIT(tr98ObjIid);
	if((rst = zcfgFe98NameToSeqnum(tr98ObjNamePtr, &seqnum, &tr98ObjIid)) != ZCFG_SUCCESS){
		return ZCFG_INVALID_OBJECT;
	}

	int rpcAppliedObj = zcfgRpcSpeedObjsAskRpcObject(tr98ObjName);

	//printf("%s: seqnum %u, %hhu.%hhu.%hhu\n", __FUNCTION__, seqnum, tr98ObjIid.idx[0], tr98ObjIid.idx[0], tr98ObjIid.idx[0]);
	if(zcfgTr98GpvSpeedObjs){
		struct json_object *speedObj = (rpcAppliedObj) ? zcfgTr98SpeedObjsRetrieve(zcfgTr98GpvSpeedObjs, seqnum, &tr98ObjIid) :
				zcfgTr98SpeedObjsRetObj(zcfgTr98GpvSpeedObjs, seqnum, &tr98ObjIid);
		if(speedObj){
			ztrdbg(ZTR_DL_INFO, "%s: speedobj %s retrieved\n", __FUNCTION__, tr98ObjName);
			*value = rpcAppliedObj ? JSON_OBJ_COPY(speedObj) : speedObj;

			return ZCFG_SUCCESS;
		}
	}

	if(tr98Obj[handler].getObj){
		struct json_object *queryObj = NULL;
		rst = tr98Obj[handler].getObj(tr98ObjNamePtr, handler, &queryObj, TR98_GET_UPDATE);
		//printf("%s: %s   query handle rout, rst=%d\n", __FUNCTION__, tr98ObjNamePtr, rst);
		enableSaveSpeedObj = !rpcAppliedObj ? enableSaveSpeedObj : 1;
		if((rst == ZCFG_SUCCESS) && zcfgTr98GpvSpeedObjs && enableSaveSpeedObj){
			zcfgTr98SpeedObjsAddObj(zcfgTr98GpvSpeedObjs, seqnum, &tr98ObjIid, queryObj);
		}
		//*value = queryObj;
		*value = rpcAppliedObj ? JSON_OBJ_COPY(queryObj) : queryObj;

		return rst;
	}
	else
		return ZCFG_OBJECT_WITHOUT_PARAMETER;

}

zcfgRet_t zcfgFeTr98ObjGet(char *tr98ObjName, struct json_object **value)
{
	zcfgRet_t ret;
	char tr98ClassName[257] = {0};
	int handler = 0;

	ztrdbg(ZTR_DL_DEBUG,  "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);

	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;
		if(tr98Obj[handler].getObj != NULL && newtr98ObjName[0] != '\0')
			return tr98Obj[handler].getObj(newtr98ObjName, handler, value, TR98_GET_UPDATE);
		else if(tr98Obj[handler].getObj != NULL)
			return tr98Obj[handler].getObj(tr98ObjName, handler, value, TR98_GET_UPDATE);
		else
			return ZCFG_OBJECT_WITHOUT_PARAMETER;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}

	return ZCFG_SUCCESS;
}

struct json_object *zcfgFe98ParamGuardInit(const char *parmList, void *addition)
{
	if(!parmList)
		return NULL;

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	if(addition)
		ztrdbg(ZTR_DL_DEBUG,  "%s: has addition\n", __FUNCTION__);

	struct json_object *parmListObj = json_tokener_parse((const char *)parmList);
	if(!parmListObj) {
		ztrdbg(ZTR_DL_ERR,  "%s: parmList object build fail: %s\n", __FUNCTION__, parmList);
		return NULL;
	}

	return parmListObj;
}

zcfgRet_t zcfgFe98ParamGuard(struct json_object *parmList, const char *parmName, struct json_object *param)
{
	struct json_object *paramValueObjs, *paramValueObj = NULL;
	bool secured = false;
	enum json_type parmtype = json_type_null;
	const char *paramString = NULL;

	if(!parmList || !parmName)
		return ZCFG_INTERNAL_ERROR;

	if(!param)
		return ZCFG_NO_SUCH_PARAMETER;

	parmtype = json_object_get_type(param);
	if(parmtype == json_type_string)
		paramString = json_object_get_string(param);


	paramValueObjs = json_object_object_get(parmList, parmName);
	if(!paramValueObjs)
		return ZCFG_NO_SUCH_PARAMETER;

	if((!paramString || !strlen(paramString)) && json_object_object_get(paramValueObjs, "Emp"))
		secured = true;
	else if(paramString && strlen(paramString)) {
		paramValueObj = json_object_object_get(paramValueObjs, paramString);
		secured = (paramValueObj) ? true : false;
		if(paramValueObj)
			ztrdbg(ZTR_DL_DEBUG,  "%s: has parameter %s %s\n", __FUNCTION__, parmName, paramString);
	}

	return (secured == true) ? ZCFG_SUCCESS : ZCFG_INVALID_PARAM_VALUE;
}

void zcfgFe98ParamGuardRelease(struct json_object *parmList)
{
	json_object_put(parmList);
}

// enableSaveSpeedObj: 1, would save queried obj to 'speed objects' while 'GpvSpeedObjs' utilized
zcfgRet_t zcfgFeTr98ObjSpvGet(const char *tr98ObjName, struct json_object **value, int enableSaveSpeedObj)
{
	struct json_object *tr98SpvObj = NULL;


	//if(zcfgFeTr98QryRootPath() && zcfgObjectPathListPickObjPath(tr98ObjName))
	if((zcfgFeTr98QryRootPath() && enableRootQuryBlackObjPath &&
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, rootQuryBlackObjPathIdx)) ||
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, quryBlackObjPathIdx))
		return ZCFG_NO_MORE_INSTANCE;

	tr98SpvObj = zcfgFeSessionTr98SpvObjRetrieve();
	if(!tr98SpvObj){

		return zcfgFeTr98ObjQueryAndSpeed(tr98ObjName, value, enableSaveSpeedObj);

	}


	struct json_object *obj = zcfgFeRetrieveTr98SpvObject(tr98SpvObj, tr98ObjName, NULL);
	if(obj){
		*value = JSON_OBJ_COPY(obj);
		return ZCFG_SUCCESS;
	}else
		return zcfgFeTr98ObjGet(tr98ObjName, value);
}

zcfgRet_t zcfgFeTr98ObjSpvDelete(const char *tr98ObjName)
{
	struct json_object *tr98SpvObj = zcfgFeSessionTr98SpvObjRetrieve();

	if(tr98SpvObj)
		zcfgFeDeleteTr98SpvObject(tr98SpvObj, tr98ObjName);

	return zcfgFeTr98ObjDel(tr98ObjName);
}

struct json_object *zcfgFeSessionTr98SpvObjRetrieve()
{
	return sessTr98SpvObj;
}

void zcfgFeSessionTr98SpvObjSet(struct json_object *tr98SpvObj)
{
	sessTr98SpvObj = tr98SpvObj;
}

zcfgRet_t zcfgFeSessionTr98SpvObjectClear()
{
	json_object_put(sessTr98SpvObj);
	sessTr98SpvObj = NULL;

	return ZCFG_SUCCESS;
}


struct json_object *zcfgFeNewTr98SpvObject()
{
	struct json_object *tr98SpvObj = json_object_new_array();

	return tr98SpvObj;
}

bool zcfgFeTr98StatusRunSpvValid(struct json_object *multiobj)
{
	bool runSpvValid = false;
	return (multiobj) ? (json_object_object_get(multiobj, "runSpvValid") ? true : runSpvValid) : runSpvValid;
}

bool zcfgFeTr98SpvAndApplyOnSessionComplete(struct json_object *multiobj)
{
	bool tr98applyonsesscomp = false;
	return (multiobj) ? (json_object_object_get(multiobj, "tr98applyonsesscomp") ? true : tr98applyonsesscomp) : tr98applyonsesscomp;
}

int zcfgFeTr98QryRootPath()
{
	return tr98QryRootPath;
}

void zcfgFeTr98SetQryRootPath(int qryRootPath)
{
	tr98QryRootPath = qryRootPath;
}

zcfgRet_t zcfgFeAddTr98SpvObject(struct json_object *tr98SpvObj, const char *objpath, struct json_object *obj, struct json_object *spv)
{
	struct json_object *arrayObj;
	int n = 0, arrayLength;

	if(!tr98SpvObj || !objpath || !obj)
		return ZCFG_INTERNAL_ERROR;

	ztrdbg(ZTR_DL_INFO,  "%s: %s\n", __FUNCTION__, objpath);

	arrayLength = json_object_array_length(tr98SpvObj);

	arrayObj = json_object_new_object();
	json_object_object_add(arrayObj, "name", json_object_new_string(objpath));
	json_object_object_add(arrayObj, "obj", obj);
	if(spv)
		json_object_object_add(arrayObj, "spv", spv);

	n = arrayLength;
	json_object_array_put_idx(tr98SpvObj, n, arrayObj);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeDeleteTr98SpvObject(struct json_object *tr98SpvObj, const char *objpath)
{
	struct json_object *arrayObj;
	int objseq = -1, arrayLength, n = 0;
	const char *arrayObjPath;

	if(!tr98SpvObj || !objpath)
		return ZCFG_INTERNAL_ERROR;

	ztrdbg(ZTR_DL_INFO,  "%s: %s\n", __FUNCTION__, objpath);

	arrayLength = json_object_array_length(tr98SpvObj);

	while(n != arrayLength && (objseq == -1)){

		arrayObj = json_object_array_get_idx(tr98SpvObj, n);
		if(!arrayObj)
			continue;

		arrayObjPath = json_object_get_string(json_object_object_get(arrayObj, "name"));
		if(arrayObjPath && !strcmp(arrayObjPath, objpath))
			objseq = n;

		n++;
	}

	if(objseq >= 0)
		json_object_array_delete_idx(tr98SpvObj, objseq);

	return ZCFG_SUCCESS;
}

struct json_object *zcfgFeRetrieveTr98SpvObject(struct json_object *tr98SpvObj, const char *objpath, struct json_object **spv)
{
	struct json_object *arrayObj;
	struct json_object *obj = NULL;
	int arrayLength, n = 0;
	const char *arrayObjPath;

	if(!tr98SpvObj || !objpath)
		return NULL;

	ztrdbg(ZTR_DL_INFO,  "%s: %s\n", __FUNCTION__, objpath);

	arrayLength = json_object_array_length(tr98SpvObj);

	while(n != arrayLength){

		arrayObj = json_object_array_get_idx(tr98SpvObj, n);
		if(!arrayObj)
			continue;

		arrayObjPath = json_object_get_string(json_object_object_get(arrayObj, "name"));
		if(arrayObjPath && !strcmp(arrayObjPath, objpath)){
			obj = json_object_object_get(arrayObj, "obj");
			if(spv)
				*spv = json_object_object_get(arrayObj, "spv");
			return obj;
		}
		n++;
	}

	return obj;
}

zcfgRet_t zcfgFeTr98SpvObjectAppendReplace(struct json_object *tr98SpvObj, struct json_object *appendSpvObj)
{
	struct json_object *arrayObj;
	struct json_object *obj = NULL, *spv = NULL;
	int arrayLength, n = 0;
	const char *arrayObjPath;

	if(!tr98SpvObj || !appendSpvObj)
		return ZCFG_INTERNAL_ERROR;

	arrayLength = json_object_array_length(appendSpvObj);

	ztrdbg(ZTR_DL_INFO,  "%s: appendSpvObj n=%d\n", __FUNCTION__, arrayLength);

	while(n != arrayLength){
		arrayObj = json_object_array_get_idx(appendSpvObj, n);
		if(!arrayObj)
			continue;

		arrayObjPath = json_object_get_string(json_object_object_get(arrayObj, "name"));
		if(arrayObjPath){
			zcfgFeDeleteTr98SpvObject(tr98SpvObj, arrayObjPath);

			obj = json_object_object_get(arrayObj, "obj");
			spv = json_object_object_get(arrayObj, "spv");
			zcfgFeAddTr98SpvObject(tr98SpvObj, arrayObjPath, JSON_OBJ_COPY(obj), JSON_OBJ_COPY(spv));
		}
		n++;
	}

	return ZCFG_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// Tr98RpcQueryHandleRout
//--------------------------------------------------------------------------------------------------
static zcfgRet_t tr98GpnObjHostRout(const char *tr98ObjName, int handler, objIndex_t *objIid, struct json_object **tr98Jobj, void *data)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	objIndex_t objHostIid;
	struct json_object *hostsObj = NULL;
	char tr181ObjName[MAX_TR181_PATHNAME] = {0};

	tr98Parameter_t *parmList = tr98Obj[handler].parameter;

	if(!parmList)
		return ZCFG_INVALID_OBJECT;

	//if((rst = zcfgFe98To181MappingNameGet(tr98ObjName, tr181ObjName)) != ZCFG_SUCCESS){
	//	return rst;
	//}

	if(!tr98Jobj)
		return rst;

	IID_INIT(objHostIid);
	if(feObjJsonGet(RDM_OID_HOSTS, &objHostIid, &hostsObj, TR98_GET_UPDATE) == ZCFG_SUCCESS)
		json_object_put(hostsObj);

	rst = parmListSetObjParamNameAttr(tr98Jobj, parmList);

	return rst;
}

#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
static zcfgRet_t tr98GpnObjWlanCfgRout(const char *tr98ObjName, int handler, objIndex_t *objIid, struct json_object **tr98Jobj, void *data)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	objIndex_t objSsidIid;
	struct json_object *ssidObj = NULL;
	char tr181ObjName[MAX_TR181_PATHNAME] = {0};
	int attr = 0;

	tr98Parameter_t *parmList = tr98Obj[handler].parameter;

	if(!parmList)
		return ZCFG_INVALID_OBJECT;

	if((rst = zcfgFe98To181MappingNameGet(tr98ObjName, tr181ObjName)) != ZCFG_SUCCESS){
		return rst;
	}

	IID_INIT(objSsidIid);
	sscanf(tr181ObjName, "WiFi.SSID.%hhu", &objSsidIid.idx[0]);
	objSsidIid.level = 1;

	if(feObjJsonGet(RDM_OID_WIFI_SSID, &objSsidIid, &ssidObj, TR98_GET_WITHOUT_UPDATE) == ZCFG_SUCCESS){
		attr = json_object_get_int(json_object_object_get(ssidObj, "X_ZYXEL_Attribute"));
		json_object_put(ssidObj);
	}

	if(attr != 0)
		return ZCFG_INVALID_OBJECT;

	if(!tr98Jobj)
		return rst;

	rst = parmListSetObjParamNameAttr(tr98Jobj, parmList);

	return rst;
}
#endif

static zcfgRet_t tr98GpnObjWanDevRout(const char *tr98ObjName, int handler, objIndex_t *objIid, struct json_object **tr98Jobj, void *data)
{
	zcfgRet_t rst;
	char tr181ObjName[MAX_TR181_PATHNAME] = {0};
	bool wwan = false;

	if(!tr98ObjName || handler < 0)
		return ZCFG_INVALID_OBJECT;

	tr98Parameter_t *parmList = tr98Obj[handler].parameter;
	if(!parmList)
		return ZCFG_INVALID_OBJECT;

	if((rst = zcfgFe98To181MappingNameGet(tr98ObjName, tr181ObjName)) != ZCFG_SUCCESS){
		return rst;
	}

	if(!tr98Jobj)
		return rst;

	if(strstr(tr181ObjName, "USB"))
		wwan = true;

	rst = parmListSetObjParamNameAttr(tr98Jobj, parmList);
	if(!wwan){
		json_object_object_del(*tr98Jobj, "X_ZYXEL_3GEnable");
		json_object_object_del(*tr98Jobj, "X_ZYXEL_PinNumber");
	}

	return 0;
}
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
static zcfgRet_t tr98GpnObjWanDslIfCfgStsCuDayRout(const char *tr98ObjName, int handler, objIndex_t *objIid, struct json_object **tr98Jobj, void *data)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr98WanIfaceName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181DslPathName[MAX_TR181_PATHNAME] = {0};

	ztrdbg(ZTR_DL_DEBUG,  "%s\n", __FUNCTION__);

	tr98Parameter_t *parmList = tr98Obj[handler].parameter;

	if(!parmList)
		return ZCFG_INVALID_OBJECT;

	if(!tr98Jobj){
		strcpy(tr98WanIfaceName, tr98ObjName);
		char *p = strstr(tr98WanIfaceName, ".WANDSLInterfaceConfig");
		*p = '\0';
		ret = zcfgFe98To181MappingNameGet(tr98WanIfaceName, tr181DslPathName);

		return ret;
	}

	ret = WANDslIfaceCfgStatsSubObjGet(tr98ObjName, handler, tr98Jobj, TR98_GET_UPDATE);
	if(ret != ZCFG_SUCCESS)
		return ret;

	ret = parmListSetObjParamNameAttr(tr98Jobj, parmList);

	return ret;
}
#endif
static void zcfgFeTr98QueryHandleRoutInit(struct json_object **tr98QueryHandleRoutObj, tr98QueryHandleRout_t *tr98QryHandleRoutList)
{
	bool running;
	int n = 0;
	uint32_t nameSeqNum;
	char seqStr[30] = {0};

	printf("%s\n", __FUNCTION__);

	if(*tr98QueryHandleRoutObj)
		return;

	*tr98QueryHandleRoutObj = json_object_new_object();
	running = tr98QryHandleRoutList[n].rout ? true : false;

	while(running){
		nameSeqNum = tr98QryHandleRoutList[n].nameSeqNum;
		sprintf(seqStr, "%u", ARRAY_SEQNUM(nameSeqNum));

		json_object_object_add(*tr98QueryHandleRoutObj, seqStr, json_object_new_int64((intptr_t)(tr98QryHandleRoutList[n].rout)));
		n++;
		running = tr98QryHandleRoutList[n].rout ? true : false;
	}

}

void zcfgFeTr98GpnQueryHandleRoutInit()
{
	if(tr98GpnQueryHandleRoutObj){
		json_object_put(tr98GpnQueryHandleRoutObj);
		tr98GpnQueryHandleRoutObj = NULL;
	}
	zcfgFeTr98QueryHandleRoutInit(&tr98GpnQueryHandleRoutObj, tr98GpnObjHandleRoutList);
}


#ifdef ZCFG_TR181_RPC_OBJECT_HANDLE_ROUT
//--------------------------------------------------------------------------------------------------
// Tr181SpvHandleRout
//--------------------------------------------------------------------------------------------------

void zcfgFeTr181SpvHandleRoutInit()
{
	bool running;
	int n = 0;
	uint32_t oid;
	char oidStr[30] = {0};

	printf("%s\n", __FUNCTION__);

	if(tr181SpvHandleRoutObj)
		return;

	tr181SpvHandleRoutObj = json_object_new_object();
	running = spvHandleRoutList[n].rout ? true : false;

	while(running){
		oid = spvHandleRoutList[n].oid;
		sprintf(oidStr, "%u", oid);

		json_object_object_add(tr181SpvHandleRoutObj, oidStr, json_object_new_int64((intptr_t)(spvHandleRoutList[n].rout)));
		n++;
		running = spvHandleRoutList[n].rout ? true : false;
	}

}

zcfgRet_t zcfgFeMultiObjJsonSpvHandleRout(struct json_object *multiJobj, char *faultString)
{
	zcfgRet_t rst = ZCFG_SUCCESS, faultrst = ZCFG_SUCCESS;
	uint32_t oid;
	objIndex_t objIid;
	spvHandleRout handleRout;
	struct json_object *spvHdlRoutObj;

	//printf("%s\n", __FUNCTION__);

	json_object_object_foreach(multiJobj, objnumStr, objnumJobj) {
		if(!objnumJobj)
			continue;

		if(!(spvHdlRoutObj = json_object_object_get(tr181SpvHandleRoutObj, objnumStr)))
			continue;

		handleRout = (spvHandleRout )json_object_get_int64(spvHdlRoutObj);
		sscanf(objnumStr, "%u", &oid);
		printf("%s: object %u has spv rout\n", __FUNCTION__, oid);

		json_object_object_foreach(objnumJobj, objIndexStr, objIdxJObj) {
			if(!objIdxJObj)
				continue;

			IID_INIT(objIid);
			if(sscanf(objIndexStr, "%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", &objIid.level, &objIid.idx[0],
					&objIid.idx[1], &objIid.idx[2], &objIid.idx[3], &objIid.idx[4], &objIid.idx[5]) <= 0 ){
				continue;
			}
			rst = handleRout(oid, &objIid, multiJobj, objIdxJObj, faultString);
			if(rst != ZCFG_SUCCESS && rst != ZCFG_OBJECT_UPDATED)
				faultrst = rst;	
		}

	}
	rst = (faultrst != ZCFG_SUCCESS) ? faultrst : rst;
	return rst;
}

static zcfgRet_t spvDhcpv4SrvOptRout(uint32_t oid, objIndex_t *objIid, struct json_object *multiJobj, struct json_object *spvObj, char *faultString)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	struct json_object *paramObj = NULL;
	const char *paramValueStr;

	//printf("%s\n", __FUNCTION__);

	paramObj = json_object_object_get(spvObj, "Value");
	if(paramObj && json_object_get_type(paramObj) == json_type_hex){
		char ascStr[256+1] = {0}, paramstr[256+1] = {0}, *paramValueStrP = NULL;

		paramValueStr = json_object_get_string(paramObj);
		int paramValueStrLen = strlen(paramValueStr);
		if(paramValueStr[0] == 34 && paramValueStr[paramValueStrLen-1] == 34){
			strncpy(paramstr, paramValueStr+1, paramValueStrLen-2);
			paramValueStrP = paramstr;
		}else{
			paramValueStrP = paramValueStr;
		}
		printf("%s: param value: %s\n", __FUNCTION__, paramValueStrP);
		if(hexStr2AscStr(paramValueStrP, ascStr) >= 0){
			json_object_object_add(spvObj, "Value", json_object_new_string(ascStr));
			rst = ZCFG_OBJECT_UPDATED;
			printf("%s: asc str: %s\n", __FUNCTION__, ascStr);
		}else{
			rst = ZCFG_REQUEST_REJECT;
		}
	}

	return rst;
}

//--------------------------------------------------------------------------------------------------
// Tr181QueryHandleRout
//--------------------------------------------------------------------------------------------------

void zcfgFeTr181QueryHandleRoutInit()
{
	bool running;
	int n = 0;
	uint32_t oid;
	char oidStr[30] = {0};

	printf("%s\n", __FUNCTION__);

	if(tr181QueryHandleRoutObj)
		return;

	tr181QueryHandleRoutObj = json_object_new_object();
	running = queryHandleRoutList[n].rout ? true : false;

	while(running){
		oid = queryHandleRoutList[n].oid;
		sprintf(oidStr, "%u", oid);

		json_object_object_add(tr181QueryHandleRoutObj, oidStr, json_object_new_int64((intptr_t)(queryHandleRoutList[n].rout)));
		n++;
		running = queryHandleRoutList[n].rout ? true : false;
	}

}

// data: {
//  "datatype":
//  "dataobject":
// }
//
// "datatype" == "objparaminfo": dataobject contains object params, involving param attr and type


// modify object param value, strip/add param
zcfgRet_t zcfgFeTr181QueryHandleRout(uint32_t oid, objIndex_t *objIid, struct json_object *qryObj, void *data)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	qryHandleRout handleRout;
	char oidStr[30] = {0};

	if(zcfgQueryObjectNotReplied(oid, objIid) == ZCFG_NO_SUCH_OBJECT)
		return ZCFG_REQUEST_REJECT;

	sprintf(oidStr, "%u", oid);
	struct json_object *qryHdlRoutObj = json_object_object_get(tr181QueryHandleRoutObj, oidStr);

	if(qryHdlRoutObj){
		handleRout = (qryHandleRout )json_object_get_int64(qryHdlRoutObj);
		rst = handleRout(oid, objIid, qryObj, data);
	}

	return rst;
}

static zcfgRet_t queryDhcpv4SrvOptRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	struct json_object *paramObj = NULL;
	const char *paramValueStr;

	//printf("%s\n", __FUNCTION__);

	if(!queryObj){
		return ZCFG_SUCCESS;
	}

	paramObj = json_object_object_get(queryObj, "Value");
	if(paramObj && (paramValueStr = json_object_get_string(paramObj)) && strlen(paramValueStr)){
		char hexStr[256*2+1] = {0};

		printf("%s: param value: %s\n", __FUNCTION__, paramValueStr);
		if(ascStr2HexStr(paramValueStr, hexStr) >= 0){
			json_object_object_add(queryObj, "Value", json_object_new_string(hexStr));
			rst = ZCFG_SUCCESS;
			printf("%s: hex str: %s\n", __FUNCTION__, hexStr);
		}else{
			rst = ZCFG_REQUEST_REJECT;
		}
	}

	return rst;
}
int hexStrToOption(char *hexStr, unsigned char *hexOpt)
{
	unsigned char *dataPtr;
	char *ptr = hexStr;
	int value, i;

	memset(hexOpt, 0, 512); //hexOpt size is 512

	dataPtr = hexOpt;

	/* fill data */
	while ( sscanf(ptr, "%02x%n", &value, &i) == 1 )
	{
		*dataPtr++ = value;
		ptr += i;
	}
	
	return 1;
}
void maskBitsToStr(unsigned char maskBits, char *maskStr)
{
	unsigned char i = maskBits;
	unsigned char buf[4];
	unsigned int mask = 0;
	unsigned int mask2 = 0;

	while(i > 0){
		mask |= (1 << (32 - i));
		i--;
	}

	mask2 = htonl(mask);

	memset(buf, 0, sizeof(buf));
	memcpy(buf, &mask2, sizeof(buf));
	//sprintf(maskStr, "%d.%d.%d.%d", buf[3], buf[2], buf[1], buf[0]);
	sprintf(maskStr, "%d.%d.%d.%d", buf[0], buf[1], buf[2], buf[3]);
}
static zcfgRet_t queryDhcpv4ReqOptRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	struct json_object *paramObj = NULL, *paramTObj = NULL;
	const char *paramValueStr;
	char valueStr[ReqOptLen+1];
	char hexStr[ReqOptLen*2+1]; //Need reserve enough len for transfer valueStr from Str to HexStr
	char dst_str[24], gateway[17], mask[17];
	unsigned char dst[4];
	int i = 0;
	int dest_len_byte = 0;
	//printf("%s\n", __FUNCTION__);

	if(!queryObj){
		return ZCFG_SUCCESS;
	}
	paramTObj = json_object_object_get(queryObj, "Tag");
	paramObj = json_object_object_get(queryObj, "Value");
	
	memset(valueStr, 0, sizeof(valueStr));
	memset(hexStr, 0, sizeof(hexStr));
	if(paramObj && (paramValueStr = json_object_get_string(paramObj)) && strlen(paramValueStr) && (json_object_get_int(paramTObj) == 121)){
		unsigned char option[512]; //hexOpt size is 512

		if(hexStrToOption(paramValueStr, option)){
			i = 2;
			while(i < option[1] + 2){
				char cmd[256] = {0};
				int nCount=0;
				//ignore default route in option 121
				if(option[i] == 0){
					i += 5;
					continue;
				}
				/* init buffer */
				memset(dst, 0, sizeof(dst));
				memset(dst_str, 0, sizeof(dst_str));
				memset(gateway, 0, sizeof(gateway));
				/* parse mask */
				maskBitsToStr(option[i], mask);

				/* parse dest  */
				dest_len_byte = option[i]/8 + ((option[i]%8 != 0)?1:0);
				i++;

				memcpy(dst, &option[i], dest_len_byte);
				sprintf(dst_str, "%d.%d.%d.%d", dst[0], dst[1], dst[2], dst[3]);
				i += dest_len_byte;

				/* parse gateway  */
				sprintf(gateway, "%d.%d.%d.%d", option[i], option[i + 1], option[i + 2], option[i + 3]);
				i += 4;				
				zos_snprintf(cmd, sizeof(cmd), "%s/%s-%s ",dst_str, mask, gateway);
				nCount = sizeof(valueStr) - strlen(valueStr);
				strncat(valueStr,cmd, nCount-1);
			}
			
			if(ascStr2HexStr(valueStr, hexStr) >= 0){
				json_object_object_add(queryObj, "Value", json_object_new_string(hexStr));
			}
			rst = ZCFG_SUCCESS;
		}else{
			rst = ZCFG_REQUEST_REJECT;
		}
	}

	return rst;
}
#ifdef CAAAC


static zcfgRet_t queryUPnPRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	printf("%s\n", __FUNCTION__);

	if(queryObj){
		json_object_object_del(queryObj, "UPnPWLANAccessPoint");
	}

	if(data){
		struct json_object *dataObject = zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(data);
		if(dataObject)
			json_object_object_del(dataObject, "UPnPWLANAccessPoint");
	}


	return ZCFG_SUCCESS;
}


static zcfgRet_t queryFeatureFlagRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	printf("%s\n", __FUNCTION__);

	if(queryObj){
		json_object_object_del(queryObj, "DisableWifiHWButton");
	}

	if(data){
		struct json_object *dataObject = zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(data);
		if(dataObject)
			json_object_object_del(dataObject, "DisableWifiHWButton");
	}

	return ZCFG_SUCCESS;
}


static zcfgRet_t queryGuiCustomRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	printf("%s\n", __FUNCTION__);

	if(queryObj){
		json_object_object_del(queryObj, "CbtGuestWifi");
	}

	if(data){
		struct json_object *dataObject = zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(data);
		if(dataObject)
			json_object_object_del(dataObject, "CbtGuestWifi");
	}

	return ZCFG_SUCCESS;
}


static zcfgRet_t queryOiWzardRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{
	printf("%s\n", __FUNCTION__);

	if(queryObj){
		json_object_object_del(queryObj, "OiWifiEnable");
		json_object_object_del(queryObj, "OiWifiVLANID");
	}

	if(data){
		struct json_object *dataObject = zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(data);
		if(dataObject){
			json_object_object_del(dataObject, "OiWifiEnable");
			json_object_object_del(dataObject, "OiWifiVLANID");
		}
	}

	return ZCFG_SUCCESS;
}


static zcfgRet_t queryExtRout(uint32_t oid, objIndex_t *objIid, struct json_object *queryObj, void *data)
{

	printf("%s\n", __FUNCTION__);

	if(queryObj){
		json_object_object_del(queryObj, "WPSRunning");
		json_object_object_del(queryObj, "WPSApIndex");
		json_object_object_del(queryObj, "WPSRunningStatus");
	}

	if(data){
		struct json_object *dataObject = zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(data);
		if(dataObject){
			json_object_object_del(dataObject, "WPSRunning");
			json_object_object_del(dataObject, "WPSApIndex");
			json_object_object_del(dataObject, "WPSRunningStatus");
		}
	}

	return ZCFG_SUCCESS;
}

#endif

#endif

#ifdef ZCFG_APPLY_RPC_METHOD_AND_SUBSEQUENT_ROUT
//e_TR98_SENT_DHCP_OPT
static zcfgRet_t spvSntDhcpOptSubRout(const char *objPathName, objIndex_t *objIid, struct json_object *tr98SpvObj, struct json_object *sessTr98SpvObj)
{
	char pathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	objIndex_t wanObjIid;
	zcfgRet_t rst = ZCFG_SUCCESS;

	ztrdbg(ZTR_DL_INFO,  "%s\n", __FUNCTION__);
	//(!objIid || !tr98SpvObj || !sessTr98SpvObj)
#if 0
	IID_INIT(wanObjIid);
	memcpy(&wanObjIid, objIid, sizeof(objIndex_t));
	wanObjIid.level--;
	wanObjIid.idx[3] = 0;
	zcfgFe98SeqnumToName(e_TR98_WAN_IP_CONN, wanObjIid, pathName);
	if(!strstr(pathName, "WANIPConnection"))
		return ZCFG_EMPTY_OBJECT;
#endif
	strcpy(pathName, objPathName);
	char *dhcpcltstr = strstr(pathName, ".DHCPClient");
	if(dhcpcltstr)
		*dhcpcltstr = '\0';

	struct json_object *spv;
	struct json_object *wanObj = zcfgFeRetrieveTr98SpvObject(tr98SpvObj, pathName, &spv);
	if(wanObj){
		struct json_object *appendWanObj = JSON_OBJ_COPY(wanObj);
		struct json_object *appendSpvObj = JSON_OBJ_COPY(spv);
		if((rst = zcfgFeDeleteTr98SpvObject(tr98SpvObj, pathName)) == ZCFG_SUCCESS)
			rst = zcfgFeAddTr98SpvObject(tr98SpvObj, pathName, appendWanObj, appendSpvObj);

		return rst;
	}

	wanObj = zcfgFeRetrieveTr98SpvObject(sessTr98SpvObj, pathName, &spv);
	if(wanObj){
		struct json_object *appendWanObj = JSON_OBJ_COPY(wanObj);
		struct json_object *appendSpvObj = JSON_OBJ_COPY(spv);
		rst = zcfgFeAddTr98SpvObject(tr98SpvObj, pathName, appendWanObj, appendSpvObj);

		return rst;
	}else{
		rst = zcfgFeTr98ObjGet(pathName, &wanObj);
		if(rst != ZCFG_SUCCESS)
			return rst;
		spv = json_object_new_object();
		json_object_object_add(wanObj, "Reset", json_object_new_boolean(true));
		json_object_object_add(spv, "Reset", json_object_new_boolean(true));
		rst = zcfgFeAddTr98SpvObject(tr98SpvObj, pathName, wanObj, spv);

		return rst;
	}

	// no subsequent spv obj
	return ZCFG_EMPTY_OBJECT;
}

static rpcSubRout existSpvSubseqRout(zcfg_name_t seqnum)
{
	rpcSubRout rout = 0;
	int n = 0;

	while(spvSubseqRoutList[n].nameSeqNum && (spvSubseqRoutList[n].nameSeqNum != seqnum))
		n++;

	rout = spvSubseqRoutList[n].nameSeqNum ? spvSubseqRoutList[n].rout : 0;

	return rout;
}

// on a spv, add its subsequent process
bool zcfgAddSpvSubsequent(struct json_object *tr98SpvObj)
{
	zcfg_name_t seqnum;
	objIndex_t tr98ObjIid;
	rpcSubRout subseqRout;
	const char *objpathnames[e_TR98_LAST-e_TR98_START], *arrayObjPath;
	int n = 0, nObj = 0, arrayLength;
	struct json_object *arrayObj, *sessTr98SpvObj;


	sessTr98SpvObj = zcfgFeSessionTr98SpvObjRetrieve();
	arrayLength = json_object_array_length(tr98SpvObj);
	ztrdbg(ZTR_DL_INFO,  "%s: tr98SpvObj n=%d\n", __FUNCTION__, arrayLength);

	while(n != arrayLength){
		arrayObj = json_object_array_get_idx(tr98SpvObj, n);
		if(!arrayObj)
			continue;

		if((arrayObjPath = json_object_get_string(json_object_object_get(arrayObj, "name"))))
			objpathnames[nObj++] = arrayObjPath;

		n++;
	}

	char pathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	bool subsequentSpv = false;

	n = 0;
	while(n != nObj){
		arrayObjPath = objpathnames[n];
		strcpy(pathName, arrayObjPath);
		IID_INIT(tr98ObjIid);
		if((zcfgFe98NameToSeqnum(pathName, &seqnum, &tr98ObjIid) == ZCFG_SUCCESS) &&
				(subseqRout = existSpvSubseqRout(seqnum))){
			ztrdbg(ZTR_DL_INFO,  "%s: there's spv subsequent process on %s\n", __FUNCTION__, pathName);

			if(subseqRout(pathName, &tr98ObjIid, tr98SpvObj, sessTr98SpvObj) == ZCFG_SUCCESS)
				subsequentSpv = true;
		}

		n++;
	}

	return subsequentSpv;
}
#endif

//-------------------------------------------------------------------------------------------------------------------------------------
// TR98 spv api, utilized by tr98 handle rout

struct json_object *zcfgFeHandRoutRetrieveTr98Spv(const char *objpath, struct json_object *tr98Jobj, struct json_object *multiJobj)
{
	if(multiJobj)
		return json_object_object_get(multiJobj, "spv");

	// while there's no multiJobj

	return tr98Jobj;
}
//-------------------------------------------------------------------------------------------------------------------------------------

zcfgRet_t zcfgFe98StaticNameTableInit(void)
{
	int maxTableEntries = e_TR98_LAST;
	int n = 0, n_Entries;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(tr98StaticNameTable)
		return ZCFG_SUCCESS;

	if(!(tr98StaticNameTable = json_object_new_c_object(maxTableEntries))) {
		zcfgLog(ZCFG_LOG_INFO, "%s: Tr98 static name table allocation fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	n_Entries = e_TR98_LAST-e_TR98_START;
	while(n < n_Entries) {
		json_object_object_add(tr98StaticNameTable, (const char *)tr98Obj[n].name, 
			    (struct json_object *)&tr98NameObjList[n]);
		n++;
	}

	zcfgFe98InfoDataSetInit();

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFe98InfoDataSetInit()
{
#ifdef ZCFG_RUNNING_WANDEV_INDEX_CUSTOMIZATION
	StaticWanDevOption = true;
#endif

	return ZCFG_SUCCESS;
}
void zcfgFe98BondingTag(bool type)
{
#ifdef ZYXEL_SUPPORT_TR098_BRCM_BONDING_LINE
	isBondingWanProcess = type;
#endif //ZYXEL_SUPPORT_TR098_BRCM_BONDING_LINE
}

bool zcfgFe98StaticWanDevStatus(void)
{
	return StaticWanDevOption;
}

zcfgRet_t zcfgFe98To181ObjMappingNameAdd(const char *tr98PathName, const char *tr181PathName, zcfg_name_t *nameSeqNum, objIndex_t *tr98ObjIid)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t tr181ObjIid, objIid;
	zcfg_offset_t oid;
	zcfg_name_t seqnum = 0;
	char pathName[OBJ_NAME_BUFF_SIZE] = {0};
	const char *tr181Name = NULL;

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	return ZCFG_SUCCESS;
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!tr98PathName || !tr181PathName || !nameSeqNum || !tr98ObjIid)
		return ZCFG_INTERNAL_ERROR;

	*nameSeqNum = 0;
	IID_INIT((*tr98ObjIid));

	IID_INIT(objIid);
	if((ret = zcfgFe98NameToSeqnum(tr98PathName, &seqnum, &objIid)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: NameToSeqnum fail!\n", __FUNCTION__);
		return ret;
	}

	IID_INIT(tr181ObjIid);
	if(strncmp(tr181PathName, "Device", 6)) {
		sprintf(pathName, "Device.%s", tr181PathName);
		tr181Name = pathName;
	}
	else {
		tr181Name = tr181PathName;
	}
	if((oid = zcfgFeObjNameToObjId((char *)tr181Name, &tr181ObjIid)) == ZCFG_NO_SUCH_OBJECT) {
		zcfgLog(ZCFG_LOG_INFO, "%s Name To ObjId fail!\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	if((ret = zcfg98To181ObjMappingAdd(seqnum, &objIid, oid, &tr181ObjIid)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s 98To181 mapping add fail!\n", __FUNCTION__);
		return ret;
	}

	*nameSeqNum = seqnum;
	memcpy(tr98ObjIid, &objIid, sizeof(objIndex_t));

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFe98To181ObjMappingNameGet(const char *tr98PathName, char *tr181PathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_name_t nameSeqNum = 0;
	objIndex_t tr181ObjIid, objIid;
	zcfg_offset_t oid;
	char queriedTr181PathName[OBJ_NAME_BUFF_SIZE] = {0};

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	return ZCFG_SUCCESS;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if(!tr98PathName || !tr181PathName || strlen(tr98PathName) == 0)
		return ZCFG_INTERNAL_ERROR;

	ztrdbg(ZTR_DL_DEBUG,  "%s: *** query tr98PathName=%s\n", __FUNCTION__, tr98PathName);
	
 	IID_INIT(objIid);
	if((ret = zcfgFe98NameToSeqnum(tr98PathName, &nameSeqNum, &objIid)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: NameToSeqnum fail!\n", __FUNCTION__);
		return ret;
	}
	//printf("e_TR98_LAN_DEV=%d\n", e_TR98_LAN_DEV);


	IID_INIT(tr181ObjIid);
	if((ret = zcfgObjMappingGet(ZCFG_MSG_REQUEST_TR98_OBJMAPPING, nameSeqNum, &objIid, &oid, &tr181ObjIid)) != ZCFG_SUCCESS) {
		ztrdbg(ZTR_SL_DEBUG, "%s: 98To181 mapping query fail\n", __FUNCTION__);
		return ret;
	}

	if((ret = zcfgFeObjIdToName(oid, &tr181ObjIid, queriedTr181PathName)) != ZCFG_SUCCESS) {
		ztrdbg(ZTR_SL_DEBUG, "%s: ObjId to Name fail\n", __FUNCTION__);
		return ret;	
	}

	tr181PathName[0] = '\0';
	if (!strncmp(queriedTr181PathName, "Device.", strlen("Device.")))
		strcpy(tr181PathName, queriedTr181PathName + strlen("Device."));
	else
		strcpy(tr181PathName, queriedTr181PathName);
	
	ztrdbg(ZTR_DL_DEBUG,  "%s: 98181mapqried: %s %s\n", __FUNCTION__, tr98PathName, tr181PathName);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFe181To98ObjMappingNameGet(const char *tr181PathName, char *tr98PathName)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_name_t nameSeqNum = 0;
	objIndex_t tr181ObjIid, objIid;
	zcfg_offset_t oid;

#ifndef ZCFG_DYNAMIC_NAMEMAPPING
	return ZCFG_SUCCESS;
#endif

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	ztrdbg(ZTR_DL_DEBUG,  "%s: *** tr181PathName=%s\n", __FUNCTION__, tr181PathName);

	if(!tr98PathName || !tr181PathName)
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(tr181ObjIid);
	if((oid = zcfgFeObjNameToObjId((char *)tr181PathName, &tr181ObjIid)) == ZCFG_NO_SUCH_OBJECT) {
		zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	IID_INIT(objIid);
	if((ret = zcfgObjMappingGet(ZCFG_MSG_REQUEST_TR181_OBJMAPPING, oid, &tr181ObjIid, &nameSeqNum, &objIid)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: 181To98 mapping query fail\n", __FUNCTION__);
		return ret;
	}

 	//IID_INIT(objIid);
	if((ret = zcfgFe98SeqnumToName(nameSeqNum, &objIid, tr98PathName)) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: SeqnumToName fail\n", __FUNCTION__);
		return ret;
	}

	ztrdbg(ZTR_DL_DEBUG,  "%s: 18198mapqried: %s %s\n", __FUNCTION__, tr181PathName, tr98PathName);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFe98NameToSeqnum(const char *tr98PathName, zcfg_name_t *seqnum, objIndex_t *tr98ObjIid)
{
	const char *pos = NULL;
	int n = 0, level = 0;
	bool subpathname = false;
	char instNumStr[5] = {0};
	char pathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	int pathNameLen = 0, instNumStrLen = 0;
	objIndex_t objIid;
	tr98NameObj_t *entry = NULL;

	ztrdbg(ZTR_SL_DEBUG, "%s\n", __FUNCTION__);

	if(!tr98PathName || !seqnum || !tr98ObjIid || (strlen(tr98PathName) <= 0))
		return ZCFG_INTERNAL_ERROR;

	*seqnum = 0;
	IID_INIT(*tr98ObjIid);

	IID_INIT(objIid);
	pos = (tr98PathName + n);
	while(*pos != '\0') {
		if(*pos == '.') {
			subpathname = (_isdigit(*(pos + 1)) && !subpathname) ? true : false;
			pathName[pathNameLen++] = *pos;
		}
		else if(subpathname == true) {
			if(_isdigit(*pos)) {
				if(instNumStrLen == 0) {
					pathName[pathNameLen++] = 'i';
				}
				instNumStr[instNumStrLen++] = *pos;
				if((instNumStrLen+1) >= sizeof(instNumStr))
					return ZCFG_NO_SUCH_OBJECT;
			}
			else {
				subpathname = false;
				if(instNumStrLen && pathName[pathNameLen-1] == 'i') {
					strcat(&pathName[pathNameLen-1], instNumStr);
					pathNameLen += (instNumStrLen-1);
					memset(instNumStr, 0, sizeof(instNumStr));
					instNumStrLen = 0;
				}
				else
				pathName[pathNameLen++] = *pos;
			}
		}
		else {
			pathName[pathNameLen++] = *pos;
		}
		pos = tr98PathName+(++n);
		if(((subpathname == false) && instNumStrLen) || ((*pos == '\0') && instNumStrLen)) {
			int idx = 0;
			if(level < SUPPORTED_INDEX_LEVEL)
				objIid.idx[level] = ((idx = atoi(instNumStr)) > 255) ? 255 : idx;

			level++;
			instNumStrLen = 0;
			memset(instNumStr, 0, sizeof(instNumStr));
		}
	}

	//zcfgLog(ZCFG_LOG_INFO, "%s: parsed, %s\n", __FUNCTION__, pathName);

	if(level >= SUPPORTED_INDEX_LEVEL) {
		ztrdbg(ZTR_SL_DEBUG, "%s: Object index error\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	objIid.level = level;
	entry = (tr98NameObj_t *)json_object_object_get(tr98StaticNameTable, (const char *)pathName);
	if(!entry)
		return ZCFG_NO_SUCH_OBJECT;

	if(!IS_SEQNUM_RIGHT((entry->nameSeqNum)))
		return ZCFG_NO_SUCH_OBJECT;

	ztrdbg(ZTR_SL_DEBUG, "%s: seqnum, %u %hu,%hu,%hu,%hu,%hu,%hu\n", __FUNCTION__, entry->nameSeqNum,
		objIid.idx[0], objIid.idx[1], objIid.idx[2], objIid.idx[3], objIid.idx[4], objIid.idx[5]);

	*seqnum = (zcfg_name_t)entry->nameSeqNum;
	memcpy(tr98ObjIid, &objIid, sizeof(objIndex_t));

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFe98SeqnumToName(zcfg_name_t seqnum, objIndex_t *objIid, char *tr98PathName)
{
	int nameListSeqnum = 0, n = 0, level = 0;
	int tr98PathNameLen = 0;
	const char *name = NULL;
	char pathName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char instNumStr[5] = {0};
	const char *pos = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	if(!(IS_SEQNUM_RIGHT(seqnum)) || !objIid || !tr98PathName)
		return ZCFG_INTERNAL_ERROR;

	nameListSeqnum = ARRAY_SEQNUM(seqnum);
	name = (tr98NameObjList[nameListSeqnum].tr98Obj)->name;
	strcpy(pathName, name);

	pos = (pathName + n);
	while(*pos != '\0' && (n != TR98_MAX_OBJ_NAME_LENGTH-1)) {
		if((level < SUPPORTED_INDEX_LEVEL) && *pos == 'i' && (n != 0) && (*(pos - 1) == '.' && (*(pos + 1) == '.' || *(pos + 1) == '\0'))) {
			memset(instNumStr, 0, sizeof(instNumStr));

			sprintf(instNumStr, "%hhu", objIid->idx[level]);

			level++;
			strcat(tr98PathName, instNumStr);
			tr98PathNameLen += strlen(instNumStr);

		}
		else {
			*(tr98PathName + (tr98PathNameLen++)) = *pos;
		}
		pos = (pathName + (++n));
	}
	*(tr98PathName + tr98PathNameLen) = '\0';

	if(level >= SUPPORTED_INDEX_LEVEL) {
		zcfgLog(ZCFG_LOG_INFO, "%s: Object index error\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	zcfgLog(ZCFG_LOG_INFO, "%s: translated, %s\n", __FUNCTION__, tr98PathName);

	return ZCFG_SUCCESS;
}

// return 'n,n+1,n+2,..'
char *zcfgFeTr98ObjQueryInstanceArray(const char *tr98PathName)
{
	char *instances = NULL;
	zcfg_name_t nameSeqNum;
	objIndex_t tr98ObjIid;
	zcfgRet_t rst = ZCFG_SUCCESS;

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if(!tr98PathName || strlen(tr98PathName) == 0)
		return NULL;

	ztrdbg(ZTR_DL_DEBUG,  "%s: *** query tr98PathName=%s\n", __FUNCTION__, tr98PathName);

 	IID_INIT(tr98ObjIid);
	if(zcfgFe98NameToSeqnum(tr98PathName, &nameSeqNum, &tr98ObjIid) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: NameToSeqnum fail!\n", __FUNCTION__);
		return NULL;
	}

	if((rst = zcfgFeMsgObjQryInstances(REQUEST_QUERY_TR98_OBJ_INSTANCE_ARRAY, nameSeqNum, &tr98ObjIid, &instances)) != ZCFG_SUCCESS) {
		if(rst == ZCFG_NO_MORE_INSTANCE){
			// mapping not built
			char res[] = "notbuilt";
			instances = malloc(strlen(res)+1);
			strcpy(instances, res);
		}else if(rst == ZCFG_EMPTY_OBJECT){
			// object not added
			char res[] = "emptyobj";
			instances = malloc(strlen(res)+1);
			strcpy(instances, res);
		}
		ztrdbg(ZTR_SL_DEBUG, "%s: query obj instances fail\n", __FUNCTION__);
		return instances;
	}

	ztrdbg(ZTR_DL_DEBUG,  "%s: 98mapqried instances: %s %s\n", __FUNCTION__, tr98PathName, instances);

	return instances;
}

zcfgRet_t zcfgTr98MapAddObjName()
{
	struct json_object *zcfgTr98MapTable = NULL;
	uint32_t seqnum, arrayIdx;
	char *tr98PathName = NULL;
	struct json_object *jObj;

	if(!(zcfgTr98MapTable = json_object_from_file(ZCFG_TR98_MAP_PATH))) {
		ztrdbg(ZTR_SDL_DEBUG, "%s: retrieve tr98 mapping data fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	seqnum = e_TR98_START;
	while(seqnum != e_TR98_LAST){
		arrayIdx = ARRAY_SEQNUM(seqnum);
		jObj = json_object_array_get_idx(zcfgTr98MapTable, arrayIdx);
		if(!jObj){
			seqnum++;
			continue;
		}

		tr98PathName = (tr98NameObjList[arrayIdx].tr98Obj)->name;
		if(!tr98PathName || strlen(tr98PathName) == 0){
			zcfgLog(ZCFG_LOG_INFO, "%s: %u SeqnumToName fail\n", __FUNCTION__, seqnum);
			seqnum++;
			continue;
		}

		json_object_object_add(jObj, "name", json_object_new_string(tr98PathName));

		seqnum++;
	}

	if(json_object_to_file(ZCFG_TR98_MAP_ADDOBJNAME, zcfgTr98MapTable) < 0) {
		ztrdbg(ZTR_SDL_DEBUG, "%s: save tr98 mapping obj num/name fail\n", __FUNCTION__);
		json_object_put(zcfgTr98MapTable);
		return ZCFG_INTERNAL_ERROR;
	}
	json_object_put(zcfgTr98MapTable);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgTr181MapAddObjName()
{
	struct json_object *zcfgTr181MapTable = NULL;

	if(!(zcfgTr181MapTable = json_object_from_file(ZCFG_TR181_MAP_PATH))) {
		ztrdbg(ZTR_SDL_DEBUG, "%s: retrieve tr181 mapping data fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_object_foreach(zcfgTr181MapTable, objnumStr, objnumJobj) {
		if(!objnumJobj)
			continue;

		int objnum = atoi(objnumStr);
		const char *name = zcfgFeTr181OidToName(objnum);
		if(!name){
			continue;
		}
		json_object_object_add(objnumJobj, "name", json_object_new_string(name));
	}
	if(json_object_to_file(ZCFG_TR181_MAP_ADDOBJNAME, zcfgTr181MapTable) < 0) {
		ztrdbg(ZTR_SDL_DEBUG, "%s: save tr181 mapping data fail\n", __FUNCTION__);
		json_object_put(zcfgTr181MapTable);
		return ZCFG_INTERNAL_ERROR;
	}

	json_object_put(zcfgTr181MapTable);

	return ZCFG_SUCCESS;
}


int charHex2Int(char hex)
{
	//printf("hex: %hhu\n", hex);

	if (hex >= 0x30 && hex < 0x3a)
		return hex-0x30;

	else if (hex >= 0x41 && hex <= 0x46)
		return 10+(hex-0x41);

	else if (hex >= 0x61 && hex <= 0x66)
		return 10+(hex-0x61);

	else return -1;
}

int hexStr2AscStr(const char *hexStr, char *ascStr)
{
	int outputStrLen = 0;
	int digit1, digit2;
	int n = 0, len = 0;
	int running, nothex;


	if(!hexStr || !(len = strlen(hexStr)) || !ascStr)
		return 0;

	if((len%2) != 0)
		return -1;

	digit1 = charHex2Int(*(hexStr + n));
	digit2 = charHex2Int(*(hexStr + n + 1));
	nothex = (digit1 == -1 || digit2 == -1) ? 1 : 0;
	running = (digit1 >= 0 && digit2 >= 0) ? 1 : 0;

	while (running) {
		*(ascStr+outputStrLen) = (digit1 << 4)+digit2;
		outputStrLen += 1;
		n += 2;

		digit1 = charHex2Int(*(hexStr + n));
		digit2 = charHex2Int(*(hexStr + n + 1));
		nothex = (len > n && (digit1 == -1 || digit2 == -1)) ? 1 : 0;
		running = (len > n && digit1 >= 0 && digit2 >= 0) ? 1 : 0;
	}

	return outputStrLen = (nothex) ? -1 : ((outputStrLen*2 == len) ? outputStrLen : 0);
}

int ascStr2HexStr(const char *ascStr, char *hexStr)
{
	int outputStrLen = 0;
	int digit1, digit2;
	int n = 0, len = 0;
	int running, nothex;


	if(!ascStr || !(len = strlen(ascStr)) || !hexStr)
		return 0;

	running = (len > n) ? 1 : 0;

	while (running) {

		sprintf(hexStr+outputStrLen, "%02x", *(ascStr+n));

		outputStrLen += 2;
		n++;

		running = (len > n) ? 1 : 0;
	}

	return outputStrLen;
}


bool base64Char(char bs64)
{
	bool ret = true;


	if (bs64 >= 0x30 && bs64 < 0x3a)
		return ret;

	else if (bs64 >= 0x41 && bs64 <= 0x5a)
		return ret;

	else if (bs64 >= 0x61 && bs64 <= 122)
		return ret;

	else if (bs64 == '+' || bs64 == '=' || bs64 == '/')
		return ret;

	else return false;
}

char *base64StrParticularCharHandle(const char *base64Str)
{
	char *modifiedStr = NULL, *base64StrP;
	int modified = 0, running = 0;
	int n1 = 0, p1 = 0, base64StrLen;
	bool itsbase64Char;

	if(base64Str && (base64StrLen = strlen(base64Str))){
		modifiedStr = (char *)malloc(base64StrLen+1);
		memset(modifiedStr, 0, base64StrLen+1);
		running = 1;
		base64StrP = base64Str;
	}else{
		return NULL;
	}


	while(running){

		itsbase64Char = base64Char(*(base64StrP+p1));

		*(modifiedStr+n1) = itsbase64Char ? *(base64StrP+p1) : *(modifiedStr+n1);
		n1 = itsbase64Char ? n1+1 : n1;

		p1++;
		running = (*(base64StrP+p1) == '\0') ? 0 : 1;

	}

	if(n1 != base64StrLen){
		return modifiedStr;
	}else{
		ZOS_FREE(modifiedStr);
		return NULL;
	}
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


#define TR098181_OBJECT_MAPPING_NUM_NAME_CONVERT
#ifdef TR098181_OBJECT_MAPPING_NUM_NAME_CONVERT
//----------------------------------------------------------------------------------------------------
// Tr98/Tr181 object mapping convert routines -- start
static int tr98181mappingConvertDebug = 0;

void zcfgMappingConvertDataOutput(int enable)
{
	tr98181mappingConvertDebug = enable;
}

struct json_object *zcfgTr98CreatePvsNewMapping()
{
	struct json_object *zcfgTr98MapSaveTable = NULL, *jObj;
	const char *tr98PathName = NULL;
	struct json_object *zcfgTr98PvsNew = NULL;
	tr98NameObj_t *entry = NULL;
	uint32_t seqnum, arrayIdx;

	printf("%s\n", __FUNCTION__);

	if(!(zcfgTr98MapSaveTable = json_object_from_file(ZCFG_TR98_MAP_ADDOBJNAME))) {
		ztrdbg(ZTR_SDL_INFO, "%s: retrieve tr98 mapping save data fail\n", __FUNCTION__);
		return NULL;
	}

	zcfgTr98PvsNew = json_object_new_array_list((e_TR98_LAST-e_TR98_START)+100);

	seqnum = e_TR98_START;
	while(seqnum != (e_TR98_LAST+100)){
		arrayIdx = ARRAY_SEQNUM(seqnum);
		jObj = json_object_array_get_idx(zcfgTr98MapSaveTable, arrayIdx);
		if(!jObj){
			seqnum++;
			continue;
		}

		tr98PathName = json_object_get_string(json_object_object_get(jObj, "name"));
		if(!tr98PathName || strlen(tr98PathName) == 0){
			zcfgLog(ZCFG_LOG_INFO, "%s: %u SeqnumToName fail\n", __FUNCTION__, seqnum);
			seqnum++;
			continue;
		}

		entry = (tr98NameObj_t *)json_object_object_get(tr98StaticNameTable, (const char *)tr98PathName);
		if(!entry){
			//ZCFG_NO_SUCH_OBJECT;
			seqnum++;
			continue;
		}

		if(!IS_SEQNUM_RIGHT((entry->nameSeqNum))){
			//ZCFG_NO_SUCH_OBJECT;
			seqnum++;
			continue;
		}

		zcfg_name_t newnum = (zcfg_name_t)entry->nameSeqNum;

		struct json_object *newpathnameobj = json_object_new_object();
		json_object_object_add(newpathnameobj, "new", json_object_new_int(newnum));
		json_object_array_put_idx(zcfgTr98PvsNew, arrayIdx, newpathnameobj);

		seqnum++;
	}

	if(tr98181mappingConvertDebug){
		if(json_object_to_file(ZCFG_TR98_PATHNAME_PVSNEW, zcfgTr98PvsNew) < 0) {
			ztrdbg(ZTR_SDL_INFO, "%s: save tr181 mapping data fail\n", __FUNCTION__);
			json_object_put(zcfgTr98PvsNew);
			json_object_put(zcfgTr98MapSaveTable);
			return NULL;
		}
	}
	json_object_put(zcfgTr98MapSaveTable);

	return zcfgTr98PvsNew;
}

struct json_object *zcfgTr181CreatePvsNewMapping()
{
	struct json_object *zcfgTr181MapSaveTable = NULL;
	struct json_object *zcfgTr181PvsNew = NULL;

	printf("%s\n", __FUNCTION__);

	if(!(zcfgTr181MapSaveTable = json_object_from_file(ZCFG_TR181_MAP_ADDOBJNAME))) {
		ztrdbg(ZTR_SDL_INFO, "%s: retrieve tr181 mapping save data fail\n", __FUNCTION__);
		return NULL;
	}

	zcfgTr181PvsNew = json_object_new_object();

	json_object_object_foreach(zcfgTr181MapSaveTable, objnumStr, objnumJobj) {
		if(!objnumJobj)
			continue;

		const char *pathname = json_object_get_string(json_object_object_get(objnumJobj, "name"));
		if(!pathname)
			continue;

		uint32_t newTr181Objnum = zcfgTr181ObjNameOid((char *)pathname);

		struct json_object *pathnameobj = json_object_new_object();
		json_object_object_add(pathnameobj, "new", json_object_new_int(newTr181Objnum));
		json_object_object_add(zcfgTr181PvsNew, objnumStr, pathnameobj);
	}

	if(tr98181mappingConvertDebug){
		if(json_object_to_file(ZCFG_TR181_PATHNAME_PVSNEW, zcfgTr181PvsNew) < 0) {
			ztrdbg(ZTR_SDL_INFO, "%s: save tr181 mapping data fail\n", __FUNCTION__);
			json_object_put(zcfgTr181PvsNew);
			json_object_put(zcfgTr181MapSaveTable);
			return NULL;
		}
	}

	json_object_put(zcfgTr181MapSaveTable);

	return zcfgTr181PvsNew;
}

static uint32_t zcfgTr98QueryNewSeqnum(struct json_object *zcfgTr98PvsNew, uint32_t seqnum)
{
	uint32_t arrayIdx = ARRAY_SEQNUM(seqnum);
	struct json_object *pvsSeqNumObj = json_object_array_get_idx(zcfgTr98PvsNew, arrayIdx);
	uint32_t newTr98Seqnum = (uint32_t)json_object_get_int(json_object_object_get(pvsSeqNumObj, "new"));
	return newTr98Seqnum;
}

static uint32_t zcfgTr181QueryNewSeqnum(struct json_object *zcfgTr181PvsNew, uint32_t objnum)
{
	struct json_object *pathnameobj;
	char objnumStr[30] = {0};
	sprintf(objnumStr, "%u", objnum);
	pathnameobj = json_object_object_get(zcfgTr181PvsNew, objnumStr);
	uint32_t newTr181Objnum = (uint32_t)json_object_get_int(json_object_object_get(pathnameobj, "new"));

	return newTr181Objnum;
}

zcfgRet_t zcfgTr98MappingConvert(struct json_object *zcfgTr98PvsNew, struct json_object *zcfgTr181PvsNew)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	FILE *fmap, *fmapconvert;
	char line[TR98_MAX_OBJ_NAME_LENGTH], lineOutput[TR98_MAX_OBJ_NAME_LENGTH];
	char objStr[30] = {0}, idxstr[30] = {0};
	char parmName[MAX_PARMNAME_LEN] = {0};
	char *str, *str181mapping, *strmapping;
	struct json_object *zcfgTr98MapConvert, *zcfgTr98MapConverted, *idxObj;
	int complete = 0, arrayIdx = 0;

	printf("%s\n", __FUNCTION__);
	remove(ZCFG_TR98_MAP_CONVERTED);

	fmap = fopen(ZCFG_TR98_MAP_ADDOBJNAME, "r");
	if(!fmap){
		printf("%s: open mapping files fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	fmapconvert = fopen(ZCFG_TR98_MAP_CONVERT, "w+");
	if(!fmapconvert){
		printf("%s: open mapping files fail\n", __FUNCTION__);
		fclose(fmap);
		fmap = NULL;
		return ZCFG_INTERNAL_ERROR;
	}

	while(!complete){
		memset(line, 0, sizeof(line));
		str = fgets(line, TR98_MAX_OBJ_NAME_LENGTH, fmap);
		if(!str){
			//empty
			complete = 1;
			continue;
		}

		if((str181mapping = strstr(line, "181mapping"))){
			//181mapping":"
			str181mapping += 13;
			memset(objStr, 0, sizeof(objStr));
			memset(idxstr, 0, sizeof(idxstr));
			int n = sscanf(str181mapping, "%[^_]_%[^_]", objStr, idxstr);
			if(n < 2){
				if(tr98181mappingConvertDebug)
					printf("%s: sscanf mapping string fail, %s\n", __FUNCTION__, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			uint32_t objnum = (uint32_t)atol(objStr);
			uint32_t newTr181Objnum = zcfgTr181QueryNewSeqnum(zcfgTr181PvsNew, objnum);
			if(!newTr181Objnum){
				if(tr98181mappingConvertDebug)
					printf("%s: query %u new objnum fail, %s\n", __FUNCTION__, objnum, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			sprintf(lineOutput, "      \"181mapping\":\"%u_%s", newTr181Objnum, idxstr);
			fputs((const char *)lineOutput, fmapconvert);
		}else if((strmapping = strstr(line, "\"mapping\""))){
			//mapping":"
			strmapping += 11;
			memset(objStr, 0, sizeof(objStr));
			memset(idxstr, 0, sizeof(idxstr));
			memset(parmName, 0, sizeof(parmName));
			int n = sscanf(strmapping, "%[^_]_%[^_]_%s", objStr, idxstr, parmName);
			if(n < 3){
				if(tr98181mappingConvertDebug)
					printf("%s: sscanf mapping string fail, %s\n", __FUNCTION__, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			uint32_t objnum = (uint32_t)atol(objStr);
			uint32_t newTr181Objnum = zcfgTr181QueryNewSeqnum(zcfgTr181PvsNew, objnum);
			if(!newTr181Objnum){
				if(tr98181mappingConvertDebug)
					printf("%s: query %u new objnum fail, %s\n", __FUNCTION__, objnum, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			sprintf(lineOutput, "      \"mapping\":\"%u_%s_%s", newTr181Objnum, idxstr, parmName);
			fputs((const char *)lineOutput, fmapconvert);
		}else{
			fputs((const char *)line, fmapconvert);
		}
	}

	fclose(fmapconvert);
	fclose(fmap);
	fmapconvert = NULL;
	fmap = NULL;


	if(!(zcfgTr98MapConvert = json_object_from_file(ZCFG_TR98_MAP_CONVERT))) {
		printf("%s: retrieve tr98 tr181-objnum converted data fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	int arrayListEntryNum = e_TR98_LAST-e_TR98_START;
	if(!(zcfgTr98MapConverted = json_object_new_array_list(arrayListEntryNum))) {
		rst = ZCFG_INTERNAL_ERROR;
		goto complete;
	}

	struct json_object *zcfgTr98MapAttrList = NULL, *attrListObj = NULL, *zcfgTr98MapAttrListConverted = NULL;

	if(!(zcfgTr98MapAttrList = json_object_from_file(ZCFG_TR98_MAP_ATTR_LIST_PATH))) {
		printf("%s: retrieve tr98 attr list fail\n", __FUNCTION__);
	}else if(!(zcfgTr98MapAttrListConverted = json_object_new_array_list(arrayListEntryNum))) {
		json_object_put(zcfgTr98MapAttrList);
		rst = ZCFG_INTERNAL_ERROR;
		goto complete;
	}

	int seqnum = e_TR98_START;
	while(seqnum != (e_TR98_LAST+100)){
		arrayIdx = ARRAY_SEQNUM(seqnum);
		idxObj = json_object_array_pull_idx(zcfgTr98MapConvert, arrayIdx);
		if(!idxObj){
			seqnum++;
			continue;
		}

		json_object_object_del(idxObj, "name");

		uint32_t newTr98Seqnum = zcfgTr98QueryNewSeqnum(zcfgTr98PvsNew, seqnum);
		if(!IS_SEQNUM_RIGHT(newTr98Seqnum)){
			seqnum++;
			continue;
		}else{
			uint32_t newArrayIdx = ARRAY_SEQNUM(newTr98Seqnum);
			json_object_array_put_idx(zcfgTr98MapConverted, newArrayIdx, idxObj);
			if(zcfgTr98MapAttrList && (attrListObj = json_object_array_pull_idx(zcfgTr98MapAttrList, arrayIdx)))
				json_object_array_put_idx(zcfgTr98MapAttrListConverted, newArrayIdx, attrListObj);
		}
		seqnum++;
	}


	if(json_object_to_file(ZCFG_TR98_MAP_CONVERTED, zcfgTr98MapConverted) < 0) {
		printf("%s: save tr98 tr181 data-converted fail\n", __FUNCTION__);
		remove(ZCFG_TR98_MAP_CONVERTED);
		rst = ZCFG_INTERNAL_ERROR;
	}

	if(zcfgTr98MapAttrListConverted){
		remove(ZCFG_TR98_MAP_ATTR_LIST_PATH);
		if(json_object_to_file(ZCFG_TR98_MAP_ATTR_LIST_PATH, zcfgTr98MapAttrListConverted) < 0) {
			printf("%s: save tr98 attr list converted fail\n", __FUNCTION__);
			rst = ZCFG_INTERNAL_ERROR;
		}
		json_object_put(zcfgTr98MapAttrList);
		json_object_put(zcfgTr98MapAttrListConverted);
	}

complete:
	if(!tr98181mappingConvertDebug)
		remove(ZCFG_TR98_MAP_CONVERT);

	json_object_put(zcfgTr98MapConvert);
	json_object_put(zcfgTr98MapConverted);

	return rst;
}

zcfgRet_t zcfgTr181MappingConvert(struct json_object *zcfgTr98PvsNew, struct json_object *zcfgTr181PvsNew)
{
	zcfgRet_t rst = ZCFG_SUCCESS;
	FILE *fmap, *fmapconvert;
	char line[TR98_MAX_OBJ_NAME_LENGTH], lineOutput[TR98_MAX_OBJ_NAME_LENGTH];;
	char objStr[30] = {0}, idxstr[30] = {0};
	char parmName[MAX_PARMNAME_LEN] = {0};
	char *str, *str98mapping, *strmapping;
	int complete = 0;
	char newTr181ObjNumStr[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	printf("%s\n", __FUNCTION__);
	remove(ZCFG_TR181_MAP_CONVERTED);

	fmap = fopen(ZCFG_TR181_MAP_ADDOBJNAME, "r");
	if(!fmap){
		printf("%s: open mapping files fail\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	fmapconvert = fopen(ZCFG_TR181_MAP_CONVERT, "w+");
	if(!fmapconvert){
		printf("%s: open mapping files fail\n", __FUNCTION__);
		fclose(fmap);
		fmap = NULL;
		return ZCFG_INTERNAL_ERROR;
	}

	while(!complete){
		memset(line, 0, sizeof(line));
		str = fgets(line, TR98_MAX_OBJ_NAME_LENGTH, fmap);
		if(!str){
			complete = 1;
			continue;
		}

		if((str98mapping = strstr(line, "98mapping"))){
			//98mapping":"
			str98mapping += 12;
			memset(objStr, 0, sizeof(objStr));
			memset(idxstr, 0, sizeof(idxstr));
			int n = sscanf(str98mapping, "%[^_]_%[^_]", objStr, idxstr);
			if(n < 2){
				if(tr98181mappingConvertDebug)
					printf("%s: sscanf mapping string fail, %s\n", __FUNCTION__, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			uint32_t objnum = (uint32_t)atol(objStr);
			uint32_t newTr98Objnum = zcfgTr98QueryNewSeqnum(zcfgTr98PvsNew, objnum);
			if(!newTr98Objnum){
				if(tr98181mappingConvertDebug)
					printf("%s: query %u new tr98 objnum fail, %s\n", __FUNCTION__, objnum, line);
				//rst = ZCFG_NO_SUCH_OBJECT;
				fputs((const char *)line, fmapconvert);
				continue;
			}
			sprintf(lineOutput, "      \"98mapping\":\"%u_%s", newTr98Objnum, idxstr);
			fputs((const char *)lineOutput, fmapconvert);
		}else if((strmapping = strstr(line, "\"mapping\""))){
			//mapping":"
			strmapping += 11;
			memset(objStr, 0, sizeof(objStr));
			memset(idxstr, 0, sizeof(idxstr));
			memset(parmName, 0, sizeof(parmName));
			int n = sscanf(strmapping, "%[^_]_%[^_]_%s", objStr, idxstr, parmName);
			if(n < 3){
				if(tr98181mappingConvertDebug)
					printf("%s: sscanf mapping string fail, %s\n", __FUNCTION__, line);
				fputs((const char *)line, fmapconvert);
				continue;
			}
			uint32_t objnum = (uint32_t)atol(objStr);
			uint32_t newTr98Objnum = zcfgTr98QueryNewSeqnum(zcfgTr98PvsNew, objnum);
			if(!newTr98Objnum){
				if(tr98181mappingConvertDebug)
					printf("%s: query %u new tr98 objnum fail, %s\n", __FUNCTION__, objnum, line);
				//rst = ZCFG_NO_SUCH_OBJECT;
				fputs((const char *)line, fmapconvert);
				continue;
			}
			sprintf(lineOutput, "      \"mapping\":\"%u_%s_%s", newTr98Objnum, idxstr, parmName);
			fputs((const char *)lineOutput, fmapconvert);
		}else{
			fputs((const char *)line, fmapconvert);
		}
	}

	fclose(fmapconvert);
	fclose(fmap);
	fmapconvert = NULL;
	fmap = NULL;

	struct json_object *zcfgTr181MapConvert = NULL, *zcfgTr181MapConverted = NULL;

	if(!(zcfgTr181MapConvert = json_object_from_file(ZCFG_TR181_MAP_CONVERT))) {
		printf("%s: retrieve tr181 tr98-objnum converted data fail\n", __FUNCTION__);
		rst = ZCFG_INTERNAL_ERROR;
		goto complete;
	}

	zcfgTr181MapConverted = json_object_new_object();

	json_object_object_foreach(zcfgTr181MapConvert, tr181ObjNumStr, objnumJobj) {
		if(!objnumJobj || !strlen(tr181ObjNumStr))
			continue;

		json_object_object_del(objnumJobj, "name");

		uint32_t objnum = (uint32_t)atol(tr181ObjNumStr);
		uint32_t newTr181Objnum = zcfgTr181QueryNewSeqnum(zcfgTr181PvsNew, objnum);
		if(!newTr181Objnum){
			if(tr98181mappingConvertDebug)
				printf("%s: query %u new tr181 objnum fail\n", __FUNCTION__, objnum);
			rst = ZCFG_NO_SUCH_OBJECT;
			continue;
		}

		json_object_object_pull(zcfgTr181MapConvert, tr181ObjNumStr);
		sprintf(newTr181ObjNumStr, "%u", newTr181Objnum);
		json_object_object_add(zcfgTr181MapConverted, newTr181ObjNumStr, objnumJobj);
	}

	json_object_put(zcfgTr181MapConvert);

	if(json_object_to_file(ZCFG_TR181_MAP_CONVERTED, zcfgTr181MapConverted) < 0) {
		printf("%s: save tr98 tr181 data-converted fail\n", __FUNCTION__);
		remove(ZCFG_TR181_MAP_CONVERTED);
		rst = ZCFG_INTERNAL_ERROR;
	}

complete:
	if(!tr98181mappingConvertDebug)
		remove(ZCFG_TR181_MAP_CONVERT);

	json_object_put(zcfgTr181MapConverted);

	return rst;
}
#endif
// Tr98/Tr181 object mapping convert routines -- end
//----------------------------------------------------------------------------------------------------

#ifdef ZCFG_PARM_NAMEMAPPING
zcfgRet_t zcfgFe98ParmMappingToName(const char *notifyName, char *tr98NotifyName)
{
	char seqnumStr[30] = {0};
	char idxstr[30] = {0}, instNumStr[5];
	char parmName[MAX_PARMNAME_LEN] = {0};
	uint32_t seqnum = 0;
	char *c = NULL;
	objIndex_t tr98ObjIid;
	int n = 0, instNumStrLen = 0;

	if(!notifyName || !tr98NotifyName)
		return ZCFG_INTERNAL_ERROR;

	memset(instNumStr, 0, sizeof(instNumStr));

	*tr98NotifyName = '\0';

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	sscanf(notifyName, "%[^_]_%[^_]_%s", seqnumStr, idxstr, parmName);

	ztrdbg(ZTR_DL_DEBUG, "%s: %s\n", __FUNCTION__, notifyName);
	//zcfgLog(ZCFG_LOG_INFO, "%s: %s\n", __FUNCTION__, notifyName);

	seqnum = atoi(seqnumStr);
	if(!(IS_SEQNUM_RIGHT(seqnum)))
		return ZCFG_INTERNAL_ERROR;

	IID_INIT(tr98ObjIid);
	c = (idxstr + n);
	while(*c != '\0') {
			if(_isdigit(*c)) {
				instNumStr[instNumStrLen++] = *c;
				if((instNumStrLen+1) >= sizeof(instNumStr))
					return ZCFG_NO_SUCH_OBJECT;
			}
		else if(*c == '.' && instNumStrLen) {
			int idx = 0;
			if(tr98ObjIid.level < SUPPORTED_INDEX_LEVEL)
			tr98ObjIid.idx[tr98ObjIid.level++] = ((idx = atoi(instNumStr)) > 255) ? 255 : idx;
			instNumStrLen = 0;
			memset(instNumStr, 0, sizeof(instNumStr));
		}
		c = idxstr+(++n);
		if((*c == '\0') && instNumStrLen) {
			int idx = 0;
			if(tr98ObjIid.level < SUPPORTED_INDEX_LEVEL)
			tr98ObjIid.idx[tr98ObjIid.level++] = ((idx = atoi(instNumStr)) > 255) ? 255 : idx;
			instNumStrLen = 0;
			memset(instNumStr, 0, sizeof(instNumStr));
		}
	}

	if(zcfgFe98SeqnumToName(seqnum, &tr98ObjIid, tr98NotifyName) != ZCFG_SUCCESS) {
		zcfgLog(ZCFG_LOG_INFO, "%s: seqnum to name fail\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	if(strlen(parmName) > 0) {
		n = strlen(tr98NotifyName);
		*(tr98NotifyName + n++) = '.';
		*(tr98NotifyName + n) = '\0';
		strcat(tr98NotifyName, parmName);
	}

	zcfgLog(ZCFG_LOG_INFO, "%s: %s\n", __FUNCTION__, tr98NotifyName);

	return ZCFG_SUCCESS;
}
#endif

// Delete a term in csv string
char *csvStringStripTerm(const char *csv, const char *term)
{
	char *new_Csv = NULL;
	char *str_1 = NULL, *str_2 = NULL;
	int csvLen = strlen(csv);

	if(!csv || !term || (csvLen <= 0))
		return NULL;

	str_1 = strstr(csv, term);
	if(!str_1) {
		return NULL;
	}

	new_Csv = malloc(csvLen);
	new_Csv[0]='\0';

	if(str_1 == csv) {
		str_2 = str_1 + strlen(term);
		if(str_2[0] != '\0') {
			str_2++;
			strcpy(new_Csv, str_2);
		}
	}
	else {
		str_2 = str_1 + strlen(term);
		ZOS_STRNCPY(new_Csv, csv, (str_1-csv));
		if(str_2[0] != '\0') {
			strcat(new_Csv, str_2);
		}
	}

	return new_Csv;
}

bool strdigit(char *str)
{
	int n;
	bool strdigit = (str && *str != '\0');

	n = 0;
	while(*(str+n) != '\0' && strdigit){
		strdigit = _isdigit(*(str+n));
		n++;
	}

	return strdigit;
}

static bool objectPathToObjPathIdx(char *objectPathName, objPathIdx_t *objPathIdx)
{
	char *tmp = NULL;
	char *token = NULL;
	int n = 0, len = 0;
	char *objectPath = NULL;

	if(!objectPathName || !(len = strlen(objectPathName)) || len > 256)
		return 0;

	if(objectPathName[len-1] == '.')
		objectPathName[len-1] = '\0';


	int dataModelType = !strncmp(objectPathName, tr98objhead, strlen(tr98objhead)) ? DATA_MODEL_TR98 :
			(!strncmp(objectPathName, tr181objhead, strlen(tr181objhead)) ? DATA_MODEL_TR181 : DATA_MODEL_NOTDEFINED);
	if(dataModelType == DATA_MODEL_NOTDEFINED || dataModelType != runningDataModel)
		return 0;

	//printf("%s: %s\n", __func__, objectPathName);

	objPathIdx->objnum = 0;
	objectPath = malloc(len+3);
	memset(objectPath, 0, len+3);

	token = strtok_r(objectPathName, ".", &tmp);
	while(token) {

		if(!strcmp(token, "i")){
			objPathIdx->idx[n++] = -1;
			strcat(objectPath, "i");
		}else if(strdigit(token)){
			objPathIdx->idx[n++] = atoi(token);
			strcat(objectPath, "i");
		}else{
			strcat(objectPath, token);
		}

		token = strtok_r(NULL, ".", &tmp);
		if(token) strcat(objectPath, ".");
	}

	//printf("%s: objectPath: %s\n", __FUNCTION__, objectPath);
	int objectPathLen = strlen(objectPath);
	if(runningDataModel == DATA_MODEL_TR98){
		tr98NameObj_t *entry = json_object_object_get(tr98StaticNameTable, (const char *)objectPath);
		if(!entry && *(objectPath+objectPathLen-1) != 'i'){
			*(objectPath+objectPathLen) = '.';
			*(objectPath+objectPathLen+1) = 'i';
			objPathIdx->idx[n++] = -1;
			entry = json_object_object_get(tr98StaticNameTable, (const char *)objectPath);
		}
		objPathIdx->objnum = (entry && IS_SEQNUM_RIGHT((int)entry->nameSeqNum)) ? (int)entry->nameSeqNum : 0;
	}else{
		objIndex_t objIid;
		IID_INIT(objIid);
		int objnum = 0;
		if((objnum = zcfgFeObjNameToObjId(objectPath, &objIid)) < 0 && *(objectPath+objectPathLen-1) != 'i'){
			*(objectPath+objectPathLen) = '.';
			*(objectPath+objectPathLen+1) = 'i';
			objPathIdx->idx[n++] = -1;
			IID_INIT(objIid);
			objnum = zcfgFeObjNameToObjId(objectPath, &objIid);
		}
		objPathIdx->objnum = (objnum >= 0) ? objnum : 0;
	}

	if(objPathIdx->objnum){
		strcpy(objPathIdx->objpath, objectPath);
	}
	free(objectPath);

	//printf("%s: objPathIdx: %u, idx:%d,%d,%d,%d,%d,%d\n", __FUNCTION__, objPathIdx->objnum, objPathIdx->idx[0],
	//		objPathIdx->idx[1], objPathIdx->idx[2], objPathIdx->idx[3], objPathIdx->idx[4], objPathIdx->idx[5]);

	return objPathIdx->objnum ? 1 : 0;
}

void zcfgEnableRootQuryBlackObjPath(int enable)
{

	enableRootQuryBlackObjPath = enable;
}

// objectPathList: objpath1,objpath2,objpath3
// possibilities:
//    i=='i'/digits
//   objpath1=obj1.i.obj2.i.obj3
//   objpath2=obj1.n.obj2.i.obj3
//   objpath3=obj4.obj5.n.obj6.i
static bool zcfgObjectPathListToStruct(const char *objectPathList, int save, objPathIdx_t **objectPathIdxStructs)
{
	char *objectPathName, *tmp;
	char *objectPathListSave;
	int objPathListLen, nObjectPath = 0, n;
	objPathIdx_t *objPathIdxNew;

	if(!objectPathList || !(objPathListLen = strlen(objectPathList))){
		printf("%s: clear objectPathList\n", __FUNCTION__);
		if(*objectPathIdxStructs){

			ZOS_FREE(*objectPathIdxStructs);
		}
		return 0;
	}

	printf("%s: objectPathList: %s\n", __FUNCTION__, objectPathList);
	objectPathListSave = malloc(objPathListLen+1);
	memset(objectPathListSave, 0, objPathListLen+1);
	strcpy(objectPathListSave, objectPathList);

	objectPathName = strtok_r(objectPathListSave, ",", &tmp);
	while(objectPathName) {
		nObjectPath++;
		objectPathName = strtok_r(NULL, ",", &tmp);
	}

	objPathIdxNew = malloc(sizeof(objPathIdx_t)*(nObjectPath+1));
	memset(objPathIdxNew, 0, sizeof(objPathIdx_t)*(nObjectPath+1));
	strcpy(objectPathListSave, objectPathList);
	bool objPathIdxOk = 1;
	objectPathName = strtok_r(objectPathListSave, ",", &tmp);
	n = 0;
	while(objectPathName && objPathIdxOk) {
		objPathIdxOk = objectPathToObjPathIdx(objectPathName, &objPathIdxNew[n++]);
		objectPathName = strtok_r(NULL, ",", &tmp);
	}

	if(!objPathIdxOk){
		ZOS_FREE(objPathIdxNew);
		ZOS_FREE(objectPathListSave);
	}else if(!save){
		ZOS_FREE(objPathIdxNew);
		ZOS_FREE(objectPathListSave);
		return objPathIdxOk;
	}else{

		if(*objectPathIdxStructs)
		{
			ZOS_FREE(*objectPathIdxStructs);
		}
		*objectPathIdxStructs = objPathIdxNew;

		ZOS_FREE(objectPathListSave);
	}

	return objPathIdxOk;
}

static bool objPathIdxMatch(objPathIdx_t *objPathIdx, zcfg_name_t objnum, objIndex_t *objIid)
{
	int n = 0;

	//printf("%s: objnum %u %u\n", __func__, objPathIdx->objnum, objnum);

	if(objPathIdx->objnum != objnum)
		return 0;

	while(n != SUPPORTED_INDEX_LEVEL){
		if((objPathIdx->idx[n] == 0 && !objIid->idx[n]) ||
			(objPathIdx->idx[n] == -1 && objIid->idx[n]) ||
			(objPathIdx->idx[n] == objIid->idx[n]))
			n++;
		else
			return 0;
	}

	return 1;
}

static bool objectPathIdxMatch(objPathIdx_t *objPathIdx_1, objPathIdx_t *objPathIdx_2)
{
	//!objPathIdx_1 || !objPathIdx_2

	//printf("%s: %s %s, objnum %u %u\n", __func__, objPathIdx_1->objpath, objPathIdx_2->objpath, objPathIdx_1->objnum, objPathIdx_2->objnum);

	if(objPathIdx_1->objnum != objPathIdx_2->objnum){
		int l = 0;
		char *objPath_1, *objPath_2;
		char *tmp1, *tmp2;
		char *token1, *token2;
		int len, running, match = 0;

		len = strlen(objPathIdx_1->objpath);
		objPath_1 = ZOS_MALLOC(len+1);
		strcpy(objPath_1, objPathIdx_1->objpath);
		len = strlen(objPathIdx_2->objpath);
		objPath_2 = ZOS_MALLOC(len+1);
		strcpy(objPath_2, objPathIdx_2->objpath);
		token1 = strtok_r(objPath_1, ".", &tmp1);
		token2 = strtok_r(objPath_2, ".", &tmp2);
		running = (token1 && token2) ? 1 : 0;

		while(running) {
			if((!strcmp(token1, "i") || strdigit(token1)) &&
					(!strcmp(token2, "i") || strdigit(token2))){
				l++;
				match = ((objPathIdx_1->idx[l-1] == -1 && objPathIdx_2->idx[l-1]) ||
							(objPathIdx_1->idx[l-1] == objPathIdx_2->idx[l-1])) ? 1 : 0;
			}else{
				match = strcmp(token1, token2) ? 0 : 1;
			}

			if(match){
				token1 = strtok_r(NULL, ".", &tmp1);
				token2 = strtok_r(NULL, ".", &tmp2);
				running = (token1 && token2) ? 1 : 0;
				match = running ? match : ((token1 && !token2) ? 0 : match);
			}else{
				running = 0;
			}
		}

		ZOS_FREE(objPath_1);
		ZOS_FREE(objPath_2);

		return match;
	}else{
		int n = 0;

		while(n != SUPPORTED_INDEX_LEVEL){
			if((objPathIdx_1->idx[n] == 0 && !objPathIdx_2->idx[n]) ||
				(objPathIdx_1->idx[n] == -1 && objPathIdx_2->idx[n]) ||
				(objPathIdx_1->idx[n] == objPathIdx_2->idx[n]))
				n++;
			else
				return 0;
		}

		return 1;
	}
}

bool zcfgSetRootQueryNotReplyObjects(const char *objectPathList, int save)
{
	return zcfgObjectPathListToStruct(objectPathList, save, &rootQuryBlackObjPathIdx);
}

bool zcfgSetQueryNotReplyObjects(const char *objectPathList, int save)
{
	return zcfgObjectPathListToStruct(objectPathList, save, &quryBlackObjPathIdx);
}

// creat a data object containning 'objParamInfo'
struct json_object *zcfgFeTr181QueryHandleRoutAddDataObjParamInfo(struct json_object *objParamInfo)
{
	struct json_object *dataObject;

	dataObject = json_object_new_object();
	json_object_object_add(dataObject, "datatype", json_object_new_string("objparaminfo"));
	json_object_object_add(dataObject, "dataobject", objParamInfo);

	return dataObject;
}

struct json_object *zcfgFeTr181QueryHandleRoutPullDataObjParamInfo(struct json_object *dataObject)
{
	return json_object_object_pull(dataObject, "dataobject");
}

static struct json_object *zcfgFeTr181QueryHandleRoutRetrieveDataObjParamInfo(void *data)
{
	struct json_object *dataObject = NULL;

	const char *datatype = json_object_get_string(json_object_object_get((struct json_object *)data, "datatype"));
	if(datatype && !strcmp(datatype, "objparaminfo"))
		dataObject = json_object_object_get(data, "dataobject");

	return dataObject;
}

static bool zcfgQueryObjectPathIdxNotRepliedPathName(const char *objectPathName, objPathIdx_t *objectPathIdx)
{
	objIndex_t objIid;
	zcfg_name_t oid;
	objPathIdx_t *objPathIdx = NULL;
	int n = 0, objectPathNameLen = 0;
	objPathIdx_t objPathIdxSt = {0};
	char *objectPathNameDup = NULL;

	if(!objectPathName || !(objectPathNameLen = strlen(objectPathName)) || !objectPathIdx)
		return 0;

	int dataModelType = !strncmp(objectPathName, tr98objhead, strlen(tr98objhead)) ? DATA_MODEL_TR98 :
			(!strncmp(objectPathName, tr181objhead, strlen(tr181objhead)) ? DATA_MODEL_TR181 : DATA_MODEL_NOTDEFINED);
	if(dataModelType == DATA_MODEL_NOTDEFINED || dataModelType != runningDataModel)
		return 0;

	objectPathNameDup = malloc(objectPathNameLen+1);
	memset(objectPathNameDup, 0, objectPathNameLen+1);
	strcpy(objectPathNameDup, objectPathName);

	bool rst = objectPathToObjPathIdx(objectPathNameDup, &objPathIdxSt);
	free(objectPathNameDup);
	if(!rst) return rst;

	n = 0;
	objPathIdx = (objectPathIdx) ? &objectPathIdx[n++] : NULL;

	while(objPathIdx && objPathIdx->objnum){
		//if(objPathIdxMatch(objPathIdx, oid, &objIid))
		if(objectPathIdxMatch(objPathIdx, &objPathIdxSt))
			return 1;
		else
			objPathIdx = &objectPathIdx[n++];
	}

	return 0;
}

#if 0
bool zcfgObjectPathListPickObjPath(const char *objectPathName)
{
	objIndex_t tr98ObjIid;
	zcfg_name_t seqnum;
	objPathIdx_t *objPathIdx = NULL;
	int n = 0;

	if(!enableRootQuryBlackObjPath)
		return 0;

	IID_INIT(tr98ObjIid);
	if(zcfgFe98NameToSeqnum(objectPathName, &seqnum, &tr98ObjIid) != ZCFG_SUCCESS)
		return 0;

	//printf("%s: object: %u, idx:%hhu,%hhu,%hhu,%hhu,%hhu,%hhu\n", __FUNCTION__, seqnum, tr98ObjIid.idx[0],
	//		tr98ObjIid.idx[1], tr98ObjIid.idx[2], tr98ObjIid.idx[3], tr98ObjIid.idx[4], tr98ObjIid.idx[5]);

	objPathIdx = rootQuryBlackObjPathIdx ? &rootQuryBlackObjPathIdx[n++] : NULL;

	while(objPathIdx->objnum){
		if(objPathIdxMatch(objPathIdx, seqnum, &tr98ObjIid))
			return 1;
		else
			objPathIdx = &rootQuryBlackObjPathIdx[n++];
	}

	return 0;
}
#endif

// 1: obj's set as not replied on query
static bool zcfgQueryObjectPathIdxNotReplied(zcfg_offset_t oid, objIndex_t *objIid, objPathIdx_t *objectPathIdx)
{
	objPathIdx_t *objPathIdx = NULL;
	int n = 0, rst = 0;

	if(!objectPathIdx)
		return 0;

	//printf("%s: object: %u, idx:%hhu,%hhu,%hhu,%hhu,%hhu,%hhu\n", __FUNCTION__, seqnum, tr98ObjIid.idx[0],
	//		tr98ObjIid.idx[1], tr98ObjIid.idx[2], tr98ObjIid.idx[3], tr98ObjIid.idx[4], tr98ObjIid.idx[5]);

	objPathIdx = objectPathIdx ? &objectPathIdx[n++] : NULL;

    if (objectPathIdx == NULL)
    {
        return 0;
    }
    else
    {
        while (objPathIdx->objnum)
        {
            if (objPathIdxMatch(objPathIdx, oid, objIid))
            {
                return 1;
            }
            else
            {
                objPathIdx = &objectPathIdx[n++];
            }
        }
    }

	return 0;
}

zcfgRet_t zcfgQueryPathNameNotReplied(const char *objectPathName)
{
	bool rst;

	rst = zcfgQueryObjectPathIdxNotRepliedPathName(objectPathName, quryBlackObjPathIdx);
	if(rst)
		return ZCFG_NO_SUCH_OBJECT;
	//if(enableRootQuryBlackObjPath && rootQuryBlackObjPathIdx){
	//	rst = zcfgQueryObjectPathIdxNotRepliedPathName(objectPathName, rootQuryBlackObjPathIdx);
	//	if(rst)
	//		return ZCFG_NO_SUCH_OBJECT;
	//}
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgQueryObjectNotReplied(zcfg_offset_t oid, objIndex_t *objIid)
{
	bool rst;

	rst = zcfgQueryObjectPathIdxNotReplied(oid, objIid, quryBlackObjPathIdx);
	if(rst)
		return ZCFG_NO_SUCH_OBJECT;

	if(enableRootQuryBlackObjPath && rootQuryBlackObjPathIdx){
		rst = zcfgQueryObjectPathIdxNotReplied(oid, objIid, rootQuryBlackObjPathIdx);
		if(rst)
			return ZCFG_NO_SUCH_OBJECT;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98ObjNameGet(char *tr98ObjName, struct json_object **value)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);

	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

		if(tr98Obj[handler].getObj != NULL && newtr98ObjName[0] != '\0'){
			zcfgRet_t rst = tr98Obj[handler].getObj(newtr98ObjName, handler, value, TR98_GET_WITHOUT_UPDATE);
			return rst;
		}else if(tr98Obj[handler].getObj != NULL){
			zcfgRet_t rst = tr98Obj[handler].getObj(tr98ObjName, handler, value, TR98_GET_WITHOUT_UPDATE);
			return rst;
		}else
			return ZCFG_OBJECT_WITHOUT_PARAMETER;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98ObjNameObjGet(char *tr98ObjName, struct json_object **value)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	char result[256] = {0};
	char tr98ObjNameTmp[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char *ptr;
	struct json_object *obj = NULL;
	int queryMethod = 0;
	char seqStr[30] = {0};

	//Bryan,Wrong Parameter response by GPN
#if 0
	char *delim=".";
	int index = 0;
	char *token,*buf[10];
	char tr98_string[256];
	strcpy(tr98_string,tr98ObjName);
	token = strtok(tr98_string,delim);
	while(token != NULL){
		buf[index] = token;
		//printf("buf[%d] %s\n",index,buf[index]);
		index++;
		token = strtok(NULL,delim);
	}
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((zcfgFeTr98QryRootPath() && enableRootQuryBlackObjPath &&
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, rootQuryBlackObjPathIdx)) ||
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, quryBlackObjPathIdx))
		return ZCFG_NO_MORE_INSTANCE;


	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler == -1)
		return ZCFG_INVALID_OBJECT;

	tr98Parameter_t *parmList = tr98Obj[handler].parameter;
	if(!parmList)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;

	if (ReplaceWANDeviceIndex(tr98ObjName, tr98ObjNameTmp) != ZCFG_SUCCESS)
		return ZCFG_INVALID_OBJECT;

	const char *tr98ObjNamePtr = (tr98ObjNameTmp[0] != '\0') ? (const char *)tr98ObjNameTmp : (const char *)tr98ObjName;
	zcfg_name_t seqnum;
	objIndex_t tr98ObjIid;
	IID_INIT(tr98ObjIid);
	if((ret = zcfgFe98NameToSeqnum(tr98ObjNamePtr, &seqnum, &tr98ObjIid)) != ZCFG_SUCCESS){
		return ZCFG_INVALID_OBJECT;
	}

	if(zcfgTr98GpnSpeedObjs){
		struct json_object *speedObj = zcfgTr98SpeedObjsRetrieve(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid);
		if(speedObj && value){
			ztrdbg(ZTR_DL_INFO, "%s: speedobj %s retrieved\n", __FUNCTION__, tr98ObjName);
			parmListSetObjParamNameAttr(&speedObj, parmList);
			*value = speedObj;

			return ZCFG_SUCCESS;
		}
	}

	sprintf(seqStr, "%u", handler);
	tr98QryHandleRout gpnQueryHdlRout = zcfgTr98GpnHandleRoutPick(seqStr);
	if( strcmp( tr98Obj[handler].name, TR98_LAN_DEV_WLAN_CFG)==0){
		/*hide MESH preset & backhaul SSIDs via handler */
		ret = tr98Obj[handler].getObj(tr98ObjNamePtr, handler, &obj, TR98_GET_WITHOUT_UPDATE);
		queryMethod = 1;
	}else if(gpnQueryHdlRout){
		if(value)
			ret = gpnQueryHdlRout((const char *)tr98ObjNamePtr, handler, &tr98ObjIid, &obj, NULL);
		else
			ret = gpnQueryHdlRout((const char *)tr98ObjNamePtr, handler, &tr98ObjIid, NULL, NULL);
		queryMethod = 2;
	}else{
		ret = zcfgFe98To181MappingNameGet(tr98ObjNamePtr, result);
		queryMethod = 3;
	}

	if(queryMethod == 1){
		if(ret == ZCFG_SUCCESS && value){
			*value = obj;
			if(zcfgTr98GpnSpeedObjs){
				parmListSetObjParamNameAttr(&obj, parmList);
				zcfgTr98SpeedObjsAddObj(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid, obj);
			}
		}else{
			json_object_put(obj);
		}
		return ret;
	}else if(queryMethod == 2){
		if(ret == ZCFG_SUCCESS && value){
			*value = obj;
			if(zcfgTr98GpnSpeedObjs){
				zcfgTr98SpeedObjsAddObj(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid, obj);
			}
		}else{
			json_object_put(obj);
		}
		return ret;
	}else if(queryMethod == 3){
		if( (ret == ZCFG_SUCCESS)){
			if(zcfgTr98GpnSpeedObjs && value){
				parmListSetObjParamNameAttr(&obj, parmList);
				zcfgTr98SpeedObjsAddObj(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid, obj);
				*value = obj;
			}
			return ret;
		}else {
			if(!tr98Obj[handler].getObj)
				return ZCFG_OBJECT_WITHOUT_PARAMETER;

			ret = tr98Obj[handler].getObj(tr98ObjNamePtr, handler, &obj, TR98_GET_WITHOUT_UPDATE);
			if(ret == ZCFG_SUCCESS && value){
				*value = obj;
				if(zcfgTr98GpnSpeedObjs){
					parmListSetObjParamNameAttr(&obj, parmList);
					zcfgTr98SpeedObjsAddObj(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid, obj);
				}
			}else{
				json_object_put(obj);
			}
			return ret;
		}
	}

#if 0
	if(handler != -1) {
		if(tr98Obj[handler].getObj != NULL){
			//special case
			strcpy( tr98ObjNameTmp, tr98ObjName );
			if (ReplaceWANDeviceIndex(tr98ObjName, tr98ObjNameTmp) != ZCFG_SUCCESS)
				return ZCFG_INVALID_OBJECT;

			if( strcmp( tr98Obj[handler].name, TR98_LAN_HOST_CONF_MGMT) == 0 ){
				ptr = strstr(tr98ObjNameTmp, ".LANHostConfigManagement");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_LAN_ETH_INTF_CONF_STAT)==0){
				ptr = strstr(tr98ObjNameTmp, ".Stats");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WLAN_CFG_STAT)==0){
				ptr = strstr(tr98ObjNameTmp, ".Stats");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WPS)==0){
				ptr = strstr(tr98ObjNameTmp, ".WPS");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_LAN_DEV_WLAN_CFG)==0){
				/*hide MESH preset & backhaul SSIDs via handler */
				return tr98Obj[handler].getObj(tr98ObjName, handler, value, TR98_GET_WITHOUT_UPDATE);
			}
			else if(strcmp(tr98Obj[handler].name, TR98_WAN_COMM_INTF_CONF)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANCommonInterfaceConfig");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
#if 0
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_DSL_INTF_CONF)==0){
				if(!strcmp(buf[2],"3")){ //Bryan,Wrong Parameter response by GPN
					return ZCFG_NO_SUCH_OBJECT;
				}
				ptr = strstr(tr98ObjNameTmp, ".WANDSLInterfaceConfig");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
#endif
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_DSL_INTF_TEST_PARAM)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANDSLInterfaceConfig.TestParams");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_DSL_DIAG)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANDSLDiagnostics");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_DSL_LINK_CONF)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANDSLLinkConfig");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_ATM_F5_LO_DIAG)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANATMF5LoopbackDiagnostics");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_ATM_F4_LO_DIAG)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_WANATMF4LoopbackDiagnostics");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_DHCP_CLIENT)==0){
				ptr = strstr(tr98ObjNameTmp, ".DHCPClient");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_IP_INTF_ROUTERADVER)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_RouterAdvertisement");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_IP_INTF_DHCPV6SRV)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_DHCPv6Server");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_PTM_LINK_CONF)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANPTMLinkConfig");
				*ptr = '\0';
				if( zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result) == ZCFG_SUCCESS ){					
					if(strstr(result, "PTM") == NULL) {
						return ZCFG_NO_SUCH_OBJECT;
					}
					return ZCFG_SUCCESS;
				}
				else
					return ZCFG_NO_SUCH_OBJECT;
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_PTM_LINK_CONF_STAT)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANPTMLinkConfig.Stats");
				*ptr = '\0';
				if( zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result) == ZCFG_SUCCESS ){					
					if(strstr(result, "PTM") == NULL) {
						return ZCFG_NO_SUCH_OBJECT;
					}
					return ZCFG_SUCCESS;
				}
				else
					return ZCFG_NO_SUCH_OBJECT;
					
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_ETH_LINK_CONF)==0){
				ptr = strstr(tr98ObjNameTmp, ".WANEthernetLinkConfig");
				*ptr = '\0';
				if( zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result) == ZCFG_SUCCESS ){					
					if(strstr(result, "Ethernet") == NULL) {
						return ZCFG_NO_SUCH_OBJECT;
					}
					return ZCFG_SUCCESS;
				}
				else
					return ZCFG_NO_SUCH_OBJECT;
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_IP_CONN_STAT)==0){
				ptr = strstr(tr98ObjNameTmp, ".Stats");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_PPP_CONN_STAT)==0){
				ptr = strstr(tr98ObjNameTmp, ".Stats");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_IP_CONN_DHCPV6CLIENT)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_DHCPv6Client");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_PPP_CONN_DHCPV6CLIENT)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_DHCPv6Client");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_IP_CONN_IPV6RD)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_IPv6rd");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_PPP_CONN_IPV6RD)==0){
				ptr = strstr(tr98ObjNameTmp, ".X_ZYXEL_IPv6rd");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}			
#if 0
			else if( strcmp( tr98Obj[handler].name, TR98_WAN_ETH_INTF_CONF)==0){
				if(!strcmp(buf[2],"1") || !strcmp(buf[2],"2")) //Bryan,Wrong Parameter response by GPN
					return ZCFG_NO_SUCH_OBJECT;
				ptr = strstr(tr98ObjNameTmp, ".WANEthernetInterfaceConfig");
				*ptr = '\0';
				return zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			}
#endif
			memset(tr98ObjNameTmp, 0, sizeof(tr98ObjNameTmp));
			if (ReplaceWANDeviceIndex((const char *)tr98ObjName, tr98ObjNameTmp) != ZCFG_SUCCESS)
				return ZCFG_INTERNAL_ERROR;

			if(tr98ObjNameTmp[0] != '\0')
				ret = zcfgFe98To181MappingNameGet(tr98ObjNameTmp, result);
			else
				ret = zcfgFe98To181MappingNameGet(tr98ObjName, result);

			if( (ret == ZCFG_SUCCESS))
				return ret;
			else {
				return tr98Obj[handler].getObj(tr98ObjName, handler, value, TR98_GET_WITHOUT_UPDATE);
			}
		}
		else
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

	}
	else {
		return ZCFG_INVALID_OBJECT;
	}
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98ObjParamSet(char *paramName, uint32_t type, struct json_object *setJobj, char *value)
{
	struct json_object *setValue = NULL;
	
	if(setJobj == NULL) {
		zcfgLog(ZCFG_LOG_ERR, "The setJobj pointer is NULL value\n");
		return ZCFG_INTERNAL_ERROR;
	}

	switch (type) {
		case json_type_boolean:
			setValue = json_object_new_boolean(atoi(value));
			break;
		case json_type_int:
			setValue = json_object_new_int(atoi(value));
			break;
		case json_type_uint8:
		case json_type_uint16:
		case json_type_uint32:
		case json_type_ulong:
			// setValue = json_object_new_int((uint32_t)atoi(value));
			setValue = json_object_new_int(strtoul(value, NULL, 10));
			break;
		case json_type_base64:
			{
				char *modifiedValue = base64StrParticularCharHandle((const char *)value);
				char *optValue = NULL;
				int decodeOptValueLen = 0;
				if(modifiedValue){
					optValue = modifiedValue;
				}else{
					int valueLen = strlen(value);
					if(!valueLen){
						setValue = json_object_new_string("");
						break;
					}
					optValue = malloc(valueLen+1);
					ZOS_STRNCPY(optValue, value, (valueLen+1));
				}
				char *decodeOptValue = base64_decode((unsigned char *)optValue, &decodeOptValueLen);
				printf("%s: base64 decoded: %s\n", __FUNCTION__, decodeOptValue ? decodeOptValue : "null" );

				setValue = json_object_new_string(decodeOptValue);

				ZOS_FREE(optValue);
				ZOS_FREE(decodeOptValue);
			}
			break;
		case json_type_string:
			setValue = json_object_new_string(value);
			break;
		case json_type_time:
			setValue = json_object_new_string(value);
			break;
		default:
			zcfgLog(ZCFG_LOG_ERR, "Type not support\n");
			return ZCFG_ERROR_PARAMETER_TYPE;
			break;
	}

	json_object_object_add(setJobj, paramName, setValue);
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98ObjSet(char *tr98ObjName, struct json_object *value, char *pfault)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	//printf("value %s\n", json_object_to_json_string(value));

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

		if(tr98Obj[handler].setObj != NULL && newtr98ObjName[0] != '\0')
			return tr98Obj[handler].setObj(newtr98ObjName, handler, value, NULL, pfault);
		else if(tr98Obj[handler].setObj != NULL)
			return tr98Obj[handler].setObj(tr98ObjName, handler, value, NULL, pfault);
		else
			return ZCFG_SET_READ_ONLY_PARAM;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98MultiObjSet(char *tr98ObjName, struct json_object *value, struct json_object *multiJobj, char *paramfault)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

		if(tr98Obj[handler].setObj != NULL && newtr98ObjName[0] != '\0'){
			return tr98Obj[handler].setObj(newtr98ObjName, handler, value, multiJobj, paramfault);}
		else if(tr98Obj[handler].setObj != NULL){
			return tr98Obj[handler].setObj(tr98ObjName, handler, value, multiJobj, paramfault);}
		else
			return ZCFG_SET_READ_ONLY_PARAM;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98ObjAdd(char *tr98ObjName, int *idx)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
#ifdef CONFIG_TAAAB_DSL_BINDING
    char *search = NULL;
    char *oriindex = NULL;
    char synctr98ObjName[128] = {0};
	int index = 0;
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

		if(!(tr98Obj[handler].attr & OBJECT_ATTR_CREATE)){
			return ZCFG_INVALID_ARGUMENTS;
		}

		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

#ifdef CONFIG_TAAAB_DSL_BINDING
					if((handler == (e_TR98_WAN_IP_CONN - e_TR98_START)) || 
					   (handler == (e_TR98_WAN_PPP_CONN - e_TR98_START)) ||
					   (handler == (e_TR98_WAN_CONN_DEV - e_TR98_START)) ){
						strcpy(synctr98ObjName,newtr98ObjName);		
						if(!strncmp(synctr98ObjName,"InternetGatewayDevice.WANDevice.",32) && strlen(synctr98ObjName)>33)
						{
						  search = strstr(synctr98ObjName, "InternetGatewayDevice.WANDevice.");
						  oriindex = search + 32;
						
						  if(*oriindex == '1'){   //it is ADSL now, it need to also add object on VDSL.
								 *(search + 32) = '2';
								 if(tr98Obj[handler].addObj != NULL)
									tr98Obj[handler].addObj(synctr98ObjName, &index);
						  }
						  else if(*oriindex == '2'){  //it is VDSL now, it need to also add object on ADSL.
							 *(search + 32) = '1';
							 if(tr98Obj[handler].addObj != NULL)
								tr98Obj[handler].addObj(synctr98ObjName, &index);
						  }
						  else{
							  //do nothing for eth & other wantype add
						  } 			  
						}
						else
							return ZCFG_INVALID_ARGUMENTS;
					}
#endif

		if(tr98Obj[handler].addObj != NULL && newtr98ObjName[0] != '\0')
			return tr98Obj[handler].addObj(newtr98ObjName, idx);
		else if(tr98Obj[handler].addObj != NULL)
			return tr98Obj[handler].addObj(tr98ObjName, idx);
		else
			return ZCFG_INVALID_ARGUMENTS;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}

	return ZCFG_SUCCESS;	
}

zcfgRet_t zcfgFeTr98ObjDel(char *tr98ObjName)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
#ifdef CONFIG_TAAAB_DSL_BINDING
	char *search = NULL;
	char *oriindex = NULL;
	char synctr98ObjName[128] = {0};
#endif

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

		if(!(tr98Obj[handler].attr & OBJECT_ATTR_CREATE)){
			return ZCFG_INVALID_ARGUMENTS;
		}

		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

#ifdef CONFIG_TAAAB_DSL_BINDING
			if((handler == (e_TR98_WAN_IP_CONN - e_TR98_START)) ||
			   (handler == (e_TR98_WAN_PPP_CONN - e_TR98_START)) ||
			   (handler == (e_TR98_WAN_CONN_DEV - e_TR98_START))){
				strcpy(synctr98ObjName,newtr98ObjName);		
				if(!strncmp(synctr98ObjName,"InternetGatewayDevice.WANDevice.",32) && strlen(synctr98ObjName)>33)
				{
						search = strstr(synctr98ObjName, "InternetGatewayDevice.WANDevice.");
						oriindex = search + 32;
								
						if(*oriindex == '1'){	// it is ADSL now, it need to also delete object on VDSL.
							*(search + 32) = '2';
							if(tr98Obj[handler].delObj != NULL)
							   tr98Obj[handler].delObj(synctr98ObjName);
						}
						else if(*oriindex == '2'){	//it is VDSL now, it need to also delete object on ADSL.
							*(search + 32) = '1';
							if(tr98Obj[handler].delObj != NULL)
							   tr98Obj[handler].delObj(synctr98ObjName);
						}
						else{	
						   //do nothing for eth & other wantype del
						}
				}
				else
					return ZCFG_INVALID_ARGUMENTS;	
			}
#endif

		if(tr98Obj[handler].delObj != NULL && newtr98ObjName[0] != '\0')
			return tr98Obj[handler].delObj(newtr98ObjName);
		else if(tr98Obj[handler].delObj != NULL)
			return tr98Obj[handler].delObj(tr98ObjName);
		else
			return ZCFG_INVALID_ARGUMENTS;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98DefaultValueSet(struct json_object *tr98Jobj, char *paramName, int type)
{
	switch(type) {
		case json_type_uint8:
		case json_type_uint16:
		case json_type_uint32:
		case json_type_int:
		case json_type_ulong:
			json_object_object_add(tr98Jobj, paramName, json_object_new_int(0));
			break;
		case json_type_string:
			json_object_object_add(tr98Jobj, paramName, json_object_new_string(""));
			break;
		case json_type_boolean:
			json_object_object_add(tr98Jobj, paramName, json_object_new_boolean(false));
			break;
		default:
			printf("%s : Unknown Type\n", __FUNCTION__);
			break;
	}

	return ZCFG_SUCCESS;
}

int zcfgFeTr98ObjAttrGet(char *tr98ObjName)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}
		
	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		return tr98Obj[handler].attr;
	}
	else {
		return ZCFG_INVALID_OBJECT;
	}
	
}

#if 1 //__ZYXEL__, Mark
zcfgRet_t zcfgFeGetTr181NameParm(const char *notifyName, char *objName, char *objParm){
	char *ptr = NULL;
	const char *ptr2 = NULL;
	int n_Len, n_Len2;
	char *IpIntObjName = "IP.Interface";
	char *PppIntObjName = "PPP.Interface";
#if 1 //__ZYXEL__, Mark, 20140512	
	objIndex_t ipInfIid;
	rdm_IpIface_t *ipIntfObj = NULL;
#endif
	
	if(notifyName == NULL || objName== NULL || objParm == NULL) return ZCFG_INVALID_ARGUMENTS;

	ztrdbg(ZTR_DL_INFO, "%s: notifyName: %s\n", __FUNCTION__, notifyName);
	
	*objName = '\0';
	*objParm = '\0';
	
	ptr = strrchr(notifyName, '.');
	if(ptr == NULL) {
		printf("Invalid arguments of %s\n", notifyName);
		return ZCFG_INVALID_ARGUMENTS;
	}
	strcpy(objParm, ptr+1);
	
	if(!strncmp(notifyName, "Device.", strlen("Device."))){
		ptr2 = notifyName + strlen("Device.");
		n_Len = ptr-ptr2;
		if(n_Len) strncpy(objName, ptr2, n_Len);
	}
	else {
		n_Len = ptr-notifyName;
		if(n_Len) strncpy(objName, notifyName, n_Len);
	}
	
	n_Len = strlen(IpIntObjName);
	n_Len2 = strlen(PppIntObjName);
	ptr = NULL;
	if(!strncmp(objName, IpIntObjName, n_Len)){
		if((ptr = strstr(objName+n_Len, "IPv4Address")) && *(ptr-1) == '.'){
			*(ptr-1) = '\0';
		}
#if 1 //__ZYXEL__, Mark, 20140512, mapping IP.Interface.i.IPv4Address.IPAddress to PPP.Interface.i.IPCP.LocalIPAddress
		if(!strcmp(objParm, "IPAddress")){
			IID_INIT(ipInfIid);
			ipInfIid.level = 1;
			sscanf(objName, "IP.Interface.%hhu", &ipInfIid.idx[0]);
			if( zcfgFeObjStructGetWithoutUpdate(RDM_OID_IP_IFACE, &ipInfIid, (void **)&ipIntfObj) == ZCFG_SUCCESS){
				printf("%s: ipIntfObj->LowerLayers=%s\n",__FUNCTION__, ipIntfObj->LowerLayers);
				if(!strncmp(ipIntfObj->LowerLayers, PppIntObjName, n_Len2)){
					strcpy(objName, ipIntfObj->LowerLayers);
					strcpy(objParm, "LocalIPAddress");
				}
				zcfgFeObjStructFree(ipIntfObj);
			}
		}
#endif
	}
	else if(!strncmp(objName, PppIntObjName, n_Len2)){
		if((ptr = strstr(objName+n_Len2, "IPCP")) && *(ptr-1) == '.'){
			*(ptr-1) = '\0';
		}
	}
	
	ztrdbg(ZTR_DL_DEBUG, "%s: notifyName=%s, objName=%s, objParm=%s\n", __FUNCTION__, notifyName, objName, objParm);
	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t zcfgFeTr98Notify(struct json_object *tr181NotifyInfo, struct json_object **tr98NotifyInfo)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	char tr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	char tr181ObjName[128] = {0};
	char tr181ParamName[64] = {0};
	char tr98ClassName[128] = {0};
	char *token = NULL, *tmp = NULL;
	int handler = 0;

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if(tr181NotifyInfo == NULL && tr98NotifyInfo == NULL) {
		printf("Invalid arguments\n");
		return ZCFG_INVALID_ARGUMENTS;
	}

	json_object_object_foreach(tr181NotifyInfo, tr181NotifyName, tr181ParamVal) {
		/*Get object path name*/
#if 0 //__ZYXEL__, Mark
		strcpy(tr181ObjName, tr181NotifyName);
		ptr = strrchr(tr181ObjName, '.');
		if(ptr == NULL) {
			printf("Invalid arguments of %s\n", tr181NotifyName);
			return ZCFG_INVALID_ARGUMENTS;
		}
		strcpy(tr181ParamName, ptr+1);

		*ptr = '\0';
#else
		if(!strncmp(tr181NotifyName, (const char *)"Device", strlen("Device"))) {
		if(zcfgFeGetTr181NameParm(tr181NotifyName, tr181ObjName, tr181ParamName) != ZCFG_SUCCESS)
			continue;
			
		ztrdbg(ZTR_DL_DEBUG, "%s: tr181NotifyName=%s, tr181ObjName=%s, tr181ParamName=%s\n", __FUNCTION__, tr181NotifyName, tr181ObjName, tr181ParamName);
#endif
			if((ret = zcfgFe181To98MappingNameGet(tr181ObjName, tr98ObjName)) != ZCFG_SUCCESS)
				continue;

			printf("TR98 Object Name %s\n", tr98ObjName);
	
			if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
				return ret;
			}

			handler = zcfgFeTr98HandlerGet(tr98ClassName);
			if(handler != -1) {
				if(tr98Obj[handler].notify != NULL)
					ret = tr98Obj[handler].notify(tr98ObjName, tr181ParamName, tr181ParamVal, handler, tr98NotifyInfo);
			}
		}
#ifdef ZCFG_PARM_NAMEMAPPING
		else if(!strncmp(tr181NotifyName, (const char *)TR98MAPPING_PREFIX, strlen(TR98MAPPING_PREFIX))) {
			const char *notifyName = tr181NotifyName + strlen(TR98MAPPING_PREFIX);
			char *tr98NotifyName = (char *)malloc(TR98_MAX_OBJ_NAME_LENGTH + MAX_PARMNAME_LEN);

			memset(tr98NotifyName, 0, TR98_MAX_OBJ_NAME_LENGTH + MAX_PARMNAME_LEN);
			if((ret = zcfgFe98ParmMappingToName(notifyName, tr98NotifyName)) != ZCFG_SUCCESS) {

				ZOS_FREE(tr98NotifyName);
				return ret;
			}

			if(strlen(tr98NotifyName) > 0) {
				if(*tr98NotifyInfo == NULL)
				*tr98NotifyInfo = json_object_new_object();

			json_object_object_add(*tr98NotifyInfo, tr98NotifyName, JSON_OBJ_COPY(tr181ParamVal));
			}

			ZOS_FREE(tr98NotifyName);
		}
#endif
		else { /*Not found, maybe one by one mapping*/
			/*Replace Device to InternetGatewayDevice */
			token = strtok_r(tr181ObjName, ".", &tmp);
			if(token != NULL && !strcmp(token, "Device")) {
				strcpy(tr98ObjName, "InternetGatewayDevice.");
				strcat(tr98ObjName, tmp);
			}
			else {
				printf("Invalid arguments of %s\n", tr181NotifyName);
				return ZCFG_INVALID_ARGUMENTS;
			}
			
			if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
				return ret;
			}

			handler = zcfgFeTr98HandlerGet(tr98ClassName);
			if(handler != -1) {
				if(tr98Obj[handler].notify != NULL)
					ret = tr98Obj[handler].notify(tr98ObjName, tr181ParamName, tr181ParamVal, handler, tr98NotifyInfo);
			}
		}
	}

	return ret;
}

void zcfgDumpTr98ObjParam()
{
	int n = e_TR98_IGD;
	int nthObj = n-e_TR98_START;
	FILE *pu = fopen("/data/tr98objparamlist", "w+");

	while(nthObj != (e_TR98_LAST-e_TR98_START)){
		// nthObj 'obj name'
		fprintf(pu, "\n\n%s: tr98 object: %u, name: %s\n", __FUNCTION__, nthObj, tr98Obj[nthObj].name);

		tr98Parameter_t *paramList = tr98Obj[nthObj].parameter;
		while(tr98Obj[nthObj].parameter && paramList->name != NULL) {
			fprintf(pu, "parameter name %s\n", paramList->name);
			paramList++;
		}
		nthObj++;
	}
	fclose(pu);
}

zcfgRet_t zcfgFeTr98ParamList(char *tr98ObjName)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	ztrdbg(ZTR_DL_INFO, "%s: tr98ClassName %s\n", __FUNCTION__, tr98ClassName);

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;
		if(paramList == NULL)
		{
			return ZCFG_SUCCESS;
		}
		while(paramList->name != NULL) {
			printf("parameter name %s\n", paramList->name);
			paramList++;
		}
	}
	
	return ZCFG_SUCCESS;
}

int zcfgFeTr98ParamAttrChk(int paramAttr, int notification)
{

	if((paramAttr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST) && (notification == 2)) {
		printf("%s: ActiveNotify request is denied!\n", __FUNCTION__);		
		return ZCFG_REQUEST_REJECT;
	}
	else if((paramAttr & PARAMETER_ATTR_DENY_PASSIVENOTIFY_REQUEST) && (notification == 1)) {
		printf("%s: PassiveNotify request is denied!\n", __FUNCTION__);		
		return ZCFG_REQUEST_REJECT;
	}
	else if((paramAttr & PARAMETER_ATTR_DENY_DISABLENOTIFY_REQUEST) && (notification == 0)) {
		printf("%s: DisableNotify request is denied!\n", __FUNCTION__);		
		return ZCFG_REQUEST_REJECT;
	}

	return ZCFG_SUCCESS;
}

void zcfgRpcTr98GpnSpeedObjsEnable()
{
	if(!zcfgTr98GpnSpeedObjs){
		zcfgTr98GpnSpeedObjs = json_object_new_array_list((e_TR98_LAST-e_TR98_START)+20);
	}
}

void zcfgRpcTr98GpnSpeedObjsClear()
{
	if(zcfgTr98GpnSpeedObjs){
		json_object_put(zcfgTr98GpnSpeedObjs);
		zcfgTr98GpnSpeedObjs = NULL;
	}

}

bool zcfgRpcTr98GpnSpeedObjsUtilized()
{
	return zcfgTr98GpnSpeedObjs ? true : false;
}


void zcfgRpcTr98GpvSpeedObjsEnable()
{
	if(!zcfgTr98GpvSpeedObjs){
		zcfgTr98GpvSpeedObjs = json_object_new_array_list((e_TR98_LAST-e_TR98_START)+20);
	}
}

void zcfgRpcTr98GpvSpeedObjsClear()
{
	if(zcfgTr98GpvSpeedObjs){
		json_object_put(zcfgTr98GpvSpeedObjs);
		zcfgTr98GpvSpeedObjs = NULL;
	}

}

bool zcfgRpcTr98GpvSpeedObjsUtilized()
{
	return zcfgTr98GpvSpeedObjs ? true : false;
}

static zcfgRet_t zcfgObjIidToMappingName(uint32_t objnum, objIndex_t *objIid, char *mappingName)
{
	char idxstr[5];
	int n = 0;

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	if(!objIid || !mappingName)
		return ZCFG_INTERNAL_ERROR;

	if(objnum)
		sprintf(mappingName, "%u_", objnum);

	n = 0;
	if(objIid->idx[n] == 0) {
		strcat(mappingName, "-");
	}
	else {
		while(n < SUPPORTED_INDEX_LEVEL)
		{
			if(objIid->idx[n] == 0)
				break;

			memset(idxstr, 0, sizeof(idxstr));
			if(!n)
				sprintf(idxstr, "%d", objIid->idx[n]);
			else
				sprintf(idxstr, ".%d", objIid->idx[n]);

			strcat(mappingName, idxstr);
			n++;
		}
	}

	ztrdbg(ZTR_DL_INFO, "%s: mapping name, %s\n", __FUNCTION__, MSG_EMPTY(mappingName));

	return ZCFG_SUCCESS;
}

static void zcfgTr98ObjSetParmAttr(struct json_object *tr98parm, struct json_object *objParamsAttr)
{
	struct json_object *obj, *attrObj;

	if(!tr98parm || !objParamsAttr)
		return;

	json_object_object_foreach(tr98parm, parmName, valueObj) {
		int attr = (obj = json_object_object_get(objParamsAttr, parmName)) ? ((attrObj=json_object_object_get(obj, "attr")) ? json_object_get_int(attrObj) : -1) : -1;
		if(attr >= 0){
			json_object_object_add(tr98parm, parmName, json_object_new_int(attr));
		}else{
			attr = json_object_get_int(valueObj);
			if(attr & PARAMETER_ATTR_DEFAULTATTR) {
				if((attr & PARAMETER_ATTR_DENY_DISABLENOTIFY_REQUEST) &&
									(attr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST))
					attr = 1;

				if((attr & PARAMETER_ATTR_DENY_DISABLENOTIFY_REQUEST) &&
									(attr & PARAMETER_ATTR_DENY_PASSIVENOTIFY_REQUEST))
					attr = 2;

				if((attr & PARAMETER_ATTR_DENY_PASSIVENOTIFY_REQUEST) &&
									(attr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST))
					attr = 0;

				json_object_object_add(tr98parm, parmName, json_object_new_int(attr));
			}
		}
	}
}

static zcfgRet_t parmListSetObjParamNameAttr(struct json_object **tr98parm, tr98Parameter_t *parmList)
{
	struct json_object *paramObj = NULL;
	int attr;

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	if(*tr98parm == NULL)
	{
		*tr98parm = json_object_new_object();
	}else{
		int attr = json_object_get_int(json_object_object_get(*tr98parm, "attr"));
		if(attr != 0){  // ZCFG_GPN_SPEED_OBJ_ATTR_ADDED
			return ZCFG_SUCCESS;
		}
	}

	int nparam = json_object_object_length(*tr98parm);

	while(parmList && parmList->name){
		if(nparam){
			if(json_object_object_get(*tr98parm, parmList->name))
				json_object_object_add(*tr98parm, parmList->name, json_object_new_int(parmList->attr));
		}else{
			json_object_object_add(*tr98parm, parmList->name, json_object_new_int(parmList->attr));
		}
		parmList++;
	}
	json_object_object_add(*tr98parm, "attr", json_object_new_int(1));

	return ZCFG_SUCCESS;
}

//static struct json_object *retrieveAttrListObj(struct json_object *zcmdTr98MapAttrList, zcfg_name_t seqnum, objIndex_t *tr98ObjIid)
static struct json_object *zcfgTr98SpeedObjsRetrieve(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid)
{
	struct json_object *arrayObj = NULL, *tr98MappingIdxObj = NULL;
	uint32_t arrayIdx;
	char tr98ObjIidStr[20] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	arrayIdx = ARRAY_SEQNUM(seqnum);
	if((arrayObj = json_object_array_get_idx(tr98SpeedObjs, arrayIdx)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no this object\n", __FUNCTION__);
		return NULL;
	}

	zcfgObjIidToMappingName(0, tr98ObjIid, tr98ObjIidStr);
	if((tr98MappingIdxObj = json_object_object_get(arrayObj, tr98ObjIidStr)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no index object\n", __FUNCTION__);

	}

	return tr98MappingIdxObj;
}

// return obj and delete obj entry
static struct json_object *zcfgTr98SpeedObjsRetObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid)
{
	struct json_object *arrayObj = NULL, *tr98MappingIdxObj = NULL;
	uint32_t arrayIdx;
	char tr98ObjIidStr[20] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);
	arrayIdx = ARRAY_SEQNUM(seqnum);
	if((arrayObj = json_object_array_get_idx(tr98SpeedObjs, arrayIdx)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no this object\n", __FUNCTION__);
		return NULL;
	}

	zcfgObjIidToMappingName(0, tr98ObjIid, tr98ObjIidStr);
	if((tr98MappingIdxObj = json_object_object_pull(arrayObj, tr98ObjIidStr)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no index object\n", __FUNCTION__);
	}

	ztrdbg(ZTR_DL_INFO, "%s: obj %u \n", __FUNCTION__, seqnum);

	return tr98MappingIdxObj;
}

static zcfgRet_t zcfgTr98SpeedObjsAddObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid, struct json_object *obj)
{
	struct json_object *arrayObj = NULL;
	uint32_t arrayIdx;
	char tr98ObjIidStr[20] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if(!tr98SpeedObjs || !(IS_SEQNUM_RIGHT(seqnum)) || !tr98ObjIid)
		return ZCFG_INTERNAL_ERROR;

	arrayIdx = ARRAY_SEQNUM(seqnum);
	if((arrayObj = json_object_array_get_idx(tr98SpeedObjs, arrayIdx)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no this object\n", __FUNCTION__);
		arrayObj = json_object_new_object();
		json_object_array_put_idx(tr98SpeedObjs, arrayIdx, arrayObj);
	}
	zcfgObjIidToMappingName(0, tr98ObjIid, tr98ObjIidStr);

	if(!obj)
		return ZCFG_INTERNAL_ERROR;

	ztrdbg(ZTR_DL_INFO,  "%s: obj %u\n", __FUNCTION__, seqnum);
	json_object_object_add(arrayObj, (const char *)tr98ObjIidStr, obj);

	return ZCFG_SUCCESS;
}

static zcfgRet_t zcfgTr98SpeedObjsDeleteObj(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid)
{
	struct json_object *arrayObj = NULL;
	uint32_t arrayIdx;
	char tr98ObjIidStr[20] = {0};

	ztrdbg(ZTR_DL_DEBUG, "%s\n", __FUNCTION__);

	if(!tr98SpeedObjs || !(IS_SEQNUM_RIGHT(seqnum)) || !tr98ObjIid)
		return ZCFG_INTERNAL_ERROR;

	arrayIdx = ARRAY_SEQNUM(seqnum);
	if((arrayObj = json_object_array_get_idx(tr98SpeedObjs, arrayIdx)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no this object\n", __FUNCTION__);
		return ZCFG_NO_SUCH_OBJECT;
	}

	zcfgObjIidToMappingName(0, tr98ObjIid, tr98ObjIidStr);

	ztrdbg(ZTR_DL_INFO,  "%s: obj %u\n", __FUNCTION__, seqnum);
	json_object_object_del(arrayObj, (const char *)tr98ObjIidStr);

	return ZCFG_SUCCESS;
}

static zcfgRet_t zcfgTr98SpeedObjsAddParam(struct json_object *tr98SpeedObjs, zcfg_name_t seqnum, objIndex_t *tr98ObjIid, const char *parmname, struct json_object *paramObj)
{
	struct json_object *arrayObj = NULL, *tr98MappingIdxObj = NULL;
	uint32_t arrayIdx;
	char tr98ObjIidStr[20] = {0};

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	if(!tr98SpeedObjs || !parmname || !(IS_SEQNUM_RIGHT(seqnum)) || !tr98ObjIid)
		return ZCFG_INTERNAL_ERROR;

	arrayIdx = ARRAY_SEQNUM(seqnum);
	if((arrayObj = json_object_array_get_idx(tr98SpeedObjs, arrayIdx)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no this object\n", __FUNCTION__);
		arrayObj = json_object_new_object();
		json_object_array_put_idx(tr98SpeedObjs, arrayIdx, arrayObj);
	}

	zcfgObjIidToMappingName(0, tr98ObjIid, tr98ObjIidStr);
	if((tr98MappingIdxObj = json_object_object_get(arrayObj, tr98ObjIidStr)) == NULL) {
		//zcfgLog(ZCFG_LOG_INFO, "%s: no index object\n", __FUNCTION__);
		tr98MappingIdxObj = json_object_new_object();
	}

	//struct json_object *paramObj = json_object_new_object();
	//zcfgFeTr98ObjParamSet(parmname, type, paramObj, value);

	json_object_object_add(tr98MappingIdxObj, (const char *)parmname, paramObj);

	return ZCFG_SUCCESS;
}


zcfgRet_t zcfgFeTr98QueryAttrObjName(const char *tr98ObjName, struct json_object **tr98parm)
{
	zcfgRet_t rst;

	if((zcfgFeTr98QryRootPath() && enableRootQuryBlackObjPath &&
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, rootQuryBlackObjPathIdx)) ||
			zcfgQueryObjectPathIdxNotRepliedPathName(tr98ObjName, quryBlackObjPathIdx))
		return ZCFG_NO_MORE_INSTANCE;

#ifdef ZCFG_QUERY_TR98_PARM_ATTR_LIST
	rst = zcfgFeTr98QueryAttrObjNameAndParms(tr98ObjName, tr98parm);
	if(rst == ZCFG_SUCCESS)
		json_object_object_del(*tr98parm, "attr");
#else
	rst = zcfgFeTr98ObjNameGet(tr98ObjName, tr98parm);
#endif

	return rst;
}

#ifdef ZCFG_QUERY_TR98_PARM_ATTR_LIST
void zcfgTr98AttrListEnable()
{
	struct json_object *obj;
	if(!zcfgTr98AttrList){
		zcfgTr98AttrList = (obj = json_object_from_file(ZCFG_TR98_MAP_ATTR_LIST_PATH)) ? obj :
				json_object_new_array_list((e_TR98_LAST-e_TR98_START)+20);
	}
}

void *zcfgTr98AttrListSync(struct json_object *zcfgTr98NewAttrList)
{
	if(!zcfgTr98NewAttrList || !zcfgTr98AttrList)
		return NULL;
	//ztrdbg(ZTR_DL_INFO,  "%s\n", __FUNCTION__);
	printf("%s\n", __FUNCTION__);
	json_object_put(zcfgTr98AttrList);
	zcfgTr98AttrList = zcfgTr98NewAttrList;

	return NULL;
}

void zcfgTr98AttrListClear()
{
	if(zcfgTr98AttrList){
		json_object_put(zcfgTr98AttrList);
		zcfgTr98AttrList = NULL;
	}
}

bool zcfgTr98AttrListUtilized()
{
	return zcfgTr98AttrList ? 1 : 0;
}


static zcfgRet_t zcfgAttrListMultiParmAttrSetSync(json_object *zcfgAttrList, zcfgFeSetParmAttrLst_t *setParmAttrLst)
{
	zcfgFeSetParmAttr_t *setParmAttr;
	zcfgParmSetAttr_t *parmSetAttr;


	if(!zcfgAttrList || !setParmAttrLst)
		return ZCFG_EMPTY_OBJECT;

	setParmAttr = setParmAttrLst->start;
	while(setParmAttr) {

		if(strlen(setParmAttr->parmName)){
			parmSetAttr = &setParmAttr->parmSetAttr;
			struct json_object *attrListObj = zcfgTr98SpeedObjsRetrieve(zcfgAttrList, parmSetAttr->objnum, &parmSetAttr->objIid);
			struct json_object *paramObj = attrListObj ? json_object_object_get(attrListObj, setParmAttr->parmName) : NULL;
			//ztrdbg(ZTR_DL_INFO,  "%s: obj %u, %s\n", __FUNCTION__, parmSetAttr->objnum, setParmAttr->parmName);
			printf("%s: obj %u, %s\n", __FUNCTION__, parmSetAttr->objnum, setParmAttr->parmName);
			if(paramObj){
				json_object_object_add(paramObj, "attr", json_object_new_int(parmSetAttr->attr));
			}else{
				paramObj = json_object_new_object();
				json_object_object_add(paramObj, "attr", json_object_new_int(parmSetAttr->attr));
				if(!attrListObj){
				attrListObj = json_object_new_object();
					json_object_object_add(attrListObj, setParmAttr->parmName, paramObj);
				zcfgTr98SpeedObjsAddObj(zcfgAttrList, parmSetAttr->objnum, &parmSetAttr->objIid, attrListObj);
				}else{
					json_object_object_add(attrListObj, setParmAttr->parmName, paramObj);
				}
			}
		}

		setParmAttr = setParmAttr->next;
	}

	return ZCFG_SUCCESS;
}

// sync Tr98 set-attr track list to 'zcfgTr98AttrList'
zcfgRet_t zcfgTr98AttrListMultiParmAttrSetSync(zcfgFeSetParmAttrLst_t *setParmAttrLst)
{

	zcfgRet_t rst = zcfgAttrListMultiParmAttrSetSync(zcfgTr98AttrList, setParmAttrLst);
	//if(rst == ZCFG_SUCCESS)
	//	json_object_to_file("/data/zcfg_tr98_acsset_parmattrlist.json", zcfgTr98AttrList);

	return rst;
}

// add a 'set Tr98 object/param attr' record to a track list
zcfgRet_t zcfgSetParmAttrLstAddTr98ObjParmAttr(zcfgFeSetParmAttrLst_t *setParmAttrTrackLst, const char *tr98ObjName, const char *parmName, int notification)
{
	uint32_t seqnum;
	objIndex_t tr98ObjIid;
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	if(!setParmAttrTrackLst)
		return ZCFG_EMPTY_OBJECT;

	if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_OBJECT_WITHOUT_PARAMETER;

	const char *tr98ObjNameStr = (newtr98ObjName[0] != '\0') ? newtr98ObjName : tr98ObjName;
	IID_INIT(tr98ObjIid);
	zcfgFe98NameToSeqnum((const char *)tr98ObjNameStr, &seqnum, &tr98ObjIid);

	zcfgFeSetParmAttr_t *setParmAttr = NULL;
	uint32_t attr = notification | PARAMETER_ATTR_TR98;
	if((setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, parmName, 0, NULL, NULL, attr)))
		zcfgSetParmAttrAdd(setParmAttrTrackLst, setParmAttr);

	return ZCFG_SUCCESS;
}


zcfgRet_t zcfgFeTr98QueryAttrObjNameAndParms(const char *tr98ObjName, struct json_object **tr98parm)
{
	zcfgRet_t ret;
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
	const char *tr98ObjNamePtr;
	//tr98Parameter_t *parmList;
	zcfg_name_t seqnum = 0;
	objIndex_t tr98ObjIid;
	struct json_object *attrListObj;

	//if(!zcfgTr98GpnSpeedObjs)
	//	ztrdbg(ZTR_DL_DEBUG, "%s: zcfgTr98GpnSpeedObjs not enabled\n", __FUNCTION__);

	ret = zcfgFeTr98QueryObjParamNames(tr98ObjName, tr98parm, NULL);
	if(ret != ZCFG_SUCCESS)
		return ret;


	if(!zcfgTr98AttrList || !tr98parm)
		//return zcfgFeTr98ObjNameGet((char *)tr98ObjName, tr98parm);
		return ret;

	if (ReplaceWANDeviceIndex(tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_INVALID_OBJECT;

	tr98ObjNamePtr = (newtr98ObjName[0] != '\0') ? (const char *)newtr98ObjName : tr98ObjName;
	ret = zcfgFe98NameToSeqnum(tr98ObjNamePtr, &seqnum, &tr98ObjIid);
	if(ret != ZCFG_SUCCESS){
		//json_object_put(*tr98parm);
		return ZCFG_INVALID_OBJECT;
	}

	attrListObj = zcfgTr98SpeedObjsRetrieve(zcfgTr98AttrList, seqnum, &tr98ObjIid);
	if(attrListObj){
		// set attrListObj attrs to tr98parm
		zcfgTr98ObjSetParmAttr(*tr98parm, attrListObj);
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98QueryAttrObjExist(const char *tr98ObjName, struct json_object **tr98parm)
{
#if 0
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[OBJ_NAME_BUFF_SIZE] = {0};
	const char *tr98ObjNamePtr;

	if (ReplaceWANDeviceIndex(tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_INVALID_OBJECT;

	tr98ObjNamePtr = (newtr98ObjName[0] != '\0') ? (const char *)newtr98ObjName : tr98ObjName;

	if(zcfgFe98To181MappingNameGet(tr98ObjNamePtr, tr181ObjName) != ZCFG_SUCCESS)
		return zcfgFeTr98ObjNameGet((char *)tr98ObjName, tr98parm);

	return ZCFG_SUCCESS;
#endif
	return zcfgFeTr98QueryAttrObjNameAndParms(tr98ObjName, tr98parm);
}


#if 0
static zcfgRet_t qryAndSetParamObjAttr(struct json_object *mappingIdxObj, struct json_object *tr98parm, tr98Parameter_t *parmList)
{
	struct json_object *idxObjParm = NULL;
	int attr;

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	while(parmList && parmList->name){
		if(json_object_object_get(tr98parm, parmList->name)){
			idxObjParm = (mappingIdxObj) ? json_object_object_get(mappingIdxObj, parmList->name) : NULL;
			attr = (idxObjParm) ? json_object_get_int(json_object_object_get(idxObjParm, "attr")) : 0;
			json_object_object_add(tr98parm, parmList->name, json_object_new_int(attr));
		}
		parmList++;
	}

	return ZCFG_SUCCESS;
}

static zcfgRet_t parmListQryAndSetParamObjAttr(struct json_object *mappingIdxObj, struct json_object **tr98parm, tr98Parameter_t *parmList)
{
	struct json_object *idxObjParm = NULL;
	int attr;

	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	*tr98parm = json_object_new_object();
	while(parmList && parmList->name){
		idxObjParm = (mappingIdxObj) ? json_object_object_get(mappingIdxObj, parmList->name) : NULL;
		attr = (idxObjParm) ? json_object_get_int(json_object_object_get(idxObjParm, "attr")) : 0;
		json_object_object_add(*tr98parm, parmList->name, json_object_new_int(attr));
		parmList++;
	}

	return ZCFG_SUCCESS;
}

static zcfgRet_t clearParamObjAttr(struct json_object *tr98parm, tr98Parameter_t *parmList)
{
	ztrdbg(ZTR_DL_INFO, "%s\n", __FUNCTION__);

	while(parmList && parmList->name){
		if(json_object_object_get(tr98parm, parmList->name))
			json_object_object_add(tr98parm, parmList->name, json_object_new_int(0));

		parmList++;
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeTr98QueryAttrObjNameGet(const char *tr98ObjName, struct json_object **tr98parm, void *zcmdTr98MapAttrList)
{
	zcfgRet_t ret;
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[OBJ_NAME_BUFF_SIZE] = {0};
	const char *tr98ObjNamePtr;
	tr98Parameter_t *parmList;
	zcfg_name_t seqnum = 0;
	objIndex_t tr98ObjIid;
	struct json_object *attrListObj;

	if(!zcmdTr98MapAttrList)
		return zcfgFeTr98ObjNameGet((char *)tr98ObjName, tr98parm);

	if (ReplaceWANDeviceIndex(tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_INVALID_OBJECT;

	tr98ObjNamePtr = (newtr98ObjName[0] != '\0') ? (const char *)newtr98ObjName : tr98ObjName;
	if(zcfgFe98To181MappingNameGet(tr98ObjNamePtr, tr181ObjName) != ZCFG_SUCCESS){
		if((ret = zcfgFeTr98ObjNameGet((char *)tr98ObjName, tr98parm)) != ZCFG_SUCCESS)
			return ret;
	}

	ret = zcfgFe98NameToSeqnum(tr98ObjNamePtr, &seqnum, &tr98ObjIid);
	if(ret != ZCFG_SUCCESS){
		json_object_put(*tr98parm);
		return ZCFG_INVALID_OBJECT;
	}

	int handler = ARRAY_SEQNUM(seqnum);

	// if seqnum specific objs,
	//	 zcfgFeTr98ObjNameGet(tr98ObjName, tr98parm);

	parmList = tr98Obj[handler].parameter;
	if(!parmList){
		printf("%s: no parmList\n", __FUNCTION__);
		json_object_put(*tr98parm);
		return ZCFG_INVALID_OBJECT;
	}

	attrListObj = retrieveAttrListObj(zcmdTr98MapAttrList, seqnum, &tr98ObjIid);
	if(attrListObj && *tr98parm){ //objNameQried
		qryAndSetParamObjAttr(attrListObj, *tr98parm, parmList);
		return ZCFG_SUCCESS;
	}
	else if(!attrListObj && *tr98parm) { //objNameQried
		clearParamObjAttr(*tr98parm, parmList);
		return ZCFG_SUCCESS;
	}

	parmListQryAndSetParamObjAttr(attrListObj, tr98parm, parmList);

	return ZCFG_SUCCESS;
}
#endif
#endif

zcfgRet_t zcfgFeTr98QueryObjParamNames(const char *tr98ObjName, struct json_object **tr98ParamObj, struct json_object *tr98QueriedRdm)
{
	zcfgRet_t ret;
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0}, tr181ObjName[OBJ_NAME_BUFF_SIZE] = {0};
	const char *tr98ObjNamePtr;
	tr98Parameter_t *parmList;
	zcfg_name_t seqnum = 0;
	objIndex_t tr98ObjIid;
	struct json_object *attrListObj;
	int requiringObjQuery = !tr98ParamObj ? 0 : 1;
	struct json_object *tr98parm = NULL;
	char seqStr[30] = {0};

	if (ReplaceWANDeviceIndex(tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
		return ZCFG_INVALID_OBJECT;

	tr98ObjNamePtr = (newtr98ObjName[0] != '\0') ? (const char *)newtr98ObjName : tr98ObjName;
	ret = zcfgFe98NameToSeqnum(tr98ObjNamePtr, &seqnum, &tr98ObjIid);
	if(ret != ZCFG_SUCCESS){
		return ZCFG_INVALID_OBJECT;
	}
	int handler = ARRAY_SEQNUM(seqnum);

	parmList = tr98Obj[handler].parameter;
	if(!parmList){
		//printf("%s: no parmList\n", __FUNCTION__);
		return ZCFG_OBJECT_WITHOUT_PARAMETER;
	}

	if(tr98QueriedRdm){
	}

	if(zcfgTr98GpnSpeedObjs){
		struct json_object *speedObj = tr98ParamObj ?  zcfgTr98SpeedObjsRetObj(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid) :
		zcfgTr98SpeedObjsRetrieve(zcfgTr98GpnSpeedObjs, seqnum, &tr98ObjIid);

		if(speedObj){
			ztrdbg(ZTR_DL_INFO, "%s: speedobj %s retrieved\n", __FUNCTION__, tr98ObjName);
			parmList = tr98Obj[handler].parameter;
			// check object attr
			parmListSetObjParamNameAttr(&speedObj, parmList);
			*tr98ParamObj = speedObj;

			return ZCFG_SUCCESS;
		}
	}

	sprintf(seqStr, "%u", handler);
	tr98QryHandleRout gpnQueryHdlRout = zcfgTr98GpnHandleRoutPick(seqStr);
	if(gpnQueryHdlRout){
		ret = gpnQueryHdlRout((const char *)tr98ObjNamePtr, handler, &tr98ObjIid, &tr98parm, NULL);
		if(ret != ZCFG_SUCCESS)
			return ret;
		else{
			if(!requiringObjQuery)
				json_object_put(tr98parm);
			else{
				*tr98ParamObj = tr98parm;
			}
		}
	}else if(zcfgFe98To181MappingNameGet(tr98ObjNamePtr, tr181ObjName) != ZCFG_SUCCESS){
		if((ret = zcfgFeTr98ObjNameGet((char *)tr98ObjNamePtr, &tr98parm)) != ZCFG_SUCCESS)
			return ret;
		else{
			if(!requiringObjQuery)
				json_object_put(tr98parm);
			else{
				parmListSetObjParamNameAttr(&tr98parm, parmList);
				*tr98ParamObj = tr98parm;
			}
		}
		return ret;
	}else if(!requiringObjQuery){
		return ZCFG_SUCCESS;
	}else{
		parmListSetObjParamNameAttr(&tr98parm, parmList);
		*tr98ParamObj = tr98parm;
		return ZCFG_SUCCESS;
	}

}

#ifdef ZCFG_PARM_NAMEMAPPING
int zcfgFeTr98ObjParamAttrGetByName(const char *tr98ObjName, const char *name)
{
	zcfgRet_t ret;
	zcfg_name_t seqnum = 0; 
	zcfg_offset_t oid = 0;
	objIndex_t tr98ObjIid, objIid;
	char queriedParmName[MAX_PARMNAME_LEN] = {0}, tr98ClassName[128] = {0};
	uint32_t attr = 0;
	int handler = 0;
	tr98Parameter_t *paramList = NULL;

	ztrdbg(ZTR_SL_DEBUG, "%s\n", __FUNCTION__);

	if((ret = zcfgFeTr98ClassNameGet((char *)tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;
		if(paramList == NULL)
		{
			return ZCFG_NO_SUCH_PARAMETER;
		}

		while(paramList->name!= NULL) {
			if(strcmp(paramList->name, name) == 0){
				break;
			}
			paramList++;
		}
	}
	else {
		return ZCFG_NO_SUCH_PARAMETER;
	}

	if(!paramList->name) {
		return ZCFG_NO_SUCH_PARAMETER;
	}

	if(paramList->attr & PARAMETER_ATTR_DEFAULTATTR) {
		if((paramList->attr & PARAMETER_ATTR_DENY_DISABLENOTIFY_REQUEST) && 
							(paramList->attr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST))
			return 1;

		if((paramList->attr & PARAMETER_ATTR_DENY_DISABLENOTIFY_REQUEST) && 
							(paramList->attr & PARAMETER_ATTR_DENY_PASSIVENOTIFY_REQUEST))
			return 2;

		if((paramList->attr & PARAMETER_ATTR_DENY_PASSIVENOTIFY_REQUEST) && 
							(paramList->attr & PARAMETER_ATTR_DENY_ACTIVENOTIFY_REQUEST))
			return 0;
	}

	IID_INIT(tr98ObjIid);
	{
		char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

		if(newtr98ObjName[0] != '\0')
			ret = zcfgFe98NameToSeqnum((const char *)newtr98ObjName, &seqnum, &tr98ObjIid);
		else
			ret = zcfgFe98NameToSeqnum((const char *)tr98ObjName, &seqnum, &tr98ObjIid);
	}

	if(ret == ZCFG_SUCCESS) {
		attr |= PARAMETER_ATTR_TR98;
		if((ret = zcfgFeParmAttrQry(seqnum, &tr98ObjIid, name, &oid, &objIid, queriedParmName, &attr)) != ZCFG_SUCCESS) {
			ztrdbg(ZTR_SL_DEBUG, "%s: parm attr query fail, %u %s\n", __FUNCTION__, seqnum, name);
			return ret;
			//printf("%s: parm attr query fail, %u %s\n", __FUNCTION__, seqnum, name);
			//return 0;
		}
			//confirm data if required
		if(attr & PARAMETER_ATTR_NOATTR) {
			return 0;
			//return ZCFG_NO_SUCH_PARAMETER;
		}
		//printf("%s: attr 0x%x\n", __FUNCTION__, attr);
		return PARAMETER_ATTR_NOTIFY_RETRIEVE(attr);
	}

	return ZCFG_NO_SUCH_PARAMETER;
}
#endif

int zcfgFeTr98ParamAttrGetByName(char *tr98ObjName, char *name)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		//if(tr98Obj[handler].getAttr != NULL){
		if(ret != ZCFG_SUCCESS){
			char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
			if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
				return ZCFG_NO_SUCH_PARAMETER;

			if(newtr98ObjName[0] != '\0')
				ret = tr98Obj[handler].getAttr(newtr98ObjName, handler, name);
			else
				ret = tr98Obj[handler].getAttr(tr98ObjName, handler, name);

#ifdef ZCFG_ABANDON_NO_TR181_MAPPING_PARM
			if(ret == 0)
				return ZCFG_NO_SUCH_PARAMETER;
			else
				return ret;
#else
			if(ret == 0) {
				paramList = tr98Obj[handler].parameter;
				if(paramList == NULL)
					return ZCFG_NO_SUCH_PARAMETER;

				while(paramList->name!= NULL) {
					if(strcmp(paramList->name, name) == 0){
						return paramList->attr;
					}
					paramList++;
				}
			}
			else
				return ret;
#endif
		}
		else {
#ifdef ZCFG_ABANDON_NO_TR181_MAPPING_PARM
			char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};
			if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
				return ZCFG_NO_SUCH_PARAMETER;

			if(newtr98ObjName[0] != '\0')
				int attrVal = zcfgFeTr98ObjParamAttrGetByName((const char *)newtr98ObjName, (const char *)name);
			else
				int attrVal = zcfgFeTr98ObjParamAttrGetByName((const char *)tr98ObjName, (const char *)name);

			if(attrVal < 0) {
				return attrVal;
			}
#endif
			paramList = tr98Obj[handler].parameter;
			if(paramList == NULL)
			{
				return ZCFG_NO_SUCH_PARAMETER;
			}

			while(paramList->name!= NULL) {
				if(strcmp(paramList->name, name) == 0){
					return paramList->attr;
				}
				paramList++;
			}
		}
	}
	
	return ZCFG_NO_SUCH_PARAMETER;
}

zcfgRet_t zcfgFeTr98AllParamAttrGetByObjName(const char *tr98ObjName, struct json_object *result)
{
	zcfgRet_t ret;
	char tr98ClassName[256] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	//char paraName[64] = {0};

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;

		if(paramList == NULL) {
			return ZCFG_NO_SUCH_PARAMETER;
		}

		while(paramList->name != NULL) {
			json_object_object_add(result, paramList->name, json_object_new_int(paramList->attr));
			paramList++;
		}
	}
	else {
		return ZCFG_NO_SUCH_OBJECT;
	}

	return ZCFG_SUCCESS;
}

#ifdef ZCFG_PARM_NAMEMAPPING

zcfgFeSetParmAttr_t *zcfgFeTr98ParamAttrSetByName(uint32_t objnum, objIndex_t *objIid, const char *parmName, uint32_t setAttrObjNum, objIndex_t *setAttrObjIid, const char *setAttrParmName, uint32_t attr)
{
	int n = 0;
	zcfgFeSetParmAttr_t *setParmAttr = NULL;

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	setParmAttr = (zcfgFeSetParmAttr_t *)malloc(sizeof(zcfgFeSetParmAttr_t));
	memset(setParmAttr, 0, sizeof(zcfgFeSetParmAttr_t));

	ztrdbg(ZTR_DL_DEBUG, "%s: obj %u %hu,%hu,%hu,%hu,%hu,%hu %s, attr 0x%x\n", __FUNCTION__, objnum,
			objIid->idx[0], objIid->idx[1], objIid->idx[2], objIid->idx[3], objIid->idx[4], objIid->idx[5], parmName, attr);

	setParmAttr->parmSetAttr.objnum = objnum;
	memcpy(&setParmAttr->parmSetAttr.objIid, objIid, sizeof(objIndex_t));
	setParmAttr->parmSetAttr.attr = attr;
	setParmAttr->parmSetAttr.parmNameLen = parmName ? strlen(parmName) : 0;
	if(setParmAttr->parmSetAttr.parmNameLen){
		strncpy(setParmAttr->parmName, parmName, MAX_PARMNAME_LEN-1);
		setParmAttr->parmSetAttr.parmNameLen++;
		setParmAttr->parmSetAttr.parmNameLen += ((n = (setParmAttr->parmSetAttr.parmNameLen % UNITSIZE)) > 0) ? (UNITSIZE - n) : 0;
	}

	setParmAttr->parmSetAttr.setAttrObjNum = setAttrObjNum;
	if(setAttrObjIid)
		memcpy(&setParmAttr->parmSetAttr.setAttrObjIid, setAttrObjIid, sizeof(objIndex_t));
	else
		memset(&setParmAttr->parmSetAttr.setAttrObjIid, 0, sizeof(objIndex_t));
	n = 0;
	setParmAttr->parmSetAttr.setAttrParmNameLen = (setAttrParmName) ? strlen(setAttrParmName) : 0;
	if(setParmAttr->parmSetAttr.setAttrParmNameLen > 0) {
		strncpy(setParmAttr->setAttrParmName, setAttrParmName, MAX_PARMNAME_LEN-1);
		setParmAttr->parmSetAttr.setAttrParmNameLen++;
		setParmAttr->parmSetAttr.setAttrParmNameLen += ((n = (setParmAttr->parmSetAttr.setAttrParmNameLen % UNITSIZE)) > 0) ? (UNITSIZE - n) : 0;
	}

	return setParmAttr;
}


int zcfgFeTr98ParamAttrMultiSetByName(char *tr98ObjName, char *name, int notify, zcfgFeSetParmAttrLst_t *setParmAttrLst)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	zcfg_name_t seqnum = 0;
	zcfg_offset_t oid = 0;
	objIndex_t tr98ObjIid, objIid;
	char queriedParmName[MAX_PARMNAME_LEN] = {0};
	uint32_t queriedAttr = 0;
	zcfgFeSetParmAttr_t *setParmAttr = NULL;
	tr98Parameter_t *paramList = NULL;
	bool qriedNoSuchParam = false;
	char newtr98ObjName[TR98_MAX_OBJ_NAME_LENGTH] = {0};

	zcfgLog(ZCFG_LOG_INFO, "%s\n", __FUNCTION__);

	//if(!tr98ObjName || !name || !setParmAttrLst)
	//	return ZCFG_INTERNAL_ERROR;

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	paramList = tr98Obj[handler].parameter;
	if((handler == -1) || !paramList) {
		return ZCFG_NO_SUCH_PARAMETER;
	}

	while(paramList->name) {
		if(!strcmp(paramList->name, name)) {
			if((ret = zcfgFeTr98ParamAttrChk(paramList->attr, notify)) != ZCFG_SUCCESS)
				return ret;
			
			break;
		}
		paramList++;
	}

	if(!paramList->name) {
		return ZCFG_NO_SUCH_PARAMETER;
	}
	else {
		if(paramList->attr & PARAMETER_ATTR_DEFAULTATTR)
			return ZCFG_SUCCESS;
	}

	IID_INIT(tr98ObjIid);
	{
		if (ReplaceWANDeviceIndex((const char *)tr98ObjName, newtr98ObjName) != ZCFG_SUCCESS)
			return ZCFG_OBJECT_WITHOUT_PARAMETER;

		if(newtr98ObjName[0] != '\0')
			ret = zcfgFe98NameToSeqnum((const char *)newtr98ObjName, &seqnum, &tr98ObjIid);
		else
			ret = zcfgFe98NameToSeqnum((const char *)tr98ObjName, &seqnum, &tr98ObjIid);
	}

	if(ret == ZCFG_SUCCESS){
		queriedAttr |= PARAMETER_ATTR_TR98;
		if((ret = zcfgFeParmAttrQry(seqnum, &tr98ObjIid, (const char *)name, &oid, &objIid, queriedParmName, &queriedAttr)) != ZCFG_SUCCESS) {
			zcfgLog(ZCFG_LOG_INFO, "%s: tr98 objnum %u %s query fail\n", __FUNCTION__, seqnum, name);
			qriedNoSuchParam = true;
		}
	}
	else {

		return ZCFG_NO_SUCH_PARAMETER;
	}

	queriedAttr = (queriedAttr & ~PARAMETER_ATTR_NOTIFY_MASK) | (notify & PARAMETER_ATTR_NOTIFY_MASK);

#ifdef ZCFG_PARM_NAMEMAPPING_NOT_APPLIED
	if(qriedNoSuchParam == true || !strcmp(queriedParmName, "notapplied")) {
		// no tr98/tr181 mapping, not applicable, not implemented, and something as that
		queriedAttr = PARAMETER_ATTR_NOT_APPLIED | PARAMETER_ATTR_TR98 | (notify & PARAMETER_ATTR_NOTIFY_MASK);
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, name, 0, NULL, NULL, queriedAttr);
		zcfgSetParmAttrAdd(setParmAttrLst, setParmAttr);
		return ZCFG_SUCCESS;
	}
	else if(queriedAttr & PARAMETER_ATTR_TR98){
		setParmAttr = zcfgFeTr98ParamAttrSetByName(seqnum, &tr98ObjIid, name, 0, NULL, NULL, queriedAttr);
		zcfgSetParmAttrAdd(setParmAttrLst, setParmAttr);
		return ZCFG_SUCCESS;
	}

#else
	if(qriedNoSuchParam == true)
		return ZCFG_NO_SUCH_PARAMETER;
#endif

	if(IS_HANDLE_ROUTINE_REQUIRED(queriedAttr)) {
		struct json_object *multiJobj = NULL;
		int nObj = 0, n = 0;

		printf("%s: set parm attr handle routine is required\n", __FUNCTION__);
		if(!tr98Obj[handler].setAttr) {
			printf("%s: no set parm attr handle routine\n", __FUNCTION__);
			return ZCFG_NO_SUCH_PARAMETER;
		}

		multiJobj = json_object_new_array();
		if(newtr98ObjName[0] != '\0')
			ret = tr98Obj[handler].setAttr(newtr98ObjName, handler, name, queriedAttr, multiJobj);
		else
			ret = tr98Obj[handler].setAttr(tr98ObjName, handler, name, queriedAttr, multiJobj);

		if(ret != ZCFG_SUCCESS) {
			printf("%s: set parm attr handle routine fail\n", __FUNCTION__);
		}
		if((nObj = json_object_array_length(multiJobj))) {
			printf("%s: there're %d set attr items\n", __FUNCTION__, nObj);

			while(n < nObj) {
				setParmAttr = (zcfgFeSetParmAttr_t *)json_object_get_int(json_object_array_get_idx(multiJobj, n));
				if(!setParmAttr) {
					n++;
					continue;
				}

				if(ret == ZCFG_SUCCESS) {
					zcfgSetParmAttrAdd(setParmAttrLst, setParmAttr);
				}
				else{
					ZOS_FREE(setParmAttr);
				}

				n++;
			}
		}
		json_object_put(multiJobj);
		if(ret != ZCFG_SUCCESS)
			return ret;
	}
	else {
		setParmAttr = zcfgFeTr98ParamAttrSetByName(oid, &objIid, queriedParmName, 0, NULL, NULL, (queriedAttr & PARAMETER_ATTR_NOTIFY_MASK));
		zcfgSetParmAttrAdd(setParmAttrLst, setParmAttr);
	}

	return ZCFG_SUCCESS;
}
#else
int zcfgFeTr98ParamAttrMultiSetByName(char *tr98ObjName, char *name, int notify, struct json_object *multiJobj)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	
	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		if(tr98Obj[handler].setAttr != NULL){
			return tr98Obj[handler].setAttr(tr98ObjName, handler, name, notify, multiJobj);
		}
		else{
			return ZCFG_REQUEST_REJECT;
		}
	}
	
	return ZCFG_NO_SUCH_PARAMETER;
}
#endif

zcfgRet_t zcfgFeTr98QueryParamListUnit(const char *tr98ObjName, const char *parmName, tr98Parameter_t *queryParmList)
{
	char tr98ClassName[200] = {0};
	int handler = 0;
	tr98Parameter_t *parmList = NULL;

	if(!tr98ObjName || !parmName || !queryParmList)
		return ZCFG_INTERNAL_ERROR;
	if(zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName) != ZCFG_SUCCESS){
		return ZCFG_NO_SUCH_OBJECT;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler == -1)
		return ZCFG_NO_SUCH_OBJECT;

	parmList = tr98Obj[handler].parameter;
	while(parmList && parmList->name){
		if(!strcmp(parmList->name, parmName)){
			memcpy(queryParmList, parmList, sizeof(tr98Parameter_t));
			return ZCFG_SUCCESS;
		}
		parmList++;
	}

	return ZCFG_NO_SUCH_PARAMETER;
}

int zcfgFeTr98ParamTypeGetByName(char *tr98ObjName, char *name)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;
		if(paramList == NULL)
		{
			return ZCFG_NO_SUCH_PARAMETER;
		}
		while(paramList->name!= NULL) {
			if(strcmp(paramList->name, name) == 0){
				return paramList->type;
			}
			paramList++;
		}
	}
	
	return ZCFG_NO_SUCH_PARAMETER;
}

int zcfgFeTr98ParamAttrContentGetByName(char *tr98ObjName, char *name)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	
	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}
	
	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;
		if(paramList == NULL)
		{
			return ZCFG_NO_SUCH_PARAMETER;
		}
		while(paramList->name!= NULL) {
			if(strcmp(paramList->name, name) == 0){
				return paramList->attr;
			}
			paramList++;
		}
	}
	
	return ZCFG_NO_SUCH_PARAMETER;
}

zcfgRet_t zcfgFeTr98ParamChk(char *tr98ObjName, char *name, char *value)
{
	zcfgRet_t ret;
	char tr98ClassName[128] = {0};
	int handler = 0;
	tr98Parameter_t *paramList = NULL;
	json_type type;
	char *p = NULL;
	bool found = false;
	int len = 0, index = 0;

	if((ret = zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName)) != ZCFG_SUCCESS) {
		return ret;
	}

	handler = zcfgFeTr98HandlerGet(tr98ClassName);
	if(handler != -1) {
		paramList = tr98Obj[handler].parameter;
		if(paramList == NULL)
		{
			return ZCFG_NO_SUCH_PARAMETER;
		}
		while(paramList->name!= NULL) {
			if(strcmp(paramList->name, name) == 0){
				found = true;
				break;
			}
			paramList++;
		}
	}
	if(!found){
		return ZCFG_NO_SUCH_PARAMETER;
	}
		
	if(paramList->attr & PARAMETER_ATTR_READONLY) {
		zcfgLog(ZCFG_LOG_ERR, "Can not set a read-only parameter %s\n", name);
		return ZCFG_SET_READ_ONLY_PARAM;
	}
	
	type = paramList->type;
	if(type == json_type_string || type == json_type_base64)
	{
		if(strlen(value) >= paramList->len) {
			return ZCFG_INVALID_PARAM_VALUE_LENGTH;
		}
	}
	switch(type){
		case json_type_int:
			p = value;
			if(atol(value)<0){
				if(*p != '-'){
					return ZCFG_INVALID_PARAM_VALUE;
				}
				p++;
			}
			while(*p!='\0'){
				if(!isdigit(*p)){
					return ZCFG_INVALID_PARAM_VALUE;
				}
				p++;
			}
			break;
		case json_type_uint8:
		case json_type_uint16:
		case json_type_uint32:
		case json_type_ulong:
			p = value;
			while(*p!='\0'){
				if(!isdigit(*p)){
					return ZCFG_INVALID_PARAM_VALUE;
				}
				p++;
			}
			break;
		case json_type_boolean:
			if(strcasecmp(value, "true")==0 ){
				strcpy(value, "1");
				break;
			}
			else if(strcasecmp(value, "false")==0 ){
				strcpy(value, "0");
				break;
			}
			if( (strcmp(value, "0")!=0) && (strcmp(value, "1")!=0)){
				return ZCFG_INVALID_PARAM_VALUE;
			}
			break;
		case json_type_time:/*0001-01-01T00:00:00Z*/
			len = strlen(value);
			if(len != 19 && len != 20){
				return ZCFG_INVALID_PARAM_VALUE;
			}
			p = value;
			for(index = 0; index<len; index++){
				if(index==4 || index==7){
					if(!(*p == '-')){
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
				else if(index == 10){
					if(!(*p == 'T')){
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
				else if(index == 13 || index ==16){
					if(!(*p == ':')){
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
				else if(index == 19){
					if(!(*p == 'Z')){
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
				else{
					if(!isdigit(*p)){
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}	
				p++;
			}
			break;
		default:
			break;
	}
	
	return ZCFG_SUCCESS;
}


#ifdef ZCFG_RPC_REQUEST_ADDITIONS
struct json_object *zcfgFeRpcRequestAdditionNew()
{
	struct json_object *additions = json_object_new_object();
	zcfgFeRpcRequestAddRpcDelayApply(additions, false);

	return additions;
}

zcfgRet_t zcfgFeRpcRequestAddAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject, bool utilizeOnConfigApply)
{
	struct json_object *rpcRequestAttrs = NULL;

	if(!rpcAdditions || !attrName || !attrObject)
		return ZCFG_INTERNAL_ERROR;

	rpcRequestAttrs = json_object_object_get(rpcAdditions, "rpcrequestattrs");
	if(!rpcRequestAttrs){
		rpcRequestAttrs = json_object_new_object();
		json_object_object_add(rpcAdditions, "rpcrequestattrs", rpcRequestAttrs);
	}

	json_object_object_add(rpcRequestAttrs, attrName, attrObject);
	if(utilizeOnConfigApply)
		zcfgFeRpcRequestAddRpcApplyAttr(rpcRequestAttrs, "runonconfigapply", json_object_new_boolean(true));
	else
		zcfgFeRpcRequestAddRpcApplyAttr(rpcRequestAttrs, "runonconfigrequest", json_object_new_boolean(true));

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeRpcRequestAddReplyAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject)
{
	struct json_object *rpcReplyAttrs = NULL;

	if(!rpcAdditions || !attrName || !attrObject)
		return ZCFG_INTERNAL_ERROR;

	rpcReplyAttrs = json_object_object_get(rpcAdditions, "rpcreplyattrs");
	if(!rpcReplyAttrs){
		rpcReplyAttrs = json_object_new_object();
		json_object_object_add(rpcAdditions, "rpcreplyattrs", rpcReplyAttrs);
	}

	json_object_object_add(rpcReplyAttrs, attrName, attrObject);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeRpcRequestAddRpcApplyAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject)
{
	struct json_object *rpcApplySetAttrs = NULL;

	if(!rpcAdditions || !attrName || !attrObject)
		return ZCFG_INTERNAL_ERROR;

	rpcApplySetAttrs = json_object_object_get(rpcAdditions, "rpcapplysetattrs");
	if(!rpcApplySetAttrs){
		rpcApplySetAttrs = json_object_new_object();
		json_object_object_add(rpcAdditions, "rpcapplysetattrs", rpcApplySetAttrs);
	}

	json_object_object_add(rpcApplySetAttrs, attrName, attrObject);

	return ZCFG_SUCCESS;
}


zcfgRet_t zcfgFeRpcRequestAddMethod(struct json_object *rpcAdditions, const char *methodName, bool runMethodOnConfigApply)
{
	struct json_object *rpcRequestMethods = NULL;

	if(!rpcAdditions || !methodName)
		return ZCFG_INTERNAL_ERROR;

	rpcRequestMethods = json_object_object_get(rpcAdditions, "rpcrequestmethods");
	if(!rpcRequestMethods){
		rpcRequestMethods = json_object_new_object();
		json_object_object_add(rpcAdditions, "rpcrequestmethods", rpcRequestMethods);
	}

	struct json_object *rpcApplySetAttrs = json_object_object_get(rpcAdditions, "rpcapplysetattrs");
	if(!rpcApplySetAttrs){
		rpcApplySetAttrs = json_object_new_object();
		json_object_object_add(rpcAdditions, "rpcapplysetattrs", rpcApplySetAttrs);
	}

	struct json_object *rpcRequestMethod = json_object_object_get(rpcRequestMethods, methodName);
	if(!rpcRequestMethod){
		rpcRequestMethod = json_object_new_object();
		json_object_object_add(rpcRequestMethods, methodName, rpcRequestMethod);
		if(runMethodOnConfigApply)
			zcfgFeRpcRequestAddRpcApplyAttr(rpcApplySetAttrs, "runonconfigapply", json_object_new_boolean(true));
		else
			zcfgFeRpcRequestAddRpcApplyAttr(rpcApplySetAttrs, "runonconfigrequest", json_object_new_boolean(true));
	}

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeRpcRequestSpecifyMethodParam(struct json_object *rpcAdditions, const char *methodName, const char *param, struct json_object *value)
{
	struct json_object *rpcRequestMethods = NULL;

	if(!rpcAdditions || !methodName || !param || !value)
		return ZCFG_INTERNAL_ERROR;

	rpcRequestMethods = json_object_object_get(rpcAdditions, "rpcrequestmethods");
	if(!rpcRequestMethods)
		return ZCFG_NO_SUCH_OBJECT;

	struct json_object *rpcRequestMethod = json_object_object_get(rpcRequestMethods, methodName);
	if(!rpcRequestMethod)
		return ZCFG_NO_SUCH_OBJECT;

	json_object_object_add(rpcRequestMethod, param, value);

	return ZCFG_SUCCESS;
}

zcfgRet_t zcfgFeRpcObjectRetrieveMethodReply(struct json_object *rpcAdditions, const char *methodName, struct json_object **methodReply)
{
	struct json_object *rpcReplyMethods = NULL, *rpcReplyMethod = NULL;
	zcfgRet_t methodReplyRst = ZCFG_NO_SUCH_PARAMETER;

	if(!rpcAdditions || !methodName || !methodReply)
		return ZCFG_INTERNAL_ERROR;

	*methodReply = NULL;
	struct json_object *rpcAdditionsReply = json_object_object_get(rpcAdditions, "rpcadditionsreply");
	if(rpcAdditionsReply)
		rpcReplyMethods = json_object_object_get(rpcAdditions, "rpcreplymethod");

	if(rpcReplyMethods){
		rpcReplyMethod = json_object_object_get(rpcReplyMethods, methodName);
		if(rpcReplyMethod){
			struct json_object *methodReplyRstObj = json_object_object_get(rpcReplyMethod, "_methodreplyrst");
			methodReplyRst = methodReplyRstObj ? json_object_get_int(methodReplyRstObj) : ZCFG_NO_SUCH_PARAMETER;
			*methodReply = rpcReplyMethod;
		}
	}

	return methodReplyRst;
}

struct json_object *zcfgFeRpcRequestRetrieveMethodReplyParam(struct json_object *methodReply, const char *parmName)
{
	struct json_object *paramObj;
	paramObj = (methodReply && parmName) ? json_object_object_get(methodReply, parmName) : NULL;
	return paramObj;
}
#else
zcfgRet_t zcfgFeRpcRequestAddAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject, bool utilizeOnConfigApply)
{
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zcfgFeRpcRequestAddReplyAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject)
{
	return ZCFG_INTERNAL_ERROR;
}


zcfgRet_t zcfgFeRpcRequestAddRpcApplyAttr(struct json_object *rpcAdditions, const char *attrName, struct json_object *attrObject)
{
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zcfgFeRpcRequestAddMethod(struct json_object *rpcAdditions, const char *methodName, bool runMethodOnConfigApply)
{
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zcfgFeRpcRequestSpecifyMethodParam(struct json_object *rpcAdditions, const char *methodName, const char *param, struct json_object *value)
{
	return ZCFG_INTERNAL_ERROR;
}


zcfgRet_t zcfgFeRpcObjectRetrieveMethodReply(struct json_object *rpcAdditions, const char *methodName, struct json_object **methodReply)
{
	return ZCFG_INTERNAL_ERROR;
}

struct json_object *zcfgFeRpcRequestRetrieveMethodReplyParam(struct json_object *methodReply, const char *parmName)
{
	return NULL;
}

#endif

void zcfgFeTr98GuiCustomizationSet(char *parmName, void *value)
{
	if(!strcmp(parmName, "Flag1")){
		guiFlag1 = (uint32_t)value;
		//printf("%s: guiFlag1=%u\n", __FUNCTION__, guiFlag1);
	}
}

zcfgSubObjNameList_t* zcfgFeTr98SubObjNameListGet(char *tr98ObjName){
	zcfgSubObjNameList_t *head = NULL, *now = NULL, *node = NULL;
	char tr98ClassName[128] = {0};
	int c = 0;
	char *subName = NULL;
	char *ptr = NULL;
	bool find = false;
	bool hiddenVoIP = false;
	objIndex_t objIid;
	struct json_object *GuiCustoMization = NULL;
	IID_INIT(objIid);
	//if(zcfgFeObjJsonGet(RDM_OID_GUI_CUSTOMIZATION, &objIid, &GuiCustoMization) != ZCFG_SUCCESS)
	//	return NULL;

	//if(json_object_get_int(json_object_object_get(GuiCustoMization, "Flag1")) & HIDE_VOIP)
	if(guiFlag1 & HIDE_VOIP)
		hiddenVoIP = true;		
	
	zcfgFeJsonObjFree(GuiCustoMization);

	if(zcfgFeTr98ClassNameGet(tr98ObjName, tr98ClassName) != ZCFG_SUCCESS) {
		return NULL;
	}

	while(tr98Obj[c].name != NULL) {
		find = false;
		if((strncmp(tr98Obj[c].name, tr98ClassName, strlen(tr98ClassName)) == 0) && (strlen(tr98Obj[c].name)>strlen(tr98ClassName))){
			ptr = strchr(tr98Obj[c].name+strlen(tr98ClassName)+1, '.');
			if( ptr == NULL ){
				find = true;
			}
			else if( /*index*/ strcmp(ptr, ".i") == 0){
				find = true;
			}
			if( !find){
				c++;
				continue;
			}
			
			node = (zcfgSubObjNameList_t*)malloc(sizeof(zcfgSubObjNameList_t));
			subName = tr98Obj[c].name + strlen(tr98ClassName) + 1;
			#ifndef CONFIG_ZYXEL_TR140
			if(!strcmp(subName, "Services")){
				if(hiddenVoIP){
					c++;
					continue;
				}
			}
			#endif

			node->objName = (char *)malloc(strlen(subName)+1);
			strcpy(node->objName, subName);
			if(head == NULL) {
				node->next = NULL;
				head = node;
				now = node;
			}
			else {
				now->next = node;
				now = node;
				now->next = NULL;
			}

		}
		c++;
	}
	
	return head;
}

/*improve the performance of get object names*/
zcfgRet_t feObjStructGet(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeObjStructGet(oid, objIid, requestStruct);
	else return zcfgFeObjStructGetWithoutUpdate(oid, objIid, requestStruct);
}

zcfgRet_t feObjStructGetNext(zcfg_offset_t oid, objIndex_t *objIid, void **requestStruct, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeObjStructGetNext(oid, objIid, requestStruct);
	else return zcfgFeObjStructGetNextWithoutUpdate(oid, objIid, requestStruct);
}

zcfgRet_t feObjStructGetSubNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, void **requestStruct, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeSubInStructGetNext(oid, objIid, insIid,requestStruct);
	else return zcfgFeSubInStructGetNextWithoutUpdate(oid, objIid, insIid, requestStruct);
}

zcfgRet_t feObjJsonGet(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeObjJsonGet(oid, objIid, value);
	else return zcfgFeObjJsonGetWithoutUpdate(oid, objIid, value);
}
	
zcfgRet_t feObjJsonGetNext(zcfg_offset_t oid, objIndex_t *objIid, struct json_object **value, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeObjJsonGetNext(oid, objIid, value);
	else return zcfgFeObjJsonGetNextWithoutUpdate(oid, objIid, value);
}
		
zcfgRet_t feObjJsonSubNext(zcfg_offset_t oid, objIndex_t *objIid, objIndex_t *insIid, struct json_object **value, bool flag){
	if(flag == TR98_GET_UPDATE) return zcfgFeSubInObjJsonGetNext(oid, objIid, insIid, value);
	else return zcfgFeSubInObjJsonGetNextWithoutUpdate(oid, objIid, insIid, value);
}
