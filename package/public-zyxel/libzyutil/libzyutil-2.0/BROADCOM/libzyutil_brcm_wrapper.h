#ifdef BROADCOM_PLATFORM
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>

#include <ctype.h>
#include <unistd.h>
#include "zcfg_common.h"
#include "zcfg_debug.h"
#if defined(BROADCOM_UBOOT_PLATFORM) && defined(BCM_SDK_504L02)
#include "zyfwinfo.h"
#else
#include "bcm_hwdefs.h"
#endif
#include "board.h"

#if defined(TARGET_ROOTFS_UBIFS)
#define MagicNubmer 4255
#else
#define MagicNubmer 1985
#endif

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf);
int nvramDataWrite(NVRAM_DATA *nvramData);
int nvramDataGet(NVRAM_DATA *nvramData);

int zyFwInfoGet(ZY_FW_INFO *zyFwInfo);
int zyFwInfoNonBootGet(ZY_FW_INFO *zyFwInfo);


#if defined(ABUU_CUSTOMIZATION) //IChiaHuang 20200323
#define LED_NAME_LEN 24
int zyLedSet(char *name,int action);
char* zyLedNameMap(int index);
int zyLedIndexMaxNumGet(void);

typedef enum {
    ZY_POWER_G,
    ZY_POWER_R,
    ZY_INET_G,
    ZY_INET_R,
    ZY_ADSL0,
    ZY_VDSL0,
    ZY_ADSL1,
    ZY_VDSL1,
    ZY_VOIP0_G,
    ZY_VOIP0_O,
    ZY_VOIP1_G,
    ZY_VOIP1_O,
    ZY_WLAN0,
    ZY_WPS0,
    ZY_WLAN1,
    ZY_WPS1,
    ZY_HW_ETH0,
    ZY_HW_ETH1,
    ZY_HW_ETH2,
    ZY_HW_ETH3,
    ZY_HW_ETH4,
    ZY_HW_AGGREGATE,
//  ZY_HW_AGGREGATEACT,
//  ZY_HW_AGGREGATELNK,
    ZY_HW_WLAN0,
    ZY_HW_WLAN1,
    ZY_KHWLEDALL,
    ZY_KHWLEDALLEXCEPTPOWER,
    ZY_MAX_LED
}ZY_LEDTYPE;

typedef struct
{
    char zy_LedName[LED_NAME_LEN];
    ZY_LEDTYPE index;
} ZY_LEDCTL_INFO_COMMON, *PZY_LEDCTL_INFO_COMMON;

typedef struct
{
    ZY_LEDTYPE ledType;
    char zy_LedName[LED_NAME_LEN];
    int index;
    int port;
    int control;  //sw : BOARD_IOCTL_LED_CTRL , hw : BOARD_IOCTL_HW_LED_CTRL
} ZY_LEDCTL_INFO, *PZY_LEDCTL_INFO;

//common(all project) (Add only, don't delete)
static ZY_LEDCTL_INFO LedctlInfo[] =
{
//sw    {ledType        zy_LedName      index            port            control}
        {ZY_POWER_G,    "POWER_G",      kLedPowerOn,      -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_POWER_R,    "POWER_R",      kLedPostFail,     -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_ADSL0,      "ADSL0",        kLedAdsl,         -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_VDSL0,      "VDSL0",        kLedVdsl,         -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_INET_G,     "INET_G",       kLedInternetData, -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_INET_R,     "INET_R",       kLedInternetData, -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_VOIP0_G,    "VOIP0_G",      kLedVoip1,        -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_VOIP0_O,    "VOIP0_O",      kLedVoip1Msg,     -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_WLAN0,      "Wlan0",        0,                -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_WPS0,       "WPS0",         kLedSes,          -1,             BOARD_IOCTL_LED_CTRL},
        {ZY_WPS1,       "WPS1",         kLedSes2,         -1,             BOARD_IOCTL_LED_CTRL},
//hw    {ledType        zy_LedName      index             port            control}
        {ZY_HW_ETH0,    "eth0",         kHWLedEth,        -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_ETH1,    "eth1",         kHWLedEth,        -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_ETH2,    "eth2",         kHWLedEth,        -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_ETH3,    "eth3",         kHWLedEth,        -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_ETH4,    "eth4",         kHWLedEth,        -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_WLAN1,   "Wlan1",        0,                -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_HW_AGGREGATE,    "AGGREGATE",       kHWLedAggregate,     -1,     BOARD_IOCTL_HW_LED_CTRL},
      //{ZY_HW_AGGREGATEACT,    "AGGREGATEACT",       kHWLedAggregateAct,     -1,     BOARD_IOCTL_HW_LED_CTRL},
      //{ZY_HW_AGGREGATELNK,    "AGGREGATELNK",       kHWLedAggregateLnk,     -1,     BOARD_IOCTL_HW_LED_CTRL},
        {ZY_KHWLEDALLEXCEPTPOWER, "kHWLedAllExceptPower", kHWLedAllExceptPower,     -1,     BOARD_IOCTL_HW_LED_CTRL},
        {ZY_KHWLEDALL,    "kHWLedAll",      kHWLedAll,          -1,             BOARD_IOCTL_HW_LED_CTRL},
        {ZY_MAX_LED,    "NULL",         0,                -1,             0}
};
#define zyLedctlInfo LedctlInfo

int boardIoctl_LED(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf, int result);
#endif

#endif
