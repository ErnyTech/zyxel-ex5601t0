tr98Parameter_t para_IpPingDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 29, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Host", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "NumberOfRepetitions", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DataBlockSize", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DSCP", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "SuccessCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "FailureCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "AverageResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "MinimumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "MaximumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_TraceRtDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 29, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Host", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "NumberOfTries", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DataBlockSize", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DSCP", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "MaxHopCount", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "RouteHopsNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_RtHop[] = {
	{ "HopHost", PARAMETER_ATTR_READONLY, 257, json_type_string},
	{ "HopHostAddress", PARAMETER_ATTR_READONLY, 33, json_type_string},
	{ "HopErrorCode", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "HopRTTimes", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_NsLookUpDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 29, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "HostName", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "DNSServer", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "NumberOfRepetitions", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "SuccessCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "ResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_NsLookUpResult[] = {
	{ "Status", PARAMETER_ATTR_READONLY, 29, json_type_string},
	{ "AnswerType", PARAMETER_ATTR_READONLY, 46, json_type_string},
	{ "HostNameReturned", PARAMETER_ATTR_READONLY, 257, json_type_string},
	{ "IPAddresses", PARAMETER_ATTR_READONLY, 46, json_type_string},
	{ "DNSServerIP", PARAMETER_ATTR_READONLY, 46, json_type_string},
	{ "ResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_SvrSelectDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 29, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "ProtocolVersion", PARAMETER_ATTR_WRITE, 9, json_type_string},
	{ "Protocol", PARAMETER_ATTR_WRITE, 11, json_type_string},
	{ "HostList", PARAMETER_ATTR_WRITE, 2601, json_type_string},
	{ "NumberOfRepetitions", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "FastestHost", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "MinimumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "AverageResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "MaximumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IPAddressUsed", PARAMETER_ATTR_WRITE, 46, json_type_string},
	{ NULL, 0, 0}
};

tr98Parameter_t para_PerpDiag[] = {
	{ "DownloadTransports", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ "DownloadDiagnosticMaxConnections", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "DownloadDiagnosticsMaxIncrementalResult", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UploadTransports", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ "UploadDiagnosticsMaxConnections", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UploadDiagnosticsMaxIncrementalResult", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UDPEchoDiagnosticsMaxResults", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IPv4DownloadDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv6DownloadDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv4UploadDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv6UploadDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv4UDPEchoDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv6UDPEchoDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv4ServerSelectionDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "IPv6ServerSelectionDiagnosticsSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ NULL, 0, 0}
};

tr98Parameter_t para_DlDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "DownloadURL", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "DownloadTransports", PARAMETER_ATTR_WRITE, 11, json_type_string},
	{ "DSCP", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "EthernetPriority", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_NumOfThreads", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
	{ "X_ZYXEL_EnableIperf", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfServerAddress", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_ZYXEL_IperfPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfHWoffload", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfUdp", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfBandWidth", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfBytes", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfIPv4Only", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfIPv6Only", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
    { "TimeBasedTestDuration", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeBasedTestMeasurementInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeBasedTestMeasurementOffset", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ProtocolVersion", PARAMETER_ATTR_WRITE, 9, json_type_string},
    { "NumberOfConnections", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
    { "EnablePerConnectionResults", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
    { "DownloadDiagnosticMaxConnections", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "DownloadDiagnosticsMaxIncrementalResult", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "IPAddressUsed", PARAMETER_ATTR_READONLY, 46, json_type_string},
	{ "ROMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "BOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TestBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestBytesReceivedUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceivedUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSentUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "PeriodOfFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TCPOpenRequestTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TCPOpenResponseTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "PerConnectionResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IncrementalResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	//{ "X_ZYXEL_TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32}, replaced by TotalBytesSent
	//{ "X_ZYXEL_SessionTimeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32}, replaced by TimeBasedTestDuration
	//{ "X_ZYXEL_NumOfThreads", PARAMETER_ATTR_WRITE, 0, json_type_uint32},  replaced by NumberOfConnections
#ifdef ZYXEL_DL_DIAG_CALCULATION
	{ "X_ZYXEL_Latency", PARAMETER_ATTR_READONLY, 17, json_type_string},
	{ "X_ZYXEL_RoundTripTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "X_ZYXEL_DownloadSpeed", PARAMETER_ATTR_READONLY, 17, json_type_string},
#endif
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
	{ "X_ZYXEL_IperfDownloadSpeed", PARAMETER_ATTR_READONLY, 18, json_type_string},
#endif
	{ NULL, 0, 0}
};

tr98Parameter_t para_DlDiagPerConnRst[] = {
	{ "ROMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "BOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TestBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TCPOpenRequestTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TCPOpenResponseTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ NULL, 0, 0}
};

tr98Parameter_t para_DlDiagIncremRst[] = {
	{ "TestBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "StartTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EndTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ NULL, 0, 0}
};

tr98Parameter_t para_UlDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "UploadURL", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "UploadTransports", PARAMETER_ATTR_WRITE, 11, json_type_string},
	{ "DSCP", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "EthernetPriority", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TestFileLength", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_NumOfThreads", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
	{ "X_ZYXEL_EnableIperf", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfServerAddress", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "X_ZYXEL_IperfPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfHWoffload", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfUdp", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfBandWidth", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfBytes", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "X_ZYXEL_IperfIPv4Only", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "X_ZYXEL_IperfIPv6Only", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
#endif
    { "TimeBasedTestDuration", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeBasedTestMeasurementInterval", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "TimeBasedTestMeasurementOffset", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ProtocolVersion", PARAMETER_ATTR_WRITE, 9, json_type_string},
    { "NumberOfConnections", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
    { "EnablePerConnectionResults", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
    { "UploadDiagnosticsMaxConnections", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UploadDiagnosticsMaxIncrementalResult", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "IPAddressUsed", PARAMETER_ATTR_READONLY, 46, json_type_string},
	{ "ROMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "BOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TestBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestBytesSentUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceivedUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSentUnderFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "PeriodOfFullLoading", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TCPOpenRequestTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TCPOpenResponseTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "PerConnectionResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "IncrementalResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	//{ "X_ZYXEL_TestBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},  replaced by TestBytesSent
	//{ "X_ZYXEL_TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32}, replaced by TotalBytesReceived
	//{ "X_ZYXEL_SessionTimeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32}, replaced by TimeBasedTestDuration
	//{ "X_ZYXEL_NumOfThreads", PARAMETER_ATTR_WRITE, 0, json_type_uint32},  replaced by NumberOfConnections
#ifdef ZYXEL_DIAG_IPERF_SUPPORT
	{ "X_ZYXEL_IperfUploadSpeed", PARAMETER_ATTR_READONLY, 18, json_type_string},
#endif
	{ NULL, 0, 0}
};

tr98Parameter_t para_UlDiagPerConnRst[] = {
	{ "ROMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "BOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EOMTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TestBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TCPOpenRequestTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TCPOpenResponseTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ NULL, 0, 0}
};

tr98Parameter_t para_UlDiagIncremRst[] = {
	{ "TestBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TotalBytesSent", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "StartTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "EndTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ NULL, 0, 0}
};

tr98Parameter_t para_UdpEchoCfg[] = {
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "SourceIPAddress", PARAMETER_ATTR_WRITE, 17, json_type_string},
	{ "UDPPort", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "EchoPlusEnabled", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "EchoPlusSupported", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "PacketsReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "PacketsResponded", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "BytesReceived", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "BytesResponded", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TimeFirstPacketReceived", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TimeLastPacketReceived", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ NULL, 0, 0}
};

tr98Parameter_t para_UdpEchoDiag[] = {
	{ "DiagnosticsState", PARAMETER_ATTR_WRITE, 29, json_type_string},
	{ "Interface", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Host", PARAMETER_ATTR_WRITE, 257, json_type_string},
	{ "Port", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "NumberOfRepetitions", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Timeout", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DataBlockSize", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "DSCP", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "InterTransmissionTime", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "ProtocolVersion", PARAMETER_ATTR_WRITE, 9, json_type_string},
	{ "IPAddressUsed", PARAMETER_ATTR_WRITE, 46, json_type_string},
	{ "SuccessCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "FailureCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "AverageResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "MinimumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "MaximumResponseTime", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "EnableIndividualPacketResults", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "IndividualPacketResultNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "UDPEchoDiagnosticsMaxResults", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_UdpEchoDiagPakRst[] = {
	{ "PacketSuccess", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "PacketSendTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "PacketReceiveTime", PARAMETER_ATTR_READONLY, 27, json_type_time},
	{ "TestGenSN", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestRespSN", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestRespRcvTimeStamp", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestRespReplyTimeStamp", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "TestRespReplyFailureCount", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};