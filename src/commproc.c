
#include "global.h"
#include "xui.h"
/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int  AskModemReDial(int iRet);
static int  ProcRecvPacket(void);
static void SaveRecvPackData(void);
static int  AdjustCommParam(void);
int DispCommErrMsg(int iErrCode);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void(*receiveDisplay)();

// 交易处理
// process transaction
int TranProcess(void)
{
	int		iRet, iRetryPIN;
	return FinishEmvTran();
}

// 预拨号
// pre-connect to host
int PreDial(void)
{
	int		iRet;

	if( !glSysParam.stEdcInfo.bPreDial )
	{
		return 0;
	}

	if( ChkOnBase()!=0 )
	{
		return 0;
	}

	iRet = AdjustCommParam();
	if( iRet!=0 )
	{
		return iRet;
	}

	return CommDial(DM_PREDIAL);
}

// 连接主机
// connect to host
int ConnectHost(void)
{
	int		iRet;

	while( 1 )
	{
		// 设置通信参数（从ACQ取得IP，电话号码）
		// set communication parameters(get IP address, tel NO. of acquirer, etc.)
		iRet = AdjustCommParam();
		if (iRet!=0)
		{
			if ((glCommCfg.ucCommTypeBak!=CT_NONE) && 
				(glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))		// switch to next connection
			{
				// switch to the backup communication type if the premier comm type is not existed
				glCommCfg.ucCommType = glSysParam.stTxnCommCfg.ucCommTypeBak;
				CommSwitchType(glSysParam.stTxnCommCfg.ucCommTypeBak);
				continue;
			}

			if( iRet!=ERR_NO_TELNO )
			{

				DispCommErrMsg(iRet);
				return ERR_NO_DISP;
			}
			return iRet;
		}

        SetOffBase(OffBaseDisplay);

		kbflush();
		DispDial();
		iRet = CommDial(DM_DIAL);
		if (iRet==0)
		{
			logTrace("Dial successful");
			return 0;
		}

		// 是否按过取消
		// if pressed cancel key
		if ((kbhit()==0) && (getkey()==KEYCANCEL))
		{
			return ERR_USERCANCEL;
		}

		// 如果第一套通信方式连接失败，则切换到备用通信方式
		// switch to the backup communication type if the premier comm type was failed
		if ((glCommCfg.ucCommTypeBak!=CT_NONE) && (glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))
		{
			glCommCfg.ucCommType = glCommCfg.ucCommTypeBak;
			CommSwitchType(glCommCfg.ucCommTypeBak);
			continue;
		}

		// 非Modem 错误，直接返回
		// return directly if it's not a Modem type error
		if( (iRet & MASK_COMM_TYPE)!=ERR_COMM_MODEM_BASE )
		{
			DispCommErrMsg(iRet);
			return ERR_NO_DISP;
		}

		// Modem 错误，询问是否重拨
		// it's a Modem type error, and ask to re-dial
		if (AskModemReDial(iRet))
		{
			return ERR_USERCANCEL;
		}
	}

	return 0;
}

// Modified by Kim_LinHB 2014-5-31
int AskModemReDial(int iRet)
{
	int	iResult;

	if( iRet==ERR_COMM_MODEM_OCCUPIED || 
		iRet==ERR_COMM_MODEM_NO_LINE ||
		iRet==ERR_COMM_MODEM_NO_LINE_2 )
	{
		iResult = DispCommErrMsg(iRet);
	}
	else{
		if( iRet==ERR_COMM_MODEM_LINE || 
			iRet==ERR_COMM_MODEM_NO_ACK ||
			iRet==ERR_COMM_MODEM_LINE_BUSY )
		{
			DispResult(iRet);

			Gui_ClearScr();
			iResult = (unsigned char)Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("RE DIAL ?"), gl_stCenterAttr, GUI_BUTTON_YandN, 5, NULL);
		}
		else
		{
			unsigned char szBuff[255];
			PubBeepErr();

			sprintf(szBuff, "%s\n   %02X", _T("DIAL FAIL,RETRY?"), (uchar)(iRet & MASK_ERR_CODE));
			
			Gui_ClearScr();
			iResult = (unsigned char)Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, 5, NULL);
		}
	}

	//if( iResult == GUI_ERR_TIMEOUT || iResult == GUI_ERR_USERCANCELLED )
	if(iResult != GUI_OK)
	{
		return ERR_USERCANCEL;
	}

	return 0;
}


// Modified by Kim_LinHB 2014-6-8 v1.01.0000
// voice referral dialing
int ReferralDial(const uchar *pszPhoneNo)
{
	uchar	ucRet, szTelNo[50];
	
	if( pszPhoneNo==NULL || *pszPhoneNo==0 )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("TEL# ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	sprintf((char*)szTelNo, "%s%s.", glSysParam.stEdcInfo.szPabx, pszPhoneNo);
	
	Gui_ClearScr();
	while( 1 )
	{
		OnHook();
#if !defined(WIN32)
		ModemExCommand("AT-STE=0",NULL,NULL,0);
#endif
		ucRet = ModemDial(NULL, szTelNo, 1);
		if( ucRet==0x83 )
		{	// 旁置电话、并线电话均空闲(仅用于发号转人工接听方式)
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("PLS USE PHONE"), gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);
		}
		DelayMs(1000);
		if( ucRet==0x06 || ucRet==0x00 || PubWaitKey(0)==KEYENTER )
		{
			return 0;
		}
		if( ucRet==0x0D )
		{
		
			OnHook();
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("LINE BUSY"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
			return ERR_NO_DISP;
		}
		PubWaitKey(1);
	}
	return 0;
}

// 调整通讯参数(只是设置Modem电话号码)
// get communication parameter from appropriate source, and set into glCommCfg
int AdjustCommParam(void)
{	
	glCommCfg = glPosParams.commConfig;
	return CommSetCfgParam(&glCommCfg);
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int DispCommErrMsg(int iErrCode)
{
	COMM_ERR_MSG	stCommErrMsg;

	unsigned char szBuff[200];

//	SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
	CommGetErrMsg(iErrCode, &stCommErrMsg);

	if (iErrCode<0)
	{
		sprintf(szBuff, "%s\n%d", _T(stCommErrMsg.szMsg), iErrCode);
	}
	else
	{
		sprintf(szBuff, "%s\n%02X", _T(stCommErrMsg.szMsg), (iErrCode & MASK_ERR_CODE));
	}

	Gui_ClearScr();
	PubBeepErr();
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 5, NULL);
}


int dialHost() {
	int haltMode = FALSE;
	int iRet = CommDial(DM_DIAL);
	logTrace("Initial CommDial::iRet=%d", iRet);
	if (iRet != 0)
	{
		//Retry
		if (iRet == -5) {
			//CommOnHook(true);
			CommOnHook(haltMode);
			DispDial();
			if (0 != (iRet = CommDial(DM_DIAL))) {
				DispCommErrMsg(iRet);
				logTrace("CommDial failed");
				CommOnHook(haltMode);
				return iRet;
			}
		}
		else {
			CommOnHook(haltMode);
			return iRet;
		}
	}

	return iRet;
}


/**
*
* @param dataIn
* @param inlen
* @param dataOut
* @param outlen
* @return
*/
int sendSocketRequest(char* dataIn, int inlen, char* dataOut, int* outlen) {
	logTrace(__func__);
	int iRet = -1;
	int haltMode = FALSE;

	DispDial();
	iRet = dialHost();
	if (iRet != 0)
	{
		return iRet;
	}

	logTrace("Host connected");
	DispSend();

#ifdef APP_DEBUG
	PubDebugOutput("ISO REQ:", dataIn, inlen, DEVICE_COM1, HEX_MODE);
#endif
	iRet = CommTxd(dataIn, inlen, glPosParams.requestTimeOutSec);	// "no timeout" is forbidden
	if (iRet != 0)
	{
		DispCommErrMsg(iRet);
		CommOnHook(haltMode);
		return ERR_NO_DISP;
	}

	if (receiveDisplay) {
		receiveDisplay();
	}
	else {
		DispReceive();
	}
	

	ushort uiTimeOut;
	//Added by Kim_LinHB 2014-6-6 v1.01.0000
	switch (glCommCfg.ucCommType)
	{
	case CT_WIFI:
	case CT_TCPIP:
		uiTimeOut = glPosParams.requestTimeOutSec;
		break;
	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:
		uiTimeOut = glPosParams.requestTimeOutSec;
		break;
	case CT_RS232:
	case CT_MODEM:
	case CT_BLTH:
	default:
		uiTimeOut = glPosParams.requestTimeOutSec;
		break;
	}

	*outlen = MAX(*outlen, LEN_MAX_COMM_DATA);
	memset(dataOut, 0, *outlen);
	iRet = CommRxd(dataOut, *outlen, uiTimeOut, outlen);
	if (iRet != 0)
	{
		DispCommErrMsg(iRet);
		CommOnHook(haltMode);
		return ERR_NO_DISP;
	}

	logTrace("Response: %s", dataOut);
	if (dataOut[0] == '0') {
		logTrace("Posvas mode");
		//Posvas nonesense. Sending Iso response without 2-byte header
		memmove(dataOut + 2, dataOut, *outlen);
		dataOut[0] = *outlen >> 8;
		dataOut[1] = *outlen;
		*outlen += 2;
	}

#ifdef APP_DEBUG
	//PubDebugOutput("ISO RESP:", dataOut, *outlen, DEVICE_COM1, HEX_MODE);
#endif

	CommOnHook(haltMode);
	return 0;
}

// end of file

