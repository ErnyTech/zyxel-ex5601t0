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
{TR98_IGD,                      0,              para_Root,              rootObjGet,              NULL,                   NULL,             NULL,            NULL},
#ifdef CONFIG_PACKAGE_ZyIMS
{TR98_SERVICE,					0,				para_Service,			NULL,					 NULL,					 NULL,			   NULL,			NULL},
{TR98_VOICE_SRV,				ATTR_INDEXNODE,	para_VoiceSrv,			voiceSrvObjGet,			 NULL,					 NULL,			   NULL, voiceNotify, voiceSrvAttrGet, voiceSrvAttrSet},
{TR98_VOICE_CAPB,				0,				para_VoiceCapb,			voiceCapbObjGet,			 NULL,					 NULL,			   NULL, voiceNotify, voiceCapbAttrGet, voiceCapbAttrSet},
{TR98_VOICE_CAPB_SIP,			0,				para_VoiceCapbSip,		voiceCapbSipObjGet,		 NULL,					 NULL,			   NULL, voiceNotify, voiceCapbSipAttrGet, voiceCapbSipAttrSet},
//{TR98_VOICE_CAPB_MGCP                     },
//{TR98_VOICE_CAPB_H323     },
{TR98_VOICE_CAPB_CODEC,			ATTR_INDEXNODE,	para_VoiceCapbCodec,	voiceCapbCodecObjGet,		 NULL,					 NULL,			   NULL, voiceNotify, voiceCapbCodecAttrGet, voiceCapbCodecAttrSet},
{TR98_VOICE_PROF,				ATTR_INDEX_CREA,para_VoiceProf,			voiceProfObjGet,			 voiceProfSet,			 voiceProfAdd,	   voiceProfDel, voiceNotify, voiceProfAttrGet, voiceProfAttrSet},
//{TR98_VOICE_PROF_SRV_PRO_INFO         },
{TR98_VOICE_PROF_SIP,			0,				para_VoiceProfSip,		voiceProfSipObjGet,		 voiceProfSipSet,		 NULL,			   NULL, voiceNotify, voiceProfSipAttrGet, voiceProfSipAttrSet},
//{TR98_VOICE_PROF_SIP_SUBSCRIBE_OBJ      },
//{TR98_VOICE_PROF_SIP_RESP_MAP_OBJ       },
{TR98_VOICE_PROF_RTP,			0,				para_VoiceProfRtp,		voiceProfRtpObjGet,			voiceProfRtpObjSet,		NULL,		NULL, voiceNotify, voiceProfRtpAttrGet, voiceProfRtpAttrSet},
{TR98_VOICE_PROF_RTP_RTCP,		0,				para_VoiceProfRtpRtcp,	voiceProfRtpRtcpObjGet,		voiceProfRtpRtcpObjSet,	NULL,		NULL, voiceNotify, voiceProfRtpRtcpAttrGet, voiceProfRtpRtcpAttrSet},
{TR98_VOICE_PROF_RTP_SRTP,		0,				para_VoiceProfRtpSrtp,	voiceProfRtpSrtpObjGet,		voiceProfRtpSrtpObjSet,	NULL,		NULL, voiceNotify, voiceProfRtpSrtpAttrGet, voiceProfRtpSrtpAttrSet},
//{TR98_VOICE_PROF_RTP_REDUNDANCY         },
{TR98_VOICE_PROF_NUM_PLAN, 0 ,	para_VoiceProfNumPlan, voiceProfNumPlanObjGet, voiceProfNumPlanObjSet,	NULL,	NULL, voiceNotify, voiceProfNumPlanAttrGet, voiceProfNumPlanAttrSet},
{TR98_VOICE_PROF_NUM_PLAN_PREFIX_INFO, ATTR_INDEX_CREA,	para_VoiceProfNumPlanPrefixInfo, voiceProfNumPlanPrefixInfoObjGet, voiceProfNumPlanPrefixInfoObjSet,	NULL,	NULL, voiceNotify, voiceProfNumPlanPrefixInfoAttrGet, voiceProfNumPlanPrefixInfoAttrSet},
//{TR98_VOICE_PROF_TONE                   },
//{TR98_VOICE_PROF_TONE_EVENT             },
//{TR98_VOICE_PROF_TONE_DESCRIPTION       },
//{TR98_VOICE_PROF_TONE_PATTERN           },
{TR98_VOICE_PROF_FAX_T38, 0 ,	para_VoiceProfFaxT38, voiceProfFaxT38ObjGet, voiceProfFaxT38ObjSet,	NULL,	NULL, voiceNotify, voiceProfFaxT38AttrGet, voiceProfFaxT38AttrSet},
{TR98_VOICE_LINE,				ATTR_INDEX_CREA,para_VoiceLine,			voiceLineObjGet,			 voiceLineSet,			 voiceLineAdd,	   voiceLineDel,	voiceNotify, voiceLineAttrGet, voiceLineAttrSet},
{TR98_VOICE_LINE_SIP,			0,				para_VoiceLineSip,		voiceLineSipObjGet,		 voiceLineSipSet,		 NULL,			   NULL,	voiceNotify, voiceLineSipAttrGet, voiceLineSipAttrSet},
//{TR98_VOICE_LINE_SIP_EVENT_SUBS         },
//{TR98_VOICE_LINE_RINGER                 },
//{TR98_VOICE_LINE_RINGER_EVENT           },
//{TR98_VOICE_LINE_RINGER_DESCRIPTION     },
//{TR98_VOICE_LINE_RINGER_PATTERN         },
{TR98_VOICE_LINE_CALLING_FEATURE,			0 ,  para_VoiceLineCallingFeature , voiceLineCallingFeatureObjGet  , voiceLineCallingFeatureObjSet , NULL ,NULL , voiceNotify, voiceLineCallingFeatureAttrGet ,voiceLineCallingFeatureAttrSet },
{TR98_VOICE_LINE_PROCESSING,	0,				para_VoiceLineProcessing, voiceLineProcObjGet,		 voiceLineProcSet,		 NULL,			   NULL,	voiceNotify, voiceLineProcAttrGet, voiceLineProcAttrSet},
{TR98_VOICE_LINE_CODEC,			0, 				para_VoiceLineCodec, 	voiceLineCodecObjGet,       NULL,					 NULL,			   NULL,	voiceNotify, voiceLineCodecAttrGet, voiceLineCodecAttrSet},
{TR98_VOICE_LINE_CODEC_LIST,	ATTR_INDEXNODE,	para_VoiceLineCodecList, voiceLineCodecListObjGet,	 voiceLineCodecListObjSet,					 NULL,			   NULL,	voiceNotify, voiceLineCodecListAttrGet, voiceLineCodecListAttrSet},
//{TR98_VOICE_LINE_SESSION                },
{TR98_VOICE_LINE_STATS,			0,              para_VoiceLineStats,    voiceLineStatsObjGet,       voiceLineStatsObjSet,                   NULL,       NULL,	voiceNotify,voiceLineStatsAttrGet,voiceLineStatsAttrSet},
{TR98_VOICE_PHY_INTF,			ATTR_INDEXNODE,	para_VoicePhyIntf,		voicePhyIntfObjGet,		 voicePhyIntfSet,		 NULL,			   NULL,	voiceNotify, voicePhyIntfAttrGet, voicePhyIntfAttrSet},
{TR98_VOICE_FXS_CID,            0,              para_VoiceFxsCID,       voiceFxsCIDObjGet,       voiceFxsCIDObjSet,         NULL,             NULL,            voiceNotify,      voiceFxsCIDAttrGet,   voiceFxsCIDAttrSet},
//{TR98_VOICE_PSTN                        },
{TR98_VOICE_COMMON,				0,				para_VoiceCommon, 		voiceCommonObjGet,			 voiceCommonSet, 	NULL, 		NULL, 	NULL},
//{TR98_VOICE_PHONE_BOOK                  },
{TR98_VOICE_CALL_POLICY_BOOK,	0,				para_VoiceCallPolicyBook,	voiceCallPolicyBookObjGet,		voiceCallPolicyBookObjSet,	NULL,	NULL,	NULL},
//{TR98_VOICE_FXO                         },
//{TR98_VOICE_FXO_PORT                    },
#endif
#ifdef AVAST_AGENT_PACKAGE
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{TR98_ISP_SERVICE,				ATTR_INDEX_CREA,para_IspSrv,		    ispSrvObjGet, 		 ispSrvObjSet,		   ispSrvObjAdd,   ispSrvObjDel,  NULL, NULL, NULL},
#endif
#endif
{TR98_SVR_SELECT_DIAG,          0,              para_SvrSelectDiag,     srvSelectDiagObjGet,     srvSelectDiagObjSet,    NULL,             NULL,            NULL},
{TR98_CAPB,                     0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_PERF_DIAG,                ATTR_EXTENDER_HIDDEN,              para_PerpDiag,          perfDiagObjGet,          NULL,                   NULL,             NULL,            NULL},
{TR98_DEV_INFO,                 0,              para_DevInfo,           devInfoObjGet,           devInfoObjSet,          NULL,             NULL,            devInfoObjNotify, devInfoObjAttrGet, devInfoObjAttrSet},
{TR98_MemoryStatus, 			0,				para_MemoryStatus,		memoryStatusObjGet, 	 NULL,					 NULL,			   NULL,			NULL},
{TR98_VEND_CONF_FILE,           ATTR_INDEXNODE, para_DevInfoVendorCfg,                   devInfoVendorCfgObjGet,                    devInfoVendorCfgObjSet,                   NULL,             NULL,            NULL},
{TR98_PROC_ST, 				0,				    para_ProcSt,			devProcStGet,			 NULL, 		 NULL,			   NULL,			NULL},
{TR98_PROC_ST_PS,           ATTR_INDEXNODE,     para_ProcStPs,          devProcStPsGet,           NULL,          NULL,             NULL,        NULL},
//{TR98_DEV_CONF,                 0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
#ifdef CONFIG_TAAAB_CUSTOMIZATION_TR069
{TR98_DEV_CONF,				0,				para_DevConf,					devConfObjGet, 				 devConfObjSet,				 NULL,	NULL,	NULL,	devConfAttrGet, devConfObjAttrSet},
#endif
{TR98_MGMT_SRV,                 0,              para_MgmtSrv,           mgmtSrvObjGet,           mgmtSrvObjSet,          NULL,             NULL,            NULL, mgmtSrvObjAttrGet, mgmtSrvObjAttrSet},
{TR98_MGAB_DEV,                 ATTR_INDEXNODE, para_MgabDev,           mgabDevObjGet,           NULL,                   NULL,             NULL,            NULL},
{TR98_TIME,                     0,              para_Time,              timeObjGet,              timeObjSet,             NULL,             NULL,            NULL},
//{TR98_USR_INTF,                 0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_CAPT_PORTAL,              0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_L3_FWD,                   0,              para_L3Fwd,             l3fwdObjGet,             l3fwdObjSet,                   NULL,             NULL,            NULL, NULL, NULL},
{TR98_FWD,                      ATTR_INDEX_CREA,para_Fwd,               l3fwdFwdTbObjGet,        l3fwdFwdTbObjSet,       l3fwdFwdTbObjAdd, l3fwdFwdTbObjDel,  NULL, NULL, l3fwdFwdTbObjAttrSet},
#if defined(L3IPV6FWD) && !defined(CONFIG_TAAAB_CUSTOMIZATION_TR069)
{TR98_IPV6_FWD,					ATTR_INDEX_CREA,para_Ipv6Fwd,			l3fwdIpv6FwdTbObjGet,	  l3fwdIpv6FwdTbObjSet,	 l3fwdIpv6FwdTbObjAdd, l3fwdIpv6FwdTbObjDel,  NULL, NULL, NULL},
#endif
{TR98_L2_BR,                    ATTR_EXTENDER_HIDDEN,                   para_L2Br,              l2BridingObjGet,         NULL,                   NULL,             NULL,            l2BridingObjNotify, l2BridingObjAttrGet, l2BridingObjAttrSet},
{TR98_BR,                       ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Br,                l2BrObjGet,              l2BrObjSet,             l2BrObjAdd,       l2BrObjDel,      l2BrObjNotify, l2BrObjAttrGet, l2BrObjAttrSet},
{TR98_PORT,                     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Port,              l2BrPortObjGet,           l2BrPortObjSet,         l2BrPortObjAdd,   l2BrPortObjDel,   NULL,         NULL,           NULL},
{TR98_VLAN,                     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,  para_Vlan,              l2BrVlanObjGet,          l2BrVlanObjSet,        l2BrVlanObjAdd,   l2BrVlanObjDel,  l2BrVlanObjNotify, l2BrVlanObjAttrGet, l2BrVlanObjAttrSet},
{TR98_FILTER,                   ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Filter,            l2BrFilterObjGet,        l2BrFilterObjSet,       l2BrFilterObjAdd, l2BrFilterObjDel,l2BrFilterObjNotify, l2BrFilterObjAttrGet, l2BrFilterObjAttrSet},
{TR98_MARK,                     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Mark,              NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_AVAI_INTF,                ATTR_INDEXNODE  | ATTR_EXTENDER_HIDDEN,  para_AvaiIntf,          l2BrAvailableIntfObjGet, NULL,                   NULL,             NULL,            l2BrAvailableIntfObjNotify, l2BrAvailableIntfObjAttrGet, l2BrAvailableIntfObjAttrSet},
#ifdef CONFIG_ZCFG_BE_MODULE_QOS
{TR98_QUE_MGMT,                 ATTR_EXTENDER_HIDDEN,               para_qMgmt,          	qMgmtObjGet,     		   qMgmtSet,               NULL,             NULL,            qMgmtNotify, qMgmtAttrGet, qMgmtAttrSet},
{TR98_CLS,                      ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Cls,          	qMgmtClsObjGet,			   qMgmtClsSet,            qMgmtClsAdd,      qMgmtClsDel,     qMgmtClsNotify, qMgmtClsAttrGet, qMgmtClsAttrSet},
//{TR98_APP,                      ATTR_INDEX_CREA, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_FLOW,                     ATTR_INDEX_CREA, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_POLICER,                  ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Policer,          qMgmtPolicerGet,            qMgmtPolicerSet,          qMgmtPolicerAdd,     qMgmtPolicerDel,     NULL},
{TR98_QUE,                      ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Que,              qMgmtQueObjGet,			   qMgmtQueSet,            qMgmtQueAdd,      qMgmtQueDel,     qMgmtQueNotify, qMgmtQueAttrGet, qMgmtQueAttrSet},
{TR98_SHAPER,                   ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_Shaper,			qMgmtShaperObjGet,	   	   qMgmtShaperSet,         qMgmtShaperAdd,   qMgmtShaperDel,  qMgmtShaperNotify, qMgmtShaperAttrGet, qMgmtShaperAttrSet},
//{TR98_QUE_STAT,                 ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
#endif
{TR98_LAN_CONF_SEC,             ATTR_EXTENDER_HIDDEN,               para_LanConfSec,        lanConfSecObjGet,        lanConfSecObjSet,       NULL,             NULL,            lanConfSecObjNotify, lanConfSecObjAttrGet, lanConfSecObjAttrSet},
{TR98_IP_PING_DIAG,             0,              para_IpPingDiag,        ipPingDiagObjGet,        ipPingDiagObjSet,       NULL,             NULL,            NULL},
{TR98_TRA_RT_DIAG,              0,              para_TraceRtDiag,       traceRtDiagObjGet,       traceRtDiagObjSet,      NULL,             NULL,            NULL},
{TR98_RT_HOP,                   ATTR_INDEXNODE, para_RtHop,		        routeHopsObjGet,         NULL,                   NULL,             NULL,            NULL},
{TR98_NSLOOKUP_DIAG,            0,              para_NsLookUpDiag,      nsLookUpDiagObjGet,      nsLookUpDiagObjSet,     NULL,             NULL,            NULL},
{TR98_NSLOOKUP_RESULT,          ATTR_INDEXNODE, para_NsLookUpResult,	nsLookUpResultObjGet,    NULL,                   NULL,             NULL,            NULL},
{TR98_DL_DIAG,                  0,              para_DlDiag,			dlDiagObjGet,            dlDiagObjSet,           NULL,             NULL,            NULL},
{TR98_DL_DIAG_PER_CONN_RST,		ATTR_INDEXNODE,	para_DlDiagPerConnRst,  dlDiagPerConnRstObjGet,	 NULL,			         NULL,			   NULL,			NULL},
{TR98_DL_DIAG_INCREM_RST,       ATTR_INDEXNODE, para_DlDiagIncremRst,	dlDiagIncremRstObjGet,   NULL,                   NULL,             NULL,            NULL},
{TR98_UL_DIAG,                  0,              para_UlDiag,			ulDiagObjGet,            ulDiagObjSet,           NULL,             NULL,            NULL},
{TR98_UL_DIAG_PER_CONN_RST, 	ATTR_INDEXNODE,	para_UlDiagPerConnRst,	ulDiagPerConnRstObjGet,	 NULL,					 NULL,			   NULL,			NULL},
{TR98_UL_DIAG_INCREM_RST,		ATTR_INDEXNODE,	para_UlDiagIncremRst,	ulDiagIncremRstObjGet,	 NULL,					 NULL,			   NULL,			NULL},
{TR98_UDP_ECHO_CONF,            0,              para_UdpEchoCfg,        udpEchoCfgObjGet,        udpEchoCfgObjSet,       NULL,             NULL,            NULL},
{TR98_UDP_ECHO_DIAG,			0,				para_UdpEchoDiag,		udpEchoDiagObjGet,		 udpEchoDiagObjSet,		 NULL,			   NULL,			NULL},
{TR98_UDP_ECHO_DIAG_PAK_RST,	ATTR_INDEXNODE,	para_UdpEchoDiagPakRst, udpEchoDiagPakRstObjGet, NULL, NULL, NULL, NULL, NULL, NULL},
{TR98_LAN_DEV,                  ATTR_INDEX_CREA,para_LanDev,            lanDevObjGet,            NULL,                   lanDevObjAdd,     lanDevObjDel,    NULL},
{TR98_LAN_HOST_CONF_MGMT,       0,              para_LanHostConfMgmt,   lanHostConfMgmtObjGet,   lanHostConfMgmtObjSet,  NULL,             NULL,            NULL},
{TR98_IP_INTF,                  ATTR_INDEX_CREA,para_IpIntf,		    lanIpIntfObjGet,         lanIpIntfObjSet,        lanIpIntfObjAdd,  lanIpIntfObjDel,	NULL},
#ifdef LANIPALIAS
{TR98_IP_ALIAS,            ATTR_INDEX_CREA, para_IpAlias, lanIpAliasObjGet, lanIpAliasObjSet, lanIpAliasObjAdd, lanIpIntfObjDel, NULL, NULL, NULL},
#endif
#ifdef IPV6INTERFACE_PROFILE
{TR98_IP_INTF_IPV6ADDR,         ATTR_INDEX_CREA,para_IpIntfV6Addr,		lanIpIntfV6AddrObjGet,   lanIpIntfV6AddrObjSet,  lanIpIntfV6AddrObjAdd,  lanIpIntfV6AddrObjDel,	NULL},
{TR98_IP_INTF_IPV6PREFIX,		ATTR_INDEX_CREA,para_IpIntfV6Prefix,	lanIpIntfV6PrefixObjGet, lanIpIntfV6PrefixObjSet,lanIpIntfV6PrefixObjAdd,lanIpIntfV6PrefixObjDel, NULL},
#endif
#ifdef ROUTERADVERTISEMENT_PROFILE
{TR98_IP_INTF_ROUTERADVER,      ATTR_EXTENDER_HIDDEN,				para_IpIntfRouterAdver, lanIpIntfRouterAdverObjGet, lanIpIntfRouterAdverObjSet,        NULL,  NULL,	NULL},
#endif
#ifdef IPV6SERVER_PROFILE
{TR98_IP_INTF_DHCPV6SRV,        ATTR_EXTENDER_HIDDEN,				para_IpIntfV6Srv,		lanIpIntfV6SrvObjGet,    lanIpIntfV6SrvObjSet,        NULL,  NULL,	NULL},
#endif
{TR98_DHCP_STATIC_ADDR,         ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DhcpStaticAddr,    lanDhcpStaticAddrObjGet, lanDhcpStaticAddrObjSet,lanDhcpStaticAddrObjAdd,lanDhcpStaticAddrObjDel, lanDhcpStaticAddrObjNotify, lanDhcpStaticAddrObjAttrGet, lanDhcpStaticAddrObjAttrSet},
#ifdef ZYXEL_OPAL_EXTENDER
{TR98_DHCP_OPT,                 ATTR_INDEX_CREA, para_DhcpOpt,           lanDhcpOptObjGet,        lanDhcpOptObjSet,       lanDhcpOptObjAdd, lanDhcpOptObjDel,lanDhcpOptObjNotify, lanDhcpOptObjAttrGet, lanDhcpOptObjAttrSet},
#else
{TR98_DHCP_OPT,                 ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DhcpOpt,           lanDhcpOptObjGet,        lanDhcpOptObjSet,       lanDhcpOptObjAdd, lanDhcpOptObjDel,lanDhcpOptObjNotify, lanDhcpOptObjAttrGet, lanDhcpOptObjAttrSet},
#endif
#ifdef ZyXEL_DHCP_OPTION125
{TR98_VENDOR_SPECIFIC,                 0,              para_VendorSpecific,          zyExtVendorSpecObjGet,          zyExtVendorSpecObjSet,         NULL,             NULL,            NULL, NULL, NULL},
{TR98_VENDOR_SPECIFIC_PROF,           ATTR_INDEX_CREA, para_VendorSpecificProf,     zyExtVendorSpecProfObjGet,     zyExtVendorSpecProfObjSet,    zyExtVendorSpecProfObjAdd,zyExtVendorSpecProfObjDel,NULL, NULL, NULL},
#endif
//{TR98_DHCP_COND_SERVPOOL,       ATTR_INDEX_CREA,para_DhcpCondServPool,  lanDhcpCondServPoolObjGet,  lanDhcpCondServPoolObjSet,  lanDhcpCondServPoolObjAdd,  lanDhcpCondServPoolObjDel, lanDhcpCondServPoolObjNotify, lanDhcpCondServPoolObjAttrGet, lanDhcpCondServPoolObjAttrSet},
{TR98_DHCP_COND_SERVPOOL,       ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DhcpCondServPool,  lanDhcpCondServPoolObjGet,  lanDhcpCondServPoolObjSet,  lanDhcpCondServPoolObjAdd,  lanDhcpCondServPoolObjDel, NULL, NULL, NULL},
{TR98_SERVPOOL_DHCP_STATICADDR, ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DhcpCondServPoolDhcpStaticAddr,NULL, NULL, NULL, NULL, NULL, NULL, NULL},
{TR98_SERVPOOL_DHCP_OPT,        ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DhcpCondServPoolDhcpOpt, lanDhcpCondServPoolOptObjGet,  lanDhcpCondServPoolOptObjSet,  lanDhcpCondServPoolOptObjAdd,  lanDhcpCondServPoolOptObjDel, NULL, NULL, NULL},
{TR98_LAN_ETH_INTF_CONF,        ATTR_INDEXNODE, para_LanEthIntfConf,    lanEthIntfConfObjGet,    lanEthIntfConfObjSet,   NULL,             NULL,            lanEthIntfConfObjNotify, lanEthIntfConfObjAttrGet, lanEthIntfConfObjAttrSet},
{TR98_LAN_ETH_INTF_CONF_STAT,   0,              para_LanEthIntfConfStat,lanEthIntfConfStatObjGet,NULL,                   NULL,             NULL,            NULL},
//{TR98_LAN_USB_INTF_CONF,        ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_LAN_USB_INTF_CONF_STAT,   0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_LAN_DEV_WLAN_CFG, ATTR_INDEXNODE,	para_LanDevWlanCfg,     lanDevWlanCfgObjGet,     lanDevWlanCfgObjSet,    NULL, NULL, lanDevWlanCfgObjNotify, lanDevWlanCfgObjAttrGet, lanDevWlanCfgObjAttrSet},
{TR98_WLAN_CFG_STAT,            0,      para_LanDevWlanCfgStat, lanDevWlanCfgStatObjGet, NULL,                   NULL, NULL, NULL},
{TR98_WPS,                      0,      para_LanDevWlanCfgWps,  lanDevWlanCfgWpsObjGet,  lanDevWlanCfgWpsObjSet, NULL, NULL, NULL},
#endif
//{TR98_REG,            ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL, NULL, NULL},
{TR98_ASSOC_DEV,        ATTR_INDEXNODE, para_LanDevWlanCfgAssocDev, lanDevWlanCfgAssocDevObjGet,           NULL, NULL, NULL, NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_ASSOC_DEV_STAT,			0,      para_LanDevWlanCfgAssocDevStat, lanDevWlanCfgAssocDevStatObjGet,   NULL, NULL, NULL, NULL},
{TR98_WEP_KEY,  ATTR_INDEXNODE | ATTR_EXTENDER_HIDDEN, para_WepKey, lanDevWlanCfgWEPKeyObjGet, lanDevWlanCfgWEPKeyObjSet, NULL, NULL, NULL},
{TR98_PSK,              ATTR_INDEXNODE, para_Psk,               lanDevWlanCfgPskObjGet,  lanDevWlanCfgPskObjSet, NULL, NULL, NULL},
#endif
//{TR98_AP_WMM_PARAM,   ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL, NULL, NULL},
//{TR98_STA_WMM_PARAM,  ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL, NULL, NULL},
#ifdef ZYXEL_ELIMINATE_DHCP_LEASE
{TR98_HOSTS,                    0, para_Hosts,				lanDevHostsObjGet,			lanDevHostsObjSet,                   NULL,             NULL,            lanDevHostsObjNotify, NULL, lanDevHostsObjAttrSet},
#else
{TR98_HOSTS,                    0, para_Hosts,				lanDevHostsObjGet,			NULL,                   NULL,             NULL,            lanDevHostsObjNotify, NULL, lanDevHostsObjAttrSet},
#endif
{TR98_HOST,                     ATTR_INDEXNODE, para_Host,				lanDevHostObjGet,			lanDevHostObjSet,                   NULL,             NULL,            lanDevHostObjNotify, lanDevHostObjAttrGet, lanDevHostObjAttrSet},
#ifdef ONECONNECT
{TR98_ONECONNECT_EXT,			0,				para_OneConnectExt, 	oneConnectExtObjGet,		 oneConnectExtObjSet,		   NULL,			 NULL,			  NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_NEIBOR_WIFI_DIAG,	       0,              para_NeiborWifiDiag,	wifiNeiborDiagObjGet, wifiNeiborDiagObjSet, NULL, NULL, wifiNeiborDiagObjNotify, wifiNeiborDiagObjAttrGet, wifiNeiborDiagObjAttrSet},
{TR98_NEIBOR_WIFI_DIAG_RESULT, ATTR_INDEXNODE, para_NeiborWifiDiagResult, wifiNeiborDiagResultObjGet, NULL, NULL, NULL, wifiNeiborDiagResultObjNotify, wifiNeiborDiagResultObjAttrGet, wifiNeiborDiagResultObjAttrSet},
#endif
{TR98_LAN_INTF,                 0, para_LanDev,           lanIfacesObjGet,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_LAN_INTF_ETH_INTF_CONF,   ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_USB_INTF_CONF,            ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_LAN_INTF_WLAN_CFG,        ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_WAN_DEV,                  ATTR_INDEXNODE | ATTR_EXTENDER_HIDDEN,	para_WanDev,            WANDeviceObjGet,         WANDeviceObjSet,                   NULL,             NULL,            NULL},
{TR98_WAN_COMM_INTF_CONF,       ATTR_EXTENDER_HIDDEN,              para_WANCommIfaceCfg,   WANCommIfaceCfgObjGet,   NULL,                   NULL,             NULL,            NULL},
//{TR98_WAN_COMM_INTF_CONNECT,    ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_DSL_INTF_CONF,        ATTR_EXTENDER_HIDDEN,              para_WANDslIfaceCfg,    WANDslIfaceCfgObjGet,    WANDslIfaceCfgObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_DSL_INTF_TEST_PARAM,  ATTR_EXTENDER_HIDDEN,              para_WANDslIfTestParam, WANDslIfTestParamObjGet,    NULL,                   NULL,             NULL,            NULL},
{TR98_WAN_DSL_INTF_CONF_STAT,   ATTR_EXTENDER_HIDDEN,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_TTL,                      0,              para_WANDslIfaceCfgStatTotal,  WANDslIfaceCfgTtlObjGet,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_ST,                       0,              para_WANDslIfaceCfgStatSt,     WANDslIfaceCfgStObjGet,                    NULL,                   NULL,             NULL,            NULL},
{TR98_TTL,                      ATTR_EXTENDER_HIDDEN,              para_WANDslIfaceCfgStatObjs,  WANDslIfaceCfgStatsSubObjGet,                    NULL,                   NULL,             NULL,            NULL},
{TR98_ST,                       ATTR_EXTENDER_HIDDEN,              para_WANDslIfaceCfgStatObjs,     WANDslIfaceCfgStatsSubObjGet,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_LST_SHOWTIME,             0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_CURRENT_DAY,              ATTR_EXTENDER_HIDDEN,              para_WANDslIfaceCfgStatObjs,   WANDslIfaceCfgStatsSubObjGet,                    NULL,                   NULL,             NULL,            NULL, NULL, NULL},
{TR98_QTR_HR,                   ATTR_EXTENDER_HIDDEN,              para_WAN_QTR_HR,        WANDslIfaceCfgStatsSubObjGet,                    NULL,                   NULL,             NULL,            NULL},
#endif // CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_ETH_INTF_CONF,        ATTR_EXTENDER_HIDDEN,              para_WANEthIntfConf,    WANEthIfaceCfgObjGet,	 WANEthIfaceCfgObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_ETH_INTF_CONF_STAT,   ATTR_EXTENDER_HIDDEN,              para_WANEthIntfConfStat,WANEthIfaceCfgStObjGet,  NULL,                   NULL,             NULL,            NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_WAN_DSL_DIAG,             ATTR_EXTENDER_HIDDEN,              para_WANDslDiag,        WANDslDiagObjGet,        WANDslDiagObjSet,       NULL,             NULL,            NULL},
#endif
{TR98_WAN_CONN_DEV,             ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANConnDev,        WANConnDevObjGet,        NULL,                   WANConnDevObjAdd, WANConnDevObjDel,NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_ATM_LINK
{TR98_WAN_DSL_LINK_CONF,        ATTR_EXTENDER_HIDDEN,              para_WANDslLinkConf,    WANDslLinkConfObjGet,    WANDslLinkConfObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_ATM_F5_LO_DIAG,       ATTR_EXTENDER_HIDDEN,              para_WanAtmF5LoDiag,    WANAtmF5LoConfObjGet,    WANAtmF5LoConfObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_ATM_F4_LO_DIAG,		ATTR_EXTENDER_HIDDEN,				para_WanAtmF4LoDiag,	WANAtmF4LoConfObjGet,	 WANAtmF4LoConfObjSet,	 NULL,			   NULL,			NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PTM_LINK
{TR98_WAN_PTM_LINK_CONF,        ATTR_EXTENDER_HIDDEN,              para_WANPtmLinkConf,    WANPtmLinkConfObjGet,    WANPtmLinkConfObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_PTM_LINK_CONF_STAT,   ATTR_EXTENDER_HIDDEN,              para_WANPtmLinkConfStat,WANPtmLinkConfStObjGet,  NULL,                   NULL,             NULL,            NULL},
#endif
{TR98_WAN_ETH_LINK_CONF,        ATTR_EXTENDER_HIDDEN,              para_WANEthLinkConf,    WANEthLinkConfObjGet,    NULL,                   NULL,             NULL,            NULL},
//{TR98_WAN_POTS_LINK_CONF,       0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
{TR98_WAN_IP_CONN,              ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANIpConn,         WANIpConnObjGet,         WANIpConnObjSet,        WANIpConnObjAdd,  WANIpConnObjDel, WANIpConnObjNotify, WANIpConnObjAttrGet, WANIpPppConnObjAttrSet},
{TR98_WAN_IP_CONN_MLD,	        ATTR_EXTENDER_HIDDEN,              para_WANIpConnMld,      WANConnMldObjGet,        WANConnMldObjSet,       NULL, 			   NULL, 			WANConnMldObjNotify, NULL, WANConnMldObjAttrSet},
{TR98_DHCP_CLIENT,              ATTR_EXTENDER_HIDDEN,              para_DhcpClient,        DhcpClientObjGet,        DhcpClientObjSet,       NULL,             NULL, DhcpClientObjNotify, DhcpClientObjAttrGet, DhcpClientObjAttrSet},
{TR98_SENT_DHCP_OPT,            ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_SentDhcpOpt,      DhcpClientSentOptObjGet, DhcpClientSentOptObjSet, DhcpClientSentOptObjAdd, DhcpClientSentOptObjDel, DhcpClientOptObjNotify, DhcpClientSentOptObjAttrGet, DhcpClientSentOptObjAttrSet},
{TR98_REQ_DHCP_OPT,             ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_ReqDhcpOpt,       DhcpClientReqOptObjGet,  DhcpClientReqOptObjSet, DhcpClientReqOptObjAdd, DhcpClientReqOptObjDel, DhcpClientOptObjNotify, DhcpClientReqOptObjAttrGet, DhcpClientReqOptObjAttrSet},
{TR98_WAN_IP_PORT_MAP,          ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANIpPortMap,      WANPortMappingObjGet,       WANPortMappingSet,      WANPortMappingAdd, WANPortMappingDel, WANPortMappingNotify, WANPortMappingAttrGet, WANPortMappingAttrSet},
{TR98_WAN_IP_PORT_TRIGGER,      ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANIpPortTrigger, WANPortTriggerObjGet,    WANPortTriggerSet,      WANPortTriggerAdd, WANPortTriggerDel, WANPortTriggerNotify, WANPortTriggerAttrGet, WANPortTriggerAttrSet},
{TR98_WAN_IP_ADDR_MAPPING,      ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANIpAddrMapping, WANAddrMappingObjGet,    WANAddrMappingSet,      WANAddrMappingAdd, WANAddrMappingDel, WANAddrMappingNotify, WANAddrMappingAttrGet, WANAddrMappingAttrSet},
{TR98_WAN_IP_CONN_STAT,         ATTR_EXTENDER_HIDDEN,              para_WANIpConnStat,     WANConnStObjGet,         NULL,                   NULL,     NULL, WANConnStObjNotify, WANConnStObjAttrGet, WANConnStObjAttrSet},
#ifdef IPV6INTERFACE_PROFILE
{TR98_WAN_IP_CONN_IPV6ADDR,     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANIpv6Addr,      WANIpv6AddrObjGet,        WANIpv6AddrObjSet,     WANIpv6AddrObjAdd, WANIpv6AddrObjDel,            NULL},
{TR98_WAN_IP_CONN_IPV6PREFIX, 	ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANIpv6Prefix, 	WANIpv6PrefixObjGet,	 WANIpv6PrefixObjSet,   WANIpv6PrefixObjAdd, WANIpv6PrefixObjDel,			NULL},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{TR98_WAN_IP_CONN_DHCPV6CLIENT, ATTR_EXTENDER_HIDDEN, 				para_WANIpDhcpV6Client,     WANIpv6DhcpV6ObjGet, WANIpv6DhcpV6ObjSet,    NULL,             NULL,            NULL},
#endif
#ifdef IPV6RD_PROFILE
{TR98_WAN_IP_CONN_IPV6RD,       ATTR_EXTENDER_HIDDEN,              para_WANIpv6Rd,     	WANIpv6RdObjGet,         WANIpv6RdObjSet,        NULL,             NULL,            NULL},
#endif
#ifdef ROUTEINFO_INTERFACE
{TR98_WAN_IP_CONN_ROUTEINFO,    ATTR_EXTENDER_HIDDEN,				para_WANIpIntfRouteInfo,WANIpIntfRouteInfoObjGet, NULL,        NULL,  NULL,	NULL},
#endif
{TR98_WAN_PPP_CONN,             ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANPppConn,        WANPppConnObjGet,        WANPppConnObjSet,       WANPppConnObjAdd, WANPppConnObjDel, WANPppConnObjNotify, WANPppConnObjAttrGet, WANIpPppConnObjAttrSet},
{TR98_WAN_PPP_PORT_MAP,         ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANPppPortMap,     WANPortMappingObjGet,    WANPortMappingSet,      WANPortMappingAdd, WANPortMappingDel, WANPortMappingNotify, WANPortMappingAttrGet, WANPortMappingAttrSet},
{TR98_WAN_PPP_PORT_TRIGGER,     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANPppPortTrigger, WANPortTriggerObjGet,    WANPortTriggerSet,      WANPortTriggerAdd, WANPortTriggerDel, WANPortTriggerNotify, WANPortTriggerAttrGet, WANPortTriggerAttrSet},
{TR98_WAN_PPP_ADDR_MAPPING,     ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_WANPppAddrMapping, WANAddrMappingObjGet,    WANAddrMappingSet,      WANAddrMappingAdd, WANAddrMappingDel, WANAddrMappingNotify, WANAddrMappingAttrGet, WANAddrMappingAttrSet},
{TR98_WAN_PPP_CONN_STAT,        ATTR_EXTENDER_HIDDEN,              para_WANPppConnStat,    WANConnStObjGet,         NULL,                   NULL,   NULL, WANConnStObjNotify, WANConnStObjAttrGet, WANConnStObjAttrSet},
{TR98_WAN_PPP_CONN_MLD,	        ATTR_EXTENDER_HIDDEN,              para_WANPppConnMld,     WANConnMldObjGet,        WANConnMldObjSet,       NULL, 			   NULL, 			WANConnMldObjNotify, NULL, WANConnMldObjAttrSet},
#ifdef IPV6INTERFACE_PROFILE
{TR98_WAN_PPP_CONN_IPV6ADDR,    ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANPppv6Addr,     WANPppv6AddrObjGet,        WANPppv6AddrObjSet,     WANPppv6AddrObjAdd, WANPppv6AddrObjDel,            NULL},
{TR98_WAN_PPP_CONN_IPV6PREFIX, 	ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN, para_WANPppv6Prefix,	WANPppv6PrefixObjGet,	 WANPppv6PrefixObjSet,   WANPppv6PrefixObjAdd, WANPppv6PrefixObjDel,			NULL},
#endif
#ifdef DHCPV6CLIENT_PROFILE
{TR98_WAN_PPP_CONN_DHCPV6CLIENT, 0, 			para_WANPppDhcpV6Client, WANPppv6DhcpV6ObjGet, WANPppv6DhcpV6ObjSet,    NULL,             NULL,            NULL},
#endif
#ifdef IPV6RD_PROFILE
{TR98_WAN_PPP_CONN_IPV6RD,      0,				para_WANPppv6Rd,        WANPppv6RdObjGet,         WANPppv6RdObjSet,        NULL,             NULL,            NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WWAN_BACKUP
{TR98_WWAN_INTERFACE_CONFIG,	0,	para_WANWwanIntfConf,	wwanObjGet,   wwanObjSet,   NULL,             NULL,            NULL},
{TR98_WWAN_INTERFACE_CONFIG_STATUS,	0,	para_WANWwanIntfStatsConf,	wwanObjStatsGet,   NULL,   NULL,             NULL,            NULL},
#endif
{TR98_WWAN_EMAIL_CONFIG,	ATTR_INDEXNODE,	para_WANWwanEmailConf,	wwanEmailObjGet,   wwanEmailObjSet,   NULL,             NULL,            NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_OPTICAL
{TR98_WAN_PON_INTF_CONF,        0,				para_WANPonIntfConf,     WANPonIfaceCfgObjGet,   WANPonIfaceCfgObjSet,   NULL,             NULL,            NULL},
{TR98_WAN_PON_INTF_CONF_STAT,   0,			    para_WANPonIntfConfStat, WANPonIfaceCfgStObjGet, NULL,                   NULL,             NULL,            NULL},
#endif
{TR98_FIREWALL,                 ATTR_EXTENDER_HIDDEN,              para_Firewall,          firewallObjGet,          firewallObjSet,         NULL,             NULL,            firewallObjNotify, firewallObjAttrGet, firewallObjAttrSet},
{TR98_FIREWALL_LEVEL,           ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_FirewallLevel,     firewallLevelObjGet,     firewallLevelObjSet,    firewallLevelObjAdd,firewallLevelObjDel,firewallLevelObjNotify, firewallLevelObjAttrGet, firewallLevelObjAttrSet},
{TR98_FIREWALL_CHAIN,           ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_FirewallChain,     firewallChainObjGet,     firewallChainObjSet,    firewallChainObjAdd,firewallChainObjDel,firewallChainObjNotify, firewallChainObjAttrGet, firewallChainObjAttrSet},
{TR98_FIREWALL_CHAIN_RULE,      ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_FirewallChainRule, firewallRuleObjGet,      firewallRuleObjSet,     firewallRuleObjAdd,firewallRuleObjDel,firewallRuleObjNotify, firewallRuleObjAttrGet, firewallRuleObjAttrSet},
{TR98_STD_UPNP, 				ATTR_EXTENDER_HIDDEN,				NULL,					NULL,					 NULL,					NULL,			  NULL, 	   NULL, NULL, NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{TR98_STD_UPNP_DEV, 			ATTR_EXTENDER_HIDDEN,				para_Upnp,				zyExtUPnPObjGet,		 zyExtUPnPObjSet,		NULL,			  NULL, 		   NULL, NULL, NULL},
#endif
#ifdef ZYXEL_IPV6SPEC
{TR98_IPV6SPEC,					0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_PPP,					0,				para_Ipv6Ppp,				zyIpv6PppObjGet, 				NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_PPP_INTF, 			ATTR_INDEXNODE,	para_Ipv6PppIntf,			zyIpv6PppIntfObjGet, 			zyIpv6PppIntfObjSet, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_PPP_INTF_IPV6CP,		0,				para_Ipv6PppIpv6cp,			zyIpv6PppIpv6cpObjGet,			NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_IP, 					0,				para_Ipv6Ip,				zyIpv6IpObjGet,  				zyIpv6IpObjSet, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_IP_INTF, 			ATTR_INDEXNODE,	para_Ipv6IpIntf,			zyIpv6IpIntfObjGet,  			zyIpv6IpIntfObjSet, 			NULL,	NULL,	NULL},
{TR98_IPV6_IP_INTF_IPV6ADDR,	ATTR_INDEX_CREA,para_Ipv6IpIntfIpv6Addr, 	zyIpv6IpIntfIpv6AddrObjGet, 	zyIpv6IpIntfIpv6AddrObjSet,	   	zyIpv6IpIntfIpv6AddrObjAdd, zyIpv6IpIntfIpv6AddrObjDel,			NULL},
{TR98_IPV6_IP_INTF_IPV6PREFIX,	ATTR_INDEX_CREA,para_Ipv6IpIntfIpv6Prefix,	zyIpv6IpIntfIpv6PrefixObjGet, 	zyIpv6IpIntfIpv6PrefixObjSet,   zyIpv6IpIntfIpv6PrefixObjAdd, zyIpv6IpIntfIpv6PrefixObjDel,	NULL},
{TR98_IPV6_ROUTING,				0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_ROUTER,				ATTR_INDEXNODE,	para_Ipv6Router,			zyIpv6RouterObjGet,		NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_ROUTER_IPV6FWD,		ATTR_INDEX_CREA,para_Ipv6RouterIpv6Fwd,		zyIpv6RouterIpv6FwdObjGet,	zyIpv6RouterIpv6FwdObjSet, zyIpv6RouterIpv6FwdObjAdd,	zyIpv6RouterIpv6FwdObjDel,	NULL,	NULL,	NULL},
{TR98_IPV6_ROUTEINFO, 			0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_ROUTEINFO_INTF,		ATTR_INDEXNODE,	para_Ipv6RouteInfoIntf,		zyIpv6RouteInfoIntfObjGet,	NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_ND,					0,				para_Ipv6ND, 				zyIpv6NDObjGet,	zyIpv6NDObjSet, NULL, NULL,	NULL,	NULL,	NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_NEIGHBOR_DISCOVERY_INTFSET
{TR98_IPV6_ND_INTF,				ATTR_INDEXNODE,	para_Ipv6NDIntf,			zyIpv6NDIntfObjGet, zyIpv6NDIntfObjSet, NULL, NULL, NULL,	NULL,	NULL},
#endif
{TR98_IPV6_ROUTERADVER,			0,				para_Ipv6RouterAdver,		zyIpv6RouterAdverObjGet,  NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_ROUTERADVER_INTF,	ATTR_INDEXNODE,	para_Ipv6RouterAdverIntf,	zyIpv6RouterAdverIntfObjGet,  zyIpv6RouterAdverIntfObjSet,	NULL,	NULL,	NULL,	NULL,	NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_RADV_INTFSET_OPT
{TR98_IPV6_ROUTERADVER_OPT,		ATTR_INDEX_CREA,para_Ipv6RouterAdverOpt,	zyIpv6RouterAdverOptObjGet,  zyIpv6RouterAdverOptObjSet,	zyIpv6RouterAdverOptObjAdd,	zyIpv6RouterAdverOptObjDel,	NULL},
#endif
{TR98_IPV6_HOSTS,				0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_HOSTS_HOST, 			ATTR_INDEXNODE, para_HostsHost,				zyIpv6HostsHostObjGet,	 NULL,			NULL,		NULL, 	zyIpv6HostsHostObjNotify, zyIpv6HostsHostObjAttrGet, zyIpv6HostsHostObjAttrSet},
{TR98_IPV6_HOSTS_HOST_IPV6ADDR,	ATTR_INDEXNODE,	para_HostsHostIpv6Addr, 	zyIpv6HostsHostIpv6AddrObjGet,	NULL, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DNS,					0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_DNS_CLIENT, 			0,				NULL,						NULL,					 NULL,					 NULL,				NULL,			NULL},
{TR98_IPV6_DNS_SERVER,			ATTR_INDEX_CREA,para_Ipv6DnsSvr,			zyIpv6DnsSvrObjGet,		zyIpv6DnsSvrObjSet, zyIpv6DnsSvrObjAdd,	zyIpv6DnsSvrObjDel,	NULL,	NULL,	NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_DNS_RELAY
{TR98_IPV6_DNS_RELAY, 			0,				para_Ipv6DnsRly,			zyIpv6DnsRlyObjGet,		zyIpv6DnsRlyObjSet,					 NULL,				NULL,			NULL},
{TR98_IPV6_DNS_RELAYFWD,		ATTR_INDEX_CREA,para_Ipv6DnsRlyFwd,			zyIpv6DnsRlyFwdObjGet, 	zyIpv6DnsRlyFwdObjSet, zyIpv6DnsRlyFwdObjAdd,	zyIpv6DnsRlyFwdObjDel,	NULL,	NULL,	NULL},
#endif
{TR98_IPV6_DHCPV6,				0,				para_Ipv6Dhcpv6,			zyIpv6Dhcpv6ObjGet, 	NULL, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_CLIENT,		ATTR_INDEXNODE,	para_Ipv6Dhcpv6Client,		zyIpv6Dhcpv6ClientObjGet,	zyIpv6Dhcpv6ClientObjSet, NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_CLIENT_SRV,	ATTR_INDEXNODE,	para_Ipv6Dhcpv6CntSrvAddr,	zyIpv6Dhcpv6CntSrvObjGet, 	NULL,    NULL, NULL,			NULL},
{TR98_IPV6_DHCPV6_CLIENT_SENT,	ATTR_INDEX_CREA,para_Ipv6Dhcpv6CntSentAddr,	zyIpv6Dhcpv6CntSentObjGet, 	zyIpv6Dhcpv6CntSentObjSet, zyIpv6Dhcpv6CntSentObjAdd,	zyIpv6Dhcpv6CntSentObjDel,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_CLIENT_RCV,	ATTR_INDEXNODE,	para_Ipv6Dhcpv6CntRrvAddr,	zyIpv6Dhcpv6CntRrvObjGet,	NULL,	 NULL, NULL,			NULL},
{TR98_IPV6_DHCPV6_SERVER,		0,				para_Ipv6Dhcpv6Srv,			zyIpv6Dhcpv6SrvObjGet, 		zyIpv6Dhcpv6SrvObjSet, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_POOL,	ATTR_INDEX_CREA,para_Ipv6Dhcpv6SrvPool,		zyIpv6Dhcpv6SrvPoolObjGet,	zyIpv6Dhcpv6SrvPoolObjSet, zyIpv6Dhcpv6SrvPoolObjAdd, zyIpv6Dhcpv6SrvPoolObjDel,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_CNT,	ATTR_INDEXNODE,			para_Ipv6Dhcpv6SrvCnt, 		zyIpv6Dhcpv6SrvCntObjGet,	zyIpv6Dhcpv6SrvCntObjSet, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_CNT_V6ADDR,	ATTR_INDEXNODE,	para_Ipv6Dhcpv6SrvCntV6Addr, 	zyIpv6Dhcpv6SrvCntV6AddrObjGet,	NULL, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_CNT_V6PREFIX,	ATTR_INDEXNODE,	para_Ipv6Dhcpv6SrvCntV6Prefix,	zyIpv6Dhcpv6SrvCntV6PrefixObjGet,	NULL, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_CNT_OPT,		ATTR_INDEXNODE,	para_Ipv6Dhcpv6SrvCntOpt,	zyIpv6Dhcpv6SrvCntOptObjGet, NULL, NULL, NULL,	NULL,	NULL,	NULL},
{TR98_IPV6_DHCPV6_SERVER_OPT,	ATTR_INDEX_CREA,para_Ipv6Dhcpv6SrvOpt, 		zyIpv6Dhcpv6SrvOptObjGet,	zyIpv6Dhcpv6SrvOptObjSet, zyIpv6Dhcpv6SrvOptObjAdd, zyIpv6Dhcpv6SrvOptObjDel,	NULL,	NULL,	NULL},
#endif
#ifdef ZYXEL_IPV6_MAP
{TR98_IPV6MAP,                  0,              para_MAP,                   zyExtMAPObjGet,             zyExtMAPObjSet,           NULL,                     NULL,                       NULL, NULL, NULL},
{TR98_IPV6MAP_DOMAIN,           ATTR_INDEX_CREA,para_MAPDomain,             zyExtMAPDomainObjGet,       zyExtMAPDomainObjSet,     zyExtMAPDomainObjAdd,     zyExtMAPDomainObjDel,       NULL, NULL, NULL},
{TR98_IPV6MAP_DOMAIN_RULE,      ATTR_INDEX_CREA,para_MAPDomainRule,         zyExtMAPDomainRuleObjGet,   zyExtMAPDomainRuleObjSet, zyExtMAPDomainRuleObjAdd, zyExtMAPDomainRuleObjDel,   NULL, NULL, NULL},
#endif
#ifdef ZYXEL_GRE_SUPPORT
{TR98_XGRE,                     0,              para_XGRE,                  zyXGREObjGet,               NULL,               NULL,               NULL,               NULL,   NULL,   NULL},
{TR98_XGRE_TUNNEL,              ATTR_INDEX_CREA,para_XGRETunnel,            zyXGRETunnelObjGet,         zyXGRETunnelObjSet, zyXGRETunnelObjAdd, zyXGRETunnelObjDel, NULL,   NULL,   NULL},
{TR98_XGRE_TUNNEL_STATS,        0,              para_XGRETunnelStats,       zyXGRETunnelStatsObjGet,    NULL,               NULL,               NULL,               NULL,   NULL,   NULL},
#endif
#ifdef ZYXEL_TAAAG_TR69ALG////MSTC_TTALG, RichardFeng
{TR98_TAAAB_H323Alg,             	0,				para_H323Alg,        			zyTTH323AlgObjGet,        			 zyTTH323AlgObjSet,       			 NULL,	NULL,	zyTTH323AlgObjNotify,	zyTTH323AlgObjAttrGet,	zyTTH323AlgObjAttrSet},
{TR98_TAAAB_SIPAlg,				0,				para_SIPAlg,					zyTTSIPAlgObjGet,					 zyTTSIPAlgObjSet, 				 NULL,	NULL,	zyTTSIPAlgObjNotify,	zyTTSIPAlgObjAttrGet,	zyTTSIPAlgObjAttrSet},
{TR98_TAAAB_RTSPAlg,             	0,				para_RTSPAlg,        			zyTTRTSPAlgObjGet,        			 zyTTRTSPAlgObjSet,       			 NULL,	NULL,	zyTTRTSPAlgObjNotify,	zyTTRTSPAlgObjAttrGet,	zyTTRTSPAlgObjAttrSet},
{TR98_TAAAB_PPTPAlg,             	0,				para_PPTPAlg,        			zyTTPPTPAlgObjGet,        			 zyTTPPTPAlgObjSet,       			 NULL,	NULL,	zyTTPPTPAlgObjNotify,	zyTTPPTPAlgObjAttrGet,	zyTTPPTPAlgObjAttrSet},
{TR98_TAAAB_IPSecAlg,             	0,				para_IPSecAlg,        			zyTTIPSecAlgObjGet,        			 zyTTIPSecAlgObjSet,       			 NULL,	NULL,	zyTTIPSecAlgObjNotify,	zyTTIPSecAlgObjAttrGet,	zyTTIPSecAlgObjAttrSet},
{TR98_TAAAB_L2TPAlg,             	0,				para_L2TPAlg,        			zyTTL2TPAlgObjGet,        			 zyTTL2TPAlgObjSet,       			 NULL,	NULL,	zyTTL2TPAlgObjNotify,	zyTTL2TPAlgObjAttrGet,	zyTTL2TPAlgObjAttrSet},
{TR98_TAAAB_IGMPProxy,				0,				para_IGMPProxy,					zyTTIGMPProxyObjGet,					 zyTTIGMPProxyObjSet, 				 NULL,	NULL,	zyTTIGMPProxyObjNotify,	zyTTIGMPProxyObjAttrGet,	zyTTIGMPProxyObjAttrSet},
{TR98_TAAAB_SMTP,             		0,				para_SMTP,        				zyTTSMTPObjGet,        			 zyTTSMTPObjSet,       			 NULL,	NULL,	zyTTSMTPObjNotify,	zyTTSMTPObjAttrGet,	zyTTSMTPObjAttrSet},
{TR98_TAAAB_ICMP,					0,				para_ICMP,						zyTTICMPObjGet, 				 zyTTICMPObjSet,				 NULL,	NULL,	zyTTICMPObjNotify,	zyTTICMPObjAttrGet, zyTTICMPObjAttrSet},
{TR98_TAAAB_NTP,					0,				para_NTP,						zyTTNTPObjGet, 				 zyTTNTPObjSet,				 NULL,	NULL,	zyTTNTPObjNotify,	zyTTNTPObjAttrGet, zyTTNTPObjAttrSet},
#endif
#ifdef ZYXEL_TAAAG_EDNS//"Add ppp username to edns option for TAAAB"
{TR98_TAAAB_EDNS,             		0,				para_EDNS,        				zyTTEDNSObjGet,        			 zyTTEDNSObjSet,       			 NULL,	NULL,	zyTTEDNSObjNotify,	zyTTEDNSObjAttrGet,	zyTTEDNSObjAttrSet},
#endif
#ifdef CONFIG_TAAAB_PORTMIRROR
{TR98_TAAAB_PORTMIRROR, 		0,				para_TTPORTMIRROR,					zyTTPORTMIRRORObjGet,					 zyTTPORTMIRRORObjSet,					 NULL,	NULL,	zyTTPORTMIRRORObjNotify, zyTTPORTMIRRORObjAttrGet,	zyTTPORTMIRRORObjAttrSet},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_PKT_CAPTURE
{TR98_TAAAB_PACKETCAPTURE, 		0,				para_TTPACKETCAPTURE,					zyTTPACKETCAPTUREObjGet,					 zyTTPACKETCAPTUREObjSet,					 NULL,	NULL,	zyTTPACKETCAPTUREObjNotify, zyTTPACKETCAPTUREObjAttrGet,	zyTTPACKETCAPTUREObjAttrSet},
{TR98_TAAAB_PACKETCAPTUREUPLOAD, 		0,				para_TTPACKETCAPTUREUPLOAD,					zyTTPACKETCAPTUREUPLOADObjGet,					 zyTTPACKETCAPTUREUPLOADObjSet,					 NULL,	NULL,	zyTTPACKETCAPTUREUPLOADObjNotify, zyTTPACKETCAPTUREUPLOADObjAttrGet,	zyTTPACKETCAPTUREUPLOADObjAttrSet},
#endif
#if ZYXEL_EXT
{TR98_ZYXEL_EXT,                0,              para_extend,            zyExtObjGet,             zyExtObjSet, NULL, NULL, NULL, NULL, NULL},
#ifdef ZYXEL_LANDING_PAGE_FEATURE
{TR98_LANDING_PAGE, 			0,				para_Landing_Page,		zyExtLandingPageObjGet,	 zyExtLandingPageObjSet,	NULL,	NULL,	NULL,	NULL,	NULL},
#endif
#if defined(ZyXEL_Online_Firmware_Upgrade) || defined(ZYXEL_WEB_GUI_ONLINE_FW_UPGRADE_VIA_URL)
{TR98_ONLINE_FWUPGRADE, 			0,				para_Online_Firmware_Upgrade,		zyExtOnlineFWUgradeObjGet,	 zyExtOnlineFWUgradeObjSet,	NULL,	NULL,	NULL,	NULL,	NULL},
#endif
#ifdef ABUU_CUSTOMIZATION_WLAN_SCHEDULER
{TR98_WlanScheduler,                0,              para_WlanScheduler,          zyExtWlanSchedulerObjGet,         	zyExtWlanSchedulerObjSet,        NULL,             NULL,            zyExtWlanSchedulerObjNotify, zyExtWlanSchedulerObjAttrGet, zyExtWlanSchedulerObjAttrSet},
{TR98_WlanScheduler_Rule,           ATTR_INDEX_CREA,para_WlanSchedulerRule,      zyExtWlanSchedulerRuleObjGet,		zyExtWlanSchedulerRuleObjSet,    zyExtWlanSchedulerRuleObjAdd, zyExtWlanSchedulerRuleObjDel, NULL, NULL, NULL},
#endif
{TR98_FEATURE_FLAG,				0,              para_Feature_Flag,		zyExtFeatureFlagObjGet,	 zyExtFeatureFlagObjSet,	NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_DNS,						ATTR_EXTENDER_HIDDEN,				para_Dns,				zyExtDnsObjGet,			 zyExtDnsObjSet, NULL, NULL, NULL, NULL, NULL},
{TR98_DNS_RT_ENTRY,             ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DnsRtEntry,        zyExtDnsRtObjGet,        zyExtDnsRtObjSet,       zyExtDnsRtObjAdd, zyExtDnsRtObjDel,zyExtDnsRtObjNotify, NULL, zyExtDnsRtObjAttrSet},
//{TR98_DNS_ENTRY, 	            ATTR_INDEX_CREA,para_DnsEntry, 		    zyExtDnsEntryObjGet,     zyExtDnsEntryObjSet,    zyExtDnsEntryObjAdd, zyExtDnsEntryObjDel,zyExtDnsEntryObjNotify, zyExtDnsEntryObjAttrGet, zyExtDnsEntryObjAttrSet},
{TR98_DNS_ENTRY, 	            ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_DnsEntry, 		    zyExtDnsEntryObjGet,     zyExtDnsEntryObjSet,    zyExtDnsEntryObjAdd, zyExtDnsEntryObjDel, NULL, NULL, NULL},
{TR98_D_DNS,                    ATTR_EXTENDER_HIDDEN,              para_DDns,              zyExtDDnsObjGet,         zyExtDDnsObjSet,        NULL,             NULL,            zyExtDDnsObjNotify, zyExtDDnsObjAttrGet, zyExtDDnsObjAttrSet},
#ifdef ZCFG_TR64
{TR98_TR064, 	                0,              para_Tr064,	            zyExtTr064ObjGet,	     zyExtTr064ObjSet,	     NULL,             NULL,            zyExtTr064ObjNotify, NULL, zyExtTr064ObjAttrSet},
#else
#endif
//{TR98_VLAN_GROUP,               ATTR_INDEX_CREA,para_VlanGroup,         zyExtVlanGroupObjGet,	 zyExtVlanGroupObjSet,	 zyExtVlanGroupObjAdd, zyExtVlanGroupObjDel, zyExtVlanGroupObjNotify, zyExtVlanGroupObjAttrGet, zyExtVlanGroupObjAttrSet},
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_VLAN_GROUP
{TR98_VLAN_GROUP,               ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_VlanGroup,         zyExtVlanGroupObjGet,	 zyExtVlanGroupObjSet,	 zyExtVlanGroupObjAdd, zyExtVlanGroupObjDel, NULL, NULL, NULL},
#endif
{TR98_EMAIL_NOTIFY,             ATTR_EXTENDER_HIDDEN,              NULL,       			NULL,  					 NULL, 					 NULL,             NULL,            NULL, NULL, NULL},
//{TR98_EMAIL_SERVICE,            ATTR_INDEX_CREA,para_EmailSrv,	        zyExtEmailSrvObjGet,     zyExtEmailSrvObjSet,	 zyExtEmailSrvObjAdd, zyExtEmailSrvObjDel,  zyExtEmailSrvObjNotify, zyExtEmailSrvObjAttrGet, zyExtEmailSrvObjAttrSet},
{TR98_EMAIL_SERVICE,            ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_EmailSrv,	        zyExtEmailSrvObjGet,     zyExtEmailSrvObjSet,	 zyExtEmailSrvObjAdd, zyExtEmailSrvObjDel, NULL, NULL, NULL},
//{TR98_EMAIL_EVENT_CFG,          ATTR_INDEX_CREA,para_EmailEvent,	    zyExtEmailEventObjGet,   zyExtEmailEventObjSet,	 zyExtEmailEventObjAdd, zyExtEmailEventObjDel, zyExtEmailEventObjNotify, zyExtEmailEventObjAttrGet, zyExtEmailEventObjAttrSet},
{TR98_EMAIL_EVENT_CFG,          ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_EmailEvent,	    zyExtEmailEventObjGet,   zyExtEmailEventObjSet,	 zyExtEmailEventObjAdd, zyExtEmailEventObjDel, NULL, NULL, NULL},
{TR98_SYS_INFO,                 0,              para_SysInfo,           zyExtSysInfoObjGet,      zyExtSysInfoObjSet,     NULL,             NULL,            zyExtSysInfoObjNotify, zyExtSysInfoObjAttrGet, zyExtSysInfoObjAttrSet},
#ifdef CONFIG_ZCFG_BE_MODULE_GPON_INFO
{TR98_GPON_INFO,				0, 				para_GponInfo,			zyExtGponInfoObjGet,		 NULL,					 NULL,			   NULL,			NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SNMP
//{TR98_SNMP, 	                0,              para_Snmp,	            zyExtSnmpObjGet, 	     zyExtSnmpObjSet,     NULL,             NULL,            zyExtSnmpObjNotify, zyExtSnmpObjAttrGet, zyExtSnmpObjAttrSet},
{TR98_SNMP, 	                ATTR_EXTENDER_HIDDEN,              para_Snmp,	            zyExtSnmpObjGet, 	     zyExtSnmpObjSet,     NULL,             NULL,            NULL, NULL, NULL},
{TR98_SNMP_TRAP,                ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_SnmpTrap,          zyExtSnmpTrapObjGet,     zyExtSnmpTrapObjSet,    zyExtSnmpTrapObjAdd,	zyExtSnmpTrapObjDel, NULL, NULL, NULL},
#endif
//{TR98_SCHEDULE,                 ATTR_INDEX_CREA,para_Schedule,          scheduleObjGet,          scheduleObjSet,         scheduleObjAdd,   scheduleObjDel,  scheduleObjNotify, scheduleObjAttrGet, scheduleObjAttrSet},
{TR98_SCHEDULE,                 ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_Schedule,          scheduleObjGet,          scheduleObjSet,         scheduleObjAdd,   scheduleObjDel,  NULL, NULL, NULL},
#if defined(IAAAA_CUSTOMIZATION) || defined(ZYXEL_SP_DOMAIN_TIMER)
{TR98_REMO_MGMT,		0,				para_RemoMgmt,			zyExtRemoteMgmtObjGet,			 zyExtRemoteMgmtObjSet,					 NULL,				NULL,			NULL,		NULL,		NULL},
#else
{TR98_REMO_MGMT,				0,				NULL,					NULL,					 NULL,					 NULL,				NULL,			NULL},
#endif
{TR98_REMO_SRV, 			    ATTR_INDEX_CREA ,para_RemoSrv,		    zyExtRemoteMgmtSrvObjGet,		 zyExtRemoteMgmtSrvObjSet,		 zyExtRemoteMgmtSrvObjAdd, zyExtRemoteMgmtSrvObjDel, zyExtRemoteMgmtSrvObjNotify, zyExtRemoteMgmtSrvObjAttrGet, zyExtRemoteMgmtSrvObjAttrSet},
{TR98_SP_REMO_SRV, 			    ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_SpRemoSrv,		    zyExtSpRemoteMgmtSrvObjGet,		 zyExtSpRemoteMgmtSrvObjSet,	zyExtSpRemoteMgmtSrvObjAdd, zyExtSpRemoteMgmtSrvObjDelete, NULL, NULL, NULL},
{TR98_REMO_TRUSTDOMAIN,         ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_RemoTrustDomain,   zyExtRemoteMgmtTrustDomainObjGet,                    zyExtRemoteMgmtTrustDomainObjSet,                   zyExtRemoteMgmtTrustDomainObjAdd,            zyExtRemoteMgmtTrustDomainObjDel,            NULL},
{TR98_SP_TRUSTDOMAIN,           ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_SpTrustDomain,   zyExtSpRemoteMgmtTrustDomainObjGet,                    zyExtSpRemoteMgmtTrustDomainObjSet,                   zyExtSpRemoteMgmtTrustDomainObjAdd,            zyExtSpRemoteMgmtTrustDomainObjDel,            NULL},
#ifdef ABUU_CUSTOMIZATION
{TR98_REMO_IFACETRUSTDOMAIN,	ATTR_INDEX_CREA,para_IfaceTrustDomain,		zyExtIfaceTrustDomainObjGet,		zyExtIfaceTrustDomainObjSet,		zyExtIfaceTrustDomainObjAdd, 		zyExtIfaceTrustDomainObjDel, NULL, NULL, NULL},
{TR98_REMO_IFACETRUSTDOMAININFO,ATTR_INDEX_CREA,para_IfaceTrustDomainInfo,	zyExtIfaceTrustDomainInfoObjGet,	zyExtIfaceTrustDomainInfoObjSet,	zyExtIfaceTrustDomainInfoObjAdd, 	zyExtIfaceTrustDomainInfoObjDel, NULL, NULL, NULL},
#endif
{TR98_IEEE8021AG,               ATTR_EXTENDER_HIDDEN,              para_8021ag,	        zyExt8021agObjGet, 	     zyExt8021agObjSet,		NULL,             NULL,            zyExt8021agObjNotify, zyExt8021agObjAttrGet, zyExt8021agObjAttrSet},
#ifdef ZYXEL_ETHOAM_TMSCTL
{TR98_IEEE8023AH,				0,				para_8023ah,			zyExt8023ahObjGet,		 zyExt8023ahObjSet, 	NULL,			  NULL, 		   zyExt8023ahObjNotify, zyExt8023ahObjAttrGet, zyExt8023ahObjAttrSet},
#endif
//{TR98_SAMBA,         		    0,              para_Samba,	   		    zyExtSambaObjGet, 	     zyExtSambaObjSet,		NULL,             NULL,            zyExtSambaObjNotify, zyExtSambaObjAttrGet, zyExtSambaObjAttrSet},
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{TR98_SAMBA,         		    ATTR_EXTENDER_HIDDEN,              para_Samba,	   		    zyExtSambaObjGet, 	     zyExtSambaObjSet,		NULL,             NULL,            NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_UPNP
{TR98_UPNP,         		    ATTR_EXTENDER_HIDDEN,              para_Upnp,	   		    zyExtUPnPObjGet, 	     zyExtUPnPObjSet,		NULL,             NULL,            NULL, NULL, NULL},
#endif
{TR98_IGMP,                     ATTR_EXTENDER_HIDDEN,              para_Igmp,              zyExtIgmpObjGet,         zyExtIgmpObjSet,        NULL,             NULL,            zyExtIgmpObjNotify, NULL, NULL},
{TR98_MLD,                      ATTR_EXTENDER_HIDDEN,              para_Mld,               zyExtMldObjGet,          zyExtMldObjSet,         NULL,             NULL,            zyExtMldObjNotify,  NULL, NULL},
#ifdef ZYXEL_TR69_DATA_USAGE
{TR98_DATA_USAGE,         		    0,              para_DataUsage,	   		    zyExtDataUsageObjGet, 	     zyExtDataUsageObjSet,		NULL,             NULL,            NULL, NULL, NULL},
{TR98_DATA_USAGE_LAN,                           ATTR_INDEX_CREA,              para_DataUsageLan,                         zyExtDataUsageLanObjGet,            NULL,              NULL,             NULL,            NULL, NULL, NULL},
{TR98_DATA_USAGE_WAN,                           ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,              para_DataUsageWan,                         zyExtDataUsageWanObjGet,            NULL,              NULL,             NULL,            NULL, NULL, NULL},
#endif
#if defined(SAAAD_RANDOM_PASSWORD) || defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
#ifdef ABUU_CUSTOMIZATION
{TR98_LOG_CFG,                  0,              para_LogCfg,              zyExtLogCfgObjGet,         zyExtLogCfgObjSet,        NULL,             NULL,            NULL, NULL, NULL},
#else
{TR98_LOG_CFG,                  0,              NULL,              NULL,         NULL,        NULL,             NULL,            NULL, NULL, NULL},
#endif
{TR98_LOG_CFG_GP,               ATTR_INDEX_CREA,para_LogCfgGp,          zyExtLogCfgGpObjGet,       zyExtLogCfgGpObjSet,        NULL,             NULL,            NULL,   NULL,    NULL},
#if defined(SAAAE_TR69_LOGINACCOUNT) || defined(ZYXEL_SUPPORT_TR98_LOGIN_ACCOUNT)
{TR98_LOG_CFG_GP_ACCOUNT,       ATTR_INDEX_CREA,para_LogCfgGpAccount,   zyExtLogCfgGpAccountObjGet,		zyExtLogCfgGpAccountObjSet,        zyExtLogCfgGpAccountObjAdd,             zyExtLogCfgGpAccountObjDel,            NULL, NULL, NULL},
#else
{TR98_LOG_CFG_GP_ACCOUNT,       ATTR_INDEX_CREA,para_LogCfgGpAccount,   zyExtLogCfgGpAccountObjGet,		zyExtLogCfgGpAccountObjSet,        zyExtLogCfgGpAccountObjAdd,             NULL,            NULL, NULL, NULL},
#endif
#endif
{TR98_LOG_SETTING,              ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_LogSetting,        zyExtLogSettingObjGet,			zyExtLogSettingObjSet,		NULL,             NULL,            zyExtLogSettingObjNotify, zyExtLogSettingObjAttrGet, zyExtLogSettingObjAttrSet},
{TR98_LOG_CLASSIFY,				ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_LogClassify,		zyExtLogClassifyObjGet,			NULL, 						zyExtLogClassifyObjAdd,	zyExtLogClassifyObjDel,	zyExtLogClassifyObjNotify, zyExtLogClassifyObjAttrGet, zyExtLogClassifyObjAttrSet},
{TR98_LOG_CATEGORY,             ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_LogCategory,       zyExtLogCategoryObjGet,         zyExtLogCategoryObjSet,		zyExtLogCategoryObjAdd,	zyExtLogCategoryObjDel,	zyExtLogCategoryObjNotify, zyExtLogCategoryObjAttrGet, zyExtLogCategoryObjAttrSet},
{TR98_MACFILTER,		    	ATTR_EXTENDER_HIDDEN, 				para_MacFilter,			zyExtMacFilterObjGet,			zyExtMacFilterObjSet,		NULL,             NULL,            zyExtMacFilterObjNotify, zyExtMacFilterObjAttrGet, zyExtMacFilterObjAttrSet},
{TR98_MACFILTER_WHITELIST,		ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_MacFilter_WhiteList, zyExtMacFilterWhiteListObjGet,zyExtMacFilterWhiteListObjSet, zyExtMacFilterWhiteListObjAdd, zyExtMacFilterWhiteListObjDel, zyExtMacFilterWhiteListObjNotify, zyExtMacFilterWhiteListObjAttrGet, zyExtMacFilterWhiteListObjAttrSet},
{TR98_PAREN_CTL,                ATTR_EXTENDER_HIDDEN,              para_ParenCtl,          zyExtParenCtlObjGet,         	zyExtParenCtlObjSet,        NULL,             NULL,            zyExtParenCtlObjNotify, zyExtParenCtlObjAttrGet, zyExtParenCtlObjAttrSet},
//{TR98_PAREN_CTL_PROF,           ATTR_INDEX_CREA,para_ParenCtlProf,      zyExtParenCtlProfObjGet,		zyExtParenCtlProfObjSet,    zyExtParenCtlProfObjAdd, zyExtParenCtlProfObjDel, zyExtParenCtlProfObjNotify, zyExtParenCtlProfObjAttrGet, zyExtParenCtlProfObjAttrSet},
{TR98_PAREN_CTL_PROF,           ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_ParenCtlProf,      zyExtParenCtlProfObjGet,		zyExtParenCtlProfObjSet,    zyExtParenCtlProfObjAdd, zyExtParenCtlProfObjDel, NULL, NULL, NULL},
{TR98_SEC,    		            ATTR_EXTENDER_HIDDEN,              para_Sec,    		    zyExtSecObjGet,         		NULL,     			  		NULL,             NULL,            zyExtSecObjNotify, zyExtSecObjAttrGet, zyExtSecObjAttrSet},
//{TR98_SEC_CERT,  		        ATTR_INDEX_CREA,para_SecCert, 		    zyExtSecCertObjGet,				zyExtSecCertObjSet,  		zyExtSecCertObjAdd, zyExtSecCertObjDel, zyExtSecCertObjNotify, zyExtSecCertObjAttrGet, zyExtSecCertObjAttrSet},
{TR98_SEC_CERT,  		        ATTR_INDEX_CREA | ATTR_EXTENDER_HIDDEN,para_SecCert, 		    zyExtSecCertObjGet,				zyExtSecCertObjSet,  		zyExtSecCertObjAdd, zyExtSecCertObjDel, NULL, NULL, NULL},
{TR98_GUI_CUSTOMIZATION,		0,				para_GUI_Customization, zyExtCustomizationObjGet,	 	zyExtCustomizationObjSet,	NULL,	NULL,	NULL,	NULL,	NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
{TR98_DSL,                      0,              para_DSL,				zyExtDSLGet,					zyExtDSLSet,				NULL,	NULL,	NULL,	NULL,	NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
{TR98_ONECONNECT,	0, 				para_OneConnect,		zyExtOneConnectObjGet,		 zyExtOneConnectObjSet,		   NULL,             NULL,            NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_SPEED_TEST
{TR98_ONECONNECT_SPEEDTESTINFO,	0, 				para_SpeedTestInfo,		zyExtSpeedTestInfoObjGet,		 zyExtSpeedTestInfoObjSet,		   NULL,             NULL,            NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
{TR98_AP_STEERING,	0, 				para_ApSteering,		zyExtApSteeringObjGet,		 zyExtApSteeringObjSet,		   NULL,             NULL,            NULL},
#endif // CONFIG_ZCFG_BE_MODULE_ZYXEL_AP_STEERING
#ifdef CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
{TR98_PROXIMITY_SENSOR,	0, 	para_ProximitySensor,	zyExtProximitySensorObjGet,		zyExtProximitySensorObjSet,		   NULL,             NULL,            NULL},
#endif // CONFIG_ZCFG_BE_MODULE_PROXIMITY_SENSOR
#ifdef ZYXEL_BANDWIDTH_IMPROVEMENT
{TR98_BANDWIDTH_IMPROVEMENT,	0, 	para_BandwidthImprovement,	zyExtBandwidthImprovementObjGet,		zyExtBandwidthImprovementObjSet,		   NULL,             NULL,            NULL},
#endif //ZYXEL_BANDWIDTH_IMPROVEMENT
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_WLANCONFIG, ATTR_INDEX_CREA, 	para_WLANConfiguration,	zyExtWLANConfigurationObjGet,	zyExtWLANConfigurationObjSet, NULL, NULL, NULL},
{TR98_AIRTIMEFAIRNESS,          0,  para_AirtimeFairness,   zyExtAirtimeFairnessObjGet,     zyExtAirtimeFairnessObjSet,   NULL, NULL, NULL},
#endif
{TR98_ACCOUNT_RESETPASSWD, ATTR_INDEX_CREA, para_AccountResetPassword, zyExtAccountResetPasswordObjGet, zyExtAccountResetPasswordObjSet, NULL, NULL, NULL},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#if OBSOLETED
#else
//{TR98_WAN_DSL_CONN_MGMT,        0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
//{TR98_WAN_DSL_CONN_SRVC,        ATTR_INDEXNODE, NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIG
{TR98_PRINT_SERVER,             0,              para_Ipp_PrintServer,   zyExtIppPrintServObjGet,          zyExtIppPrintServObjSet,         NULL,             NULL,            NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_IPK_PACKAGE
{TR98_IPK_PKG,           ATTR_INDEXNODE,              para_Ipk_Package,                zyExtIpkPackageGet,          zyExtIpkPackageSet,                   NULL,             NULL,            NULL, NULL, NULL},
{TR98_IPK_PKG_INSTALL,   ATTR_INDEXNODE,              para_Ipk_Package_Install,        zyExtIpkPackageInstallGet,   zyExtIpkPackageInstallSet,                   NULL,             NULL,            NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_WIFI
{TR98_WiFiCommunity,     0,             para_WiFiCommunity, zyExtWiFiCommunityObjGet, zyExtWiFiCommunityObjSet, NULL, NULL, NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_EASYMESH
{TR98_EasyMesh,       0, para_EasyMesh,       zyExtEasyMeshObjGet, zyExtEasyMeshObjSet, NULL, NULL, NULL, NULL, NULL},
#ifdef ZYXEL_WIND_ITALY_CUSTOMIZATION
{TR98_VendorEasyMesh, 0, para_VendorEasyMesh, zyExtEasyMeshObjGet, zyExtEasyMeshObjSet, NULL, NULL, NULL, NULL, NULL},
#endif //ZYXEL_WIND_ITALY_CUSTOMIZATION
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONESSID
{TR98_ONESSID,        0, para_OneSsid,        zyExtOneSsidObjGet, zyExtOneSsidObjSet,   NULL, NULL, NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_SFP_POWER_MGMT
{TR98_SFP_MGMT,					0,				para_SfpMgmt,			zyExtSfpMgmObjGet,				zyExtSfpMgmObjSet,			NULL,			NULL,		NULL,	NULL,	NULL},
#endif
#ifdef ZYXEL_WIFI_HW_BUTTON_SWITCH
{TR98_TAAAB_Configuration_Button,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_Button_WiFi,				0,				para_HwWiFiButton,					zyExtHwWiFiButtonObjGet, 				 zyExtHwWiFiButtonObjSet,				 NULL,	NULL,	zyExtHwWiFiButtonObjNotify,	zyExtHwWiFiButtonObjAttrGet, zyExtHwWiFiButtonObjAttrSet},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_QUANTWIFI
{TR98_TAAAB_Configuration_Quantwifi,				0,				para_Quantwifi,					zyExtQuantwifiObjGet, 				 zyExtQuantwifiObjSet,				 NULL,	NULL,	zyExtQuantwifiObjNotify,	zyExtQuantwifiObjAttrGet, zyExtQuantwifiObjAttrSet},
#endif
#ifdef ZYXEL_XMPP
/* TR98 Full Path Name          Attribute       Parameter List          Get func                 Set func                Add func          Del func         Notify func Get Attr func       Set Attr func*/
{TR98_XMPP,                     0,              para_Xmpp,              xmppObjGet,               NULL,                     NULL,           NULL,           NULL},
{TR98_XMPP_CONNECTION,          ATTR_INDEXNODE,              para_XmppConn,          xmppObjGet,               xmppObjSet,               NULL,           NULL,           NULL},
{TR98_XMPP_CONNECTION_SERVER,   ATTR_INDEXNODE,              para_XmppConnSrv,       xmppConnSrvObjGet,        xmppConnSrvObjSet,        NULL,           NULL,           NULL},
#endif
#ifdef MSTC_TAAAG_MULTI_USER
{TR98_Nat_DMZ, 			0,				para_NatDMZOpt, 				zyNatDMZObjGet,				 zyNatDMZObjSet,				 NULL,	NULL,	NULL, NULL, NULL},
#endif
#if defined(ZYXEL_IGMPDIAG) || defined(ZYXEL_CUSTOMIZATION_SYSLOG)
{TR98_TAAAB_Function, 			0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
#endif
#ifdef ZYXEL_IGMPDIAG
{TR98_TTIGMPDiagnostics,             	0,				para_IGMPDiagnostics,        			zyTTIGMPDiagnosticsObjGet,        			 zyTTIGMPDiagnosticsObjSet,       			 NULL,	NULL,	zyTTIGMPDiagnosticsObjNotify,	zyTTIGMPDiagnosticsObjAttrGet,	zyTTIGMPDiagnosticsObjAttrSet},
#endif
#ifdef ZYXEL_TAAAB_WAN_CONN_DIAG
{TR98_TAAAB_WanConnDiagnostics,             	0,				para_TTWanConnDiagnostics,        			zyTTWanConnDiagObjGet,        			 zyTTWanConnDiagObjSet,       			 NULL,	NULL,	zyTTWanConnDiagObjNotify,	zyTTWanConnDiagObjAttrGet,	zyTTWanConnDiagObjAttrSet},
#endif
#ifdef ZYXEL_CUSTOMIZATION_SYSLOG
{TR98_TAAAB_Syslog,             	0,				para_Syslog,        			zyTTSyslogObjGet,        			 zyTTSyslogObjSet,       			 NULL,	NULL,	zyTTSyslogObjNotify,	zyTTSyslogObjAttrGet,	zyTTSyslogObjAttrSet},
{TR98_TAAAB_SyslogUpload,				0,				para_SyslogUpload,					zyTTSyslogUploadObjGet,					 zyTTSyslogUploadObjSet, 				 NULL,	NULL,	zyTTSyslogUploadObjNotify,	zyTTSyslogUploadObjAttrGet,	zyTTSyslogUploadObjAttrSet},
#endif
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER) || defined(TENDER_TAAAB_FEATURE)
{TR98_TAAAB,             	0,				NULL,        			NULL,        			 NULL,       			 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Users,				0,				para_Users,					zyTTUsersObjGet,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Users_User,             	ATTR_INDEX_CREA,				para_User,        			zyTTUserObjGet,        			 zyTTUserObjSet,       			 zyTTUserObjAdd,	zyTTUserObjDel,	zyTTUserObjNotify,	zyTTUserObjAttrGet,	zyTTUserObjAttrSet},
{TR98_TAAAB_UserInterface,				0,				para_UserInterface,					zyTTUserInterfaceObjGet, 				 zyTTUserInterfaceObjSet,				 NULL,	NULL,	zyTTUserInterfaceObjNotify,	zyTTUserInterfaceObjAttrGet, zyTTUserInterfaceObjAttrSet},
{TR98_TAAAB_UserInterface_RemoteAccess, 			0,				para_RemoteLocalAccess, 				zyTTRemoteAccessObjGet,				 zyTTRemoteAccessObjSet,				 NULL,	NULL,	zyTTRemoteAccessObjNotify, zyTTRemoteAccessObjAttrGet, zyTTRemoteAccessObjAttrSet},
{TR98_TAAAB_UserInterface_LocalAccess, 			0,				para_RemoteLocalAccess, 				zyTTLocalAccessObjGet,				 zyTTLocalAccessObjSet,				 NULL,	NULL,	zyTTLocalAccessObjNotify, zyTTLocalAccessObjAttrGet, zyTTLocalAccessObjAttrSet},
{TR98_TAAAB_Firewall, 			0,				para_TTFirewall, 				zyTTFirewallObjGet,				 zyTTFirewallObjSet,				 NULL,	NULL,	zyTTFirewallObjNotify, zyTTFirewallObjAttrGet, zyTTFirewallObjAttrSet},
{TR98_TAAAB_UPnP, 			0,				NULL,				NULL,				 NULL,				 NULL,	NULL,	NULL, NULL, NULL},
{TR98_TAAAB_UPnP_Device, 			0,				para_UPnPDevice, 				zyTTUPnPDeviceObjGet,				 zyTTUPnPDeviceObjSet,				 NULL,	NULL,	zyTTUPnPDeviceObjNotify, zyTTUPnPDeviceObjAttrGet, zyTTUPnPDeviceObjAttrSet},
{TR98_TAAAB_Configuration,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Broad, 			0,				para_GUIAccessRights, 				zyTTBroadbandObjGet,				 zyTTBroadbandObjSet,				 NULL,	NULL,	zyTTBroadbandObjNotify, zyTTBroadbandObjAttrGet, zyTTBroadbandObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetDSL,				0,				para_GUIAccessRights,					zyTTInternetDSLObjGet, 				 zyTTInternetDSLObjSet,				 NULL,	NULL,	zyTTInternetDSLObjNotify,	zyTTInternetDSLObjAttrGet, zyTTInternetDSLObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_InetWAN,				0,				para_GUIAccessRights,					zyTTInternetWANObjGet, 				 zyTTInternetWANObjSet,				 NULL,	NULL,	zyTTInternetWANObjNotify,	zyTTInternetWANObjAttrGet, zyTTInternetWANObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVDSL,				0,				para_GUIAccessRights,					zyTTIPTVDSLObjGet, 				 zyTTIPTVDSLObjSet,				 NULL,	NULL,	zyTTIPTVDSLObjNotify,	zyTTIPTVDSLObjAttrGet, zyTTIPTVDSLObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_IPTVWAN,				0,				para_GUIAccessRights,					zyTTIPTVWANObjGet, 				 zyTTIPTVWANObjSet,				 NULL,	NULL,	zyTTIPTVWANObjNotify,	zyTTIPTVWANObjAttrGet, zyTTIPTVWANObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_MamgWAN,				0,				para_GUIAccessRights,					zyTTMamgWANObjGet, 				 zyTTMamgWANObjSet,				 NULL,	NULL,	zyTTMamgWANObjNotify,	zyTTMamgWANObjAttrGet, zyTTMamgWANObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSDSL,				0,				para_GUIAccessRights,					zyTTACSDSLObjGet,				 zyTTACSDSLObjSet, 			 NULL,	NULL,	zyTTACSDSLObjNotify,	zyTTACSDSLObjAttrGet, zyTTACSDSLObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_ACSWAN,				0,				para_GUIAccessRights,					zyTTACSWANObjGet,				 zyTTACSWANObjSet, 			 NULL,	NULL,	zyTTACSWANObjNotify,	zyTTACSWANObjAttrGet, zyTTACSWANObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Broadband_Advanced,				0,				para_GUIAccessRights,					zyTTAdvancedObjGet,				 zyTTAdvancedObjSet, 			 NULL,	NULL,	zyTTAdvancedObjNotify,	zyTTAdvancedObjAttrGet, zyTTAdvancedObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_General,			0,				para_GUIAccessRights,				zyTTGeneralObjGet,				 zyTTGeneralObjSet,				 NULL,	NULL,	zyTTGeneralObjNotify, zyTTGeneralObjAttrGet, zyTTGeneralObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_MoreAP,			0,				para_GUIAccessRights,				zyTTMoreAPObjGet,				 zyTTMoreAPObjSet, 			 NULL,	NULL,	zyTTMoreAPObjNotify, zyTTMoreAPObjAttrGet, zyTTMoreAPObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WPS,			0,				para_GUIAccessRights,				zyTTWPSObjGet,				 zyTTWPSObjSet, 			 NULL,	NULL,	zyTTWPSObjNotify, zyTTWPSObjAttrGet, zyTTWPSObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_WMM,			0,				para_GUIAccessRights,				zyTTWMMObjGet,				 zyTTWMMObjSet, 			 NULL,	NULL,	zyTTWMMObjNotify, zyTTWMMObjAttrGet, zyTTWMMObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Wireless_Others,			0,				para_GUIAccessRights,				zyTTOthersObjGet,				 zyTTOthersObjSet, 			 NULL,	NULL,	zyTTOthersObjNotify, zyTTOthersObjAttrGet, zyTTOthersObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNetworking,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANSetup,			0,				para_GUIAccessRights,				zyTTLANSetupObjGet,				 zyTTLANSetupObjSet, 			 NULL,	NULL,	zyTTLANSetupObjNotify, zyTTLANSetupObjAttrGet, zyTTLANSetupObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_HomeNking_LANVLAN, 		0,				para_GUIAccessRights,				zyTTLANVLANObjGet,				 zyTTLANVLANObjSet, 			 NULL,	NULL,	zyTTLANVLANObjNotify, zyTTLANVLANObjAttrGet, zyTTLANVLANObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_Routing, 		0,				para_GUIAccessRights,				zyTTRoutingObjGet, 			 zyTTRoutingObjSet,			 NULL,	NULL,	zyTTRoutingObjNotify, zyTTRoutingObjAttrGet, zyTTRoutingObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT,			0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_NetworkSetting_NAT_ALG, 		0,				para_GUIAccessRights,				zyTTALGObjGet, 			 zyTTALGObjSet,			 NULL,	NULL,	zyTTALGObjNotify, zyTTALGObjAttrGet, zyTTALGObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Security_Firewall_DoS,		0,				para_GUIAccessRights,				zyTTDoSObjGet,			 zyTTDoSObjSet, 		 NULL,	NULL,	zyTTDoSObjNotify, zyTTDoSObjAttrGet, zyTTDoSObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_SystemMonitor_Log, 	0,				para_GUIAccessRights,				zyTTLogObjGet,			 zyTTLogObjSet, 		 NULL,	NULL,	zyTTLogObjNotify, zyTTLogObjAttrGet, zyTTLogObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_RemoteMGMT, 	0,				para_GUIAccessRights,				zyTTRemoteMGMTObjGet,			 zyTTRemoteMGMTObjSet, 		 NULL,	NULL,	zyTTRemoteMGMTObjNotify, zyTTRemoteMGMTObjAttrGet, zyTTRemoteMGMTObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_TR069, 	0,				para_GUIAccessRights,				zyTTTR069ObjGet,			 zyTTTR069ObjSet, 		 NULL,	NULL,	zyTTTR069ObjNotify, zyTTTR069ObjAttrGet, zyTTTR069ObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Time, 	0,				para_GUIAccessRights,				zyTTTimeObjGet,			 zyTTTimeObjSet, 		 NULL,	NULL,	zyTTTimeObjNotify, zyTTTimeObjAttrGet, zyTTTimeObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_LogSetting, 	0,				para_GUIAccessRights,				zyTTLogSettingObjGet,			 zyTTLogSettingObjSet, 		 NULL,	NULL,	zyTTLogSettingObjNotify, zyTTLogSettingObjAttrGet, zyTTLogSettingObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_FirmwareUpgrade, 	0,				para_GUIAccessRights,				zyTTFirmwareUpgradeObjGet,			 zyTTFirmwareUpgradeObjSet, 		 NULL,	NULL,	zyTTFirmwareUpgradeObjNotify, zyTTFirmwareUpgradeObjAttrGet, zyTTFirmwareUpgradeObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_Configuration, 	0,				para_GUIAccessRights,				zyTTConfigurationObjGet,			 zyTTConfigurationObjSet, 		 NULL,	NULL,	zyTTConfigurationObjNotify, zyTTConfigurationObjAttrGet, zyTTConfigurationObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Maintenance_EDNS, 	0,				para_GUIAccessRights,				zyTTEDNSPageObjGet,			 zyTTEDNSPageObjSet,		 NULL,	NULL,	zyTTEDNSPageObjNotify, zyTTEDNSPageObjAttrGet, zyTTEDNSPageObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Engdebug, 	0,				para_GUIAccessRights,				zyTTEngdebugObjGet,			 zyTTEngdebugObjSet,		 NULL,	NULL,	zyTTEngdebugObjNotify, zyTTEngdebugObjAttrGet, zyTTEngdebugObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Capture,	0,				para_GUIAccessRights,				zyTTCaptureObjGet,			 zyTTCaptureObjSet,		 NULL,	NULL,	zyTTCaptureObjNotify, zyTTCaptureObjAttrGet, zyTTCaptureObjAttrSet},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu,				0,				NULL,					NULL,					 NULL,					 NULL,	NULL,	NULL,	NULL,	NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_RemoteAccess,	0,				para_GUIAccessRights,				zyTTEMRemoteAccessObjGet,			 zyTTEMRemoteAccessObjSet,		 NULL,	NULL,	NULL, NULL, NULL},
{TR98_TAAAB_Configuration_GUI_AccessRights_Page_Easymenu_EM_LocalAccess,	0,				para_GUIAccessRights,				zyTTEMLoaclAccessObjGet,			 zyTTEMLoaclAccessObjSet,		 NULL,	NULL,	NULL, NULL, NULL},
{TR98_TAAAB_Configuration_Shell,	0,				para_Shell,				zyTTShellObjGet,			 zyTTShellObjSet, 	 NULL,	NULL,	zyTTShellObjNotify, zyTTShellObjAttrGet, zyTTShellObjAttrSet},
#endif
#ifdef CONFIG_ZYXEL_TR140
#ifndef CONFIG_PACKAGE_ZyIMS
{TR98_SERVICE,					0,				NULL,			NULL,					 NULL,					 NULL,			   NULL,			NULL},
#endif
{TR98_StorageService,           ATTR_INDEXNODE,              para_SS,                SSObjGet,          SSObjSet,                   NULL,             NULL,            NULL, NULL, NULL},
{TR98_StorageService_UserAccount,			ATTR_INDEX_CREA,				para_SSUserAccount,				SSUserAccountObjGet,		   SSUserAccountObjSet,				   SSUserAccountObjAdd,			 SSUserAccountObjDel,			  NULL, NULL, NULL},
{TR98_StorageService_Capabilites,			0,				para_SSCapabilites,				SSCapabilitesObjGet,		   NULL,				   NULL,			 NULL,			  NULL, NULL, NULL},
{TR98_StorageService_NetInfo,			0,				para_SSNetInfo, 			SSNetInfoObjGet,		   NULL,				   NULL,			 NULL,			  NULL, NULL, NULL},
#ifdef CONFIG_ZCFG_BE_MODULE_SAMBA
{TR98_StorageService_NetworkServer,			0,				para_SSNetworkServer,				SSNetworkServerObjGet,		   SSNetworkServerObjSet,				   NULL,			 NULL,			  NULL, NULL, NULL},
#endif
{TR98_StorageService_FTPServer,			0,				para_SSFTPServer,				SSFTPServerObjGet,		   SSFTPServerObjSet,				   NULL,			 NULL,			  NULL, NULL, NULL},
{TR98_StorageService_PhysicalMedium,			ATTR_INDEXNODE, 			 para_SSPhysicalMedium,				 SSPhysicalMediumObjGet,			NULL,					NULL,			  NULL, 		   NULL, NULL, NULL},
{TR98_StorageService_LogicalVolume,			ATTR_INDEXNODE, 			para_SSLogicalVolume, 			SSLogicalVolumeObjGet,		   NULL, 			   NULL, 		 NULL,			  NULL, NULL, NULL},
{TR98_StorageService_LogicalVolume_Folder,			ATTR_INDEXNODE, 			 para_SSFolder,				 SSFolderObjGet,			NULL,					NULL,			  NULL, 		   NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_LIFEMOTE
{TR98_Configuration_Lifemote,				0,				para_Lifemote,					zyExtLifemoteObjGet, 				 zyExtLifemoteObjSet,				 NULL,	NULL,	zyExtLifemoteObjNotify,	zyExtLifemoteObjAttrGet, zyExtLifemoteObjAttrSet},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MODSW_ZYEE
{TR98_ZyEE_SoftwareMoudles,                                         0,                          para_SoftwareMoudles,                       ZyEEObjGet,              ZyEEObjSet,               NULL,            NULL,             NULL,            NULL},
{TR98_ZyEE_SoftwareMoudles_EeConf, 								    0,			                para_ZyeeExecEnvConf,						ZyEEObjGet, 			 ZyEEObjSet,			   NULL,		NULL,			  NULL, 		   NULL},
#ifdef CONFIG_TAAAB_CUSTOMIZATION
{TR98_ZyEE_SoftwareMoudles_ExecEnv,                                 ATTR_INDEX_CREA,            para_ZyeeExecEnv,                           ZyEEObjGet,              ZyEEObjSet,               ZyEEObjAdd,      NULL,             NULL,            NULL},
#else
{TR98_ZyEE_SoftwareMoudles_ExecEnv,                                 ATTR_INDEX_CREA,            para_ZyeeExecEnv,                           ZyEEObjGet,              ZyEEObjSet,               NULL,      NULL,             NULL,            NULL},
#endif
{TR98_ZyEE_SoftwareMoudles_DeploymentUnit,                          ATTR_INDEXNODE,             para_ZyeeDeploymentUnit,                    ZyEEObjGet,              ZyEEObjSet,               NULL,            NULL,             NULL,            NULL},
{TR98_ZyEE_SoftwareMoudles_ExecutionUnit,                           ATTR_INDEXNODE,             para_ZyeeExecutionUnit,                     ZyEEObjGet,              ZyEEObjSet,               NULL,            NULL,             NULL,            NULL},
{TR98_ZyEE_SoftwareMoudles_ExecutionUnit_Extensions,                0,                          para_ZyeeExecutionUnitExtensions,           ZyEEObjGet,              ZyEEObjSet,               NULL,            NULL,             NULL,            NULL},
{TR98_DUStateChangeComplPolicy,                                     0,                          para_ZyeeDUStateChangeComplPolicy,          ZyEEObjGet,              ZyEEObjSet,               NULL,            NULL,             NULL,            NULL},
#endif
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
{TR98_MultipleConfig,             0,              para_MultipleConfig,   zyExtMultipleConfigObjGet,          zyExtMultipleConfigObjSet,         NULL,             NULL,            NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_CELLWAN
{TR98_CELL,             0,              para_Cell,              zyExtCellObjGet,            NULL,                       NULL,   NULL,	NULL, NULL, NULL},
{TR98_CELL_INTF,        ATTR_INDEXNODE, para_CellIntf,          zyExtCellIntfObjGet,	    zyExtCellIntfObjSet,	    NULL,	NULL,	NULL, NULL, NULL},
{TR98_CELL_INTF_USIM,   ATTR_INDEXNODE, para_CellIntfUSIM,      zyExtCellIntfUSIMObjGet,	zyExtCellIntfUSIMObjSet,	NULL,	NULL,	NULL, NULL, NULL},
{TR98_CELL_AP,          ATTR_INDEXNODE, para_CellAccessPoint,   zyExtCellAccessPointObjGet,	zyExtCellAccessPointObjSet,	NULL,	NULL,	NULL, NULL, NULL},
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_MULTI_WAN
{TR98_MULTI_WAN,                0,              para_MultiWan,         zyExtMultiWanObjGet,		zyExtMultiWanObjSet,		NULL,					NULL,							NULL, NULL, NULL},
{TR98_MULTI_WAN_INTF,           ATTR_INDEXNODE,para_MultiWanIntf,      zyExtMultiWanIntfObjGet,	zyExtMultiWanIntfObjSet,	zyExtMultiWanIntfObjAdd,	zyExtMultiWanIntfObjDel,	NULL, NULL, NULL},
#endif
#ifdef ZYXEL_RUNNINGWAN_CUSTOMIZATION
{TR98_RUNNING_WAN_CONN,         ATTR_INDEX_CREA,para_RUNNINGWANConn,    RUNNINGWANConnObjGet,         NULL,                   NULL,             NULL,            NULL},
#endif
{TR98_WAN_IP_V4ROUTINGTABLE,   ATTR_INDEXNODE, para_IPV4RoutingTable,  WANV4RoutingTableObjGet, NULL,                   NULL,             NULL,            NULL},
{TR98_WAN_IP_V6ROUTINGTABLE,   ATTR_INDEXNODE, para_IPV6RoutingTable,  WANV6RoutingTableObjGet, NULL,                   NULL,             NULL,            NULL},
{NULL,                          0,              NULL,                   NULL,                    NULL,                   NULL,             NULL,            NULL}
};

