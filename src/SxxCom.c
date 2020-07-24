
#include "global.h"
#include "SxxCom.h"

/********************** Internal macros declaration ************************/
#define TCPMAXSENDLEN		10240

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static uchar SocketCheck(int sk);

/********************** Internal variables declaration *********************/
static int sg_iSocket = -1;

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// Sxx TCP connection
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpConnect(const char *pszIP, short sPort, uchar bSSL, uchar ucDialMode, int iTimeoutSec)
{
	logd((__func__));
	int		iRet = 0;
	struct net_sockaddr stServer_addr;
	uchar szCA[120] = {0};
	uchar szCert[120] = {0};
	uchar szKey[120] = {0};

	//Modified by Kevin_Wu, for support SSL noneblock mode 20160913
	//PreDial Mode
	if(ucDialMode == DM_PREDIAL)
	{
        if(bSSL)
		{
			if(GetEnv("CA_CRT", szCA) != 0)
	        {
	           // return ERR_COMM_INV_PARAM;
	        }

	        if(GetEnv("CLI_CRT", szCert) != 0)
	        {
	          //  return ERR_COMM_INV_PARAM;
	        }

	        if(GetEnv("CLI_KEY", szKey) != 0)
	        {
	           // return ERR_COMM_INV_PARAM;
	        }
		
			SxxSSLObjInit(szCA, szCert, szKey);
		
			iRet = SxxSSLConnect((char*)pszIP, sPort, iTimeoutSec);
		}
				
		return iRet;
	}

	//Normal Dial mode
	if(ucDialMode == DM_DIAL)
	{	
		if(bSSL)
		{	
			if(SxxSSLGetSocket() >= 0)
			{
				iRet = SxxSSLConnectCheck(iTimeoutSec);
			}
			else
			{
				if(GetEnv("CA_CRT", szCA) != 0)
				{
					//return ERR_COMM_INV_PARAM;
				}

				if(GetEnv("CLI_CRT", szCert) != 0)
				{
					//return ERR_COMM_INV_PARAM;
				}

				if(GetEnv("CLI_KEY", szKey) != 0)
				{
					//return ERR_COMM_INV_PARAM;
				}
				
				SxxSSLObjInit(szCA, szCert, szKey);
				
				iRet = SxxSSLConnect((char*)pszIP, sPort, iTimeoutSec);
				if (iRet < 0)
				{
					return iRet;
				}
				
				iRet = SxxSSLConnectCheck(iTimeoutSec);
			}
		}
		else
		{
			// Bind IP
			iRet = SockAddrSet(&stServer_addr, (char *)pszIP, sPort);
			if (iRet!=0)
			{
				return iRet;
			}
			// Setup socket
			iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
			if (iRet < 0) // Modified By Kim 2014-12-10
			{
				return iRet;	
			}
			sg_iSocket = iRet;
			// set connection timeout
			if (iTimeoutSec<3)
			{
				iTimeoutSec = 3;
			}

			iRet = Netioctl(sg_iSocket, CMD_TO_SET, iTimeoutSec*1000);
			if (iRet<0) // Modified By Kim 2014-12-10
			{
				return iRet;
			}

			// Connect to remote IP
			iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
			if (iRet!=0)
			{
				NetCloseSocket(sg_iSocket);
				sg_iSocket = -1;
			}
		}
	}
	
	return iRet;
}

//Sxx TCP/IP send data
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	logd((__func__));

	int iRet;
	int iSendLen;
	int iSumLen;
	
	if(bSSL)
	{
		SxxSSLTxd(psTxdData, uiDataLen, 30);
	}
	else
	{
		iRet = Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
		if (iRet < 0)
		{
			return iRet;
		}

		iSumLen = 0;
		while(1)
		{
			if (uiDataLen > TCPMAXSENDLEN)
			{
				iSendLen = TCPMAXSENDLEN;
				uiDataLen = uiDataLen - TCPMAXSENDLEN;
			}
			else
			{
				iSendLen = uiDataLen;
			}
			
			iRet = NetSend(sg_iSocket, (uchar*)psTxdData+iSumLen, iSendLen, 0);
			
			if (iRet < 0)
			{
				return iRet;
			}
			if (iRet != iSendLen)
			{
				return -1;
			}
			iSumLen = iSumLen + iSendLen;
			if (iSendLen <= TCPMAXSENDLEN)
			{
				break;
			}	
		}
	
	}
	return 0;
}

//Sxx TCP/IP receive
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen)
{
	int iRet;


	DelayMs(200);
	
	if(bSSL)
	{
		iRet = SxxSSLRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		if (iRet < 0)
		{
			return iRet;
		}
	}
	else
	{
		iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);

		if (iRet < 0)
		{
			return iRet;
		}

		iRet = NetRecv(sg_iSocket, psRxdData, uiExpLen, 0);
		if (iRet < 0)
		{
			return iRet;
		}
		*puiOutLen = iRet;
	}
	
	return 0;
}

// Sxx TCP/IP close socket
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpOnHook(uchar bSSL)
{
	int iRet;

	if(bSSL){
	    SxxSSLClose();
	    return 0;
	}
	
	iRet = NetCloseSocket(sg_iSocket);
	if (iRet < 0)
	{
		sg_iSocket = -1;
		return iRet;
	}
	sg_iSocket = -1;
	return 0;
}

uchar SocketCheck(int sk)
{
	int event;
	if(sk<0) return RET_TCPCLOSED;
	
	event = Netioctl(sk, CMD_EVENT_GET, 0);
	if(event<0)
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}	
	
	if(event&(SOCK_EVENT_CONN|SOCK_EVENT_WRITE|SOCK_EVENT_READ))
	{
         return RET_TCPOPENED;
	}
	else if(event&(SOCK_EVENT_ERROR))
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}

	return RET_TCPOPENING;
}

int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec)
{
	int	iRet;

	if (ucForceStart && (DhcpCheck()==0))
	{
		DhcpStop();
	}

	if (ucForceStart || (DhcpCheck()!=0))
	{
		iRet = DhcpStart();
		if (iRet < 0)
		{
			return iRet;
		}

		TimerSet(TIMER_TEMPORARY, (ushort)(ucTimeOutSec*10));
		while (TimerCheck(TIMER_TEMPORARY)!=0)
		{
			DelayMs(200);
			iRet = DhcpCheck();
			if (iRet==0)
			{
				return 0;
			}
		}

		return iRet;
	}

	return 0;
}

int SxxLANTcpDial(const TCPIP_PARA *pstTcpPara, uchar ucDialMode)
{
	int		iRet;
	uchar	ucRedoDhcp, ucSecondIP;
	uchar   szIpFromDNS[25 + 1] = {0};

	uchar bSSL = 0;
    uchar szSSL[120];

	bSSL = glCommCfg.ucPortMode;
	logTrace("Is ssl: %d", bSSL);
	
	//Modify by Kevin_Wu, for supporting SSL none block mode 20160913
	if(!(bSSL && (SxxSSLGetSocket() >=0)))
	{
		CommOnHook(FALSE);
	}

	if (pstTcpPara->ucDhcp)
	{
		iRet = SxxDhcpStart(FALSE, 30);
		if (iRet!=0)
		{	
			return iRet;
		}
	}

	ucRedoDhcp = FALSE;
	ucSecondIP = FALSE;

TAG_RETRY_IP:

	// Connect to remote IP
	if (ucSecondIP)
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost2.szIP)){
            iRet = DnsResolve((char *)pstTcpPara->stHost2.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, ucDialMode, 8);	
			}	
        }
        else
		{	
			iRet = SxxTcpConnect(pstTcpPara->stHost2.szIP, (short)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, ucDialMode, 8);
		}
	}
	else
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost1.szIP) > 0){
			iRet = DnsResolve((char *)pstTcpPara->stHost1.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
	        if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, ucDialMode, 8);
			}
			
	    }
	    else
		{
			iRet = SxxTcpConnect(pstTcpPara->stHost1.szIP, (short)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, ucDialMode, 8);
		}
	}
	
	//Modified by Kevin_Wu 20160913
	if (iRet < 0)
	{
		if (!ucSecondIP)
		{
			if (pstTcpPara->ucDhcp && !ucRedoDhcp)
			{
				// If fail, suspect the DHCP
				iRet = SxxDhcpStart(FALSE, 10);
				if (iRet!=0)
				{
					return iRet;
				}
				ucRedoDhcp = TRUE;
				goto TAG_RETRY_IP;
			}

			if ( ChkIfValidPort(pstTcpPara->stHost2.szPort) &&
			      ChkIfValidIp(pstTcpPara->stHost2.szIP) &&
				strcmp((char *)(pstTcpPara->stHost2.szIP), (char *)(pstTcpPara->stHost2.szIP)))
			{
				ucSecondIP = TRUE;
				goto TAG_RETRY_IP;
			}
		}
	}

	return iRet;
}

// initial the wireless module
int SXXWlInit(const WIRELESS_PARAM *pstWlPara)
{
	int iRet;

	WlSelSim(pstWlPara->ucUsingSlot);

	iRet = WlInit(pstWlPara->szSimPin);
	
	if(iRet == WL_RET_ERR_INIT_ONCE)
	{
		iRet = 0;
	}

	if(iRet < 0)
	{
		return iRet;
	}

	SXXWlDispSignal();
	
	iRet = WlPppCheck();
	if ((iRet == 0) || (iRet == WL_RET_ERR_DIALING) || (iRet == 1))	// ret = 1 means module busy
	{
		return 0;
	}

	iRet = WlPppLogin((uchar*)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, 0, 3600);
	if(iRet == NET_ERR_TIMEOUT) //Indicate none block mode
	{
		iRet = 0;
	}

	return iRet;
}

// check if PPP is linked, if not, build PPP link firstly, and then build TCP link,
// otherwise build TCP link directly
int SXXWlDial(const WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval, uchar ucDialMode)
{
	logd((__func__));

	int		iRet;
	int		iRetryTime;
	uchar	ucSecondIP;
	uchar   szIpFromDNS[25] = {0};

	uchar bSSL = 0;
    uchar szSSL[120];

	bSSL = glCommCfg.ucPortMode;
	logTrace("Is ssl: %d", bSSL);
	
	//Modify by Kevin_Wu, for supporting SSL none block mode 20160913
	if(!(bSSL && (SxxSSLGetSocket() >=0)))
	{
		logd(("Non block mode adjust"));
		CommOnHook(FALSE);
	}

	SXXWlDispSignal();
	
	if (iTimeOut < 1)
	{
		iTimeOut = 1;
	}

	// ********** Pre-dial **********
	if (ucDialMode == DM_PREDIAL)
	{
		logd(("Predialing"));
		iRet = WlPppCheck();

		//If Connect success, establish SSL connect
		if(bSSL && (SxxSSLGetSocket() < 0) && iRet == 0)
		{
			goto TCPCONNECT;
		}
		
		if ((iRet==0) || (iRet==WL_RET_ERR_DIALING) || (iRet==1))	// ret=1 means module busy
		{
			return 0;
		}

		iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, 0, iAliveInterval);
		return 0;
	}

	// ********** Full-dial **********
	logd(("Full dial"));
	// ********** Check PPP connection **********
	TimerSet(TIMER_TEMPORARY, (ushort)(iTimeOut*10));
	while (TimerCheck(TIMER_TEMPORARY)!=0)
	{
		iRet = WlPppCheck();
		if (iRet == 0)
		{
			logd(("PPP connected, jumping to TCPCONNECT"));
			goto TCPCONNECT; 
			
		}
	}

	// ********** Take PPP dial action **********
	iRetryTime = 3;
	while(iRetryTime--)
	{
		logd(("WlPppLogin"));
		//iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, iTimeOut*1000, iAliveInterval);
		iRet = WlPppLoginEx("*99***1#", (uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, iTimeOut * 1000, iAliveInterval);
		if (iRet != 0)
		{
			DelayMs(100);
			continue;
		}

		iRet = WlPppCheck();
		if (iRet == 0)
		{
			logd(("PPP Connected"));
			break;
		}
	}

	if (iRetryTime <= 0 && iRet != 0)
	{
		return iRet;
	}

	// ********** Connect IP **********
TCPCONNECT:	
	if(!bSSL)
	{
		logd(("Not ssl,  connect socket directly"));
		iRet = SocketCheck(sg_iSocket);  //come from R&D, tom
	//	ScrPrint(0, 7, ASCII, "tang[SocketCheck(%i)]",iRet); DelayMs(1000);
		if (iRet == RET_TCPOPENED)
		{
			return 0;
		}
	}

	ucSecondIP = FALSE;

_RETRY_SECOND_IP:
	if (ucSecondIP)
	{
	    if(!ChkIfValidIp(pstWlPara->stHost2.szIP)){
            iRet = DnsResolve((char *)pstWlPara->stHost2.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
		    {
				iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstWlPara->stHost2.szPort), bSSL, ucDialMode, 8);	
		    }   
        }
        else
		{   
			iRet = SxxTcpConnect(pstWlPara->stHost2.szIP, (short)atoi((char *)pstWlPara->stHost2.szPort), bSSL, ucDialMode, 8);
		}
	}
	else
	{
	    if(!ChkIfValidIp(pstWlPara->stHost1.szIP)){
			logd(("Need to resolve IP"));
            iRet = DnsResolve((char *)pstWlPara->stHost1.szIP, (char *)szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
			{
				iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstWlPara->stHost1.szPort), bSSL, ucDialMode, 8);
			} 

        }
        else
		{
			logd(("Valid IP, connecting straight, IP: %s, Port: %s", pstWlPara->stHost1.szIP, pstWlPara->stHost1.szPort));
			iRet = SxxTcpConnect(pstWlPara->stHost1.szIP, (short)atoi((char *)pstWlPara->stHost1.szPort), bSSL, ucDialMode, 8);
		}
	}

	if (iRet < 0)
	{
	    if ( ChkIfValidPort(pstWlPara->stHost2.szPort) &&
             ChkIfValidIp(pstWlPara->stHost2.szIP) &&
             strcmp((char *)(pstWlPara->stHost2.szIP), (char *)(pstWlPara->stHost2.szIP)))
        {
			ucSecondIP = TRUE;
			goto _RETRY_SECOND_IP;
		}

		return iRet;
	}

	return 0;
}

// send data (wireless)
int SXXWlSend(const uchar *psTxdData, ushort usDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, usDataLen, bSSL, uiTimeOutSec);
}

// receive data (wireless)
int SXXWlRecv(uchar *psRxdData, ushort usExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *pusOutLen)
{
	DelayMs(200);
	return SxxTcpRxd(psRxdData, usExpLen, uiTimeOutSec, bSSL, pusOutLen);
}

// close the TCP link
int SXXWlCloseTcp(uchar bSSL)
{
	return SxxTcpOnHook(bSSL);
}

// close the PPP link
void SXXWlClosePPP(void)
{
	WlPppLogout(); 
	return;
}

// display the wireless signal
void SXXWlDispSignal(void)
{
	uchar	ucRet, ucLevel;
	
	
	ucRet = WlGetSignal(&ucLevel);
	if( ucRet!=RET_OK )
	{
		ScrSetIcon(ICON_SIGNAL, CLOSEICON);
		return;
	}
	
	ScrSetIcon(ICON_SIGNAL, (uchar)(5-ucLevel));
}


