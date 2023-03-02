#include <ctype.h>
#include <json/json.h>
//#include <json/json_object.h>
#include <time.h>
#include <string.h>

#include "zcfg_common.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_rdm_struct.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
//#include "zcfg_eid.h"
#include "zcfg_msg.h"
#include "zcfg_fe_dal_common.h"
#include "zcfg_fe_Qos.h"
#include <zos_api.h>

#define ETHTYPE_NA        -1
#define ETHTYPE_IP        2048
#define ETHTYPE_ARP       2054
#define ETHTYPE_PPPOEDISC 34915
#define ETHTYPE_PPPOESES  34916
#define ETHTYPE_8021Q     33024
#define ETHTYPE_IPV6      34525
#ifdef ABUU_CUSTOMIZATION_QOS
#define ETHTYPE_IP_IPV6   36573
#endif

dal_param_t QosClass_param[] = {
	{"idx",										dalType_int,				0, 0, 		NULL,NULL,													0},
	{"Index",									dalType_int, 				0, 0, 		NULL,NULL,													dal_Add_ignore|dal_Edit|dal_Delete},
	{"Queue",									dalType_QueueList,			0, 0, 		NULL,NULL,													0},
	{"Enable",									dalType_boolean,			0, 0, 		NULL,NULL,													dal_Add},
	{"Name",									dalType_string, 			0, 0,		NULL,NULL,													dal_Add},
	{"Order",									dalType_int, 				0, 0, 		NULL,NULL,													dal_Add},
	{"Interface",								dalType_ClassIntf, 			0, 0, 		NULL,NULL,													dal_Add},
	{"TargetInterface",							dalType_string, 			0, 0, 		NULL,NULL,													0},
#ifdef ABUU_CUSTOMIZATION_QOS
    {"Ethertype",                               dalType_string,             0, 0,       NULL,"NA|IP|ARP|PPPoE_DISC|PPPoE_SES|802.1Q|IPv6|IP_IPv6",  dal_Add},
#else
	{"Ethertype",								dalType_string, 			0, 0, 		NULL,"NA|IP|ARP|PPPoE_DISC|PPPoE_SES|802.1Q|IPv6",			dal_Add},
#endif
	{"SourceIPEnable",							dalType_boolean,			0, 0, 		NULL,NULL,													0},
	{"SourceIP",								dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceMask",								dalType_string, 			0, 0,		NULL,NULL,													0},
	{"SourceIPPrefix",							dalType_v6AddrPrefix, 		0, 0, 		NULL,NULL,													0},
	{"SourceIPExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"SourcePortEnable",						dalType_boolean,			0, 0, 		NULL,NULL,													0},
	{"SourcePort",								dalType_int, 				-1,65535, 	NULL,NULL,													dalcmd_Forbid},
	{"SourcePortRangeMax",						dalType_int, 				-1,65535, 	NULL,NULL,													dalcmd_Forbid},
	{"SourcePortRange",							dalType_PortRange, 			0, 0, 	    NULL,NULL,													0},
	{"SourcePortExclude",						dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"SourceMACEnable",							dalType_boolean,			0, 0, 		NULL,NULL,													0},
	{"SourceMACAddress",						dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceMACMask",							dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceMACExclude",						dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DestIPEnable",							dalType_boolean,			0, 0, 		NULL,NULL,													0},
	{"DestIP",									dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"DestMask",								dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"DestIPPrefix",							dalType_v6AddrPrefix, 		0, 0, 		NULL,NULL,													0},
	{"DestIPExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DestPortEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DestPort",								dalType_int, 				-1,65535, 	NULL,NULL,													dalcmd_Forbid},
	{"DestPortRangeMax",						dalType_int, 				-1,65535, 	NULL,NULL,													dalcmd_Forbid},
	{"DestPortRange",							dalType_PortRange, 			0, 0, 	    NULL,NULL,													0},
	{"DestPortExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DestMACEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DestMACAddress",							dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"DestMACMask",								dalType_string,				0, 0, 		NULL,NULL,													0},
	{"DestMACExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"ServiceEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"Service",									dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"ServiceExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"ProtocolEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"Protocol",								dalType_string,			    0, 0, 		NULL,"ICMP|IGMP|TCP|UDP|UserDefined",						0},
	{"ProtocolNumber",							dalType_int,			    0, 255, 	NULL,NULL,													0},
	{"ProtocolExclude",							dalType_boolean,    		0, 0, 		NULL,NULL,													0},
	{"DhcpEnable",								dalType_boolean,			0, 0, 		NULL,NULL,                            						0},
	{"Dhcp",									dalType_string,			    0, 0, 		NULL,"op60|op61|op77|op125",								0},
	{"SourceVendorClassID",						dalType_string,             0, 0, 		NULL,NULL,													0},
	{"SourceVendorClassIDExclude",				dalType_boolean,            0, 0, 		NULL,NULL,													0},
	{"SourceClientIDType",						dalType_string,             0, 0, 		NULL,NULL,													0},
	{"SourceClientID",							dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceClientIDExclude",					dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"SourceUserClassID",						dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceUserClassIDExclude",				dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"SourceVendorSpecificInfoEnterprise",		dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceVendorSpecificInfo",				dalType_string, 			0, 0, 		NULL,NULL,													0},
	{"SourceVendorSpecificInfoExclude",			dalType_boolean,		    0, 0, 		NULL,NULL,													0},
	{"IPLengthEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"IPLengthMin",								dalType_int, 				20,1500, 	NULL,NULL,													0},
	{"IPLengthMax",								dalType_int, 				20,1500, 	NULL,NULL,													0},
	{"IPLengthExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DSCPCheckEnable",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DSCPCheck",								dalType_int, 				0, 63, 		NULL,NULL,													0},
	{"DSCPExclude",								dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"EthernetPriorityEnable",					dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"EthernetPriorityCheck",					dalType_int, 				0, 7, 		NULL,NULL,													0},
	{"EthernetPriorityExclude",					dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"VLANIDCheckEnable",						dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"VLANIDCheck",								dalType_int, 				1, 4094, 	NULL,NULL,													0},
	{"VLANIDExclude",							dalType_boolean,			0, 0, 		NULL,NULL,													0},
	{"TCPACK",									dalType_int, 				0, 0, 		NULL,NULL,													0},
	{"TCPACKExclude",							dalType_boolean, 			0, 0, 		NULL,NULL,													0},
	{"DSCPMark",								dalType_int, 				-1,63,		NULL,NULL,													0},
#ifndef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	{"VLANIDAction",							dalType_int, 				0, 3, 		NULL,NULL,													0},
	{"VLANIDMark",								dalType_int, 				1, 4094, 	NULL,NULL,													0},
#endif
	{"EthernetPriorityMark",					dalType_int, 				-1,7, 		NULL,NULL,													0},
	{"ForwardingPolicy",						dalType_RouteIntf, 			0, 0, 		NULL,NULL,													0},
	{"defaultQ",								dalType_boolean, 			0, 0, 		NULL,NULL,													0},	
	{NULL,										0,							0, 0,		NULL,NULL,													0}
};


typedef struct s_QosClass{
	int Index;
	int Queue; // the queue id which the class belong to
	bool Enable;
	char *Name;
	/* order */
	int Order;
	/* from interface */
	char *Interface;
	/* target interface */
	char *TargetInterface;
	/* ether type*/
	int Ethertype;
	/* source addr */
	char *SourceIP;
	char *SourceMask;
	bool SourceIPExclude;
	/* source port*/
	int	SourcePort;
	int SourcePortRangeMax;
	bool SourcePortExclude;
	/* source Mac */
	char *SourceMACAddress;
	char *SourceMACMask;
	bool SourceMACExclude;
	/* destination addr */
	char *DestIP;
	char *DestMask;
	bool DestIPExclude;
	/* destination port */
	int DestPort;
	int DestPortRangeMax;
	bool DestPortExclude;
	/* destination Mac */
	char *DestMACAddress;
	char *DestMACMask;
	bool DestMACExclude;
	/* service */
	char *Service;
	bool ServiceExclude;
	/* protocol */
	int Protocol;
	bool ProtocolExclude;
	/* dhcp options */
	char *Dhcp;
	char *SourceVendorClassID;
	bool SourceVendorClassIDExclude;
	char *SourceClientIDType;
	char *SourceClientID;
	bool SourceClientIDExclude;
	char *SourceUserClassID;
	bool SourceUserClassIDExclude;
	int SourceVendorSpecificInfoEnterprise;
	char *SourceVendorSpecificInfo;
	bool SourceVendorSpecificInfoExclude;
	/* packet length */
	int IPLengthMin;
	int IPLengthMax;
	bool IPLengthExclude;
	/* DSCP check */
	int DSCPCheck;
	bool DSCPExclude;
	/* vlan prio check */
	int EthernetPriorityCheck;
	bool EthernetPriorityExclude;
	/* vlan Id check */
	int VLANIDCheck;
	bool VLANIDExclude;
	/* tcp ack */
	bool TCPACK;
	bool TCPACKExclude;
	/* DSCP mark */
	int DSCPMark;
	/* vlan mark */
	int VLANIDAction;
	int EthernetPriorityMark;
	int VLANIDMark;
	/* class route */
	int ForwardingPolicy;
	int TrafficClass;
	bool defaultQ;
	/* enable check*/
	bool IPLengthEnable;
	bool DSCPCheckEnable;
	bool EthernetPriorityEnable;
	bool VLANIDCheckEnable;
	bool SourceIPEnable;
	bool SourcePortEnable;
	bool SourceMACEnable;
	bool DestIPEnable;
	bool DestPortEnable;
	bool DestMACEnable;
	bool ServiceEnable;
	bool ProtocolEnable;
	bool DhcpEnable;
} s_QosClass;

typedef struct s_QosClassDel {
	int Index;
}s_QosClassDel;

extern uint32_t StrToIntConv(char *str);

/*!
 *  Copy values from Jobj to ClassObj_Info
 *  @param[in]     method            "POST" or "PUT"
 *  @param[in]     Jobj              Request object from GUI or CLI
 *  @param[out]    ClassObj_Info     Output object
 *
 *  @return        ZCFG_SUCCESS  - success.
 *                 ZCFG_INVALID_PARAM_VALUE - parameter value invalid.
 *  @note          Some parameters (e.g. Ethertype) are validated in this function
 */
zcfgRet_t getQosClassBasicInfo(const char *method, struct json_object *Jobj, s_QosClass *ClassObj_Info)
{
	zcfgRet_t ret = ZCFG_SUCCESS;
	int tmp_idx = 0;
	char *tmp1 = NULL, *tmp2 = NULL, *op125 = NULL, *tmp3 = NULL;
	char *sourceaddr = NULL, *destaddr = NULL, *protocol = NULL, *srcport = NULL,*dstport = NULL;
	char *routeintf = NULL, *queue = NULL;
	char *ethertype = NULL;
	char *clear = "";
	int enterprise = 0;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};

	IID_INIT(classIid);
	if(json_object_object_get(Jobj, "idx")) { // treat as Order number
		tmp_idx = json_object_get_int(json_object_object_get(Jobj, "idx"));
		if(tmp_idx >= 0){
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
				if(tmp_idx == json_object_get_int(json_object_object_get(classObj, "Order"))) {
					ClassObj_Info->Index = classIid.idx[0];
					zcfgFeJsonObjFree(classObj);
					break;
				}
				zcfgFeJsonObjFree(classObj);
			}
		}
		else{
			ClassObj_Info->Index = tmp_idx;
		}
	}
	else if(json_object_object_get(Jobj, "iid")) {
		ClassObj_Info->Index = json_object_get_int(json_object_object_get(Jobj, "iid"));
	}
	else if(json_object_object_get(Jobj, "Index")) {
		tmp_idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
		if(convertIndextoIid(tmp_idx, &classIid, RDM_OID_QOS_CLS, NULL, NULL, NULL) == ZCFG_SUCCESS) {
			ClassObj_Info->Index = classIid.idx[0];
		}
	}
	else {
		printf("%s(): fail to get index\n", __FUNCTION__);
	}

	if(json_object_object_get(Jobj, "defaultQ")){
		ClassObj_Info->Queue = json_object_get_int(json_object_object_get(Jobj, "Queue"));
		ClassObj_Info->defaultQ = json_object_get_boolean(json_object_object_get(Jobj, "defaultQ"));
	}
	else{
		if(json_object_object_get(Jobj, "Queue")){
			queue = (char *)json_object_get_string(json_object_object_get(Jobj, "Queue"));
			if(!strcmp(queue,"0"))
				ClassObj_Info->defaultQ = true;
			else
				ClassObj_Info->defaultQ = false;
			sscanf(queue, "%d", &(ClassObj_Info->Queue));
		}
		else{
			if(!strcmp(method,"POST")){
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
				ClassObj_Info->defaultQ = false;
				ClassObj_Info->Queue = 4;
#else
				ClassObj_Info->defaultQ = true;
				ClassObj_Info->Queue = 0;
#endif
			}
		}
	}
	if(json_object_object_get(Jobj, "Enable"))
		ClassObj_Info->Enable = json_object_get_boolean(json_object_object_get(Jobj, "Enable"));
	if(json_object_object_get(Jobj, "Name"))
		ClassObj_Info->Name = (char*)json_object_get_string(json_object_object_get(Jobj, "Name"));
	else
		ClassObj_Info->Name = NULL;
	if(json_object_object_get(Jobj, "Order"))
		ClassObj_Info->Order = json_object_get_int(json_object_object_get(Jobj, "Order"));
	if(json_object_object_get(Jobj, "Interface"))
		ClassObj_Info->Interface = (char*)json_object_get_string(json_object_object_get(Jobj, "Interface"));
#if defined(ZYXEL_WEB_GUI_SHOW_QOS_CLS_TARGET_INTF) || defined(ABUU_CUSTOMIZATION_QOS)
	if(json_object_object_get(Jobj, "TargetInterface"))
		ClassObj_Info->TargetInterface = (char*)json_object_get_string(json_object_object_get(Jobj, "TargetInterface"));
#endif
	if(json_object_object_get(Jobj, "Ethertype")){
		ethertype = (char *)json_object_get_string(json_object_object_get(Jobj, "Ethertype"));
		if(!strcmp(ethertype,"NA"))			
			ClassObj_Info->Ethertype = ETHTYPE_NA;
		else if(!strcmp(ethertype,"IP"))					
			ClassObj_Info->Ethertype = ETHTYPE_IP;
		else if(!strcmp(ethertype,"ARP"))			
			ClassObj_Info->Ethertype = ETHTYPE_ARP;
		else if(!strcmp(ethertype,"PPPoE_DISC"))		
			ClassObj_Info->Ethertype = ETHTYPE_PPPOEDISC;
		else if(!strcmp(ethertype,"PPPoE_SES"))					
			ClassObj_Info->Ethertype = ETHTYPE_PPPOESES;
		else if(!strcmp(ethertype,"802.1Q"))					
			ClassObj_Info->Ethertype = ETHTYPE_8021Q;
		else if(!strcmp(ethertype,"IPv6"))				
			ClassObj_Info->Ethertype = ETHTYPE_IPV6;
#ifdef ABUU_CUSTOMIZATION_QOS
        else if (!strcmp(ethertype,"IP_IPv6"))
        {
            ClassObj_Info->Ethertype = ETHTYPE_IP_IPV6;
        }
#endif
		else
		{
			dbg_printf("%s:%d: Ethertype (%s) is in DAL enumeration list but there's no corresponding code to process it.\n",__FUNCTION__,__LINE__,ethertype);
			printf("Invalid \"Ethertype\" value %s.\n",ethertype);
			return ZCFG_INVALID_PARAM_VALUE;
		}
	}
	if(json_object_object_get(Jobj, "SourceIPEnable")){
		ClassObj_Info->SourceIPEnable = json_object_get_boolean(json_object_object_get(Jobj, "SourceIPEnable"));
		if(ClassObj_Info->SourceIPEnable == true){
			if(json_object_object_get(Jobj, "SourceIPPrefix")){
				tmp1 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceIPPrefix"));
				sourceaddr = strdup(tmp1);
				ClassObj_Info->SourceIP = strtok_r(sourceaddr, "/", &(ClassObj_Info->SourceMask));
			}
			else {
				ClassObj_Info->SourceIP = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceIP"));
				ClassObj_Info->SourceMask = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceMask"));
			}
			ClassObj_Info->SourceIPExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceIPExclude"));
		}
		else{
			ClassObj_Info->SourceIP = clear;
			ClassObj_Info->SourceMask = clear;
			ClassObj_Info->SourceIPExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "SourcePortEnable")){
		ClassObj_Info->SourcePortEnable = json_object_get_boolean(json_object_object_get(Jobj, "SourcePortEnable"));
		if(ClassObj_Info->SourcePortEnable == true){	
			if(json_object_object_get(Jobj, "SourcePortRange")){
				srcport = (char*)json_object_get_string(json_object_object_get(Jobj, "SourcePortRange"));
				if(strchr(srcport, ':') != NULL){
					sscanf(srcport, "%d:%d", &(ClassObj_Info->SourcePort), &(ClassObj_Info->SourcePortRangeMax));
					if(ClassObj_Info->SourcePort == -1)
						ClassObj_Info->SourcePortRangeMax = -1;
				}
				else{
					ClassObj_Info->SourcePort = atoi(srcport);
					ClassObj_Info->SourcePortRangeMax = -1;
				}
			}
			else{
				if(json_object_object_get(Jobj, "SourcePort"))
					ClassObj_Info->SourcePort = json_object_get_int(json_object_object_get(Jobj, "SourcePort"));
				if(json_object_object_get(Jobj, "SourcePortRangeMax"))
					ClassObj_Info->SourcePortRangeMax = json_object_get_int(json_object_object_get(Jobj, "SourcePortRangeMax"));
				else
					ClassObj_Info->SourcePortRangeMax = -1;
			}
			ClassObj_Info->SourcePortExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourcePortExclude"));
		}
		else{
			ClassObj_Info->SourcePort = -1;
			ClassObj_Info->SourcePortRangeMax = -1;
			ClassObj_Info->SourcePortExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "SourceMACEnable")){
		ClassObj_Info->SourceMACEnable = json_object_get_boolean(json_object_object_get(Jobj, "SourceMACEnable"));
		if(ClassObj_Info->SourceMACEnable == true){	
			ClassObj_Info->SourceMACAddress = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceMACAddress"));
			ClassObj_Info->SourceMACMask = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceMACMask"));
			ClassObj_Info->SourceMACExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceMACExclude"));
		}
		else{
			ClassObj_Info->SourceMACAddress = clear;
			ClassObj_Info->SourceMACMask = clear;
			ClassObj_Info->SourceMACExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "DestIPEnable")){
		ClassObj_Info->DestIPEnable = json_object_get_boolean(json_object_object_get(Jobj, "DestIPEnable"));
		if(ClassObj_Info->DestIPEnable == true){
			if(json_object_object_get(Jobj, "DestIPPrefix")){
				tmp2 = (char*)json_object_get_string(json_object_object_get(Jobj, "DestIPPrefix"));
				destaddr = strdup(tmp2);
				ClassObj_Info->DestIP = strtok_r(destaddr, "/", &(ClassObj_Info->DestMask));
			}
			else{
				ClassObj_Info->DestIP = (char*)json_object_get_string(json_object_object_get(Jobj, "DestIP"));
				ClassObj_Info->DestMask = (char*)json_object_get_string(json_object_object_get(Jobj, "DestMask"));
			}
			ClassObj_Info->DestIPExclude = json_object_get_boolean(json_object_object_get(Jobj, "DestIPExclude"));
		}
		else{
			ClassObj_Info->DestIP = clear;
			ClassObj_Info->DestMask = clear;
			ClassObj_Info->DestIPExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "DestPortEnable")){
		ClassObj_Info->DestPortEnable = json_object_get_boolean(json_object_object_get(Jobj, "DestPortEnable"));
		if(ClassObj_Info->DestPortEnable == true){	
			if(json_object_object_get(Jobj, "DestPortRange")){
				dstport = (char*)json_object_get_string(json_object_object_get(Jobj, "DestPortRange"));
				if(strchr(dstport, ':') != NULL){
					sscanf(dstport, "%d:%d", &(ClassObj_Info->DestPort), &(ClassObj_Info->DestPortRangeMax));
					if(ClassObj_Info->DestPort == -1)
						ClassObj_Info->DestPortRangeMax = -1;
				}
				else{
					ClassObj_Info->DestPort = atoi(dstport);
					ClassObj_Info->DestPortRangeMax = -1;
				}
			}
			else{
				if(json_object_object_get(Jobj, "DestPort"))
					ClassObj_Info->DestPort = json_object_get_int(json_object_object_get(Jobj, "DestPort"));
				if(json_object_object_get(Jobj, "DestPortRangeMax"))
					ClassObj_Info->DestPortRangeMax = json_object_get_int(json_object_object_get(Jobj, "DestPortRangeMax"));
				else
					ClassObj_Info->DestPortRangeMax = -1;
			}
			ClassObj_Info->DestPortExclude = json_object_get_boolean(json_object_object_get(Jobj, "DestPortExclude"));
		}
		else{
			ClassObj_Info->DestPort = -1;
			ClassObj_Info->DestPortRangeMax = -1;
			ClassObj_Info->DestPortExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "DestMACEnable")){
		ClassObj_Info->DestMACEnable = json_object_get_boolean(json_object_object_get(Jobj,"DestMACEnable"));
		if(ClassObj_Info->DestMACEnable == true){	
			ClassObj_Info->DestMACAddress = (char*)json_object_get_string(json_object_object_get(Jobj, "DestMACAddress"));
			ClassObj_Info->DestMACMask = (char*)json_object_get_string(json_object_object_get(Jobj, "DestMACMask"));
			ClassObj_Info->DestMACExclude = json_object_get_boolean(json_object_object_get(Jobj, "DestMACExclude"));
		}
		else{
			ClassObj_Info->DestMACAddress = clear;
			ClassObj_Info->DestMACMask = clear;
			ClassObj_Info->DestMACExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "ServiceEnable")){
		ClassObj_Info->ServiceEnable = json_object_get_boolean(json_object_object_get(Jobj, "ServiceEnable"));
		if(ClassObj_Info->ServiceEnable == true){
			ClassObj_Info->Service = (char*)json_object_get_string(json_object_object_get(Jobj, "Service"));
			ClassObj_Info->ServiceExclude = json_object_get_boolean(json_object_object_get(Jobj, "ServiceExclude"));
		}
		else{
			ClassObj_Info->Service = clear;
			ClassObj_Info->ServiceExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "ProtocolEnable")){
		ClassObj_Info->ProtocolEnable = json_object_get_boolean(json_object_object_get(Jobj, "ProtocolEnable"));
		if(ClassObj_Info->ProtocolEnable == true){
			if(json_object_object_get(Jobj, "Protocol")){
				tmp3 = (char*)json_object_get_string(json_object_object_get(Jobj, "Protocol"));
				protocol = strdup(tmp3);
				if(!strcmp(protocol,"ICMP"))
					ClassObj_Info->Protocol = 1;
				else if(!strcmp(protocol,"IGMP"))
					ClassObj_Info->Protocol = 2;
				else if(!strcmp(protocol,"TCP"))
					ClassObj_Info->Protocol = 6;
				else if(!strcmp(protocol,"UDP"))
					ClassObj_Info->Protocol = 17;
				else
					ClassObj_Info->Protocol = json_object_get_int(json_object_object_get(Jobj, "ProtocolNumber"));
				ZOS_FREE(protocol);
			}
			else{
				ClassObj_Info->Protocol = json_object_get_int(json_object_object_get(Jobj, "ProtocolNumber"));
			}
			ClassObj_Info->ProtocolExclude = json_object_get_boolean(json_object_object_get(Jobj, "ProtocolExclude"));
		}
		else{
			ClassObj_Info->Protocol = -1;
			ClassObj_Info->ProtocolExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "DhcpEnable")){
		ClassObj_Info->DhcpEnable = json_object_get_boolean(json_object_object_get(Jobj, "DhcpEnable"));
		if(ClassObj_Info->DhcpEnable == true){
			ClassObj_Info->Dhcp = (char*)json_object_get_string(json_object_object_get(Jobj, "Dhcp"));
			if(!strcmp(ClassObj_Info->Dhcp,"op60")){
				ClassObj_Info->SourceVendorClassID = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceVendorClassID"));
				ClassObj_Info->SourceVendorClassIDExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceVendorClassIDExclude"));
				ClassObj_Info->SourceClientIDType = clear;		
				ClassObj_Info->SourceClientID = clear;		
				ClassObj_Info->SourceClientIDExclude = false;		
				ClassObj_Info->SourceUserClassID = clear;		
				ClassObj_Info->SourceUserClassIDExclude = false;		
				ClassObj_Info->SourceVendorSpecificInfoEnterprise = 0;		
				ClassObj_Info->SourceVendorSpecificInfo = clear;		
				ClassObj_Info->SourceVendorSpecificInfoExclude = false;
			}
			if(!strcmp(ClassObj_Info->Dhcp,"op61")){
				ClassObj_Info->SourceClientIDType = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceClientIDType"));
				ClassObj_Info->SourceClientID = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceClientID"));
				ClassObj_Info->SourceClientIDExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceClientIDExclude"));
				ClassObj_Info->SourceVendorClassID = clear;		
				ClassObj_Info->SourceVendorClassIDExclude = false;	
				ClassObj_Info->SourceUserClassID = clear;		
				ClassObj_Info->SourceUserClassIDExclude = false;		
				ClassObj_Info->SourceVendorSpecificInfoEnterprise = 0;		
				ClassObj_Info->SourceVendorSpecificInfo = clear;		
				ClassObj_Info->SourceVendorSpecificInfoExclude = false;
			}
			if(!strcmp(ClassObj_Info->Dhcp,"op77")){
				ClassObj_Info->SourceUserClassID = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceUserClassID"));
				ClassObj_Info->SourceUserClassIDExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceUserClassIDExclude"));
				ClassObj_Info->SourceVendorClassID = clear;		
				ClassObj_Info->SourceVendorClassIDExclude = false;		
				ClassObj_Info->SourceClientIDType = clear;		
				ClassObj_Info->SourceClientID = clear;		
				ClassObj_Info->SourceClientIDExclude = false;		
				ClassObj_Info->SourceVendorSpecificInfoEnterprise = 0;		
				ClassObj_Info->SourceVendorSpecificInfo = clear;		
				ClassObj_Info->SourceVendorSpecificInfoExclude = false;
			}
			if(!strcmp(ClassObj_Info->Dhcp,"op125")){
				op125 = (char *)json_object_get_string(json_object_object_get(Jobj, "SourceVendorSpecificInfoEnterprise"));
				sscanf(op125, "%x", &enterprise);
				ClassObj_Info->SourceVendorSpecificInfoEnterprise = enterprise;
				ClassObj_Info->SourceVendorSpecificInfo = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceVendorSpecificInfo"));
				ClassObj_Info->SourceVendorSpecificInfoExclude = json_object_get_boolean(json_object_object_get(Jobj, "SourceVendorSpecificInfoExclude"));
				ClassObj_Info->SourceVendorClassID = clear;		
				ClassObj_Info->SourceVendorClassIDExclude = false;		
				ClassObj_Info->SourceClientIDType = clear;		
				ClassObj_Info->SourceClientID = clear;		
				ClassObj_Info->SourceClientIDExclude = false;
				ClassObj_Info->SourceUserClassID = clear;		
				ClassObj_Info->SourceUserClassIDExclude = false;
			}
		}
		else{
			ClassObj_Info->SourceVendorClassID = clear;		
			ClassObj_Info->SourceVendorClassIDExclude = false;		
			ClassObj_Info->SourceClientIDType = clear;		
			ClassObj_Info->SourceClientID = clear;		
			ClassObj_Info->SourceClientIDExclude = false;		
			ClassObj_Info->SourceUserClassID = clear;		
			ClassObj_Info->SourceUserClassIDExclude = false;		
			ClassObj_Info->SourceVendorSpecificInfoEnterprise = 0;		
			ClassObj_Info->SourceVendorSpecificInfo = clear;		
			ClassObj_Info->SourceVendorSpecificInfoExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "IPLengthEnable")){
		ClassObj_Info->IPLengthEnable = json_object_get_boolean(json_object_object_get(Jobj, "IPLengthEnable"));
		if(ClassObj_Info->IPLengthEnable == true){
			ClassObj_Info->IPLengthMin = json_object_get_int(json_object_object_get(Jobj, "IPLengthMin"));
			ClassObj_Info->IPLengthMax = json_object_get_int(json_object_object_get(Jobj, "IPLengthMax"));
			ClassObj_Info->IPLengthExclude = json_object_get_boolean(json_object_object_get(Jobj, "IPLengthExclude"));
		}
		else{
			ClassObj_Info->IPLengthMin = 0;
			ClassObj_Info->IPLengthMax = 0;
			ClassObj_Info->IPLengthExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "DSCPCheckEnable")){
		ClassObj_Info->DSCPCheckEnable = json_object_get_boolean(json_object_object_get(Jobj, "DSCPCheckEnable"));
		if(ClassObj_Info->DSCPCheckEnable == true){
			ClassObj_Info->DSCPCheck = json_object_get_int(json_object_object_get(Jobj, "DSCPCheck"));
			ClassObj_Info->DSCPExclude = json_object_get_boolean(json_object_object_get(Jobj, "DSCPExclude"));
		}
		else{
			ClassObj_Info->DSCPCheck = -1;
			ClassObj_Info->DSCPExclude = false;
		}
	}
	if(json_object_object_get(Jobj, "EthernetPriorityEnable")){
		ClassObj_Info->EthernetPriorityEnable = json_object_get_boolean(json_object_object_get(Jobj, "EthernetPriorityEnable"));
		if(ClassObj_Info->EthernetPriorityEnable == true){
			ClassObj_Info->EthernetPriorityCheck = json_object_get_int(json_object_object_get(Jobj, "EthernetPriorityCheck"));
			ClassObj_Info->EthernetPriorityExclude = json_object_get_boolean(json_object_object_get(Jobj, "EthernetPriorityExclude"));
		}
		else{
			ClassObj_Info->EthernetPriorityCheck = -1;
			ClassObj_Info->EthernetPriorityExclude = false;
		}		
	}		
	if(json_object_object_get(Jobj, "VLANIDCheckEnable")){
		ClassObj_Info->VLANIDCheckEnable = json_object_get_boolean(json_object_object_get(Jobj, "VLANIDCheckEnable"));
		if(ClassObj_Info->VLANIDCheckEnable == true){
			ClassObj_Info->VLANIDCheck = json_object_get_int(json_object_object_get(Jobj, "VLANIDCheck"));
			ClassObj_Info->VLANIDExclude = json_object_get_boolean(json_object_object_get(Jobj, "VLANIDExclude"));
		}
		else{
			ClassObj_Info->VLANIDCheck = -1;
			ClassObj_Info->VLANIDExclude = false;
		}
	}

	if(json_object_object_get(Jobj, "TCPACK")){
		ClassObj_Info->TCPACK = json_object_get_boolean(json_object_object_get(Jobj, "TCPACK"));
		if(json_object_object_get(Jobj, "TCPACKExclude"))
			ClassObj_Info->TCPACKExclude = json_object_get_boolean(json_object_object_get(Jobj, "TCPACKExclude"));
	}

	if(json_object_object_get(Jobj, "DSCPMark"))
		ClassObj_Info->DSCPMark = json_object_get_int(json_object_object_get(Jobj, "DSCPMark"));
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	if(json_object_object_get(Jobj, "EthernetPriorityMark"))
		ClassObj_Info->EthernetPriorityMark = json_object_get_int(json_object_object_get(Jobj, "EthernetPriorityMark"));
#else
	if(json_object_object_get(Jobj, "VLANIDAction")){
		ClassObj_Info->VLANIDAction = json_object_get_int(json_object_object_get(Jobj, "VLANIDAction"));
		if(ClassObj_Info->VLANIDAction == 0 || ClassObj_Info->VLANIDAction == 3){
			ClassObj_Info->EthernetPriorityMark = 0;
			ClassObj_Info->VLANIDMark = 0;
		}
		else{
			ClassObj_Info->EthernetPriorityMark = json_object_get_int(json_object_object_get(Jobj, "EthernetPriorityMark"));
			ClassObj_Info->VLANIDMark = json_object_get_int(json_object_object_get(Jobj, "VLANIDMark"));
		}
	}
#endif	
	if(json_object_object_get(Jobj, "ForwardingPolicy")){
		routeintf = (char *)json_object_get_string(json_object_object_get(Jobj, "ForwardingPolicy"));
		sscanf(routeintf, "%d", &(ClassObj_Info->ForwardingPolicy));
	}
	return ret;
}

void getQosClassDelKey(struct json_object *Jobj, s_QosClassDel *QosClassDel_Info) {
	int tmp_idx = 0;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};

	IID_INIT(classIid);
	if(json_object_object_get(Jobj, "idx")) { // treat as Order number
		tmp_idx = json_object_get_int(json_object_object_get(Jobj, "idx"));
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
			if(tmp_idx == json_object_get_int(json_object_object_get(classObj, "Order"))) {
				QosClassDel_Info->Index = classIid.idx[0];
				zcfgFeJsonObjFree(classObj);
				break;
			}
			zcfgFeJsonObjFree(classObj);
		}
	}
	else if(json_object_object_get(Jobj, "iid")) {
		QosClassDel_Info->Index = json_object_get_int(json_object_object_get(Jobj, "iid"));
	}
	else if(json_object_object_get(Jobj, "Index")) {
		tmp_idx = json_object_get_int(json_object_object_get(Jobj, "Index"));
		if(convertIndextoIid(tmp_idx, &classIid, RDM_OID_QOS_CLS, NULL, NULL, NULL) == ZCFG_SUCCESS) {
			QosClassDel_Info->Index = classIid.idx[0];
		}
	}
	else {
		printf("%s(): fail to get index\n", __FUNCTION__);
	}

	return;
}

void prepare_qos_classObj(struct json_object *Jobj, json_object *classObj, s_QosClass *ClassObj_Info) {
	if(json_object_object_get(Jobj, "Enable"))
		json_object_object_add(classObj, "Enable", json_object_new_boolean(ClassObj_Info->Enable));
	if(json_object_object_get(Jobj, "Name"))
		json_object_object_add(classObj, "X_ZYXEL_Name", json_object_new_string(ClassObj_Info->Name));
	if(json_object_object_get(Jobj, "Order"))
		json_object_object_add(classObj, "Order", json_object_new_int(ClassObj_Info->Order));
	if(json_object_object_get(Jobj, "Interface"))
		json_object_object_add(classObj, "Interface", json_object_new_string(ClassObj_Info->Interface));
#if defined(ZYXEL_WEB_GUI_SHOW_QOS_CLS_TARGET_INTF) || defined(ABUU_CUSTOMIZATION_QOS)
	if(json_object_object_get(Jobj, "TargetInterface"))
		json_object_object_add(classObj, "X_ZYXEL_TargetInterface", json_object_new_string(ClassObj_Info->TargetInterface));
#endif
	if(json_object_object_get(Jobj, "Ethertype"))
		json_object_object_add(classObj, "Ethertype", json_object_new_int(ClassObj_Info->Ethertype));
	if(json_object_object_get(Jobj, "SourceIPEnable")){
		json_object_object_add(classObj, "SourceIP", json_object_new_string(ClassObj_Info->SourceIP));
		json_object_object_add(classObj, "SourceMask", json_object_new_string(ClassObj_Info->SourceMask));
		json_object_object_add(classObj, "SourceIPExclude", json_object_new_boolean(ClassObj_Info->SourceIPExclude));
	}
	if(json_object_object_get(Jobj, "SourcePortEnable")){
		json_object_object_add(classObj, "SourcePort", json_object_new_int(ClassObj_Info->SourcePort));
		json_object_object_add(classObj, "SourcePortRangeMax", json_object_new_int(ClassObj_Info->SourcePortRangeMax));
		json_object_object_add(classObj, "SourcePortExclude", json_object_new_boolean(ClassObj_Info->SourcePortExclude));
	}
	if(json_object_object_get(Jobj, "SourceMACEnable")){
		json_object_object_add(classObj, "SourceMACAddress", json_object_new_string(ClassObj_Info->SourceMACAddress));
		json_object_object_add(classObj, "SourceMACMask", json_object_new_string(ClassObj_Info->SourceMACMask));
		json_object_object_add(classObj, "SourceMACExclude", json_object_new_boolean(ClassObj_Info->SourceMACExclude));
	}
	if(json_object_object_get(Jobj, "DestIPEnable")){
		json_object_object_add(classObj, "DestIP", json_object_new_string(ClassObj_Info->DestIP));
		json_object_object_add(classObj, "DestMask", json_object_new_string(ClassObj_Info->DestMask));
		json_object_object_add(classObj, "DestIPExclude", json_object_new_boolean(ClassObj_Info->DestIPExclude));
	}
	if(json_object_object_get(Jobj, "DestPortEnable")){
		json_object_object_add(classObj, "DestPort", json_object_new_int(ClassObj_Info->DestPort));
		json_object_object_add(classObj, "DestPortRangeMax", json_object_new_int(ClassObj_Info->DestPortRangeMax));
		json_object_object_add(classObj, "DestPortExclude", json_object_new_boolean(ClassObj_Info->DestPortExclude));
	}
	if(json_object_object_get(Jobj, "DestMACEnable")){
		json_object_object_add(classObj, "DestMACAddress", json_object_new_string(ClassObj_Info->DestMACAddress));
		json_object_object_add(classObj, "DestMACMask", json_object_new_string(ClassObj_Info->DestMACMask));
		json_object_object_add(classObj, "DestMACExclude", json_object_new_boolean(ClassObj_Info->DestMACExclude));
	}
	if(json_object_object_get(Jobj, "ServiceEnable")){
		json_object_object_add(classObj, "X_ZYXEL_Service", json_object_new_string(ClassObj_Info->Service));
		json_object_object_add(classObj, "X_ZYXEL_ServiceExclude", json_object_new_boolean(ClassObj_Info->ServiceExclude));
	}
	if(json_object_object_get(Jobj, "ProtocolEnable")){
		json_object_object_add(classObj, "Protocol", json_object_new_int(ClassObj_Info->Protocol));
		json_object_object_add(classObj, "ProtocolExclude", json_object_new_boolean(ClassObj_Info->ProtocolExclude));
	}
	if(json_object_object_get(Jobj, "DhcpEnable")){
		json_object_object_add(classObj, "SourceVendorClassID", json_object_new_string(ClassObj_Info->SourceVendorClassID));
		json_object_object_add(classObj, "SourceVendorClassIDExclude", json_object_new_boolean(ClassObj_Info->SourceVendorClassIDExclude));
		json_object_object_add(classObj, "X_ZYXEL_SourceClientIDType", json_object_new_string(ClassObj_Info->SourceClientIDType));
		json_object_object_add(classObj, "SourceClientID", json_object_new_string(ClassObj_Info->SourceClientID));
		json_object_object_add(classObj, "SourceClientIDExclude", json_object_new_boolean(ClassObj_Info->SourceClientIDExclude));
		json_object_object_add(classObj, "SourceUserClassID", json_object_new_string(ClassObj_Info->SourceUserClassID));
		json_object_object_add(classObj, "SourceUserClassIDExclude", json_object_new_boolean(ClassObj_Info->SourceUserClassIDExclude));
		json_object_object_add(classObj, "SourceVendorSpecificInfoEnterprise", json_object_new_int(ClassObj_Info->SourceVendorSpecificInfoEnterprise));
		json_object_object_add(classObj, "SourceVendorSpecificInfo", json_object_new_string(ClassObj_Info->SourceVendorSpecificInfo));
		json_object_object_add(classObj, "SourceVendorSpecificInfoExclude", json_object_new_boolean(ClassObj_Info->SourceVendorSpecificInfoExclude));
	}
	if(json_object_object_get(Jobj, "IPLengthEnable")){
		json_object_object_add(classObj, "IPLengthMin", json_object_new_int(ClassObj_Info->IPLengthMin));
		json_object_object_add(classObj, "IPLengthMax", json_object_new_int(ClassObj_Info->IPLengthMax));
		json_object_object_add(classObj, "IPLengthExclude", json_object_new_boolean(ClassObj_Info->IPLengthExclude));
	}
	if(json_object_object_get(Jobj, "DSCPCheckEnable")){
		json_object_object_add(classObj, "DSCPCheck", json_object_new_int(ClassObj_Info->DSCPCheck));
		json_object_object_add(classObj, "DSCPExclude", json_object_new_boolean(ClassObj_Info->DSCPExclude));
	}
	if(json_object_object_get(Jobj, "EthernetPriorityEnable")){
		json_object_object_add(classObj, "EthernetPriorityCheck", json_object_new_int(ClassObj_Info->EthernetPriorityCheck));
		json_object_object_add(classObj, "EthernetPriorityExclude", json_object_new_boolean(ClassObj_Info->EthernetPriorityExclude));
	}		
	if(json_object_object_get(Jobj, "VLANIDCheckEnable")){
		json_object_object_add(classObj, "VLANIDCheck", json_object_new_int(ClassObj_Info->VLANIDCheck));
		json_object_object_add(classObj, "VLANIDExclude", json_object_new_boolean(ClassObj_Info->VLANIDExclude));
	}
	if(json_object_object_get(Jobj, "TCPACK"))
		json_object_object_add(classObj, "TCPACK", json_object_new_boolean(ClassObj_Info->TCPACK));
	if(json_object_object_get(Jobj, "TCPACKExclude"))
		json_object_object_add(classObj, "TCPACKExclude", json_object_new_boolean(ClassObj_Info->TCPACKExclude));
	if(json_object_object_get(Jobj, "DSCPMark"))
		json_object_object_add(classObj, "DSCPMark", json_object_new_int(ClassObj_Info->DSCPMark));
#ifdef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
	if(json_object_object_get(Jobj, "EthernetPriorityMark"))
		json_object_object_add(classObj, "EthernetPriorityMark", json_object_new_int(ClassObj_Info->EthernetPriorityMark));
#else
	if(json_object_object_get(Jobj, "VLANIDAction")){
		json_object_object_add(classObj, "X_ZYXEL_VLANIDAction", json_object_new_int(ClassObj_Info->VLANIDAction));
		json_object_object_add(classObj, "EthernetPriorityMark", json_object_new_int(ClassObj_Info->EthernetPriorityMark));
		json_object_object_add(classObj, "X_ZYXEL_VLANIDMark", json_object_new_int(ClassObj_Info->VLANIDMark));
	}
#endif
	if(json_object_object_get(Jobj, "ForwardingPolicy"))
	json_object_object_add(classObj, "ForwardingPolicy", json_object_new_int(ClassObj_Info->ForwardingPolicy));
	json_object_object_add(classObj, "TrafficClass", json_object_new_int(ClassObj_Info->TrafficClass));

}

char* find_changed_traffic_class(int org_classIdx, int new_clsQueueIdx, bool defaultQ, int *changedQueueIdx, e_ActionType action) {
	int TrafficClass = -1;
	char *TrafficClasses_group_str = NULL;
	char *new_TrafficClasses_group_str = NULL;
	char *TrafficClasses_single_str = NULL;
	char buff[32] = {0};
	char *rest_str = NULL;
	bool FindQueue = false;
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};

	// the original class Iid idx
	TrafficClass = org_classIdx;

	// find the mapped queue of the original class
	IID_INIT(queueIid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
		TrafficClasses_group_str = (char*)json_object_get_string(json_object_object_get(queueObj, "TrafficClasses"));
		strcpy(buff,TrafficClasses_group_str);
		TrafficClasses_single_str = strtok_r(buff, ",", &rest_str);
			while(TrafficClasses_single_str != NULL) {
				if(StrToIntConv(TrafficClasses_single_str) == TrafficClass) {
					FindQueue = true;
					*changedQueueIdx = queueIid.idx[0];
					break;
				}
				TrafficClasses_single_str = strtok_r(rest_str, ",", &rest_str);
			}
		zcfgFeJsonObjFree(queueObj);

		if(FindQueue == true) {
			break;
		}
	}

	// case 1: if the mapped queue iid != the set queueIid, it means queue change for this class
	// case 2: for delete class, remove reference in TrafficClasses of old queue 
	if(FindQueue == true) {
		IID_INIT(queueIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
			if(queueIid.idx[0] == *changedQueueIdx) {
				if((defaultQ)||(queueIid.idx[0] != new_clsQueueIdx) || (action == e_ActionType_DELETE)) {
					TrafficClasses_group_str = (char*)json_object_get_string(json_object_object_get(queueObj, "TrafficClasses"));
					new_TrafficClasses_group_str = malloc(strlen(TrafficClasses_group_str) + 1);
					new_TrafficClasses_group_str[0] = '\0';
					if(TrafficClasses_group_str != NULL) {
						TrafficClasses_single_str = strtok_r(TrafficClasses_group_str, ",", &rest_str);
						while(TrafficClasses_single_str != NULL) {
							// if the TrafficClass in queue include the original TrafficClass, remove it
							if(StrToIntConv(TrafficClasses_single_str) != TrafficClass) {
								strcat(new_TrafficClasses_group_str, TrafficClasses_single_str);
								strcat(new_TrafficClasses_group_str, ",");
							}
							TrafficClasses_single_str = strtok_r(rest_str, ",", &rest_str);
						}
					}
				}
				zcfgFeJsonObjFree(queueObj);
				break;
			}
			zcfgFeJsonObjFree(queueObj);
		}
	}
	
	return new_TrafficClasses_group_str;
}

void update_traffic_class_in_qos_queue(int org_classIdx, int new_clsQueueIdx, bool defaultQ, objIndex_t classIid, e_ActionType action) {
	char *TrafficClasses = NULL;
	char *NewTrafficClasses = NULL;
	char *new_TrafficClasses_group_str = NULL;
	char tmp[8] = {0};
	int changedQueueIdx = 0;
	int array_size = 0;
	json_object *queueObj = NULL;
	objIndex_t queueIid = {0};
	IID_INIT(queueIid);

	// update the TrafficClasses of RDM_OID_QOS_QUE which the original class related
	if((action == e_ActionType_EDIT) || (action == e_ActionType_DELETE)) {
		
		new_TrafficClasses_group_str = find_changed_traffic_class(org_classIdx, new_clsQueueIdx, defaultQ, &changedQueueIdx, action);
		if(new_TrafficClasses_group_str != NULL) {
			while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
				if(changedQueueIdx == queueIid.idx[0]) {
					/* remove reference in TrafficClasses of old queue */
					json_object_object_add(queueObj, "TrafficClasses", json_object_new_string(new_TrafficClasses_group_str));
					zcfgFeObjJsonSetWithoutApply(RDM_OID_QOS_QUE, &queueIid, queueObj, NULL);
					zcfgFeJsonObjFree(queueObj);
					break;
				}
				zcfgFeJsonObjFree(queueObj);
			}
			ZOS_FREE(new_TrafficClasses_group_str);
		}
	}

	if(action != e_ActionType_DELETE) {
		IID_INIT(queueIid);
		while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_QOS_QUE, &queueIid, &queueObj) == ZCFG_SUCCESS) {
			// find the queue which the class belong to and set its TrafficClasses 
			if((!defaultQ) && (queueIid.idx[0] == new_clsQueueIdx) && (new_clsQueueIdx != changedQueueIdx)) {
				TrafficClasses = (char*)json_object_get_string(json_object_object_get(queueObj, "TrafficClasses"));
				// NewTrafficClasses = TrafficClasses + NewAddClass
				array_size = strlen(TrafficClasses) + 10;
				NewTrafficClasses = (char*)malloc(array_size); 
				strcpy(NewTrafficClasses, TrafficClasses);
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "%d", classIid.idx[0]);
				strcat(NewTrafficClasses, tmp);
				strcat(NewTrafficClasses, ",");
				json_object_object_add(queueObj, "TrafficClasses", json_object_new_string(NewTrafficClasses));
				zcfgFeObjJsonSetWithoutApply(RDM_OID_QOS_QUE, &queueIid, queueObj, NULL);
				zcfgFeJsonObjFree(queueObj);
				ZOS_FREE(NewTrafficClasses);
				break;
			}
			zcfgFeJsonObjFree(queueObj);
		}
	}

	return;
}

void zcfgFeDalShowQosClass(struct json_object *Jarray){
	int i, len = 0, protocol = 0;		
	char *ethertype = NULL, *dhcpoption = NULL;
	bool showIndex = false;	
	struct json_object *obj, *obj_idx = NULL;			
	if(json_object_get_type(Jarray) != json_type_array){						
		printf("wrong Jobj format!\n");						
		return;	
	}	
	len = json_object_array_length(Jarray);	
	obj_idx = json_object_array_get_idx(Jarray, 0);	
	showIndex = json_object_get_boolean(json_object_object_get(obj_idx, "ShowIndex"));	
	if(len == 1 && showIndex){	
		obj = json_object_array_get_idx(Jarray, 0);		
		ethertype = (char *)json_object_get_string(json_object_object_get(obj, "Ethertype"));
		dhcpoption = (char *)json_object_get_string(json_object_object_get(obj, "dhcp"));
		protocol = json_object_get_int(json_object_object_get(obj, "Protocol"));
		printf("Class Configuration\n");
		printf("%-50s %s \n","Index :", json_object_get_string(json_object_object_get(obj, "Index")));		
		printf("%-50s %s \n","Order :", json_object_get_string(json_object_object_get(obj, "Order")));
		printf("%-50s %s \n","Enable :", json_object_get_string(json_object_object_get(obj, "Enable")));	
		printf("%-50s %s \n","Name :", json_object_get_string(json_object_object_get(obj, "Name")));
		printf("Criteria Configuration\n");
		printf("%-50s %s \n","Interface :", json_object_get_string(json_object_object_get(obj, "Interface")));		
		printf("%-50s %s \n","Ether type :", json_object_get_string(json_object_object_get(obj, "Ethertype")));
		if(!strcmp(ethertype,"IP") || !strcmp(ethertype,"IPv6")){
			printf("%-50s %s \n","Source IP Address :", json_object_get_string(json_object_object_get(obj, "SourceIP")));
			if(!strcmp(ethertype,"IP"))
				printf("%-50s %s \n","Source Subnet Mask :", json_object_get_string(json_object_object_get(obj, "SourceMask")));
			if(!strcmp(ethertype,"IPv6"))
				printf("%-50s %s \n","Source Prefix Length :", json_object_get_string(json_object_object_get(obj, "SourceMask")));
			printf("%-50s %s \n","Source IP Exclude :", json_object_get_string(json_object_object_get(obj, "SourceIPExclude")));
			if(protocol == 6 || protocol == 17){
				printf("%-50s %s \n","Source Port Start :", json_object_get_string(json_object_object_get(obj, "SourcePort")));
				printf("%-50s %s \n","Source Port End :", json_object_get_string(json_object_object_get(obj, "SourcePortRangeMax")));
			}
		}
		printf("%-50s %s \n","Source MAC Address :", json_object_get_string(json_object_object_get(obj, "SourceMACAddress")));	
		printf("%-50s %s \n","Source MAC Mask :", json_object_get_string(json_object_object_get(obj, "SourceMACMask")));
		printf("%-50s %s \n","Source MAC Exclude :", json_object_get_string(json_object_object_get(obj, "SourceMACExclude")));
		if(!strcmp(ethertype,"IP") || !strcmp(ethertype,"IPv6")){
			printf("%-50s %s \n","Destination IP Address :", json_object_get_string(json_object_object_get(obj, "DestIP")));
			if(!strcmp(ethertype,"IP"))
				printf("%-50s %s \n","Destination Subnet Mask :", json_object_get_string(json_object_object_get(obj, "DestMask")));
			if(!strcmp(ethertype,"IPv6"))
				printf("%-50s %s \n","Destination Prefix Length :", json_object_get_string(json_object_object_get(obj, "DestMask")));
			printf("%-50s %s \n","Destination IP Exclude :", json_object_get_string(json_object_object_get(obj, "DestIPExclude")));
			if(protocol == 6 || protocol == 17){
				printf("%-50s %s \n","Destination Port Start :", json_object_get_string(json_object_object_get(obj, "DestPort")));
				printf("%-50s %s \n","Destination Port End :", json_object_get_string(json_object_object_get(obj, "DestPortRangeMax")));
			}
		}
		printf("%-50s %s \n","Destination MAC Address :", json_object_get_string(json_object_object_get(obj, "DestMACAddress")));	
		printf("%-50s %s \n","Destination MAC Mask :", json_object_get_string(json_object_object_get(obj, "DestMACMask")));	
		printf("%-50s %s \n","Destination MAC Exclude :", json_object_get_string(json_object_object_get(obj, "DestMACExclude")));
		if(!strcmp(ethertype,"IP") || !strcmp(ethertype,"IPv6")){
			printf("%-50s %s \n","IP protocol :", json_object_get_string(json_object_object_get(obj, "Protocol")));
			printf("%-50s %s \n","IP protocol Exclude :", json_object_get_string(json_object_object_get(obj, "ProtocolExclude")));
			if(!strcmp(ethertype,"IP")){
				if(!strcmp(dhcpoption,"Option 60")){
					printf("%-50s %s \n","DHCP :", "Option 60");	
					printf("%-50s %s \n","Vendor Class ID :", json_object_get_string(json_object_object_get(obj, "SourceVendorClassID")));
					printf("%-50s %s \n","Vendor Class ID Exclude :", json_object_get_string(json_object_object_get(obj, "SourceVendorClassIDExclude")));
				}
				if(!strcmp(dhcpoption,"Option 61")){
					printf("%-50s %s \n","DHCP :", "Option 61");	
					printf("%-50s %s \n","Client ID Type :", json_object_get_string(json_object_object_get(obj, "SourceClientIDType")));
					printf("%-50s %s \n","Client ID :", json_object_get_string(json_object_object_get(obj, "SourceClientID")));
					printf("%-50s %s \n","Client ID Exclude :", json_object_get_string(json_object_object_get(obj, "SourceClientIDExclude")));
				}
				if(!strcmp(dhcpoption,"Option 77")){
					printf("%-50s %s \n","DHCP :", "Option 77");	
					printf("%-50s %s \n","User Class ID :", json_object_get_string(json_object_object_get(obj, "SourceUserClassID")));
					printf("%-50s %s \n","User Class ID Exclude :", json_object_get_string(json_object_object_get(obj, "SourceUserClassIDExclude")));	
				}
				if(!strcmp(dhcpoption,"Option 125")){
					printf("%-50s %s \n","DHCP :", "Option 125");	
					printf("%-50s %s \n","Enterprise Number :", json_object_get_string(json_object_object_get(obj, "SourceVendorSpecificInfoEnterprise")));
					printf("%-50s %s \n","Vendor Specific Info :", json_object_get_string(json_object_object_get(obj, "SourceVendorSpecificInfo")));	
					printf("%-50s %s \n","Vendor Specific Info Exclude :", json_object_get_string(json_object_object_get(obj, "SourceVendorSpecificInfoExclude")));	
				}
			}
			printf("%-50s %s \n","IP Packet Length Min :", json_object_get_string(json_object_object_get(obj, "IPLengthMin"))); 
			printf("%-50s %s \n","IP Packet Length Max :", json_object_get_string(json_object_object_get(obj, "IPLengthMax")));
			printf("%-50s %s \n","IP Packet Length Exclude :", json_object_get_string(json_object_object_get(obj, "IPLengthExclude")));
			printf("%-50s %s \n","DSCP :", json_object_get_string(json_object_object_get(obj, "DSCPCheck")));
			printf("%-50s %s \n","DSCP Exclude :", json_object_get_string(json_object_object_get(obj, "DSCPExclude")));
		}
	
		if(!strcmp(ethertype,"802.1Q")){
			printf("%-50s %s \n","802.1P :", json_object_get_string(json_object_object_get(obj, "EthernetPriorityCheck")));	
			printf("%-50s %s \n","802.1P Exclude :", json_object_get_string(json_object_object_get(obj, "EthernetPriorityExclude")));
			printf("%-50s %s \n","VLAN ID :", json_object_get_string(json_object_object_get(obj, "VLANIDCheck")));
			printf("%-50s %s \n","VLAN ID Exclude :", json_object_get_string(json_object_object_get(obj, "VLANIDExclude")));
		}

		if(protocol == 6){
			printf("%-50s %s \n","TCP ACK :", json_object_get_string(json_object_object_get(obj, "TCPACK")));
			printf("%-50s %s \n","TCP ACK Exclude :", json_object_get_string(json_object_object_get(obj, "TCPACKExclude")));
		}	
		printf("Packet Modification\n");
		printf("%-50s %s \n","DSCP Mark :", json_object_get_string(json_object_object_get(obj, "DSCPMark")));	
		printf("%-50s %s \n","802.1P Mark :", json_object_get_string(json_object_object_get(obj, "EthernetPriorityMark")));
#ifndef ZYXEL_WEB_GUI_ECONET_QoS_STYLE
		printf("%-50s %s \n","VLAN ID Action :", json_object_get_string(json_object_object_get(obj, "VLANIDAction")));
		printf("%-50s %s \n","VLAN ID Tag :", json_object_get_string(json_object_object_get(obj, "VLANIDMark")));
#endif
		printf("Class Routing\n");
		printf("%-50s %s \n","To Interface :", json_object_get_string(json_object_object_get(obj, "ForwardingPolicy")));
		printf("Outgoing Queue Selection\n");
		printf("%-50s %s \n","To Queue :", json_object_get_string(json_object_object_get(obj, "Queue")));
	}	
	else{		
		printf("%-6s %-7s %-7s %-15s %-10s %-17s\n","Index", "Order", "Enable", "Name", "Ether Type", "To Queue");			
		for(i=0;i<len;i++){				
			obj = json_object_array_get_idx(Jarray, i);							
			printf("%-6s %-7s %-7s %-15s %-10s %-17s\n",			
			json_object_get_string(json_object_object_get(obj, "Index")),			
			json_object_get_string(json_object_object_get(obj, "Order")),			
			json_object_get_string(json_object_object_get(obj, "Enable")),			
			json_object_get_string(json_object_object_get(obj, "Name")),
			json_object_get_string(json_object_object_get(obj, "Ethertype")),
			json_object_get_string(json_object_object_get(obj, "Queue")));		
		}	
	}
}

zcfgRet_t zcfgFeDalQosClassGet(struct json_object *Jobj, struct json_object *Jarray, char *replyMsg){	
	zcfgRet_t ret = ZCFG_SUCCESS;		
	struct json_object *pramJobj = NULL;
	struct json_object *classObj = NULL;
	objIndex_t classIid = {0};
	char *SourceVendorClassID = NULL;
	char *SourceClientIDType = NULL;
	char *SourceUserClassID = NULL;
	char *SourceVendorSpecificInfo = NULL;
	char Policy[20] = {0};
	char number[20] = {0};
	int count = 1,index = 0,trafficclass = 0, forward = 0, ethertype = 0;

	IID_INIT(classIid);	
	if(json_object_object_get(Jobj, "Index")){
		index = json_object_get_int(json_object_object_get(Jobj, "Index"));		
		ret = convertIndextoIid(index, &classIid, RDM_OID_QOS_CLS, NULL, NULL, replyMsg);
		if(ret != ZCFG_SUCCESS)			
			return ret;		
		if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_CLS, &classIid , &classObj) == ZCFG_SUCCESS){
			pramJobj = json_object_new_object();
			ethertype = json_object_get_int(json_object_object_get(classObj, "Ethertype"));
			trafficclass = json_object_get_int(json_object_object_get(classObj, "TrafficClass"));
			SourceVendorClassID = (char *)json_object_get_string(json_object_object_get(classObj, "SourceVendorClassID"));
			if(strcmp(SourceVendorClassID,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 60"));
			SourceClientIDType = (char *)json_object_get_string(json_object_object_get(classObj, "X_ZYXEL_SourceClientIDType"));
			if(strcmp(SourceClientIDType,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 61"));
			SourceUserClassID = (char *)json_object_get_string(json_object_object_get(classObj, "SourceUserClassID"));
			if(strcmp(SourceUserClassID,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 77"));
			SourceVendorSpecificInfo = (char *)json_object_get_string(json_object_object_get(classObj, "SourceVendorSpecificInfo"));
			if(strcmp(SourceVendorSpecificInfo,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 125"));
			if(!strcmp(SourceVendorClassID,"") && !strcmp(SourceClientIDType,"") && !strcmp(SourceUserClassID,"") && !strcmp(SourceVendorSpecificInfo,""))
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string(""));
			forward = json_object_get_int(json_object_object_get(classObj, "ForwardingPolicy"));
			sprintf(Policy,"%d",forward);
#if defined(ZYXEL_WEB_GUI_SHOW_QOS_CLS_TARGET_INTF) || defined(ABUU_CUSTOMIZATION_QOS)
			if (json_object_object_get(classObj, "X_ZYXEL_TargetInterface"))
				json_object_object_add(pramJobj, "TargetInterface", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_TargetInterface")));
#endif
			json_object_object_add(pramJobj, "ForwardingPolicy", json_object_new_string(Policy));
			json_object_object_add(pramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(classObj, "Interface")));
			json_object_object_add(pramJobj, "ShowIndex", json_object_new_boolean(true));			
			json_object_object_add(pramJobj, "Index", json_object_new_int(index));	
			json_object_object_add(pramJobj, "iid", json_object_new_int(classIid.idx[0]));
			json_object_object_add(pramJobj, "Policer",  JSON_OBJ_COPY(json_object_object_get(classObj, "Policer")));
			json_object_object_add(pramJobj, "Order",  JSON_OBJ_COPY(json_object_object_get(classObj, "Order")));
			json_object_object_add(pramJobj, "Enable",  JSON_OBJ_COPY(json_object_object_get(classObj, "Enable")));			
			json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_Name")));			
			json_object_object_add(pramJobj, "SourceIP", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceIP")));	
			json_object_object_add(pramJobj, "SourceMask", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMask")));	
			json_object_object_add(pramJobj, "SourceIPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceIPExclude")));	
			json_object_object_add(pramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePort")));	
			json_object_object_add(pramJobj, "SourcePortRangeMax", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePortRangeMax")));	
			json_object_object_add(pramJobj, "SourcePortExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePortExclude")));	
			json_object_object_add(pramJobj, "SourceMACAddress", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACAddress")));	
			json_object_object_add(pramJobj, "SourceMACMask", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACMask")));	
			json_object_object_add(pramJobj, "SourceMACExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACExclude")));	
			json_object_object_add(pramJobj, "DestIP", JSON_OBJ_COPY(json_object_object_get(classObj, "DestIP")));	
			json_object_object_add(pramJobj, "DestMask", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMask")));	
			json_object_object_add(pramJobj, "DestIPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestIPExclude")));	
			json_object_object_add(pramJobj, "DestPort", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPort")));	
			json_object_object_add(pramJobj, "DestPortRangeMax", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPortRangeMax")));	
			json_object_object_add(pramJobj, "DestPortExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPortExclude")));	
			json_object_object_add(pramJobj, "DestMACAddress", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACAddress")));	
			json_object_object_add(pramJobj, "DestMACMask", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACMask")));	
			json_object_object_add(pramJobj, "DestMACExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACExclude")));	
			json_object_object_add(pramJobj, "Service", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_Service")));
			json_object_object_add(pramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(classObj, "Protocol")));	
			json_object_object_add(pramJobj, "ProtocolExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "ProtocolExclude")));	
			json_object_object_add(pramJobj, "SourceVendorClassID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorClassID")));	
			json_object_object_add(pramJobj, "SourceVendorClassIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorClassIDExclude")));	
			json_object_object_add(pramJobj, "SourceClientIDType", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_SourceClientIDType")));	
			json_object_object_add(pramJobj, "SourceClientID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceClientID")));	
			json_object_object_add(pramJobj, "SourceClientIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceClientIDExclude")));	
			json_object_object_add(pramJobj, "SourceUserClassID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceUserClassID")));	
			json_object_object_add(pramJobj, "SourceUserClassIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceUserClassIDExclude")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfoEnterprise", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfoEnterprise")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfo", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfo")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfoExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfoExclude")));	
			json_object_object_add(pramJobj, "IPLengthMin", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthMin")));	
			json_object_object_add(pramJobj, "IPLengthMax", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthMax")));	
			json_object_object_add(pramJobj, "IPLengthExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthExclude")));	
			json_object_object_add(pramJobj, "DSCPCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPCheck")));	
			json_object_object_add(pramJobj, "DSCPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPExclude")));	
			json_object_object_add(pramJobj, "EthernetPriorityCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityCheck")));	
			json_object_object_add(pramJobj, "EthernetPriorityExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityExclude")));
			json_object_object_add(pramJobj, "VLANIDCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "VLANIDCheck")));	
			json_object_object_add(pramJobj, "VLANIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "VLANIDExclude")));	
			json_object_object_add(pramJobj, "TCPACK", JSON_OBJ_COPY(json_object_object_get(classObj, "TCPACK")));	
			json_object_object_add(pramJobj, "TCPACKExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "TCPACKExclude")));	
			json_object_object_add(pramJobj, "DSCPMark", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPMark")));	
			json_object_object_add(pramJobj, "VLANIDAction", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_VLANIDAction")));	
			json_object_object_add(pramJobj, "EthernetPriorityMark", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityMark")));	
			json_object_object_add(pramJobj, "VLANIDMark", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_VLANIDMark")));	

			if(ethertype == ETHTYPE_NA)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("NA"));
			if(ethertype == ETHTYPE_IP)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("IP"));
			if(ethertype == ETHTYPE_ARP)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("ARP"));
			if(ethertype == ETHTYPE_PPPOEDISC)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("PPPoE_DISC"));
			if(ethertype == ETHTYPE_PPPOESES)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("PPPoE_SES"));
			if(ethertype == ETHTYPE_8021Q)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("802.1Q"));
			if(ethertype == ETHTYPE_IPV6)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("IPv6"));
#ifdef ABUU_CUSTOMIZATION_QOS
            if (ethertype == ETHTYPE_IP_IPV6)
            {
                json_object_object_add(pramJobj, "Ethertype", json_object_new_string("IP_IPv6"));
            }
#endif
			sprintf(number, "%d", trafficclass);
			json_object_object_add(pramJobj, "Queue", json_object_new_string(number));
			json_object_array_add(Jarray, pramJobj);									
			zcfgFeJsonObjFree(classObj);			
		}	
	}	
	else{		
		while(zcfgFeObjJsonGetNext(RDM_OID_QOS_CLS, &classIid , &classObj) == ZCFG_SUCCESS){						
			sprintf(number, "%d", classIid.idx[0]);
			pramJobj = json_object_new_object();
			ethertype = json_object_get_int(json_object_object_get(classObj, "Ethertype"));
			trafficclass = json_object_get_int(json_object_object_get(classObj, "TrafficClass"));
			SourceVendorClassID = (char *)json_object_get_string(json_object_object_get(classObj, "SourceVendorClassID"));
			if(strcmp(SourceVendorClassID,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 60"));
			SourceClientIDType = (char *)json_object_get_string(json_object_object_get(classObj, "X_ZYXEL_SourceClientIDType"));
			if(strcmp(SourceClientIDType,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 61"));
			SourceUserClassID = (char *)json_object_get_string(json_object_object_get(classObj, "SourceUserClassID"));
			if(strcmp(SourceUserClassID,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 77"));
			SourceVendorSpecificInfo = (char *)json_object_get_string(json_object_object_get(classObj, "SourceVendorSpecificInfo"));
			if(strcmp(SourceVendorSpecificInfo,"")!= 0)
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string("Option 125"));
			if(!strcmp(SourceVendorClassID,"") && !strcmp(SourceClientIDType,"") && !strcmp(SourceUserClassID,"") && !strcmp(SourceVendorSpecificInfo,""))
				json_object_object_add(pramJobj, "Dhcp", json_object_new_string(""));
			forward = json_object_get_int(json_object_object_get(classObj, "ForwardingPolicy"));
			sprintf(Policy,"%d",forward);
#if defined(ZYXEL_WEB_GUI_SHOW_QOS_CLS_TARGET_INTF) || defined(ABUU_CUSTOMIZATION_QOS)
			if (json_object_object_get(classObj, "X_ZYXEL_TargetInterface"))
				json_object_object_add(pramJobj, "TargetInterface", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_TargetInterface")));
#endif
			json_object_object_add(pramJobj, "ForwardingPolicy", json_object_new_string(Policy));
			json_object_object_add(pramJobj, "Interface", JSON_OBJ_COPY(json_object_object_get(classObj, "Interface")));
			json_object_object_add(pramJobj, "ShowIndex", json_object_new_boolean(true));			
			json_object_object_add(pramJobj, "Index", json_object_new_int(count));
			json_object_object_add(pramJobj, "iid", json_object_new_int(classIid.idx[0]));
			json_object_object_add(pramJobj, "Policer",  JSON_OBJ_COPY(json_object_object_get(classObj, "Policer")));
			json_object_object_add(pramJobj, "Order",  JSON_OBJ_COPY(json_object_object_get(classObj, "Order")));
			json_object_object_add(pramJobj, "Enable",  JSON_OBJ_COPY(json_object_object_get(classObj, "Enable")));			
			json_object_object_add(pramJobj, "Name",  JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_Name")));	
			json_object_object_add(pramJobj, "Queue", json_object_new_string(number));
			json_object_object_add(pramJobj, "SourceIP", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceIP")));	
			json_object_object_add(pramJobj, "SourceMask", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMask")));	
			json_object_object_add(pramJobj, "SourceIPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceIPExclude")));	
			json_object_object_add(pramJobj, "SourcePort", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePort")));	
			json_object_object_add(pramJobj, "SourcePortRangeMax", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePortRangeMax")));	
			json_object_object_add(pramJobj, "SourcePortExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourcePortExclude")));	
			json_object_object_add(pramJobj, "SourceMACAddress", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACAddress")));	
			json_object_object_add(pramJobj, "SourceMACMask", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACMask")));	
			json_object_object_add(pramJobj, "SourceMACExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceMACExclude")));	
			json_object_object_add(pramJobj, "DestIP", JSON_OBJ_COPY(json_object_object_get(classObj, "DestIP")));	
			json_object_object_add(pramJobj, "DestMask", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMask")));	
			json_object_object_add(pramJobj, "DestIPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestIPExclude")));	
			json_object_object_add(pramJobj, "DestPort", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPort")));	
			json_object_object_add(pramJobj, "DestPortRangeMax", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPortRangeMax")));	
			json_object_object_add(pramJobj, "DestPortExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestPortExclude")));	
			json_object_object_add(pramJobj, "DestMACAddress", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACAddress")));	
			json_object_object_add(pramJobj, "DestMACMask", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACMask")));	
			json_object_object_add(pramJobj, "DestMACExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DestMACExclude")));	
			json_object_object_add(pramJobj, "Service", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_Service")));	
			json_object_object_add(pramJobj, "ServiceExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_ServiceExclude")));	
			json_object_object_add(pramJobj, "Protocol", JSON_OBJ_COPY(json_object_object_get(classObj, "Protocol")));	
			json_object_object_add(pramJobj, "ProtocolExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "ProtocolExclude")));	
			json_object_object_add(pramJobj, "SourceVendorClassID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorClassID")));	
			json_object_object_add(pramJobj, "SourceVendorClassIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorClassIDExclude")));	
			json_object_object_add(pramJobj, "SourceClientIDType", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_SourceClientIDType")));	
			json_object_object_add(pramJobj, "SourceClientID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceClientID")));	
			json_object_object_add(pramJobj, "SourceClientIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceClientIDExclude")));	
			json_object_object_add(pramJobj, "SourceUserClassID", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceUserClassID")));	
			json_object_object_add(pramJobj, "SourceUserClassIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceUserClassIDExclude")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfoEnterprise", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfoEnterprise")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfo", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfo")));	
			json_object_object_add(pramJobj, "SourceVendorSpecificInfoExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "SourceVendorSpecificInfoExclude")));	
			json_object_object_add(pramJobj, "IPLengthMin", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthMin")));	
			json_object_object_add(pramJobj, "IPLengthMax", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthMax")));	
			json_object_object_add(pramJobj, "IPLengthExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "IPLengthExclude")));	
			json_object_object_add(pramJobj, "DSCPCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPCheck")));	
			json_object_object_add(pramJobj, "DSCPExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPExclude")));	
			json_object_object_add(pramJobj, "EthernetPriorityCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityCheck")));	
			json_object_object_add(pramJobj, "EthernetPriorityExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityExclude")));
			json_object_object_add(pramJobj, "VLANIDCheck", JSON_OBJ_COPY(json_object_object_get(classObj, "VLANIDCheck")));	
			json_object_object_add(pramJobj, "VLANIDExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "VLANIDExclude")));	
			json_object_object_add(pramJobj, "TCPACK", JSON_OBJ_COPY(json_object_object_get(classObj, "TCPACK")));	
			json_object_object_add(pramJobj, "TCPACKExclude", JSON_OBJ_COPY(json_object_object_get(classObj, "TCPACKExclude")));	
			json_object_object_add(pramJobj, "DSCPMark", JSON_OBJ_COPY(json_object_object_get(classObj, "DSCPMark")));	
			json_object_object_add(pramJobj, "VLANIDAction", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_VLANIDAction")));	
			json_object_object_add(pramJobj, "EthernetPriorityMark", JSON_OBJ_COPY(json_object_object_get(classObj, "EthernetPriorityMark")));	
			json_object_object_add(pramJobj, "VLANIDMark", JSON_OBJ_COPY(json_object_object_get(classObj, "X_ZYXEL_VLANIDMark")));	

			if(ethertype == ETHTYPE_NA)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("NA"));
			if(ethertype == ETHTYPE_IP)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("IP"));
			if(ethertype == ETHTYPE_ARP)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("ARP"));
			if(ethertype == ETHTYPE_PPPOEDISC)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("PPPoE_DISC"));
			if(ethertype == ETHTYPE_PPPOESES)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("PPPoE_SES"));
			if(ethertype == ETHTYPE_8021Q)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("802.1Q"));
			if(ethertype == ETHTYPE_IPV6)
				json_object_object_add(pramJobj, "Ethertype",json_object_new_string("IPv6"));
#ifdef ABUU_CUSTOMIZATION_QOS
            if (ethertype == ETHTYPE_IP_IPV6)
            {
                json_object_object_add(pramJobj, "Ethertype", json_object_new_string("IP_IPv6"));
            }
#endif
			json_object_array_add(Jarray, pramJobj);						
			count++;							
			zcfgFeJsonObjFree(classObj);
		}
	}	
	return ret;
}

bool checkdhcptype(struct json_object *Jobj, s_QosClass *ClassObj_Info)
{
	char *tmp1 = NULL, *tmp2 = NULL;
	int len1 = 0, len2 = 0;
	if(!strcmp(ClassObj_Info->Dhcp,"op60")){
		tmp1 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceVendorClassID"));
		len1 = strlen(tmp1);
		if(len1>124){
			printf("SourceVendorClassID length can not greater than 124.\n");				
			return false; 
		}
	}
	if(!strcmp(ClassObj_Info->Dhcp,"op61")){
		tmp1 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceClientIDType"));
		tmp2 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceClientID"));
		len1 = strlen(tmp1);
		len2 = strlen(tmp2);
		if(len1>2){
			printf("SourceClientIDType length can not greater than 2.\n");				
			return false; 
		}
		while(*tmp1 != '\0'){
			if(!isxdigit(*tmp1)){
				printf("SourceClientIDType should be Hex Number.\n");				
				return false; 
			}
			tmp1++;
		}

		if(len2>32 || (len2%2 != 0)){
			printf("SourceClientID length can not greater than 32 and should be even length.\n");				
			return false; 
		}
		while(*tmp2 != '\0'){
			if(!isxdigit(*tmp2)){
				printf("SourceClientID should be Hex Number.\n");				
				return false; 
			}
			tmp2++;
		}
	}
	if(!strcmp(ClassObj_Info->Dhcp,"op77")){
		tmp1 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceUserClassID"));
		len1 = strlen(tmp1);
		if(len1>32 || (len1%2 != 0)){
			printf("SourceUserClassID length can not greater than 32 and should be even length.\n");				
			return false; 
		}
		while(*tmp1 != '\0'){
			if(!isxdigit(*tmp1)){
				printf("SourceUserClassID should be Hex Number.\n");				
				return false; 
			}
			tmp1++;
		}
	}
	if(!strcmp(ClassObj_Info->Dhcp,"op125")){
		tmp1 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceVendorSpecificInfoEnterprise"));
		tmp2 = (char*)json_object_get_string(json_object_object_get(Jobj, "SourceVendorSpecificInfo"));
		len1 = strlen(tmp1);
		len2 = strlen(tmp2);
		if(len1 != 8){
			printf("SourceVendorSpecificInfoEnterprise length should be equal to 8.\n");				
			return false; 
		}
		while(*tmp1 != '\0'){
			if(!isxdigit(*tmp1)){
				printf("SourceVendorSpecificInfoEnterprise should be Hex Number.\n");				
				return false; 
			}
			tmp1++;
		}
		if(len2>32 || (len2%2 != 0)){
			printf("SourceVendorSpecificInfo length can not greater than 32 and should be even length.\n");				
			return false; 
		}
		while(*tmp2 != '\0'){
			if(!isxdigit(*tmp2)){
				printf("SourceVendorSpecificInfo should be Hex Number.\n");				
				return false; 
			}
			tmp2++;
		}
	}
	return true;
}

zcfgRet_t zcfgFeDalQosClassAdd(const char *method, struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	e_ActionType action = e_ActionType_ADD; //ADD, EDIT, DELETE
	s_QosClass ClassObj_Info;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};
	IID_INIT(classIid);
	memset(&ClassObj_Info,0,sizeof(ClassObj_Info));

	if((ret = getQosClassBasicInfo(method, Jobj, &ClassObj_Info)) != ZCFG_SUCCESS)
		return ret;

	zcfg_offset_t oid;		
	oid = RDM_OID_QOS_CLS;		
	if(ClassObj_Info.Name != NULL){	
		ret = dalcmdParamCheck(NULL, ClassObj_Info.Name , oid, "X_ZYXEL_Name", "Name",  replyMsg);				
		if(ret != ZCFG_SUCCESS)						
			return ret;
	}
	if(json_object_object_get(Jobj, "IPLengthEnable")){
		if(ClassObj_Info.IPLengthEnable == true){
			if(ClassObj_Info.IPLengthMin > ClassObj_Info.IPLengthMax){ 	
				printf("IPLengthMax need to equal or greater than IPLengthMin.\n");				
				return ZCFG_INVALID_PARAM_VALUE; 
			}	
		}
	}

	if(json_object_object_get(Jobj, "DhcpEnable")){
		if(ClassObj_Info.DhcpEnable == true){
			if(checkdhcptype(Jobj, &ClassObj_Info)== false)
				return ZCFG_INVALID_PARAM_VALUE; 
		}
	}
	
	// POST RDM_OID_QOS_CLS
	ret = zcfgFeObjJsonAdd(RDM_OID_QOS_CLS, &classIid, NULL);		
	if(ret == ZCFG_SUCCESS) {		
		ret = zcfgFeObjJsonGet(RDM_OID_QOS_CLS, &classIid, &classObj);		
	}	

	if(ret == ZCFG_SUCCESS) {
	// Fill the TrafficClasses into RDM_OID_QOS_CLS
		ClassObj_Info.TrafficClass = classIid.idx[0];

	// update RDM_OID_QOS_QUE - TrafficClasses
		update_traffic_class_in_qos_queue(ClassObj_Info.Index, ClassObj_Info.Queue, ClassObj_Info.defaultQ, classIid, action);
		
		// PUT RDM_OID_QOS_CLS
		prepare_qos_classObj(Jobj, classObj, &ClassObj_Info);
		zcfgFeObjJsonBlockedSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
		zcfgFeJsonObjFree(classObj);
		return ret;
	}
	else{				
		printf("%s: Object retrieve fail\n", __FUNCTION__);				
		return ret;	
	}
}

zcfgRet_t zcfgFeDalQosClassEdit(const char *method, struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	e_ActionType action = e_ActionType_EDIT; //ADD, EDIT, DELETE
	int ethertype = 0;
	s_QosClass ClassObj_Info;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};

	memset(&ClassObj_Info, 0, sizeof(ClassObj_Info));
	if((ret = getQosClassBasicInfo(method, Jobj, &ClassObj_Info)) != ZCFG_SUCCESS)
		return ret;

	IID_INIT(classIid);
	classIid.level = 1;
	classIid.idx[0] = ClassObj_Info.Index;

	if(ClassObj_Info.Name != NULL){	
		ret = dalcmdParamCheck(&classIid, ClassObj_Info.Name, RDM_OID_QOS_CLS, "X_ZYXEL_Name", "Name",  replyMsg);
		if(ret != ZCFG_SUCCESS)						
			return ret;	
	}	
	
	if(json_object_object_get(Jobj, "DhcpEnable")){
		if(ClassObj_Info.DhcpEnable == true){
			if(checkdhcptype(Jobj, &ClassObj_Info)== false)
				return ZCFG_INVALID_PARAM_VALUE; 
		}
	}
	
	// Fill the TrafficClasses into RDM_OID_QOS_CLS
	ClassObj_Info.TrafficClass = classIid.idx[0];
	
	// PUT RDM_OID_QOS_CLS
	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {
		if(json_object_object_get(Jobj, "Ethertype")){	
			ethertype = json_object_get_int(json_object_object_get(classObj, "Ethertype"));
			//different ethertype, all related parameter need to be default
			if(ClassObj_Info.Ethertype != ethertype){
				json_object_object_add(classObj, "SourceIP", json_object_new_string(""));				
				json_object_object_add(classObj, "SourceMask", json_object_new_string(""));				
				json_object_object_add(classObj, "SourceIPExclude", json_object_new_boolean(false));				
				json_object_object_add(classObj, "SourcePort", json_object_new_int(-1));
				json_object_object_add(classObj, "SourcePortRangeMax", json_object_new_int(-1));				
				json_object_object_add(classObj, "SourcePortExclude", json_object_new_boolean(false));							
				json_object_object_add(classObj, "DestIP", json_object_new_string(""));
				json_object_object_add(classObj, "DestMask", json_object_new_string(""));				
				json_object_object_add(classObj, "DestIPExclude", json_object_new_boolean(false));				
				json_object_object_add(classObj, "DestPort", json_object_new_int(-1));
				json_object_object_add(classObj, "DestPortRangeMax", json_object_new_int(-1));				
				json_object_object_add(classObj, "DestPortExclude", json_object_new_boolean(false));	
				json_object_object_add(classObj, "X_ZYXEL_Service", json_object_new_string(""));
				json_object_object_add(classObj, "X_ZYXEL_ServiceExclude", json_object_new_boolean(false));
				json_object_object_add(classObj, "Protocol", json_object_new_int(-1));				
				json_object_object_add(classObj, "ProtocolExclude", json_object_new_boolean(false));				
				json_object_object_add(classObj, "SourceVendorClassID", json_object_new_string(""));
				json_object_object_add(classObj, "SourceVendorClassIDExclude", json_object_new_boolean(false));				
				json_object_object_add(classObj, "SourceClientID", json_object_new_string(""));	
				json_object_object_add(classObj, "SourceClientIDType", json_object_new_string(""));
				json_object_object_add(classObj, "SourceClientIDExclude", json_object_new_boolean(false));
				json_object_object_add(classObj, "SourceUserClassID", json_object_new_string(""));				
				json_object_object_add(classObj, "SourceUserClassIDExclude", json_object_new_boolean(false));				
				json_object_object_add(classObj, "SourceVendorSpecificInfoEnterprise", json_object_new_int(0));
				json_object_object_add(classObj, "SourceVendorSpecificInfo", json_object_new_string(""));				
				json_object_object_add(classObj, "SourceVendorSpecificInfoExclude", json_object_new_boolean(false));	
				json_object_object_add(classObj, "IPLengthMin", json_object_new_int(0));
				json_object_object_add(classObj, "IPLengthMax", json_object_new_int(0));
				json_object_object_add(classObj, "IPLengthExclude", json_object_new_boolean(false));
				json_object_object_add(classObj, "DSCPCheck", json_object_new_int(-1));				
				json_object_object_add(classObj, "DSCPExclude", json_object_new_boolean(false));
				json_object_object_add(classObj, "TCPACK", json_object_new_boolean(false));				
				json_object_object_add(classObj, "TCPACKExclude", json_object_new_boolean(false));	
				json_object_object_add(classObj, "EthernetPriorityCheck", json_object_new_int(-1));				
				json_object_object_add(classObj, "EthernetPriorityExclude", json_object_new_boolean(false));	
				json_object_object_add(classObj, "VLANIDCheck", json_object_new_int(-1));
				json_object_object_add(classObj, "VLANIDExclude", json_object_new_boolean(false));	
			}
		}

		// update RDM_OID_QOS_QUE - TrafficClasses
		if(json_object_object_get(Jobj, "idx") == NULL){
			update_traffic_class_in_qos_queue(classIid.idx[0], ClassObj_Info.Queue, ClassObj_Info.defaultQ, classIid, action);
		}
		else{
			update_traffic_class_in_qos_queue(ClassObj_Info.Index, ClassObj_Info.Queue, ClassObj_Info.defaultQ, classIid, action);
		}
		prepare_qos_classObj(Jobj, classObj,&ClassObj_Info);
		zcfgFeObjJsonBlockedSet(RDM_OID_QOS_CLS, &classIid, classObj, NULL);
		zcfgFeJsonObjFree(classObj);			
		return ret;
	}
	else{							
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Class fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_CLS);		
		return ZCFG_INTERNAL_ERROR;				
	}
}

zcfgRet_t zcfgFeDalQosClassDelete(struct json_object *Jobj, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;
	//e_ActionType action = e_ActionType_DELETE; //ADD, EDIT, DELETE
	s_QosClassDel QosClassDel_Info;
	json_object *classObj = NULL;
	objIndex_t classIid = {0};

	memset(&QosClassDel_Info, 0, sizeof(QosClassDel_Info));
	getQosClassDelKey(Jobj, &QosClassDel_Info);
	IID_INIT(classIid);
	classIid.level = 1;
	classIid.idx[0] = QosClassDel_Info.Index;

	if(zcfgFeObjJsonGetWithoutUpdate(RDM_OID_QOS_CLS, &classIid, &classObj) == ZCFG_SUCCESS) {			
		zcfgFeObjJsonDel(RDM_OID_QOS_CLS, &classIid, NULL);
		// update RDM_OID_QOS_QUE - TrafficClasses
		//update_traffic_class_in_qos_queue(QosClassDel_Info.Index, -1, false, classIid, action);

		zcfgFeJsonObjFree(classObj);
		return ret;	
	}
	else{							
		printf("%s():(ERROR) zcfgFeObjJsonGetWithoutUpdate get Qos Class fail oid(%d)\n ",__FUNCTION__,RDM_OID_QOS_CLS);							
		return ZCFG_INTERNAL_ERROR;				
	}
}

zcfgRet_t zcfgFeDalQosClass(const char *method, struct json_object *Jobj, struct json_object *Jarray, char *replyMsg) {
	zcfgRet_t ret = ZCFG_SUCCESS;

	if(!strcmp(method, "POST")) {
		ret = zcfgFeDalQosClassAdd(method,Jobj, replyMsg);
	}
	else if(!strcmp(method, "PUT")) {
		ret = zcfgFeDalQosClassEdit(method,Jobj, replyMsg);
	}
	else if(!strcmp(method, "DELETE")) {
		ret = zcfgFeDalQosClassDelete(Jobj, replyMsg);
	}
	else if(!strcmp(method, "GET")) {
		ret = zcfgFeDalQosClassGet(Jobj, Jarray, replyMsg);
	}
	else {
		printf("Unknown method:%s\n", method);
	}

	return ret;
}
