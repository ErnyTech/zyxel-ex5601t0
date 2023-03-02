
tr98Parameter_t para_extend[] = {
#ifdef ZYXEL_ACS_WPS_ACTIVATION
	{ "WPSRunning", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "WPSApIndex", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "WPSRunningStatus", PARAMETER_ATTR_READONLY, 257, json_type_string},
#endif
	{ NULL, 0, 0}
};

tr98Parameter_t para_Dns[] = {
	{ "SupportedRecordTypes", PARAMETER_ATTR_READONLY, 14, json_type_string},
	{ "X_ZYXEL_BoundInterfaceList", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "X_ZYXEL_RedirectLANSideDNSQuery", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_ActiveDNSServers", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ "X_ZYXEL_DNSQueryScenario", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0}
};

tr98Parameter_t para_DnsRtEntry[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "DomainName", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "RouteInterface", PARAMETER_ATTR_READONLY, 33, json_type_string},
	{ "DNSServers", PARAMETER_ATTR_READONLY, 65, json_type_string},
	{ "SubnetMask", PARAMETER_ATTR_WRITE, 18, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_DnsEntry[] = {
	{ "HostName", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "IPv4Address", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_DDns[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "ServiceProvider", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "DDNSType", PARAMETER_ATTR_WRITE, 9, json_type_string},
	{ "HostName", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "UserName", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 65, json_type_string},
	{ "IPAddressPolicy", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "UserIPAddress", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "Wildcard", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Offline", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "AuthenticationResult", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ "UpdatedTime", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "DynamicIP", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "UpdateURL", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "ConnectionType", PARAMETER_ATTR_WRITE, 12, json_type_string},
	{ NULL, 0, 0}
};
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER) || defined(GFIBER_CUSTOMIZATION)
tr98Parameter_t para_RemoMgmt[] = {
#ifdef IAAAA_CUSTOMIZATION
	{ "BoundWANInterfaceList", PARAMETER_ATTR_WRITE, 257, json_type_string},
#endif
#ifdef ZYXEL_SP_DOMAIN_TIMER
	{ "SPRemoteTimer", PARAMETER_ATTR_WRITE, 0, json_type_int},
#endif
#ifdef GFIBER_CUSTOMIZATION
	{ "HttpsUrl", PARAMETER_ATTR_READONLY|PARAMETER_ATTR_ACTIVENOTIFY, 257, json_type_string},
	{ "AssistPw", PARAMETER_ATTR_READONLY|PARAMETER_ATTR_ACTIVENOTIFY, 257, json_type_string},
#endif
	{ NULL, 0, 0}
};
#endif
tr98Parameter_t para_RemoSrv[] = {
	{ "Name", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Protocol", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "Port",                    PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_int},
#if ZYXEL_TR69_REMOTE_MANAGE_SEPARATE_PARAMETER //Louis
	{ "LAN", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "WLAN", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "WAN", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "TrustDomain", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#else
	{ "Mode",                    PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 11, json_type_string},
	{ "OldMode",                 PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 11, json_type_string},
#endif
	{ "RestartDeamon",           PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "BoundInterfaceList",      PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 129, json_type_string},
	{ "TrustAll",                PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "DisableSshPasswordLogin", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ NULL, 0, 0}
};

tr98Parameter_t para_SpRemoSrv[] = {
	{ "Name", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#ifdef ZYXEL_SP_DOMAIN_TIMER
	{ "TimerEnable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
	{ "Port", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ NULL, 0, 0}
};

tr98Parameter_t para_RemoTrustDomain[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IPAddress", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "SubnetMask", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_SpTrustDomain[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IPAddress", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "SubnetMask", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0}
};

#ifdef ABUU_CUSTOMIZATION
tr98Parameter_t para_IfaceTrustDomain[] = {
	{ "AccountLevel", PARAMETER_ATTR_WRITE, 49, json_type_string},
	{ "BoundInterfaceName", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_IfaceTrustDomainInfo[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IPAddress", PARAMETER_ATTR_WRITE, 49, json_type_string},
	{ "SubnetMask", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ NULL, 0, 0}
};
#endif

tr98Parameter_t para_Igmp[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IgmpVersion", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "QueryInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "QueryResponseInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "LastMemberQueryInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "RobustnessValue", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxGroups", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxSources", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxMembers", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "LanToLanEnable", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "FastLeave", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "JoinImmediate", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "SnoopingEnable", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Interface", PARAMETER_ATTR_READONLY, TR98_MAX_OBJ_NAME_LENGTH*5, json_type_string},
	{ "ifName", PARAMETER_ATTR_READONLY, 321, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 19, json_type_string},
	{ "SnoopingBridgeIfName", PARAMETER_ATTR_READONLY, 101, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_Mld[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "MldVersion", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "QueryInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "QueryResponseInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "LastMemberQueryInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "RobustnessValue", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "LanToLanEnable", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxGroups", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxSources", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxMembers", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "FastLeave", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "SnoopingEnable", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Interface", PARAMETER_ATTR_READONLY, TR98_MAX_OBJ_NAME_LENGTH*5, json_type_string},
	{ "ifName", PARAMETER_ATTR_READONLY, 321, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 19, json_type_string},
	{ "SnoopingBridgeIfName", PARAMETER_ATTR_READONLY, 101, json_type_string},
	{ NULL, 0, 0}
};

#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
tr98Parameter_t para_LogCfg[] = {
	{ "AutoDisableHighUser", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0}
};
#endif
tr98Parameter_t para_LogCfgGp[] = {
	{ "Level", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ "Page", PARAMETER_ATTR_WRITE, 2049, json_type_string},
#ifdef ABUU_CUSTOMIZATION
	{ "AccessEnable", PARAMETER_ATTR_WRITE, 19, json_type_string},
#endif
	{ "GP_Privilege", PARAMETER_ATTR_WRITE, 2049, json_type_string},
	{ NULL, 0, 0}
};
tr98Parameter_t para_LogCfgGpAccount[] = {
	{ "Enabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "EnableQuickStart", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "Username", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "AutoShowQuickStart", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "Page", PARAMETER_ATTR_WRITE, 2049, json_type_string},
	{ "DefaultPassword", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "ResetDefaultPassword", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "PasswordHash", PARAMETER_ATTR_READONLY | PARAMETER_ATTR_PASSWORD, 17, json_type_string},
	{ "AccountCreateTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "AccountRetryTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "AccountIdleTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "AccountLockTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "RemoHostAddress", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 49, json_type_string},
	{ "DotChangeDefPwd", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "AutoGenPwdBySn", PARAMETER_ATTR_WRITE  | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "GetConfigByFtp", PARAMETER_ATTR_WRITE  | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "Privilege", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "CardHide", PARAMETER_ATTR_WRITE, 513, json_type_string},
	{ "SshKeyBaseAuthPublicKey", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD | PARAMETER_ATTR_EXTENDER_HIDDEN, 4097, json_type_string},
	{ NULL, 0, 0}
};
#endif

tr98Parameter_t para_LogSetting[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LogServer", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "UDPPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ZCFG_LOG_Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LogMode", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "Interval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "RemoteLogFormat", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_LogClassify[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 30, json_type_string},
	{ "Describe", PARAMETER_ATTR_READONLY, 51, json_type_string},
	{ "File", PARAMETER_ATTR_READONLY, 51, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_LogCategory[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Name", PARAMETER_ATTR_READONLY, 30, json_type_string},
	{ "Describe", PARAMETER_ATTR_READONLY, 50, json_type_string},
	{ "Level", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Filter", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "LocalLogFormat", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ NULL, 0, 0}
};

#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
tr98Parameter_t para_WlanScheduler[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0}
};

tr98Parameter_t para_WlanSchedulerRule[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Name", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "Description", PARAMETER_ATTR_WRITE, 256, json_type_string},
	//{ "SSID", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Days", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	//{ "TimeStartDate", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "TimeStartHour", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeStartMin", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	//{ "TimeStopDate", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "TimeStopHour", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeStopMin", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0}
};

#endif
tr98Parameter_t para_MacFilter[] = {
	{ "MaxNumberOfEntries", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Restrict", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_MacFilter_WhiteList[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "MACAddress", PARAMETER_ATTR_WRITE, 18, json_type_string},
	{ "HostName", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ParenCtl[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0}
};

tr98Parameter_t para_ParenCtlProf[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Name", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "WebRedirect", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "ServicePolicy", PARAMETER_ATTR_WRITE, 11, json_type_string},
	{ "Type", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MACAddressList", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "ScheduleRuleList", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "URLFilter", PARAMETER_ATTR_WRITE, 580, json_type_string},
	{ "NetworkServiceList", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ NULL, 0, 0}
};

#ifdef ZyXEL_IPP_PRINTSERVER
tr98Parameter_t para_Ipp_PrintServer[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IppName", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "IppMake", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ NULL, 0, 0}
};
#endif

#ifdef ONECONNECT
tr98Parameter_t para_OneConnect[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SpeedTestInfo[] = {
	{ "Start", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "UploadSpeedResult", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "DownloadSpeedResult", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_AP_STEERING
tr98Parameter_t para_ApSteering[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "WiFi_24G_NormalToBad_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "WiFi_24G_NormalToGood_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "WiFi_24G_Upgrade_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "WiFi_5G_NormalToBad_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "WiFi_5G_NormalToGood_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "RE_Threshold_Adjustment", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "Idle_Pkt_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "STA_Num_Difference_Threshold", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "SteeringTime", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "ProhibitAfterSteering", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "ClearSteeringStatus", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "ScanOtherAPsInterval", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "LoadBalancingInterval", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "LogLevel", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "Prohibit_Steering_OverWDS_AP", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "DisableSteering", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //#ifdef ZYXEL_AP_STEERING

#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
tr98Parameter_t para_ProximitySensor[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR

#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
tr98Parameter_t para_BandwidthImprovement[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "BWMInterval", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "TriggerLording", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "DeTriggerLording", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ NULL, 0, 0, 0}
};
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
tr98Parameter_t para_WLANConfiguration[] = {
	{ "X_ZYXEL_PSK24", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "X_ZYXEL_PSK5", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_AccountResetPassword[] = {
	{ "Username", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "X_ZYXEL_ResetDefaultPassword", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_AirtimeFairness[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

tr98Parameter_t para_SysInfo[] = {
	{ "HostName", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "IsHostNameModify", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "DomainName", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 65, json_type_string},
	{ "Contact", PARAMETER_ATTR_WRITE, 101, json_type_string},
	{ "Location", PARAMETER_ATTR_WRITE, 51, json_type_string},
	{ "OnlineGuide", PARAMETER_ATTR_WRITE, 129, json_type_string},
#ifdef CAAAB_CUSTOMIZATION
	{ "CBT_AccountReset", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
	{ NULL, 0, 0, 0}
};

#ifdef ZYXEL_SFP_MODULE_SUPPORT
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
tr98Parameter_t para_GponInfo[] = {
	{ "Enable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "RxPowerSignal", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "TxPowerSignal", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "Temperature", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "VendorName", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "Model", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "VendorSerial", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ NULL, 0, 0, 0}
};
#else
tr98Parameter_t para_GponInfo[] = {
	{ "RxPowerSignal", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "TxPowerSignal", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "Temperature", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "SFP_Vendor", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "SFP_Model", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "SFP_Serial", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "SFP_Presence", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif

tr98Parameter_t para_Snmp[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Port", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ROCommunity", PARAMETER_ATTR_WRITE, 51, json_type_string},
	{ "RWCommunity", PARAMETER_ATTR_WRITE, 51, json_type_string},
	{ "TrapCommunity", PARAMETER_ATTR_WRITE, 51, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SnmpTrap[] = {
	{ "Host", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Port", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TarpVer", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_Tr064[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_EmailSrv[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "SMTPServerAddress", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "SMTPServerPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "AuthUser", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "AuthPass", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 65, json_type_string},
	{ "EmailAddress", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "SMTPSecurity", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "RequiresTLS", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_EmailEvent[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Event", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "EmailTo", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "EmailSubject", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "EmailBody", PARAMETER_ATTR_WRITE, 1025, json_type_string},
	{ "SericeRef", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_VlanGroup[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "GroupName", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "VlanId", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "BrRefKey", PARAMETER_ATTR_WRITE, 9, json_type_string},
	{ "IntfList", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "TagList", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_8021ag[] = {
	{ "MDLevel", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "IsPtmLink", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "VLANID", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "DestMacAddress", PARAMETER_ATTR_WRITE, 18, json_type_string},
	{ "SendLoopBack", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LoopBackResult", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "SendLinkTrace", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LinkTraceResult", PARAMETER_ATTR_WRITE, 321, json_type_string},
#ifdef ZYXEL_ETHOAM_TMSCTL
	{ "Enabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Y1731Mode", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Interface", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "MDName", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "MAID", PARAMETER_ATTR_WRITE, 129, json_type_string},
	{ "LMPID", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "RMPID", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "CCMEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "CCMInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "LinkTraceTTL", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "TmsPid", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
#endif
	{ NULL, 0, 0, 0}
};

#ifdef ZYXEL_ETHOAM_TMSCTL
tr98Parameter_t para_8023ah[] = {
	{ "Enabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Interface", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "VariableRetrievalEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LinkEventsEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "RemoteLoopbackEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "ActiveModeEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "AutoEventEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "OAMID", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TmsPid", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
#endif

tr98Parameter_t para_Sec[] = {
	{ "CertificateNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SecCert[] = {
	{ "X_ZYXEL_Name", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "X_ZYXEL_Type", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "X_ZYXEL_Certificate", PARAMETER_ATTR_WRITE, 4097, json_type_string},
	{ "X_ZYXEL_PrivateKey", PARAMETER_ATTR_WRITE, 4097, json_type_string},
	{ "X_ZYXEL_SigningRequest", PARAMETER_ATTR_WRITE, 4097, json_type_string},
	{ "X_ZYXEL_CommonName", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "X_ZYXEL_OrgName", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "X_ZYXEL_StateName", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ "X_ZYXEL_CountryName", PARAMETER_ATTR_WRITE, 3, json_type_string},
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LastModif", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "SerialNumber", PARAMETER_ATTR_WRITE, 65, json_type_string},
	{ "Issuer", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "NotBefore", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "NotAfter", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "Subject", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "SubjectAlt", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "SignatureAlgorithm", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_Samba[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "PidFileName", PARAMETER_ATTR_READONLY, 257, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_Upnp[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_NATTEnable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

#ifdef ZYXEL_LANDING_PAGE_FEATURE
tr98Parameter_t para_Landing_Page[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "URL", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
tr98Parameter_t para_Online_Firmware_Upgrade[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "SystemModelName", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "FWInfoURL", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ "FWFileName", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "FWVersion", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "FWReleaseDate", PARAMETER_ATTR_WRITE, 20, json_type_string}, //need check
	{ "FWSize", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ "HttpSevrPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "FWURL", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

tr98Parameter_t para_Feature_Flag[] = {
	{ "Customer", PARAMETER_ATTR_WRITE           | PARAMETER_ATTR_EXTENDER_HIDDEN, 33, json_type_string},
	{ "checkDuplicateVlan", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "WANPriority", PARAMETER_ATTR_WRITE        | PARAMETER_ATTR_EXTENDER_HIDDEN, 16, json_type_string},
	{ "SfpDelayTimes", PARAMETER_ATTR_WRITE      | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_uint32},
	{ "SipDelayTimes", PARAMETER_ATTR_WRITE      | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_uint32},
	{ "CertIgnoreTime", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#ifdef ZY_DISABLE_WIFI_HW_BTN
    { "DisableWifiHWButton", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
#endif
	// altibox request
	{ "DownloadScriptTR69", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "FactoryResetOnUpgrade", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
    { "LEDBlinking", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_GUI_Customization[] = {
	{ "Flag1", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_int},
	{ "Flag2", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_int},
	{ "Flag3", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_int},
	{ "BlockSpecialChar", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "BlockChar", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "AllowFilter", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "AllowCharList", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "EthWanAsLan", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "HideChangeDefault", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "HideSkipPassword", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
tr98Parameter_t para_EasyMesh[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
tr98Parameter_t para_VendorEasyMesh[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif //CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH

tr98Parameter_t para_WiFiCommunity[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_DSL[] =
{
	{ "X_ZYXEL_ModemLabID", PARAMETER_ATTR_WRITE, 13, json_type_string},
	{ NULL, 0, 0, 0}
};

#ifdef ZYXEL_ONESSID
tr98Parameter_t para_OneSsid[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif //#ifdef ZYXEL_ONESSID

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
tr98Parameter_t para_SfpMgmt[] = {
#ifdef ABPY_CUSTOMIZATION
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Timer", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT

#ifdef ZYXEL_IPV6_MAP
tr98Parameter_t para_MAP[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_MAPDomain[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	//{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "TransportMode", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "WANInterface", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "IPv6Prefix", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "BRIPv6Prefix", PARAMETER_ATTR_WRITE, 49, json_type_string},
	//{ "DSCPMarkPolicy", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "PSIDOffset", PARAMETER_ATTR_WRITE, 15, json_type_uint32},
	{ "PSIDLength", PARAMETER_ATTR_WRITE, 16, json_type_uint32},
	{ "PSID", PARAMETER_ATTR_WRITE, 65535, json_type_uint32},
	//{ "IncludeSystemPorts", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_Softwire46Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_MAPDomainRule[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	//{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Origin", PARAMETER_ATTR_READONLY, 13, json_type_string},
	{ "IPv6Prefix", PARAMETER_ATTR_WRITE, 49, json_type_string},
	{ "IPv4Prefix", PARAMETER_ATTR_WRITE, 18, json_type_string},
	//{ "EABitsLength", PARAMETER_ATTR_WRITE, 48, json_type_uint32},
	{ "IsFMR", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "PSIDOffset", PARAMETER_ATTR_WRITE, 15, json_type_uint32},
	{ "PSIDLength", PARAMETER_ATTR_WRITE, 16, json_type_uint32},
	{ "PSID", PARAMETER_ATTR_WRITE, 65535, json_type_uint32},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_TR69_DATA_USAGE
tr98Parameter_t para_DataUsage[] = {
	{ "DataUsageUpdate", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_DataUsageLan[] = {
	{ "Timestamp", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Upload_Mbps", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "Download_Mbps", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_DataUsageWan[] = {
	{ "Timestamp", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Upload_Mbps", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "Download_Mbps", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_TAAAG_TR69ALG//MSTC_TTALG, RichardFeng
tr98Parameter_t para_H323Alg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_SIPAlg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_RTSPAlg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_PPTPAlg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_IPSecAlg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_L2TPAlg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_IGMPProxy[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_SMTP[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_ICMP[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_NTP[] = {
	{ "Interface", PARAMETER_ATTR_WRITE, 512, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
tr98Parameter_t para_EDNS[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "State", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
tr98Parameter_t para_Users[] = {
	{ "UserNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
 
#ifdef MSTC_TAAAG_MULTI_USER
tr98Parameter_t para_NatDMZOpt[] = {
#ifdef ZYXEL_SUPPORT_DMZ_MANAGEMENT
	{ "X_ZYXEL_DMZHost_Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
	{ "X_ZYXEL_DMZHost", PARAMETER_ATTR_WRITE, 33, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif
 
tr98Parameter_t para_User[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Level", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "Username", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 64, json_type_string},
	{ "RemoteAccessCapable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LocalAccessCapable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Allowed_RA_Protocols", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Allowed_LA_Protocols", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_UserInterface[] = {
	{ "PasswordRequired", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "PasswordUserSelectable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "PasswordReset", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_RemoteLocalAccess[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Port", PARAMETER_ATTR_WRITE, 64, json_type_string},	
	{ "SupportedProtocols", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Protocol", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_TTFirewall[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_UPnPDevice[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_GUIAccessRights[] = {
	{ "VisibilityLevel", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Limitation", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Lock", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_Shell[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 64, json_type_string},	
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
tr98Parameter_t para_HwWiFiButton[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
tr98Parameter_t para_Lifemote[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "URL", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "State", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_SUPPORT_QUANTWIFI
tr98Parameter_t para_Quantwifi[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "URL", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "State", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ NULL, 0, 0, 0}
};
#endif

#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
tr98Parameter_t para_DevConf[] = {
	{ "ConfigFile", PARAMETER_ATTR_WRITE, 64*1024, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef CONFIG_TAAAB_PORTMIRROR
tr98Parameter_t para_TTPORTMIRROR[] = {
    { "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Key", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "MonitorInterface", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Direction", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "MirrorInterface", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "History", PARAMETER_ATTR_READONLY, 512, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif
#ifdef CONFIG_TAAAB_PACKETCAPTURE
tr98Parameter_t para_TTPACKETCAPTURE[] = {
    { "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Key", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "MonitorInterface", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Direction", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "FileName", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Storage", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "Filter", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "History", PARAMETER_ATTR_READONLY, 512, json_type_string},
	{ "Clear", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_TTPACKETCAPTUREUPLOAD[] = {
	{ "State", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "FileName", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "FileSource", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "URL", PARAMETER_ATTR_WRITE, 128, json_type_string},
	{ "Protocol", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "Username", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif
#ifdef CONFIG_ZYXEL_TR140
tr98Parameter_t para_SS[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "PhysicalMediumNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "LogicalVolumeNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UserAccountNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSUserAccount[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Username", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSCapabilites[] = {
	{ "FTPCapable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "SFTPCapable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "HTTPCapable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "HTTPSCapable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "HTTPWritable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "VolumeEncryptionCapable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "SupportedNetworkProtocols", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "SupportedFileSystemTypes", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSNetInfo[] = {
	{ "HostName", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "DomainName", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSNetworkServer[] = {
	{ "AFPEnable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "NFSEnable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "SMBEnable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "NetworkProtocolAuthReq", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "SambaHostName", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSFTPServer[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "MaxNumUsers", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IdleTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "PortNumber", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSPhysicalMedium[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "ConnectionType", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Removable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "Capacity", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSLogicalVolume[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "PhysicalReference", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ "Enable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "FileSystem", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Capacity", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UsedSpace", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "Encrypted", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "FolderNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SSFolder[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Enable", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "UserAccountAccess", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
#endif
#ifdef ZYXEL_IGMPDIAG 
tr98Parameter_t para_IGMPDiagnostics[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "ChannelAddress", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "ChannelStatus", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ "StreamServerAddress", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ "NumberOfIGMPJoins", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "NumberOfIGMPLeaves", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IGMPJoinDelay", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG 
tr98Parameter_t para_TTWanConnDiagnostics[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_LocalDomain", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_RemoteDomain", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_PingLocal", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_PingRemote", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_TraceRoute", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_DNS1", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_DNS2", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_DNSQuery", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_NTP1", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_NTP2", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_NTPServer", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "IPTV_TestChannel", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Internet_ServisState", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "Internet_ConnectionStatus", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Internet_VlanID", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "Internet_Username", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_IPAddress", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_DNS1", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_DNS2", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_LanDNS1", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_LanDNS2", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_PingGateway", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Internet_Check_LocalDns", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Internet_Check_RemoteDns", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Internet_Check_PingLocal", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Internet_Check_PingRemote", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Internet_Check_TraceRoute", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Internet_WebPage", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "QuantWifi_WanSpeed", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "QuantWifi_LanSpeed", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "IPTV_ServisState", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPTV_ConnectionStatus", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "IPTV_VlanID", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "IPTV_IPAddress", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "IPTV_PingDHCP", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "IPTV_CheckDNS", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "IPTV_CheckNTP", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "IPTV_CheckLiveStream", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Management_ServisState", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "Management_ConnectionStatus", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Management_VlanID", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "Management_IPAddress", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Management_DNS1", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Management_DNS2", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Management_PingDHCP", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Management_PingDNS", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Management_Server", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
tr98Parameter_t para_Syslog[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Target", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Server", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Port", PARAMETER_ATTR_WRITE, 0, json_type_uint32},	
	{ "Level", PARAMETER_ATTR_WRITE, 0, json_type_uint32},	
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_SyslogUpload[] = {
	{ "State", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "URL", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Protocol", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Username", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
tr98Parameter_t para_MultipleConfig[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 8, json_type_string},
	{ "SetConfig", PARAMETER_ATTR_WRITE, 8, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif //#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
tr98Parameter_t para_MultiWan[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_MultiWanIntf[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "LinkType", PARAMETER_ATTR_WRITE, 8, json_type_string},
	{ "Name", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "LowerLayers", PARAMETER_ATTR_WRITE, 128, json_type_string},
//	{ "Mode", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "Priority", PARAMETER_ATTR_WRITE, 0, json_type_int},
//	{ "Failback", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "MultiIface", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ NULL, 0, 0, 0}
};
#endif


tr98Parameter_t para_IPV4RoutingTable[] = {
	{ "Destination", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ "Gateway", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Mask", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Flags", PARAMETER_ATTR_READONLY, 4, json_type_string},
	{ "Metric", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "Interface", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_IPV6RoutingTable[] = {
	{ "Destination", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ "Gateway", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Flags", PARAMETER_ATTR_READONLY, 4, json_type_string},
	{ "Metric", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "Interface", PARAMETER_ATTR_READONLY, 64, json_type_string},	
	{ NULL, 0, 0, 0}
};
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
tr98Parameter_t para_Cell[] = {
	{ "InterfaceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "AccessPointNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_CellIntf[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "LastChange", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "LowerLayers", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ "Upstream", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IMEI", PARAMETER_ATTR_READONLY, 15, json_type_string},
	{ "SupportedAccessTechnologies", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "PreferredAccessTechnology", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "CurrentAccessTechnology", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "AvailableNetworks", PARAMETER_ATTR_READONLY, 1024, json_type_string},
	{ "NetworkRequested", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "NetworkInUse", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "RSSI", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "UpstreamMaxBitRate", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "DownstreamMaxBitRate", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_SupportedBands", PARAMETER_ATTR_READONLY, 1024, json_type_string},
	{ "X_ZYXEL_PreferredBands", PARAMETER_ATTR_WRITE, 1024, json_type_string},
	{ "X_ZYXEL_CurrentBand", PARAMETER_ATTR_READONLY, 1024, json_type_string},
	{ "X_ZYXEL_CellID", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_RFCN", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_RSRP", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_RSRQ", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_CPICHRSCP", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_CPICHEcNo", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_PhyCellID", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_UplinkBandwidth", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_DownlinkBandwidth", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_TAC", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_PrimaryScramblingCode", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_LAC", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_RAC", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_BSIC", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_SINR", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_CQI", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_MCS", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_RI", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_PMI", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_HW_NAT_Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_ModuleSoftwareVersion", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_CellIntfUSIM[] = {
	{ "Status", PARAMETER_ATTR_READONLY, 16, json_type_string},
	{ "IMSI", PARAMETER_ATTR_READONLY, 15, json_type_string},
	{ "ICCID", PARAMETER_ATTR_READONLY, 20, json_type_string},
	{ "MSISDN", PARAMETER_ATTR_READONLY, 15, json_type_string},
	{ "PINCheck", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "PIN", PARAMETER_ATTR_WRITE, 4, json_type_string},
	{ "X_ZYXEL_PUK", PARAMETER_ATTR_WRITE, 8, json_type_string},
	{ "X_ZYXEL_NewPIN", PARAMETER_ATTR_WRITE, 4, json_type_string},
	{ "X_ZYXEL_AutoUnlockPIN", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_PIN_Protection", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_PIN_STATE", PARAMETER_ATTR_WRITE, 32, json_type_string},
	{ "X_ZYXEL_PIN_RemainingAttempts", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ "X_ZYXEL_PUK_RemainingAttempts", PARAMETER_ATTR_READONLY, 0, json_type_int},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_CellAccessPoint[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "APN", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Username", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "X_ZYXELX_ZYXEL_Auth", PARAMETER_ATTR_WRITE, 8, json_type_string},
	{ "X_ZYXEL_ManualAPN", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_AuthenticationType", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "X_ZYXEL_PDP_Type", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ NULL, 0, 0, 0}
};
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
tr98Parameter_t para_Ipk_Package[] = {
	{ "Name", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Version", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Script", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Data", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "FixCustID", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Hash", PARAMETER_ATTR_WRITE, 256, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ NULL, 0, 0, 0}
};
tr98Parameter_t para_Ipk_Package_Install[] = {
   {"Install", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
   {"ForceInstall", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
   { NULL, 0, 0, 0}
};
#endif
