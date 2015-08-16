/*
 * myfilelog.c
 *
 *  Created on: 2011-8-1
 *      Author: tmc
 */

#include "common.h"
#include "myfilelog.h"

#include "confile.h"

#include<time.h>

void log_info(char *info)
{
	char news[BUFFER_SIZE];
	FILE *fp;

	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/

	if ((fp = fopen(getItem("log"),"a+")) == NULL)
	{
		printf("can't open file\n");
	}
	sprintf(news,"%d-%d-%d %d:%d:%d[info]%s",(1900+p->tm_year),( 1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec,info);
	fputs(news,fp);

	fclose(fp);
}

void log_error(char *error)
{
	char news[BUFFER_SIZE];
	FILE *fp;

	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/

	if ((fp = fopen(getItem("log"),"a+")) == NULL)
	{
		printf("can't open file\n");
	}
	sprintf(news,"%d-%d-%d %d:%d:%d[error]%s",(1900+p->tm_year),( 1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec,error);
	fputs(news,fp);

	fclose(fp);
}

void log_warning(char *warning)
{
	char news[BUFFER_SIZE];
	FILE *fp;
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/

	if ((fp = fopen(getItem("log"),"a+")) == NULL)
	{
		printf("can't open file\n");
	}
	sprintf(news,"%d-%d-%d %d:%d:%d[warning]%s",(1900+p->tm_year),( 1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec,warning);
	fputs(news,fp);

	fclose(fp);
}
