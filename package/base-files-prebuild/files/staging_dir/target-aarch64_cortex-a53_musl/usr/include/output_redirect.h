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
#ifndef _OUTPUT_REDIRECT_H
#define _OUTPUT_REDIRECT_H

#define MAX_OUTPUT_REDIRECT 5
#define STD_FD_PATH_SIZE 128

typedef struct output_redirect_s { 
	int pid; 
	char stdout[STD_FD_PATH_SIZE]; 
	char stderr[STD_FD_PATH_SIZE]; 
} output_redirect_t;

extern char my_stdout[STD_FD_PATH_SIZE];
extern char my_stderr[STD_FD_PATH_SIZE];

extern pid_t get_pid_by_name(char *name );
extern int getfd( int pid, char *standout, char *standerr );
extern int set_output_redirect_all ( int pid, char *standout, char *standerr);
extern int restore_output_redirect_all ( void  );
extern void init_output_redirect(void);
#endif
