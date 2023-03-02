#ifndef _LIBZCFG_FE_RDM_STRUCT_H
#define _LIBZCFG_FE_RDM_STRUCT_H

#ifdef ZCFG_MEMORY_LEAK_DETECT

#include "zos_api.h"

#define zcfgFeObjStructGetNext(oid, iid, obj) _zcfgFeObjStructGetNext(oid, iid, obj, __FILE__, __LINE__)
#define zcfgFeObjStructGetNextWithoutUpdate(oid, iid, obj) _zcfgFeObjStructGetNextWithoutUpdate(oid, iid, obj, __FILE__, __LINE__)
#define zcfgFeObjStructGet(oid, iid, obj) _zcfgFeObjStructGet(oid, iid, obj, __FILE__, __LINE__)
#define zcfgFeObjStructGetWithoutUpdate(oid, iid, obj) _zcfgFeObjStructGetWithoutUpdate(oid, iid, obj, __FILE__, __LINE__)
#define zcfgFeSubInStructGetNext(oid, iid, iid2, obj) _zcfgFeSubInStructGetNext(oid, iid, iid2, obj, __FILE__, __LINE__)
#define zcfgFeSubInStructGetNextWithoutUpdate(oid, iid, iid2, obj) _zcfgFeSubInStructGetNextWithoutUpdate(oid, iid, iid2, obj, __FILE__, __LINE__)

zcfgRet_t _zcfgFeObjStructGetNext(zcfg_offset_t, objIndex_t *, void **, const char *, int);
zcfgRet_t _zcfgFeObjStructGetNextWithoutUpdate(zcfg_offset_t, objIndex_t *, void **, const char *, int);
zcfgRet_t _zcfgFeObjStructGet(zcfg_offset_t, objIndex_t *, void **, const char *, int);
zcfgRet_t _zcfgFeObjStructGetWithoutUpdate(zcfg_offset_t, objIndex_t *, void **, const char *, int);
zcfgRet_t _zcfgFeSubInStructGetNext(zcfg_offset_t , objIndex_t *, objIndex_t *, void **, const char *, int);
zcfgRet_t _zcfgFeSubInStructGetNextWithoutUpdate(zcfg_offset_t, objIndex_t *, objIndex_t *, void **, const char *, int);

#else

zcfgRet_t zcfgFeObjStructGetNext(zcfg_offset_t, objIndex_t *, void **);
zcfgRet_t zcfgFeObjStructGetNextWithoutUpdate(zcfg_offset_t , objIndex_t *, void **);
zcfgRet_t zcfgFeObjStructGet(zcfg_offset_t, objIndex_t *, void **);
zcfgRet_t zcfgFeObjStructGetWithoutUpdate(zcfg_offset_t, objIndex_t *, void **);
zcfgRet_t zcfgFeSubInStructGetNext(zcfg_offset_t, objIndex_t *, objIndex_t *, void **);
zcfgRet_t zcfgFeSubInStructGetNextWithoutUpdate(zcfg_offset_t, objIndex_t *, objIndex_t *, void **);

#endif

zcfgRet_t zcfgFeObjStructSet(zcfg_offset_t, objIndex_t *, void *, char *);
zcfgRet_t zcfgFeObjStructSetWithoutApply(zcfg_offset_t, objIndex_t *, void *, char *);
zcfgRet_t zcfgFeObjStructAdd(zcfg_offset_t, objIndex_t *, char *);
zcfgRet_t zcfgFeObjStructDel(zcfg_offset_t, objIndex_t *, char *);

#if 1 /* non-blocking config apply, ZyXEL, John */
zcfgRet_t zcfgFeObjStructBlockedSet(zcfg_offset_t oid, objIndex_t *objIid, void *requestStruct, char *feedbackMsg);
zcfgRet_t zcfgFeObjStructBlockedAdd(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg);
zcfgRet_t zcfgFeObjStructBlockedDel(zcfg_offset_t oid, objIndex_t *objIid, char *feedbackMsg);
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
#define zcfgFeObjStructFree(obj) ZOS_FREE(obj)
#else
#define zcfgFeObjStructFree(obj) \
    {\
        do \
        { \
            if ((obj) == NULL) \
            { \
                printf("Free NULL pointer. %s %d\n", __FILE__, __LINE__); \
            } \
            else \
            { \
                free((void *)(obj)); \
                (obj) = NULL; \
            } \
        } \
        while (0); \
    }
#endif

#endif
