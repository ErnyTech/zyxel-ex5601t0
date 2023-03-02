#ifndef _PLATFORM_CONFIG_H_
#define _PLATFORM_CONFIG_H_

/*
GE: >=
GT: >
LE: <=
LT: <
EQ: ==
*/

#ifdef BROADCOM_PLATFORM
#define BRCMSDK_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

/* 4.16L02 */
#if BRCM_SDK_VER <= BRCMSDK_VERSION(4,16,2)
#define BRCM_VER_LE_416L02 1
#endif

/* 4.16L04 */
#if BRCM_SDK_VER >= BRCMSDK_VERSION(4,16,4)
#define BRCM_VER_GE_416L04 1
#endif

/* 5.02L04 */
#if BRCM_SDK_VER < BRCMSDK_VERSION(5,02,4)
#define BRCM_VER_LT_502L04 1
#endif

#if BRCM_SDK_VER >= BRCMSDK_VERSION(5,02,4)
#define BRCM_VER_GE_502L04 1
#endif

/* 5.02L05 */
#if BRCM_SDK_VER >= BRCMSDK_VERSION(5,02,5)
#define BRCM_VER_GE_502L05 1
#endif

#if BRCM_SDK_VER == BRCMSDK_VERSION(5,02,5)
#define BRCM_VER_EQ_502L05 1
#endif

/* 5.02L07 */
#if BRCM_SDK_VER >= BRCMSDK_VERSION(5,02,7)
#define BRCM_VER_GE_502L07 1
#endif

/* 5.04L02 */
#if BRCM_SDK_VER >= BRCMSDK_VERSION(5,04,2)
#define BRCM_VER_GE_504L02 1
#endif

#endif
#endif
