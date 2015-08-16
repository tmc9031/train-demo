/*
 * confile.c
 *
 *  Created on: 2011-5-14
 *      Author: root
 */

#include "common.h"
#include "confile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_NAME "ini.txt"

/*
 * 在单进程单线程中，为了方便操作，可用全局变量，
 * 在多进程，多线程中，尽可能不使用全局变量
 * 尽量养成不使用全局变量的习惯
 */

/* 全局变量 */
/* 头结点  */
 cfgList cHead = NULL;
 accList aHead = NULL;
/* 尾结点  */
 cfgList cL = NULL;
 accList aL = NULL;

int confile()
{
	readToList(FILE_NAME);

//	PrtInfo();

//	ExitFree();

	puts("服务器配置文件已加载。"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}

cfgList cfgNode_newhead()
{
	void *h=NULL;

	h=malloc(sizeof(cfgNode));
	memset(h,0,sizeof(cfgNode));

	return (cfgList)h;
}

accList accNode_newhead()
{
	void *h=NULL;

	h=malloc(sizeof(accNode));
	memset(h,0,sizeof(accNode));

	return (accList)h;
}

int readToList(char *strFilename)
{
	void(*ptrFun)(char *) = NULL;
	FILE *fp = NULL;
	char strBuf[BUF_SIZE] = {0};
	char *ptrStr = NULL;

	fp = fopen(strFilename, "r");
	if(fp == NULL)
	{
		Error_log("[错误] fopen:%s",strerror(errno));
		perror("fopen");
		fp=default_file(strFilename);
		if(fp == NULL)
		{
			return FILE_OPEN_ERROR;
		}
	}

	cHead=cfgNode_newhead();
	aHead=accNode_newhead();

	cL=cHead;
	aL=aHead;

	while(fgets(strBuf, BUF_SIZE, fp) != NULL)
	{
		for(ptrStr = strBuf; *ptrStr == ' ' && *ptrStr != '\0';ptrStr++); /* 跳过开头的空格 */
		switch(*ptrStr)
		{
			case '\r':
			case '\n':
			case ';':
			case '#':break;
			case '[':
					/*  一并去除后面的空格和\n，主要是最后的空格 */
					delSpace(ptrStr);
					if(strcasecmp(ptrStr, "[CFG]") == 0)
					{	/*  函数指针 */
						ptrFun = insertCfgNode;
						break;
					}else if(strcasecmp(ptrStr, "[ACCOUNT]") == 0)
					{
						ptrFun = insertAccNode;
						break;
					}
					break;
			default:delSpace(ptrStr);
					ptrFun(ptrStr);	/* 由函数指针 引导处理方式 */
					break;
		}
	}
	fclose(fp);
	return EXIT_SUCCESS;
}

FILE *default_file(char *strFilename)
{
	FILE *fp=NULL;

	fp = fopen(strFilename, "w+");
	if(fp == NULL)
	{
		Error_log("[错误] default_fopen:%s",strerror(errno));
		perror("default_fopen");
		return NULL;
	}

	fputs("[cfg]\n",fp);
	fputs("ip = 192.168.10.100\n",fp);
	fputs("queue = 10\n",fp);
	fputs("port = 50000\n",fp);
	fputs("alarm = 10\n",fp);
	fputs("log = /tmp/log.txt\n",fp);

	fputs("[account]\n",fp);
	fputs("acc = A/admin/admin\n",fp);
	fputs("acc = U/user1/111\n",fp);
	fputs("acc = U/user2/222\n",fp);

	fflush(fp);
	rewind(fp);
	return fp;
}

char* getItem(char *key)
{
	cfgNode *cTemp = cHead;

	cTemp = cTemp->next;
	for(; cTemp != NULL; cTemp = cTemp->next)
	{
		if(strcmp(key, cTemp->data.key) == 0)
			return cTemp->data.item;
	}
	return NULL;
}

accNode *getUser_Info(char *user_name)
{
	accNode *aTemp = aHead;

	char un_user_name[32]="\0";
	int i=0;

	strcpy(un_user_name,user_name);
	for(i=0; i<32; i++)
	{
		if(un_user_name[i] == '\n')
		{
			un_user_name[i]='\0';
			break;
		}
	}

	for(aTemp = aTemp->next; aTemp != NULL; aTemp = aTemp->next)
	{
		if(strcmp(un_user_name, aTemp->data.user) == 0)
			break;
	}
	return aTemp;
}

/*
 * //查找学生学号
Snode *stu_check_num(Snode *head,int num)
{
	Snode *s=head;

	s=s->next;
	while(s!=NULL && num!=s->num)
	{
		s=s->next;
	}
	return s;
}
 */

 void insertAccNode(char *Data)
{
	accNode *temp = NULL;

	temp = (accNode*)malloc(sizeof(accNode));
	strtok(Data, "=/");/* 忽略第一个acc字符*/
	strcpy(temp->data.perm, strtok(NULL, "=/\r\n"));
	strcpy(temp->data.user, strtok(NULL, "=/\r\n"));
	strcpy(temp->data.pswd, strtok(NULL, "=/"));

	/* 尾插 */
	aL->next = temp;
	aL = temp;
	aL->next = NULL;
}

 void insertCfgNode(char *Data)
{
	cfgNode *temp = NULL;

	temp = (cfgNode*)malloc(sizeof(cfgNode));
	strcpy(temp->data.key, strtok(Data, "=\r\n"));
	strcpy(temp->data.item, strtok(NULL, "=\r\n"));

	/* 尾插 */
	cL->next = temp;
	cL = temp;
	cL->next = NULL;
}

 void delSpace(char *Data)
{
	char *ptr = Data;
	/*  找到后面的第一个空格或\n */
	for(; *ptr != '\0' && *ptr != ' ' && *ptr != '\n' && *ptr != '\r'; ptr++, Data++);
	/*  判断是否是有空格导致的for中断 */
	if(*ptr == '\0') return; /* 无空格返回*/
	/*  有空格则消空格 */
	for(; *Data != '\0' ; Data++)
	{
		if(*Data != ' ' && *Data != '\n' && *Data != '\r')
		{
			*ptr = *Data;
			ptr++;
		}
	}
	*ptr = '\0'; /* 最后填上结束符 */
}
void ExitFree()
{
	cfgNode *cp = NULL;
	cfgNode *cTemp = NULL;
	accNode *ap = NULL;
	accNode *aTemp = NULL;

	cp=cHead->next;
	free(cHead);
	while(cp != NULL)
	{
		cTemp = cp->next;
		free(cp);
		cp = cTemp;
	}

	ap=aHead->next;
	free(aHead);
	while(ap != NULL)
	{
		aTemp = ap->next;
		free(ap);
		ap = aTemp;
	}
}

void PrtInfo()
{
	cfgNode *cTemp = cHead;
	accNode *aTemp = aHead;

	aTemp = aTemp->next;
	while(aTemp != NULL)
	{
		printf("%-5s%-16s%-16s\n",aTemp->data.perm, aTemp->data.user, aTemp->data.pswd);
		aTemp = aTemp->next;
	}
	cTemp = cTemp->next;
	while(cTemp != NULL)
	{
		printf("%-10s%-10s\n",cTemp->data.key, cTemp->data.item);
		cTemp = cTemp->next;
	}
}
