
/****************************************************************************
NAME
    SxxComSSL.h

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    Kevin_Wu    2016.08.26      - created
****************************************************************************/

#ifndef _SXXCOMSSL_H
#define _SXXCOMSSL_H

typedef struct _SSLCon
{
	int sslSock;
	char szCA[25];
	char szCert[25];
    char szKey[25];
} SSLCon;

int SxxSSLObjInit(char *ca, char *cert, char *key);

int SxxSSLGetSocket();

int SxxSSLConnect(const char *pszIP, short sPort, int iTimeout);

int SxxSSLConnectCheck(int iTimeoutSec);

int SxxSSLTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec);

int SxxSSLRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);

int SxxSSLClose(void);

int SxxSSLSingleProcess();

#endif
