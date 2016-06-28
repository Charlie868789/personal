#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include "atmel9260.h"
#include "atmel9260file.h"
#include "socket.h"
#define DEBUG


int atmel9260_communication()
{
	int flag=-1;
	int port = 4321;
	int clientfd;
	int lens;
	int RunFlag;
	int i;
	int RunMod;
	struct timeval timeo;
	unsigned char Sha1sum[20];
	unsigned char Sha2sum[20];
	//读取软件版本,以及其他相关信息
	char RecvBuf[20];
	char rmmbuf[1024]="rm -rf ";
	char fns[1024]="/sdcard/";
	char fnns[1024];
	char cps[100];
	unsigned char RevSum[20];
	ModInfo.ModServion_maincomm = (char *)malloc(40);
	ModInfo.ModServion_dataserver =(char *)malloc(40);
	ModInfo.ModServerIp = (char *)malloc(40);
	ModInfo.StationName = (char *)malloc(40);
	flag = ReadModinfo(ModInfo);
#ifdef DEBUG
	printf("flag :%d\n",flag);
#endif
	if( flag < 0)
	{
		printf("ReadModuInfo Error\n");
		return -1;
	}
#ifdef DEBUG
	printf("Module->ModServion_maincomm: %s \n",ModInfo.ModServion_maincomm); //采集程序版本号
	printf("Module->ModServion_dataserver: %s \n",ModInfo.ModServion_dataserver); //socket通信程序版本号
	printf("Module->ModServerIp: %s \n",ModInfo.ModServerIp); //服务器IP地址
	printf("Module->StationName: %s \n",ModInfo.StationName); //基站名称
#endif
	//与服务器建立链接,作为客户端
	while(TCPClientInit(&clientfd) == -1)
		sleep(1);
	while(TCPClientConnect(clientfd, ModInfo.ModServerIp,port)!=0)
		sleep(1);
#ifdef DEBUG
	printf("connet server ok\n");
#endif
	//向服务器发送当前模块信息
	timeo.tv_sec = 10;
	timeo.tv_usec = 0;
	lens = TCPWrite(clientfd, ModInfo.ModServion_maincomm,strlen(ModInfo.ModServion_maincomm));
	if(lens != strlen(ModInfo.ModServion_maincomm)) return -1;
	usleep(20);
	lens = TCPWrite(clientfd, ModInfo.ModServion_dataserver,strlen(ModInfo.ModServion_dataserver));
	if(lens != strlen(ModInfo.ModServion_dataserver)) return -1;
	setsockopt(clientfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeo,sizeof(struct timeval));	//等待10S，接受服务器命令
	TCPBlockRead(clientfd, RecvBuf, 9); //暂时还没收到命令
#ifdef DEBUG
	printf("recv command :%s \n",RecvBuf);
#endif
	if(strcmp(RecvBuf,"RecvMode")==0)  //接受数据模式
		RunFlag = 1;
	else if(strcmp(RecvBuf,"ChagMode")==0) //改变天线值
		RunFlag = 2;
	else if(strcmp(RecvBuf,"UpdaMode")==0) //远程更新
		RunFlag = 3;
	else if(strcmp(RecvBuf,"EtcMod") == 0) //接受配置文件，服务器端还没有加载
		RunFlag = 4;
	else 
		RunFlag = 1;//服务器没有响应默认是传输数据文件
#ifdef DEBUG
	printf("RunFlag %d\n",RunFlag);
#endif
	switch(RunFlag)
	{
		case 1 :  //传送传感器数据文件
			while(1)
			{
				strcat(ModInfo.StationName,".txt");
				strcat(fns,ModInfo.StationName); //数据文件的绝对路径
#ifdef DEBUG
				printf("fns %s\n",fns);
#endif
				GetSha1Sum(fns,RevSum); //计算传感器数据文件的哈希值
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x",RevSum[i]);
				printf("\n");
#endif
				lens = TCPWrite(clientfd, RevSum,sizeof(RevSum)); //发送文件哈希值
				if(lens != sizeof(RevSum)) return -1;
				TCPSendFile(ModInfo.StationName,clientfd);//文件传
				usleep(10);
				bzero(RecvBuf,sizeof(RecvBuf));
				TCPBlockRead(clientfd, RecvBuf, 10);//接受服务器响应信息，确认发送的文件是否正确
#ifdef DEBUG
				printf("mod %s\n",RecvBuf);
#endif
				usleep(10);
/*
 * 接收服务器的响应，如果发送个服务器的数据文件正确，则退出发送，否则，则发送两次，再有错误则也退出。
 */
				if(strcmp(RecvBuf,"RecvRight") == 0)  
				{
					strcat(rmmbuf,fns);
#ifdef DEBUG
					printf("%s\n",rmmbuf);
#endif
					system(rmmbuf);//删除数据文件
					break;
				}
				if(strcmp(RecvBuf,"RecvError") == 0)
				{
					DataFlag++;
					if(DataFlag >= 2) //发送错误的次数超过2次
					{
						DataFlag = 1;
						break;
					}
				}
				if(strlen(RecvBuf) < 9)
				{
					strcat(rmmbuf,fns);
#ifdef DEBUG
					printf("%s\n",rmmbuf);
#endif
					system(rmmbuf);//删除数据文件
					break;	
				}
			}
			break;
		case 2 :	//遥控动作,接受文件
			while(1)
			{
				TCPBlockRead(clientfd,  Sha1sum, sizeof(Sha1sum));//接受文件哈希值

				TCPRecvFile(clientfd,"/sdcard/",fnns);//接收调整动作文件
#ifdef DEBUG
				printf("fnns %s \n",fnns);
#endif
				GetSha1Sum(fnns,Sha2sum);
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha2sum[i]);
				printf("\n");
#endif
				usleep(20);
/*
 * 检测文件摘要文件是否匹配
 *##############################################################################################
 */
				for(i=0;i<20;i++)
				{
					if(Sha1sum[i] == Sha2sum[i])
					{
						if(i==19)
						{
							TCPWrite(clientfd,"RecvRight",10);
							RomeFlag = 5;
						}
					}
					else
					{
						TCPWrite(clientfd,"RecvError",10);
						remove(fnns);
						RomeFlag=2;//遥控动作暂时这样处理
						break;
					}
				}

/*
 *#############################################################################################
 */
				bzero(Sha1sum, sizeof(Sha1sum));
				bzero(Sha2sum, sizeof(Sha2sum));
				if(RomeFlag == 5) { RomeFlag =1 ;break;}
				if(RomeFlag == 2) { RomeFlag =1 ;break;} //也是一样的处理方式
				usleep(200);
			}
			break;
		case 3:   //远程更新
			while(1)
			{
				TCPBlockRead(clientfd,  Sha1sum, sizeof(Sha1sum));//接受文件哈希值
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha1sum[i]);
				printf("\n");
#endif
				bzero(fnns,sizeof(fnns));
				TCPRecvFile(clientfd,"/sdcard/",fnns);//接收远程更新文件
#ifdef DEBUG
				printf("fnns %s \n",fnns);
#endif
				//由fnns可以知道是那个文件，则这里需要修改配置文件module.ini
				GetSha1Sum(fnns,Sha2sum);
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha1sum[i]);
				printf("\n");
#endif
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha2sum[i]);
				printf("\n");
#endif
				for(i=0;i<20;i++)
				{
					if(Sha1sum[i] == Sha2sum[i])
					{
						if(i==19)
						{
							TCPWrite(clientfd,"RecvRight",10);
							chmod(fnns,0766);//添加可执行权限
							YKFlag = 5;
						}
					}
					else
					{
						TCPWrite(clientfd,"RecvError",10);
						remove(fnns);//删除错误的文件
						YKFlag=2;
						break;
					}
				}
				bzero(Sha1sum, sizeof(Sha1sum));
				bzero(Sha2sum, sizeof(Sha2sum));
				if(YKFlag == 5) { YKFlag =1 ;break;}
				if(YKFlag == 2) { YKFlag =1 ;break;} //
				usleep(200);
			}
			break;
		case 4: //接受配置文件
			while(1)
			{
				TCPBlockRead(clientfd,  Sha1sum, sizeof(Sha1sum));//接受文件哈希值
				TCPRecvFile(clientfd,"/etc/",fnns);//接收远程更新文件
#ifdef DEBUG
				printf("fnns %s \n",fnns);
#endif
				GetSha1Sum(fnns,Sha2sum);
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha1sum[i]);
				printf("\n");
#endif
#ifdef DEBUG
				for(i=0;i<20;i++)
					printf("%02x", Sha2sum[i]);
				printf("\n");
#endif
				usleep(20);
				for(i=0;i<20;i++)
				{
					if(Sha1sum[i] == Sha2sum[i])
					{
						if(i==19)
						{
							TCPWrite(clientfd,"RecvRight",10);
							chmod(fns,0766);//添加可执行权限							
							YKFlag = 5;
						}
					}
					else
					{
						TCPWrite(clientfd,"RecvError",10);
						remove(fns);//删除错误的文件
						YKFlag++;
						break;
					}
				}
				bzero(Sha1sum, sizeof(Sha1sum));
				bzero(Sha2sum, sizeof(Sha2sum));
				if(YKFlag == 5) { YKFlag =1 ;break;}
				if(YKFlag == 2) { YKFlag =1 ;break;}
				usleep(200);
	
			}
			break;
		default:
			break;
	}
	free(ModInfo.ModServion_maincomm);
	free(ModInfo.ModServion_dataserver);
	free(ModInfo.ModServerIp);
	return 1;
}
