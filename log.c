#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#define LOGDATAMAXLENGTH 1024
#define PATHMAXLENGTH	 250
#define LOGSQUEMAXLENGTH 50



typedef struct log
{
	char log[LOGDATAMAXLENGTH];
	int len;
	int state;//0 :enmpty  1 have data
	int op;//0:write log 1:start a new logfile 2:save current log file
	char path[PATHMAXLENGTH];//path name
}LOGDATA;
typedef struct logsque
{
	LOGDATA Logs[LOGSQUEMAXLENGTH];
	int indexhead;
	int indextail;
	pthread_spinlock_t spinlock;//
	FILE* fp;//Current LogFile
	
}LOGSQUE;

LOGSQUE logSque;
struct sched_param			clock_interrupt_thread_sched;
pthread_attr_t			clock_interrupt_thread_attr;
pthread_t			clock_interrupt_thread;

static int Thread_Attr_Set( pthread_attr_t *attr, struct sched_param *sched );

static void log_handler(void);
static void * pFun(void *ptr);
static char makefilesFlow(char Path[]);
char initLogM()
{
	char ret = 0;
	int i = 0;
	int status;
	
	clock_interrupt_thread_sched.sched_priority = 1;	
	status = Thread_Attr_Set(&clock_interrupt_thread_attr, &clock_interrupt_thread_sched);

	status = pthread_create(&clock_interrupt_thread, \
							&clock_interrupt_thread_attr, \
							(void *)pFun, \
							(void *)NULL);


		
	
	logSque.indexhead = 1;
	logSque.indextail = 0;
	pthread_spin_init(&(logSque.spinlock),PTHREAD_PROCESS_PRIVATE);
	
	for(i = 0; i<LOGSQUEMAXLENGTH;i++)
	{
		memset(logSque.Logs[i].log,0,LOGDATAMAXLENGTH);
		logSque.Logs[i].len = 0;
		logSque.Logs[i].state = 0;
		logSque.Logs[i].op = 0;
		memset(logSque.Logs[i].path,0,PATHMAXLENGTH);		
	}
	logSque.fp = NULL;
	
	return ret;
}
char newLogFile(char *name)
{
	char ret = 0;
	int indexnexthead = 0;
	pthread_spin_lock(&(logSque.spinlock)); //上锁
       
	indexnexthead = logSque.indexhead;
	if(indexnexthead == (LOGSQUEMAXLENGTH - 1))
	{
		indexnexthead = 0;
	}else
	{
		indexnexthead++;
	}
	
	if(indexnexthead != logSque.indextail) 
	{
		logSque.Logs[logSque.indexhead].state = 0;
		memset(logSque.Logs[logSque.indexhead].path,0,PATHMAXLENGTH);
		memcpy(logSque.Logs[logSque.indexhead].path,name,strlen(name));
		logSque.Logs[logSque.indexhead].op = 1;
		
		logSque.indexhead = indexnexthead;
	}
	
        pthread_spin_unlock(&(logSque.spinlock)); //解锁
	return ret;
}
char putLog(char *log,int len)
{
	char ret = 0;
	int indexnexthead = 0;
	pthread_spin_lock(&(logSque.spinlock)); //上锁
       
	indexnexthead = logSque.indexhead;
	if(indexnexthead == (LOGSQUEMAXLENGTH - 1))
	{
		indexnexthead = 0;
	}else
	{
		indexnexthead++;
	}
	
	if(indexnexthead != logSque.indextail) 
	{
		logSque.Logs[logSque.indexhead].state = 0;
		memset(logSque.Logs[logSque.indexhead].log,0,LOGDATAMAXLENGTH);
		memcpy(logSque.Logs[logSque.indexhead].log,log,len);
		logSque.Logs[logSque.indexhead].len = len;
		logSque.Logs[logSque.indexhead].op = 0;
		
		logSque.indexhead = indexnexthead;
	}
	
        pthread_spin_unlock(&(logSque.spinlock)); //解锁
	
	return ret;
}
char saveLogFile()
{
	char ret = 0;

	int indexnexthead = 0;
	pthread_spin_lock(&(logSque.spinlock)); //上锁
       
	indexnexthead = logSque.indexhead;
	if(indexnexthead == (LOGSQUEMAXLENGTH - 1))
	{
		indexnexthead = 0;
	}else
	{
		indexnexthead++;
	}
	
	if(indexnexthead != logSque.indextail) 
	{
		logSque.Logs[logSque.indexhead].state = 0;
		
		logSque.Logs[logSque.indexhead].op = 2;
		
		logSque.indexhead = indexnexthead;
	}
	
        pthread_spin_unlock(&(logSque.spinlock)); //解锁
	
	return ret;
}
void log_handler(void)
{	
	printf("log_handler \n");
	char ret = 0;
	int i = 0;
	LOGDATA Log;
	char destNamePath[PATHMAXLENGTH] = {0};
	int indextailnext = logSque.indextail;
	printf("head = %d,tail = %d",logSque.indexhead,logSque.indextail);
	if(1)
	{
		pthread_spin_lock(&(logSque.spinlock)); //上锁
		Log = logSque.Logs[logSque.indextail];		

		memset(logSque.Logs[logSque.indextail].log,0,LOGDATAMAXLENGTH);
		logSque.Logs[logSque.indextail].len = 0;
		logSque.Logs[logSque.indextail].state = 0;
		logSque.Logs[logSque.indextail].op = 0;
		memset(logSque.Logs[logSque.indextail].path,0,PATHMAXLENGTH);

		if(indextailnext == (LOGSQUEMAXLENGTH - 1))
		{
			indextailnext = 0;
		}else
		{
			indextailnext++;
		}
		
		if(indextailnext == logSque.indexhead)
		{
			
		}else
		{
			logSque.indextail = indextailnext;
		}	
		pthread_spin_unlock(&(logSque.spinlock)); //解锁

		switch(Log.op)
		{
			case 0:
			if((Log.len == 0)||(logSque.fp == NULL))
			{
				break;
			}
			fwrite(Log.log, Log.len, 1, logSque.fp);	
			
			break;
			case 1:
			if(ret==0)
			{
				for(i = 0; i <= strlen(Log.path); i++)
				{
					destNamePath[i] = Log.path[i];
				}
				for(i = strlen(Log.path); i > 0; i--)
				{
					if(destNamePath[i] != '/')
					{
						destNamePath[i] = 0;

					}else
					{
						break;
					}
				}
		
				if(0 != access(destNamePath, F_OK))
				{
					makefilesFlow(destNamePath);
				}
			}


			if (0 == ret)
			{	logSque.fp = NULL;
				logSque.fp = fopen(Log.path, "rb+");
				if (NULL == logSque.fp)
				{
					logSque.fp = fopen(Log.path, "wb+");
					if (NULL != logSque.fp)
					{
						ret = 0;
					}
					else 
					{	
				
						;
					}
				}
				else
				{
			
				}
			}

			break;
			case 2:
			fclose(logSque.fp);
			logSque.fp = NULL;
			
			break;
			default:
			
			break;
		}
	}
}


char makefilesFlow(char Path[])
{
	char ret = 0;
	int i = 0;
	char namePath[100];
	memset(namePath, 0, 100);
	for(i = 0; i < strlen(Path)-1; i++)
	{
		namePath[i] = Path[i];
	}
	for(i = strlen(Path); i > 0; i--)
	{
		if(namePath[i] != '/')
		{
			namePath[i] = 0;										
		}else
		{
			break;
		}
	}
			
	if(0 != access(namePath, F_OK))
	{
		
		if(0 != makefilesFlow(namePath))
		{
			return 1;
		}
		
	}
	
	umask(0000);
	if(0 != mkdir(Path, 0777))
		ret = 3;
	else
		ret = 0;
	
	return ret;
}

static int Thread_Attr_Set( pthread_attr_t *attr, struct sched_param *sched )
{
    int iStatus;
    iStatus = pthread_attr_init( attr );
    if( iStatus != 0 )
    {
        fprintf(stderr, "[pthread_attr_init]failed.  Terminating with an error\n");
        return -1;
    }

    iStatus = pthread_attr_setinheritsched( attr, PTHREAD_EXPLICIT_SCHED );
    if( iStatus != 0 )
    {
        fprintf(stderr, "[pthread_attr_setinheritsched]failed.  Terminating with an error\n");
        return -1;
    }

    iStatus = pthread_attr_setschedpolicy( attr, SCHED_FIFO );
    if( iStatus != 0 )
    {
        fprintf(stderr, "Thread Schedule Policy failed.  Terminating with an error\n");
        return -1;
    }
    
    iStatus = pthread_attr_setschedparam( attr, sched );
    if(iStatus != 0)
    {
        fprintf(stderr, "[pthread_attr_setschedparam]failed.  Terminating with an error\n");
        return -1;
    }

    return 0;
}

void * pFun(void *ptr)
{
	while(1)
	{
		log_handler();
		usleep(5000);
	}
}







