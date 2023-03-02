#ifdef MTK_PLATFORM

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "libzyutil_wrapper.h"
#include "libzyutil_mtk_wrapper.h"
#include "zcfg_msg.h"
#ifdef ZYXEL_MULTI_PROVINCE_VENDOR
#include "zyxel_province.h"
#endif

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
int zyUtilGetCoFwProductName(char *prname, int prnameLen);
#endif

static zcfgRet_t zyUtilReqSendWithResponse(uint32_t msgType, zcfgEid_t srcEid, zcfgEid_t dstEid, int payloadLen, const char *payload, char **returnStr)
    {
        zcfgRet_t ret;
        void *sendBuf = NULL;
        void *recvBuf = NULL;
        zcfgMsg_t *sendMsgHdr = NULL;
        char *recv_str = NULL;
        int buf_size = 0;
    
        //printf("atsh payload length %d\n", payloadLen);
    
        buf_size = sizeof(zcfgMsg_t)+payloadLen+1;
        sendBuf = (char *)ZOS_MALLOC(buf_size);
        sendMsgHdr = (zcfgMsg_t*)sendBuf;
    
        sendMsgHdr->type = msgType;
        sendMsgHdr->length = payloadLen;
        sendMsgHdr->srcEid = srcEid; //eid defined at zcfg_eid.h
        sendMsgHdr->dstEid = dstEid;
    
        if(payload != NULL)
            memcpy(sendBuf+sizeof(zcfgMsg_t), payload, payloadLen);
    
        ret = zcfgMsgSendAndGetReply(sendMsgHdr, (zcfgMsg_t **)&recvBuf, 0);
    
        if(ret == ZCFG_SUCCESS) {
            recv_str = (char *)recvBuf+sizeof(zcfgMsg_t);
            //printf("Receive message : %s\n", recv_str);
    
            *returnStr = recv_str;
            /*switch(msgType){
                case ZCFG_MSG_FAKEROOT_SYS_ATSH:
                    //printf("reply msgType = %d\n", msgType);
                    break;
            }*/
        }
    
        //free(sendBuf);
        return ret;
    }

char *getUbootEnv(char *name, int maxLength)
{
	uboot_env_t ubootEnv;
	uboot_env_t *ubootEnv_p = &ubootEnv;

	memset(ubootEnv_p, 0, sizeof(uboot_env_t));
	strncpy(ubootEnv_p->envName, name, maxLength+1);
	zyUtilReqSendWithResponse(ZCFG_MSG_GET_UBOOT_ENV, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(uboot_env_t), ubootEnv_p, (char**)&ubootEnv_p);

	if (ubootEnv_p->envContent == NULL)
	{
		ZLOG_ERROR("getUbootEnv error");
		return NULL;
	}

	return ubootEnv_p->envContent;
}

int setUbootEnv(char *name, char *value, int maxLength)
{
	uboot_env_t ubootEnv;
	uboot_env_t *ubootEnv_p = &ubootEnv;

	memset(ubootEnv_p, 0, sizeof(uboot_env_t));
	strncpy(ubootEnv_p->envName, name, maxLength+1);
	strncpy(ubootEnv_p->envContent, value, 128);
	zyUtilReqSendWithResponse(ZCFG_MSG_SET_UBOOT_ENV, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(uboot_env_t), ubootEnv_p, (char**)&ubootEnv_p);

	return ZCFG_SUCCESS;
}

char *getATSHInfo(void)
{
    char *atsh_info = "";
    zcfgRet_t ret;

    ret = zyUtilReqSendWithResponse(ZCFG_MSG_GET_ATSH_INFO, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(ZY_ATSH_INFO), atsh_info, (char**)&atsh_info);

    if (ret != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("getATSHInfo error");
    }

    return atsh_info;
}

unsigned long _simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((*cp == 'x') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	}
	while (isxdigit(*cp) &&
	       (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

int ahex2int(char a, char b)
{
    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}

#ifdef ZYXEL_L_ENDIAN
#define ZY_HOST_TO_TARGET16(x)  bswap_16(x)
#else
#define ZY_HOST_TO_TARGET16(x) (x)
#endif

uint16_t zyxelChecksum(uint8_t *start, uint32_t len)
{
    uint8_t *ptr;
    uint32_t i;
    uint32_t checksum = 0;
    uint16_t result;

    ptr = start;

    for ( i=0 ; i<len ; i++)
    {
        checksum += *ptr;
        ptr++;
    }

    result = (checksum & 0xffff) + (checksum >>16 );
    return result;
}

static int _getzyfwinfo(char *data, int size, int zyfwinfo_idx)
{
    char tmp_fname[128] = {'\0'};
    FILE *zyfwinfo_fh;
    int num_bytes = 0;

    sprintf(tmp_fname, "/dev/ubi0_%d", zyfwinfo_idx);
    zyfwinfo_fh = fopen(tmp_fname, "r");

    if( zyfwinfo_fh )
    {
        num_bytes = fread(data, 1, size, zyfwinfo_fh);

        if( ferror(zyfwinfo_fh) )
        {
            printf("%s: Error fread zyfwinfo%d!\n", __FUNCTION__, zyfwinfo_idx);
            num_bytes = 0;
        }

        fclose(zyfwinfo_fh);
    }
    else
    {
        printf("%s: Error fopen zyfwinfo%d!\n", __FUNCTION__, zyfwinfo_idx);
    }

    return num_bytes;
}

int zyFwInfoGet(PZY_FW_INFO pzyFwInfo)
{
    uint16_t zyCheckSum_old = 0;
    uint16_t zyCheckSum_new = 0;
    int num_bytes_read = 0;
    FILE *fp = NULL;
    char buf[32] = {'\0'};
    int index = 0;
    int status;
#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
    char cofw_prname[ZYFWINFI_PRODUCTNAME_LEN] = {0};
    int cofw_prnameLen = ZYFWINFI_PRODUCTNAME_LEN;
#endif

    status = system("sh /lib/zyxel/get_zyfwinfo_index.sh boot");
    if ( WIFEXITED(status) )
    {
        index = WEXITSTATUS(status);
    }
    else
    {
        printf("Failed to get zyfwinfo index\n");
        return ZCFG_INTERNAL_ERROR;
    }

    num_bytes_read = _getzyfwinfo((char *)pzyFwInfo, ZY_FW_INFO_SIZE, index);

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
    if (zyUtilGetCoFwProductName(cofw_prname, cofw_prnameLen) > 0)
    {
	if(strcmp(pzyFwInfo->productName, cofw_prname))
	{
	    //cut the last line feed char
	    strncpy(pzyFwInfo->productName, cofw_prname, strlen(cofw_prname)-1);
	    //printf("%s,%d, %s\n", __FUNCTION__, __LINE__, pzyFwInfo->productName);
	}
    }
#endif

    // check CRC
    zyCheckSum_old = pzyFwInfo->zy_checksum;
    pzyFwInfo->zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)pzyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);

    if ( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old) )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int mtd_open(const char *mtd, int flags)
{
	FILE *fp = NULL;
	char dev[256] = {0};
	int i;

	if ((fp = ZOS_FOPEN("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, mtd)) {
				snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
				ZOS_FCLOSE(fp);
				return open(dev, flags);
			}
		}
		ZOS_FCLOSE(fp);
	}

	return open(mtd, flags);
}

int getzyfwinfoNonboot(char *data, int size, int zyfwinfo_idx)
{
    char tmp_fname[128] = {'\0'};
    char ubi_dev[5] = {'\0'};
    FILE *zyfwinfo_fh;
    FILE *ubi_nonboot;
    int num_bytes = 0;

    ubi_nonboot = fopen("/tmp/ubi_unmount", "r");
    if( ubi_nonboot )
    {
        if(fgets(ubi_dev, sizeof(ubi_dev), ubi_nonboot) != NULL)
        {
            sprintf(tmp_fname, "/dev/%s_%d", ubi_dev, zyfwinfo_idx);
        }

        if( ferror(ubi_nonboot) )
        {
            printf("%s: Error fread ubi_nonboot!\n", __FUNCTION__);
            num_bytes = 0;
        }

        fclose(ubi_nonboot);
    }
    else
    {
        printf("%s: Error fopen ubi_nonboot!\n", __FUNCTION__);
        return ZCFG_INTERNAL_ERROR;
    }

    zyfwinfo_fh = fopen(tmp_fname, "r");
    if( zyfwinfo_fh )
    {
        num_bytes = fread(data, 1, size, zyfwinfo_fh);

        if( ferror(zyfwinfo_fh) )
        {
            printf("%s: Error fread zyfwinfo%d!\n", __FUNCTION__, zyfwinfo_idx);
            num_bytes = 0;
        }

        fclose(zyfwinfo_fh);
    }
    else
    {
        printf("%s: Error fopen zyfwinfo%d!\n", __FUNCTION__, zyfwinfo_idx);
    }

    return num_bytes;
}

int _setzyfwinfoNonboot(char *data, int size, int zyfwinfo_idx)
{
    char tmp_fname[128] = {'\0'};
    char ubi_dev[5] = {'\0'};
    char sysCmd[128] = {'\0'};
    int fwrite_byte = 0;
    FILE *ubi_nonboot = NULL;
    FILE *zyfwinfo_nonboot = NULL;

    ubi_nonboot = fopen("/tmp/ubi_unmount", "r");
    if( ubi_nonboot )
    {
        if(fgets(ubi_dev, sizeof(ubi_dev), ubi_nonboot) != NULL)
        {
            sprintf(tmp_fname, "/dev/%s_%d", ubi_dev, zyfwinfo_idx);
        }

        if( ferror(ubi_nonboot) )
        {
            printf("%s: Error fread ubi_nonboot!\n", __FUNCTION__);
            fclose(ubi_nonboot);
            return ZCFG_INTERNAL_ERROR;
        }

        fclose(ubi_nonboot);
    }
    else
    {
        printf("%s: Error fopen ubi_nonboot!\n", __FUNCTION__);
        return ZCFG_INTERNAL_ERROR;
    }

    zyfwinfo_nonboot = fopen("/tmp/zyfwinfo_nonboot", "wb");
    if( zyfwinfo_nonboot )
    {
        fwrite_byte = fwrite(data, 1, size, zyfwinfo_nonboot);
        if(fwrite_byte < size)
        {
            printf("%s: Error fwrite zyfwinfo_nonboot!\n", __FUNCTION__);
            fclose(zyfwinfo_nonboot);
            return ZCFG_INTERNAL_ERROR;
        }

        fclose(zyfwinfo_nonboot);
    }
    else
    {
        printf("%s: Error fopen zyfwinfo_nonboot!\n", __FUNCTION__);
        return ZCFG_INTERNAL_ERROR;
    }

    zos_snprintf(sysCmd, sizeof(sysCmd), "ubiupdatevol %s /tmp/zyfwinfo_nonboot", tmp_fname);
    system(sysCmd);

    return ZCFG_SUCCESS;
}

int zyFwInfoNonBootGet(PZY_FW_INFO pzyFwInfo, int *index)
{
    uint16_t zyCheckSum_old = 0;
    uint16_t zyCheckSum_new = 0;
    int num_bytes_read = 0;
    FILE *fp = NULL;
    char buf[32] = {'\0'};
    int status;

    status = system("sh /lib/zyxel/get_zyfwinfo_index.sh nonboot attach");
    if ( WIFEXITED(status) )
    {
        *index = WEXITSTATUS(status);
    }
    else
    {
        printf("Failed to get zyfwinfo index\n");
        return ZCFG_INTERNAL_ERROR;
    }

    num_bytes_read = getzyfwinfoNonboot((char *)pzyFwInfo, ZY_FW_INFO_SIZE, *index);

    // check CRC
    zyCheckSum_old = pzyFwInfo->zy_checksum;
    pzyFwInfo->zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)pzyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);

    if ( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old) )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int zyFwInfoNonBootSet(PZY_FW_INFO pzyFwInfo, int index)
{
    FILE *fp = NULL;
    char buf[32] = {'\0'};

    if ( _setzyfwinfoNonboot((char *)pzyFwInfo, ZY_FW_INFO_SIZE, index) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    return ZCFG_SUCCESS;
}

/*-------------------------------------------------
 * eraseRomdPartition() :
 *-------------------------------------------------
int eraseRomdPartition(void)
{
	int ret = 0;
	char mtddev[16];

	if( zyUtilMtdDevGetByName(ROMD_PARTITON_NAME, mtddev) == ZCFG_SUCCESS){
		if (zyUtilMtdErase(mtddev) != ZCFG_SUCCESS)
			return ZCFG_INTERNAL_ERROR;
	}
	else
		return ZCFG_INTERNAL_ERROR;
	return ZCFG_SUCCESS;
}*/

#if 0
zcfgRet_t zyUtilGetMrdInfo(PMRD_DATA mrd)
{
	return ZCFG_SUCCESS;
}
#endif

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
int zyUtilGetCoFwProductName(char *prname, int prnameLen)
{
    FILE *pf = fopen(MTK_COFW_PRODUCT_NAME_PATH, "r");
    int n = -1;

    if (!pf || !prname)
    {
	return -1;
    }

    n = fread(prname, 1, prnameLen-1, pf);

    fclose(pf);

    return n;
}
#endif

#if 0
zcfgRet_t zyUtilSetMrdInfo(PMRD_DATA mrd)
{
	return ZCFG_SUCCESS;
}
#endif

#ifdef SUPPORT_LANVLAN
zcfgRet_t zyUtilISetIfState(char *ifName, bool up)
{
	/*Todo*/
	return ZCFG_SUCCESS;
}
#endif

void filter_string(char *tempstr,char *remove, int size)
{
    int count = 0;
    int len = strlen(tempstr);
    char *str2=(char *)malloc(sizeof(char)*len+1);
    char *str1 = tempstr;
    char *strx = str2;

    while(*str1 != '\0')
    {
        if (*str1 == *remove)
        {
            count++;
        }else{
            *str2 = *str1;
            str2++;
        }
        str1++;
    }
    *str2 = '\0';
    strncpy(tempstr, strx, size+1);
    free(strx);
}

zcfgRet_t zyUtilIGetBaseMAC(char *mac)
{
	char *str = NULL;
	int i;

	str = getUbootEnv(ENV_MAC_ADDRESS, strlen(ENV_MAC_ADDRESS));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	filter_string(str, ":", (MAC_ADDRESS_LEN - 1)); //xx:xx:xx:xx:xx:xx => xxxxxxxxxxxx
	char *buf = str;
	while(*buf)
	{
		*buf = toupper((unsigned char)*buf);
		buf++;
	}

	memset(mac,0,sizeof(mac));
	ZOS_STRNCPY(mac, str, MAC_ADDRESS_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBaseMACwithColon(char *mac)
{
	char *str = NULL;

	str = getUbootEnv(ENV_MAC_ADDRESS, strlen(ENV_MAC_ADDRESS));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(mac, str, ZCFG_MAC_ADDRESS_LEN+5);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetOnuType(int *num)
{
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
	char *str = NULL;

	str = getUbootEnv(ENV_MAC_ADDRESS_QUANTITY, strlen(ENV_MAC_ADDRESS_QUANTITY));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	sscanf(str, "%d", num);

	return ZCFG_SUCCESS;
}

/*!
 *  Get Nth MAC base on base MAC and GDM MASK.
 *  Let MAC`s increment can in the range og GDM MASK.
 *  For example: when GDM MASK is 0xf8, MAC can be x2,x3,x4,x5,x6,x7,x0,x1
 *
 *  @param[inout]  pucaMacAddr   input Base MAC and out N`th MAC, should be 6 bytes.
 *  @param[in]     n             Get N`th MAC.
 *  @param[in]     _macQuantity  Mac quantity.
 *
 *  @note  The MAC length should be 6 bytes.
 */
static void getNthMacAddr_special( unsigned char *pucaMacAddr, unsigned long n, unsigned char _gdm_mask)
{
}

/*!
 *  Get Nth MAC.
 *
 *  @param[out]    pucaMacAddr   out N`th MAC, should be 6 bytes.
 *  @param[in]     n             Get N`th MAC.
 *
 *  @note  The MAC length should be 6 bytes.
 */
zcfgRet_t zyUtilIGetNthMAC(char *mac, int nth)
{
    int i = 0;
    unsigned long cal = 0;
    char mac_tmp[ZCFG_MAC_ADDRESS_LEN] = {0};
    int  val[6] = {0};
    zyUtilIGetBaseMAC(mac_tmp);

    for( i = 0; i < 6; ++i)
    {
        val[i] = ahex2int(*(mac_tmp+(i*2)),*(mac_tmp+((i*2)+1)));
    }

    cal = (val[3]  << 16) | (val[4]  << 8) | val[5] ;
    cal = (cal + nth) & 0xffffff;
    val[3] = (cal >> 16) & 0xff;
    val[4] = (cal >> 8) & 0xff;
    val[5] = (cal ) & 0xff;

    for (i=0; i<6 ; i++)
    {
        mac[i] = val[i];
    }
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNthMACwithColon(char *mac, int nth)
{
    int i = 0;
    unsigned long cal = 0;
    char mac_tmp[ZCFG_MAC_ADDRESS_LEN] = {0};
    int  val[6] = {0};
    zyUtilIGetBaseMAC(mac_tmp);

    for( i = 0; i < 6; ++i)
    {
        val[i] = ahex2int(*(mac_tmp+(i*2)),*(mac_tmp+((i*2)+1)));
    }

    cal = (val[3]  << 16) | (val[4]  << 8) | val[5] ;
    cal = (cal + nth) & 0xffffff;
    val[3] = (cal >> 16) & 0xff;
    val[4] = (cal >> 8) & 0xff;
    val[5] = (cal ) & 0xff;

    zos_snprintf(mac , ZCFG_MAC_ADDRESS_LEN+5, "%02X:%02X:%02X:%02X:%02X:%02X",
        val[0],
        val[1],
        val[2],
        val[3],
        val[4],
        val[5]
    );

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilSetSoftwareLedState(ZYLED_INDEX index ,ZYLED_ACTION action, ZYLED_RATE blink_rate)
{
    char led_file_name[128] = {0}, sysCmd[128] = {0};


    switch(index) {
        case ZYLED_POWER_GREEN:
            ZOS_STRNCPY(led_file_name, "zyled-green-pwr", sizeof(led_file_name));
            break;
        case ZYLED_POWER_RED:
            ZOS_STRNCPY(led_file_name, "zyled-red-pwr", sizeof(led_file_name));
            break;
#ifdef ZYXEL_OPAL_EXTENDER
        case ZYLED_LINKQUALITY_GREEN:
            ZOS_STRNCPY(led_file_name, "zyled-green-signal", sizeof(led_file_name));;
            break;
        case ZYLED_LINKQUALITY_RED:
            ZOS_STRNCPY(led_file_name, "zyled-red-signal", sizeof(led_file_name));
            break;
        case ZYLED_LINKQUALITY_AMBER:
            return ZCFG_INTERNAL_ERROR;
#endif
        default:
            break;
    }
    zos_snprintf(sysCmd, sizeof(sysCmd), "echo %d > /sys/class/leds/%s/brightness", action, led_file_name);
    ZOS_SYSTEM(sysCmd);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFeatureBits(char *featurebits)
{
    char *str = NULL;

    str = getUbootEnv(ENV_FEATUREBITS, strlen(ENV_FEATUREBITS));
    if(str == NULL)
        return ZCFG_INTERNAL_ERROR;
    ZOS_STRNCPY(featurebits, str, ZCFG_FEATUREBITS_HEX_LEN);

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetModelID(char *model_id)
{
    char *str = NULL;

    str = getUbootEnv(ENV_FEATUREBITS, strlen(ENV_FEATUREBITS));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

    model_id[0] = str[1];
    model_id[1] = str[3];
    model_id[2] = str[5];
    model_id[3] = str[7];

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
	ZY_ATSH_INFO buf = {0};

	memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
	ZOS_STRNCPY(pdname, buf.zyMRDInfoData.productName, ZCFG_PRODUCTNAME_LEN);

	return ZCFG_SUCCESS;
}
zcfgRet_t zyUtilIGetDSLVersion(char *DSLversion)
{
    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetWiFiDriverVersion(char *wifidriverversion)
{
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBootVersion(char *bootversion)
{
	ZY_ATSH_INFO buf = {0};

	memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
	zos_snprintf(bootversion, sizeof(bootversion), "V%s", buf.build_version);

	return ZCFG_SUCCESS;
}
	
zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
	ZY_ATSH_INFO buf = {0};

	memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
	ZOS_STRNCPY(fwversion, buf.zyFwInfoData.firmwareVersion, ZYFWINFO_FIRMWAREVERSION_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetExternalFirmwareVersion(char *fwversion)
{
	ZY_ATSH_INFO buf = {0};

	memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
	ZOS_STRNCPY(fwversion, buf.zyFwInfoData.firmwareVersionEx, ZYFWINFO_FIRMWAREVERSION_LEN);

	return ZCFG_SUCCESS;
}

/*
 * The input string must have the length, "SUB_SERIAL_NUM_STR_LEN+1".
 */
zcfgRet_t zyUtilIGetSubSerialNumber(char *sub_serialnumber)
{
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	char *str = NULL;
	int i;
    int tmp = 0;
    char buf[3] = {0};
    unsigned char featureBits[MTK_FEATUREBITS_LEN];

	str = getUbootEnv(ENV_FEATUREBITS, strlen(ENV_FEATUREBITS));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	for(i=0;i<MTK_FEATUREBITS_LEN;i++)
	{
		strncpy(buf, str + 2*i, 2);
		buf[2] = '\0';
		tmp = _simple_strtoul(buf, NULL, 16) & 0xff;
		featureBits[i] = (unsigned char)tmp;
	}

	if(zyUtilIsAlphanumericString(featureBits+FEATUREBITS_SUB_SERIAL_NUM_OFFSET, FEATUREBITS_SUB_SERIAL_NUM_LEN) != 0)
		return ZCFG_INTERNAL_ERROR;
	else
		strncpy(sub_serialnumber, featureBits+FEATUREBITS_SUB_SERIAL_NUM_OFFSET, FEATUREBITS_SUB_SERIAL_NUM_LEN);

#endif
	return ZCFG_SUCCESS;
}

/*
 * The input string must have the length, "SERIAL_NUM_STR_LEN+1".
 */
zcfgRet_t zyUtilIGetSerialNumber(char *serialnumber)
{
	ZY_ATSH_INFO buf = {0};
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	char sub_serialnumber[2] = {'\0'};
#endif

	memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
	ZOS_STRNCPY(serialnumber, buf.zyMRDInfoData.serialNumber, ZCFG_SERIALNUMBER_LEN);

#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	if(zyUtilIGetSubSerialNumber(sub_serialnumber) == ZCFG_SUCCESS)
		strncat(serialnumber, sub_serialnumber, FEATUREBITS_SUB_SERIAL_NUM_LEN);
	else
		return ZCFG_INTERNAL_ERROR;
#endif
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetGponSerialNumber(char *gpon_serialnumber)
{
	char *str = NULL;

	str = getUbootEnv(ENV_GPON_SERIAL_NUMBER, strlen(ENV_GPON_SERIAL_NUMBER));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(gpon_serialnumber, str, ZCFG_SERIALNUMBER_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetCountryCode(char *country_code)
{
	char *str = NULL;

	str = getUbootEnv(ENV_COUNTRY_CODE, strlen(ENV_COUNTRY_CODE));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(country_code, str, ZCFG_COUNTRYCODE_HEX_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFeatureBit(char *featurebit)
{
	char *str = NULL;

	str = getUbootEnv(ENV_MAINFEATUREBIT, strlen(ENV_MAINFEATUREBIT));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(featurebit, str, ZCFG_FEATUREBIT_LEN);

	return ZCFG_SUCCESS;
}

#if defined(BUILD_ZYXEL_VOICE_MTK_D2_VE_SUPPORT)
zcfgRet_t zyUtilIGetD2Key(char *d2_key)
{
	char *str = NULL;

	str = getUbootEnv(ENV_D2KEY, strlen(ENV_D2KEY));
	if(str == NULL)
		return ZCFG_INTERNAL_ERROR;

	ZOS_STRNCPY(d2_key, str, D2KEY_LENGTH+1);

	return ZCFG_SUCCESS;
}
#endif

#ifdef ZYXEL_AT_SET_TUTK_UID
int zyUtilIGetTUTKUID(char *buf, int len)
{
    char *str = NULL;

    str = getUbootEnv(ENV_TUTK_UID, strlen(ENV_TUTK_UID));
    if(str == NULL)
        return ZCFG_INTERNAL_ERROR;

    ZOS_STRNCPY(buf, str, len);

    return ZCFG_SUCCESS;
}
#endif

#ifdef MOS_SUPPORT
//zcfgRet_t zyUtilGetImageHeaderByImgIndex(int imgIndex, struct trx_header *imgHeader)
zcfgRet_t zyUtilGetImageHeaderByImgIndex(int imgIndex, void *imgHeader)
{

	return ret;
}

//zcfgRet_t zyUtilSetImageHeaderByImgIndex(int imgIndex, struct trx_header *imgHeader)
zcfgRet_t zyUtilSetImageHeaderByImgIndex(int imgIndex, void *imgHeader)
{

	return ret;
}
#endif

zcfgRet_t zyUtilIGetBootingFlag(int *boot_flag)
{
    char mtd_dev_ubi[16] = {'\0'};
    char mtd_dev_ubi2[16] = {'\0'};
    int mtd_num_ubi = 0;
    int mtd_num_ubi2 = 0;
    int mtd_num = 0;
    FILE *fp = NULL;

    if( (zyUtilMtdDevGetByName("ubi", mtd_dev_ubi)) == ZCFG_SUCCESS )
    {
       sscanf(mtd_dev_ubi, "/dev/mtd%d", &mtd_num_ubi);
    }
    else
    {
        printf("Failed to get mtd device ubi\n");
        return ZCFG_INTERNAL_ERROR;
    }

    if( (zyUtilMtdDevGetByName("ubi2", mtd_dev_ubi2)) == ZCFG_SUCCESS )
    {
       sscanf(mtd_dev_ubi2, "/dev/mtd%d", &mtd_num_ubi2);
    }
    else
    {
        printf("Failed to get mtd device ubi2\n");
        return ZCFG_INTERNAL_ERROR;
    }

	fp = fopen("/sys/devices/virtual/ubi/ubi0/mtd_num", "r");
	if (fp == NULL) {
		printf("ERROR : fopen fail\n");
		return ZCFG_INTERNAL_ERROR;
	}

	fscanf(fp, "%d", &mtd_num);
	fclose(fp);

    if(mtd_num == mtd_num_ubi)
        *boot_flag = 1;
    else if(mtd_num == mtd_num_ubi2)
        *boot_flag = 2;

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilISetBootingFlag(int boot_flag, bool usingfakeRootcommn)
{
    ZY_FW_INFO zyFwInfo_nonboot;
    ZY_FW_INFO zyFwInfo_current;
    int index = 0;

    if ( zyFwInfoGet(&zyFwInfo_current) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    if ( zyFwInfoNonBootGet(&zyFwInfo_nonboot, &index) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    if( zyFwInfo_nonboot.seq_num  == ZYFWINFI_MAX_SEQNUM )
        zyFwInfo_nonboot.seq_num = 0;
    else
        zyFwInfo_nonboot.seq_num = zyFwInfo_current.seq_num + 1;

    if ( zyFwInfoNonBootSet((char *)(&zyFwInfo_nonboot), index) == ZCFG_INTERNAL_ERROR )
    {
        printf("Failed to setzyfwinfoNonboot\n");
        return ZCFG_INTERNAL_ERROR;;
    }

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilGetBootImageHeader(void *booting_header)
{

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilGetSpecifiedBootImageHeader(int bootFlag, void *booting_header)
{

	return ZCFG_SUCCESS;
}

#if defined(ZYXEL_SUPPORT_EXT_FW_VER) || defined(ZYXEL_SUPPORT_BACKUP_FW_VER)
zcfgRet_t zyUtilGetDualImageExtInfo(bool bBackup, char *fwversion)
{
	ZY_FW_INFO zyFwInfo_nonboot;
	ZY_ATSH_INFO buf = {0};
	int index = 0;
	
	if(bBackup == true)
	{
		if ( zyFwInfoNonBootGet(&zyFwInfo_nonboot, &index) == ZCFG_INTERNAL_ERROR )
			return ZCFG_INTERNAL_ERROR;

		ZOS_STRNCPY(fwversion, zyFwInfo_nonboot.firmwareVersion, ZYFWINFO_FIRMWAREVERSION_LEN);
	}
	else if(bBackup == false)
	{
		memcpy(&buf, (PZY_ATSH_INFO)getATSHInfo(), sizeof(ZY_ATSH_INFO));
		ZOS_STRNCPY(fwversion, buf.zyFwInfoData.firmwareVersion, ZYFWINFO_FIRMWAREVERSION_LEN);
	}
	else
	{
		zyutil_dbg_printf("ERROR : bBackup == NULL\n");
		return 0;
	}
	
	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t zyUtilGetDualImageInfo(int bootFlag, char *fwversion)
{

	
	return ZCFG_SUCCESS;
}


int zyUtilGetGPIO(int gpioNum)
{
	/*This function is only used to check VMG3926-B10A and VMG8924-B10B, so Cross Platform is not need*/
	return 0;
}

int zyUtilSetGPIOoff(int gpioNum)
{
	/*This function is only used to check VMG3926-B10A and VMG8924-B10B, so Cross Platform is not need*/
	return 0;
}

int zyUtilSetGPIOon(int gpioNum)
{
	/*This function is only used to check VMG3926-B10A and VMG8924-B10B, so Cross Platform is not need*/
	return 0;
}


float transferOpticalTemperature(int input_value)
{
	int i;
	int lsb = 0;
	int msb = 0;
	float MSByte = 0.0;
	float LSByte = 0.0;
	float resultValue = 0.0;

	lsb = (0xff & input_value);
	msb = (0xff & (input_value >> 8));

	/* LSB */
	for ( i = 0 ; i < 8 ; i++ )
	{
		LSByte += ((1.0/256)*(lsb&(1<<i)));
	}

	if ( 0x80 & msb ) {
		MSByte = ((msb^0xff) + 1)*(-1);
	} else {
		MSByte = msb;
	}

	resultValue = MSByte + LSByte;

	return resultValue;
}


float logFunction(float x, int base, int decimalplaces)
{
	int integer = 0;
	float return_value = 0.0;
	float decimalplaces_count = 0.1;
	if (x <= 0.0)
		return -40;  //assume it is x = 1 when x is 0.

	while(x < 1)
	{
		integer = integer -1;
		x = x*base;
	}
	while(x >= base)
	{
		integer = integer + 1;
		x = x/base;
	}
	return_value = integer;
	while(decimalplaces > 0)
	{
		x = x*x*x*x*x*x*x*x*x*x;
		int digit = 0;
		while (x >= base)
		{
			digit = digit + 1;
			x = x/base;
		}

		return_value = return_value + (digit * decimalplaces_count);
		decimalplaces = decimalplaces - 1;
		decimalplaces_count = decimalplaces_count * 0.1;
	}

	return return_value;
}

float transferOpticalPower(float input_value)
{
	float return_value = 0.0;

	return_value = 10*logFunction(input_value, 10, 4);

	return return_value;
}

zcfgRet_t zyUtilGetWlanReserveAreaOffset(char *ifName, unsigned int *offset)
{
#if 0 // z56s
#ifdef TCSUPPORT_WLAN_AC	
	if(strstr(ifName, "rai") != NULL) {
		*offset = EEPROM_RA_AC_OFFSET;
	}
	else 
#endif
	{
		*offset = EEPROM_RA_OFFSET;
	}
#endif
	return ZCFG_SUCCESS;
}

/*!
 *  get size of rootfs
 *
 *  @return     size of rootfs (byte)
 *
 *  @note If get rootfs fail, popen fail, or fgets fail, return 0.
 */
int ras_size(void)
{

	FILE *fptr = NULL;
	char size[128] = {'\0'};
	char sysCmd[128] = {0};
	zos_snprintf(sysCmd, sizeof(sysCmd), "ubinfo -d 0 | grep Total | awk  {'print $7'}");

	if((fptr = popen(sysCmd, "r")) == NULL)
	{
		zyutil_dbg_printf("Can't fopen %s\n", sysCmd);
		return ZCFG_INTERNAL_ERROR;
	}

	if(fgets(size, sizeof(size) - 1, fptr) == NULL)
	{
		zyutil_dbg_printf("Can't fgets %s\n", sysCmd);
		fclose(fptr);
		return ZCFG_INTERNAL_ERROR;
	}
	if(size[strlen(size) - 1] == '\n')
		size[strlen(size) - 1] = '\0';
	fclose(fptr);

	return (atoi(size + 1)) - (5*1024*1024);
}

/*!
 *  check endian by c code
 *
 *  @return     0 - invalid cpu type
 *              1 - mips : big endian
 *              2 - arm : little endian
 *
 *  @note If the invalid cpu type, return 0.
 */
int endian_check_bycode(void)
{
	typedef union {
	  unsigned long l;
	  unsigned char c[4];
	} EndianTest;
	EndianTest et;
	  et.l = 0x12345678;
	  if (et.c[0] == 0x78 && et.c[1] == 0x56 && et.c[2] == 0x34 && et.c[3] == 0x12) {
		printf("Little Endiann\n");
		return 2;
	  } else if (et.c[0] == 0x12 && et.c[1] == 0x34 && et.c[2] == 0x56 && et.c[3] == 0x78) {
		printf("Big Endiann\n");
		return 1;
	  } else {
		printf("Unknown Endiann\n");
		return 0;
	  }
	 /* 
	 for (int i = 0; i < 4; i++)
	 {
		printf("%p : 0x%02Xn", &et.c[i], et.c[i]);
	 }
	 */
}

/*!
 *  check endian
 *
 *  @return     0 - invalid cpu type
 *              1 - mips : big endian
 *              2 - arm : little endian
 *
 *  @note If the invalid cpu type, popen fail, or fgets fail, return 0.
 */
int endian_check(void)
{
	FILE *fp = NULL;
	char buffer[64] = {0};
	char cpu[64] = {0};

	fp = popen("uname -m", "r");
	if (fp == NULL)
	{
	    zyutil_dbg_printf("ERROR : popen uname -m fail\n");
		return 0;
	}

	if (fgets(buffer, 64, fp) == NULL)
	{
	    zyutil_dbg_printf("ERROR : fgets fail\n");
	    pclose(fp);
		return 0;
	}

	sscanf(buffer, "%s", cpu);
	pclose(fp);

	if (strstr(cpu, "mips") != NULL)//big endian
	{
#ifdef	TCSUPPORT_LITTLE_ENDIAN 
		return 2;
#else
		return 1;
#endif
	}
	else if (strstr(cpu, "arm") != NULL)//little endian
	{
		return 2;
	}
	else
	{
		zyutil_dbg_printf("ERROR : invalid cpu type : %s\n", cpu);
		return 0;
	}
}

/*!
 *  check upload file (FW) magic number check
 *
 *  @param[in]  uploadfilename	the name of upload file
 *
 *  @return     0 - non-match
 *              1 - match
 *
 *  @note If uploadfilename is NULL, popen fail, fgets fail, or invalid cpu_type return 0.
 */
int magic_number_check(char *uploadfilename)
{
	int header_number1=-1;
	int header_number2=-1;
	int cpu_type=-1;
	FILE *fp = NULL;
	char buffer[64] = {0};
	char cmd[256] = {0};

	if (uploadfilename == NULL)
	{
	    zyutil_dbg_printf("ERROR : uploadfilename == NULL\n");
		return 0;
	}

	zos_snprintf(cmd, sizeof(cmd), "hexdump -n 4 /var/fw/%s", uploadfilename);
	fp = popen(cmd, "r");
	if (fp == NULL)
	{
	    zyutil_dbg_printf("ERROR : popen hexdump -n 4 /var/fw/%s fail\n", uploadfilename);
		return 0;
	}

	if (fgets(buffer, 64, fp) == NULL)
	{
	    zyutil_dbg_printf("ERROR : fgets fail\n");
	    pclose(fp);
		return 0;
	}

	sscanf(buffer, "0000000 %d %d", &header_number1, &header_number2);
	pclose(fp);

	//cpu_type = endian_check();
#if 1 //for special cpu architecture	
	cpu_type = endian_check_bycode();
#else
	cpu_type = endian_check();
#endif

	switch(cpu_type)
	{
		case 1 : //mips : big endian
		{
			if (header_number1 == HeaderNubmer1 && header_number2 == HeaderNubmer2)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		case 2: //arm : little endian
		{
			if (header_number1 == HeaderNubmer2 && header_number2 == HeaderNubmer1)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		default:
		{
			zyutil_dbg_printf("ERROR : invalid cpu_type : %d\n", cpu_type);
			return 0;
		}
	}
}

//-----------------------------------------------------
//mapping broadcom/libzyutil_wrapper.c.
//Avoid mistakes, So given the empty function
int zyzigbeereset_get(void)
{
    return -1;
}

int zyzigbeereset_set(void)
{
    return -1;
}

int zyupsstate_get(void)
{
    return -1;
}

int zysfpoperation(int val ,int input)
{
    return -1;
}
//-----------------------------------------------------

#ifdef ZYXEL_DETECT_PRODUCT_TYPE
int zyUtilAskBoardFeaturesStringBits(char *bits, int bitsLen)
{

   return 0;
}

int zyUtilAskProductName(char *prname, int prnameLen)
{

   return 0;
}

/*
*/
// return: -1: err, 1: has dsl, 0: dsl not on board
int zyUtilAskBoardDslSupport(const char *bits)
{
     return 1;
}

// return: -1: err, 1: has voip, 0: voip not on board
int zyUtilAskBoardVoIPSupport(const char *bits)
{
     return 1;
}
#endif

int zyGet81sn(char *buf, int len)
{
    char *str = NULL;

    str = getUbootEnv(ENV_SERIAL_NUMBER_81, strlen(ENV_SERIAL_NUMBER_81));
    if(str == NULL)
        return ZCFG_INTERNAL_ERROR;

    ZOS_STRNCPY(buf, str, len);

    return ZCFG_SUCCESS;
}


#ifdef ZYXEL_AT_SET_KEY
//int zyUtilIsPrintableString(char *tmp, int len){
//    int i=0;
//    for( i=0 ; i<len ; i++ )
//    {
//        if( (tmp[i] < 0x20) || (tmp[i] > 0x7e) )
//            return ZCFG_INTERNAL_ERROR;
//    }
//    return ZCFG_SUCCESS;
//}
extern int zyUtilIsPrintableString(char *tmp, int len);
#ifdef ZYXEL_AT_SET_SUPERVISOR_KEY
int zyGetSupervisorPWD(char *buf, int len)
{
	zcfgRet_t ret;
	char *result = NULL;
	char key[128] = {0};

	strcpy(key, ENV_SUPERVISOR);
	ret = zyUtilReqSendWithResponse(ZCFG_MSG_ATCK_GET, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(key), key, (char**)&result);
    if (ret != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Failed to get supervisor key\n");
    }

    if(strlen(result) < len)
	    ZOS_STRNCPY(buf, result, len);

	return ZCFG_SUCCESS;
}
int zyGetSupervisorPWDMaxLength(void)
{
	return MTK_SUPER_PWD_LEN;
}
#endif
int zyGetAdminPWD(char *buf, int len)
{
	zcfgRet_t ret;
	char *result = NULL;
	char key[128] = {0};

	strcpy(key, ENV_ADMIN);
	ret = zyUtilReqSendWithResponse(ZCFG_MSG_ATCK_GET, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(key), key, (char**)&result);
    if (ret != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Failed to get admin key\n");
    }

    if(strlen(result) < len)
	    ZOS_STRNCPY(buf, result, len);

	return ZCFG_SUCCESS;
}
int zyGetAdminPWDMaxLength(void)
{
	return MTK_ADMIN_PWD_LEN;
}
int zyGetPSK(char *buf, int len)
{
	zcfgRet_t ret;
	char *result = NULL;
	char key[128] = {0};

	strcpy(key, ENV_WPAPSKKEY);
	ret = zyUtilReqSendWithResponse(ZCFG_MSG_ATCK_GET, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, sizeof(key), key, (char**)&result);
    if (ret != ZCFG_SUCCESS)
    {
        ZLOG_ERROR("Failed to get PSK key key\n");
    }

    if(strlen(result) < len)
	    ZOS_STRNCPY(buf, result, len);

	return ZCFG_SUCCESS;
}
int zyGetPSKMaxLength(void)
{
	return MTK_KEYPASSPHRASE_LEN;
}
#endif


#if defined(ZYXEL_BOARD_DETECT_BY_GPIO)
/******* 
   dummy function, 
   prevent BRCM API compile failed in ECONET platform in esmd.c 
********/
zcfgRet_t zyUtilIGetScratchPadSetting(char *psp_wantype, char *psp_wanrate)
{
    return ZCFG_SUCCESS;
}
#endif //defined(ZYXEL_BOARD_DETECT_BY_GPIO)

#ifdef ZYXEL_MULTI_PROVINCE_VENDOR

zcfgRet_t zyUtilICheckModelName(struct PROVINCE_INFO_T *pro_info_t)
{
	return ZCFG_SUCCESS;
}


zcfgRet_t zyUtilGetProvinceInfo(struct PROVINCE_INFO_T *province_t)
{
	return ZCFG_SUCCESS;
}
#endif  //end ZYXEL_MULTI_PROVINCE_VENDOR

#endif	//end MTK_PLATFORM
