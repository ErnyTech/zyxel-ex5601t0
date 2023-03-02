#ifdef BROADCOM_UBOOT_PLATFORM
#include "libzyutil_wrapper.h"
#include "libzyutil_brcm_wrapper.h"

/*from common.c*/
#include "board.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "zos_api.h"
#include "zcfg_msg.h"
#include "zyfwinfo.h"
#include <byteswap.h>
#include <sys/mman.h>

// ref: bcm_flashutil.c start
#define PKGTB_METADATA_SIZE 256                                 //FIXME: Must match uboot value
#define PKGTB_METADATA_RDWR_SIZE (PKGTB_METADATA_SIZE + 1024 )  //FIXME: Must match uboot value
#define PKGTB_METADATA_MAX_SIZE 4096
// end

#define ZYFWINFO_VOL_ID_1 18  //ZYXEL : support zyfwinfo (sync bcm_imgif_pkgtb.c)
#define ZYFWINFO_VOL_ID_2 19  //ZYXEL : support zyfwinfo (sync bcm_imgif_pkgtb.c)

#define BOARD_DEVICE_NAME  "/dev/brcmboard"

//fdt magic  D0 0D FE ED
#define FdtMagicNubmer_1 "0DD0"
#define FdtMagicNubmer_2 "EDFE"

int rasSize = 0;

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen; // using this to identify the gpio number.
        IoctlParms.offset = offset;
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");
    return boardFd;
}

/*
 *  This function is using for access memory with 4 bytes size
 *  type: 1 means write memory, and 0 means read memory
 *  addr: the access memory address. For example 0xFFFF0000
 *  value: read out or write into memory through it.
 */
zcfgRet_t memaccess(int type, unsigned long long addr, unsigned long *value)
{
    int siz = 4;
    BOARD_MEMACCESS_IOCTL_PARMS parms;
    int fd;

    if ((fd = open (BOARD_DEVICE_NAME, O_RDWR)) < 0)
    {
        printf ("Can't open /dev/brcmboard ");
    }

    parms.address = addr;
    parms.size = siz;
    parms.space = BOARD_MEMACCESS_IOCTL_SPACE_REG;
    parms.count = 1;
    parms.buf = (char *)value;
    if( type == 0)
        parms.op = BOARD_MEMACCESS_IOCTL_OP_READ;
    else if( type == 1)
    parms.op = BOARD_MEMACCESS_IOCTL_OP_WRITE;
    else
        return ZCFG_INTERNAL_ERROR;

    ioctl(fd, BOARD_IOCTL_MEM_ACCESS, &parms);
    close(fd);

    //printf("[%s]reg %08x value was %08x\n", __func__, addr, *value );
    return ZCFG_SUCCESS;
}


// get metadata val start (get booting partition)
#if 1 // ref: bcm_flashutil.c  &  crc.c
typedef struct
{
    unsigned int crc;
    char data[1024];
} __attribute__((__packed__)) env_t;

typedef struct
{
    unsigned int word0;
    unsigned int size;
    env_t mdata_obj;
} __attribute__((__packed__)) MDATA;

static unsigned int local_crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

unsigned int genUtl_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ local_crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

static unsigned int pkgtb_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc_initial)
{
    unsigned int crc_final = genUtl_getCrc32(pdata, size, crc_initial ^ 0xffffffff) ^ 0xffffffff;
    return crc_final;
}

static int validate_metadata(MDATA *mdatap, int * committed, int *valid, int *seq)
{
    int valid_img_idx[2] = {0};
    int committed_idx = 0;
    int seq_img_idx[2] = {-1};
    uint32_t crc;
    int ret = 0;
    int i;
    char * commitp = mdatap->mdata_obj.data;
    char * validp =  commitp + strlen(commitp) + 1;
    char * seqp =    validp  + strlen(validp)  + 1;
    crc = pkgtb_getCrc32((unsigned char*)mdatap->mdata_obj.data, (mdatap->size - 4) & 0xffff, 0);

    if( crc ==  mdatap->mdata_obj.crc )
    {
        sscanf(commitp,"COMMITTED=%d", &committed_idx);
        sscanf(validp,"VALID=%d,%d", &valid_img_idx[0], &valid_img_idx[1]);
        //FIXME: Magic numbers
        for( i=0; i<2; i++ )
        {
            if( valid_img_idx[i] )
                valid[valid_img_idx[i]-1] = valid_img_idx[i];
        }
        *committed = committed_idx;

        // preset the sequence numbers incase the field doesn't exist
        if ((*committed == 1) && valid[0])
        {
            seq[0] = 1;
            if (valid[1])
                seq[1] = 0;
        }
        else if ((*committed == 2) && valid[1])
        {
            seq[1] = 1;
            if (valid[0])
                seq[0] = 0;
        }

        sscanf(seqp,"SEQ=%d,%d", &seq_img_idx[0], &seq_img_idx[1]);
        for( i=0; i<2; i++ )
        {
            if( seq_img_idx[i] != -1)
                seq[i] = seq_img_idx[i];
        }

        //printf("rd_metadata: committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);
    }
    else
    {
        printf("ERROR: metadata crc failed! exp: 0x%08x  calc: 0x%08x\n", mdatap->mdata_obj.crc, crc);
        *committed = 0;
        valid[0] = 0;
        valid[1] = 0;
        ret = -1;
    }
    return ret;
}

int getNandMetadata( char * data, int size , int mdata_idx)
{
    char mdata_fname[128];
    FILE * mdata_fh;
    int num_bytes = 0;
    sprintf(mdata_fname, "/dev/ubi0_%d", mdata_idx);
    mdata_fh = fopen(mdata_fname, "r");

    if( mdata_fh )
    {
        num_bytes = fread(data, 1, size, mdata_fh);

        if( ferror(mdata_fh) )
        {
            printf("%s: Error fread metadata%d!\n", __FUNCTION__, mdata_idx);
            num_bytes = 0;
        }

        fclose(mdata_fh);
    }
    else
    {
        printf("%s: Error fopen metadata%d!\n", __FUNCTION__, mdata_idx);
    }

    return num_bytes;
}

static int get_metadata_val( int * committed, int * valid, int * seq)
{
    MDATA * mdatap = (MDATA *)malloc(PKGTB_METADATA_MAX_SIZE);
    int ret = -1;
    int num_bytes_read = 0;
    int mdata_idx;

    for( mdata_idx=1; (mdata_idx<=2) && (ret < 0); mdata_idx++ )
    {
        num_bytes_read = getNandMetadata( (char*)mdatap, PKGTB_METADATA_RDWR_SIZE, mdata_idx);

        if( num_bytes_read >=  PKGTB_METADATA_SIZE )
            ret = validate_metadata(mdatap, committed, valid, seq);
        else
            fprintf(stderr, "ERROR!!! unable to retrieve metadata%d!", mdata_idx);
    }
    free(mdatap);

    if( ret )
        fprintf(stderr, "ERROR!!! unable to retrieve/parse metadata!");

    return ret;
}

static int getImgCommitStatus( int img_idx, char * commit_flag  )
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( committed_img == img_idx )
        *commit_flag = '1';
    else
        *commit_flag = '0';
    return 0;
}

#endif
// get metadata val end

//get/set env start
zcfgRet_t GetEnvThroughProc(char *name,int len, char *return_buf)
{
    FILE *fp = NULL;
    char filename[128];
    sprintf(filename, "/proc/environment/%s",name);

    if( access( filename, F_OK ) != -1 )
    {
        if ((fp = fopen(filename, "r")) == NULL)
        {
            //printf("could not open %s", filename);
            return ZCFG_INTERNAL_ERROR;
        }
    }
    else
		return ZCFG_INTERNAL_ERROR;
		
    if( NULL == fgets(return_buf, len, fp) )
    {
        //printf("Could not read %s", filename);
        fclose(fp);
        return ZCFG_INTERNAL_ERROR;
    }

    fclose(fp);
    return ZCFG_SUCCESS;
}

zcfgRet_t SetEnvThroughProc(char *name, int len, char *str)
{
    FILE *fptr = NULL;
    if((fptr = fopen("/proc/nvram/set", "w")) == NULL)
    {
        printf("[%s][%s][%d] : [%s] set error\n", __FILE__, __func__, __LINE__, name);
        return ZCFG_INTERNAL_ERROR;
    }

    if (fptr)
    {
        if (NULL == str)
        {
            fprintf(fptr, "%s=", name);
        }
        else
        {
            if (strlen(str) >= len)
            {
                printf("[%s][%s][%d] : [%s] len error.  input:%d   cmp:%d\n", __FILE__, __func__, __LINE__, name, strlen(str), len);
                fclose(fptr);
                return ZCFG_INTERNAL_ERROR;
            }
            else
            {
                fprintf(fptr, "%s=%s", name, str);
            }
        }
        fclose(fptr);
        return ZCFG_SUCCESS;
    }
    else
        return ZCFG_INTERNAL_ERROR;
}

//get/set env end


//env start
#ifdef ZYXEL_AT_SET_KEY
extern int zyUtilIsPrintableString(char *tmp, int len);

#ifdef ZYXEL_AT_SET_SUPERVISOR_KEY
int zyGetSupervisorPWD(char *buf, int len)
{
    if (len > NVRAM_SUPERVISOR_LEN || len <= 0)
        return ZCFG_INTERNAL_ERROR;

    if ( GetEnvThroughProc(NVRAM_SUPERVISOR, len, buf) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int zyGetSupervisorPWDMaxLength(void)
{
    return NVRAM_SUPER_PWD_LEN;
}
#endif

int zyGetAdminPWD(char *buf, int len)
{
    if (len > NVRAM_ADMIN_LEN || len <= 0)
        return ZCFG_INTERNAL_ERROR;

    if ( GetEnvThroughProc(NVRAM_ADMIN, len, buf) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int zyGetAdminPWDMaxLength(void)
{
    return NVRAM_ADMIN_PWD_LEN;
}

int zyGetPSK(char *buf, int len)
{
    if (len > NVRAM_WPAPSKKEY_LEN || len <= 0)
        return ZCFG_INTERNAL_ERROR;

    if ( GetEnvThroughProc(NVRAM_WPAPSKKEY, len, buf) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int zyGetPSKMaxLength(void){
    return NVRAM_KEYPASSPHRASE_LEN;
}
#endif

//MAC start
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
    char tmp_mac[MAC_ADDRESS_WITH_COLON] = {0};
    char *buf = tmp_mac;

    if ( GetEnvThroughProc(NVRAM_UCABASEMACADDR, MAC_ADDRESS_WITH_COLON, tmp_mac) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    filter_string(tmp_mac, ":", (MAC_ADDRESS_LEN - 1)); //XX:XX:XX:XX:XX:XX =>XXXXXXXXXXXX
    //Convert the characters to capitals
    while(*buf)
    {
        *buf = toupper((unsigned char)*buf);
        buf++;
    }
    strncpy(mac, tmp_mac, MAC_ADDRESS_LEN);
    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetBaseMACwithColon(char *mac)
{
//XX:XX:XX:XX:XX:XX
    if ( GetEnvThroughProc(NVRAM_UCABASEMACADDR, MAC_ADDRESS_WITH_COLON, mac) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

int ahex2int(char a, char b)
{
    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}


zcfgRet_t zyUtilIGetLastMAC(char *mac, int nth, char *base_mac)
{
    int i =0;
    unsigned long cal = 0;
    int  val[6] = {0};

    for( i = 0; i < 6; ++i)
    {
        val[i] = ahex2int(*(base_mac+(i*2)),*(base_mac+((i*2)+1)));
    }

    cal = (val[3]  << 16) | (val[4]  << 8) | val[5] ;
    cal = (cal + nth -1) & 0xffffff;
    val[3] = (cal >> 16) & 0xff;
    val[4] = (cal >> 8) & 0xff;
    val[5] = (cal ) & 0xff;

    zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X",
        val[0],
        val[1],
        val[2],
        val[3],
        val[4],
        val[5]
    );
    return ZCFG_SUCCESS;
}


/*!
 *  Get Nth MAC.
 *  Broadcom platform do not need to use this function.
 *
 *  @param[out]    pucaMacAddr   out N`th MAC, should be 6 bytes.
 *  @param[in]     n             Get N`th MAC.
 *
 *  @note  The MAC length should be 6 bytes.
 */
zcfgRet_t zyUtilIGetNthMAC(char *mac, int nth)
{
    int i =0;
    unsigned long cal = 0;
    char mac_tmp[13] = {0};
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

    zos_snprintf(mac , MAC_ADDRESS_LEN, "%02X%02X%02X%02X%02X%02X",
        val[0],
        val[1],
        val[2],
        val[3],
        val[4],
        val[5]
    );
    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetNthMACwithColon(char *mac, int nth)
{
    int i =0;
    unsigned long cal = 0;
    char mac_tmp[13] = {0};
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

    zos_snprintf(mac , MAC_ADDRESS_WITH_COLON, "%02X:%02X:%02X:%02X:%02X:%02X",
        val[0],
        val[1],
        val[2],
        val[3],
        val[4],
        val[5]
    );
    return ZCFG_SUCCESS;
}


//default : 13
zcfgRet_t zyUtilIGetNumberOfMAC(int *num)
{
    char tmp[4] = {0};
    if ( GetEnvThroughProc(NVRAM_ULNUMMACADDRS, sizeof(tmp), tmp) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    *num=atoi(tmp);
    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetFeatureBits(char *featurebits)
{
	//TODO, fill here if you need.
	return ZCFG_INTERNAL_ERROR;
}

//"04080007" => 0x04 0x08 0x00 0x07
zcfgRet_t zyUtilIGetModelID(char *model_id)
{
    int i = 0;
    char features[NVRAM_FEATUREBITS_HEX_LEN] = {0};

    if ( GetEnvThroughProc(NVRAM_FEATUREBITS, NVRAM_FEATUREBITS_HEX_LEN, features) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    for( i = 0; i < 4; ++i)
    {
       strncpy(model_id+i, features+((2*i)+1), 1);
    }

    return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetProductName(char *pdname)
{
    if ( GetEnvThroughProc(NVRAM_PRODUCTNAME, NVRAM_PRODUCTNAME_LEN, pdname) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetSerialNumber(char *serianumber)
{
    if ( GetEnvThroughProc(NVRAM_SERIALNUMBER, NVRAM_SERIALNUMBER_LEN, serianumber) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;
    else
        return ZCFG_SUCCESS;
}

zcfgRet_t zyUtilIGetDefaultGponSnPw(char *serianumber, char *password)
{
    if ( GetEnvThroughProc(NVRAM_GPONSERIALNUMBER, NVRAM_GPON_SERIAL_NUMBER_LEN, serianumber) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    if ( GetEnvThroughProc(NVRAM_GPONPASSWORD, NVRAM_GPON_PASSWORD_LEN, password) == ZCFG_INTERNAL_ERROR )
        return ZCFG_INTERNAL_ERROR;

    return ZCFG_SUCCESS;
}
//env end


//zyfwinfo start
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


int get_booting_partition_num(void)
{
    char commitflag1 = 0;
    char commitflag2 = 0;

    getImgCommitStatus(1, &commitflag1);
    getImgCommitStatus(2, &commitflag2);

    if (commitflag1 == '1')
    {
        return 1;
    }
    else if (commitflag2 == '1')
    {
        return 2;
    }
    else
    {
        printf("[%s][%s][%d]Error\n",__FILE__,__func__,__LINE__);
        return ZCFG_INTERNAL_ERROR;
    }
}


int getzyfwinfo ( char * data, int size , int zyfwinfo_idx)
{
    char tmp_fname[128];
    FILE * zyfwinfo_fh;
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
    uint16_t zyCheckSum_old=0;
    uint16_t zyCheckSum_new=0;
    int num_bytes_read = 0;
    short int num = -1;

    num = get_booting_partition_num();

    //'1' => Boot partition
    if (num == 1)
    {
        num_bytes_read = getzyfwinfo((char *)pzyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_1);
    }
    else if (num == 2)
    {
        num_bytes_read = getzyfwinfo((char *)pzyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_2);
    }
    else
    {
        printf("[%s][%s][%d]Error\n",__FILE__,__func__,__LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    //check CRC
    zyCheckSum_old = pzyFwInfo->zy_checksum;
    pzyFwInfo->zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)pzyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);

    if( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old))
    {
        return ZCFG_INTERNAL_ERROR;
    }
    else
    {
        return ZCFG_SUCCESS;
    }
}


int zyFwInfoNonBootGet(PZY_FW_INFO pzyFwInfo)
{
    uint16_t zyCheckSum_old=0;
    uint16_t zyCheckSum_new=0;
    int num_bytes_read = 0;
    short int num = -1;

    num = get_booting_partition_num();

    //'0' => nonboot
    if (num == 2)
    {
        num_bytes_read = getzyfwinfo((char *)pzyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_1);
    }
    else if (num == 1)
    {
        num_bytes_read = getzyfwinfo((char *)pzyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_2);
    }
    else
    {
        printf("[%s][%s][%d]Error\n",__FILE__,__func__,__LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    //check CRC
    zyCheckSum_old = pzyFwInfo->zy_checksum;
    pzyFwInfo->zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)pzyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);

    if( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old))
    {
        return ZCFG_INTERNAL_ERROR;
    }
    else
    {
        return ZCFG_SUCCESS;
    }
}


zcfgRet_t zyUtilIGetFirmwareVersion(char *fwversion)
{
    uint16_t zyCheckSum_old=0;
    uint16_t zyCheckSum_new=0;
    int num_bytes_read = 0;
    short int num = -1;
    ZY_FW_INFO zyFwInfo = {0};

    num = get_booting_partition_num();

    //'1' => Boot partition
    if (num == 1)
    {
        num_bytes_read = getzyfwinfo((char *)&zyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_1);
    }
    else if (num == 2)
    {
        num_bytes_read = getzyfwinfo((char *)&zyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_2);
    }
    else
    {
        printf("[%s][%s][%d]Error\n",__FILE__,__func__,__LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    //check CRC
    zyCheckSum_old = zyFwInfo.zy_checksum;
    zyFwInfo.zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)&zyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);


    if( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old))
    {
        return ZCFG_INTERNAL_ERROR;
    }
    else
    {
        ZOS_STRNCPY(fwversion, zyFwInfo.firmwareVersion, NVRAM_FIRMWAREVERSION_LEN);
        return ZCFG_SUCCESS;
    }
}


zcfgRet_t zyUtilIGetExternalFirmwareVersion(char *fwversion)
{
    uint16_t zyCheckSum_old=0;
    uint16_t zyCheckSum_new=0;
    int num_bytes_read = 0;
    short int num = -1;
    ZY_FW_INFO zyFwInfo = {0};

    //'1' => Boot partition
    if (num == 1)
    {
        num_bytes_read =getzyfwinfo((char *)&zyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_1);
    }
    else if (num == 2)
    {
        num_bytes_read =getzyfwinfo((char *)&zyFwInfo, ZY_FW_INFO_SIZE, ZYFWINFO_VOL_ID_2);
    }
    else
    {
        printf("[%s][%s][%d]Error\n",__FILE__,__func__,__LINE__);
        return ZCFG_INTERNAL_ERROR;
    }

    //check CRC
    zyCheckSum_old = zyFwInfo.zy_checksum;
    zyFwInfo.zy_checksum = 0;
    zyCheckSum_new = zyxelChecksum((uint8_t *)&zyFwInfo, ZY_FW_INFO_SIZE);
    zyCheckSum_new = ZY_HOST_TO_TARGET16(zyCheckSum_new);


    if( (num_bytes_read <  ZY_FW_INFO_SIZE) && (zyCheckSum_new != zyCheckSum_old))
    {
        return ZCFG_INTERNAL_ERROR;
    }
    else
    {
        ZOS_STRNCPY(fwversion, zyFwInfo.firmwareVersionEx, NVRAM_FIRMWAREVERSION_LEN);
        return ZCFG_SUCCESS;
    }
}
//-------------------
//zyfwinfo end

#ifdef SUPPORT_LANVLAN
zcfgRet_t zyUtilISetIfState(char *ifName, bool up)
{
    int sockfd = 0;
    struct ifreq ifr;
    zcfgRet_t ret = ZCFG_SUCCESS;

    if(ifName == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : Cannot bring up NULL interface\n", __FUNCTION__);
        ret = ZCFG_INTERNAL_ERROR;
    }
    else
    {
        /* Create a channel to the NET kernel. */
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            zcfgLog(ZCFG_LOG_ERR, "%s : Cannot create socket to the NET kernel\n", __FUNCTION__);
            ret = ZCFG_INTERNAL_ERROR;
        }
        else
        {
            strncpy(ifr.ifr_name, ifName, IFNAMSIZ);
            // get interface flags
            if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) != -1)
            {
                if(up)
                    ifr.ifr_flags |= IFF_UP;
                else
                    ifr.ifr_flags &= (~IFF_UP);

                if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
                {
                    zcfgLog(ZCFG_LOG_ERR, "%s : Cannot ioctl SIOCSIFFLAGS on the socket\n", __FUNCTION__);
                    ret = ZCFG_INTERNAL_ERROR;
                }
            }
            else
            {
                zcfgLog(ZCFG_LOG_ERR, "%s : Cannot ioctl SIOCGIFFLAGS on the socket\n", __FUNCTION__);
                ret = ZCFG_INTERNAL_ERROR;
            }
            close(sockfd);
        }
    }
    return ret;
}
#endif

zcfgRet_t zyUtilIGetDSLVersion(char *DSLversion)
{
    char buf[64] = {0};
    char sysCmd[1024] = {0};
    FILE *fp = NULL;
    char *action = NULL;


    zos_snprintf(sysCmd, sizeof(sysCmd), "xdslctl --version 2>&1 | tee /var/DSLversion");
    ZOS_SYSTEM(sysCmd);

    fp = ZOS_FOPEN("/var/DSLversion", "r");
    if(fp != NULL)
    {
        while(fgets(buf, sizeof(buf), fp) != NULL)
        {
            buf[strlen(buf)-1] = '\0'; //remove '\n'
            if((action = strstr(buf,"version -")) != NULL)
            {
                action= action + strlen("version - ");
                ZOS_STRNCPY(DSLversion, action, 64);
            }
        }
        ZOS_FCLOSE(fp);
    }
    return ZCFG_SUCCESS;
}


int zyUtilGetGPIO(int gpioNum)
{
    int ret = 0;
    ret = boardIoctl(BOARD_IOCTL_GET_GPIO, 0, NULL, gpioNum, 0, NULL);
    return ret;
}

int zyUtilSetGPIOoff(int gpioNum)
{
    int ret = 0;
    ret = boardIoctl(BOARD_IOCTL_SET_GPIO, 0, NULL, gpioNum, 1, NULL);
    return ret;
}

int zyUtilSetGPIOon(int gpioNum)
{
    int ret = 0;
    ret = boardIoctl(BOARD_IOCTL_SET_GPIO, 0, NULL, gpioNum, 0, NULL);
    return ret;
}

#if defined(ZYXEL_BOARD_DETECT_BY_GPIO) || defined(CONFIG_BCM96856)
#if defined(ZYXEL_BOARD_DETECT_BY_GPIO) && defined(ZYXEL_SUPPORT_RDPAWAN_CONFIGURE)
int zyUtilISetScratchPadSetting(const char *key, const void *buf, int bufLen)
{
    //to do ....
    return ZCFG_INTERNAL_ERROR;
}
#endif

zcfgRet_t zyUtilIGetScratchPadSetting(char *psp_wantype, char *psp_wanrate)
{
    //to do ....
    return ZCFG_INTERNAL_ERROR;
}

zcfgRet_t zyUtilIGUIGetRDPAWAN(char *psp_wantype, char *psp_wanrate)
{
    //to do ....
    return ZCFG_INTERNAL_ERROR;
}
#endif


void processLed(const int32_t ledType, const int32_t state)
{
    boardIoctl(BOARD_IOCTL_LED_CTRL, 0, NULL, ledType, state, NULL);
}



int32_t  get_BrcmLedIndex(ZYLED_INDEX index) {
    int32_t LEDindex;
    switch(index)
    {
        case ZYLED_POWER_GREEN:
            LEDindex = kLedPowerOn;
        break;
        case ZYLED_POWER_RED:
            LEDindex = kLedPostFail;
        break;
#ifdef ZYXEL_OPAL_EXTENDER
        case ZYLED_LINKQUALITY_GREEN:
            LEDindex = kLedQualityGreen;
        break;
        case ZYLED_LINKQUALITY_RED:
            LEDindex = kLedQualityRed;
        break;
        case ZYLED_LINKQUALITY_AMBER:
            LEDindex = kLedQualityAmber;
        break;
        case ZYLED_WIRELESS_GREEN:
            LEDindex = kLedWiFiWPS;
        break;
#endif
#if defined(LTE_SUPPORT)
        case ZYLED_CELLSIGNAL1_GREEN:
            LEDindex = kLedCellSignal1;
        break;
        case ZYLED_CELLSIGNAL2_GREEN:
            LEDindex = kLedCellSignal2;
        break;
        case ZYLED_CELLSIGNAL3_GREEN:
            LEDindex = kLedCellSignal3;
        break;
#endif
        default:
            LEDindex = kLedStateOff;
        break;
    }
    return LEDindex;
}

int32_t  get_BrcmLedState(ZYLED_ACTION action, ZYLED_RATE blink_rate) {
    int32_t state = 0;
    switch(action)
    {
        case ZYLED_ACTION_OFF:
            state = kLedStateOff;
        break;
        case ZYLED_ACTION_ON:
            state = kLedStateOn;
        break;
        case ZYLED_ACTION_BLINK:
            if(blink_rate == ZYLED_RATE_SLOW)
            {
                state = kLedStateSlowBlinkContinues;
            }
            else if(blink_rate == ZYLED_RATE_FAST)
            {
                state = kLedStateFastBlinkContinues;
            }
        break;
#ifdef ZYXEL_OPAL_EXTENDER
        case ZYLED_ACTION_UP:
            state = kLedStateUp;
        break;
        case ZYLED_ACTION_DOWN:
            state = kLedStateDown;
        break;
#endif
        default:
            state = kLedStateOff;
        break;
    }
    return state;
}

zcfgRet_t zyUtilSetSoftwareLedState(ZYLED_INDEX index ,ZYLED_ACTION action, ZYLED_RATE blink_rate)
{
    processLed(get_BrcmLedIndex(index), get_BrcmLedState(action, blink_rate));
    return ZCFG_SUCCESS;
}


/*!
 *  get size of rootfs
 *
 *  @return     size of rootfs (byte)
 *
 *  @note  If get rootfs fail, popen fail, or fgets fail, return 0.
 */
int ras_size()
{
	char *recvBuf = NULL;
	zcfgRet_t ret = ZCFG_SUCCESS;
#if 0
    FILE *fp = NULL;
    char buffer[128] = {0};
    char filename[128];
    int size = 0;
    snprintf(filename, sizeof(filename), "/proc/environment/%s", "mtdparts");

    if( access( filename, F_OK ) != -1 )
    {
        if ((fp = fopen(filename, "r")) == NULL)
        {
            printf("could not open %s\n", filename);
            return 0;
        }
    }
    else{
        printf("could not access filename:%s\n", filename);
        return 0;
    }

    if( NULL == fgets(buffer, 128, fp) )
    {
        printf("Could not read %s\n", filename);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    sscanf(buffer, "%*[^,],%d[^@]", &size);
#endif

	if (rasSize)
	{
		return (rasSize / 2);
	}
	else
	{
		ret = zyUtilMsgSend(ZCFG_MSG_FAKEROOT_GET_RAS_SIZE, ZCFG_EID_SYS_CMD, ZCFG_EID_ESMD, 0, NULL, (char**)&recvBuf);
		if (ret == ZCFG_SUCCESS)
		{
			sscanf(recvBuf, "%*[^,],%d[^@]", &rasSize);
			zcfgLog(ZCFG_LOG_DEBUG, "%s : rasSize=%d \n", __FUNCTION__, rasSize);
		}
		else
		{
			zcfgLog(ZCFG_LOG_ERR, "%s : Fail to get rasSize of rootfs \n", __FUNCTION__);
		}
		return (rasSize / 2);
	}
}

//check fdt header
/*!
 *  check upload file (FW) magic number
 *
 *  @param[in]  uploadfilename   the name of upload file
 *
 *  @return     0 - non-match
 *              1 - match
 *
 *  @note  If uploadfilename is NULL, popen fail, or fgets fail, return 0.
 */
int magic_number_check(char *uploadfilename)
{
    char fdt_magic_1[5] = {0};
    char fdt_magic_2[5] = {0};
    char buffer[64] = {0};

    if (uploadfilename == NULL)
    {
        zyutil_dbg_printf("ERROR : uploadfilename == NULL\n");
        return 0;
    }

    memcpy(&buffer, uploadfilename, sizeof(buffer) - 1);
    zos_snprintf(fdt_magic_1, sizeof(fdt_magic_1), "%02X%02X", buffer[1], buffer[0]);
    zos_snprintf(fdt_magic_2, sizeof(fdt_magic_2), "%02X%02X", buffer[3], buffer[2]);

    if (!(strncmp(fdt_magic_1, FdtMagicNubmer_1,4)) && !(strncmp(fdt_magic_2, FdtMagicNubmer_2,4)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#ifdef ZYXEL_ZIGBEE_SUPPORT
/*!
 *  ZIGBEE reset (get state)
 *
 *  @return    -1 - ioctl failed
 *              0 - Low
 *              1 - High
 *              2 - ZIGBEE not supported
 *  @note get zigbee reset state
 */
int zyzigbeereset_get(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1)
    {
        ioctlParms.result = -2;//Because -1 to 2 has been used, the initial value of -2

        rc = ioctl(boardFd, BOARD_IOCTL_GET_ZIGBEE_RESET, &ioctlParms);
        close(boardFd);

        if (rc < 0 || (ioctlParms.result != 0 && ioctlParms.result != 1) )
        {
            if (ioctlParms.result == -1)
            {
                zyutil_dbg_printf("ZIGBEE not supported\n");
                return 2;
            }
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }
        zyutil_dbg_printf("get zigbee reset value: %d\n",ioctlParms.result);
        return ioctlParms.result;
    }
    else
    {
        return -1;
    }
}

/*!
 *  ZIGBEE reset
 *
 *  @return    -1 - ioctl failed
 *              0 - Fail to reset zigbee module
 *              1 - Reset zigbee module succeed
 *  @note ZIGBEE reset
 */
int zyzigbeereset_set(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1)
    {
        ioctlParms.strLen =  1;//start up zigbee reset
        ioctlParms.result = -1;

        rc = ioctl(boardFd, BOARD_IOCTL_SET_ZIGBEE_RESET, &ioctlParms);
        close(boardFd);

        if(rc < 0)
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }

        if (ioctlParms.result == 1)
        {
            zyutil_dbg_printf("Reset zigbee module succeed.\n");
            return ioctlParms.result;
        }
        else
        {
            zyutil_dbg_printf("Fail to reset zigbee module.\n");
            return ioctlParms.result;
        }
    }
    else
    {
        return -1;
    }
}
#endif

#ifdef ZYXEL_UPS_FEATURE
/*!
 *  get UPS state
 *
 *  @return    -1            - ioctl failed
 *             Globe value   - UPS state
 *
 *  @note   Globe value: ups_active_status
 *         (bit3,bit2,bit1,bit0) => (miss battery, on battery, low battery, fail battery)
 *          bit0 : 1(active) - fail Self Test, 0(inactive) - battery is charged
 *          bit1 : 1(active) - battery < 45% capacity, 0(inactive) - battery full capacity
 *          bit2 : 1(active) - operating from battery, 0(inactive) - operating from utility line
 *          bit3 : 1(active) - battery is missing, 0(inactive) - battery is present
 */
int zyupsstate_get(void)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if(boardFd != -1)
    {
        ioctlParms.result = -1;

        rc = ioctl(boardFd, BOARD_IOCTL_GET_UPS_STATE, &ioctlParms);
        close(boardFd);

        if (rc < 0)
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }
        return ioctlParms.result;
    }
    else
    {
        return -1;
    }
}
#endif

/*!
 *  sfp operation
 *  @param[in]  subvalue    - SFP operation options (0-6)
 *              input       - active/inactive
 *
 *  @return       -1        - ioctl failed
 *                 0        - inactive
 *                 1        - active
 *
 *  @note get/set sfp
 */
int zysfpoperation(int subvalue ,int input)
{
    int boardFd = 0;
    int rc = 0;
    BOARD_IOCTL_PARMS ioctlParms;

    if ((subvalue > (ZYGOIOSATAT_MAX_NUM_COMMANDS - 1)) || subvalue < GET_SFP_PRESENT || (input != 0 && input != 1) )
    {
        return -1;
    }

    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
    if(boardFd != -1)
    {
        ioctlParms.result = -1;
        ioctlParms.strLen = subvalue;
        ioctlParms.offset = input;

        rc = ioctl(boardFd, BOARD_IOCTL_SET_GET_SFP, &ioctlParms);
        close(boardFd);

        if (rc < 0)
        {
            zyutil_dbg_printf("ioctl failed\n");
            return -1;
        }

        return ioctlParms.result;
    }
    else
    {
        return -1;
    }
}
#endif
