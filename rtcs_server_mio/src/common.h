/*
 * ---远程终端控制系统---
 *
 * Name: RTCS
 *
 * common.h
 *
 *  Created on: 2011-7-29
 *      Author: tmc
 */

#ifndef COMMON_H_
#define COMMON_H_

#include<sys/types.h>
#include<ctype.h>
#include<strings.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/time.h>
#include<stdio.h>
#include<string.h>
#include<sys/select.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/wait.h>

#define RET_OK 0
#define RET_ERR -1
#define ARG_ERROR -2
#define FILE_OPEN_ERROR -3
#define KILL_SERVER -9

#define BUFFER_SIZE 512


struct head
{
	char type; /* 数据类型 */
	int len;   /* 后续数据长度 */
}PTL;

int my_read(int fd,void *buf,size_t len);
int my_write(int fd,void *buf,size_t len);

#endif /* COMMON_H_ */
