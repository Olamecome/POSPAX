/****************************************************************************
NAME
	SxxCom.h - 无线模块封装函数

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)

****************************************************************************/

#ifndef _SWIRELESS_H
#define _SWIRELESS_H

#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef uint
#define uint    unsigned int
#endif
#ifndef ushort
#define ushort  unsigned short
#endif
#ifndef ulong
#define ulong   unsigned long
#endif

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif



/****************************************************************************
 功    能：	TCP/IP 发送数据。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
 传入参数：
			psTxdData：指向数据缓冲区
			uiDataLen：数据长度
			iTimeout：发送超时时间。不足3秒按3秒设置
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SxxTcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec);


/****************************************************************************
 功    能：	TCP/IP 接收数据。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
 传入参数：
			psRxdData：指向数据接收缓冲区
			uiExpLen：缓冲区可接收的最大数据长度
			iTimeout：接收超时时间。不足3秒按3秒设置
			puiOutLen：实际收到数据的长度
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SxxTcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen);


/****************************************************************************
 功    能：	TCP/IP 关闭socket。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
 传入参数：
			无
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SxxTcpOnHook(uchar bSSL);


/****************************************************************************
 Function:		S系列的DHCP执行
 Param In:
	ucForceStart	bool,是否强制执行
	ucTimeOutSec	超时时间
 Param Out:		none
 Return Code:
	0			OK
	other		fail
****************************************************************************/
int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec);


/****************************************************************************
 Function:		S系列LAN的TCP连接
 Param In:
	pstTcpPara：所使用的TCPIP参数
 Param Out:		none
 Return Code:
	0			OK
	other		fail
****************************************************************************/
int SxxLANTcpDial(const TCPIP_PARA *pstTcpPara, uchar ucDialMode);


/****************************************************************************
 功    能：	初始化无线模块，开机后只需调用一次。
 传入参数：
			pstWlPara：用于初始化的参数  
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SXXWlInit(const WIRELESS_PARAM *pstWlPara);


/****************************************************************************
 功    能：	先判断PPP链路是否联通，若没有联通先进行PPP链接，成功后再进行TCP连
            接；若已联通则直接进行TCP连接。
 传入参数：
			pstWlPara：用于初始化的参数
 			iTimeOut：进行PPP连接时的超时时间，进行TCP连接时无用，单位：秒。
			iAliveInterval：PPP链路保持激活的时间，单位：秒。
			ucPredial：是否预拨号
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SXXWlDial(const WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval, uchar ucPredial);


/****************************************************************************
 功    能：	发送数据。
 传入参数：
			psTxdData：待发送的数据。
			uiDataLen：待发送数据的长度。
			usTimeOut：发送数据超时时间，单位：秒。
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SXXWlSend(const uchar *psTxdData, ushort usDataLen, uchar bSSL, ushort uiTimeOutSec);


/****************************************************************************
 功    能：	接收数据。
 传入参数：
			uiExpLen：期望接收的最大数据长度。
			uiTimeOut：等待接收数据的超时时间，单位：秒。
 传出参数：
			psRxdData：接收到的数据。
			puiOutLen：接收到数据的长度。
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SXXWlRecv(uchar *psRxdData, ushort usExpLen, ushort usTimeOut, uchar bSSL, ushort *pusOutLen);


/****************************************************************************
 功    能：	关闭TCP连接。
 传入参数：
			无  
 传出参数：
			无
 返 回 码：	0   成功
            <0  失败
****************************************************************************/
int SXXWlCloseTcp(uchar bSSL);


/****************************************************************************
 功    能：	关闭PPP链路。
 传入参数：
			无  
 传出参数：
			无
 返 回 码：	无
****************************************************************************/
void SXXWlClosePPP(void);


/****************************************************************************
 功    能：	显示信号强度。
 传入参数：
			无  
 传出参数：
			无
 返 回 码：	无
****************************************************************************/
void SXXWlDispSignal(void);

/****************************************************************************
 功    能：	建立TCP连接
 传入参数：
			pszIP：IP。
			sPort：端口。
			iTimeout：发送数据超时时间，单位：秒。  
 传出参数：
			无
 返 回 码：	0   成功
			<0  失败
****************************************************************************/
int SxxTcpConnect(const char *pszIP, short sPort, uchar bSSL, uchar ucDialMode, int iTimeout);

#endif
