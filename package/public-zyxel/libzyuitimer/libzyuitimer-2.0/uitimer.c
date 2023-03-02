/**
 *  @brief		user interface of timer.
 *  @details	user interface of timer.
 *  @author
	- unknown(porting from ZyNOS)
 *  @copyright
.


 *  Copyright (C) 2013 ZyXEL Communications, Corp.
 *  All Rights Reserved.
 *
 *  ZyXEL Confidential;
 *  Protected as an unpublished work, treated as confidential,
 *  and hold in trust and in strict confidence by receiving party.
 *  Only the employees who need to know such ZyXEL confidential information
 *  to carry out the purpose granted under NDA are allowed to access.
 *
 *  The computer program listings, specifications and documentation
 *  herein are the property of ZyXEL Communications, Corp. and shall
 *  not be reproduced, copied, disclosed, or used in whole or in part
 *  for any reason without the prior express written permission of
 *  ZyXEL Communications, Corp.
 *  @file				uitimer.c
 */

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include "timer_list.h"
#include "uitimer.h"
#include <sys/prctl.h> //Michael.20130508.001: Add
#include <errno.h> // for ESRCH

#include "zos_api.h"
#include "zlog_api.h"

#define UPTIME_FILE "/proc/uptime"
#define UI_TIMER_TASK_NAME "libzyuitimer"

#ifndef BOOL
typedef unsigned char BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

void *UITimerProc(void *arg);
void* freeTime( void* pData );

pthread_t timer_thread = 0;
pthread_mutex_t timerMutex;
pthread_mutex_t saveConfigTimerMutex;
struct timer_list time_list;
struct timer_list periodic_time_list;
struct timer_list temp_list;
BOOL timer_thread_running = FALSE;


void UITimer_Init(void){
#if 1/*ZyXEL, rt_priority_Eason*/
	pthread_attr_t attributes;
	struct sched_param schedparams;
#endif

	if(!timer_thread_running){
		timer_list_init(&time_list);
		timer_list_init(&periodic_time_list);
		pthread_mutex_init(&timerMutex, NULL);
		pthread_mutex_init(&saveConfigTimerMutex, NULL);
		timer_thread_running = TRUE;
#if 1 /*ZyXEL, rt_priority_Eason*/
		pthread_attr_init( &attributes );
		pthread_attr_setschedpolicy( &attributes, SCHED_RR );
		schedparams.sched_priority = 75;
		pthread_attr_setschedparam( &attributes, &schedparams );
		pthread_create( (pthread_t*)&timer_thread, &attributes, (void*)UITimerProc, 0 );
#else
		pthread_create(&timer_thread, NULL, UITimerProc, 0);
#endif
	}
	return;
}

void UITimer_Destory(void){
	if(timer_thread_running){
		UIDelAllTimer();
		timer_thread_running = FALSE;
		pthread_mutex_unlock(&timerMutex);
		if (pthread_kill(timer_thread, 0) == ESRCH)
		{
			printf("the specified thread did not exists or already quit\n");
			exit(EXIT_FAILURE);
		}
		//pthread_cancel(timer_thread);
		pthread_join(timer_thread, NULL);
		timer_thread = 0;
	}
	return;
}

void *UITimerProc(void *arg)
{

	timeval_t        timer_val;
	CMTimer_t               *uitimer=NULL;
	timer_list_iterator_t   iter;

	prctl(PR_SET_NAME, UI_TIMER_TASK_NAME, 0, 0, 0);
	ZLOG_INFO("thread starts");
	//printf("%s(): The UI Timer System (thread task: libzyuitimer, pid=%d) is running...\n", __FUNCTION__, getpid());

	// for pthread_cancel attribute to terminal the thread
	//if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
	//	printf("Thread pthread_setcancelstate failed");
	//	exit(EXIT_FAILURE);
	//}
	//if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) != 0) {
	//	printf("Thread pthread_setcanceltype failed");
	//	exit(EXIT_FAILURE);
	//}

#if 0
	while (timer_thread_running)
	{
		//gettimeofday(&timer_val, NULL);
		getSystemUptime(&timer_val);
		memset(&iter,0,sizeof(timer_list_iterator_t));
		pthread_mutex_lock(&timerMutex);
		uitimer =(CMTimer_t*) timer_list_get_first( &time_list, &iter );
		pthread_mutex_unlock(&timerMutex);
		while(uitimer && timer_thread_running){
			if( (uitimer->timeout.tv_sec>timer_val.tv_sec) ||
				((uitimer->timeout.tv_sec == timer_val.tv_sec) && (uitimer->timeout.tv_usec > timer_val.tv_usec))){
				pthread_mutex_lock(&timerMutex);
				uitimer = (CMTimer_t*) timer_list_get_next( &iter );
				pthread_mutex_unlock(&timerMutex);
				continue;
			}
			pthread_mutex_lock(&timerMutex);
			timer_list_iterator_remove(&iter);
			pthread_mutex_unlock(&timerMutex);
			if(uitimer->fn) {
				uitimer->fn(uitimer->TimerID, uitimer->data);
			}
			ZOS_FREE(uitimer);
			uitimer =NULL;
			memset(&iter,0,sizeof(timer_list_iterator_t));
			pthread_mutex_lock(&timerMutex);
			uitimer =(CMTimer_t*) timer_list_get_first( &time_list, &iter );
			pthread_mutex_unlock(&timerMutex);
		}

		memset(&iter,0,sizeof(timer_list_iterator_t));
		pthread_mutex_lock(&timerMutex);
		uitimer =(CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
		pthread_mutex_unlock(&timerMutex);
		while(uitimer && timer_thread_running){
			if( 	(uitimer->timeout.tv_sec>timer_val.tv_sec) ||
			((uitimer->timeout.tv_sec == timer_val.tv_sec) &&(uitimer->timeout.tv_usec > timer_val.tv_usec))){
				pthread_mutex_lock(&timerMutex);
				uitimer =(CMTimer_t*) timer_list_get_next( &iter );
				pthread_mutex_unlock(&timerMutex);
				continue;
			}
			if(uitimer->fn) {
				uitimer->fn(uitimer->TimerID, uitimer->data);
			}
			memset(&(uitimer->timeout), 0, sizeof(timeval_t));
			memcpy(&(uitimer->timeout), &timer_val, sizeof(timeval_t));
			uitimer->timeout.tv_sec  +=  uitimer->TimerLen/1000;
			uitimer->timeout.tv_usec  +=  (uitimer->TimerLen%1000)*1000;
			pthread_mutex_lock(&timerMutex);
			uitimer =(CMTimer_t*) timer_list_get_next( &iter );
			pthread_mutex_unlock(&timerMutex);
		}

		timer_val.tv_sec = 0;
		timer_val.tv_usec = IDLE_TIME;
		select(0, NULL, NULL, NULL, (struct timeval *)&timer_val);
	}

#else
	proc_element_t *temp_element = NULL;
	timer_list_init(&temp_list);

	while(timer_thread_running){
		//**********************************
		//*** For One-shot/Non-Periodic Timer.
		//**********************************
		//gettimeofday(&timer_val, NULL);
		getSystemUptime(&timer_val);
		memset(&iter, 0, sizeof(timer_list_iterator_t));

		pthread_mutex_lock(&timerMutex);
		uitimer = (CMTimer_t*) timer_list_get_first( &time_list, &iter );
		while(uitimer && timer_thread_running){
			if(	(uitimer->timeout.tv_sec > timer_val.tv_sec) ||
				((uitimer->timeout.tv_sec == timer_val.tv_sec) &&(uitimer->timeout.tv_usec > timer_val.tv_usec))	){
				uitimer = (CMTimer_t*) timer_list_get_next( &iter );

				continue;
			}

			timer_list_iterator_remove(&iter);

			temp_element = (proc_element_t*)malloc(sizeof(proc_element_t));
			temp_element->element = (void*)malloc(sizeof(CMTimer_t));
			memcpy(temp_element->element, uitimer, sizeof(CMTimer_t));
			temp_element->type = ONE_SHOT_TIMER;
			//Timeout timer record to temp List.
			timer_list_add(&temp_list, temp_element, 0);
			temp_element = NULL;
			//Moving timeout function execution after checking One-shot/Non-Periodic Timer list.

			ZOS_FREE(uitimer);
			uitimer = NULL;
			memset(&iter, 0, sizeof(timer_list_iterator_t));
			uitimer = (CMTimer_t*) timer_list_get_first( &time_list, &iter );
		}
		//pthread_mutex_unlock(&timerMutex);

		//**********************************
		//*** For Periodic Timer.
		//**********************************
		memset(&iter, 0, sizeof(timer_list_iterator_t));

		//pthread_mutex_lock(&timerMutex);
		uitimer = (CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
		while(uitimer && timer_thread_running){
			if(	(uitimer->timeout.tv_sec > timer_val.tv_sec) ||
				((uitimer->timeout.tv_sec == timer_val.tv_sec) &&(uitimer->timeout.tv_usec > timer_val.tv_usec))	){
				uitimer = (CMTimer_t*) timer_list_get_next( &iter );

				continue;
			}

			temp_element = (proc_element_t*)malloc(sizeof(proc_element_t));

			pthread_mutex_lock(&uitimer->timerMutex);
			uitimer->status = (uitimer->status != DEL_STATUS) ? PROC_STATUS : uitimer->status;
			pthread_mutex_unlock(&uitimer->timerMutex);
			temp_element->element = (void*)uitimer;
			temp_element->type = PERIODIC_TIMER;
			//memcpy(&(temp_element->iter), &iter, sizeof(timer_list_iterator_t));
			//printf("%s()  Add to Tmp TimerId(%d) status(%d).. sec:%d usec:%d\n",__FUNCTION__, uitimer->TimerID, uitimer->status, uitimer->timeout.tv_sec, uitimer->timeout.tv_usec);
			//printf("Parse periodic_time_list to add to temp_list type:%d id:%d time:%d\n", temp_element->type, uitimer->TimerID, uitimer->TimerLen);
			//Timeout timer record to temp List.
			timer_list_add(&temp_list, temp_element, 0);
			temp_element = NULL;
			uitimer = (CMTimer_t*) timer_list_get_next( &iter );
		}
		pthread_mutex_unlock(&timerMutex);

		memset(&iter, 0, sizeof(timer_list_iterator_t));

		temp_element = (proc_element_t*)timer_list_get_first( &temp_list, &iter );
		while(temp_element && timer_thread_running){
			uitimer = (CMTimer_t*)temp_element->element;

			//The timeout function is executed

			/*Amber.20200923: Add mutex lock for avoid another thread call UIDelPeriodicTimer*() and change the uitimer->status to DEL_STATUS
			 *                when UITimerProc() reset the uitimer parameter value and just unlock the uiimer mutex.
			 *                In this scenario, UITimerProc() will free uitimer structure due to the uitimer status is in DEL_STATUS but the uitimer pointer still not NULL in 'periodic_time_list'.
			 */
			if(temp_element->type == PERIODIC_TIMER){
				TMRFUNC ptimerfn = 0;
				pthread_mutex_lock(&timerMutex);
				pthread_mutex_lock(&uitimer->timerMutex);
				//printf("%s()  PERIODIC_TIMER\tTimerId(%d) status(%d).. \n",__FUNCTION__, uitimer->TimerID, uitimer->status);
				if(uitimer->status == PROC_STATUS){
					//Periodic timer keep going
					ptimerfn = uitimer->fn;
					memset(&(uitimer->timeout), 0, sizeof(timeval_t));
					memcpy(&(uitimer->timeout), &timer_val, sizeof(timeval_t));
					uitimer->timeout.tv_sec  +=  uitimer->TimerLen / 1000;
					uitimer->timeout.tv_usec  +=  (uitimer->TimerLen % 1000) * 1000;
					uitimer->status = IDLE_STATUS;
				}
				else//(uitimer->status == DEL_STATUS)
				{
					//Delete periodic timer
					//timer_list_iterator_remove(&(temp_element->iter));
					//printf("%s()  Delete TimerID(%d) .. \n", __FUNCTION__, uitimer->TimerID);
				}

				pthread_mutex_unlock(&uitimer->timerMutex);
				pthread_mutex_unlock(&timerMutex);

				if(ptimerfn) {
					ptimerfn(uitimer->TimerID, uitimer->data);
				}

				if(uitimer->status == DEL_STATUS){
					ZOS_FREE(uitimer);
				}
			}
			else{
				if(uitimer->fn) {
					uitimer->fn(uitimer->TimerID, uitimer->data);
				}

				ZOS_FREE(uitimer);
			}
			ZOS_FREE(temp_element);

			timer_list_iterator_remove(&iter);

			memset(&iter, 0, sizeof(timer_list_iterator_t));
			temp_element = (proc_element_t*)timer_list_get_first( &temp_list, &iter );
		}
		
		timer_val.tv_sec = 0;
		timer_val.tv_usec = IDLE_TIME; /* 100 ms idle */
		select(0, NULL, NULL, NULL, (struct timeval *)&timer_val);
	}
#endif

	pthread_exit(NULL);
	return NULL;
}

/*change "timerLen" type to uint32 for longer timeout, ZyXEL*/
int UIAddTimer(int id,uint32_t timerLen,TMRFUNC timefn,void *privdata)
{
	timer_list_iterator_t iter;
	CMTimer_t *uitimer,*intimer;
	int res=-1;

	uitimer = ZOS_MALLOC(sizeof(CMTimer_t));
	if (uitimer==NULL)
	{
		return res;
	}
	memset(uitimer, 0, sizeof(CMTimer_t));

//	gettimeofday(&uitimer->timeout, NULL);
	getSystemUptime(&uitimer->timeout);
	uitimer->TimerID = id;
	uitimer->TimerLen = timerLen;
#if 1  /* Solve inaccurate timer problem. Root cause: no consider that milisecond carry to second. Wayne fixed at 20090703. */
	uitimer->timeout.tv_usec  +=  (timerLen % 1000) * 1000;
	uitimer->timeout.tv_sec  +=  timerLen / 1000 + uitimer->timeout.tv_usec / 1000000;
	uitimer->timeout.tv_usec  =  uitimer->timeout.tv_usec % 1000000;
#else
	uitimer->timeout.tv_sec  +=  timerLen/1000;
	uitimer->timeout.tv_usec  +=  (timerLen%1000)*1000;
#endif
	uitimer->data = (void*)privdata;
	uitimer->fn = timefn;

	pthread_mutex_lock(&timerMutex);
	intimer =(CMTimer_t*) timer_list_get_first( &time_list, &iter );
	while(intimer){
		if(intimer->TimerID==id){
			intimer->TimerID=id;
			intimer->timeout.tv_sec=uitimer->timeout.tv_sec;
			intimer->timeout.tv_usec=uitimer->timeout.tv_usec;
			intimer->data= (void*)privdata;
			intimer->fn=timefn;
			pthread_mutex_unlock(&timerMutex);
			return 0;
		}

		intimer=timer_list_get_next( &iter );
	}

	res=timer_list_add(& time_list, uitimer, iter.pos )>0 ? 0: -1;
	pthread_mutex_unlock(&timerMutex);

	return res;
}

int UIAddPeriodicTimer(int id,int timerLen,TMRFUNC timefn,void *privdata)
{
	timer_list_iterator_t   iter;
	CMTimer_t               *uitimer,
	                        *intimer;
	int                     res=-1;

	uitimer = ZOS_MALLOC(sizeof(CMTimer_t));
	if (uitimer == NULL)
	{
		return res;
	}
	memset(uitimer, 0, sizeof(CMTimer_t));

	//gettimeofday(&uitimer->timeout, NULL);
	getSystemUptime(&uitimer->timeout);
	uitimer->TimerID = id;
	uitimer->TimerLen = timerLen;
#if 1  /* Solve inaccurate timer problem. Root cause: no consider that milisecond carry to second. Wayne fixed at 20090703. */
	uitimer->timeout.tv_usec  +=  (timerLen % 1000) * 1000;
	uitimer->timeout.tv_sec  +=  timerLen / 1000 + uitimer->timeout.tv_usec / 1000000;
	uitimer->timeout.tv_usec  =  uitimer->timeout.tv_usec % 1000000;
#else
	uitimer->timeout.tv_sec  +=  timerLen/1000;
	uitimer->timeout.tv_usec  +=  (timerLen%1000)*1000;
#endif
	uitimer->data = (void*)privdata;
	uitimer->status = IDLE_STATUS;
	uitimer->fn = timefn;

	pthread_mutex_init(&uitimer->timerMutex, NULL);

	pthread_mutex_lock(&timerMutex);
	intimer =(CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
	while(intimer){
		if(intimer->TimerID==id){
			intimer->TimerID=id;
			intimer->timeout.tv_sec=uitimer->timeout.tv_sec;
			intimer->timeout.tv_usec=uitimer->timeout.tv_usec;
			intimer->data= (void*)privdata;
			intimer->status = uitimer->status;
			intimer->fn=timefn;
			pthread_mutex_unlock(&timerMutex);
			return 0;
		}

		intimer=timer_list_get_next( &iter );
	}
	res=timer_list_add(& periodic_time_list, uitimer, iter.pos )>0 ? 0: -1;
	pthread_mutex_unlock(&timerMutex);

	return res;
}

int UIDelTimer(int id, void **dataptr){
	int ret = TIMER_STOP_SUCCESS;
	CMTimer_t *uitimer=NULL;
	timer_list_iterator_t iter;
	bool found = FALSE;

	if (id <= 0) {
		return TIMER_STOP_NOT_FOUND;
	}

	pthread_mutex_lock(&timerMutex);
	uitimer =(CMTimer_t*) timer_list_get_first( &time_list, &iter );
	while(uitimer){
		if(uitimer->TimerID !=id){
			uitimer =(CMTimer_t*) timer_list_get_next( &iter );

			if((NULL == uitimer) && !found) {
				ret = TIMER_STOP_NOT_FOUND;
			}
			continue;
		}
		found = id;
		if(NULL != dataptr){
			*dataptr = uitimer->data;
		}
		timer_list_iterator_remove(&iter);
		ZOS_FREE(uitimer);
		uitimer = NULL;
		memset(&iter,0,sizeof(timer_list_iterator_t));
		uitimer =(CMTimer_t*) timer_list_get_first( &time_list, &iter );
	}
	pthread_mutex_unlock(&timerMutex);

	return ret;
}

// Add from ZyIMS
int UIDelTimer_retDataPtr( int id, void **dataPtr ){
	int ret = TIMER_STOP_SUCCESS;
	CMTimer_t *uitimer = NULL;
	timer_list_iterator_t iter;
	bool isEverFound = FALSE;

	if (id <= 0) {
		ret = TIMER_STOP_NOT_FOUND;
		return ret;
	}

	if (NULL == dataPtr){
		ret = TIMER_STOP_FAIL;
		return ret;
	}

	pthread_mutex_lock(&timerMutex);
	uitimer = (CMTimer_t*) timer_list_get_first( &time_list, &iter );
	while(uitimer){
		if(uitimer->TimerID != id){

			uitimer = (CMTimer_t*) timer_list_get_next( &iter );

			if((NULL == uitimer) && !isEverFound) {
				ret = TIMER_STOP_NOT_FOUND;
			}
			continue;
		}
		isEverFound = TRUE;

		timer_list_iterator_remove(&iter);

		*dataPtr = uitimer->data;

		ZOS_FREE(uitimer);
		uitimer = NULL;
		memset(&iter, 0, sizeof(timer_list_iterator_t));
		uitimer = (CMTimer_t*) timer_list_get_first( &time_list, &iter );
	}
	pthread_mutex_unlock(&timerMutex);

	return ret; //Ret: 0:Success, (-1):Fail/Error, 1:Timer-ID is NOT in the Timer List.
}

int UIDelPeriodicTimer(int id, void **dataptr){
	int ret = TIMER_STOP_SUCCESS;
	CMTimer_t *uitimer=NULL;
	timer_list_iterator_t iter;
	bool found = FALSE;

	if (id <= 0) {
		return TIMER_STOP_NOT_FOUND;
	}
	//printf("%s(): delete timer id(%d).\n", __FUNCTION__, id);

	pthread_mutex_lock(&timerMutex);
	uitimer =(CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
	while(uitimer){
		if(uitimer->TimerID !=id){

			uitimer =(CMTimer_t*) timer_list_get_next( &iter );

			if((NULL == uitimer) && !found) {
				ret = TIMER_STOP_NOT_FOUND;
			}
			continue;
		}
		found = TRUE;

		if(NULL != dataptr){
			*dataptr = uitimer->data;
		}

		if(uitimer->status == PROC_STATUS){
			//Marking the timer has been deleted
			pthread_mutex_lock(&uitimer->timerMutex);
			uitimer->status = DEL_STATUS;
			pthread_mutex_unlock(&uitimer->timerMutex);
			timer_list_iterator_remove(&iter);
			uitimer =(CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
		}
		else if(uitimer->status == IDLE_STATUS){
			timer_list_iterator_remove(&iter);
			ZOS_FREE(uitimer);
			uitimer = NULL;
			memset(&iter,0,sizeof(timer_list_iterator_t));

			uitimer =(CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
		}
		else
		{
			uitimer = (CMTimer_t*) timer_list_get_next( &iter );
		}
	}
	pthread_mutex_unlock(&timerMutex);

	return ret;
}

// Add from ZyIMS
int UIDelPeriodicTimer_retDataPtr( int id, void **dataPtr ){
	int ret = TIMER_STOP_SUCCESS;
	CMTimer_t *uitimer = NULL;
	timer_list_iterator_t iter;
	bool isEverFound = FALSE;

	//printf("%s(): delete timer id(%d).\n", __FUNCTION__, id);

	if (id <= 0) {
		ret = TIMER_STOP_NOT_FOUND;
		return ret;
	}

	if (NULL == dataPtr){
		ret = TIMER_STOP_FAIL;
		return ret;
	}

	pthread_mutex_lock(&timerMutex);
	uitimer = (CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
	while(uitimer){
		if(uitimer->TimerID != id){

			uitimer = (CMTimer_t*) timer_list_get_next( &iter );

			if((NULL == uitimer) && !isEverFound) {
				ret = TIMER_STOP_NOT_FOUND;
			}
			continue;
		}
		isEverFound = TRUE;

		if(uitimer->status == PROC_STATUS){
			//Marking the timer has been deleted
			pthread_mutex_lock(&uitimer->timerMutex);
			uitimer->status = DEL_STATUS;
			pthread_mutex_unlock(&uitimer->timerMutex);
			uitimer = (CMTimer_t*) timer_list_get_next( &iter );
		}
		else{
			timer_list_iterator_remove(&iter);

			*dataPtr = uitimer->data;

			ZOS_FREE(uitimer);
			uitimer = NULL;
			memset(&iter, 0, sizeof(timer_list_iterator_t));

			uitimer = (CMTimer_t*) timer_list_get_first( &periodic_time_list, &iter );
		}
	}
	pthread_mutex_unlock(&timerMutex);

	return ret; //Ret: 0:Success, (-1):Fail/Error, 1:Timer-ID is NOT in the Timer List.
}

int UIDelAllTimer(void){
	struct timer_list *list;

	pthread_mutex_lock(&timerMutex);
	list=&time_list;
	if(list->nb_elt>0){
		timer_list_special_free( &time_list, freeTime );
		timer_list_init( &time_list );
	}
	list=&periodic_time_list;
	if(list->nb_elt>0){
		timer_list_special_free( &periodic_time_list, freeTime );
		timer_list_init( &periodic_time_list );
	}
	pthread_mutex_unlock(&timerMutex);

	return 0;
}


void* freeTime( void* pData )
{
	CMTimer_t *uitimer = (CMTimer_t*)pData;

	if (uitimer)
	{
		ZOS_FREE( uitimer );
	}

	return NULL;
}

int getSystemUptime(timeval_t *timeout){
	FILE *uptimefd = NULL;
	char *currTime = NULL;
	char *pch = NULL;
	char buf[256]="";
	char strUptime[128]="";
	char uptime_sec[128]="";
	int i;
	char uptime_usec[128]="";
	char *tmp = NULL;

	uptimefd = fopen(UPTIME_FILE,"r");

	if(uptimefd != NULL){
		fgets(buf, 256, uptimefd);
		//currTime = strtok (buf," ");
		currTime = strtok_r(buf, " ", &tmp);
		if(currTime == NULL){
			printf("System internal error!\n");
			fclose(uptimefd);
			return 1;
		}

		sprintf(strUptime, "%s", currTime);
		strcat(strUptime, "0000");
		//printf("strUptime   : %s\n", strUptime);
		sprintf(uptime_sec, "%s", strUptime);

		for(i=0; i<=strlen(uptime_sec); i++){
			if(uptime_sec[i] == '.'){
				uptime_sec[i] = '\0';
				timeout->tv_sec = atoi(uptime_sec);
				//printf("time->sec   : %d\n", timeout->tv_sec);
				break;
			}
		}

		pch = strstr(strUptime, ".");
		pch = pch + 1;
		strcpy(uptime_usec, pch);

		timeout->tv_usec = atoi(uptime_usec);
		//printf("usec = %d\n", timeout->tv_usec);

		fclose(uptimefd);
		return 0;
	}

	return 1;
}

void zyTms_get(struct timespec *tms)
{
	struct timespec ts;
	int rc;

	if(tms == NULL){
		return;
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(rc == 0){
		tms->tv_sec = ts.tv_sec;
		tms->tv_nsec = ts.tv_nsec;
	}
	else{
		printf("clock_gettime failed, set timestamp to 0\n");
		tms->tv_sec = 0;
		tms->tv_nsec = 0;
	}
}
