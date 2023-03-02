#include <ctype.h>
#include <json/json.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

#define CWMP_SOAP_MSG_DEBUG (1 << 1)

dal_param_t CELLWAN_STATUS_param[]={
{ NULL,	0,	0,	0,	NULL}
};

zcfgRet_t zcfgFeDalCellWanStatusGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	objIndex_t iid = {0};
	struct json_object *paramJobj = NULL;
	struct json_object *cellObj = NULL, *intfObj = NULL, *intfUsimObj = NULL, *intfZyIpPassthruObj = NULL;
	struct json_object *intfZySccJarray = NULL, *intfZySccObj = NULL;
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NR_NSA
	struct json_object *intfZyNrNsaObj = NULL;
#endif
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NBR_CELLS
	bool intfZyNbrEnable = false;
	struct json_object *intfZyNbrJarray = NULL, *intfZyNbrObj = NULL;
#endif
	bool intfZySccEnable = false;
#ifdef TELIA_CUSTOMIZATION	//Add frequency information
	const char *bands_without_info = NULL;
	char bands_buffer[128];
#endif
	paramJobj = json_object_new_object();

	IID_INIT(iid);
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL, &iid, &cellObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "CELL_Roaming_Enable", JSON_OBJ_COPY(json_object_object_get(cellObj, "RoamingEnabled")));
		zcfgFeJsonObjFree(cellObj);
	}

	IID_INIT(iid);
	iid.level=1;
	iid.idx[0]=1;
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF, &iid, &intfObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "INTF_Status", JSON_OBJ_COPY(json_object_object_get(intfObj, "Status")));
		json_object_object_add(paramJobj, "INTF_IMEI", JSON_OBJ_COPY(json_object_object_get(intfObj, "IMEI")));
		json_object_object_add(paramJobj, "INTF_Current_Access_Technology", JSON_OBJ_COPY(json_object_object_get(intfObj, "CurrentAccessTechnology")));
		json_object_object_add(paramJobj, "INTF_Network_In_Use", JSON_OBJ_COPY(json_object_object_get(intfObj, "NetworkInUse")));
		json_object_object_add(paramJobj, "INTF_RSSI", JSON_OBJ_COPY(json_object_object_get(intfObj, "RSSI")));
		json_object_object_add(paramJobj, "INTF_Supported_Bands", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_SupportedBands")));
		json_object_object_add(paramJobj, "INTF_Current_Band", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_CurrentBand")));
		json_object_object_add(paramJobj, "INTF_Cell_ID", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_CellID")));
		json_object_object_add(paramJobj, "INTF_PhyCell_ID", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_PhyCellID")));
		json_object_object_add(paramJobj, "INTF_Uplink_Bandwidth", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_UplinkBandwidth")));
		json_object_object_add(paramJobj, "INTF_Downlink_Bandwidth", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_DownlinkBandwidth")));
		json_object_object_add(paramJobj, "INTF_RFCN", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_RFCN")));
		json_object_object_add(paramJobj, "INTF_RSRP", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_RSRP")));
		json_object_object_add(paramJobj, "INTF_RSRQ", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_RSRQ")));
		json_object_object_add(paramJobj, "INTF_RSCP", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_CPICHRSCP")));
		json_object_object_add(paramJobj, "INTF_EcNo", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_CPICHEcNo")));
		json_object_object_add(paramJobj, "INTF_TAC", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_TAC")));
		json_object_object_add(paramJobj, "INTF_LAC", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_LAC")));
		json_object_object_add(paramJobj, "INTF_RAC", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_RAC")));
		json_object_object_add(paramJobj, "INTF_BSIC", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_BSIC")));
		json_object_object_add(paramJobj, "INTF_SINR", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_SINR")));
		json_object_object_add(paramJobj, "INTF_CQI", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_CQI")));
		json_object_object_add(paramJobj, "INTF_MCS", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_MCS")));
		json_object_object_add(paramJobj, "INTF_RI", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_RI")));
		json_object_object_add(paramJobj, "INTF_PMI", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_PMI")));
		json_object_object_add(paramJobj, "INTF_Module_Software_Version", JSON_OBJ_COPY(json_object_object_get(intfObj, "X_ZYXEL_ModuleSoftwareVersion")));
		zcfgFeJsonObjFree(intfObj);
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_USIM, &iid, &intfUsimObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "USIM_Status", JSON_OBJ_COPY(json_object_object_get(intfUsimObj, "Status")));
		json_object_object_add(paramJobj, "USIM_IMSI", JSON_OBJ_COPY(json_object_object_get(intfUsimObj, "IMSI")));
		json_object_object_add(paramJobj, "USIM_ICCID", JSON_OBJ_COPY(json_object_object_get(intfUsimObj, "ICCID")));
		json_object_object_add(paramJobj, "USIM_PIN_Protection", JSON_OBJ_COPY(json_object_object_get(intfUsimObj, "X_ZYXEL_PIN_Protection")));
		json_object_object_add(paramJobj, "USIM_PIN_Remaining_Attempts", JSON_OBJ_COPY(json_object_object_get(intfUsimObj, "X_ZYXEL_PIN_RemainingAttempts")));
		zcfgFeJsonObjFree(intfUsimObj);
	}
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_ZY_IP_PASS_THRU, &iid, &intfZyIpPassthruObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "Passthru_Enable", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "Enable")));
		json_object_object_add(paramJobj, "Passthru_Mode", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "ConnectionMode")));
		json_object_object_add(paramJobj, "Passthru_MacAddr", JSON_OBJ_COPY(json_object_object_get(intfZyIpPassthruObj, "MACAddress")));
		zcfgFeJsonObjFree(intfZyIpPassthruObj);
	}
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NR_NSA
	if((ret = zcfgFeObjJsonGet(RDM_OID_CELL_INTF_ZY_NR_NSA, &iid, &intfZyNrNsaObj)) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "NSA_Enable", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "Enable")));
		json_object_object_add(paramJobj, "NSA_MCC", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "MCC")));
		json_object_object_add(paramJobj, "NSA_MNC", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "MNC")));
		json_object_object_add(paramJobj, "NSA_PCI", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "PhyCellID")));
		json_object_object_add(paramJobj, "NSA_RFCN", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "RFCN")));
		json_object_object_add(paramJobj, "NSA_Band", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "Band")));
		json_object_object_add(paramJobj, "NSA_RSSI", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "RSSI")));
		json_object_object_add(paramJobj, "NSA_RSRP", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "RSRP")));
		json_object_object_add(paramJobj, "NSA_RSRQ", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "RSRQ")));
		json_object_object_add(paramJobj, "NSA_SINR", JSON_OBJ_COPY(json_object_object_get(intfZyNrNsaObj, "SINR")));
		zcfgFeJsonObjFree(intfZyNrNsaObj);
	}
#endif

	IID_INIT(iid);
	intfZySccJarray = json_object_new_array();
	while(zcfgFeObjJsonGetNext(RDM_OID_CELL_INTF_ZY_SCC, &iid, &intfZySccObj) == ZCFG_SUCCESS) {
		intfZySccEnable = json_object_get_boolean(json_object_object_get(intfZySccObj, "Enable"));
#ifdef TELIA_CUSTOMIZATION	//Add frequency information
		bands_without_info = json_object_get_string(json_object_object_get(intfZySccObj, "Band"));
		
		if (!strcmp(bands_without_info, "LTE_BC1")){
			snprintf(bands_buffer, sizeof(bands_buffer), "%s", "LTE_BC1 (LTE 2100)");
			json_object_object_add(intfZySccObj, "Band", json_object_new_string(bands_buffer));
		}
		else if (!strcmp(bands_without_info, "LTE_BC3")){
			snprintf(bands_buffer, sizeof(bands_buffer), "%s", "LTE_BC3 (LTE 1800+)");
			json_object_object_add(intfZySccObj, "Band", json_object_new_string(bands_buffer));
		}
		else if (!strcmp(bands_without_info, "LTE_BC7")){
			snprintf(bands_buffer, sizeof(bands_buffer), "%s", "LTE_BC7 (LTE 2600)");
			json_object_object_add(intfZySccObj, "Band", json_object_new_string(bands_buffer));
		}
		else if (!strcmp(bands_without_info, "LTE_BC20")){
			snprintf(bands_buffer, sizeof(bands_buffer), "%s", "LTE_BC20 (LTE 800 DD)");
			json_object_object_add(intfZySccObj, "Band", json_object_new_string(bands_buffer));
		}
		else if (!strcmp(bands_without_info, "LTE_BC28")){
			snprintf(bands_buffer, sizeof(bands_buffer), "%s", "LTE_BC28 (LTE 700 APT)");
			json_object_object_add(intfZySccObj, "Band", json_object_new_string(bands_buffer));
		}
#endif					
		if(intfZySccEnable)
			json_object_array_add(intfZySccJarray, JSON_OBJ_COPY(intfZySccObj));
		zcfgFeJsonObjFree(intfZySccObj);
	}
	json_object_object_add(paramJobj, "SCC_Info", intfZySccJarray);

#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NBR_CELLS
	IID_INIT(iid);
	intfZyNbrJarray = json_object_new_array();
	while(zcfgFeObjJsonGetNext(RDM_OID_CELL_INTF_ZY_NBR_CELL, &iid, &intfZyNbrObj) == ZCFG_SUCCESS) {
		intfZyNbrEnable = json_object_get_boolean(json_object_object_get(intfZyNbrObj, "Enable"));
		if(intfZyNbrEnable)
			json_object_array_add(intfZyNbrJarray, JSON_OBJ_COPY(intfZyNbrObj));
		zcfgFeJsonObjFree(intfZyNbrObj);
	}
	json_object_object_add(paramJobj, "NBR_Info", intfZyNbrJarray);
#endif
	json_object_array_add(Jarray, paramJobj);

	return ret;
}


void zcfgFeDalShowCellWanStatus(struct json_object *Jarray){
	struct json_object *obj = NULL;
	struct json_object *intfZySccObj = NULL;
	int idxOfScc;
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NBR_CELLS
	struct json_object *intfZyNbrObj = NULL;
	int idxOfNbr;
#endif

	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
		
	printf("\nCellular WAN Status:\n");
	printf("%-30s %-10s \n","IMEI:", json_object_get_string(json_object_object_get(obj, "INTF_IMEI")));
	printf("%-30s %-10s \n","Module Software Version:", json_object_get_string(json_object_object_get(obj, "INTF_Module_Software_Version")));
	printf("%-30s %-10s \n","Status:", json_object_get_string(json_object_object_get(obj, "INTF_Status")));
	printf("%-30s %-10s \n","Current Access Technology:", json_object_get_string(json_object_object_get(obj, "INTF_Current_Access_Technology")));
	printf("%-30s %-10s \n","Network In Use:", json_object_get_string(json_object_object_get(obj, "INTF_Network_In_Use")));	
#ifdef TELIA_CUSTOMIZATION
	if (!strcmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")), "LTE_BC1"))
		printf("%-30s %-10s \n","Current Band:", "LTE_BC1 (LTE 2100)");
	else if (!strcmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")), "LTE_BC3"))
		printf("%-30s %-10s \n","Current Band:", "LTE_BC3 (LTE 1800+)");
	else if (!strcmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")), "LTE_BC7"))
		printf("%-30s %-10s \n","Current Band:", "LTE_BC7 (LTE 2600)");
	else if (!strcmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")), "LTE_BC20"))
		printf("%-30s %-10s \n","Current Band:", "LTE_BC20 (LTE 800 DD)");
	else if (!strcmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")), "LTE_BC28"))
		printf("%-30s %-10s \n","Current Band:", "LTE_BC28 (LTE 700 APT)");
#else	
	printf("%-30s %-10s \n","Current Band:", json_object_get_string(json_object_object_get(obj, "INTF_Current_Band")));
#endif
	if(-120 == json_object_get_int(json_object_object_get(obj, "INTF_RSSI")))
		printf("%-30s %-10s \n","RSSI:", "N/A");
	else
		printf("%-30s %-10s \n","RSSI:", json_object_get_string(json_object_object_get(obj, "INTF_RSSI")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_Cell_ID")))
		printf("%-30s %-10s \n","Cell ID:", "N/A");
	else
		printf("%-30s %-10s \n","Cell ID:", json_object_get_string(json_object_object_get(obj, "INTF_Cell_ID")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_PhyCell_ID")))
		printf("%-30s %-10s \n","Physical Cell ID:", "N/A");
	else
		printf("%-30s %-10s \n","Physical Cell ID:", json_object_get_string(json_object_object_get(obj, "INTF_PhyCell_ID")));
	switch(json_object_get_int(json_object_object_get(obj, "INTF_Downlink_Bandwidth"))){
		case 0:
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "1.4");
			break;
		case 1: 
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "3");
			break;
		case 2: 
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "5");
			break;
		case 3: 
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "10");
			break;
		case 4: 
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "15");
			break;
		case 5: 
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "20");
			break;
		default:
			printf("%-30s %-10s \n","DL Bandwidth (MHz):", "N/A");
	}
	printf("%-30s %-10s \n","RFCN:", json_object_get_string(json_object_object_get(obj, "INTF_RFCN")));
	if(-140 == json_object_get_int(json_object_object_get(obj, "INTF_RSRP")))
		printf("%-30s %-10s \n","RSRP:", "N/A");
	else
		printf("%-30s %-10s \n","RSRP:", json_object_get_string(json_object_object_get(obj, "INTF_RSRP")));
	if(-240 == json_object_get_int(json_object_object_get(obj, "INTF_RSRQ")))
		printf("%-30s %-10s \n","RSRQ:", "N/A");
	else
		printf("%-30s %-10s \n","RSRQ:", json_object_get_string(json_object_object_get(obj, "INTF_RSRQ")));
	if(-120 == json_object_get_int(json_object_object_get(obj, "INTF_RSCP")))
		printf("%-30s %-10s \n","RSCP:", "N/A");
	else
		printf("%-30s %-10s \n","RSCP:", json_object_get_string(json_object_object_get(obj, "INTF_RSCP")));
	if(-240 == json_object_get_int(json_object_object_get(obj, "INTF_EcNo")))
		printf("%-30s %-10s \n","EcNo:", "N/A");
	else
		printf("%-30s %-10s \n","EcNo:", json_object_get_string(json_object_object_get(obj, "INTF_EcNo")));
	if(-20 == json_object_get_int(json_object_object_get(obj, "INTF_SINR")))
		printf("%-30s %-10s \n","SINR:", "N/A");
	else
		printf("%-30s %-10s \n","SINR:", json_object_get_string(json_object_object_get(obj, "INTF_SINR")));
	printf("%-30s %-10s \n","TAC:", json_object_get_string(json_object_object_get(obj, "INTF_TAC")));
	printf("%-30s %-10s \n","LAC:", json_object_get_string(json_object_object_get(obj, "INTF_LAC")));
	printf("%-30s %-10s \n","RAC:", json_object_get_string(json_object_object_get(obj, "INTF_RAC")));
	printf("%-30s %-10s \n","BSIC:", json_object_get_string(json_object_object_get(obj, "INTF_BSIC")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_CQI")))
		printf("%-30s %-10s \n","CQI:", "N/A");
	else
		printf("%-30s %-10s \n","CQI:", json_object_get_string(json_object_object_get(obj, "INTF_CQI")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_MCS")))
		printf("%-30s %-10s \n","MCS:", "N/A");
	else
		printf("%-30s %-10s \n","MCS:", json_object_get_string(json_object_object_get(obj, "INTF_MCS")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_RI")))
		printf("%-30s %-10s \n","RI:", "N/A");
	else
		printf("%-30s %-10s \n","RI:", json_object_get_string(json_object_object_get(obj, "INTF_RI")));
	if(-1 == json_object_get_int(json_object_object_get(obj, "INTF_PMI")))
		printf("%-30s %-10s \n","PMI:", "N/A");
	else
		printf("%-30s %-10s \n","PMI:", json_object_get_string(json_object_object_get(obj, "INTF_PMI")));

	if(!strncmp(json_object_get_string(json_object_object_get(obj, "INTF_Current_Access_Technology")),"LTE-A",5)){
	//Show SCC information if SCC is enabled
		idxOfScc = 0;
		while((intfZySccObj = json_object_array_get_idx(json_object_object_get(obj, "SCC_Info"), idxOfScc)) != NULL){
			printf("\nSCC %d information:\n",idxOfScc+1);

			if(-1 == json_object_get_int(json_object_object_get(intfZySccObj, "PhysicalCellID")))
				printf("%-30s %-10s \n","Physical Cell ID:", "N/A");
			else
				printf("%-30s %-10s \n","Physical Cell ID:", json_object_get_string(json_object_object_get(intfZySccObj, "PhysicalCellID")));
			printf("%-30s %-10s \n","RFCN:", json_object_get_string(json_object_object_get(intfZySccObj, "RFCN")));
			printf("%-30s %-10s \n","Band:", json_object_get_string(json_object_object_get(intfZySccObj, "Band")));
		
			if(-120 == json_object_get_int(json_object_object_get(intfZySccObj, "RSSI")))
				printf("%-30s %-10s \n","RSSI:", "N/A");
			else
				printf("%-30s %-10s \n","RSSI:", json_object_get_string(json_object_object_get(intfZySccObj, "RSSI")));
			if(-140 == json_object_get_int(json_object_object_get(intfZySccObj, "RSRP")))
				printf("%-30s %-10s \n","RSRP:", "N/A");
			else
				printf("%-30s %-10s \n","RSRP:", json_object_get_string(json_object_object_get(intfZySccObj, "RSRP")));
			if(-240 == json_object_get_int(json_object_object_get(intfZySccObj, "RSRQ")))
				printf("%-30s %-10s \n","RSRQ:", "N/A");
			else
				printf("%-30s %-10s \n","RSRQ:", json_object_get_string(json_object_object_get(intfZySccObj, "RSRQ")));
			if(-20 == json_object_get_int(json_object_object_get(intfZySccObj, "SINR")))
				printf("%-30s %-10s \n","SINR:", "N/A");
			else
				printf("%-30s %-10s \n","SINR:", json_object_get_string(json_object_object_get(intfZySccObj, "SINR")));
			idxOfScc++;
		}
	}
#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_NBR_CELLS
	idxOfNbr = 0;
	char NbrType_buffer[128];
	while((intfZyNbrObj = json_object_array_get_idx(json_object_object_get(obj, "NBR_Info"), idxOfNbr)) != NULL){
#if 0 
		printf("\nLTE Neighbour list  %d information:\n",idxOfNbr+1);
		printf("%-30s %-10s \n","Neighbour Type:", json_object_get_string(json_object_object_get(intfZyNbrObj, "NeighbourType")));
		printf("%-30s %-10s \n","Physical Cell ID:", json_object_get_string(json_object_object_get(intfZyNbrObj, "PhyCellID")));
		printf("%-30s %-10s \n","RFCN:", json_object_get_string(json_object_object_get(intfZyNbrObj, "RFCN")));
		printf("%-30s %-10s \n","RSSI:", json_object_get_string(json_object_object_get(intfZyNbrObj, "RSSI")));
		printf("%-30s %-10s \n","RSRP:", json_object_get_string(json_object_object_get(intfZyNbrObj, "RSRP")));
		printf("%-30s %-10s \n","RSRQ:", json_object_get_string(json_object_object_get(intfZyNbrObj, "RSRQ")));
#else
		if (idxOfNbr == 0) {
			printf("Neighbour list information:\n");
			printf("%-5s %-20s %-10s %-15s %-10s %-10s %-10s %-5s\n", "#", "Neighbour Type", "Mode", "PhyCellID", "RFCN", "RSSI", "RSRP", "RSRQ");
		}

		if (strstr(json_object_get_string(json_object_object_get(intfZyNbrObj, "NeighbourType")),"intra") != NULL)
			snprintf(NbrType_buffer, sizeof(NbrType_buffer), "%s", "Intra-Frequency");
                else if (strstr(json_object_get_string(json_object_object_get(intfZyNbrObj, "NeighbourType")),"inter") != NULL)
			snprintf(NbrType_buffer, sizeof(NbrType_buffer), "%s", "Inter-Frequency");
		else
			snprintf(NbrType_buffer, sizeof(NbrType_buffer), "%s", json_object_get_string(json_object_object_get(intfZyNbrObj, "NeighbourType")));

		printf("%-5d %-20s %-10s %-15s %-10s %-10s %-10s %-5s\n", idxOfNbr+1 , 
			NbrType_buffer,
			json_object_get_string(json_object_object_get(intfZyNbrObj, "ConnectionMode")),
			json_object_get_string(json_object_object_get(intfZyNbrObj, "PhyCellID")),
			json_object_get_string(json_object_object_get(intfZyNbrObj, "RFCN")),
			json_object_get_string(json_object_object_get(intfZyNbrObj, "RSSI")),
			json_object_get_string(json_object_object_get(intfZyNbrObj, "RSRP")),
			json_object_get_string(json_object_object_get(intfZyNbrObj, "RSRQ")));
#endif
		idxOfNbr++;
	}
#endif
}


zcfgRet_t zcfgFeDalCellWanStatus(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanStatusGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}

