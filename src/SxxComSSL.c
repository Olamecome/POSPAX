
/****************************************************************************
NAME
    SxxComSSL.c

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    Kevin_Wu    2016.08.26      - created
****************************************************************************/

#include "global.h"

#define DEF_TIME_OUT (20 * 1000)

//------------------------- Static Variable Area -----------------------------

SSLCon sslCon = {-1, {0}, {0}, {0}};

SSL_BUF_T  gCaCertBuffT = {NULL, 0};
SSL_BUF_T  gPosCertBuffT = {NULL, 0};
SSL_BUF_T  gPosPriKeyBuffT = {NULL, 0};

int conTimeOut = 0;
int sndTimeOut = 0;
int rcvTimeOut = 0;

char gCACertBin[2048] = {0};
char gPosCertBin[2048] = {0};
char gPosPriKeyBin[2048] = {0};


//----------------------------------------------------------------

static char Bcd2Byte(char bVal)
{
	return ((bVal >> 4) * 10) + (bVal & 0x0F);
}

static DWORD Bcd2Int(const char* pbBuf, char bLen)
{
	DWORD dwVal = 0;

	while(bLen--)
	{
		dwVal *= 100;
		dwVal += Bcd2Byte(*pbBuf++);
	}

	return dwVal;
}

static int net_connect(char *remote_addr, short remote_port, short local_port, long flag)
{
	int iSocket;
	int iError; 
	struct net_sockaddr addr;

	//1. Create TCP Socket
	iSocket = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	if (iSocket < 0)
	{
		return iSocket;	// FAIL
	};
	
	//2. Set IP address
	memset(&addr, 0, sizeof(addr));
	
	iError = SockAddrSet(&addr, remote_addr, remote_port);
	if (iError < 0)
	{
		NetCloseSocket(iSocket);
		return iError;
	}
	
	//3. Set timeout time ms
	Netioctl(iSocket, CMD_TO_SET, (conTimeOut > 0) ? conTimeOut : DEF_TIME_OUT);
	
	//4. Connect
	iError = NetConnect(iSocket, &addr, sizeof(addr));
	if (iError < 0)
	{	
		NetCloseSocket(iSocket);

		switch (iError)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			case NET_ERR_RST:
				return -ERR_SSL_NET;
			default:
				return iError;
		}
	}

	return iSocket;
}

static int net_send(int net_hd, void *buf, int size)
{
	int iRet;
	
	Netioctl(net_hd, CMD_TO_SET, (sndTimeOut > 0) ? sndTimeOut : DEF_TIME_OUT);
	
	iRet = NetSend(net_hd, buf, size, 0);

	if (iRet < 0)
	{
		switch (iRet)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			default:
				return iRet;
		}
	}

	return iRet;
}

static int net_recv(int net_hd, void *buf, int size)
{
	int iBytesReceived = 0;
	
	Netioctl(net_hd, CMD_TO_SET, (rcvTimeOut > 0) ? rcvTimeOut : DEF_TIME_OUT);
	
	iBytesReceived = NetRecv(net_hd, buf, size, 0);
	
	if (iBytesReceived < 0)
	{
		switch (iBytesReceived)
		{
			case NET_ERR_TIMEOUT:
				return -ERR_SSL_TIMEOUT;
			default:
				return iBytesReceived;
		}
	}

	return iBytesReceived;
}

static int net_close(int net_hd)
{	
	NetCloseSocket(net_hd);
	
	return 0;
}

static int ReadSysTime(SYSTEM_TIME_T *t)
{
	uchar abDateTime[7];

	GetTime(abDateTime);
	t->year = 2000 + (unsigned short)Bcd2Int(abDateTime, 1);
	t->month = (char)Bcd2Int(&abDateTime[1], 1);
	t->day = (char)Bcd2Int(&abDateTime[2], 1);
	t->hour = (char)Bcd2Int(&abDateTime[3], 1);
	t->min = (char)Bcd2Int(&abDateTime[4], 1);
	t->sec = (char)Bcd2Int(&abDateTime[5], 1);
	t->zone.hour = 8;
	t->zone.min = 0;
	
	return 0;
}

static int Random(unsigned char *buf, int len)
{
	char abRandom8[8];
	int wLength;
	int i = 0;

	while (len > 0)
	{
		PciGetRandom(abRandom8);
		
		wLength = len > 8 ? 8 : len;
		
		memcpy(buf, abRandom8, wLength);
		
		len -= wLength;
		buf += wLength;
	}
	
	return 0;
}

static unsigned long Time(unsigned long *t)
{
	char abTime[7];
	char bHour, bMinute, bSecs;
	DWORD dwCurrentTimeSecs;

	// Return elasped time in current day in seconds
	GetTime(abTime);
	
	bHour = (char)Bcd2Int(&abTime[3], 1);
	bMinute = (char)Bcd2Int(&abTime[4], 1);
	bSecs = (char)Bcd2Int(&abTime[5], 1);

	dwCurrentTimeSecs = (DWORD)(bSecs + 60 * (bMinute + 60 * bHour));

	return dwCurrentTimeSecs;
}

static char *ReasonStr(CERT_INVAL_CODE eReason)
{
	char *szDescr[] =
	{
		"CERT_BAD",
		"CERT_TIME",
		"CERT_CRL",
		"CERT_SIGN",
		"CERT_CA_TIME",
		"CERT_CA_CRL",
		"CERT_CA_SIGN",
		"CERT_MAC",
		"CERT_MEM",
		"CERT_ISSUER",
	};

	if ( eReason > 0 && eReason <= (sizeof(szDescr)/sizeof(char*)) )
	{
		return szDescr[eReason - 1];
	}

	return "N/A";
}

static int CertAck(CERT_INVAL_CODE eReason)
{
	int iRet;

	return 0; // Ignore all certificate exceptions

	if (eReason == CERT_ISSUER)
	{
		iRet = 0;	//ignore, conintue;
	}
	else
	{
		iRet = -1;
	}

	ScrCls();
	ScrPrint(0, 0, 0, "Ssl Cer Err\n");
	ScrPrint(0, 1, 0, "%d %s", eReason, ReasonStr(eReason));
	
	return iRet;
}

SSL_NET_OPS ssl_ops =
{
	net_connect,
	net_send,
	net_recv,
	net_close
};

SSL_SYS_OPS sys_ops =
{
	ReadSysTime,
	Random,
	Time,
	CertAck
};

//Read file content to buf
int RdFile2Buf(char* fName, char* buf, long fLen)
{
	int iRet = -1;
	int fd = -1;
	
	if((fd = open(fName, O_RDWR)) < 0)
	{
		return iRet;
	}

	if((iRet = read(fd, buf, fLen)) < 0)
	{
		close(fd);
		return iRet;
	}
	
	close(fd);
	
	return iRet;
}

//Get pem content start offset and length
int GetPemFContInfo(char* orgStr, int* startOffset, int* contLen,
	char* preFix, char* tailFix)
{
	char* startPos = NULL;
	char* endPos = NULL;
	char* pos = NULL;
	
	if(orgStr == NULL || preFix == NULL || tailFix == NULL)
	{
		return -1;
	}
	
	if((pos = strstr(orgStr, preFix)) == NULL)
	{
		return -1;
	}

	startPos = pos + strlen(preFix);
	
	*startOffset = (startPos - orgStr);
	
	if((endPos = strstr(orgStr, tailFix)) == NULL)
	{
		return -1;
	}

	*contLen = endPos - startPos;
	
	return *contLen;
}

//Read pem file to buf
//1. Only support read out one cert from a pem file now(not support read out multipule cert 
//		from one pem file)
//2. *pemCont will dynamic alloc in this function, donot forget free it
int RdPemF2Buf(char* pemF, char** pemCont, int* contLen, 
	char* preFix, char* tailFix)
{
	int iRet = -1;
	long fSize = 0;
	char* pfBuf = NULL;
	int startOffset;
	int i = 0;

	if(pemF == NULL || pemCont == NULL || contLen == NULL 
		|| preFix == NULL || tailFix == NULL)
	{
		goto Exit;
	}
	
	if((fSize = filesize(pemF)) < 0)
	{
		goto Exit;
	}
	
	if((pfBuf = malloc(fSize + 1)) == NULL)
	{
		goto Exit;
	}
	
	memset(pfBuf, '\0', fSize + 1);
	
	//1. Read pem all file content to buff
	if((iRet = RdFile2Buf(pemF, pfBuf, fSize)) < 0)
	{
		goto Exit;
	}
	
	//2. Get pem valid content start offset and length
	if(GetPemFContInfo(pfBuf, &startOffset, contLen, preFix, tailFix) 
		< 0)
	{
		goto Exit;
	}

	//3. Copy valid content to buff
	if(pfBuf != NULL && startOffset >= 0 && (*contLen) >= 0)
	{
		*pemCont = malloc((*contLen) + 1);
		if(*pemCont == NULL)
		{
			goto Exit;
		}
		
		memset(*pemCont, 0, (*contLen) + 1);
		
		strncpy(*pemCont, (pfBuf + startOffset) , *contLen);
	}

Exit:	

	if(pfBuf != NULL)
	{
		free(pfBuf);
		pfBuf = NULL;
	}

	return iRet;
}

//Init sslCon Object
int SxxSSLObjInit(char *ca, char *cert, char *key)
{	
	logd((__func__));
	if(ca == NULL || cert == NULL || key == NULL)
	{
		logd(("Not setting a cert"));
		memset(&sslCon, 0, sizeof(sslCon));
		sslCon.sslSock = -1;
		return 0;
	}
	
	memset(&sslCon, 0, sizeof(sslCon));
	sslCon.sslSock = -1;

	strncpy(sslCon.szCA, ca, strlen(ca));
	strncpy(sslCon.szCert, cert, strlen(cert));
	strncpy(sslCon.szKey, key, strlen(key));
	
	return 0;
}

int SxxSSLGetSocket()
{
	return sslCon.sslSock;
}

int SxxSSLConnect(const char *pszIP, short sPort, int iTimeout)
{
	logd((__func__));
	int iRet;
	
	char* pCACertPem = NULL;
	char* pPosCertPem = NULL;
	char* pPosPriKeyPem = NULL;

	int caCertlen = 0;
	int posCertlen = 0;
	int posPrikeylen = 0;
	
	conTimeOut = iTimeout * 1000;
	char params[100] = { 0 };

	/*************************** Function Register *************************/
	//1. SSL net operation set
	SslSetNetOps(&ssl_ops);

	//2. SSL system operation set
	SslSetSysOps(&sys_ops);
	/**********************************************************************/

	/*************************** SSL Socket Create ************************/
	sslCon.sslSock = SslCreate();
	
	if (sslCon.sslSock < 0)
	{
		return sslCon.sslSock;
	}

	logd(("iTimeout==%d", iTimeout));
	if (iTimeout > 0) {
		memset(params, 0, lengthOf(params));
		params[0] = 1;
		SslParaCtl(sslCon.sslSock, SSL_CMD_SET_SSL_NONBLOCK, params, 1);
	}
	/**********************************************************************/

	/*************************** Load Certification ***********************/
	
	//1. Read to buffer from pem file
	if(RdPemF2Buf(sslCon.szCA, &pCACertPem, &caCertlen, "-----BEGIN CERTIFICATE-----",
		"-----END CERTIFICATE-----") < 0)
	{
		//goto Exit;
		logd(("No CA Cert found"));
	}
	
	if(RdPemF2Buf(sslCon.szCert, &pPosCertPem, &posCertlen, "-----BEGIN CERTIFICATE-----",
		"-----END CERTIFICATE-----") < 0)
	{
		//goto Exit;
		logd(("No POS Cert found"));
	}
	
	if(RdPemF2Buf(sslCon.szKey, &pPosPriKeyPem, &posPrikeylen, "-----BEGIN RSA PRIVATE KEY-----",
		"-----END RSA PRIVATE KEY-----") < 0)
	{
		logd(("No POS Private Cert found"));	
	}


	if (!pCACertPem && !pPosCertPem && !pPosPriKeyPem) {
		logd(("No certificate supplied, ignoring all"));
		SslCertsSet(sslCon.sslSock, NULL, 0, NULL, NULL, 0, NULL);
		goto CONNECT;
	}


	logd(("Decoding certificates"));
	//2. Decode
	iRet = SslDecodePem((char *)pCACertPem, caCertlen, gCACertBin, 2048);
	if (iRet <= 0)
	{
		ScrPrint(0, 2, 0, "CA Cert error");
		goto Exit;
	}

	gCaCertBuffT.size = iRet;
	gCaCertBuffT.ptr = gCACertBin;

	iRet = SslDecodePem((char *)pPosCertPem, posCertlen, gPosCertBin, 2048);
	if (iRet <= 0)
	{
		ScrPrint(0, 2, 0, "Pos Cert error");
		goto Exit;
	}

	gPosCertBuffT.size = iRet;
	gPosCertBuffT.ptr = gPosCertBin;

	iRet = SslDecodePem((char *)pPosPriKeyPem, posPrikeylen, gPosPriKeyBin, 2048);
	if (iRet <= 0)
	{
		ScrPrint(0,2,0,"Pos PriKey error");
		goto Exit;
	}

	gPosPriKeyBuffT.size = iRet;
	gPosPriKeyBuffT.ptr = gPosPriKeyBin;

	//3. SSL certification set
	if (SslCertsSet(sslCon.sslSock, &gCaCertBuffT, 1, NULL, &gPosCertBuffT, 1, &gPosPriKeyBuffT) < 0)
	{
		SslClose(sslCon.sslSock);
		sslCon.sslSock = -1;
		goto Exit;
	}
	
	/***********************************************************************/

	/****************************** SSL Connect ****************************/

CONNECT:
	logd(("Calling SslConnect"));
	iRet = SslConnect(sslCon.sslSock, (char*)pszIP, sPort, 0, 0);
	logd(("iRet=%d", iRet));
	if(iRet < 0)
	{
		logd(("SSL Connect failed with: %d", iRet));
		SslClose(sslCon.sslSock);
		sslCon.sslSock = -1;
		goto Exit;
	}

	/***********************************************************************/

Exit:	
	
	if(pCACertPem != NULL)
	{
		free(pCACertPem);
		pCACertPem = NULL;
	}

	if(pPosCertPem != NULL)
	{
		free(pPosCertPem);
		pPosCertPem = NULL;
	}

	if(pPosPriKeyPem != NULL)
	{
		free(pPosPriKeyPem);
		pPosPriKeyPem = NULL;
	}
	
	return iRet;
}

int SxxSSLConnectCheck(int iTimeoutSec)
{
	logd((__func__));
	int iRet = 0;

	TimerSet(TIMER_TEMPORARY, (ushort)(iTimeoutSec * 3 * 10));
	
	while(TimerCheck(TIMER_TEMPORARY) != 0)
	{
		iRet = SslProcess(sslCon.sslSock);
		if(iRet < 0)
		{
			break;
		}
		
		if(iRet == 0)
		{
			break;
		}
	}
	
	if(iRet == 1 && TimerCheck(TIMER_TEMPORARY) == 0)
	{
		iRet = NET_ERR_TIMEOUT;
	}
	
	return iRet;
}

int SxxSSLTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	logd((__func__));
	int iRet;
	
	sndTimeOut = uiTimeOutSec * 1000;
	
	iRet = SslSend(sslCon.sslSock, (void *)psTxdData, uiDataLen);
	if (iRet < 0)
	{
		SslClose(sslCon.sslSock);
		sslCon.sslSock = -1;
	}
	
	return iRet;
}

int SxxSSLRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int iRet;
	
	rcvTimeOut = uiTimeOutSec * 1000;
	
	iRet = SslRecv(sslCon.sslSock, psRxdData, uiExpLen);
	if(iRet > 0) {
		*puiOutLen = iRet;
	} else {
		*puiOutLen = 0;
	}
	
	return iRet;
}

int SxxSSLClose(void)
{
	int iRet;
	
	if(sslCon.sslSock >= 0)
	{
		iRet = SslClose(sslCon.sslSock);
		
		memset(&sslCon, 0, sizeof(sslCon));
		sslCon.sslSock = -1;
	}
	
	return iRet;
}

int SxxSSLSingleProcess()
{
	return  SslProcess(sslCon.sslSock);
}
