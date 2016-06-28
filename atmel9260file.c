/*
 * readfile.c
 *
 *  Created on: 2011-7-14
 *      Author: root
 */
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
#include "atmel9260file.h"
#define MAX_STRING_LEN 1024
int ReadModinfo(struct ModuleInfo Mod)//读模块配置信息
{
	//Mod.ModServion_maincomm
	 //得到maincomm程序版本
	if( GetPrivateProfileString("Module", "Servionmaincom","",Mod.ModServion_maincomm,1024,"/etc/Module.ini") < 0)
		return -1;
	 //得到dataserver程序版本
	if( GetPrivateProfileString("Module", "Serviondataserver","",Mod.ModServion_dataserver,1024,"/etc/Module.ini") <=0)
		return -1;
	//读服务器IP地址
	if( GetPrivateProfileString("Module", "ServerIP","",Mod.ModServerIp,1024,"/etc/Module.ini") <=0)
		return -1;
	//基站名称
	if( GetPrivateProfileString("Module", "StaName","",Mod.StationName,1024,"/etc/Module.ini") <=0)
		return -1;
	return 1;
}

int WritePrivateProfileString(char *lpAppName,char *lpKeyName,char *lpString,char *lpFileName)
{
	char *token;
	FILE *stream;
	char *newstream, *beforestream, *afterstream, *addstream;
	char szBuffer[MAX_STRING_LEN];
	char szAppSeps[]="[";
	char szKeySeps[]="=";
	unsigned long nLen;
	unsigned long fileseek, operateline_len, file_len, newfile_len;
	int ret;

	int bSectionFound = 0;
	int bKeyFound =0;

	nLen = (strlen(lpKeyName) + strlen(szKeySeps) + strlen(lpString));
	addstream = (char *)malloc(nLen +1);
	addstream[nLen] = 0;
	strcpy(addstream, lpKeyName);
	strcat(addstream, szKeySeps);
	strcat(addstream, lpString);

	if((stream = fopen( lpFileName, "r+t" )) == NULL )
	{
		return -1;
	}

	while(!feof(stream))
	{
		fgets( szBuffer, MAX_STRING_LEN, stream );
		operateline_len = strlen(szBuffer);
		fileseek = ftell(stream);

		LTruncate(szBuffer, " \t");
		if(!bSectionFound)
	    {
			token = strtok(szBuffer, szAppSeps);
			if(token == szBuffer)
	                    continue;
			LTruncate(token, " \t");
			RTruncate(token, " ]\t\r\n");
			if(strcmp(token, lpAppName))
				continue;
			else
				bSectionFound = 1;
	    }
		else
		{
			nLen = strlen(szBuffer);
			token = strtok(szBuffer, szKeySeps);
			if(strlen(token) == nLen)
			{
	            		token = strtok(szBuffer, szAppSeps);
	            		if(token != szBuffer)
	            			break;
	            		else
	            			continue;
			}
			LTruncate(szBuffer, " \t");
			RTruncate(szBuffer, " \t");
			if(strcmp(szBuffer, lpKeyName))
			{
				continue;
			}
			else
			{
				bKeyFound = 1;

				break;
			}
		}
	}

	if(bSectionFound && bKeyFound)
	{
		if((ret =fseek(stream, 0, SEEK_END)) != 0)
		{
			fclose(stream);
			return -1;
		}
		file_len = ftell(stream);
		if((fileseek < operateline_len) || (file_len < fileseek))
		{
			fclose(stream);
			return -1;
		}
		beforestream = (char *)malloc(fileseek - operateline_len +2);
		beforestream[fileseek - operateline_len + 1] = 0;
		rewind(stream);
		fread(beforestream, (fileseek - operateline_len + 1), 1, stream);
		if((ret =fseek(stream, fileseek, SEEK_SET)) != 0)
		{
			fclose(stream);
			return -1;
		}
		afterstream = (char *)malloc(file_len + 1 - fileseek);
		afterstream[file_len - fileseek] = 0;
		fread(afterstream, (file_len - fileseek), 1, stream);
		newfile_len = (strlen(beforestream) + strlen(addstream) + strlen(afterstream));
		newstream = (char *)malloc(newfile_len + 1);
		newstream[newfile_len] = 0;
		strncpy(newstream,beforestream, (strlen(beforestream) - 1));
		newstream[strlen(beforestream)] = 0;
		strcat(newstream,addstream);
		strcat(newstream,afterstream);

		if(file_len > newfile_len)
			ftruncate(fileno(stream), newfile_len);
		rewind(stream);
		if((ret = fwrite(newstream, newfile_len, 1, stream)) != 1)
		{
			fclose(stream);
			return -1;
		}
		if((ret = fflush(stream)) != 0)
		{
			fclose(stream);
			return -1;
		}
	}
	fclose(stream);
	return 1;
}

/*read *.INI file string*/
int GetPrivateProfileString(char *lpAppName, char *lpKeyName,char *lpDefault,char *lpReturnedString,unsigned long nSize, char *lpFileName)
{
	char *token;
        FILE *stream;
        char szBuffer[MAX_STRING_LEN];
        char szAppSeps[]="[";
        char szKeySeps[]="=";
        unsigned long nLen;

        bool bSectionFound = false;
        bool bKeyFound = false;


        if((stream = fopen( lpFileName, "rt" )) == NULL )
        {
        	  return 0; /* open failure */
        }

        while(!feof(stream))
        {
    	         fgets( szBuffer, MAX_STRING_LEN, stream );/*read a line, stop till 1023 chars or "\n"*/
       //          printf("%s\n",szBuffer);
                LTruncate(szBuffer, " \t");/*cut off the first space an TAB*/

                if(!bSectionFound)
                {
                        token = strtok(szBuffer, szAppSeps);/*prase the string szBuffer into tokens by szAppAeps*/

                        if(token == szBuffer)
                                continue;

                        LTruncate(token, " \t");
                        RTruncate(token, " ]\t\r\n");
                        if(strcmp(token, lpAppName))
                                continue;  /* find next Section */
                        else
                                bSectionFound = true;
                }
                else
                {
        	          nLen = strlen(szBuffer);
                        token = strtok(szBuffer, szKeySeps);
                        if(strlen(token) == nLen)
                        {
                        	/* judge during the area of the Section */
                                token = strtok(szBuffer, szAppSeps);
                                if(token != szBuffer)
                                        break;                /* out of，quit */
                                else
                                        continue;        /* during，continue */
                        }

                        LTruncate(szBuffer, " \t");
                        RTruncate(szBuffer, " \t");

                        if(strcmp(szBuffer, lpKeyName))
                        {
                                continue;
                        }
                        else
                        {
                        	/* find */
                                bKeyFound = true;
                                token = strtok(NULL, szKeySeps);

                                LTruncate(token, " \t");
                                RTruncate(token, " ;,\t\n\r");
                                nLen = strlen(token);

                                if(nLen <  nSize)
                                {
                                        strcpy((char*)lpReturnedString, token);
                                }
                                else
                                {
                                        strncpy((char*)lpReturnedString, token, nSize-1);
                                        lpReturnedString[nSize-1] = 0;
                                }
                                break;
                        }
                }
        }

        fclose(stream);
        if(bSectionFound && bKeyFound)
        {
        	  return nLen;
        }

        if(lpDefault!=NULL)
        {
                nLen = strlen(lpDefault);

                if(nLen < nSize)
                {
                        strcpy((char*)lpReturnedString, lpDefault);
                }
                else
                {
                        strncpy((char*)lpReturnedString, lpDefault, nSize-1);
                        lpReturnedString[nSize-1] = 0;
                }
        }
        return 0;
}

/*read *.INI file unsigned long*/
int GetPrivateProfileUL(char *lpAppName,  char *lpKeyName,unsigned long nDefault, char *lpFileName)
{
		char *token;
        FILE *stream;
        char szBuffer[MAX_STRING_LEN];
        char szAppSeps[]="[";
        char szKeySeps[]="=";
        char *lpString;
        unsigned long nLen;
        unsigned long ret_val = nDefault;

        int bSectionFound = 0;
        int bKeyFound = 0;

        if((stream = fopen( lpFileName, "rt" )) == NULL )
        {
                return 0; /* open failure */   //cx
        }

        while(!feof(stream))
        {
                fgets( szBuffer, MAX_STRING_LEN, stream );/*read a line, stop till 1023 chars or "\n"*/
                LTruncate(szBuffer, " \t");/*cut off the first space an TAB*/
                if(!bSectionFound)
                {
                        token = strtok(szBuffer, szAppSeps);/*prase the string szBuffer into tokens by szAppAeps*/
                        if(token == szBuffer)
                                continue;

                        LTruncate(token, " \t");
                        RTruncate(token, " ]\t\r\n");
                        if(strcmp(token, lpAppName))
                                continue;  /* find next Section */
                        else
                                bSectionFound = 1;
                }
                else
                {
                        nLen = strlen(szBuffer);
                        token = strtok(szBuffer, szKeySeps);

                        if(strlen(token) == nLen)
                        {
                        	/* judge during the area of the Section */
                                token = strtok(szBuffer, szAppSeps);
                                if(token != szBuffer)
                                        break;                /* out of，quit */
                                else
                                        continue;        /* during，continue */
                        }


                        LTruncate(szBuffer, " \t");
                        RTruncate(szBuffer, " \t");

                        if(strcmp(szBuffer, lpKeyName))
                        {
                                continue;
                        }
                        else
                        {
                        	/* find */
                                bKeyFound = 1;

                                token = strtok(NULL, szKeySeps);

                                LTruncate(token, " \t");
                                RTruncate(token, " ;,\t\n\r");

                                nLen = strlen(token);
                                lpString = (char*)malloc(nLen+1);
                                strcpy(lpString, token);
                                break;
                        }
                }
        }

        fclose(stream);
        if(!(bSectionFound && bKeyFound))
        {
        		return 0;
        }
        ret_val = strtoul(lpString,NULL,0);

        free(lpString);
        return ret_val;
}

void LTruncate(char *pString, char *szFill)
{
        int i, len;
        char *pTemp;

        len = strlen(pString);

        pTemp = (char*)malloc(len+1);
        strcpy(pTemp,pString);

        for(i=0;i<len;i++)
        {
                if(!strchr(szFill, pTemp[i]))
                        break;
        }

        strcpy(pString, pTemp+i);
        free(pTemp);
}

void RTruncate(char *pString, char *szFill)
{
        int i, len;
        len = strlen(pString);

        for(i=len; i>0; i--)
        {
                if(!strchr(szFill, pString[i-1]))
                        break;
        }

        pString[i] = 0;
}


