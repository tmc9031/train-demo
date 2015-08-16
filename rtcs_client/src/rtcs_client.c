/*
 ============================================================================
 Name        : rtcs_client.c
 Author      : tmc9031@gmail.com
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "main.h"

#include<pthread.h>

#define HEART_FLAG "-ok-"

//char buffer[BUFFER_SIZE];//字符串指针
//char buffer[128];
//int val=1;

int sockfd;
int sPTL=sizeof(PTL);

int my_read(int fd,void *buf,size_t len)
{
	size_t cc,total=0;

	while(len>0)
	{
		if((cc=read(fd,(char *)buf,len)) < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			return cc;
		}
		if(cc == 0)
		{
			break;
		}
		buf=(char *)(buf)+cc;
		total+=cc;
		len-=cc;
	}
	return total;
}

int my_write(int fd,void *buf,size_t len)
{
	size_t cc,total=0;

	while(len>0)
	{
		if((cc=write(fd,(char *)buf,len)) < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			return cc;
		}
		buf=(char *)buf+cc;
		total+=cc;
		len-=cc;
	}
	return total;
}

int read_buffer(void)
{
	char buffer[BUFFER_SIZE];//字符串指针
	int ret=0;

	while(1)
	{
		if((ret=my_read(sockfd,buffer,sizeof(buffer)-1))<0)//读取到buffer中
		{
			perror("ERROR read from socket");
			break;
		}
		if(ret==0)
		{
			printf("Server disconnect\n");
			raise(SIGINT);
			break;
		}
		buffer[ret]=0;	//read封口

		memcpy(&PTL,buffer,sPTL);

		if(strcmp(buffer+sPTL,HEART_FLAG) == 0)
			write(sockfd,buffer,sPTL+strlen(buffer+sPTL));
		else
			printf("%s",buffer+sPTL);

		sleep(1);
	}
	return 0;
}

int main(int argc,char *argv[])
{

	struct sockaddr_in servaddr;
	struct hostent *server;

	char buffer[BUFFER_SIZE];//字符串指针
	int ret=0;

	if(argc<2)
	{
		fprintf(stderr,"usage %s hostname\n",argv[0]);
		return RET_ERR;
	}
	if((server=gethostbyname(argv[1]))==NULL) //由域名获取IP地址
	{
		herror("gethostbyname.");
		return RET_ERR;
	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) //创建套接字
	{
		perror("ERROR opening socket");
		return RET_ERR;
	}

	memset(&servaddr,0,sizeof(servaddr));//申请信息表空间

	servaddr.sin_family=AF_INET;//设为IPv4
	servaddr.sin_addr.s_addr=*(uint32_t *)server->h_addr;//地址转换,32位大端字序
	servaddr.sin_port=htons((uint16_t)ECHO_PORT);//设定端口,16位大端字序转换

	if((ret=connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)))<0) //创建连接
	{
		perror("ERROR connecting");
		goto failed;
	}

	pthread_t reader;

	pthread_create(&reader,NULL,(void*)(read_buffer),NULL);//(标识符,线程属性<一般不设为NULL>,运行的函数地址,函数的参数)

//	val=0;
//	pthread_join(reader,NULL);//主线程等待reader线程退出

	while(1)
	{
//		printf("Enter the message : ");
		if(fgets(buffer+sPTL,sizeof(buffer)-1-sPTL,stdin)==NULL) //输入字符到buffer指针
		{
			break;
		}
		fflush(stdin);
		memcpy(buffer,&PTL,sPTL);
		if((ret=write(sockfd,buffer,sPTL+strlen(buffer+sPTL)))<0)	//把buffer的内容 写入strlen(buffer)的字节到sockfd的socket中
		{
			perror("ERROR write to socket");
			break;
		}
	}
	failed:
		close(sockfd);
		return ret<0?RET_ERR:RET_OK;
}
