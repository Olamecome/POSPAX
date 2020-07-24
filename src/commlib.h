
/****************************************************************************
NAME
	commlib.h - ~{J5OV9+92M(Q6:/J}~}

DESCRIPTION
	1. ~{J5OV~}POS~{S&SC?*7"VP3#<{M(Q67=J=5DA,=S5D=(A"!"6O?*!"J}>]JU7"5H9&D\~}.
		implement comm feature, including connecting, disconnecting, sending/receiving, etc..
	2. ~{1>D#?iDZ2?U<SC6(J1Fw~}4#(~{=xHkD#?i:sIjGk~},~{MK3vD#?i:sJM7E~})
		this comm lib is using Timer 4#.

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	shengjx     2006.09.05      - created
****************************************************************************/

#ifndef _COMMLIB_H
#define _COMMLIB_H

#define MASK_COMM_TYPE			0xFF00		// Mask of getting error type
#define MASK_ERR_CODE			0x00FF		// Mask of getting error code

// Common error
#define ERR_COMM_ALL_BASE		0x0000		// Common error
#define ERR_COMM_INV_PARAM		0x0001		// Parameter error
#define ERR_COMM_INV_TYPE		0x0002		// Invalid Communication type
#define ERR_COMM_CANCEL			0x0003		// User Cancel
#define ERR_COMM_TIMEOUT		0x0004		// Communication Timeout
#define ERR_COMM_COMERR			0x0005
#define	ERR_COMM_TOOBIG			0x0006

//RS232 error
#define ERR_COMM_RS232_BASE		0x0100		// RS232 error

// Modem error
#define ERR_COMM_MODEM_BASE			0x0200		// Modem error
#define ERR_COMM_MODEM_OCCUPIED		(ERR_COMM_MODEM_BASE+0x02)
#define ERR_COMM_MODEM_NO_LINE		(ERR_COMM_MODEM_BASE+0x03)
#define ERR_COMM_MODEM_LINE			(ERR_COMM_MODEM_BASE+0x04)
#define ERR_COMM_MODEM_NO_ACK		(ERR_COMM_MODEM_BASE+0x05)
#define ERR_COMM_MODEM_LINE_BUSY	(ERR_COMM_MODEM_BASE+0x0D)
#define ERR_COMM_MODEM_NO_LINE_2	(ERR_COMM_MODEM_BASE+0x33)
#define ERR_COMM_MODEM_NO_PHONE_UP	(ERR_COMM_MODEM_BASE+0x83)

// TCPIP error
#define ERR_COMM_TCPIP_BASE		0x0300		// TCPIP error
#define ERR_COMM_TCPIP_OPENPORT	0x0301		// ~{4r?*6K?ZJ'0\~}
#define ERR_COMM_TCPIP_SETLIP	0x0302
#define ERR_COMM_TCPIP_SETRIP	0x0303
#define ERR_COMM_TCPIP_SETRPORT	0x0304
#define ERR_COMM_TCPIP_CONN		0x0305
#define ERR_COMM_TCPIP_TXD		0x0306
#define ERR_COMM_TCPIP_RXD		0x0307
#define ERR_COMM_TCPIP_SETGW	0x0308
#define ERR_COMM_TCPIP_SETMASK	0x0309

// GPRS/CDMA error
#define ERR_COMM_WIRELESS_BASE		0x0400		// GPRS/CDMA error

// Added by Kim_LinHB 2014-8-16 1.01.0004
#define ERR_COMM_BT_BASE			0x0500    // BT error


// Dialing mode
#define DM_PREDIAL			0		// Pre-dial/ Pre-link
#define DM_DIAL				1		// Dial/Link directly

// #define _WIRELESS_PRE_DIAL		// open for pre-connect of wireless module

// mode of sending data
#define CM_RAW				0		// Raw, For RS232
#define CM_SYNC				1		// synchronous
#define CM_ASYNC			2		// asynchronous

// for RS232 communication
#define STX             0x02
#define ETX             0x03
#define ENQ             0x05
#define ACK             0x06
#define NAK             0x15

typedef struct _tagIP_ADDR
{
    uchar       szIP[50 + 1];
    uchar       szPort[5+1];
}IP_ADDR;

// RS232 config
typedef struct _tagRS232_PARA
{
	uchar	ucPortNo;			// Port #, COM1, COM2 ....
	uchar	ucSendMode;			// mode of sending data
	uchar	szAttr[20+1];		// attribute, "9600,8,n,1", ....
}RS232_PARA;

// TCP/IP config
// TCPIP parameter
typedef struct _tagTCPIP_PARA
{
	uchar	ucDhcp;				// use DHCP : 1--TRUE, 0--FALSE
	uchar	ucPortNo;			// TCP/IP module port, COM1, COM2, TCPIP
	uchar	szNetMask[15+1];
	uchar	szGatewayIP[15+1];
	uchar	szLocalIP[15+1];
	IP_ADDR stHost1;
	IP_ADDR stHost2;
	uchar	szDNSIP[15+1];
}TCPIP_PARA;

// WIFI config
// WIFI parameter
typedef struct _tagWIFI_PARA
{
    IP_ADDR stHost1;
    IP_ADDR stHost2;
	ST_WIFI_PARAM stParam;
	ST_WIFI_AP stLastAP;
}WIFI_PARA;

// PSTN config
// PSTN parameter
typedef struct _tagPSTN_PARA
{
	uchar		ucSendMode;		// Mode of sending data
	uchar		szTelNo[100+1];	// Tel No.
	COMM_PARA	stPara;
    uchar       ucSignalLevel;  // dial voltage level, range 1-15. Use 0 for disable.
}PSTN_PARA;

// GPRS/CDMA configurations
typedef struct _tagWIRELESS_PARAM
{
    uchar       ucUsingSlot;
	uchar		szAPN[64+1];  ///CDMA: #777; GPRS: cmnet
	uchar		szUID[64+1];
	uchar		szPwd[16+1];
	uchar		szSimPin[16+1];     // SIM card PIN
	uchar		szDNS[32+1];
    IP_ADDR 	stHost1;
    IP_ADDR 	stHost2;
}WIRELESS_PARAM;

// error message of comm module
typedef struct _tagCOMM_ERR_MSG
{
	uchar		szMsg[16+1];
}COMM_ERR_MSG;


/****************************************************************************
 Function:		Refresh receiving screen (call once per sec,unsupported: RS232 raw)
 Param In:
	uiTimeLeft	time out, unit is second
 Param Out:		none
 Return Code:	none
****************************************************************************/
typedef void (*UpdWaitRspUI)(ushort uiTimeLeft);

typedef struct _tag_BT_PARAM{
	RS232_PARA		stCommParam;
	ST_BT_CONFIG	stConfig;
}BT_PARAM;

// config of comm module 
typedef struct _tagCOMM_CONFIG
{
#define CT_NONE		0			// unused
#define CT_RS232	1			// RS232
#define CT_MODEM	2			// Modem
#define CT_TCPIP	3			// TCP/IP
#define	CT_CDMA		4			// CDMA(RFU)
#define CT_GPRS		5			// GPRS(RFU)
#define CT_WIFI		6			// WIFI(RFU)
#define CT_BLTH		7			// BLUETOOTH(RFU) Bluetooth hdadd
#define CT_USB      8           // BLUETOOTH(RFU) Bluetooth hdadd
#define CT_WCDMA    9           // WCDMA(RFU)  added by Gillian

#define CT_DEMO		0xFF		// for debug/training/demo (offline)
	uchar			ucCommType;		// (RS232/modem/tcp...)
	uchar			ucCommTypeBak;	// (RS232/modem/tcp...)

	UpdWaitRspUI	pfUpdWaitUI;	// function for refresh receiving screen

	uchar			ucTCPClass_BCDHeader;	// For all TCPIP class: the length bytes are in BCD format or not
	uchar			ucPortMode;				// 1 => SSL , 0 => Open
	char			szPabx[10+1];
	uchar			bPreDial;

	RS232_PARA		stRS232Para;	// RS232
	PSTN_PARA		stPSTNPara;		// modem
	TCPIP_PARA		stTcpIpPara;	// TCP
	WIRELESS_PARAM  stWirlessPara;  // GRPS/CDMA
	WIFI_PARA		stWifiPara;		// Wifi//hdadd // Modified by Kim_LinHB 2014-08-19 v1.01.0004
	BT_PARAM		stBlueToothPara;	// Bluetooth hdadd
}COMM_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 Function:		Init comm module, and it just for wireless module
 Param In:
				pstComCfg
 Param Out:		none
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommInitModule(COMM_CONFIG *pstCfg);

/****************************************************************************
 Function:		Set comm config
 Param In:
				pstComCfg
 Param Out:		none
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommSetCfgParam(const COMM_CONFIG *pstCfg);

/****************************************************************************
 Function:		check if the specific Tel No. is matched Numbers saved
 Param In:
	pszTelNo	TEL No.
 Param Out:		none
 Return Code:
				TRUE		matched
				FALSE		mismatched
****************************************************************************/
int CommChkIfSameTelNo(const uchar *pszTelNo);

/****************************************************************************
 Function:		Switch comm type
 Param In:
				ucCommType	
 Param Out:		none
 Return Code:
				None
****************************************************************************/
void CommSwitchType(uchar ucCommType);

/****************************************************************************
 Function:		Dialing (e.g. Modem dialing or create a TCP link, etc.)
 Param In:
				ucDialMode	
					DM_PREDIAL:	return at once after Dialing
					DM_DIAL:	return after making a link
 Param Out:		none
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommDial(uchar ucDialMode);

/****************************************************************************
 Function:		send data
 Param In:
				psTxdData	data
				uiDataLen	data length
				ulTimeOut	time out, uint is second
 Param Out:		none
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommTxd(const uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec);

/****************************************************************************
 Function:		receive data
 Param In:
				uiExpLen	the expected data length
				ulTimeOut	time out, uint is second
 Param Out:
				psRxdData	data received
				puiOutLen	length of data received
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen);

/****************************************************************************
 Function:		disconnect the link
 Param In:
				bReleaseAll	
					TRUE: disconnect both data link & physical link,
					FALSE: disconnect data link
 Param Out:		none
 Return Code:
				0			OK
				other		failed
****************************************************************************/
int CommOnHook(uchar bReleaseAll);

/****************************************************************************
 Function:			get error message
 Param In:
					iErrCode		error code
 Param Out:			
					pstCommErrMsg	error code
 Return Code:		none
****************************************************************************/
void CommGetErrMsg(int iErrCode, COMM_ERR_MSG *pstCommErrMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CommLIB_H

// end of file
