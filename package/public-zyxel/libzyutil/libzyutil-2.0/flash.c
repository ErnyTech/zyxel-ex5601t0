#define _XOPEN_SOURCE 500

#include "zcfg_common.h"
#include "zyutil.h"
#include "zlog_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#ifndef __packed
#define __packed __attribute__((packed))
#endif
#include <mtd/ubi-user.h>
#include <errno.h>
#include "zcfg_system.h"
#include "zos_api.h"

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* do nothing */
#endif

#if 0 //#ifdef ZYXEL_WWAN
#define ZY_WWANPACKAGE_MAXSIZE	131072  // (1024*128) bytes
#define WWANPACKAGE_HEADER_LEN	32  // theoretically speaking 31 is match
#define FW_WWANPACKAGE_FILE	"/etc/fw_wwanpackage"
#define RUN_WWANPACKAGE_FILE	"/var/wwan/run_wwanpackage"
#endif

/* size of read/write buffer */
#define BUFSIZE (64 * 1024)
#define mtd_FILE_X       "/tmp/mtd-XXXXXX" // I-Chia 20191112

static int zyUtilOpen(const char *pathname, int flags)
{
	int fd;

	fd = open(pathname, flags);
	if(fd < 0) {
        printf("Try to open %s\n", pathname);
		if(flags & O_RDWR)
			printf("Read/Write access\n");
        else if(flags & O_RDONLY)
			printf("Read access\n");
        else if(flags & O_WRONLY)
			printf("Write access\n");

        printf("%s : %m\n", __FUNCTION__);
		return -1;
	}

	return fd;
}

zcfgRet_t zyUtilMtdDevGetByName(char *mtdname, char *mtddev)
{
	zcfgRet_t ret = ZCFG_NOT_FOUND;
	char mtd[8], size[12], erasesize[12], name[16], tmpname[16];
	int len = 0;
#if 1 //I-Chia 20191112
	char tmp_cat[32] = {0};
	char template[32] = {0};
	int  fd_template;
	FILE * fp_template;

	ZOS_STRNCPY(template, mtd_FILE_X, sizeof(template));

	if ((fd_template = mkstemp(template)) < 0 )
	{
		printf("%s: Can't create MRD file!\n",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	zos_snprintf(tmp_cat,sizeof(tmp_cat),"cat /proc/mtd > %s",template);
#endif
#if 1 //#ifdef ZYXEL_WWAN
	FILE *fp = NULL;

	if(!strcmp(mtdname, WWAN_PARTITION_NAME))
	{
#if 1 //I-Chia 20191112
		fp = popen(tmp_cat, "r");
#else
		fp = popen("cat /proc/mtd > /tmp/mtd", "r");
#endif
		if(fp != NULL)
			pclose(fp);
	}
	else
	{
		ZOS_SYSTEM(tmp_cat);
	}
#endif

	fp_template = fdopen(fd_template, "r");
	zos_snprintf(tmpname, sizeof(tmpname), "\"%s\"", mtdname);

	if ( fscanf(fp_template, "%s%s%s%s", mtd, size, erasesize, name) != 4 )
	{
		printf("%s: Can't read out the data from file!\n",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}

	while(!feof(fp_template))
	{
		if(strstr(mtd, "dev") != NULL)
		{ //skip first line
			if (fscanf(fp_template, "%s%s%s%s", mtd, size, erasesize, name) != 4 )
				printf("%s: Didnot read out the data from file!\n",__FUNCTION__);
			continue;
		}

		if(!strcmp(name, tmpname))
		{
			// Delete ':'
			len = strlen(mtd);
			mtd[len-1] = '\0';
			zos_snprintf(mtddev, (5 + sizeof(mtd)), "/dev/%s", mtd);
			ret = ZCFG_SUCCESS;
			break;
		}
		if (fscanf(fp_template, "%s%s%s%s", mtd, size, erasesize, name) != 4 )
			printf("%s: Didnot read out the data from file!\n",__FUNCTION__);
	}

		ZOS_FCLOSE(fp_template);

#if 1 // I-Chia 20191112
	remove(template);
#else
	remove("/tmp/mtd");
#endif
	return ret;
}

#define ubi_FILE_X       "/tmp/ubi-XXXXXX" // I-Chia 20191112
#define UBIDEV_MAXLEN    32
static zcfgRet_t _zyUtilUbiDevGetByMtdDev(char *mtddev, char *ubidev)
{
	char tmpmtddev[12]  = {0};
	char partitionName[12] = {0};
	char tmpUbiDevName[12] = {0};
	char tmp_command[64] = {0};
	FILE *fp = NULL;

	ZLOG_DEBUG("DEBUG: mtddev=%s\n", mtddev);
	// get partition by mtd device
	// "/dev/mtdxx" --> mtdxx
	ZOS_STRNCPY(tmpmtddev, mtddev+5, strlen(mtddev)-4);
	// assemble command
	zos_snprintf(tmp_command,sizeof(tmp_command),"cat /sys/class/mtd/%s/name",tmpmtddev);
	if((fp = popen(tmp_command, "r")) == NULL) {
		printf("%s: Fail to popen(1)\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if ( fscanf(fp, "%s", partitionName) != 1 )
	{
		printf("%s: Can't read out the data from file!\n",__FUNCTION__);
		pclose(fp);
		return ZCFG_INTERNAL_ERROR;
	}
	pclose(fp);
	ZLOG_DEBUG("DEBUG: mtddev=%s\n", partitionName);

	// get ubi device by partition name
	// assemble command
	memset(tmp_command, 0x0, sizeof(tmp_command));
	zos_snprintf(tmp_command,sizeof(tmp_command),"grep -l %s /sys/class/ubi/*/name | cut -d '/' -f 5",partitionName);
	if((fp = popen(tmp_command, "r")) == NULL) {
		printf("%s: Fail to popen(2)\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if ( fscanf(fp, "%s", tmpUbiDevName) != 1 )
	{
		printf("%s: Can't read out the data from file!\n",__FUNCTION__);
		pclose(fp);
		return ZCFG_INTERNAL_ERROR;
	}
	pclose(fp);

	// ubi0_xxx --> /dev/ubi0_xxx
	zos_snprintf(ubidev, UBIDEV_MAXLEN, "/dev/%s", tmpUbiDevName);
	ubidev[strlen(ubidev)+1] = '\0';
	ZLOG_DEBUG("DEBUG: ubidev=%s\n", ubidev);

	return ZCFG_SUCCESS;
}

//for accessing UBI volumes since ubi volumes are not mounted as mtd partitions in MTK platform
#if defined(MTK_PLATFORM)
//reference MTK/zcmd_wrapper.c
zcfgRet_t zyUtilReadBufFromVolumn(char *devName, char *buf, uint32_t buf_size)
{
    int rBytes = 0;
    char *rBuf = NULL;
    FILE *fp = NULL;

    if((fp = fopen(devName, "r")) == NULL)
    {
        printf("Can't fopen %s\n", devName);
        return ZCFG_INTERNAL_ERROR;
    }

    rBuf = (char *)ZOS_MALLOC(buf_size);

    if(rBuf)
    {
        rBytes = fread(rBuf, 1, buf_size, fp);

        if(ferror(fp))
        {
            printf("Error fread!\n");
            rBytes = 0;
        }
        fclose(fp);
    }
    else
    {
        printf("Error malloc!\n");
        return ZCFG_INTERNAL_ERROR;
    }

    if(rBytes <= 0)
    {
        printf("Error fread bytes!\n");
        return ZCFG_INTERNAL_ERROR;
    }

    memcpy(buf, rBuf, buf_size);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilWriteBufToVolumn(char *devName, char *path)
{
    char sysCmd[128] = {0};
	FILE *fp = NULL;

    zos_snprintf(sysCmd, sizeof(sysCmd), "ubiupdatevol %s %s", devName, path);
    if(system(sysCmd) < 0)
    {
		printf("update volumes %s error.\n", devName);
		return ZCFG_INTERNAL_ERROR;
    }
    sleep(1);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilUbiDevGetByName(char *volumeName, char *devName)
{
	char tmp_command[64] = {0};
	char tmpUbiDevName[12] = {0};
	FILE *fp = NULL;

	memset(tmp_command, 0x0, sizeof(tmp_command));
	zos_snprintf(tmp_command, sizeof(tmp_command), "grep -l %s /sys/class/ubi/*/name | cut -d '/' -f 5", volumeName);
	if((fp = popen(tmp_command, "r")) == NULL)
	{
		printf("%s: Fail to popen(2)\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if ( fscanf(fp, "%s", tmpUbiDevName) != 1 )
	{
		printf("%s: Can't read out the data from file!\n",__FUNCTION__);
		pclose(fp);
		return ZCFG_INTERNAL_ERROR;
	}
	pclose(fp);

	// ubi0_xxx --> /dev/ubi0_xxx
	zos_snprintf(devName, UBIDEV_MAXLEN, "/dev/%s", tmpUbiDevName);
	ZLOG_DEBUG("DEBUG: devName=%s\n", devName);

	return ZCFG_SUCCESS;
}

int32_t zyUtilGetVolumnLength(char *devName)
{
	char sysCmd[128] = {0};
	char ubidev[32] = {0};
	char data_bytes[32] = {0};
	FILE *fp = NULL;

	sscanf(devName, "/dev/%s", ubidev);
	zos_snprintf(sysCmd, sizeof(sysCmd), "cat /sys/class/ubi/%s/data_bytes", ubidev);
    if((fp = popen(sysCmd, "r")) == NULL)
	{
		printf("%s: Fail to popen(2)\n", __FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	}
	if ( fscanf(fp, "%s", data_bytes) != 1 )
	{
		printf("%s: Can't read out the data from file!\n",__FUNCTION__);
		pclose(fp);
		return ZCFG_INTERNAL_ERROR;
	}
	pclose(fp);
	
	return atoi(data_bytes);
}

zcfgRet_t zyUtilEraseVolumn(char *devName)
{
    char sysCmd[128] = {0};

    zos_snprintf(sysCmd, sizeof(sysCmd), "ubiupdatevol %s -t", devName);
    if(system(sysCmd) < 0)
    {
        printf("eraseVolumn error = %s", sysCmd);
        return ZCFG_INTERNAL_ERROR;
    }
    sleep(1);
    return ZCFG_SUCCESS;
}
#endif

//return	0 means fail, return page size when succees
unsigned int zyUtilMtdDevGetSize(char *devName)
{
	int devFd = -1;
	struct mtd_info_user mtd;

	/* Open MTD device */
	if((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1) {
		printf("Cannot open MTD device!\n");
		return 0/*ZCFG_INTERNAL_ERROR*/; //ZCFG_INTERNAL_ERROR is -5, but this function return unsigned int!
	}

	/* get some info about the flash device */
	if(ioctl(devFd, MEMGETINFO, &mtd) < 0) {
        printf("This doesn't seem to be a valid MTD flash device!\n");
		close(devFd);
		return 0/*ZCFG_INTERNAL_ERROR*/; //ZCFG_INTERNAL_ERROR is -5, but this function return unsigned int!
	}
	close(devFd);
	return mtd.size;
}

/*
 * Get NAND flash`s page size, XOR flash`s sector size.
 * @return	0 means fail, return page size when succees
 */
unsigned int zyUtilMtdDevGetPageSize(char *devName)
{
	int devFd = -1;
	struct mtd_info_user mtd;

	/* Open MTD device */
	if((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1) {
		printf("Cannot open MTD device!\n");
		return 0;
	}

	/* get some info about the flash device */
	if(ioctl(devFd, MEMGETINFO, &mtd) < 0) {
        printf("This doesn't seem to be a valid MTD flash device!\n");
		close(devFd);
		return 0;
	}
	close(devFd);
	return mtd.writesize;
}

unsigned int zyUtilMtdDevGetBlockSize(char *devName)
{
	int devFd = -1;
	struct mtd_info_user mtd;

	/* Open MTD device */
	if((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1) {
		printf("Cannot open MTD device!\n");
		return 0;
	}

	/* get some info about the flash device */
	if(ioctl(devFd, MEMGETINFO, &mtd) < 0) {
        printf("This doesn't seem to be a valid MTD flash device!\n");
		close(devFd);
		return 0;
	}
	close(devFd);
	return mtd.erasesize;
}

#if 1
static zcfgRet_t _zyUtilWriteBufToFlash_ubi(char *ubiDevName, void *buf, uint32_t buf_size, int leb_size)
{
	int fd;
	long long bytes = (long long)buf_size;

	fd = open(ubiDevName, O_RDWR);
	if (fd == -1) {
		printf("cannot open UBI volume \"%s\"", ubiDevName);
		return ZCFG_INTERNAL_ERROR;
	}

	// reference libubi.c::ubi_update_start()
	if (ioctl(fd, UBI_IOCVOLUP, &bytes))
	{
		printf("cannot start volume \"%s\" update", ubiDevName);
		close(fd);
		return ZCFG_INTERNAL_ERROR;
	}

	// reference ubiupdatevol.c::update_volume
	while (bytes) {
		ssize_t ret = 0;
		int to_write = 0;

		if( bytes >= leb_size)
			to_write = leb_size;
		else
			to_write = bytes;

		// ubiupdatevol.c::ubi_write()
		while (to_write) {
			ret = write(fd, buf, to_write);
			if (ret < 0) {
				printf("cannot write %d bytes to volume \"%s\"",
						  to_write, ubiDevName);
				close(fd);
				return ZCFG_INTERNAL_ERROR;
			}

			if (ret == 0) {
				printf("cannot write %d bytes to volume \"%s\"", to_write, ubiDevName);
				close(fd);
				return ZCFG_INTERNAL_ERROR;
			}

			to_write -= ret;
			buf += ret;
		}
		bytes -= ret;
	}
	close(fd);

    return ZCFG_SUCCESS;
}

static zcfgRet_t _zyUtilWriteBufToFlash_mtd(char *devName, void *buf, uint32_t buf_size)
{
    int devFd = -1;
    ssize_t result;
    size_t buf_size_to_write, written, bufOfs;
    struct mtd_info_user mtd;
    struct erase_info_user erase;
    void *dest = NULL;
    int pagesize = 0, unit_to_write = 0, unit_to_use = 0, meet_bad = 0, bad_blk_count = 0;
    off_t cur_ofs = 0;

    if ((devName == NULL) || (buf == NULL) || (buf_size <= 0))
    {
        ZLOG_ERROR("Incorrect buffer data!\n");
        return ZCFG_INTERNAL_ERROR;
    }

    /* Open MTD device */
    if ((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1)
    {
        ZLOG_ERROR("Failed to open %s!\n",devName);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Get info of the flash device */
    if (ioctl(devFd, MEMGETINFO, &mtd) < 0)
    {
        ZLOG_ERROR("Failed to get MTD information of %s!\n",devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }

    ZLOG_DEBUG("[%s_%d] devName=%s, buf_size=%d, MTD info: \n"
                "type=0x%08x, flags=0x%08x, size=0x%08x \n"
                "erasesize=0x%08x, writesize=0x%08x, oobsize=0x%08x\n", \
                __FILE__, __LINE__, devName, buf_size,
                mtd.type, mtd.flags, mtd.size, mtd.erasesize, mtd.writesize, mtd.oobsize);

    /* Check if partition size enough */
    if (buf_size > mtd.size)
    {
        ZLOG_ERROR("MTD device %s space is not enough!\n", devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Basic initial */
    dest = malloc(mtd.erasesize);
    unit_to_use = erase.length = mtd.erasesize; //block size, e.g. 0x20000
    buf_size_to_write = (size_t)buf_size;
    bufOfs = 0;

    /* First to erase device anyway */
    for (erase.start = 0; erase.start < mtd.size; erase.start += mtd.erasesize)
    {
        ioctl(devFd, MEMUNLOCK, &erase);
        if (ioctl(devFd, MEMERASE, &erase))
        {
            ZLOG_DEBUG("[%s_%d] Failed to erase %s block at offset 0x%08x.\n", __FILE__, __LINE__, devName, erase.start);
        } //else do nothing
    }

    for (erase.start = 0; ((erase.start < mtd.size) && (buf_size_to_write > 0)); erase.start += mtd.erasesize)
    {
        int ret;
        off_t offset = erase.start;

        ZLOG_DEBUG("[%s_%d] Going to offset=0x%08x, still %d size need to write.\n", __FILE__, __LINE__, erase.start, buf_size_to_write);

        /* Bad block checking */
        ret = ioctl(devFd, MEMGETBADBLOCK, &offset);
        if (ret)
        {
            ZLOG_DEBUG("[%s_%d] Skip this block.\n", __FILE__, __LINE__);
            cur_ofs = lseek(devFd, erase.length, SEEK_CUR);
            if (cur_ofs == (off_t)-1)
            {
                ZLOG_ERROR("[%s_%d]do lseek fail!\n", __FILE__, __LINE__);
            }
            DEBUG_PRINT("[%s_%d] Shift pointer to ofs 0x%08x.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
            bad_blk_count++;
            continue;
        } //else do nothing

        DEBUG_PRINT("[%s_%d] Erase 0x%08x-0x%08x ...\n", __FILE__, __LINE__, (unsigned int)erase.start, (unsigned int)(erase.start + erase.length));

        /* Write data */
        {
            if (buf_size_to_write < erase.length) //data not write yet
            {
                unit_to_use = buf_size_to_write; //unit_to_use is how many we want to write to this block
            } //else do nothing

            /* Unlock a block before write */
            ioctl(devFd, MEMUNLOCK, &erase);

            pagesize = mtd.writesize;
            written = 0;
            meet_bad = 0;
            unit_to_write = unit_to_use;

            // Wite by page size (mtd.writesize)
            while (unit_to_write)
            {
                ZLOG_DEBUG("[%s_%d] unit_to_use=0x%08x, unit_to_write=0x%08x, pagesize=0x%08x, bufOfs=0x%08x, written=0x%08x\n", \
                                __FILE__, __LINE__, unit_to_use, unit_to_write, pagesize, bufOfs, written);

                if (unit_to_write < mtd.writesize)
                {
                    pagesize = unit_to_write;
                } //else do nothing

                result = write(devFd, (buf + bufOfs + written), pagesize);
                if (result != pagesize)
                {
                    // Write fail
                    meet_bad = 1;
                    ZLOG_DEBUG("[%s_%d] Write to flash fail.\n", __FILE__, __LINE__);
                    cur_ofs = lseek(devFd, (offset + erase.length), SEEK_SET); //next block
                    DEBUG_PRINT("[%s_%d] Shift pointer to ofs 0x%08x\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
                    break;
                } //else do nothing
                unit_to_write -= pagesize;
                written += pagesize;
            }
        } //Write data end

        /* Data confirm */
        if (!meet_bad)
        {
            memset(dest, 0xff, sizeof(mtd.erasesize));
            cur_ofs = lseek(devFd, offset, SEEK_SET);
            DEBUG_PRINT("[%s_%d] Shift pointer back to ofs 0x%08x to reading.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);

            // Read data
            result = read(devFd, dest, sizeof(unsigned char)*unit_to_use);
            DEBUG_PRINT("[%s_%d] Read return %d\n", __FILE__, __LINE__, result);

            if (result != unit_to_use)
            {
                meet_bad = 1;
                ZLOG_DEBUG("[%s_%d] Read from flash fail.\n", __FILE__, __LINE__);
                cur_ofs = lseek(devFd, (off_t)(-result), SEEK_CUR);
                DEBUG_PRINT("[%s_%d] Shift pointer back to ofs 0x%08x.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
            }
            else
            {
                // Compare data
                ret = memcmp((buf + bufOfs), dest, sizeof(unsigned char)*unit_to_use);
                if (ret)
                {
                    meet_bad = 1;
                    ZLOG_DEBUG("[%s_%d] Flash data does not match with buffer 0x%08x-0x%08x.\n", \
                                __FILE__, __LINE__, (unsigned int)bufOfs, (unsigned int)(bufOfs + unit_to_use));
                } //else do nothing
            }
        } //Data confirm end

        /* If bad block happen, then jump to next block */
        if (meet_bad == 1)
        {
            ioctl(devFd, MEMERASE, &erase);
            continue; //go to next block without reading & comparing
        } //else do nothing

        bufOfs += unit_to_use;
        buf_size_to_write -= unit_to_use;

    } //end of for(;;)

    ZLOG_DEBUG("[%s_%d] MTD device %s has %d bad block(s).\n", __FILE__, __LINE__, devName, bad_blk_count);

    /* No good block to write */
    if ((erase.start == mtd.size) && (buf_size_to_write != 0))
    {
        for (erase.start = 0; erase.start < mtd.size; erase.start += mtd.erasesize)
        {
            ioctl(devFd, MEMUNLOCK, &erase);
            if (ioctl(devFd, MEMERASE, &erase))
            {
                ZLOG_DEBUG("[%s_%d] Failed to erase block on %s at 0x%08x\n", __FILE__, __LINE__, devName, erase.start);
            } //else do nothing
        }
        ZLOG_ERROR("Erase %s due to incomplete writing to flash.\n", devName);
        free(dest);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    } //else do nothing

    free(dest);
    close(devFd);
    return ZCFG_SUCCESS;
}

/*!
 *  Write data from buffer to flash.
 *
 *  @param[in]  (char *)    Flash (MTD device name)
 *              (void *)    Buffer
 *              uint32_t    Buffer size
 *  @return     zcfgRet_t   ZCFG_INTERNAL_ERROR/ZCFG_SUCCESS
 *
 *  NOTE:
 *  20180622 -  Rewrite this function.
 *              Set one block size as an unit of erase/read/compare action.
 *              And every time write a page size to flash (flash's character).
 *              Btw, a block needs to unlock before erase/write it.
 */
zcfgRet_t zyUtilWriteBufToFlash(char *devName, void *buf, uint32_t buf_size)
{
    int devFd = -1;
    struct mtd_info_user mtd;
    char ubidevname[32];
    zcfgRet_t ret;

    if ((devName == NULL) || (buf == NULL) || (buf_size <= 0))
    {
        ZLOG_ERROR("Incorrect buffer data!\n");
        return ZCFG_INTERNAL_ERROR;
    }

    /* Open MTD device */
    if ((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1)
    {
        ZLOG_ERROR("Failed to open %s!\n",devName);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Get info of the flash device */
    if (ioctl(devFd, MEMGETINFO, &mtd) < 0)
    {
        ZLOG_ERROR("Failed to get MTD information of %s!\n",devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }

    ZLOG_DEBUG("[%s_%d] devName=%s, buf_size=%d, MTD info: \n"
                "type=0x%08x, flags=0x%08x, size=0x%08x \n"
                "erasesize=0x%08x, writesize=0x%08x, oobsize=0x%08x\n", \
                __FILE__, __LINE__, devName, buf_size,
                mtd.type, mtd.flags, mtd.size, mtd.erasesize, mtd.writesize, mtd.oobsize);

    /* Check if partition size enough */
    if (buf_size > mtd.size)
    {
        ZLOG_ERROR("MTD device %s space is not enough!\n", devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }
    close(devFd);

    if( mtd.type == MTD_UBIVOLUME )
    {
        if (_zyUtilUbiDevGetByMtdDev(devName, ubidevname) == ZCFG_SUCCESS )
            ret = _zyUtilWriteBufToFlash_ubi(ubidevname, buf, buf_size, mtd.erasesize);
        else
            return ZCFG_INTERNAL_ERROR;
    }
    else
    {
        ret = _zyUtilWriteBufToFlash_mtd(devName, buf, buf_size);
    }

    return ret;
}
#else
//201806__ZYXEL__, fixed write fail on bad block
/*!
 *  Write data from buffer to flash.
 *
 *  @param[in]  (char *)    Flash (MTD device name)
 *              (void *)    Buffer
 *              uint32_t    Buffer size
 *  @return     zcfgRet_t   ZCFG_INTERNAL_ERROR/ZCFG_SUCCESS
 *
 *  NOTE:
 *  20180622 -  Rewrite this function.
 *              Set one block size as an unit of erase/read/compare action.
 *              And every time write a page size to flash (flash's character).
 *              Btw, a block needs to unlock before erase/write it.
 */
zcfgRet_t zyUtilWriteBufToFlash(char *devName, void *buf, uint32_t buf_size)
{
    int devFd = -1;
    ssize_t result;
    size_t buf_size_to_write, written, bufOfs;
    struct mtd_info_user mtd;
    struct erase_info_user erase;
    void *dest = NULL;
    int pagesize = 0, unit_to_write = 0, unit_to_use = 0, meet_bad = 0, bad_blk_count = 0;
    off_t cur_ofs = 0;

    if ((devName == NULL) || (buf == NULL) || (buf_size <= 0))
    {
        ZLOG_ERROR("Incorrect buffer data!\n");
        return ZCFG_INTERNAL_ERROR;
    }

    /* Open MTD device */
    if ((devFd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1)
    {
        ZLOG_ERROR("Failed to open %s!\n",devName);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Get info of the flash device */
    if (ioctl(devFd, MEMGETINFO, &mtd) < 0)
    {
        ZLOG_ERROR("Failed to get MTD information of %s!\n",devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }

    ZLOG_DEBUG("[%s_%d] devName=%s, buf_size=%d, MTD info: \n"
                "type=0x%08x, flags=0x%08x, size=0x%08x \n"
                "erasesize=0x%08x, writesize=0x%08x, oobsize=0x%08x\n", \
                __FILE__, __LINE__, devName, buf_size,
                mtd.type, mtd.flags, mtd.size, mtd.erasesize, mtd.writesize, mtd.oobsize);

    /* Check if partition size enough */
    if (buf_size > mtd.size)
    {
        ZLOG_ERROR("MTD device %s space is not enough!\n", devName);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    }

    /* Basic initial */
    dest = malloc(mtd.erasesize);
    unit_to_use = erase.length = mtd.erasesize; //block size, e.g. 0x20000
    buf_size_to_write = (size_t)buf_size;
    bufOfs = 0;

    /* First to erase device anyway */
    for (erase.start = 0; erase.start < mtd.size; erase.start += mtd.erasesize)
    {
        ioctl(devFd, MEMUNLOCK, &erase);
        if (ioctl(devFd, MEMERASE, &erase))
        {
            ZLOG_DEBUG("[%s_%d] Failed to erase %s block at offset 0x%08x.\n", __FILE__, __LINE__, devName, erase.start);
        } //else do nothing
    }

    for (erase.start = 0; ((erase.start < mtd.size) && (buf_size_to_write > 0)); erase.start += mtd.erasesize)
    {
        int ret;
        off_t offset = erase.start;

        ZLOG_DEBUG("[%s_%d] Going to offset=0x%08x, still %d size need to write.\n", __FILE__, __LINE__, erase.start, buf_size_to_write);

        /* Bad block checking */
        ret = ioctl(devFd, MEMGETBADBLOCK, &offset);
        if (ret)
        {
            ZLOG_DEBUG("[%s_%d] Skip this block.\n", __FILE__, __LINE__);
            cur_ofs = lseek(devFd, erase.length, SEEK_CUR);
            if (cur_ofs == (off_t)-1)
            {
                ZLOG_ERROR("[%s_%d]do lseek fail!\n", __FILE__, __LINE__);
            }
            DEBUG_PRINT("[%s_%d] Shift pointer to ofs 0x%08x.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
            bad_blk_count++;
            continue;
        } //else do nothing

        DEBUG_PRINT("[%s_%d] Erase 0x%08x-0x%08x ...\n", __FILE__, __LINE__, (unsigned int)erase.start, (unsigned int)(erase.start + erase.length));

        /* Write data */
        {
            if (buf_size_to_write < erase.length) //data not write yet
            {
                unit_to_use = buf_size_to_write; //unit_to_use is how many we want to write to this block
            } //else do nothing

            /* Unlock a block before write */
            ioctl(devFd, MEMUNLOCK, &erase);

            pagesize = mtd.writesize;
            written = 0;
            meet_bad = 0;
            unit_to_write = unit_to_use;

            // Wite by page size (mtd.writesize)
            while (unit_to_write)
            {
                ZLOG_DEBUG("[%s_%d] unit_to_use=0x%08x, unit_to_write=0x%08x, pagesize=0x%08x, bufOfs=0x%08x, written=0x%08x\n", \
                                __FILE__, __LINE__, unit_to_use, unit_to_write, pagesize, bufOfs, written);

                if (unit_to_write < mtd.writesize)
                {
                    pagesize = unit_to_write;
                } //else do nothing

                result = write(devFd, (buf + bufOfs + written), pagesize);
                if (result != pagesize)
                {
                    // Write fail
                    meet_bad = 1;
                    ZLOG_DEBUG("[%s_%d] Write to flash fail.\n", __FILE__, __LINE__);
                    cur_ofs = lseek(devFd, (offset + erase.length), SEEK_SET); //next block
                    DEBUG_PRINT("[%s_%d] Shift pointer to ofs 0x%08x\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
                    break;
                } //else do nothing
                unit_to_write -= pagesize;
                written += pagesize;
            }
        } //Write data end

        /* Data confirm */
        if (!meet_bad)
        {
            memset(dest, 0xff, sizeof(mtd.erasesize));
            cur_ofs = lseek(devFd, offset, SEEK_SET);
            DEBUG_PRINT("[%s_%d] Shift pointer back to ofs 0x%08x to reading.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);

            // Read data
            result = read(devFd, dest, sizeof(unsigned char)*unit_to_use);
            DEBUG_PRINT("[%s_%d] Read return %d\n", __FILE__, __LINE__, result);

            if (result != unit_to_use)
            {
                meet_bad = 1;
                ZLOG_DEBUG("[%s_%d] Read from flash fail.\n", __FILE__, __LINE__);
                cur_ofs = lseek(devFd, (off_t)(-result), SEEK_CUR);
                DEBUG_PRINT("[%s_%d] Shift pointer back to ofs 0x%08x.\n", __FILE__, __LINE__, (unsigned int)cur_ofs);
            }
            else
            {
                // Compare data
                ret = memcmp((buf + bufOfs), dest, sizeof(unsigned char)*unit_to_use);
                if (ret)
                {
                    meet_bad = 1;
                    ZLOG_DEBUG("[%s_%d] Flash data does not match with buffer 0x%08x-0x%08x.\n", \
                                __FILE__, __LINE__, (unsigned int)bufOfs, (unsigned int)(bufOfs + unit_to_use));
                } //else do nothing
            }
        } //Data confirm end

        /* If bad block happen, then jump to next block */
        if (meet_bad == 1)
        {
            ioctl(devFd, MEMERASE, &erase);
            continue; //go to next block without reading & comparing
        } //else do nothing

        bufOfs += unit_to_use;
        buf_size_to_write -= unit_to_use;

    } //end of for(;;)

    ZLOG_DEBUG("[%s_%d] MTD device %s has %d bad block(s).\n", __FILE__, __LINE__, devName, bad_blk_count);

    /* No good block to write */
    if ((erase.start == mtd.size) && (buf_size_to_write != 0))
    {
        for (erase.start = 0; erase.start < mtd.size; erase.start += mtd.erasesize)
        {
            ioctl(devFd, MEMUNLOCK, &erase);
            if (ioctl(devFd, MEMERASE, &erase))
            {
                ZLOG_DEBUG("[%s_%d] Failed to erase block on %s at 0x%08x\n", __FILE__, __LINE__, devName, erase.start);
            } //else do nothing
        }
        ZLOG_ERROR("Erase %s due to incomplete writing to flash.\n", devName);
        free(dest);
        close(devFd);
        return ZCFG_INTERNAL_ERROR;
    } //else do nothing

    free(dest);
    close(devFd);
    return ZCFG_SUCCESS;
}
#endif

#ifndef NEW_MTD_DRIVER 

static struct nand_oobinfo none_oobinfo = {
	.useecc = MTD_NANDECC_OFF,
};

int zyUtilReadBufFromFlash(char *devName, void *buf, uint32_t buf_size)
{
	int oobinfochanged = 0;
	unsigned long start_addr = 0;
	unsigned long ofs, end_addr = 0;
	unsigned long long blockstart = 1;
	int devFd = -1;
	struct mtd_info_user mtd;
	struct nand_oobinfo old_oobinfo;
	int ret, pagesize, badblock = 0;
	unsigned char readbuf[4096];

	/* Open MTD device */
	if((devFd = zyUtilOpen(devName, O_RDONLY)) == -1) {
		return -1;
	}

	/* Fill in MTD device capability structure */
	if(ioctl(devFd, MEMGETINFO, &mtd) != 0) {
		printf("This doesn't seem to be a valid MTD flash device!\n");
		close(devFd);
		return -1;
	}

	printf("mtd.oobsize %d mtd.writesize %d\n", mtd.oobsize, mtd.writesize);

	/* Make sure device page sizes are valid */
	if (!(mtd.oobsize == 216 && mtd.writesize == 4096) &&
		!(mtd.oobsize == 256 && mtd.writesize == 4096) &&
		!(mtd.oobsize == 128 && mtd.writesize == 4096) &&
		!(mtd.oobsize == 128 && mtd.writesize == 2048) &&
		!(mtd.oobsize == 108 && mtd.writesize == 2048) &&
		!(mtd.oobsize == 64 && mtd.writesize == 2048) &&
		!(mtd.oobsize == 32 && mtd.writesize == 1024) &&
		!(mtd.oobsize == 16 && mtd.writesize == 512) &&
		!(mtd.oobsize == 8 && mtd.writesize == 256)) {
		printf("Unknown flash (not normal NAND)\n");
		close(devFd);
		return -1;
	}

#if 0
	/* Read the real oob length */
	oob.length = mtd.oobsize;
#endif
	/* No ecc*/
	ret = ioctl(devFd, MTDFILEMODE, (void *) MTD_MODE_RAW);
	if(ret == 0) {
		oobinfochanged = 2;
	}
	else {
		switch(errno) {
			case ENOTTY:
				if(ioctl(devFd, MEMGETOOBSEL, &old_oobinfo) != 0) {
					perror("MEMGETOOBSEL");
					close(devFd);
					return -1;
				}

				if(ioctl(devFd, MEMSETOOBSEL, &none_oobinfo) != 0) {
					perror("MEMSETOOBSEL");
					close(devFd);
					return -1;
				}

				oobinfochanged = 1;
				break;
			default:
				perror("MTDFILEMODE");
				close(devFd);
				return -1;
		}
	}

	/* Initialize start/end addresses and block size */
	end_addr = start_addr + buf_size;

	pagesize = mtd.writesize;

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr ; ofs+=pagesize) {
		// new eraseblock , check for bad block
		if (blockstart != (ofs & (~mtd.erasesize + 1))) {
			blockstart = ofs & (~mtd.erasesize + 1);
			if ((badblock = ioctl(devFd, MEMGETBADBLOCK, &blockstart)) < 0) {
				perror("ioctl(MEMGETBADBLOCK)");
				goto closeall;
			}
		}

		if (badblock) {
				continue;
		}
		else {
			/* Read page data and exit on failure */
			if (pread(devFd, readbuf, pagesize, ofs) != pagesize) {
				perror("pread");
				goto closeall;
			}
		}

		if(buf_size < pagesize) {
			memcpy(buf, readbuf, buf_size);
		}
		else {
			memcpy(buf, readbuf, pagesize);
			buf += pagesize;
			buf_size -= pagesize;
		}
	}

	/* reset oobinfo */
	if(oobinfochanged == 1) {
		if(ioctl(devFd, MEMSETOOBSEL, &old_oobinfo) != 0) {
			perror ("MEMSETOOBSEL");
			close(devFd);
			return -1;
		}
	}

	/* Close the output file and MTD device */
	close(devFd);

	return 0;

closeall:
	/* The new mode change is per file descriptor ! */
	if(oobinfochanged == 1) {
		if(ioctl(devFd, MEMSETOOBSEL, &old_oobinfo) != 0)  {
			perror("MEMSETOOBSEL");
		}
	}

	close(devFd);
	return -1;
}
#else
int zyUtilReadBufFromFlash(char *devName, void *buf, uint32_t buf_size)
{
	unsigned long start_addr = 0;
	unsigned long ofs, end_addr = 0;
	unsigned long long blockstart = 1;
	int devFd = -1;
	struct mtd_info_user mtd;
	int pagesize, badblock = 0;
	unsigned char readbuf[4096];

	/* Open MTD device */
	if((devFd = zyUtilOpen(devName, O_RDONLY)) == -1) {
		return -1;
	}

	/* Fill in MTD device capability structure */
	if(ioctl(devFd, MEMGETINFO, &mtd) != 0) {
		printf("This doesn't seem to be a valid MTD flash device!\n");
		close(devFd);
		return -1;
	}

	ZLOG_DEBUG("devName=%s: mtd.oobsize %d mtd.writesize %d\n", devName, mtd.oobsize, mtd.writesize);

	if( mtd.type == MTD_UBIVOLUME ) {
		/* Make sure device page sizes are valid */
		printf("Read UBI volume.\n");
		// When the mtd partition is UBI volume, the mtd.oobsize is always 0.
		if (!(mtd.writesize == 4096) &&
			!(mtd.writesize == 2048) &&
			!(mtd.writesize == 1024)) {
			printf("Unknown flash (not normal NAND)\n");
			close(devFd);
			return -1;
		}
	}else {
		/* Make sure device page sizes are valid */
		if (!(mtd.oobsize == 216 && mtd.writesize == 4096) &&
			!(mtd.oobsize == 256 && mtd.writesize == 4096) &&
			!(mtd.oobsize == 128 && mtd.writesize == 4096) &&
			!(mtd.oobsize == 128 && mtd.writesize == 2048) &&
			!(mtd.oobsize == 108 && mtd.writesize == 2048) &&
			!(mtd.oobsize == 64 && mtd.writesize == 2048) &&
			!(mtd.oobsize == 32 && mtd.writesize == 1024) &&
			!(mtd.oobsize == 16 && mtd.writesize == 512) &&
			!(mtd.oobsize == 8 && mtd.writesize == 256)) {
			printf("Unknown flash (not normal NAND)\n");
			close(devFd);
			return -1;
		}
	}

	/* Initialize start/end addresses and block size */
	// partition size
	if(buf_size > mtd.size)
		goto closeall;
	
	end_addr = mtd.size;

	pagesize = mtd.writesize;

	/* Dump the flash contents */
	for (ofs = start_addr; (ofs < end_addr) && (buf_size > 0); ofs+=pagesize) {
		// new eraseblock , check for bad block
		if (blockstart != (ofs & (~mtd.erasesize + 1))) {
			blockstart = ofs & (~mtd.erasesize + 1);
			if ((badblock = ioctl(devFd, MEMGETBADBLOCK, &blockstart)) < 0) {
				perror("ioctl(MEMGETBADBLOCK)");
				goto closeall;
			}
		}

		if (badblock) {
			ofs += mtd.erasesize - pagesize;
			continue;
		}
		else {
			/* Read page data and exit on failure */
			if (pread(devFd, readbuf, pagesize, ofs) != pagesize) {
				perror("pread");
				goto closeall;
			}
		}

		if(buf_size < pagesize) {
			memcpy(buf, readbuf, buf_size);
			buf_size = 0;
		}
		else {
			memcpy(buf, readbuf, pagesize);
			buf += pagesize;
			buf_size -= pagesize;
		}
	}

	/* Close the output file and MTD device */
	close(devFd);

	return 0;

closeall:

	close(devFd);
	return -1;
}
#endif

#if 1 //#ifdef ZYXEL_WWAN
int zyUtilMtdErase(const char *devName)
{
	int fd;
	int ubifd;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
	char ubidevname[32];

	/* Open MTD device */
	if((fd = zyUtilOpen(devName, O_SYNC | O_RDWR)) == -1) {
		fprintf(stderr, "Could not open mtd device: %s\n", devName);
		return ZCFG_INTERNAL_ERROR;
	}
	
	/* Fill in MTD device capability structure */
	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", devName);
		close(fd);
		return ZCFG_INTERNAL_ERROR;
		//exit(1);
	}

	if( mtdInfo.type == MTD_UBIVOLUME ) {
		long long bytes = 0;
		close(fd);

		if (_zyUtilUbiDevGetByMtdDev(devName, ubidevname) == ZCFG_SUCCESS ) {
			// reference ubiupdatevol.c::truncate_volume()
			ubifd = open(ubidevname, O_RDWR);
			if (ubifd == -1) {
				fprintf(stderr, "Could not open ubi device: %s\n", ubidevname);
				return ZCFG_INTERNAL_ERROR;
			}
			// reference libubi.c::ubi_update_start()
			if (ioctl(ubifd, UBI_IOCVOLUP, &bytes)) {
				fprintf(stderr, "Could not wipe out UBI volume: %s\n", ubidevname);
				close(fd);
				return ZCFG_INTERNAL_ERROR;
			}
			close(fd);
		}else
			return ZCFG_INTERNAL_ERROR;
	}
	else {
		mtdEraseInfo.length = mtdInfo.erasesize;

		/* Erase flash */
		for (mtdEraseInfo.start = 0; mtdEraseInfo.start < mtdInfo.size; mtdEraseInfo.start += mtdInfo.erasesize)
		{
			ioctl(fd, MEMUNLOCK, &mtdEraseInfo);
			if (ioctl(fd, MEMERASE, &mtdEraseInfo))
			{
				fprintf(stderr, "Failed to erase %s block at 0x%08x.\n", devName, mtdEraseInfo.start);
			}
		}
		close(fd);
		return 0;
	}

	return 0;

}


/*______________________________________________________________________________
**	zy_updateWWANPackage
**
**	descriptions:
**		update run time WWAN Package.
**
**
**	parameters:
**
**
**	return:
**		1: Fail
**		0: OK
**____________________________________________________________________________
*/
int
zyUtilUpdateWWANPackage(void)
{
	int fd, i, length;
	char *packageBuf = NULL, *bufPoint = NULL, *flashwwan;
	char fwPkgRevision[16], flashPkgRevision[16], cmd_buf[128], mtddev[32];
	zcfgRet_t ret; 
	FILE *fp = NULL;
	//SINT32 filesize = 0;

	/* get FW WWAN Package REVISION */
	fwPkgRevision[0] = '\0';
	fd = open(FW_WWANPACKAGE_FILE, O_RDONLY);
	if(fd < 0){
		printf("Error: %s, open FW_WWANPACKAGE_FILE fail!\n", __func__);
	}
	else{
		packageBuf = calloc(WWANPACKAGE_HEADER_LEN, sizeof(char));	
		if (packageBuf == NULL)
		{
			printf("packageBuf allocation failed.\n");
			return 1;
		}
		if (read(fd, packageBuf, WWANPACKAGE_HEADER_LEN) != WWANPACKAGE_HEADER_LEN)
	    {
	        printf("Error: %s read function fail.\n", __func__);
		 	free(packageBuf);
	        return 1;
	    }

		if(!(bufPoint = strstr(packageBuf, "REVISION:"))){
			printf("Error: %s, FW isn't a WWAN Package!\n", __func__);
		}
		else{
			bufPoint = strstr(bufPoint, ":");
			bufPoint++;
			while(*(bufPoint) == ' '){
				bufPoint++;
			}
			for(i = 0; (bufPoint[i] == '.') || ((bufPoint[i] >= '0') && (bufPoint[i] <= '9')); i++){
				fwPkgRevision[i] = bufPoint[i];
			}
			fwPkgRevision[i] = '\0';
		}
		bufPoint = NULL;
		free(packageBuf);
		close(fd);
	}


	/* get Flash WWAN Package REVISION */
    flashPkgRevision[0] = '\0';
	flashwwan = calloc(ZY_WWANPACKAGE_MAXSIZE, sizeof(char));
	if(flashwwan == NULL){
		printf("Error: %s, flashwwan allocation failed!\n", __func__);
		//free(flashwwan);
		return 1;
	}
#if defined(MTK_PLATFORM)
	if((ret = zyUtilUbiDevGetByName(WWAN_PARTITION_NAME, mtddev)) != ZCFG_SUCCESS)
#else
	if((ret = zyUtilMtdDevGetByName(WWAN_PARTITION_NAME, mtddev)) != ZCFG_SUCCESS)
#endif
	{
		printf("Error: %s, Can't get WWAN MTD!\n", __func__);
	}
	else{
		packageBuf = calloc(WWANPACKAGE_HEADER_LEN, sizeof(char));
		if(packageBuf == NULL){
			printf("packageBuf allocation failed.\n");
			free(flashwwan);
			return 1;
		}
#if defined(MTK_PLATFORM)
		zyUtilReadBufFromVolumn(mtddev, (void *)flashwwan, ZY_WWANPACKAGE_MAXSIZE);
#else
		zyUtilReadBufFromFlash(mtddev, (void *)flashwwan, ZY_WWANPACKAGE_MAXSIZE);
#endif
	    if (flashwwan[0] == '\0')
	    {
			printf("Notice: %s, WWAN MTD is empty!\n", __func__);
	    }
		if(strncpy(packageBuf, flashwwan, WWANPACKAGE_HEADER_LEN) == NULL ){
			printf("%s: Could not get Revision\n", __func__);
			if(flashwwan != NULL){
				free(flashwwan);
				free(packageBuf);
			}
			return 1;
		}
		else {
			if(!(bufPoint = strstr(packageBuf, "REVISION:"))){
				printf("Notice: %s, No WWAN Package in Flash!\n", __func__);
			}
			else{
				bufPoint = strstr(bufPoint, ":");
				bufPoint++;
				while(*(bufPoint) == ' '){
					bufPoint++;
				}
				for(i = 0; (bufPoint[i] == '.') || ((bufPoint[i] >= '0') && (bufPoint[i] <= '9')); i++){
					flashPkgRevision[i] = bufPoint[i];
				}
				flashPkgRevision[i] = '\0';
			}
			bufPoint = NULL;
			free(packageBuf);
			}
	}
	
	/* use newer Revision */
	if((fwPkgRevision[0] == '\0') && (flashPkgRevision[0] == '\0')){
		printf("%s: Could not get Revision\n", __func__);
		free(flashwwan);
		return 1;
	}
	else{
		/*zos_snprintf(cmd_buf, sizeof(cmd_buf), "rm -rf %s", RUN_WWANPACKAGE_FILE);
		ZOS_SYSTEM(cmd_buf);*/
		unlink(RUN_WWANPACKAGE_FILE);
		if (strcmp(flashPkgRevision, fwPkgRevision) > 0){
			//printf("%s: USE Flash WWAN Package\n", __func__); //debug
			bufPoint = strstr(flashwwan, "END");
			length = (bufPoint - flashwwan);
		    fd = open(RUN_WWANPACKAGE_FILE, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
			if(fd < 0){
				printf("Error: %s, open file fail!\n", __func__);
				unlink(RUN_WWANPACKAGE_FILE);
				return 1;
			}
			if(write(fd, flashwwan, length) != length){
				printf("Error: %s write fail!\n", __func__);
				unlink(RUN_WWANPACKAGE_FILE);
				return 1;
			}
			close(fd);
			bufPoint = NULL;
			//free(flashwwan);
		}
		else{
			//printf("%s: USE FW WWAN Package\n", __func__); //debug
			//zos_snprintf(cmd_buf, sizeof(cmd_buf), "cp %s %s", FW_WWANPACKAGE_FILE, RUN_WWANPACKAGE_FILE);
			//ZOS_SYSTEM(cmd_buf);
			//fp = popen(cmd_buf, "r");
			//if(fp == NULL){
			//	printf("copy fw_wwanpackage fail\n");
			//}
			//else
			//	pclose(fp);
			//zyxel "cp" replacement
			if (ZOS_COPY_FILE(RUN_WWANPACKAGE_FILE, FW_WWANPACKAGE_FILE) != ZOS_SUCCESS)
			{   printf("copy fw_wwanpackage fail\n");    }
		}
		if(flashwwan != NULL)
			free(flashwwan);
	}
	return 0;
}

#endif
