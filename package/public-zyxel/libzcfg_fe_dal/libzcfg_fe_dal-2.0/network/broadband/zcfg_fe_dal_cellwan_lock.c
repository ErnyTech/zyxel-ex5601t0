#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_PCI_LOCK

#define LOCKED_LIST_MAX_NUM (ZYXEL_PCI_LOCK_NUM)

dal_param_t CELLWAN_LOCK_param[]={
{"New", dalType_boolean, 0, 0, NULL},
{"LK_Enable", dalType_boolean, 0, 0, NULL},
{"Index", dalType_int, 1, 8, NULL},
{"PhyCellID", dalType_int, 0, 503, NULL},
{"RFCN", dalType_int, 0, 16777216, NULL},
{ NULL,	0,	0,	0,	NULL}

};

zcfgRet_t zcfgFeDalCellWanLockGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *intfZyLockObj = NULL;
	struct json_object *lockedListArray = NULL, *lockedListObj = NULL;
	objIndex_t iid = {0};
	bool intfZyLockEnable = false;

	paramJobj = json_object_new_object();

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &iid, &intfZyLockObj) == ZCFG_SUCCESS){
		intfZyLockEnable = Jgetb(intfZyLockObj, "Enable");
		json_object_object_add(paramJobj, "LK_Enable", json_object_new_boolean(intfZyLockEnable));
		json_object_object_add(paramJobj, "LK_ConnectionMode", JSON_OBJ_COPY(json_object_object_get(intfZyLockObj, "ConnectionMode")));
		json_object_object_add(paramJobj, "LK_NumberOfPCILocked", JSON_OBJ_COPY(json_object_object_get(intfZyLockObj, "NumberOfPCILocked")));
		zcfgFeJsonObjFree(intfZyLockObj);
	}

	IID_INIT(iid);
	lockedListArray = json_object_new_array();
	while(zcfgFeObjJsonGetNext(RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST, &iid, &lockedListObj) == ZCFG_SUCCESS) {
		json_object_array_add(lockedListArray, lockedListObj);
	}
	json_object_object_add(paramJobj, "LK_LockedList", lockedListArray);

	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowCellWanLock(struct json_object *Jarray){
	objIndex_t iid = {0};
	int i, len = 0, index = 0, numbers = 0;
	struct json_object *Jobj = NULL;
	struct json_object *lockedListObj = NULL;
	struct json_object *lockedListArray = NULL;

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	Jobj = json_object_array_get_idx(Jarray, 0);
	printf("%-30s %s\n","Lock Enable:", Jgets(Jobj, "LK_Enable"));
	printf("%-30s %s\n","Number Of PCI Locked:", Jgets(Jobj, "LK_NumberOfPCILocked"));
	numbers = Jgeti(Jobj, "LK_NumberOfPCILocked");

	if(numbers){
		lockedListArray = json_object_object_get(Jobj, "LK_LockedList");
		len = json_object_array_length(lockedListArray);
		printf("%-10s %-10s %-25s\n","Index", "PhyCellID", "RFCN");
		for(i=0; i<len; i++){
			IID_INIT(iid);
			lockedListObj = json_object_array_get_idx(lockedListArray, i);
			index = i+1;
			printf("%-10d %-10s %-25s\n",
				index, Jgets(lockedListObj, "PhyCellID"), Jgets(lockedListObj, "RFCN"));

		}
		zcfgFeJsonObjFree(lockedListArray);
	}

}

zcfgRet_t zcfgFeDalCellWanLockEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	zcfg_offset_t oid;
	objIndex_t iid = {0};
	bool pciInvalid = false;
	bool convertFail = false;
	bool intfZyLockEnable = false;
	int i, len = 0, index = 0, pci = 0, rfcn = 0;
	struct json_object *lockObj = NULL;
	struct json_object *lockedListObj = NULL;
	struct json_object *lockedListTmpObj = NULL;
	struct json_object *lockedListArray = NULL;

	if(json_object_object_get(Jobj, "LK_Enable"))
		intfZyLockEnable = Jgetb(Jobj, "LK_Enable");

	/* set by dalcmd */
	if(intfZyLockEnable &&
		(lockedListArray = json_object_object_get(Jobj, "LK_LockedList")) == NULL){

		if(json_object_object_get(Jobj, "Index"))
			index = Jgeti(Jobj, "Index");

		if(index){
			if(json_object_object_get(Jobj, "PhyCellID"))
				pci = Jgeti(Jobj, "PhyCellID");
			if(json_object_object_get(Jobj, "RFCN"))
				rfcn = Jgeti(Jobj, "RFCN");

			IID_INIT(iid);
			oid = RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST;
			if((ret = convertIndextoIid(index, &iid, oid, NULL, NULL, replyMsg)) != ZCFG_SUCCESS)
				return ret;

			if((ret = zcfgFeObjJsonGetWithoutUpdate(oid, &iid, &lockedListObj)) == ZCFG_SUCCESS){
				if(json_object_object_get(Jobj, "PhyCellID"))
					json_object_object_add(lockedListObj, "PhyCellID", json_object_new_int(pci));
				if(json_object_object_get(Jobj, "RFCN"))
					json_object_object_add(lockedListObj, "RFCN", json_object_new_int(rfcn));
				zcfgFeObjJsonSet(oid, &iid, lockedListObj, NULL);
			}
			zcfgFeJsonObjFree(lockedListObj);

			IID_INIT(iid);
			while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &lockedListObj) == ZCFG_SUCCESS){
				if(json_object_object_get(lockedListObj, "PhyCellID"))
					if((pci = Jgeti(lockedListObj, "PhyCellID")) == -1)
						pciInvalid = true;
				zcfgFeJsonObjFree(lockedListObj);
			}
		}
		else{
			IID_INIT(iid);
			oid = RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST;
			while(zcfgFeObjJsonGetNextWithoutUpdate(oid, &iid, &lockedListObj) == ZCFG_SUCCESS){
				len += 1;
				if(json_object_object_get(lockedListObj, "PhyCellID")){
					if((pci = Jgeti(lockedListObj, "PhyCellID")) == -1){
						if(replyMsg) strcpy(replyMsg, "PCI or RFCN is invalid.");
						zcfgFeJsonObjFree(lockedListObj);
						return ZCFG_INVALID_PARAM_VALUE;
					}
				}
				zcfgFeJsonObjFree(lockedListObj);
			}
			if(!len){
				if(replyMsg) strcpy(replyMsg, "No rule specified. Please at least add one rule.");
				return ZCFG_EMPTY_OBJECT;
			}
		}
	}

	/* set by webgui_vue */
	if(intfZyLockEnable &&
		(lockedListArray = json_object_object_get(Jobj, "LK_LockedList")) != NULL){
		len = json_object_array_length(lockedListArray);
		oid = RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST;
		for(i=0; i<len; i++){
			IID_INIT(iid);
			lockedListTmpObj = json_object_array_get_idx(lockedListArray, i);
			index = i+1;
			pci = Jgeti(lockedListTmpObj, "PhyCellID");
			rfcn = Jgeti(lockedListTmpObj, "RFCN");

			if(pci == -1){
				pciInvalid = true;
				continue;
			}

			if((ret = convertIndextoIid(index, &iid, oid, NULL, NULL, replyMsg)) != ZCFG_SUCCESS){
				convertFail = true;
				continue;
			}

			if((ret = zcfgFeObjJsonGetWithoutUpdate(oid, &iid, &lockedListObj)) == ZCFG_SUCCESS){
				if(json_object_object_get(lockedListTmpObj, "PhyCellID"))
					json_object_object_add(lockedListObj, "PhyCellID", json_object_new_int(pci));
				if(json_object_object_get(lockedListTmpObj, "RFCN"))
					json_object_object_add(lockedListObj, "RFCN", json_object_new_int(rfcn));
				zcfgFeObjJsonSet(oid, &iid, lockedListObj, NULL);
			}
			zcfgFeJsonObjFree(lockedListObj);
		}
		zcfgFeJsonObjFree(lockedListArray);
	}

	if(pciInvalid){
		if(replyMsg) strcpy(replyMsg, "PCI or RFCN is invalid.");
		Jadds(Jobj, "__multi_lang_replyMsg", "zylang.Broadband.CellWan.Lock.Warning.pci_is_invalid");
		return ZCFG_INVALID_PARAM_VALUE;
	}
	if(convertFail){
		if(replyMsg) strcpy(replyMsg, "Index error.");
		return ZCFG_NO_SUCH_OBJECT;
	}

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &iid, &lockObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "LK_Enable"))
			json_object_object_add(lockObj, "Enable", json_object_new_boolean(intfZyLockEnable));
		zcfgFeObjJsonSet(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &iid, lockObj, NULL);
	}
	zcfgFeJsonObjFree(lockObj);

	return ret;
}	

zcfgRet_t zcfgFeDalCellWanLockAdd(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	int numbers = 0;
	struct json_object *lockObj = NULL;
	objIndex_t lockObjIid = {0}, lockedListObjIid = {0};

	IID_INIT(lockObjIid);
	lockObjIid.level=1;
	lockObjIid.idx[0]=1;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &lockObjIid, &lockObj)) == ZCFG_SUCCESS){
		numbers = Jgeti(lockObj, "NumberOfPCILocked");
	}

	if(numbers >= LOCKED_LIST_MAX_NUM){
		if(replyMsg) strcpy(replyMsg, "Over the upper bound!");
		zcfgFeJsonObjFree(lockObj);
		return ZCFG_EXCEED_MAX_INS_NUM;
	}

	IID_INIT(lockedListObjIid);
	lockedListObjIid.level=1;
	lockedListObjIid.idx[0]=1;
	if((ret = zcfgFeObjJsonAdd(RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST, &lockedListObjIid, NULL)) != ZCFG_SUCCESS){
		printf("[%d] %s: zcfgFeObjJsonAdd fail.\n", __LINE__, __func__);
		if(replyMsg) strcpy(replyMsg, "zcfgFeObjJsonAdd fail");
		zcfgFeJsonObjFree(lockObj);
		return ret;
	}	

	numbers += 1;
	json_object_object_add(lockObj, "NumberOfPCILocked", json_object_new_int(numbers));
	if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &lockObjIid, lockObj, NULL)) != ZCFG_SUCCESS){
		printf("[%d] %s: zcfgFeObjJsonSetWithoutApply fail.\n", __LINE__, __func__);
		if(replyMsg) strcpy(replyMsg, "zcfgFeObjJsonSetWithoutApply fail");
	}
	zcfgFeJsonObjFree(lockObj);

	return ret;
}

zcfgRet_t zcfgFeDalCellWanLockDelete(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	zcfg_offset_t oid;
	int index = 0, numbers = 0;
	struct json_object *lockObj = NULL;

	index = Jgeti(Jobj, "Index");
	oid = RDM_OID_CELL_INTF_ZY_PCI_LOCKED_LIST;
	if((ret = convertIndextoIid(index, &iid, oid, NULL, NULL, replyMsg)) != ZCFG_SUCCESS)
		return ret;

	if((ret = zcfgFeObjJsonDel(oid, &iid, NULL)) != ZCFG_SUCCESS){
		printf("[%d] %s zcfgFeObjJsonDel fail.\n", __LINE__, __func__);
		if(replyMsg) strcpy(replyMsg, "zcfgFeObjJsonDel fail");
		return ret;
	}

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;
	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &iid, &lockObj)) == ZCFG_SUCCESS){
		numbers = Jgeti(lockObj, "NumberOfPCILocked");
		numbers -= 1;
		json_object_object_add(lockObj, "NumberOfPCILocked", json_object_new_int(numbers));
		if((ret = zcfgFeObjJsonSetWithoutApply(RDM_OID_CELL_INTF_ZY_PCI_LOCK, &iid, lockObj, NULL)) != ZCFG_SUCCESS){
			printf("[%d] %s: zcfgFeObjJsonSetWithoutApply fail.\n", __LINE__, __func__);
		}
		zcfgFeJsonObjFree(lockObj);
	}

	return ret;
}


zcfgRet_t zcfgFeDalCellWanLock(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanLockGet(Jobj, Jarray, replyMsg);
	else if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanLockEdit(Jobj, replyMsg);
	else if(!strcmp(method, "POST"))
		ret = zcfgFeDalCellWanLockAdd(Jobj, replyMsg);
	else if(!strcmp(method, "DELETE"))
		ret = zcfgFeDalCellWanLockDelete(Jobj, replyMsg);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

#endif

