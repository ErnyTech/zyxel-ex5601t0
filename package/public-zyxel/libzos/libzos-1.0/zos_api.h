#ifndef _ZOS_API_H
#define _ZOS_API_H
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "zlog_api.h"

//==============================================================================
#define ZOS_MAX_PROCESS_TIME       2000 // milli-second
#define BUFFER_SIZE         2048
#define SYNC_TYPE                0
#define NORMAL_TYPE              1

#ifdef ZCFG_MEMORY_LEAK_DETECT
#define ZCFG_ALLOC_RECORD_LEN 64
void dumpZcfgAllocRecord();
extern void dumpZcfgAllocFrontendRecord();
void dumpAllocRecord();
void recordAllocData(char *, char *, int);
void recordZcfgAllocData(void *, const char *, int, int);
void _ZOS_FREE(void*);
void* _ZOS_MALLOC(int, const char *, int);
#endif

//==============================================================================
/*
    Public MACRO
*/
#ifdef ZCFG_MEMORY_LEAK_DETECT
#define ZOS_MALLOC(_size_) _ZOS_MALLOC(_size_, __FILE__, __LINE__)
#else
#define ZOS_MALLOC(_size_) \
    ({\
        void *_p_ = NULL; \
        do \
        { \
            _p_ = calloc(1, (size_t)(_size_)); \
            if (_p_ == NULL) \
            { \
                ZLOG_ERROR("fail to calloc memory, size = %zu", (size_t)(_size_)); \
            } \
        } \
        while (0); \
        _p_; \
    })
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
#define ZOS_FREE(_p_) do{_ZOS_FREE((void *)_p_);_p_=NULL;}while(0)
#else
#define ZOS_FREE(_p_) \
    {\
        do \
        { \
            if ((_p_) == NULL) \
            { \
                ZLOG_DEBUG("Free NULL pointer"); \
            } \
            else \
            { \
                free((void *)(_p_)); \
                (_p_) = NULL; \
            } \
        } \
        while (0); \
    }
#endif

#define ZOS_SUCCESS	0
#define ZOS_FAIL	-1

/*
    use system()
*/
#define ZOS_SYSTEM(_fmt_, ...) \
    ({ \
        int    zb = zos_system(true, _fmt_, ##__VA_ARGS__); \
        if (zb == ZOS_FAIL) \
        { \
            ZLOG_ERROR("fail to run system()"); \
        } \
        zb; \
    })

/*
    use zcfg_system() for security
*/
#define ZOS_SYSTEM_S(_fmt_, ...) \
    ({ \
        int    zb = zos_system(false, _fmt_, ##__VA_ARGS__); \
        if (zb == ZOS_FAIL) \
        { \
            ZLOG_ERROR("fail to run system()"); \
        } \
        zb; \
    })


#define ZOS_STRDUP(str) _zos_strdup(str, __FILE__, __LINE__)
#define ZOS_STRNCPY(d, s, l) do{strncpy(d, s, l-1);*(d+l-1) = '\0';}while(0)
#define ZOS_STRCAT(dst, src, bufferSize) zos_strcat(dst, src, bufferSize)
#define ZOS_MOVE_FILE(dst, src)       zos_copyFile(dst, src, MOVE)
#define ZOS_COPY_FILE(dst, src)       zos_copyFile(dst, src, COPY)
#define ZOS_COPY_LINK_FILE(dst, src)  zos_copyFile(dst, src, COPY_LINK)
#define ZOS_REMOVE_DIR(dirPath)       zos_removeDir(dirPath)
#define ZOS_MKDIR(dirPath)            zos_mkdir(dirPath)
#define ZOS_TOUCH_FILE(fileName)      zos_touchFile(fileName)
#define ZOS_FCLOSE_S(fp)              zos_fclose(fp, SYNC_TYPE)
#define ZOS_FCLOSE(fp)                zos_fclose(fp, NORMAL_TYPE)
#define ZOS_FOPEN(fileName, mode)     fopen(fileName, mode)

#define ZOS_ECHO_ACCOUNT_INFO(fileName, strValue, strLength, mode)    zos_echoAccountInfo(fileName, strValue, strLength, mode, __FUNCTION__, __LINE__)
#define ZOS_ECHO_STR_TO_FILE(fileName, strValue, strLength, mode)    zos_echoStrToFile(fileName, strValue, strLength, mode, __FUNCTION__, __LINE__)
#define ZOS_IFCONFIG_UP(intfName)                 zos_ifconfigUp(intfName, __FUNCTION__, __LINE__)
#define ZOS_IFCONFIG_DOWN(intfName)               zos_ifconfigDown(intfName, __FUNCTION__, __LINE__)
#define ZOS_IFCONFIG_SET_FLAGS(intfName, flags)       zos_ifconfigSetFlags(intfName, flags, __FUNCTION__, __LINE__)

#define ZOS_IFCONFIG_SET_MTU(intfName, mtu)       zos_ifconfigSetMtu(intfName, mtu, __FUNCTION__, __LINE__)
#define ZOS_IFCONFIG_SET_MAC(intfName, mac)       zos_ifconfigSetMac(intfName, mac, __FUNCTION__, __LINE__)
#define ZOS_IFCONFIG_SET_IP(intfName, ip, netmask, broadcast, ifconfig_action) \
                                                  zos_ifconfigSetIPAddr(intfName, ip, netmask, broadcast, ifconfig_action, __FUNCTION__, __LINE__)

/*
    !!!!!! WARNNING !!!!!!
    "DO NOT" use these macro when writing new code.
	It's a interim solution for skipping insecure checking,
      to remind the code owner to fix these unsafe function.
*/
//interim solution macro -->
#define ZOS_STRCPY_M(dst, src) strcpy(dst, src)
#define ZOS_STRCAT_M(dst, src) strcat(dst, src)
#define ZOS_SPRINTF_M(str, fromat, ...) sprintf(str, fromat, ##__VA_ARGS__)
//interim solution macro <--

typedef enum
{
    MOVE,
    COPY,
	COPY_LINK,
} file_action;

typedef enum
{
    IFCONFIG_NONE=0,
    IFCONFIG_UP,
    IFCONFIG_DOWN

} ifconfig_action;

//==============================================================================
/*!
 *  system command
 *
 *  @param [in] b_linux     if true then use linux system() else use _zcfg_system()
 *  @param [in] format      string format of command
 *  @param [in] ...         arguments required by the format
 *
 *  @return ZOS_SUCCESS  - command executed successful
 *          ZOS_FAIL     - fail to execute command
 */
bool zos_system(
    bool        b_linux,
    const char  *format,
    ...
);

/*!
 *  work as snprintf(), avoid the issue from man page
 *      "However, the standards explicitly note that the results are undefined
 *      if source and destination buffers overlap when calling sprintf(),
 *      snprintf(), vsprintf(), and vsnprintf()."
 *
 *  @param [in] buf          string buffer to print to
 *  @param [in[ size         maximum size of the string
 *  @param [in] format       string format of command
 *  @param [in] ...          arguments required by the format
 *
 *  @return >= 0 - successful, the number of characters printed
 *          <  0 - failed
 */
int zos_snprintf(
    char        *buf,
    size_t      size,
    const char  *format,
    ...
);

/*!
 *  get my pid which is shown in top command.
 *
 *  @return uint32_t      my pid
 */
uint32_t zos_pid_get();

/*!
 *  get monotonic time in milli-second.
 *
 *  @return uint32_t    milli-second
 *                      0: failed
 */
uint32_t zos_mtime_get();

/*!
 *  
 * Appends a copy of the source string to the destination string.
 * The terminating null character in destination is overwritten by the first character of source,
 * and a null-character is included at the end of the new string formed by the concatenation of both in destination.
 * Destination and source shall not overlap.
 *  
 *  @param[in] char *destination         Pointer to the destination array, which should contain a C string, and be large enough to contain the concatenated resulting string.
 *  @param[in] const char *source      C string to be appended. This should not overlap destination.
 *  @param[in] bufferSize                   Size of the destination array.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_strcat(char *destination, const char *source, size_t bufferSize);


/*!
 *  Move or copy the source to destination.
 *  Both source and destination should be files.
 *  If the source file is a symbolic link, COPY will copy the file it points to,
 *  and COPY_LINK will copy the symbolic link itself.
 *
 *  @param[in] const char *dst        Destination file name.
 *  @param[in] const char *src        Source file name.
 *  @param[in] file_action action     MOVE, COPY or COPY_LINK.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_copyFile(char *dst, char *src, file_action action);

/*!
 *  Remove the directory.
 *
 *  @param[in] const char *dirPath    Path of the directory.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_removeDir(const char *dirPath);

/*!
 *  Create a directory.
 *
 *  @param[in] const char *dirPath    Path of the directory.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_mkdir(const char *dirPath);

/*!
 *  Create a file.
 *
 *  @param[in] char *fileName         Name of the file.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_touchFile(char *fileName);

/*!
 *  If the type is SYNC, ensure the buffered data are written into the file and disk then do fclose.
 *  If the type is NORMAL, just do fclose.
 *
 *  @param[in] FILE *fp               Pointer to a FILE structure.
 *  @param[in] int type               SYNC_TYPE or NORMAL_TYPE.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_fclose(FILE *fp, int type);

/*!
 *  The same function to strdup, but able to trace memory allocation record
 *  Need use macro ZOS_STRDUP, but not _zos_strdup
 */
char *_zos_strdup(const char *, const char *, int);

/*!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*System Replacement--Start*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*!
 *  Echo account info to /var/passwd, /var/shadow, /var/group...
 *
 *  For the security consideration,
 *  please note that do not print the password info
 *  even if in zlog debug mode
 *
 *  @param[in] char *file        File Name
 *  @param[in] char *strValue    string that will be written in the file
 *  @param[in] char *strLength   string length that will be written in the file
 *  @param[in] char *opentype    open file with mode "w","a","w+","a+"
 *  @param[in] char *funcName    the caller's function name
 *  @param[in] int   line        the caller's line
 *
 *  @return                      ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_echoAccountInfo(char *fileName, char *strValue, int strLength , char* opentype, const char *funcName, int line);

/*!
 *  Echo string to file : echo XXX > FILE
 *
 *  @param[in] char *file        File Name
 *  @param[in] char *strValue    string that will be written in the file
 *  @param[in] char *strLength   string length that will be written in the file
 *  @param[in] char *opentype    open file with mode "w","a","w+","a+"
 *  @param[in] char *funcName    the caller's function name
 *  @param[in] int   line        the caller's line
 *
 *  @return                      ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_echoStrToFile(char *fileName, char *strValue, int strLength , char* opentype, const char *funcName, int line);

/*!
 *  Set interface up
 *  For the command: ifconfig %s up
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigUp(char *interfaceName, const char *funcName, int line);

/*!
 *  Set interface down
 *  For the command: ifconfig %s down
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigDown(char *interfaceName, const char *funcName, int line);

/*!
 *  Set interface flag
 *  For the command: ifconfig %s promisc...
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] short int  flags       the flags in ifreq
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigSetFlags(char *interfaceName, short int flags, const char *funcName, int line);

/*!
 *  Set MTU
 *  For the command: ifconfig %s mtu
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] char *macStr           mac address "xx:xx:xx:xx:xx:xx"
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigSetMtu(char *interfaceName, int mtu, const char *funcName, int line);

/*!
 *  Set Mac Address
 *  For the command: ifconfig %s hw ether %s
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] char *macStr           mac address "xx:xx:xx:xx:xx:xx"
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigSetMac(char *interfaceName, char *macStr, const char *funcName, int line);

/*!
 *  Set IP Address, Netmask, Broadcast 
 *  For the command: ifconfig %s %s netmask %s
 *                   ifconfig %s %s netmask %s broadcast %s
 *                   ifconfig %s:%d %s
 *                   +up/down
 *
 *  @param[in] char *interfaceName    the interface name
 *  @param[in] char *ip               ip address "x.x.x.x"
 *  @param[in] char *netmask          netmask address "x.x.x.x" (optional)
 *  @param[in] char *broadcast        broadcast address "x.x.x.x"(optional)
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_ifconfigSetIPAddr(char *interfaceName, char *ip, char *netmask, char *broadcast, ifconfig_action action, const char *funcName, int line);

/*!!!!!!!!!!!!!!!!!!!!!!!*/
/*System Replacement--End*/
/*!!!!!!!!!!!!!!!!!!!!!!!*/

//==============================================================================
#endif /* _ZOS_API_H */
