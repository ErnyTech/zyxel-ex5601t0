/*Parameter*/
extern tr98Parameter_t para_extend[];
extern tr98Parameter_t para_Dns[];
extern tr98Parameter_t para_DnsRtEntry[];
extern tr98Parameter_t para_DnsEntry[];
extern tr98Parameter_t para_DDns[];
extern tr98Parameter_t para_Tr064[];
extern tr98Parameter_t para_EmailSrv[];
extern tr98Parameter_t para_EmailEvent[];
extern tr98Parameter_t para_VlanGroup[];
extern tr98Parameter_t para_SysInfo[];
#ifdef ZYXEL_SFP_MODULE_SUPPORT
extern tr98Parameter_t para_GponInfo[];
#endif
extern tr98Parameter_t para_Snmp[];
extern tr98Parameter_t para_SnmpTrap[];
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER) || defined(GFIBER_CUSTOMIZATION)
extern tr98Parameter_t para_RemoMgmt[];
#endif
extern tr98Parameter_t para_RemoSrv[];
extern tr98Parameter_t para_SpRemoSrv[];
extern tr98Parameter_t para_8021ag[];
#ifdef ZYXEL_ETHOAM_TMSCTL
extern tr98Parameter_t para_8023ah[];
#endif
extern tr98Parameter_t para_RemoTrustDomain[];
extern tr98Parameter_t para_SpTrustDomain[];
#ifdef ABUU_CUSTOMIZATION
extern tr98Parameter_t para_IfaceTrustDomain[];
extern tr98Parameter_t para_IfaceTrustDomainInfo[];
#endif
extern tr98Parameter_t para_Igmp[];
extern tr98Parameter_t para_Mld[];
#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
extern tr98Parameter_t para_LogCfg[];
#endif
extern tr98Parameter_t para_LogCfgGp[];
extern tr98Parameter_t para_LogCfgGpAccount[];
#endif
extern tr98Parameter_t para_LogSetting[];
extern tr98Parameter_t para_LogClassify[];
extern tr98Parameter_t para_LogCategory[];
extern tr98Parameter_t para_MacFilter[];
extern tr98Parameter_t para_MacFilter_WhiteList[];
extern tr98Parameter_t para_ParenCtl[];
extern tr98Parameter_t para_ParenCtlProf[];
extern tr98Parameter_t para_Sec[];
extern tr98Parameter_t para_SecCert[];
extern tr98Parameter_t para_Samba[];
extern tr98Parameter_t para_Upnp[];
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
extern tr98Parameter_t para_WlanScheduler[];
extern tr98Parameter_t para_WlanSchedulerRule[];
#endif
#ifdef ZYXEL_IPV6_MAP
extern tr98Parameter_t para_MAP[];
extern tr98Parameter_t para_MAPDomain[];
extern tr98Parameter_t para_MAPDomainRule[];
#endif

#ifdef ZyXEL_IPP_PRINTSERVER
extern tr98Parameter_t para_Ipp_PrintServer[];
#endif

#ifdef ONECONNECT
extern tr98Parameter_t para_OneConnect[];
extern tr98Parameter_t para_SpeedTestInfo[];
#endif

#ifdef ZYXEL_AP_STEERING
extern tr98Parameter_t para_ApSteering[];
#endif //#ifdef ZYXEL_AP_STEERING

#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
extern tr98Parameter_t para_ProximitySensor[];
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR

#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT 
extern tr98Parameter_t para_BandwidthImprovement[];
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT

extern tr98Parameter_t para_Landing_Page[];
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
extern tr98Parameter_t para_Online_Firmware_Upgrade[];
#endif
extern tr98Parameter_t para_Feature_Flag[];
extern tr98Parameter_t para_GUI_Customization[];

extern tr98Parameter_t para_DSL[];

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
extern tr98Parameter_t para_WLANConfiguration[];
extern tr98Parameter_t para_AccountResetPassword[];
extern tr98Parameter_t para_AirtimeFairness[];
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
extern tr98Parameter_t para_EasyMesh[];
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
extern tr98Parameter_t para_VendorEasyMesh[];
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif

extern tr98Parameter_t para_WiFiCommunity[];

#ifdef ZYXEL_ONESSID
extern tr98Parameter_t para_OneSsid[];
#endif

#ifdef ZYXEL_TR69_DATA_USAGE
extern tr98Parameter_t para_DataUsage[];
extern tr98Parameter_t para_DataUsageLan[];
extern tr98Parameter_t para_DataUsageWan[];
#endif
#ifdef ZYXEL_TAAAG_TR69ALG/////MSTC_TTALG, RichardFeng
extern tr98Parameter_t para_H323Alg[];
extern tr98Parameter_t para_SIPAlg[];
extern tr98Parameter_t para_RTSPAlg[];
extern tr98Parameter_t para_PPTPAlg[];
extern tr98Parameter_t para_IPSecAlg[];
extern tr98Parameter_t para_L2TPAlg[];
extern tr98Parameter_t para_IGMPProxy[];
extern tr98Parameter_t para_SMTP[];
extern tr98Parameter_t para_ICMP[];
extern tr98Parameter_t para_NTP[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
extern tr98Parameter_t para_SfpMgmt[];
#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
extern tr98Parameter_t para_EDNS[];
#endif

#ifdef MSTC_TAAAG_MULTI_USER
extern tr98Parameter_t para_NatDMZOpt[];
#endif

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
extern tr98Parameter_t para_Users[];
extern tr98Parameter_t para_User[];
extern tr98Parameter_t para_UserInterface[];
extern tr98Parameter_t para_RemoteLocalAccess[];
extern tr98Parameter_t para_TTFirewall[];
extern tr98Parameter_t para_UPnPDevice[];
extern tr98Parameter_t para_GUIAccessRights[];
extern tr98Parameter_t para_Shell[];
#endif

#ifdef ZYXEL_IGMPDIAG 
extern tr98Parameter_t para_IGMPDiagnostics[];
#endif

#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG 
extern tr98Parameter_t para_TTWanConnDiagnostics[];
#endif

#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
extern tr98Parameter_t para_Syslog[];
extern tr98Parameter_t para_SyslogUpload[];
#endif

#if (defined(CONFIG_TAAAB_PORTMIRROR) || defined(CONFIG_TAAAB_PORTMIRROR_E))
extern tr98Parameter_t para_TTPORTMIRROR[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
extern tr98Parameter_t para_TTPACKETCAPTURE[];
extern tr98Parameter_t para_TTPACKETCAPTUREUPLOAD[];
#endif

#ifdef CONFIG_ZYXEL_TR140
extern tr98Parameter_t para_SS[];
extern tr98Parameter_t para_SSUserAccount[];
extern tr98Parameter_t para_SSCapabilites[];
extern tr98Parameter_t para_SSNetInfo[];
extern tr98Parameter_t para_SSNetworkServer[];
extern tr98Parameter_t para_SSFTPServer[];
extern tr98Parameter_t para_SSPhysicalMedium[];
extern tr98Parameter_t para_SSLogicalVolume[];
extern tr98Parameter_t para_SSFolder[];
#endif

#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
extern tr98Parameter_t para_HwWiFiButton[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
extern tr98Parameter_t para_Lifemote[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
extern tr98Parameter_t para_Quantwifi[];
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069 
extern tr98Parameter_t para_DevConf[];
#endif

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
extern tr98Parameter_t para_MultipleConfig[];
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
extern tr98Parameter_t para_Cell[];
extern tr98Parameter_t para_CellIntf[];
extern tr98Parameter_t para_CellIntfUSIM[];
extern tr98Parameter_t para_CellAccessPoint[];
extern zcfgRet_t zyExtCellObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtCellIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtCellIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtCellIntfUSIMObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtCellIntfUSIMObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtCellAccessPointObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtCellAccessPointObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
extern tr98Parameter_t para_MultiWan[];
extern tr98Parameter_t para_MultiWanIntf[];
/* Why put the related information separate in this file? */
extern zcfgRet_t zyExtMultiWanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMultiWanObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtMultiWanIntfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMultiWanIntfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtMultiWanIntfObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtMultiWanIntfObjDel(char *tr98FullPathName);
#endif


extern tr98Parameter_t para_IPV4RoutingTable[];
extern tr98Parameter_t para_IPV6RoutingTable[];

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
extern tr98Parameter_t para_Ipk_Package[];
extern tr98Parameter_t para_Ipk_Package_Install[];
#endif
/*Handler Function*/

//InternetGatewayDevice.X_ZYXEL_EXT.
zcfgRet_t zyExtObjGet(char *, int , struct json_object **, bool );
zcfgRet_t zyExtObjSet(char *, int , struct json_object *, struct json_object *, char *);

//InternetGatewayDevice.X_ZYXEL_EXT.DNS
extern zcfgRet_t zyExtDnsObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtDnsObjSet(char *, int , struct json_object *, struct json_object *, char *);

/*InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i*/
extern zcfgRet_t zyExtDnsRtObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtDnsRtObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtDnsRtObjAdd(char *, int *);
extern zcfgRet_t zyExtDnsRtObjDel(char *);
extern zcfgRet_t zyExtDnsRtObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtDnsRtObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtDnsRtObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.i*/
extern zcfgRet_t zyExtDnsEntryObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtDnsEntryObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtDnsEntryObjAdd(char *, int *);
extern zcfgRet_t zyExtDnsEntryObjDel(char *);
extern zcfgRet_t zyExtDnsEntryObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtDnsEntryObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtDnsEntryObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.DynamicDNS*/
extern zcfgRet_t zyExtDDnsObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtDDnsObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtDDnsObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtDDnsObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtDDnsObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.TR064*/
extern zcfgRet_t zyExtTr064ObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtTr064ObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtTr064ObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtTr064ObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtTr064ObjAttrSet(char *, int, char *, int , struct json_object *);

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
/*InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i*/
extern zcfgRet_t zyExtVlanGroupObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtVlanGroupObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtVlanGroupObjAdd(char *, int *);
extern zcfgRet_t zyExtVlanGroupObjDel(char *);
extern zcfgRet_t zyExtVlanGroupObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtVlanGroupObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtVlanGroupObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

/*InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i*/
extern zcfgRet_t zyExtEmailSrvObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtEmailSrvObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtEmailSrvObjAdd(char *, int *);
extern zcfgRet_t zyExtEmailSrvObjDel(char *);
extern zcfgRet_t zyExtEmailSrvObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtEmailSrvObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtEmailSrvObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i*/
extern zcfgRet_t zyExtEmailEventObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtEmailEventObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtEmailEventObjAdd(char *, int *);
extern zcfgRet_t zyExtEmailEventObjDel(char *);
extern zcfgRet_t zyExtEmailEventObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtEmailEventObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtEmailEventObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.System_Info*/
extern zcfgRet_t zyExtSysInfoObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtSysInfoObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtSysInfoObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSysInfoObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSysInfoObjAttrSet(char *, int, char *, int , struct json_object *);

#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
/*InternetGatewayDevice.X_ZYXEL_EXT.Gpon_Info*/
extern zcfgRet_t zyExtGponInfoObjGet(char *, int, struct json_object **, bool);
#endif

/*InternetGatewayDevice.X_ZYXEL_EXT.SNMP*/
extern zcfgRet_t zyExtSnmpObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtSnmpObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtSnmpObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSnmpObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSnmpObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i*/
extern zcfgRet_t zyExtSnmpTrapObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtSnmpTrapObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtSnmpTrapObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtSnmpTrapObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtSnmpTrapObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSnmpTrapObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSnmpTrapObjAttrSet(char *, int, char *, int , struct json_object *);
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER) || defined(GFIBER_CUSTOMIZATION)
/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.*/
extern zcfgRet_t zyExtRemoteMgmtObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtRemoteMgmtObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif
#ifdef GFIBER_CUSTOMIZATION
extern zcfgRet_t zyExtRemoteMgmtObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtRemoteMgmtObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtRemoteMgmtObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.i*/
extern zcfgRet_t zyExtRemoteMgmtSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtRemoteMgmtSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtRemoteMgmtSrvObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtRemoteMgmtSrvObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtRemoteMgmtSrvObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtRemoteMgmtSrvObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtRemoteMgmtSrvObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpService.i*/
extern zcfgRet_t zyExtSpRemoteMgmtSrvObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtSpRemoteMgmtSrvObjDelete(char *tr98FullPathName);
extern zcfgRet_t zyExtSpRemoteMgmtSrvObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSpRemoteMgmtSrvObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *);

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.TrustDomain.i*/
extern zcfgRet_t zyExtRemoteMgmtTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtRemoteMgmtTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtRemoteMgmtTrustDomainObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtRemoteMgmtTrustDomainObjDel(char *tr98FullPathName);

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpTrustDomain.i*/
extern zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtSpRemoteMgmtTrustDomainObjDel(char *tr98FullPathName);

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i*/
#ifdef ABUU_CUSTOMIZATION
extern zcfgRet_t zyExtIfaceTrustDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtIfaceTrustDomainObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtIfaceTrustDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtIfaceTrustDomainObjDel(char *tr98FullPathName);

/*InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i.InterfaceTrustDomainInfo.i*/
extern zcfgRet_t zyExtIfaceTrustDomainInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtIfaceTrustDomainInfoObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtIfaceTrustDomainInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtIfaceTrustDomainInfoObjDel(char *tr98FullPathName);
#endif

extern zcfgRet_t zyExtIgmpObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtIgmpObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtIgmpObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtIgmpObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtIgmpObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtMldObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtMldObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtMldObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtMldObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtMldObjAttrSet(char *, int, char *, int , struct json_object *);

#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
extern zcfgRet_t zyExtLogCfgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtLogCfgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif
extern zcfgRet_t zyExtLogCfgGpObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtLogCfgGpObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtLogCfgGpAccountObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtLogCfgGpAccountObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtLogCfgGpAccountObjAdd(char *tr98FullPathName, int *idx);
#if defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT) || defined(SAAAE_TR69_LOGINACCOUNT)
extern zcfgRet_t zyExtLogCfgGpAccountObjDel(char *tr98FullPathName);
#endif
#endif

/*InternetGatewayDevice.X_ZYXEL_EXT.8021ag*/
extern zcfgRet_t zyExt8021agObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExt8021agObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExt8021agObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExt8021agObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExt8021agObjAttrSet(char *, int, char *, int , struct json_object *);

#ifdef ZYXEL_ETHOAM_TMSCTL
/*InternetGatewayDevice.X_ZYXEL_EXT.8023ah*/
extern zcfgRet_t zyExt8023ahObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExt8023ahObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExt8023ahObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExt8023ahObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExt8023ahObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
/*InternetGatewayDevice.X_ZYXEL_EXT.SAMBA*/
extern zcfgRet_t zyExtSambaObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtSambaObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtSambaObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSambaObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSambaObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
/*InternetGatewayDevice.X_ZYXEL_EXT.UPNP*/
extern zcfgRet_t zyExtUPnPObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtUPnPObjSet(char *, int, struct json_object *, struct json_object *, char *);
#endif
/*InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting*/
extern zcfgRet_t zyExtLogSettingObjGet(char *, int , struct json_object **, bool );
extern zcfgRet_t zyExtLogSettingObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtLogSettingObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtLogSettingObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtLogSettingObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i*/
extern zcfgRet_t zyExtLogClassifyObjGet(char *, int , struct json_object **, bool );
extern zcfgRet_t zyExtLogClassifyObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtLogClassifyObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtLogClassifyObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtLogClassifyObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtLogClassifyObjAttrSet(char *, int, char *, int , struct json_object *);

/*InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i.Category.i*/
extern zcfgRet_t zyExtLogCategoryObjGet(char *, int , struct json_object **, bool );
extern zcfgRet_t zyExtLogCategoryObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtLogCategoryObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtLogCategoryObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtLogCategoryObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtLogCategoryObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtLogCategoryObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtMacFilterObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtMacFilterObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtMacFilterObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtMacFilterObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtMacFilterObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtMacFilterWhiteListObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t zyExtMacFilterWhiteListObjSet(char *, int , struct json_object *, struct json_object *, char *);
extern zcfgRet_t zyExtMacFilterWhiteListObjAdd(char *, int *);
extern zcfgRet_t zyExtMacFilterWhiteListObjDel(char *);
extern zcfgRet_t zyExtMacFilterWhiteListObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtMacFilterWhiteListObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtMacFilterWhiteListObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtParenCtlObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtParenCtlObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtParenCtlObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtParenCtlObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtParenCtlObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtParenCtlProfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtParenCtlProfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtParenCtlProfObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtParenCtlProfObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtParenCtlProfObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtParenCtlProfObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtParenCtlProfObjAttrSet(char *, int, char *, int , struct json_object *);

#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
//zyExtWlanSchedulerObjGet
extern zcfgRet_t zyExtWlanSchedulerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtWlanSchedulerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtWlanSchedulerObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtWlanSchedulerObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtWlanSchedulerObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtWlanSchedulerRuleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtWlanSchedulerRuleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtWlanSchedulerRuleObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtWlanSchedulerRuleObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtWlanSchedulerRuleObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtWlanSchedulerRuleObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtWlanSchedulerRuleObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

extern zcfgRet_t zyExtSecObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSecObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSecObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSecObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyExtSecCertObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSecCertObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtSecCertObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtSecCertObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtSecCertObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtSecCertObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtSecCertObjAttrSet(char *, int, char *, int , struct json_object *);

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
/*InternetGatewayDevice.X_ZYXEL_EXT.PrintServer */
extern zcfgRet_t zyExtIppPrintServObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtIppPrintServObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
extern zcfgRet_t zyExtOneConnectObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtOneConnectObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
extern zcfgRet_t zyExtSpeedTestInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSpeedTestInfoObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif

#ifdef ZYXEL_AP_STEERING
extern zcfgRet_t zyExtApSteeringObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtApSteeringObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif //#ifdef ZYXEL_AP_STEERING

#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
extern zcfgRet_t zyExtProximitySensorObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtProximitySensorObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR

#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT 
extern zcfgRet_t zyExtBandwidthImprovementObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtBandwidthImprovementObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
extern zcfgRet_t zyExtWLANConfigurationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtWLANConfigurationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);

extern zcfgRet_t zyExtAccountResetPasswordObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtAccountResetPasswordObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);

extern zcfgRet_t zyExtAirtimeFairnessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtAirtimeFairnessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

#ifdef ZYXEL_LANDING_PAGE_FEATURE
extern zcfgRet_t zyExtLandingPageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtLandingPageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *);
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
extern zcfgRet_t zyExtOnlineFWUgradeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtOnlineFWUgradeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif
extern zcfgRet_t zyExtFeatureFlagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtFeatureFlagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);

extern zcfgRet_t zyExtCustomizationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtCustomizationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);

#ifdef CONFIG_ZCFG_BE_MODULE_DSL
extern zcfgRet_t zyExtDSLGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtDSLSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif

#ifdef ZYXEL_EASYMESH
extern zcfgRet_t zyExtEasyMeshObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtEasyMeshObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif

extern zcfgRet_t zyExtWiFiCommunityObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtWiFiCommunityObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);

#ifdef ZYXEL_ONESSID
extern zcfgRet_t zyExtOneSsidObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtOneSsidObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
extern zcfgRet_t zyExtSfpMgmObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtSfpMgmObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
#endif

#ifdef ZYXEL_IPV6_MAP
extern zcfgRet_t zyExtMAPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMAPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *);
extern zcfgRet_t zyExtMAPDomainObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMAPDomainObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtMAPDomainObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtMAPDomainObjDel(char *tr98FullPathName);
extern zcfgRet_t zyExtMAPDomainRuleObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMAPDomainRuleObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtMAPDomainRuleObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyExtMAPDomainRuleObjDel(char *tr98FullPathName);
#endif

#ifdef ZYXEL_TR69_DATA_USAGE
extern zcfgRet_t zyExtDataUsageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtDataUsageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtDataUsageLanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtDataUsageWanObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
#endif

#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng 
extern zcfgRet_t zyTTH323AlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTH323AlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTH323AlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTH323AlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTH323AlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTSIPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTSIPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTSIPAlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTSIPAlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTSIPAlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTRTSPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTRTSPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTRTSPAlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTRTSPAlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTRTSPAlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTPPTPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTPPTPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTPPTPAlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTPPTPAlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTPPTPAlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTIPSecAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTIPSecAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTIPSecAlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTIPSecAlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTIPSecAlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTL2TPAlgObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTL2TPAlgObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTL2TPAlgObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTL2TPAlgObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTL2TPAlgObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTIGMPProxyObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTIGMPProxyObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTIGMPProxyObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTIGMPProxyObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTIGMPProxyObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTSMTPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTSMTPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTSMTPObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTSMTPObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTSMTPObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTICMPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTICMPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTICMPObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTICMPObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTICMPObjAttrSet(char *, int, char *, int , struct json_object *);

extern zcfgRet_t zyTTNTPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTNTPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTNTPObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTNTPObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTNTPObjAttrSet(char *, int, char *, int , struct json_object *);

#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
extern zcfgRet_t zyTTEDNSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTEDNSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTEDNSObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTEDNSObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTEDNSObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef MSTC_TAAAG_MULTI_USER
extern zcfgRet_t zyNatDMZObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyNatDMZObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif

#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
extern zcfgRet_t zyExtHwWiFiButtonObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtHwWiFiButtonObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtHwWiFiButtonObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtHwWiFiButtonObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtHwWiFiButtonObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
extern zcfgRet_t devConfObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t devConfObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern int devConfAttrGet(char *, int, char *);
extern zcfgRet_t devConfObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
extern zcfgRet_t zyExtQuantwifiObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtQuantwifiObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtQuantwifiObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtQuantwifiObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtQuantwifiObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
extern zcfgRet_t zyTTUsersObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTUserObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTUserObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTUserObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t zyTTUserObjDel(char *tr98FullPathName);
extern zcfgRet_t zyTTUserObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTUserObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTUserObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTUserInterfaceObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTUserInterfaceObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTUserInterfaceObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTUserInterfaceObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTUserInterfaceObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTRemoteAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTRemoteAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTRemoteAccessObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTRemoteAccessObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTRemoteAccessObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTLocalAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTLocalAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTLocalAccessObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTLocalAccessObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTLocalAccessObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTFirewallObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTFirewallObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTFirewallObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTFirewallObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTFirewallObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTUPnPDeviceObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTUPnPDeviceObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTUPnPDeviceObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTUPnPDeviceObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTUPnPDeviceObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTBroadbandObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTBroadbandObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTBroadbandObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTBroadbandObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTBroadbandObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTInternetDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTInternetDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTInternetDSLObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTInternetDSLObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTInternetDSLObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTInternetWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTInternetWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTInternetWANObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTInternetWANObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTInternetWANObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTIPTVDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTIPTVDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTIPTVDSLObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTIPTVDSLObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTIPTVDSLObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTIPTVWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTIPTVWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTIPTVWANObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTIPTVWANObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTIPTVWANObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTMamgWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTMamgWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTMamgWANObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTMamgWANObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTMamgWANObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTACSDSLObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTACSDSLObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTACSDSLObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTACSDSLObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTACSDSLObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTACSWANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTACSWANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTACSWANObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTACSWANObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTACSWANObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTAdvancedObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTAdvancedObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTAdvancedObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTAdvancedObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTAdvancedObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTGeneralObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTGeneralObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTGeneralObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTGeneralObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTGeneralObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTMoreAPObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTMoreAPObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTMoreAPObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTMoreAPObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTMoreAPObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTWPSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTWPSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTWPSObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTWPSObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTWPSObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTWMMObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTWMMObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTWMMObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTWMMObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTWMMObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTOthersObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTOthersObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTOthersObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTOthersObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTOthersObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTLANSetupObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTLANSetupObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTLANSetupObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTLANSetupObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTLANSetupObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTLANVLANObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTLANVLANObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTLANVLANObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTLANVLANObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTLANVLANObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTRoutingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTRoutingObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTRoutingObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTRoutingObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTRoutingObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTALGObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTALGObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTALGObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTALGObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTALGObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTDoSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTDoSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTDoSObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTDoSObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTDoSObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTLogObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTLogObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTLogObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTLogObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTLogObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTRemoteMGMTObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTRemoteMGMTObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTRemoteMGMTObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTRemoteMGMTObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTRemoteMGMTObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTTR069ObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTTR069ObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTTR069ObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTTR069ObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTTR069ObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTEMRemoteAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTEMRemoteAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTEMLoaclAccessObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTEMLoaclAccessObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTTimeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTTimeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTTimeObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTTimeObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTTimeObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTLogSettingObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTLogSettingObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTLogSettingObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTLogSettingObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTLogSettingObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTFirmwareUpgradeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTFirmwareUpgradeObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTFirmwareUpgradeObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTFirmwareUpgradeObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTFirmwareUpgradeObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTConfigurationObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTConfigurationObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTConfigurationObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTConfigurationObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTConfigurationObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTEDNSPageObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTEDNSPageObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTEDNSPageObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTEDNSPageObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTEDNSPageObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTEngdebugObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTEngdebugObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTEngdebugObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTEngdebugObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTEngdebugObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTCaptureObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTCaptureObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTCaptureObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTCaptureObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTCaptureObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTShellObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTShellObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTShellObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTShellObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTShellObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef CONFIG_ZYXEL_TR140
extern zcfgRet_t SSObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t SSUserAccountObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSUserAccountObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t SSUserAccountObjAdd(char *tr98FullPathName, int *idx);
extern zcfgRet_t SSUserAccountObjDel(char *tr98FullPathName);
extern zcfgRet_t SSCapabilitesObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSNetInfoObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSNetworkServerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSNetworkServerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t SSFTPServerObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSFTPServerObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t SSPhysicalMediumObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSLogicalVolumeObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t SSFolderObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
extern zcfgRet_t zyExtLifemoteObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtLifemoteObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t zyExtLifemoteObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyExtLifemoteObjAttrGet(char *, int, char *);
extern zcfgRet_t zyExtLifemoteObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef CONFIG_TAAAB_PORTMIRROR
zcfgRet_t GetPMHistory(char *value);
extern zcfgRet_t zyTTPORTMIRRORObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTPORTMIRRORObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTPORTMIRRORObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTPORTMIRRORObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTPORTMIRRORObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
zcfgRet_t GetPacketCaptureHistory(char *value);
extern zcfgRet_t zyTTPACKETCAPTUREObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTPACKETCAPTUREObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTPACKETCAPTUREObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTPACKETCAPTUREObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTPACKETCAPTUREObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTPACKETCAPTUREUPLOADObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTPACKETCAPTUREUPLOADObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTPACKETCAPTUREUPLOADObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTPACKETCAPTUREUPLOADObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTPACKETCAPTUREUPLOADObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef ZYXEL_IGMPDIAG 
extern zcfgRet_t zyTTIGMPDiagnosticsObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTIGMPDiagnosticsObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTIGMPDiagnosticsObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTIGMPDiagnosticsObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTIGMPDiagnosticsObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
extern zcfgRet_t zyTTWanConnDiagObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTWanConnDiagObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTWanConnDiagObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTWanConnDiagObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTWanConnDiagObjAttrSet(char *, int, char *, int , struct json_object *);
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
extern zcfgRet_t zyTTSyslogObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTSyslogObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTSyslogObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTSyslogObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTSyslogObjAttrSet(char *, int, char *, int , struct json_object *);
extern zcfgRet_t zyTTSyslogUploadObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyTTSyslogUploadObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyTTSyslogUploadObjNotify(char *, char *, struct json_object *, int, struct json_object **);
extern int zyTTSyslogUploadObjAttrGet(char *, int, char *);
extern zcfgRet_t zyTTSyslogUploadObjAttrSet(char *, int, char *, int , struct json_object *);
#endif

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
extern zcfgRet_t zyExtMultipleConfigObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtMultipleConfigObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif

/* InternetGatewayDevice.X_ZYXEL_IPV4RoutingTable*/
extern zcfgRet_t WANV4RoutingTableObjGet(char *, int, struct json_object **, bool);
/* InternetGatewayDevice.X_ZYXEL_IPV6RoutingTable*/
extern zcfgRet_t WANV6RoutingTableObjGet(char *, int, struct json_object **, bool);

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
extern zcfgRet_t zyExtIpkPackageGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtIpkPackageSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
extern zcfgRet_t zyExtIpkPackageInstallGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t zyExtIpkPackageInstallSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj);
#endif
