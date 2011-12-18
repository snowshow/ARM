#include <stdio.h> // *printf
#include <stdarg.h> // va_list
#include <time.h> // time(), localtime()
#include <errno.h> // errno
#include <string.h> // strerror()
#include <sys/types.h> // getpid()
#include <unistd.h> // getpid()

#include "log.h"
#include "rephandler.h"

FILE* logfile;
char buffer[256];
char service[256];

int loginit(char* lgf)
{
	logfile = NULL;
	logfile = fopen(lgf, "a");
	if (logfile == NULL) {
		return -1;
	}
	sprintf(service, "handler");
	return 0;
}

void lservice(char* serv)
{
	sprintf(service, serv);
}

void lerror(int log, char* error)
{
	lprintf(log, "%s::error: %s", error, strerror(errno));
}

void lprintf(int log, const char *format, ...)
{
	va_list va;
	time_t t;
	struct tm *date;
	
	if (logfile == NULL)
		return;
	
	time(&t);
	date = localtime(&t);
	fprintf(logfile, "[%d/%d %d:%d:%d] \t",
			date->tm_mday, date->tm_mon,
			date->tm_hour, date->tm_min, date->tm_sec);
	
	fprintf(logfile, "<%s> \t", service);
	
	switch (log) {
		case LOG_INFO:
			fprintf(logfile, "INFO \t");
			break;
		case LOG_NOTICE:
			fprintf(logfile, "NOTICE \t");
			break;
		case LOG_WARNING:
			fprintf(logfile, "WARNING \t");
			break;
		case LOG_ERR:
			fprintf(logfile, "ERROR \t");
			break;
	}
	fprintf(logfile, "\t");
	
	va_start(va, format);
	vfprintf(logfile, format, va);
	va_end(va);
	
	fprintf(logfile, "\n");
	fflush(logfile);
} 
