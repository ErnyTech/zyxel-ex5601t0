#ifndef _ZCFG_FE_DAL_H
#define _ZCFG_FE_DAL_H

int zcfgFeDalGetReturnValue(struct json_object *Jobj, const char *key, void* value);
int zcfgFeDalSetReturnValue(struct json_object *Jobj, json_type type, const char *key, void* value);
zcfgRet_t zcfgFeDalHandler(const char *handlerName, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);
zcfgRet_t getWanLanList(struct json_object **Jarray);
zcfgRet_t getLanPortList(struct json_object **Jarray);
#if defined(MSTC_TAAAG_MULTI_USER) || defined(MSTC_RAAAA_MULTI_USER)
zcfgRet_t getmultiuserList(struct json_object **Jarray, const char *userName);
#endif

void invalidCharFilter(char *input, int maxlen);
bool HostNameFilter(const char *input);
bool DomainFilter(const char *input);
bool IPFilter(const char *input);
bool IPv6Filter(const char *input);

int isDigitString(char *getString);
#ifdef CONFIG_ABZQ_CUSTOMIZATION
zcfgRet_t insertLoginUserInfo(struct json_object *Jobj, const char *userName, const char *level, const char *remoteAddress, const char *accessMode, const char *srvName);
#else
zcfgRet_t insertLoginUserInfo(struct json_object *Jobj, const char *userName, const char *level, const char *remoteAddress);
#endif
zcfgRet_t getLoginUserInfo(struct json_object *Jobj, char *userName, char *level, char *remoteAddress);
zcfgRet_t dalReqSendWithoutResponse(char *payload);
bool isSameSubNet( char *lanIp, char *lansubnetMask, char *staticIP);

extern bool validateMacAddr1(const char *macAddr, const char *delim);
bool findInList(char *list, char *target, char *separator);

void zcfgInitDAL();

#endif
