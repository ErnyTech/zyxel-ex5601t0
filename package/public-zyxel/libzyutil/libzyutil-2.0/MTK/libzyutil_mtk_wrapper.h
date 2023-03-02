#ifdef MTK_PLATFORM
#include <ctype.h>
#include <unistd.h>
#include "zcfg_common.h"
#include "zyfwinfo.h"
#include <byteswap.h>

#if 1//defined(ZYXEL)
//#include "tc_partition.h"
//#include "trx.h"
//#include "mrd.h"                      /* see $(STAGING_DIR_HOST)/include/mrd.h; mrd structure */
#define FLASH_READ_CMD                "/sbin/mtd -q -q readflash %s %lu %lu %s"
#define FLASH_WRITE_CMD               "/sbin/mtd -q -q writeflash %s %lu %lu %s"
#define BOOT_FLAG_SIZE                1
#if defined(IMG_BOOT_FLAG_OFFSET)
#define BOOT_FLAG_OFFSET              IMG_BOOT_FLAG_OFFSET
#else
#if defined(TCSUPPORT_CT)
#define BOOT_FLAG_OFFSET              (EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#else
#define BOOT_FLAG_OFFSET              0 //(MRD_RA_OFFSET - BOOT_FLAG_SIZE)
#endif
#endif
#define BOOT_FLAG_FILE                "/tmp/boot_flag"
#ifdef MOS_SUPPORT
#define BOOT_FLAG_WRITEFILE           "/tmp/boot_flag_write"
#define IMG_HEADER_FILE               "/tmp/imgHeader"
#define IMG_HEADER_WRITEFILE          "/tmp/imgHeader_write"
#endif
//#define MRDINFO_PARTITON_NAME         "bootloader"
//#define MRDINFO_OFFSET                MRD_OFFSET
//#define MRDINFO_SIZE                  MRD_SIZE
//#define MRDINFO_FILE_X                "/tmp/MRD_info-XXXXXX"
//#define MRDINFO_FILE                  "/tmp/MRD_info"
#define MAIN_ROOTFS_PARTITION_NAME    "tclinux"
#define SLAVE_ROOTFS_PARTITION_NAME   "tclinux_slave"
#define BOOTIMG_HEADER_OFFSET         0x0
#define BOOTIMG_HEADER_SIZE           sizeof(struct trx_header)
#define BOOTIMG_HEADER_FILE           "/tmp/BootImg_info"
#define ROMD_PARTITON_NAME				"rom-d"
#define DATA_PARTITON_NAME				"data"
#define HeaderNubmer1 7573
#define HeaderNubmer2 7973

#endif

//zcfgRet_t zyUtilGetMrdInfo(PMRD_DATA mrd);
//zcfgRet_t zyUtilSetMrdInfo(PMRD_DATA mrd);
int endian_check();

#if 0 // def ZYXEL_DETECT_PRODUCT_TYPE
#define ECONET_BOARDFEATURES_PROC_PATH "/proc/tc3162/prtype"
#define ECONET_BOARDFEATURES_PRNAME_PATH "/proc/tc3162/prname"
#define ECONET_BOARDFEATURES_NO_DSL		(1 << 0)
#define ECONET_BOARDFEATURES_NO_VOIP	(1 << 1)
#endif

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
#define MTK_COFW_PRODUCT_NAME_PATH "/proc/zyxel/mrd_product_name"
#endif

/* Uboot environments */
#define ENV_VENDOR_NAME             "VendorName"
#define ENV_PRODUCT_NAME            "ProductName"
#define ENV_SERIAL_NUMBER           "SerialNumber"
#define ENV_GPON_SERIAL_NUMBER      "gponsn"
#define ENV_MAC_ADDRESS             "ethaddr"
#define ENV_WPAPSKKEY               "WpaPskKey"
#define ENV_ADMIN                   "admin"
#define ENV_SUPERVISOR              "supervisor"
#define ENV_MAC_ADDRESS_QUANTITY    "nummacaddrs"
#define ENV_COUNTRY_CODE            "countryCode"
#define ENV_DEBUGFLAG               "EngDebugFlag"
#define ENV_MAINFEATUREBIT          "FeatureBit"
#define ENV_FEATUREBITS             "FeatureBits"
#define ENV_SERIAL_NUMBER_81        "sn81"
#define ENV_ZLD_VERSION             "zld_ver"
#define ENV_ZLD_DATE                "zld_date"
#define ENV_ZLD_TIME                "zld_time"
#define ENV_TUTK_UID                "T_UID"

#define MTK_VENDOR_NAME_LEN         32
#define MTK_PRODUCT_NAME_LEN        32
#define MTK_MAC_ADDR_LEN            6
#define MTK_FEATUREBITS_LEN         30
#define MTK_SERIAL_NUM_LEN          14
#define MTK_GPON_SERIAL_NUM_LEN     8
#define MTK_BUILD_VERSION_LEN       4
#define MTK_BUILD_DATE_LEN          11
#define MTK_BUILD_TIME_LEN          9
#define MTK_KEYPASSPHRASE_LEN       16
#define MTK_SUPER_PWD_LEN           10
#define MTK_ADMIN_PWD_LEN           10

#define FEATUREBITS_SUB_SERIAL_NUM_LEN    1
#define FEATUREBITS_SUB_SERIAL_NUM_OFFSET 16

#if defined(BUILD_ZYXEL_VOICE_MTK_D2_VE_SUPPORT)
#define D2KEY_LENGTH                44
#define ENV_D2KEY                   "d2key"
#endif

typedef struct uboot_env_s {
    char envName[64];
    char envContent[128];
} uboot_env_t;

#endif
