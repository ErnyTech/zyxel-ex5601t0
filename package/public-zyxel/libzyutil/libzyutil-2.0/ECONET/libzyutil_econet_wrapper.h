#ifdef ECONET_PLATFORM
#include <ctype.h>
#include <unistd.h>
#include "zcfg_common.h"

#if 1//defined(ZYXEL)
#include "tc_partition.h"
#include "trx.h"
#include "mrd.h"                      /* see $(STAGING_DIR_HOST)/include/mrd.h; mrd structure */
#define FLASH_READ_CMD                "/sbin/mtd -q -q readflash %s %lu %lu %s"
#define FLASH_WRITE_CMD               "/sbin/mtd -q -q writeflash %s %lu %lu %s"
#define BOOT_FLAG_SIZE                1
#if defined(IMG_BOOT_FLAG_OFFSET)
#define BOOT_FLAG_OFFSET              IMG_BOOT_FLAG_OFFSET
#else
#if defined(TCSUPPORT_CT)
#define BOOT_FLAG_OFFSET              (EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#else
#define BOOT_FLAG_OFFSET              (MRD_RA_OFFSET - BOOT_FLAG_SIZE)
#endif
#endif
#define BOOT_FLAG_FILE                "/tmp/boot_flag"
#ifdef MOS_SUPPORT
#define BOOT_FLAG_WRITEFILE           "/tmp/boot_flag_write"
#define IMG_HEADER_FILE               "/tmp/imgHeader"
#define IMG_HEADER_WRITEFILE          "/tmp/imgHeader_write"
#endif
#define MRDINFO_PARTITON_NAME         "bootloader"
#define MRDINFO_OFFSET                MRD_OFFSET
#define MRDINFO_SIZE                  MRD_SIZE
#define MRDINFO_FILE_X                "/tmp/MRD_info-XXXXXX"
#define MRDINFO_FILE                  "/tmp/MRD_info"
#define MAIN_ROOTFS_PARTITION_NAME    "tclinux"
#define SLAVE_ROOTFS_PARTITION_NAME   "tclinux_slave"
#define BOOTIMG_HEADER_OFFSET         0x0
#define BOOTIMG_HEADER_SIZE           sizeof(struct trx_header)
#define BOOTIMG_HEADER_FILE           "/tmp/BootImg_info"
#define ROMD_PARTITON_NAME				"rom-d"
#define DATA_PARTITON_NAME				"data"
#define HeaderNubmer1 3252
#define HeaderNubmer2 4448

#endif

zcfgRet_t zyUtilGetMrdInfo(PMRD_DATA mrd);
#ifndef TCSUPPORT_UBOOT
zcfgRet_t zyUtilSetMrdInfo(PMRD_DATA mrd);
#else
zcfgRet_t zyUtilSetMrdInfo(char *name, char *value);
#endif
int endian_check();

#ifdef ZYXEL_DETECT_PRODUCT_TYPE
#define ECONET_BOARDFEATURES_PROC_PATH "/proc/tc3162/prtype"
#define ECONET_BOARDFEATURES_PRNAME_PATH "/proc/tc3162/prname"
#define ECONET_BOARDFEATURES_NO_DSL		(1 << 0)
#define ECONET_BOARDFEATURES_NO_VOIP	(1 << 1)
#endif

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
#define ECONET_COFW_PRODUCT_NAME_PATH "/proc/zyxel/mrd_product_name"
#endif

#endif
