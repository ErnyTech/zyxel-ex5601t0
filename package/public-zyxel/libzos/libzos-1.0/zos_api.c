/*!
 *  @file zos_api.c
 *      OS porting layer
 *
 *  @author     CP Wang
 *  @date       2017-12-15 19:26:21
 *  @copyright  Copyright 2017 Zyxel Communications Corp. All Rights Reserved.
 */
//==============================================================================
#include <stdio.h>
#include <stdlib.h>     // system()
#include <stdarg.h>     // va_list()
#include <sys/types.h>
#include <sys/wait.h>   // waitpid()
#include <string.h>     // strchr()
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <utime.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>         // for syscall(), fork(), execvp(), _exit()
#include <sys/syscall.h>    // for syscall()

#include "zos_api.h"
#include "zlog_api.h"

//==============================================================================
#define ZOS_CMD_MAX_LEN     1024

#ifdef ZCFG_MEMORY_LEAK_DETECT
#include <pthread.h>
#include <json/json.h>

struct json_object *ZcfgAllocLineRecordJobj = NULL;
struct json_object *ZcfgAllocAddrRecordJobj = NULL;
pthread_mutex_t recordDataMutex = PTHREAD_MUTEX_INITIALIZER;

// remove this file data in leakage information.
#undef json_object_new_object
#undef json_object_new_string
#undef json_object_new_array
#endif

int default_umask = S_IWGRP | S_IWOTH;

//==============================================================================
/*
    copy from
        build_dir\target-arm_v7-a_uClibc-0.9.32_eabi\zcfg_be-2.0\libzcfg_be\libzcfg_be_common\zcfg_be_common.c
*/
/*
 * zcfg_system():
 * Used to prevent command injection
 * usage likes system(), but does not support sepcial characters except & "
 * & must be seperate and can not be appended to the parameters
 * e.g.
 * Correct case : zcfg_system("ping 8.8.8.8 &") -> excute "ping 8.8.8.8" in background
 * Wrong case : zcfg_system("ping 8.8.8.8&")  -> 8.8.8.8& will be treate as a argement
 *
 * zcfg_system("ping \"8.8.8.8 4.4.4.4\"")  -> 8.8.8.8 4.4.4.4 will be treate as a argement since they are enclosed in double quotation marks
 * zcfg_system("ping 8.8.8.8 > /tmp/result)  -> not support redirect (>)
 *
 * plan to support redirect (>) in future.
 */
static int _zcfg_system(const char *cmd)
{
	pid_t pid, pid2;
	char *argv[128] = {NULL};
	char buf[ZOS_CMD_MAX_LEN+1] = {0};
	int i = 0, j = 0;
	char *p = NULL;
	int status=0;
	int background = 0;
#ifdef ABUU_CUSTOMIZATION
	int preindex = 0, argvindex = 0;
#endif

	strncpy(buf, cmd, ZOS_CMD_MAX_LEN);
	while(i<ZOS_CMD_MAX_LEN && cmd[i]!='\0')
	{	
		if(buf[i] != ' '){
			if(buf[i] == '\"'){
				argv[j++] = &buf[++i];
				p = strchr(&buf[i], '\"');
				if(p != NULL){
					i = p - buf;
					buf[i] = '\0';
				}
				else
					return -1;
				
			}
#ifdef ABUU_CUSTOMIZATION
			else if (buf[i] == '&') {
					// word & ls. word &ls
					argv[j++] = "&";
					buf[--i] = '\0';
					break;
			}
#endif
			else{
				argv[j++] = &buf[i];
#ifdef ABUU_CUSTOMIZATION
				p = strchr(&buf[i], ';');
				if (p != NULL) {
					i = p - buf;
					//printf("buf[i]: %c remove\n",buf[i]);
					buf[i] = '\0';
				}
				p = strchr(&buf[i], '|');
				if (p != NULL) {
					i = p - buf;
					//printf("buf[i]: %c remove\n",buf[i]);
					buf[i] = '\0';
				}
				preindex = i;
				argvindex = j - 1;
#endif
				p = strchr(&buf[i], ' ');
				if(p != NULL){
					i = p - buf;
					buf[i] = '\0';
				}
				else {
#ifdef ABUU_CUSTOMIZATION
					// word&ls
					p = strchr(argv[argvindex], '&');
					if (p != NULL) {
						i = p-argv[argvindex];
						buf[preindex+i] = '\0';
					}
#endif
					break;
				}

#ifdef ABUU_CUSTOMIZATION
				// word& ls
				p = strchr(argv[argvindex], '&');
				if (p != NULL) {
					i = p-argv[argvindex];
					buf[preindex+i] = '\0';
					break;
				}
#endif

			}
		}
		i++;
	}

	/*printf("[www] ");
	for (int tmp = 0; tmp < j; tmp++) {
		printf("%s ",argv[tmp]);
	}
	printf("\n");*/

	if(*argv[j-1] == '&'){
		background = 1;
		argv[j-1] = NULL;
	}

	if((pid = fork()) < 0){
		status = -1;
	}
	else if(pid == 0){
		if(!background){
			execvp(argv[0], argv);
			_exit(127);
		}
		else{
			if((pid2 = fork()) < 0){
				_exit(-1);
			}
			else if(pid2 == 0){
				execvp(argv[0], argv);
				_exit(127);
			}
			else{
				_exit(0);
			}
		}
	}
	else if(pid >0){
		if(waitpid(pid, &status, 0)<0){
			if (errno != EINTR) {
				status = -1;
			}
		}
	}

	return status;
} // _zcfg_system

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
)
{
    char        cmd[ZOS_CMD_MAX_LEN + 1] = {0};
    va_list     ap;
    int         r;

    memset(cmd, 0, sizeof(cmd));

    if (format == NULL)
    {
        ZLOG_ERROR("format == NULL");
        return ZOS_FAIL;
    }

    va_start(ap, format);

    r = vsnprintf(cmd, ZOS_CMD_MAX_LEN, format, ap);

    va_end(ap);

    if (r < 0 || r > ZOS_CMD_MAX_LEN)
    {
        ZLOG_ERROR("fail to vsnprintf()");
        return ZOS_FAIL;
    }

    if (b_linux)
    {
        if (system(cmd) == -1)
        {
            ZLOG_ERROR("fail to system(%s)", cmd);
            return ZOS_FAIL;
        }
    }
    else
    {
        if (_zcfg_system(cmd) == -1)
        {
            ZLOG_ERROR("fail to _zcfg_system(%s)", cmd);
            return ZOS_FAIL;
        }
    }

    return ZOS_SUCCESS;

} // zos_system

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
)
{
    char        *tmp_buf;
    va_list     arg;
    int         ret = -1;

    if (buf == NULL)
    {
        ZLOG_ERROR("buf == NULL");
        return ret;
    }

    if (format == NULL)
    {
        ZLOG_ERROR("format == NULL");
        return ret;
    }

    tmp_buf = ZOS_MALLOC(size);
    if (tmp_buf == NULL)
    {
        ZLOG_ERROR("fail to allocate memory");
        return ret;
    }

    va_start(arg, format);
    ret = vsnprintf(tmp_buf, size, format, arg);
    va_end(arg);

    if (ret >= 0)
    {
        memset(buf, 0, size);

        if (ret > 0)
        {
            strcpy(buf, tmp_buf);
        }
    }

    ZOS_FREE(tmp_buf);

    return ret;

} // zos_snprintf

/*!
 *  get my pid which is shown in top command.
 *
 *  @return uint32_t      my pid
 */
uint32_t zos_pid_get()
{
    return (uint32_t)(syscall(__NR_gettid));
}

/*!
 *  get monotonic time in milli-second.
 *
 *  @return uint32_t    milli-second
 *                      0: failed
 */
uint32_t zos_mtime_get()
{
    struct timespec     tp;

    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
    {
        ZLOG_ERROR("fail to get monotonic time");
        return 0;
    }

    return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}


//==============================================================================
#ifdef ZCFG_MEMORY_LEAK_DETECT
void dumpZcfgAllocRecord()
{
	dumpAllocRecord();
	dumpZcfgAllocFrontendRecord();
}

void dumpAllocRecord()
{
	int totalSize = 0;
	int size = 0;
	
	if(ZcfgAllocAddrRecordJobj != NULL){
#if 0 //hide ZcfgAllocAddrRecordJobj
		printf("============================================================\n");
		printf("libzos Alloc Address Record\n");
		printf("%s\n", json_object_to_json_string(ZcfgAllocAddrRecordJobj));
		printf("============================================================\n\n");
#endif

		json_object_object_foreach(ZcfgAllocAddrRecordJobj, key, val){
			//printf("KEY = %s\n", key);
			size = json_object_get_int(json_object_object_get(json_object_object_get(ZcfgAllocAddrRecordJobj, key), "Size"));
			//printf("LeakSize = %d\n", leakSize);
			totalSize += size;
		}
	}
	else
		printf("libzos ZcfgAllocAddrRecordJobj is not init.\n");

	if(ZcfgAllocLineRecordJobj != NULL){
		printf("============================================================\n");
		printf("libzos Alloc File Line Record\n");
		printf("%s\n", json_object_to_json_string(ZcfgAllocLineRecordJobj));
		printf("============================================================\n\n");

		printf("TotalSize = %d\n\n", totalSize);
	}
	else
		printf("libzos ZcfgAllocLineRecordJobj is not init.\n");
}
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
void recordAllocData(char *pointerAddr, char *fileLine, int size)
{
	int allocCnt = 0;
	struct json_object *fileLineArray = NULL;
	struct json_object *pointerAddrObj = NULL;
	
	if(ZcfgAllocAddrRecordJobj == NULL)
		ZcfgAllocAddrRecordJobj = json_object_new_object();
	if(ZcfgAllocLineRecordJobj == NULL)
		ZcfgAllocLineRecordJobj = json_object_new_object();

	if(ZcfgAllocAddrRecordJobj != NULL){
		pointerAddrObj = json_object_object_get(ZcfgAllocAddrRecordJobj, pointerAddr);
		if(pointerAddrObj == NULL){
			pointerAddrObj = json_object_new_object();
			fileLineArray = json_object_new_array();
			json_object_array_add(fileLineArray, json_object_new_string(fileLine));
			json_object_object_add(pointerAddrObj, "FileLine", fileLineArray);
			json_object_object_add(pointerAddrObj, "Size", json_object_new_int(size));
			json_object_object_add(ZcfgAllocAddrRecordJobj, pointerAddr, pointerAddrObj);
		}
		else{
			fileLineArray = json_object_object_get(pointerAddrObj, "FileLine");
			json_object_array_add(fileLineArray, json_object_new_string(fileLine));
		}
		
	}
	
	if(ZcfgAllocLineRecordJobj != NULL){
		allocCnt = json_object_get_int(json_object_object_get(ZcfgAllocLineRecordJobj, fileLine));
		allocCnt++;
		json_object_object_add(ZcfgAllocLineRecordJobj, fileLine, json_object_new_int(allocCnt));
	}
}
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
void* _ZOS_MALLOC(int _size_, const char *fileName, int line)
{
	char fileLine[ZCFG_ALLOC_RECORD_LEN] = {0};
	char pointerAddr[ZCFG_ALLOC_RECORD_LEN] = {0};

	void *_p_ = NULL;
	_p_ = calloc(1, (size_t)(_size_));
	if (_p_ == NULL)
	{
		ZLOG_ERROR("fail to calloc memory, size = %zu", (size_t)(_size_));
	}
	else
	{
		sprintf(pointerAddr, "%p", _p_);
		sprintf(fileLine, "%s:%d", fileName, line);

		pthread_mutex_lock(&recordDataMutex);
		recordAllocData(pointerAddr, fileLine, _size_);
		pthread_mutex_unlock(&recordDataMutex);
	}

	return _p_;
}
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
void recordZcfgAllocData(void *addr, const char *fileName, int line, int size)
{
	char fileLine[ZCFG_ALLOC_RECORD_LEN] = {0};
	char pointerAddr[ZCFG_ALLOC_RECORD_LEN] = {0};

	if(addr == NULL)
		return;
	
	sprintf(pointerAddr, "%p", addr);
	sprintf(fileLine, "%s:%d", fileName, line);

	pthread_mutex_lock(&recordDataMutex);
	recordAllocData(pointerAddr, fileLine, size);
	pthread_mutex_unlock(&recordDataMutex);
}
#endif

#ifdef ZCFG_MEMORY_LEAK_DETECT
void _ZOS_FREE(void* obj)
{
	char pointerAddr[ZCFG_ALLOC_RECORD_LEN] = {0};
	struct json_object *pointerAddrObj = NULL;
	struct json_object *fileLineArray = NULL;
	const char *fileLine = NULL;
	int allocCnt = 0;
	
	if(obj == NULL){
		ZLOG_DEBUG("Free NULL pointer");
		return;
	}
	pthread_mutex_lock(&recordDataMutex);
	if(ZcfgAllocAddrRecordJobj != NULL){
		sprintf(pointerAddr, "%p", obj);
		pointerAddrObj = json_object_object_get(ZcfgAllocAddrRecordJobj, pointerAddr);
		fileLineArray = json_object_object_get(pointerAddrObj, "FileLine");
		if(fileLineArray != NULL){
			int len = json_object_array_length(fileLineArray);
			for(int i=0;i<len;i++){
				fileLine = json_object_get_string(json_object_array_get_idx(fileLineArray, i));
				if(fileLine != NULL && *fileLine != '\0' && ZcfgAllocLineRecordJobj != NULL){
					allocCnt = json_object_get_int(json_object_object_get(ZcfgAllocLineRecordJobj, fileLine));
					allocCnt --;
					if(allocCnt > 0)
						json_object_object_add(ZcfgAllocLineRecordJobj, fileLine, json_object_new_int(allocCnt));
					else
						json_object_object_del(ZcfgAllocLineRecordJobj, fileLine);
				}
			}
		}
		json_object_object_del(ZcfgAllocAddrRecordJobj, pointerAddr);
	}
	pthread_mutex_unlock(&recordDataMutex);
	
	free(obj);
}
#endif


/*!
 *  The same function to strdup, but able to trace memory allocation record
 *  Need use macro ZOS_STRDUP, but not _zos_strdup
 */
char *_zos_strdup(const char *str, const char *fileName, int line)
{
	int len = 0;
	char *dupStr = NULL;
	
	len = strlen(str) + 1;
	dupStr = (char *)ZOS_MALLOC(len);
	memcpy(dupStr, str, len);
	
#ifdef ZCFG_MEMORY_LEAK_DETECT
	recordZcfgAllocData(dupStr, fileName, line, 0);
#endif
		
	return dupStr;
}

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
int zos_strcat(char *destination, const char *source, size_t bufferSize)
{
	if(destination == NULL || source == NULL || bufferSize == 0)
		return ZOS_FAIL;
	
	while (*destination != '\0') {
		destination++;
		bufferSize--;
		if(bufferSize <= 1)
			return ZOS_FAIL;
	}

	while (bufferSize > 1) {
		*destination = *source;
		if(*source == '\0')
			return ZOS_SUCCESS;
		
		bufferSize--;
		destination++;
		source++;
	}
	*destination = '\0';
	if(*source != '\0')
		return ZOS_FAIL;
	else
		return ZOS_SUCCESS;

}


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
int zos_copyFile(char *dst, char *src, file_action action)
{
	FILE *fp_src = NULL;
	FILE *fp_dst = NULL;
	char *buffer = NULL;
    size_t readNum = 0;
	size_t fileSize = 0;
	struct stat srcState, srcLinkState, dstState;
	int srcIsDir = 0;
	int dstIsDir = 0;

	if( (action != MOVE) && (action != COPY) && (action != COPY_LINK) )
	{
		printf("Action should be MOVE, COPY, or COPY_LINK\n");
		goto fail4;
	}

	if(access(src, F_OK))
	{
		printf("Source does not exist\n");
		goto fail4;
	}
	stat(src, &srcState);
	srcIsDir = S_ISDIR(srcState.st_mode);

	if(access(dst, F_OK))
	{
		dstIsDir = 0;
	}
	else
	{
		stat(dst, &dstState);
		dstIsDir = S_ISDIR(dstState.st_mode);
	}

	if(srcIsDir || dstIsDir)
	{
		printf("Both source and destination should be files\n");
		goto fail4;
	}

	//Link destination to the file that source points to
	lstat(src, &srcLinkState);
	if(S_ISLNK(srcLinkState.st_mode))
	{
		if(action == MOVE || action == COPY_LINK)
		{
			char linkBuf[100] = {'\0'};
			if(readlink(src, linkBuf, sizeof(linkBuf)) == -1)
			{
				printf("Read the contents of the link file error\n");
				goto fail4;
			}

			if(access(dst, F_OK) == 0)
			{
				remove(dst);
			}
			symlink(linkBuf, dst);
			if(action == MOVE)
			{
				remove(src);
			}

			chmod(dst, srcLinkState.st_mode);
			struct timeval times[2];
			times[0].tv_sec = srcLinkState.st_atime;
			times[0].tv_usec = 0;
			times[1].tv_sec = srcLinkState.st_mtime;
			times[1].tv_usec = 0;
			lutimes(dst, times);

			return ZOS_SUCCESS;
		}
	}

	fp_src = fopen(src, "rb");
	if (fp_src == NULL)
	{
		printf("Open source file error\n");
		goto fail4;
	}

	//Get file size of source
	fileSize = srcState.st_size;
	printf("file size of source = %d\n", fileSize);

	//Allocate memory for read and write
	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	if (buffer == NULL)
	{
		printf("Allocate memory error\n");
		goto fail3;
	}

	//Read source file into buffer and copy to destination file
	fp_dst = fopen(dst, "wb");
	if (fp_dst == NULL)
	{
		printf("Open destination file error\n");
		goto fail2;
	}

	int writeSize = BUFFER_SIZE;
	long int remain = fileSize;
	while(remain > 0)
	{
		if(remain < BUFFER_SIZE)
		{
			writeSize = remain;
		}

		readNum = fread(buffer, sizeof(char), writeSize, fp_src);
		if(readNum != writeSize)
		{
			printf("Read source file error\n");
			goto fail1;
		}
		if (fwrite(buffer, sizeof(char), readNum, fp_dst) != readNum)
		{
			printf("Write file error\n");
			goto fail1;
		}
		remain -= readNum/* BUFFER_SIZE */;
	}
	chmod(dst, srcState.st_mode & ~default_umask);

	zos_fclose(fp_src, NORMAL_TYPE);
	if(action == MOVE)
	{
		chmod(dst, srcState.st_mode);
		struct utimbuf dstFileTime;
		dstFileTime.modtime = srcState.st_mtime;
		dstFileTime.actime = srcState.st_atime;
		utime(dst, &dstFileTime);
		remove(src);
	}

	zos_fclose(fp_dst, SYNC_TYPE);
	free(buffer);
	return ZOS_SUCCESS;

fail1:
	zos_fclose(fp_dst, SYNC_TYPE);
fail2:
	free(buffer);
fail3:
	zos_fclose(fp_src, NORMAL_TYPE);
fail4:
	return ZOS_FAIL;
}

/*!
 *  Remove the directory.
 *
 *  @param[in] const char *dirPath    Path of the directory.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_removeDir(const char *dirPath)
{
	DIR *dir = NULL;
	struct dirent *dp = NULL;
	char *buffer = NULL;
	size_t pathLenth = 0;
	struct stat bufferState;
	int getBufferState = 0;
	int ret = 0;

	dir = opendir(dirPath);
	if(dir == NULL)
	{
		printf("Open directory error\n");
		goto fail2;
	}

	while( (dp = readdir(dir)) != NULL )
	{
		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		//Allocate memory for lenth of path
		pathLenth = strlen(dirPath) + strlen(dp->d_name) + 2;
		buffer = (char *)malloc(pathLenth);
		if (buffer == NULL)
		{
			printf("Allocate memory error\n");
			goto fail1;
		}

		snprintf(buffer, pathLenth, "%s/%s", dirPath, dp->d_name);
		getBufferState = lstat(buffer, &bufferState);
		if(!getBufferState)
		{
			if(S_ISDIR(bufferState.st_mode))
			{
				ret = zos_removeDir(buffer);
			}
			else
			{
				ret = remove(buffer);
			}
		}
		free(buffer);
	}

	closedir(dir);

	if (!ret)
	{
		ret = rmdir(dirPath);
	}

	return ret;

fail1:
	closedir(dir);
fail2:
	return ZOS_FAIL;
}

/*!
 *  Create a directory.
 *
 *  @param[in] const char *dirPath    Path of the directory.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_mkdir(const char *dirPath)
{
	char *pathBuffer = NULL;
	int i = 0;
	int len = 0;
	struct stat pathState;

	pathBuffer = (char *)calloc(sizeof(char), strlen(dirPath) + 2);
	if (pathBuffer == NULL)
	{
		printf("Allocate memory error\n");
		goto fail2;
	}

	strncpy(pathBuffer, dirPath, strlen(dirPath) + 1);
	len = strlen(pathBuffer);
	if(pathBuffer[len-1] != '/')
	{
		strcat(pathBuffer, "/");
		len++;
	}

	for(i=1;i<len;i++)
	{
		if(pathBuffer[i] == '/')
		{
			pathBuffer[i] = '\0';

			stat(pathBuffer, &pathState);
			if(access(pathBuffer, F_OK) == -1)
			{
				if(mkdir(pathBuffer, 0777 & ~default_umask) == -1)
				{
					printf("Create the directory error: %s\n", strerror(errno));
					goto fail1;
				}
			}
			else
			{
				if(!S_ISDIR(pathState.st_mode))
				{
					printf("Error: File %s exists\n", pathBuffer);
					goto fail1;
				}
			}
			pathBuffer[i] = '/';
		}
	}

	free(pathBuffer);
	return ZOS_SUCCESS;

fail1:
	free(pathBuffer);
fail2:
	return ZOS_FAIL;
}

/*!
 *  Create a file.
 *
 *  @param[in] char *fileName         Name of the file.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_touchFile(char *fileName)
{
	if(fileName == NULL)
	{
		return ZOS_FAIL;
	}

	//If the file name exists, update its timestamp
	if(access(fileName, F_OK) == 0)
	{
		utime(fileName, NULL);
		return ZOS_SUCCESS;
	}

	else
	{
		FILE *file = fopen(fileName, "wb");
		if(file == NULL)
		{
			printf("Open file error\n");
			return ZOS_FAIL;
		}
		zos_fclose(file, SYNC_TYPE);
	}

	return ZOS_SUCCESS;
}

/*!
 *  If the type is SYNC_TYPE, ensure the buffered data are written into the file and disk then do fclose.
 *  If the type is NORMAL_TYPE, just do fclose.
 *
 *  @param[in] FILE *fp               Pointer to a FILE structure.
 *  @param[in] int type               SYNC_TYPE or NORMAL_TYPE.
 *  @return                           ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_fclose(FILE *fp, int type)
{
	if(fp == NULL)
	{
		printf("FILE pointer is NULL\n");
		return ZOS_FAIL;
	}

	if( (type != SYNC_TYPE) && (type != NORMAL_TYPE) )
	{
		printf("type should be SYNC_TYPE or NORMAL_TYPE\n");
		return ZOS_FAIL;
	}

	if(type == SYNC_TYPE)
	{
		int fd = -1;
		int flag = 0;
		fd = fileno(fp);
		if(fd == -1)
		{
			printf("Get file descriptor error\n");
			return ZOS_FAIL;
		}
		flag = fcntl(fd, F_GETFL);
		if( (flag & O_ACCMODE) != O_RDONLY )
		{
			if( (flag & O_NONBLOCK) == 0 )
			{
				fflush(fp);
			}
			fsync(fd);
		}
	}

	fclose(fp);
	return ZOS_SUCCESS;
}

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
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 
 *  @return                      ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_echoAccountInfo(char *fileName, char *strValue, int strLength , char* opentype, const char *funcName, int line)
{
    FILE *fp = NULL;
	
	ZLOG_DEBUG("Set account info, funcName=%s, line=%d", funcName, line);

    if((fp = ZOS_FOPEN(fileName, opentype)) != NULL)
    {
        if (fwrite(strValue , sizeof(char), strLength , fp) != strLength)
        {
            ZLOG_ERROR("fail to write %s",fileName);
            ZOS_FCLOSE(fp);
            return ZOS_FAIL;
        }
    }
    else
    {
        ZLOG_ERROR("fail to open %s",fileName);
        return ZOS_FAIL;
    }
    ZOS_FCLOSE(fp);
    return ZOS_SUCCESS;
}

/*!
 *  Echo string to file : echo XXX > FILE
 *
 *  @param[in] char *file        File Name
 *  @param[in] char *strValue    string that will be written in the file
 *  @param[in] char *strLength   string length that will be written in the file
 *  @param[in] char *opentype    open file with mode "w","a","w+","a+"
 *  @param[in] char *funcName         the caller's function name
 *  @param[in] int   line             the caller's line
 *
 *  @return                      ZOS_SUCCESS: 0, ZOS_FAIL: -1
 */
int zos_echoStrToFile(char *fileName, char *strValue, int strLength , char* opentype, const char *funcName, int line)
{
    FILE *fp = NULL;

    ZLOG_DEBUG("echo %s to %s, funcName=%s, line=%d", strValue, fileName, funcName, line);
    if((fp = ZOS_FOPEN(fileName, opentype)) != NULL)
    {
        if (fwrite(strValue , sizeof(char), strLength , fp) != strLength)
        {
            ZLOG_ERROR("fail to write %s",fileName);
            ZOS_FCLOSE(fp);
            return ZOS_FAIL;
        }
    }
    else
    {
        ZLOG_ERROR("fail to open %s",fileName);
        return ZOS_FAIL;
    }
    ZOS_FCLOSE(fp);

    return ZOS_SUCCESS;
}

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
int zos_ifconfigUp(char *interfaceName, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    int sockfd;         /* socket fd we use to manipulate stuff with */

    ZLOG_DEBUG("interface %s up, funcName=%s, line=%d",interfaceName, funcName, line);
    memset(&ifr, 0, sizeof(ifr));
    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);

    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    /*Get FLAGS*/
    if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr)) < 0)
    {
       ZLOG_ERROR("SIOCGIFFLAGS ioctl error: %s",strerror(errno));
       close(sockfd);
       return ZOS_FAIL;
    }

    /*Fill parameters*/
    ifr.ifr_flags |= IFF_UP;
    ZLOG_DEBUG("set %s up", interfaceName);

    /*Set FLAGS*/
    if ((ioctl(sockfd, SIOCSIFFLAGS, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFFLAGS ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    close(sockfd);

    return ret;
}

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
int zos_ifconfigDown(char *interfaceName, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    int sockfd;         /* socket fd we use to manipulate stuff with */

    ZLOG_DEBUG("interface %s down, funcName=%s, line=%d",interfaceName, funcName, line);
    memset(&ifr, 0, sizeof(ifr));
    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);

    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    /*Get FLAGS*/
    if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr)) < 0)
    {
       ZLOG_ERROR("SIOCGIFFLAGS ioctl error: %s",strerror(errno));
       close(sockfd);
       return ZOS_FAIL;
    }

    /*Fill parameters*/
    ifr.ifr_flags &= ~IFF_UP;
    ZLOG_DEBUG("set %s down", interfaceName);

    /*Set FLAGS*/
    if ((ioctl(sockfd, SIOCSIFFLAGS, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFFLAGS ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    close(sockfd);

    return ret;
}

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
int zos_ifconfigSetFlags(char *interfaceName, short int flags, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    int sockfd;         /* socket fd we use to manipulate stuff with */

    ZLOG_DEBUG("interface %s set flags, funcName=%s, line=%d",interfaceName, funcName, line);
    memset(&ifr, 0, sizeof(ifr));
    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);

    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    /*Get FLAGS*/
    if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr)) < 0)
    {
       ZLOG_ERROR("SIOCGIFFLAGS ioctl error: %s",strerror(errno));
       close(sockfd);
       return ZOS_FAIL;
    }

    /*Fill parameters*/
    ifr.ifr_flags |= flags;
    ZLOG_DEBUG("set %s flags", interfaceName);

    /*Set FLAGS*/
    if ((ioctl(sockfd, SIOCSIFFLAGS, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFFLAGS ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    close(sockfd);

    return ret;
}

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
int zos_ifconfigSetMtu(char *interfaceName, int mtu, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    int sockfd;         /* socket fd we use to manipulate stuff with */

    ZLOG_DEBUG("interface %s set MTU %d, funcName=%s, line=%d",interfaceName, mtu, funcName, line);
    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    /*Fill parameters*/
    memset(&ifr, 0, sizeof(ifr));
    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);
    ifr.ifr_mtu = mtu;

    /*Set MTU*/
    if ((ioctl(sockfd, SIOCSIFMTU, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFMTU ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    close(sockfd);

    return ret;
}

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
int zos_ifconfigSetMac(char *interfaceName, char *macStr, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    int sockfd;         /* socket fd we use to manipulate stuff with */

    ZLOG_DEBUG("interface %s set MAC %s, funcName=%s, line=%d",interfaceName, macStr, funcName, line);
    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    /*Fill parameters*/
    memset(&ifr, 0, sizeof(ifr));
    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);
    ifr.ifr_addr.sa_family = ARPHRD_ETHER;
    if (macStr)
    {
        sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &ifr.ifr_hwaddr.sa_data[0],
               &ifr.ifr_hwaddr.sa_data[1],
               &ifr.ifr_hwaddr.sa_data[2],
               &ifr.ifr_hwaddr.sa_data[3],
               &ifr.ifr_hwaddr.sa_data[4],
               &ifr.ifr_hwaddr.sa_data[5]);
        ZLOG_DEBUG("set %s mac:%s", interfaceName, macStr);
    }
    else
    {
        ZLOG_ERROR("set %s unknown mac", interfaceName);
        close(sockfd);
        return ZOS_FAIL;
    }

    /*Set HWADDR*/
    if ((ioctl(sockfd, SIOCSIFHWADDR, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFHWADDR ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    close(sockfd);

    return ret;
}

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
int zos_ifconfigSetIPAddr(char *interfaceName, char *ip, char *netmask, char *broadcast, ifconfig_action action, const char *funcName, int line)
{
    int ret = ZOS_SUCCESS;
    struct ifreq ifr;
    struct sockaddr_in sin;
    int sockfd;         /* socket fd we use to manipulate stuff with */
	
    memset(&ifr, 0, sizeof(ifr));
    ZLOG_DEBUG("interface %s Set IP Address %s, netmask=%s, broadcast=%s, action=%d, funcName=%s, line=%d", interfaceName, ip, (netmask!=NULL)? netmask:"",(broadcast!=NULL)? broadcast:"", action, funcName, line);
    /* Create a channel to the NET kernel. */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ZLOG_ERROR("socket error: %s",strerror(errno));
        return ZOS_FAIL;
    }

    ZOS_STRNCPY(ifr.ifr_name, interfaceName, IF_NAMESIZE);
    sin.sin_family = AF_INET;
    /* ip address*/
    inet_aton(ip, &sin.sin_addr);
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    if ((ioctl(sockfd, SIOCSIFADDR, &ifr)) < 0)
    {
        ZLOG_ERROR("SIOCSIFADDR ioctl error: %s",strerror(errno));
        close(sockfd);
        return ZOS_FAIL;
    }
    /* netmask*/
    if (netmask)
    {
        inet_aton(netmask, &sin.sin_addr);
        memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
        if ((ioctl(sockfd, SIOCSIFNETMASK, &ifr)) < 0)
        {
            ZLOG_ERROR("SIOCSIFNETMASK ioctl error: %s",strerror(errno));
            close(sockfd);
            return ZOS_FAIL;
        }
    }
    /* broadcast */
    if (broadcast)
    {
       inet_aton(broadcast, &sin.sin_addr);
       memcpy(&ifr.ifr_broadaddr, &sin, sizeof(struct sockaddr));
       if ((ioctl(sockfd, SIOCSIFBRDADDR, &ifr)) < 0)
       {
           ZLOG_ERROR("SIOCSIFBRDADDR ioctl error: %s",strerror(errno));
           close(sockfd);
           return ZOS_FAIL;
       }
    }
    /*FLAGS for up/down*/
    if (action != IFCONFIG_NONE)
    {
        /*Get FLAGS*/
        if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr)) < 0)
        {
            ZLOG_ERROR("SIOCGIFFLAGS ioctl error: %s",strerror(errno));
            close(sockfd);
            return ZOS_FAIL;
        }

        if (action == IFCONFIG_UP)
        {   ifr.ifr_flags |= IFF_UP;    }
        else if (action == IFCONFIG_DOWN)
        {   ifr.ifr_flags &= ~IFF_UP;   }
        /*Set FLAGS*/
        if ((ioctl(sockfd, SIOCSIFFLAGS, &ifr)) < 0)
        {
          ZLOG_ERROR("SIOCSIFFLAGS ioctl error: %s",strerror(errno));
          close(sockfd);
          return ZOS_FAIL;
        }
    }
    close(sockfd);
    return ret;
}

