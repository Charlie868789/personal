/*
 * atmel9260file.h
 *
 *  Created on: 2011-7-14
 *      Author: root
 */

#ifndef ATMEL9260FILE_H_
#define ATMEL9260FILE_H_
#include <stdbool.h>
#include "atmel9260.h"

int WritePrivateProfileString(
			char *lpAppName,//是INI文件中的一个字段名
			char *lpKeyName,//是lpAppName下的一个键名,通俗讲就是变量名
			char *lpString,//是键值,也就是变量的值,不过必须为LPCTSTR型或CString型的
			char *lpFileName//是完整的INI文件名
	);
int GetPrivateProfileString(
			char *lpAppName, //是INI文件中的一个字段名
			char *lpKeyName, //是lpAppName下的一个键名,通俗讲就是变量名
			char *lpDefault,  //如果INI文件中没有前两个参数指定的字段名或键名,则将此值赋给变量
			char *lpReturnedString, //接收INI文件中的值的CString对象,即目的缓存器
			unsigned long nSize, //目的缓存器的大小
			char *lpFileName //是完整的INI文件名
	);

int GetPrivateProfileUL(
			char *lpAppName,  //是INI文件中的一个字段名
			char *lpKeyName, //是lpAppName下的一个键名,通俗讲就是变量名
			unsigned long nDefault, //如果INI文件中没有前两个参数指定的字段名或键名,则将此值赋给变量
			char *lpFileName  //是完整的INI文件名
	);
int GetPrivateKeyName(
			char *lpAppName, //是INI文件中的一个字段名
			char *lpKeyName, //是lpAppName下的一个键名,通俗讲就是变量名
			char *lpDefault,  //如果INI文件中没有前两个参数指定的字段名或键名,则将此值赋给变量
			char *lpReturnedString, //接收INI文件中的值的CString对象,即目的缓存器
			unsigned long nSize, //目的缓存器的大小
			char *lpFileName //是完整的INI文件名
	);

void LTruncate(
		   char *pString,
		   char *szFill
	);

void RTruncate(
		  char *pString,
		  char *szFill
	);

int ReadModinfo(struct ModuleInfo ModInfo);//读模块配置信息

#endif

