#ifndef _ZCFG_FE_DAL_RDMACL_H
#define _ZCFG_FE_DAL_RDMACL_H

//
// on white list,
//  iid null implies no restriction on iid
//  params not null means it would reply only these listed
typedef struct rdmObjectACL_s {
	zcfg_offset_t oid;
	int method;
	const char **iid;
	const char **params;
}rdmObjectACL_t;

#ifdef ZCFG_MEMORY_LEAK_DETECT

typedef zcfgRet_t (*queryRdmObjRout)(zcfg_offset_t objOid, objIndex_t *objIid, struct json_object **response, const char *fileName, int line);
typedef zcfgRet_t (*queryRdmSubObjRout)(zcfg_offset_t objOid, objIndex_t *objIid, objIndex_t *childIid, struct json_object **response, const char *fileName, int line);

#define zcfgFeDalQueryRdmObjHandler(oid, method, Jobj, Jarray, replyMsg) _zcfgFeDalQueryRdmObjHandler(oid, method, Jobj, Jarray, replyMsg, __FILE__, __LINE__)
zcfgRet_t _zcfgFeDalQueryRdmObjHandler(zcfg_offset_t oid, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg, const char *fileName, int line);

#else

typedef zcfgRet_t (*queryRdmObjRout)(zcfg_offset_t objOid, objIndex_t *objIid, struct json_object **response);
typedef zcfgRet_t (*queryRdmSubObjRout)(zcfg_offset_t objOid, objIndex_t *objIid, objIndex_t *childIid, struct json_object **response);

zcfgRet_t zcfgFeDalQueryRdmObjHandler(zcfg_offset_t oid, const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg);

#endif

#endif
