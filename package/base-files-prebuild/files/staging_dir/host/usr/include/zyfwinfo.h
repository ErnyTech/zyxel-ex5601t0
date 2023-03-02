
#ifndef ZYFWINFO
#define ZYFWINFO

#include <stdint.h>

#define ZYFWINFI_PRODUCTNAME_LEN           32
#define ZYFWINFI_BUILDINFO_LEN             10
#define ZYFWINFI_FIRMWAREVERSION_LEN       30
#define ZYFWINFI_MODELID_LEN               4

#define DEFAULT_ZLOADER_VER_MAJ  1
#define DEFAULT_ZLOADER_VER_MIN  0
#define DEFAULT_ZLOADER_VER_ECO  0

#define ZYXEL_MAGIC	0x5a595845	// 'Z', 'Y', 'X', 'E'
#define ZYFWINFI_VERSION 2

#define ZYFWINFI_MAX_SEQNUM 9999
#define ZYFWINFI_MIN_SEQNUM 0

typedef struct
{
	uint32_t magic;
	uint16_t version;
	uint16_t seq_num; // sequence number, should be 0 ~ 9999
	uint16_t length;
	char buildInfo[ZYFWINFI_BUILDINFO_LEN];
	char productName[ZYFWINFI_PRODUCTNAME_LEN];
	char firmwareVersion[ZYFWINFI_FIRMWAREVERSION_LEN];
	char firmwareVersionEx[ZYFWINFI_FIRMWAREVERSION_LEN];
	uint16_t kernelChecksum;
	uint16_t rootFSChecksum;
	char modelId[ZYFWINFI_MODELID_LEN];
	char chUnused[134];
	uint16_t zy_checksum;
} ZY_FW_INFO, *PZY_FW_INFO;
#define ZY_FW_INFO_SIZE sizeof(ZY_FW_INFO)	//should be 256 bytes

#define OPT_LEN_IS_VALID(_maxLen) ( strlen(optarg) < (_maxLen) )

#endif
