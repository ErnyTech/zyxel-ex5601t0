#ifndef ZYEE_ZCFG_SERVICE_H
#define ZYEE_ZCFG_SERVICE_H

int zyee_zcfg_service_init(char *service);
int zyee_zcfg_service_run(void);
int zyee_zcfg_service_cleanup(void);
int zyee_zcfg_service_oid_add_watch(unsigned int oid, int (*fn)(void *, void *, unsigned int, objIndex_t *));
int zyee_zcfg_rdm_oid_remap(const char *oidname);

#endif
