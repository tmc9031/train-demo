/*
 ============================================================================
 Name        : rtcs_server_mio.c
 Author      : tmc9031@gmail.com
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "common.h"
#include "confile.h"
#include "daemon.h"
#include "mysyslog.h"

#define ADMIN_DOWN "$kill\n"
#define PS_A_GREP "ps -A|grep Project_daemon"
#define EXIT_KEY "$exit\n"
#define HEART_FLAG "-ok-"

int loop=1;

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

int u_n_cmp_n_n(char *u_n,char *n_n)
{
	char u_n_buf[32]="\0";
	int i=0;

	strcpy(u_n_buf,n_n);
	for(i=0; i<32; i++)
	{
		if(u_n_buf[i] == '\n')
		{
			u_n_buf[i]='\0';
			break;
		}
	}

	if(strcmp(u_n, u_n_buf) == 0)
		return 1;
	else
		return 0;
}

int main(int argc,char *argv[])
{
	struct sockaddr_in servaddr,remote;
	int request_sock,new_sock;
	int nfound,fd,maxfd,bytesread;
	uint32_t addrlen;
	fd_set rset,set;
	struct timeval timeout;
	char buf[BUFFER_SIZE];
	char buf_temp[BUFFER_SIZE];

	  FILE *stream=NULL;
	  pid_t pid;

	char phase[100]="\0";
	int phase_user[100]={0};
	int phase_passwd[100]={0};
	accNode *user_pswd[100] = {NULL};
	int ok_num[100]={0};
	int over_alarm=0;
	int sPTL=sizeof(PTL);

	my_openlog();

	confile();

	if((request_sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		Error_log("[错误] new_socket:%s",strerror(errno));
		perror("ERROR opening socket");
		return RET_ERR;
	}
	memset(&servaddr,0,sizeof(servaddr));

//	设置监听套接字地址重用
	int opt=1;
	if((setsockopt(request_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)))<0)
	{
		perror("ERROR setsockopt");
	}

	servaddr.sin_family=AF_INET;
//	servaddr.sin_addr.s_addr=INADDR_ANY;
	inet_pton(AF_INET,getItem("ip"),&servaddr.sin_addr.s_addr);
	servaddr.sin_port=htons((uint16_t)atoi(getItem("port")));

	if(bind(request_sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
	{
		Error_log("[错误] bind_socket:%s",strerror(errno));
		perror("ERROR on binding");
		return RET_ERR;
	}
	else
		printf("服务器已成功绑定IP：%s\n",getItem("ip"));

//	make_daemon();
	set_signal(SIGUSR2,sig_routine);//设置接收守护进程发来的关闭信号
	Info_log("[通知] 服务器已启动 IP:%s Port:%s",getItem("ip"),getItem("port"));

	if(listen(request_sock,atoi(getItem("queue")))<0)
	{
		Error_log("[错误] listen_socket:%s",strerror(errno));
		perror("ERROR on listen");
		return RET_ERR;
	}

	FD_ZERO(&set);
	FD_SET(request_sock,&set);
	maxfd=request_sock;
	over_alarm=atoi(getItem("alarm"));

	while(1)
	{
		rset=set;
		timeout.tv_sec=over_alarm;
		timeout.tv_usec=0;

		if((nfound=select(maxfd+1,&rset,(fd_set*)0,(fd_set*)0,&timeout))<0)
		{
			perror("select");
			return -1;
		}
		else if(nfound==0)
		{
			printf("select_超时%d秒...\n",over_alarm);
			fflush(stdout);
//			Warning_log("[警告] select_超时%d秒",over_alarm);

			int fd;
			for(fd=5; fd<=maxfd; fd++)
			{
				if(phase[fd] != '\0')
//				if(!FD_ISSET(fd,&rset))
				{
					if(ok_num[fd] > 0)
					{
						FD_CLR(fd,&set);
						close(fd);
						phase[fd]='\0';
						Warning_log("[警告] 客户端超时断开 Socket:%d",fd);
					}
					else
					{
						ok_num[fd]++;
						printf("向%d号fd客户端发送心跳包...\n",fd);

						PTL.type=phase[fd];
						PTL.len=0;
						memcpy(buf,&PTL,sPTL);
						strcpy(buf+sPTL,HEART_FLAG);

						if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
						{
							Error_log("[错误] write:%s",strerror(errno));
							perror("write");

							FD_CLR(fd,&set);
							close(fd);
							phase[fd]='\0';
							Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
						}
					}
				}
			}
			continue;
		}

		if(FD_ISSET(request_sock,&rset))
		{
			addrlen=sizeof(remote);
			if((new_sock=accept(request_sock,(struct sockaddr*)&remote,&addrlen))<0)
			{
				Error_log("[错误] accept:%s",strerror(errno));
				perror("accept");
				return -1;
			}
			Info_log("[通知] 客户端连接 IP:%s Port:%d Socket:%d",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port),new_sock);
			printf("connection from host %s,port %d,socket %d\r\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port),new_sock);

			ok_num[new_sock]=0;

			phase_user[new_sock]=3;
			phase_passwd[new_sock]=3;

			phase[new_sock]='1';
			PTL.type=phase[new_sock];
			PTL.len=0;
			memcpy(buf,&PTL,sPTL);
			sprintf(buf+sPTL,"输入用户名(还剩%d次)：\n",phase_user[new_sock]);
			write(new_sock,buf,sPTL+strlen(buf+sPTL));

			FD_SET(new_sock,&set);
			if(new_sock > maxfd) maxfd=new_sock;

			FD_CLR(request_sock,&rset);
			nfound--;
		}

		printf("maxfd:%d号\n",maxfd);
		for(fd=0; fd<=maxfd && nfound>0; fd++)
		{
			if(FD_ISSET(fd,&rset))
			{
				nfound--;
				if((bytesread=read(fd,buf,sizeof(buf)-1))<0)
				{
					Error_log("[错误] read:%s",strerror(errno));
					perror("read");
				}
				if(bytesread==0)
				{
					fprintf(stderr,"server: end of file on %d\r\n",fd);

					FD_CLR(fd,&set);
					close(fd);
					phase[fd]='\0';
					Info_log("[通知] 客户端断开 Socket:%d",fd);

					continue;
				}
				buf[bytesread]=0;

				memcpy(&PTL,buf,sPTL);
				phase[fd]=PTL.type;

				if(strcmp(buf+sPTL,HEART_FLAG) == 0)
				{
					ok_num[fd]--;
					printf("第%d号fd客户端成功接收心跳包！\n",fd);
				}
				else
				switch(phase[fd])
				{
					case '1': //验证( buf+8)的内容是不是"user"
						if((user_pswd[fd] = getUser_Info(buf+sPTL)) == NULL)
						{
							phase_user[fd]--;

							if(phase_user[fd] != 0)
							{
								PTL.type='1';
								PTL.len=0;
								memcpy(buf,&PTL,sPTL);
								sprintf(buf+sPTL,"输入用户名(还剩%d次)：\n",phase_user[fd]);

								if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("write");

									FD_CLR(fd,&set);
									close(fd);
									phase[fd]='\0';
									Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
								}
							}
							else
							{
								PTL.type='1';
								PTL.len=0;
								memcpy(buf,&PTL,sPTL);
								strcpy(buf+sPTL,"输入用户名超过次数限制！\n");

								if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("write");

									FD_CLR(fd,&set);
									close(fd);
									phase[fd]='\0';
									Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
								}

								FD_CLR(fd,&set);
								close(fd);
								phase[fd]='\0';
								Info_log("[通知] 客户端断开 Socket:%d",fd);
							}
						}
						else
						{
							phase[fd]='2';
							PTL.type=phase[fd];
							PTL.len=0;
							memcpy(buf,&PTL,sPTL);
							sprintf(buf+sPTL,"输入密码(还剩%d次)：\n",phase_passwd[fd]);

							if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
							{
								Error_log("[错误] write:%s",strerror(errno));
								perror("write");

								FD_CLR(fd,&set);
								close(fd);
								phase[fd]='\0';
								Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
							}
						}
						break;

					case '2': //验证( buf+8)的内容是不是"passwd"
						if(u_n_cmp_n_n(user_pswd[fd]->data.pswd, buf+sPTL) == 0)
						{
							phase_passwd[fd]--;

							if(phase_passwd[fd] != 0)
							{
								PTL.type='2';
								PTL.len=0;
								memcpy(buf,&PTL,sPTL);
								sprintf(buf+sPTL,"输入密码(还剩%d次)：\n",phase_passwd[fd]);

								if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("write");

									FD_CLR(fd,&set);
									close(fd);
									phase[fd]='\0';
									Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
								}
							}
							else
							{
								PTL.type='2';
								PTL.len=0;
								memcpy(buf,&PTL,sPTL);
								strcpy(buf+sPTL,"输入密码超过次数限制！\n");

								if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("write");

									FD_CLR(fd,&set);
									close(fd);
									phase[fd]='\0';
									Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
								}

								FD_CLR(fd,&set);
								close(fd);
								phase[fd]='\0';
								Info_log("[通知] 客户端断开 Socket:%d",fd);
							}
						}
						else
						{
							if(strcmp(user_pswd[fd]->data.perm,"A") == 0)
								phase[fd]='A';
							else
								phase[fd]='U';

							PTL.type=phase[fd];
							PTL.len=0;
							memcpy(buf,&PTL,sPTL);
							strcpy(buf+sPTL,"欢迎使用远程控制终端！！！($exit：退出)\n");

							if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)
							{
								Error_log("[错误] write:%s",strerror(errno));
								perror("write");

								FD_CLR(fd,&set);
								close(fd);
								phase[fd]='\0';
								Warning_log("[警告] 客户端异常断开 Socket:%d",fd);
							}
						}
						break;

					case 'A': //管理员接收指令
						  //use popen start a process with pipe
						if(strcmp(buf+sPTL,ADMIN_DOWN) == 0)
						{
							  FILE *stream=NULL;
							  char str_pid[512]="\0";

							  stream = popen(PS_A_GREP, "r");
							  if (stream == NULL) {
							    perror("popen error");
							    exit(-1);
							  }
							  fgets(str_pid,sizeof(str_pid),stream);
							  pclose(stream);

							  Warning_log("[警告] 管理员关闭服务器 Socket:%d",fd);
							  kill(atoi(str_pid),SIGUSR1);
						}
						else if(strcmp(buf+sPTL,EXIT_KEY) == 0)
						{
							FD_CLR(fd,&set);
							close(fd);
							phase[fd]='\0';
							Info_log("[通知] 客户端断开 Socket:%d",fd);
						}
						else
						{
							stream = popen(buf+sPTL, "r");
							if (stream == NULL)
							{
								Error_log("[错误] popen_error:%s",strerror(errno));
								perror("popen error");
								return FILE_OPEN_ERROR;
							}

							PTL.type=phase[fd];
							PTL.len=0;
							memcpy(buf,&PTL,sPTL);
							while(fgets(buf+sPTL,sizeof(buf)-1-sPTL,stream) != NULL)
							{
								if(my_write(fd,buf,sPTL+strlen(buf+sPTL))<0)	//把buffer的内容 写入strlen(buffer)的字节到sockfd的socket中
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("ERROR write to socket");
									break;
								}
								usleep(50000);
							}
							pclose(stream);
						}
						break;

					case 'U': //接收指令
						  //use popen start a process with pipe
						if(strcmp(buf+sPTL,EXIT_KEY) != 0)
						{
							stream = popen(buf+sPTL, "r");
							if (stream == NULL)
							{
								Error_log("[错误] popen_error:%s",strerror(errno));
								perror("popen error");
								return FILE_OPEN_ERROR;
							}

							PTL.type=phase[fd];
							PTL.len=0;
							memcpy(buf,&PTL,sPTL);
							while(fgets(buf+sPTL,sizeof(buf)-1-sPTL,stream) != NULL)
							{
								if(write(fd,buf,sPTL+strlen(buf+sPTL))<0)	//把buffer的内容 写入strlen(buffer)的字节到sockfd的socket中
								{
									Error_log("[错误] write:%s",strerror(errno));
									perror("ERROR write to socket");
									break;
								}
								usleep(5000);
							}
							pclose(stream);
						}
						else
						{
							FD_CLR(fd,&set);
							close(fd);
							phase[fd]='\0';
							Info_log("[通知] 客户端断开 Socket:%d",fd);
						}
						break;

					case 'Q': //关闭客户端的连接
						if(strcmp(buf+sPTL,"$exit\n") == 0)
						{
							FD_CLR(fd,&set);
							close(fd);
						}
						break;
				}
			}

		}
	}

	printf("[%d]:main quit!!!",getpid()); /* prints !!!Hello World!!! */
	Info_log("[通知] 服务器已关闭 IP:%s Port:%s",getItem("ip"),getItem("port"));
	closelog();
	ExitFree();
	return KILL_SERVER;
}
