
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "zcfg_debug.h"

#define MAX_DEBUGMSG_SIZE 500
void zcfgDebug(int logLev, const char *fmt,...)
{
#ifdef ZCFG_LOG_USE_DEBUGCFG
	char zcfgDebugMsgStr[MAX_DEBUGMSG_SIZE] = {0};
	va_list ap;
	va_start(ap, fmt);

	if(myDebugCfgObj){
		if(((myDebugCfgObjLevel & ZCFG_DEBUGCFG_SYSLOG) || (logLev & ZCFG_DEBUGCFG_SYSLOG))
			&& ((myDebugCfgObjLevel & ZCFG_DEBUGCFG_LOGLEVMASK) >= (logLev & ZCFG_DEBUGCFG_LOGLEVMASK))){

			if((logLev & ZCFG_DEBUGCFG_OPTIONMASK) && !((myDebugCfgObjLevel & ZCFG_DEBUGCFG_OPTIONMASK) & logLev))
				goto complete;

			vsnprintf(zcfgDebugMsgStr, MAX_DEBUGMSG_SIZE, fmt, ap);
			syslog((logLev & ZCFG_DEBUGCFG_LOGLEVMASK), "%s", zcfgDebugMsgStr);
		}
		if(((myDebugCfgObjLevel & ZCFG_DEBUGCFG_STDLOG) || (logLev & ZCFG_DEBUGCFG_STDLOG))
			&& ((myDebugCfgObjLevel & ZCFG_DEBUGCFG_LOGLEVMASK) >= (logLev & ZCFG_DEBUGCFG_LOGLEVMASK))){

			if((logLev & ZCFG_DEBUGCFG_OPTIONMASK) && !((myDebugCfgObjLevel & ZCFG_DEBUGCFG_OPTIONMASK) & logLev))
				goto complete;

			vprintf(fmt, ap);
		}
	}else if(ZCFG_LOG_LEVEL >= logLev){
		vsnprintf(zcfgDebugMsgStr, MAX_DEBUGMSG_SIZE, fmt, ap);
		syslog(logLev, "%s", zcfgDebugMsgStr);
	}

complete:
	va_end(ap);
	return;
#endif
}

unsigned int zcfgMsgDebugCfgLevel(void)
{
	printf("%s\n", __FUNCTION__);
	return myDebugCfgObjLevel;
}

void zcfgMsgDebugCfgLevelSet(unsigned int debugCfgLevel)
{
	printf("%s\n", __FUNCTION__);
	myDebugCfgObjLevel = debugCfgLevel;
}

