/*Parameter*/
extern tr98Parameter_t para_IpPingDiag[];
extern tr98Parameter_t para_TraceRtDiag[];
extern tr98Parameter_t para_RtHop[];
extern tr98Parameter_t para_NsLookUpDiag[];
extern tr98Parameter_t para_NsLookUpResult[];
extern tr98Parameter_t para_SvrSelectDiag[];
extern tr98Parameter_t para_PerpDiag[];
extern tr98Parameter_t para_DlDiag[];
extern tr98Parameter_t para_DlDiagPerConnRst[];
extern tr98Parameter_t para_DlDiagIncremRst[];
extern tr98Parameter_t para_UlDiag[];
extern tr98Parameter_t para_UlDiagPerConnRst[];
extern tr98Parameter_t para_UlDiagIncremRst[];
extern tr98Parameter_t para_UdpEchoCfg[];
extern tr98Parameter_t para_UdpEchoDiag[];
extern tr98Parameter_t para_UdpEchoDiagPakRst[];
/*Handler Function*/

/* InternetGatewayDevice.IPPingDiagnostics */
extern zcfgRet_t ipPingDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t ipPingDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);

/* InternetGatewayDevice.TraceRouteDiagnostics */
extern zcfgRet_t traceRtDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t traceRtDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);

/* InternetGatewayDevice.TraceRouteDiagnostics.RouteHops.i */
extern zcfgRet_t routeHopsObjGet(char *, int, struct json_object **, bool);

/* InternetGatewayDevice.NsLookUpDiagnostics */
extern zcfgRet_t nsLookUpDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t nsLookUpDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);

/* InternetGatewayDevice.NsLookUpDiagnostics.Result.i */
extern zcfgRet_t nsLookUpResultObjGet(char *, int, struct json_object **, bool);

/* InternetGatewayDevice.ServerSelectionDiagnostics */
extern zcfgRet_t srvSelectDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t srvSelectDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);
	
/* InternetGatewayDevice.Capabilities.PerformanceDiagnostic */
extern zcfgRet_t perfDiagObjGet(char *, int, struct json_object **, bool);
/* InternetGatewayDevice.DownloadDiagnostics */
extern zcfgRet_t dlDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t dlDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t dlDiagPerConnRstObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t dlDiagIncremRstObjGet(char *, int, struct json_object **, bool);	
/* InternetGatewayDevice.UploadDiagnostics */
extern zcfgRet_t ulDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t ulDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);
extern zcfgRet_t ulDiagPerConnRstObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t ulDiagIncremRstObjGet(char *, int, struct json_object **, bool);	
/* InternetGatewayDevice.UDPEchoConfig */
extern zcfgRet_t udpEchoCfgObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t udpEchoCfgObjSet(char *, int, struct json_object *, struct json_object *, char *);
/* InternetGatewayDevice.X_ZYXEL_UDPEchoDiagnostics */
extern zcfgRet_t udpEchoDiagObjGet(char *, int, struct json_object **, bool);
extern zcfgRet_t udpEchoDiagObjSet(char *, int, struct json_object *, struct json_object *, char *);
/* InternetGatewayDevice.X_ZYXEL_UDPEchoDiagnostics.IndividualPacketResult */
zcfgRet_t udpEchoDiagPakRstObjGet(char *, int , struct json_object **, bool );