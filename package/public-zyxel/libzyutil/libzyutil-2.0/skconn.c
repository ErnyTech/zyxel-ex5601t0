
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#include "zcfg_common.h"
#include "zcfg_debug.h"
#include "zyutil.h"
#ifdef ZYXEL_IPV6_UPLOAD_DIAG_ENABLE
#include "errno.h"
#endif

#define TIME_STAMP_DATA_SIZE  32


unsigned int zyUtilComputeMicroInterval(struct timeval *sTm, struct timeval *nTm)
{
   time_t sec = 0;
   suseconds_t ms;
   long long msec = 0;
   unsigned int max;

   if(!nTm || !sTm) return 0;
   if(sTm->tv_sec > nTm->tv_sec) {
      return 0;
   }

   sec = nTm->tv_sec - sTm->tv_sec;
   if(sec == 0) {
      if(sTm->tv_usec > nTm->tv_usec) {
         return 0;
      }
      msec = nTm->tv_usec - sTm->tv_usec;
   }else {
      msec = sec * 1000000;
      memset(&ms, 0xff, sizeof(ms));
      msec += ms-sTm->tv_usec;
      msec += nTm->tv_usec;
   }
   memset(&max, 0xff, sizeof(max));

   if(msec > (long long)max) {
      msec = msec - max;
      memcpy(sTm, nTm, sizeof(struct timeval));
   }

   return (unsigned int)msec;

}

void zyUtilMakeTimeStamp(struct timeval *tv, char *timeStamp, unsigned int timeStampSz)
{
   struct tm *localtm = NULL;
   unsigned int outputTmSpLen = 0;
   char tmsp[TIME_STAMP_DATA_SIZE+10] = {0};
      
   if(!tv) return;

   //year-mon-day,time,usec
   if(!timeStampSz) return;

   localtm = localtime((const time_t *)&tv->tv_sec);
   zos_snprintf(tmsp, sizeof(tmsp), "%d-%02d-%02dT%02d:%02d:%02d.%ld", (localtm->tm_year+1900), (localtm->tm_mon+1),
       localtm->tm_mday, localtm->tm_hour, localtm->tm_min, localtm->tm_sec, (long)tv->tv_usec); 
   outputTmSpLen = ((timeStampSz-1) < strlen(tmsp)) ? (timeStampSz-1) : strlen(tmsp);
   strncpy(timeStamp, tmsp, outputTmSpLen);
   timeStamp[outputTmSpLen] = '\0';
}

/*
  while domain is AF_UNIX, 'addr' is path name and 'port' could ignore
  'skconn' must not assign NULL
*/
int zyUtilTcpSvrInit(const int domain, const char *addr, const int port, skconn_t *skconn)
{
   int st = 0;
   int len, rt = 0;

   printf("%s\n", __FUNCTION__);

   if(!addr || !skconn) {
      return -1;
   }

   if ((st = socket(domain, SOCK_STREAM, 0)) < 0) {
      zcfgLog(ZCFG_LOG_INFO, "%s: create message socket error \n", __FUNCTION__);
      return 0;
   }

   memset(skconn, 0, sizeof(skconn_t));
   skconn->domain = domain;
   if(domain == AF_UNIX) {
      struct sockaddr_un svrAddr;

      memset(&svrAddr, 0, sizeof(struct sockaddr_un));
      svrAddr.sun_family = AF_UNIX;
      if(strlen(addr) > sizeof(svrAddr.sun_path)) {
         close(st);
         return -1;
      }
      ZOS_STRNCPY(svrAddr.sun_path, addr, sizeof(svrAddr.sun_path));
      unlink(addr);
      len = sizeof(svrAddr.sun_family) + strlen(svrAddr.sun_path);
      rt = bind(st, (struct sockaddr*)&svrAddr, len);
   }else if(domain == AF_INET) {
      struct sockaddr_in svrSkAddr;
      struct in_addr addrn;

      memset(&svrSkAddr, 0, sizeof(struct sockaddr_in));
      svrSkAddr.sin_family = AF_INET;
      inet_aton(addr, &addrn);
      svrSkAddr.sin_addr.s_addr = skconn->ip = addrn.s_addr;
      skconn->port = port;
      svrSkAddr.sin_port = htons(port);
      len = sizeof(struct sockaddr);
      rt = bind(st, (struct sockaddr*)&svrSkAddr, len);
   }else {
      close(st);
      return -1;
   }
   skconn->st = st;
   skconn->type = SOCK_STREAM;

   if (rt < 0) {
      zcfgLog(ZCFG_LOG_INFO, "bind message socket error !!");
      close(st);
      return -1;
   }

   if (listen(st, TCP_BACKLOG) < 0) {
      zcfgLog(ZCFG_LOG_INFO, "listen message socket error !!");
      close(st);
      return -1;
   }

   return 1;
}

int zyUtilTcpConnInit(const int domain, const char *addr, const int port, skconn_t *skconn)
{
   int st;
   int len, rt = 0;

   printf("%s\n", __FUNCTION__);

   if(!addr || !skconn) {
      return -1;
   }

   if ((st = socket(domain, SOCK_STREAM, 0)) < 0) {
      zcfgLog(ZCFG_LOG_INFO, "%s: create message socket error \n", __FUNCTION__);
      return 0;
   }

   memset(skconn, 0, sizeof(skconn_t));
   skconn->domain = domain;
   if(domain == AF_UNIX) {
      struct sockaddr_un svrAddr;

      memset(&svrAddr, 0, sizeof(struct sockaddr_un));
      svrAddr.sun_family = AF_UNIX;
      if(strlen(addr) > sizeof(svrAddr.sun_path)) {
         close(st);
         return -1;
      }
      ZOS_STRNCPY(svrAddr.sun_path, addr, sizeof(svrAddr.sun_path));
      len = sizeof(svrAddr.sun_family) + strlen(svrAddr.sun_path);

      rt = connect(st, (struct sockaddr*)&svrAddr, len);
   }else if(domain == AF_INET) {
      struct sockaddr_in svrSkAddr;
      struct in_addr addrn;

      memset(&svrSkAddr, 0, sizeof(struct sockaddr_in));
      svrSkAddr.sin_family = AF_INET;
      inet_aton(addr, &addrn);
      svrSkAddr.sin_addr.s_addr = skconn->ip = addrn.s_addr;
      skconn->port = port;
      svrSkAddr.sin_port = htons(port);
      len = sizeof(struct sockaddr);

      rt = connect(st, (struct sockaddr*)&svrSkAddr, len);
#ifdef ZYXEL_IPV6_UPLOAD_DIAG_ENABLE
   }else if(domain == AF_INET6) { /* For upload diagnostic using IPv6, Bug #37644, Ian */
      struct sockaddr_in6 svrSkAddr;

      memset(&svrSkAddr, 0, sizeof(struct sockaddr_in6));
      svrSkAddr.sin6_family = AF_INET6;
      inet_pton(AF_INET6, addr, &svrSkAddr.sin6_addr);
      skconn->port = port;
      svrSkAddr.sin6_port = htons(port);
      len = sizeof(struct sockaddr_in6);

      rt = connect(st, (struct sockaddr*)&svrSkAddr, len);
      if(rt<0)
         printf("error: %s\n", strerror(errno));
#endif
   }else {
      close(st);
      return -1;
   }
   skconn->st = st;
   skconn->type = SOCK_STREAM;

   if(rt < 0)
   {
      zcfgLog(ZCFG_LOG_INFO, "connect failed!!");
      close(st);
      return -1;
   }

   return 1;
}

/*
  while domain is AF_UNIX, 'addr' is path name and 'port' could ignore
  'skconn' must not assign NULL
*/
int zyUtilUdpSvrInit(const int domain, const char *addr, const int port, skconn_t *skconn)
{
   int st = 0;
   int len, rt = 0;

   printf("%s\n", __FUNCTION__);

   if(!addr || !skconn) {
      return -1;
   }

   if ((st = socket(domain, SOCK_DGRAM, 0)) < 0) {
      zcfgLog(ZCFG_LOG_INFO, "%s: create message socket error \n", __FUNCTION__);
      return 0;
   }

   memset(skconn, 0, sizeof(skconn_t));
   skconn->domain = domain;
   if(domain == AF_UNIX) {
      struct sockaddr_un svrAddr;

      memset(&svrAddr, 0, sizeof(struct sockaddr_un));
      svrAddr.sun_family = AF_UNIX;
      if(strlen(addr) > sizeof(svrAddr.sun_path)) {
         close(st);
         return -1;
      }
      ZOS_STRNCPY(svrAddr.sun_path, addr, sizeof(svrAddr.sun_path));
      unlink(addr);
      len = sizeof(svrAddr.sun_family) + strlen(svrAddr.sun_path);
      rt = bind(st, (struct sockaddr*)&svrAddr, len);
   }else if(domain == AF_INET) {
      struct sockaddr_in svrSkAddr;
      struct in_addr addrn;

      memset(&svrSkAddr, 0, sizeof(struct sockaddr_in));
      svrSkAddr.sin_family = AF_INET;
      inet_aton(addr, &addrn);
      svrSkAddr.sin_addr.s_addr = skconn->ip = addrn.s_addr;
      skconn->port = port;
      svrSkAddr.sin_port = htons(port);
      len = sizeof(struct sockaddr);
      rt = bind(st, (struct sockaddr*)&svrSkAddr, len);
   }else {
      close(st);
      return -1;
   }
   skconn->st = st;
   skconn->type = SOCK_DGRAM;

   if (rt < 0) {
      zcfgLog(ZCFG_LOG_INFO, "bind message socket error !!");
      close(st);
      return -1;
   }

   return 1;
}

int zyUtilUdpConnInit(const int domain, const char *addr, const int port, skconn_t *skconn)
{
   int st;
   int len, rt = 0;

   printf("%s\n", __FUNCTION__);

   if(!addr || !skconn) {
      return -1;
   }

   if ((st = socket(domain, SOCK_DGRAM, 0)) < 0) {
      zcfgLog(ZCFG_LOG_INFO, "%s: create message socket error \n", __FUNCTION__);
      return 0;
   }

   memset(skconn, 0, sizeof(skconn_t));
   skconn->domain = domain;
   if(domain == AF_UNIX) {
      struct sockaddr_un svrAddr;

      memset(&svrAddr, 0, sizeof(struct sockaddr_un));
      svrAddr.sun_family = AF_UNIX;
      if(strlen(addr) > sizeof(svrAddr.sun_path)) {
         close(st);
         return -1;
      }
      ZOS_STRNCPY(svrAddr.sun_path, addr, sizeof(svrAddr.sun_path));
      len = sizeof(svrAddr.sun_family) + strlen(svrAddr.sun_path);

      rt = connect(st, (struct sockaddr*)&svrAddr, len);
   }else if(domain == AF_INET) {
      struct sockaddr_in svrSkAddr;
      struct in_addr addrn;

      memset(&svrSkAddr, 0, sizeof(struct sockaddr_in));
      svrSkAddr.sin_family = AF_INET;
      inet_aton(addr, &addrn);
      svrSkAddr.sin_addr.s_addr = skconn->ip = addrn.s_addr;
      skconn->port = port;
      svrSkAddr.sin_port = htons(port);
      len = sizeof(struct sockaddr);

      rt = connect(st, (struct sockaddr*)&svrSkAddr, len);
   }else {
      close(st);
      return -1;
   }
   skconn->st = st;
   skconn->type = SOCK_DGRAM;

   if(rt < 0)
   {
      zcfgLog(ZCFG_LOG_INFO, "connect failed!!");
      close(st);
      return -1;
   }

   return 1;
}
