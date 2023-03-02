#include <ctype.h>
#include <json/json.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"

#ifdef ZYXEL_WEB_GUI_CELLWAN_SUPPORT_GNSS_LOCATION

dal_param_t CELLWAN_GNSS_param[]={
{"GNSS_Enable", dalType_boolean, 0, 0, NULL},
{"GNSS_ScanOnBoot", dalType_boolean, 0, 0, NULL},
{"GNSS_DisplayFormat", dalType_int, 0, 2, NULL},
{ NULL,	0,	0,	0,	NULL}

};

zcfgRet_t zcfgFeDalCellWanGnssEdit(struct json_object *Jobj, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *CellZyGnssLocObj = NULL;
	objIndex_t CellZyGnssLocIid = {0};
	bool GnssEnable = false;
	bool GnssScanOnBoot = false;	
	int GnssDisplayFormat = 0;

	IID_INIT(CellZyGnssLocIid);

	GnssEnable = json_object_get_boolean(json_object_object_get(Jobj, "GNSS_Enable"));
	GnssScanOnBoot = json_object_get_boolean(json_object_object_get(Jobj, "GNSS_ScanOnBoot"));
	GnssDisplayFormat = json_object_get_int(json_object_object_get(Jobj, "GNSS_DisplayFormat"));

	if((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_CELL_ZY_GNSS_LOC, &CellZyGnssLocIid, &CellZyGnssLocObj)) == ZCFG_SUCCESS){
		if(json_object_object_get(Jobj, "GNSS_Enable"))
			json_object_object_add(CellZyGnssLocObj, "Enable", json_object_new_boolean(GnssEnable));
		if(json_object_object_get(Jobj, "GNSS_ScanOnBoot"))
			json_object_object_add(CellZyGnssLocObj, "ScanOnBoot", json_object_new_boolean(GnssScanOnBoot));
		if(json_object_object_get(Jobj, "GNSS_DisplayFormat"))
			json_object_object_add(CellZyGnssLocObj, "DisplayFormat", json_object_new_int(GnssDisplayFormat));
		zcfgFeObjJsonSet(RDM_OID_CELL_ZY_GNSS_LOC, &CellZyGnssLocIid, CellZyGnssLocObj, NULL);
	}

	if (CellZyGnssLocObj) zcfgFeJsonObjFree(CellZyGnssLocObj);

	return ret;
}	

zcfgRet_t zcfgFeDalCellWanGnssGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;
	struct json_object *paramJobj = NULL, *CellZyGnssLocObj = NULL;
	objIndex_t CellZyGnssLocIid = {0};
	paramJobj = json_object_new_object();

	IID_INIT(CellZyGnssLocIid);

	if(zcfgFeObjJsonGet(RDM_OID_CELL_ZY_GNSS_LOC, &CellZyGnssLocIid, &CellZyGnssLocObj) == ZCFG_SUCCESS){
		json_object_object_add(paramJobj, "GNSS_Enable", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "Enable")));
		json_object_object_add(paramJobj, "GNSS_ScanOnBoot", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "ScanOnBoot")));
		json_object_object_add(paramJobj, "GNSS_DisplayFormat", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "DisplayFormat")));

		json_object_object_add(paramJobj, "GNSS_ScanStatus", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "ScanStatus")));
		json_object_object_add(paramJobj, "GNSS_Latitude", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "Latitude")));
		json_object_object_add(paramJobj, "GNSS_Longitude", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "Longitude")));
		json_object_object_add(paramJobj, "GNSS_HDOP", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "HDOP")));
		json_object_object_add(paramJobj, "GNSS_Elevation", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "Elevation")));
		json_object_object_add(paramJobj, "GNSS_PositioningMode", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "PositioningMode")));
		json_object_object_add(paramJobj, "GNSS_CourseOverGround", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "CourseOverGround")));
		json_object_object_add(paramJobj, "GNSS_SpeedOverGround", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "SpeedOverGround")));
		json_object_object_add(paramJobj, "GNSS_LastFixTime", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "LastFixTime")));
		json_object_object_add(paramJobj, "GNSS_NumberOfSatellites", JSON_OBJ_COPY(json_object_object_get(CellZyGnssLocObj, "NumberOfSatellites")));

		zcfgFeJsonObjFree(CellZyGnssLocObj);
	}

	json_object_array_add(Jarray, paramJobj);
	return ret;
}

void zcfgFeDalShowCellWanGnss(struct json_object *Jarray){
	struct json_object *obj = NULL;

	double D_Latitude = 0;
	double D_Longitude = 0; 
	double D_Elevation = 0;
	double m=1000000;
	double n=10;
	int status = -1;
	int position = 0;
	
	if(json_object_get_type(Jarray) != json_type_array){
		printf("wrong Jobj format!\n");
		return;
	}

	obj = json_object_array_get_idx(Jarray, 0);
	
	printf("\nCellular GNSS location information:\n");
#if 1 //Phase II showing
	if(json_object_get_boolean(json_object_object_get(obj, "GNSS_Enable")))
		printf("GNSS function Enable.\n");
	else
		printf("GNSS function Disable.\n");

	printf("%-30s %-10s \n","ScanOnBoot:", json_object_get_boolean(json_object_object_get(obj, "GNSS_ScanOnBoot"))? "Enable":"Disable");
	//printf("%-30s %-10lld \n","DisplayFormat:", json_object_get_int(json_object_object_get(obj, "GNSS_DisplayFormat")));
#endif	
	status = (int)json_object_get_int(json_object_object_get(obj, "GNSS_ScanStatus"));
	if (status < 0 ) 
		printf("%-30s %-10s \n","ScanStatus:", "not fixed");
	else if ( status  == 0 )
		printf("%-30s %-10s \n","ScanStatus:", "fixed");
	else if ( status > 500 )
		printf("%-30s %-10s%d \n","ScanStatus:", "Error code: ", status); 
	else
		printf("%-30s %-10s \n","ScanStatus:", "not fixed - unknown"); 
	
	D_Latitude = (double)json_object_get_int(json_object_object_get(obj, "GNSS_Latitude"));
	D_Longitude = (double)json_object_get_int(json_object_object_get(obj, "GNSS_Longitude"));
	D_Elevation = (double)json_object_get_int(json_object_object_get(obj, "GNSS_Elevation"));

	printf("%-30s %.5f,%.5f,%.1fz \n","GNSS Location:",D_Latitude/m,D_Longitude/m,D_Elevation/n);
	//URL example https://www.google.com/maps/search/?api=1&query=25.033459,121.501280
	printf("%-30s %s%.5f,%.5f\n","Google Map URL:", "https://www.google.com/maps/search/?api=1&query=",D_Latitude/m, D_Longitude/m);
	printf("%-30s %-10.1f \n","Horizontal precision:", (double)json_object_get_int(json_object_object_get(obj, "GNSS_HDOP"))/n);

	position = (int)json_object_get_int(json_object_object_get(obj, "GNSS_PositioningMode"));
	if (position == 2 || position == 3) //EG16 module not support positioning mode.
	printf("%-30s %-10d \n","PositioningMode:", position);

	printf("%-30s %-10.1f \n","CourseOverGround:", (double)json_object_get_int(json_object_object_get(obj, "GNSS_CourseOverGround"))/n);
	printf("%-30s %-10.1f \n","SpeedOverGround:", (double)json_object_get_int(json_object_object_get(obj, "GNSS_SpeedOverGround"))/n);
	printf("%-30s %-10s \n","LastFixTime:", json_object_get_string(json_object_object_get(obj, "GNSS_LastFixTime")));
	printf("%-30s %-10lld \n","NumberOfSatellites:", json_object_get_int(json_object_object_get(obj, "GNSS_NumberOfSatellites")));
		
}


zcfgRet_t zcfgFeDalCellWanGnss(const char *method, struct json_object *Jobj,  struct json_object *Jarray, char *replyMsg){
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "PUT"))
		ret = zcfgFeDalCellWanGnssEdit(Jobj, NULL);
	else if(!strcmp(method, "GET"))
		ret = zcfgFeDalCellWanGnssGet(Jobj, Jarray, NULL);
	else
		printf("Unknown method:%s\n", method);

	return ret;
}
#endif
