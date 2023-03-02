#include <unistd.h>
#include "zcfg_fe_tr98.h"
#include "zcfg_msg.h"
#include "zcfg_eid.h"
#include "ztr69_common.h" 
#include "ztr69cli.h"
#include "zos_api.h"

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

int ch = -1; // for getopt
bool verbose = false;
bool debug = false;
bool help = false;
int _index = 0;

#define ZTR69_COMMON_LOG_LEVEL_MUST 0
#define ZTR69_COMMON_LOG_LEVEL_OUTPUT 1 
#define ZTR69_COMMON_LOG_LEVEL_INFO 3
#define ZTR69_COMMON_LOG_LEVEL_DEBUG 9

int gloglevel = ZTR69_COMMON_LOG_LEVEL_OUTPUT;



#define ZTR69_COMMONLOG(logLev,format, ...) {\
    if(gloglevel >= logLev) {\
	if(debug){ fprintf(stdout,"%s(%s:%d) ", __func__,__FILE__,__LINE__); } \
        fprintf(stdout, format, ##__VA_ARGS__);} \
	}			 

ztr69ParamterList_t Tr181List;

error_message_t error_messages[] = 
{
	{ZTR69_COMMON_OK,"ZTR69_COMMON_OK"},	
	{ZTR69_COMMON_REQUEST_DENIED,"ZTR69_COMMON_REQUEST_DENIED"},
	{ZTR69_COMMON_INTERNAL_ERROR,"ZTR69_COMMON_INTERNAL_ERROR"},
	{ZTR69_COMMON_INVALID_ARGUMENTS,"ZTR69_COMMON_INVALID_ARGUMENTS"},
	{ZTR69_COMMON_RESOURCES_EXCEEDED,"ZTR69_COMMON_RESOURCES_EXCEEDED"},
	{ZTR69_COMMON_INVALID_PARAMETER_NAME,"ZTR69_COMMON_INVALID_PARAMETER_NAME"},
	{ZTR69_COMMON_INVALID_PARAMETER_TYPE,"ZTR69_COMMON_INVALID_PARAMETER_TYPE"},
	{ZTR69_COMMON_INVALID_PARAMETER_VALUE,"ZTR69_COMMON_INVALID_PARAMETER_VALUE"},
	{ZTR69_COMMON_ATTEMPT_TO_SET_NONWRITABLE_PARAMETER,"ZTR69_COMMON_ATTEMPT_TO_SET_NONWRITABLE_PARAMETER"},
	{0xDEADBEEF,"Unknow Error"}
};

void show_parameter_list(ztr69ParamterList_t *pCwmpPList)
{
	int i;
	bool bstringType;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	for(i=0;i<pCwmpPList->List.arrayNum;++i)
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"%s: ",Tr181List.List.parameterValueStruct[i].name);
		if(!strcmp(Tr181List.List.parameterValueStruct[i].xsiType,XSD_STRING) || !strcmp(Tr181List.List.parameterValueStruct[i].xsiType,XSD_DATETIME))
			bstringType = true;
		else
			bstringType = false;
		if(bstringType){
			ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"\"%s\"\n",Tr181List.List.parameterValueStruct[i].value);}
		else{
			ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"%s\n",Tr181List.List.parameterValueStruct[i].value);}
	}
}

char* get_error_message(tr069ERR_e ret)
{
	error_message_t *error_message_p = error_messages;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	while(error_message_p->errorno != 0xDEADBEEF)
	{
		if(ret==error_message_p->errorno)
			break;
		error_message_p++;
	}
	return error_message_p->error_message;
}

void error_print(tr069ERR_e ret, char * input)
{
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"%s:%s\n",get_error_message(ret),input);
	return;
}

void faultBuf_print(ztr69ParamterList_t *pCwmpPList)
{
	int i=0;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	if(pCwmpPList==NULL)
		return;
	error_print(pCwmpPList->faultBuf.faultCode,"");
	for(i=0;i<pCwmpPList->faultBuf.arrayNum;++i)
		error_print(pCwmpPList->faultBuf.setParameterValuesFault[i].faultCode,pCwmpPList->faultBuf.setParameterValuesFault[i].parameterName);
	return;
}

int gethandler(int argc, char **argv)
{
	tr069ERR_e ret = TR069_OK;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	ztr69_common_para_clear(&Tr181List);
	for(_index=0;_index<argc;++_index)
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"argv[%d] = %s\n",_index,argv[_index]);
		ret = ztr69_common_get(argv[_index],&Tr181List);
		if(ret!=TR069_OK)
			error_print(ret,argv[_index]);
		else
			show_parameter_list(&Tr181List);
		ztr69_common_para_clear(&Tr181List);
	}
	return 0;
}
int sethandler(int argc, char **argv)
{
	tr069ERR_e ret = TR069_OK;
	char ori_message_backup[256];
	char *token;
	char *rest;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	ztr69_common_para_clear(&Tr181List);
	for(_index=0;_index<argc;++_index)
	{
		token = NULL;
		rest = argv[_index];
		ZOS_STRNCPY(ori_message_backup,argv[_index],sizeof(ori_message_backup));
		token=strtok_r(rest,"=",&rest);
		if(rest[0]=='\"' && rest[strlen(rest)] == '\"')
		{
			rest[strlen(rest)] = '\0';
			++rest;
		}
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"token = %s\nrest = %s\n",token,rest);
		ret = ztr69_common_para_add(&Tr181List,token,rest);
		if(ret!=TR069_OK)
		{
			error_print(ret,ori_message_backup);
			ztr69_common_para_clear(&Tr181List);
			return 0;
		}
	}
	ret = ztr69_common_set(&Tr181List);
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"ret = %d\n",ret);
	if(ret!=TR069_OK)
	{
		error_print(ret,"");
		faultBuf_print(&Tr181List);
	}
	else
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"Set success.\n");
	}
	ztr69_common_para_clear(&Tr181List);
	return 0;
}

int addhandler(int argc, char **argv)
{
	uint16_t instanceNum = 0;
	tr069ERR_e ret = TR069_OK;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	for(_index=0;_index<argc;++_index)
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"argv[%d] = %s\n",_index,argv[_index]);
		ret = ztr69_common_add(argv[_index],&instanceNum);
		if(ret!=TR069_OK)
		{
			error_print(ret,argv[_index]);
			break;
		}
		if(gloglevel == ZTR69_COMMON_LOG_LEVEL_MUST)
		{
			ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_MUST,"%hu\n",instanceNum);
		}
		else
		{
			ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"%s%hu. add success.\n",argv[_index],instanceNum);
		}
	}
	return 0;
}

int delhandler(int argc, char **argv)
{
	tr069ERR_e ret = TR069_OK;
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	for(_index=0;_index<argc;++_index)
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"argv[%d] = %s\n",_index,argv[_index]);
		ret = ztr69_common_delete(argv[_index]);
		if(ret!=TR069_OK)
		{
			error_print(ret,argv[_index]);
			break;
		}
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"%s delete success.\n",argv[_index]);
	}
	return 0;
}
void show_help()
{
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"Usage: libztr69_common <option> [action] {PATH}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"<option>\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   -v                 show verbose\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   -d                 show debug info(just for develop)\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   -h                 show help\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   -q                 quilt execute\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"[action]\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   get                get datamodel parameter in {PATH}; get {PATH}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   set                set datamodel parameter in {PATH}; set {PATH}={value}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   add                add datamodel object in {PATH}; add {PATH}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT,"   del                del datamodel object in {PATH}; def {PATH}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT," {PATH}\n");
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_OUTPUT," follow the rule of TR-69. please add '.' after the object you want get or add or del\n");
}

static ztr69_common_clihandler_t tr181cliHandler[] = {
	{"get",gethandler},
	{"set",sethandler},
	{"add",addhandler},
	{"del",delhandler},
	{"",NULL}
};

int main(int argc, char *argv[])
{
	char *opt = NULL;
	ztr69_common_clihandler_t *tr181handler;

	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_DEBUG,"\n");
	ztr69_common_init(ZTR69_COMMON_LOG_LEVEL_OUTPUT);
	ztr69_common_para_alloc(&Tr181List);
	zcfgEidInit(ZCFG_EID_CUSTOMER, NULL);
	tr181handler = tr181cliHandler;
	while((ch=getopt(argc, argv,"vdhq"))!= -1)
	{
		ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"ch = %d\n",ch);
		switch(ch){
			case 'v':
				gloglevel = ZTR69_COMMON_LOG_LEVEL_INFO;
				break;
			case 'd':
				debug = 1;
				gloglevel = ZTR69_COMMON_LOG_LEVEL_DEBUG;
				break;
			case 'h':
				help = 1;
				break;
			case 'q':
				gloglevel = ZTR69_COMMON_LOG_LEVEL_MUST;
				break;
			default:
				help = 1;
				break;
		}	 
	}
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"optind = %d\n",optind);
	opt = argv[optind];
	ZTR69_COMMONLOG(ZTR69_COMMON_LOG_LEVEL_INFO,"opt = %s\n",opt);
	if( help || !(argc > 1) || !tr181handler->handler)
		show_help();
	else if(argc > 1)
	{
		while(tr181handler->handler)
		{
			if(!strcmp(opt,tr181handler->Name))
			{
				tr181handler->handler(argc-optind-1,argv+optind+1);
				break;
			}
			tr181handler++;
		}
	}
	ztr69_common_para_free(&Tr181List);

    return 0;
}
