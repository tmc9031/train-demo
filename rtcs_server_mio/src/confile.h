/*
 * confile.h
 *
 *  Created on: 2011-3-11
 *      Author: root
 */

#ifndef CONFILE_H_
#define CONFILE_H_


#define BUF_SIZE 512

typedef struct _cfg
{
	char key[64];
	char item[64];
}cfg;

typedef struct _node
{
	cfg data;
	struct _node *next;
}cfgNode, *cfgList;


typedef struct _acc
{
	char perm[32];
	char user[32];
	char pswd[32];
}acc;

typedef struct _accNode
{
	acc data;
	struct _accNode *next;
}accNode, *accList;


int confile();
int readToList(char*);	/* 从文件读数据成链表 */
void ExitFree();			/* 程序结束时回收资源 */
char* getItem(char*);		/*  获取CFG对应key的数据 */
accNode *getUser_Info(char *user_name);
void PrtInfo();			/* 仅测试时打印数据内容 */

void insertAccNode(char*);	/* 链表插入帐户数据 */
void insertCfgNode(char*);	/* 链表插入配置数据 */
void delSpace(char *);		/* 去空格  */
FILE *default_file(char *strFilename);

#endif /* CONFILE_H_ */
