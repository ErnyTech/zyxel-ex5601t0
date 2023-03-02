tr98Parameter_t para_MgmtSrv[] = {
	{ "EnableCWMP", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "URL", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Username", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "PeriodicInformEnable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "PeriodicInformInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "PeriodicInformTime", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 21, json_type_time},
	{ "ParameterKey", PARAMETER_ATTR_READONLY    | PARAMETER_ATTR_EXTENDER_HIDDEN, 33, json_type_string},
	{ "ConnectionRequestURL", PARAMETER_ATTR_READONLY|PARAMETER_ATTR_ACTIVENOTIFY, 257, json_type_string},
	{ "ConnectionRequestUsername", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "ConnectionRequestPassword", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "UpgradesManaged", PARAMETER_ATTR_WRITE        | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_boolean},
	{ "KickURL", PARAMETER_ATTR_READONLY             | PARAMETER_ATTR_EXTENDER_HIDDEN, 257, json_type_string},
	{ "DownloadProgressURL", PARAMETER_ATTR_READONLY | PARAMETER_ATTR_EXTENDER_HIDDEN, 257, json_type_string},
	{ "DefaultActiveNotificationThrottle", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "UDPConnectionRequestAddress", PARAMETER_ATTR_READONLY, 257, json_type_string},
	{ "UDPConnectionRequestAddressNotificationLimit", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "STUNEnable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "STUNServerAddress", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "STUNServerPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "STUNUsername", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "STUNPassword", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "STUNMaximumKeepAlivePeriod", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ "STUNMinimumKeepAlivePeriod", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "NATDetected", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "ManageableDeviceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	// Device.ManagementServer.EmbeddedDevice.{i}.
	{ "EmbeddedDeviceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	// Device.ManagementServer.VirtualDevice.{i}.
	{ "VirtualDeviceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	// Device.ManagementServer.InformParameter.{i}.
	{ "InformParameterNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "X_ZYXEL_ConnectionRequestUDPPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_ConnectionRequestPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_BoundInterface",     PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 33, json_type_string},
	{ "X_ZYXEL_BoundInterfaceList", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 129, json_type_string},
	{ "X_ZYXEL_DataModelSpec", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "X_ZYXEL_SerialNumber_Type", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_V4TrustDomain", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 65, json_type_string},
    { "X_ZYXEL_ROMD_Action", PARAMETER_ATTR_WRITE, 9, json_type_string},
	{ "X_ZYXEL_V6TrustDomain", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 257, json_type_string},
	{ "X_ZYXEL_IPv6_Protocol", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IPv4_Protocol", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_AcsSpvNotifyRequired", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_AcsUrlModifyAndClearTransData", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_UtilizeDelayApply", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_SpvOnSessionComplete", PARAMETER_ATTR_WRITE, 0, json_type_boolean},	
#ifdef ZYXEL_TR69_SUPPLEMENTAL_ACS
	{ "X_ZYXEL_Supplemental_EnableCWMP", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_Supplemental_URL", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_Username", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_Password", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_ConnReqURL", PARAMETER_ATTR_READONLY|PARAMETER_ATTR_ACTIVENOTIFY, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_ConnReqUsername", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_ConnReqPassword", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_PASSWORD, 257, json_type_string},
	{ "X_ZYXEL_Supplemental_ConnReqPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
#endif
	//{ "X_ZYXEL_ReplyAllObjectsWhileRootQuery", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	//{ "X_ZYXEL_QueryRootButNotRepliedObjects", PARAMETER_ATTR_WRITE, 513, json_type_string},
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
	{ "X_TTNET_ACS_IP", PARAMETER_ATTR_WRITE, 65, json_type_string},
#endif
	//{ "X_ZYXEL_QueryButNotRepliedObjects", PARAMETER_ATTR_WRITE, 513, json_type_string},
#ifdef ZYXEL_XMPP
    { "SupportedConnReqMethods", PARAMETER_ATTR_READONLY, 32, json_type_string},
    { "ConnReqXMPPConnection", PARAMETER_ATTR_WRITE, 128, json_type_string},
    { "ConnReqAllowedJabberIDs", PARAMETER_ATTR_WRITE, 8223, json_type_string},
    { "ConnReqJabberID", PARAMETER_ATTR_READONLY, 256, json_type_string},
#endif
#ifdef ABWO_CUSTOMIZATION
    { "X_ZYXEL_PreSharedKeyReadable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
#ifdef ABWW_STB_PROVISION_Y2020
	{ "X_SUNRISE_STB_ProvCode", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_SUNRISE_STB_URL", PARAMETER_ATTR_WRITE, 257, json_type_string},
#endif
    { "X_ZYXEL_Feature", PARAMETER_ATTR_WRITE | PARAMETER_ATTR_EXTENDER_HIDDEN, 0, json_type_uint32},
    { "X_ZYXEL_DelayReboot", PARAMETER_ATTR_WRITE, 0, json_type_int},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_MgabDev[] = {
	{ "ManufacturerOUI", PARAMETER_ATTR_READONLY, 7, json_type_string},
	{ "SerialNumber", PARAMETER_ATTR_READONLY, 65, json_type_string},
	{ "ProductClass", PARAMETER_ATTR_READONLY, 65, json_type_string},
	{ "Host", PARAMETER_ATTR_READONLY, 1025, json_type_string},
	{ NULL, 0, 0, 0}
};

