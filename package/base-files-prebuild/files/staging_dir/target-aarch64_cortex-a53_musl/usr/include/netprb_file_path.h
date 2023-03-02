#ifndef _NETPRB_FILE_PATH_H
#define _NETPRB_FILE_PATH_H

/*PERSISTENT_STORAGE_PATH is the persistent flash dir*/
#ifndef PERSISTENT_STORAGE_PATH
#if 1 //zcfg
#define PERSISTENT_STORAGE_PATH       "/data"
#else
#define PERSISTENT_STORAGE_PATH       "/etc"
#endif
#endif

#define NETPROBE_DATABASE_MEM_PATH    "/tmp/netprobe.db"
#define NETPROBE_ZY1905_JSON_PATH     "/tmp/zy1905.json"

#if 1 //zcfg
#define NETPROBE_DATABASE_FLASH_PATH  PERSISTENT_STORAGE_PATH"/netprobe.db"
#define NETPROBE_NETMAP_RECORD_PATH   PERSISTENT_STORAGE_PATH"/netmap_record"
#define NETPROBE_DATABASE_VER_PATH    PERSISTENT_STORAGE_PATH"/netprb_db_ver"

#else
#define NETPROBE_DATABASE_FLASH_PATH  PERSISTENT_STORAGE_PATH"/netprobe.db"
#define NETPROBE_NETMAP_RECORD_PATH   PERSISTENT_STORAGE_PATH"/netmap_record"
#define NETPROBE_DATABASE_VER_PATH    PERSISTENT_STORAGE_PATH"/netprb_db_ver"
#endif

/*
 *  The format of NETPRB_DHCP_TABLE_PATH must be:
 *  MAC IP HostName
 *  ex:
 *    00:f7:6f:f1:3a:61 192.168.1.34 TPDC-SW2-iPad
 *
 */
#if 0
#define NETPROBE_DHCP_TABLE_PATH         "/tmp/dhcp.leases2"
#define NETPROBE_GUEST1_DHCP_TABLE_PATH  "/tmp/dhcp_guest_lan1.leases2"
#define NETPROBE_GUEST2_DHCP_TABLE_PATH  "/tmp/dhcp_guest_lan2.leases2"
#define NETPROBE_GUEST3_DHCP_TABLE_PATH  "/tmp/dhcp_guest_lan3.leases2"
#endif
#if 1 //zcfg
#define DHCP_LEASE_TABLE_PATH         "/var/dnsmasq/dnsmasq.leases"
#endif

#define NETPROBE_P0F_CLIENT_BIN_PATH  "/usr/bin/p0f-client"
#define NETPROBE_P0F_SOCK_PATH        "/tmp/p0f.sock"

#define ARP_TABLE_PATH                "/proc/net/arp"

#define _PATH_WIFI_24G				"/tmp/assoc_list.24g"
#define _PATH_WIFI_5G				"/tmp/assoc_list.5g"

#if defined(TC_SDK_7_3_245_300) && defined(ZYXEL_EASYMESH)
#define NETPROBE_DEVICE_LIST	"/tmp/ntprb_device_list"
#define UPDATE_DEVICE_LIST		"/tmp/update_device_list"
#endif

#if defined(ZY_ECN_NO_MESH_CUSTOM)
#define _PATH_ECNT_MAP_ASSOC_LIST	"/tmp/ecnt_map_assoc_list"
#endif

#endif
