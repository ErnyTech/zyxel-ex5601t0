#ifdef ECONET_PLATFORM

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "libzyutil_wrapper.h"
#include "libzyutil_econet_wrapper.h"
#include "zcfg_msg.h"
#ifdef ZYXEL_MULTI_PROVINCE_VENDOR
#include "zyxel_province.h"
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

int read_mtd_info(const char *mtd, struct trx_header *p_trx_header)
{
	int mtd_fd = -1;
	int trx_header_len = sizeof(struct trx_header);

	if(-1 == (mtd_fd = mtd_open(mtd,O_RDONLY)))
	{
		printf("ERROR!Open mtd %s fail",mtd);
		return -1;
	}

	memset(p_trx_header,0,trx_header_len);

	if(trx_header_len != read(mtd_fd,p_trx_header,trx_header_len))
	{
		printf("ERROR!Read mtd %s fail",mtd);
		close(mtd_fd);
		return -1;
	}

	close(mtd_fd);
	return 0;
}

#if 0
static void dumpMRDinfo(PMRD_DATA mrd)
{
    int i;
    printf("\nMRD Dump(%d):\n", sizeof(MRD_DATA));
    printf("SDRAM_CONF_array[4]: 0x%02X 0x%02X 0x%02X 0x%02X\n",
           mrd->SDRAM_CONF_array[0], mrd->SDRAM_CONF_array[1], mrd->SDRAM_CONF_array[2], mrd->SDRAM_CONF_array[3]);
    printf("VENDOR_NAME[32]    : %s\n", mrd->VENDOR_NAME);
    printf("PRODUCT_NAME[32]   : %s\n", mrd->PRODUCT_NAME);
    printf("FUNC_BYTE[4]       : 0x%02X 0x%02X 0x%02X 0x%02X\n",
           mrd->FUNC_BYTE[0], mrd->FUNC_BYTE[1], mrd->FUNC_BYTE[2], mrd->FUNC_BYTE[3]);
    printf("ETHER_ADDR[6]      : %02X:%02X:%02X:%02X:%02X:%02X\n",
           mrd->ETHER_ADDR[0], mrd->ETHER_ADDR[1], mrd->ETHER_ADDR[2], mrd->ETHER_ADDR[3],
           mrd->ETHER_ADDR[4], mrd->ETHER_ADDR[5]);
    printf("COUNTRY_array      : 0x%02X\n", mrd->COUNTRY_array);
    printf("ether_gpio         : 0x%02X\n", mrd->ether_gpio);
    printf("power_gpio[2]      : 0x%02X 0x%02X\n", mrd->power_gpio[0], mrd->power_gpio[1]);
    printf("restore_flag       : 0x%02X\n", mrd->restore_flag);
    printf("dsl_gpio           : 0x%02X\n", mrd->dsl_gpio);
    printf("internet_gpio      : 0x%02X\n", mrd->internet_gpio);
    printf("SN[18]             : %s\n", mrd->SN);
    printf("SerialNumber[14]   : %s\n", mrd->SerialNumber);
    printf("NumMacAddrs        : %d\n", mrd->NumMacAddrs);
    printf("MrdVer             : 0x%02X\n", mrd->MrdVer);
    printf("EngDebugFlag       : 0x%02X\n", mrd->EngDebugFlag);
    printf("MainFeatureBit     : 0x%02X\n", mrd->MainFeatureBit);
    printf("FeatureBits[30]    :");
    for (i=0; i<30; i++) {
        if (i%15==0) puts("\n     ");
        printf("0x%02X ", mrd->FeatureBits[i]);
    }
    printf("\nHTP_Switch         : 0x%02X\n", mrd->HTP_Switch);
    printf("WpaPskKey[16]      :");
    for (i=0; i<16; i++) {
        if (i%16==0) puts("\n     ");
        printf("0x%02X ", mrd->WpaPskKey[i]);
    }
    printf("\nBM_VER[3]          : %d.%d.%d\n", mrd->BM_VER_MAJ, mrd->BM_VER_MIN, mrd->BM_VER_ECO);
    printf("builddate[4]       : %02X/%02X/%02X%02X\n", mrd->builddate[0], mrd->builddate[1],
           mrd->builddate[2], mrd->builddate[3]);
    printf("buildtime[3]       : %02X:%02X:%02X\n", mrd->buildtime[0], mrd->buildtime[1],
           mrd->buildtime[2]);
}
#else
#define dumpMRDinfo(x)
#endif

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

#if defined(TCSUPPORT_UBOOT)
/*
   ref. struct from below header
   bootloader/Uboot/u-boot-2014.04-rc1/include/image.h

   in order to prevent writing flash at zld.bin udpate, 
   read flash content directly instead of using fw_getenv() to get
	ENV_ZLD_VERSION
	ENV_ZLD_DATE
	ENV_ZLD_TIME

   2021/09/14, Trevor
*/
#define IH_NMLEN 32
#define CONFIG_ZLOADER_FLASH_START_ADDR 0x50000
typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;
#endif

zcfgRet_t zyUtilGetMrdInfo(PMRD_DATA mrd)
{
#ifndef TCSUPPORT_UBOOT
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	char username[16] = {0};
	void *sysCmd_reply=NULL;
	char filename[32] = {0};
#if 1 //IChia 20200117
	int  fd_template;
	FILE *fp_template;
#endif

	// clean the MRD info
	memset(mrd, 0x0, sizeof(MRD_DATA));
#if 1
	// Get MRD info from bootloader partition with offset 0xff00
	int uid = getuid();
	cuserid(username); 

	ZOS_STRNCPY(filename, MRDINFO_FILE_X, sizeof(filename));
#if 1 //IChia 20200117
	if ((fd_template = mkstemp(filename)) < 0 ){
#else
	if (mkstemp(filename) < 0 ){
#endif
		printf("%s: Can't create MRD file!\n",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	} 

	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,filename,(long unsigned int)MRDINFO_SIZE,(long unsigned int)MRDINFO_OFFSET,MRDINFO_PARTITON_NAME);

    if(uid == 0){
	    ZOS_SYSTEM(sysCmd);
    }else{
        zyUtilReqSendWithResponse(ZCFG_MSG_FAKEROOT_COMMAND_REPLY, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, strlen(sysCmd), sysCmd, (char**)&sysCmd_reply);

        //if((sysCmd_reply == NULL) && strcmp(sysCmd_reply, "1") != 0){
        if(!sysCmd_reply || (sysCmd_reply && strcmp(sysCmd_reply, "1") != 0)){
            printf("%s: Update Fail!!! \n", __FUNCTION__);        
        }
        ZOS_FREE(sysCmd_reply);
	    zos_snprintf(sysCmd,sizeof(sysCmd),"chmod 666 %s | chown %s %s",filename, username, filename);
        zyUtilReqSendWithResponse(ZCFG_MSG_FAKEROOT_COMMAND_REPLY, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, strlen(sysCmd), sysCmd, (char**)&sysCmd_reply);

        //if((sysCmd_reply == NULL) && strcmp(sysCmd_reply, "1") != 0){
        if(!sysCmd_reply || (sysCmd_reply && strcmp(sysCmd_reply, "1") != 0)){
            printf("%s: Update Fail!!! \n", __FUNCTION__);        
        }
        ZOS_FREE(sysCmd_reply);
    }

    /*Read MRD info to buffer*/
#if 1  //IChia 20200117
    if ((fp_template = fdopen(fd_template, "rb")) == NULL) {
#else
    if((fptr = ZOS_FOPEN(filename, "rb")) == NULL) {
#endif
        printf("%s: Can't fopen %s\n", __FUNCTION__,filename);
		unlink(filename);
        return ZCFG_INTERNAL_ERROR;
    }

#else
	/* check MRDINFO_FILE exists or not. */
	if ((fptr = ZOS_FOPEN(MRDINFO_FILE, "rb")) == NULL)
    {
		// Get MRD info from bootloader partition with offset 0xff00
    	printf("%s: %s does not exist, so create new one.\n", __func__, MRDINFO_FILE);
		zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,MRDINFO_FILE,(long unsigned int)MRDINFO_SIZE,(long unsigned int)MRDINFO_OFFSET,MRDINFO_PARTITON_NAME);
		ZOS_SYSTEM(sysCmd);
		chmod(MRDINFO_FILE, 438); // 438(decimal) = 666(octal)
	}
	/*Read MRD info to buffer*/
#endif

#if 1 //IChia 20200117
	fread(mrd, 1, (uint32_t)MRDINFO_SIZE, fp_template);
#else
	fread(mrd, 1, (uint32_t)MRDINFO_SIZE, fptr);
#endif
#ifdef ZYXEL_DETECT_PRODUCT_TYPE
	char prname[MRD_PRODUCT_NAME_LEN] = {0};
	int prnameLen = MRD_PRODUCT_NAME_LEN;
	if(zyUtilAskProductName(prname, prnameLen) > 0){
		if(strcmp(mrd->PRODUCT_NAME, prname)){
			/*Length of mrd->PRODUCT_NAME is MRD_PRODUCT_NAME_LEN, 32, Yen-Chun,Lin, 20210324*/
			ZOS_STRNCPY(mrd->PRODUCT_NAME, prname,MRD_PRODUCT_NAME_LEN);
			zyUtilSetMrdInfo(mrd);
		}
	}
#endif

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
	char cofw_prname[MRD_PRODUCT_NAME_LEN] = {0};
	int cofw_prnameLen = MRD_PRODUCT_NAME_LEN;
    if (zyUtilGetCoFwProductName(cofw_prname, cofw_prnameLen) > 0)
    {
        if(strcmp(mrd->PRODUCT_NAME, cofw_prname))
        {
			//cut the last line feed char
            strncpy(mrd->PRODUCT_NAME, cofw_prname, strlen(cofw_prname)-1);
            //printf("%s,%d, %s\n", __FUNCTION__, __LINE__, mrd->PRODUCT_NAME);
        }
    }
#endif

	dumpMRDinfo(mrd);
#if 1 //IChia 20200117
	ZOS_FCLOSE(fp_template);
#else
	ZOS_FCLOSE(fptr);
#endif
	unlink(filename);
#else
	char *str = NULL;
	char name[32] = {0};
	int i;
	char buf[3] = {0};
	int tmp;
	uint16_t tmp1, tmp2, tmp3, tmp4;
	uint16_t tmp5, tmp6, tmp7;

	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	char username[16] = {0};
	char filename[32] = {0};
	int  fd_template;
	FILE *fp_template;
	image_header_t zloader_buf;
	char bufName[IH_NMLEN+1];

	int uid = getuid();
	cuserid(username); 

	strcpy(filename, MRDINFO_FILE_X);
	if ((fd_template = mkstemp(filename)) < 0 ){
		printf("%s: Can't create MRD file!\n",__FUNCTION__);
		return ZCFG_INTERNAL_ERROR;
	} 

	sprintf(sysCmd,FLASH_READ_CMD,filename,(long unsigned int)sizeof(image_header_t),(long unsigned int)CONFIG_ZLOADER_FLASH_START_ADDR,MRDINFO_PARTITON_NAME);

    if (uid == 0)
	{
		system(sysCmd);
	}

    /*Read zloader info to buffer*/
    if ((fp_template = fdopen(fd_template, "rb")) == NULL) 
	{
        printf("%s: Can't fopen %s\n", __FUNCTION__,filename);
		unlink(filename);
        return ZCFG_INTERNAL_ERROR;
    }

	memset(bufName, 0x0, IH_NMLEN);
	memset(&zloader_buf, 0x0, sizeof(image_header_t));
	memset(mrd, 0x0, sizeof(MRD_DATA));
	fread(&zloader_buf, 1, sizeof(image_header_t), fp_template);

	//printf("%s,%d,zloader_buf.ih_name=%s\n", __FUNCTION__, __LINE__, zloader_buf.ih_name);
	strncpy(bufName, zloader_buf.ih_name, IH_NMLEN);
	bufName[IH_NMLEN]='\0';
	//printf("%s,%d,bufName=%s\n", __FUNCTION__, __LINE__, bufName);
	sscanf(&bufName, "zld-%2hu.%2hu %2hu/%2hu/%d %2hu:%2hu:%2hu", &tmp1, &tmp2, &tmp3, &tmp4, &tmp, &tmp5, &tmp6, &tmp7);

	mrd->BM_VER_MAJ = (unsigned char)tmp1;
	mrd->BM_VER_MIN = (unsigned char)tmp2;
	mrd->BM_VER_ECO = '0';
	
	mrd->builddate[0] = (unsigned char)tmp3;
	mrd->builddate[1] = (unsigned char)tmp4;

	/* handle year */
	tmp3 = tmp/100; /* '20'XX  */
	tmp4 = tmp%100; /*  XX'21' */
	mrd->builddate[2] = (unsigned char)tmp3;
	mrd->builddate[3] = (unsigned char)tmp4;
		
	mrd->buildtime[0] = (unsigned char)tmp5;
	mrd->buildtime[1] = (unsigned char)tmp6;
	mrd->buildtime[2] = (unsigned char)tmp7;

	fclose(fp_template);
	unlink(filename);

	str = fw_getenv(ENV_VENDOR_NAME);
	if(str)
	{
		strncpy(mrd->VENDOR_NAME, str, MRD_VENDOR_NAME_LEN);
	}

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
	char cofw_prname[MRD_PRODUCT_NAME_LEN] = {0};
	int cofw_prnameLen = MRD_PRODUCT_NAME_LEN;
    if (zyUtilGetCoFwProductName(cofw_prname, cofw_prnameLen) > 0)
    {
        if(strcmp(mrd->PRODUCT_NAME, cofw_prname))
        {
			//cut the last line feed char
            strncpy(mrd->PRODUCT_NAME, cofw_prname, strlen(cofw_prname)-1);
            //printf("%s,%d, %s\n", __FUNCTION__, __LINE__, mrd->PRODUCT_NAME);
        }
    }
#else
	str = fw_getenv(ENV_PRODUCT_NAME);
	if(str)
	{
		strncpy(mrd->PRODUCT_NAME, str, MRD_PRODUCT_NAME_LEN);
	}
#endif

	str = fw_getenv(ENV_SERIAL_NUMBER);
	if(str)
	{
		strncpy(mrd->SerialNumber, str, MRD_SERIAL_NUM_LEN);
	}

	str = fw_getenv(ENV_GPON_SERIAL_NUMBER);
	if(str)
	{
		for(i=0;i<MRD_GPON_SERIAL_NUM_LEN;i++)
		{
			strncpy(buf, str + 2*i, 2);
			buf[2] = '\0';
			sscanf(buf, "%x", &tmp);
			mrd->GponSerialNumber[i] = (unsigned char)tmp;
		}
	}

	str = fw_getenv(ENV_MAC_ADDRESS);
	if(str)
	{
		int temp_addr[MRD_MAC_ADDR_LEN] = {0};
		sscanf(str, "%x:%x:%x:%x:%x:%x", &temp_addr[0], &temp_addr[1], &temp_addr[2], &temp_addr[3], &temp_addr[4], &temp_addr[5]);
		for(i=0;i<MRD_MAC_ADDR_LEN;i++)
		{
			mrd->ETHER_ADDR[i] = (unsigned char)temp_addr[i];
		}
	}

	str = fw_getenv(ENV_WPAPSKKEY);
	if(str)
	{
		strncpy(mrd->WpaPskKey, str, MRD_KEYPASSPHRASE_LEN);
	}

	str = fw_getenv(ENV_ADMIN);
	if(str)
	{
		strncpy(mrd->admin, str, MRD_ADMIN_PWD_LEN);
	}

	str = fw_getenv(ENV_SUPERVISOR);
	if(str)
	{
		strncpy(mrd->supervisor, str, MRD_SUPER_PWD_LEN);
	}

	str = fw_getenv(ENV_MAC_ADDRESS_QUANTITY);
	if(str)
	{
		sscanf(str, "%hhu", &mrd->NumMacAddrs);
	}

	str = fw_getenv(ENV_COUNTRY_CODE);
	if(str)
	{
		sscanf(str, "%hhu", &mrd->COUNTRY_array);
	}

	str = fw_getenv(ENV_DEBUGFLAG);
	if(str)
	{
		sscanf(str, "%hhu", &mrd->EngDebugFlag);
	}

	str = fw_getenv(ENV_MAINFEATUREBIT);
	if(str)
	{
		sscanf(str, "%hhu", &mrd->MainFeatureBit);
	}

	str = fw_getenv(ENV_ONU_TYPE);
	if(str)
	{
		sscanf(str, "%hhu", &mrd->onu_type);
	}

	str = fw_getenv(ENV_FEATUREBITS);
	if(str)
	{
		for(i=0;i<MRD_FEATUREBITS_LEN;i++)
		{
			strncpy(buf, str + 2*i, 2);
			buf[2] = '\0';
			sscanf(buf, "%x", &tmp);
			mrd->FeatureBits[i] = (unsigned char)tmp;
		}
	}

	str = fw_getenv(ENV_SERIAL_NUMBER_81);
	if(str)
	{
		strncpy(mrd->SN81, str, MRD_SN81_LEN);
	}
#endif
	return ZCFG_SUCCESS;
}

#if defined (ZYXEL_BOARD_DETECT_BY_GPIO)
int zyUtilGetCoFwProductName(char *prname, int prnameLen)
{
	if(!prname){
		return -1;
	}

	FILE *pf = ZOS_FOPEN(ECONET_COFW_PRODUCT_NAME_PATH, "r");
	if(pf != NULL){
		int n = fread(prname, 1, prnameLen-1, pf);
		ZOS_FCLOSE(pf);
		return n;
	}
	else{
		return -1;
	}
}
#endif

#ifndef TCSUPPORT_UBOOT
zcfgRet_t zyUtilSetMrdInfo(PMRD_DATA mrd)
#else
zcfgRet_t zyUtilSetMrdInfo(char *name, char *value)
#endif
{
#ifndef TCSUPPORT_UBOOT
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	void *sysCmd_reply=NULL;

    /*Write MRD info into file*/
    if((fptr = ZOS_FOPEN(MRDINFO_FILE, "wb+")) == NULL) {
        printf("Can't fopen %s\n", MRDINFO_FILE);
		unlink(MRDINFO_FILE);
        return ZCFG_INTERNAL_ERROR;
    }
	fwrite(mrd, 1, (uint32_t)MRDINFO_SIZE, fptr);
	ZOS_FCLOSE_S(fptr);
	dumpMRDinfo(mrd);

	// Set MRD info from file into bootloader partition with offset 0xff00
	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_WRITE_CMD,MRDINFO_FILE,(long unsigned int)MRDINFO_SIZE,(long unsigned int)MRDINFO_OFFSET,MRDINFO_PARTITON_NAME);
	//ZOS_SYSTEM(sysCmd);
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

	zyUtilReqSendWithResponse(ZCFG_MSG_FAKEROOT_COMMAND_REPLY, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, strlen(sysCmd), sysCmd, (char**)&sysCmd_reply);

    //if((sysCmd_reply == NULL) && strcmp(sysCmd_reply, "1") != 0){
    if(!sysCmd_reply || (sysCmd_reply && strcmp(sysCmd_reply, "1") != 0)){
    	printf("%s: Update Fail!!! \n", __FUNCTION__);        
    }
	ZOS_FREE(sysCmd_reply);
	unlink(MRDINFO_FILE);
#else
	int argc_env = 3;
	char *argv_env[3];
	argv_env[0] = "fw_setenv";
	argv_env[1] = name;
	argv_env[2] = value;
	if(fw_setenv(argc_env, argv_env) != 0)
	{
		fprintf(stderr, "fw_setenv failed ! \n");
		return ZCFG_INTERNAL_ERROR;
	}
#endif
	return ZCFG_SUCCESS;
}

#ifdef SUPPORT_LANVLAN
zcfgRet_t zyUtilISetIfState(char *ifName, bool up)
{
	/*Todo*/
	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t zyUtilIGetBaseMAC(char *mac)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X",
		mrd_buff.ETHER_ADDR[0],
		mrd_buff.ETHER_ADDR[1],
        	mrd_buff.ETHER_ADDR[2],
        	mrd_buff.ETHER_ADDR[3],
        	mrd_buff.ETHER_ADDR[4],
        	mrd_buff.ETHER_ADDR[5]
	);

	//printf("mac:%s\n",mac);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBaseMACwithColon(char *mac)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	zos_snprintf(mac , MAC_ADDRESS_WITH_COLON, "%02X:%02X:%02X:%02X:%02X:%02X",
		mrd_buff.ETHER_ADDR[0],
		mrd_buff.ETHER_ADDR[1],
        	mrd_buff.ETHER_ADDR[2],
        	mrd_buff.ETHER_ADDR[3],
        	mrd_buff.ETHER_ADDR[4],
        	mrd_buff.ETHER_ADDR[5]
	);

	//printf("mac:%s\n",mac);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetOnuType(int *num)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	if(num) *num =  mrd_buff.onu_type;
	
	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	if(num) *num =  mrd_buff.NumMacAddrs;

	//printf("NumMacAddrs:%d\n",num);

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
	unsigned char mac_mask = (~_gdm_mask)&0xff;
	unsigned int macsequence = pucaMacAddr[5]&mac_mask;
    pucaMacAddr[5] = (pucaMacAddr[5]&_gdm_mask) | ((macsequence + n) & mac_mask);
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
	unsigned char targetMac[MRD_MAC_ADDR_LEN]={0};
	int _macQuantity=0;
	struct mrd mrd_buff;

#if defined(ZYXEL_LAN_WAN_WLAN_SHARE_THE_SAME_MAC)
	unsigned char _gdm_mask = 0xf8;
#else
	unsigned char _gdm_mask = 0xf0;
#endif

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return ZCFG_INTERNAL_ERROR;
	memcpy(targetMac, mrd_buff.ETHER_ADDR, MRD_MAC_ADDR_LEN);
	if( ZCFG_SUCCESS != zyUtilIGetNumberOfMAC(&_macQuantity) )
		return ZCFG_INTERNAL_ERROR;

	//Make sure the MAC we used are in the same MASK.
#if defined(ZYXEL_LAN_WAN_WLAN_SHARE_THE_SAME_MAC)
	if( _macQuantity != 8 ) {
		_macQuantity = 8;
	}
#else
	if( _macQuantity != 16 ) {
		_macQuantity = 16;
	}
#endif

	if( nth >= _macQuantity )
		return ZCFG_INTERNAL_ERROR;

	getNthMacAddr_special(targetMac, nth, _gdm_mask);
	//printf("%2d`s MAC Address with GDM MASK 0x%02X: %02X %02X %02X %02X %02X %02X\n", (unsigned int)nth, _gdm_mask, targetMac[0],targetMac[1],targetMac[2],targetMac[3],targetMac[4],targetMac[5]);
	memcpy(mac, targetMac, MRD_MAC_ADDR_LEN);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNthMACwithColon(char *mac, int nth)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilSetSoftwareLedState(ZYLED_INDEX index ,ZYLED_ACTION action, ZYLED_RATE blink_rate)
{
	char led_file_name[128] = {0}, sysCmd[128] = {0};

	switch(index) {
		case ZYLED_POWER_GREEN:
			ZOS_STRNCPY(led_file_name, "led_pwr_green", sizeof(led_file_name));
			break;
		case ZYLED_POWER_RED:
			ZOS_STRNCPY(led_file_name, "led_pwr_red", sizeof(led_file_name));
			break;
#ifdef ZYXEL_OPAL_EXTENDER
		case ZYLED_LINKQUALITY_GREEN:
			ZOS_STRNCPY(led_file_name, "led_quality_green", sizeof(led_file_name));;
			break;
		case ZYLED_LINKQUALITY_RED:
			ZOS_STRNCPY(led_file_name, "led_quality_red", sizeof(led_file_name));
			break;
		case ZYLED_LINKQUALITY_AMBER:
			ZOS_STRNCPY(led_file_name, "led_quality_amber", sizeof(led_file_name));
			break;
#endif
		default:
			break;
	}
#ifdef ZYXEL_ECHO_REPLACEMENT
	char echoString[4] = {0};
	char echoFile[32] = {0};
	zos_snprintf(echoString, sizeof(echoString), "%d\n", action);
	zos_snprintf(echoFile, sizeof(echoFile), "/proc/tc3162/%s", led_file_name);
	ZOS_ECHO_STR_TO_FILE(echoFile, echoString, strlen(echoString), "w");
#else
	zos_snprintf(sysCmd, sizeof(sysCmd), "echo %d > /proc/tc3162/%s", action, led_file_name);
	ZOS_SYSTEM(sysCmd);
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFeatureBits(char *featurebits)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGetModelID(char *model_id)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return ZCFG_INTERNAL_ERROR;

	memcpy(model_id, mrd_buff.FeatureBits, 4);

	//printf("ModelID:%x %x %x %x\n",model_id[0] model_id[1] model_id[2] model_id[3]);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	/*Length of mrd->PRODUCT_NAME is MRD_PRODUCT_NAME_LEN, 32, Yen-Chun,Lin, 20210324*/
	ZOS_STRNCPY(pdname, mrd_buff.PRODUCT_NAME, MRD_PRODUCT_NAME_LEN);

	//printf("ProductName:%s\n",pdname);

	return ZCFG_SUCCESS;
}
zcfgRet_t zyUtilIGetDSLVersion(char *DSLversion)
{
	char buf[64] = {0};
	char sysCmd[1024] = {0};
	FILE *fp = NULL;

	fp = ZOS_FOPEN("/proc/tc3162/adsl_fwver", "r");
	if(fp != NULL){
		while(fgets(buf, sizeof(buf), fp) != NULL){
			if(strstr(buf, "FwVer")){
				buf[strlen(buf)-2] = '\0'; //remove '\r\n'
				char *str1 = strchr(buf, ':'); str1++;
				char *str2 = strstr(str1, "HwVer");
				if((str2 - str1) < 65)
					ZOS_STRNCPY(DSLversion, str1, (str2 - str1));
				else
					ZOS_STRNCPY(DSLversion, str1, 64);
				return ZCFG_SUCCESS;
			}
		}		
		fclose(fp);
	}
    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetWiFiDriverVersion(char *wifidriverversion)
{
	zos_snprintf(wifidriverversion, 64, "%s", ECONET_WIFI_DRIVER_VERSION);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBootVersion(char *bootversion)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;
	
	zos_snprintf(bootversion, 64,"V%d.%d%d", mrd_buff.BM_VER_MAJ, mrd_buff.BM_VER_MIN, mrd_buff.BM_VER_ECO);

	return ZCFG_SUCCESS;
}
	
zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
	struct trx_header trx_header_buff;

	if(0 != zyUtilGetBootImageHeader((void *)&trx_header_buff))
		return ZCFG_INTERNAL_ERROR;

	memset(fwversion, 0x0, SW_VERSION_LEN);
	if( trx_header_buff.magic == TRX_MAGIC2 )	// check if have tclinux header!
		ZOS_STRNCPY(fwversion, trx_header_buff.swVersionInt, SW_VERSION_LEN);
	else
		return ZCFG_INTERNAL_ERROR;

	//printf("FirmwareVersion:%s\n",fwversion);

#ifdef ZYXEL_RMA_FW_SUPPORT
	/* PeterSu: force modify the first four chars to be "RMA_" */
	fwversion[0] = 'R';
	fwversion[1] = 'M';
	fwversion[2] = 'A';
	fwversion[3] = '_';
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetExternalFirmwareVersion(char *fwversion)
{
	struct trx_header trx_header_buff;

	if(0 != zyUtilGetBootImageHeader((void *)&trx_header_buff))
		return -1;

	/*Length of trx_header->swVersionInt is SW_VERSION_LEN, 32, Yen-Chun,Lin, 20210324*/
	ZOS_STRNCPY(fwversion, trx_header_buff.swVersionExt, SW_VERSION_LEN);

	return ZCFG_SUCCESS;
}

/*
 * The input string must have the length, "SERIAL_NUM_STR_LEN+1".
 */
zcfgRet_t zyUtilIGetSerialNumber(char *serianumber)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	/*Length of mrd->SerialNumber is MRD_SERIAL_NUM_LEN, 14, Yen-Chun,Lin, 20210324*/
	ZOS_STRNCPY(serianumber, mrd_buff.SerialNumber, MRD_SERIAL_NUM_LEN);

#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	/* need to append sub-sequence number */
	if( zyUtilIsAlphanumericString(mrd_buff.FeatureBits+MRD_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, MRD_FEATUREBITS_SUB_SERIAL_NUM_LEN) == 0 )
		strncat(serianumber, mrd_buff.FeatureBits+MRD_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, MRD_FEATUREBITS_SUB_SERIAL_NUM_LEN);
#endif
	//printf("SerialNumber:%s\n",serianumber);

	return ZCFG_SUCCESS;
}

/*
 * The input string must have the length, "SUB_SERIAL_NUM_STR_LEN+1".
 */
zcfgRet_t zyUtilIGetSubSerialNumber(char *sub_serianumber)
{
#ifdef CONFIG_ZYXEL_OPAL_EXTENDER_WITH_NPACK
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;

	if( zyUtilIsAlphanumericString(mrd_buff.FeatureBits+MRD_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, MRD_FEATUREBITS_SUB_SERIAL_NUM_LEN) != 0 )
		return -1;

	strncpy(sub_serianumber, mrd_buff.FeatureBits+MRD_FEATUREBITS_SUB_SERIAL_NUM_OFFSET, MRD_FEATUREBITS_SUB_SERIAL_NUM_LEN);

	//printf("SUB-SerialNumber:%s\n",sub_serianumber);
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetGponSerialNumber(char *serianumber)
{
	struct mrd mrd_buff;

	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;
#ifndef TC_SDK_7_3_251_900
	memcpy(serianumber, mrd_buff.GponSerialNumber,MRD_GPON_SERIAL_NUM_LEN);
#endif
	//printf("SerialNumber: %s\n",serianumber);

	return ZCFG_SUCCESS;
}

#ifdef MOS_SUPPORT
zcfgRet_t zyUtilGetImageHeaderByImgIndex(int imgIndex, struct trx_header *imgHeader)
{
	FILE *fp = NULL;
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;
	char sysCmd[128] = {0};

	zos_snprintf(sysCmd, sizeof(sysCmd), FLASH_READ_CMD,
		IMG_HEADER_FILE, (long unsigned int)BOOTIMG_HEADER_SIZE, (long unsigned int)BOOTIMG_HEADER_OFFSET, (1 == imgIndex) ? SLAVE_ROOTFS_PARTITION_NAME : MAIN_ROOTFS_PARTITION_NAME);
	ZOS_SYSTEM(sysCmd);

	/*because chmod command-line has been locked only for root use*/
	chmod(IMG_HEADER_FILE, 438);//438(decimal) = 666(octal)

	/*Read boot flag to buffer*/
	fp = ZOS_FOPEN(IMG_HEADER_FILE, "rb");
	if(NULL == fp)
	{
		printf("Can't fopen %s\n", IMG_HEADER_FILE);
	}
	else
	{
		memset(imgHeader, 0x0, BOOTIMG_HEADER_SIZE);
		fread(imgHeader, 1, (uint32_t)BOOTIMG_HEADER_SIZE, fp);

		ZOS_FCLOSE(fp);

		ret = ZCFG_SUCCESS;
	}

	unlink(IMG_HEADER_FILE);

	return ret;
}

zcfgRet_t zyUtilSetImageHeaderByImgIndex(int imgIndex, struct trx_header *imgHeader)
{
	FILE *fp = NULL;
	char *sysCmd_reply = NULL;
	char sysCmd[128] = {0};
	zcfgRet_t ret = ZCFG_INTERNAL_ERROR;
	zcfgEid_t eid = zcfgMsgQueryCurrentEid();

	if((0 != imgIndex) && (1 != imgIndex))
	{
		printf("%s(): what the imgIndex: %d\n", __func__, imgIndex);
	}
	else if(0 == eid)
	{
		printf("%s(): get eid fail, %d\n", __func__, eid);
	}
	else
	{
		fp = ZOS_FOPEN(IMG_HEADER_WRITEFILE, "wb");
		if(NULL == fp)
		{
			printf("%s(): Fail to open %s\n", __func__, IMG_HEADER_WRITEFILE);
		}
		else
		{
			fwrite(imgHeader, 1, sizeof(struct trx_header), fp);

			ZOS_FCLOSE(fp);

			zos_snprintf(sysCmd, sizeof(sysCmd), FLASH_WRITE_CMD,
				IMG_HEADER_WRITEFILE, (long unsigned int)sizeof(struct trx_header), 0, (1 == imgIndex) ? SLAVE_ROOTFS_PARTITION_NAME : MAIN_ROOTFS_PARTITION_NAME);

			if(ZCFG_SUCCESS != zyUtilReqSendWithResponse(ZCFG_MSG_FAKEROOT_COMMAND_REPLY, eid, ZCFG_EID_ESMD_GROUND, strlen(sysCmd), sysCmd, &sysCmd_reply))
			{
				printf("%s(): zyUtilReqSendWithResponse fail\n", __func__);
			}
			else if((!sysCmd_reply) ||
				(0 != strcmp(sysCmd_reply, "1")))
			{
				printf("%s(): Update Fail!!!\n", __func__);
				ZOS_FREE(sysCmd_reply);
			}
			else
			{
				ZOS_FREE(sysCmd_reply);
				ret = ZCFG_SUCCESS;
			}
		}
	}

	unlink(IMG_HEADER_WRITEFILE);

	return ret;
}
#endif

zcfgRet_t zyUtilIGetBootingFlag(int *boot_flag)
{
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	char bootflag[2] = {0};

#ifdef MOS_SUPPORT
	/* to handle mos reset boot_flag */
	if(-1 != access(BOOTIMAGE_FILE, F_OK))
	{
		/* file exists */
		fptr = ZOS_FOPEN(BOOTIMAGE_FILE, "r");
		if(NULL == fptr)
		{
			printf("fail to fopen %s\n", BOOTIMAGE_FILE);

			return ZCFG_INTERNAL_ERROR;
		}
	}
	else
	{
		/* file doesn't exist */
		/* so need to read boot_flag */
		zos_snprintf(sysCmd, sizeof(sysCmd), FLASH_READ_CMD,
			BOOT_FLAG_FILE, (long unsigned int)BOOT_FLAG_SIZE, (long unsigned int)BOOT_FLAG_OFFSET, RESERVEAREA_NAME);
		ZOS_SYSTEM(sysCmd);

		chmod(BOOT_FLAG_FILE, 438);// 438(decimal) = 666(octal)

		/* Read boot flag to buffer */
		fptr = fopen(BOOT_FLAG_FILE, "rb");
		if(NULL == fptr)
		{
			unlink(BOOT_FLAG_FILE);
			printf("fail to fopen %s\n", BOOT_FLAG_FILE);

			return ZCFG_INTERNAL_ERROR;
		}
	}
#else
	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,BOOT_FLAG_FILE,(long unsigned int)BOOT_FLAG_SIZE,(long unsigned int)BOOT_FLAG_OFFSET,RESERVEAREA_NAME);
	ZOS_SYSTEM(sysCmd);
#if 0 // because chmod command-line has been locked only for root use
	zos_snprintf(sysCmd,sizeof(sysCmd),"chmod 666 %s",BOOT_FLAG_FILE);
	ZOS_SYSTEM(sysCmd);
#else
	chmod(BOOT_FLAG_FILE, 438); // 438(decimal) = 666(octal)
#endif
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

    /*Read boot flag to buffer*/
    if((fptr = ZOS_FOPEN(BOOT_FLAG_FILE, "rb")) == NULL) {
        unlink(BOOT_FLAG_FILE);
        printf("Can't fopen %s\n", BOOT_FLAG_FILE);
        return ZCFG_INTERNAL_ERROR;
    }
#endif

	memset(bootflag, 0x0, 2);
	fread(bootflag, 1, (uint32_t)BOOT_FLAG_SIZE, fptr);
	*boot_flag = bootflag[0] - '0';

    /*
     * ref. bootrom/bootram/init/main.c:: readBootFlagFromFlash() 
     * this byte may be "FF"" when NAND flash just burned
     */
    if (*boot_flag != 0 && *boot_flag != 1)
    {
        *boot_flag = 0;
    }

	//printf("%s : Current boot flag is %s(%d)\n", __FUNCTION__, bootflag, *boot_flag);
	ZOS_FCLOSE(fptr);
	unlink(BOOT_FLAG_FILE);

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilISetBootingFlag(int boot_flag,bool usingfakeRootcommn)
{
	char sysCmd[128] = {0};
    void *sysCmd_reply=NULL;    
    
	if(boot_flag !=0 && boot_flag != 1) return ZCFG_INTERNAL_ERROR;
#ifdef ZYXEL_ECHO_REPLACEMENT
	char echoString[4] = {0};
	zos_snprintf(echoString, sizeof(echoString), "%d\n", boot_flag);
#ifdef MOS_SUPPORT
	ZOS_ECHO_STR_TO_FILE(BOOT_FLAG_WRITEFILE, echoString, strlen(echoString), "w");
#else
	ZOS_ECHO_STR_TO_FILE(BOOT_FLAG_FILE, echoString, strlen(echoString), "w");
#endif
#else//! ZYXEL_ECHO_REPLACEMENT
#ifdef MOS_SUPPORT
	zos_snprintf(sysCmd, sizeof(sysCmd), "echo %d > %s", boot_flag, BOOT_FLAG_WRITEFILE);
#else
	zos_snprintf(sysCmd, sizeof(sysCmd), "echo %d > %s", boot_flag, BOOT_FLAG_FILE);
#endif
	ZOS_SYSTEM(sysCmd);
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);
#endif //#ifdef ZYXEL_ECHO_REPLACEMENT
#ifdef MOS_SUPPORT
	zos_snprintf(sysCmd, sizeof(sysCmd), FLASH_WRITE_CMD,
		BOOT_FLAG_WRITEFILE, (long unsigned int)BOOT_FLAG_SIZE, (long unsigned int)BOOT_FLAG_OFFSET, RESERVEAREA_NAME);
#else
	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_WRITE_CMD,BOOT_FLAG_FILE,(long unsigned int)BOOT_FLAG_SIZE,(long unsigned int)BOOT_FLAG_OFFSET,RESERVEAREA_NAME);
#endif
	if(!usingfakeRootcommn){
		ZOS_SYSTEM(sysCmd);
		printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);
	}else{
		//system(sysCmd);
		//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);
		zcfgEid_t id = zcfgMsgQueryCurrentEid();
		zyUtilReqSendWithResponse(ZCFG_MSG_FAKEROOT_COMMAND_REPLY, id, ZCFG_EID_ESMD_GROUND, strlen(sysCmd), sysCmd, (char**)&sysCmd_reply);

		//if((sysCmd_reply == NULL) && strcmp(sysCmd_reply, "1") != 0){
		if(!sysCmd_reply || (sysCmd_reply && strcmp(sysCmd_reply, "1") != 0)){
			printf("%s: Update Fail!!! \n", __FUNCTION__);        
		}
	}
#ifdef MOS_SUPPORT
	unlink(BOOT_FLAG_WRITEFILE);
#else
	unlink(BOOT_FLAG_FILE);
#endif

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilGetBootImageHeader(void *booting_header)
{
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	struct trx_header *local_booting_header = (struct trx_header *)booting_header;
	int currentBootFlag;

	zyUtilIGetBootingFlag(&currentBootFlag);
	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,BOOTIMG_HEADER_FILE,(long unsigned int)BOOTIMG_HEADER_SIZE,(long unsigned int)BOOTIMG_HEADER_OFFSET, \
			(currentBootFlag==1)?SLAVE_ROOTFS_PARTITION_NAME:MAIN_ROOTFS_PARTITION_NAME);
	ZOS_SYSTEM(sysCmd);
#if 0 // because chmod command-line has been locked only for root use
	zos_snprintf(sysCmd,sizeof(sysCmd),"chmod 666 %s",BOOTIMG_HEADER_FILE);
	ZOS_SYSTEM(sysCmd);
#else
	chmod(BOOTIMG_HEADER_FILE, 438); // 438(decimal) = 666(octal)
#endif
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

    /*Read boot flag to buffer*/
    if((fptr = ZOS_FOPEN(BOOTIMG_HEADER_FILE, "rb")) == NULL) {
		unlink(BOOTIMG_HEADER_FILE);
        printf("Can't fopen %s\n", BOOTIMG_HEADER_FILE);
        return ZCFG_INTERNAL_ERROR;
    }

	memset(local_booting_header, 0x0, BOOTIMG_HEADER_SIZE);
	fread(local_booting_header, 1, (uint32_t)BOOTIMG_HEADER_SIZE, fptr);
	//printf("%s : Current sequence is %d\n", __FUNCTION__, local_booting_header->imageSequence);
	ZOS_FCLOSE(fptr);
	unlink(BOOTIMG_HEADER_FILE);

	if( local_booting_header->magic != TRX_MAGIC2 )	// check if have tclinux header!
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilGetSpecifiedBootImageHeader(int bootFlag, void *booting_header)
{
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	struct trx_header *local_booting_header = (struct trx_header *)booting_header;

	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,BOOTIMG_HEADER_FILE,(long unsigned int)BOOTIMG_HEADER_SIZE,(long unsigned int)BOOTIMG_HEADER_OFFSET, \
			(bootFlag==1)?SLAVE_ROOTFS_PARTITION_NAME:MAIN_ROOTFS_PARTITION_NAME);
	ZOS_SYSTEM(sysCmd);

	chmod(BOOTIMG_HEADER_FILE, 438); // 438(decimal) = 666(octal)
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

    /*Read boot flag to buffer*/
    if((fptr = ZOS_FOPEN(BOOTIMG_HEADER_FILE, "rb")) == NULL) {
		unlink(BOOTIMG_HEADER_FILE);
        printf("Can't fopen %s\n", BOOTIMG_HEADER_FILE);
        return ZCFG_INTERNAL_ERROR;
    }

	memset(local_booting_header, 0x0, BOOTIMG_HEADER_SIZE);
	fread(local_booting_header, 1, (uint32_t)BOOTIMG_HEADER_SIZE, fptr);
	//printf("%s : Current sequence is %d\n", __FUNCTION__, local_booting_header->imageSequence);
	ZOS_FCLOSE(fptr);
	unlink(BOOTIMG_HEADER_FILE);

	if( local_booting_header->magic != TRX_MAGIC2 )	// check if have tclinux header!
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}

#if defined(ZYXEL_SUPPORT_EXT_FW_VER) || defined(ZYXEL_SUPPORT_BACKUP_FW_VER)
zcfgRet_t zyUtilGetDualImageExtInfo(bool bBackup, char *fwversion)
{
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	struct trx_header local_booting_header;
	int currentBootFlag;

	zyUtilIGetBootingFlag(&currentBootFlag);

#if defined(ZYXEL_SUPPORT_BACKUP_FW_VER)
	if ( bBackup == true )
	{
		currentBootFlag = !currentBootFlag;
	}
#endif

	zos_snprintf(sysCmd,sizeof(sysCmd),FLASH_READ_CMD,BOOTIMG_HEADER_FILE,(long unsigned int)BOOTIMG_HEADER_SIZE,(long unsigned int)BOOTIMG_HEADER_OFFSET, \
			(currentBootFlag==1)?SLAVE_ROOTFS_PARTITION_NAME:MAIN_ROOTFS_PARTITION_NAME);
	ZOS_SYSTEM(sysCmd);
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

    /*Read boot flag to buffer*/
    if((fptr = ZOS_FOPEN(BOOTIMG_HEADER_FILE, "rb")) == NULL) {
		unlink(BOOTIMG_HEADER_FILE);
        printf("Can't fopen %s\n", BOOTIMG_HEADER_FILE);
        return ZCFG_INTERNAL_ERROR;
    }

	memset(&local_booting_header, 0x0, BOOTIMG_HEADER_SIZE);
	fread(&local_booting_header, 1, (uint32_t)BOOTIMG_HEADER_SIZE, fptr);
	//printf("%s : Current sequence is %d\n", __FUNCTION__, local_booting_header->imageSequence);
	ZOS_FCLOSE(fptr);
	unlink(BOOTIMG_HEADER_FILE);

	memset(fwversion, 0x0, SW_VERSION_LEN);
	if( local_booting_header.magic == TRX_MAGIC2 )	// check if have tclinux header!
		ZOS_STRNCPY(fwversion, local_booting_header.swVersionExt, SW_VERSION_LEN);
	else
		return ZCFG_INTERNAL_ERROR;

	return ZCFG_SUCCESS;
}
#endif

zcfgRet_t zyUtilGetDualImageInfo(int bootFlag, char *fwversion)
{
	char sysCmd[128] = {0};
	FILE *fptr = NULL;
	struct trx_header local_booting_header;

	zos_snprintf(sysCmd, sizeof(sysCmd),FLASH_READ_CMD,BOOTIMG_HEADER_FILE,(long unsigned int)BOOTIMG_HEADER_SIZE,(long unsigned int)BOOTIMG_HEADER_OFFSET, \
			(bootFlag==1)?SLAVE_ROOTFS_PARTITION_NAME:MAIN_ROOTFS_PARTITION_NAME);
	ZOS_SYSTEM(sysCmd);
	//printf("%s:sysCmd = %s\n", __FUNCTION__, sysCmd);

    /*Read boot flag to buffer*/
    if((fptr = ZOS_FOPEN(BOOTIMG_HEADER_FILE, "rb")) == NULL) {
		unlink(BOOTIMG_HEADER_FILE);
        printf("Can't fopen %s\n", BOOTIMG_HEADER_FILE);
        return ZCFG_INTERNAL_ERROR;
    }

	memset(&local_booting_header, 0x0, BOOTIMG_HEADER_SIZE);
	fread(&local_booting_header, 1, (uint32_t)BOOTIMG_HEADER_SIZE, fptr);
	//printf("%s : Current sequence is %d\n", __FUNCTION__, local_booting_header->imageSequence);
	ZOS_FCLOSE(fptr);
	unlink(BOOTIMG_HEADER_FILE);

	// fix beGpon crash when read inactive tclinux which is empty or invalid
	//printf("%s: local_booting_header.magic=%d\n", __FUNCTION__, local_booting_header.magic);
	memset(fwversion, 0x0, SW_VERSION_LEN);
	if(local_booting_header.magic == TRX_MAGIC2)
		/*Length of trx_header->swVersionInt is SW_VERSION_LEN, 32, Yen-Chun,Lin, 20210324*/
		ZOS_STRNCPY(fwversion, local_booting_header.swVersionInt, SW_VERSION_LEN);
	else
		return ZCFG_INTERNAL_ERROR;

	
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
#ifdef TCSUPPORT_WLAN_AC	
	if(strstr(ifName, "rai") != NULL) {
		*offset = EEPROM_RA_AC_OFFSET;
	}
	else 
#endif
	{
		*offset = EEPROM_RA_OFFSET;
	}

	return ZCFG_SUCCESS;
}

/*!
 *  get size of tclinux_slave
 *
 *  @return     size of tclinux_slave (byte)
 *
 *  @note If get tclinux_slave fail, popen fail, or fgets fail, return 0.
 */
int ras_size()
{
	int size=0;
	FILE *fp = NULL;
	char buffer[64] = {0};

	fp = popen("cat /proc/mtd  | grep tclinux_slave | awk '{print $2}'", "r");

	if (fp == NULL)
	{
		zyutil_dbg_printf("ERROR : popen fail\n");
		return 0;
	}

	if (fgets(buffer, 64, fp) == NULL)
	{
		zyutil_dbg_printf("ERROR : fgets fail\n");
		pclose(fp);
		return 0;
	}

	sscanf(buffer, "%X", &size);
	pclose(fp);

	return size;

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
int endian_check_bycode()
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
int endian_check()
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
   FILE *pf = ZOS_FOPEN(ECONET_BOARDFEATURES_PROC_PATH, "r");
   if(!pf || !bits){
     if(pf != NULL) {ZOS_FCLOSE(pf); pf = NULL;}
     return -1;
   }

   int n = fread(bits, 1, bitsLen-1, pf);
   ZOS_FCLOSE(pf);
   return n;
}

int zyUtilAskProductName(char *prname, int prnameLen)
{
   FILE *pf = ZOS_FOPEN(ECONET_BOARDFEATURES_PRNAME_PATH, "r");

   if(!pf || !prname){
     if(pf != NULL) {ZOS_FCLOSE(pf); pf = NULL;}
     return -1;
   }

   int n = fread(prname, 1, prnameLen-1, pf);

   ZOS_FCLOSE(pf);

   return n;
}

/*
000  (2 port FXS) VMG8623-T50B
001  (1 port FXS) EMG5523-T50B (VMG8623-T50B abandon VD WAN)
010  (0 port FXS) VMG3625-T50B (VMG8623-T50B VoIP not involved)
011               EMG3525-T50B (VMG8623-T50B but VD WAN, VoIP not there)
110               VMG3625-T50C
111               EMG3525-T50C (VMG3625-T50C remove VD WAN)
*/
// return: -1: err, 1: has dsl, 0: dsl not on board
int zyUtilAskBoardDslSupport(const char *bits)
{
   int n = 0;
   char prtype[100] = {0}, **p = NULL;

   n = bits ? strlen(bits) : 0;

   if(n < 3){
     return -1;
   }
   printf("%s: %s\n", __FUNCTION__, bits);
   strncpy(prtype, bits + (n-3), sizeof(prtype)-1);

   unsigned long type = strtoul((const char *)prtype, p, 2);
   if(type & ECONET_BOARDFEATURES_NO_DSL)
     return 0;
   else
     return 1;
}

// return: -1: err, 1: has voip, 0: voip not on board
int zyUtilAskBoardVoIPSupport(const char *bits)
{
   int n = 0;
   char prtype[100] = {0}, **p = NULL;

   n = bits ? strlen(bits) : 0;
   if(n < 3){
     return -1;
   }
   strncpy(prtype, bits + (n-3), sizeof(prtype)-1);

   unsigned long type = strtoul((const char *)prtype, p, 2);
   if(type & ECONET_BOARDFEATURES_NO_VOIP)
     return 0;
   else
     return 1;
}
#endif

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
int zyGetSupervisorPWD(char *buf, int len){
    MRD_DATA mrd = {0};
    int _string_len=0;

    if( (MRD_SUPER_PWD_LEN+1) != len)
        return ZCFG_INTERNAL_ERROR;

    if (ZCFG_SUCCESS != zyUtilGetMrdInfo(&mrd))
        return ZCFG_INTERNAL_ERROR;

    _string_len = strlen(mrd.supervisor);
    if( (_string_len == 0) || zyUtilIsPrintableString(mrd.supervisor, (_string_len>MRD_SUPER_PWD_LEN)?MRD_SUPER_PWD_LEN:_string_len) != 0 )
        return ZCFG_INTERNAL_ERROR;

    memcpy(buf, mrd.supervisor, MRD_SUPER_PWD_LEN);
    printf("%s Set to default passowrd\n", __FUNCTION__);

    return ZCFG_SUCCESS;
}
int zyGetSupervisorPWDMaxLength(void){
	return MRD_SUPER_PWD_LEN;
}
#endif
int zyGetAdminPWD(char *buf, int len){
    MRD_DATA mrd = {0};
    int _string_len=0;

    if( (MRD_ADMIN_PWD_LEN+1) != len)
        return ZCFG_INTERNAL_ERROR;

    if (ZCFG_SUCCESS != zyUtilGetMrdInfo(&mrd))
        return ZCFG_INTERNAL_ERROR;

    _string_len = strlen(mrd.admin);
    if( (_string_len == 0) || zyUtilIsPrintableString(mrd.admin, (_string_len>MRD_ADMIN_PWD_LEN)?MRD_ADMIN_PWD_LEN:_string_len) != 0 )
        return ZCFG_INTERNAL_ERROR;

    memcpy(buf, mrd.admin, MRD_ADMIN_PWD_LEN);
    printf("%s Set to default passowrd\n", __FUNCTION__);

    return ZCFG_SUCCESS;
}
int zyGetAdminPWDMaxLength(void){
	return MRD_ADMIN_PWD_LEN;
}
int zyGetPSK(char *buf, int len){
    MRD_DATA mrd = {0};
    int _string_len=0;

    if( (MRD_KEYPASSPHRASE_LEN+1) != len)
        return ZCFG_INTERNAL_ERROR;

    if (ZCFG_SUCCESS != zyUtilGetMrdInfo(&mrd))
        return ZCFG_INTERNAL_ERROR;

    _string_len = strlen(mrd.WpaPskKey);
    if( (_string_len == 0) || zyUtilIsPrintableString(mrd.WpaPskKey, (_string_len>MRD_KEYPASSPHRASE_LEN)?MRD_KEYPASSPHRASE_LEN:_string_len) != 0 )
        return ZCFG_INTERNAL_ERROR;

    memcpy(buf, mrd.WpaPskKey, MRD_KEYPASSPHRASE_LEN);
    return ZCFG_SUCCESS;
}
int zyGetPSKMaxLength(void){
	return MRD_KEYPASSPHRASE_LEN;
}
#endif

int zyGetCountryCode(){
    MRD_DATA mrd = {0};

    if (ZCFG_SUCCESS != zyUtilGetMrdInfo(&mrd))
        return ZCFG_INTERNAL_ERROR;

    return mrd.COUNTRY_array;
}

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
	struct mrd mrd;
	if((0 != zyUtilGetMrdInfo(&mrd)) || (pro_info_t == NULL))
		return -1;
	/* compare 'pro_info_t->model_name' and 'mrd.PRODUCT_NAME' */	
	if(strcmp(pro_info_t->model_name, mrd.PRODUCT_NAME)){
		strncpy(mrd.PRODUCT_NAME, pro_info_t->model_name, sizeof(mrd.PRODUCT_NAME));				
		if (zyUtilSetMrdInfo(&mrd) != 0) {
			printf("%s\n", ZCFG_SET_ATTR_ERROR);
			return -1;
		}
	}
	return ZCFG_SUCCESS;
}


zcfgRet_t zyUtilGetProvinceInfo(struct PROVINCE_INFO_T *province_t)
{
	char province[MRD_PROVINCE_NAME_LEN]={0}, vendor[MRD_VENDOR_NAME_LEN]={0};
	struct mrd mrd_buff;
	if(0 != zyUtilGetMrdInfo(&mrd_buff))
		return -1;
	memcpy(province, mrd_buff.PROVINCE_NAME, MRD_PROVINCE_NAME_LEN);
	memcpy(vendor, mrd_buff.VENDOR_NAME, MRD_VENDOR_NAME_LEN);

	uint32_t i = 0;
	uint32_t max = (sizeof(province_setting_tables) / sizeof(province_setting_tables[0]));
	for (i = 0; i < max ; i++) 
	{
		if (!strcmp(province, province_setting_tables[i].province_code) &&
			!strcmp(vendor, province_setting_tables[i].vendor_code))
		{
			memcpy( &(province_t->province_code) , &(province_setting_tables[i].province_code), sizeof(province_t->province_code));			
			memcpy( &(province_t->vendor_code) , &(province_setting_tables[i].vendor_code), sizeof(province_t->vendor_code));
			memcpy( &(province_t->ext_sw_ver) , &(province_setting_tables[i].ext_sw_ver), sizeof(province_t->ext_sw_ver));
			memcpy( &(province_t->hw_ver) , &(province_setting_tables[i].hw_ver), sizeof(province_t->hw_ver));
			memcpy( &(province_t->model_name) , &(province_setting_tables[i].model_name), sizeof(province_t->model_name));
			break;
		}
	}
 	if ( (province_t->province_code !=NULL) && (province_t->vendor_code!=NULL) &&
		!strcmp(province, province_t->province_code) &&	!strcmp(vendor, province_t->vendor_code)){
 		return ZCFG_SUCCESS;
	} else{		
 		return ZCFG_NOT_FOUND;
 	}
}
#endif  //end ZYXEL_MULTI_PROVINCE_VENDOR

#endif	//end ECONET_PLATFORM
