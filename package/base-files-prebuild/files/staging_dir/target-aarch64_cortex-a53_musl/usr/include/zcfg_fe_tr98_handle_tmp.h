/*All TR98 Full Path Name*/
#define TR98_IGD								"InternetGatewayDevice"
#define TR98_SERVICE                              "InternetGatewayDevice.Services"
#define TR98_VOICE_SRV                            "InternetGatewayDevice.Services.VoiceService.i"
#define TR98_VOICE_CAPB                           "InternetGatewayDevice.Services.VoiceService.i.Capabilities"
#define TR98_VOICE_CAPB_SIP                       "InternetGatewayDevice.Services.VoiceService.i.Capabilities.SIP"
#define TR98_VOICE_CAPB_MGCP                      "InternetGatewayDevice.Services.VoiceService.i.Capabilities.MGCP"
#define TR98_VOICE_CAPB_H323                      "InternetGatewayDevice.Services.VoiceService.i.Capabilities.H323"
#define TR98_VOICE_CAPB_CODEC                     "InternetGatewayDevice.Services.VoiceService.i.Capabilities.Codec.i"
#define TR98_VOICE_PROF                           "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i"
#define TR98_VOICE_PROF_SRV_PRO_INFO              "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.ServiceProviderInfo"
#define TR98_VOICE_PROF_SIP                       "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.SIP"
#define TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ         "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.SIP.EventSubscribe.i"
#define TR98_VOICE_PROF_SIP_RESP_MAP_OBJ          "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.SIP.ResponseMap.i"
#define TR98_VOICE_PROF_RTP                       "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.RTP"
#define TR98_VOICE_PROF_RTP_RTCP                  "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.RTP.RTCP"
#define TR98_VOICE_PROF_RTP_SRTP                  "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.RTP.SRTP"
#define TR98_VOICE_PROF_RTP_REDUNDANCY            "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.RTP.Redundancy"
#define TR98_VOICE_PROF_NUM_PLAN                  "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.NumberingPlan"
#define TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO      "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.NumberingPlan.PrefixInfo.i"
#define TR98_VOICE_PROF_TONE                      "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Tone"
#define TR98_VOICE_PROF_TONE_EVENT                "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Tone.Event.i"
#define TR98_VOICE_PROF_TONE_DESCRIPTION          "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Tone.Description.i"
#define TR98_VOICE_PROF_TONE_PATTERN              "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Tone.Pattern.i"
#define TR98_VOICE_PROF_FAX_T38                   "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.FaxT38"
#define TR98_VOICE_LINE                           "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i"
#define TR98_VOICE_LINE_SIP                       "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.SIP"
#define TR98_VOICE_LINE_SIP_EVENT_SUBS            "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.SIP.EventSubscribe.i"
#define TR98_VOICE_LINE_RINGER                    "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Ringer"
#define TR98_VOICE_LINE_RINGER_EVENT              "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Ringer.Event.i"
#define TR98_VOICE_LINE_RINGER_DESCRIPTION        "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Ringer.Description.i"
#define TR98_VOICE_LINE_RINGER_PATTERN            "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Ringer.Pattern.i"
#define TR98_VOICE_LINE_CALLING_FEATURE           "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.CallingFeatures"
#define TR98_VOICE_LINE_PROCESSING                "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.VoiceProcessing"
#define TR98_VOICE_LINE_CODEC                     "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Codec"
#define TR98_VOICE_LINE_CODEC_LIST                "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Codec.List.i"
#define TR98_VOICE_LINE_SESSION                   "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Session.i"
#define TR98_VOICE_LINE_STATS                     "InternetGatewayDevice.Services.VoiceService.i.VoiceProfile.i.Line.i.Stats"
#define TR98_VOICE_PHY_INTF                       "InternetGatewayDevice.Services.VoiceService.i.PhyInterface.i"
#define TR98_VOICE_FXS_CID                        "InternetGatewayDevice.Services.VoiceService.i.PhyInterface.i.X_ZYXEL_CID"
#define TR98_VOICE_PSTN                           "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_PSTN.i"
#ifdef USE_OLD_VOIP_PREFIX
#define TR98_VOICE_COMMON                         "InternetGatewayDevice.Services.VoiceService.i.X_ZyXEL_COM_Common"
#else
#define TR98_VOICE_COMMON                         "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_Common"
#endif
#define TR98_VOICE_PHONE_BOOK                     "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_VoicePhoneBook.i"
#define TR98_VOICE_CALL_POLICY_BOOK               "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_VoicePhoneBook.CallPolicyBook"
#define TR98_VOICE_FXO                            "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_FXO"
#define TR98_VOICE_FXO_PORT                       "InternetGatewayDevice.Services.VoiceService.i.X_ZYXEL_FXO.Port.i"
#ifdef AVAST_AGENT_PACKAGE
#define TR98_ISP_SERVICE                          "InternetGatewayDevice.Services.IspServices.i"
#endif
#define TR98_SVR_SELECT_DIAG          "InternetGatewayDevice.ServerSelectionDiagnostics"
#define TR98_CAPB                     "InternetGatewayDevice.Capabilities"
#define TR98_PERF_DIAG                "InternetGatewayDevice.Capabilities.PerformanceDiagnostic"
#define TR98_DEV_INFO                 "InternetGatewayDevice.DeviceInfo"
#define TR98_MemoryStatus             "InternetGatewayDevice.DeviceInfo.MemoryStatus"
#define TR98_VEND_CONF_FILE           "InternetGatewayDevice.DeviceInfo.VendorConfigFile.i"
#define TR98_PROC_ST                  "InternetGatewayDevice.DeviceInfo.ProcessStatus"
#define TR98_PROC_ST_PS               "InternetGatewayDevice.DeviceInfo.ProcessStatus.Process.i"
#define TR98_DEV_CONF                 "InternetGatewayDevice.DeviceConfig"
#define TR98_MGMT_SRV                 "InternetGatewayDevice.ManagementServer"
#define TR98_MGAB_DEV                 "InternetGatewayDevice.ManagementServer.ManageableDevice.i"
#define TR98_TIME                     "InternetGatewayDevice.Time"
#define TR98_USR_INTF                 "InternetGatewayDevice.UserInterface"
#define TR98_CAPT_PORTAL              "InternetGatewayDevice.CaptivePortal"
#define TR98_L3_FWD                   "InternetGatewayDevice.Layer3Forwarding"
#define TR98_FWD                      "InternetGatewayDevice.Layer3Forwarding.Forwarding.i"
#ifdef L3IPV6FWD
#define TR98_IPV6_FWD                 "InternetGatewayDevice.Layer3Forwarding.X_ZYXEL_IPv6Forwarding.i"
#endif
#define TR98_L2_BR                    "InternetGatewayDevice.Layer2Bridging"
#define TR98_BR                       "InternetGatewayDevice.Layer2Bridging.Bridge.i"
#define TR98_PORT                     "InternetGatewayDevice.Layer2Bridging.Bridge.i.Port.i"
#define TR98_VLAN                     "InternetGatewayDevice.Layer2Bridging.Bridge.i.VLAN.i"
#define TR98_FILTER                   "InternetGatewayDevice.Layer2Bridging.Filter.i"
#define TR98_MARK                     "InternetGatewayDevice.Layer2Bridging.Marking.i"
#define TR98_AVAI_INTF                "InternetGatewayDevice.Layer2Bridging.AvailableInterface.i"
#define TR98_QUE_MGMT                 "InternetGatewayDevice.QueueManagement"
#define TR98_CLS                      "InternetGatewayDevice.QueueManagement.Classification.i"
#define TR98_APP                      "InternetGatewayDevice.QueueManagement.App.i"
#define TR98_FLOW                     "InternetGatewayDevice.QueueManagement.Flow.i"
#define TR98_POLICER                  "InternetGatewayDevice.QueueManagement.Policer.i"
#define TR98_QUE                      "InternetGatewayDevice.QueueManagement.Queue.i"
#define TR98_SHAPER					  "InternetGatewayDevice.QueueManagement.X_ZYXEL_Shaper.i"
#define TR98_QUE_STAT                 "InternetGatewayDevice.QueueManagement.QueueStats.i"
#define TR98_LAN_CONF_SEC             "InternetGatewayDevice.LANConfigSecurity"
#define TR98_IP_PING_DIAG             "InternetGatewayDevice.IPPingDiagnostics"
#define TR98_TRA_RT_DIAG              "InternetGatewayDevice.TraceRouteDiagnostics"
#define TR98_RT_HOP                   "InternetGatewayDevice.TraceRouteDiagnostics.RouteHops.i"
#define TR98_NSLOOKUP_DIAG            "InternetGatewayDevice.NsLookUpDiagnostics"
#define TR98_NSLOOKUP_RESULT          "InternetGatewayDevice.NsLookUpDiagnostics.Result.i"
#define TR98_DL_DIAG                  "InternetGatewayDevice.DownloadDiagnostics"
#define TR98_DL_DIAG_PER_CONN_RST     "InternetGatewayDevice.DownloadDiagnostics.PerConnectionResult.i"
#define TR98_DL_DIAG_INCREM_RST       "InternetGatewayDevice.DownloadDiagnostics.IncrementalResult.i"
#define TR98_UL_DIAG                  "InternetGatewayDevice.UploadDiagnostics"
#define TR98_UL_DIAG_PER_CONN_RST     "InternetGatewayDevice.UploadDiagnostics.PerConnectionResult.i"
#define TR98_UL_DIAG_INCREM_RST       "InternetGatewayDevice.UploadDiagnostics.IncrementalResult.i"
#define TR98_UDP_ECHO_CONF            "InternetGatewayDevice.UDPEchoConfig"
#define TR98_UDP_ECHO_DIAG            "InternetGatewayDevice.UDPEchoDiagnostics"
#define TR98_UDP_ECHO_DIAG_PAK_RST    "InternetGatewayDevice.UDPEchoDiagnostics.IndividualPacketResult.i"
#define TR98_LAN_DEV                  "InternetGatewayDevice.LANDevice.i"
#define TR98_LAN_HOST_CONF_MGMT       "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement"
#define TR98_IP_INTF                  "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i"
#ifdef LANIPALIAS
#define TR98_IP_ALIAS         "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPAlias.i"
#endif
#ifdef IPV6INTERFACE_PROFILE
#define TR98_IP_INTF_IPV6ADDR         "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Address.i"
#define TR98_IP_INTF_IPV6PREFIX       "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_IPv6Prefix.i"
#endif
#ifdef ROUTERADVERTISEMENT_PROFILE
#define TR98_IP_INTF_ROUTERADVER      "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_RouterAdvertisement"
#endif
#ifdef IPV6SERVER_PROFILE
#define TR98_IP_INTF_DHCPV6SRV        "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.IPInterface.i.X_ZYXEL_DHCPv6Server"
#endif
#define TR98_DHCP_STATIC_ADDR         "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPStaticAddress.i"
#define TR98_DHCP_OPT                 "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPOption.i"
#ifdef ZyXEL_DHCP_OPTION125
#define TR98_VENDOR_SPECIFIC          "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.X_ZYXEL_VendorSpecific"
#define TR98_VENDOR_SPECIFIC_PROF     "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.X_ZYXEL_VendorSpecific.Profile.i"
#endif
#define TR98_DHCP_COND_SERVPOOL       "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i"
#define TR98_SERVPOOL_DHCP_STATICADDR "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i.DHCPStaticAddress.i"
#define TR98_SERVPOOL_DHCP_OPT        "InternetGatewayDevice.LANDevice.i.LANHostConfigManagement.DHCPConditionalServingPool.i.DHCPOption.i"
#define TR98_LAN_ETH_INTF_CONF        "InternetGatewayDevice.LANDevice.i.LANEthernetInterfaceConfig.i"
#define TR98_LAN_ETH_INTF_CONF_STAT   "InternetGatewayDevice.LANDevice.i.LANEthernetInterfaceConfig.i.Stats"
#define TR98_LAN_USB_INTF_CONF        "InternetGatewayDevice.LANDevice.i.LANUSBInterfaceConfig.i"
#define TR98_LAN_USB_INTF_CONF_STAT   "InternetGatewayDevice.LANDevice.i.LANUSBInterfaceConfig.i.Stats"
#define TR98_LAN_DEV_WLAN_CFG         "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i"
#define TR98_WLAN_CFG_STAT            "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.Stats"
#define TR98_WPS                      "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.WPS"
#define TR98_REG                      "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.WPS.Registrar.i"
#define TR98_ASSOC_DEV                "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.AssociatedDevice.i"
#define TR98_ASSOC_DEV_STAT           "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.AssociatedDevice.i.Stats"
#define TR98_WEP_KEY                  "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.WEPKey.i"
#define TR98_PSK                      "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.PreSharedKey.i"
#define TR98_AP_WMM_PARAM             "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.APWMMParameter.i"
#define TR98_STA_WMM_PARAM            "InternetGatewayDevice.LANDevice.i.WLANConfiguration.i.STAWMMParameter.i"
#define TR98_HOSTS                    "InternetGatewayDevice.LANDevice.i.Hosts"
#define TR98_HOST                     "InternetGatewayDevice.LANDevice.i.Hosts.Host.i"
#ifdef ONECONNECT
#define TR98_ONECONNECT_EXT			  "InternetGatewayDevice.LANDevice.i.Hosts.Host.i.X_ZYXEL_EXT"
#endif
#define TR98_NEIBOR_WIFI_DIAG         "InternetGatewayDevice.X_ZYXEL_NeighboringWiFiDiagnostic"
#define TR98_NEIBOR_WIFI_DIAG_RESULT  "InternetGatewayDevice.X_ZYXEL_NeighboringWiFiDiagnostic.Result.i"
#define TR98_LAN_INTF                 "InternetGatewayDevice.LANInterfaces"
#define TR98_LAN_INTF_ETH_INTF_CONF   "InternetGatewayDevice.LANInterfaces.LANEthernetInterfaceConfig.i"
#define TR98_USB_INTF_CONF            "InternetGatewayDevice.LANInterfaces.USBInterfaceConfig.i"
#define TR98_LAN_INTF_WLAN_CFG        "InternetGatewayDevice.LANInterfaces.WLANConfiguration.i"
#define TR98_WAN_DEV                  "InternetGatewayDevice.WANDevice.i"
#define TR98_WAN_COMM_INTF_CONF       "InternetGatewayDevice.WANDevice.i.WANCommonInterfaceConfig"
#define TR98_WAN_COMM_INTF_CONNECT    "InternetGatewayDevice.WANDevice.i.WANCommonInterfaceConfig.Connection.i"
#define TR98_WAN_DSL_INTF_CONF        "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig"
#define TR98_WAN_DSL_INTF_TEST_PARAM  "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.TestParams"
#define TR98_WAN_DSL_INTF_CONF_STAT   "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats"
#define TR98_TTL                      "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.Total"
#define TR98_ST                       "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.Showtime"
#define TR98_LST_SHOWTIME             "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.LastShowtime"
#define TR98_CURRENT_DAY              "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.CurrentDay"
#define TR98_QTR_HR                   "InternetGatewayDevice.WANDevice.i.WANDSLInterfaceConfig.Stats.QuarterHour"
#define TR98_WAN_ETH_INTF_CONF        "InternetGatewayDevice.WANDevice.i.WANEthernetInterfaceConfig"
#define TR98_WAN_ETH_INTF_CONF_STAT   "InternetGatewayDevice.WANDevice.i.WANEthernetInterfaceConfig.Stats"
#define TR98_WAN_DSL_CONN_MGMT        "InternetGatewayDevice.WANDevice.i.WANDSLConnectionManagement"
#define TR98_WAN_DSL_CONN_SRVC        "InternetGatewayDevice.WANDevice.i.WANDSLConnectionManagement.ConnectionService.i"
#define TR98_WAN_DSL_DIAG             "InternetGatewayDevice.WANDevice.i.WANDSLDiagnostics"
#define TR98_WAN_CONN_DEV             "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i"
#define TR98_WAN_DSL_LINK_CONF        "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANDSLLinkConfig"
#define TR98_WAN_ATM_F5_LO_DIAG       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANATMF5LoopbackDiagnostics"
#define TR98_WAN_ATM_F4_LO_DIAG       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WANATMF4LoopbackDiagnostics"
#define TR98_WAN_PTM_LINK_CONF        "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPTMLinkConfig"
#define TR98_WAN_PTM_LINK_CONF_STAT   "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPTMLinkConfig.Stats"
#define TR98_WAN_ETH_LINK_CONF        "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANEthernetLinkConfig"
#define TR98_WAN_POTS_LINK_CONF       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPOTSLinkConfig"
#define TR98_WAN_IP_CONN              "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i"
#define TR98_DHCP_CLIENT              "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient"
#define TR98_SENT_DHCP_OPT            "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient.SentDHCPOption.i"
#define TR98_REQ_DHCP_OPT             "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.DHCPClient.ReqDHCPOption.i"
#define TR98_WAN_IP_PORT_MAP          "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.PortMapping.i"
#define TR98_WAN_IP_PORT_TRIGGER      "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_PortTriggering.i"
#define TR98_WAN_IP_ADDR_MAPPING      "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_AddrMapping.i"
#define TR98_WAN_IP_CONN_STAT         "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.Stats"
#define TR98_WAN_IP_CONN_MLD		  "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_MLD"
#ifdef IPV6INTERFACE_PROFILE
#define TR98_WAN_IP_CONN_IPV6ADDR     "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Address.i"
#define TR98_WAN_IP_CONN_IPV6PREFIX	  "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6Prefix.i"
#endif
#ifdef DHCPV6CLIENT_PROFILE
#define TR98_WAN_IP_CONN_DHCPV6CLIENT "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_DHCPv6Client"
#endif
#ifdef IPV6RD_PROFILE
#define TR98_WAN_IP_CONN_IPV6RD	      "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_IPv6rd"
#endif
#ifdef ROUTEINFO_INTERFACE
#define TR98_WAN_IP_CONN_ROUTEINFO    "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANIPConnection.i.X_ZYXEL_RouteInformation"
#endif
#define TR98_WAN_PPP_CONN             "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i"
#define TR98_WAN_PPP_PORT_MAP         "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.PortMapping.i"
#define TR98_WAN_PPP_PORT_TRIGGER     "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_PortTriggering.i"
#define TR98_WAN_PPP_ADDR_MAPPING     "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_AddrMapping.i"
#define TR98_WAN_PPP_CONN_STAT        "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.Stats"
#define TR98_WAN_PPP_CONN_MLD		  "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_MLD"
#ifdef IPV6INTERFACE_PROFILE
#define TR98_WAN_PPP_CONN_IPV6ADDR     "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Address.i"
#define TR98_WAN_PPP_CONN_IPV6PREFIX   "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6Prefix.i"
#endif
#ifdef DHCPV6CLIENT_PROFILE
#define TR98_WAN_PPP_CONN_DHCPV6CLIENT "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_DHCPv6Client"
#endif
#ifdef IPV6RD_PROFILE
#define TR98_WAN_PPP_CONN_IPV6RD       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.WANPPPConnection.i.X_ZYXEL_IPv6rd"
#endif
#define TR98_WWAN_INTERFACE_CONFIG       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig"
#define TR98_WWAN_INTERFACE_CONFIG_STATUS       "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig.Stats"
#define TR98_WWAN_EMAIL_CONFIG               "InternetGatewayDevice.WANDevice.i.WANConnectionDevice.i.X_ZYXEL_WWANInterfaceConfig.Email.i"
#ifdef CONFIG_XPON_SUPPORT
#define TR98_WAN_PON_INTF_CONF        "InternetGatewayDevice.WANDevice.i.X_ZYZEL_WANPonInterfaceConfig"
#define TR98_WAN_PON_INTF_CONF_STAT   "InternetGatewayDevice.WANDevice.i.X_ZYZEL_WANPonInterfaceConfig.Stats"
#endif

#define TR98_FIREWALL                 "InternetGatewayDevice.Firewall"
#define TR98_FIREWALL_LEVEL           "InternetGatewayDevice.Firewall.Level.i"
#define TR98_FIREWALL_CHAIN           "InternetGatewayDevice.Firewall.Chain.i"
#define TR98_FIREWALL_CHAIN_RULE      "InternetGatewayDevice.Firewall.Chain.i.Rule.i"

#define TR98_STD_UPNP				  "InternetGatewayDevice.UPnP"
#define TR98_STD_UPNP_DEV			  "InternetGatewayDevice.UPnP.Device"
#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng
#define TR98_TAAAB_H323Alg                "InternetGatewayDevice.X_TTNET.Configuration.H323_ALG"
#define TR98_TAAAB_SIPAlg                 "InternetGatewayDevice.X_TTNET.Configuration.SIP_ALG"
#define TR98_TAAAB_RTSPAlg                "InternetGatewayDevice.X_TTNET.Configuration.RTSP_ALG"
#define TR98_TAAAB_PPTPAlg                "InternetGatewayDevice.X_TTNET.Configuration.PPTP_ALG"
#define TR98_TAAAB_IPSecAlg               "InternetGatewayDevice.X_TTNET.Configuration.IPSec_ALG"
#define TR98_TAAAB_L2TPAlg                "InternetGatewayDevice.X_TTNET.Configuration.L2TP_ALG"
#define TR98_TAAAB_IGMPProxy              "InternetGatewayDevice.X_TTNET.Configuration.IGMPProxy"
#define TR98_TAAAB_SMTP               	   "InternetGatewayDevice.X_TTNET.Configuration.SMTP"
#define TR98_TAAAB_ICMP	               "InternetGatewayDevice.X_TTNET.Configuration.ICMP"
#define TR98_TAAAB_NTP               	   "InternetGatewayDevice.X_TTNET.Configuration.NTP"
#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
#define TR98_TAAAB_EDNS             	   "InternetGatewayDevice.X_TTNET.Configuration.EDNS"
#endif

#ifdef CONFIG_TAAAB_PORTMIRROR
#define TR98_TAAAB_PORTMIRROR                "InternetGatewayDevice.X_TTNET.PortMirroring"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
#define TR98_TAAAB_PACKETCAPTURE                "InternetGatewayDevice.X_TTNET.PacketCapture"
#define TR98_TAAAB_PACKETCAPTUREUPLOAD                "InternetGatewayDevice.X_TTNET.PacketCapture.Upload"
#endif


#define TR98_ZYXEL_EXT                "InternetGatewayDevice.X_ZYXEL_EXT"
#define TR98_DNS                      "InternetGatewayDevice.X_ZYXEL_EXT.DNS"
#define TR98_DNS_RT_ENTRY             "InternetGatewayDevice.X_ZYXEL_EXT.DNSRouteEntry.i"
#define TR98_DNS_ENTRY     		      "InternetGatewayDevice.X_ZYXEL_EXT.DNSEntry.i"
#define TR98_D_DNS                    "InternetGatewayDevice.X_ZYXEL_EXT.DynamicDNS"
#ifdef ZCFG_TR64
#define TR98_TR064	                  "InternetGatewayDevice.X_ZYXEL_EXT.TR064"
#else
#endif
#define TR98_VLAN_GROUP	              "InternetGatewayDevice.X_ZYXEL_EXT.VlanGroup.i"
#define TR98_EMAIL_NOTIFY             "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification"
#define TR98_EMAIL_SERVICE            "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailService.i"
#define TR98_EMAIL_EVENT_CFG          "InternetGatewayDevice.X_ZYXEL_EXT.EmailNotification.EmailEventConfig.i"
#define TR98_SYS_INFO                 "InternetGatewayDevice.X_ZYXEL_EXT.System_Info"
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
#define TR98_GPON_INFO                "InternetGatewayDevice.X_ZYXEL_EXT.Gpon_Info"
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
#define TR98_SNMP	                  "InternetGatewayDevice.X_ZYXEL_EXT.SNMP"
#define TR98_SNMP_TRAP                "InternetGatewayDevice.X_ZYXEL_EXT.SNMP.TrapAddr.i"
#endif
#define TR98_SCHEDULE                 "InternetGatewayDevice.X_ZYXEL_EXT.Schedule.i"
#define TR98_REMO_MGMT                "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement"
#define TR98_REMO_SRV                 "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.Service.i"
#define TR98_SP_REMO_SRV              "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpService.i"
#define TR98_REMO_TRUSTDOMAIN		  "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.TrustDomain.i"
#define TR98_SP_TRUSTDOMAIN		  "InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.SpTrustDomain.i"
#ifdef ABUU_CUSTOMIZATION
#define TR98_REMO_IFACETRUSTDOMAIN		"InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i"
#define TR98_REMO_IFACETRUSTDOMAININFO	"InternetGatewayDevice.X_ZYXEL_EXT.RemoteManagement.InterfaceTrustDomain.i.TrustDomainInfo.i"
#endif
#define TR98_IEEE8021AG               "InternetGatewayDevice.X_ZYXEL_EXT.IEEE8021ag"
#ifdef ZYXEL_ETHOAM_TMSCTL
#define TR98_IEEE8023AH               "InternetGatewayDevice.X_ZYXEL_EXT.IEEE8023ah"
#endif
#define TR98_SAMBA		              "InternetGatewayDevice.X_ZYXEL_EXT.SAMBA"
#define TR98_UPNP		              "InternetGatewayDevice.X_ZYXEL_EXT.UPNP"
#define TR98_IGMP                     "InternetGatewayDevice.X_ZYXEL_EXT.IGMP"
#define TR98_MLD                      "InternetGatewayDevice.X_ZYXEL_EXT.MLD"
#define TR98_LOG_SETTING              "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting"
#define TR98_LOG_CLASSIFY             "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i"
#define TR98_LOG_CATEGORY		      "InternetGatewayDevice.X_ZYXEL_EXT.Log_Setting.LogClassify.i.Category.i"

#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#define TR98_LOG_CFG           		  "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg"
#define TR98_LOG_CFG_GP           	  "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.i"
#define TR98_LOG_CFG_GP_ACCOUNT       "InternetGatewayDevice.X_ZYXEL_EXT.LoginCfg.LogGp.i.Account.i"
#endif

#define TR98_MACFILTER		  		  "InternetGatewayDevice.X_ZYXEL_EXT.MacFilter"
#define TR98_MACFILTER_WHITELIST	  "InternetGatewayDevice.X_ZYXEL_EXT.MacFilter.WhiteList.i"
#define TR98_PAREN_CTL                "InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl"
#define TR98_PAREN_CTL_PROF           "InternetGatewayDevice.X_ZYXEL_EXT.ParentalControl.Profile.i"
#define TR98_SEC		              "InternetGatewayDevice.X_ZYXEL_EXT.Security"
#define TR98_SEC_CERT		          "InternetGatewayDevice.X_ZYXEL_EXT.Security.Certificate.i"
#define TR98_GUI_CUSTOMIZATION		  "InternetGatewayDevice.X_ZYXEL_EXT.GUI_Customization"
#define TR98_DSL                      "InternetGatewayDevice.X_ZYXEL_EXT.DSL"



#ifdef ONECONNECT
#define TR98_ONECONNECT		          "InternetGatewayDevice.X_ZYXEL_EXT.OneConnect"
#define TR98_ONECONNECT_SPEEDTESTINFO "InternetGatewayDevice.X_ZYXEL_EXT.SpeedTestInfo"
#endif

#ifdef ZYXEL_AP_STEERING
#define TR98_AP_STEERING		      "InternetGatewayDevice.X_ZYXEL_EXT.ApSteering"
#endif //#ifdef ZYXEL_AP_STEERING

#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
#define TR98_PROXIMITY_SENSOR "InternetGatewayDevice.X_ZYXEL_EXT.ProximitySensor"
#endif //#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR

#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
#define TR98_BANDWIDTH_IMPROVEMENT "InternetGatewayDevice.X_ZYXEL_EXT.BandwidthImprovement"
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT

#ifdef ZYXEL_EASYMESH
#define TR98_EasyMesh			"InternetGatewayDevice.X_ZYXEL_EXT.EasyMesh"
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
#define TR98_VendorEasyMesh		"InternetGatewayDevice.X_ZYXEL_Mesh"
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif

#define TR98_WiFiCommunity			"InternetGatewayDevice.X_ZYXEL_EXT.WiFiCommunity"

#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
#define TR98_MultipleConfig           "InternetGatewayDevice.X_ZYXEL_EXT.MultipleConfig"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
#define TR98_PRINT_SERVER           "InternetGatewayDevice.X_ZYXEL_EXT.PrintServer"
#endif

#ifdef ZYXEL_LANDING_PAGE_FEATURE
#define TR98_LANDING_PAGE			"InternetGatewayDevice.X_ZYXEL_EXT.LandingPage"
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
#define TR98_ONLINE_FWUPGRADE			"InternetGatewayDevice.X_ZYXEL_EXT.OnlineFWUgrade"
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
#define TR98_WlanScheduler			"InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler"
#define TR98_WlanScheduler_Rule			"InternetGatewayDevice.X_ZYXEL_EXT.WlanScheduler.Rule.i"
#endif
#define TR98_FEATURE_FLAG			"InternetGatewayDevice.X_ZYXEL_EXT.FeatureFlag"

#ifdef ZYXEL_ONESSID
#define TR98_ONESSID                  "InternetGatewayDevice.X_ZYXEL_EXT.X_ZYXEL_ONESSID"
#endif 

#ifdef ZYXEL_TR69_DATA_USAGE
#define TR98_DATA_USAGE                  "InternetGatewayDevice.X_ZYXEL_DataUsage"
#define TR98_DATA_USAGE_LAN                  "InternetGatewayDevice.X_ZYXEL_DataUsage.Lan.i"
#define TR98_DATA_USAGE_WAN                  "InternetGatewayDevice.X_ZYXEL_DataUsage.Wan.i"
#endif

#ifdef ZYXEL_IPV6_MAP
#define TR98_IPV6MAP                  "InternetGatewayDevice.X_ZYXEL_MAP"
#define TR98_IPV6MAP_DOMAIN           "InternetGatewayDevice.X_ZYXEL_MAP.Domain.i"
#define TR98_IPV6MAP_DOMAIN_RULE      "InternetGatewayDevice.X_ZYXEL_MAP.Domain.i.Rule.i"
#endif

#ifdef ZYXEL_IPV6SPEC
#define TR98_IPV6SPEC					"InternetGatewayDevice.IPv6Specific"
#define TR98_IPV6_PPP					"InternetGatewayDevice.IPv6Specific.PPP"
#define TR98_IPV6_PPP_INTF				"InternetGatewayDevice.IPv6Specific.PPP.Interface.i"
#define TR98_IPV6_PPP_INTF_IPV6CP		"InternetGatewayDevice.IPv6Specific.PPP.Interface.i.IPv6CP"
#define TR98_IPV6_IP					"InternetGatewayDevice.IPv6Specific.IP"
#define TR98_IPV6_IP_INTF				"InternetGatewayDevice.IPv6Specific.IP.Interface.i"
#define TR98_IPV6_IP_INTF_IPV6ADDR		"InternetGatewayDevice.IPv6Specific.IP.Interface.i.IPv6Address.i"
#define TR98_IPV6_IP_INTF_IPV6PREFIX	"InternetGatewayDevice.IPv6Specific.IP.Interface.i.IPv6Prefix.i"
#define TR98_IPV6_ROUTING				"InternetGatewayDevice.IPv6Specific.Routing"
#define TR98_IPV6_ROUTER				"InternetGatewayDevice.IPv6Specific.Routing.Router.i"
#define TR98_IPV6_ROUTER_IPV6FWD		"InternetGatewayDevice.IPv6Specific.Routing.Router.i.IPv6Forwarding.i"
#define TR98_IPV6_ROUTEINFO				"InternetGatewayDevice.IPv6Specific.Routing.RouteInformation"
#define TR98_IPV6_ROUTEINFO_INTF		"InternetGatewayDevice.IPv6Specific.Routing.RouteInformation.InterfaceSetting.i"
#define TR98_IPV6_ND					"InternetGatewayDevice.IPv6Specific.NeighborDiscovery"
#define TR98_IPV6_ND_INTF				"InternetGatewayDevice.IPv6Specific.NeighborDiscovery.InterfaceSetting.i"
#define TR98_IPV6_ROUTERADVER			"InternetGatewayDevice.IPv6Specific.RouterAdvertisement"
#define TR98_IPV6_ROUTERADVER_INTF		"InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.i"
#define TR98_IPV6_ROUTERADVER_OPT		"InternetGatewayDevice.IPv6Specific.RouterAdvertisement.InterfaceSetting.i.Option.i"
#define TR98_IPV6_HOSTS					"InternetGatewayDevice.IPv6Specific.Hosts"
#define TR98_IPV6_HOSTS_HOST			"InternetGatewayDevice.IPv6Specific.Hosts.Host.i"
#define TR98_IPV6_HOSTS_HOST_IPV6ADDR	"InternetGatewayDevice.IPv6Specific.Hosts.Host.i.IPv6Address.i"
#define TR98_IPV6_DNS					"InternetGatewayDevice.IPv6Specific.DNS"
#define TR98_IPV6_DNS_CLIENT			"InternetGatewayDevice.IPv6Specific.DNS.Client"
#define TR98_IPV6_DNS_SERVER			"InternetGatewayDevice.IPv6Specific.DNS.Client.Server.i"
#define TR98_IPV6_DNS_RELAY				"InternetGatewayDevice.IPv6Specific.DNS.Relay"
#define TR98_IPV6_DNS_RELAYFWD			"InternetGatewayDevice.IPv6Specific.DNS.Relay.Forwarding.i"
#define TR98_IPV6_DHCPV6				"InternetGatewayDevice.IPv6Specific.DHCPv6"
#define TR98_IPV6_DHCPV6_CLIENT			"InternetGatewayDevice.IPv6Specific.DHCPv6.Client.i"
#define TR98_IPV6_DHCPV6_CLIENT_SRV		"InternetGatewayDevice.IPv6Specific.DHCPv6.Client.i.Server.i"
#define TR98_IPV6_DHCPV6_CLIENT_SENT	"InternetGatewayDevice.IPv6Specific.DHCPv6.Client.i.SentOption.i"
#define TR98_IPV6_DHCPV6_CLIENT_RCV		"InternetGatewayDevice.IPv6Specific.DHCPv6.Client.i.ReceivedOption.i"
#define TR98_IPV6_DHCPV6_SERVER			"InternetGatewayDevice.IPv6Specific.DHCPv6.Server"
#define TR98_IPV6_DHCPV6_SERVER_POOL	"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i"
#define TR98_IPV6_DHCPV6_SERVER_CNT		"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i.Client.i"
#define TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR		"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i.Client.i.IPv6Address.i"
#define TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX	"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i.Client.i.IPv6Prefix.i"
#define TR98_IPV6_DHCPV6_SERVER_CNT_OPT	"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i.Client.i.Option.i"
#define TR98_IPV6_DHCPV6_SERVER_OPT		"InternetGatewayDevice.IPv6Specific.DHCPv6.Server.Pool.i.Option.i"
#endif

#ifdef ZYXEL_GRE_SUPPORT
#define TR98_XGRE						"InternetGatewayDevice.X_GRE"
#define TR98_XGRE_TUNNEL				"InternetGatewayDevice.X_GRE.Tunnel.i"
#define TR98_XGRE_TUNNEL_STATS			"InternetGatewayDevice.X_GRE.Tunnel.i.Stats"
#endif

#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
#define TR98_TAAAB_Configuration_Button             "InternetGatewayDevice.X_TTNET.Configuration.Buttons"
#define TR98_TAAAB_Configuration_Button_WiFi               "InternetGatewayDevice.X_TTNET.Configuration.Buttons.WiFi"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
#define TR98_TAAAB_Configuration_Quantwifi            "InternetGatewayDevice.X_TTNET.Configuration.QuantWiFiAgent"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
#define TR98_SFP_MGMT						"InternetGatewayDevice.X_ZYXEL_EXT.SFP_Management"
#endif

#ifdef MSTC_TAAAG_MULTI_USER
#define TR98_Nat_DMZ                "InternetGatewayDevice.X_ZYXEL_EXT.X_5067F0_SecDmzHostCfg"
#endif

#if defined(ZYXEL_IGMPDIAG) || defined(ZYXEL_CUSTOMIZATION_SYSLOG)
#define TR98_TAAAB_Function                "InternetGatewayDevice.X_TTNET.Function"
#endif
#ifdef ZYXEL_IGMPDIAG
#define TR98_TTIGMPDiagnostics           "InternetGatewayDevice.X_TTNET.Function.IGMPDiagnostics"
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
#define TR98_TAAAB_WanConnDiagnostics    "InternetGatewayDevice.X_TTNET.Function.Connection"
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
#define TR98_TAAAB_Syslog                "InternetGatewayDevice.X_TTNET.Configuration.Syslog"
#define TR98_TAAAB_SyslogUpload                "InternetGatewayDevice.X_TTNET.Function.SyslogUpload"
#endif

#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
#define TR98_TAAAB                "InternetGatewayDevice.X_TTNET"
#define TR98_TAAAB_Users                "InternetGatewayDevice.X_TTNET.Users"
#define TR98_TAAAB_Users_User                "InternetGatewayDevice.X_TTNET.Users.User.i"
#define TR98_TAAAB_UserInterface                "InternetGatewayDevice.X_TTNET.UserInterface"
#define TR98_TAAAB_UserInterface_RemoteAccess                "InternetGatewayDevice.X_TTNET.UserInterface.RemoteAccess"
#define TR98_TAAAB_UserInterface_LocalAccess                "InternetGatewayDevice.X_TTNET.UserInterface.LocalAccess"
#define TR98_TAAAB_Firewall  				"InternetGatewayDevice.X_TTNET.Firewall"
#define TR98_TAAAB_UPnP                "InternetGatewayDevice.X_TTNET.UPnP"
#define TR98_TAAAB_UPnP_Device                "InternetGatewayDevice.X_TTNET.UPnP.Device"
#define TR98_TAAAB_Configuration                "InternetGatewayDevice.X_TTNET.Configuration"
#define TR98_TAAAB_Configuration_GUI                "InternetGatewayDevice.X_TTNET.Configuration.GUI"
#define TR98_TAAAB_Configuration_GUI_AccessRights                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Internet_DSL"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Internet_WAN"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.IPTV_DSL"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.IPTV_WAN"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.ETHMANAGEMENT_WAN"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.ACS_DSL"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.ACS_WAN"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Broadband.Broadband.Advanced"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.General"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.MoreAP"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS               "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.WPS"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.WMM"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Wireless.Others"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.HomeNetworking"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.HomeNetworking.LANSetup"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.HomeNetworking.LANVLAN"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.Routing"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.NAT"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.NetworkSetting.NAT.ALG"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Security"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Security.Firewall"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Security.Firewall.DoS"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.SystemMonitor"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.SystemMonitor.Log"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.RemoteMGMT"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.TR069"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.Time"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.LogSetting"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.FirmwareUpgrade"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.Configuration"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Maintenance.EDNS"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Engdebug"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Capture"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Easymenu"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Easymenu.EM_RemoteAccess"
#define TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess                "InternetGatewayDevice.X_TTNET.Configuration.GUI.AccessRights.Page.Easymenu.EM_LocalAccess"
#define TR98_TAAAB_Configuration_Shell                "InternetGatewayDevice.X_TTNET.Configuration.Shell"
#endif
#ifdef CONFIG_ZYXEL_TR140
#define TR98_StorageService                           "InternetGatewayDevice.Services.StorageService.i"
#define TR98_StorageService_UserAccount               "InternetGatewayDevice.Services.StorageService.i.UserAccount.i"
#define TR98_StorageService_Capabilites               "InternetGatewayDevice.Services.StorageService.i.Capabilites"
#define TR98_StorageService_NetInfo                   "InternetGatewayDevice.Services.StorageService.i.NetInfo"
#define TR98_StorageService_NetworkServer             "InternetGatewayDevice.Services.StorageService.i.NetworkServer"
#define TR98_StorageService_FTPServer             "InternetGatewayDevice.Services.StorageService.i.FTPServer"
#define TR98_StorageService_PhysicalMedium            "InternetGatewayDevice.Services.StorageService.i.PhysicalMedium.i"
#define TR98_StorageService_LogicalVolume             "InternetGatewayDevice.Services.StorageService.i.LogicalVolume.i"
#define TR98_StorageService_LogicalVolume_Folder      "InternetGatewayDevice.Services.StorageService.i.LogicalVolume.i.Folder.i"
#endif
#ifdef X_ZYXEL_SUPPORT_LIFEMOTE
#define TR98_Configuration_Lifemote             "InternetGatewayDevice.X_ZYXEL_LifeMoteAgent"
#elif ZYXEL_SUPPORT_LIFEMOTE
#define TR98_Configuration_Lifemote             "InternetGatewayDevice.X_TTNET.Configuration.LifemoteAgent"
#endif

#ifdef ZYXEL_XMPP
#define TR98_XMPP                       "InternetGatewayDevice.XMPP"
#define TR98_XMPP_CONNECTION            "InternetGatewayDevice.XMPP.Connection.i"
#define TR98_XMPP_CONNECTION_SERVER     "InternetGatewayDevice.XMPP.Connection.i.Server.i"
#endif

#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
#define TR98_WLANCONFIG                 "InternetGatewayDevice.X_ZYXEL_EXT.WLANConfiguration.i"
#define TR98_ACCOUNT_RESETPASSWD             "InternetGatewayDevice.User.i"
#define TR98_AIRTIMEFAIRNESS                 "InternetGatewayDevice.X_ZYXEL_EXT.AirtimeFairness"
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
/*Support ZyEE*/
#define TR98_ZyEE_SoftwareMoudles                                   "InternetGatewayDevice.SoftwareModules"
#define TR98_ZyEE_SoftwareMoudles_EeConf                            "InternetGatewayDevice.SoftwareModules.X_ZYXEL_ExecEnvConfigure"
#define TR98_ZyEE_SoftwareMoudles_ExecEnv                           "InternetGatewayDevice.SoftwareModules.ExecEnv.i"
#define TR98_ZyEE_SoftwareMoudles_DeploymentUnit                    "InternetGatewayDevice.SoftwareModules.DeploymentUnit.i"
#define TR98_ZyEE_SoftwareMoudles_ExecutionUnit                     "InternetGatewayDevice.SoftwareModules.ExecutionUnit.i"
#define TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions          "InternetGatewayDevice.SoftwareModules.ExecutionUnit.i.Extensions"
#define TR98_DUStateChangeComplPolicy                               "InternetGatewayDevice.DUStateChangeComplPolicy"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
#define TR98_MULTI_WAN			"InternetGatewayDevice.X_ZYXEL_MultiWan"
#define TR98_MULTI_WAN_INTF		"InternetGatewayDevice.X_ZYXEL_MultiWan.Interface.i"
#endif

#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
#define TR98_RUNNING_WAN_CONN         "InternetGatewayDevice.X_ZYXEL_ActiveWANDevice.i"
#endif

#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
#define TR98_CELL		        "InternetGatewayDevice.X_ZYXEL_Cellular"
#define TR98_CELL_INTF		    "InternetGatewayDevice.X_ZYXEL_Cellular.Interface.i"
#define TR98_CELL_AP            "InternetGatewayDevice.X_ZYXEL_Cellular.AccessPoint.i"
#define TR98_CELL_INTF_USIM		"InternetGatewayDevice.X_ZYXEL_Cellular.Interface.i.USIM"
#endif

#define TR98_WAN_IP_V4ROUTINGTABLE    "InternetGatewayDevice.X_ZYXEL_IPV4RoutingTable.i"
#define TR98_WAN_IP_V6ROUTINGTABLE    "InternetGatewayDevice.X_ZYXEL_IPV6RoutingTable.i"

#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
#define TR98_IPK_PKG             "InternetGatewayDevice.Services.X_ZYXEL_Package.i"
#define TR98_IPK_PKG_INSTALL     "InternetGatewayDevice.Services.X_ZYXEL_Package_Install"
#endif

#define OBSOLETED 1
//#define ZYXEL_EXT 1

#define ATTR_INDEX_CREA ATTR_INDEXNODE|OBJECT_ATTR_CREATE
#define ATTR_EXTENDER_HIDDEN OBJECT_ATTR_EXTENDER_HIDDEN

/* TR98 Full Path Name          Attribute	    Parameter List          Get func                 Set func                Add func          Del func         Notify func	Get Attr func		Set Attr func*/
tr98Object_t tr98Obj[] = {
{TR98_IGD},
#ifdef CONFIG_PACKAGE_ZyIMS
{TR98_SERVICE},
{TR98_VOICE_SRV},
{TR98_VOICE_CAPB},
{TR98_VOICE_CAPB_SIP},
//{TR98_VOICE_CAPB_MGCP                     }},
//{TR98_VOICE_CAPB_H323     }},
{TR98_VOICE_CAPB_CODEC},
{TR98_VOICE_PROF},
//{TR98_VOICE_PROF_SRV_PRO_INFO         }},
{TR98_VOICE_PROF_SIP},
//{TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ      }},
//{TR98_VOICE_PROF_SIP_RESP_MAP_OBJ       }},
{TR98_VOICE_PROF_RTP},
{TR98_VOICE_PROF_RTP_RTCP},
{TR98_VOICE_PROF_RTP_SRTP},
//{TR98_VOICE_PROF_RTP_REDUNDANCY         }},
{TR98_VOICE_PROF_NUM_PLAN},
{TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO},
//{TR98_VOICE_PROF_TONE                   }},
//{TR98_VOICE_PROF_TONE_EVENT             }},
//{TR98_VOICE_PROF_TONE_DESCRIPTION       }},
//{TR98_VOICE_PROF_TONE_PATTERN           }},
{TR98_VOICE_PROF_FAX_T38},
{TR98_VOICE_LINE},
{TR98_VOICE_LINE_SIP},
//{TR98_VOICE_LINE_SIP_EVENT_SUBS         }},
//{TR98_VOICE_LINE_RINGER                 }},
//{TR98_VOICE_LINE_RINGER_EVENT           }},
//{TR98_VOICE_LINE_RINGER_DESCRIPTION     }},
//{TR98_VOICE_LINE_RINGER_PATTERN         }},
{TR98_VOICE_LINE_CALLING_FEATURE},
{TR98_VOICE_LINE_PROCESSING},
{TR98_VOICE_LINE_CODEC},
{TR98_VOICE_LINE_CODEC_LIST},
//{TR98_VOICE_LINE_SESSION                }},
{TR98_VOICE_LINE_STATS},
{TR98_VOICE_PHY_INTF},
{TR98_VOICE_FXS_CID},
//{TR98_VOICE_PSTN                        }},
{TR98_VOICE_COMMON},
//{TR98_VOICE_PHONE_BOOK                  }},
{TR98_VOICE_CALL_POLICY_BOOK},
//{TR98_VOICE_FXO                         }},
//{TR98_VOICE_FXO_PORT                    }},
#endif
#ifdef AVAST_AGENT_PACKAGE
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{TR98_ISP_SERVICE},
#endif
#endif
{TR98_SVR_SELECT_DIAG},
{TR98_CAPB},
{TR98_PERF_DIAG},
{TR98_DEV_INFO},
{TR98_MemoryStatus},
{TR98_VEND_CONF_FILE},
{TR98_PROC_ST},
{TR98_PROC_ST_PS},
//{TR98_DEV_CONF},
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
{TR98_DEV_CONF},
#endif
{TR98_MGMT_SRV},
{TR98_MGAB_DEV},
{TR98_TIME},
//{TR98_USR_INTF},
//{TR98_CAPT_PORTAL},
{TR98_L3_FWD},
{TR98_FWD},
#if defined(L3IPV6FWD) && !defined(CONFIG_TAAAB_CUSTOMIZATION_TR069)
{TR98_IPV6_FWD},
#endif
{TR98_L2_BR},
{TR98_BR},
{TR98_PORT},
{TR98_VLAN},
{TR98_FILTER},
{TR98_MARK},
{TR98_AVAI_INTF},
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
{TR98_QUE_MGMT},
{TR98_CLS},
//{TR98_APP},
//{TR98_FLOW},
{TR98_POLICER},
{TR98_QUE},
{TR98_SHAPER},
//{TR98_QUE_STAT},
#endif
{TR98_LAN_CONF_SEC},
{TR98_IP_PING_DIAG},
{TR98_TRA_RT_DIAG},
{TR98_RT_HOP},
{TR98_NSLOOKUP_DIAG},
{TR98_NSLOOKUP_RESULT},
{TR98_DL_DIAG},
{TR98_DL_DIAG_PER_CONN_RST},
{TR98_DL_DIAG_INCREM_RST},
{TR98_UL_DIAG},
{TR98_UL_DIAG_PER_CONN_RST},
{TR98_UL_DIAG_INCREM_RST},
{TR98_UDP_ECHO_CONF},
{TR98_UDP_ECHO_DIAG},
{TR98_UDP_ECHO_DIAG_PAK_RST},
{TR98_LAN_DEV},
{TR98_LAN_HOST_CONF_MGMT},
{TR98_IP_INTF},
#ifdef LANIPALIAS
{TR98_IP_ALIAS},
#endif
#ifdef IPV6INTERFACE_PROFILE
{TR98_IP_INTF_IPV6ADDR},
{TR98_IP_INTF_IPV6PREFIX},
#endif
#ifdef ROUTERADVERTISEMENT_PROFILE
{TR98_IP_INTF_ROUTERADVER},
#endif
#ifdef IPV6SERVER_PROFILE
{TR98_IP_INTF_DHCPV6SRV},
#endif
{TR98_DHCP_STATIC_ADDR},
#ifdef ZYXEL_OPAL_EXTENDER
{TR98_DHCP_OPT},
#else
{TR98_DHCP_OPT},
#endif
#ifdef ZyXEL_DHCP_OPTION125
{TR98_VENDOR_SPECIFIC},
{TR98_VENDOR_SPECIFIC_PROF},
#endif
//{TR98_DHCP_COND_SERVPOOL},
{TR98_DHCP_COND_SERVPOOL},
{TR98_SERVPOOL_DHCP_STATICADDR},
{TR98_SERVPOOL_DHCP_OPT},
{TR98_LAN_ETH_INTF_CONF},
{TR98_LAN_ETH_INTF_CONF_STAT},
//{TR98_LAN_USB_INTF_CONF},
//{TR98_LAN_USB_INTF_CONF_STAT},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_LAN_DEV_WLAN_CFG},
{TR98_WLAN_CFG_STAT},
{TR98_WPS},
#endif
//{TR98_REG},
{TR98_ASSOC_DEV},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_ASSOC_DEV_STAT},
{TR98_WEP_KEY},
{TR98_PSK},
#endif
//{TR98_AP_WMM_PARAM},
//{TR98_STA_WMM_PARAM},
#ifdef ZYXEL_ELIMINATE_DHCP_LEASE
{TR98_HOSTS},
#else
{TR98_HOSTS},
#endif
{TR98_HOST},
#ifdef ONECONNECT
{TR98_ONECONNECT_EXT},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_NEIBOR_WIFI_DIAG},
{TR98_NEIBOR_WIFI_DIAG_RESULT},
#endif
{TR98_LAN_INTF},
//{TR98_LAN_INTF_ETH_INTF_CONF},
//{TR98_USB_INTF_CONF},
//{TR98_LAN_INTF_WLAN_CFG},
{TR98_WAN_DEV},
{TR98_WAN_COMM_INTF_CONF},
//{TR98_WAN_COMM_INTF_CONNECT},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_DSL_INTF_CONF},
{TR98_WAN_DSL_INTF_TEST_PARAM},
{TR98_WAN_DSL_INTF_CONF_STAT},
//{TR98_TTL},
//{TR98_ST},
{TR98_TTL},
{TR98_ST},
//{TR98_LST_SHOWTIME},
{TR98_CURRENT_DAY},
{TR98_QTR_HR},
#endif // CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_ETH_INTF_CONF},
{TR98_WAN_ETH_INTF_CONF_STAT},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_DSL_DIAG},
#endif
{TR98_WAN_CONN_DEV},
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
{TR98_WAN_DSL_LINK_CONF},
{TR98_WAN_ATM_F5_LO_DIAG},
{TR98_WAN_ATM_F4_LO_DIAG},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
{TR98_WAN_PTM_LINK_CONF},
{TR98_WAN_PTM_LINK_CONF_STAT},
#endif
{TR98_WAN_ETH_LINK_CONF},
//{TR98_WAN_POTS_LINK_CONF},
{TR98_WAN_IP_CONN},
{TR98_WAN_IP_CONN_MLD},
{TR98_DHCP_CLIENT},
{TR98_SENT_DHCP_OPT},
{TR98_REQ_DHCP_OPT},
{TR98_WAN_IP_PORT_MAP},
{TR98_WAN_IP_PORT_TRIGGER},
{TR98_WAN_IP_ADDR_MAPPING},
{TR98_WAN_IP_CONN_STAT},
#ifdef IPV6INTERFACE_PROFILE
{TR98_WAN_IP_CONN_IPV6ADDR},
{TR98_WAN_IP_CONN_IPV6PREFIX},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{TR98_WAN_IP_CONN_DHCPV6CLIENT},
#endif
#ifdef IPV6RD_PROFILE
{TR98_WAN_IP_CONN_IPV6RD},
#endif
#ifdef ROUTEINFO_INTERFACE
{TR98_WAN_IP_CONN_ROUTEINFO},
#endif
{TR98_WAN_PPP_CONN},
{TR98_WAN_PPP_PORT_MAP},
{TR98_WAN_PPP_PORT_TRIGGER},
{TR98_WAN_PPP_ADDR_MAPPING},
{TR98_WAN_PPP_CONN_STAT},
{TR98_WAN_PPP_CONN_MLD},
#ifdef IPV6INTERFACE_PROFILE
{TR98_WAN_PPP_CONN_IPV6ADDR},
{TR98_WAN_PPP_CONN_IPV6PREFIX},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{TR98_WAN_PPP_CONN_DHCPV6CLIENT},
#endif
#ifdef IPV6RD_PROFILE
{TR98_WAN_PPP_CONN_IPV6RD},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
{TR98_WWAN_INTERFACE_CONFIG},
{TR98_WWAN_INTERFACE_CONFIG_STATUS},
#endif
{TR98_WWAN_EMAIL_CONFIG},
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
{TR98_WAN_PON_INTF_CONF},
{TR98_WAN_PON_INTF_CONF_STAT},
#endif
{TR98_FIREWALL},
{TR98_FIREWALL_LEVEL},
{TR98_FIREWALL_CHAIN},
{TR98_FIREWALL_CHAIN_RULE},
{TR98_STD_UPNP},
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{TR98_STD_UPNP_DEV},
#endif
#ifdef ZYXEL_IPV6SPEC
{TR98_IPV6SPEC},
{TR98_IPV6_PPP},
{TR98_IPV6_PPP_INTF},
{TR98_IPV6_PPP_INTF_IPV6CP},
{TR98_IPV6_IP},
{TR98_IPV6_IP_INTF},
{TR98_IPV6_IP_INTF_IPV6ADDR},
{TR98_IPV6_IP_INTF_IPV6PREFIX},
{TR98_IPV6_ROUTING},
{TR98_IPV6_ROUTER},
{TR98_IPV6_ROUTER_IPV6FWD},
{TR98_IPV6_ROUTEINFO},
{TR98_IPV6_ROUTEINFO_INTF},
{TR98_IPV6_ND},
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
{TR98_IPV6_ND_INTF},
#endif
{TR98_IPV6_ROUTERADVER},
{TR98_IPV6_ROUTERADVER_INTF},
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
{TR98_IPV6_ROUTERADVER_OPT},
#endif
{TR98_IPV6_HOSTS},
{TR98_IPV6_HOSTS_HOST},
{TR98_IPV6_HOSTS_HOST_IPV6ADDR},
{TR98_IPV6_DNS},
{TR98_IPV6_DNS_CLIENT},
{TR98_IPV6_DNS_SERVER},
#ifdef CONFIG_ZCFG_BE_MODULE_DNS_RELAY
{TR98_IPV6_DNS_RELAY},
{TR98_IPV6_DNS_RELAYFWD},
#endif
{TR98_IPV6_DHCPV6},
{TR98_IPV6_DHCPV6_CLIENT},
{TR98_IPV6_DHCPV6_CLIENT_SRV},
{TR98_IPV6_DHCPV6_CLIENT_SENT},
{TR98_IPV6_DHCPV6_CLIENT_RCV},
{TR98_IPV6_DHCPV6_SERVER},
{TR98_IPV6_DHCPV6_SERVER_POOL},
{TR98_IPV6_DHCPV6_SERVER_CNT},
{TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR},
{TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX},
{TR98_IPV6_DHCPV6_SERVER_CNT_OPT},
{TR98_IPV6_DHCPV6_SERVER_OPT},
#endif
#ifdef ZYXEL_IPV6_MAP
{TR98_IPV6MAP},
{TR98_IPV6MAP_DOMAIN},
{TR98_IPV6MAP_DOMAIN_RULE},
#endif
#ifdef ZYXEL_GRE_SUPPORT
{TR98_XGRE},
{TR98_XGRE_TUNNEL},
{TR98_XGRE_TUNNEL_STATS},
#endif
#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng
{TR98_TAAAB_H323Alg},
{TR98_TAAAB_SIPAlg},
{TR98_TAAAB_RTSPAlg},
{TR98_TAAAB_PPTPAlg},
{TR98_TAAAB_IPSecAlg},
{TR98_TAAAB_L2TPAlg},
{TR98_TAAAB_IGMPProxy},
{TR98_TAAAB_SMTP},
{TR98_TAAAB_ICMP},
{TR98_TAAAB_NTP},
#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
{TR98_TAAAB_EDNS},
#endif
#ifdef CONFIG_TAAAB_PORTMIRROR
{TR98_TAAAB_PORTMIRROR},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
{TR98_TAAAB_PACKETCAPTURE},
{TR98_TAAAB_PACKETCAPTUREUPLOAD},
#endif
#if ZYXEL_EXT
{TR98_ZYXEL_EXT},
#ifdef ZYXEL_LANDING_PAGE_FEATURE
{TR98_LANDING_PAGE},
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
{TR98_ONLINE_FWUPGRADE},
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
{TR98_WlanScheduler},
{TR98_WlanScheduler_Rule},
#endif
{TR98_FEATURE_FLAG},
{TR98_DNS},
{TR98_DNS_RT_ENTRY},
//{TR98_DNS_ENTRY},
{TR98_DNS_ENTRY},
{TR98_D_DNS},
#ifdef ZCFG_TR64
{TR98_TR064},
#else
#endif
//{TR98_VLAN_GROUP},
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
{TR98_VLAN_GROUP},
#endif
{TR98_EMAIL_NOTIFY},
//{TR98_EMAIL_SERVICE},
{TR98_EMAIL_SERVICE},
//{TR98_EMAIL_EVENT_CFG},
{TR98_EMAIL_EVENT_CFG},
{TR98_SYS_INFO},
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
{TR98_GPON_INFO},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
//{TR98_SNMP},
{TR98_SNMP},
{TR98_SNMP_TRAP},
#endif
//{TR98_SCHEDULE},
{TR98_SCHEDULE},
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER)
{TR98_REMO_MGMT},
#else
{TR98_REMO_MGMT},
#endif
{TR98_REMO_SRV},
{TR98_SP_REMO_SRV},
{TR98_REMO_TRUSTDOMAIN},
{TR98_SP_TRUSTDOMAIN},
#ifdef ABUU_CUSTOMIZATION
{TR98_REMO_IFACETRUSTDOMAIN},
{TR98_REMO_IFACETRUSTDOMAININFO},
#endif
{TR98_IEEE8021AG},
#ifdef ZYXEL_ETHOAM_TMSCTL
{TR98_IEEE8023AH},
#endif
//{TR98_SAMBA},
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{TR98_SAMBA},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{TR98_UPNP},
#endif
{TR98_IGMP},
{TR98_MLD},
#ifdef ZYXEL_TR69_DATA_USAGE
{TR98_DATA_USAGE},
{TR98_DATA_USAGE_LAN},
{TR98_DATA_USAGE_WAN},
#endif
#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
{TR98_LOG_CFG},
#else
{TR98_LOG_CFG},
#endif
{TR98_LOG_CFG_GP},
#if defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
{TR98_LOG_CFG_GP_ACCOUNT},
#else
{TR98_LOG_CFG_GP_ACCOUNT},
#endif
#endif
{TR98_LOG_SETTING},
{TR98_LOG_CLASSIFY},
{TR98_LOG_CATEGORY},
{TR98_MACFILTER},
{TR98_MACFILTER_WHITELIST},
{TR98_PAREN_CTL},
//{TR98_PAREN_CTL_PROF},
{TR98_PAREN_CTL_PROF},
{TR98_SEC},
//{TR98_SEC_CERT},
{TR98_SEC_CERT},
{TR98_GUI_CUSTOMIZATION},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_DSL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
{TR98_ONECONNECT},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
{TR98_ONECONNECT_SPEEDTESTINFO},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
{TR98_AP_STEERING},
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
{TR98_PROXIMITY_SENSOR},
#endif // CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
{TR98_BANDWIDTH_IMPROVEMENT},
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_WLANCONFIG},
{TR98_AIRTIMEFAIRNESS},
#endif
{TR98_ACCOUNT_RESETPASSWD},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#if OBSOLETED
#else
//{TR98_WAN_DSL_CONN_MGMT},
//{TR98_WAN_DSL_CONN_SRVC},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
{TR98_PRINT_SERVER},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{TR98_IPK_PKG},
{TR98_IPK_PKG_INSTALL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_WiFiCommunity},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
{TR98_EasyMesh},
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
{TR98_VendorEasyMesh},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
{TR98_ONESSID},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
{TR98_SFP_MGMT},
#endif
#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
{TR98_TAAAB_Configuration_Button},
{TR98_TAAAB_Configuration_Button_WiFi},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
{TR98_TAAAB_Configuration_Quantwifi},
#endif
#ifdef ZYXEL_XMPP
/* TR98 Full Path Name          Attribute       Parameter List          Get func                 Set func                Add func          Del func         Notify func Get Attr func       Set Attr func*/
{TR98_XMPP},
{TR98_XMPP_CONNECTION},
{TR98_XMPP_CONNECTION_SERVER},
#endif
#ifdef MSTC_TAAAG_MULTI_USER
{TR98_Nat_DMZ},
#endif
#if defined(ZYXEL_IGMPDIAG) || defined(ZYXEL_CUSTOMIZATION_SYSLOG)
{TR98_TAAAB_Function},
#endif
#ifdef ZYXEL_IGMPDIAG
{TR98_TTIGMPDiagnostics},
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
{TR98_TAAAB_WanConnDiagnostics},
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
{TR98_TAAAB_Syslog},
{TR98_TAAAB_SyslogUpload},
#endif
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
{TR98_TAAAB},
{TR98_TAAAB_Users},
{TR98_TAAAB_Users_User},
{TR98_TAAAB_UserInterface},
{TR98_TAAAB_UserInterface_RemoteAccess},
{TR98_TAAAB_UserInterface_LocalAccess},
{TR98_TAAAB_Firewall},
{TR98_TAAAB_UPnP},
{TR98_TAAAB_UPnP_Device},
{TR98_TAAAB_Configuration},
{TR98_TAAAB_Configuration_GUI},
{TR98_TAAAB_Configuration_GUI_AccessRights},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess},
{TR98_TAAAB_Configuration_Shell},
#endif
#ifdef CONFIG_ZYXEL_TR140
#ifndef CONFIG_PACKAGE_ZyIMS
{TR98_SERVICE},
#endif
{TR98_StorageService},
{TR98_StorageService_UserAccount},
{TR98_StorageService_Capabilites},
{TR98_StorageService_NetInfo},
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{TR98_StorageService_NetworkServer},
#endif
{TR98_StorageService_FTPServer},
{TR98_StorageService_PhysicalMedium},
{TR98_StorageService_LogicalVolume},
{TR98_StorageService_LogicalVolume_Folder},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
{TR98_Configuration_Lifemote},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
{TR98_ZyEE_SoftwareMoudles},
{TR98_ZyEE_SoftwareMoudles_EeConf},
#ifdef CONFIG_TAAAB_CUSTOMIZATION
{TR98_ZyEE_SoftwareMoudles_ExecEnv},
#else
{TR98_ZyEE_SoftwareMoudles_ExecEnv},
#endif
{TR98_ZyEE_SoftwareMoudles_DeploymentUnit},
{TR98_ZyEE_SoftwareMoudles_ExecutionUnit},
{TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions},
{TR98_DUStateChangeComplPolicy},
#endif
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
{TR98_MultipleConfig},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
{TR98_CELL},
{TR98_CELL_INTF},
{TR98_CELL_INTF_USIM},
{TR98_CELL_AP},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
{TR98_MULTI_WAN},
{TR98_MULTI_WAN_INTF},
#endif
#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
{TR98_RUNNING_WAN_CONN},
#endif
{TR98_WAN_IP_V4ROUTINGTABLE},
{TR98_WAN_IP_V6ROUTINGTABLE},
{NULL}
};

