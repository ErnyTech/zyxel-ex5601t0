#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tr98Object_s {
    char            *name;             /*object full path name*/
}tr98Object_t;

#include "zcfg_tr98_common.h"
#include "zcfg_fe_tr98_handle_tmp.h"

typedef struct tr98_map_chk_s {
	zcfg98sn_t      sn;
    char            *name;             /*object full path name*/
	char            *sn_name;
}tr98_map_chk_t;

/*to be check*/
tr98_map_chk_t tr98_map_chk[] = {
	{e_TR98_CAPB,                   TR98_CAPB,                   "e_TR98_CAPB"},                   //68
	{e_TR98_IP_PING_DIAG,           TR98_IP_PING_DIAG,           "e_TR98_IP_PING_DIAG"},           //107
#ifdef CONFIG_ZCFG_BE_MODULE_DSL
	{e_TR98_WAN_DSL_INTF_CONF_STAT, TR98_WAN_DSL_INTF_CONF_STAT, "e_TR98_WAN_DSL_INTF_CONF_STAT"}, //180
#endif
	{e_TR98_WAN_ETH_INTF_CONF,      TR98_WAN_ETH_INTF_CONF,      "e_TR98_WAN_ETH_INTF_CONF"},      //187
	{e_TR98_FIREWALL,               TR98_FIREWALL,               "e_TR98_FIREWALL"},               //251
#ifdef ZYXEL_IPV6SPEC
	{e_TR98_IPV6_HOSTS,             TR98_IPV6_HOSTS,             "e_TR98_IPV6_HOSTS"},             //282
#endif
	{e_TR98_SYS_INFO,               TR98_SYS_INFO,               "e_TR98_SYS_INFO"},               //363
	{e_TR98_REMO_SRV,               TR98_REMO_SRV,               "e_TR98_REMO_SRV"},               //373
	{e_TR98_GUI_CUSTOMIZATION,      TR98_GUI_CUSTOMIZATION,      "e_TR98_GUI_CUSTOMIZATION"},      //415
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_ONECONNECT
	{e_TR98_ONECONNECT,             TR98_ONECONNECT,             "e_TR98_ONECONNECT"},             //420
#endif
#ifdef CONFIG_ZCFG_BE_MODULE_ZYXEL_IPP_CONFIGS
	{e_TR98_PRINT_SERVER,           TR98_PRINT_SERVER,           "e_TR98_PRINT_SERVER"},           //441
#endif
	{e_TR98_WAN_IP_V6ROUTINGTABLE,  TR98_WAN_IP_V6ROUTINGTABLE,  "e_TR98_WAN_IP_V6ROUTINGTABLE"},
	{0, NULL, NULL}
};

int main()
{
	int c = 0, sn = 0, chk_len = 0, tr98obj_len = 0;

	while(tr98_map_chk[c].name != NULL) {
		sn = tr98_map_chk[c].sn;
		
		chk_len     = strlen(tr98_map_chk[c].name);
		tr98obj_len = strlen(tr98Obj[sn - e_TR98_START].name);

		if(chk_len != tr98obj_len) {
			goto fail;
		}
		else if(!strncmp(tr98_map_chk[c].name, tr98Obj[sn - e_TR98_START].name, strlen(tr98_map_chk[c].name))) {
			printf("Mapping correct : %30s is mapping to %s\n", tr98_map_chk[c].sn_name, tr98Obj[sn - e_TR98_START].name);
		}
		else {
			goto fail;
		}
		c++;
	}

	return 0;

fail:
	//remove("./zcfg_fe_tr98_handle_tmp.h");
	printf("Mapping error   : %30s is mapping to %s\n", tr98_map_chk[c].sn_name, tr98Obj[sn - e_TR98_START].name);
	printf("Please make sure the mapping of tr98Obj in zcfg_fe_tr98_handle.h,\n");
	printf("zcfg98sn in zcfg/include/zcfg_tr98_common.h and tr98NameObjList in libzcfg_fe_tr98-2.0/zcfg_fe_tr98.c is correct.\n");
	return -1;
}
