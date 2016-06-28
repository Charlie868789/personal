/*
 * socket.c
 *
 *  Created on: 2011-7-14
 *      Author: root
 */
#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define DEBUG

int TCPClientInit( int *clientfd)
{
	*clientfd = socket(PF_INET, SOCK_STREAM, 0);
	return *clientfd;
}
int TCPClientConnect( const int clientfd, const char *addr, int port)
{
	struct sockaddr_in client;

	/// initialize value in client
	bzero(&client, sizeof(client));
	client.sin_family = PF_INET;
	client.sin_port = htons( port);
	inet_aton(addr, &client.sin_addr);

	/// Connecting to server
	return connect(clientfd, (struct sockaddr*)&client, sizeof(client));
}
int TCPWrite( int clientfd, unsigned char* buf, int size)
{
	int len= 0;
	len= send( clientfd, buf, size, MSG_NOSIGNAL);
	return len;
}
int TCPBlockRead( int clientfd, unsigned char* buf, int size)
{
	int opts;
	opts = fcntl(clientfd, F_GETFL);
	opts = (opts & ~O_NONBLOCK);
	fcntl(clientfd, F_SETFL, opts);
	return recv( clientfd, buf, size, 0);
}
int TCPSendFile(char *fns,int socket_fd)
{
    char buf[MAXDATASIZE];
    size_t ssize,rsize;
    int i=0;
    FILE *fp;
    char fn[200]="/sdcard/";
    strcat(fn,fns);
#ifdef DEBUG
    printf("fn %s\n",fn);
#endif
    while(1)
    {
	    TCPWrite(socket_fd,fns,strlen(fns));//发送文件名
	    usleep(20);
            if((fp=fopen(fn,"rb"))==NULL)
            {
                    perror("Can't open file");
                    return -1;
            }
            /*Send file*/
            while(!feof(fp))
            {
                    bzero(buf,sizeof(buf));
                    rsize=fread(buf,1,MAXDATASIZE,fp);
                    ssize=TCPWrite(socket_fd, buf,rsize);
		    Delay(800);//发送一个包后延时200MS
                    i++;
#ifdef DEBUG
                    printf("i=%d,rsize=%d,ssize=%d\n",i,rsize,ssize);
#endif
                    if(rsize < MAXDATASIZE)
                    	break;
            }
            bzero(buf,sizeof(buf));
            fclose(fp);
            break;
    }
    return 1;
}

int TCPRecvFile(int socket_fd,char *fns,char *fnns)
{
    int fp;
    char fn[20];
    char fnn[100]={0};
    unsigned char buf[MAXDATASIZE];
    ssize_t ssize;
    ssize_t rsize;
    int check;
    int i=0;
    bzero(fn,sizeof(fn));	//接收文件名,fnns为返回文件路径
    bzero(fnn,sizeof(fnn));
    bzero(buf,sizeof(buf));
    rsize = TCPBlockRead(socket_fd,fn,14);//接收文件名
    usleep(20);
    strcat(fnn,fns);
    strcat(fnn,fn);//子路径,
#ifdef DEBUG
    printf("fnn %s \n",fnn);
#endif
    if((fp = open(fnn,O_WRONLY|O_TRUNC|O_CREAT)) == -1)
    {
    	perror("can't open file");
	close(fp);
        return -1;
    }
    while(1)
   {
    	memset(buf,0,MAXDATASIZE);
    	rsize = TCPBlockRead(socket_fd,buf,sizeof(buf));	//接收文件
    	write(fp,buf,rsize);
    	i++;
    	printf("write i = %d,rsize = %d \n",i,rsize);
	if(rsize < MAXDATASIZE)
	{
		break;
	}
   }
   strcpy(fnns,fnn);
   close(fp);
   bzero(fn,sizeof(fn));
   bzero(fnn,sizeof(fnn));
   bzero(buf,sizeof(buf));
   return 1;
}
int Delay(int times)
{
	int i;
	for(i=1;i<=times;i++)
	{
		usleep(1000);//1ms
	}
	return 0;
}
