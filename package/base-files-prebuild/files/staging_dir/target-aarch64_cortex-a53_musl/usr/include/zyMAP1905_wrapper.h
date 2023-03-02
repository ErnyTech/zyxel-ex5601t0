/****************************************************************************
 *
 * Copyright (C) 2013 by ZyXEL
 *
 ****************************************************************************/

/**
 * @file   zyMAP1905_wrapper.h
 * @date   Jan 2019
 * @brief  Wrapper and register function for Broadcom IEEE1905
 */

#ifndef __ZY1905_WRAPPER_H__
#define __ZY1905_WRAPPER_H__

#include <time.h>
#include <stdbool.h>
#include "zyMAP1905_global.h"
#include "zyMAP1905_dataManager.h"
#include "zyMAP1905_message.h"
#define ZY1905_CONFIG  "/tmp/.zy1905_conf"
#define MAP_WTS_BSS_INFO_CFG_FILE "/etc/map/wts_bss_info_config"
#define WLAN_24G_DAT_FILE "/etc/wireless/mediatek/mt7986-ax6000.dbdc.b0.dat"       //WLAN 2.4GHz configuration profile path
#define WLAN_5G_DAT_FILE "/etc/wireless/mediatek/mt7986-ax6000.dbdc.b1.dat" //WLAN 5GHz configuration profile path

#define I5_MAX_VNDR_DATA_BUF 1514
#define IEEE1905_MAC_ADDR_LEN   6

#define ZY_ASSOC_FILE_5G "/tmp/.mtk_assoc_list_5g"
#define ZY_ASSOC_FILE_24G "/tmp/.mtk_assoc_list_24g"

typedef struct {
  unsigned char neighbor_al_mac[6]; /* While Sending/Encoding a Vendor MSG/TLV this is Destination AL_MAC
                                      * While Receiving/Decoding a Vendor MSG/TLV this is Source AL_MAC */
  unsigned char *vendorSpec_msg; /* Vendor data */
  unsigned int vendorSpec_len; /* Vendor data length */
} zy1905VendorData;

/* Vendor Specific Information of each device on the topology */
typedef struct {
  zyDevConfTable    zyDevConf;      /*configurations of IEEE 1905.1 device */
  unsigned short    zyWiFiNbrNum;
  zyWIFINeighbor    *zyWifiNeighbor; /* WIFI Neighbor list */
  SecConf           zySecConf;  /* security configuration */
} zy1905DmDeviceExt;

typedef struct {
  IntfPwrState    zyPwrState;   /**< Power state of this Interface */
} zy1905DmIntfExt;

typedef void *zy1905mtk_getDmData(int opt_type, void *arg, void *output);
typedef RetCode zy1905mtk_writeJsonFile();
typedef int zy1905mtk_sendVenderSpecMsg(zy1905VendorData *msg_data, unsigned short *msgId);
typedef int zy1905mtk_sendVendSpecMcast(zy1905VendorData *msg_data, unsigned short *msgId);
typedef int zy1905mtk_cmdMsgSend(int cmd, void *data, size_t datalen);

typedef struct zy1905_call_bks {
    zy1905mtk_getDmData *getDmData;
    zy1905mtk_writeJsonFile *writeJsonFile;
    zy1905mtk_sendVenderSpecMsg *sendVendSpecMsg;
    zy1905mtk_sendVendSpecMcast *sendVendSpecMcast;
    zy1905mtk_cmdMsgSend *cmdMsgSend;
} zy1905_call_bks_t;

void zyMAP1905_Func_reg(zy1905_call_bks_t *i5_cbs, bool isCmd);

void *zyMAP1905_DmData_get(int opt_type, void *arg, void *output);

RetCode zyMAP1905_JsonDataModel_write();

void zyMAP1905_DeviceDm_init(zy1905DmDeviceExt *zy_DeviceDm);

void zyMAP1905_DeviceDm_update(zy1905DmDeviceExt *zy_DeviceDm);

void *zyMAP1905_DevIntfDm_init(zy1905DmIntfExt* zy_IntfeDm, unsigned short media_type);

void zyMAP1905_WifiChan_get(char* ifname, unsigned char* chan_result);

bool zyMAP1905_CtrlFromNbrCap_get(int option);

void zyMAP1905_VendSpecMsg_handle(void *msg_data, unsigned short msgId);

int zyMAP1905_RebootQuery_send(void *pCmd, int argc, char *argv[]); //2

int zyMAP1905_SyslogDbg_set(void *pCmd, int argc, char *argv[]); //d

RetCode zyMAP1905_VenderSpecMsg_send(unsigned char *macDa, unsigned short *msgId, unsigned char *specData);

RetCode zyMAP1905_VenderSpecMcastMsg_send(unsigned char *specData, unsigned short *msgId);

RetCode zyMAP1905_mdid_update(zyTopology_MdidInfo_t *pTopoMdid);

#endif
