/*
 * mysyslog.c
 *
 *  Created on: 2011-7-31
 *      Author: tmc
 */



#include "common.h"
#include "mysyslog.h"

#include<syslog.h>
#include<stdarg.h>

#define APP_NAME "Project_server"

void my_openlog()
{
	openlog(APP_NAME,(LOG_CONS|LOG_NDELAY|LOG_PID),LOG_LOCAL0);
}

void my_closelog()
{
	closelog();
}

void Error_log(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);

	vsyslog(LOG_ERR,fmt,ap);

	va_end(ap);
}

void Warning_log(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);

	vsyslog(LOG_WARNING,fmt,ap);

	va_end(ap);
}

void Info_log(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);

	vsyslog(LOG_INFO,fmt,ap);

	va_end(ap);
}
