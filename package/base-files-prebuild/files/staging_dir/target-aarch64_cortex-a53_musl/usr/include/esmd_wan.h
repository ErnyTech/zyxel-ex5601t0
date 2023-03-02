#ifndef _ESMD_WAN_H
#define _ESMD_WAN_H

extern zcfgRet_t esmdSendAndGetReplay(zcfgEid_t dstEid, int msg_type, zcfg_offset_t rdmObjId, objIndex_t *objIid, int payloadLen, char *payload, char *reserve, void **replayPayload);
extern int esmdMWANInit( void );
extern int esmdMWANLinkChange( char *buf);
#endif
