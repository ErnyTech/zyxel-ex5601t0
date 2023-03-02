/*************************************************************************** 
*  Copyright (C) 2008-2020 ZyXEL Communications, Corp.
*  All Rights Reserved.
*
* ZyXEL Confidential; Need to Know only.
* Protected as an unpublished work.
*
* The computer program listings, specifications and documentation
* herein are the property of ZyXEL Communications, Corp. and shall
* not be reproduced, copied, disclosed, or used in whole or in part
* for any reason without the prior express written permission of
* ZyXEL Communications, Corp.
*
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "output_redirect.h"

#define OUTPUT_DEBUG
//#define OUTPUT_DEBUG printf


int output_redirect_cnt = 0;
output_redirect_t output_redirect[5];
char my_stdout[STD_FD_PATH_SIZE];
char my_stderr[STD_FD_PATH_SIZE];
int output_redirect_stdout_fd = -1;
int output_redirect_stderr_fd = -1;

int
get_proc_fd_path(int pid, char *file, char *path,int pathsize)
{
	char buf[128];
	int size;
	OUTPUT_DEBUG("%s: %d pid=%d\n",__FUNCTION__,__LINE__,pid);
	snprintf(buf,sizeof(buf),"/proc/%d/fd/%s",pid,file);
	size = readlink(buf,path,pathsize);
	path[size]='\0';
	OUTPUT_DEBUG("%s: buf=%s, path=%s, size=%d, %d\n",__FUNCTION__,buf,path,size,pathsize);
	return 0;
}

pid_t
get_pid_by_name(char *name )
{
	FILE *fp;
	char cmd[128];
	char buf[128];
	char *cp, *cphead;
	pid_t pid = -1;

	snprintf(cmd,sizeof(cmd),"ps | grep \" %s\"",name);
	if ( (fp = popen(cmd,"r") ) != NULL ) {
		cp = fgets(buf,128,fp);
		if ( cp == NULL ) {
			goto out;
		}
		while ( ( *cp == ' ' ) || ( *cp == '\t' ) ) cp ++;
		cphead = cp;
		while ( ( *cp != ' ' ) && ( *cp != '\t' ) ) cp ++;
		cp = '\0';
		
		pid = atoi(cphead);
		pclose(fp);
		OUTPUT_DEBUG("%s: pid = %d\n",__FUNCTION__,(int)pid);
	}
out:
	return pid;
} /* get_pid_by_name */

int 
getfd( int pid, char *standout, char *standerr )
{
	int ret = 0;
	get_proc_fd_path( pid, "1", standout,STD_FD_PATH_SIZE);
	get_proc_fd_path( pid, "2", standerr,STD_FD_PATH_SIZE);
	return ret;
}

int 
set_output_redirect ( char *ouput, int *redirect_fd, int oldfd )
{
	*redirect_fd = open (ouput,O_RDWR);
	if ( *redirect_fd >= 0 ) {
		dup2(*redirect_fd, oldfd);
		close(*redirect_fd);
		OUTPUT_DEBUG("%s: redirect = %d\n",__FUNCTION__, *redirect_fd);
		return 0;
	}
	else {
		OUTPUT_DEBUG("%s: fail = %d\n",__FUNCTION__, *redirect_fd);
	}
	/* fail */
	return -1;
} /* set_output_redirect */


int 
set_output_redirect_all ( int pid, char *standout, char *standerr)
{
	int ret = 0;

	if ( ( output_redirect_stdout_fd != -1 ) ||
		( output_redirect_stderr_fd != -1 ) ) {
			ret = -1;
		goto out;
	}

	if ( set_output_redirect ( standout, &output_redirect_stdout_fd, 1) != 0 ) {
		ret = -3;
		goto out;
	}
	if ( set_output_redirect ( standerr, &output_redirect_stderr_fd, 2) != 0 ) {
		ret = -4;
		goto out;
	}
out:
	if ( ret ) {
		OUTPUT_DEBUG("%s: ret = %d\n", __FUNCTION__,ret);
	}
	return ret;
}

int 
restore_output_redirect_all ( void  )
{
	int ret = 0;
	
	if ( set_output_redirect ( my_stdout, &output_redirect_stdout_fd, 1) != 0 ) {
		ret = -1;
		/* force restore */
	}
	if ( set_output_redirect ( my_stderr, &output_redirect_stderr_fd, 2) != 0 ) {
		ret -= 10;
		/* force restore */
	}
	output_redirect_stdout_fd = -1;
	output_redirect_stderr_fd = -1;

	return ret;
}


void
init_output_redirect(void)
{
	getfd( getpid(), my_stdout, my_stderr );
	memset(output_redirect, 0 , sizeof(output_redirect));
}

