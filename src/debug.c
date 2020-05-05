/*------------------------------------------------------------
* FileName: debug.c
* Author: linhb
* Date: 2014-12-02
------------------------------------------------------------*/
#include <posapi.h>
#include <posapi_all.h>
#include <posapi_s80.h>
#include "debug.h"

static unsigned int debugPort = 0;


#ifdef APP_DEBUG

#define LEN_DBGDATA         1024    // Maximum debug data length
#define MAX_CHARS           5       // Max characters per line in debug usage

static void GetBaseName(const uchar *pszFullPath, uchar *pszBaseName);

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void DispHexMsg(const uchar *pszTitle, const uchar *psMsg, uint uiMsgLen, short nTimeOut)
{
	uint	i, iLineNum;
	GUI_PAGELINE	stBuff[100];
	GUI_PAGE		stHexMsgPage;

	memset(stBuff, 0, sizeof(stBuff));

	iLineNum = 0;
	// Format message
	uiMsgLen = MIN(uiMsgLen, LEN_DBGDATA);
	for (i=0; i<uiMsgLen; i+=MAX_CHARS)
	{
		if (uiMsgLen-i<MAX_CHARS)
		{
			DispHexLine(stBuff[iLineNum].szLine, i, psMsg+i, uiMsgLen-i);
		}
		else
		{
			DispHexLine(stBuff[iLineNum].szLine, i, psMsg+i, MAX_CHARS);
		}
		stBuff[iLineNum++].stLineAttr = gl_stLeftAttr;
	}   // end of for (pszBuff=

	Gui_CreateInfoPage(pszTitle, gl_stTitleAttr, stBuff, iLineNum, &stHexMsgPage);
	// Display message
	Gui_ClearScr();
	Gui_ShowInfoPage(&stHexMsgPage, FALSE, USER_OPER_TIMEOUT);
}

// print a line as hexadecimal format
int DispHexLine(uchar *pszBuffInOput, uint uiOffset, const uchar *psMsg, uint uiMsgLen)
{
	uint	i;
	uchar	*p = pszBuffInOput;

	// Print line information
	pszBuffInOput += sprintf((char *)pszBuffInOput, "%04Xh:", uiOffset);

	for (i=0; i<uiMsgLen; i++)
	{
		pszBuffInOput += sprintf((char *)pszBuffInOput, " %02X", psMsg[i]);
	}
	for (; i<MAX_CHARS; i++)
	{   // append blank spaces, if needed
		pszBuffInOput += sprintf((char *)pszBuffInOput, "   ");
	}

	return (pszBuffInOput-p);
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
// For Debug use, display file name and line
void DispAssert(const uchar *pszFileName, ulong ulLineNo)
{
	uchar	szFName[30];
	uchar	szBuff[200];

	GetBaseName(pszFileName, szFName);
	sprintf(szBuff, "FILE:%.11s\nLINE:%ld", szFName, ulLineNo);

	Gui_ClearScr();
	PubLongBeep();
	Gui_ShowMsgBox("Assert Failure", gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, -1, NULL);
}

// get basename of a full path name
void GetBaseName(const uchar *pszFullPath, uchar *pszBaseName)
{
	uchar	*pszTmp;

	*pszBaseName = 0;
	if (!pszFullPath || !*pszFullPath)
	{
		return;
	}

	pszTmp = (uchar *)&pszFullPath[strlen((char *)pszFullPath)-1];
	while( pszTmp>=pszFullPath && *pszTmp!='\\' && *pszTmp!='/' )
	{
		pszTmp--;
	}
	sprintf((char *)pszBaseName, "%s", (char *)(pszTmp+1));
}

#endif

void setDebugPort(unsigned int port) {
	debugPort = port;
}

// bool g_bConn=false;
// int g_hnd=-1;
void send_log(char *a_szTxt)
{
#ifdef  APP_DEBUG
	unsigned char ucRet;

	//	ucRet=PortOpen(0,(unsigned char*)"230400,8,n,1");
	ucRet = PortOpen(debugPort, (unsigned char*)"115200,8,n,1");
	if (ucRet != 0)
		return;

	PortSends(debugPort, (unsigned char*)a_szTxt, strlen(a_szTxt));

	PortClose(debugPort);

	// 	if(!g_bConn)
	// 	{
	// 		EthSet("192.168.103.199", "255.255.255.0", "192.168.103.254",(char*)NULL);
	// 		g_bConn=true;
	// 	}
	// 
	// 	if(g_hnd<0)
	// 	{
	// 		g_hnd=NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	// 		NET_SOCKADDR socket_host;
	// 		SockAddrSet(&socket_host, "192.168.103.200", 5070);
	// 		if(NetConnect(g_hnd,&socket_host,sizeof(socket_host))<0)
	// 		{
	// 			NetCloseSocket(g_hnd);
	// 			g_hnd=-1;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if(NetSend (g_hnd, a_szTxt,strlen(a_szTxt), 0)<0)
	// 		{
	// 			NetCloseSocket(g_hnd);
	// 			g_hnd=-1;
	// 		}
	// 	}

#endif //  APP_DEBUG
}

void prepare_header(char *a_szDest, char*a_szFile, long a_lLine)
{
	char *p = (char*)NULL, *p1 = (char*)NULL;

	p = a_szFile;
	while (1)
	{
		p1 = strchr(p, '\\');
		if (p1 == NULL)
			break;
		p = p1 + 1;
	}

	sprintf(a_szDest, "F:%s|L:%05ld| ", p, a_lLine);
}


char *log_formater(char* a_szFmt, ...)
{
	static char szMessage[0x2000 + 1];
	va_list xArgList;

	va_start(xArgList, a_szFmt);
	vsprintf(szMessage, a_szFmt, xArgList);
	va_end(xArgList);

	strcat(szMessage, "\r\n");

	return (szMessage);
}

void log_printf(char*a_szFile, long a_lLine, char* a_szMsg)
{
	//WARNING!!! DON'T LOG VERY LONG TEXT HERE, CAUSES TERMINAL TO CRASH, USE LOGDIRECT
	char szMessage[0x2080 + 1];

	memset(szMessage, 0, sizeof(szMessage));
	prepare_header(szMessage, a_szFile, a_lLine);

	strncat(szMessage, a_szMsg, sizeof(szMessage) - strlen(szMessage) - 1);

	send_log(szMessage);
}


void log_hex_printf(char*a_szFile, long a_lLine, char* a_szMessage, unsigned char* a_ucBuffer, short a_sBufferLen)
{
	char szMsg[100];
	int i;

	memset(szMsg, 0, sizeof(szMsg));
	prepare_header(szMsg, a_szFile, a_lLine);
	strcat(szMsg, "\r\n");
	strcat(szMsg, a_szMessage);
	//    strcat (szMsg, "\r\n");
	send_log(szMsg);

	for (i = 0; i < a_sBufferLen; i++)
	{
		//        if (((i + 1) % 16) == 0)
		if ((i % 16) == 0)
			send_log("\r\n");

		memset(szMsg, 0, sizeof(szMsg));
		sprintf(szMsg, "%02X ", (char)a_ucBuffer[i]);
		send_log(szMsg);
	}

	send_log("\r\n");
	return;
}

void logDirect(char* tag, char* message)
{	
	send_log(tag);
	send_log(message);
	send_log("\r\n");
}

#ifdef _WIN32
void cDebugPortStr() {

}

void cDebugPortHex() {

}

#endif // _WIN32