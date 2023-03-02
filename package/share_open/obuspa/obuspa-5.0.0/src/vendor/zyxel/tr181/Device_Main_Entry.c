/*
 * device_tr181_main.c
 *
 *  Created on: Feb 21, 2022
 *
 */
#include "device_data_node_api.h"
#include "vendor_data_model.h"

#define DEVICE_ROOT "Device"

// Add entry point functions of data node for TRXXX data model
//Device.ATM
extern int DEVICE_ATM_Init(void);
extern int DEVICE_ATM_Start(void);
extern int DEVICE_ATM_Stop(void);
//Device.Bridging
extern int DEVICE_BRIDGING_Init(void);
extern int DEVICE_BRIDGING_Start(void);
extern int DEVICE_BRIDGING_Stop(void);
//Device.BulkData
extern int DEVICE_BulkData_Init(void);
extern int DEVICE_BulkData_Start(void);
extern int DEVICE_BulkData_Stop(void);
//Device.CaptivePortal
extern int DEVICE_CAPTIVEPORTAL_Init(void);
extern int DEVICE_CAPTIVEPORTAL_Start(void);
extern int DEVICE_CAPTIVEPORTAL_Stop(void);
//Device.DeviceInfo
extern int DEVICE_DEVICEINFO_Init(void);
extern int DEVICE_DEVICEINFO_Start(void);
extern int DEVICE_DEVICEINFO_Stop(void);
//Device.DHCPv4
extern int DEVICE_DHCPV4_Init(void);
extern int DEVICE_DHCPV4_Start(void);
extern int DEVICE_DHCPV4_Stop(void);
//Device.DHCPv6
extern int DEVICE_DHCPV6_Init(void);
extern int DEVICE_DHCPV6_Start(void);
extern int DEVICE_DHCPV6_Stop(void);
//Device.DLNA
extern int DEVICE_DLNA_Init(void);
extern int DEVICE_DLNA_Start(void);
extern int DEVICE_DLNA_Stop(void);
//Device.DNS
extern int DEVICE_DNS_Init(void);
extern int DEVICE_DNS_Start(void);
extern int DEVICE_DNS_Stop(void);
//Device.DSL
extern int DEVICE_DSL_Init(void);
extern int DEVICE_DSL_Start(void);
extern int DEVICE_DSL_Stop(void);
//Device.DSLITE
extern int DEVICE_DSLITE_Init(void);
extern int DEVICE_DSLITE_Start(void);
extern int DEVICE_DSLITE_Stop(void);
//Device.Ethernet
extern int DEVICE_ETHERNET_Init(void);
extern int DEVICE_ETHERNET_Start(void);
extern int DEVICE_ETHERNET_Stop(void);
//Device.FAP
extern int DEVICE_FAP_Init(void);
extern int DEVICE_FAP_Start(void);
extern int DEVICE_FAP_Stop(void);
//Device.FaultMgmt
extern int DEVICE_FAULTMGMT_Init(void);
extern int DEVICE_FAULTMGMT_Start(void);
extern int DEVICE_FAULTMGMT_Stop(void);
//Device.Firewall
extern int DEVICE_FIREWALL_Init(void);
extern int DEVICE_FIREWALL_Start(void);
extern int DEVICE_FIREWALL_Stop(void);
//Device.GatewayInfo
extern int DEVICE_GATEWAYINFO_Init(void);
extern int DEVICE_GATEWAYINFO_Start(void);
extern int DEVICE_GATEWAYINFO_Stop(void);
//Device.Ghn
extern int DEVICE_GHN_Init(void);
extern int DEVICE_GHN_Start(void);
extern int DEVICE_GHN_Stop(void);
//Device.GRE
extern int DEVICE_GRE_Init(void);
extern int DEVICE_GRE_Start(void);
extern int DEVICE_GRE_Stop(void);
//Device.Homeplug
extern int DEVICE_HOMEPLUG_Init(void);
extern int DEVICE_HOMEPLUG_Start(void);
extern int DEVICE_HOMEPLUG_Stop(void);
//Device.Hosts
extern int DEVICE_HOSTS_Init(void);
extern int DEVICE_HOSTS_Start(void);
extern int DEVICE_HOSTS_Stop(void);
//Device.HPNA
extern int DEVICE_HPNA_Init(void);
extern int DEVICE_HPNA_Start(void);
extern int DEVICE_HPNA_Stop(void);
//Device.IEEE8021x
extern int DEVICE_IEEE8021X_Init(void);
extern int DEVICE_IEEE8021X_Start(void);
extern int DEVICE_IEEE8021X_Stop(void);
//Device.InterfaceStack.i
extern int DEVICE_INTERFACESTACK_Init(void);
extern int DEVICE_INTERFACESTACK_Start(void);
extern int DEVICE_INTERFACESTACK_Stop(void);
//Device.IP
extern int DEVICE_IP_Init(void);
extern int DEVICE_IP_Start(void);
extern int DEVICE_IP_Stop(void);
//Device.IPsec
extern int DEVICE_IPSEC_Init(void);
extern int DEVICE_IPSEC_Start(void);
extern int DEVICE_IPSEC_Stop(void);
//Device.IPv6rd
extern int DEVICE_IPV6RD_Init(void);
extern int DEVICE_IPV6RD_Start(void);
extern int DEVICE_IPV6RD_Stop(void);
//Device.LANConfigSecurity
extern int DEVICE_LANCONFIGSECURITY_Init(void);
extern int DEVICE_LANCONFIGSECURITY_Start(void);
extern int DEVICE_LANCONFIGSECURITY_Stop(void);
//Device.ManagementServer
extern int DEVICE_MANAGEMENTSERVER_Init(void);
extern int DEVICE_MANAGEMENTSERVER_Start(void);
extern int DEVICE_MANAGEMENTSERVER_Stop(void);
//Device.MAP
extern int DEVICE_MAP_Init(void);
extern int DEVICE_MAP_Start(void);
extern int DEVICE_MAP_Stop(void);
//Device.MoCA
extern int DEVICE_MOCA_Init(void);
extern int DEVICE_MOCA_Start(void);
extern int DEVICE_MOCA_Stop(void);
//Device.NAT
extern int DEVICE_NAT_Init(void);
extern int DEVICE_NAT_Start(void);
extern int DEVICE_NAT_Stop(void);
//Device.NeighborDiscovery
extern int DEVICE_NEIGHBORDISCOVERY_Init(void);
extern int DEVICE_NEIGHBORDISCOVERY_Start(void);
extern int DEVICE_NEIGHBORDISCOVERY_Stop(void);
//Device.Optical
extern int DEVICE_OPTICAL_Init(void);
extern int DEVICE_OPTICAL_Start(void);
extern int DEVICE_OPTICAL_Stop(void);
//Device.PeriodicStatistics
extern int DEVICE_PERRIODICSTATISTICS_Init(void);
extern int DEVICE_PERRIODICSTATISTICS_Start(void);
extern int DEVICE_PERRIODICSTATISTICS_Stop(void);
//Device.PPP
extern int DEVICE_PPP_Init(void);
extern int DEVICE_PPP_Start(void);
extern int DEVICE_PPP_Stop(void);
//Device.PTM
extern int DEVICE_PTM_Init(void);
extern int DEVICE_PTM_Start(void);
extern int DEVICE_PTM_Stop(void);
//Device.QoS
extern int DEVICE_QOS_Init(void);
extern int DEVICE_QOS_Start(void);
extern int DEVICE_QOS_Stop(void);
//Device.RouterAdvertisement
extern int DEVICE_ROUTERADV_Init(void);
extern int DEVICE_ROUTERADV_Start(void);
extern int DEVICE_ROUTERADV_Stop(void);
//Device.Routing
extern int DEVICE_ROUTING_Init(void);
extern int DEVICE_ROUTING_Start(void);
extern int DEVICE_ROUTING_Stop(void);
//Device.Security
extern int DEVICE_Security_Init(void);
extern int DEVICE_Security_Start(void);
extern int DEVICE_Security_Stop(void);
//Device.SelfTestDiagnostics
extern int DEVICE_SELFTESTDIAG_Init(void);
extern int DEVICE_SELFTESTDIAG_Start(void);
extern int DEVICE_SELFTESTDIAG_Stop(void);
//Device.Services
extern int DEVICE_SERVICES_Init(void);
extern int DEVICE_SERVICES_Start(void);
extern int DEVICE_SERVICES_Stop(void);
//Device.SmartCardReaders
extern int DEVICE_SMARTCARDREADERS_Init(void);
extern int DEVICE_SMARTCARDREADERS_Start(void);
extern int DEVICE_SMARTCARDREADERS_Stop(void);
//Device.SoftwareModules
extern int DEVICE_SOFTWAREMODULES_Init(void);
extern int DEVICE_SOFTWAREMODULES_Start(void);
extern int DEVICE_SOFTWAREMODULES_Stop(void);
//Device.Time
extern int DEVICE_Time_Init(void);
extern int DEVICE_Time_Start(void);
extern int DEVICE_Time_Stop(void);
//Device.UPA
extern int DEVICE_UPA_Init(void);
extern int DEVICE_UPA_Start(void);
extern int DEVICE_UPA_Stop(void);
//Device.UPnP
extern int DEVICE_UPNP_Init(void);
extern int DEVICE_UPNP_Start(void);
extern int DEVICE_UPNP_Stop(void);
//Device.USB
extern int DEVICE_USB_Init(void);
extern int DEVICE_USB_Start(void);
extern int DEVICE_USB_Stop(void);
//Device.UserInterface
extern int DEVICE_USERINTERFACE_Init(void);
extern int DEVICE_USERINTERFACE_Start(void);
extern int DEVICE_USERINTERFACE_Stop(void);
//Device.Users
extern int DEVICE_USERS_Init(void);
extern int DEVICE_USERS_Start(void);
extern int DEVICE_USERS_Stop(void);
//Device.WiFi
extern int DEVICE_WIFI_Init(void);
extern int DEVICE_WIFI_Start(void);
extern int DEVICE_WIFI_Stop(void);
//Device.XMPP
extern int DEVICE_XMPP_Init(void);
extern int DEVICE_XMPP_Start(void);
extern int DEVICE_XMPP_Stop(void);


DEVICE_DATA_NODE_CB TR181_Data_Node[] = {
	{ DEVICE_ATM_Init, DEVICE_ATM_Start, DEVICE_ATM_Stop }, // Device.ATM
	{ DEVICE_BRIDGING_Init, DEVICE_BRIDGING_Start, DEVICE_BRIDGING_Stop }, // Device.Bridging
	{ DEVICE_BulkData_Init, DEVICE_BulkData_Start, DEVICE_BulkData_Stop }, // Device.BulkData
	{ DEVICE_CAPTIVEPORTAL_Init, DEVICE_CAPTIVEPORTAL_Start, DEVICE_CAPTIVEPORTAL_Stop }, // Device.CaptivePortal
	{ DEVICE_DEVICEINFO_Init, DEVICE_DEVICEINFO_Start, DEVICE_DEVICEINFO_Stop }, // Device.DeviceInfo
	{ DEVICE_DHCPV4_Init, DEVICE_DHCPV4_Start, DEVICE_DHCPV4_Stop }, // Device.DHCPv4
	{ DEVICE_DHCPV6_Init, DEVICE_DHCPV6_Start, DEVICE_DHCPV6_Stop }, // Device.DHCPv6
	{ DEVICE_DLNA_Init, DEVICE_DLNA_Start, DEVICE_DLNA_Stop }, // Device.DLNA
	{ DEVICE_DNS_Init, DEVICE_DNS_Start, DEVICE_DNS_Stop }, // Device.DNS
	{ DEVICE_DSL_Init, DEVICE_DSL_Start, DEVICE_DSL_Stop }, // Device.DSL
	{ DEVICE_DSLITE_Init, DEVICE_DSLITE_Start, DEVICE_DSLITE_Stop }, // Device.DSLite
	{ DEVICE_ETHERNET_Init, DEVICE_ETHERNET_Start, DEVICE_ETHERNET_Stop }, // Device.Ethernet
	{ DEVICE_FAP_Init, DEVICE_FAP_Start, DEVICE_FAP_Stop }, // Device.FAP
	{ DEVICE_FAULTMGMT_Init, DEVICE_FAULTMGMT_Start, DEVICE_FAULTMGMT_Stop }, // Device.FaultMgmt
	{ DEVICE_FIREWALL_Init, DEVICE_FIREWALL_Start, DEVICE_FIREWALL_Stop }, // Device.Firewall
	{ DEVICE_GATEWAYINFO_Init, DEVICE_GATEWAYINFO_Start, DEVICE_GATEWAYINFO_Stop }, // Device.GatewayInfo
	{ DEVICE_GHN_Init, DEVICE_GHN_Start, DEVICE_GHN_Stop }, // Device.Ghn
	{ DEVICE_GRE_Init, DEVICE_GRE_Start, DEVICE_GRE_Stop }, // Device.GRE
	{ DEVICE_HOMEPLUG_Init, DEVICE_HOMEPLUG_Start, DEVICE_HOMEPLUG_Stop }, // Device.Homeplug
	{ DEVICE_HOSTS_Init, DEVICE_HOSTS_Start, DEVICE_HOSTS_Stop }, // Device.Hosts
	{ DEVICE_HPNA_Init, DEVICE_HPNA_Start, DEVICE_HPNA_Stop }, // Device.HPNA
	{ DEVICE_IEEE8021X_Init, DEVICE_IEEE8021X_Start, DEVICE_IEEE8021X_Stop }, // Device.IEEE8021x
	{ DEVICE_INTERFACESTACK_Init, DEVICE_INTERFACESTACK_Start, DEVICE_INTERFACESTACK_Stop }, // Device.InterfaceStack.i
	{ DEVICE_IP_Init, DEVICE_IP_Start, DEVICE_IP_Stop }, // Device.IP
	{ DEVICE_IPSEC_Init, DEVICE_IPSEC_Start, DEVICE_IPSEC_Stop }, // Device.IPsec
	{ DEVICE_IPV6RD_Init, DEVICE_IPV6RD_Start, DEVICE_IPV6RD_Stop }, // Device.IPv6rd
	{ DEVICE_LANCONFIGSECURITY_Init, DEVICE_LANCONFIGSECURITY_Start, DEVICE_LANCONFIGSECURITY_Stop }, // Device.LANConfigSecurity
	{ DEVICE_MANAGEMENTSERVER_Init, DEVICE_MANAGEMENTSERVER_Start, DEVICE_MANAGEMENTSERVER_Stop }, // Device.ManagementServer
	{ DEVICE_MAP_Init, DEVICE_MAP_Start, DEVICE_MAP_Stop }, // Device.MAP
	{ DEVICE_MOCA_Init, DEVICE_MOCA_Start, DEVICE_MOCA_Stop }, // Device.MoCA
	{ DEVICE_NAT_Init, DEVICE_NAT_Start, DEVICE_NAT_Stop }, // Device.NAT
	{ DEVICE_NEIGHBORDISCOVERY_Init, DEVICE_NEIGHBORDISCOVERY_Start, DEVICE_NEIGHBORDISCOVERY_Stop }, // Device.NeighborDiscovery
	{ DEVICE_OPTICAL_Init, DEVICE_OPTICAL_Start, DEVICE_OPTICAL_Stop }, // Device.Optical
	{ DEVICE_PERRIODICSTATISTICS_Init, DEVICE_PERRIODICSTATISTICS_Start, DEVICE_PERRIODICSTATISTICS_Stop }, // Device.PeriodicStatistics
	{ DEVICE_PPP_Init, DEVICE_PPP_Start, DEVICE_PPP_Stop }, // Device.PPP
	{ DEVICE_PTM_Init, DEVICE_PTM_Start, DEVICE_PTM_Stop }, // Device.PTM
	{ DEVICE_QOS_Init, DEVICE_QOS_Start, DEVICE_QOS_Stop }, // Device.QoS
	{ DEVICE_ROUTERADV_Init, DEVICE_ROUTERADV_Start, DEVICE_ROUTERADV_Stop }, // Device.RouterAdvertisement
	{ DEVICE_ROUTING_Init, DEVICE_ROUTING_Start, DEVICE_ROUTING_Stop }, // Device.Routing
	{ DEVICE_Security_Init, DEVICE_Security_Start, DEVICE_Security_Stop }, // Device.Security
	{ DEVICE_SELFTESTDIAG_Init, DEVICE_SELFTESTDIAG_Start, DEVICE_SELFTESTDIAG_Stop }, // Device.SelfTestDiagnostics
	{ DEVICE_SERVICES_Init, DEVICE_SERVICES_Start, DEVICE_SERVICES_Stop }, // Device.Services
	{ DEVICE_SMARTCARDREADERS_Init, DEVICE_SMARTCARDREADERS_Start, DEVICE_SMARTCARDREADERS_Stop }, // Device.SmartCardReaders
	{ DEVICE_SOFTWAREMODULES_Init, DEVICE_SOFTWAREMODULES_Start, DEVICE_SOFTWAREMODULES_Stop }, // Device.SoftwareModules
	{ DEVICE_Time_Init, DEVICE_Time_Start, DEVICE_Time_Stop }, // Device.Time
	{ DEVICE_UPA_Init, DEVICE_UPA_Start, DEVICE_UPA_Stop }, // Device.UPA
	{ DEVICE_UPNP_Init, DEVICE_UPNP_Start, DEVICE_UPNP_Stop }, // Device.UPnP
	{ DEVICE_USB_Init, DEVICE_USB_Start, DEVICE_USB_Stop }, // Device.USB
	{ DEVICE_USERINTERFACE_Init, DEVICE_USERINTERFACE_Start, DEVICE_USERINTERFACE_Stop }, // Device.UserInterface
	{ DEVICE_USERS_Init, DEVICE_USERS_Start, DEVICE_USERS_Stop }, // Device.Users
	{ DEVICE_WIFI_Init, DEVICE_WIFI_Start, DEVICE_WIFI_Stop }, // Device.WiFi
	{ DEVICE_XMPP_Init, DEVICE_XMPP_Start, DEVICE_XMPP_Stop }, // Device.XMPP
};


//------------------------------------------------------------------------------
// Data node callback functions

int DEVICE_ROOT_Get_Value_CB(dm_req_t *req, char *buf, int len){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return Get_Object_Parameter_Value(req,buf,len);
}

int DEVICE_ROOT_Set_Value_CB(dm_req_t *req, char *buf){
	//USP_LOG_Debug("req->path[%s]\n",req->path);
	return Set_Object_Parameter_Value(req,buf);
}

DM_REGISTER_DATA_NODE_CB Register_DEVICE_ROOT = {
	DEVICE_ROOT_Get_Value_CB,
	DEVICE_ROOT_Set_Value_CB,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/*********************************************************************//**
 **
 ** Device_TR181_Init
 **
 ** Initialises this component, and registers all parameters and vendor hooks, which it implements
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/

int Device_TR181_Init(void) {
	int err = USP_ERR_OK;
	int i = 0;

	err |= Register_Object_Paramter_Node(DEVICE_ROOT,&Register_DEVICE_ROOT);
	if(err == USP_ERR_OK){
		DEVICE_DATA_NODE_CB *pDataNode = NULL;
		for(i= 0;i < sizeof(TR181_Data_Node)/sizeof(TR181_Data_Node[0]);i++){
				pDataNode =  &TR181_Data_Node[i];
				if(pDataNode->device_data_node_init != NULL){
					err |= pDataNode->device_data_node_init();
				}

		}
	}
	return err;
}

/*********************************************************************//**
 **
 ** Device_TR181_Start
 **
 ** Called after data model has been registered and after instance numbers have been read from the USP database
 ** Typically this function is used to seed the data model with instance numbers or
 ** initialise internal data structures which require the data model to be running to access parameters
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/
int Device_TR181_Start(void) {
	int err = USP_ERR_OK;
	int i = 0;
	DEVICE_DATA_NODE_CB *pDataNode = NULL;
	for(i= 0;i < sizeof(TR181_Data_Node)/sizeof(TR181_Data_Node[0]);i++){
			pDataNode =  &TR181_Data_Node[i];
			if(pDataNode->device_data_node_start != NULL){
				err |= pDataNode->device_data_node_start();
			}
	}
	return err;
}
/*********************************************************************//**
 **
 ** Device_TR181_Stop
 **
 ** Called when stopping USP agent gracefully, to free up memory and shutdown
 ** any vendor processes etc
 **
 ** \param   None
 **
 ** \return  USP_ERR_OK if successful
 **
 **************************************************************************/
int Device_TR181_Stop(void) {
	int err = USP_ERR_OK;
	int i = 0;
	DEVICE_DATA_NODE_CB *pDataNode = NULL;
	for(i= 0;i < sizeof(TR181_Data_Node)/sizeof(TR181_Data_Node[0]);i++){
			pDataNode =  &TR181_Data_Node[i];
			if(pDataNode->device_data_node_stop != NULL){
				err |= pDataNode->device_data_node_stop();
			}
	}
	return err;
}

