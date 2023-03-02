#ifndef _ZCMD_TOOL_H
#define _ZCMD_TOOL_H

#ifdef ZCFG_MEMORY_LEAK_DETECT
#define zcmdStructToJString(rqst_struct, jstr, oid, schema) _zcmdStructToJString(rqst_struct, jstr, oid, schema, __FILE__, __LINE__)
#define zcmdJStringToStruct(objStruct, jstr, oid, schema) _zcmdJStringToStruct(objStruct, jstr, oid, schema, __FILE__, __LINE__)

void _zcmdStructToJString(void *, char **, zcfg_offset_t, void *, const char *, int);
zcfgRet_t _zcmdJStringToStruct(void **, const char *, zcfg_offset_t, void *, const char *, int);
#else

/*Used to transform the requested structure to json string*/
void zcmdStructToJString(void *, char **, zcfg_offset_t, void *);
/*Used to parse json string and fill up the requested structure*/
zcfgRet_t zcmdJStringToStruct(void **, const char *, zcfg_offset_t, void *);
#endif


#endif
