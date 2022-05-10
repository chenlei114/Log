#include "log.h"
#define  LOGPATH "/log.txt"
void main()
{
	initLogM();
	newLogFile(LOGPATH);
    putLog("log1\n",5);
    putLog("log1\n",5);
    putLog("log1\n",5);
    putLog("log1\n",5);
    putLog("log1\n",5);
    putLog("log1\n",5);
    putLog("log1\n",5);
	putLog("log1_end\n",5);
    saveLogFile();
    sleep(1);
}
