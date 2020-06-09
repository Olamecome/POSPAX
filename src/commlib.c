
#include "global.h"

/********************** Internal macros declaration ************************/
// for TCP/IP module
// Removed by Kim_LinHB 2014-4-4 TCPIP for Pxx

#define LEN_WORKBUF			(1024*4)
#define TIMER_TEMPORARY		4       // Temporary timer(Shared by different modules)

#define TCPMAXSENDLEN 10240
extern ushort ModemExCommand(uchar *CmdStr, uchar *RespData,
					  ushort *Dlen,ushort Timeout10ms);

/********************** Internal structure declaration *********************/
typedef struct _tagERR_INFO
{
	int		iErrCode;
	uchar	szMsg[16+1];
}ERR_INFO;

/********************** Internal functions declaration *********************/
static int CommInitGprsCdma(const COMM_CONFIG *pstCfg);
static int CommInitLAN(const COMM_CONFIG *pstCfg);
static int CommInitWifi(COMM_CONFIG *pstCfg);
//Added by Kim_LinHB 2014-8-16 v1.01.0004
static int CommInitBT(const COMM_CONFIG *pstCfg);

static int RS232Dial(uchar ucDialMode);
static int RS232Txd(const uchar *psTxdData, ushort uiDataLen);
static int RS232RawTxd(const uchar *psTxdData, ushort uiDataLen);
static int RS232NacTxd(const uchar *psTxdData, ushort uiDataLen);
static int RS232AsyncTxd(const uchar *psTxdData, ushort uiDataLen);
static int RS232Rxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int RS232RawRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int RS232NacRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int RS232AsyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int RS232OnHook(uchar bReleaseAll);

static int TcpDial(uchar ucDialMode);
static int TcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec);
static int TcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen);
static int TcpOnHook(uchar bSSL, uchar bReleaseAll);

// Added by Kim_LinHB 2014-08-19 v1.01.0004
static int WIFIDial(uchar ucDialMode);
static int WIFITxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec);
static int WIFIRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen);
static int WIFIOnHook(uchar bSSL, uchar bReleaseAll);
static void WIFIDispSignal(void);

// PSTN (Dial) functions
static int PSTNDial(uchar ucDialMode);
static int PSTNTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec);
static int PSTNRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int PSTNSyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int PSTNAsyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int PSTNOnHook(uchar bReleaseAll);

// Added by Kim_LinHB 2014-08-15 v1.01.0004
static int BTDial(uchar ucDialMode);
static int BTTxd(const uchar *psTxdData, ushort uiDataLen);
static int BTRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);
static int BTOnHook(uchar bReleaseAll);

static void  GetAllErrMsg(int iErrCode, const ERR_INFO *pstInfo, COMM_ERR_MSG *pstCommErrMsg);

static void  CalcCRC32(const uchar *psData, ushort uiLength, uchar sCRC[4]);
static uchar CalcLRC(const uchar *psData, ushort uiLength, uchar ucInit);
static int   Conv2AsyncTxd(const uchar *psTxdData, ushort uiDataLen);
static int   UpdateTimer(uchar ucTimerNo, ushort *puiTickCnt);

/********************** Internal variables declaration *********************/
static COMM_CONFIG	sg_stCurCfg = {0xFF};
static uchar		sg_sWorkBuf[LEN_WORKBUF+50];

// Shared error messages
static ERR_INFO		sg_stCommErrMsg[] =
{
	{ERR_COMM_INV_PARAM, _T_NOOP("INVALID PARAM")},
	{ERR_COMM_INV_TYPE,  _T_NOOP("INV COMM TYPE")},
	{ERR_COMM_CANCEL,    _T_NOOP("USER CANCEL")},
	{ERR_COMM_TIMEOUT,   _T_NOOP("TIMEOUT")},
	{ERR_COMM_COMERR,    _T_NOOP("COMM ERROR")},
	{ERR_COMM_TOOBIG,    _T_NOOP("DATA TOO BIG")},
	{0, ""},
};

// RS232 error messages
static ERR_INFO		sg_stRS232ErrMsg[] =
{
	{0x01, _T_NOOP("PORT ERROR")},
	{0x02, _T_NOOP("INVALID PORT")},
	{0x03, _T_NOOP("PORT CLOSED")},
	{0x04, _T_NOOP("OVERFLOW")},
	{0x05, _T_NOOP("NOT AVAIL PORT")},
	{0xF0, _T_NOOP("NOT AVAIL PORT")},
	{0xFE, _T_NOOP("INVALID PARAM")},
	{0xFF, _T_NOOP("TIMEOUT")},
	{0, ""},
};

// TCP/IP error messages for Pxx model
static ERR_INFO		sg_stTCPErrMsg[] =
{
	{ERR_COMM_TCPIP_OPENPORT, _T_NOOP("OPEN PORT ERR")},
	{ERR_COMM_TCPIP_SETLIP,   _T_NOOP("LOCAL IP ERR")},
	{ERR_COMM_TCPIP_SETRIP,   _T_NOOP("REMOTE IP ERR")},
	{ERR_COMM_TCPIP_SETRPORT, _T_NOOP("SET PORT ERR")},
	{ERR_COMM_TCPIP_CONN,     _T_NOOP("TCP CONN ERR")},
	{ERR_COMM_TCPIP_TXD,      _T_NOOP("SEND DATA ERR")},
	{ERR_COMM_TCPIP_RXD,      _T_NOOP("RECV DATA ERR")},
	{ERR_COMM_TCPIP_SETGW,    _T_NOOP("SET GW ERR")},
	{ERR_COMM_TCPIP_SETMASK,  _T_NOOP("SET MASK ERR")},
	{0, ""},
};

// modem error messages
static ERR_INFO	sg_stModemErrMsg[] =
{
	{0x01, _T_NOOP("SEND OVERFLOW")},
	{0x02, _T_NOOP("PHONE OCCUPIED")},
	{0x03, _T_NOOP("NO DIAL TONE")},
	{0x04, _T_NOOP("LINE BREAK")},
	{0x05, _T_NOOP("NO ACK")},
	{0x06, _T_NOOP("IN DIALING")},
	{0x07, _T_NOOP("UNSUPPORTED")},
	{0x08, _T_NOOP("RECV NOT EMPTY")},
	{0x09, _T_NOOP("BUFFER NOT EMPTY")},
	{0x0A, _T_NOOP("IN DIALING")},
	{0x0B, _T_NOOP("LINE READY")},
	{0x0C, _T_NOOP("RECV REJECTED")},
	{0x0D, _T_NOOP("LINE BUSY")},
	{0x0F, _T_NOOP("INVALID TEL NO")},
	{0x33, _T_NOOP("LINE READY ?")},
	{0xE9, _T_NOOP("TONE NOT STOP")},
	{0xF0, _T_NOOP("NOT AVAIL PORT")},
	{0xFD, _T_NOOP("USER CANCEL")},
	{0xFE, _T_NOOP("INV DATA LENGTH")},
	{0, ""},
};

static ERR_INFO	sg_stSxxErrMsg[] =
{
	//SSL Error
	{ -105,	_T_NOOP("HANDSHAKE ERROR") },
	{ -108,	_T_NOOP("NETWORK ERROR") },
	{ -109,	_T_NOOP("SSL SOCKET ERROR") },
	{ -110,	_T_NOOP("SSL TIMEOUT") },
	{ -111,	_T_NOOP("SOCKET CONNECTED") },
	{ -112,	_T_NOOP("DISCONNECTED") },
	{ -113,	_T_NOOP("SOCKET IS CLOSED") },
	{ -117,	_T_NOOP("SSL CERT ERROR") },
	// Wireless error
	{WL_RET_ERR_PORTINUSE,	_T_NOOP("INTERNAL ERR")},	// 模块口被占用
	{WL_RET_ERR_NORSP,		_T_NOOP("INTERNAL ERR")},	// 模块没有应答
	{WL_RET_ERR_RSPERR,		_T_NOOP("INTERNAL ERR")},	// 模块应答错误
	{WL_RET_ERR_PORTNOOPEN,	_T_NOOP("INTERNAL ERR")},	// 模块串口没有打开
	{WL_RET_ERR_NEEDPIN,	_T_NOOP("NEED SIM PIN")},	// 需要PIN码
	{WL_RET_ERR_NEEDPUK,	_T_NOOP("NEED SIM PUK")},	// 需要PUK解PIN码
	{WL_RET_ERR_PARAMER,	_T_NOOP("INVALID CONFIG")},	// 参数错误
	{WL_RET_ERR_ERRPIN,		_T_NOOP("INVALID SIMPIN")},	// 密码错误
	{WL_RET_ERR_NOSIM,		_T_NOOP("NO SIM CARD")},	// 没有插入SIM卡
	{WL_RET_ERR_NOTYPE,		_T_NOOP("INVALID TYPE")},	// 不支持的类型
	{WL_RET_ERR_NOREG,		_T_NOOP("SIM NOT REG.")},	// 网络没有注册
//	{WL_RET_ERR_INIT_ONCE,	_T_NOOP("INITED.")},		// 模块已初始化
	{WL_RET_ERR_LINEOFF,	_T_NOOP("LINE OFF")},		// 连接断开
	{WL_RET_ERR_TIMEOUT,	_T_NOOP("TIMEOUT")},		// 超时
	{WL_RET_ERR_REGING,		_T_NOOP("REGING")},			// 网络注册中
	{WL_RET_ERR_PORTCLOSE,	_T_NOOP("INTERNAL ERR")},	// 关闭串口出错
	{WL_RET_ERR_MODEVER,	_T_NOOP("INTERNAL ERR")},	// 错误的模块版本
//	{WL_RET_ERR_DIALING,	_T_NOOP("INTERNAL ERR")},	// 拨号中  
	{WL_RET_ERR_ONHOOK,		_T_NOOP("INTERNAL ERR")},	// 关机中
	{WL_RET_ERR_PPP_BRK,	_T_NOOP("DISCONNECTED")},	// 发现PPP断开
	{WL_RET_ERR_NOSIG,		_T_NOOP("NO SIGNAL")},		// 网络无信号
	{WL_RET_ERR_POWEROFF,	_T_NOOP("INTERNAL ERR")},	// 模块已下电
	{WL_RET_ERR_BUSY,		_T_NOOP("INTERNAL ERR")},	// 模块忙
	{WL_RET_ERR_OTHER,		_T_NOOP("INTERNAL ERR")},	// 其他未知错误
	// TCPIP stack error
	{NET_ERR_MEM,			_T_NOOP("INTERNAL ERR")},	// 内存不够
	{NET_ERR_BUF,			_T_NOOP("INTERNAL ERR")},	// 缓冲区错误
	{NET_ERR_ABRT,			_T_NOOP("CONN. FAIL")},		// 试图建立连接失败
	{NET_ERR_RST,			_T_NOOP("CONN. CLOSED")},	// 连接被对方复位（收到对方的Reset）
	{NET_ERR_CLSD,			_T_NOOP("CONN. CLOSED")},	// 连接已关闭
	{NET_ERR_CONN,			_T_NOOP("CONN. FAIL")},		// 连接未成功建立
	{NET_ERR_VAL,			_T_NOOP("INVALID PARAM")},	// 错误变量
	{NET_ERR_ARG,			_T_NOOP("INVALID PARAM")},	// 错误参数
	{NET_ERR_RTE,			_T_NOOP("INTERNAL ERR")},	// 错误路由(route)
	{NET_ERR_USE,			_T_NOOP("INTERNAL ERR")},	// 地址、端口使用中
	{NET_ERR_IF,			_T_NOOP("INTERNAL ERR")},	// 底层硬件错误
//	{NET_ERR_ISCONN										// 连接已建立
	{NET_ERR_TIMEOUT,		_T_NOOP("TIMEOUT")},		// 超时
	{NET_ERR_AGAIN,			_T_NOOP("PLS RETRY")},		// 请求资源不存在，请重试
//	{NET_ERR_EXIST										// 已存在
	{NET_ERR_SYS,			_T_NOOP("UNSUPPORTED")},	// 系统不支持
	{NET_ERR_PASSWD,		_T_NOOP("INVALID PWD")},	// 错误密码
	{NET_ERR_MODEM,			_T_NOOP("DIAL FAIL")},		// 拨号失败
	{NET_ERR_LINKDOWN,		_T_NOOP("LINK DOWN")},		// 数据链路已断开，请重新拨号
	{NET_ERR_LOGOUT,		_T_NOOP("LOG OUT")},		// Logout
	{NET_ERR_PPP,			_T_NOOP("CONN. FAIL")},		// PPP断开
	{NET_ERR_STR,			_T_NOOP("INTERNAL ERR")},	// String Too Long, Illeage
	{NET_ERR_DNS,			_T_NOOP("DNS FAIL")},		// DNS Failure: No such Name
	{NET_ERR_INIT,			_T_NOOP("NOT INITED")},		// No Init
	{0, ""},
};

/********************** external reference declaration *********************/
extern char *strstr(const char *, const char *);

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// initial communication module
int CommInitGprsCdma(const COMM_CONFIG *pstCfg)
{
	CommSetCfgParam(pstCfg);

	WlSelSim(pstCfg->stWirlessPara.ucUsingSlot);
	return SXXWlInit(&pstCfg->stWirlessPara);
}

int CommInitLAN(const COMM_CONFIG *pstCfg)
{
	if (pstCfg->stTcpIpPara.ucDhcp)
	{
		return SxxDhcpStart(FALSE, 30);		// If already get DHCP in manager or other EDC, no need to re-start.
	}

	return 0;
}

int CommInitWifi(COMM_CONFIG *pstCfg)
{
	int iRet;

	CommSetCfgParam(pstCfg);

	iRet = WifiCheck(NULL);
	if(iRet > 0)
		return 0;
	else if (-3 == iRet)
	{
		WifiOpen();
	}

	Gui_ClearScr();
	Gui_ShowMsgBox("WIFI", gl_stTitleAttr, _T("Connecting ..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

#ifdef _MIPS_
	WifiConnectAp(&pstCfg->stWifiPara.stLastAP, &pstCfg->stWifiPara.stParam);
#else
	WifiConnect(&pstCfg->stWifiPara.stLastAP, &pstCfg->stWifiPara.stParam);
#endif

	TimerSet(TIMER_TEMPORARY, 200); // 20 sec for checking link
	while(1)
	{
		iRet = WifiCheck(NULL);
		if(0 == TimerCheck(TIMER_TEMPORARY))
		{
			return ERR_COMM_TIMEOUT;
		}
		if (!kbhit() && getkey() == KEYCANCEL)
		{
			return ERR_USERCANCEL;
		}

		if(0 == iRet)
			continue;

		if (iRet < 0)
		{
			return iRet;
		}
		else
		{
			uchar szPrompt[255];
			sprintf(szPrompt, "%s\n%s", "Connected to", pstCfg->stWifiPara.stLastAP.Ssid);
			Gui_ClearScr();
			Gui_ShowMsgBox("WIFI", gl_stTitleAttr, szPrompt, gl_stCenterAttr, GUI_BUTTON_OK, 5, NULL);
			if(pstCfg->stWifiPara.stParam.DhcpEnable == 1)
			{
				TCPIP_PARA stLocalInfo;
				if(0 == NetDevGet(WIFI_ROUTE_NUM, 
					stLocalInfo.szLocalIP,
					stLocalInfo.szNetMask,
					stLocalInfo.szGatewayIP, 
					stLocalInfo.szDNSIP))
				{
					SplitIpAddress(stLocalInfo.szLocalIP, pstCfg->stWifiPara.stParam.Ip);	
					SplitIpAddress(stLocalInfo.szNetMask, pstCfg->stWifiPara.stParam.Mask);
					SplitIpAddress(stLocalInfo.szGatewayIP, pstCfg->stWifiPara.stParam.Gate);
					SplitIpAddress(stLocalInfo.szDNSIP, pstCfg->stWifiPara.stParam.Dns);
				}
			}
			WIFIDispSignal();
			return 0;
		}
	}
}

// Added by Kim_LinHB 2014-8-16
static int CommInitBT(const COMM_CONFIG *pstCfg)
{
	int iRet;
	CommSetCfgParam(pstCfg);

#ifdef _MIPS_
	iRet = BtOpen(pstCfg->stBlueToothPara.stCommParam.ucPortNo,
		pstCfg->stBlueToothPara.stCommParam.szAttr);
	if(iRet != BT_RET_OK && iRet != BT_RET_ERROR_NOTCLOSE)
		return iRet;

	iRet = BtSetRole(BT_ROLE_SLAVE);
	if(iRet != BT_RET_OK)
		return iRet;

	iRet = BtSetName(pstCfg->stBlueToothPara.stConfig.name);
	if(iRet != BT_RET_OK)
		return iRet;

	iRet = BtSetPin(pstCfg->stBlueToothPara.stConfig.pin);
	if(iRet != BT_RET_OK)
		return iRet;
#else
	iRet = BtOpen();
	if(iRet != 0 && iRet != BT_RET_ERROR_CONNECTED)
		return iRet;

	iRet = BtSetConfig((ST_BT_CONFIG *)&pstCfg->stBlueToothPara.stConfig);
	if(iRet != 0)
		return iRet;

	iRet = PortOpen(COM_BT, NULL);
	if(iRet != 0)
		return (ERR_COMM_RS232_BASE | iRet);
#endif
	return 0;
}


int CommInitModule(COMM_CONFIG *pstCfg)
{
	logTrace(__func__);
	int		ii, iRet;

	switch(pstCfg->ucCommType)
	{
	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:   // added by Gillian 2015/11/23
		iRet = WlOpenPort();
		iRet = WlSendCmd("AT+LBSSTART\r", NULL, 0, 3000, 100);
		WlClosePort();
		for (ii=0; ii<3; ii++)
		{
			iRet = CommInitGprsCdma(pstCfg);
			if (iRet==0)
			{
				break;
			}
		}
		return iRet;

	case CT_TCPIP:
		return CommInitLAN(pstCfg);

	case CT_WIFI:
		return CommInitWifi(pstCfg);
	case CT_BLTH:
		return CommInitBT(pstCfg);
	default:
		return 0;
	}
}

// Modified by Kim_LinHB 2014-5-31
// 设置通讯模块参数
// set communication module parameters
int CommSetCfgParam(const COMM_CONFIG *pstCfg)
{
	logTrace(__func__);
	int		iLen;

	if( pstCfg==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}
// 	if (!ChkIfSupportCommType(pstCfg->ucCommType))
// 	{
// 		return ERR_COMM_INV_TYPE;
// 	}

	switch( pstCfg->ucCommType )
	{
	case CT_RS232:
	case CT_DEMO:
		iLen = strlen((char *)pstCfg->stRS232Para.szAttr);
		if( iLen<10 || iLen>20 )	// fast but unsuitable
		{
			return ERR_COMM_INV_PARAM;
		}
		break;
	case CT_BLTH:
		iLen = strlen((char *)pstCfg->stBlueToothPara.stCommParam.szAttr);
		if( iLen<10 || iLen>20 )	// fast but unsuitable
		{
			return ERR_COMM_INV_PARAM;
		}	
		break;

	case CT_MODEM:
		if( pstCfg->stPSTNPara.szTelNo[0]==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
		if( pstCfg->stPSTNPara.ucSendMode!=CM_ASYNC &&
			pstCfg->stPSTNPara.ucSendMode!=CM_SYNC )
		{
			return ERR_COMM_INV_PARAM;
		}
		break;
		
	
	case CT_WIFI:
		if( !ChkIfValidIp(pstCfg->stWifiPara.stHost1.szIP) ||
			!ChkIfValidPort(pstCfg->stWifiPara.stHost1.szPort))
		{
			return ERR_COMM_INV_PARAM;
		}
		break;

	case CT_TCPIP:
		if( !ChkIfValidIp(pstCfg->stTcpIpPara.szLocalIP) )
		{
			return ERR_COMM_INV_PARAM;
		}
		if( !ChkIfValidIp(pstCfg->stTcpIpPara.stHost1.szIP) ||
			!ChkIfValidPort(pstCfg->stTcpIpPara.stHost1.szPort))
		{
			return ERR_COMM_INV_PARAM;
		}
	    break;

	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		if( strlen((char *)(pstCfg->stWirlessPara.szAPN))==0 )
		{
			return ERR_COMM_INV_PARAM;
		}
		if( !ChkIfValidIp(pstCfg->stWirlessPara.stHost1.szIP) ||
			!ChkIfValidPort(pstCfg->stWirlessPara.stHost1.szPort))
		{
			return ERR_COMM_INV_PARAM;
		}
		break;

	default:
		return ERR_COMM_INV_TYPE;
	}

	memcpy(&sg_stCurCfg, pstCfg, sizeof(COMM_CONFIG));

	return 0;
}

// 检查指定的电话号码是否与已经存储的参数一致
// check if the specific Tel No. is matched Numbers saved
int CommChkIfSameTelNo(const uchar *pszTelNo)
{
	if( sg_stCurCfg.ucCommType==0xFF )
	{
		return FALSE;
	}

	if( sg_stCurCfg.ucCommType!=CT_MODEM )
	{
		return TRUE;
	}

	if( strcmp((char *)sg_stCurCfg.stPSTNPara.szTelNo, (char *)pszTelNo)==0 )
	{
		return TRUE;
	}

	return FALSE;
}

void CommSwitchType(uchar ucCommType)
{
	sg_stCurCfg.ucCommType = ucCommType;
}

// Modified by Kim_LinHB 2014-5-31
// "Dial" for all kinds of communication.
// for modem it is dialing, for GPRS/CDMA/WIFI/LAN it is connecting TCPIP
int CommDial(uchar ucDialMode)
{
	logd((__func__));
	int		iRet;

	switch( sg_stCurCfg.ucCommType )
	{
	case CT_RS232:
		iRet = RS232Dial(ucDialMode);
		break;
		
	case CT_BLTH:
		iRet = BTDial(ucDialMode);
		break;

	case CT_MODEM:
		iRet = PSTNDial(ucDialMode);
		break;

	case CT_WIFI:
		RouteSetDefault(12); //MEDC-10
		iRet = WIFIDial(ucDialMode);
		break;

	case CT_TCPIP:
		RouteSetDefault(0);
		iRet = TcpDial(ucDialMode);
	    break;

	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		RouteSetDefault(11);
		iRet = SXXWlDial(&sg_stCurCfg.stWirlessPara, 60, 3600, ucDialMode);
		break;

	case CT_DEMO:
		iRet = 0;
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
	}

	return iRet;
}

// Modified by Kim_LinHB 2014-5-31
// 通讯模块发送数据
// send data for all kinds of communication.
int CommTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	int		iRet;
	
	uchar bSSL = 0;
    uchar szSSL[120];

	if( psTxdData==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}

	
    if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
		logTrace("Is ssl: %d", bSSL);
    }
	
	switch( sg_stCurCfg.ucCommType )
	{
	case CT_RS232:
		iRet = RS232Txd(psTxdData, uiDataLen);
		break;
		
	case CT_BLTH:
		iRet = BTTxd(psTxdData, uiDataLen);
		break;

	case CT_MODEM:
		iRet = PSTNTxd(psTxdData, uiDataLen, uiTimeOutSec);
		break;

	case CT_WIFI:
		iRet = WIFITxd(psTxdData, uiDataLen, bSSL, uiTimeOutSec);
		break;
		
	case CT_TCPIP:
		iRet = TcpTxd(psTxdData, uiDataLen, bSSL, uiTimeOutSec);
		break;

	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		iRet = SXXWlSend(psTxdData, uiDataLen, bSSL, uiTimeOutSec);
		break;

	case CT_DEMO:
		iRet = 0;
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
	}

	return iRet;
}

// Modified by Kim_LinHB 2014-5-31
// 通讯模块接收数据
// receive for all kinds of communication.
int CommRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iRet;

	uchar bSSL = 0;
    uchar szSSL[120];
	
	if( psRxdData==NULL )
	{
		return ERR_COMM_INV_PARAM;
	}
	
    if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }

	switch( sg_stCurCfg.ucCommType )
	{
	case CT_RS232:
		iRet = RS232Rxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	case CT_BLTH:
		iRet = BTRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	case CT_MODEM:
		iRet = PSTNRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;
		

	case CT_WIFI:
		iRet = WIFIRxd(psRxdData, uiExpLen, uiTimeOutSec, bSSL, puiOutLen);
		break;

	case CT_TCPIP:
	    iRet = TcpRxd(psRxdData, uiExpLen, uiTimeOutSec, bSSL, puiOutLen);
		break;

	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		iRet = SXXWlRecv(psRxdData, uiExpLen, (ushort)(uiTimeOutSec), bSSL, puiOutLen);
		break;

	case CT_DEMO:
		PubWaitKey(2);
		iRet = 0;
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
	}

	return iRet;
}

// 通讯模块断开链路(MODEM挂机或者TCP断开TCP连接等)
// disconnect
// for modem it is OnHook, for GPRS/CDMA/WIFI/LAN it is closing socket.
int CommOnHook(uchar bReleaseAll)
{
	int		iRet;
	
	uchar bSSL = 0;
    uchar szSSL[120];
	
	if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }
	
	switch( sg_stCurCfg.ucCommType )
	{
	case CT_RS232:
		iRet = RS232OnHook(bReleaseAll);
		break;

	case CT_BLTH:
		iRet = BTOnHook(bReleaseAll);
		break;

	case CT_MODEM:
		iRet = PSTNOnHook(bReleaseAll);
		break;
   	case CT_WIFI:
		iRet = WIFIOnHook(bSSL, bReleaseAll);
		break;
	case CT_TCPIP:
		iRet = TcpOnHook(bSSL, bReleaseAll);
		break;

	case CT_WCDMA:
	case CT_CDMA:
	case CT_GPRS:
		iRet = SXXWlCloseTcp(bSSL);
		if (bReleaseAll)
		{
			SXXWlClosePPP();
		}
		break;

	case CT_DEMO:
		iRet = 0;
		break;

	default:
		iRet = ERR_COMM_INV_TYPE;
	}

	return iRet;
}
 
// Modified by Kim_LinHB 2014-5-31
// 获取通讯错误信息
// Retrieve error message for specific error code
void CommGetErrMsg(int iErrCode, COMM_ERR_MSG *pstCommErrMsg)
{
	if( pstCommErrMsg==NULL )
	{
		return;
	}

	if ((sg_stCurCfg.ucCommType==CT_TCPIP) || (sg_stCurCfg.ucCommType==CT_WIFI) 
		|| (sg_stCurCfg.ucCommType==CT_GPRS) || (sg_stCurCfg.ucCommType==CT_CDMA )
		|| (sg_stCurCfg.ucCommType==CT_WCDMA))
	{
		sprintf((char *)pstCommErrMsg->szMsg, "COMM ERR:%d", iErrCode);
		GetAllErrMsg(iErrCode, sg_stSxxErrMsg, pstCommErrMsg);
		return;
	}

	sprintf((char *)pstCommErrMsg->szMsg, "COMM ERR:%04X", iErrCode);

	switch( iErrCode & MASK_COMM_TYPE )
	{
	case ERR_COMM_ALL_BASE:
		GetAllErrMsg(iErrCode, sg_stCommErrMsg, pstCommErrMsg);
		break;

	case ERR_COMM_RS232_BASE:
		GetAllErrMsg(iErrCode, sg_stRS232ErrMsg, pstCommErrMsg);
		break;

	case ERR_COMM_MODEM_BASE:
		GetAllErrMsg(iErrCode, sg_stModemErrMsg, pstCommErrMsg);
	    break;

	case ERR_COMM_TCPIP_BASE:
		GetAllErrMsg(iErrCode, sg_stTCPErrMsg, pstCommErrMsg);
	    break;
	}
}

void GetAllErrMsg(int iErrCode, const ERR_INFO *pstInfo, COMM_ERR_MSG *pstCommErrMsg)
{
	int		iCnt;

	for(iCnt=0; pstInfo[iCnt].iErrCode!=0; iCnt++)
	{
		if( pstInfo[iCnt].iErrCode==iErrCode ||
			pstInfo[iCnt].iErrCode==(iErrCode & MASK_ERR_CODE) )
		{
			sprintf((char *)pstCommErrMsg->szMsg, "%.16s", pstInfo[iCnt].szMsg);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// RS232 通讯模块
// RS232 module
//////////////////////////////////////////////////////////////////////////
// Open port
int RS232Dial(uchar ucDialMode)
{
	uchar	ucRet;

	if( ucDialMode==DM_PREDIAL )
	{
		return 0;
	}

	ucRet = PortOpen(sg_stCurCfg.stRS232Para.ucPortNo,
					(void *)sg_stCurCfg.stRS232Para.szAttr);

	if( ucRet!=0 )
	{
		return (ERR_COMM_RS232_BASE | ucRet);
	}

	return 0;
}

// 串口发送数据
// send data in different predefined format
int RS232Txd(const uchar *psTxdData, ushort uiDataLen)
{
	int		iRet;

	switch( sg_stCurCfg.stRS232Para.ucSendMode )
	{
	case CM_RAW:
		iRet = RS232RawTxd(psTxdData, uiDataLen);
		break;

	case CM_SYNC:
		iRet = RS232NacTxd(psTxdData, uiDataLen);
		break;

	case CM_ASYNC:
		iRet = RS232AsyncTxd(psTxdData, uiDataLen);
		break;

	default:
		iRet = ERR_COMM_INV_PARAM;
	}

	return iRet;
}

// 串口直接发送
// send raw data
int RS232RawTxd(const uchar *psTxdData, ushort uiDataLen)
{
	uchar	ucRet;

	while( uiDataLen-->0 )
	{
		ucRet = PortSend(sg_stCurCfg.stRS232Para.ucPortNo, *psTxdData++);
		if( ucRet!=0 )
		{
			return (ERR_COMM_RS232_BASE | ucRet);
		}
	}

	return 0;
}

// send data in NAC format
// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
int RS232NacTxd(const uchar *psTxdData, ushort uiDataLen)
{
	int		iRet;

	if( uiDataLen>LEN_WORKBUF )
	{
		return ERR_COMM_TOOBIG;
	}

	sg_sWorkBuf[0] = STX;
	sg_sWorkBuf[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;	// convert to BCD
	sg_sWorkBuf[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
	memcpy(&sg_sWorkBuf[3], psTxdData, uiDataLen);
	sg_sWorkBuf[3+uiDataLen]   = ETX;
	sg_sWorkBuf[3+uiDataLen+1] = CalcLRC(psTxdData, uiDataLen, (uchar)(sg_sWorkBuf[1] ^ sg_sWorkBuf[2] ^ ETX));

	iRet = RS232RawTxd(sg_sWorkBuf, (ushort)(uiDataLen+5));	// data
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

// 串口异步发送
// send in async data format
int RS232AsyncTxd(const uchar *psTxdData, ushort uiDataLen)
{
	int		iRet;

	// 转换发送数据到工作缓冲区
	// convert format
	iRet = Conv2AsyncTxd(psTxdData, uiDataLen);
	if( iRet!=0 )
	{
		return iRet;
	}

	return RS232RawTxd(sg_sWorkBuf, (ushort)(uiDataLen+8));
}

// 串口接收
// receive data in different predefined format
int RS232Rxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iRet;

#ifdef _WIN32
	uiTimeOutSec = 5;
	DelayMs(500);
#endif

	switch( sg_stCurCfg.stRS232Para.ucSendMode )
	{
	case CM_RAW:
		iRet = RS232RawRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	case CM_SYNC:
		iRet = RS232NacRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	case CM_ASYNC:
		iRet = RS232AsyncRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	default:
		iRet = ERR_COMM_INV_PARAM;
	}

	return iRet;
}

// 串口直接接收
// receive raw data
int RS232RawRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	uchar   ucRet;
	ushort	uiReadCnt, uiTemp;

	uiReadCnt = uiTemp = 0;
	TimerSet(TIMER_TEMPORARY, (ushort)(uiTimeOutSec*10));
	while( uiReadCnt<uiExpLen )
	{
		if( TimerCheck(TIMER_TEMPORARY)==0 )
		{
			if( uiReadCnt>0 )	// received data already
			{
				break;
			}
			return ERR_COMM_TIMEOUT;
		}

		ucRet = PortRecv(sg_stCurCfg.stRS232Para.ucPortNo, psRxdData, uiTemp);
		if( ucRet==0x00 )
		{	// receive successfully, continue
			uiTemp = 80;
			psRxdData++;
			uiReadCnt++;
		}
		else if( ucRet==0xFF )
		{
			if( uiReadCnt>0 )
			{
				break;
			}
			return ERR_COMM_TIMEOUT;
		}
		else
		{	// got an error which is not timeout, return error code
			return (ERR_COMM_RS232_BASE | ucRet);
		}
	}   // end of while( uiReadCnt<uiExpLen

	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiReadCnt;
	}

	return 0;
}

// receive data in NAC format
// STX+Len1+Len2+strings+ETX+CKS, CKS = Len1 -- ETX (^)
int RS232NacRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iResult;
	uchar	ucRet;
	ushort	uiReadCnt, uiLength;

	if( uiExpLen>LEN_WORKBUF )
	{
		return ERR_COMM_TOOBIG;
	}

	uiReadCnt = uiLength = 0;
	memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));

	TimerSet(TIMER_TEMPORARY, 10);
	while( 1 )
	{
		iResult = UpdateTimer(TIMER_TEMPORARY, &uiTimeOutSec);
		if( iResult!=0 )
		{
			return iResult;
		}

		ucRet = PortRecv(sg_stCurCfg.stRS232Para.ucPortNo, &sg_sWorkBuf[uiReadCnt], 0);
		if( ucRet!=0 )
		{
			if( ucRet==0xFF )
			{
				continue;
			}
			return ERR_COMM_COMERR;
		}
		if( sg_sWorkBuf[0]!=STX )
		{
			continue;
		}

		uiReadCnt++;
		if( uiReadCnt==3 )
		{
			uiLength =  ((sg_sWorkBuf[1]>>4) & 0x0F) * 1000 + (sg_sWorkBuf[1] & 0x0F) * 100 +
						((sg_sWorkBuf[2]>>4) & 0x0F) * 10   + (sg_sWorkBuf[2] & 0x0F);
		}
		if( uiReadCnt==uiLength+5 )
		{	// read data ok, verify it ...
			if( sg_sWorkBuf[uiReadCnt-2]==ETX &&
			        CalcLRC(&sg_sWorkBuf[1], (ushort)(uiReadCnt-2), 0) == sg_sWorkBuf[uiReadCnt-1] )
			{
				break;
			}
			return ERR_COMM_COMERR;
		}
	}

	memcpy(psRxdData, &sg_sWorkBuf[3], uiLength);
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiLength;
	}

	return 0;
}

// 串口异步接收
// receive data in async format
int RS232AsyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iResult;
	uchar	ucRet, sCRC[4];
	ushort	uiLength, uiReadCnt, uiRetryCnt;

	if( uiExpLen>LEN_WORKBUF )
	{
		return ERR_COMM_TOOBIG;
	}

	uiReadCnt = uiLength = uiRetryCnt =0;
	memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));

	TimerSet(TIMER_TEMPORARY, 10);
	while( 1 )
	{
		iResult = UpdateTimer(TIMER_TEMPORARY, &uiTimeOutSec);
		if( iResult!=0 )
		{
			return iResult;
		}

		ucRet = PortRecv(sg_stCurCfg.stRS232Para.ucPortNo, &sg_sWorkBuf[uiReadCnt], 0);
		if( ucRet!=0 )
		{
			if( ucRet==0xFF )
			{
				continue;
			}
			return ERR_COMM_COMERR;
		}
		if( sg_sWorkBuf[0]!=STX )
		{
			continue;
		}

		uiReadCnt++;
		if( uiReadCnt==4 )
		{
			uiLength = (ushort)(sg_sWorkBuf[2]<<8 | sg_sWorkBuf[3]);
		}
		if( uiReadCnt!=uiLength+8 )
		{
			continue;
		}

		// 0xF0 is new ProTims command, erase welcome messages!
		if( sg_sWorkBuf[1]==0xF0 )
		{
			uiReadCnt = uiLength = 0;
			memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));
			continue;
		}

		// verify data
		CalcCRC32(&sg_sWorkBuf[1], (ushort)(uiLength+3), sCRC);
		if( sg_sWorkBuf[1]==0x80 && memcmp(sCRC, &sg_sWorkBuf[uiReadCnt-4], 4)==0 )
		{
			PortSend(sg_stCurCfg.stRS232Para.ucPortNo, ACK);
			break;
		}

		if( ++uiRetryCnt>3 )
		{
			return ERR_COMM_COMERR;
		}
		PortSend(sg_stCurCfg.stRS232Para.ucPortNo, NAK);
		uiReadCnt = uiLength = 0;
		memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));
	}

	memcpy(psRxdData, &sg_sWorkBuf[4], uiLength);
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiLength;
	}

	return 0;
}

// 串口关闭
// Close RS232 port
int RS232OnHook(uchar bReleaseAll)
{
	uchar	ucRet;

	ucRet = PortClose(sg_stCurCfg.stRS232Para.ucPortNo);
	if( ucRet==0 )
	{
		return 0;
	}

	return (ERR_COMM_RS232_BASE | ucRet);
}

//////////////////////////////////////////////////////////////////////////
// TCP 通讯模块
// TCP module
//////////////////////////////////////////////////////////////////////////
// 建立TCP连接
// setup TCP connection
int TcpDial(uchar ucDialMode)
{
	return SxxLANTcpDial(&sg_stCurCfg.stTcpIpPara, ucDialMode);
}



// 发送数据
// send data
int TcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	//Modified by Kevin_Wu, for supporting SSL
	return SxxTcpTxd(psTxdData, uiDataLen, bSSL, uiTimeOutSec);
}

// 接收数据
// receive data
int TcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen)
{
	//Modified by Kevin Wu, for supporting SSL
	return SxxTcpRxd(psRxdData, uiExpLen, uiTimeOutSec, bSSL, puiOutLen);
}

// 断开TCP连接
// Close socket
int TcpOnHook(uchar bSSL, uchar bReleaseAll)
{
	//Modified by Kevin Wu, for supporting SSL
	return SxxTcpOnHook(bSSL);
}

//////////////////////////////////////////////////////////////////////////
// WIFI 通讯模块
// WIFI module
//////////////////////////////////////////////////////////////////////////
// Added by Kim_LinHB 2014-08-19 v1.01.0004
static int WIFIDial(uchar ucDialMode)
{
	int		iRet;
	uchar	ucSecondIP;
	uchar   szIpFromDNS[25] = {0};
	
	uchar bSSL = 0;
    uchar szSSL[120];
	
	if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }

	// Added by Kim_LinHB 2014-08-22 v1.01.0004
	if(WifiCheck(NULL) <= 0)
		return -4; //WIFI_RET_ERROR_NOTCONN

	WIFIDispSignal();
		
	if(!(bSSL && (SxxSSLGetSocket() >=0)))
	{
		CommOnHook(FALSE);
	}

	ucSecondIP = FALSE;

TAG_RETRY_IP:

    // Connect to remote IP
    if (ucSecondIP)
    {
        if(!ChkIfValidIp(sg_stCurCfg.stWifiPara.stHost2.szIP)){
            iRet = DnsResolve(sg_stCurCfg.stWifiPara.stHost2.szIP, szIpFromDNS, sizeof(szIpFromDNS));
            if(RET_OK == iRet)
                iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)sg_stCurCfg.stWifiPara.stHost2.szPort), bSSL, ucDialMode, 8);
		}
		else
			iRet = SxxTcpConnect(sg_stCurCfg.stWifiPara.stHost2.szIP, (short)atoi((char *)sg_stCurCfg.stWifiPara.stHost2.szPort), bSSL, ucDialMode, 8);
    }
    else
    {
        if(!ChkIfValidIp(sg_stCurCfg.stWifiPara.stHost1.szIP)){
             iRet = DnsResolve(sg_stCurCfg.stWifiPara.stHost1.szIP, szIpFromDNS, sizeof(szIpFromDNS));
             if(RET_OK == iRet){
                 iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)sg_stCurCfg.stWifiPara.stHost1.szPort), bSSL, ucDialMode, 8);
             }
         }
         else{
             iRet = SxxTcpConnect(sg_stCurCfg.stWifiPara.stHost1.szIP, (short)atoi((char *)sg_stCurCfg.stWifiPara.stHost1.szPort), bSSL, ucDialMode, 8);
         }
    }

	if (iRet < 0)
	{
		if (!ucSecondIP)
		{
			if (ChkIfValidIp(sg_stCurCfg.stWifiPara.stHost2.szIP) && ChkIfValidPort(sg_stCurCfg.stWifiPara.stHost2.szPort) &&
				(strcmp((char *)(sg_stCurCfg.stWifiPara.stHost1.szIP), (char *)(sg_stCurCfg.stWifiPara.stHost2.szIP))!=0))
			{
				ucSecondIP = TRUE;
				goto TAG_RETRY_IP;
			}
		}
	}

	return iRet;
}

static int WIFITxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, uiDataLen, bSSL, uiTimeOutSec);
}

static int WIFIRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen)
{
	return  SxxTcpRxd(psRxdData, uiExpLen, uiTimeOutSec, bSSL, puiOutLen);
}

static int WIFIOnHook(uchar bSSL, uchar bReleaseAll)
{
	int iRet;
	
	iRet = SxxTcpOnHook(bSSL);
	if(bReleaseAll)
	{
#ifdef _MIPS_
		WifiDisconAp();
#else
		WifiDisconnect();
#endif
		iRet = WifiClose();
		//added by Kevin Liu 20160616
		ScrSetIcon(ICON_WIFI, CLOSEICON);
	}
	return iRet;
}

//added by Kevin Liu 20160616
// display the wifi signal
static void WIFIDispSignal(void)
{
	int	icRet;
	ST_WIFI_AP Ap;
	
	icRet = WifiCheck(&Ap);
	if( icRet < RET_OK )
	{
		ScrSetIcon(ICON_WIFI, CLOSEICON);
		return;
	}
	
	ScrSetIcon(ICON_WIFI, icRet);
}


//////////////////////////////////////////////////////////////////////////
// MODEM 通讯模块
// Modem module
//////////////////////////////////////////////////////////////////////////

// Send AT command before every dial action.
static ushort PSTNSendATCmdBeforeDial(void)
{
#ifndef WIN32
    ushort  usRet;
    uchar   szCmd[128];

    usRet = ModemExCommand("AT-STE=0",NULL,NULL,0);
    if (usRet!=0)
    {
        //PubDebugOutput(NULL, "AT-STE ERR\n", 12, DEVICE_COM1, ASC_MODE);
        return usRet;    
    }
    //PubDebugOutput(NULL, "AT-STE OK\n", 11, DEVICE_COM1, ASC_MODE);
    if (sg_stCurCfg.stPSTNPara.ucSignalLevel > 0)
    {
        sprintf((char *)szCmd, "ATS91=%d", (int)sg_stCurCfg.stPSTNPara.ucSignalLevel);
        usRet = ModemExCommand(szCmd,NULL,NULL,0);
        if (usRet!=0)
        {
            //PubDebugOutput(NULL, "ATS91 ERR\n", 11, DEVICE_COM1, ASC_MODE);
            return usRet;    
        }
        //PubDebugOutput(NULL, "ATS91 OK\n", 10, DEVICE_COM1, ASC_MODE);
    }
#endif

    return 0;
}
// modem 拨号
// modem dial
int PSTNDial(uchar ucDialMode)
{
	uchar	ucRet;

	// 处理预拨号
	// predial
	if( ucDialMode==DM_PREDIAL )
	{
		OnHook();
        PSTNSendATCmdBeforeDial();
		ucRet = ModemDial(&sg_stCurCfg.stPSTNPara.stPara,
			sg_stCurCfg.stPSTNPara.szTelNo, 0);
		if( ucRet!=0 )
		{
			return (ERR_COMM_MODEM_BASE | ucRet);
		}
		return 0;
	}

	// inquire pre-dialing result
	while( 1 )
	{
		ucRet = ModemCheck();
		if( ucRet==0x00 )
		{
			return 0;
		}
		if( ucRet!=0x0A )
		{
			break;
		}
	}

	// redial here.
	OnHook();
    PSTNSendATCmdBeforeDial();	
	ucRet = ModemDial(&sg_stCurCfg.stPSTNPara.stPara,
					sg_stCurCfg.stPSTNPara.szTelNo, 1);
	if( ucRet==0x00 )
	{
		if( sg_stCurCfg.stPSTNPara.ucSendMode==CM_ASYNC )
		{
			DelayMs(6000);
		}
		return 0;
	}

	return (ERR_COMM_MODEM_BASE | ucRet);
}

// Modem 发送
// modem send data
int PSTNTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	int		iResult;
	uchar	ucRet;

	switch( sg_stCurCfg.stPSTNPara.ucSendMode )
	{
	case CM_ASYNC:
		iResult = Conv2AsyncTxd(psTxdData, uiDataLen);
		if( iResult!=0 )
		{
			return iResult;
		}
		ucRet = ModemTxd(sg_sWorkBuf, (ushort)(uiDataLen+8));
		break;

	case CM_SYNC:
		ucRet = ModemTxd((uchar*)psTxdData, uiDataLen);
		break;

	default:
		return ERR_COMM_INV_PARAM;
	}
	if( ucRet!=0 )
	{
		return (ERR_COMM_MODEM_BASE | ucRet);
	}

	return 0;
}

// Modem receive data
int PSTNRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iRet;

	switch( sg_stCurCfg.stPSTNPara.ucSendMode )
	{
	case CM_ASYNC:
		iRet = PSTNAsyncRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	case CM_SYNC:
		iRet = PSTNSyncRxd(psRxdData, uiExpLen, uiTimeOutSec, puiOutLen);
		break;

	default:
		iRet = ERR_COMM_INV_PARAM;
	}

	return iRet;
}

// Modem sync receive
int PSTNSyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iResult;
	uchar	ucRet;
	ushort	uiLength;

	TimerSet(TIMER_TEMPORARY, 10);
	while( 1 )
	{
		iResult = UpdateTimer(TIMER_TEMPORARY, &uiTimeOutSec);
		if( iResult!=0 )
		{
			return iResult;
		}

		ucRet = ModemRxd(psRxdData, &uiLength);
		if( ucRet!=0x0C )
		{
			break;
		}
	}
	if( ucRet!=0 )
	{
		return (ERR_COMM_MODEM_BASE | ucRet);
	}
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiLength;
	}

	return 0;
}

// Modem async receive
int PSTNAsyncRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int		iResult;
	uchar	ucRet, sCRC[4];
	ushort	uiLength, uiReadCnt, uiRetryCnt;

	if( uiExpLen>LEN_WORKBUF )
	{
		return ERR_COMM_TOOBIG;
	}

	uiReadCnt = uiLength = uiRetryCnt =0;
	memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));

	TimerSet(TIMER_TEMPORARY, 10);
	while( 1 )
	{
		iResult = UpdateTimer(TIMER_TEMPORARY, &uiTimeOutSec);
		if( iResult!=0 )
		{
			return iResult;
		}

		ucRet = ModemAsyncGet(&sg_sWorkBuf[uiReadCnt]);
		if( ucRet!=0 )
		{
			continue;
		}
		if( sg_sWorkBuf[0]!=STX )
		{
			continue;
		}

		uiReadCnt++;
		if( uiReadCnt==4 )
		{
			uiLength = (ushort)(sg_sWorkBuf[2]<<8 | sg_sWorkBuf[3]);
		}
		if( uiReadCnt!=uiLength+8 )
		{
			continue;
		}

		// 0xF0 is new ProTims command, erase welcome messages!
		if( sg_sWorkBuf[1]==0xF0 )
		{
			uiReadCnt = uiLength = 0;
			memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));
			continue;
		}

		// verify data
		CalcCRC32(&sg_sWorkBuf[1], (ushort)(uiLength+3), sCRC);
		if( sg_sWorkBuf[1]==0x80 && memcmp(sCRC, &sg_sWorkBuf[uiReadCnt-4], 4)==0 )
		{
			ModemTxd((uchar *)"\x06", 1);	// ACK
			break;
		}

		if( ++uiRetryCnt>3 )
		{
			return ERR_COMM_COMERR;
		}
		ModemTxd((uchar *)"\x15", 1);		// NAK
		uiReadCnt = uiLength = 0;
		memset(sg_sWorkBuf, 0, sizeof(sg_sWorkBuf));
	}

	memcpy(psRxdData, &sg_sWorkBuf[4], uiLength);
	if( puiOutLen!=NULL )
	{
		*puiOutLen = uiLength;
	}

	return 0;
}

// Modem 挂机
// Modem onhook
int PSTNOnHook(uchar bReleaseAll)
{
	uchar	ucRet, ucCnt;

	for(ucCnt=0; ucCnt<3; ucCnt++)
	{
		ucRet = OnHook();
		if( ucRet==0 )
		{
			return 0;
		}
		DelayMs(50);
	}

	return (ERR_COMM_MODEM_BASE | ucRet);
}

//////////////////////////////////////////////////////////////////////////
// BT 通讯模块
// BT module
//////////////////////////////////////////////////////////////////////////

// Added by Kim_LinHB 2014-08-15 v1.01.0004
static int BTDial(uchar ucDialMode)
{
	int	iRet = 0;

	if( ucDialMode==DM_PREDIAL )
	{
		return 0;
	}

	if(IsBtOpened())
	{
		goto BT_DATA_C;
	}

#ifdef _MIPS_
	iRet = BtOpen(sg_stCurCfg.stBlueToothPara.stCommParam.ucPortNo,
		sg_stCurCfg.stBlueToothPara.stCommParam.szAttr);
BT_DATA_C:
#else
	iRet = BtOpen();
BT_DATA_C:
	PortOpen(COM_BT, NULL);
#endif

	return iRet;
}

static int BTTxd(const uchar *psTxdData, ushort uiDataLen)
{
	uchar	ucRet;

	PortReset(sg_stCurCfg.stBlueToothPara.stCommParam.ucPortNo);

	while( uiDataLen-->0 )
	{
		ucRet = PortSend(sg_stCurCfg.stBlueToothPara.stCommParam.ucPortNo, *psTxdData++);
		if( ucRet!=0 )
		{
			return (ERR_COMM_RS232_BASE | ucRet);
		}
	}

	return 0;
}

// 串口接收
// receive data in different predefined format
static int BTRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	uchar   ucRet;
	ushort	uiReadCnt, uiTemp;
	uchar	ucTimeOutCnt = 0;
	
	DelayMs(1000);

 	uiReadCnt = uiTemp = 0;
 	TimerSet(TIMER_TEMPORARY, (ushort)(uiTimeOutSec*10));
 	while( uiReadCnt<uiExpLen )
 	{
 		if( TimerCheck(TIMER_TEMPORARY)==0 )
 		{
 			if( uiReadCnt>0 )	// received data already
 			{
 				break;
 			}
 			return ERR_COMM_TIMEOUT;
 		}
 
 		ucRet = PortRecv(sg_stCurCfg.stBlueToothPara.stCommParam.ucPortNo, psRxdData, uiTemp);
 		if( ucRet==0x00 )
 		{	// receive successfully, continue
 			uiTemp = 80;
 			psRxdData++;
 			uiReadCnt++;
			ucTimeOutCnt = 0;
			TimerSet(TIMER_TEMPORARY, (ushort)(uiTimeOutSec*10));
 		}
 		else if( ucRet==0xFF)
 		{
 			if( uiReadCnt>0 && uiTemp > 0)
 			{
				if(ucTimeOutCnt < 10)
				{
					DelayMs(20);
					++ucTimeOutCnt;
					continue;
				}

 				break;
 			}
  			//return ERR_COMM_TIMEOUT;
 		}
 		else
 		{	// got an error which is not timeout, return error code
			return (ERR_COMM_RS232_BASE | ucRet);
 		}
 	}   // end of while( uiReadCnt<uiExpLen
 
 	if( puiOutLen!=NULL )
 	{
 		*puiOutLen = uiReadCnt;
 	}
 
 	return 0;
}

static int BTOnHook(uchar bReleaseAll)
{
#if defined(_Dxxx_) && !defined(_MIPS_)
	PortClose(COM_BT);
#endif
	if(bReleaseAll) 
	{
		return BtClose();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// GPRS/CDMA 通讯模块
// GPRS/CDMA module
//////////////////////////////////////////////////////////////////////////

#define WIRELESS_ERR(a)		(ERR_COMM_WIRELESS_BASE|(a))

//////////////////////////////////////////////////////////////////////////
// 公共数据转换模块
// Shared functions for data conversion
//////////////////////////////////////////////////////////////////////////
// 计算CRC32
// Calculate CRC32
void CalcCRC32(const uchar *psData, ushort uiLength, uchar sCRC[4])
{
	ulong	ulRSL, tl;
	ushort	ii;
	uchar	ucTemp, k;

	ulRSL = 0xFFFFFFFFL;
	for(ii=0; ii<uiLength; ii++)
	{
		ucTemp = (uchar)ulRSL;
		ucTemp = ucTemp^psData[ii];
		tl = (ulong)ucTemp;
		for(k=0; k<8; k++)
		{
			if( tl&1 )
			{
				tl = 0xedb88320L^(tl>>1);
			}
			else
			{
				tl = tl>>1;
			}
		}
		ulRSL = tl^(ulRSL>>8);
	}
	ulRSL ^= 0xFFFFFFFFL;

	sCRC[0] = (uchar)(ulRSL>>24);
	sCRC[1] = (uchar)(ulRSL>>16);
	sCRC[2] = (uchar)(ulRSL>>8);
	sCRC[3] = (uchar)(ulRSL);
}

// 计算LRC
// Calculate LRC
uchar CalcLRC(const uchar *psData, ushort uiLength, uchar ucInit)
{
	while( uiLength>0 )
	{
		ucInit ^= *psData++;
		uiLength--;
	}

	return ucInit;
}

// 转换为异步发送数据格式,并存储在sg_sWorkBuf中
// Convert to async format, then store in sg_sWorkBuf
int Conv2AsyncTxd(const uchar *psTxdData, ushort uiDataLen)
{
	if( uiDataLen>LEN_WORKBUF )
	{
		return ERR_COMM_TOOBIG;
	}

	sg_sWorkBuf[0] = STX;
	sg_sWorkBuf[1] = 0x80;
	sg_sWorkBuf[2] = (uchar)(uiDataLen>>8);
	sg_sWorkBuf[3] = (uchar)(uiDataLen & 0xFF);
	memcpy(&sg_sWorkBuf[4], psTxdData, uiDataLen);
	CalcCRC32(&sg_sWorkBuf[1], (ushort)(uiDataLen+3), &sg_sWorkBuf[4+uiDataLen]);

	return 0;
}

int UpdateTimer(uchar ucTimerNo, ushort *puiTickCnt)
{
	logd((__func__));
	if( TimerCheck(ucTimerNo)==0 )
	{
		if( *puiTickCnt==0 )
		{
			return ERR_COMM_TIMEOUT;
		}
		DispReceive();
		(*puiTickCnt)--;
		if( sg_stCurCfg.pfUpdWaitUI!=NULL )
		{
			(*sg_stCurCfg.pfUpdWaitUI)(*puiTickCnt);
		}
		TimerSet(ucTimerNo, 10);
	}

	return 0;
}

// Added by Kim_LinHB 2014-08-22 v1.01.0004

unsigned char IsBtOpened(void)
{
#ifdef _MIPS_
	if(BtIoCtrl(eBtCmdGetLinkStatus, NULL, 0, NULL, 0) >=0)
		return 1;
	return 0;
#else
	ST_BT_STATUS stSlave; 
	return (BtGetStatus(&stSlave) == 0x00);
#endif
}


// end of file
