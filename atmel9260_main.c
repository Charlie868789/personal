/*************************************************************
//基站测试仪主机系统主函数，完成通信程序的调用
//作者：陈岭
//单位：cuit  
//时间:2010-11-10
*************************************************************/
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
#include "socket.h"
#include "sha1sum.h"
#include "atmel9260.h"
#include "atmel9260file.h"
#define DEBUG 1
int main(int argc,char *argv[])
{
    unsigned long gprsbusy;
    int com;
    bool ret;
    system("/bin/data_right"); // 给软件加上数字签名
    gprsbusy = GetPrivateProfileUL("Module", "GprsBusy", 0, "/etc/Module.ini");
#ifdef DEBUG
    printf("gprsbusy number is :%ld\n",gprsbusy);
#endif
    if(gprsbusy == 0)      //判断GPRS是否在忙状态
    {
	printf("gprs busy wait next time \n");
	return 0;
    }
    //如果在不在忙状态，则告诉系统占用GPRS模块
    ret = WritePrivateProfileString("Module","GprsBusy","0\n","/etc/Module.ini");
#ifdef DEBUG
    printf("write module number :%d\n",ret);
#endif
    com = atmel9260_communication(); //与服务器建立通信机制
    if(com < 0)
    {
#ifdef DEBUG
        printf("atmel_client_communication() foucntion error");
#endif
	ret = WritePrivateProfileString("Module","GprsBusy","1\n","/etc/Module.ini");//释放GPRS网络
        exit(1);
    }
    ret = WritePrivateProfileString("Module","GprsBusy","1\n","/etc/Module.ini");//释放GPRS网络
#ifdef DEBUG
    printf("run end\n");
#endif
    return 0;
}
