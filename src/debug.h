/*------------------------------------------------------------
* FileName: debug.h
* Author: linhb
* Date: 2014-12-02
------------------------------------------------------------*/
#ifndef DEBUG_H
#define DEBUG_H

/***************************************************************************************
For Debug use
***************************************************************************************/
#include "global.h"
#ifdef APP_DEBUG

#define _DEBUG


/****************************************************************************
 Function:      Show HEX message on screen.
 Param In:		pszTitle    Title of the message.
				psMsg       Pointer of message to be displayed.
				uiMsgLen    Bytes of the message.
				nTimeOut    Seconds between user interaction.
 Param Out:
				none
 Return Code:
				none
 Description:
****************************************************************************/
void DispHexMsg(const uchar *pszTitle, const uchar *psMsg, uint uiMsgLen, short nTimeOut);

/****************************************************************************
 Function:      print a line as hexadecimal format
 Param In:		pszBuffInOput		data buffer
				uiOffset    offset of the data buffer
				psMsg		Pointer of message to be displayed.
				uiMsgLen    Bytes of the message.
 Param Out:
				pszBuffInOput
 Return Code:
				none
 Description:
****************************************************************************/
int DispHexLine(uchar *pszBuffInOput, uint uiOffset, const uchar *psMsg, uint uiMsgLen);

/****************************************************************************
 Function:      For Debug use, display file name and line
 Param In:		pszFileName		file name
				ulLineNo		line no
 Param Out:
				none
 Return Code:
				none
 Description:
****************************************************************************/
void DispAssert(const uchar *pszFileName, ulong ulLineNo);

    // debug macro for boolean expression
#define _POS_DEBUG_WAIT		15
#define PubASSERT(expr) if( !(expr) ){DispAssert((uchar *)__FILE__, (ulong)(__LINE__));}

// print string debug information

// print string debug information
#define PubTRACE0(sz) {ScrCls();ScrGotoxy(0,0);Lcdprintf(sz);PubWaitKey(_POS_DEBUG_WAIT);}
#define PubTRACE1(sz, p1) {ScrCls();ScrGotoxy(0,0);Lcdprintf(sz, p1);PubWaitKey(_POS_DEBUG_WAIT);}	   
#define PubTRACE2(sz, p1, p2) {ScrCls();ScrGotoxy(0,0);Lcdprintf(sz, p1, p2);PubWaitKey(_POS_DEBUG_WAIT);}
#define PubTRACE3(sz, p1, p2, p3) {ScrCls();ScrGotoxy(0,0);Lcdprintf(sz,p1,p2,p3);PubWaitKey(_POS_DEBUG_WAIT);}

#define PubTRACEHEX(t, s, l)    DispHexMsg((t), (s), (l), _POS_DEBUG_WAIT)

#else /* _POS_DEBUG */

#define PubASSERT(expr)
#define PubTRACE0(sz)
#define PubTRACE1(sz, p1)
#define PubTRACE2(sz, p1, p2)
#define PubTRACE3(sz, p1, p2, p3)
#define PubTRACEHEX(t, s, l)

#endif /* _POS_DEBUG */


void setDebugPort(unsigned int port);


char* log_formater(char* a_szFmt, ...);
void log_printf(char*a_szFile, long a_lLine, char* a_szMsg);
void log_hex_printf(char*a_szFile, long a_lLine, char* a_szMessage, unsigned char* a_ucBuffer, short a_sBufferLen);


#ifdef _DEBUG
#define LOG_PRINTF(a)           log_printf (__FILE__,__LINE__, log_formater a)
#define LOG_HEX_PRINTF(a,b,c)   log_hex_printf(__FILE__,__LINE__,a,b,c)
#else
#define LOG_PRINTF(a)
#define LOG_HEX_PRINTF(a,b,c)
#endif

void logDirect(char* tag, char* message);

#endif
