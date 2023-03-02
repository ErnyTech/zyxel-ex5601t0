#ifndef ZCFG_TR98_COMMON_H
#define ZCFG_TR98_COMMON_H

#define IS_SEQNUM_RIGHT(seqnum) ((seqnum >= e_TR98_START) && (seqnum <= e_TR98_LAST))
#define ARRAY_SEQNUM(seqnum) (seqnum-e_TR98_START)

enum {
	PARMATTR_TR98 = 1,
	PARMATTR_TR181 = (1 << 1)
};

// Tr98 sequence num
typedef enum {
	e_TR98_START = 100,
	e_TR98_IGD = e_TR98_START,
#ifdef CONFIG_PACKAGE_ZyIMS
	e_TR98_SERVICE                             ,
	e_TR98_VOICE_SRV                           ,
	e_TR98_VOICE_CAPB                          ,
	e_TR98_VOICE_CAPB_SIP                      ,
//	e_TR98_VOICE_CAPB_MGCP                     ,
//	e_TR98_VOICE_CAPB_H323                     ,
	e_TR98_VOICE_CAPB_CODEC                    ,
	e_TR98_VOICE_PROF                          ,
//	e_TR98_VOICE_PROF_SRV_PRO_INFO             ,
	e_TR98_VOICE_PROF_SIP                      ,
//	e_TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ        ,
//	e_TR98_VOICE_PROF_SIP_RESP_MAP_OBJ         ,
	e_TR98_VOICE_PROF_RTP                      ,
	e_TR98_VOICE_PROF_RTP_RTCP                 ,
	e_TR98_VOICE_PROF_RTP_SRTP                 ,
//	e_TR98_VOICE_PROF_RTP_REDUNDANCY           ,
	e_TR98_VOICE_PROF_NUM_PLAN                 ,
	e_TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO     ,
//	e_TR98_VOICE_PROF_TONE                     ,
//	e_TR98_VOICE_PROF_TONE_EVENT               ,
//	e_TR98_VOICE_PROF_TONE_DESCRIPTION         ,
//	e_TR98_VOICE_PROF_TONE_PATTERN             ,
	e_TR98_VOICE_PROF_FAX_T38                  ,
	e_TR98_VOICE_LINE                          ,
	e_TR98_VOICE_LINE_SIP                      ,
//	e_TR98_VOICE_LINE_SIP_EVENT_SUBS           ,
//	e_TR98_VOICE_LINE_RINGER                   ,
//	e_TR98_VOICE_LINE_RINGER_EVENT             ,
//	e_TR98_VOICE_LINE_RINGER_DESCRIPTION       ,
//	e_TR98_VOICE_LINE_RINGER_PATTERN           ,
	e_TR98_VOICE_LINE_CALLING_FEATURE          ,
	e_TR98_VOICE_LINE_PROCESSING               ,
	e_TR98_VOICE_LINE_CODEC                    ,
	e_TR98_VOICE_LINE_CODEC_LIST               ,
//	e_TR98_VOICE_LINE_SESSION                  ,
	e_TR98_VOICE_LINE_STATS                    ,
	e_TR98_VOICE_PHY_INTF                      ,
	e_TR98_VOICE_FXS_CID                       ,
//	e_TR98_VOICE_PSTN                          ,
	e_TR98_VOICE_COMMON                        ,
//	e_TR98_VOICE_PHONE_BOOK                    ,
	e_TR98_VOICE_CALL_POLICY_BOOK              ,
//	e_TR98_VOICE_FXO                           ,
//	e_TR98_VOICE_FXO_PORT                      ,
#endif
#ifdef AVAST_AGENT_PACKAGE
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
	e_TR98_ISP_SERVICE             ,
#endif
#endif
	e_TR98_SVR_SELECT_DIAG         ,
	e_TR98_CAPB                    ,
	e_TR98_PERF_DIAG               ,
	e_TR98_DEV_INFO                ,
	e_TR98_MemoryStatus            ,
	e_TR98_VEND_CONF_FILE          ,
	e_TR98_DEV_INFO_PS_STAT         ,
	e_TR98_DEV_INFO_PS_STAT_PS      ,
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	e_TR98_DEV_CONF         ,
#endif
//	e_TR98_DEV_CONF                ,
	e_TR98_MGMT_SRV                ,
	e_TR98_MGAB_DEV                ,
	e_TR98_TIME                    ,
//	e_TR98_USR_INTF                ,
//	e_TR98_CAPT_PORTAL             ,
	e_TR98_L3_FWD                  ,
	e_TR98_FWD                     ,
#if defined(L3IPV6FWD) && !defined(CONFIG_TAAAB_CUSTOMIZATION_TR069)
	e_TR98_IPV6FWD                 ,
#endif
	e_TR98_L2_BR                   ,
	e_TR98_BR                      ,
	e_TR98_PORT                    ,
	e_TR98_VLAN                    ,
	e_TR98_FILTER                  ,
	e_TR98_MARK                    ,
	e_TR98_AVAI_INTF               ,
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
	e_TR98_QUE_MGMT                ,
	e_TR98_CLS                     ,
//	e_TR98_APP                     ,
//	e_TR98_FLOW                    ,
	e_TR98_POLICER                 ,
	e_TR98_QUE                     ,
	e_TR98_SHAPER                  ,
//	e_TR98_QUE_STAT                ,
#endif
	e_TR98_LAN_CONF_SEC            ,
	e_TR98_IP_PING_DIAG            ,
	e_TR98_TRA_RT_DIAG             ,
	e_TR98_RT_HOP                  ,
	e_TR98_NSLOOKUP_DIAG           ,
	e_TR98_NSLOOKUP_RESULT         ,
	e_TR98_DL_DIAG                 ,
	e_TR98_DL_DIAG_PER_CONN_RST    ,
	e_TR98_DL_DIAG_INCREM_RST      ,
	e_TR98_UL_DIAG                 ,
	e_TR98_UL_DIAG_PER_CONN_RST    ,
	e_TR98_UL_DIAG_INCREM_RST      ,
	e_TR98_UDP_ECHO_CONF           ,
	e_TR98_UDP_ECHO_DIAG           ,
	e_TR98_UDP_ECHO_DIAG_PAK_RST,
	e_TR98_LAN_DEV                 ,
	e_TR98_LAN_HOST_CONF_MGMT      ,
	e_TR98_IP_INTF                 ,
#ifdef LANIPALIAS
	e_TR98_IP_ALIAS                ,
#endif
#ifdef IPV6INTERFACE_PROFILE
	e_TR98_IP_INTF_IPV6ADDR        ,
	e_TR98_IP_INTF_IPV6PREFIX      ,
#endif
#ifdef ROUTERADVERTISEMENT_PROFILE
	e_TR98_IP_INTF_ROUTERADVER     ,
#endif
#ifdef IPV6SERVER_PROFILE
	e_TR98_IP_INTF_DHCPV6SRV       ,
#endif
	e_TR98_DHCP_STATIC_ADDR        ,
	e_TR98_DHCP_OPT                ,
#ifdef ZyXEL_DHCP_OPTION125
	e_TR98_VENDOR_SPECIFIC,
	e_TR98_VENDOR_SPECIFIC_PROF,	
#endif	
	e_TR98_DHCP_COND_SERVPOOL      ,
	e_TR98_SERVPOOL_DHCP_STATICADDR,
	e_TR98_SERVPOOL_DHCP_OPT       ,
	e_TR98_LAN_ETH_INTF_CONF       ,
	e_TR98_LAN_ETH_INTF_CONF_STAT  ,
//	e_TR98_LAN_USB_INTF_CONF       ,
//	e_TR98_LAN_USB_INTF_CONF_STAT  ,
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	e_TR98_LAN_DEV_WLAN_CFG        ,
	e_TR98_WLAN_CFG_STAT           ,
	e_TR98_WPS                     ,
#endif
//	e_TR98_REG                     ,
	e_TR98_ASSOC_DEV               ,
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	e_TR98_ASSOC_DEV_STAT          ,
	e_TR98_WEP_KEY                 ,
	e_TR98_PSK                     ,
#endif
#ifdef ZYXEL_SUPPORT_8021X
	e_TR98_RADIUS                  ,
#endif
//	e_TR98_AP_WMM_PARAM            ,
//	e_TR98_STA_WMM_PARAM           ,
	e_TR98_HOSTS                   ,
	e_TR98_HOST                    ,
#ifdef ONECONNECT
	e_TR98_ONECONNECT_EXT          ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	e_TR98_NEIBOR_WIFI_DIAG        ,
	e_TR98_NEIBOR_WIFI_DIAG_RESULT ,
#endif
	e_TR98_LAN_INTF                ,
//	e_TR98_LAN_INTF_ETH_INTF_CONF  ,
//	e_TR98_USB_INTF_CONF           ,
//	e_TR98_LAN_INTF_WLAN_CFG       ,
	e_TR98_WAN_DEV                 ,
	e_TR98_WAN_COMM_INTF_CONF      ,
//	e_TR98_WAN_COMM_INTF_CONNECT   ,
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	e_TR98_WAN_DSL_INTF_CONF       ,
	e_TR98_WAN_DSL_INTF_TEST_PARAM ,
	e_TR98_WAN_DSL_INTF_CONF_STAT  ,
	e_TR98_TTL                     ,
	e_TR98_ST                      ,
//	e_TR98_LST_SHOWTIME            ,
	e_TR98_CURRENT_DAY             ,
	e_TR98_QTR_HR                  ,
#endif // CONFIG_ZCFG_BE_MODULE_DSL
	e_TR98_WAN_ETH_INTF_CONF       ,
	e_TR98_WAN_ETH_INTF_CONF_STAT  ,
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	e_TR98_WAN_DSL_DIAG            ,
#endif
	e_TR98_WAN_CONN_DEV            ,
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
	e_TR98_WAN_DSL_LINK_CONF       ,
	e_TR98_WAN_ATM_F5_LO_DIAG      ,
	e_TR98_WAN_ATM_F4_LO_DIAG      ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
	e_TR98_WAN_PTM_LINK_CONF       ,
	e_TR98_WAN_PTM_LINK_CONF_STAT  ,
#endif
	e_TR98_WAN_ETH_LINK_CONF       ,
//	e_TR98_WAN_POTS_LINK_CONF      ,
	e_TR98_WAN_IP_CONN             ,
	e_TR98_WAN_IP_CONN_MLD		   , 
	e_TR98_DHCP_CLIENT             ,
	e_TR98_SENT_DHCP_OPT           ,
	e_TR98_REQ_DHCP_OPT            ,
	e_TR98_WAN_IP_PORT_MAP         ,
	e_TR98_WAN_IP_PORT_TRIGGER     ,
	e_TR98_WAN_IP_ADDR_MAPPING     ,
	e_TR98_WAN_IP_CONN_STAT        ,
#ifdef IPV6INTERFACE_PROFILE
	e_TR98_WAN_IP_CONN_IPV6ADDR    ,
	e_TR98_WAN_IP_CONN_IPV6PREFIX	 ,
#endif
#ifdef DHCPV6CLIENT_PROFILE
	e_TR98_WAN_IP_CONN_DHCPV6CLIENT,
#endif
#ifdef IPV6RD_PROFILE
	e_TR98_WAN_IP_CONN_IPV6RD	     ,
#endif
#ifdef ROUTEINFO_INTERFACE
	e_TR98_WAN_IP_CONN_ROUTEINFO	 ,
#endif
	e_TR98_WAN_PPP_CONN            ,
	e_TR98_WAN_PPP_PORT_MAP        ,
	e_TR98_WAN_PPP_PORT_TRIGGER    ,
	e_TR98_WAN_PPP_ADDR_MAPPING    ,
	e_TR98_WAN_PPP_CONN_STAT       ,
	e_TR98_WAN_PPP_CONN_MLD		   ,
#ifdef IPV6INTERFACE_PROFILE
	e_TR98_WAN_PPP_CONN_IPV6ADDR    ,
	e_TR98_WAN_PPP_CONN_IPV6PREFIX  ,
#endif
#ifdef DHCPV6CLIENT_PROFILE
	e_TR98_WAN_PPP_CONN_DHCPV6CLIENT,
#endif
#ifdef IPV6RD_PROFILE
	e_TR98_WAN_PPP_CONN_IPV6RD      ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
	e_TR98_WWAN_INTERFACE_CONFIG ,
	e_TR98_WWAN_INTERFACE_CONFIG_STATUS,
#endif
	e_TR98_WWAN_EMAIL_CONFIG ,
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
	e_TR98_WAN_PON_INTF_CONF       ,
	e_TR98_WAN_PON_INTF_CONF_STAT  ,
#endif
	e_TR98_FIREWALL                ,
	e_TR98_FIREWALL_LEVEL          ,
	e_TR98_FIREWALL_CHAIN          ,
	e_TR98_FIREWALL_CHAIN_RULE     ,
	e_TR98_STD_UPNP		           ,
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
	e_TR98_STD_UPNP_DEV		       ,
#endif
#ifdef ZYXEL_IPV6SPEC
	e_TR98_IPV6SPEC					,
	e_TR98_IPV6_PPP					,
	e_TR98_IPV6_PPP_INTF			,
	e_TR98_IPV6_PPP_INTF_IPV6CP		,
	e_TR98_IPV6_IP					,
	e_TR98_IPV6_IP_INTF				,
	e_TR98_IPV6_IP_INTF_IPV6ADDR	,
	e_TR98_IPV6_IP_INTF_IPV6PREFIX	,
	e_TR98_ROUTING					,
	e_TR98_ROUTER					,
	e_TR98_ROUTER_IPV6FWD			,
	e_TR98_ROUTEINFO				,
	e_TR98_ROUTEINFO_INTF			,
	e_TR98_ND						,
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
	e_TR98_ND_INTF					,
#endif
	e_TR98_RTADR					,
	e_TR98_RTADR_INTF				,
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
	e_TR98_RTADR_INTF_OPT			,
#endif
	e_TR98_IPV6_HOSTS				,
	e_TR98_IPV6_HOSTS_HOST			,
	e_TR98_IPV6_HOSTS_IPV6ADR		,
	e_TR98_IPV6_DNS					,
	e_TR98_IPV6_DNS_CLIENT			,
	e_TR98_IPV6_DNS_SERVER			,
#ifdef CONFIG_ZCFG_BE_MODULE_DNS_RELAY
	e_TR98_IPV6_DNS_RELAY			,
	e_TR98_IPV6_DNS_RELAYFWD		,
#endif
	e_TR98_IPV6_DHCPV6				,
	e_TR98_IPV6_DHCPV6_CLIENT		,
	e_TR98_IPV6_DHCPV6_CLIENT_SRV	,
	e_TR98_IPV6_DHCPV6_CLIENT_SENT	,
	e_TR98_IPV6_DHCPV6_CLIENT_RCV	,
	e_TR98_IPV6_DHCPV6_SERVER		,
	e_TR98_IPV6_DHCPV6_SERVER_POOL	,
	e_TR98_IPV6_DHCPV6_SERVER_CNT	,
	e_TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR	,
	e_TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX	,
	e_TR98_IPV6_DHCPV6_SERVER_CNT_OPT	,
	e_TR98_IPV6_DHCPV6_SERVER_OPT	,
#endif
#ifdef ZYXEL_IPV6_MAP
	e_TR98_IPV6MAP                  ,
	e_TR98_IPV6MAP_DOMAIN           ,
	e_TR98_IPV6MAP_DOMAIN_RULE      ,
#endif
#ifdef ZYXEL_GRE_SUPPORT
	e_TR98_GRE              		,
	e_TR98_GRE_TUNNEL				,
	e_TR98_GRE_TUNNEL_STATS			,
#endif
#ifdef ZYXEL_TAAAG_TR69ALG
	e_TR98_TAAAB_H323Alg		   ,
	e_TR98_TAAAB_SIPAlg		   ,
	e_TR98_TAAAB_RTSPAlg		   ,
	e_TR98_TAAAB_PPTPAlg		   ,
	e_TR98_TAAAB_IPSecAlg		   ,
	e_TR98_TAAAB_L2TPAlg		   ,
	e_TR98_TAAAB_IGMPProxy		   ,
	e_TR98_TAAAB_SMTP			   ,
	e_TR98_TAAAB_ICMP       ,
	e_TR98_TAAAB_NTP			   ,
#endif
#ifdef ZYXEL_TAAAG_EDNS//Add ppp username to edns option for TAAAB
	e_TR98_TAAAB_EDNS			   ,
#endif
#ifdef CONFIG_TAAAB_PORTMIRROR	
    e_TR98_TAAAB_PORTMIRROR            ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
	e_TR98_TAAAB_PACKETCAPTURE 	   ,
	e_TR98_TAAAB_PACKETCAPTUREUPLOAD  ,
#endif
#if ZYXEL_EXT
	e_TR98_ZYXEL_EXT               ,
#ifdef ZYXEL_LANDING_PAGE_FEATURE
	e_TR98_LANDING_PAGE 		   ,
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
	e_TR98_ONLINE_FWUPGRADE 		   ,
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
	e_TR98_WLAN_SCHEDULER 		   ,
	e_TR98_WLAN_SCHEDULER_Rule 		   ,
#endif
	e_TR98_FEATURE_FLAG 		   ,
	e_TR98_DNS                     ,
	e_TR98_DNS_RT_ENTRY            ,
	e_TR98_DNS_ENTRY			   ,
	e_TR98_D_DNS                   ,
#ifdef ZCFG_TR64
	e_TR98_TR064				   ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
	e_TR98_VLAN_GROUP			   ,
#endif
	e_TR98_EMAIL_NOTIFY			   ,
	e_TR98_EMAIL_SERVICE		   ,
	e_TR98_EMAIL_EVENT_CFG		   ,
	e_TR98_SYS_INFO				   ,
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
    e_TR98_GPON_INFO               ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
	e_TR98_SNMP					   ,
	e_TR98_SNMP_TRAP			   ,
#endif
	e_TR98_SCHEDULE                ,
	e_TR98_REMO_MGMT               ,
	e_TR98_REMO_SRV                ,
	e_TR98_SP_REMO_SRV               ,
	e_TR98_REMO_TRUSTDOMAIN        ,
	e_TR98_SP_TRUSTDOMAIN        ,
#ifdef ABUU_CUSTOMIZATION	
	e_TR98_IFACETRUSTDOMAIN        ,
	e_TR98_IFACETRUSTDOMAININFO    ,
#endif
	e_TR98_IEEE8021AG              ,
#ifdef ZYXEL_ETHOAM_TMSCTL
	e_TR98_IEEE8023AH              ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
	e_TR98_SAMBA		           ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
	e_TR98_UPNP		           ,
#endif
	e_TR98_IGMP                    ,
	e_TR98_MLD                     ,	
#ifdef ZYXEL_TR69_DATA_USAGE
	e_TR98_DATA_USAGE                   ,
	e_TR98_DATA_USAGE_LAN               ,
	e_TR98_DATA_USAGE_WAN               ,
#endif
#if defined(SAAAD_RANDOM_PASSWORD) || defined(SOL_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
	e_TR98_LOG_CFG           		 ,
	e_TR98_LOG_CFG_GP           	 ,
	e_TR98_LOG_CFG_GP_ACCOUNT       ,
#endif
	e_TR98_LOG_SETTING             ,
	e_TR98_LOG_CLASSIFY            ,
	e_TR98_LOG_CATEGORY	     ,
	e_TR98_MACFILTER				,
	e_TR98_MACFILTER_WHITELIST		,
	e_TR98_PAREN_CTL			   ,
	e_TR98_PAREN_CTL_PROF 		   ,
	e_TR98_SEC			 		   ,
	e_TR98_SEC_CERT		 		   ,
	e_TR98_GUI_CUSTOMIZATION		,
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	e_TR98_DSL,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	e_TR98_ONECONNECT,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
	e_TR98_ONECONNECT_SPEEDTESTINFO,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
	e_TR98_AP_STEERING              ,
#endif 
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
	e_TR98_PROXIMITY_SENSOR         ,
#endif
#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
        e_TR98_BANDWIDTH_IMPROVEMENT         ,
#endif
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	e_TR98_WLANCONFIG,
	e_TR98_AIRTIMEFAIRNESS,
#endif
	e_TR98_ACCOUNT_RESETPASSWD,
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#if OBSOLETED
#else
//	e_TR98_WAN_DSL_CONN_MGMT       ,
//	e_TR98_WAN_DSL_CONN_SRVC       ,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
	e_TR98_PRINT_SERVER,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
	e_TR98_IPK_PKG,
	e_TR98_IPK_PKG_INSTALL,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
	e_TR98_WiFiCommunity,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
	e_TR98_EasyMesh	,
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
	e_TR98_VendorEasyMesh,
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
	e_TR98_ONESSID,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
	e_TR98_SFP_MGMT,
#endif
#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
	e_TR98_TAAAB_Configuration_Button,
	e_TR98_TAAAB_Configuration_Button_WiFi,
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
	e_TR98_TAAAB_Configuration_Quantwifi,
#endif
#ifdef ZYXEL_XMPP
       e_TR98_XMPP          ,
       e_TR98_XMPP_CONNECTION          ,
       e_TR98_XMPP_CONNECTION_SERVER          ,
#endif
#ifdef MSTC_TAAAG_MULTI_USER
	e_TR98_Nat_DMZ		,
#endif
#if defined(ZYXEL_IGMPDIAG) || defined(ZYXEL_CUSTOMIZATION_SYSLOG) || defined(ZYXEL_TAAAB_WAN_CONN_DIAG)
    e_TR98_TAAAB_Function          ,
#endif
#ifdef ZYXEL_IGMPDIAG
    e_TR98_TTIGMPDiagnostics          ,
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
    e_TR98_TAAAB_WanConnDiagnostics          ,
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
    e_TR98_TAAAB_Syslog          ,
    e_TR98_TAAAB_SyslogUpload          ,
#endif
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
	e_TR98_TT		,
	e_TR98_TAAAB_Users		,
	e_TR98_TAAAB_Users_User		,
	e_TR98_TAAAB_UserInterface		,
	e_TR98_TAAAB_UserInterface_RemoteAccess		,
	e_TR98_TAAAB_UserInterface_LocalAccess		,
	e_TR98_TAAAB_Firewall		,
	e_TR98_TAAAB_UPnP		,
	e_TR98_TAAAB_UPnP_Device		,
	e_TR98_TAAAB_Configuration,
	e_TR98_TAAAB_Configuration_GUI		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture		,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess,
	e_TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess,
	e_TR98_TAAAB_Configuration_Shell		,
#endif
#ifdef CONFIG_ZYXEL_TR140
#ifndef CONFIG_PACKAGE_ZyIMS
				e_TR98_SERVICE					  ,
#endif
		e_TR98_StorageService			  ,
		e_TR98_StorageService_UserAccount 			,
		e_TR98_StorageService_Capabilites			  ,
		e_TR98_StorageService_NetInfo			  ,
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
		e_TR98_StorageService_NetworkServer 			,
#endif
		e_TR98_StorageService_FTPServer 			,
		e_TR98_StorageService_PhysicalMedium			 ,
		e_TR98_StorageService_LogicalVolume 		   ,
		e_TR98_StorageService_LogicalVolume_Folder			  ,
#endif
#if defined(ZYXEL_SUPPORT_LIFEMOTE) || defined(X_ZYXEL_SUPPORT_LIFEMOTE)
	e_TR98_Configuration_Lifemote         ,
#endif
#ifdef CONFIG_ZYXEL_MODSW_ZYEE
    e_TR98_ZyEE_SoftwareMoudles			                        ,
    e_TR98_ZyEE_SoftwareMoudles_EeConf                          ,
	e_TR98_ZyEE_SoftwareMoudles_ExecEnv		                    ,
    e_TR98_ZyEE_SoftwareMoudles_DeploymentUnit		            ,
	e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit		            ,
    e_TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions        ,
    e_TR98_ZyEE_DUStateChangeComplPolicy                        ,
#endif
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
	e_TR98_MultipleConfig   ,
#endif
#ifdef CONFIG_ZYXEL_CELLWAN_SUPPORT
	e_TR98_CELL,
	e_TR98_CELL_INTF,
	e_TR98_CELL_INTF_USIM,
	e_TR98_CELL_AP,
#endif
#ifdef CONFIG_ZYXEL_MULTI_WAN
	e_TR98_WAN_MULTI_WAN,
	e_TR98_WAN_MULTI_WAN_INTF,
#endif
#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
	e_TR98_RUNNING_WAN_CONN        ,
#endif
	e_TR98_WAN_IP_V4ROUTINGTABLE   ,
	e_TR98_WAN_IP_V6ROUTINGTABLE   ,
	e_TR98_LAST
} zcfg98sn_t;
#endif
