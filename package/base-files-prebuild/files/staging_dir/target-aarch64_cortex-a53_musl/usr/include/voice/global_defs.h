#ifndef __ZYIMS_GLOBAL_DEFS_H__
#define __ZYIMS_GLOBAL_DEFS_H__

#define MAX_IFNAME_LEN		16 //Michael.20150518.001: Add by moving the ones defined in both 'sys_itf.h' & 'mm_core_rtp.h' to THIS new created header file 'global_defs.h' as a centralized definition.
#define IFNAME_LEN_MAX		MAX_IFNAME_LEN

#define IPV4_ADDR_STR_LEN	16 //Michael.20190911.001: Add the Generic/Common Utility function 'sys_misc.c/IPv4_BinaryFormToStr()' with the related Macro Constant 'global_defs.h/IPV4_ADDR_STR_LEN(16)' to improve the debug message.
//Michael.20200304.002.B: Add to define some more/additional IPv4 Address-related (Macro) Constants to help coding.
#define IPV4_ADDR_UINT		uint32
#define IPV4_ADDR_UINT_SIZE	32
#define IP_ADDR_STR_LEN	IPV4_ADDR_STR_LEN
#define IP_ADDR_UINT		IPV4_ADDR_UINT
#define IP_ADDR_UINT_SIZE	IPV4_ADDR_UINT_SIZE
//Michael.20200304.002.E: Add.


//Michael.20200304.001.B: Add to (a)define some (Macro) Constants and (b)implement some Generic Utility Macro Functions for the TCP/UDP Port (Range) Validation by referring to the info./knowledge of the web page: "https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers".
#define UDP_PORT_VALID_BEGIN  0
#define UDP_PORT_VALID_END  65535
#define UDP_PORT_WELL_KNOWN_BEGIN  0
#define UDP_PORT_WELL_KNOWN_END  1023
#define UDP_PORT_REGISTERED_BEGIN  1024
#define UDP_PORT_REGISTERED_END  49151
#define UDP_PORT_DYNAMIC_PRIVATE_BEGIN  49152
#define UDP_PORT_DYNAMIC_PRIVATE_END  65535

#define TCP_PORT_VALID_BEGIN  UDP_PORT_VALID_BEGIN
#define TCP_PORT_VALID_END  UDP_PORT_VALID_END
#define TCP_PORT_WELL_KNOWN_BEGIN  UDP_PORT_WELL_KNOWN_BEGIN
#define TCP_PORT_WELL_KNOWN_END  UDP_PORT_WELL_KNOWN_END
#define TCP_PORT_REGISTERED_BEGIN  UDP_PORT_REGISTERED_BEGIN
#define TCP_PORT_REGISTERED_END  UDP_PORT_REGISTERED_END
#define TCP_PORT_DYNAMIC_PRIVATE_BEGIN  UDP_PORT_DYNAMIC_PRIVATE_BEGIN
#define TCP_PORT_DYNAMIC_PRIVATE_END  UDP_PORT_DYNAMIC_PRIVATE_END

#define UDP_TCP_PORT_VALID_BEGIN  UDP_PORT_VALID_BEGIN
#define UDP_TCP_PORT_VALID_END  UDP_PORT_VALID_END
#define UDP_TCP_PORT_WELL_KNOWN_BEGIN  UDP_PORT_WELL_KNOWN_BEGIN
#define UDP_TCP_PORT_WELL_KNOWN_END  UDP_PORT_WELL_KNOWN_END
#define UDP_TCP_PORT_REGISTERED_BEGIN  UDP_PORT_REGISTERED_BEGIN
#define UDP_TCP_PORT_REGISTERED_END  UDP_PORT_REGISTERED_END
#define UDP_TCP_PORT_DYNAMIC_PRIVATE_BEGIN  UDP_PORT_DYNAMIC_PRIVATE_BEGIN
#define UDP_TCP_PORT_DYNAMIC_PRIVATE_END  UDP_PORT_DYNAMIC_PRIVATE_END

#define IS_VALID_UDP_PORT(p)  ((p >= 0) && (p <= 65535))
#define IS_UDP_PORT_VALID  IS_VALID_UDP_PORT
#define IS_UDP_PORT_VALID_GE_1024(p)  ((p >= 1024) && (p <= 65535)) //GE (Greater Than or Equal): >=.
#define IS_UDP_PORT_TYPE_WELL_KNOWN(p)  ((p >= 0) && (p <= 1023))
#define IS_UDP_PORT_TYPE_REGISTERED(p)  ((p >= 1024) && (p <= 49151))
#define IS_UDP_PORT_TYPE_DYNAMIC_PRIVATE_EPHEMERAL(p)  ((p >= 49152) && (p <= 65535))

#define IS_VALID_TCP_PORT  IS_VALID_UDP_PORT
#define IS_TCP_PORT_VALID  IS_VALID_UDP_PORT
#define IS_TCP_PORT_VALID_GE_1024  IS_UDP_PORT_VALID_GE_1024
#define IS_TCP_PORT_TYPE_WELL_KNOWN  IS_UDP_PORT_TYPE_WELL_KNOWN
#define IS_TCP_PORT_TYPE_REGISTERED  IS_UDP_PORT_TYPE_REGISTERED
#define IS_TCP_PORT_TYPE_DYNAMIC_PRIVATE_EPHEMERAL  IS_UDP_PORT_TYPE_DYNAMIC_PRIVATE_EPHEMERAL

#define IS_VALID_UDP_TCP_PORT  IS_VALID_UDP_PORT
#define IS_UDP_TCP_PORT_VALID  IS_VALID_UDP_PORT
#define IS_UDP_TCP_PORT_VALID_GE_1024  IS_UDP_PORT_VALID_GE_1024
#define IS_UDP_TCP_PORT_TYPE_WELL_KNOWN  IS_UDP_PORT_TYPE_WELL_KNOWN
#define IS_UDP_TCP_PORT_TYPE_REGISTERED  IS_UDP_PORT_TYPE_REGISTERED
#define IS_UDP_TCP_PORT_TYPE_DYNAMIC_PRIVATE_EPHEMERAL  IS_UDP_PORT_TYPE_DYNAMIC_PRIVATE_EPHEMERAL
//Michael.20200304.001.E: Add.


//Michael.20150527.002.B: Add new definitions for the Phone Port Index Min Value.
#define PHONE_INDEX_BASE	0
#define PHONE_INDEX_MIN	0
//Michael.20150527.002.E: Add.


//Anna.20210302.B: Add new definitions for the string length of time information.
#define TIME_INFO_STR_LEN 16 //hhhh:mm:ss.
#define TIME_DATA_STR_LEN TIME_INFO_STR_LEN
//Anna.20210302.E: Add.

#endif //__ZYIMS_GLOBAL_DEFS_H__.

