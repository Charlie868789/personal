#ifndef ATMEL9260_H_
#define ATMEL9260_H_

extern struct ModuleInfo
{
	unsigned char *ModServion_maincomm;//串口软件模块版本
	unsigned char *ModServion_dataserver;//服务器通信软件模块版本
	unsigned char *ModServerIp;
	unsigned char *StationName;
}ModIn;
struct ModuleInfo ModInfo;
static int DataFlag=1;//记录发送文件的次数
static int RomeFlag=1;
static int YKFlag=1;
#endif
