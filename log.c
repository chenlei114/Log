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

struct logsque
{
	LOGDATA Logs[LOGSQUEMAXLENGTH];
	int indexhead;
	int indextail;
	pthread_spinlock_t spinlock;//
	FILE* fp;//Current LogFile
	
}LOGSQUE;

struct log
{
	char log[LOGDATAMAXLENGTH];
	int len;
	int state;//0 :enmpty  1 have data
	int op;//0:write log 1:start a new logfile 2:save current log file
	char path[PATHMAXLENGTH];//path name
}LOGDATA;

LOGSQUE logSque;
static void log_handler(int sig_num);
static char makefilesFlow(char Path[]);
char initLogM()
{
	char ret = 0;
	int i = 0;
	signal(SIGALRM, log_handler);
	struct itimerval olditv;
	struct itimerval itv;
	itv.it_interval.tv_sec = 0; //定时周期为1秒钟。
	itv.it_interval.tv_usec = 3000;
	itv.it_value.tv_sec = 0; //定时器启动以后将在50微秒以后正式开始计时。
	itv.it_value.tv_usec = 50;
	setitimer(ITIMER_REAL, &itv, &olditv);
	
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
		Logs[logSque.indexhead].op = 1;
		
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
		Logs[logSque.indexhead].len = len;
		Logs[logSque.indexhead].op = 0;
		
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
		
		Logs[logSque.indexhead].op = 2;
		
		logSque.indexhead = indexnexthead;
	}
	
        pthread_spin_unlock(&(logSque.spinlock)); //解锁
	
	return ret;
}
void log_handler(int sig_num)
{	char ret = 0;
	int i = 0;
	LOGDATA Log;
	destNamePath[PATHMAXLENGTH] = {0};
	int indextailnext = logSque.indextail;
	if(sig_num = SIGALRM)
	{
		pthread_spin_lock(&(logSque.spinlock)); //上锁
		log = logSque.Logs[logSque.indextail];
		logSque.Logs[logSque.indextail].

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

		switch(log.op)
		{
			case 0:
			if((log.len == 0)||(logSque.fd == NULL))
			{
				break;
			}
			fwrite(log.log, log.len, 1, logSque.fp);	
			
			break;
			case 1:
			if(ret==0)
			{
				for(i = 0; i <= strlen(log.path); i++)
				{
					destNamePath[i] = log.path[i];
				}
				for(i = strlen(log.path); i > 0; i--)
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
				logSque.fp = fopen(log.path, "rb+");
				if (NULL == logSque.fp)
				{
					fp = fopen(log.path, "wb+");
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









