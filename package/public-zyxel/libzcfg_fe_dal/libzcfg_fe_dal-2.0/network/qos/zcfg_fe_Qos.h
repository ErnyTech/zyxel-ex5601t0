#ifndef ZCFG_FE_QOS_H
#define ZCFG_FE_QOS_H

#define QOS_DAL_DEBUG 0

#define QOS_MAX_SHAPING_RATE 10000000

typedef enum e_ActionType {
	e_ActionType_ADD = 0,
	e_ActionType_EDIT,
	e_ActionType_DELETE
}e_ActionType;

#if 0
void debug_RDM_item(int RDM_OID, int Iid_idx, char *item, int type) {
	int res_int = 0;
	int ret;
	json_object *obj = NULL;
	objIndex_t Iid = {0};
	IID_INIT(Iid);

	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID, &Iid, &obj) == ZCFG_SUCCESS) {
		if((Iid.idx[0] == Iid_idx) || (Iid_idx == -1)) {
			if(type == 1) {
				res_int = json_object_get_int(json_object_object_get(obj, item));
				printf("debug_RDM_item: OID %d, iid %d, item %s, value %d\n", RDM_OID, Iid.idx[0], item, res_int);
			}
		}
		zcfgFeJsonObjFree(obj);
	}
	return;
}
#endif
#endif